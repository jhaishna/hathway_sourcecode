/*******************************************************************
 *
* Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_tax_calc.c /cgbubrm_7.5.0.portalbase/3 2016/05/26 05:35:49 ansiddha Exp $ ";
#endif

/************************************************************************
 * Contains the following operations:
 *
 *   PCM_OP_CUST_POL_TAX_CALC - can be used to calculate taxes using those
 *                              custom tax rates.
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_rate.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "mso_cust.h"

// MSO Customiztion Start
#include "ops/bill.h"
#include "mso_ops_flds.h"
#define SERVICE_TAX "Service_Tax"
#define EDUCATION_CESS "Education_Cess"
#define SHE_CESS "Secondary_Higher_Education_Cess"
#define VAT_PLUS_SERVICE_TAX "VAT_ServiceTax"
#define VAT "VAT"
#define IGST "IGST"
#define CGST "CGST"
#define SGST "SGST"

#define MSO_SRVC_TAX "MSO_Service_Tax"
#define MSO_VAT "MSO_VAT"
#define MSO_VAT_ST "MSO_VAT_ST"
#define MSO_ET	"MSO_ET"
#define MSO_ET_MAIN "MSO_ET_MAIN"
#define MSO_ET_ADDI "MSO_ET_ADDI"
#define MAIN_CONN "MAIN"
#define ADDI_CONN "ADDI"

#define BILLINFO_ID_CATV "CATV"
#define BILLINFO_ID_BB "BB"

#define ITEMS_STAT_AVAIL 0
#define ITEMS_STAT_PENDING 1
#define ITEMS_STAT_OPEN 2
#define ITEMS_STAT_CLOSED 4

#define ITEM_TYPE_DEPOSIT  "/item/payment/mso_deposit"
#define ITEM_TYPE_CYC_ARR_ET  "/item/cycle_arrear/mso_et"
#define ITEM_TYPE_CYC_ARR_ANNUAL_AMC  "/item/cycle_arrear/mso_hw_amc"
#define ITEM_TYPE_CYC_FWD_AMC "/item/cycle_forward/mso_hw_amc"
#define ITEM_TYPE_MISC  "/item/misc"
#define ITEM_TYPE_PUR_ATP  "/item/mso_purchase_atp"
#define ITEM_TYPE_PUR_OTP  "/item/mso_purchase_outright"
#define ITEM_TYPE_PUR_ADN_FDP  "/item/mso_purchase_adn_fdp"
#define ITEM_TYPE_PUR_HW  "/item/cycle_arrear/mso_sb_onetime_hw"
#define ITEM_TYPE_PUR_SRVC  "/item/cycle_arrear/mso_sb_onetime_srvc"
#define ITEM_TYPE_CYC_ARR_RENT "/item/cycle_arrear/mso_hw_rental"
#define ITEM_ST "/item/Service_Tax"
#define ITEM_ECESS "/item/Education_Cess"
#define ITEM_SHECESS "/item/Secondary_Higher_Education_Cess"
#define ITEM_VAT "/item/VAT"
#define ITEM_MIN_COMMIT "/item/mso_catv_commitment"
#define EVT_ACCT_ADJ "/event/billing/adjustment/account"
#define ITEM_WS_SETTLEMENT "/item/settlement/ws"
#define ITEM_ADJ "/item/adjustment"
#define ITEM_PAYMENT "/item/payment"
#define ITEM_PYMT_REVERSAL "/item/payment/reversal"
#define ITEM_GST "/item/gst"

extern pin_flist_t *VAT_Table;
extern pin_flist_t *ET_Table;
extern cm_cache_t fm_cust_pol_gst_rate_ptr;


static char* mso_retrieve_vat_code(
	pcm_context_t	*ctxp,
	char			*taxcode,
	poid_t			*acct_pdp,
	poid_t			*balgrp_pdp,
	pin_errbuf_t	*ebufp);

static void 
mso_vat_apply_tax(
	pcm_context_t	*ctxp,
	char			*taxcode,
	pin_decimal_t	*amt_gross,
	pin_flist_t		**out_flistpp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_EXPORT void mso_retrieve_et_code(
	pcm_context_t	*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_EXPORT void 
mso_et_apply_tax(
	pcm_context_t	*ctxp,
	char			*taxcode,
	pin_decimal_t	*amt_gross,
	pin_decimal_t	*scale,
	pin_flist_t		**out_flistpp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void mso_retrieve_catv_billed_items(
	pcm_context_t	*ctxp,
    poid_t			*balgrp_pdp,
	poid_t			*service_pdp,
    pin_flist_t		**r_flistpp,
    pin_errbuf_t	*ebufp);
	
	
PIN_EXPORT 
void mso_retrieve_gst_rate(
	pcm_context_t	*ctxp,
	char		*tax_code,
	poid_t		*acct_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

static void 
mso_gst_apply_tax(
	pcm_context_t	*ctxp,
	pin_decimal_t	*tax_rate,
	pin_decimal_t	*amt_gross,
	pin_flist_t		**out_flistpp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

// MSO Customization End

/************************************************************************
 * Forward declarations.
 ************************************************************************/
EXPORT_OP void
op_cust_pol_tax_calc(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);

void
fm_cust_pol_tax_calc(
	cm_nap_connection_t	*connp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_build_subtotal(
	pin_flist_t		*os_flistp,
	int32			j_level,
	pin_decimal_t		*tax_amt,
	pin_decimal_t		*tax_rate,
	pin_decimal_t		*amt_taxed,
	pin_decimal_t		*amt_exempt,
	pin_decimal_t		*amt_gross,
	char			*name,
	char			*descr,
	int32			loc_mode,
	int32			precision,
	int32			round_mode,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_do_juris_only(
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);

static pin_decimal_t *
fm_cust_pol_get_amt_exempt(
	pin_flist_t		*in_flistp,
	int32			j_level,
	pin_decimal_t		*g_amt,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_parse_tax_locale(
	char			*locale,
	char			*city,
	char			*state,
	char			*zip,
	char			*country,
	char			*code,
	size_t			city_len,
	size_t			state_len,
	size_t			zip_len,
	size_t			country_len,
	size_t			code_len);

static int32 
fm_cust_pol_is_locale_in_juri_list(
	char		*j_list,
	char		*locale);
PIN_IMPORT void
fm_mso_utils_get_profile(
        pcm_context_t   *ctxp,
        poid_t          *account_pdp,
        pin_flist_t     **ret_flistpp,
        pin_errbuf_t    *ebufp);
/* Symbol defines */
#define STR_LEN		480
#define TOK_LEN		80
#define RatePrecision	6

/* Global variable (from fm_cust_pol_tax_init)  */
extern pin_flist_t *Tax_Table;
PIN_IMPORT void fm_utils_tax_get_taxcodes();


/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_TAX_CALC command
 *******************************************************************/
void
op_cust_pol_tax_calc(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != PCM_OP_CUST_POL_TAX_CALC) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_tax_calc opcode error", ebufp);
		return;
	}

	/*
	 * Debug - What we got.
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_tax_calc input flist", in_flistp);

	/*
	 * Do the actual op in a sub. 
	 */
	fm_cust_pol_tax_calc(connp, in_flistp, out_flistpp, ebufp);

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_tax_calc error", ebufp);
	} else {
		/*
		 * Debug: What we're sending back.
		 */
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_tax_calc return flist", *out_flistpp);
	}

	return;
}


/*******************************************************************
 * fm_cust_pol_tax_calc()
 *
 * Use this policy to do your tax calculation using the tax data 
 * that was loaded into the internal data structure that was 
 * declared in fm_cust_pol_tax_init.
 *
 *******************************************************************/
void 
fm_cust_pol_tax_calc(
	cm_nap_connection_t	*connp,
	pin_flist_t	*in_flistp,
	pin_flist_t	**out_flistpp,
	pin_errbuf_t	*ebufp)
{
	pcm_context_t       *ctxp = connp->dm_ctx;
	cm_op_info_t        *opstackp = connp->coip;
	pin_flist_t	*t_flistp = NULL;   /* in taxes array */
	pin_flist_t	*ot_flistp = NULL;  /* out taxes array */
	pin_flist_t	*os_flistp = NULL;  /* out subtotal array */
	pin_flist_t	*flistp = NULL;
	pin_flist_t			*r_flistp = NULL;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t	cookie2 = NULL;
	int32				rec_id1 = 0;
	int32		elemid = 0;
	int32		elemid2 = 0;
	int32		elemcnt = 0;
	int32		j_level = 0;
	char		*j_list = NULL;
	char		*descr = NULL;
	char		*str_tax_rate = NULL;
	void		*vp = NULL;
	char		*taxcode = NULL;
	char		*locale = NULL;
	char		city[TOK_LEN];
	char		state[TOK_LEN];
	char		zip[TOK_LEN];
	char		county[TOK_LEN];
	char		country[TOK_LEN];
	char		code[TOK_LEN];
	char		name[STR_LEN];
	char		msg[STR_LEN];
	pin_decimal_t	*tax_rate=NULL; 
	pin_decimal_t	*tot_tax=NULL;
	pin_decimal_t	*tax_amt=NULL;
	pin_decimal_t	*amt_gross=NULL;
	pin_decimal_t	*amt_taxed = NULL;
	pin_decimal_t	*amt_exempt = NULL;
	pin_decimal_t		*scale = NULL;
	pin_decimal_t		*temp1 = NULL;
	pin_decimal_t		*service_tax_amount = NULL;
	pin_decimal_t		*item_total = NULL;
	pin_decimal_t		*temp = NULL;
	pin_decimal_t		*gst_rate = NULL;
	
	pin_flist_t			*item_flistp = NULL;
	pin_flist_t			*bill_tax_iflistp = NULL;
	pin_flist_t			*event_flistp = NULL;
	pin_flist_t			*et_in_flistp = NULL;
	pin_flist_t			*et_out_flistp = NULL;
	pin_flist_t			*rs_flistp = NULL;
	pin_flist_t			*cycle_info_flistp = NULL;
	pin_flist_t			*gst_rate_flistp = NULL;
	 pin_flist_t                     *main_tax_flistp = NULL;
	int32 		tax_pkg = PIN_RATE_TAXPKG_CUSTOM;
	int32		tax_rule = 0;
	int32		loc_mode = 0;
	int32		precision = 6;
	int32				*taxrule = NULL;
	int32		round_mode = ROUND_HALF_UP;
	time_t		*startTm = NULL;
	time_t		*endTm = NULL;
	time_t		*transDt = NULL;
	time_t		now = 0;
	poid_t				*service_pdp = NULL;
	poid_t				*acct_pdp = NULL;
	poid_t				*balgrp_pdp = NULL;
	poid_t				*item_pdp = NULL;
	char				*taxcode_old = NULL;
	char				*item_type = NULL;
	char				*event_type = NULL;
	char				*main_tax_code = NULL;
	pin_flist_t                     *events_flistp = NULL;
    pin_decimal_t                   *new_scale = NULL;
    pin_decimal_t                   *subscription_amt = NULL;
    pin_decimal_t                   *accumulated_tax_amt = NULL;
	int32				*subscription_fee_flag = NULL;
	int32				*fdp_active = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	opstackp = connp->coip;	
	opstackp = connp->coip;	
	while(opstackp != NULL)
	{
		if(opstackp->opcode == PCM_OP_BILL_TAX_EVENT)
		{
			bill_tax_iflistp = opstackp->in_flistp;
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "bill_tax_iflistp input flist", bill_tax_iflistp);
			event_flistp = PIN_FLIST_SUBSTR_GET(bill_tax_iflistp,PIN_FLD_EVENT,1, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "event flist", event_flistp);
			service_pdp = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_SERVICE_OBJ,1,ebufp);

			cycle_info_flistp = PIN_FLIST_SUBSTR_GET(event_flistp,PIN_FLD_CYCLE_INFO,1, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "cycle info flist", cycle_info_flistp);
			if(cycle_info_flistp)
			{
				temp1 = (pin_decimal_t *)PIN_FLIST_FLD_GET(cycle_info_flistp,PIN_FLD_SCALE,1, ebufp);
				if (temp1)
				{
					scale = (pin_decimal_t *)pbo_decimal_round(temp1, 0, ROUND_UP, ebufp);
				}
				else
				{
					scale = pbo_decimal_from_str("1.0", ebufp);
				}
			}
			// For Adjustments we can ignore the Service Poid, hence resetting the EBUF
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,	"fm_cust_pol_tax_calc Service Retrieval error", ebufp);
				PIN_ERRBUF_CLEAR(ebufp);
			}
			PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Service POID: ", service_pdp);
		}
		opstackp = opstackp->next;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "updated bill_tax_iflistp input flist", bill_tax_iflistp);

	service_tax_amount = pbo_decimal_from_str("0.0",ebufp);
	/* Create the return flist */
	*out_flistpp = PIN_FLIST_CREATE(ebufp);

	/* Set the account POID */
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, (void *)vp, ebufp);

	/* check for jurisdiction only ? */
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_COMMAND, 1, ebufp);
	if ((vp != (void*)NULL) && (*(int32*)vp == PIN_CUST_TAX_VAL_JUR)) {
		fm_cust_pol_do_juris_only(in_flistp, out_flistpp, ebufp);
		goto CleanUp;
		/***********/
        }

	/* get the transaction date = END_T */
	transDt = (time_t*) PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_END_T,
		1, ebufp);
	if (transDt == (time_t*)NULL) {
		/* not available. use current time */
		now = pin_virtual_time((time_t*)NULL);
		transDt = &now;
	}

	/* get precision, if any */
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ROUNDING, 1, ebufp);
        if (vp != (void*)NULL) {
		precision = *(int32*)vp;
	}

	/* get rounding mode, if any */
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ROUNDING_MODE, 1, ebufp);
	if (vp != (void*)NULL) {
		round_mode = *(int32*)vp;
	}
	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	event_type = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);
	main_tax_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_TAXES, 0, 0, ebufp); 
	main_tax_code = PIN_FLIST_FLD_GET(main_tax_flistp, PIN_FLD_TAX_CODE, 0, ebufp); 

        if(main_tax_code && strcmp(main_tax_code, "") != 0 && strcmp(main_tax_code, MSO_ET)!= 0 && strcmp(main_tax_code, MSO_ET_MAIN) != 0 &&
                strcmp(main_tax_code, MSO_ET_ADDI) != 0)
	{
		mso_retrieve_gst_rate(ctxp, main_tax_code, acct_pdp, &gst_rate_flistp, ebufp);
	}
	/* Loop through array of incoming taxes */
	while ((t_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_TAXES,
		&elemid, 1, &cookie, ebufp)) != (pin_flist_t*)NULL) {

		/* initialize */
		tot_tax = pbo_decimal_from_str("0.0",ebufp);
		tax_amt = (pin_decimal_t*)NULL;
		tax_rate = (pin_decimal_t*)NULL;
		amt_gross = (pin_decimal_t*)NULL;
		amt_taxed = (pin_decimal_t*)NULL;
		amt_exempt = (pin_decimal_t*)NULL;
		j_level = elemcnt = 0;
		descr = "Zero Taxes"; 
		loc_mode = 0;

		/* add a tax array into the return flist */
		ot_flistp = PIN_FLIST_ELEM_ADD(*out_flistpp, 
			PIN_FLD_TAXES, elemid, ebufp);

		/* Add package type to the taxes array */
		PIN_FLIST_FLD_SET(ot_flistp, PIN_FLD_TAXPKG_TYPE,
			(void*)&tax_pkg, ebufp);

		/* add the total taxes to the taxes array */
		PIN_FLIST_FLD_PUT(ot_flistp, PIN_FLD_TAX, 
			(void*)pbo_decimal_round (tot_tax,
			(int32)precision,round_mode,ebufp), ebufp);

		/* get taxcode */
		taxcode = (char*) PIN_FLIST_FLD_GET(t_flistp,
			PIN_FLD_TAX_CODE, 1, ebufp);
		if (taxcode == (char*)NULL) {
			/* no taxcode? then skip */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"No taxcode!  Skipping ...");
			continue;
			/*******/
		}
		sprintf(msg,"MSO Taxcode is : %s ",	taxcode);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Testing Flist", t_flistp);

		/* get tax locale = PIN_FLD_SHIP_TO */
		locale = (char*) PIN_FLIST_FLD_GET(t_flistp,
			PIN_FLD_SHIP_TO, 1, ebufp);
		if (locale == (char*)NULL) {
			/* no tax locale? then skip */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"No SHIP_TO locale!  Skipping ...");
			continue;
			/*******/
		}

		/* get location mode */
		vp =  PIN_FLIST_FLD_GET(t_flistp,
			PIN_FLD_LOCATION_MODE, 1, ebufp);
		if (vp) {
			loc_mode = *(int32 *)vp;
		}
		/* initialize */
		city[0] = county[0] = state[0] = zip[0] = country[0] = '\0';
		code[0] = name[0] = '\0'; 

		/* parse the locale */
		fm_cust_pol_parse_tax_locale(locale, city, state, zip,
			country, code, sizeof(city), sizeof(state),
			sizeof(zip), sizeof(country), sizeof(code));

		/* get taxable amount */
		vp = PIN_FLIST_FLD_GET(t_flistp, 
			PIN_FLD_AMOUNT_TAXED, 1, ebufp);
		if (vp == (void*)NULL) {
			/* no amount?  default to zero */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"No AMOUNT_TAXED!  Defaulting to 0.");
			amt_gross = pbo_decimal_from_str("0.0",ebufp);
		} else {
			amt_gross = pbo_decimal_copy((pin_decimal_t*)vp, ebufp);
		}

		/* construct the taxed jurisdiction */
		if (*code) {
			sprintf(name, "%s; %s; %s; %s; %s; [%s]",
				country, state, city, county, zip, code);
		} else {
			sprintf(name, "%s; %s; %s; %s; %s",
				country, state, city, county, zip);
		}
		if (taxcode && (strcmp(taxcode, IGST)==0 || strcmp(taxcode, CGST)==0 || strcmp(taxcode, SGST)==0))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "IGST Matched");
			//balgrp_pdp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
			if(strcmp(taxcode, IGST)==0){
				gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_IGST_RATE, 0, ebufp);	//IGST
			}
			else if(strcmp(taxcode, SGST)==0){
				gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_SGST_RATE, 0, ebufp);	//SGST
			}
			else if(strcmp(taxcode, CGST)==0){
				gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_CGST_RATE, 0, ebufp);			//CGST
			}

			sprintf(msg,"Tax Rate is : %s ", pbo_decimal_to_str(gst_rate, ebufp));

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Before Applying GST", ot_flistp);
			mso_gst_apply_tax(ctxp, gst_rate, amt_gross, &ot_flistp, &r_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After Applying GST", ot_flistp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After Applying GST", r_flistp);

			tax_amt = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_TAX, 1, ebufp);
			tax_rate = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PERCENT, 1, ebufp);
			amt_gross = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_AMOUNT_GROSS, 1, ebufp);
			amt_taxed = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_AMOUNT_TAXED, 1, ebufp);
			descr = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_DESCR, 1, ebufp);
			vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_TYPE, 1, ebufp);
			if (vp) {
				j_level = *(int32 *)vp;
			}
			else {
				j_level = 0;
			}
			str_tax_rate = pbo_decimal_to_str(tax_rate,ebufp);

			os_flistp = PIN_FLIST_ELEM_ADD(ot_flistp, PIN_FLD_SUBTOTAL, elemcnt++, ebufp);
			PIN_FLIST_FLD_PUT(os_flistp, PIN_FLD_TAX, (void*)pbo_decimal_round(tax_amt, (int32)precision, round_mode, ebufp ), ebufp);
			/* tot_tax += tax_amt; */
			pbo_decimal_add_assign(tot_tax, tax_amt, ebufp);
			amt_exempt = pbo_decimal_from_str("0.0", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cumulative tax rule applied.");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After Applying ot_flistp", ot_flistp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After Applying os_flistp", os_flistp);
			fm_cust_pol_build_subtotal(os_flistp, j_level, tax_amt, tax_rate, amt_taxed, amt_exempt,
				amt_gross, name, descr, loc_mode, precision, round_mode, ebufp); 

			PIN_ERRBUF_CLEAR(ebufp);
			continue;
		}		
		else if (taxcode && ((strcmp(taxcode, MSO_ET_MAIN)==0)||(strcmp(taxcode,MSO_ET_ADDI)==0)))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET Matched");
//			taxcode_old = (char *)taxcode;
//			r_flistp = PIN_FLIST_CREATE(ebufp);
//			sprintf(taxcode_old,"%s",	taxcode);
//			acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
			balgrp_pdp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);

			et_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, et_in_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_TAX_CODE, taxcode, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ET Code Retrieval Input Flist", et_in_flistp);
			mso_retrieve_et_code(ctxp, et_in_flistp, &et_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ET Code Retrieval Output Flist", et_out_flistp);

			taxcode = PIN_FLIST_FLD_GET(et_out_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
			taxrule = (int32 *)PIN_FLIST_FLD_GET(et_out_flistp, PIN_FLD_TAXPKG_TYPE, 1, ebufp);

			/* Read Scale, subscription_charge_amt and accumulated_tax_amt as per new design */
                        events_flistp = PIN_FLIST_ELEM_GET(event_flistp,PIN_FLD_EVENTS,0,1,ebufp);
                        if(events_flistp)
                        {
                                new_scale = PIN_FLIST_FLD_GET(events_flistp,PIN_FLD_SCALE,1,ebufp);
                                subscription_amt = PIN_FLIST_FLD_GET(events_flistp,PIN_FLD_AMOUNT,1,ebufp);
                                accumulated_tax_amt = PIN_FLIST_FLD_GET(events_flistp,PIN_FLD_TAX,1,ebufp);
				subscription_fee_flag = PIN_FLIST_FLD_GET(events_flistp,PIN_FLD_CYCLE_FEE_FLAGS,1,ebufp);
				fdp_active = PIN_FLIST_FLD_GET(events_flistp,PIN_FLD_FLAGS,1,ebufp);
                        }	
			
			if (taxcode!=NULL)
			{
				sprintf(msg,"MSO ET Taxcode is : %s ",	taxcode);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}
			else
			{
				sprintf(msg,"MSO ET Taxcode Doesn't Match");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				return;
			}
			
			item_total = pbo_decimal_from_str("0.0",ebufp);
			if (*taxrule == 1)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET - Flat Fee Rule");

				if(fdp_active != NULL && *fdp_active != 1 && subscription_fee_flag != NULL && *subscription_fee_flag != 1)
				{
					new_scale = pbo_decimal_from_str("0.0",ebufp); //Set new_scale=0 so that tax is not applied
				}
			}
			else if (*taxrule == 2)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET - Subscription + ST");
				amt_gross = pbo_decimal_add(subscription_amt,accumulated_tax_amt,ebufp);

				if(pbo_decimal_is_zero(subscription_amt,ebufp) == 1 && pbo_decimal_is_zero(accumulated_tax_amt,ebufp) == 1)
				{
					amt_gross = pbo_decimal_from_str("0.0",ebufp); //Set amt_gross=0 so that tax is not applied
				}
			}
			else if (*taxrule == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET - Subscription only");
				amt_gross = pbo_decimal_copy(subscription_amt,ebufp);

				if(pbo_decimal_is_zero(subscription_amt,ebufp) == 1)
				{
					amt_gross = pbo_decimal_from_str("0.0",ebufp); //Set amt_gross=0 so that tax is not applied
				}
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Overriding the amt_gross to zero");
				PIN_ERRBUF_CLEAR(ebufp);
				amt_gross = pbo_decimal_from_str("0.0",ebufp);
			}


/*			else 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET - Standard Percentage Rule");
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Service POID: ", service_pdp);
				mso_retrieve_catv_billed_items(ctxp, balgrp_pdp, service_pdp, &r_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill items search flist", r_flistp);
				rs_flistp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_RESULTS, 1, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Billed items search Error");
		        }
				if (rs_flistp)
				{
				
					rec_id1 = 0;
					cookie1 = NULL;
					if (*taxrule == 2)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET - Subscription + ST");
						//Looping for all the pending item elements
						while((item_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS, &rec_id1, 1, &cookie1,  ebufp))!=(pin_flist_t*)NULL)
						{
							
							vp = PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_ITEM_TOTAL, 0, ebufp);
							temp = pbo_decimal_copy( (pin_decimal_t*)vp, ebufp);
							if(pbo_decimal_is_null(temp, ebufp))
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "ITEM_TOTAL Null error");
							}	
							pbo_decimal_add_assign(item_total, temp, ebufp);
							temp = pbo_decimal_from_str("0.0",ebufp);
							if(pbo_decimal_is_null(item_total, ebufp))
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "ITEM_TOTAL Addition error");
							}
						}
					}
					else if (*taxrule == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET - Subscription only");
						//Looping for all the pending item elements
						while((item_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS, &rec_id1, 1, &cookie1,  ebufp))!=(pin_flist_t*)NULL)
						{
							item_pdp = PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_POID, 1, ebufp);
							item_type = (char *)PIN_POID_GET_TYPE(item_pdp);
							if (item_type && ((strcmp(item_type,"/item/Service_Tax")==0) || (strcmp(item_type,"/item/Education_Cess")==0) || (strcmp(item_type,"/item/Secondary_Higher_Education_Cess")==0)))
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Skipping ST Items");
							}
							else
							{
								vp = PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_ITEM_TOTAL, 0, ebufp);
								temp = pbo_decimal_copy( (pin_decimal_t*)vp, ebufp);
								if(pbo_decimal_is_null(temp, ebufp))
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "ITEM_TOTAL Null error");
								}	
								pbo_decimal_add_assign(item_total, temp, ebufp);
								temp = pbo_decimal_from_str("0.0",ebufp);
								if(pbo_decimal_is_null(item_total, ebufp))
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "ITEM_TOTAL Addition error");
								}
							}
						}
					}
				}
				else 
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Overriding the amt_gross to zero, as no items found for this billing cycle");
					PIN_ERRBUF_CLEAR(ebufp);
					amt_gross = pbo_decimal_from_str("0.0",ebufp);
				}
			}
			amt_gross = pbo_decimal_copy(item_total, ebufp);
*/
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test4");
			if (pbo_decimal_is_null(amt_gross, ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Overriding the amt_gross to zero, as no items found for this billing cycle");
				amt_gross = pbo_decimal_from_str("0.0",ebufp);
				
			}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test5");

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Before Applying ET", ot_flistp)
			//mso_et_apply_tax(ctxp, taxcode, amt_gross, scale, &ot_flistp, &r_flistp, ebufp);
			mso_et_apply_tax(ctxp, taxcode, amt_gross, new_scale, &ot_flistp, &r_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After Applying ET", ot_flistp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After Applying ET", r_flistp);

			tax_amt = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_TAX, 1, ebufp);
			tax_rate = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PERCENT, 1, ebufp);
			amt_gross = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_AMOUNT_GROSS, 1, ebufp);
			amt_taxed = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_AMOUNT_TAXED, 1, ebufp);
			descr = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_DESCR, 1, ebufp);
			vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_TYPE, 1, ebufp);
			if (vp) {
				j_level = *(int32 *)vp;
			}
			else {
				j_level = 0;
			}
			str_tax_rate = pbo_decimal_to_str(tax_rate,ebufp);

			os_flistp = PIN_FLIST_ELEM_ADD(ot_flistp, PIN_FLD_SUBTOTAL, elemcnt++, ebufp);
			PIN_FLIST_FLD_PUT(os_flistp, PIN_FLD_TAX, (void*)pbo_decimal_round(tax_amt, (int32)precision, round_mode, ebufp ), ebufp);
			/* tot_tax += tax_amt; */
			pbo_decimal_add_assign(tot_tax, tax_amt, ebufp);
			amt_exempt = pbo_decimal_from_str("0.0", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cumulative tax rule applied.");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After Applying ot_flistp", ot_flistp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After Applying os_flistp", os_flistp);
			fm_cust_pol_build_subtotal(os_flistp, j_level, tax_amt, tax_rate, amt_taxed, amt_exempt,
				amt_gross, name, descr, loc_mode, precision, round_mode, ebufp); 

			PIN_ERRBUF_CLEAR(ebufp);
			continue;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Taxcode neither matching VAT / ET ...");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Tax Table", Tax_Table);
			elemid2 = 0; cookie2 = NULL;
			/* compute tax for each matching taxcode in the table */
			while ((flistp = PIN_FLIST_ELEM_GET_NEXT(Tax_Table, 
				PIN_FLD_TAXES, &elemid2, 1, &cookie2, ebufp))
						!= (pin_flist_t*)NULL) 
			{

			/* get the taxcode */
			vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_TAX_CODE,
				1, ebufp);

			/* taxcode match? */
			if (vp) {
				if (strcmp((char*)vp, taxcode) != 0) {
					/* taxcode did not match */
					sprintf(msg,"Checking taxcode %s with %s. Skipping ...",
						taxcode, (char*)vp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					continue;
					/*******/
				} else {
					/* taxcode matches */
				sprintf(msg, "Checking taxcode %s with %s. Matched!", 
						taxcode, (char*)vp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				}
			}
			else {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "no tax code available");	
			}
			/* get validity dates */
			startTm = (time_t*) PIN_FLIST_FLD_GET(flistp,
				PIN_FLD_START_T, 1, ebufp);
			endTm = (time_t*) PIN_FLIST_FLD_GET(flistp,
				PIN_FLD_END_T, 1, ebufp);

			/* tax rate valid? */
			if (startTm && endTm && *startTm && *endTm &&
			    ((*transDt < *startTm) || (*transDt >= *endTm))) {
				/* tax rate is not valid */
				sprintf(msg,
				"Taxcode %s has expired rate. Skipping ...\n",
					taxcode);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				continue;
				/*******/
			} else {
				/* tax rate is valid */
				sprintf(msg, "Taxcode %s has valid rate!\n",
					taxcode);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}

			/* get jurisdiction level */
			vp =  PIN_FLIST_FLD_GET(flistp, 
				PIN_FLD_TYPE, 1, ebufp);
			if (vp) {
				j_level = *(int32 *)vp;
			}
			else {
				j_level = 0;
			}
			switch (j_level) {
			case PIN_RATE_TAX_JUR_FED:
				locale = country;
				break;
			case PIN_RATE_TAX_JUR_STATE:
				locale = state;
				break;
			case PIN_RATE_TAX_JUR_COUNTY:
				locale = county;
				break;
			case PIN_RATE_TAX_JUR_CITY:
				locale = city;
				break;
			default:
				/* unsupported level */
				sprintf(msg,
		"Unsupported jurisdiction level %d!  Skipping ...", j_level);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				continue;
				/*******/
			}

			/* check for location mode */
			if (loc_mode != PIN_RATE_LOC_ADDRESS) {
#ifdef MATCH_NPA_ONLY
				code[3] = '\0';
#endif
				locale = code;
			}

			/* get jurisdiction list (nexus) for the level */
			j_list = (char*) PIN_FLIST_FLD_GET(flistp, 
				PIN_FLD_NAME, 1, ebufp);

			if (j_list) {
				/* is locale in jurisdiction list? */
				if (j_list && (strstr(j_list, "*") || 
                                               fm_cust_pol_is_locale_in_juri_list(j_list, locale))) {
					/* locale in jurisdiction list */
					sprintf(msg,
			     		"Locale %s is in juris list '%s' for taxcode %s\n",
						locale, j_list, taxcode);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				} else {
					/* locale not in jurisdiction list */
					sprintf(msg,
	      				"Locale %s not in juris list '%s' for taxcode %s. Skipping ...\n",
						locale, j_list, taxcode);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					continue;
					/*******/
				} 
			}
			else {
				continue;
			}

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_cust_pol_get_amt_exempt");
			/* get exemptions, if any */
			amt_exempt = fm_cust_pol_get_amt_exempt(in_flistp,
				j_level, amt_gross, ebufp);
			
			/* get tax rate */
			vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_PERCENT,
				1, ebufp);
			if (vp == (void*)NULL) {
				/* no tax rate?  default to zero */
				tax_rate = pbo_decimal_from_str("0.0",ebufp);
			} else {
				tax_rate = pbo_decimal_copy((pin_decimal_t*)vp,
					ebufp);
			}
		
			str_tax_rate = pbo_decimal_to_str(tax_rate,ebufp);
			sprintf(msg, "Tax rate for taxcode %s is %s.", 
				taxcode,str_tax_rate); 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

			/* get tax rule */
			vp =  PIN_FLIST_FLD_GET(flistp, 
				PIN_FLD_TAX_WHEN, 1, ebufp);
			if (vp) {
				tax_rule = *(int32*)vp;
			}
			else {
				tax_rule = 0;
			}
			/* this is the amount that will be taxed */
			/* amt_taxed = amt_gross - amt_exempt; */
			amt_taxed = pbo_decimal_subtract(amt_gross,
				amt_exempt,ebufp);


				/* tax on tax? */
				if (tax_rule == 1 || tax_rule == 5) {
					if(taxcode && ((strcmp(taxcode, EDUCATION_CESS) ==0) || 0 == strcmp(taxcode, SHE_CESS)))
					{
						amt_taxed = pbo_decimal_copy(service_tax_amount, ebufp);
					}
					else
					{
						/* amt_taxed += tot_tax; */
						pbo_decimal_add_assign(amt_taxed,
							tot_tax, ebufp);
					}
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
							"Tax on tax rule applied.");
				}

			/* calculate tax */
			if (tax_rule == 2 || tax_rule == 6) {
				/* The tax_rate is the flat fee */
				/* tax_amt = tax_rate; 
				   tax_rate = 0.0;
				*/
				tax_amt = tax_rate;
//					tax_rate = pbo_decimal_from_str("0.0", ebufp);
					tax_rate = pbo_decimal_from_str("1.0", ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"Flat fee rule applied.");
				} else if (tax_rule == 3 || tax_rule == 7) {
					/* The tax is included in the amount */
					pin_decimal_t *base_amt = NULL; 
					pin_decimal_t *adj_rate = NULL;

				/* adj_rate = 1 + tax_rate */
				adj_rate = pbo_decimal_from_str("1.0", ebufp);
				pbo_decimal_add_assign(adj_rate, tax_rate,
					ebufp); 
                                /* base_amt = amt_taxed / (1 + tax_rate); */
				base_amt = pbo_decimal_divide(amt_taxed,
					adj_rate, ebufp);
                                /* tax_amt = amt_taxed - base_amt; */
				tax_amt = pbo_decimal_subtract(amt_taxed,
					base_amt, ebufp);
                                /* amt_taxed = base_amt; */
				pbo_decimal_destroy(&amt_taxed);
				amt_taxed = base_amt;
				pbo_decimal_destroy(&adj_rate);

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Inclusive tax rule applied.");
			} else {
				/* The tax_rate is a percentage */
				/* tax_amt = amt_taxed * tax_rate; */
				tax_amt = pbo_decimal_multiply(amt_taxed,
					tax_rate, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Standard tax rule applied.");
			}

			/* add a subtotal array into the taxes array */ 
			os_flistp = PIN_FLIST_ELEM_ADD(ot_flistp, 
				PIN_FLD_SUBTOTAL, elemcnt++, ebufp);

			/* include the tax in the total? */
			if (tax_rule >= 0 && tax_rule <= 3) {
				PIN_FLIST_FLD_PUT(os_flistp, PIN_FLD_TAX, 
					(void*)pbo_decimal_round(tax_amt,
					 (int32)precision, round_mode, ebufp ), ebufp);
				/* tot_tax += tax_amt; */
				pbo_decimal_add_assign(tot_tax, tax_amt, ebufp);
				
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Cumulative tax rule applied.");
			} else {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Non-cumulative tax rule applied.");
			}
				if( taxcode && (strcmp(taxcode, SERVICE_TAX)== 0))
				{
					service_tax_amount = pbo_decimal_copy(tot_tax, ebufp);
				}
			/* get tax description */
			descr = (char*) PIN_FLIST_FLD_GET(flistp, 
				PIN_FLD_DESCR, 1, ebufp);

			/* build the subtotals array */
			fm_cust_pol_build_subtotal(os_flistp, j_level,
				tax_amt, tax_rate, amt_taxed, amt_exempt,
				amt_gross, name, descr, loc_mode, precision,
				round_mode, ebufp); 
 
                	if (amt_taxed) {
                       		 pbo_decimal_destroy(&amt_taxed);
                	}
                	if ( amt_exempt) {
                       		 pbo_decimal_destroy(&amt_exempt);
                	}
                	if ( tax_rate ) {
                       		 pbo_decimal_destroy(&tax_rate);
                	}
                	if ( tax_amt ) {
                        	pbo_decimal_destroy(&tax_amt);
                	}
			if (str_tax_rate ) {
				free(str_tax_rate);
			}

			}
		}
		if (elemcnt == 0) {
			/* No taxes computed.  Just add a (default)
			 * subtotal array into the taxes array.
			 */ 
			os_flistp = PIN_FLIST_ELEM_ADD(ot_flistp, 
				PIN_FLD_SUBTOTAL, elemcnt, ebufp);

			tax_amt = pbo_decimal_from_str("0.0", ebufp);
		        tax_rate = pbo_decimal_from_str("0.0", ebufp);
        		amt_taxed = pbo_decimal_from_str("0.0", ebufp);
        		amt_exempt = pbo_decimal_from_str("0.0", ebufp); 


			/* initialize subtotal array */ 
			fm_cust_pol_build_subtotal(os_flistp, j_level, tax_amt,
				tax_rate, amt_taxed, amt_exempt, amt_gross,
				name, descr, loc_mode, precision, round_mode,
				ebufp); 
		}


		/* add the total taxes to the taxes array */
		PIN_FLIST_FLD_PUT(ot_flistp, PIN_FLD_TAX, 
			(void*)pbo_decimal_round(tot_tax,
			(int32)precision,round_mode,ebufp), ebufp);

		if (amt_gross) {
			pbo_decimal_destroy(&amt_gross);
		}
		if (amt_taxed) {
			pbo_decimal_destroy(&amt_taxed);
		}
		if ( amt_exempt) {
			pbo_decimal_destroy(&amt_exempt);
		}
		if ( tot_tax ) {
			pbo_decimal_destroy(&tot_tax);
		}
		if ( tax_rate ) {
			pbo_decimal_destroy(&tax_rate);
		}
		if ( tax_amt ) {
			pbo_decimal_destroy(&tax_amt);
		}

	} /* end while */

CleanUp:
	
	/* Error? */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_tax_calc error", ebufp);
	}
	PIN_FLIST_DESTROY_EX(&et_in_flistp, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"D1");
	if (!pbo_decimal_is_null(temp, ebufp) ) {
		pbo_decimal_destroy(&temp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"D2");
/*	if ( !pbo_decimal_is_null(temp1, ebufp) ) {
		pbo_decimal_destroy(&temp1);
	}*/
	if ( !pbo_decimal_is_null(scale, ebufp) ) {
		pbo_decimal_destroy(&scale);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"D3");
	if ( !pbo_decimal_is_null(service_tax_amount, ebufp) ) {
		pbo_decimal_destroy(&service_tax_amount);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"D4");
	if ( !pbo_decimal_is_null(item_total, ebufp) ) {
		pbo_decimal_destroy(&item_total);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"D5");
	return;
}


/*******************************************************************
 * fm_cust_pol_build_subtotal ()
 *
 *******************************************************************/
static void
fm_cust_pol_build_subtotal(
	pin_flist_t		*os_flistp,
	int32			j_level,
	pin_decimal_t		*tax_amt,
	pin_decimal_t		*tax_rate,
	pin_decimal_t		*amt_taxed,
	pin_decimal_t		*amt_exempt,
	pin_decimal_t		*amt_gross,
	char			*name,
	char			*descr,
	int32			loc_mode,
	int32			precision,
	int32			round_mode,
	pin_errbuf_t		*ebufp)
{
	int32		subtype = PIN_RATE_TAX_TYPE_SALES;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/* fill in the subtotal array */
	PIN_FLIST_FLD_SET(os_flistp, PIN_FLD_TYPE, (void*)&j_level,ebufp);
	PIN_FLIST_FLD_SET(os_flistp, PIN_FLD_NAME, (void*)name, ebufp);
	PIN_FLIST_FLD_PUT(os_flistp, PIN_FLD_AMOUNT_GROSS,
		(void*)pbo_decimal_round(amt_gross,
		 (int32)precision,round_mode,ebufp), ebufp);
	PIN_FLIST_FLD_PUT(os_flistp, PIN_FLD_TAX,
		(void*)pbo_decimal_round(tax_amt,
	 	(int32)precision,round_mode,ebufp), ebufp);
	PIN_FLIST_FLD_PUT(os_flistp, PIN_FLD_PERCENT,
		(void*)pbo_decimal_round(tax_rate, 
		(int32)RatePrecision, round_mode,ebufp), ebufp);
	PIN_FLIST_FLD_PUT(os_flistp, PIN_FLD_AMOUNT_TAXED,
		(void*)pbo_decimal_round(amt_taxed, 
	        (int32)precision,round_mode,ebufp ), ebufp);
	PIN_FLIST_FLD_PUT(os_flistp, PIN_FLD_AMOUNT_EXEMPT,
		(void*)pbo_decimal_round(amt_exempt, 
		(int32)precision,round_mode,ebufp), ebufp);
	PIN_FLIST_FLD_SET(os_flistp, PIN_FLD_SUBTYPE, (void*)&subtype, ebufp);
	PIN_FLIST_FLD_SET(os_flistp, PIN_FLD_DESCR, (void*)descr, ebufp);
	PIN_FLIST_FLD_SET(os_flistp, PIN_FLD_LOCATION_MODE,
		(void*)&loc_mode, ebufp);

	return;
}


/*******************************************************************
 * fm_cust_pol_do_juris_only ()
 *
 *******************************************************************/
static void
fm_cust_pol_do_juris_only(
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp)
{
	int32		result = 1;

	/* no address validation. just return PASSED */
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_RESULT, (void *)&result, ebufp);

	return;
}


/*******************************************************************
 * fm_cust_pol_get_amt_exempt ()
 *
 *******************************************************************/
static pin_decimal_t *
fm_cust_pol_get_amt_exempt(
	pin_flist_t		*in_flistp,
	int32			j_level,
	pin_decimal_t		*g_amt,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t	*flistp = NULL;
	pin_cookie_t	cookie = NULL;
	int32		elemid = 0;
	pin_decimal_t	*rate =NULL; 
	void		*vp = NULL;

	/* Walk the EXEMPTIONS looking for matching jurisdiction level */
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_EXEMPTIONS,
		&elemid, 1, &cookie, ebufp)) != (pin_flist_t*)NULL) {

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "within expemtiins array list");
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_TYPE, 0, ebufp);
		if (vp && (j_level == *(int32*)vp)) {
			/* Match found */
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "within expemtiins match found");
			rate = (pin_decimal_t*)PIN_FLIST_FLD_GET(flistp,
				PIN_FLD_PERCENT, 0, ebufp);
			return pbo_decimal_multiply(g_amt, rate, ebufp); 
			/*****/
		}
	}

	return pbo_decimal_from_str("0.0",ebufp);
}


/*******************************************************************
 * fm_cust_pol_parse_tax_locale()
 *
 *******************************************************************/
static void
fm_cust_pol_parse_tax_locale(
	char		*locale,
	char		*city,
	char		*state,
	char		*zip,
	char		*country,
	char		*code,
	size_t      city_len,
	size_t      state_len,
	size_t      zip_len,
	size_t      country_len,
	size_t      code_len)
{
	char	*p = NULL;
	char	delim = ';';

	/* format = "city; state; zip; country"  */
	if (locale != (char*)NULL) {
		p = locale;
		fm_utils_tax_parse_fld(&p, city, city_len, delim);
		fm_utils_tax_parse_fld(&p, state, state_len, delim);
		fm_utils_tax_parse_fld(&p, zip, zip_len, delim);
		fm_utils_tax_parse_fld(&p, country, country_len, delim);
		fm_utils_tax_parse_fld(&p, code, code_len, delim);

		/* get rid of any "-" in Zip+4 code */
		p = (char*)strchr(zip, '-');
		while (p && (*p != '\0')) {
			*p = *(p + 1);
			p++;
		}
	}

	/* get rid of square brackets */
	if (code[0] == '[') {
		p = code;
		while ((*(p+1) != ']') && (*(p+1) != ',')) {
			*p = *(p+1);
			p++;
		}
		*p = '\0';
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_is_locale_in_juri_list()
 * This function checks if the given locale is in the 
 * jurisdiction list (';' separeted). It trims the leading/trailing 
 * spaces and ignore case (case insensitive) during comparison. 
 *******************************************************************/
static int32
fm_cust_pol_is_locale_in_juri_list(
        char            *j_list,
        char            *locale){
	
	char *curr = j_list;
	char *end;
	char *next;

	size_t locale_size = strlen(locale);

	/* Trim leading spaces */
	while(isspace(*curr)) curr++;

	/* Compare the given locale with each one in jurisdiction list */
	while ((next=strchr(curr, ';')) != NULL) {

		/* Trim trailing spaces */
		end = next-1;
		while(end > curr && isspace(*end))  end--;
		

		if(locale_size == (end-curr+1) && 
		   strncasecmp(curr, locale, locale_size) == 0){
			return PIN_BOOLEAN_TRUE;
		}

		/* Go for next locale in the list*/
		curr = next + 1;

		/* Trim leading spaces */
		while(isspace(*curr)) curr++;

	}

	/* Check the last one */
	end = curr+strlen(curr)-1;
	while(end > curr && isspace(*end)) end--;
	if(locale_size == (end-curr+1) && 
		strncasecmp(curr, locale, locale_size) == 0){
		return PIN_BOOLEAN_TRUE;
	}

	return	PIN_BOOLEAN_FALSE;
}

static char* mso_retrieve_vat_code(
	pcm_context_t	*ctxp,
	char			*taxcode,
	poid_t			*acct_pdp,
	poid_t			*balgrp_pdp,
	pin_errbuf_t	*ebufp)
	{
		pin_flist_t		*s_flistp = NULL;
		pin_flist_t		*ret_flistp = NULL;
		pin_flist_t		*r_in_flistp = NULL;
		pin_flist_t		*rs_flistp = NULL;
		pin_flist_t		*flistp = NULL;
		char			msg[STR_LEN];
		char			*binfo_id = NULL;
		char			*tax_map = NULL;
		poid_t			*pdp = NULL;
		int64			db = 0;
		int32			s_flags = 256;
		int32			id = 1;
		int32			srvc_type = 0;
	
		db = PIN_POID_GET_DB(balgrp_pdp);
		s_flistp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /billinfo 1, /balance_group 2 where 1.F1 = 2.F2 and 2.F3 = V3", ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, balgrp_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_OBJ, balgrp_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, balgrp_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_ID, "", ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Billinfo ID search input flist", s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Billinfo ID search output flist", ret_flistp);

		rs_flistp = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_RESULTS, 0, ebufp);
		binfo_id = PIN_FLIST_FLD_TAKE(rs_flistp, PIN_FLD_BILLINFO_ID, 0, ebufp); 
		ret_flistp = NULL;
		if (!PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, binfo_id);
			r_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(r_in_flistp, MSO_FLD_VAT_ZONE, "", ebufp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, r_in_flistp, &ret_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account Read Flds Output Flist", ret_flistp);
			tax_map = PIN_FLIST_FLD_TAKE(ret_flistp, MSO_FLD_VAT_ZONE, 0, ebufp);
			
			if(binfo_id && (strcmp(binfo_id, BILLINFO_ID_CATV)==0))
			{
				sprintf(taxcode,"%s_%s", (char *)tax_map, (char*)BILLINFO_ID_CATV);
				srvc_type = 1;
			}
			else
			{
				sprintf(taxcode,"%s_%s", (char *)tax_map, (char*)BILLINFO_ID_BB);
				srvc_type = 0;
			}
			sprintf(msg,"MSO updated VAT Taxcode is : %s ",	(char *)taxcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			ret_flistp = NULL;
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_cfg_vat_mapping where F1 = V1 and F2 = V2", ebufp);
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, taxcode, ebufp);
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_SET(flistp, MSO_FLD_SERVICE_TYPE, &srvc_type, ebufp);
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_TAX_CODE, "", ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"VAT Taxcode search input flist", s_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"VAT Taxcode search output flist", ret_flistp);
		
			rs_flistp = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_RESULTS, 0, ebufp);
			if (!PIN_ERRBUF_IS_ERR(ebufp)) 
			{
				taxcode = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
				sprintf(msg,"VAT Taxcode for Processing is : %s ",	(char *)taxcode);
			}
			else
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Taxcode Retrieval error", ebufp);
				return;
			}
		}
		else 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Billinfo ID Retrieval error", ebufp);
			return;
		}
	Cleanup:
		PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, ebufp);
		PIN_POID_DESTROY(pdp, ebufp);
		if (binfo_id)
		{
			free(binfo_id);
		}

		return (char *)taxcode;
	}


static void 
mso_vat_apply_tax(
	pcm_context_t	*ctxp,
	char			*taxcode,
	pin_decimal_t	*amt_gross,
	pin_flist_t		**ot_flistpp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t		*t_flistp = NULL;   /* in taxes array */
	pin_flist_t		*flistp = NULL;
	pin_cookie_t	cookie = NULL;
	int32			elemid = 0;
	int32			elemcnt = 0;
	int32			j_level = 0;
	char			*j_list = NULL;
	char			*descr = NULL;
	char			*str_tax_rate = NULL;
	void			*vp = NULL;
	char			*locale = NULL;
	char			city[TOK_LEN];
	char			state[TOK_LEN];
	char			zip[TOK_LEN];
	char			county[TOK_LEN];
	char			country[TOK_LEN];
	char			code[TOK_LEN];
	char			name[STR_LEN];
	char			msg[STR_LEN];
	pin_decimal_t	*tax_rate=NULL; 
	pin_decimal_t	*tot_tax=NULL;
	pin_decimal_t	*tax_amt=NULL;
	pin_decimal_t	*amt_taxed = NULL;
	pin_decimal_t	*amt_exempt = NULL;
	int32 			tax_pkg = PIN_RATE_TAXPKG_CUSTOM;
	int32			tax_rule = 0;
	int32			loc_mode = 0;
	int32			precision = 6;
	int32			round_mode = ROUND_HALF_UP;
	time_t			*startTm = NULL;
	time_t			*endTm = NULL;
	time_t			*transDt = NULL;
	time_t			now = 0;

	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	if (transDt == (time_t*)NULL) {
		/* not available. use current time */
		now = pin_virtual_time((time_t*)NULL);
		transDt = &now;
	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input FLIST Inside Applying VAT", *in_flistp);
//	t_flistp = PIN_FLIST_FLD_GET(*in_flistp, PIN_FLD_TAXES, 1, ebufp);
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Taxes FLIST Inside Applying VAT", t_flistp);
//	vp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_AMOUNT_TAXED, 1, ebufp);
//	amt_gross = pbo_decimal_copy((pin_decimal_t*)vp, ebufp);
	str_tax_rate = pbo_decimal_to_str(amt_gross,ebufp);
	sprintf(msg, "Amount to be Taxed is %s.", str_tax_rate); 
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	
	elemid = 0; cookie = NULL;
	/* compute tax for each matching taxcode in the table */
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(VAT_Table, 
		PIN_FLD_TAXES, &elemid, 1, &cookie, ebufp))
					!= (pin_flist_t*)NULL) {

		/* get the taxcode */
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_TAX_CODE,
			1, ebufp);
		
		/* taxcode match? */
		if (vp) {
			if (strcmp((char*)vp, taxcode) != 0) {
				/* taxcode did not match */
				sprintf(msg,"Checking taxcode %s with %s. Skipping ...",
					taxcode, (char*)vp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				continue;
				/*******/
			} else {
				/* taxcode matches */
				sprintf(msg, "Checking taxcode %s with %s. Matched!", 
					taxcode, (char*)vp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}
		}
		else {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "no tax code available");	
		}
		/* get validity dates */
		startTm = (time_t*) PIN_FLIST_FLD_GET(flistp, PIN_FLD_START_T, 1, ebufp);
		endTm = (time_t*) PIN_FLIST_FLD_GET(flistp, PIN_FLD_END_T, 1, ebufp);

		/* tax rate valid? */
		if (startTm && endTm && *startTm && *endTm &&
			((*transDt < *startTm) || (*transDt >= *endTm))) {
			/* tax rate is not valid */
			sprintf(msg,
				"Taxcode %s has expired rate. Skipping ...\n",
				taxcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			continue;
			/*******/
		} else {
			/* tax rate is valid */
			sprintf(msg, "Taxcode %s has valid rate!\n",
				taxcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		}

		/* get jurisdiction level */
		vp =  PIN_FLIST_FLD_GET(flistp, 
			PIN_FLD_TYPE, 1, ebufp);
		if (vp) {
			j_level = *(int32 *)vp;
		}
		
		descr = (char*) PIN_FLIST_FLD_GET(flistp, PIN_FLD_DESCR, 1, ebufp);
		/* get tax rate */
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_PERCENT,
			1, ebufp);
		if (vp == (void*)NULL) {
			/* no tax rate?  default to zero */
			tax_rate = pbo_decimal_from_str("0.0",ebufp);
		} else {
			tax_rate = pbo_decimal_copy((pin_decimal_t*)vp,
				ebufp);
		}
		
			str_tax_rate = pbo_decimal_to_str(tax_rate,ebufp);
			sprintf(msg, "Tax rate for taxcode %s is %s.", 
				taxcode,str_tax_rate); 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			
			/* get tax rule */
			vp =  PIN_FLIST_FLD_GET(flistp, PIN_FLD_TAX_WHEN, 1, ebufp);
			if (vp) {
				tax_rule = *(int32*)vp;
			}
			else {
				tax_rule = 0;
			}
			amt_taxed = amt_gross;
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Assignment ERROR");
				return;
			}


			/* tax on tax? */
			if (tax_rule == 1 || tax_rule == 5) {
				/* amt_taxed += tot_tax; */
				pbo_decimal_add_assign(amt_taxed,
					tot_tax, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"Tax on tax rule applied.");
			}

			/* calculate tax */
			if (tax_rule == 2 || tax_rule == 6) {
				/* The tax_rate is the flat fee */
				/* tax_amt = tax_rate; 
				   tax_rate = 0.0;
				*/
				tax_amt = tax_rate;
//				tax_rate = pbo_decimal_from_str("0.0", ebufp);
				tax_rate = pbo_decimal_from_str("1.0", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Flat fee rule applied.");
			} else if (tax_rule == 3 || tax_rule == 7) {
				/* The tax is included in the amount */
				pin_decimal_t *base_amt = NULL; 
				pin_decimal_t *adj_rate = NULL;

				/* adj_rate = 1 + tax_rate */
				adj_rate = pbo_decimal_from_str("1.0", ebufp);
				pbo_decimal_add_assign(adj_rate, tax_rate,
					ebufp); 
                                /* base_amt = amt_taxed / (1 + tax_rate); */
				base_amt = pbo_decimal_divide(amt_taxed,
					adj_rate, ebufp);
                                /* tax_amt = amt_taxed - base_amt; */
				tax_amt = pbo_decimal_subtract(amt_taxed,
					base_amt, ebufp);
                                /* amt_taxed = base_amt; */
				pbo_decimal_destroy(&amt_taxed);
				amt_taxed = base_amt;
				pbo_decimal_destroy(&adj_rate);

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Inclusive tax rule applied.");
			} else {
				/* The tax_rate is a percentage */
				/* tax_amt = amt_taxed * tax_rate; */
				str_tax_rate = pbo_decimal_to_str(amt_taxed,ebufp);
				sprintf(msg, "Amount Gross is %s.", str_tax_rate); 
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				tax_amt = pbo_decimal_multiply(amt_taxed, tax_rate, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Standard tax rule applied.");
				str_tax_rate = pbo_decimal_to_str(tax_amt,ebufp);
				sprintf(msg, "Tax Amount = %s.", str_tax_rate); 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Applying ERROR");
				return;
			}
		}
		PIN_FLIST_FLD_SET(*ot_flistpp, PIN_FLD_TAX, tax_amt, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_TAX, tax_amt, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_PERCENT, tax_rate, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT_GROSS, amt_gross, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT_TAXED, amt_taxed, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, descr, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_TYPE, &j_level, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"VAT Processing Return FLIST", *r_flistpp);
		
		if (str_tax_rate)
		{
			free(str_tax_rate);
		}
		if (tax_amt)
		{
			pbo_decimal_destroy(&tax_amt);
		} 

		if (tax_rate)
		{
			pbo_decimal_destroy(&tax_rate);
		}
		if (amt_taxed)
		{
			pbo_decimal_destroy(&amt_taxed);
		}
}


void mso_retrieve_et_code(
	pcm_context_t	*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
	{
		pin_flist_t		*s_flistp = NULL;
		pin_flist_t		*ret_flistp = NULL;
		pin_flist_t		*r_in_flistp = NULL;
		pin_flist_t		*rs_flistp = NULL;
		pin_flist_t		*flistp = NULL;
		char			msg[STR_LEN];
		char			*binfo_id = NULL;
		char			*tax_map = NULL;
		char			*taxcode = NULL;
		poid_t			*pdp = NULL;
		poid_t			*acct_pdp = NULL;
		int64			db = 0;
		int32			s_flags = 256;
		int32			id = 1;
		int32			srvc_type = 0;
	
		acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
		taxcode = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
		db = PIN_POID_GET_DB(acct_pdp);
		r_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(r_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(r_in_flistp, MSO_FLD_ET_ZONE, "", ebufp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, r_in_flistp, &ret_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account Read Flds Output Flist", ret_flistp);
		tax_map = PIN_FLIST_FLD_GET(ret_flistp, MSO_FLD_ET_ZONE, 0, ebufp);
		ret_flistp = NULL;
		if(taxcode && (strcmp(taxcode, MSO_ET_MAIN)==0))
		{
			sprintf(taxcode,"%s_%s", (char *)tax_map, (char*)MAIN_CONN);
		}
		else if (taxcode && (strcmp(taxcode, MSO_ET_ADDI)==0))
		{
			sprintf(taxcode,"%s_%s", (char *)tax_map, (char*)ADDI_CONN);
		}
		sprintf(msg,"MSO updated ET Taxcode is : %s ", (char *)taxcode);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

		s_flistp = PIN_FLIST_CREATE(ebufp);
//		ret_flistp = PIN_FLIST_CREATE(ebufp);
		
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_cfg_et_mapping where F1 = V1", ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, taxcode, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TAX_CODE, "", ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TAXPKG_TYPE, 0, ebufp);
		//added for et config check
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_AMOUNT, 0, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"ET Taxcode search input flist", s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"ET Taxcode search output flist", ret_flistp);
		rs_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (!PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			taxcode = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
			sprintf(msg,"ET Taxcode for Processing is : %s ", (char *)taxcode);
			*r_flistpp = PIN_FLIST_COPY(rs_flistp, ebufp);
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Taxcode Retrieval error", ebufp);
			return;
		}
	
		PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
//		PIN_FLIST_DESTROY_EX(&ret_flistp, ebufp);
//		PIN_FLIST_DESTROY_EX(&rs_flistp, ebufp);
//		PIN_FLIST_DESTROY_EX(&r_in_flistp, ebufp);

/*		if (taxcode)
		{
			free(taxcode);
		}

		if (tax_map)
		{
			free(tax_map);
		}
*/
		return;
	}


void 
mso_et_apply_tax(
	pcm_context_t	*ctxp,
	char			*taxcode,
	pin_decimal_t	*amt_gross,
	pin_decimal_t	*scale,
	pin_flist_t		**ot_flistpp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t		*t_flistp = NULL;   /* in taxes array */
	pin_flist_t		*flistp = NULL;
	pin_cookie_t	cookie = NULL;
	int32			elemid = 0;
	int32			elemcnt = 0;
	int32			j_level = 0;
	char			*j_list = NULL;
	char			*descr = NULL;
	char			*str_tax_rate = NULL;
	void			*vp = NULL;
	char			*locale = NULL;
	char			city[TOK_LEN];
	char			state[TOK_LEN];
	char			zip[TOK_LEN];
	char			county[TOK_LEN];
	char			country[TOK_LEN];
	char			code[TOK_LEN];
	char			name[STR_LEN];
	char			msg[STR_LEN];
	pin_decimal_t	*tax_rate=NULL; 
	pin_decimal_t	*tot_tax=NULL;
	pin_decimal_t	*tax_amt=NULL;
	pin_decimal_t	*amt_taxed = NULL;
	pin_decimal_t	*amt_exempt = NULL;
	int32 			tax_pkg = PIN_RATE_TAXPKG_CUSTOM;
	int32			tax_rule = 0;
	int32			loc_mode = 0;
	int32			precision = 6;
	int32			round_mode = ROUND_HALF_UP;
	time_t			*startTm = NULL;
	time_t			*endTm = NULL;
	time_t			*transDt = NULL;
	time_t			now = 0;

	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	if (transDt == (time_t*)NULL) {
		/* not available. use current time */
		now = pin_virtual_time((time_t*)NULL);
		transDt = &now;
	}

	str_tax_rate = pbo_decimal_to_str(amt_gross,ebufp);
	sprintf(msg, "Amount to be Taxed is %s.", str_tax_rate); 
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	sprintf(msg, "Tax Code %s.", taxcode); 
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ET Table", ET_Table);
	
	elemid = 0; cookie = NULL;
	/* compute tax for each matching taxcode in the table */
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(ET_Table, 
		PIN_FLD_TAXES, &elemid, 1, &cookie, ebufp))
					!= (pin_flist_t*)NULL) {

		/* get the taxcode */
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_TAX_CODE,
			1, ebufp);
		
		/* taxcode match? */
		if (vp) {
			if (strcmp((char*)vp, taxcode) != 0) {
				/* taxcode did not match */
				sprintf(msg,"Checking taxcode %s with %s. Skipping ...",
					taxcode, (char*)vp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				continue;
				/*******/
			} else {
				/* taxcode matches */
				sprintf(msg, "Checking taxcode %s with %s. Matched!", 
					taxcode, (char*)vp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}
		}
		else {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "no tax code available");	
		}
		/* get validity dates */
		startTm = (time_t*) PIN_FLIST_FLD_GET(flistp, PIN_FLD_START_T, 1, ebufp);
		endTm = (time_t*) PIN_FLIST_FLD_GET(flistp, PIN_FLD_END_T, 1, ebufp);

		/* tax rate valid? */
		if (startTm && endTm && *startTm && *endTm &&
		    ((*transDt < *startTm) || (*transDt >= *endTm))) {
			/* tax rate is not valid */
			sprintf(msg,
			"Taxcode %s has expired rate. Skipping ...\n",
				taxcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			continue;
			/*******/
		} else {
			/* tax rate is valid */
			sprintf(msg, "Taxcode %s has valid rate!\n",
				taxcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		}

		/* get jurisdiction level */
		vp =  PIN_FLIST_FLD_GET(flistp, 
			PIN_FLD_TYPE, 1, ebufp);
		if (vp) {
			j_level = *(int32 *)vp;
		}
		
		descr = (char*) PIN_FLIST_FLD_GET(flistp, PIN_FLD_DESCR, 1, ebufp);
		/* get tax rate */
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_PERCENT,
			1, ebufp);
		if (vp == (void*)NULL) {
			/* no tax rate?  default to zero */
			tax_rate = pbo_decimal_from_str("0.0",ebufp);
		} else {
			tax_rate = pbo_decimal_copy((pin_decimal_t*)vp,
				ebufp);
		}
		
			str_tax_rate = pbo_decimal_to_str(tax_rate,ebufp);
			sprintf(msg, "Tax rate for taxcode %s is %s.", 
				taxcode,str_tax_rate); 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			
			/* get tax rule */
			vp =  PIN_FLIST_FLD_GET(flistp, PIN_FLD_TAX_WHEN, 1, ebufp);
			if (vp) {
				tax_rule = *(int32*)vp;
			}
			else {
				tax_rule = 0;
			}
//			amt_taxed = amt_gross;
			amt_taxed = pbo_decimal_copy(amt_gross,ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Assignment ERROR");
				return;
			}

			/* tax on tax? */
			if (tax_rule == 1 || tax_rule == 5) {
				/* amt_taxed += tot_tax; */
				pbo_decimal_add_assign(amt_taxed,
					tot_tax, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"Tax on tax rule applied.");
			}

			/* calculate tax */
			if (tax_rule == 2 || tax_rule == 6) {
				/* The tax_rate is the flat fee */
				/* tax_amt = tax_rate; 
				   tax_rate = 0.0;
				*/
				// Multiplying the Scale with Flat Tax Rate for the Advance Charging with multiple months
				pbo_decimal_multiply_assign(tax_rate, scale, ebufp);
				tax_amt = tax_rate;

//				tax_rate = pbo_decimal_from_str("0.0", ebufp);
				tax_rate = pbo_decimal_from_str("1.0", ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Flat fee rule applied.");
			} else if (tax_rule == 3 || tax_rule == 7) {
				// The tax is included in the amount 
				pin_decimal_t *base_amt = NULL; 
				pin_decimal_t *adj_rate = NULL;

				// adj_rate = 1 + tax_rate 
				adj_rate = pbo_decimal_from_str("1.0", ebufp);
				pbo_decimal_add_assign(adj_rate, tax_rate,
					ebufp); 

				// base_amt = amt_taxed / (1 + tax_rate); 
				base_amt = pbo_decimal_divide(amt_taxed,
					adj_rate, ebufp);

				// tax_amt = amt_taxed - base_amt; 
				tax_amt = pbo_decimal_subtract(amt_taxed,
					base_amt, ebufp);

				// amt_taxed = base_amt; 
				pbo_decimal_destroy(&amt_taxed);
				amt_taxed = base_amt;
				pbo_decimal_destroy(&adj_rate);

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Inclusive tax rule applied.");
			}
			else {
				/* The tax_rate is a percentage */
				/* tax_amt = amt_taxed * tax_rate; */
				str_tax_rate = pbo_decimal_to_str(amt_taxed,ebufp);
				sprintf(msg, "Amount Gross is %s.", str_tax_rate); 
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				tax_amt = pbo_decimal_multiply(amt_taxed, tax_rate, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
					"Standard tax rule applied.");
				str_tax_rate = pbo_decimal_to_str(tax_amt,ebufp);
				sprintf(msg, "Tax Amount = %s.", str_tax_rate); 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Applying ERROR");
				return;
			}
		}
		PIN_FLIST_FLD_SET(*ot_flistpp, PIN_FLD_TAX, tax_amt, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_TAX, (pin_decimal_t *)tax_amt, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_PERCENT, (pin_decimal_t *)tax_rate, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT_GROSS, (pin_decimal_t *)amt_gross, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT_TAXED, (pin_decimal_t *)amt_taxed, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, descr, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_TYPE, &j_level, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"ET Processing Return FLIST", *r_flistpp);

		if (str_tax_rate)
		{
			free(str_tax_rate);
		}

		if (tax_amt)
		{
			pbo_decimal_destroy(&tax_amt);
		}
		if (tax_rate)
		{
			pbo_decimal_destroy(&tax_rate);
		}
		if (amt_taxed)
		{
			pbo_decimal_destroy(&amt_taxed);
		}

}


static void mso_retrieve_catv_billed_items(
	pcm_context_t	*ctxp,
	poid_t			*balgrp_pdp,
	poid_t			*service_pdp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
	{
		u_int64			id = (u_int64)1;
		u_int64			db = 0;
		pin_flist_t 	*s_flistp = NULL;
		pin_flist_t 	*sr_flistp = NULL;
		pin_flist_t		*flistp = NULL;
		pin_flist_t 	*ret_flistp = NULL;
		pin_flist_t		*rs_flistp = NULL;
		poid_t			*billinfo_pdp = NULL;
		poid_t			*svc_pdp = NULL;
		poid_t			*type1_pdp = NULL;
		poid_t			*type2_pdp = NULL;
		poid_t			*type3_pdp = NULL;
		poid_t			*type4_pdp = NULL;
		poid_t			*type5_pdp = NULL;
		poid_t			*type6_pdp = NULL;
		poid_t			*type7_pdp = NULL;
		poid_t			*type8_pdp = NULL;
		poid_t			*type9_pdp = NULL;
		poid_t			*type10_pdp = NULL;
		poid_t			*type11_pdp = NULL;
		poid_t			*type12_pdp = NULL;
		poid_t			*type13_pdp = NULL;
		poid_t			*pdp = NULL;
		int32			s_flags = 512;
		int32			temp = 0;
		int32 			i_status1 = ITEMS_STAT_PENDING;
		int32 			i_status2 = ITEMS_STAT_OPEN;
		time_t			*start_t = (time_t *)NULL;
		time_t			*end_t = (time_t *)NULL;
		void			*vp = NULL;
		char			msg[100];
	
		PIN_ERRBUF_CLEAR(ebufp);

		db = PIN_POID_GET_DB(balgrp_pdp);
		s_flistp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
//		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /bill 1, /billinfo 2, /balance_group 3 where 2.F1 = 3.F2 and 3.F3 = V3 and 2.F4 = 1.F5 and 2.F6 = V6", ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /billinfo 1, /balance_group 2 where 1.F1 = 2.F2 and 2.F3 = V3 and 1.F4 = V4", ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, balgrp_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
//		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACTUAL_LAST_BILL_OBJ, NULL, ebufp)
//		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
//		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
//		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_ID, BILLINFO_ID_CATV, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACTG_LAST_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACTG_NEXT_T, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Bill Dates search input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Bill Dates search output flist", ret_flistp);

		rs_flistp = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_RESULTS, 1, ebufp);
		start_t = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_ACTG_LAST_T, 1, ebufp); 
		vp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_ACTG_NEXT_T, 1, ebufp); 
        
		end_t = ((time_t *)vp);
		*end_t = ((*end_t)+21600);

		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Bill Details retrieval Error");
			return;
        }
		PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
		s_flistp = PIN_FLIST_CREATE(ebufp);
		sr_flistp = PIN_FLIST_CREATE(ebufp);

		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /item where F1 = V1 and (F2.type <> V2 and F3.type <> V3 and F4.type <> V4 and F5.type <> V5 and F6.type <> V6 and F7.type <> V7 and F8.type <> V8 and F9.type <> V9 and F10.type <> V10 and F11.type <> V11 and F12.type <> V12 and F13.type <> V13 and F14.type <> V14 and F15.type <> V15 and F16.type <> V16 and F17.type <> V17)  and (F18 = V18) ", ebufp);
//		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /item where F1 = V1 and (F2 >= V2 and F3 <= V3) and (F4.type <> V4 and F5.type <> V5 and F6.type <> V6 and F7.type <> V7 and F8.type <> V8 and F9.type <> V9 and F10.type <> V10 and F11.type <> V11 and F12.type <> V12 and F13.type <> V13 ) and (F14 = V14 or F15 = V15)", ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);

/*		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_CREATED_T, (void *)start_t, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_CREATED_T, (void *)end_t, ebufp);

		//ET Applicable only for Subscription, hence ignoring all the other items
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_DEPOSIT, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_CYC_ARR_ET, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_CYC_ARR_ANNUAL_AMC, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_MISC, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_ATP, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_OTP, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 10, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_ADN_FDP, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 11, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_HW, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 12, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_SRVC, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 13, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_VAT, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 14, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, &i_status1, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 15, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, &i_status2, ebufp);
*/
/*		
		
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 14, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_ECESS, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 15, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_SHECESS, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 16, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_ST, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
*/
		//ET Applicable only for Subscription, hence ignoring all the other items
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_VAT, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_SRVC, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_DEPOSIT, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_CYC_ARR_ET, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_CYC_ARR_ANNUAL_AMC, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_MISC, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_ATP, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_OTP, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 10, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_CYC_FWD_AMC, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 11, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_HW, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 12, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_TYPE_CYC_ARR_RENT, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 13, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_MIN_COMMIT, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 14, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_WS_SETTLEMENT, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 15, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_ADJ, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 16, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_PAYMENT, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 17, ebufp);
		pdp = PIN_POID_CREATE(db, ITEM_PYMT_REVERSAL, id, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 18, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, &i_status1, ebufp);

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, (void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILL_OBJ, (void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ITEM_TOTAL, (void *)NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Billed items search input flist", s_flistp);
		/* Get the Billed item details for the service_obj */
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &sr_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Billed items search output flist", sr_flistp);
		rs_flistp = NULL;
		rs_flistp = PIN_FLIST_FLD_GET(sr_flistp, PIN_FLD_RESULTS, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Billed items search Error");
			return;
        }
		*r_flistpp = PIN_FLIST_COPY(sr_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&sr_flistp, ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
 	return;
}

void 
mso_retrieve_gst_rate(
	pcm_context_t	*ctxp,
	char		*tax_code,
	poid_t		*acct_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*account_info_flistp = NULL;
        pin_flist_t             *lco_info_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t             *exemption_flistp = NULL;
	pin_flist_t		*profile_flistp = NULL;

	char			msg[STR_LEN];
	char			*binfo_id = NULL;
	char			*tax_map = NULL;
	char			*gst_state = NULL;
	char			key_str[200];
		
	poid_t			*srch_pdp = NULL;
	poid_t			*lco_pdp = NULL;

	int64			db = 0;
	int32			s_flags = 256;
	int32			id = 1;
	int32			srvc_type = 0;
	int32			gst_type = -1;
	int32			exemption_flag = 0;
	int32			*exemp_type = NULL;
	int32			elem_id = 0;
		
	time_t			current_t = 0;
	time_t			*start_t = NULL;
	time_t			*end_t = NULL;
		
	pin_decimal_t		*igst_rate = NULL;
	pin_decimal_t		*cgst_rate = NULL;
	pin_decimal_t   	*sgst_rate = NULL;
	pin_decimal_t		*zero_tax = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*one = pbo_decimal_from_str("1.0", ebufp);
	pin_decimal_t		*hundred = pbo_decimal_from_str("100.0", ebufp);
	pin_decimal_t		*tmp_rate = NULL;
	pin_decimal_t		*exempted_percent = NULL;
	pin_decimal_t		*apply_percent = NULL;

	pin_cookie_t		cookie = NULL;

	
	
	cm_cache_key_iddbstr_t	kids;
	int32			err = PIN_ERR_NONE;
	pin_flist_t		*flistp = (pin_flist_t *)NULL;
	
	db = PIN_POID_GET_DB(acct_pdp);
	current_t = pin_virtual_time(NULL);
	fm_mso_utils_get_profile(ctxp, acct_pdp, &profile_flistp, ebufp);
	if(profile_flistp)
	{
		lco_pdp = PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
		gst_type = *(int32 *)PIN_FLIST_FLD_GET(profile_flistp, PIN_FLD_CONN_TYPE, 0, ebufp);
	}
       	if(PIN_ERRBUF_IS_ERR(ebufp) || gst_type == -1)
       	{	
               	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error is getting GST Type");
		goto CLEANUP;
       	}

	/*r_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(r_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account Read Flds Input Flist", r_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, r_in_flistp, &r_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&r_in_flistp, ebufp);*/
	fm_mso_get_account_info(ctxp, acct_pdp, &account_info_flistp, ebufp);
        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR ,"Error Getting account details");
                goto CLEANUP;
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account Read Flds Output Flist", account_info_flistp);
	
	nameinfo_flistp = PIN_FLIST_ELEM_GET(account_info_flistp, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, 0, ebufp); 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Nameinfo flist", nameinfo_flistp);
	gst_state = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_STATE, 0, ebufp); 
	
	while ((exemption_flistp = PIN_FLIST_ELEM_GET_NEXT(account_info_flistp, PIN_FLD_EXEMPTIONS, &elem_id, 1, &cookie, ebufp )) != NULL)
	{
		exempted_percent = PIN_FLIST_FLD_GET(exemption_flistp, PIN_FLD_PERCENT, 0, ebufp);
		exemp_type = PIN_FLIST_FLD_GET(exemption_flistp, PIN_FLD_TYPE, 0, ebufp);
		start_t = PIN_FLIST_FLD_GET(exemption_flistp, PIN_FLD_START_T, 0, ebufp);
		end_t = PIN_FLIST_FLD_GET(exemption_flistp, PIN_FLD_END_T, 0, ebufp);
		if(exemp_type && *exemp_type == 0 && start_t && end_t && *start_t <= current_t && *end_t >= current_t)
		{
			apply_percent = pbo_decimal_subtract(one, exempted_percent, ebufp);
			exemption_flag = 1;
			break;
		}
	}

	if(exemption_flag != 1)	
	{
		/*r_lco_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_lco_in_flistp, PIN_FLD_POID, lco_pdp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO Account Read Flds Input Flist", r_in_flistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, r_lco_in_flistp, &r_lco_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&r_in_flistp, ebufp);*/
		fm_mso_get_account_info(ctxp, lco_pdp, &lco_info_flistp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR ,"Error Getting LCO account details");
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO Account Read Flds Output Flist", lco_info_flistp);

		elem_id = 0;
		cookie = NULL;
		exemption_flistp = NULL;
		while ((exemption_flistp = PIN_FLIST_ELEM_GET_NEXT(lco_info_flistp, PIN_FLD_EXEMPTIONS, &elem_id, 1, &cookie, ebufp )) != NULL)
		{
			exempted_percent = PIN_FLIST_FLD_GET(exemption_flistp, PIN_FLD_PERCENT, 0, ebufp);
			exemp_type = PIN_FLIST_FLD_GET(exemption_flistp, PIN_FLD_TYPE, 0, ebufp);
			start_t = PIN_FLIST_FLD_GET(exemption_flistp, PIN_FLD_START_T, 0, ebufp);
			end_t = PIN_FLIST_FLD_GET(exemption_flistp, PIN_FLD_END_T, 0, ebufp);
			if(exemp_type && *exemp_type == 0 && start_t && end_t && *start_t <= current_t && *end_t >= current_t)
			{
				apply_percent = pbo_decimal_subtract(one, exempted_percent, ebufp);
				exemption_flag = 1;
				break;
			}
		}
	}
		

	/*
	* If the cache is not enabled, short circuit right away
	*/
	if (fm_cust_pol_gst_rate_ptr == (cm_cache_t *)NULL) {
		return;
	}

		/*
		* See if the entry is in our data dictionary cache
		*/

	sprintf(key_str, "%s|%s", tax_code, gst_state);
	kids.id = 0;    /* Not relevant for us */
	kids.db = 0;    /* Not relevant for us */
	kids.str = key_str;

	flistp = cm_cache_find_entry(fm_cust_pol_gst_rate_ptr, &kids, &err);
	switch(err) {
		case PIN_ERR_NONE:
		break;
		case PIN_ERR_NOT_FOUND:
			PIN_ERRBUF_CLEAR(ebufp);       /* No real error... */
		break;
		default:
			pin_set_err(ebufp, PIN_ERRLOC_CM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					err, 0, 0, 0);
		return;
			/*****/
	}

	if (!PIN_ERRBUF_IS_ERR(ebufp) && flistp) 
	{
		tmp_rate = PIN_FLIST_FLD_GET(flistp, MSO_FLD_IGST_RATE, 0, ebufp);	//IGST
		igst_rate = pbo_decimal_divide(tmp_rate, hundred, ebufp);

		if(apply_percent && !pbo_decimal_is_null(apply_percent, ebufp))
		{
			pbo_decimal_multiply_assign(igst_rate, apply_percent, ebufp);
		}
		tmp_rate = PIN_FLIST_FLD_GET(flistp, MSO_FLD_SGST_RATE, 0, ebufp);	//SGST
		sgst_rate = pbo_decimal_divide(tmp_rate, hundred, ebufp);
		if(apply_percent && !pbo_decimal_is_null(apply_percent, ebufp))
		{
			pbo_decimal_multiply_assign(sgst_rate, apply_percent, ebufp);
		}
		tmp_rate = PIN_FLIST_FLD_GET(flistp, MSO_FLD_CGST_RATE, 0, ebufp);					//CGST
		cgst_rate = pbo_decimal_divide(tmp_rate, hundred, ebufp);
		if(apply_percent && !pbo_decimal_is_null(apply_percent, ebufp))
		{
			pbo_decimal_multiply_assign(cgst_rate, apply_percent, ebufp);
		}
		sprintf(msg, "IGST Tax Rate for Processing is : %s", pbo_decimal_to_str(igst_rate, ebufp));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		sprintf(msg, "SGST Tax Rate for Processing is : %s", pbo_decimal_to_str(sgst_rate, ebufp));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		sprintf(msg, "CGST Tax Rate for Processing is : %s", pbo_decimal_to_str(cgst_rate, ebufp));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		if(gst_type == 0)
		{
			PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_SGST_RATE, sgst_rate, ebufp);	//SGST
			PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_CGST_RATE, cgst_rate, ebufp);					//CGST
		}
		else if(gst_type == 1)
		{
			PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_IGST_RATE, igst_rate, ebufp);   //IGST
		}
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Tax Rate Retrieval error", ebufp);
		goto CLEANUP;
	}
	
CLEANUP:
	PIN_FLIST_DESTROY_EX(&account_info_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&lco_info_flistp, ebufp);
		
	PIN_POID_DESTROY(srch_pdp, ebufp);
		
	if(!pbo_decimal_is_null(zero_tax, ebufp))
		pbo_decimal_destroy(&zero_tax);
	if(!pbo_decimal_is_null(sgst_rate, ebufp))
		pbo_decimal_destroy(&sgst_rate);
	if(!pbo_decimal_is_null(cgst_rate, ebufp))
		pbo_decimal_destroy(&cgst_rate);
	if(!pbo_decimal_is_null(igst_rate, ebufp))
		pbo_decimal_destroy(&igst_rate);
	if(!pbo_decimal_is_null(hundred, ebufp))
		pbo_decimal_destroy(&hundred);
        if(!pbo_decimal_is_null(one, ebufp))
                pbo_decimal_destroy(&one);
	if(!pbo_decimal_is_null(apply_percent, ebufp))
		pbo_decimal_destroy(&apply_percent);
	return;
}

static void 
mso_gst_apply_tax(
	pcm_context_t	*ctxp,
	pin_decimal_t	*tax_rate,
	pin_decimal_t	*amt_gross,
	pin_flist_t	**ot_flistpp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	int32			j_level = 0;
	char			*descr = "GST Tax Component";
	pin_decimal_t		*tax_amt=NULL;
	pin_decimal_t		*amt_taxed = NULL;
	pin_decimal_t		*hundred = pbo_decimal_from_str("100.0", ebufp);
	char			msg [255];
	int32                  	precision = 6;
	int32                   round_mode = ROUND_HALF_UP;
	

	sprintf(msg, "Amount to be Taxed is %s.", pbo_decimal_to_str(amt_gross, ebufp)); 
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

	tax_amt = pbo_decimal_multiply(amt_gross, tax_rate, ebufp);
	pbo_decimal_round_assign(tax_amt, precision, round_mode, ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Standard tax rule applied.");
	sprintf(msg, "Tax Amount = %s.", pbo_decimal_to_str(tax_amt, ebufp)); 
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Tax Calculation ERROR");
		return;
	}
	
	PIN_FLIST_FLD_SET(*ot_flistpp, PIN_FLD_TAX, tax_amt, ebufp);
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_TAX, tax_amt, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_PERCENT, tax_rate, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT_GROSS, amt_gross, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT_TAXED, amt_taxed, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, descr, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_TYPE, &j_level, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"GST Processing Return FLIST", *r_flistpp);
	
	if (!pbo_decimal_is_null(tax_amt, ebufp))
	{
		pbo_decimal_destroy(&tax_amt);
	}
        if (!pbo_decimal_is_null(hundred, ebufp))
        {
                pbo_decimal_destroy(&hundred);
        }

	return;
}