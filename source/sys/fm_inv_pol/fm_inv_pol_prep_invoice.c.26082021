/*******************************************************************
 *
* Copyright (c) 1999, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_inv_pol_prep_invoice.c /cgbubrm_mainbrm.portalbase/2 2019/02/19 22:42:38 agangrad Exp $";

#endif

/*******************************************************************
 * Contains the PCM_OP_INV_POL_PREP_INVOICE operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
 
#include "pcm.h"
#include "ops/bill.h"
#include "ops/inv.h"
#include "ops/cust.h"
#include "ops/bal.h"
#include "ops/subscription.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_inv.h"

#include "fm_bal.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_subscription.h"
#include "pin_bal.h"

#include "fm_inv_pol_common.h"
#include "fm_bill_utils.h"

#include "pin_rate.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"

#define PENALTY_TAX_CODE "9984"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_inv_pol_prep_invoice(
		cm_nap_connection_t	*connp,
		int32			opcode,
		int32			flags,
		pin_flist_t		*i_flistp,
		pin_flist_t		**o_flistpp,
		pin_errbuf_t		*ebufp);
		
static void
fm_inv_pol_update_event_bi(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp);
	
static pin_decimal_t * 
fm_inv_pol_get_event_bi_amount(
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp);

static void
fm_inv_pol_prep_invoice(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_prep_invoice_currency(
	pcm_context_t		*ctxp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_prep_invoice_currency_opcode(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_prep_invoice_address(
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_prep_invoice_handle_invoice_data_overflow(
	pcm_context_t		*ctxp,
	pin_flist_t		*out_flistp,
	int32		        fld_no,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_prep_invoice_rmevents(
	pin_flist_t		*out_flistp,
	int32			fld_no,
	pin_errbuf_t		*ebufp);

static int
fm_inv_pol_prep_invoice_rmevents_hasimpact(
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

static void			
fm_inv_pol_prep_invoice_rmevents_balimpact(
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_prep_invoice_consolidate_events(
	pin_flist_t		*item_flistp,
	pin_errbuf_t		*ebufp);

static void			
fm_inv_pol_prep_invoice_consolidate_rerate_events(
	pin_flist_t		*item_flistp,
        poid_t			*a_pdp,
        int32			*type,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_prep_invoice_balance_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

static void 
fm_inv_pol_prep_invoice_parent_subord_plan_details(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
        int32           combined_inv_flags,
	int32		is_wholesale_billing,
	pin_errbuf_t	*ebufp);

static void
fm_inv_pol_prep_invoice_purchase_prod_disc_details(
        pcm_context_t   *ctxp,
        pin_flist_t     *out_flistp,
	poid_t		*acc_pdp,
	poid_t		*billinfo_pdp,
	poid_t		*bill_pdp,
	int32		*iter,
        pin_errbuf_t    *ebufp);

static void
fm_inv_pol_prep_invoice_add_parent(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
	int32			is_wholesale_billing,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_fetch_devices_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        int32                   combined_inv_flags,
	int32			is_wholesale_billing,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_update_device_totals(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_update_device_totals_items(
		pcm_context_t           *ctxp,
		pin_flist_t             *out_flistp,
		int32			fldno,
		pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_update_device_totals_events(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_flist_t             *dev_flistp,
        pin_flist_t             *svc_flistp,
        pin_flist_t             *item_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_get_subordinate_accounts(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_get_accounts_from_sub_items(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_aggregate_subordinate_accounts_totals(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_reorder_device_centric(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);
 
static void
fm_inv_pol_prep_invoice_add_account_num_to_devices(
        pcm_context_t   *ctxp,
        pin_flist_t     *out_flistp,
        pin_errbuf_t    *ebufp);
	
static void
fm_inv_pol_filter_services(
	pcm_context_t	*ctxp,
	poid_t		*bi_pdp,
	poid_t		*a_pdp,
	int32		parent_flags,
        int32           combined_inv_flags,
	pin_flist_t     **svc_bal_outflistpp,
	int32		is_wholesale_billing,
	pin_errbuf_t    *ebufp);

static void
fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp);

static void
fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes_for_service(
	pcm_context_t   *ctxp,
	int32		is_corr_bill,
	pin_flist_t     *svc_flistp,
	pin_errbuf_t    *ebufp);

static void
fm_inv_pol_prep_invoice_add_ratetag(
        pin_flist_t             *out_flistp,
        int32                   fld_no,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_addevents_ratetag(
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);

/* CI Corrective Invoicing */
static void
fm_inv_pol_prep_invoice_calculate_delta_amounts(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_drop_uncorrected_events_items(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_drop_uncorrected_events(
        pcm_context_t           *ctxp,
        pin_flist_t             *item_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_calculate_delta_amounts_bill(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_calculate_delta_amounts_items(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
	int32			fldno,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_calculate_delta_amounts_events(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_delta_process_adj_events(
        pcm_context_t           *ctxp,
	int32			add_corr_to_item,
        pin_flist_t             *out_flistp,
        pin_flist_t             *event_flistp,
        pin_flist_t             *item_flistp,
        pin_flist_t             *ci_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_inv_pol_prep_invoice_prior_event_item_corrections(
        pcm_context_t           *ctxp,
        pin_flist_t             *item_flistp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

static int
fm_inv_pol_prep_invoice_check_for_only_allocations(
		pcm_context_t	*ctxp,
		pin_flist_t	*out_flistp, 
		poid_t		*item1_pdp, 
		pin_errbuf_t	*ebufp);

static void
fm_inv_pol_prep_invoice_add_subord_trial_invoice_details(
        pin_flist_t     *big_flistp,
        pin_errbuf_t    *ebufp);

static void
fm_inv_pol_prep_invoice_add_subord_invoice_hyperlink_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        int32                   combined_inv_flags,
        pin_errbuf_t            *ebufp);

/* Uncomment the #define XML_WRITE_OUT_FILE 1
 * if you want the invoice to be dumped as an XML file.
 * Default will be not to dump the XML file.
 */
/*
#define XML_WRITE_OUT_FILE 1
*/

#ifdef XML_WRITE_OUT_FILE
/* function to dump the flist to an xml file. */
static void
fm_inv_pol_format_invoice_xml_write_out_file(
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

#endif



/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
extern void
fm_inv_make_invoice_sponsortotal_recalc(
	pin_flist_t		*event_flistp,
	pin_errbuf_t		*ebufp);

extern int32 fm_bill_check_wholesale_billinfo(
        pcm_context_t           *ctxp,
        poid_t                  *bi_pdp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Local Defines
 *******************************************************************/
#define ITEM_CYCLE_TAX	"/item/cycle_tax" 
#define RERATE_ADJUSTMENT "/event/billing/adjustment/event"

/*******************************************************************
 * Globals
 *******************************************************************/
extern int fm_inv_pol_show_rerate_details;
extern int32 perf_features_flags;
extern int32 fm_inv_pol_service_enabled;

extern int32 event_cache_enabled;


/*******************************************************************
 * Main routine for the PCM_OP_INV_POL_PREP_INVOICE operation.
 *******************************************************************/
void
op_inv_pol_prep_invoice(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_INV_POL_PREP_INVOICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_prep_invoice opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_inv_pol_prep_invoice: input flist", i_flistp);

	/***********************************************************
	 * Call the function that does the actual work.
	 ***********************************************************/
	fm_inv_pol_prep_invoice(ctxp, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_prep_invoice error", ebufp);
		goto cleanup;
	}

	/***************************************************
	 * Point the real return flist to the right thing.
	 ***************************************************/

	*o_flistpp = r_flistp;

	/***************************************************
	 * Debug: What we're sending back.
	 ***************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_inv_pol_prep_invoice return flist", r_flistp);

	return;
cleanup:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	*o_flistpp = NULL;
	return;
}


/*******************************************************************
 * fm_inv_pol_prep_invoice():
 *
 * PURPOSE:
 * In this opcode one can change the contents of the input flist
 * (i_flistp) in preparation prior to formatting the invoice.
 * It is here that any customizations of the invoice contents
 * should take place, by changing the code or calling additional
 * functions. 
 *
 * These functions for example could perform additional searches
 * in order to include further information in the final invoice
 * or calculate derived values.
 *
 * It is important to make sure that any changes to the structure of
 * the input flist in this policy opcode can be handled by the
 * policy opcode PCM_OP_INV_POL_FORMAT_INVOICE.
 *
 *
 * FLIST CONTAINING INVOICE INFORMATION:
 * The input flist (i_flistp) has the following structure by default:
 * 
 *	0 PIN_FLD_POID			POID
 *	0 PIN_FLD_BILLINFO		ARRAY []
 *	0 PIN_FLD_ACCTINFO		ARRAY []
 *	0 PIN_FLD_PAYINFO		ARRAY []
 *	0 PIN_FLD_AR_ITEMS		ARRAY []
 *	1   PIN_FLD_EVENTS		ARRAY []	
 *	0 PIN_FLD_SUB_ITEMS		ARRAY []
 *	1   PIN_FLD_EVENTS		ARRAY []	
 *	0 PIN_FLD_OTHER_ITEMS		ARRAY []
 *	1   PIN_FLD_EVENTS		ARRAY []	
 *	0 PIN_FLD_NAMEINFO		ARRAY []
 *	0 PIN_FLD_CURRENCIES		ARRAY []   
 *	0 PIN_FLD_INHERITED_INFO	SUBSTR [] (optionally)
 *
 * The above structure is the default, and can be changed here in
 * whatever form required, as long as the opcode PCM_OP_INV_POL_FORMAT_
 * INVOICE can handle it. 
 *
 * The contents of the input flist  can be reviewed by setting the CM's
 * debug level to '3'. Then look out for the flist logging 
 * 'op_inv_pol_prep_invoice: input flist'.
 *
 *
 * ADDING FURTHER INFORMATION ON THE INVOICE:
 * If any additional information is to be included in the invoice,
 * you MUST add it to the output flist.
 *
 *
 * Additional fields can be stored in an invoice object by adding them
 * to PIN_FLD_INHERITED_INFO. The field PIN_FLD_HEADER_STR and
 * PIN_FLD_HEADER_NUM are provided by default for storing additional
 * information in the /invoice object. 
 *
 * If any information that is required to be stored in the /invoice
 * object, then add a PIN_FLD_INHERITED_INFO substructure to the output flist.
 * Thereby this information will end up in the storable /invoice object 
 * (providing the fields exist), but will NOT be part of the 'formatted'
 * invoice. For example:
 *
 *	0 PIN_FLD_POID			POID
 *	0 PIN_FLD_BILLINFO		ARRAY []	
 *	0 PIN_FLD_ACCTINFO		ARRAY []
 *	...
 *	0 PIN_FLD_INHERITED_INFO	SUBSTR
 *	1   PIN_FLD_HEADER_STR		STR
 *	1   PIN_FLD_HEADER_NUM		UINT
 *	...
 *
 *
 * Currently the following functions are called in this policy opcode:   
 *
 *      fm_inv_pol_prep_invoice_handle_invoice_data_overflow()
 *		This function loops through events of the given item type.
 *		In each event, if invoice_data exists and is marked as 
 *		MAX_LENGTH_EXCEEDED, read the required event fields 
 *		and add those fields to the event.
 * 	fm_inv_pol_prep_invoice_address()
 *		This function determines what is the right address
 *		to use on the invoice
 *	fm_inv_pol_prep_invoice_currency()
 *		This function determines if a secondary currency
 *		exists, and if so converts certain amounts into this
 *		secondary currency
 *	fm_inv_pol_prep_invoice_rmevents()
 *		This function removes any events that do not have
 *		a currency balance impact
 *	
 *******************************************************************/
static void
fm_inv_pol_prep_invoice(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*payinfo_flistp = NULL;
	pin_flist_t		*binfo_flistp = NULL;
	pin_flist_t		*billinfo_flistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*corr_info_flistp = NULL;
	pin_flist_t		*bi_flistp = NULL;
	pin_flist_t             *bir_flistp = NULL;
	char                    *name = NULL;
	char			trial_watermark[256] = "Trial Invoice";
	int32			elemid = 0;
	int32			clean_elem = 0;
	int32			inv_type = 0;
	int32			in_inv_flags = 0;
	int32			is_corr_bill = 0;
	int32			inv_flags = 0;
	int32			combined_inv_flags = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		clean_cookie = NULL;

	int32                   is_wholesale_billing = 0;
	poid_t           	*ar_billinfo_pdp = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Take a copy of the input flist, because we change it,
	 * but don't own it.
	 ***********************************************************/
	out_flistp = PIN_FLIST_COPY(i_flistp, ebufp);


	temp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO,
                        0, 1, ebufp);
        if (temp_flistp) {
                ar_billinfo_pdp = (poid_t *)PIN_FLIST_FLD_GET(temp_flistp, 
						PIN_FLD_AR_BILLINFO_OBJ, 1, ebufp);
        }

        if (ar_billinfo_pdp == NULL) {
                vp = (poid_t*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
                bi_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_POID, (void *)vp, ebufp);
                PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_AR_BILLINFO_OBJ, (void *)NULL, ebufp);
                PCM_OPREF (ctxp, PCM_OP_READ_FLDS, 0, bi_flistp, &bir_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&bi_flistp, NULL);
                ar_billinfo_pdp = (poid_t *)PIN_FLIST_FLD_GET(bir_flistp,
						 PIN_FLD_AR_BILLINFO_OBJ, 1, ebufp);
        }

        is_wholesale_billing = fm_bill_check_wholesale_billinfo(ctxp, ar_billinfo_pdp, ebufp);

	PIN_FLIST_DESTROY_EX(&bir_flistp, NULL);

	/*
	 * Get the input flags from the input flist and the flags
	 * from the BILLINFO.PIN_FLD_FLAGS and create a combined flag
	 * for later use
	 */

	bi_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_BILLINFO,
                        	0, 0, ebufp);

        /* In regular trial billing - BILLINFO.PIN_FLD_FLAGS 
	 * contains PIN_FLD_INV_FLAGS 
	 */
        vp = PIN_FLIST_FLD_GET(bi_flistp,
                                PIN_FLD_FLAGS, 1, ebufp);
        if ( vp ) {
                inv_flags =  *(int32 *)vp;
        }

        /* In enhanced trial billing with -subord and parent_only option -
	 * input contains PIN_FLD_INV_FLAGS 
	 */
        vp = PIN_FLIST_FLD_GET(out_flistp,
                                PIN_FLD_INV_FLAGS, 1, ebufp);
        if ( vp ) {
                in_inv_flags =  *(int32 *)vp;
        }

        combined_inv_flags = inv_flags | in_inv_flags;


	/*
	 * Save some info from PAYINFO since prep_invoice_address()
	 * will be dropping PAYINFO
	 */

                
	elemid = 0;
	cookie = NULL;
	payinfo_flistp = PIN_FLIST_ELEM_GET(out_flistp,
				PIN_FLD_PAYINFO, 0, 0,  ebufp);
	vp = PIN_FLIST_FLD_GET(payinfo_flistp, 
				PIN_FLD_INV_TYPE, 0, ebufp);		
	if ( vp ) {
		inv_type =  *(int32 *)vp;
	}

        /************************************************
         * Read event data that could not be stored 
         * in INVOICE_DATA cache
         * 
         * Edit the following function to add any customized 
         * event fields
         *************************************************/
        if(event_cache_enabled){
                fm_inv_pol_prep_invoice_handle_invoice_data_overflow(ctxp, 
                                        out_flistp, PIN_FLD_AR_ITEMS, ebufp);

                fm_inv_pol_prep_invoice_handle_invoice_data_overflow(ctxp, 
                                        out_flistp, PIN_FLD_SUB_ITEMS, ebufp);

                fm_inv_pol_prep_invoice_handle_invoice_data_overflow(ctxp, 
                                        out_flistp, PIN_FLD_OTHER_ITEMS, ebufp);
        }

	/***********************************************************
	 * Set up the final invoicing address information.
	 * For A/R accounts, get address info from payinfo object.
	 * For subordinate accounts, get address info from nameinfo
	 * in the account object.
	 *
  	 * By default the output flist (out_flistp) will  have one
	 * PIN_FLD_NAMEINFO array at the root level and if required
	 * a PIN_FLD_PAYINFO.
	 ***********************************************************/
	fm_inv_pol_prep_invoice_address(out_flistp, ebufp);

	/***********************************************************
	 * Error?
	 * Check to see if any of the above function calls failed.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		goto cleanup;
	}


	/***********************************************************
	 * Check if a secondary currency is set in the /account 
	 * object. If so, then get hold of the secondary currency
	 * details. 
	 ***********************************************************/
	fm_inv_pol_prep_invoice_currency(ctxp, out_flistp, ebufp);

	/* BIP CORE INVOICING -  hierarchy support 
	 * this should be called before 
	 * fm_inv_pol_prep_invoice_fetch_devices_info
	 * because we need the subord info to help us fetch subord devices
	 */

	if (!is_wholesale_billing) {
		fm_inv_pol_prep_invoice_get_subordinate_accounts(ctxp, out_flistp, 
					ebufp);
	}
	/* BIP CORE INVOICING -  get the parent for this account */
	fm_inv_pol_prep_invoice_add_parent(ctxp, out_flistp, is_wholesale_billing, ebufp);
	

        if (!(perf_features_flags & INV_PERF_FEAT_SKIP_DEVICES_INFO)) {
		/* BIP CORE INVOICING -  get devices for this account 
		 * and services 
		 */
		fm_inv_pol_prep_invoice_fetch_devices_info(ctxp, out_flistp, 
					combined_inv_flags, is_wholesale_billing, ebufp);

		/* BIP CORE INVOICING -  update device totals */
		fm_inv_pol_prep_invoice_update_device_totals(ctxp, out_flistp, 
					ebufp);
	}

	/***********************************************************
	 * Error?
	 * Check to see if any of the above function calls failed.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		goto cleanup;
	}

	
	/***********************************************************
	* Call the function that filters out all non-balance 
	* impacting events and moves certain PIN_FLD_BAL_IMPACTS
	* information up to the event level. 
	***********************************************************/
	fm_inv_pol_prep_invoice_rmevents(out_flistp, 
					PIN_FLD_AR_ITEMS, ebufp);
	fm_inv_pol_prep_invoice_rmevents(out_flistp, 
					PIN_FLD_SUB_ITEMS, ebufp);
	fm_inv_pol_prep_invoice_rmevents(out_flistp, 
					PIN_FLD_OTHER_ITEMS, ebufp);

        /***********************************************************
        * Call the function to add rate tag at event level.
	***********************************************************/
        if ((perf_features_flags & INV_PERF_FEAT_NO_RMEVENTS)) {
                fm_inv_pol_prep_invoice_add_ratetag(out_flistp,
                                                PIN_FLD_AR_ITEMS, ebufp);
                fm_inv_pol_prep_invoice_add_ratetag(out_flistp,
                                                PIN_FLD_SUB_ITEMS, ebufp);
                fm_inv_pol_prep_invoice_add_ratetag(out_flistp,
                                                PIN_FLD_OTHER_ITEMS, ebufp);
        }


	/*
	 * CI Corrective Invoicing
	 */

	/************************************************************
	 * Check if the bill is CORRECTIVE
	 ************************************************************/
        billinfo_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_BILLINFO,
                        0, 0, ebufp);
        name = PIN_FLIST_FLD_GET(billinfo_flistp, PIN_FLD_NAME, 0,ebufp);
	if (name && (strncmp(name,PIN_OBJ_NAME_CORRECTIVE_BILL,
			strlen(PIN_OBJ_NAME_CORRECTIVE_BILL)) == 0)) {
       		is_corr_bill = 1; 
		fm_inv_pol_prep_invoice_calculate_delta_amounts (ctxp, 
						out_flistp, ebufp);

		/* Check for correction letter. Drop uncorrected events
		 * and items for correction letter.
		 */
		if ( inv_type &  PIN_INV_CORRECTIVE_TYPE_TO_USE ) {
			fm_inv_pol_prep_invoice_drop_uncorrected_events_items (
						ctxp, out_flistp, ebufp);
		}
	}

	/* BIP CORE INVOICING - reorder for device centric */
	/* comment this function out if you want the invoice to be 
	 * regular non-device centric.
	 * Default will be device centric.
	 * note: if SKIP_DEVICE_INFO perf_features flag is set
	 * there wont be any devices so it will be re-ordered
	 * under dummy devices and tax and account_no info
	 * will also be under the corresponding dummy devices.
	 */
	if(!fm_inv_pol_service_enabled){
		fm_inv_pol_prep_invoice_reorder_device_centric(ctxp, 
					out_flistp, ebufp);

		/* Aggregate the realtime and cycle taxes.
		 * call this after calling reorder_device_centric()
		 * because we need the device.service grouping to
		 * aggregate per service
		 */
		fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes(
					ctxp, out_flistp, ebufp);

		/* BIP CORE INVOICING - add the account numbers 
		 * to the subords 
	 	 * and devices arrays 
	 	 */
		fm_inv_pol_prep_invoice_add_account_num_to_devices(ctxp,
					out_flistp, ebufp);
	}


	/***********************************************************
         * Error?
         * Check to see if any of the above function calls failed.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                goto cleanup;
        }

        if (!(perf_features_flags & INV_PERF_FEAT_SKIP_BALANCE_DETAILS)) {
		fm_inv_pol_prep_invoice_balance_details(ctxp,out_flistp,ebufp);
	}

	/***********************************************************
         * Error?
         * Check to see if any of the above function calls failed.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                goto cleanup;
        }

        if (!(perf_features_flags & INV_PERF_FEAT_SKIP_PLAN_DETAILS)) {
		fm_inv_pol_prep_invoice_parent_subord_plan_details(ctxp,
					out_flistp, combined_inv_flags, is_wholesale_billing, ebufp);
	}

	/***********************************************************
         * Error?
         * Check to see if any of the above function calls failed.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                goto cleanup;
        }

	/**********************************************************
	* We added lot of arrays in the big_flistp for processing
	* special events. We cleanup all those additional arrays.
	*********************************************************/
	if (is_corr_bill) { 
		clean_elem = 0;
		clean_cookie = NULL;
		while (( temp_flistp = PIN_FLIST_ELEM_TAKE_NEXT(out_flistp,
			PIN_FLD_RESULTS, &clean_elem, 1,
                                &clean_cookie, ebufp )) != NULL ) {
			PIN_FLIST_DESTROY_EX( &temp_flistp, NULL );
		}
		 corr_info_flistp = PIN_FLIST_ELEM_GET(out_flistp,
                                PIN_FLD_CORRECTION_INFO, 0, 1, ebufp);
		if (corr_info_flistp) {
			clean_elem = 0;
			clean_cookie = NULL;
			while (( temp_flistp = PIN_FLIST_ELEM_TAKE_NEXT(out_flistp,
				PIN_FLD_TRANSFERS_INTO, &clean_elem, 1,
					&clean_cookie, ebufp )) != NULL ) {
				PIN_FLIST_DESTROY_EX( &temp_flistp, NULL );
			}
		}
	}

	/* Set watermark for trial invoices */
	if (combined_inv_flags & PIN_INV_TYPE_TRIAL_INVOICE) {
		   PIN_FLIST_FLD_SET( out_flistp, PIN_FLD_MESSAGE, 
				(void *)trial_watermark, ebufp );

		/* Save the subord bill details during subord trial bill run */
		if (!(in_inv_flags & PIN_INV_TYPE_PARENT) &&
                     (in_inv_flags & PIN_INV_TYPE_SUBORDINATE)){
			fm_inv_pol_prep_invoice_add_subord_trial_invoice_details(out_flistp, ebufp);
		}
        }

	/* HIT invoicing mode
	 * In this mode if the hierarchy size is greater than the threshold
	 * the parent invoice is separate and the subord invoices are 
	 * separate, In this case we build an index or table of contents
	 * with hyperlink info for each of the separate subordinate 
	 * invoices. Then in the HIT rtf templates we create a table 
	 * with these hyperlinks to the individual subordinate invoices.
	 * We call this function only when this invoice is PARENT with NO
	 * SUBORDS. (For parent with SUBORDS the subord invoices are part
	 * of the parent invoice so we don't have to build this 
	 * hyperlink index).
	 */
	if (  (combined_inv_flags & PIN_INV_TYPE_PARENT) &&
             !(combined_inv_flags & PIN_INV_TYPE_SUBORDINATE)) {
		fm_inv_pol_prep_invoice_add_subord_invoice_hyperlink_info (
				ctxp, out_flistp, combined_inv_flags, ebufp);
	}

	/***********************************************************
	 * You may add additional function calls to customer defined
	 * functions here. For example additional searches could be
	 * added, further manipulation of the input flist could be 
	 * undertaken, information removed from the input flist, etc.
	 *
	 * Example function call:
	 * fm_inv_pol_prep_invoice_myfunc(output_flistp, ebufp);
	 *
	 ***********************************************************/

#ifdef XML_WRITE_OUT_FILE
        /* Uncomment the #define XML_WRITE_OUT_FILE 1 near the top
         * of this file to use this function.
         * Default will be not to dump the XML file here.
         */
        /* dump the flist to an xml file. */
        fm_inv_pol_format_invoice_xml_write_out_file(out_flistp, ebufp);
#endif
	
	/***********************************************************
	 * Assign the final output flist to return flist pointer.
	 ***********************************************************/

	*ret_flistpp = out_flistp;
	return;

cleanup:
	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
	*ret_flistpp = (pin_flist_t *)NULL;
	return;

}


/*******************************************************************
 * fm_inv_pol_prep_invoice_currency():
 *
 * This function checks to see whether a secondary currency is
 * specified in the /account object.
 *	
 * If no secondary currency is specified this function will return
 * immediately. Otherwise it will call the opcode 
 * PCM_OP_BILL_CURRENCY_CONVERT_AMOUNTS to convert the following 
 * amounts:
 *	PIN_FLD_PREVIOUS_TOTAL
 *	PIN_FLD_SUBORDS_TOTAL
 *	PIN_FLD_CURRENT_TOTAL
 *	PIN_FLD_TOTAL_DUE
 *
 * The resulting amounts are put onto the out_flistp as a second 
 * array element of PIN_FLD_BILLINFO.
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_currency(
	pcm_context_t		*ctxp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*acct_flistp = NULL;
	pin_flist_t		*bill_flistp = NULL;
	pin_flist_t		*icur_flistp = NULL;
	pin_flist_t		*amount_flistp = NULL;
	pin_flist_t		*sec_flistp = NULL;
	pin_flist_t		*rsec_flistp = NULL;

	int32			elemid = 0;
	pin_cookie_t		cookie = NULL;
	int32			secondary_currency = 0;
	poid_t			*b_pdp = NULL;
	void			*vp = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_inv_pol_prep_invoice" \
	 "_currency: input flist (out_flistp)", out_flistp);

	/***********************************************************
	 * Get the secondary currency from the PIN_FLD_ACCTINFO
	 * array (/account object) to check if a secondary currency
	 * exists.
	 ***********************************************************/
	elemid = 0;
	cookie = NULL;
	acct_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, PIN_FLD_ACCTINFO,
			&elemid, 0, &cookie,  ebufp);
	
	vp = PIN_FLIST_FLD_GET(acct_flistp, PIN_FLD_CURRENCY_SECONDARY, 0, ebufp);

	if (vp) secondary_currency = *(int32 *)vp;
	else secondary_currency = 0;

	/***********************************************************
	 * If we don't have a secondary currency return from this
	 * function, else convert the currency amounts.
	 ***********************************************************/
	if (secondary_currency != 0) {

		/***********************************************************
	 	 * We have got a secondary currency
	 	 ***********************************************************/
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				 "We have got a secondary currency");

		/***************************************************
	 	 * Get hold of the PIN_FLD_BILLINFO array from the 
	 	 * output flist for the currency amounts.
	 	 ***************************************************/
		elemid = 0;
		cookie = NULL;
		bill_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, 
				PIN_FLD_BILLINFO, &elemid, 0, &cookie,  ebufp);

		/***********************************************************
	 	 * Build up the input flist for the opcode call 
		 * PCM_OP_BILL_CURRENCY_CONVERT_AMOUNTS in order to 
		 * convert the currency amounts.
		 *		 
		 * As input we need the primary currency, secondary currency,
		 * time stamp and by default the four primary currency 
		 * amounts which will be converted to secondary currency
		 * values. 
		 *
		 * Add further currency amounts to the input flist that
		 * you want to convert to the secondary currency.
	 	 ***********************************************************/
	 	icur_flistp = PIN_FLIST_CREATE(ebufp);

		b_pdp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_BILL_OBJ, 0, 
						 ebufp);
 
		PIN_FLIST_FLD_SET(icur_flistp, PIN_FLD_POID, (void *)b_pdp,
						 ebufp);

		vp = (void *)PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_CURRENCY,
						 0, ebufp);
		PIN_FLIST_FLD_SET(icur_flistp, PIN_FLD_CURRENCY_SRC, 
					(void *)vp, ebufp);

		PIN_FLIST_FLD_SET(icur_flistp, PIN_FLD_CURRENCY_DST, 
					(void *)&secondary_currency, ebufp);

		vp = (void *)PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_CREATED_T,
						0, ebufp);	
		PIN_FLIST_FLD_SET(icur_flistp, PIN_FLD_END_T, (void *)vp,
					ebufp);
	
		/***********************************************************
	 	 * Now add the currency values onto the input flist
		 * (icur_flistp).
		 * We want to get and convert the following currency value 
		 * from the bill object:
		 *	PIN_FLD_PREVIOUS_TOTAL
		 *	PIN_FLD_SUBORDS_TOTAL
		 *	PIN_FLD_CURRENT_TOTAL
		 *	PIN_FLD_TOTAL_DUE
		 *
		 * Note: Additional values can be added onto the input
		 *	 flist, as required.
	 	 ***********************************************************/
		amount_flistp = PIN_FLIST_ELEM_ADD(icur_flistp, 
					PIN_FLD_AMOUNTS, 0, ebufp);
		vp = (void *)PIN_FLIST_FLD_GET(bill_flistp, 
					PIN_FLD_PREVIOUS_TOTAL, 0, ebufp);	
		PIN_FLIST_FLD_SET(amount_flistp, PIN_FLD_AMOUNT_SRC, 
					(void *)vp, ebufp);
		
		amount_flistp = PIN_FLIST_ELEM_ADD(icur_flistp, 
					PIN_FLD_AMOUNTS, 1, ebufp);
		vp = (void *)PIN_FLIST_FLD_GET(bill_flistp, 
					PIN_FLD_SUBORDS_TOTAL, 0, ebufp);	
		PIN_FLIST_FLD_SET(amount_flistp, PIN_FLD_AMOUNT_SRC, 
					(void *)vp, ebufp);

		amount_flistp = PIN_FLIST_ELEM_ADD(icur_flistp, 
					PIN_FLD_AMOUNTS, 2, ebufp);
		vp = (void *)PIN_FLIST_FLD_GET(bill_flistp, 
					PIN_FLD_CURRENT_TOTAL, 0, ebufp);	
		PIN_FLIST_FLD_SET(amount_flistp, PIN_FLD_AMOUNT_SRC, 
					(void *)vp, ebufp);

		amount_flistp = PIN_FLIST_ELEM_ADD(icur_flistp, 
					PIN_FLD_AMOUNTS, 3, ebufp);
		vp = (void *)PIN_FLIST_FLD_GET(bill_flistp, 
					PIN_FLD_TOTAL_DUE, 0, ebufp);	
		PIN_FLIST_FLD_SET(amount_flistp, PIN_FLD_AMOUNT_SRC, 
					(void *)vp, ebufp);

		/***********************************************************
	 	 * Debug what we got. Input flist to opcode.
	 	 ***********************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		 "fm_inv_pol_prep_invoice_currency: input flist to convert" \
		 " currency opcode", icur_flistp);

		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_prep_invoice_currency: error building up" \
			" input flist for opcode", ebufp);

			goto cleanup;
			/***********/
		}

		/***********************************************************
	 	 * Execute opcode to convert currency amounts.
	 	 ***********************************************************/
 		fm_inv_pol_prep_invoice_currency_opcode(ctxp, icur_flistp,
                        &rsec_flistp, ebufp);

		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_prep_invoice_currency: error calling" \
			"function fm_inv_pol_prep_invoice_currency_opcode",
			ebufp);

			goto cleanup;
			/***********/
		}

		/***********************************************************
	 	 * Add the secondary currency values to the output flist as
		 * second PIN_FLD_BILLINFO element.
		 *
		 * NOTE: If the code above has been modified to include
		 * 	 additional currency values to be converted,
		 * 	 then the return value also has be assigned to the
		 * 	 output flist (out_flistp).
		 *
		 * 	 The values returned correspond to the input value
		 *	 array position according to the array element.
	 	 ***********************************************************/
		sec_flistp = PIN_FLIST_ELEM_ADD(out_flistp, PIN_FLD_BILLINFO,
						 1, ebufp);
		/* PIN_FLD_PREVIOUS_TOTAL */
		tmp_flistp = PIN_FLIST_ELEM_GET(rsec_flistp, PIN_FLD_AMOUNTS,	
						 0, 0, ebufp);
		vp = (void *)PIN_FLIST_FLD_GET(tmp_flistp, 
					PIN_FLD_AMOUNT_DST, 0, ebufp);	
		PIN_FLIST_FLD_SET(sec_flistp, PIN_FLD_PREVIOUS_TOTAL, 
					(void *)vp, ebufp);

		/* PIN_FLD_SUBORDS_TOTAL */
		tmp_flistp = PIN_FLIST_ELEM_GET(rsec_flistp, PIN_FLD_AMOUNTS,	
						 1, 0, ebufp);
		vp = (void *)PIN_FLIST_FLD_GET(tmp_flistp, 
					PIN_FLD_AMOUNT_DST, 0, ebufp);	
		PIN_FLIST_FLD_SET(sec_flistp, PIN_FLD_SUBORDS_TOTAL, 
					(void *)vp, ebufp);

		/* PIN_FLD_CURRENT_TOTAL */
		tmp_flistp = PIN_FLIST_ELEM_GET(rsec_flistp, PIN_FLD_AMOUNTS,	
						 2, 0, ebufp);
		vp = (void *)PIN_FLIST_FLD_GET(tmp_flistp, 
					PIN_FLD_AMOUNT_DST, 0, ebufp);	
		PIN_FLIST_FLD_SET(sec_flistp, PIN_FLD_CURRENT_TOTAL, 
					(void *)vp, ebufp);

		/* PIN_FLD_TOTAL_DUE */
		tmp_flistp = PIN_FLIST_ELEM_GET(rsec_flistp, PIN_FLD_AMOUNTS,	
						 3, 0, ebufp);
		vp = (void *)PIN_FLIST_FLD_GET(tmp_flistp, 
					PIN_FLD_AMOUNT_DST, 0, ebufp);	
		PIN_FLIST_FLD_SET(sec_flistp, PIN_FLD_TOTAL_DUE, 
					(void *)vp, ebufp);

		/***********************************************************
	 	 * Get the PIN_FLD_CURRENCY_RATE and 
	 	 * PIN_FLD_CURRENCY_OPERATOR from rsec_flistp and add it
		 * to the output flist. We add it to the PIN_FLD_CURRENCIES
		 * array, where the element ID equals our secondary
		 * currency (the element already exists).
	 	 ***********************************************************/
		tmp_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_CURRENCIES,
				secondary_currency, 0, ebufp);

		vp = PIN_FLIST_FLD_GET(rsec_flistp, PIN_FLD_CURRENCY_RATE, 0,
				ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CURRENCY_RATE,
				(void *)vp, ebufp);

		vp = PIN_FLIST_FLD_GET(rsec_flistp, PIN_FLD_CURRENCY_OPERATOR,
				0, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CURRENCY_OPERATOR,
				(void *)vp, ebufp);


		/***********************************************************
	 	 * Debug what we got. Flist with all the currency details.
	 	 ***********************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		 "fm_inv_pol_prep_invoice_currency: out_flistp " \
		 " with all the currency information", out_flistp);
	
	} /* end - if secondary_currency */

	/***********************************************************
	 * error? 
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "op_inv_prep_invoice_currency: error processing" \
		 " secondary currency", ebufp);

		goto cleanup;
		/***********/
	}


	/***********************************************************
	 * We return the out_flistp.  
	 ***********************************************************/

	/***********************************************************
	 * Free up all memory that has been allocated.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&icur_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rsec_flistp, NULL);

	return;

cleanup:
	PIN_FLIST_DESTROY_EX(&icur_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rsec_flistp, NULL);

	return;
}


/*******************************************************************
 * fm_inv_pol_prep_invoice_currency_opcode():
 *
 * This function executes the opcode 
 * PCM_OP_BILL_CURRENCY_CONVERT_AMOUNTS.
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_currency_opcode(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t	     *r_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got. Print input flist.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_currency_opcode: input flist",
			 i_flistp);

	/***********************************************************
	 * Execute the opcode.
	 ***********************************************************/
        if (perf_features_flags & INV_PERF_FEAT_USE_OPREF) {
		PCM_OPREF(ctxp, PCM_OP_BILL_CURRENCY_CONVERT_AMOUNTS, 0, 
			i_flistp, &r_flistp, ebufp);
        } else {
		PCM_OP(ctxp, PCM_OP_BILL_CURRENCY_CONVERT_AMOUNTS, 0, 
			i_flistp, &r_flistp, ebufp);
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_currency_opcode: return from" \
		 " opcode PCM_OP_BILL_CURRENCY_CONVERT_AMOUNTS error", ebufp);

		goto cleanup;
		/***********/
	}

	/***********************************************************
	 * Debug what we got. 
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_currency_opcode: output flist" \
		" PCM_OP_BILL_CURRENCY_CONVERT_AMOUNTS", r_flistp);


	/***********************************************************
	 * Assign the ret_flistpp.
	 ***********************************************************/
	*ret_flistpp = r_flistp;

	return;

cleanup:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	*ret_flistpp = (pin_flist_t *)NULL;

	return;
}



/*******************************************************************
 * fm_inv_pol_prep_invoice_handle_invoice_data_overflow():
 *
 * This function loops through events of the given item type.
 * In each event, if invoice_data exists and is marked as 
 * MAX_LENGTH_EXCEEDED, read the required event fields and add those 
 * fields to the event.
 * 
 * The event fields read in this function should be kept in sync 
 *  with corresponding logic in pipeline and real time code. 
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_handle_invoice_data_overflow(
	pcm_context_t		*ctxp,
	pin_flist_t		*out_flistp,
	int32			fld_no,
	pin_errbuf_t		*ebufp){

	pin_flist_t		*item_flistp = NULL;
	pin_cookie_t		item_cookie = NULL;
	int32			item_elemid = 0;

	pin_flist_t		*event_flistp = NULL;
	pin_cookie_t		event_cookie = NULL;
	int32			event_elemid = 0;

	pin_flist_t		*tmp_flistp = NULL;
        pin_flist_t		*tmp2_flistp = NULL;
	pin_cookie_t		tmp_cookie = NULL;
	int32			tmp_elemid = 0;

	void			*vp = NULL;
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
        char                    *event_typep = NULL;

	char			*svc_type = NULL;
	char			*svc_code = NULL;
	char			svc[61]; /*30(svc_code len) + 30 (svc_type len) */

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);


	/***********************************************************
	 * Loop through the items array.
	 ***********************************************************/
	while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
				fld_no, &item_elemid, 1, &item_cookie, ebufp))
				!= (pin_flist_t *)NULL) {

		/***************************************************
		 * Loop through the events array for this item
		 ***************************************************/
		event_flistp = NULL;
		event_cookie = NULL;
		event_elemid = 0;

		while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp,
			PIN_FLD_EVENTS, &event_elemid, 1,
			&event_cookie, ebufp)) != (pin_flist_t *)NULL) {

                        /*************************************************
	                 * Skip if invoice_data is not marked as 
                         * max length exceeded, 
	                 *************************************************/
                        vp = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_INVOICE_DATA, 1, ebufp);
                        if (!vp || 
                            (strcmp((char *)vp, MAX_LENGTH_EXCEEDED_STRING )) != 0) {
                                continue;
                        }

	              
                        /*************************************************
                         * Skip if BAL_IMPACTS already read.
                         * If invoice_data is set to MAX_LENGTH_EXCEEDED, 
                         * it is unlikely that the BAL_IMPACT is read. 
                         * This check is just for precaution
                         *************************************************/
                        tmp_cookie = NULL;
                        if((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(event_flistp,
		                PIN_FLD_BAL_IMPACTS, &tmp_elemid, 1,
		                &tmp_cookie, ebufp)) != (pin_flist_t *)NULL) {
                                continue;
                        }

                        /***************************************************
                         * Debug what event we found.
                         ***************************************************/
	                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		                "fm_inv_pol_prep_invoice_handle_invoice_data_overflow:"
                                " invoice_data length exceeded found",
		                 event_flistp);

                        /* drop PIN_FLD_INVOICE_DATA */
                        PIN_FLIST_FLD_DROP(event_flistp, PIN_FLD_INVOICE_DATA, ebufp);
              
                 
                        /***************************************************
                        * Construct input flist to read event fields
                        *
                        * NOTE: If you have customization elsewhere that 
                        * includes more event object fields into the 
                        * PIN_FLD_INVOICE_DATA cache, the same fields 
                        * need to be included in the event flist by
                        * reading them through the call to PCM_OP_READ_FLDS below.
                        ***************************************************/
                        i_flistp = PIN_FLIST_CREATE(ebufp);   

                        vp = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_EVENT_OBJ, 0, ebufp);
                        event_typep = (char*) PIN_POID_GET_TYPE( (poid_t*) vp ) ;

                        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID,           vp, ebufp);
                               
                        tmp_flistp = PIN_FLIST_ELEM_ADD(i_flistp,  PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, ebufp);
                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RESOURCE_ID, NULL, ebufp);
                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_QUANTITY,    NULL, ebufp);
                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RATE_TAG,    NULL, ebufp);
                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ITEM_OBJ,    NULL, ebufp);
                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_AMOUNT,      NULL, ebufp);
                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_IMPACT_TYPE, NULL, ebufp);
                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DISCOUNT,    NULL, ebufp);
                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TAX_CODE,    NULL, ebufp);
                        

                        /* Additional fields to read for /event/delayed/session/telco */
                        if (strstr(event_typep, "/event/delayed/session/telco" )){
                                tmp_flistp = PIN_FLIST_SUBSTR_ADD(i_flistp, PIN_FLD_TELCO_INFO, ebufp);
                                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRIMARY_MSID, NULL, ebufp);
                                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CALLED_TO   , NULL, ebufp);
                                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SVC_TYPE ,    NULL, ebufp);
                                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SVC_CODE ,    NULL, ebufp);
                                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_USAGE_CLASS,  NULL, ebufp);

                                if (strstr(event_typep, "/event/delayed/session/telco/gsm" )){
                                        tmp_flistp = PIN_FLIST_SUBSTR_ADD(i_flistp, PIN_FLD_GSM_INFO, ebufp);
                                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NUMBER_OF_UNITS ,   NULL, ebufp);
                                }

                                if (strstr(event_typep, "/event/delayed/session/telco/gprs" )){
                                        tmp_flistp = PIN_FLIST_SUBSTR_ADD(i_flistp, PIN_FLD_GPRS_INFO, ebufp);
                                        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NUMBER_OF_UNITS ,   NULL, ebufp);
                                }
                        }


                        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, i_flistp, &r_flistp, ebufp);       
                        if (PIN_ERR_IS_ERR(ebufp)) {
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,  
                                        "fm_inv_pol_prep_invoice_handle_invoice_data_overflow: " \
	                                " error reading event fields", i_flistp);
                                PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
                                goto cleanup;
                        }

                        /* set fields for /event/delayed/session/telco */
                        if (strstr(event_typep, "/event/delayed/session/telco" )){
                                tmp_flistp = (poid_t*) PIN_FLIST_FLD_TAKE(r_flistp, PIN_FLD_TELCO_INFO, 0, ebufp);
                                
                                /*set calling number */                              
                                vp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_PRIMARY_MSID, 1, ebufp);
                                if(vp) PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_CALLING_NUMBER, vp, ebufp);
                                
                                /*set called number */
                                vp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_CALLED_TO , 1, ebufp);
                                if(vp) PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_CALLED_NUMBER, vp, ebufp);

                                /*set usage class*/
                                vp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_USAGE_CLASS , 1, ebufp);
                                if(vp) PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_USAGE_CLASS, vp, ebufp);

                                /*set SVC code*/
                                svc_type = (char *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SVC_TYPE, 1, ebufp);
                                svc_code = (char *)PIN_FLIST_FLD_GET(tmp_flistp , PIN_FLD_SVC_CODE, 1, ebufp);
                                if(svc_type && svc_code){
                                        pin_snprintf(svc, sizeof(svc), "%s%s", svc_type, svc_code);
                                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_SVC_CODE, svc, ebufp);
                                }        

                                PIN_FLIST_DESTROY_EX(&tmp_flistp, NULL); 
  
                                /*set number of units*/
                                if (strstr(event_typep, "/event/delayed/session/telco/gsm" )){
                                        tmp_flistp = (poid_t*) PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_GSM_INFO, 0, ebufp);
                                        vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NUMBER_OF_UNITS, 1, ebufp);
                                        if(vp) PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_NUMBER_OF_UNITS, vp, ebufp);
                                        PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_GSM_INFO, ebufp); 
                                }
              
                                if (strstr(event_typep, "/event/delayed/session/telco/gprs" )){
                                        tmp_flistp = (poid_t*) PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_GPRS_INFO, 0, ebufp);
                                        vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NUMBER_OF_UNITS, 1, ebufp);
                                        if(vp) PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_NUMBER_OF_UNITS, vp, ebufp);
                                        PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_GPRS_INFO, ebufp); 
                                }
                        }
                           

                        /* drop PIN_FLD_POID and concat to event flist*/
                        PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp); 
                        PIN_FLIST_CONCAT(event_flistp, r_flistp, ebufp);

                        
                        PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
                        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

                        /* 
                         * calculates the total amounts for the given event by
                         * looping through the PIN_FLD_BAL_IMPACT array and adding up the
                         * PIN_FLD_AMOUNTS if the PIN_FLD_ITEM_OBJ is the same as the POID
                         * passed into the function.
                         */
                        fm_inv_make_invoice_sponsortotal_recalc( event_flistp, ebufp);

   
                }   /* end event loop */

	}	/* end item loop */


cleanup :

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_inv_pol_prep_invoice_handle_invoice_data_overflow error",  ebufp);                
        }
	return;
}



/*******************************************************************
 * fm_inv_pol_prep_invoice_address():
 *
 * Set up the final invoicing address information.
 * For A/R accounts, get address info from payinfo object.
 * For subordinate accounts, get address info from nameinfo
 * in the account object.
 *
 * The final address information is added to the input flist 
 * (out_flistp) as PIN_FLD_NAMEINFO at the root level.
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_address(
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*a_flistp = NULL;
	pin_flist_t		*bi_flistp = NULL;
	pin_flist_t		*addr_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*payinfo_flistp = NULL;
	pin_bill_type_t		*bill_type = NULL;

	void			*vp = NULL;
	int32			elemid = 0;
	pin_cookie_t		cookie = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got. Print input flist.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	 "fm_inv_pol_prep_invoice_address: input flist (out_flistp)",
	  out_flistp);


	/***********************************************************
	 * Get hold of the PIN_FLD_ACCTINFO array.
	 ***********************************************************/
	a_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_ACCTINFO,
						0, 0, ebufp);

	/***********************************************************
	 * Move the whole PIN_FLD_NAMEINFO array to the root level
	 * of the output flist (out_flistp). 
	 ***********************************************************/
	elemid = 0;
	cookie = NULL;
	while ((tmp_flistp = PIN_FLIST_ELEM_TAKE_NEXT(a_flistp,
		PIN_FLD_NAMEINFO, &elemid, 1, &cookie, ebufp)) !=
		(pin_flist_t *)NULL) {

		PIN_FLIST_ELEM_PUT(out_flistp, tmp_flistp, PIN_FLD_NAMEINFO,
			elemid, ebufp);
	}

	/***********************************************************
	 * Get hold of the PIN_FLD_BILLINFO array.
	 ***********************************************************/
	bi_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_BILLINFO,
						0, 0, ebufp);


	/***********************************************************
	 * Check if we have a bill type 'invoice', in which case we
	 * move parts of the PIN_FLD_INV_INFO to the root level of 
	 * out_flistp. If it is any other bill type we remove the
	 * whole PIN_FLD_PAYINFO array.
	 *
	 * If it's not an invoice-type account, we will construct
	 * PIN_FLD_NAME field in the NAMINFO array in order to 
	 * conform with the case in invoice-type accounts.
	 ***********************************************************/
	bill_type = (pin_bill_type_t *)PIN_FLIST_FLD_GET(bi_flistp, 
		PIN_FLD_PAY_TYPE, 1, ebufp);

	if (bill_type != NULL) {
	if (*bill_type == PIN_BILL_TYPE_INVOICE) {

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
		 "fm_inv_pol_prep_invoice_address: a 'invoice' bill");

		/****************************************************
	 	 * Get the PIN_FLD_INV_INFO from PIN_FLD_PAYINFO
		 * array element.
	 	 ***************************************************/
		elemid = 0;
		cookie = NULL;
		payinfo_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
				  PIN_FLD_PAYINFO, &elemid, 0, &cookie,  ebufp);

		elemid = 0;
		cookie = NULL;
		tmp_flistp = PIN_FLIST_ELEM_TAKE_NEXT(payinfo_flistp, 
				PIN_FLD_INV_INFO, &elemid, 0, &cookie, ebufp);

		/****************************************************
	 	 * Move across the required fields from the PIN_FLD_
		 * INV_INFO array to PIN_FLD_NAMEINFO (addr_flistp). 
	 	 ***************************************************/
		elemid = 0;
		cookie = NULL;
		addr_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
				PIN_FLD_NAMEINFO, &elemid, 0, &cookie, ebufp);

		vp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_ADDRESS, 0, ebufp);
		PIN_FLIST_FLD_PUT(addr_flistp, PIN_FLD_ADDRESS, 
					(void *)vp, ebufp);

		vp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_CITY, 0, ebufp);
		PIN_FLIST_FLD_PUT(addr_flistp, PIN_FLD_CITY, 
					(void *)vp, ebufp);

		vp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_STATE, 0, ebufp);
		PIN_FLIST_FLD_PUT(addr_flistp, PIN_FLD_STATE, (void *)vp,
					 ebufp);

		vp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_ZIP, 0, ebufp);
		PIN_FLIST_FLD_PUT(addr_flistp, PIN_FLD_ZIP, (void *)vp,
					 ebufp);

		vp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_COUNTRY, 0, ebufp);
		PIN_FLIST_FLD_PUT(addr_flistp, PIN_FLD_COUNTRY,
					(void *)vp, ebufp);

		vp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
		PIN_FLIST_FLD_PUT(addr_flistp, PIN_FLD_NAME,
					(void *)vp, ebufp);

		/****************************************************
	 	 * Move the rest of PIN_FLD_INV_INFO to the root
		 * level.
	 	 ***************************************************/
		elemid = 0;
		cookie = NULL;
		PIN_FLIST_ELEM_PUT(out_flistp, tmp_flistp, PIN_FLD_INV_INFO,
			elemid, ebufp);
	} else { /* All other types of accounts */
		char cust_name[BUFSIZ] = {""};

		elemid = 0;
		cookie = NULL;
		while ((addr_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
			PIN_FLD_NAMEINFO, &elemid, 1, &cookie, ebufp)) !=
			(pin_flist_t *)NULL) {

			vp = PIN_FLIST_FLD_GET(addr_flistp,
				PIN_FLD_FIRST_NAME, 0, ebufp);
			if (vp) {
				strcpy(cust_name, (char *)vp);
			}
			vp = PIN_FLIST_FLD_GET(addr_flistp,
				PIN_FLD_MIDDLE_NAME, 0, ebufp);
			strcat(cust_name, " ");

			if (vp && (*(char *)vp != '\0')) {
				strcat(cust_name, (char *)vp);
				strcat(cust_name, " ");
			}

			vp = PIN_FLIST_FLD_GET(addr_flistp,
				PIN_FLD_LAST_NAME, 0, ebufp);
			if (vp) {
				strcat(cust_name, (char *)vp);
			}
			PIN_FLIST_FLD_SET(addr_flistp, PIN_FLD_NAME,
				(void *)cust_name, ebufp);
		}
	}
	}
	
	/***********************************************************
	 * Remove the PIN_FLD_PAYINFO array, or what's left of it. 
	 ***********************************************************/
	PIN_FLIST_ELEM_DROP(out_flistp, PIN_FLD_PAYINFO, PIN_ELEMID_ANY, ebufp);

	
	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_prep_invoice_address: error" \
			" setting up final address information", ebufp);

		goto cleanup;
		/***********/
	}

	/***********************************************************
	 * Debug what we got. Flist with the final address 
	 * information.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	 "fm_inv_pol_prep_invoice_address: flist (out_flistp)" \
	 " with the final address information", out_flistp);

	/***********************************************************
	 * We return the out_flistp.  
	 ***********************************************************/
	return;

cleanup:
	return;
}


/*******************************************************************
 * fm_inv_pol_prep_invoice_rmevents():
 *
 * This function removes any events that do not have a currency 
 * balance impact. 
 * It also calls a function to 'merge' the balance impact array
 * into the event, thereby removing the separate balance impact
 * array.
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_rmevents(
	pin_flist_t		*out_flistp,
	int32			fld_no,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*event_flistp = NULL;
	pin_flist_t		*tmpevent_flistp = NULL;
	pin_cookie_t		cookie = NULL;
	int32			elemid = 0;
	pin_cookie_t		inner_cookie = NULL;
	int32			inner_elemid = 0;

	int			has_impact = PIN_BOOLEAN_FALSE;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_inv_pol_prep_invoice_rmevents: input flist (out_flistp)", 
		out_flistp);

	/***********************************************************
	 * Loop through the items array.
	 ***********************************************************/
	while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
				fld_no, &elemid, 1, &cookie, ebufp))
				!= (pin_flist_t *)NULL) {

		/***************************************************
	    	 * Debug what we got.
	 	 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_inv_pol_prep_invoice_remevents: item flist",
			 item_flistp);

		/***************************************************
	    	 * Create temporary event flist
	 	 ***************************************************/
		tmpevent_flistp = PIN_FLIST_CREATE(ebufp);

		/***************************************************
		 * Loop through the events array for this item
		 ***************************************************/
		event_flistp = NULL;
		inner_cookie = NULL;
		inner_elemid = 0;

		while ((event_flistp = PIN_FLIST_ELEM_TAKE_NEXT(item_flistp,
			PIN_FLD_EVENTS, &inner_elemid, 1,
			&inner_cookie, ebufp)) != (pin_flist_t *)NULL) {

				PIN_FLIST_ELEM_PUT(tmpevent_flistp, 
							event_flistp,
							PIN_FLD_EVENTS,
							inner_elemid, ebufp);
                }

		fm_inv_pol_prep_invoice_consolidate_events(
				tmpevent_flistp, ebufp);

		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_inv_pol_prep_invoice_rmevents: error" \
				" consolidating events ", ebufp);
			goto cleanup;
		}

                if (!(perf_features_flags & INV_PERF_FEAT_NO_RMEVENTS)) {

			event_flistp = NULL;
			inner_cookie = NULL;
			inner_elemid = 0;

			while ((event_flistp = PIN_FLIST_ELEM_TAKE_NEXT(tmpevent_flistp,
				PIN_FLD_EVENTS, &inner_elemid, 1,
				&inner_cookie, ebufp)) != (pin_flist_t *)NULL) {

				has_impact =
					 fm_inv_pol_prep_invoice_rmevents_hasimpact(
						event_flistp, ebufp);

				if (has_impact == PIN_BOOLEAN_FALSE) {

					PIN_FLIST_DESTROY_EX(&event_flistp, NULL);
		
				}
				else {

					fm_inv_pol_prep_invoice_rmevents_balimpact(
						event_flistp, ebufp);
					PIN_FLIST_ELEM_PUT(item_flistp, 
								event_flistp,
								PIN_FLD_EVENTS,
								inner_elemid, ebufp);
				}

			}	/* end event loop */

			if (PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_inv_pol_prep_invoice_rmevents: error" \
					" fm_inv_pol_prep_invoice_rmevents_balimpact()",
					ebufp);
				goto cleanup;
			}

		}

		/***************************************************
		 * If INV_PERF_FEAT_NO_RMEVENTS is set,
		 * copy tmpevent_flistp back to item_flistp
		 ***************************************************/

		else {
			inner_cookie = NULL;
			inner_elemid = 0;

			while ((tmp_flistp = PIN_FLIST_ELEM_TAKE_NEXT(tmpevent_flistp,
					PIN_FLD_EVENTS, &inner_elemid, 1,
					&inner_cookie, ebufp)) != (pin_flist_t *)NULL) {

				PIN_FLIST_ELEM_PUT(item_flistp, tmp_flistp, 
							PIN_FLD_EVENTS, inner_elemid,
							ebufp);
			}
		}
	
		/***********************************************************
	 	* Error?
	 	***********************************************************/
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_inv_pol_prep_invoice_rmevents: error" \
				" reconstructing item flist", ebufp);
			goto cleanup;
		}


		/***************************************************
	    	* Destroy the tmpevent_flistp
	 	***************************************************/
		PIN_FLIST_DESTROY_EX(&tmpevent_flistp, NULL);

		/***************************************************
	    	 * Debug what we got. See what's the difference.
	 	 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_inv_pol_prep_invoice_remevents: item flist (RM)",
			 item_flistp);

	}	/* end item loop */


cleanup :

	PIN_FLIST_DESTROY_EX(&tmpevent_flistp, NULL);

	/***********************************************************
	* We return the out_flistp.
	************************************************************/

	return;
}


/*******************************************************************
 * fm_inv_pol_prep_invoice_rmevents_hasimpact():
 *
 * This function determines whether an event has  any balance 
 * impact. 
 * If there is no impact 0 (FALSE) is returned, else
 * a value > 0 is returned.
 *
 *******************************************************************/
static int			
fm_inv_pol_prep_invoice_rmevents_hasimpact(
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp)
{
	int32			counter = 0;

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_inv...hasimpact:" \
				" input flistp", i_flistp);

	
	/***********************************************************
	 * See if we have any PIN_FLD_BAL_IMPACTS array.
	 ***********************************************************/

	counter = PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_BAL_IMPACTS, ebufp);
                  
	/* NSC - DEBUG purpose only */
	if (counter == 0) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_inv...hasimpact:" \
		 " event HAS NO balance  impact");
	}
	else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_inv...hasimpact:" \
		 " event HAS balance impact"); 
	}

	return(counter);
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_rmevents_balimpact():
 *
 * This function moves the PIN_FLD_RATE to the event level.
 * Then it removes the PIN_FLD_BAL_IMPACTS array. 
 *
 * We assign the first PIN_FLD_RATE value we come across, but if
 * there is more than one rate tag, we  add an '*' to the rate tag
 * string.
 *
 *******************************************************************/
static void			
fm_inv_pol_prep_invoice_rmevents_balimpact(
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*flistp = NULL;
	pin_cookie_t		cookie = NULL;
	int32			elemid = 0;
	int32			counter = 0;
	void			*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_inv...balimpact:" \
				" input flistp", i_flistp);

	/***********************************************************
	 * Loop through the balance impact array.
	 ***********************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
	 	  &elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		/***************************************************
	 	 * Move the rate tag to the event level. 
		 * We pick up the first rate tag we come across.
	 	 ***************************************************/
		if (counter == 0) {
			vp = PIN_FLIST_FLD_TAKE(flistp, PIN_FLD_RATE_TAG, 0, 
						 ebufp);
		}
		counter++;
	
		/***************************************************
	 	 * Drop the PIN_FLD_BAL_IMPACT element. 
	 	 ***************************************************/
		PIN_FLIST_ELEM_DROP(i_flistp, PIN_FLD_BAL_IMPACTS, elemid,
			 ebufp);

		/***************************************************
	 	 * Reset cookie, to make sure the loop does not skip
		 * any fields.
	 	 ***************************************************/
		cookie = (pin_cookie_t)NULL;
	}

	/***********************************************************
	 * If there is more than one PIN_FLD_BAL_IMPACT, then
 	 * add a '*' to the rate tag string before moving it to
	 * the event level.
	 ***********************************************************/
	if ((counter > 1) && (vp)) {
		vp = pin_realloc(vp, strlen((char *)vp) + 2);
		if (vp) {
			sprintf((char *)vp, "%s*", (char *)vp);
			PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_LABEL, vp,
						 ebufp);
		} else {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_inv_pol_prep_invoice no memory", ebufp);
			return;
		}
	}
	else {
		PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_LABEL, vp, ebufp);
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_prep_invoice_rmevents_balimpact: error" \
			" removing PIN_FLD_BAL_IMPACTS", ebufp);
	}

	/***********************************************************
	 * We return the event that was passed in (i_flistp).
	 ***********************************************************/

	return;
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_consolidate_events():
 *
 * If the system is configured to show rerated details (Default 
 * is to not show the rerated details), then return;
 * ELSE walk thru the events and drop all events that have been
 * re-rated (RERATE_OBJ set). Balance impacts for re-rated (shadow)
 * events are already consolidated into the last re-rated event.
 *
 *******************************************************************/
static void			
fm_inv_pol_prep_invoice_consolidate_events(
	pin_flist_t		*item_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*flistp = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		prev_cookie = NULL;
	poid_t			*re_pdp = NULL;
        poid_t			*e_pdp = NULL;
	int32			elemid = 0;
        int32			dflag = PIN_BOOLEAN_TRUE;
        int32			event_type = 0;
        char*			etype = NULL;
        void			*vp = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * See, if we need to do anything.
	 ***********************************************************/
	if (!fm_inv_pol_show_rerate_details) {
		return;
	}
	
	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Consolidate event i/p flist:" \
				" input flistp", item_flistp);

	/***********************************************************
	 * Loop through the events. 
	 ***********************************************************/

	cookie = NULL;
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp, PIN_FLD_EVENTS,
	 	  	&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		/***************************************************
	 	 * If event is re-rated, drop it from the flist.
		 * The bal-impacts for the re-rated events are 
		 * already consolidated into the last re-rated
		 * event. For RE re-rating, if shadow events are
		 * generated, (re-rating results in either a shadow
		 * event of the same type or results in an adjustment
		 * event based on billing date and GL Posting date)
		 * the last re-rated event is the consolidated event.
		 * eg., Re-rating of E1 results in E2. RE Re-rating of
		 * E1 & E2 results in E3, thus forming a chain:
		 * E1 ---> E2 ---> E3. The E3 event in the chain is
		 * the consolidated event. Events E1 & E2 have
		 * redundant information and need to be dropped.
		 * For cycle, purchase or cancel event, rerating
		 * creates a new CF/P/C events, as well as a new
		 * adjustment event. But for usage events, only
		 * an adjustment event is created. Hence, Invoicing
		 * needs to handle shadow events differently.
	 	 ***************************************************/

		re_pdp = (poid_t *)PIN_FLIST_FLD_GET(flistp, 
				PIN_FLD_RERATE_OBJ, 1, ebufp);

		if (re_pdp && !PIN_POID_IS_NULL(re_pdp)) {

		     /***********************************************************
	 	     * Mark this flist to be dropped later.
	 	     ***********************************************************/
                     PIN_FLIST_FLD_SET(flistp, PIN_FLD_BOOLEAN, &dflag, ebufp);

		     /***********************************************************
	 	     * Now get the event type.
	 	     ***********************************************************/
                     e_pdp = (poid_t *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_EVENT_OBJ, 1, ebufp);

		     if (e_pdp && !PIN_POID_IS_NULL(e_pdp)) {
			     etype = (char *)PIN_POID_GET_TYPE(e_pdp);
		     } else {
			     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			         "fm_inv_pol_prep_invoice_consolidate_events: error" \
				 " getting PIN_FLD_EVENT_OBJ field from flistp ", 
			         ebufp);
			     goto cleanup;
		     }

                     if ((strstr(etype, PIN_OBJ_TYPE_EVENT_CYCLE_BASE)) ||
                         (strstr(etype, PIN_OBJ_TYPE_EVENT_PRODUCT_FEE_PURCHASE)) ||
                         (strstr(etype, PIN_OBJ_TYPE_EVENT_PRODUCT_FEE_CANCEL)) ||
                         (strstr(etype, PIN_OBJ_TYPE_EVENT_PRODUCT_ACTION_PURCHASE)) ||
                         (strstr(etype, PIN_OBJ_TYPE_EVENT_PRODUCT_ACTION_CANCEL)) ||
                         (strstr(etype, PIN_OBJ_TYPE_EVENT_DEAL_PURCHASE)) ||
                         (strstr(etype, PIN_OBJ_TYPE_EVENT_DEAL_CANCEL))) {

                            event_type = INV_EVENT_CYCLE_PURCHASE_CANCEL ;

                     } else if ( strstr(etype, "/event/billing/debit") ||
				strstr(etype, "/event/billing/payment") ||
				strstr(etype, "/event/billing/writeoff") ||
				strstr(etype, "/event/billing/dispute") ||
				strstr(etype, "/event/billing/settlement") ||
				strstr(etype, "/event/billing/refund") ||
				strstr(etype, "/event/billing/charge") ||
				strstr(etype, "/event/billing/item") ||
				strstr(etype, "/event/billing/cycle/tax") ||
				strstr(etype, "/event/billing/cdc_update") ||
				strstr(etype, "/event/billing/cdcd_update") ||
				strstr(etype, "/event/billing/mfuc_update") ||
				strstr(etype, "/event/billing/lcupdate") ||
				strstr(etype, "/event/billing/reversal")) {

                            event_type = INV_EVENT_SKIPPED ;

                     } else {

                            event_type = INV_EVENT_USAGE ;
                     }

		     /***********************************************************
	 	     * Now work on the adjustment events.
	 	     ***********************************************************/

                     fm_inv_pol_prep_invoice_consolidate_rerate_events(
				item_flistp, re_pdp, &event_type, ebufp);

		     if (PIN_ERR_IS_ERR(ebufp)) {
			     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			         "fm_inv_pol_prep_invoice_consolidate_events: error" \
				 " consolidating adjustment events from re-rating", 
			         ebufp);
			     goto cleanup;
		    }
		}
	}

	/***********************************************************
	 * Errors?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_prep_invoice_consolidate_events: error" \
			" marking and consolidating adjustment events ", 
			ebufp);
		goto cleanup;
	}


	/***********************************************************
	* Now run through the item_flist again, and drop all the
	* shadow events, which have been marked for deletion.
	***********************************************************/

	cookie = NULL;
        elemid = 0;

	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp, PIN_FLD_EVENTS,
	 	  	&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_BOOLEAN, 1, ebufp);

                if (vp && (*(int *)vp) == dflag) {

                    PIN_FLIST_ELEM_DROP(item_flistp, PIN_FLD_EVENTS,elemid, ebufp);    
                    cookie = prev_cookie;
		}

                prev_cookie = cookie;
	}

	/***********************************************************
	 * Errors?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_prep_invoice_consolidate_events: error" \
			" dropping shadow events ", 
			ebufp);
	} else {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Consolidate event o/p flist:" \
			" output flistp", item_flistp);
	}

cleanup :

	return;

}

/*******************************************************************
 * fm_inv_pol_prep_invoice_consolidate_rerate_events():
 *
 * This function walks through the adjustment events created by
 * re-rating. If this adjustment event is for an original cycle,
 * purchase or cancel event, then mark it for deletion and return.
 * For Usage events, walk through the PIN_FLD_BAL_IMPACTS array,
 * and recalculate the PIN_FLD_TOTAL.
 * This PIN_FLD_TOTAL should show the new impact, and not the 
 * differential impact.
 *
 *******************************************************************/
static void			
fm_inv_pol_prep_invoice_consolidate_rerate_events(
	pin_flist_t		*item_flistp,
        poid_t                  *a_pdp,
        int32                   *orig_event_type,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*bal_flistp = NULL;
	pin_flist_t	*total_flistp = NULL;
        pin_decimal_t	*amountp = (pin_decimal_t *)NULL;        
        pin_decimal_t	*totalp = (pin_decimal_t *)NULL;
        pin_cookie_t	cookie = NULL;
        pin_cookie_t	cookie1 = NULL;
	poid_t		*re_pdp = NULL;
        poid_t		*e_pdp = NULL;
        int32		*bal_impact_type = NULL;
        int32		dflag = PIN_BOOLEAN_TRUE;
        int32		bal_change = PIN_BOOLEAN_FALSE;
        int32		elemid = 0;
        int32		elemid1 = 0;
        int32		resID = 0;
	double		init_bal = 0;
        char		*etype = NULL;
        void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

 	totalp = pin_decimal("0.0", ebufp);


	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Consolidate re-rate events i/p flist:" \
				" input flistp", item_flistp);


	/***********************************************************
	* Loop through the item_flist, and get the adjustment event
	* for the original CF/P/C or Usage event.
	***********************************************************/

	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp, PIN_FLD_EVENTS,
			&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

	   e_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_EVENT_OBJ, 0, ebufp);
	   etype = (char *)PIN_POID_GET_TYPE(e_pdp);

	   if (!strcmp(etype, "/event/billing/adjustment/event") &&
		!(PIN_POID_COMPARE(e_pdp, a_pdp, 0, ebufp)) ) {

		/***********************************************************
		* If the original event was a CF/P/C event, then this
		* adjustment event needs to be dropped. Hence, mark it.
		***********************************************************/
		if ( *orig_event_type == INV_EVENT_CYCLE_PURCHASE_CANCEL ) {

			PIN_FLIST_FLD_SET(flistp, PIN_FLD_BOOLEAN, &dflag, ebufp); 

		}

		/***********************************************************
		* If the original event was a Usage event, then  check if
		* adjustment event was re-rated. In that case, only the 
		* latest adjustment event needs to be preserved, and all
		* intermediate adjustment events needs to be dropped.
		***********************************************************/

		else if ( *orig_event_type == INV_EVENT_USAGE ) {

			re_pdp = (poid_t *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_RERATE_OBJ, 1, ebufp);

			/***********************************************************
			* If the RERATE_OBJ field is populated, means it has been
			* rerated again, so mark this event for deletion.
			***********************************************************/
			if (re_pdp && !PIN_POID_IS_NULL(re_pdp)) {

				PIN_FLIST_FLD_SET(flistp, PIN_FLD_BOOLEAN, &dflag, ebufp); 

			} else {

				/***********************************************************
				* This adjustment event is the latest and will have the
				* final balance impacts. Loop through the BAL_IMPACTS
				* array, and recalculate the total charges. And then 
				* populate this value in the PIN_FLD_TOTAL field, in the 
				* adjustment event.
				* Here, we add up balance impact values(PIN_FLD_AMOUNT),
				* for the array elements which were created by rerating
				* (distinguished by the field PIN_FLD_IMPACT_TYPE )
				***********************************************************/

				elemid1 = 0;
				cookie1 = NULL;

				while ((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(
						flistp, PIN_FLD_BAL_IMPACTS,
						&elemid1, 1, &cookie1, ebufp))
						!= (pin_flist_t *)NULL) {

					bal_impact_type = (int32 *)PIN_FLIST_FLD_GET(
						bal_flistp, PIN_FLD_IMPACT_TYPE,
						0, ebufp);

					vp = PIN_FLIST_FLD_GET(bal_flistp,
						PIN_FLD_RESOURCE_ID, 1, ebufp);
					if (vp)  {
						resID = *((int32 *)vp);
					}

					if (PIN_ERR_IS_ERR(ebufp)) {
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"fm_inv_pol_prep_invoice_consolidate_rerate_events:" \
							" error getting Impact_type or resource_id ", ebufp);
						goto cleanup;
					}

					if (bal_impact_type && ((*bal_impact_type == PIN_IMPACT_TYPE_RERATED) ||
					    (*bal_impact_type == PIN_IMPACT_TYPE_RERATED_TAX) ||
					    (*bal_impact_type == PIN_IMPACT_TYPE_DISCOUNT_RERATED) ||
					    (*bal_impact_type == PIN_IMPACT_TYPE_INTEGRATE_RERATED) ||
					    (*bal_impact_type == PIN_IMPACT_TYPE_ECE_RERATED) ||
					    (*bal_impact_type == PIN_IMPACT_TYPE_SHARING_RERATED)))
					{
						bal_change = PIN_BOOLEAN_TRUE;
						amountp = (pin_decimal_t *)PIN_FLIST_FLD_GET(
								bal_flistp, PIN_FLD_AMOUNT,
								0, ebufp);
						pbo_decimal_add_assign(totalp, amountp, ebufp);

						if (PIN_ERR_IS_ERR(ebufp)) {
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
								"fm_inv_pol_prep_invoice_consolidate_rerate_events:" \
								" error from pbo_decimal_add_assign() ", ebufp);
							goto cleanup;
						}
					}

				}

				if (bal_change) {
					total_flistp =
						(pin_flist_t *)PIN_FLIST_ELEM_GET(
							flistp, PIN_FLD_TOTAL,
							resID, 1, ebufp);
					PIN_FLIST_FLD_SET(total_flistp,
						PIN_FLD_AMOUNT, totalp, ebufp);

					if (PIN_ERR_IS_ERR(ebufp)) {
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"fm_inv_pol_prep_invoice_consolidate_rerate_events:" \
							" error populating PIN_FLD_AMOUNT ", ebufp);
						goto cleanup;

					}

					/* Reset totalp */
					pbo_decimal_destroy(&totalp);
					totalp = pbo_decimal_from_double(init_bal, ebufp);
				}
			}
		}
		else {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"The original event for this adjustment event" \
				" was not a usage, cycle, purchase, cancel", flistp);
		}
	    }
	    else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Not an adjustment event for the original event ",
				 flistp);
	    }
	}

	/***********************************************************
	 * Errors?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_prep_invoice_consolidate_rerate_events: error" \
			" consolidate re-rated events ", ebufp);
	} else {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Consolidate re-rate events o/p flist:" \
			" output flistp", item_flistp);
	}

cleanup:

	pbo_decimal_destroy(&totalp);
	return;
}

/*******************************************************************
 *fm_inv_pol_prep_invoice_balance_details ():
 *
 * It will read balance details.
 *
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_balance_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t     *svc_flistp = NULL;
        pin_flist_t     *si_flistp = NULL;
        pin_flist_t     *sr_flistp = NULL;
        pin_flist_t     *gbi_flistp = NULL;
        pin_flist_t     *gbo_flistp = NULL;
        pin_flist_t     *flistp = NULL;
        pin_flist_t     *bi_flistp = NULL;
        pin_flist_t     *acct_flistp = NULL;
        pin_flist_t     *dev_flistp = NULL;
	pin_flist_t     *transfer_flistp = NULL;

        poid_t          *a_pdp = NULL;
        poid_t          *ss_pdp = NULL;
        poid_t          *temp_pdp = NULL;
        poid_t          *srvc_acct_pdp = NULL;

        int32           elemid = 0;
        int32           t_elemid = 0;
        int32           elemid_d = 0;
        int32           elemid_s = 0;
        int32           srvc_status = 0;
        int32           service_transferred = PIN_BOOLEAN_FALSE;

        pin_cookie_t    cookie = NULL;
        pin_cookie_t    t_cookie = NULL;
        pin_cookie_t    cookie_d = NULL;
        pin_cookie_t    cookie_s = NULL;

        time_t          start_t = 0;
        time_t          real_start_t = 0;
        time_t          end_t = 0;
        time_t          real_end_t = 0;
        time_t          eff_t = 0;
        void            *vp = NULL;
        int32           resID = 0;
        char            name_buf[255] = "";
        pin_flist_t     *bal_flistp = NULL;
        pin_cookie_t    bi_cookie = NULL;

        int32           bal_flags = 0;
        int32           bal_mode = PIN_BAL_READ_BALGRP_CALC_ONLY;
        int32           p_bal_flags = PIN_BAL_GET_BASED_ON_PERIOD;
        int32           w_bal_flags = PIN_BAL_GET_WITHIN_PERIOD;
        int32           is_corr_bill = 0;
	char            *b_name = NULL;

        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        /***********************************************************
         * Debug what we got.
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_prep_invoice_add_balance_details: input flist",
                out_flistp);

        acct_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_ACCTINFO, 0, 0, ebufp);
        a_pdp = (poid_t *)PIN_FLIST_FLD_GET(acct_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

        elemid = 0;
        cookie = NULL;
        bi_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
                                PIN_FLD_BILLINFO, &elemid, 0, &cookie,  ebufp);

        vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_START_T, 0, ebufp);
        if (vp) {
                start_t = *((time_t *)vp);
        }

        vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_END_T, 0, ebufp);
        if (vp) {
                end_t = *((time_t *)vp);
        }

	b_name = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_NAME, 0,ebufp);
	if (b_name && (strncmp(b_name, PIN_OBJ_NAME_CORRECTIVE_BILL,
		  strlen(PIN_OBJ_NAME_CORRECTIVE_BILL)) == 0)) {
		is_corr_bill = 1;
	}

        /***********************************************************
         * Need to make start_t as end_t and end_t++ to get the next
         * cycle's buckets instead of the already expired buckets
         ***********************************************************/
        real_start_t = end_t++;

        elemid_d = 0;
        cookie_d = NULL;
        while ((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
                        PIN_FLD_DEVICES, &elemid_d, 1,
                        &cookie_d, ebufp)) != (pin_flist_t *)NULL) {

                elemid_s = 0;
                cookie_s = NULL;
                while ((svc_flistp = PIN_FLIST_ELEM_GET_NEXT(
                                dev_flistp, PIN_FLD_SERVICES, &elemid_s,
                                1, &cookie_s, ebufp)) != (pin_flist_t *)NULL) {

                        ss_pdp = (poid_t *)PIN_FLIST_FLD_GET (svc_flistp,
                                       PIN_FLD_SERVICE_OBJ, 1, ebufp);
                        if (!PIN_POID_IS_NULL(ss_pdp)) {
                                 /*************************************************
                                 * Handle Line Transfer: 
                                 * case1: If line transfer A -> B in mid cycle
                                 * case 2: Line transfer A-> B and then B-> A
                                 * case 3: A -> B then B -> A then A -> C etc..
                                 *
                                 * Read the TRANSFER_LIST from the service object.
                                 * to find out if the current service is transferred.
                                 *************************************************/
                                si_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_POID, (void *)ss_pdp, ebufp);
                                PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_STATUS, NULL, ebufp);
                                PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
                                transfer_flistp = PIN_FLIST_ELEM_ADD(si_flistp,
                                        PIN_FLD_TRANSFER_LIST, PIN_ELEMID_ANY, ebufp);
                                PIN_FLIST_FLD_SET(transfer_flistp,
                                        PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
                                PIN_FLIST_FLD_SET(transfer_flistp,
                                        PIN_FLD_EFFECTIVE_T, 0, ebufp);

                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                        "pol_prep_inv: Input search for transfer list", si_flistp);

                                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &sr_flistp, ebufp);

                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                        "pol_prep_inv: Output search for transfer list", sr_flistp);

                                PIN_FLIST_DESTROY_EX(&si_flistp, NULL);

                                 /* Iterate through the TRANSFER_LIST array */
                                t_elemid = 0;
                                t_cookie = NULL;
                                service_transferred = PIN_BOOLEAN_FALSE;
                                while ((transfer_flistp =
                                        PIN_FLIST_ELEM_GET_NEXT(sr_flistp,
                                                PIN_FLD_TRANSFER_LIST, &t_elemid, 1,
                                                        &t_cookie, ebufp)) !=
                                                                (pin_flist_t *)NULL) {
                                        srvc_acct_pdp = PIN_FLIST_FLD_GET(transfer_flistp,
                                                PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

                                        /*****************************************
                                        * Compare the account poids.
                                        * TODO: Multiple transfers in same cycle
                                        * case 2: A -> B and B -> A
                                        * case 3: A -> B then B-> and then A -> C etc..
                                        ******************************************/
                                        if (!PIN_POID_COMPARE(a_pdp, srvc_acct_pdp, 0, ebufp)) {
                                                service_transferred = PIN_BOOLEAN_TRUE;
                                                PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG,
                                                        "service is transferred..." );
                                                        break;
                                        }
                                }
                                /**********************************************
                                * Check if the line transfer happened in this
                                * current cycle.
                                ***********************************************/
                                if (service_transferred == PIN_BOOLEAN_TRUE) {
                                        vp = PIN_FLIST_FLD_GET(transfer_flistp,
                                                PIN_FLD_EFFECTIVE_T, 0, ebufp);
                                        if (vp) {
                                                eff_t = *((time_t *)vp);
                                        }
                                        if ((eff_t < end_t) && (eff_t > start_t)){
                                                real_end_t = eff_t;
                                                real_start_t = start_t;
                                                bal_flags = w_bal_flags;
                                        }

                                } else {
                                        real_end_t = end_t;
                                        bal_flags = p_bal_flags;
                                }
                                PIN_FLIST_DESTROY_EX(&sr_flistp, NULL);
                        }
			/**************************************
                        * Get Balances
                        **************************************/
                        gbi_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(gbi_flistp, PIN_FLD_POID,
                                        (void *)a_pdp, ebufp);
                        PIN_FLIST_FLD_SET(gbi_flistp, PIN_FLD_SERVICE_OBJ,
                                        (void *)ss_pdp, ebufp);
                        PIN_FLIST_FLD_SET(gbi_flistp, PIN_FLD_START_T,
                                        &real_start_t, ebufp);
                        PIN_FLIST_FLD_SET(gbi_flistp, PIN_FLD_END_T,
                                        &real_end_t, ebufp);
                        PIN_FLIST_ELEM_SET(gbi_flistp, NULL, PIN_FLD_BALANCES,
                                        PCM_RECID_ALL, ebufp);


			if ((perf_features_flags & INV_PERF_FEAT_SKIP_BALGRP_LOCK) &&
					(!is_corr_bill)) {
				PIN_FLIST_FLD_SET(gbi_flistp, PIN_FLD_READ_BALGRP_MODE,
						&bal_mode, ebufp);
			}
                        PIN_FLIST_FLD_SET(gbi_flistp, PIN_FLD_FLAGS,
                                        &bal_flags, ebufp);

                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_inv_pol_prep_invoice_add_bg_details: GET_BALANCES input flist",
                                gbi_flistp);

                        PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES, 0, gbi_flistp,
                                &gbo_flistp, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_inv_pol_prep_invoice_add_bg_details: GET_BALANCES output flist",
                                gbo_flistp);

                        bi_cookie = NULL;
                        while((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(gbo_flistp,
                                        PIN_FLD_BALANCES, &resID, 1, &bi_cookie, ebufp )) != NULL) {
                                fm_utils_beid_name(ctxp, resID, name_buf, sizeof(name_buf));
                                PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_NAME, (void *)name_buf, ebufp);
                        }

                        PIN_FLIST_ELEM_PUT (svc_flistp, gbo_flistp,
                                        PIN_FLD_BALANCE_GROUPS, 0, ebufp);

                        PIN_FLIST_DESTROY_EX(&gbi_flistp, NULL);
                }
        }

        /***********************************************************
         * error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                 "fm_inv_pol_prep_invoice_add_bg_details: error ",
                ebufp);

        } else {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_prep_invoice_add_bg_details: output flist",
                        out_flistp);
        }
}
        
/*******************************************************************
 * fm_inv_pol_prep_invoice_fetch_devices_info():
 *
 * This function gets the devices for this account and services
 *
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_fetch_devices_info(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
        int32           combined_inv_flags,	
	int32           is_wholesale_billing,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*arg_flistp = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*res1_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*tmp1_flistp = NULL;
        pin_flist_t	*tmp_svc_flistp = NULL;
	pin_flist_t	*svc_bal_outflistpp = NULL;
	pin_flist_t	*svc1_flistp = NULL;
	pin_flist_t	*svc2_flistp = NULL;
	pin_flist_t	*tk_tmp_flistp = NULL;
	pin_flist_t	*bi_flistp = NULL;
        poid_t		*tmp_a_pdp = NULL;
	poid_t		*a_pdp = NULL;
	poid_t		*s_pdp = NULL;
	poid_t		*bi_pdp = NULL;
        poid_t		*srch_s_pdp = NULL;
        poid_t		*op_s_pdp = NULL;
        poid_t		*dummy_device_pdp = NULL;
        poid_t		*ar_bi_pdp = NULL;
	pin_cookie_t	cookie_r = NULL;
	pin_cookie_t	cookie_s = NULL;
	pin_cookie_t	cookie1_s = NULL;
        pin_cookie_t	old_cookie1_s = NULL;
        pin_cookie_t	cookie2_s = NULL;
	int32		elemid_r = 0;
	int32		elemid_s = 0;
	int32		elemid1_s = 0;
	int32		elemid2_s = 0;
	int32		s_flags = SRCH_DISTINCT;
	int32		parent_flags = 0;
        int32           in_inv_flags = 0;
        int32           inv_flags = 0;
	char		s_template[2*BUFSIZ] = {""};
	void		*vp = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_inv_pol_prep_invoice_fetch_devices_info: input flist", 
		out_flistp);

 	/***********************************************************
	 * Allocate the flist for searching the Devices
	 ***********************************************************/

	s_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Construct the search poid.
	 ***********************************************************/
	tmp_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_ACCTINFO, 0, 0,
					ebufp);
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, 
					0, ebufp);
	s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp), "/search",
				 -1, ebufp);
	if( s_pdp ) {
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID,
				      (void *)s_pdp, ebufp);
	}

	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

	/* Create a dummy device type of /device/num for the search template */
	dummy_device_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp), 
				"/device/num", -1, ebufp);

	/**************************************************************
	 * Add the search template to the search flist
	**************************************************************/

	/* Get the parent flags field to see if it is a parent account */
	bi_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_BILLINFO,
                        0, 0, ebufp);

	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_PARENT_FLAGS, 0, ebufp);

	if (vp) {
		parent_flags = *(int32 *)vp;
	}

	ar_bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(bi_flistp, 
			PIN_FLD_AR_BILLINFO_OBJ, 0, ebufp);

	if (parent_flags && ((combined_inv_flags & PIN_INV_TYPE_PARENT) &&
                     (combined_inv_flags & PIN_INV_TYPE_SUBORDINATE)) &&
			!is_wholesale_billing) {
		/* need to fetch devices of subords too */
		strcpy (s_template, " select X from /device 1, "
			" /billinfo 2 "
			" where ( 1.F1 = V1 or ( 2.F2 = V2 and 1.F3 = 2.F4 )) "
			" and F5.type = V5 ");

	} else {

		strcpy ( s_template, "select X from /device where F1 = V1 "
			" and F2.type = V2 ");
	}

	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, (void *)s_template,
					ebufp);

	/**************************************************************
	 * Add the search arguments
	**************************************************************/

	/* 1st arg */
	arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_SERVICES, 0,
					ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)a_pdp, 
					ebufp);

	if (parent_flags && ((combined_inv_flags & PIN_INV_TYPE_PARENT) &&
                     (combined_inv_flags & PIN_INV_TYPE_SUBORDINATE)) &&
			!is_wholesale_billing) {
		/* 2nd arg */
		arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 2, 
					ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_AR_BILLINFO_OBJ, 
					(void *)ar_bi_pdp, ebufp);

		/* 3rd arg */
		arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 3, 
						ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_SERVICES, 0,
					ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, 
					(void *)NULL, ebufp);

		/* 4th arg */
		arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 4, 
					ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, 
					(void *)NULL, ebufp);

		/* 5th arg */
		arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 5, 
					ebufp);
		PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_POID, 
					(void *)dummy_device_pdp, ebufp);

	} else {
		/* 2nd arg */
		arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 2, 
					ebufp);
		PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_POID, 
					(void *)dummy_device_pdp, ebufp);
	}

	/**************************************************************
	* Add the PIN_FLD_RESULTS
	**************************************************************/
	res_flistp = PIN_FLIST_ELEM_ADD (s_flistp,  PIN_FLD_RESULTS,
			 0, ebufp);
	vp = NULL;
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, vp, ebufp);
	PIN_FLIST_FLD_SET(res_flistp,  PIN_FLD_DEVICE_ID, vp, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_SERVICES, 
					PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_fetch_devices_info: " \
			" search flist", s_flistp);
	/***********************************************************
	 * Execute search
	 ***********************************************************/
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_prep_invoice_fetch_devices_info:error" \
			" executing opcode PCM_OP_SEARCH", ebufp);
	} else {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_fetch_devices_info: " \
			" search result", r_flistp);
		/********************************************************
		 * Filter out the SERVICES array which belong to this
		 * Billinfo only.
		 ********************************************************/

		tmp1_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_BILLINFO, 
		 				0, 0, ebufp);
		bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp1_flistp,
				PIN_FLD_BILLINFO_OBJ, 0, ebufp);
		fm_inv_pol_filter_services(ctxp, bi_pdp, a_pdp, parent_flags,
			combined_inv_flags, &svc_bal_outflistpp, is_wholesale_billing, ebufp);
		while ((res1_flistp = PIN_FLIST_ELEM_GET_NEXT(
			r_flistp, PIN_FLD_RESULTS, &elemid_s,
		1, &cookie_s, ebufp)) != (pin_flist_t *)NULL) {
			cookie1_s = NULL;
			elemid1_s = 0;
			old_cookie1_s = NULL;
			while ((svc1_flistp = PIN_FLIST_ELEM_GET_NEXT(
				res1_flistp, PIN_FLD_SERVICES, &elemid1_s,
				1, &cookie1_s, ebufp)) != (pin_flist_t *)NULL) {
				cookie2_s = NULL;
				elemid2_s = 0;
				srch_s_pdp = (poid_t *) PIN_FLIST_FLD_GET(svc1_flistp,
					PIN_FLD_SERVICE_OBJ, 0, ebufp);
				while ((svc2_flistp = PIN_FLIST_ELEM_GET_NEXT(
				svc_bal_outflistpp, PIN_FLD_RESULTS, &elemid2_s,
				1, &cookie2_s, ebufp)) != (pin_flist_t *)NULL) {
					op_s_pdp = (poid_t *) PIN_FLIST_FLD_GET(
						svc2_flistp, PIN_FLD_SERVICE_OBJ,
						0, ebufp);
					if(!PIN_POID_COMPARE(srch_s_pdp, op_s_pdp,
							0, ebufp)){
						break;
					}
				}
				if(svc2_flistp == NULL) {
					tk_tmp_flistp = PIN_FLIST_ELEM_TAKE(res1_flistp,
					PIN_FLD_SERVICES, elemid1_s, 0, ebufp);
					cookie1_s = old_cookie1_s;
					PIN_FLIST_DESTROY_EX(&tk_tmp_flistp, NULL);
				}
				old_cookie1_s = cookie1_s;
			}
		}
		PIN_FLIST_DESTROY_EX(&svc_bal_outflistpp, NULL);
	}

	/***********************************************************
	 * Take the results/devices and add it to main flist
	 ***********************************************************/
	 while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp,
			PIN_FLD_RESULTS, &elemid_r, 1, &cookie_r, ebufp)) !=
			(pin_flist_t *)NULL) {
		/* move the ACCOUNT_OBJ field to BRAND_OBJ */
		tmp_a_pdp = (poid_t *)PIN_FLIST_FLD_TAKE(res_flistp,
                                        PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

		PIN_FLIST_FLD_PUT(res_flistp, PIN_FLD_BRAND_OBJ,
                                        (void *)tmp_a_pdp, ebufp);
                /* Copy the acount obj to top level */
                tmp_svc_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET(
                                res_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1,
						ebufp);
                if (tmp_svc_flistp) {
                        tmp_a_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_svc_flistp,
                                        PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

                        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACCOUNT_OBJ,
                                        (void *)tmp_a_pdp, ebufp);
			PIN_FLIST_ELEM_SET(out_flistp, res_flistp,
				PIN_FLD_DEVICES, elemid_r, ebufp);
		}
	}

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_fetch_devices_info: error ", 
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_inv_pol_prep_invoice_fetch_devices_info: output flist", 
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_update_device_totals():
 *
 * Function to add up all the resource impacts according to
 * which device consumed them.
 * 
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_update_device_totals(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);


	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_update_device_totals: input flist",
		out_flistp);

	/* 
	 * Call function to total AR and SUB items 
	 * note: we don't have to call this funciton for OTHER items
	 * because the service obj won't be populated for these
	 * special items and hence won't belong to any specific device
	 */
	fm_inv_pol_prep_invoice_update_device_totals_items(ctxp, out_flistp, 
					PIN_FLD_AR_ITEMS, ebufp);

	fm_inv_pol_prep_invoice_update_device_totals_items(ctxp, out_flistp, 
					PIN_FLD_SUB_ITEMS, ebufp);

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_update_device_totals: error ", 
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_inv_pol_prep_invoice_update_device_totals: output flist", 
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_update_device_totals_items():
 *
 * Called by fm_inv_pol_prep_invoice_update_device_totals() 
 * for each of the three item types.
 * 
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_update_device_totals_items(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	int32		fldno,
	pin_errbuf_t	*ebufp)
{
 
	pin_flist_t	*dev_flistp = NULL;
	pin_flist_t	*svc_flistp = NULL;
	pin_flist_t	*item_flistp = NULL;
	poid_t		*d_svc_pdp = NULL;
	poid_t		*i_svc_pdp = NULL;
	pin_cookie_t	d_cookie = NULL;
	pin_cookie_t	s_cookie = NULL;
	pin_cookie_t	i_cookie = NULL;
	int32		d_elem_id = 0;
	int32		s_elem_id = 0;
	int32		i_elem_id = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    "fm_inv_pol_prep_invoice_update_device_totals_items: input flist",
	     out_flistp);

	/* loop through each device */
	while((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, 
				PIN_FLD_DEVICES, &d_elem_id, 1, 
				&d_cookie, ebufp )) != NULL) {

		s_cookie = NULL;
		s_elem_id = 0;
		/* loop through each service for the device */
		while((svc_flistp = PIN_FLIST_ELEM_GET_NEXT(dev_flistp, 
				PIN_FLD_SERVICES, &s_elem_id, 1, 
				&s_cookie, ebufp )) != NULL) {
			d_svc_pdp = (poid_t *)PIN_FLIST_FLD_GET(svc_flistp,
				PIN_FLD_SERVICE_OBJ, 0, ebufp);

			i_cookie = NULL;
			i_elem_id = 0;
			/* loop through each item in the big flist */
			while((item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
				fldno, &i_elem_id, 1, 
				&i_cookie, ebufp )) != NULL) {

				i_svc_pdp = (poid_t *)PIN_FLIST_FLD_GET(
					item_flistp, PIN_FLD_SERVICE_OBJ, 
					0, ebufp);
 				if (!PIN_POID_COMPARE(d_svc_pdp, i_svc_pdp,
					0, ebufp)) {
					/* this item matches this service/device
					 * sum up the events
					 */
					fm_inv_pol_prep_invoice_update_device_totals_events( 
						ctxp, out_flistp, dev_flistp,
						svc_flistp, item_flistp, ebufp);
				}
			}
		}
	}

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
	     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	     "fm_inv_pol_prep_invoice_update_device_totals_items: error ", 
	     ebufp);

	} else {
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
	    "fm_inv_pol_prep_invoice_update_device_totals_items: output flist", 
	    out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_update_device_totals_events();
 *
 * Called from fm_inv_pol_prep_invoice_update_device_totals_items()
 * to loop through the events for each item passed in.
 * 
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_update_device_totals_events(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_flist_t	*dev_flistp,
	pin_flist_t	*svc_flistp,
	pin_flist_t	*item_flistp,
	pin_errbuf_t	*ebufp)
{
 
	pin_flist_t	*evt_flistp = NULL;
	pin_flist_t	*bi_flistp = NULL;
	pin_flist_t	*d_tot_flistp = NULL;
	pin_flist_t	*s_tot_flistp = NULL;
	pin_decimal_t	*zero_dec = (pin_decimal_t *)NULL;
	pin_decimal_t	*bi_amt = (pin_decimal_t *)NULL;
	pin_decimal_t	*d_amt = (pin_decimal_t *)NULL;
	pin_decimal_t	*s_amt = (pin_decimal_t *)NULL;
	pin_decimal_t	*bi_qty = (pin_decimal_t *)NULL;
	pin_decimal_t	*d_qty = (pin_decimal_t *)NULL;
	pin_decimal_t	*s_qty = (pin_decimal_t *)NULL;
	pin_cookie_t	evt_cookie = NULL;
	pin_cookie_t	bi_cookie = NULL;
	int32		evt_elem_id = 0;
	int32		bi_elem_id = 0;
	int32		resID = 0;
	void		*vp = NULL;
	char		name_buf[80] = "";

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    "fm_inv_pol_prep_invoice_update_device_totals_events: input flist",
	     out_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    "fm_inv_pol_prep_invoice_update_device_totals_events: dev flist",
	     dev_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    "fm_inv_pol_prep_invoice_update_device_totals_events: svc flist",
	     svc_flistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    "fm_inv_pol_prep_invoice_update_device_totals_events: item flist",
	     item_flistp);

	zero_dec = pbo_decimal_from_str("0.0", ebufp);

	/* loop through the events */
	while((evt_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp, 
				PIN_FLD_EVENTS, &evt_elem_id, 1, 
				&evt_cookie, ebufp )) != NULL) {

		bi_cookie = NULL;
		bi_elem_id = 0;
		/* loop through the balance impacts */
		while((bi_flistp = PIN_FLIST_ELEM_GET_NEXT(evt_flistp, 
				PIN_FLD_BAL_IMPACTS, &bi_elem_id, 1, 
				&bi_cookie, ebufp )) != NULL) {
			
			/* get the resource id of the balance impact */
			vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_RESOURCE_ID,
				1, ebufp);
			if (vp) {
				resID = *((int32 *)vp);
			} else {
				resID = 0;
			}

			/* get the display name of the resource */
			fm_utils_beid_name(ctxp, resID, name_buf, sizeof(name_buf));
	
			/* see if there is an entry in the device total */
			d_tot_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET(
				dev_flistp, PIN_FLD_TOTAL, resID, 1, ebufp);
			/* if not present then add entry */
			if ( d_tot_flistp == NULL) {
				d_tot_flistp = PIN_FLIST_ELEM_ADD(dev_flistp,
					PIN_FLD_TOTAL, resID, ebufp);
				PIN_FLIST_FLD_SET(d_tot_flistp,PIN_FLD_NAME,
					(void *)name_buf, ebufp)
				PIN_FLIST_FLD_SET(d_tot_flistp,PIN_FLD_AMOUNT,
					(void *)zero_dec, ebufp)
				PIN_FLIST_FLD_SET(d_tot_flistp,PIN_FLD_QUANTITY,
					(void *)zero_dec, ebufp)
			}

			/* see if there is an entry in the service total */
			s_tot_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET(
				svc_flistp, PIN_FLD_TOTAL, resID, 1, ebufp);
			/* if not present then add entry */
			if ( s_tot_flistp == NULL) {
				s_tot_flistp = PIN_FLIST_ELEM_ADD(svc_flistp,
					PIN_FLD_TOTAL, resID, ebufp);
				PIN_FLIST_FLD_SET(s_tot_flistp,PIN_FLD_NAME,
					(void *)name_buf, ebufp)
				PIN_FLIST_FLD_SET(s_tot_flistp,PIN_FLD_AMOUNT,
					(void *)zero_dec, ebufp)
				PIN_FLIST_FLD_SET(s_tot_flistp,PIN_FLD_QUANTITY,
					(void *)zero_dec, ebufp)
			}

			/* get the amount from the balance impact */
			vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_AMOUNT,
				1, ebufp);

			/* add this amount to device and svc totals */
			if (vp) {
				bi_amt = (pin_decimal_t *)vp;
				d_amt = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(
					d_tot_flistp, PIN_FLD_AMOUNT,
					0, ebufp);
				s_amt = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(
					s_tot_flistp, PIN_FLD_AMOUNT,
					0, ebufp);

				pbo_decimal_add_assign(d_amt, bi_amt, ebufp);
				pbo_decimal_add_assign(s_amt, bi_amt, ebufp);

				PIN_FLIST_FLD_PUT(d_tot_flistp,
						PIN_FLD_AMOUNT, 
						(void *)d_amt, ebufp);
				PIN_FLIST_FLD_PUT(s_tot_flistp,
						PIN_FLD_AMOUNT, 
						(void *)s_amt, ebufp);
			}

			/* get the quantity from the balance impact */
			vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_QUANTITY,
				1, ebufp);

			/* add this quantity to device and svc totals */
			if (vp) {
				bi_qty = (pin_decimal_t *)vp;
				d_qty = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(
					d_tot_flistp, PIN_FLD_QUANTITY,
					0, ebufp);
				s_qty = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(
					s_tot_flistp, PIN_FLD_QUANTITY,
					0, ebufp);

				pbo_decimal_add_assign(d_qty, bi_qty, ebufp);
				pbo_decimal_add_assign(s_qty, bi_qty, ebufp);
				PIN_FLIST_FLD_PUT(d_tot_flistp,
						PIN_FLD_QUANTITY, 
						(void *)d_qty, ebufp);
				PIN_FLIST_FLD_PUT(s_tot_flistp,
						PIN_FLD_QUANTITY, 
						(void *)s_qty, ebufp);
			}
		}
	}

	pbo_decimal_destroy(&zero_dec);

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
	     PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	     "fm_inv_pol_prep_invoice_update_device_totals_events: error ", 
	     ebufp);

	} else {
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
	    "fm_inv_pol_prep_invoice_update_device_totals_events: output flist", 
	    out_flistp);
	}


}

/*******************************************************************
 * fm_inv_pol_prep_invoice_get_subordinate_accounts():
 *
 * Get a list of subordinate billinfos and the accounts
 * for those billinfos.
 * 
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_get_subordinate_accounts(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t     *acct_info_flistp = NULL;
	pin_flist_t     *bi_flistp = NULL;
	poid_t		*grp_pdp = NULL;
        int32           parent_flags = 0;
        void            *vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_get_subordinate_accounts: input flist",
		out_flistp);

	/* Get Group POID from input flist */
        acct_info_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_ACCTINFO,
				0, 0, ebufp);
	grp_pdp = (poid_t *)PIN_FLIST_FLD_GET(acct_info_flistp,PIN_FLD_GROUP_OBJ,
				 0, ebufp);

	 /* Get the parent flags field to see if it is a parent account */
        bi_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_BILLINFO,
                        0, 0, ebufp);

        vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_PARENT_FLAGS, 0, ebufp);

        if (vp) {
                parent_flags = *(int32 *)vp;
        }

	if ( !parent_flags ) {
		/* not a parent of a group */
		goto cleanup;
	}

	/* put the group object in the main flist so that the
	 * get_devices function will get all devices belonging to the subords
	 * the get_devices has been changed to use parent_flags, but leaving
	 * this field in the flist since existing custom templates could
	 * be using this.
	 */
	PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_GROUP_OBJ, 
					(void *)grp_pdp, ebufp);

	/* Get subordinate accounts from the SUB_ITEMS array */
	fm_inv_pol_prep_invoice_get_accounts_from_sub_items(ctxp, out_flistp, 
					ebufp);

	/* Iterate through items and add totals of subords */
	fm_inv_pol_prep_invoice_aggregate_subordinate_accounts_totals(ctxp, 
					out_flistp, ebufp);

cleanup:

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_get_subordinate_accounts: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_get_subordinate_accounts: output flist",
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_get_accounts_from_sub_items():
 *
 * Get a list of subordinate accounts
 * from the information in the SUB_ITEMS array in the input flist.
 * 
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_get_accounts_from_sub_items(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*sub_flistp = NULL;
	pin_flist_t	*sub_item_flistp = NULL;
	pin_flist_t	*sub_acct_flistp = NULL;
	pin_flist_t	*read_acct_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*tmp_sub_accts_flistp = NULL;
	poid_t		*a_pdp = NULL;
	pin_decimal_t	*zero_dec = (pin_decimal_t *)NULL;
	pin_cookie_t	cookie_i = NULL;
	int32		elemid_i = 0;
	pin_cookie_t	cookie_s = NULL;
	int32		elemid_s = 0;
	int32		elemid_sub = 0;
	int		matched = PIN_BOOLEAN_FALSE;
	void		*vp = NULL;
	char            *sub_account_number = "";

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	zero_dec = pbo_decimal_from_str("0.0", ebufp);
	tmp_sub_accts_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_get_accounts_from_sub_items: input flist",
		out_flistp);

	/***********************************************************
	 *	Iterate through PIN_FLD_SUBORD_ITEMS in the big flist 
	 *	 	for each item
	 *		iterate through SUBORD_ACCOUNTS array
	 *			if account obj in item = acct obj in array
	 *				match found
	 *		if (match not found)
	 *			add SUBORD_ACCOUNTS and init total
	 ***********************************************************/

	while ((sub_item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
			PIN_FLD_SUB_ITEMS, &elemid_i, 1, &cookie_i, ebufp))
			!= (pin_flist_t *)NULL) {
		a_pdp = (poid_t *) PIN_FLIST_FLD_GET(sub_item_flistp,
				PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	
		matched = PIN_BOOLEAN_FALSE;
		elemid_s = 0;
		cookie_s = NULL;
		while ((sub_acct_flistp = PIN_FLIST_ELEM_GET_NEXT(tmp_sub_accts_flistp,
				PIN_FLD_SUBORDS, &elemid_s, 1,
				&cookie_s, ebufp)) != (pin_flist_t *)NULL) {

			vp = PIN_FLIST_FLD_GET(sub_acct_flistp,
					PIN_FLD_POID, 0, ebufp);

			if ( PIN_POID_COMPARE (a_pdp, (poid_t *)vp, 0,
					ebufp) == 0 ) {
				
				matched = PIN_BOOLEAN_TRUE;
				break;
			}
		}
		/* If no matching SUBORD element then add this and initialize.
		 */
		if (matched == PIN_BOOLEAN_FALSE) {
			sub_flistp =  PIN_FLIST_ELEM_ADD(tmp_sub_accts_flistp,
				PIN_FLD_SUBORDS, elemid_sub, ebufp);

			elemid_sub++;
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_POID, 
					a_pdp, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ITEM_TOTAL,
					(void *)zero_dec, ebufp); 

			/* Get the account number of this subord */
			read_acct_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_acct_flistp, PIN_FLD_POID,
                                        (void *)a_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_acct_flistp, PIN_FLD_ACCOUNT_NO,
                                        (void *)NULL, ebufp);

			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_acct_flistp,
					&r_flistp, ebufp);
			sub_account_number = (char *)PIN_FLIST_FLD_GET(r_flistp,
					PIN_FLD_ACCOUNT_NO, 0, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ACCOUNT_NO,
					(void *)sub_account_number, ebufp);

			PIN_FLIST_DESTROY_EX(&read_acct_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

		}
	}

	PIN_FLIST_CONCAT(out_flistp, tmp_sub_accts_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&tmp_sub_accts_flistp, NULL);

	pbo_decimal_destroy(&zero_dec);

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_get_accounts_from_sub_items: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_get_accounts_from_sub_items: output flist",
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_aggregate_subordinate_accounts_totals():
 * 
 * Function to aggregate the totals for the subordinate accounts.
 * 
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_aggregate_subordinate_accounts_totals(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{
	
	pin_flist_t	*sub_item_flistp = NULL;
	pin_flist_t	*sub_acct_flistp = NULL;
	poid_t		*a_pdp = NULL;
	pin_decimal_t	*acct_total = (pin_decimal_t *)NULL;
	pin_decimal_t	*item_total = (pin_decimal_t *)NULL;
	pin_cookie_t	cookie_i = NULL;
	pin_cookie_t	cookie_t = NULL;
	int32		elemid_i = 0;
	int32		elemid_t = 0;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_aggregate_subordinate_accounts_totals: input flist",
		out_flistp);

	/***********************************************************
	 *	Iterate through PIN_FLD_SUBORD_ITEMS in the big flist 
	 *	 	for each item
	 *		iterate through SUBORD_ACCOUNTS array
	 *			if account obj in item = acct obj in array
	 *				aggregate totals
	 ***********************************************************/

	while ((sub_item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
			PIN_FLD_SUB_ITEMS, &elemid_i, 1, &cookie_i, ebufp))
			!= (pin_flist_t *)NULL) {
		a_pdp = (poid_t *) PIN_FLIST_FLD_GET(sub_item_flistp,
				PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		item_total = (pin_decimal_t *)PIN_FLIST_FLD_GET(sub_item_flistp,
				PIN_FLD_ITEM_TOTAL, 0, ebufp);

		elemid_t = 0;
		cookie_t = NULL;
		while ((sub_acct_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
				PIN_FLD_SUBORDS, &elemid_t, 1,
				&cookie_t, ebufp)) != (pin_flist_t *)NULL) {
			vp = PIN_FLIST_FLD_GET(sub_acct_flistp,
					PIN_FLD_POID, 0, ebufp);

			if ( PIN_POID_COMPARE (a_pdp, (poid_t *)vp, 0,
					ebufp) == 0 ) {
				/* match - add the item total to subord total */
				acct_total = (pin_decimal_t *)PIN_FLIST_FLD_TAKE
						(sub_acct_flistp,
						PIN_FLD_ITEM_TOTAL, 0, ebufp);
			 	pbo_decimal_add_assign(acct_total, 
						item_total, ebufp);
				PIN_FLIST_FLD_PUT(sub_acct_flistp,
						PIN_FLD_ITEM_TOTAL, 
						(void *)acct_total, ebufp);
				break;
			}
		}
	}


	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_aggregate_subordinate_accounts_totals: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_aggregate_subordinate_accounts_totals: output flist",
			out_flistp);
	}


}

/*******************************************************************
 * fm_inv_pol_prep_invoice_add_parent():
 *
 * Add the parent account if this is a subordinate.
 * If this has no parent then hav ea null poid in PIN_FLD_PARENT.
 * 
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_add_parent(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	int32		is_wholesale_billing,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*read_flistp = NULL;
	pin_flist_t	*parent_flistp = NULL;
	poid_t		*bi_pdp = NULL;
	poid_t		*ar_bi_pdp = NULL;
	poid_t		*parent_pdp = NULL;
	void		*vp = NULL;

       if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_add_parent: input flist",
		out_flistp);

	tmp_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_BILLINFO, 
					0, 0, ebufp);
	bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_flistp, 
					PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	ar_bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(tmp_flistp, 
					PIN_FLD_AR_BILLINFO_OBJ, 0, ebufp);
			
	if (PIN_POID_COMPARE (bi_pdp, ar_bi_pdp, 0, ebufp) == 0 ) {
		/* No parent for this account */
		parent_pdp =  PIN_POID_FROM_STR("0.0.0.0  0 0", NULL, ebufp);
	} else {
		/* This is a subord - get the parent account */
		read_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, (void *)ar_bi_pdp, 
					ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &parent_flistp, 
					ebufp);
		parent_pdp = (poid_t *)PIN_FLIST_FLD_TAKE(parent_flistp,
					PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&parent_flistp, NULL);
	}
	
	PIN_FLIST_FLD_PUT(out_flistp, PIN_FLD_PARENT, (void *)parent_pdp, 
					ebufp);

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_add_parent: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_add_parent: output flist",
			out_flistp);
	}
}

/* Uncomment the #define XML_WRITE_OUT_FILE 1 near the top 
 * of this file to use this function.
 * Default will be not to dump the XML file.
 */
#ifdef XML_WRITE_OUT_FILE
/*******************************************************************
 * fm_inv_pol_format_invoice_xml_write_out_file():
 *
 * This function converts the bif flist to xml and 
 * dumps into a XML file.
 * 
 *******************************************************************/
static void
fm_inv_pol_format_invoice_xml_write_out_file(
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	fm_inv_pol_str_buf_t	dbuf;
	char                    str[BUFSIZ];
	int			fd = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*******************************************************
	 * Initialize dbuf - will hold the xml invoice
	 *******************************************************/

	dbuf.strp = NULL;
	dbuf.strsize = 0;

	/***********************************************************
	 * Convert the flist to XML
	 ***********************************************************/
	PIN_FLIST_TO_XML( out_flistp, PIN_XML_BY_SHORT_NAME, 0,
		&dbuf.strp, &dbuf.strsize, "invoice", ebufp );

	dbuf.strsize -= 1;
	sprintf(str, "Buffer size: %d\n", dbuf.strsize);
	PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG, str );

	fd = open ("MakeInvoiceXML.xml", O_RDWR|O_CREAT, 0664);

	write (fd, dbuf.strp, dbuf.strsize);

	close (fd);
}
#endif

/*******************************************************************
 * fm_inv_pol_prep_invoice_reorder_device_centric():
 *
 * Function to re-order the items and events so that they
 * are grouped under the devices they are associated with.
 *
 * Original Structure of the big flist
 *      0 PIN_FLD_AR_ITEMS	      ARRAY []
 *      1   PIN_FLD_EVENTS	      ARRAY []
 *      0 PIN_FLD_SUB_ITEMS	     ARRAY []
 *      1   PIN_FLD_EVENTS	      ARRAY []
 *      0 PIN_FLD_OTHER_ITEMS	   ARRAY []
 *      1   PIN_FLD_EVENTS	      ARRAY []
 *
 * After reordering
 * 	0 PIN_FLD_DEVICES		ARRAY []
 *      1     PIN_FLD_SERVICES	  ARRAY []
 *      2	 PIN_FLD_AR_ITEMS      ARRAY []
 *      3   	    PIN_FLD_EVENTS      ARRAY []
 *      2	 PIN_FLD_SUB_ITEMS     ARRAY []
 *      3   	    PIN_FLD_EVENTS      ARRAY []
 *      2	 PIN_FLD_OTHER_ITEMS   ARRAY []
 *      3   	    PIN_FLD_EVENTS      ARRAY []
 * 
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_reorder_device_centric(
	pcm_context_t	   *ctxp,
	pin_flist_t	     *out_flistp,
	pin_errbuf_t	    *ebufp)
{
	pin_flist_t     *item_flistp = NULL;
	pin_flist_t     *dev_flistp = NULL;
	pin_flist_t     *svc_flistp = NULL;
	poid_t	  	*i_svc_pdp = NULL;
	poid_t	  	*d_svc_pdp = NULL;
	pin_cookie_t    i_cookie = NULL;
	pin_cookie_t    d_cookie = NULL;
	pin_cookie_t    s_cookie = NULL;
	int32	   	i_elem_id = 0;
	int32	   	d_elem_id = 0;
	int32	   	s_elem_id = 0;
	int		matched = PIN_BOOLEAN_FALSE;
	char            *dummy_device_id = "No Device";
	poid_t          *dummy_a_pdp = NULL;
	poid_t          *dummy_s_pdp = NULL;
	pin_flist_t     *dummy_device_flistp = NULL;
	pin_flist_t     *dummy_svc_flistp = NULL;


       if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_reorder_device_centric: input flist",
		out_flistp);

	 	
	i_cookie = NULL;
	i_elem_id = 0;
	while ((item_flistp = PIN_FLIST_ELEM_TAKE_NEXT(out_flistp,
			PIN_FLD_AR_ITEMS, &i_elem_id, 1, &i_cookie, ebufp))
			!= (pin_flist_t *)NULL) {

		matched = PIN_BOOLEAN_FALSE;					
		i_svc_pdp = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
				PIN_FLD_SERVICE_OBJ, 0, ebufp);

		d_cookie = NULL;
		d_elem_id = 0;
		while((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
				PIN_FLD_DEVICES, &d_elem_id, 1,
				&d_cookie, ebufp )) != NULL) {

			s_cookie = NULL;
			s_elem_id = 0;
			while((svc_flistp = PIN_FLIST_ELEM_GET_NEXT(dev_flistp,
				PIN_FLD_SERVICES, &s_elem_id, 1,
				&s_cookie, ebufp )) != NULL) {
				d_svc_pdp = (poid_t *)PIN_FLIST_FLD_GET(
					svc_flistp, PIN_FLD_SERVICE_OBJ, 
					0, ebufp);

				if (!PIN_POID_IS_NULL(d_svc_pdp) && !PIN_POID_COMPARE(d_svc_pdp, 
					i_svc_pdp, 0, ebufp)) {

					PIN_FLIST_ELEM_PUT(svc_flistp, 
						item_flistp, PIN_FLD_AR_ITEMS,
						i_elem_id, ebufp);
					matched = PIN_BOOLEAN_TRUE;
					break;
				}
			}
			
			if (matched == PIN_BOOLEAN_TRUE){
				break;
			}
		}

		/* The item does not belong to any device - put it 
	 	 * in a dummy device flist
		 */
		if (matched == PIN_BOOLEAN_FALSE){
			dummy_device_flistp = PIN_FLIST_ELEM_ADD(out_flistp,
					PIN_FLD_DEVICES,
					d_elem_id+1, ebufp);
			PIN_FLIST_FLD_SET(dummy_device_flistp,
					PIN_FLD_DEVICE_ID,
					dummy_device_id, ebufp);
			dummy_a_pdp = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
					PIN_FLD_ACCOUNT_OBJ,  0, ebufp);
			PIN_FLIST_FLD_SET(dummy_device_flistp,
					PIN_FLD_ACCOUNT_OBJ,
					dummy_a_pdp, ebufp);
			dummy_svc_flistp = PIN_FLIST_ELEM_ADD(
					dummy_device_flistp,
					PIN_FLD_SERVICES,
					0, ebufp);
			dummy_s_pdp = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
					PIN_FLD_SERVICE_OBJ,  0, ebufp);
			PIN_FLIST_FLD_SET(dummy_svc_flistp,
					PIN_FLD_SERVICE_OBJ,
					dummy_s_pdp, ebufp);
			PIN_FLIST_FLD_SET(dummy_svc_flistp,
					PIN_FLD_ACCOUNT_OBJ,
					dummy_a_pdp, ebufp);
			PIN_FLIST_ELEM_PUT(dummy_svc_flistp,
				item_flistp, PIN_FLD_AR_ITEMS,
				i_elem_id, ebufp);

		}
	}

	i_cookie = NULL;
	i_elem_id = 0;
	while ((item_flistp = PIN_FLIST_ELEM_TAKE_NEXT(out_flistp,
			PIN_FLD_SUB_ITEMS, &i_elem_id, 1, &i_cookie, ebufp))
			!= (pin_flist_t *)NULL) {

		matched = PIN_BOOLEAN_FALSE;					
		i_svc_pdp = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
				PIN_FLD_SERVICE_OBJ, 0, ebufp);

		d_cookie = NULL;
		d_elem_id = 0;
		while((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
				PIN_FLD_DEVICES, &d_elem_id, 1,
				&d_cookie, ebufp )) != NULL) {

			s_cookie = NULL;
			s_elem_id = 0;
			while((svc_flistp = PIN_FLIST_ELEM_GET_NEXT(dev_flistp,
				PIN_FLD_SERVICES, &s_elem_id, 1,
				&s_cookie, ebufp )) != NULL) {
				d_svc_pdp = (poid_t *)PIN_FLIST_FLD_GET(
					svc_flistp, PIN_FLD_SERVICE_OBJ, 
					0, ebufp);

				if (!PIN_POID_IS_NULL(d_svc_pdp) && !PIN_POID_COMPARE(d_svc_pdp, i_svc_pdp,
					0, ebufp)) {

					PIN_FLIST_ELEM_PUT(svc_flistp, 
						item_flistp, PIN_FLD_SUB_ITEMS,
						i_elem_id, ebufp);
					matched = PIN_BOOLEAN_TRUE;
					break;
				}
			}
			
			if (matched == PIN_BOOLEAN_TRUE){
				break;
			}
		}

		/* The item does not belong to any device - put it 
	 	 * in a dummy device flist
		 */
		if (matched == PIN_BOOLEAN_FALSE){
			dummy_device_flistp = PIN_FLIST_ELEM_ADD(out_flistp,
					PIN_FLD_DEVICES,
					d_elem_id+1, ebufp);
			PIN_FLIST_FLD_SET(dummy_device_flistp,
					PIN_FLD_DEVICE_ID,
					dummy_device_id, ebufp);
			dummy_a_pdp = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
					PIN_FLD_ACCOUNT_OBJ,  0, ebufp);
			PIN_FLIST_FLD_SET(dummy_device_flistp,
					PIN_FLD_ACCOUNT_OBJ,
					dummy_a_pdp, ebufp);
                        dummy_svc_flistp = PIN_FLIST_ELEM_ADD(
                                        dummy_device_flistp,
                                        PIN_FLD_SERVICES,
                                        0, ebufp);
                        dummy_s_pdp = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
                                        PIN_FLD_SERVICE_OBJ,  0, ebufp);
                        PIN_FLIST_FLD_SET(dummy_svc_flistp,
                                        PIN_FLD_SERVICE_OBJ,
                                        dummy_s_pdp, ebufp);
                        PIN_FLIST_FLD_SET(dummy_svc_flistp,
                                        PIN_FLD_ACCOUNT_OBJ,
                                        dummy_a_pdp, ebufp);
			PIN_FLIST_ELEM_PUT(dummy_svc_flistp,
				item_flistp, PIN_FLD_SUB_ITEMS,
				i_elem_id, ebufp);

		}
	}

	i_cookie = NULL;
	i_elem_id = 0;
	while ((item_flistp = PIN_FLIST_ELEM_TAKE_NEXT(out_flistp,
			PIN_FLD_OTHER_ITEMS, &i_elem_id, 1, &i_cookie, ebufp))
			!= (pin_flist_t *)NULL) {

		matched = PIN_BOOLEAN_FALSE;					
		i_svc_pdp = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
				PIN_FLD_SERVICE_OBJ, 0, ebufp);

		d_cookie = NULL;
		d_elem_id = 0;
		while((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
				PIN_FLD_DEVICES, &d_elem_id, 1,
				&d_cookie, ebufp )) != NULL) {

			s_cookie = NULL;
			s_elem_id = 0;
			while((svc_flistp = PIN_FLIST_ELEM_GET_NEXT(dev_flistp,
				PIN_FLD_SERVICES, &s_elem_id, 1,
				&s_cookie, ebufp )) != NULL) {
				d_svc_pdp = (poid_t *)PIN_FLIST_FLD_GET(
					svc_flistp, PIN_FLD_SERVICE_OBJ, 
					0, ebufp);

				if (!PIN_POID_IS_NULL(d_svc_pdp) && (!PIN_POID_COMPARE(d_svc_pdp, i_svc_pdp,
					0, ebufp))) {

					PIN_FLIST_ELEM_PUT(svc_flistp, 
					       item_flistp, PIN_FLD_OTHER_ITEMS,
						i_elem_id, ebufp);
					matched = PIN_BOOLEAN_TRUE;
					break;
				}
			}
			
			if (matched == PIN_BOOLEAN_TRUE){
				break;
			}
		}

		/* The item does not belong to any device - put it 
	 	 * in a dummy device flist
		 */
		if (matched == PIN_BOOLEAN_FALSE){
			dummy_device_flistp = PIN_FLIST_ELEM_ADD(out_flistp,
					PIN_FLD_DEVICES,
					d_elem_id+1, ebufp);
			PIN_FLIST_FLD_SET(dummy_device_flistp,
					PIN_FLD_DEVICE_ID,
					dummy_device_id, ebufp);
			dummy_a_pdp = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
					PIN_FLD_ACCOUNT_OBJ,  0, ebufp);
			PIN_FLIST_FLD_SET(dummy_device_flistp,
					PIN_FLD_ACCOUNT_OBJ,
					dummy_a_pdp, ebufp);
                        dummy_svc_flistp = PIN_FLIST_ELEM_ADD(
                                        dummy_device_flistp,
                                        PIN_FLD_SERVICES,
                                        0, ebufp);
                        dummy_s_pdp = (poid_t *)PIN_FLIST_FLD_GET(item_flistp,
                                        PIN_FLD_SERVICE_OBJ,  0, ebufp);
                        PIN_FLIST_FLD_SET(dummy_svc_flistp,
                                        PIN_FLD_SERVICE_OBJ,
                                        dummy_s_pdp, ebufp);
                        PIN_FLIST_FLD_SET(dummy_svc_flistp,
                                        PIN_FLD_ACCOUNT_OBJ,
                                        dummy_a_pdp, ebufp);
			PIN_FLIST_ELEM_PUT(dummy_svc_flistp,
				item_flistp, PIN_FLD_OTHER_ITEMS,
				i_elem_id, ebufp);

		}
	}

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_reorder_device_centric: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_reorder_device_centric: output flist",
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_add_account_num_to_devices():
 *
 * Function to add the account number information to the
 * devices array. 
 *
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_add_account_num_to_devices(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t     *subords_flistp = NULL;
	pin_flist_t     *acct_info_flistp = NULL;
	pin_flist_t     *dev_flistp = NULL;
	pin_flist_t     *cinfo_flistp = NULL;
	pin_flist_t     *corr_item_flistp = NULL;
	pin_flist_t     *other_item_flistp = NULL;
	pin_flist_t     *devices_flistp = NULL;
        pin_flist_t     *services_flistp = NULL;
	poid_t	  	*account_pdp = NULL;
	poid_t	  	*acct_pdp = NULL;
	char 		*account_number = "";
	char 		*acct_no = "";
	char 		*sub_account_number = "";
	poid_t	  	*sub_account_pdp = NULL;
	poid_t	  	*d_a_pdp = NULL;
	pin_cookie_t    sub_cookie = NULL;
	pin_cookie_t    dev_cookie = NULL;
	pin_cookie_t    corr_cookie = NULL;
	pin_cookie_t    oitem_cookie = NULL;
	pin_cookie_t    s_cookie = NULL;
        pin_cookie_t    d_cookie = NULL;
	int32	   	sub_elem_id = 0;
	int32	   	dev_elem_id = 0;
	int32		corr_elem_id = 0;
	int32		oitem_elem_id = 0;
	int32           s_elem_id = 0;
        int32           d_elem_id = 0;
        void            *vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_add_account_num_to_devices: input flist",
		out_flistp);

	/* Get the current account poid and account number */
	acct_info_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_ACCTINFO, 
					0, 0, ebufp);
	account_pdp = (poid_t *)PIN_FLIST_FLD_GET(acct_info_flistp,
					PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	account_number = (char *)PIN_FLIST_FLD_GET(acct_info_flistp,
					PIN_FLD_ACCOUNT_NO, 0, ebufp);
	 	
	/* Iterate through the devices array and add the 
	 * account number.
	 * The account can be either the current account being invoiced
	 * or one of its subords. Check the current account first and then 
	 * the subords array.
	 */
        dev_elem_id = 0;
        dev_cookie = NULL;
        while ((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
                        PIN_FLD_DEVICES, &dev_elem_id, 1,
                        &dev_cookie, ebufp)) != (pin_flist_t *)NULL) {
			
		d_a_pdp = (poid_t *)PIN_FLIST_FLD_GET(dev_flistp,
					PIN_FLD_ACCOUNT_OBJ,  0, ebufp);
		if (!PIN_POID_COMPARE(d_a_pdp, account_pdp, 0, ebufp)) {
			/* matched current account */
			PIN_FLIST_FLD_SET(dev_flistp, PIN_FLD_ACCOUNT_NO, 
					(void *)account_number, ebufp);
			continue;
		}

		/* No match - check subordinate accounts */
		sub_elem_id = 0;
		sub_cookie = NULL;
		while ((subords_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
			PIN_FLD_SUBORDS, &sub_elem_id, 1, &sub_cookie, ebufp))
			!= (pin_flist_t *)NULL) {
			sub_account_pdp = (poid_t *)PIN_FLIST_FLD_GET(
				subords_flistp, PIN_FLD_POID,  0, ebufp);
			sub_account_number = (char *)PIN_FLIST_FLD_GET(
				subords_flistp, PIN_FLD_ACCOUNT_NO,  0, ebufp);
			if (!PIN_POID_COMPARE(d_a_pdp, sub_account_pdp, 
						0, ebufp)) {
				/* Matched subord account */
				PIN_FLIST_FLD_SET(dev_flistp, 
					PIN_FLD_ACCOUNT_NO, 
					(void *)sub_account_number, ebufp);
				break;
			}
		}
	}
	
	/***********************************************************
	* Update the ACCOUNT_NO in CORRECTION_INFO.OTHER_ITEMS also
	* as ACCOUNT_NO will be picked up from the items in the
	* corrective invoice scenarios
	************************************************************/
	/* Corrective Invoice: Check CORRECTION_INFO.Other Items */
	cinfo_flistp = PIN_FLIST_ELEM_GET(out_flistp,
                                PIN_FLD_CORRECTION_INFO, 0, 1, ebufp);
	if (cinfo_flistp) {
		corr_elem_id = 0;
		corr_cookie = NULL;
		 while ((corr_item_flistp = PIN_FLIST_ELEM_GET_NEXT(cinfo_flistp,
				PIN_FLD_OTHER_ITEMS, &corr_elem_id, 1,
                                        &corr_cookie, ebufp)) != (pin_flist_t *)NULL) {
			acct_pdp = PIN_FLIST_FLD_GET(corr_item_flistp,
					PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			dev_elem_id = 0;
			dev_cookie = NULL;
			while ((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
				PIN_FLD_DEVICES, &dev_elem_id, 1,
				&dev_cookie, ebufp)) != (pin_flist_t *)NULL) {

				d_a_pdp = (poid_t *)PIN_FLIST_FLD_GET(dev_flistp,
                                        PIN_FLD_ACCOUNT_OBJ,  0, ebufp);
				if (!PIN_POID_COMPARE(d_a_pdp, acct_pdp, 0, ebufp)) {
					/***** Matched the Account *****/
					acct_no = (char *)PIN_FLIST_FLD_GET(dev_flistp,
                                        PIN_FLD_ACCOUNT_NO, 0, ebufp);
					PIN_FLIST_FLD_SET(corr_item_flistp, 
						PIN_FLD_ACCOUNT_NO, 
						(void *)acct_no, ebufp);
					break;
				}
			}	
		}
	}

	/***********************************************************
        * Update the ACCOUNT_NO in DEVICES.SERVICES.OTHER_ITEMS also
        * as ACCOUNT_NO will be picked up from the other_items in the
        * corrective invoice scenarios
        ************************************************************/
        d_elem_id = 0;
        d_cookie = NULL;
        while ((devices_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
                        PIN_FLD_DEVICES, &d_elem_id, 1,
                                &d_cookie, ebufp)) != (pin_flist_t *)NULL) {
                s_elem_id = 0;
                s_cookie = NULL;
                while ((services_flistp = PIN_FLIST_ELEM_GET_NEXT(devices_flistp,
                                PIN_FLD_SERVICES, &s_elem_id, 1,
                                        &s_cookie, ebufp)) != (pin_flist_t *)NULL) {
			oitem_elem_id = 0;
			oitem_cookie = NULL;
                         while ((other_item_flistp = PIN_FLIST_ELEM_GET_NEXT(services_flistp,
                                        PIN_FLD_OTHER_ITEMS, &oitem_elem_id, 1,
                                                &oitem_cookie, ebufp)) != (pin_flist_t *)NULL) {
                                acct_pdp = PIN_FLIST_FLD_GET(other_item_flistp,
                                                PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
                                dev_elem_id = 0;
                                dev_cookie = NULL;
                                while ((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
                                        PIN_FLD_DEVICES, &dev_elem_id, 1,
                                        &dev_cookie, ebufp)) != (pin_flist_t *)NULL) {

                                        d_a_pdp = (poid_t *)PIN_FLIST_FLD_GET(dev_flistp,
                                                PIN_FLD_ACCOUNT_OBJ,  0, ebufp);
                                        if (!PIN_POID_COMPARE(d_a_pdp, acct_pdp, 0, ebufp)) {
                                                /***** Matched the Account *****/
                                                acct_no = (char *)PIN_FLIST_FLD_GET(dev_flistp,
                                                        PIN_FLD_ACCOUNT_NO, 0, ebufp);
                                                PIN_FLIST_FLD_SET(other_item_flistp,
                                                        PIN_FLD_ACCOUNT_NO,
                                                                (void *)acct_no, ebufp);
                                                break;
                                        }
                                }
                        }
                }
        }

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_add_account_num_to_devices: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_add_account_num_to_devices: output flist",
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_filter_services():
 *
 * Function to find out the services belong to this billinfo
 * and subords billinfos if present
 *
 *******************************************************************/
static void
fm_inv_pol_filter_services(
        pcm_context_t	*ctxp,
        poid_t		*bi_pdp,
        poid_t		*a_pdp,
        int32		parent_flags,
        int32           combined_inv_flags,
        pin_flist_t	**svc_bal_outflistpp,
	int32		is_wholesale_billing,
        pin_errbuf_t	*ebufp)
{

	pin_flist_t	*svc_bal_inflistp =NULL;
	pin_flist_t	*svc1_bal_inflistp =NULL;
	pin_flist_t	*svc1_bal_outflistp =NULL;
	pin_flist_t	*s_flistp =NULL;
	pin_flist_t	*r_flistp =NULL;
	pin_flist_t	*res_flistp =NULL;
	pin_flist_t	*res1_flistp =NULL;
	pin_flist_t	*arg_flistp =NULL;
	poid_t		*bi1_pdp = NULL;
	poid_t		*a1_pdp = NULL;
	poid_t		*s_pdp = NULL;
	pin_cookie_t	cookie_s = NULL;
	int32		s_flags = SRCH_DISTINCT;
	int32		elemid_s = 0;
	char		s_template[2*BUFSIZ] = {""};
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
	
	svc_bal_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(svc_bal_inflistp, PIN_FLD_POID, (void *)bi_pdp,
								ebufp);
	PIN_FLIST_FLD_SET(svc_bal_inflistp, PIN_FLD_ACCOUNT_OBJ, (void *)a_pdp,
							ebufp);
	PCM_OP(ctxp, PCM_OP_BAL_GET_BAL_GRP_AND_SVC, 0,
			svc_bal_inflistp, svc_bal_outflistpp, ebufp);
	PIN_FLIST_DESTROY_EX(&svc_bal_inflistp, NULL);

        if (parent_flags && ((combined_inv_flags & PIN_INV_TYPE_PARENT) &&
                     (combined_inv_flags & PIN_INV_TYPE_SUBORDINATE)) &&
			!is_wholesale_billing) {

		/*****************************************************
		* Search for the subord billinfos and get the
		* corresponding services
		*****************************************************/
		s_flistp = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(bi_pdp), "/search", -1,
								ebufp);
		if( s_pdp ) {
			PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp,
							ebufp);
		}
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags,
					ebufp);
		strcpy ( s_template, " select X from /billinfo where F1 = V1 "
			" and F2 != V2 ");
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, (void *)s_template,
									ebufp);
		
		arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1,
					ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_AR_BILLINFO_OBJ,
				(void *)bi_pdp, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS,
				2, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)bi_pdp,
			ebufp);
		res1_flistp = PIN_FLIST_ELEM_ADD (s_flistp,  PIN_FLD_RESULTS,
				0, ebufp);
		PIN_FLIST_FLD_SET(res1_flistp, PIN_FLD_POID, vp, ebufp);
		PIN_FLIST_FLD_SET(res1_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
		while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(
			r_flistp, PIN_FLD_RESULTS, &elemid_s,
				1, &cookie_s, ebufp)) != (pin_flist_t *)NULL) {
			svc1_bal_inflistp = PIN_FLIST_CREATE(ebufp);
			bi1_pdp = (poid_t *)PIN_FLIST_FLD_GET(res_flistp,
						PIN_FLD_POID, 0, ebufp);
			
			PIN_FLIST_FLD_SET(svc1_bal_inflistp, PIN_FLD_POID,
						(void *)bi1_pdp, ebufp);
			a1_pdp = (poid_t *)PIN_FLIST_FLD_GET(res_flistp,
					PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			PIN_FLIST_FLD_SET(svc1_bal_inflistp, PIN_FLD_ACCOUNT_OBJ,
					(void *)a1_pdp, ebufp);
			PCM_OP(ctxp, PCM_OP_BAL_GET_BAL_GRP_AND_SVC, 0,
				svc1_bal_inflistp, &svc1_bal_outflistp, ebufp);
			PIN_FLIST_CONCAT(*svc_bal_outflistpp, svc1_bal_outflistp,
								ebufp);
			PIN_FLIST_DESTROY_EX(&svc1_bal_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&svc1_bal_outflistp, NULL);
		}
	}
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_inv_pol_filter_services: error ", ebufp);
		goto cleanup;
	} 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	"fm_inv_pol_filter_services: output flist",
	*svc_bal_outflistpp);
	cleanup:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		
}

static void
fm_inv_pol_prep_invoice_parent_subord_plan_details(
	pcm_context_t           *ctxp,
	pin_flist_t             *out_flistp,
        int32 		        combined_inv_flags,
	int32			is_wholesale_billing, 
	pin_errbuf_t            *ebufp)
	
{
	pin_flist_t     *accountinfo_flistp =NULL;
	pin_flist_t     *billinfo_flistp =NULL;
	pin_flist_t     *s_flistp =NULL;
	pin_flist_t     *r_flistp =NULL;
	pin_flist_t     *res_flistp =NULL;
	pin_flist_t     *result_flistp =NULL;
	pin_flist_t     *arg_flistp =NULL;
	poid_t          *account_pdp = NULL;
	poid_t          *bill_pdp = NULL;
	poid_t          *s_pdp = NULL;
	poid_t          *billinfo_pdp = NULL;
	poid_t          *arbillinfo_pdp = NULL;
	pin_cookie_t    cookie_s = NULL;
	int32           elemid_s = 0;
	char            s_template[2*BUFSIZ] = {""};
	void            *vp = NULL;
	int32           s_flags = SRCH_DISTINCT;
	int32		iterator = 0;
	int32		ar_hierarchy_size = 0;

               

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_parent_subord_plan_details: input flist",
			out_flistp);

	bill_pdp = (poid_t *)PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_POID, 0,
			ebufp);

	accountinfo_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_ACCTINFO, 0, 0,
			ebufp);
	account_pdp = (poid_t *) PIN_FLIST_FLD_GET(accountinfo_flistp,
			PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	billinfo_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_BILLINFO, 0, 0,
			ebufp);

	billinfo_pdp = (poid_t *) PIN_FLIST_FLD_GET(billinfo_flistp,
			PIN_FLD_BILLINFO_OBJ, 0,ebufp);
	
	arbillinfo_pdp = (poid_t *) PIN_FLIST_FLD_GET(billinfo_flistp,
			PIN_FLD_AR_BILLINFO_OBJ, 0,ebufp);

	vp = PIN_FLIST_FLD_GET(billinfo_flistp, PIN_FLD_AR_HIERARCHY_SIZE, 0,ebufp);
	if ( vp != NULL) {
		ar_hierarchy_size = *(int32 *)vp;
	}

	fm_inv_pol_prep_invoice_purchase_prod_disc_details(ctxp,out_flistp,
			account_pdp,billinfo_pdp,bill_pdp,&iterator,ebufp);

	if ((ar_hierarchy_size > 0) && 
	    (PIN_POID_COMPARE(billinfo_pdp,arbillinfo_pdp, 0, ebufp) == 0) &&
            (combined_inv_flags & PIN_INV_TYPE_PARENT) &&
            (combined_inv_flags & PIN_INV_TYPE_SUBORDINATE) &&
		!is_wholesale_billing){
		s_flistp = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(bill_pdp), "/search", -1,
				ebufp);
		if( s_pdp ) {
			PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp,
					ebufp);
		}
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags,
				ebufp);
		strcpy ( s_template, " select X from /bill where F1 = V1 ");
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, (void *)s_template, ebufp);

		arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PARENT,
				(void *)bill_pdp, ebufp);

		result_flistp = PIN_FLIST_ELEM_ADD (s_flistp,  PIN_FLD_RESULTS,
				0, ebufp);
		vp = NULL;
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, vp, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_BILLINFO_OBJ, vp, ebufp);

		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&s_flistp,NULL);

		elemid_s = 0;
		cookie_s = NULL;
		while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(
						r_flistp, PIN_FLD_RESULTS, &elemid_s,
						1, &cookie_s, ebufp)) != (pin_flist_t *)NULL) {

			account_pdp = (poid_t *)PIN_FLIST_FLD_GET(res_flistp,
					PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			bill_pdp = (poid_t *)PIN_FLIST_FLD_GET(res_flistp,
					PIN_FLD_POID, 0, ebufp);
			billinfo_pdp = (poid_t *)PIN_FLIST_FLD_GET(res_flistp,
					PIN_FLD_BILLINFO_OBJ, 0, ebufp);

			fm_inv_pol_prep_invoice_purchase_prod_disc_details(ctxp,out_flistp,
					account_pdp,billinfo_pdp,bill_pdp,&iterator,ebufp);

		}
		PIN_FLIST_DESTROY_EX(&r_flistp,NULL);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes():
 *
 * Function to aggregate the real time taxes from the tax
 * balance impacts in the events and also aggregate the
 * cycle/deferred taxes from /item/cycle_tax for each service
 *
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t     *dev_flistp = NULL;
	pin_flist_t     *svc_flistp = NULL;
	pin_flist_t     *tmp_flistp = NULL;
	pin_cookie_t    dev_cookie = NULL;
	pin_cookie_t    svc_cookie = NULL;
	int32	   	dev_elem_id = 0;
	int32	   	svc_elem_id = 0;
	int32	   	is_corr_bill = 0;
	char		*b_name = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes: input flist",
		out_flistp);

	tmp_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_BILLINFO,
                        0, 0, ebufp);
        b_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0,ebufp);
        if (b_name && (strncmp(b_name, PIN_OBJ_NAME_CORRECTIVE_BILL,
                          strlen(PIN_OBJ_NAME_CORRECTIVE_BILL)) == 0)) {
                is_corr_bill = 1;
        }

	/* 
	 * Iterate through the DEVICES
	 */
        dev_elem_id = 0;
        dev_cookie = NULL;
        while ((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
                        PIN_FLD_DEVICES, &dev_elem_id, 1,
                        &dev_cookie, ebufp)) != (pin_flist_t *)NULL) {

		/* 
		 *  Iterate through the SERVICES
	 	 */
		svc_elem_id = 0;
		svc_cookie = NULL;
		while ((svc_flistp = PIN_FLIST_ELEM_GET_NEXT(dev_flistp,
				PIN_FLD_SERVICES, &svc_elem_id, 1, 
				&svc_cookie, ebufp)) != (pin_flist_t *)NULL) {

			/*
			 * Call function to aggregate for each service 
			 */
			fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes_for_service(
					ctxp, is_corr_bill, svc_flistp, ebufp);


		}
	}

	
	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes: output flist",
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes_for_service():
 *
 * Function to aggregate the real time taxes from the tax
 * balance impacts in the events and also aggregate the
 * cycle/deferred taxes from /item/cycle_tax for the service flist
 * being passed in.
 *
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes_for_service(
	pcm_context_t	*ctxp,
	int32		is_corr_bill,
	pin_flist_t	*svc_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t     *tax_info_flistp = NULL;
	pin_flist_t     *item_flistp = NULL;
	pin_flist_t     *items_flistp = NULL;
	pin_flist_t     *event_flistp = NULL;
        pin_flist_t     *total_flistp = NULL;
	pin_flist_t     *balimpact_flistp = NULL;
	pin_flist_t     *billable_items_flistp = NULL;
	pin_cookie_t    item_cookie = NULL;
	pin_cookie_t    event_cookie = NULL;
        pin_cookie_t    total_cookie = NULL;
	pin_cookie_t    balimpact_cookie = NULL;
	pin_cookie_t    cookie = NULL;
	int32	   	elem_id = 0;
	int32	   	item_elem_id = 0;
	int32	   	event_elem_id = 0;
	int32	   	balimpact_elem_id = 0;
        int32           total_elem_id = 0;
	poid_t	  	*a_pdp = NULL;
	poid_t	  	*item_pdp = NULL;
        poid_t          *event_poid = NULL;
	poid_t		*svc_pdp = NULL;
	poid_t		*other_bill_item_pdp = NULL;
	poid_t		*bill_item_pdp = NULL;
	poid_t		*session_pdp = NULL;
        void            *vp = NULL;
	pin_decimal_t   *rt_tax_total = (pin_decimal_t *)NULL;
	pin_decimal_t   *cycle_tax_total = (pin_decimal_t *)NULL;
        pin_decimal_t   *rerate_total = (pin_decimal_t *)NULL;
	pin_decimal_t   *service_total = (pin_decimal_t *)NULL;
	pin_decimal_t   *temp_decimal = (pin_decimal_t *)NULL;
	int32		impact_type = 0;
	int32		bill_cnt = 0;
	int32		current_other_item = 0;
	char            *session_poid_type = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes_for_service: input flist",
		svc_flistp);

	rt_tax_total = pbo_decimal_from_str("0.0", ebufp);
	cycle_tax_total = pbo_decimal_from_str("0.0", ebufp);
        rerate_total = pbo_decimal_from_str("0.0", ebufp);
	service_total = pbo_decimal_from_str("0.0", ebufp);
	billable_items_flistp = PIN_FLIST_CREATE(ebufp);

	/* Iterate through the ITEMS
	 */
        item_elem_id = 0;
        item_cookie = NULL;
        while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(svc_flistp,
                        PIN_FLD_AR_ITEMS, &item_elem_id, 1,
                        &item_cookie, ebufp)) != (pin_flist_t *)NULL) {
		item_pdp =  PIN_FLIST_FLD_GET(item_flistp, 
					PIN_FLD_ITEM_OBJ, 0, ebufp);		
		items_flistp = PIN_FLIST_ELEM_ADD(billable_items_flistp,
                                        PIN_FLD_ITEMS, bill_cnt++, ebufp);	
		PIN_FLIST_FLD_SET(items_flistp, PIN_FLD_ITEM_OBJ,
			item_pdp, ebufp);
		
			
		/* get the item total */
		temp_decimal = PIN_FLIST_FLD_GET(item_flistp,
					PIN_FLD_ITEM_TOTAL, 0, ebufp);
		 
		/* Aggregate Cycle tax if taxation_switch is set */
	    	if ((pin_conf_taxation_switch == PIN_RATE_TAX_SWITCH_CYCLE) ||
                    (pin_conf_taxation_switch == PIN_RATE_TAX_SWITCH_BOTH)) {
			/* If this is a /item/cycle_tax then we aggregate the
		 	 * ITEM_TOTAL into the DEFERRED_TAX bucket.
		 	 */
			if (!PIN_POID_IS_NULL(item_pdp) &&
			   (strcmp(PIN_POID_GET_TYPE(item_pdp), ITEM_CYCLE_TAX )
				== 0)){
				if (!(perf_features_flags & 
				      INV_PERF_FEAT_SKIP_TAX_DETAILS)) {
 					pbo_decimal_add_assign(cycle_tax_total, 
						temp_decimal, ebufp);
				}
				continue;
			}
		}

		/* add up the item totals for non cycle_tax items */
		pbo_decimal_add_assign(service_total, temp_decimal, ebufp);

		/* Skip the realtime tax aggregation if 
		 * taxation_switch is not set
		 */
	    	if ((pin_conf_taxation_switch != PIN_RATE_TAX_SWITCH_EVENT) &&
                    (pin_conf_taxation_switch != PIN_RATE_TAX_SWITCH_BOTH)) {
			continue;
		}

		/* Skip the realtime tax aggregation if 
		 * perf_features SKIP_TAX flag is set 
		 */
	    	if (perf_features_flags & INV_PERF_FEAT_SKIP_TAX_DETAILS) {
			continue;
		}

		/* Now go through the events to get realtime taxes */
		event_elem_id = 0;
		event_cookie = NULL;
		while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp,
				PIN_FLD_EVENTS, &event_elem_id, 1, 
				&event_cookie, ebufp)) != (pin_flist_t *)NULL) {
			balimpact_elem_id = 0;
			balimpact_cookie = NULL;
			while ((balimpact_flistp = PIN_FLIST_ELEM_GET_NEXT(
					event_flistp, PIN_FLD_BAL_IMPACTS, 
					&balimpact_elem_id, 1, 
					&balimpact_cookie, ebufp)) 
					!= (pin_flist_t *)NULL) {
				impact_type = 0;
				vp = PIN_FLIST_FLD_GET(balimpact_flistp,
					PIN_FLD_IMPACT_TYPE, 1, ebufp);
				if (vp ) {
					impact_type = *(int32 *)vp;
                
				}
				if (impact_type != PIN_IMPACT_TYPE_TAX) {
					continue;
				}

				/* There is a tax balimpact - add this */
				temp_decimal = (pin_decimal_t *)
					PIN_FLIST_FLD_GET(balimpact_flistp, 
					PIN_FLD_AMOUNT, 0, ebufp);
				pbo_decimal_add_assign(rt_tax_total, 
					temp_decimal, ebufp);
			}

                        event_poid = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_EVENT_OBJ,
                                        0, ebufp);
                        if (!PIN_POID_IS_NULL(event_poid) &&
                                (strcmp(PIN_POID_GET_TYPE(event_poid), RERATE_ADJUSTMENT )
                                == 0)){
                        total_elem_id = 0;
                        total_cookie = NULL;
                        while ((total_flistp = PIN_FLIST_ELEM_GET_NEXT(
                                        event_flistp, PIN_FLD_TOTAL,
                                        &total_elem_id, 1,
                                        &total_cookie, ebufp))
                                        != (pin_flist_t *)NULL) {

                                        if (pin_beid_is_currency(total_elem_id)){
                                                /* There is a rerate adjustment - add this */
                                                temp_decimal = (pin_decimal_t *)
                                                        PIN_FLIST_FLD_GET(total_flistp,
                                                        PIN_FLD_AMOUNT, 0, ebufp);
                                                pbo_decimal_add_assign(rerate_total,
                                                        temp_decimal, ebufp);
                                        }
                                }
                        }
		}
	}

        item_elem_id = 0;
        item_cookie = NULL;
        while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(svc_flistp,
                        PIN_FLD_SUB_ITEMS, &item_elem_id, 1,
                        &item_cookie, ebufp)) != (pin_flist_t *)NULL) {
		item_pdp =  PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_ITEM_OBJ, 
					0, ebufp);		
		 items_flistp = PIN_FLIST_ELEM_ADD(billable_items_flistp,
                                        PIN_FLD_ITEMS, bill_cnt++, ebufp);
                PIN_FLIST_FLD_SET(items_flistp, PIN_FLD_ITEM_OBJ,
                        item_pdp, ebufp);
			
		/* get the item total */
		temp_decimal = PIN_FLIST_FLD_GET(item_flistp,
					PIN_FLD_ITEM_TOTAL, 0, ebufp);

                /* Aggregate Cycle tax if taxation_switch is set */
                if ((pin_conf_taxation_switch == PIN_RATE_TAX_SWITCH_CYCLE) ||
                    (pin_conf_taxation_switch == PIN_RATE_TAX_SWITCH_BOTH)) {
                        /* If this is a /item/cycle_tax then we aggregate the
                         * ITEM_TOTAL into the DEFERRED_TAX bucket.
                         */
                        if (!PIN_POID_IS_NULL(item_pdp) &&
                           (strcmp(PIN_POID_GET_TYPE(item_pdp), ITEM_CYCLE_TAX )
                                == 0)){
				if (!(perf_features_flags & 
				      INV_PERF_FEAT_SKIP_TAX_DETAILS)) {
 					pbo_decimal_add_assign(cycle_tax_total, 
						temp_decimal, ebufp);
				}
				continue;
                        }
                }

		/* add up the item totals for non cycle_tax items */
		pbo_decimal_add_assign(service_total, temp_decimal, ebufp);

                /* Skip the realtime tax aggregation if
                 * taxation_switch is not set
                 */
                if ((pin_conf_taxation_switch != PIN_RATE_TAX_SWITCH_EVENT) &&
                    (pin_conf_taxation_switch != PIN_RATE_TAX_SWITCH_BOTH)) {
                        continue;
                }

		/* Skip the realtime tax aggregation if 
		 * perf_features SKIP_TAX flag is set 
		 */
	    	if (perf_features_flags & INV_PERF_FEAT_SKIP_TAX_DETAILS) {
			continue;
		}

		/* Now go through the events to get realtime taxes */
		event_elem_id = 0;
		event_cookie = NULL;
		while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp,
				PIN_FLD_EVENTS, &event_elem_id, 1, 
				&event_cookie, ebufp)) != (pin_flist_t *)NULL) {
			balimpact_elem_id = 0;
			balimpact_cookie = NULL;
			while ((balimpact_flistp = PIN_FLIST_ELEM_GET_NEXT(
					event_flistp, PIN_FLD_BAL_IMPACTS, 
					&balimpact_elem_id, 1, 
					&balimpact_cookie, ebufp)) 
					!= (pin_flist_t *)NULL) {
				impact_type = 0;
				vp = PIN_FLIST_FLD_GET(balimpact_flistp,
					PIN_FLD_IMPACT_TYPE, 1, ebufp);
				if (vp ) {
					impact_type = *(int32 *)vp;
                
				}
				if (impact_type != PIN_IMPACT_TYPE_TAX) {
					continue;
				}

				/* There is a tax balimpact - add this */
				temp_decimal = (pin_decimal_t *)
					PIN_FLIST_FLD_GET(balimpact_flistp, 
					PIN_FLD_AMOUNT, 0, ebufp);
				pbo_decimal_add_assign(rt_tax_total, 
					temp_decimal, ebufp);
			}

                        event_poid = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_EVENT_OBJ,
                                        0, ebufp);
                        if (!PIN_POID_IS_NULL(event_poid) &&
                                (strcmp(PIN_POID_GET_TYPE(event_poid), RERATE_ADJUSTMENT )
                                == 0)){
                        total_elem_id = 0;
                        total_cookie = NULL;
                        while ((total_flistp = PIN_FLIST_ELEM_GET_NEXT(
                                        event_flistp, PIN_FLD_TOTAL,
                                        &total_elem_id, 1,
                                        &total_cookie, ebufp))
                                        != (pin_flist_t *)NULL) {

                                        if (pin_beid_is_currency(total_elem_id)){
                                                /* There is a rerate adjustment - add this */
                                                temp_decimal = (pin_decimal_t *)
                                                        PIN_FLIST_FLD_GET(total_flistp,
                                                        PIN_FLD_AMOUNT, 0, ebufp);
                                                pbo_decimal_add_assign(rerate_total,
                                                        temp_decimal, ebufp);
                                        }
                                }
                        }
		}
	}


#ifdef INCLUDE_AR_IN_CHARGE_SUMMARY

        item_elem_id = 0;
        item_cookie = NULL;
        while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(svc_flistp,
                        PIN_FLD_OTHER_ITEMS, &item_elem_id, 1,
                        &item_cookie, ebufp)) != (pin_flist_t *)NULL) {
		item_pdp =  PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_ITEM_OBJ, 
					0, ebufp);		
		current_other_item = 0;
		if (is_corr_bill) {
			/***************************************************************
			* In balance forward Corrective invoice enabled, if special
			* items are created for previous bills as well as current 
			* bills ( either due to re-rating or due to manual operations)
			* OTHER_ITEMS section contails all bills OTHER_ITEMS
			* Here we need to consider only OTHER_ITEMS for the current bill
			******************************************************************/
			event_elem_id = 0;
			event_cookie = NULL;
			while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp,
                                PIN_FLD_EVENTS, &event_elem_id, 1,
                                &event_cookie, ebufp)) != (pin_flist_t *)NULL) {
				bill_item_pdp =  PIN_FLIST_FLD_GET(event_flistp, 
					PIN_FLD_RELATED_BILL_ITEM_OBJ, 
						1, ebufp);		
				/*******************************************************
				* In case of older bills, we donot fetch transfer events
				* so PIN_FLD_RELATED_BILL_ITEM_OBJ field is not populated. 
				* Ex: previously created unallocated adjustment and
				* getting allocated now will not have this field.
				*********************************************************/
				if (!PIN_POID_IS_NULL(bill_item_pdp) && bill_item_pdp) {
					elem_id = 0;
					cookie = NULL;
					while ((items_flistp = PIN_FLIST_ELEM_GET_NEXT(billable_items_flistp,
						PIN_FLD_ITEMS, &elem_id, 1,
						&cookie, ebufp)) != (pin_flist_t *)NULL) {
						other_bill_item_pdp = PIN_FLIST_FLD_GET(items_flistp, 
							PIN_FLD_ITEM_OBJ, 0, ebufp);
						if (PIN_POID_COMPARE(bill_item_pdp, other_bill_item_pdp, 0,
									ebufp) == 0) {
							current_other_item = 1;
						}
					}
				}
			}
			if (!current_other_item) {
				PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG,
					"Does not belong to Current Bill. Skipping ....");
				continue;
			}
		}

		/* get the item total */
		temp_decimal = PIN_FLIST_FLD_GET(item_flistp,
					PIN_FLD_ITEM_TOTAL, 0, ebufp);

                /* Aggregate Cycle tax if taxation_switch is set */
                if ((pin_conf_taxation_switch == PIN_RATE_TAX_SWITCH_CYCLE) ||
                    (pin_conf_taxation_switch == PIN_RATE_TAX_SWITCH_BOTH)) {
                        /* If this is a /item/cycle_tax then we aggregate the
                         * ITEM_TOTAL into the DEFERRED_TAX bucket.
                         */
                        if (!PIN_POID_IS_NULL(item_pdp) &&
                           (strcmp(PIN_POID_GET_TYPE(item_pdp), ITEM_CYCLE_TAX )
                                == 0)){
				if (!(perf_features_flags & 
				      INV_PERF_FEAT_SKIP_TAX_DETAILS)) {
 					pbo_decimal_add_assign(cycle_tax_total, 
						temp_decimal, ebufp);
				}
				continue;
                        }
                }

		/* add up the item totals for non cycle_tax items */
		pbo_decimal_add_assign(service_total, temp_decimal, ebufp);

                /* Skip the realtime tax aggregation if
                 * taxation_switch is not set
                 */
                if ((pin_conf_taxation_switch != PIN_RATE_TAX_SWITCH_EVENT) &&
                    (pin_conf_taxation_switch != PIN_RATE_TAX_SWITCH_BOTH)) {
                        continue;
                }

		/* Skip the realtime tax aggregation if 
		 * perf_features SKIP_TAX flag is set 
		 */
	    	if (perf_features_flags & INV_PERF_FEAT_SKIP_TAX_DETAILS) {
			continue;
		}

		/* Now go through the events to get realtime taxes */
		event_elem_id = 0;
		event_cookie = NULL;
		while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp,
				PIN_FLD_EVENTS, &event_elem_id, 1, 
				&event_cookie, ebufp)) != (pin_flist_t *)NULL) {
			balimpact_elem_id = 0;
			balimpact_cookie = NULL;
			while ((balimpact_flistp = PIN_FLIST_ELEM_GET_NEXT(
					event_flistp, PIN_FLD_BAL_IMPACTS, 
					&balimpact_elem_id, 1, 
					&balimpact_cookie, ebufp)) 
					!= (pin_flist_t *)NULL) {
				impact_type = 0;
				vp = PIN_FLIST_FLD_GET(balimpact_flistp,
					PIN_FLD_IMPACT_TYPE, 1, ebufp);
				if (vp ) {
					impact_type = *(int32 *)vp;
                
				}
				if (impact_type != PIN_IMPACT_TYPE_TAX) {
					continue;
				}

				/* There is a tax balimpact - add this */
				temp_decimal = (pin_decimal_t *)
					PIN_FLIST_FLD_GET(balimpact_flistp, 
					PIN_FLD_AMOUNT, 0, ebufp);
				pbo_decimal_add_assign(rt_tax_total, 
					temp_decimal, ebufp);
			}

			/*****************************************************
			* In case of OTHER_ITEMS also we need to capture the
			* rerate_total if the items are allocated
			******************************************************/
			if (is_corr_bill) {
				event_poid = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_EVENT_OBJ,
					0, ebufp);
				session_pdp = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_SESSION_OBJ,
					0, ebufp);
				session_poid_type = (char *)PIN_POID_GET_TYPE(session_pdp);
				if (!PIN_POID_IS_NULL(event_poid) &&
					((strcmp(PIN_POID_GET_TYPE(event_poid), RERATE_ADJUSTMENT )
					== 0) || ((strstr(PIN_POID_GET_TYPE(event_poid), PIN_OBJ_TYPE_EVENT_CYCLE_BASE))&&
						(strstr(session_poid_type, PIN_OBJ_TYPE_EVENT_CYCLE_BASE))))){
					total_elem_id = 0;
					total_cookie = NULL;
					while ((total_flistp = PIN_FLIST_ELEM_GET_NEXT(
						event_flistp, PIN_FLD_TOTAL,
							&total_elem_id, 1,
								&total_cookie, ebufp))
								!= (pin_flist_t *)NULL) {

						if (pin_beid_is_currency(total_elem_id)){
							/* There is a rerate adjustment - add this */
							temp_decimal = (pin_decimal_t *)
								PIN_FLIST_FLD_GET(total_flistp,
									PIN_FLD_AMOUNT, 0, ebufp);
							pbo_decimal_add_assign(rerate_total,
								temp_decimal, ebufp);
						}
					}
				}
			}
		}
	}

#endif /* INCLUDE_AR_IN_CHARGE_SUMMARY */

	/* Add the PIN_FLD_TAX_INFO and put the aggregated taxes */
	tax_info_flistp = PIN_FLIST_SUBSTR_ADD(svc_flistp, PIN_FLD_TAX_INFO,
                                ebufp);	
	PIN_FLIST_FLD_PUT(tax_info_flistp, PIN_FLD_REALTIME_TAX_AMOUNT,
                                        (void *)rt_tax_total, ebufp)
	PIN_FLIST_FLD_PUT(tax_info_flistp, PIN_FLD_DEFERRED_TAX_AMOUNT,
                                        (void *)cycle_tax_total, ebufp)
        PIN_FLIST_FLD_PUT(tax_info_flistp, PIN_FLD_AMOUNT_ADJUSTED,
                                        (void *)rerate_total, ebufp)

	/* Subtract the real time tax from the item total
	 * to get the non-tax amount and set it to TOTALS 
	 */
	pbo_decimal_subtract_assign(service_total, rt_tax_total, ebufp);
        pbo_decimal_subtract_assign(service_total, rerate_total, ebufp);

	PIN_FLIST_FLD_PUT(tax_info_flistp, PIN_FLD_TOTALS,
                                        (void *)service_total, ebufp)

	/* Set the Service POID */
	svc_pdp = (poid_t *)PIN_FLIST_FLD_GET (svc_flistp, PIN_FLD_SERVICE_OBJ,
					0, ebufp);
	PIN_FLIST_FLD_SET(tax_info_flistp, PIN_FLD_SERVICE_OBJ, 
					(void *)svc_pdp, ebufp);

	PIN_FLIST_DESTROY_EX(&billable_items_flistp, NULL);
	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes_for_service: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_add_realtime_and_cycle_taxes_for_service: output flist",
			svc_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_add_ratetag():
 * This function adds rate tag to the event level.
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_add_ratetag(
        pin_flist_t             *out_flistp,
        int32                   fld_no,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *tmp_flistp = NULL;
        pin_flist_t             *item_flistp = NULL;
        pin_flist_t             *event_flistp = NULL;
        pin_flist_t             *tmpevent_flistp = NULL;
        pin_cookie_t            cookie = NULL;
        pin_cookie_t            prev_cookie = NULL;
        int32                   elemid = 0;
        pin_cookie_t            inner_cookie = NULL;
        int32                   inner_elemid = 0;

        int                     has_impact = PIN_BOOLEAN_FALSE;


        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        /***********************************************************
         * Debug what we got.
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_prep_invoice_add_ratetag: input flist (out_flistp)",
                out_flistp);

        /***********************************************************
         * Loop through the items array.
         ***********************************************************/
        while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
                                fld_no, &elemid, 1, &cookie, ebufp))
                                != (pin_flist_t *)NULL) {

                /***************************************************
                 * Debug what we got.
                 ***************************************************/
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_inv_pol_prep_invoice_add_ratetag: item flist",
                         item_flistp);

                /***************************************************
                 * Loop through the events array for this item
                 ***************************************************/
                inner_cookie = NULL;
                prev_cookie = NULL;
                inner_elemid = 0;

                while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp,
                        PIN_FLD_EVENTS, &inner_elemid, 1,
                        &inner_cookie, ebufp)) != (pin_flist_t *)NULL) {


                        /*******************************************
                         * Pass the current event to the function
                         * fm_inv...hasimpact() to determine if
                         * this event has a balance impact.
                         *******************************************/
                        has_impact =
                                 fm_inv_pol_prep_invoice_rmevents_hasimpact(
                                        event_flistp, ebufp);

                        if (has_impact) {

                                /*
                                 * Call function to loop PIN_FLD_BAL_IMPACT
                                 * array and move the PIN_FLD_RATE to the
                                 * event level.
                                 */
                                fm_inv_pol_prep_invoice_addevents_ratetag(
                                        event_flistp, ebufp);

                        }
			else {
				PIN_FLIST_ELEM_DROP(item_flistp, PIN_FLD_EVENTS,
					inner_elemid, ebufp);
				inner_cookie = prev_cookie;
				continue;
			}
			prev_cookie = inner_cookie;

                }       /* end event loop */

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_inv_pol_prep_invoice_add_ratetag: item flist",
                         item_flistp);

        }       /* end item loop */


        /***********************************************************
         * We return the out_flistp.
         ***********************************************************/
        return;
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_addevents_ratetag():
 *
 * This function copy the PIN_FLD_RATE to the event level.
 * We assign the first PIN_FLD_RATE value we come across, but if
 * there is more than one rate tag, we  add an '*' to the rate tag
 * string.
 *
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_addevents_ratetag(
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *flistp = NULL;
        pin_cookie_t            cookie = NULL;
        int32                   elemid = 0;
        int32                   counter = 0;
        void                    *vp = NULL;
        char                    *str = NULL;

        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        /***********************************************************
         * Debug what we got.
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_inv...balimpact:" \
                                " input flistp", i_flistp);

        /***********************************************************
         * Loop through the balance impact array.
         ***********************************************************/
        while ((flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
                  &elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

                /***************************************************
                 * Move the rate tag to the event level.
                 * We pick up the first rate tag we come across.
                 ***************************************************/
                if (counter == 0) {
			vp = NULL;
                        vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_RATE_TAG, 1,
                                                 ebufp);
                        if (vp != NULL) {
				if (!(strcmp((char *)vp,"") == 0 )){
					str = (char *)pin_malloc((strlen(vp) + 1));
					sprintf((char *)str, "%s", (char *)vp);
				}
			}
                }
                counter++;

        }

        /***********************************************************
         * If there is more than one PIN_FLD_BAL_IMPACT, then
         * add a '*' to the rate tag string before moving it to
         * the event level.
         ***********************************************************/
		if ((counter > 1) && (str)) {
			str = pin_realloc(str, strlen((char *)vp) + 2);
			if (str) {
				sprintf((char *)str, "%s*", (char *)vp);
				PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_LABEL, (char *)str,
							 ebufp);
			} else {
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NO_MEM, 0, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"op_inv_pol_prep_invoice no memory", ebufp);
				return;
			}
		}
		else if ((counter == 1) && (str)){
			PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_LABEL, (char *)str,
							 ebufp);
		}
		

        /***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_inv_pol_prep_invoice_addevents_ratetag: error", ebufp); 
        }

        /***********************************************************
         * We return the event that was passed in (i_flistp).
         ***********************************************************/

        return;
}


/*******************************************************************
 * fm_inv_pol_prep_invoice_purchase_prod_disc_details():
 *
 * Get active/inactive/cancel  purchase products and discounts for the billing
 * period
 *
 *******************************************************************/
static void
fm_inv_pol_prep_invoice_purchase_prod_disc_details(
	pcm_context_t           *ctxp,
	pin_flist_t             *out_flistp,
	poid_t			*acc_pdp,
	poid_t			*billinfo_pdp,
	poid_t			*bill_pdp,
	int32			*iter,
	pin_errbuf_t            *ebufp)

{
	
	pin_flist_t     	*accountinfo_flistp = NULL;
	pin_flist_t		*purchase_prod_disc_flistp = NULL;
	pin_flist_t		*billinfo_flistp = NULL;
	pin_flist_t		*gpo_return_flistp = NULL;
	pin_flist_t		*product_flistp = NULL;
	pin_flist_t		*products_flistp = NULL;
	pin_flist_t		*discount_flistp = NULL;
	pin_flist_t		*discounts_flistp = NULL;
	pin_flist_t		*field_flistp = NULL;
	pin_flist_t		*plan_name_read_flistp = NULL;
	pin_flist_t		*plan_readflds_return_flistp = NULL;
	pin_flist_t		*deal_name_read_flistp = NULL;
	pin_flist_t		*deal_readflds_return_flistp = NULL;
	pin_flist_t		*product_name_read_flistp = NULL;
	pin_flist_t		*discount_name_read_flistp = NULL;
	pin_flist_t		*product_readflds_return_flistp = NULL;
	pin_flist_t		*discount_readflds_return_flistp = NULL;
	pin_flist_t		*billinfo_read_flistp = NULL;
	pin_flist_t		*billinfo_readflds_return_flistp = NULL;
    	void            	*vp = NULL;
    	poid_t          	*a_pdp = NULL;
    	poid_t          	*null_pdp = NULL;

	time_t			bill_start_t = 0;
	time_t			bill_end_t = 0;
	time_t			purchase_start_t = 0;
	time_t			purchase_end_t = 0;
	int32			status_flags = 0;
	int32			validity_flags = 0;
	int32			inclusion_flags = 0;
	int32           	elemid = 0;
	pin_cookie_t		cookie = NULL;
	

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	status_flags = PIN_SUBS_FLG_OFFERING_STATUS_ACTIVE |
			PIN_SUBS_FLG_OFFERING_STATUS_INACTIVE  |
			PIN_SUBS_FLG_OFFERING_STATUS_CLOSED;

	inclusion_flags = PIN_SUBS_FLG_INCLUDE_ALL_ELIGIBLE_PRODS |
			PIN_SUBS_FLG_INCLUDE_ALL_ELIGIBLE_DISCS;	

    /***********************************************************
     * Debug what we got.
    ***********************************************************/

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
    		"fm_inv_pol_prep_invoice_purchase_prod_disc_details: input flist",
        out_flistp);

	billinfo_read_flistp = PIN_FLIST_CREATE(ebufp);
    	PIN_FLIST_FLD_SET(billinfo_read_flistp, PIN_FLD_POID, (void *)bill_pdp,
			ebufp);
    	PIN_FLIST_FLD_SET(billinfo_read_flistp, PIN_FLD_START_T, (void *)NULL,
			ebufp);
	PIN_FLIST_FLD_SET(billinfo_read_flistp, PIN_FLD_END_T, (void *)NULL,
			ebufp);

    	PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE,
                        billinfo_read_flistp,
                        &billinfo_readflds_return_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&billinfo_read_flistp, NULL);
	vp = PIN_FLIST_FLD_GET(billinfo_readflds_return_flistp, 
			PIN_FLD_START_T, 0, ebufp);
	if (vp) {
                bill_start_t = *((time_t *)vp);
    	}

	vp = PIN_FLIST_FLD_GET(billinfo_readflds_return_flistp, PIN_FLD_END_T, 0,
			ebufp);
	if (vp) {
                bill_end_t = *((time_t *)vp);
    	}
	PIN_FLIST_DESTROY_EX(&billinfo_readflds_return_flistp, NULL);

	purchase_prod_disc_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_SET(purchase_prod_disc_flistp, PIN_FLD_POID,
                                        (void *)acc_pdp, ebufp);

	PIN_FLIST_FLD_SET(purchase_prod_disc_flistp, PIN_FLD_SCOPE_OBJ,
			(void *)billinfo_pdp,ebufp);

	PIN_FLIST_FLD_SET(purchase_prod_disc_flistp, PIN_FLD_END_T,
			&bill_end_t, ebufp);
	PIN_FLIST_FLD_SET(purchase_prod_disc_flistp, PIN_FLD_STATUS_FLAGS,
			&status_flags, ebufp);
	PIN_FLIST_FLD_SET(purchase_prod_disc_flistp, PIN_FLD_INCLUSION_FLAGS,
			&inclusion_flags, ebufp);
	product_flistp = PIN_FLIST_ELEM_ADD(purchase_prod_disc_flistp,
			PIN_FLD_PRODUCTS, 0, ebufp);
	vp = NULL;
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_CYCLE_START_T,(void *)vp,ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_CYCLE_END_T, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PURCHASE_START_T, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PURCHASE_END_T, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_USAGE_START_T, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_USAGE_END_T, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PLAN_OBJ, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_DEAL_OBJ, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PRODUCT_OBJ, (void *)vp, ebufp);	
	discount_flistp = PIN_FLIST_ELEM_ADD(purchase_prod_disc_flistp,
			PIN_FLD_DISCOUNTS, 0, ebufp);
	vp = NULL;
	PIN_FLIST_FLD_SET(discount_flistp, PIN_FLD_CYCLE_START_T,(void *)vp,ebufp);
	PIN_FLIST_FLD_SET(discount_flistp, PIN_FLD_CYCLE_END_T, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(discount_flistp, PIN_FLD_PURCHASE_START_T, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(discount_flistp, PIN_FLD_PURCHASE_END_T, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(discount_flistp, PIN_FLD_USAGE_START_T, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(discount_flistp, PIN_FLD_USAGE_END_T, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(discount_flistp, PIN_FLD_PLAN_OBJ, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(discount_flistp, PIN_FLD_DEAL_OBJ, (void *)vp, ebufp);
	PIN_FLIST_FLD_SET(discount_flistp, PIN_FLD_DISCOUNT_OBJ, (void *)vp, ebufp);

	PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_GET_PURCHASED_OFFERINGS, 0,
			purchase_prod_disc_flistp,&gpo_return_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&purchase_prod_disc_flistp, NULL);

	elemid = 0;
	cookie = NULL;
	while ((products_flistp = PIN_FLIST_ELEM_GET_NEXT(gpo_return_flistp,
               		PIN_FLD_PRODUCTS, &elemid, 1, &cookie, ebufp)) !=
               		(pin_flist_t *)NULL) {

		vp = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PURCHASE_START_T,
				0,ebufp);
		if (vp) {
			purchase_start_t = *((time_t *)vp);
    		}

		vp = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PURCHASE_END_T, 0,ebufp);
		
		if (vp) {
			purchase_end_t = *((time_t *)vp);
        	}

		if ( ( purchase_end_t >bill_start_t || purchase_end_t  == 0 )  &&
				purchase_start_t <= bill_end_t  )  {

			field_flistp = PIN_FLIST_ELEM_ADD(out_flistp, PIN_FLD_FIELD,
					                                        (*iter)++, ebufp);
           	PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_ACCOUNT_OBJ,(void *)acc_pdp,ebufp);
			vp = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PLAN_OBJ, 1,ebufp);
			if( vp && !PIN_POID_IS_NULL( vp ) ) {
				plan_name_read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(plan_name_read_flistp, PIN_FLD_POID, (void *)vp, ebufp);
				PIN_FLIST_FLD_SET(plan_name_read_flistp, PIN_FLD_NAME, (void *)NULL, ebufp);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE,
						plan_name_read_flistp, 
						&plan_readflds_return_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&plan_name_read_flistp, NULL);
				vp = PIN_FLIST_FLD_GET(plan_readflds_return_flistp, PIN_FLD_NAME, 0, ebufp);
				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_RATE_PLAN_NAME, (void *)vp, ebufp);
				PIN_FLIST_DESTROY_EX(&plan_readflds_return_flistp, NULL);
			} else {

				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_RATE_PLAN_NAME, (void *)"N/A", ebufp);
			}

			vp = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_DEAL_OBJ, 1,ebufp);
			if( vp && !PIN_POID_IS_NULL( vp ) ) {
				deal_name_read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(deal_name_read_flistp, PIN_FLD_POID, (void *)vp, ebufp);
				PIN_FLIST_FLD_SET(deal_name_read_flistp, PIN_FLD_NAME, (void *)NULL, ebufp);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE,
						deal_name_read_flistp, &deal_readflds_return_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&deal_name_read_flistp, NULL);
				vp = PIN_FLIST_FLD_GET(deal_readflds_return_flistp, PIN_FLD_NAME, 0, ebufp);
				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_DEAL_NAME, (void *)vp, ebufp);
				PIN_FLIST_DESTROY_EX(&deal_readflds_return_flistp, NULL);
			} else {

				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_DEAL_NAME, (void *)"N/A", ebufp);
			}

			vp = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PRODUCT_OBJ, 1,ebufp);
			if( vp && !PIN_POID_IS_NULL( vp ) ) {
				product_name_read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(product_name_read_flistp, PIN_FLD_POID, (void *)vp, ebufp);
				PIN_FLIST_FLD_SET(product_name_read_flistp, PIN_FLD_NAME, (void *)NULL, ebufp);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE,
						product_name_read_flistp, &product_readflds_return_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&product_name_read_flistp, NULL);
				vp = PIN_FLIST_FLD_GET(product_readflds_return_flistp, PIN_FLD_NAME, 0, ebufp);
				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_PRODUCT_NAME, (void *)vp, ebufp);
				PIN_FLIST_DESTROY_EX(&product_readflds_return_flistp, NULL);
			} else {

				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_PRODUCT_NAME, (void *)"N/A", ebufp);
			}
			vp = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_SERVICE_OBJ, 1,ebufp);
			if( vp && !PIN_POID_IS_NULL( vp ) ) {
				PIN_FLIST_FLD_SET(field_flistp, PIN_FLD_SERVICE_OBJ, (void *)vp ,ebufp);
			} else {
				null_pdp =  PIN_POID_FROM_STR("0.0.0.0  0 0", NULL, ebufp);
            	PIN_FLIST_FLD_SET(field_flistp, PIN_FLD_SERVICE_OBJ, (void *)null_pdp, ebufp);
			}
		}

	}

	elemid = 0;
	cookie = NULL;
	while ((discounts_flistp = PIN_FLIST_ELEM_GET_NEXT(gpo_return_flistp,
               		PIN_FLD_DISCOUNTS, &elemid, 1, &cookie, ebufp)) !=
               		(pin_flist_t *)NULL) {

		vp = PIN_FLIST_FLD_GET(discounts_flistp, PIN_FLD_PURCHASE_START_T,
				0,ebufp);
		if (vp) {
			purchase_start_t = *((time_t *)vp);
    		}

		vp = PIN_FLIST_FLD_GET(discounts_flistp, PIN_FLD_PURCHASE_END_T, 0,ebufp);
		
		if (vp) {
			purchase_end_t = *((time_t *)vp);
        	}

		if ( ( purchase_end_t >bill_start_t || purchase_end_t  == 0 )  &&
				purchase_start_t <= bill_end_t  )  {

			field_flistp = PIN_FLIST_ELEM_ADD(out_flistp, PIN_FLD_FIELD,
					                                        (*iter)++, ebufp);
           		PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_ACCOUNT_OBJ,(void *)acc_pdp,ebufp);
			vp = PIN_FLIST_FLD_GET(discounts_flistp, PIN_FLD_PLAN_OBJ, 1,ebufp);
			if( vp && !PIN_POID_IS_NULL( vp ) ) {
				plan_name_read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(plan_name_read_flistp, PIN_FLD_POID, (void *)vp, ebufp);
				PIN_FLIST_FLD_SET(plan_name_read_flistp, PIN_FLD_NAME, (void *)NULL, ebufp);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE,
						plan_name_read_flistp, 
						&plan_readflds_return_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&plan_name_read_flistp, NULL);
				vp = PIN_FLIST_FLD_GET(plan_readflds_return_flistp, PIN_FLD_NAME, 0, ebufp);
				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_RATE_PLAN_NAME, (void *)vp, ebufp);
				PIN_FLIST_DESTROY_EX(&plan_readflds_return_flistp, NULL);
			} else {

				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_RATE_PLAN_NAME, (void *)"N/A", ebufp);
			}

			vp = PIN_FLIST_FLD_GET(discounts_flistp, PIN_FLD_DEAL_OBJ, 1,ebufp);
			if( vp && !PIN_POID_IS_NULL( vp ) ) {
				deal_name_read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(deal_name_read_flistp, PIN_FLD_POID, (void *)vp, ebufp);
				PIN_FLIST_FLD_SET(deal_name_read_flistp, PIN_FLD_NAME, (void *)NULL, ebufp);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE,
						deal_name_read_flistp, &deal_readflds_return_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&deal_name_read_flistp, NULL);
				vp = PIN_FLIST_FLD_GET(deal_readflds_return_flistp, PIN_FLD_NAME, 0, ebufp);
				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_DEAL_NAME, (void *)vp, ebufp);
				PIN_FLIST_DESTROY_EX(&deal_readflds_return_flistp, NULL);
			} else {

				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_DEAL_NAME, (void *)"N/A", ebufp);
			}

			vp = PIN_FLIST_FLD_GET(discounts_flistp, PIN_FLD_DISCOUNT_OBJ, 1,ebufp);
			if( vp && !PIN_POID_IS_NULL( vp ) ) {
				discount_name_read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(discount_name_read_flistp, PIN_FLD_POID, (void *)vp, ebufp);
				PIN_FLIST_FLD_SET(discount_name_read_flistp, PIN_FLD_NAME, (void *)NULL, ebufp);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE,
						discount_name_read_flistp, &discount_readflds_return_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&discount_name_read_flistp, NULL);
				vp = PIN_FLIST_FLD_GET(discount_readflds_return_flistp, PIN_FLD_NAME, 0, ebufp);
				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_DISCOUNT_INFO, (void *)vp, ebufp);
				PIN_FLIST_DESTROY_EX(&discount_readflds_return_flistp, NULL);
			} else {

				PIN_FLIST_FLD_SET(field_flistp,PIN_FLD_DISCOUNT_INFO, (void *)"N/A", ebufp);
			}
			vp = PIN_FLIST_FLD_GET(discounts_flistp, PIN_FLD_SERVICE_OBJ, 1,ebufp);
			if( vp && !PIN_POID_IS_NULL( vp ) ) {
				PIN_FLIST_FLD_SET(field_flistp, PIN_FLD_SERVICE_OBJ,
					(void *)vp ,ebufp);
			} else {
				null_pdp =  PIN_POID_FROM_STR("0.0.0.0  0 0", NULL, ebufp);
            			PIN_FLIST_FLD_SET(field_flistp, PIN_FLD_SERVICE_OBJ,
					(void *)null_pdp, ebufp);
			}
		}

	}

	
	PIN_FLIST_DESTROY_EX(&gpo_return_flistp, NULL);	
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_calculate_delta_amounts():
 *
 * Function to compute the delta amounts for events and items
 * that were changed, resulting in a corrective invoice
 * 
 * This function will add the following three fields to each event,
 * item and account levels.
 *
 * PIN_FLD_PREVIOUS_TOTAL
 * PIN_FLD_DELTA_DUE
 * PIN_FLD_CURRENT_TOTAL
 *
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_calculate_delta_amounts(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_calculate_delta_amounts: input flist",
		out_flistp);


	/*
	 * Get the bill level delta amounts
	 */
	fm_inv_pol_prep_invoice_calculate_delta_amounts_bill ( ctxp, 
					out_flistp, ebufp);

	/* 
	 * Get the item level delta amounts
	 * Need to do this only for AR and SUB items. The OTHER items
	 * are not corrected.
	 */
	fm_inv_pol_prep_invoice_calculate_delta_amounts_items ( ctxp, 
					out_flistp, PIN_FLD_AR_ITEMS, ebufp);
	fm_inv_pol_prep_invoice_calculate_delta_amounts_items ( ctxp, 
					out_flistp, PIN_FLD_SUB_ITEMS, ebufp);

	/* 
	 * Get the event level delta amounts
	 */
	fm_inv_pol_prep_invoice_calculate_delta_amounts_events ( ctxp, 
					out_flistp, ebufp);

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_calculate_delta_amounts: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_calculate_delta_amounts: output flist",
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_prior_event_item_corrections():
 *
 * Function to group all item level taxes, discounts and amounts 
 * from prior item corrections for replacement invoice
 * The following 3 additional fields in Item will be populated
 * PIN_FLD_REALTIME_TAXED_NET_AMOUNT
 * PIN_FLD_ACTG_REBATES
 * PIN_FLD_OUTSTANDING
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_prior_event_item_corrections(
        pcm_context_t   *ctxp,
        pin_flist_t     *item_flistp,
        pin_flist_t     *out_flistp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t             *ar_item_flistp = NULL;
	pin_flist_t             *other_item_flistp = NULL;
	pin_flist_t             *other_event_flistp = NULL;
	pin_flist_t             *e_bi_flistp = NULL;
	pin_flist_t             *event_flistp = NULL;
	pin_flist_t             *rel_bill_flistp = NULL;
	pin_flist_t             *billable_item_flistp = NULL;
        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie1 = NULL;
        pin_cookie_t            cookie2 = NULL;
        pin_cookie_t            cookie3 = NULL;
        pin_cookie_t            cookie_rel_bill = NULL;
        int32                   elemid = 0;
        int32                   elemid1 = 0;
        int32                   elemid2 = 0;
        int32                   elemid3 = 0;
        int32                   rel_bill_id = 0;
        int32                   impact_type = 0;
        int32                   update_event = 0;
        int32                   update_item = 0;
        int32                   billable_item_matched = 0;
        int32                   billable_item_count = 0;
        int32                   special_events_in_item = 0;
        int32                   multiple_allocations = 0;
        int32                   item_cycle_tax = 0;
        int32                   partial_allocation = 0;
	int32			oitem_status = 0;
        pin_decimal_t           *adj_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t		*zero_dec = (pin_decimal_t *)NULL;
	pin_decimal_t		*rt_tax_total = (pin_decimal_t *)NULL;
        pin_decimal_t		*disc_total = (pin_decimal_t *)NULL;
        pin_decimal_t		*temp_decimal = (pin_decimal_t *)NULL;
	pin_decimal_t   	*rel_item_amt = (pin_decimal_t *)NULL;
	pin_decimal_t		*tsfer_amt = (pin_decimal_t *)NULL;
	poid_t			*item_pdp = NULL;
	poid_t			*oitem_pdp = NULL;
	poid_t			*other_item_pdp = NULL;
	poid_t			*other_event_pdp = NULL;
	poid_t			*bill_item_pdp = NULL;
	poid_t			*session_pdp = NULL;
	poid_t			*event_pdp = NULL;
        void                    *vp = NULL;
	char			*item_type = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        /***********************************************************
         * Debug what we got.
         ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_prep_invoice_prior_event_item_corrections: out flist",
                out_flistp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_prep_invoice_prior_event_item_corrections: item flist",
                item_flistp);
	zero_dec = pbo_decimal_from_str("0.0", ebufp);

	PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_REALTIME_TAXED_NET_AMOUNT,
				(void *)zero_dec, ebufp);
	PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_ACTG_REBATES,
				(void *)zero_dec, ebufp);
	PIN_FLIST_FLD_SET(item_flistp,  PIN_FLD_OUTSTANDING,
				(void *)zero_dec, ebufp);
	item_pdp = PIN_FLIST_FLD_GET(item_flistp,
			PIN_FLD_ITEM_OBJ, 1, ebufp);
	item_type = (char *)PIN_POID_GET_TYPE(item_pdp);
	if (strstr(item_type, "cycle_tax")) {
		item_cycle_tax = 1;
		PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG,
			"item_cycle_tax prior corrections");
	}

	/********************************************
	*       Prior Item Corrections
	* For item we are adding the following:
	* PIN_FLD_REALTIME_TAXED_NET_AMOUNT  for tax
	* PIN_FLD_ACTG_REBATES for discount
	* PIN_FLD_OUTSTANDING for remaining amount
	*       Prior Event Corrections
	* PIN_FLD_TOTAL_DEBITS  for tax
	* PIN_FLD_REBATES_ISSUED for discount
	* PIN_FLD_TOTAL_CREDITS for remaining amount
	**********************************************/
	cookie = NULL;
	elemid = 0;
	while ((other_item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
			PIN_FLD_OTHER_ITEMS, &elemid, 1, &cookie,
                                ebufp)) != (pin_flist_t *)NULL) {
		oitem_pdp = PIN_FLIST_FLD_GET(other_item_flistp,
				PIN_FLD_ITEM_OBJ, 1, ebufp);
		vp = PIN_FLIST_FLD_GET(other_item_flistp,
				PIN_FLD_STATUS, 1, ebufp);
		if (vp) {
			oitem_status = *(int32 *)vp;	
		}
		tsfer_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(other_item_flistp,
                                PIN_FLD_TRANSFERED, 1, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_prep_invoice_prior_event_item_corrections: other_item_flist",
                other_item_flistp);
		elemid1 = 0;
		cookie1 = NULL;
		while ((other_event_flistp = PIN_FLIST_ELEM_GET_NEXT(
			other_item_flistp, PIN_FLD_EVENTS, &elemid1, 1,
				&cookie1, ebufp)) != (pin_flist_t *)NULL) {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_prior_event_item_corrections: other_event_flist",
			other_event_flistp);
			update_event = 0;
			update_item = 0;
			multiple_allocations = 0;
			partial_allocation = 0;
			session_pdp = PIN_FLIST_FLD_GET(other_event_flistp,
				PIN_FLD_SESSION_OBJ, 1, ebufp);

			 /******************************************************
			* In payment case we will have mutilple entries of
			* PIN_FLD_ITEMS ( nothing but event transfer buffer billable
			* items and amts)
			* This is also a case for multiple allocations
			**************************************************************/
			billable_item_flistp = PIN_FLIST_ELEM_GET(other_event_flistp,
				PIN_FLD_ITEMS, PIN_ELEMID_ANY, 1, ebufp);
			if (billable_item_flistp) {
				billable_item_count = PIN_FLIST_ELEM_COUNT(billable_item_flistp,
					PIN_FLD_ITEMS, ebufp);
			}

			/*****************************************************************
			* In the case of Account adj or bill adj we will have one item adj
			* and multiple event adjustments
			* This is also a case for multiple allocations
			******************************************************************/
			special_events_in_item = PIN_FLIST_ELEM_COUNT(other_event_flistp, 
				PIN_FLD_EVENTS, ebufp);

			if ((billable_item_count > 1) || (special_events_in_item > 1)){
				multiple_allocations = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Multiple Allocations");
			}
			
			if (multiple_allocations) {
				if (billable_item_count > 1) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Multiple Allocations - Multiple Items Case");
					if (billable_item_flistp) {
						cookie_rel_bill = NULL;
						rel_bill_id = 0;
                                                while ((rel_bill_flistp = PIN_FLIST_ELEM_GET_NEXT(billable_item_flistp, PIN_FLD_ITEMS,
                                                        &rel_bill_id, 1, &cookie_rel_bill, ebufp)) != (pin_flist_t *)NULL) {
                                                        bill_item_pdp = PIN_FLIST_FLD_GET(rel_bill_flistp,
                                                                PIN_FLD_ITEM_OBJ, 1, ebufp);
                                                        rel_item_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(rel_bill_flistp,
                                                                PIN_FLD_AMOUNT, 1, ebufp);
                                                        if (bill_item_pdp && !PIN_POID_IS_NULL(bill_item_pdp)
                                                                && PIN_POID_COMPARE (item_pdp, bill_item_pdp, 0, ebufp) == 0) {
                                                                billable_item_matched = 1;
                                                                update_item = 1;
                                                                break;
                                                        }
                                                }
                                        }
				}
				if (special_events_in_item > 1) {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Multiple Allocations - Multiple Events Case");
                                        if (billable_item_flistp) {
                                                cookie_rel_bill = NULL;
                                                rel_bill_id = 0;
                                                while ((rel_bill_flistp = PIN_FLIST_ELEM_GET_NEXT(billable_item_flistp, PIN_FLD_ITEMS,
                                                        &rel_bill_id, 1, &cookie_rel_bill, ebufp)) != (pin_flist_t *)NULL) {
                                                        bill_item_pdp = PIN_FLIST_FLD_GET(rel_bill_flistp,
                                                                PIN_FLD_ITEM_OBJ, 1, ebufp);
                                                        rel_item_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(rel_bill_flistp,
                                                                PIN_FLD_AMOUNT, 1, ebufp);
                                                        if (bill_item_pdp && !PIN_POID_IS_NULL(bill_item_pdp)
                                                                && PIN_POID_COMPARE (item_pdp, bill_item_pdp, 0, ebufp) == 0) {
                                                                billable_item_matched = 1;
                                                                update_item = 1;
                                                                break;
                                                        }
                                                }
                                        }
				}
			} else {
				/*************************************************
				* For item level actions we use
				* PIN_FLD_RELATED_BILL_ITEM_OBJ to find the
				* billable item
				**************************************************/
				bill_item_pdp = PIN_FLIST_FLD_GET(other_event_flistp,
						PIN_FLD_RELATED_BILL_ITEM_OBJ, 1, ebufp);

				if (bill_item_pdp && !PIN_POID_IS_NULL(bill_item_pdp)
						&& PIN_POID_COMPARE (item_pdp,
							bill_item_pdp, 0, ebufp) == 0 ) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"Prior Corrections -");
					/***********************************
                                        * In case of partial allocation
                                        * even if only one item array exists
                                        * if its value is different from event
					* take the amount from transfer event
                                        *************************************/
					cookie_rel_bill = NULL;
					if (billable_item_flistp) {
						rel_bill_flistp = PIN_FLIST_ELEM_GET_NEXT(billable_item_flistp,
							PIN_FLD_ITEMS, 0, 1, &cookie_rel_bill, ebufp);
						 rel_item_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(rel_bill_flistp,
                                                                PIN_FLD_AMOUNT, 1, ebufp);

						if (!(pin_decimal_is_ZERO(tsfer_amt,ebufp)) &&
							(oitem_status != PIN_ITEM_STATUS_CLOSED)) {
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
								"Prior Corrections -Partial Allocation");
							partial_allocation = 1;
						}
					}
					update_item = 1;
				}
			}
			/*************************************************
			* For event level actions session obj is used
			*************************************************/
			elemid3 = 0;
			cookie3 = NULL;
			while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(
				item_flistp, PIN_FLD_EVENTS,
					  &elemid3, 1, &cookie3,
						ebufp)) != (pin_flist_t *)NULL) {
				 event_pdp = PIN_FLIST_FLD_GET(event_flistp,
						PIN_FLD_EVENT_OBJ, 1, ebufp);
				if ( PIN_POID_COMPARE (event_pdp, session_pdp, 0,
								ebufp) == 0 ) {
					update_event = 1;
					break;
				}
			}
			if (update_item == 0 && update_event ==0) {
				continue;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                        "fm_inv_pol_prep_invoice_prior_event_item_corrections: MODIFYING other_item_flistp",
                                                other_item_flistp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                        "fm_inv_pol_prep_invoice_prior_event_item_corrections: MODIFYING other_event_flistp",
                                                other_event_flistp);
			 if (multiple_allocations || partial_allocation) {
				if (item_cycle_tax) {
					 /************************************
					* Add the amount also to Item
					**************************************/
					adj_amountp = PIN_FLIST_FLD_TAKE(item_flistp,
						PIN_FLD_REALTIME_TAXED_NET_AMOUNT, 0, ebufp);
					pbo_decimal_add_assign(adj_amountp,
							rel_item_amt, ebufp);
					PIN_FLIST_FLD_PUT(item_flistp,
						PIN_FLD_REALTIME_TAXED_NET_AMOUNT,
							adj_amountp, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"M-Updating item ccycle_tax PIN_FLD_REALTIME_TAXED_NET_AMOUNT");
				} else {
					 /************************************
					* Add the amount also to Item
					**************************************/
					adj_amountp = PIN_FLIST_FLD_TAKE(item_flistp,
						PIN_FLD_OUTSTANDING, 0, ebufp);
					pbo_decimal_add_assign(adj_amountp,
							rel_item_amt, ebufp);
					PIN_FLIST_FLD_PUT(item_flistp,
						PIN_FLD_OUTSTANDING,
							adj_amountp, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"M-Updating PIN_FLD_OUTSTANDING");
				}
			} else {

			   elemid2 = 0;
			   cookie2 = NULL;
			   while ((e_bi_flistp = PIN_FLIST_ELEM_GET_NEXT(
				  other_event_flistp, PIN_FLD_BAL_IMPACTS, 
					&elemid2, 1, &cookie2,
						ebufp)) != (pin_flist_t *)NULL) {
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_inv_pol_prep_invoice_prior_event_item_corrections: e_bi_flistp",
						e_bi_flistp);
				impact_type = 0;
				vp = PIN_FLIST_FLD_GET(e_bi_flistp,
					PIN_FLD_IMPACT_TYPE, 1, ebufp);
				if (vp ) {
					impact_type = *(int32 *)vp;

				}	
				if ((impact_type & PIN_IMPACT_TYPE_TAX) ||
					(impact_type & PIN_IMPACT_TYPE_RERATED_TAX)) {
					temp_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
							e_bi_flistp, PIN_FLD_AMOUNT, 0, ebufp);
					/************************************
					* Add the tax amount to Item
					**************************************/
					rt_tax_total = PIN_FLIST_FLD_TAKE(item_flistp,
						PIN_FLD_REALTIME_TAXED_NET_AMOUNT,
									0, ebufp);
					pbo_decimal_add_assign(rt_tax_total,
							temp_decimal, ebufp);
					PIN_FLIST_FLD_PUT(item_flistp,
						PIN_FLD_REALTIME_TAXED_NET_AMOUNT,
								rt_tax_total, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"Updating PIN_FLD_REALTIME_TAXED_NET_AMOUNT");
					/****************************************
					* Add the tax amount to Event
					****************************************/
					if (update_event) {
						rt_tax_total = PIN_FLIST_FLD_TAKE(event_flistp,
								PIN_FLD_TOTAL_DEBITS,
										0, ebufp);
						pbo_decimal_add_assign(rt_tax_total,
								temp_decimal, ebufp);
						PIN_FLIST_FLD_PUT(event_flistp,
								PIN_FLD_TOTAL_DEBITS,
										rt_tax_total, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
							"Updating PIN_FLD_TOTAL_DEBITS");
					}
				} else if ((impact_type & PIN_IMPACT_TYPE_DISCOUNT) ||
					(impact_type & PIN_IMPACT_TYPE_DISCOUNT_RERATED)) {
					temp_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
							e_bi_flistp, PIN_FLD_AMOUNT, 0, ebufp);
					/************************************
					* Add the discount amount to Item
					**************************************/
					disc_total = PIN_FLIST_FLD_TAKE(item_flistp,
							PIN_FLD_ACTG_REBATES,
									0, ebufp);
					pbo_decimal_add_assign(disc_total,
							temp_decimal, ebufp);
					PIN_FLIST_FLD_PUT(item_flistp,
							PIN_FLD_ACTG_REBATES,
								disc_total, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"Updating PIN_FLD_ACTG_REBATES");
					/************************************
					* Add the discount amount to Event
					**************************************/
					if (update_event) {
						disc_total = PIN_FLIST_FLD_TAKE(event_flistp,
								PIN_FLD_REBATES_ISSUED,
										0, ebufp);
						pbo_decimal_add_assign(disc_total,
								temp_decimal, ebufp);
						PIN_FLIST_FLD_PUT(event_flistp,
							PIN_FLD_REBATES_ISSUED,
								disc_total, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
							"Updating PIN_FLD_REBATES_ISSUED");
					}
				} else {	
					temp_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
							e_bi_flistp, PIN_FLD_AMOUNT, 0, ebufp);
						
					 /************************************
					* Add the amount also to Item
					**************************************/
					adj_amountp = PIN_FLIST_FLD_TAKE(item_flistp,
						PIN_FLD_OUTSTANDING, 0, ebufp);
					pbo_decimal_add_assign(adj_amountp,
								temp_decimal, ebufp);
					PIN_FLIST_FLD_PUT(item_flistp,
							PIN_FLD_OUTSTANDING,
								adj_amountp, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"Updating PIN_FLD_OUTSTANDING");
					/*************************************
					* Add PIN_FLD_DISCOUNT amount also 
					*************************************/
					temp_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
							e_bi_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
					disc_total = PIN_FLIST_FLD_TAKE(item_flistp,
							PIN_FLD_ACTG_REBATES,
								0, ebufp);
					pbo_decimal_add_assign(disc_total,
							temp_decimal, ebufp);
					PIN_FLIST_FLD_PUT(item_flistp,
							PIN_FLD_ACTG_REBATES,
							disc_total, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"Updating PIN_FLD_ACTG_REBATES");
					 /************************************
					* Add the amount to Event
					**************************************/
					if (update_event) {
						temp_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
								e_bi_flistp, PIN_FLD_AMOUNT, 0, ebufp);
						adj_amountp = PIN_FLIST_FLD_TAKE(event_flistp,
								PIN_FLD_TOTAL_CREDITS, 0, ebufp);
						pbo_decimal_add_assign(adj_amountp,
									temp_decimal, ebufp);
						PIN_FLIST_FLD_PUT(event_flistp,
								PIN_FLD_TOTAL_CREDITS,
										adj_amountp, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
							"Updating PIN_FLD_TOTAL_CREDITS");
						/*************************************
						* Add PIN_FLD_DISCOUNT amount also 
						*************************************/
						temp_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(
									e_bi_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
						disc_total = PIN_FLIST_FLD_TAKE(event_flistp,
								PIN_FLD_REBATES_ISSUED, 0, ebufp);
						pbo_decimal_add_assign(disc_total,
									temp_decimal, ebufp);
						PIN_FLIST_FLD_PUT(event_flistp,
								PIN_FLD_REBATES_ISSUED,
										disc_total, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
							"Updating PIN_FLD_REBATES_ISSUED");
					}
				}
			   } /* while Bal_impacts */
			}
		}
	}
	pbo_decimal_destroy(&zero_dec);

	/***********************************************************
         * error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
             "fm_inv_pol_prep_invoice_prior_event_item_corrections: error ",
             ebufp);

        } else {
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_inv_pol_prep_invoice_prior_event_item_corrections: output flist",
            out_flistp);
        }
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_drop_uncorrected_events_items():
 *
 * Function to drop events and items that did not change.
 * This is done for Correction Letter type of Corrective Invoices.
 * For replacement invoice this is not called since we keep all 
 * events and items whether changed or not.
 *
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_drop_uncorrected_events_items(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t             *item_flistp = NULL;
	pin_cookie_t            cookie = NULL;
	pin_cookie_t            prev_cookie = NULL;
	int32                   elemid = 0;
	pin_decimal_t		*adj_amountp = (pin_decimal_t *)NULL;
	void                    *vp = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_drop_uncorrected_events_items: input flist",
		out_flistp);

	/* Drop all the items that did not have a correction. Need to iterate
	 * only the AR and SUB items, the OTHER items don't have corrections.
	 */
	while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, 
				PIN_FLD_AR_ITEMS, &elemid, 1, 
				&cookie, ebufp)) != (pin_flist_t *)NULL) {

		adj_amountp = (pin_decimal_t *)PIN_FLIST_FLD_GET(item_flistp, 
					PIN_FLD_DELTA_DUE, 1, ebufp);

		if (adj_amountp && pbo_decimal_is_zero(adj_amountp, ebufp) ) {
			PIN_FLIST_ELEM_DROP(out_flistp, PIN_FLD_AR_ITEMS, 
					elemid, ebufp);
			cookie = prev_cookie;
		} else {
			/* This item had some correction. Keep only the
			 * events that were corrected and drop the events
			 * that were not changed.
			 */
			fm_inv_pol_prep_invoice_drop_uncorrected_events(ctxp,
				item_flistp, ebufp);
		}

		prev_cookie = cookie;
	}

	/* Now do the same thing for SUB ITEMS */
	cookie = NULL;
	prev_cookie = NULL;
	elemid = 0;
	while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, 
				PIN_FLD_SUB_ITEMS, &elemid, 1, 
				&cookie, ebufp)) != (pin_flist_t *)NULL) {

		adj_amountp = (pin_decimal_t *)PIN_FLIST_FLD_GET(item_flistp, 
					PIN_FLD_DELTA_DUE, 1, ebufp);

		if (adj_amountp && pbo_decimal_is_zero(adj_amountp, ebufp) ) {
			PIN_FLIST_ELEM_DROP(out_flistp, PIN_FLD_SUB_ITEMS, 
					elemid, ebufp);
			cookie = prev_cookie;
		} else {
			/* This item had some correction. Keep only the
			 * events that were corrected and drop the events
			 * that were not changed.
			 */
			fm_inv_pol_prep_invoice_drop_uncorrected_events(ctxp,
				item_flistp, ebufp);

		}

		prev_cookie = cookie;
	}


	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_drop_uncorrected_events_items: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_drop_uncorrected_events_items: output flist",
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_drop_uncorrected_events():
 *
 * Function to drop events that did not change.
 * This is done for Correction Letter type of Corrective Invoices.
 * For replacement invoice this is not called since we keep all 
 * events and items whether changed or not.
 *
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_drop_uncorrected_events(
	pcm_context_t	*ctxp,
	pin_flist_t	*item_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t             *event_flistp = NULL;
	pin_cookie_t            cookie = NULL;
	pin_cookie_t            prev_cookie = NULL;
	int32                   elemid = 0;
	pin_decimal_t		*adj_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t		*dis_amountp = (pin_decimal_t *)NULL;
	void                    *vp = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_drop_uncorrected_events: input flist",
		item_flistp);

	/* Drop all the events that did not have a correction. 
	 */
	while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp, 
				PIN_FLD_EVENTS, &elemid, 1, &cookie, 
				ebufp)) != (pin_flist_t *)NULL) {

		adj_amountp = (pin_decimal_t *)PIN_FLIST_FLD_GET(event_flistp, 
					PIN_FLD_AMOUNT_ADJUSTED, 1, ebufp);
		dis_amountp = (pin_decimal_t *)PIN_FLIST_FLD_GET(event_flistp, 
					PIN_FLD_DISCOUNT_VALUE, 1, ebufp);

		if (adj_amountp && dis_amountp && 
			pbo_decimal_is_zero(adj_amountp, ebufp) &&
			pbo_decimal_is_zero(dis_amountp, ebufp)) {
			PIN_FLIST_ELEM_DROP(item_flistp, PIN_FLD_EVENTS, 
					elemid, ebufp);
			cookie = prev_cookie;
		}
		prev_cookie = cookie;
	}

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_drop_uncorrected_events: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_drop_uncorrected_events: output flist",
			item_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_calculate_delta_amounts_bill():
 *
 * Function to compute the delta amount for the bill level
 * 
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_calculate_delta_amounts_bill(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{
	void		*vp = NULL;
	int32		elemid = 0;
	pin_cookie_t	cookie = NULL;
	pin_flist_t	*bi_flistp = NULL;
	pin_flist_t	*read_evt_flistp = NULL;
	pin_flist_t	*ci_evt_flistp = NULL;
	pin_flist_t	*cb_flistp = NULL;
	pin_flist_t	*pbi_flistp = NULL;
	pin_flist_t	*ecb_flistp = NULL;
    	poid_t		*ci_event_pdp = NULL;
    	poid_t		*parent_pdp = NULL;
	pin_decimal_t   *orig_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   *curr_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   *delta_amountp = (pin_decimal_t *)NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_calculate_delta_amounts_bill: input flist",
		out_flistp);


	/*
	 * Get the bill level delta amount
	 * The original amount is from /event/billing/corrective_invoice event
	 * This will be put in 
	 * PIN_FLD_EVENTS.PIN_FLD_EVENT_CORRECTIVE_BILL.PIN_FLD_AMOUNT_ORIG
	 * The delta can be calculated from AMOUNT_ORIG and TOTAL_DUE
	 * 
	 * Also some other information about the previous bill will be put
	 * in PIN_FLD_PREV_BILLINFO and PIN_FLD_BILLINFO
	 */
	elemid = 0;
	cookie = NULL;
	bi_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, PIN_FLD_BILLINFO, 
					&elemid, 0, &cookie,  ebufp);

	elemid = 0;
	cookie = NULL;
	pbi_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, PIN_FLD_PREV_BILLINFO, 
					&elemid, 0, &cookie,  ebufp);
	elemid = 0;
	cookie = NULL;
	ecb_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, 
					PIN_FLD_EVENTS, 
					&elemid, 0, &cookie,  ebufp);

	cb_flistp = PIN_FLIST_SUBSTR_GET(ecb_flistp, 
					PIN_FLD_EVENT_CORRECTIVE_BILL, 
					0, ebufp);

	/* Store info in the PIN_FLD_PREV_BILLINFO */

	vp = PIN_FLIST_FLD_GET(cb_flistp, PIN_FLD_ORIG_NUM, 0, ebufp);		
	PIN_FLIST_FLD_SET(pbi_flistp, PIN_FLD_ORIG_NUM, vp, ebufp);

	/*********************************************************
	* In case of sub-ord account we donot have corrective bill
	* event the AMOUNT_ORIG will be passed in the input
	* under PREV_BILLINFO
	* FYI:For child account parent's corrective bill event is passed
	* in the PIN_FLD_EVENTS as corrective bill event is not
	* generated for child
	**********************************************************/
	parent_pdp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_PARENT, 0,
					ebufp);
	if (PIN_POID_IS_NULL(parent_pdp)) {
		/*** Not a child account ***/
		vp = PIN_FLIST_FLD_GET(cb_flistp, PIN_FLD_AMOUNT_ORIG, 0, ebufp);
		PIN_FLIST_FLD_SET(pbi_flistp, PIN_FLD_AMOUNT_ORIG, vp, ebufp);
	}

	/* Store info in the PIN_FLD_BILLINFO */

	vp = PIN_FLIST_FLD_GET(cb_flistp, PIN_FLD_FLAGS, 0, ebufp);		
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_FLAGS, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(cb_flistp, PIN_FLD_INV_TYPE, 0, ebufp);		
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_INV_TYPE, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(ecb_flistp, PIN_FLD_END_T, 0, ebufp);		
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_EFFECTIVE_END_T, vp, ebufp);

	/* Calculate delta and store in BILLINFO */
	/********************************************************
	* For sub-ord acounts this is passed under PREV_BILLINFO 
	* in all other cases we take it from corrective bill event 
	* The PIN_FLD_AMOUNT_ORIG will always there in pbi_flistp
	***********************************************************/	
	orig_amountp = PIN_FLIST_FLD_GET(pbi_flistp, PIN_FLD_AMOUNT_ORIG, 
					0, ebufp);
	curr_amountp =  PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_CURRENT_TOTAL,
					0, ebufp);
	delta_amountp = pbo_decimal_subtract(curr_amountp, orig_amountp, ebufp);

	PIN_FLIST_FLD_PUT(bi_flistp, PIN_FLD_AMOUNT_ADJUSTED, 
					delta_amountp, ebufp);

	/* Cleanup */
	PIN_FLIST_DESTROY_EX(&read_evt_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ci_evt_flistp, NULL);

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_calculate_delta_amounts_bill: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_calculate_delta_amounts_bill: output flist",
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_calculate_delta_amounts_items():
 *
 * Function to compute the delta amounts for items
 * that were changed, resulting in a corrective invoice
 * 
 * This function will add the following three fields to each item.
 *
 * PIN_FLD_PREVIOUS_TOTAL
 * PIN_FLD_DELTA_DUE
 * PIN_FLD_CURRENT_TOTAL
 *
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_calculate_delta_amounts_items(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	int32		fldno,
	pin_errbuf_t	*ebufp)
{
	void		*vp = NULL;

	int32		elemid = 0;
	int32		elemid1 = 0;
	int32		elemid2 = 0;
	int32		allocation = 1;

	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie1 = NULL;
	pin_cookie_t	cookie2 = NULL;

	pin_flist_t	*bi_flistp = NULL;
	pin_flist_t	*ci_flistp = NULL;
	pin_flist_t	*read_evt_flistp = NULL;
	pin_flist_t	*ci_evt_flistp = NULL;
	pin_flist_t	*cb_flistp = NULL;
	pin_flist_t	*pbi_flistp = NULL;
	pin_flist_t	*item1_flistp = NULL;
	pin_flist_t	*item2_flistp = NULL;

    	poid_t		*ci_event_pdp = NULL;
    	poid_t		*item1_pdp = NULL;
    	poid_t		*item2_pdp = NULL;

	pin_decimal_t   *zero_dec = (pin_decimal_t *)NULL;
	pin_decimal_t   *orig_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   *curr_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   *delta_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   *adj_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   *prev_amountp = (pin_decimal_t *)NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_calculate_delta_amounts_items: input flist",
		out_flistp);


	zero_dec = pbo_decimal_from_str("0.0", ebufp);

	elemid = 0;
	cookie = NULL;
	bi_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, PIN_FLD_BILLINFO, 
					&elemid, 0, &cookie,  ebufp);

	pbi_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, PIN_FLD_PREV_BILLINFO, 
					&elemid, 0, &cookie,  ebufp);

	/* 
	 * Get the item level delta amounts
	 *
	 * Get the history_bills object details from PREV_BILLINFO
	 * Iterate throught the hb.PIN_FLD_ITEMS
	 * get PIN_FLD_DELTA_DUE
	 * Set the following three fields in the appropriate
	 * PIN_FLD_AR_ITEMS or PIN_FLD_SUB_ITEMS
	 *    PIN_FLD_PREVIOUS_TOTAL = PIN_FLD_ITEM_TOTAL - hb.PIN_FLD_DELTA_DUE
	 *    PIN_FLD_DELTA_DUE = hb.PIN_FLD_DELTA_DUE
	 *    PIN_FLD_CURRENT_TOTAL = PIN_FLD_ITEM_TOTAL
	 */

	elemid1 = 0;
	cookie1 = NULL;	
	while ((item1_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, 
				fldno, &elemid1, 1, &cookie1, 
				ebufp)) != (pin_flist_t *)NULL) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"item1_flistp: ", item1_flistp);
		
		/* initialize the totals 
		 * DELTA_DUE = 0.0 
		 * PREVIOUS_TOTAL = ITEM_TOTAL
		 * CURRENT_TOTAL = ITEM_TOTAL
		 */

		PIN_FLIST_FLD_SET(item1_flistp, PIN_FLD_DELTA_DUE,
                                        	(void *)zero_dec, ebufp)
		curr_amountp = PIN_FLIST_FLD_GET(item1_flistp, 
						PIN_FLD_DUE, 0, ebufp);
		PIN_FLIST_FLD_SET(item1_flistp, PIN_FLD_PREVIOUS_TOTAL,
                                        	curr_amountp, ebufp)
		PIN_FLIST_FLD_SET(item1_flistp, PIN_FLD_CURRENT_TOTAL,
                                        	curr_amountp, ebufp)

		item1_pdp = PIN_FLIST_FLD_GET(item1_flistp, 
						PIN_FLD_ITEM_OBJ, 0, ebufp);
		allocation = 1;
		elemid2 = 0;
		cookie2 = NULL;	
		while ((item2_flistp = PIN_FLIST_ELEM_GET_NEXT(pbi_flistp, 
				PIN_FLD_ITEMS, &elemid2, 1, &cookie2, 
				ebufp)) != (pin_flist_t *)NULL) {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"item2_flistp: ", item2_flistp);
			item2_pdp = PIN_FLIST_FLD_GET(item2_flistp, 
						PIN_FLD_ITEM_OBJ, 0, ebufp);

			if ( PIN_POID_COMPARE (item1_pdp, item2_pdp, 0,
						ebufp) == 0 ) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
					"history_bills->item matched");
				/**************************************************
				* If the item is created and the amount is 
				* unallocated in previous bill cycles and now
				* the item is partially or fully allocated
				* then we should not consider as the correction
				* fm_inv_pol_prep_invoice_check_for_only_allocations
				* returns 0 if "no items created but just allocated"
				* returns 1 if "there are items and allocations"
				*
				* Only allocation is not considered as correction
				****************************************************/
				allocation = fm_inv_pol_prep_invoice_check_for_only_allocations(ctxp,
					out_flistp, item1_pdp, ebufp);		

				if (!allocation) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"Only Allocation - items are not created in this period");
					break;
				}
	
				delta_amountp = PIN_FLIST_FLD_GET(item2_flistp, 
						PIN_FLD_DELTA_DUE, 0, ebufp);
                                

				/* Add this delta to DELTA_DUE
				 * and subtract from PREVIOUS_TOTAL
				 */
				adj_amountp = PIN_FLIST_FLD_TAKE(item1_flistp,
						PIN_FLD_DELTA_DUE,
						0, ebufp);
				/*********
				prev_amountp = PIN_FLIST_FLD_TAKE(item1_flistp,
						PIN_FLD_PREVIOUS_TOTAL,
						0, ebufp);
				*************/

				pbo_decimal_add_assign(adj_amountp,
						delta_amountp, ebufp);
				/**************
				pbo_decimal_add_assign(prev_amountp,
						delta_amountp, ebufp);
				**************/
				PIN_FLIST_FLD_PUT(item1_flistp,
						PIN_FLD_DELTA_DUE,
						adj_amountp, ebufp)
				PIN_FLIST_FLD_SET(item1_flistp,
						PIN_FLD_CURRENT_TOTAL,
						curr_amountp, ebufp)
				/**********
				PIN_FLIST_FLD_PUT(item1_flistp,
						PIN_FLD_PREVIOUS_TOTAL,
						prev_amountp, ebufp)
				************/
			}
		}
	}
		
	/* Cleanup */
	pbo_decimal_destroy(&zero_dec);

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_calculate_delta_amounts_items: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_calculate_delta_amounts_items: output flist",
			out_flistp);
	}
}

/******************************************************
 * fm_inv_pol_prep_invoice_check_for_only_allocations()
 * checks if billable item in correction_info->other_items
 * matches with the given item_poid. If match found 
 * that means its NOT just only allocation.
 * returns 0 if "no allocations"
 * returns 1 if "there are allocations"
 ******************************************************/
static int
fm_inv_pol_prep_invoice_check_for_only_allocations(
		pcm_context_t	*ctxp,
		pin_flist_t	*out_flistp, 
		poid_t		*item1_pdp, 
		pin_errbuf_t	*ebufp)
{
	
	pin_flist_t	*items_flistp = NULL;
	pin_flist_t	*mitems_flistp = NULL;
	pin_flist_t	*events_flistp = NULL;
	pin_flist_t	*corr_info_flistp = NULL;
	pin_flist_t	*other_items_flistp = NULL;

	pin_cookie_t	cookie1 = NULL;
	pin_cookie_t	cookie2 = NULL;
	pin_cookie_t	cookie3 = NULL;
	pin_cookie_t	cookie4 = NULL;

	poid_t		*item_pdp = NULL;
	poid_t		*other_item_pdp = NULL;
	poid_t		*etr_item_pdp = NULL;

	int32		elemid1 = 0;
	int32		elemid2 = 0;
	int32		elemid3 = 0;
	int32		elemid4 = 0;
	int32		found = 0;
	int32		other_item_cnt = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return 0;
	PIN_ERR_CLEAR_ERR(ebufp);

	elemid1 = 0;
        cookie1 = NULL;
        while (!found && (corr_info_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
			PIN_FLD_CORRECTION_INFO, &elemid1, 1, &cookie1,
                                ebufp)) != (pin_flist_t *)NULL) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"1_corr_info_flistp", corr_info_flistp);
		other_item_cnt = PIN_FLIST_ELEM_COUNT(corr_info_flistp,
			PIN_FLD_OTHER_ITEMS, ebufp);	
		if (other_item_cnt == 0) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
				"No OTHER_ITEMS  in CORRECTION_INFO - Only Allocation");
			break;
		}
		elemid2 = 0;
		cookie2 = NULL;
		while (!found && (other_items_flistp = PIN_FLIST_ELEM_GET_NEXT(corr_info_flistp,
				PIN_FLD_OTHER_ITEMS, &elemid2, 1, &cookie2,
					ebufp)) != (pin_flist_t *)NULL) {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"1_other_items_flistp", other_items_flistp);
			other_item_pdp = PIN_FLIST_FLD_GET(other_items_flistp,
                                PIN_FLD_ITEM_OBJ, 0, ebufp);

			elemid3 = 0;
			cookie3 = NULL;
			while (!found && (events_flistp = PIN_FLIST_ELEM_GET_NEXT(corr_info_flistp,
					PIN_FLD_EVENTS, &elemid3, 1, &cookie3,
						ebufp)) != (pin_flist_t *)NULL) {
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"events_flistp", events_flistp);
				etr_item_pdp = PIN_FLIST_FLD_GET(events_flistp,
					PIN_FLD_ITEM_OBJ, 0, ebufp);
				elemid4 = 0;
				cookie4 = NULL;
				mitems_flistp = PIN_FLIST_ELEM_GET(events_flistp,
					PIN_FLD_ITEMS, PIN_ELEMID_ANY, 1, ebufp);

				if (PIN_POID_COMPARE (etr_item_pdp, other_item_pdp,
					0, ebufp) == 0) {
					/*** Matched Special Item ****/

					while ((items_flistp = PIN_FLIST_ELEM_GET_NEXT(mitems_flistp,
						PIN_FLD_ITEMS, &elemid4, 1, &cookie4,
							ebufp)) != (pin_flist_t *)NULL) {
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
							"1_items_flistp", items_flistp);
						item_pdp = PIN_FLIST_FLD_GET(items_flistp,
							PIN_FLD_ITEM_OBJ, 0, ebufp);
						if ( PIN_POID_COMPARE (item1_pdp, item_pdp, 0,
							ebufp) == 0 ) {
							/**** Matched Billable Item ***/
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
								"Not Only Allocation");
							found = 1;
							break;
						}
					}
				}
			}
			
		}
	}
	if (!found) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"Only Allocation");
	}
	return found;
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_calculate_delta_amounts_events():
 *
 * Function to compute the delta amounts for events
 * that were changed, resulting in a corrective invoice
 * 
 * This function will add the following three fields to each event
 *
 * PIN_FLD_PREVIOUS_TOTAL
 * PIN_FLD_DELTA_DUE
 * PIN_FLD_CURRENT_TOTAL
 *
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_calculate_delta_amounts_events(
	pcm_context_t	*ctxp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{
	void		*vp = NULL;
	int32		elemid_i = 0;
	int32		elemid_e = 0;
	pin_cookie_t	cookie_i = NULL;
	pin_cookie_t	cookie_e = NULL;
	pin_flist_t	*ci_flistp = NULL;
	pin_flist_t	*item_flistp = NULL;
	pin_flist_t	*event_flistp = NULL;
	pin_decimal_t   *zero_dec = (pin_decimal_t *)NULL;
	int32		add_corr_to_item = 1;
	int32		event_cnt = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_calculate_delta_amounts_events: input flist",
		out_flistp);

	zero_dec = pbo_decimal_from_str("0.0", ebufp);

	ci_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_CORRECTION_INFO, 
					PIN_ELEMID_ANY, 1, ebufp);

	/* 
	 * Get the event level delta amounts
	 * Iterate through the AR_ITEMS and SUB_TEMS
	 *     Iterate through EVENTS and call the function
	 *     to get the deltas for that event from the corresponding
	 *     event adjsutment event.
	 */
	/* Process the AR_ITEMS */
	elemid_i = 0;
	cookie_i = NULL;
	while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, 
				PIN_FLD_AR_ITEMS, &elemid_i, 1, &cookie_i, 
				ebufp)) != (pin_flist_t *)NULL) {
		/**** Prior Corrections *****/
		PIN_FLIST_FLD_SET(item_flistp,  PIN_FLD_OUTSTANDING,
                                                (void *)zero_dec, ebufp);
		PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_REALTIME_TAXED_NET_AMOUNT,
                                                (void *)zero_dec, ebufp);
		PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_ACTG_REBATES,
                                                (void *)zero_dec, ebufp);

		/**** Current Corrections *****/
		PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_DELAYED_BAL,
                                                (void *)zero_dec, ebufp);
		PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_NEXT_BAL,
                                                (void *)zero_dec, ebufp);
		PIN_FLIST_FLD_SET(item_flistp,  PIN_FLD_CURRENT_BAL,
                                                (void *)zero_dec, ebufp);
		event_cnt = PIN_FLIST_ELEM_COUNT(item_flistp, PIN_FLD_EVENTS,
			 ebufp);
		if (event_cnt) {
			elemid_e = 0;
			cookie_e = NULL;
			add_corr_to_item = 1;
			while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp,
				PIN_FLD_EVENTS, &elemid_e, 1,
				&cookie_e, ebufp)) != (pin_flist_t *)NULL) {

				fm_inv_pol_prep_invoice_delta_process_adj_events (
					ctxp, add_corr_to_item, out_flistp, event_flistp, 
						item_flistp, ci_flistp, ebufp);	
				add_corr_to_item = 0;
			}
		}
		fm_inv_pol_prep_invoice_prior_event_item_corrections(ctxp, 
			item_flistp, out_flistp, ebufp);
	}

	/* Process the SUB_ITEMS */
	add_corr_to_item = 1;
	elemid_i = 0;
	cookie_i = NULL;
	while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp, 
				PIN_FLD_SUB_ITEMS, &elemid_i, 1, &cookie_i, 
				ebufp)) != (pin_flist_t *)NULL) {
		/**** Prior Corrections *****/
		PIN_FLIST_FLD_SET(item_flistp,  PIN_FLD_OUTSTANDING,
                                                (void *)zero_dec, ebufp);
		PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_REALTIME_TAXED_NET_AMOUNT,
                                                (void *)zero_dec, ebufp);
		PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_ACTG_REBATES,
                                                (void *)zero_dec, ebufp);

		/**** Current Corrections *****/
		PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_DELAYED_BAL,
                                                (void *)zero_dec, ebufp);
		PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_NEXT_BAL,
                                                (void *)zero_dec, ebufp);
		PIN_FLIST_FLD_SET(item_flistp,  PIN_FLD_CURRENT_BAL,
                                                (void *)zero_dec, ebufp);
		event_cnt = PIN_FLIST_ELEM_COUNT(item_flistp, PIN_FLD_EVENTS,
			 ebufp);
		if (event_cnt) {
			elemid_e = 0;
			cookie_e = NULL;
			add_corr_to_item = 1;
			while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp,
				PIN_FLD_EVENTS, &elemid_e, 1,
				&cookie_e, ebufp)) != (pin_flist_t *)NULL) {

				fm_inv_pol_prep_invoice_delta_process_adj_events (
					ctxp, add_corr_to_item, out_flistp, event_flistp, 
						item_flistp, ci_flistp, ebufp);	
				add_corr_to_item = 0;
			}
		}
		fm_inv_pol_prep_invoice_prior_event_item_corrections(ctxp, 
				item_flistp, out_flistp, ebufp);
	}
	/**********************************************************************
	* This function will get the actual corrections SETTLEMENT[] and
	* DISPUTES [] after drop already shown and which belong to different
	* bills. Commenting for now as the request is to get all corrections
	* data before dropping which we compute in the make_invoice.
	***********************************************************************/

	/* Cleanup */
	pbo_decimal_destroy(&zero_dec);

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_calculate_delta_amounts_events: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_calculate_delta_amounts_events: output flist",
			out_flistp);
	}
}

/*******************************************************************
 * fm_inv_pol_prep_invoice_delta_process_adj_events():
 *
 * Function to compute the delta amounts for events
 * that were changed, resulting in a corrective invoice
 * 
 * This function will add the following three fields to each event
 *
 * PIN_FLD_PREVIOUS_TOTAL
 * PIN_FLD_DELTA_DUE
 * PIN_FLD_CURRENT_TOTAL
 *
 *******************************************************************/

static void
fm_inv_pol_prep_invoice_delta_process_adj_events(
	pcm_context_t	*ctxp,
	int32		add_corr_to_item,
	pin_flist_t	*out_flistp,
        pin_flist_t	*event_flistp,
        pin_flist_t	*item_flistp,
        pin_flist_t	*ci_flistp,
	pin_errbuf_t	*ebufp)
{
	int32		elemid = 0;
	int32		elemid1 = 0;
	int32		elemid2 = 0;
	int32		elemid3 = 0;
	int32		elemid4 = 0;
	int32		elemidd = 0;
	int32		refresh_elem = 0;
	int32		ebit_rec_id = 0;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie1 = NULL;
	pin_cookie_t	cookie2 = NULL;
	pin_cookie_t	cookie3 = NULL;
	pin_cookie_t	cookie4 = NULL;
	pin_cookie_t	cookied = NULL;
	pin_cookie_t	ebit_cookie = NULL;
	pin_cookie_t	refresh_cookie = NULL;
	pin_cookie_t	cookie_evt = NULL;
	pin_flist_t	*sp_evt_flistp = NULL;
	pin_flist_t	*ci_item_flistp = NULL;
	pin_flist_t	*ci_event_flistp = NULL;
	pin_flist_t	*cb_flistp = NULL;
	pin_flist_t	*total_flistp = NULL;
	pin_flist_t	*balimp_flistp = NULL;
	pin_flist_t	*e_bi_flistp = NULL;
	pin_flist_t	*disp_flistp = NULL;
	pin_flist_t	*disputes_flistp = NULL;
	pin_flist_t	*e_disp_flistp = NULL;
	pin_flist_t	*e_tr_flistp = NULL;
	pin_flist_t	*ebit_flistp = NULL;
	pin_flist_t	*trans_flistp = NULL;
	pin_flist_t	*temp_flistp = NULL;
	pin_flist_t	*ebit_pdp_flistp = NULL;
	pin_flist_t	*corr_e_flistp = NULL;
	pin_flist_t	*ecb_flistp = NULL;
	pin_flist_t	*rel_bill_flistp = NULL;
	pin_flist_t	*billable_item_flistp = NULL;

	poid_list_t	*event_poid_listp = NULL;
	pin_cookie_t	cookie_rel_bill = NULL;

    	poid_t		*event_pdp = NULL;
    	poid_t		*res_item_pdp = NULL;
    	poid_t		*ebit_pdp = NULL;
    	poid_t		*ci_event_pdp = NULL;
    	poid_t		*ci_event_item_pdp = NULL;
    	poid_t		*item_pdp = NULL;
    	poid_t		*oitem_pdp = NULL;
    	poid_t		*session_pdp = NULL;
    	poid_t		*sess_pdp = NULL;
    	poid_t		*disp_e_pdp = NULL;
    	poid_t		*disp_i_pdp = NULL;
    	poid_t		*disp_pdp = NULL;
    	poid_t		*etr_pdp = NULL;
    	poid_t		*etr_e_pdp = NULL;
    	poid_t		*ci_item_pdp = NULL;
    	poid_t		*rel_bill_i_pdp = NULL;
	pin_decimal_t   *curr_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   *delta_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   *adj_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   *zero_dec = (pin_decimal_t *)NULL;
	pin_decimal_t   *rt_tax_total = (pin_decimal_t *)NULL;
        pin_decimal_t   *disc_total = (pin_decimal_t *)NULL;
        pin_decimal_t   *temp_decimal = (pin_decimal_t *)NULL;
	pin_decimal_t   *disc_amountp = (pin_decimal_t *)NULL;
	pin_decimal_t   *rel_item_amt = (pin_decimal_t *)NULL;
	pin_decimal_t   *tsfer_amt = (pin_decimal_t *)NULL;
        int32           impact_type = 0;
        int32           update_event = 0;
        int32           update_item = 0;
        int32           res_id = 0;
        int32           rel_bill_id = 0;
	int32		match_found = PIN_BOOLEAN_TRUE;
	int32		match_not_found = PIN_BOOLEAN_TRUE;
	int32		billable_item_matched = 1;
	int32		billable_item_count = 1;
	int32		multiple_allocations = 1;
	int32		partial_allocation = 0;
	int32		special_events_in_item = 1;
	int32		special_events_cnt = 1;
	int32		item_cycle_tax = 0;
	int32		oitem_status = 0;
	int32		evt_id = 0;
	int32		found_diff_bill_item = 0;
        void            *vp = NULL;
	char		*ci_event_type = NULL;
	char		*disp_type = NULL;
	char		*session_type = NULL;
	char		*e_type = NULL;
	char		*item_type = NULL;

	time_t		corr_start_t = 0;
	time_t		e_tr_end_t = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_delta_process_adj_events: out_flist",
		out_flistp);

	zero_dec = pbo_decimal_from_str("0.0", ebufp);
		
	/*
	 * In case of AR_ITEMS has Item mis event_flistp is NULL
	 * Init the following three fields in the PIN_FLD_EVENT passed in.
	 *    PIN_FLD_AMOUNT_ADJUSTED = 0
	 *    PIN_FLD_PREVIOUS_TOTAL = PIN_FLD_TOTAL.PIN_FLD_AMOUNT
	 *    PIN_FLD_CURRENT_TOTAL = PIN_FLD_TOTAL.PIN_FLD_AMOUNT
	 */
	if (event_flistp) {
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_AMOUNT_ADJUSTED,
						(void *)zero_dec, ebufp)
		total_flistp = PIN_FLIST_ELEM_GET(event_flistp, PIN_FLD_TOTAL, 
						PIN_ELEMID_ANY, 1, ebufp);
		curr_amountp = PIN_FLIST_FLD_GET(total_flistp, 
						PIN_FLD_AMOUNT, 0, ebufp);
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_PREVIOUS_TOTAL,
                                        	curr_amountp, ebufp)
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_CURRENT_TOTAL,
                                        	curr_amountp, ebufp)

		 /**************************************************************
		*  Some existing fields are reused some made sense some didn't
		***************************************************************/
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_AMOUNT_ADJUSTED,
                                                (void *)zero_dec, ebufp)
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_AMOUNT_TAX_ADJUSTED,
                                                (void *)zero_dec, ebufp)
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_DISCOUNT_VALUE,
                                                (void *)zero_dec, ebufp)

		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_OLD_RETAIL_VALUE,
                                                (void *)zero_dec, ebufp)
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_REALTIME_TAX_AMOUNT,
                                                (void *)zero_dec, ebufp)
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_OLD_DISCOUNT_VALUE,
                                                (void *)zero_dec, ebufp)
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_TOTAL_CREDITS,
						(void *)zero_dec, ebufp);
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_TOTAL_DEBITS,
						(void *)zero_dec, ebufp);
		PIN_FLIST_FLD_SET(event_flistp,  PIN_FLD_REBATES_ISSUED,
						(void *)zero_dec, ebufp);

		event_pdp = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_EVENT_OBJ, 
						0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_delta_process_adj_events: event_flist",
			event_flistp);

	   elemid1 = 0;
           cookie1 = NULL;
           while ((event_flistp) &&
                (e_bi_flistp = PIN_FLIST_ELEM_GET_NEXT(event_flistp,
                                PIN_FLD_BAL_IMPACTS, &elemid1, 1, &cookie1,
                                ebufp)) != (pin_flist_t *)NULL) {
		vp = PIN_FLIST_FLD_GET(e_bi_flistp,
			PIN_FLD_RESOURCE_ID, 1, ebufp);
		if (vp) {
			res_id = *(int32 *)vp;
		   } else res_id = 0;

		if (!PIN_BEID_IS_CURRENCY(res_id)) {
			continue;
		   }
                impact_type = 0;
                vp = PIN_FLIST_FLD_GET(e_bi_flistp,
                        PIN_FLD_IMPACT_TYPE, 1, ebufp);
                if (vp ) {
                        impact_type = *(int32 *)vp;

                }

                /*********************************************************
                *                Prior Event Totals
                * For each event we are adding  the following:
                * PIN_FLD_REALTIME_TAX_AMOUNT for tax
                * PIN_FLD_OLD_DISCOUNT_VALUE for discount
                * PIN_FLD_OLD_RETAIL_VALUE for remaining amount
                *
                *********************************************************/
		if ((impact_type & PIN_IMPACT_TYPE_TAX) ||
                        (impact_type & PIN_IMPACT_TYPE_RERATED_TAX)) {
                        /* There is a tax balimpact - add this */
                        temp_decimal = (pin_decimal_t *)PIN_FLIST_FLD_GET(e_bi_flistp,
                        PIN_FLD_AMOUNT, 0, ebufp);
                        rt_tax_total = PIN_FLIST_FLD_TAKE(event_flistp,
                                PIN_FLD_REALTIME_TAX_AMOUNT,
                                0, ebufp);
                        pbo_decimal_add_assign(rt_tax_total,
                                temp_decimal, ebufp);
                        PIN_FLIST_FLD_PUT(event_flistp,
                                PIN_FLD_REALTIME_TAX_AMOUNT,
                                rt_tax_total, ebufp);
                } else if ((impact_type & PIN_IMPACT_TYPE_DISCOUNT) ||
                        (impact_type & PIN_IMPACT_TYPE_DISCOUNT_RERATED)) {
                        temp_decimal = (pin_decimal_t *)
                                PIN_FLIST_FLD_GET(e_bi_flistp,
                                PIN_FLD_AMOUNT, 0, ebufp);
                        disc_total = PIN_FLIST_FLD_TAKE(event_flistp,
                                PIN_FLD_OLD_DISCOUNT_VALUE,
                                0, ebufp);
                        pbo_decimal_add_assign(disc_total,
                                temp_decimal, ebufp);
                        PIN_FLIST_FLD_PUT(event_flistp,
                                PIN_FLD_OLD_DISCOUNT_VALUE,
                                disc_total, ebufp);
                } else {
			delta_amountp = PIN_FLIST_FLD_GET(e_bi_flistp,
                                PIN_FLD_AMOUNT, 0, ebufp);

                        /* Add this delta to AMOUNT_ADJUSTED
                         * and CURRENT_TOTAL
                         */
                        adj_amountp = PIN_FLIST_FLD_TAKE(event_flistp,
                                PIN_FLD_OLD_RETAIL_VALUE,
                                0, ebufp);
                        curr_amountp = PIN_FLIST_FLD_TAKE(event_flistp,
                                                PIN_FLD_CURRENT_TOTAL,
                                                0, ebufp);
                        pbo_decimal_add_assign(adj_amountp,
                                                delta_amountp, ebufp);
                        pbo_decimal_add_assign(curr_amountp,
                                                delta_amountp, ebufp);

                        PIN_FLIST_FLD_PUT(event_flistp,
                                PIN_FLD_OLD_RETAIL_VALUE,
                                        adj_amountp, ebufp);
                        PIN_FLIST_FLD_PUT(event_flistp,
                                PIN_FLD_CURRENT_TOTAL,
                                        curr_amountp, ebufp);
			/**************************************
			* Get PIN_FLD_DISCOUNT field  value
			* and update it to event discounts
			* and item discounts fields.
			***************************************/
			disc_amountp = PIN_FLIST_FLD_GET(e_bi_flistp,
					PIN_FLD_DISCOUNT, 0, ebufp);
			disc_total = PIN_FLIST_FLD_TAKE(event_flistp,
					PIN_FLD_DISCOUNT_VALUE,
                                                0, ebufp);
			pbo_decimal_subtract_assign(disc_total,
					disc_amountp, ebufp);
			PIN_FLIST_FLD_PUT(event_flistp,
					PIN_FLD_DISCOUNT_VALUE,
                                                disc_total, ebufp);
                }
           }
	} /* If event_flistp */

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_prep_invoice_delta_process_adj_events: ci_flist",
                ci_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_prep_invoice_delta_process_adj_events: before item_flistp",
                item_flistp);

		
		/* 
		 * Get the event level delta amounts
		 * Iterate through the CORRECTION_INFO.OTHER_ITEMS.EVENTS	
		 * and set the delta fields in the event that was adjusted 
		 * If there was no corrections but still a corrective bill
		 * was generated, the CORRECION_INFO will not be present
		 * so skip the loop in this case.
		 */
		elemid2 = 0;
		cookie2 = NULL;
		while ((ci_flistp) &&
			((ci_item_flistp = PIN_FLIST_ELEM_GET_NEXT(ci_flistp, 
					PIN_FLD_OTHER_ITEMS, &elemid2, 1, &cookie2, 
					ebufp)) != (pin_flist_t *)NULL)) {
			oitem_pdp = PIN_FLIST_FLD_GET(ci_item_flistp,
                                PIN_FLD_ITEM_OBJ, 1, ebufp);

			vp = PIN_FLIST_FLD_GET(ci_item_flistp,
                                PIN_FLD_STATUS, 1, ebufp);
			if (vp) {
				oitem_status = *(int32 *)vp;
			}
			tsfer_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(ci_item_flistp,
				PIN_FLD_TRANSFERED, 1, ebufp);	

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"Working with Current Corrections");
			elemid3 = 0;
			cookie3 = NULL;
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"ci_flistp Looping item: ci_item_flistp flist", ci_item_flistp);
			while ((ci_event_flistp = PIN_FLIST_ELEM_GET_NEXT(
				ci_item_flistp, PIN_FLD_EVENTS, &elemid3, 1,
				&cookie3, ebufp)) != (pin_flist_t *)NULL) {
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"ci_flistp Looping event: ci_event_flistp flist", ci_event_flistp);
				update_event = 0;
				update_item = 0;
				multiple_allocations = 0;
				partial_allocation = 0;
				ci_event_pdp = PIN_FLIST_FLD_GET(ci_event_flistp, 
						PIN_FLD_EVENT_OBJ, 0, ebufp);
				ci_event_type = (char *)PIN_POID_GET_TYPE(ci_event_pdp);


				/* TBD - skip non /e/b/a/e ? for perf optim */

				/* if /event/billing/adjustment/event
				 * get the event that was adjusted from the
				 * SESSION_OBJ and update deltas for that event
				 */
				session_pdp = PIN_FLIST_FLD_GET(ci_event_flistp, 
						PIN_FLD_SESSION_OBJ, 
						1, ebufp);

				/* if there is no session object this is
				 * probably an item adjustment so don't have
				 * the event delta calculation
				 */
				if (session_pdp == NULL) {
					continue;
				}

				/************************************************************
				* Special case:
				* Settlement event's session_obj points to dispute event
				* so we need to get dispute event's session_obj for the
				* original event link
				* So we store the original event from dispute event in
				* PIN_FLD_TO_OBJ in sesion event.
				*************************************************************/
				sess_pdp = NULL;
				if (strcmp(ci_event_type, PIN_OBJ_TYPE_EVENT_SETTLEMENT) == 0) {
					elemid4 = 0;
					cookie4 = NULL;
					while ((disp_flistp = PIN_FLIST_ELEM_GET_NEXT(
						out_flistp, PIN_FLD_TRANSFERS_INTO,
						&elemid4, 1, &cookie4, ebufp))
                                                != (pin_flist_t *)NULL) {
						disp_e_pdp = PIN_FLIST_FLD_GET(disp_flistp,
							PIN_FLD_POID, 0, ebufp);
						if (PIN_POID_COMPARE(session_pdp, disp_e_pdp, 
							0, ebufp) == 0) {
							disp_i_pdp = PIN_FLIST_FLD_GET(disp_flistp,
								PIN_FLD_ITEM_OBJ, 0, ebufp);
							sess_pdp = PIN_FLIST_FLD_GET(disp_flistp,
								PIN_FLD_SESSION_OBJ, 1, ebufp);
							/******************
							PIN_FLIST_FLD_SET(ci_item_flistp,
								PIN_FLD_RELATED_ACTION_ITEM_OBJ, 
									disp_i_pdp, ebufp);
							*******************/
							break;
						}
					}	
					if (PIN_POID_IS_NULL( sess_pdp ) != 0 ) {
						session_pdp = sess_pdp;
					}
				}
				if (session_pdp == NULL) {
					continue;
				}

			/*************************************************
			*        Event level Corrections, 
			*
			* the event's session_obj
			* points to the original events
			* In case of settlement event, the session points
			* to dispute events so in sess_pdp we got
			* its dispute events session_obj which has
			* original event.
			* So add the tax, discount and amount variables
			* for Event as well as Item objects
			* In case of AR_ITEMS->  some times
			* events are not passed.
			**************************************************/
			if (event_flistp) {
				if ( PIN_POID_COMPARE (event_pdp, session_pdp, 0,
					ebufp) == 0 ) {
					update_event = 1;
					update_item = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"update_event_and_item");
				}
			}

			/*****************************************************
			*           Item level corrections
			*
			* For Item level AR actions, the event's session_obj
			* does not point to original events. 
			* PIN_FLD_RELATED_BILL_ITEM_OBJ is added to the event
			* which contains billable item obj. So add the
			* tax, discount and amount variables to Item only.
			* Payment case:
			* There will be only one item payment with event... payment
			* and one event transfer and it contains all the billable
			* items and its amounts. 
			* acct adj or bill adj:
			* There will be one item adj and multiple event adj
			* with multiple transfer events.
			* Issue is that it does not have the split for
			* tax, discount and amount. Need to work on this later.
			*****************************************************/
			item_pdp = PIN_FLIST_FLD_GET(item_flistp, 
					PIN_FLD_ITEM_OBJ, 0, ebufp);
			item_type = (char *)PIN_POID_GET_TYPE(item_pdp);
			if (strstr(item_type, "cycle_tax")) {
				item_cycle_tax = 1;
				PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG,
					"item_cycle_tax current corrections");
			}

			/******************************************************
			* In payment case we will have mutilple entries of
			* PIN_FLD_ITEMS ( nothing but event transfer buffer billable
			* items and amts)
			* This is also a case for multiple allocations
			**************************************************************/
			billable_item_flistp = PIN_FLIST_ELEM_GET(ci_event_flistp,
				PIN_FLD_ITEMS,PIN_ELEMID_ANY, 1, ebufp);
			if (billable_item_flistp) {
				billable_item_count = PIN_FLIST_ELEM_COUNT(billable_item_flistp, 
					PIN_FLD_ITEMS, ebufp);
			}
			/*****************************************************************
			* In the case of Account adj or bill adj we will have one item adj
			* and multiple event adjustments
			* This is also a case for multiple allocations
			*
			* In case of cycle forward first billing then rerate, the item
			* adjustment created contains 2 events with same RELATED_BILL_ITEM_OBJ
			* so we need to check if the events are pointing to same billable
			* items. If so we should not consider them as multiple allocation.
			*
			* Note: In case where the item has several events and some events
			* point to same billable and others point to different billable item
			* we still consider as multiple allocations. May need to revisit later.
			******************************************************************/
			special_events_cnt = PIN_FLIST_ELEM_COUNT(ci_item_flistp, PIN_FLD_EVENTS,
				ebufp);
			found_diff_bill_item = 0;
			if (special_events_cnt > 1) {
				ci_event_item_pdp = PIN_FLIST_FLD_GET(ci_event_flistp,
                                                                PIN_FLD_ITEM_OBJ, 1, ebufp);
				cookie_evt = NULL;
				evt_id = 0;				
				while ((sp_evt_flistp = PIN_FLIST_ELEM_GET_NEXT(ci_item_flistp,
					PIN_FLD_EVENTS, &evt_id, 1, &cookie_evt, ebufp)) != (pin_flist_t *)NULL) {
					rel_bill_i_pdp = PIN_FLIST_FLD_GET(sp_evt_flistp,
						PIN_FLD_ITEM_OBJ, 1, ebufp);
					if (PIN_POID_COMPARE (ci_event_item_pdp, rel_bill_i_pdp, 0, ebufp) != 0) {
						found_diff_bill_item = 1;
						break;
					}
				}
				if (!found_diff_bill_item) {
					special_events_in_item = 0;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
						"Multiple Special Events in Items - But all point to same bill items");
				} else {
					special_events_in_item = special_events_cnt;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
						"Multiple Special Events in Items - point to Different bill items");
				}
			}

			if ((billable_item_count > 1) || (special_events_in_item > 1)){
				multiple_allocations = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Multiple Allocations");
			}
			if (multiple_allocations) {
				if (billable_item_count > 1) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Multiple Allocations - Multiple Billable Items");
					if (billable_item_flistp) {
						cookie_rel_bill = NULL;
						rel_bill_id = 0;
						while ((rel_bill_flistp = PIN_FLIST_ELEM_GET_NEXT(billable_item_flistp, PIN_FLD_ITEMS,
							&rel_bill_id, 1, &cookie_rel_bill, ebufp)) != (pin_flist_t *)NULL) {
							ci_event_item_pdp = PIN_FLIST_FLD_GET(rel_bill_flistp,
								PIN_FLD_ITEM_OBJ, 1, ebufp);
							rel_item_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(rel_bill_flistp,
								PIN_FLD_AMOUNT, 1, ebufp);
							if (ci_event_item_pdp && !PIN_POID_IS_NULL(ci_event_item_pdp)
								&& PIN_POID_COMPARE (item_pdp, ci_event_item_pdp, 0, ebufp) == 0) {
								billable_item_matched = 1;
								update_item = 1;
								break;
							}
						}
					}
				}
				if (special_events_in_item > 1) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Multiple Allocations - Multiple Special Events in Items");
					if (billable_item_flistp) {
						cookie_rel_bill = NULL;
						rel_bill_id = 0;
						while ((rel_bill_flistp = PIN_FLIST_ELEM_GET_NEXT(billable_item_flistp, PIN_FLD_ITEMS,
							&rel_bill_id, 1, &cookie_rel_bill, ebufp)) != (pin_flist_t *)NULL) {
							ci_event_item_pdp = PIN_FLIST_FLD_GET(rel_bill_flistp,
								PIN_FLD_ITEM_OBJ, 1, ebufp);
							rel_item_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(rel_bill_flistp,
								PIN_FLD_AMOUNT, 1, ebufp);
							if (ci_event_item_pdp && !PIN_POID_IS_NULL(ci_event_item_pdp)
								&& PIN_POID_COMPARE (item_pdp, ci_event_item_pdp, 0, ebufp) == 0) {
								billable_item_matched = 1;
								update_item = 1;
								break;
							}
						}
					}
				} 
			} else {
				ci_event_item_pdp = PIN_FLIST_FLD_GET(ci_event_flistp, 
					PIN_FLD_RELATED_BILL_ITEM_OBJ, 1, ebufp);
				if ((!PIN_POID_IS_NULL(ci_event_item_pdp) && ci_event_item_pdp) &&
					( PIN_POID_COMPARE (item_pdp, ci_event_item_pdp, 0,
							ebufp) == 0 )) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"Current Corrections -");
					 /***********************************
                                        * In case of partial allocation
                                        * even if only one item array exists
                                        * if its value is different from event
                                        * take the amount from transfer event
                                        *************************************/
					cookie_rel_bill = NULL;
					if (billable_item_flistp) {
						rel_bill_flistp = PIN_FLIST_ELEM_GET_NEXT(billable_item_flistp,
							PIN_FLD_ITEMS, 0, 1, &cookie_rel_bill, ebufp);
						rel_item_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(rel_bill_flistp,
							PIN_FLD_AMOUNT, 1, ebufp);

						if (!(pin_decimal_is_ZERO(tsfer_amt,ebufp)) &&
							(oitem_status != PIN_ITEM_STATUS_CLOSED)) {
							partial_allocation = 1;
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
								"Current Corrections -Partial Allocation");
						}
						update_item = 1;
					}
				}
			}

			if (update_item == 0 && update_event ==0) {
				continue;
			}

			if ((multiple_allocations  || partial_allocation) && add_corr_to_item) {
				if (item_cycle_tax) {
					adj_amountp = PIN_FLIST_FLD_TAKE(item_flistp,
						PIN_FLD_NEXT_BAL,
							0, ebufp);
					pbo_decimal_add_assign(adj_amountp,
						rel_item_amt, ebufp);
					PIN_FLIST_FLD_PUT(item_flistp,
						PIN_FLD_NEXT_BAL,
							adj_amountp, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						" M-Updating item cycle tax amount for current corrections");
				} else {
					adj_amountp = PIN_FLIST_FLD_TAKE(item_flistp,
						PIN_FLD_DELAYED_BAL,
							0, ebufp);
					pbo_decimal_add_assign(adj_amountp,
						rel_item_amt, ebufp);
					PIN_FLIST_FLD_PUT(item_flistp,
						PIN_FLD_DELAYED_BAL,
							adj_amountp, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						" M-Updating item amount for current corrections");
				}
			} else {

			   elemid4 = 0;
			   cookie4 = NULL;
			   while ((balimp_flistp = PIN_FLIST_ELEM_GET_NEXT(
				ci_event_flistp, PIN_FLD_BAL_IMPACTS, 
					&elemid4, 1, &cookie4, ebufp)) 
						!= (pin_flist_t *)NULL) {
				vp = PIN_FLIST_FLD_GET(balimp_flistp,
					PIN_FLD_RESOURCE_ID, 1, ebufp);
				if (vp) {
					res_id = *(int32 *)vp;
				} else res_id = 0;

				if (!PIN_BEID_IS_CURRENCY(res_id)) {
					continue;
				}
				impact_type = 0;
				vp = PIN_FLIST_FLD_GET(balimp_flistp,
					PIN_FLD_IMPACT_TYPE, 1, ebufp);
				if (vp ) {
					impact_type = *(int32 *)vp;

				}	
				 /*********************************************************
				*         Current Event and Item Corrections
				*
				* For each event we are adding  the following:
				* PIN_FLD_AMOUNT_TAX_ADJUSTED for tax
				* PIN_FLD_DISCOUNT_VALUE for discount
				* PIN_FLD_AMOUNT_ADJUSTED for remaining amount
				*
				* For item we are adding the following:
				* PIN_FLD_NEXT_BAL  for tax
				* PIN_FLD_CURRENT_BAL for discount
				* PIN_FLD_DELAYED_BAL for remaining amount
				*********************************************************/
				if ((impact_type & PIN_IMPACT_TYPE_TAX) ||
					(impact_type & PIN_IMPACT_TYPE_RERATED_TAX)) {
					/* There is a tax balimpact - add this */
					temp_decimal = (pin_decimal_t *)
						PIN_FLIST_FLD_GET(balimp_flistp,
						PIN_FLD_AMOUNT, 0, ebufp);
					/**********************************
					* Update event tax field
					**********************************/
					if (update_event) {
						rt_tax_total = PIN_FLIST_FLD_TAKE(event_flistp,
							PIN_FLD_AMOUNT_TAX_ADJUSTED,
							0, ebufp);
						pbo_decimal_add_assign(rt_tax_total,
							temp_decimal, ebufp);
						PIN_FLIST_FLD_PUT(event_flistp,
							PIN_FLD_AMOUNT_TAX_ADJUSTED,
								rt_tax_total, ebufp);
					}

					/********************************************
					* Update item tax field
					********************************************/
					if (update_item && add_corr_to_item) {
						rt_tax_total = PIN_FLIST_FLD_TAKE(item_flistp,
							PIN_FLD_NEXT_BAL,
								0, ebufp);
						pbo_decimal_add_assign(rt_tax_total,
							temp_decimal, ebufp);
						PIN_FLIST_FLD_PUT(item_flistp,
							PIN_FLD_NEXT_BAL,
								rt_tax_total, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
							"Updating item tax for current corrections");
					}

				} else if ((impact_type & PIN_IMPACT_TYPE_DISCOUNT) ||
					(impact_type & PIN_IMPACT_TYPE_DISCOUNT_RERATED)) {
					temp_decimal = (pin_decimal_t *)
						PIN_FLIST_FLD_GET(balimp_flistp,
						PIN_FLD_AMOUNT, 0, ebufp);
					/********************************************
					* Update event discount field
					********************************************/
					if (update_event) {
						disc_total = PIN_FLIST_FLD_TAKE(event_flistp,
							PIN_FLD_DISCOUNT_VALUE,
								0, ebufp);
						pbo_decimal_add_assign(disc_total,
							temp_decimal, ebufp);
						PIN_FLIST_FLD_PUT(event_flistp,
							PIN_FLD_DISCOUNT_VALUE,
								disc_total, ebufp);
					}

					/********************************************
					* Update item discount field
					********************************************/
					if (update_item && add_corr_to_item) {
						disc_total = PIN_FLIST_FLD_TAKE(item_flistp,
								PIN_FLD_CURRENT_BAL,
									0, ebufp);
						pbo_decimal_add_assign(disc_total,
								temp_decimal, ebufp);
						PIN_FLIST_FLD_PUT(item_flistp,
								PIN_FLD_CURRENT_BAL,
									disc_total, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
							"Updating item disc for current corrections");
					}
				} else {
					delta_amountp = PIN_FLIST_FLD_GET(balimp_flistp,
						PIN_FLD_AMOUNT, 0, ebufp);

					/********************************************
					* Update event amount field
					********************************************/
					if (update_event) {
						adj_amountp = PIN_FLIST_FLD_TAKE(event_flistp,
							PIN_FLD_AMOUNT_ADJUSTED,
								0, ebufp);
						curr_amountp = PIN_FLIST_FLD_TAKE(event_flistp,
							PIN_FLD_CURRENT_TOTAL,
								0, ebufp);

						pbo_decimal_add_assign(adj_amountp,
							delta_amountp, ebufp);
						pbo_decimal_add_assign(curr_amountp,
							delta_amountp, ebufp);

						PIN_FLIST_FLD_PUT(event_flistp,
							PIN_FLD_AMOUNT_ADJUSTED,
								adj_amountp, ebufp);
						PIN_FLIST_FLD_PUT(event_flistp,
							PIN_FLD_CURRENT_TOTAL,
								curr_amountp, ebufp);
					}
					/********************************************
					* Update item amount field
					********************************************/
					if (update_item && add_corr_to_item) {
						adj_amountp = PIN_FLIST_FLD_TAKE(item_flistp,
							PIN_FLD_DELAYED_BAL,
									0, ebufp);
						pbo_decimal_add_assign(adj_amountp,
							delta_amountp, ebufp);
						PIN_FLIST_FLD_PUT(item_flistp,
							PIN_FLD_DELAYED_BAL,
								adj_amountp, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
							"Updating item amount for current corrections");
					}
					/**************************************
					* Get PIN_FLD_DISCOUNT field  value
					* and update it to event discounts
					* and item discounts fields.
					***************************************/
					/************************
					* Update event
					************************/
					if (update_event) {
						disc_amountp = PIN_FLIST_FLD_GET(balimp_flistp,
							PIN_FLD_DISCOUNT, 0, ebufp);
						disc_total = PIN_FLIST_FLD_TAKE(event_flistp,
							PIN_FLD_DISCOUNT_VALUE,
								0, ebufp);
						pbo_decimal_subtract_assign(disc_total,
							disc_amountp, ebufp);
						PIN_FLIST_FLD_PUT(event_flistp,
							PIN_FLD_DISCOUNT_VALUE,
								disc_total, ebufp);
					}

					/**************************************
					* Update Item
					**************************************/
					if (update_item && add_corr_to_item) {
						disc_amountp = PIN_FLIST_FLD_GET(balimp_flistp,
							PIN_FLD_DISCOUNT, 0, ebufp);
						disc_total = PIN_FLIST_FLD_TAKE(item_flistp,
							PIN_FLD_CURRENT_BAL,
								0, ebufp);
						pbo_decimal_subtract_assign(disc_total,
							disc_amountp, ebufp);
						PIN_FLIST_FLD_PUT(item_flistp,
							PIN_FLD_CURRENT_BAL,
								disc_total, ebufp);
					}
				}
			   } /* while event bal_impacts */
			}

		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_delta_process_adj_events: after item_flist",
				item_flistp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_prep_invoice_delta_process_adj_events: final item_flist",
                item_flistp);

	/* Cleanup */
	pbo_decimal_destroy(&zero_dec);

	/***********************************************************
	 * error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "fm_inv_pol_prep_invoice_delta_process_adj_events: error ",
		ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_prep_invoice_delta_process_adj_events: output flist",
			out_flistp);
	}
}

/**************************************************************
* fm_inv_pol_prep_invoice_add_subord_trial_invoice_details():
* This function gets the necessary bill details during 
* trial billing (-subord mode) for subordinates and puts it
* under PIN_FLD_BILLS[] so that this info is used later
* during parent's trial billing.
*   BILLS[]
*	PIN_FLD_START_T,
*	PIN_FLD_END_T,
*	PIN_FLD_AR_BILLINFO_OBJ,
*	PIN_FLD_DUE,
*	PIN_FLD_ADJUSTED,
*	PIN_FLD_WRITEOFF,
*	PIN_FLD_DISPUTED,
*	PIN_FLD_RECVD
**************************************************************/
static void 
fm_inv_pol_prep_invoice_add_subord_trial_invoice_details(
	pin_flist_t     *big_flistp, 
	pin_errbuf_t	*ebufp) 
{

	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*bi_flistp = NULL;


        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

	bi_flistp = PIN_FLIST_ELEM_GET(big_flistp, PIN_FLD_BILLINFO,
                        0, 0, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(big_flistp, PIN_FLD_BILLS, 0, ebufp);

	PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_START_T,
                                tmp_flistp, PIN_FLD_START_T, ebufp);
	PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_END_T,
                                tmp_flistp, PIN_FLD_END_T, ebufp);
	PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_AR_BILLINFO_OBJ,
                                tmp_flistp, PIN_FLD_AR_BILLINFO_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_DUE,
                                tmp_flistp, PIN_FLD_DUE, ebufp);
	PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_ADJUSTED,
                                tmp_flistp, PIN_FLD_ADJUSTED, ebufp);
	PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_WRITEOFF,
                                tmp_flistp, PIN_FLD_WRITEOFF, ebufp);
	PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_DISPUTED,
                                tmp_flistp, PIN_FLD_DISPUTED, ebufp);
	PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_RECVD,
                                tmp_flistp, PIN_FLD_RECVD, ebufp);

	/*****************************************************************
         * Error?
         *****************************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_inv_pol_prep_invoice_add_subord_trial_invoice_details error", ebufp);
        }
        return;

}



/* 
* fm_inv_pol_prep_invoice_add_subord_invoice_hyperlink_info()
* This function gets the subordinate invoices for the bill_no of this
* hierarchy and stores the information in PIN_FLD_SUBORD_INFO array
*
*   PIN_FLD_SUBORD_INFO[]
*	PIN_FLD_INVOICE_OBJ
*	PIN_FLD_IS_LINK_ATTRIBUTE
*	PIN_FLD_BILL_NO
*	PIN_FLD_ACCOUNT_OBJ
*	PIN_FLD_ACCOUNT_NO
*	PIN_FLD_FIRST_NAME
*	PIN_FLD_LAST_NAME
*

sample input flist for complex search

0 PIN_FLD_POID                      POID [0] 0.0.0.1 /search -1 0
0 PIN_FLD_FLAGS                      INT [0] 768
0 PIN_FLD_TEMPLATE                   STR [0] "select X from /invoice 1, /account 2 where ( 1.F1 = 2.F2 ) and (1.F3 = V3) "
0 PIN_FLD_RESULTS                  ARRAY [*] allocated 3, used 3
1     PIN_FLD_POID                  POID [0] NULL
1     PIN_FLD_BILL_NO                STR [0] NULL
1     PIN_FLD_LINKED_OBJ           ARRAY [2] allocated 3, used 3
2         PIN_FLD_ACCOUNT_OBJ       POID [0] NULL
2         PIN_FLD_LINK_DIRECTION    ENUM [0] -1
2         PIN_FLD_EXTRA_RESULTS    ARRAY [*] allocated 2, used 2
3             PIN_FLD_ACCOUNT_NO     STR [0] NULL
3             PIN_FLD_NAMEINFO     ARRAY [1] allocated 2, used 2
4                 PIN_FLD_FIRST_NAME STR [0] NULL
4                 PIN_FLD_LAST_NAME  STR [0] NULL
0 PIN_FLD_ARGS                     ARRAY [1] allocated 1, used 1
1     PIN_FLD_ACCOUNT_OBJ           POID [0] NULL
0 PIN_FLD_ARGS                     ARRAY [2] allocated 1, used 1
1     PIN_FLD_POID                  POID [0] NULL
0 PIN_FLD_ARGS                     ARRAY [3] allocated 1, used 1
1     PIN_FLD_BILL_NO                STR [0] "B1-24"

sample output flist from results

0 PIN_FLD_POID                      POID [0] 0.0.0.1 /search -1 0
0 PIN_FLD_RESULTS                  ARRAY [0] allocated 3, used 3
1     PIN_FLD_POID                  POID [0] 0.0.0.1 /invoice 163316 0
1     PIN_FLD_BILL_NO                STR [0] "B1-24"
1     PIN_FLD_LINKED_OBJS          ARRAY [2] allocated 1, used 1
2         PIN_FLD_LINKED_OBJ       ARRAY [0] allocated 1, used 1
3             PIN_FLD_POID          POID [0] 0.0.0.1 /account 152241 9
0 PIN_FLD_RESULTS                  ARRAY [2] allocated 3, used 3
1     PIN_FLD_POID                  POID [0] 0.0.0.1 /invoice 153816 0
1     PIN_FLD_BILL_NO                STR [0] "B1-24"
1     PIN_FLD_LINKED_OBJS          ARRAY [2] allocated 1, used 1
2         PIN_FLD_LINKED_OBJ       ARRAY [0] allocated 1, used 1
3             PIN_FLD_POID          POID [0] 0.0.0.1 /account 163508 8
0 PIN_FLD_RESULTS                  ARRAY [4] allocated 3, used 3
1     PIN_FLD_POID                  POID [0] 0.0.0.1 /invoice 160400 0
1     PIN_FLD_BILL_NO                STR [0] "B1-24"
1     PIN_FLD_LINKED_OBJS          ARRAY [2] allocated 1, used 1
2         PIN_FLD_LINKED_OBJ       ARRAY [0] allocated 1, used 1
3             PIN_FLD_POID          POID [0] 0.0.0.1 /account 156940 9
0 PIN_FLD_EXTRA_RESULTS            ARRAY [1] allocated 3, used 3
1     PIN_FLD_POID                  POID [0] 0.0.0.1 /account 152241 9
1     PIN_FLD_ACCOUNT_NO             STR [0] "parent2"
1     PIN_FLD_NAMEINFO             ARRAY [1] allocated 2, used 2
2         PIN_FLD_FIRST_NAME         STR [0] "parent2"
2         PIN_FLD_LAST_NAME          STR [0] "parent2"
0 PIN_FLD_EXTRA_RESULTS            ARRAY [3] allocated 3, used 3
1     PIN_FLD_POID                  POID [0] 0.0.0.1 /account 163508 8
1     PIN_FLD_ACCOUNT_NO             STR [0] "subord3"
1     PIN_FLD_NAMEINFO             ARRAY [1] allocated 2, used 2
2         PIN_FLD_FIRST_NAME         STR [0] "subord3"
2         PIN_FLD_LAST_NAME          STR [0] "subord3"
0 PIN_FLD_EXTRA_RESULTS            ARRAY [5] allocated 3, used 3
1     PIN_FLD_POID                  POID [0] 0.0.0.1 /account 156940 9
1     PIN_FLD_ACCOUNT_NO             STR [0] "subord2"
1     PIN_FLD_NAMEINFO             ARRAY [1] allocated 2, used 2
2         PIN_FLD_FIRST_NAME         STR [0] "subord2"
2         PIN_FLD_LAST_NAME          STR [0] "subord2"

*/


static void
fm_inv_pol_prep_invoice_add_subord_invoice_hyperlink_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *out_flistp,
        int32                   combined_inv_flags,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t     *a_flistp = NULL;
        pin_flist_t     *bi_flistp = NULL;
        pin_flist_t     *arg_flistp = NULL;
        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
        pin_flist_t     *r_res_flistp = NULL;
        pin_flist_t     *r_eres_flistp = NULL;
        pin_flist_t     *lo_flistp = NULL;
        pin_flist_t     *er_flistp = NULL;
        pin_flist_t     *ni_flistp = NULL;
        pin_flist_t     *tmp_subords_flistp = NULL;
        pin_flist_t     *sub_flistp = NULL;
        poid_t          *a_pdp = NULL;
        poid_t          *s_pdp = NULL;
        pin_cookie_t    cookie_r = NULL;
        pin_cookie_t    cookie_er = NULL;
        int32           elemid_r = 0;
        int32           elemid_er = 0;
        int32           elemid_sub = 0;
        int32           direction = -1;
        int32           s_flags = SRCH_DISTINCT;
        int64           poid_id = 0;
        char            s_template[2*BUFSIZ] = {""};
        void            *vp = NULL;


        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_add_subord_invoice_hyperlink_info input out_flistp :", out_flistp);


        /***********************************************************
         * Create search fliist
         ***********************************************************/

        s_flistp = PIN_FLIST_CREATE(ebufp);

        /***********************************************************
         * Construct the search poid.
         ***********************************************************/
        a_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_ACCTINFO, 0, 0,
                                        ebufp);
        a_pdp = (poid_t *)PIN_FLIST_FLD_GET(a_flistp, PIN_FLD_ACCOUNT_OBJ,
                                        0, ebufp);
        s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp), "/search",
                                 -1, ebufp);
        if( s_pdp ) {
                PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID,
                                      (void *)s_pdp, ebufp);
        }
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	
	strcpy ( s_template, "select X from /invoice 1, /account 2 "
			" where 1.F1 = 2.F2 "
                        " and 1.F3 = V3 ");
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,(void *)s_template, ebufp);

        /* 1st arg */
        arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)NULL, ebufp);

        /* 2nd arg */
        arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)NULL, ebufp);

        /* 3rd arg */
        arg_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 3, ebufp);
        bi_flistp = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_BILLINFO,
					0, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_BILL_NO, 0, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_BILL_NO, vp, ebufp);

	/* Add the RESULTS array */
        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_BILL_NO, (void *)NULL, ebufp);
        lo_flistp = PIN_FLIST_ELEM_ADD (res_flistp, PIN_FLD_LINKED_OBJ, 
					2, ebufp);
        PIN_FLIST_FLD_SET(lo_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(lo_flistp, PIN_FLD_LINK_DIRECTION, 
					(void *)&direction, ebufp);
        er_flistp = PIN_FLIST_ELEM_ADD (lo_flistp, PIN_FLD_EXTRA_RESULTS, 
					PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(er_flistp, PIN_FLD_ACCOUNT_NO, (void *)NULL, ebufp);
        ni_flistp = PIN_FLIST_ELEM_ADD (er_flistp, PIN_FLD_NAMEINFO, 
					1, ebufp);
        PIN_FLIST_FLD_SET(ni_flistp, PIN_FLD_FIRST_NAME, (void *)NULL, ebufp);
        PIN_FLIST_FLD_SET(ni_flistp, PIN_FLD_LAST_NAME, (void *)NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_add_subord_invoice_hyperlink_info input s_flistp :", s_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, 
				s_flistp, &r_flistp, ebufp);
                                
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_add_subord_invoice_hyperlink_info output r_flistp :",r_flistp);
	
	/* Rearrange the results and put into out_flistp */
        tmp_subords_flistp = PIN_FLIST_CREATE(ebufp);

	while ((r_res_flistp = PIN_FLIST_ELEM_GET_NEXT(
			r_flistp, PIN_FLD_RESULTS, &elemid_r,
                	1, &cookie_r, ebufp)) != (pin_flist_t *)NULL) {
        	sub_flistp = PIN_FLIST_ELEM_ADD (tmp_subords_flistp, 
			PIN_FLD_SUBORD_INFO, elemid_sub++, ebufp);
		vp = PIN_FLIST_FLD_GET(r_res_flistp, PIN_FLD_POID, 0, ebufp);
        	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_INVOICE_OBJ, vp, ebufp);

		poid_id = PIN_POID_GET_ID ( vp );
        	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_IS_LINK_ATTRIBUTE, 
				(void *)&poid_id, ebufp);

		vp = PIN_FLIST_FLD_GET(r_res_flistp, PIN_FLD_BILL_NO, 0, ebufp);
        	PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_BILL_NO, vp, ebufp);

		while ((r_eres_flistp = PIN_FLIST_ELEM_GET_NEXT(
			r_flistp, PIN_FLD_EXTRA_RESULTS, &elemid_er,
                	1, &cookie_er, ebufp)) != (pin_flist_t *)NULL) {

			vp = PIN_FLIST_FLD_GET(r_eres_flistp, 
				PIN_FLD_POID, 0, ebufp);
        		PIN_FLIST_FLD_SET(sub_flistp, 
				PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
			vp = PIN_FLIST_FLD_GET(r_eres_flistp, 
				PIN_FLD_ACCOUNT_NO, 0, ebufp);
        		PIN_FLIST_FLD_SET(sub_flistp, 
				PIN_FLD_ACCOUNT_NO, vp, ebufp);
        		ni_flistp = PIN_FLIST_ELEM_GET(r_eres_flistp, 
				PIN_FLD_NAMEINFO, 1, 0, ebufp);
			vp = PIN_FLIST_FLD_GET(ni_flistp, 
				PIN_FLD_FIRST_NAME, 0, ebufp);
        		PIN_FLIST_FLD_SET(sub_flistp, 
				PIN_FLD_FIRST_NAME, vp, ebufp);
			vp = PIN_FLIST_FLD_GET(ni_flistp, 
				PIN_FLD_LAST_NAME, 0, ebufp);
        		PIN_FLIST_FLD_SET(sub_flistp, 
				PIN_FLD_LAST_NAME, vp, ebufp);

			break;
		}
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_add_subord_invoice_hyperlink_info  tmp_subords_flistp :", tmp_subords_flistp);
	
	PIN_FLIST_CONCAT(out_flistp, tmp_subords_flistp, ebufp);
                        
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&tmp_subords_flistp, NULL);

	/*****************************************************************
         * Error?
         *****************************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_inv_pol_prep_invoice_add_subord_invoice_hyperlink_info error", ebufp);
        } else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_inv_pol_prep_invoice_add_subord_invoice_hyperlink_info  return out_flistp :", out_flistp);
	}
        return;

}

