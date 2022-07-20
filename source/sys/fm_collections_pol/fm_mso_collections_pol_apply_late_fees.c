/*
 *      Copyright (c) 2001-2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_collections_pol_apply_late_fees.c:CollectionsR1Int:3:2007-Oct-01 12:07:15 %";
#endif

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#include "cm_fm.h"
#include "cm_cache.h"
#include "ops/collections.h"
#include "ops/bill.h"
#include "ops/bal.h"
#include "ops/act.h"
#include "fm_utils.h"
#include "fm_collections.h"
#include "pin_rate.h"
#include "mso_ops_flds.h"
#include "pin_currency.h"
#include "mso_lifecycle_id.h"


#ifdef MSDOS
#include <WINDOWS.h>
#endif

#define SERVICE_TAX "MSO_Service_Tax"
#define BB_ACTION "BB_Custom_Late_Fee"
#define LATE_FEE_EVENT "/event/billing/late_fee"
#define ACTION_COMPLETED 2
/*******************************************************************
 * Routines defined here.
 *******************************************************************/

EXPORT_OP void
op_mso_collections_pol_apply_late_fees(
	cm_nap_connection_t	*connp, 
	int32			opcode, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

extern void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);


static void
fm_mso_collections_pol_apply_late_fees(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp);

static int32
fm_mso_collections_pol_get_currency(
	pcm_context_t		*ctxp, 
	poid_t			*a_pdp, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_collections_pol_calc_late_fees(
	pcm_context_t		*ctxp, 
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);


void fm_mso_get_pay_indicator(
	pcm_context_t           *ctxp,
	poid_t                  *bi_pdp,
	int32                   **pay_indpp,
	pin_errbuf_t            *ebufp);
/*******************************************************************
 * Variables/functions defined elsewhere
 *******************************************************************/
PIN_IMPORT void fm_mso_utils_get_service_details(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        int32                   serv_type,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void fm_mso_utils_get_close_service_details(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        int32                   serv_type,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);


PIN_IMPORT void fm_mso_utils_read_any_object(
        pcm_context_t           *ctxp,
        poid_t                  *objp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp);

void fm_mso_get_service_prov_details(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        int32                   serv_type,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_COLLECTIONS_POL_APPLY_LATE_FEES 
 *******************************************************************/
void
op_mso_collections_pol_apply_late_fees(
	cm_nap_connection_t	*connp, 
	int32			opcode, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_COLLECTIONS_POL_APPLY_LATE_FEES) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_collections_apply_late_fees opcode error", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_collections_apply_late_fees input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_mso_collections_pol_apply_late_fees(ctxp, flags, in_flistp, 
			ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_collections_apply_late_fees error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_collections_apply_late_fees return flist", 
			*ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_mso_collections_pol_apply_late_fees()
 *
 * This function applies late fees for the billinfo.
 * The late fees can be a fixed amount or a percentage of the current
 * overdue amount or the combination of the two.
 *******************************************************************/
static void
fm_mso_collections_pol_apply_late_fees(
	pcm_context_t		*ctxp, 
	int32			flags, 
	pin_flist_t		*in_flistp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*input_flistp = NULL;
	pin_flist_t		*output_flistp = NULL;
	pin_flist_t		*lr_flistp = NULL;
	pin_flist_t		*event_flistp = NULL;
	pin_flist_t		*total_flistp = NULL;

	poid_t			*a_pdp = NULL;
	poid_t			*event_pdp = NULL;
	void			*vp = NULL;
	char			*prog_name = "pin_collections_process";
	int32			impact_type = PIN_IMPACT_TYPE_PRERATED;
	int64			db = 0;
	int32			currency = PIN_CURRENCY_INR;
	int32			c_status = 0;
	int32			calc_status = 0;
	int32			*status = NULL;
	time_t			pvt=pin_virtual_time(NULL);
	pin_decimal_t           *latefee_fixed_amt = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_collections_pol_apply_late_fees input flist",
		in_flistp);

	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	db = PIN_POID_GET_DB(a_pdp);
	event_pdp = PIN_POID_CREATE(db, LATE_FEE_EVENT, (int64)-1, ebufp);

	input_flistp = PIN_FLIST_CREATE(ebufp); 
	PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_POID, a_pdp, ebufp);
	event_flistp = PIN_FLIST_SUBSTR_ADD(input_flistp, PIN_FLD_EVENT, ebufp);
	PIN_FLIST_FLD_PUT(event_flistp, PIN_FLD_POID, event_pdp, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BILLINFO_OBJ, event_flistp, PIN_FLD_BILLINFO_OBJ, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_DESCR, "Late Fee For Broadband Customers", ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_SYS_DESCR, "Late Fee For Broadband Customers", ebufp);
	total_flistp = PIN_FLIST_ELEM_ADD(event_flistp, PIN_FLD_TOTAL, 
					currency, ebufp);

	/* Get the late fee amount */
	fm_mso_collections_pol_calc_late_fees(ctxp,in_flistp ,&lr_flistp,ebufp);
	
	PIN_ERR_LOG_FLIST(3, "Calc fee flist:",lr_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Error calculating the Late Fee", ebufp);
		goto CLEANUP;
	}
	status = PIN_FLIST_FLD_GET(lr_flistp, PIN_FLD_STATUS, 1, ebufp);
	if(status)	
	{
	 	 calc_status = *status;
	}	
	if( calc_status ==1)
	{
	   PIN_ERR_LOG_MSG(1, "LPF cancelled");
	   *ret_flistpp = PIN_FLIST_COPY(lr_flistp, ebufp);
	   goto CLEANUP;		
	}

	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_START_T , &pvt, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_END_T, &pvt, ebufp);

	vp = PIN_FLIST_FLD_GET(lr_flistp,PIN_FLD_ACTION_NAME,0,ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_NAME , (void *)vp,ebufp);
	vp = PIN_FLIST_FLD_GET(lr_flistp,PIN_FLD_SERVICE_OBJ,0,ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_SERVICE_OBJ , (void *)vp,ebufp);
	
	vp = PIN_FLIST_FLD_GET(lr_flistp,PIN_FLD_AMOUNT,0,ebufp);
	latefee_fixed_amt = (pin_decimal_t *)vp;
	if((pbo_decimal_is_null(latefee_fixed_amt,ebufp)  || pbo_decimal_is_zero (latefee_fixed_amt,ebufp)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"NO Let fees to be applied  hence avoicing");
		// Error flist being created
		*ret_flistpp = PIN_FLIST_CREATE(ebufp); 
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, a_pdp, ebufp);
		c_status = ACTION_COMPLETED;
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &c_status, ebufp);

		goto CLEANUP;
	}

	PIN_FLIST_FLD_SET(total_flistp, PIN_FLD_AMOUNT , (void *)vp,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_ar_debit_note prepared flist for PCM_OP_ACT_USAGE", input_flistp);

	PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, input_flistp, &output_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_collections_pol_apply_late_fees "
			"err calling PCM_OP_ACT_USAGE error",ebufp);
		goto CLEANUP;
	}
	c_status = ACTION_COMPLETED;
	PIN_FLIST_FLD_SET(output_flistp, PIN_FLD_STATUS, &c_status, ebufp);
	*ret_flistpp = PIN_FLIST_COPY(output_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_collections_pol_apply_late_fees return flist",
		*ret_flistpp);

	if (!PIN_ERRBUF_IS_ERR(ebufp))
	{
		fm_mso_create_lifecycle_evt(ctxp, in_flistp, *ret_flistpp, ID_LATE_FEE ,ebufp );
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&lr_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&input_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&output_flistp, NULL);
}


/*******************************************************************
 * This function calculates the late fees for the billinfo.
 * The late fees can be a fixed amount or a percentage of the current
 * overdue amount or the combination of the two.
 *******************************************************************/

static void
fm_mso_collections_pol_calc_late_fees(
	pcm_context_t		*ctxp, 
	pin_flist_t		*in_flistp ,
	pin_flist_t		**ret_flistpp ,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*ca_flistp = NULL;
	pin_flist_t		*cca_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*act_flds_flistp = NULL;
	pin_flist_t             *act_flds_oflistp = NULL;
	void			*vp = NULL;
	poid_t			*ca_pdp = NULL;
	poid_t			*a_pdp = NULL;
	poid_t			*bi_pdp = NULL;
	poid_t			*action_pdp = NULL;
	poid_t			*service_pdp = NULL;
        pin_decimal_t           *latefee_fixed_amt = NULL;
	char 			*action_name = NULL;
	double			due_amount = 0;
	int32			serv_type_bb = 2;
	int32			serv_type_catv = 0;
	int32			*pay_indicator = NULL;
	int32			*business_type = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	char			tmp [256];
	char			*corp = "95" ;
	int32			cancelled = 1;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Custom Late Fee Calculation");

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_collections_pol_calc_late_fees input flist",
		in_flistp);

	action_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	
	act_flds_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(act_flds_flistp, PIN_FLD_POID, a_pdp, ebufp)
	PIN_FLIST_FLD_SET(act_flds_flistp, PIN_FLD_BUSINESS_TYPE, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(3, "Account readflds flistp", act_flds_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, act_flds_flistp, &act_flds_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&act_flds_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(1, "Error in Account read:");
                return;
	}
	PIN_ERR_LOG_FLIST(3, "Account readflds out flistp", act_flds_oflistp);
        business_type = PIN_FLIST_FLD_GET(act_flds_oflistp, PIN_FLD_BUSINESS_TYPE, 0, ebufp);	
	sprintf(tmp, "%d", *business_type);
	if(strncmp(tmp, corp, 2) == 0)
	{
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "BB corp account not eligible for LPF");
		*ret_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &cancelled, ebufp);
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_collections_pol_calc_late_fees result  flist",
                *ret_flistpp);
                //pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                //PIN_ERR_PERMISSION_DENIED, 0, 0, 0);
                goto cleanup;
        }


	fm_mso_get_pay_indicator(ctxp, bi_pdp, &pay_indicator, ebufp);

	sprintf(tmp,"Pay_indictor is: %ld", *pay_indicator);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		goto cleanup;

	if(*pay_indicator == 1)
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "It's a PRPAID Service");
        /* Get the action config object */
        i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, (void *)action_pdp, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CONFIG_ACTION_OBJ, NULL, ebufp);

        PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, i_flistp,
                        &r_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		goto cleanup;

        ca_pdp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp,
                        PIN_FLD_CONFIG_ACTION_OBJ, 0, ebufp);

	fm_mso_utils_read_any_object(ctxp, ca_pdp, &ca_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
			"Error reading the collections configuration!!");
                goto cleanup;
        }
	cca_flistp = PIN_FLIST_ELEM_GET(ca_flistp, PIN_FLD_CONFIG_ACTION_INFO,
					PIN_ELEMID_ANY, 0, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		goto cleanup;

        action_name = (char *)PIN_FLIST_FLD_GET(cca_flistp,
                        PIN_FLD_ACTION_NAME, 0, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Collection Action Name is:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, action_name);

	vp = (pin_decimal_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DUE, 0, ebufp);
	if( !pbo_decimal_is_null(vp,ebufp) )
		due_amount = pbo_decimal_to_double(vp, ebufp);

	/* MSO Customization for Broadband Customers */
	// Fixed payindicator type, 0 to 1 to handle advance customer
	if ( *pay_indicator == 1 && due_amount != 0 && strcmp(action_name, BB_ACTION) == 0)
	{
		if (due_amount >= 200 && due_amount <= 2000)
		{
			latefee_fixed_amt = pbo_decimal_from_str("50.0", ebufp);
		}
		else if (due_amount > 2001 && due_amount < 5000)
		{
			latefee_fixed_amt = pbo_decimal_from_str("100.0", ebufp);
		}
		else if (due_amount >= 5001)
		{
			latefee_fixed_amt = pbo_decimal_from_str("200.0", ebufp);
		}
		else
		{
			latefee_fixed_amt = pbo_decimal_from_str("0.0", ebufp);
		}
	}
	else    /* For Non-Brodband Customers */
	{
              latefee_fixed_amt = pbo_decimal_from_str("0.0", ebufp);
	}
	if (strcmp(action_name, BB_ACTION) == 0)
	{
		fm_mso_get_service_prov_details(ctxp, a_pdp, serv_type_bb,
						&srch_flistp, ebufp);
		result_flistp = PIN_FLIST_ELEM_GET(srch_flistp, PIN_FLD_RESULTS, 
			PIN_ELEMID_ANY, 1, ebufp);
		// dont charge LPF for inactive customers 
		/*if (result_flistp == NULL) 
		{
			fm_mso_utils_get_close_service_details(ctxp, a_pdp, serv_type_bb,
						&srch_flistp, ebufp);
		}*/
	}
	else
	{
		fm_mso_utils_get_service_details(ctxp, a_pdp, serv_type_catv,
						&srch_flistp, ebufp);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
		goto cleanup;
        result_flistp = PIN_FLIST_ELEM_GET(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if(result_flistp == NULL) {
	        *ret_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &cancelled, ebufp);
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_collections_pol_calc_late_fees result  flist",
                *ret_flistpp);
	//	pin_set_err(ebufp, PIN_ERRLOC_FM,
	//	PIN_ERRCLASS_SYSTEM_DETERMINATE,
	//	PIN_ERR_NOT_FOUND, 0, 0, 0);
		goto cleanup;
	}
	service_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);

        ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, a_pdp, ebufp);
	PIN_FLIST_FLD_PUT(ret_flistp, PIN_FLD_AMOUNT, latefee_fixed_amt, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ACTION_NAME, action_name, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_collections_pol_calc_late_fees result  flist",
		*ret_flistpp);
cleanup:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ca_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&act_flds_oflistp, NULL);
        //pbo_decimal_destroy(&latefee_fixed_amt);
	if(pay_indicator)
      		free(pay_indicator); 
}

/**************************************************************************
* Function:     fm_mso_get_pay_indicator()
* Owner:        Hrish Kulkarni
* Decription:   For getting pay indicator of an account.
*
**************************************************************************/
void fm_mso_get_pay_indicator(
	pcm_context_t           *ctxp,
	poid_t                  *bi_pdp,
	int32                   **pay_indpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *o_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *inv_flistp = NULL;
	poid_t                  *s_pdp = NULL;
	int32                   flags = 256;
	int64                   db = 0;
	int64                   id = -1;
	int32                   pay_indicator = 0;
	char                    *s_template = "select  X from /payinfo/invoice 1, /billinfo 2 where 1.F1 = 2.F2 and 2.F3 = V3 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error before calling fm_mso_get_pay_indicator", ebufp);
			return;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_get_pay_indicator");
	db = PIN_POID_GET_DB(bi_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	/*******************************************************
	PCM_OP_SEARCH Input:
	0 PIN_FLD_POID POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_FLAGS INT [0] 256
	0 PIN_FLD_TEMPLATE STR [0] "select  X from /payinfo/invoice 1, /billinfo 2 where 1.F1 = 2.F2 and 2.F3 = V3 "
	0 PIN_FLD_ARGS ARRAY [1]
	1       PIN_FLD_POID POID [0] NULL
	0 PIN_FLD_ARGS ARRAY [2]
	1       PIN_FLD_PAYINFO_OBJ POID [0] NULL
	0 PIN_FLD_ARGS ARRAY [3]
	1       PIN_FLD_POID POID [0] 0.0.0.1 /billinfo 184750
	0 PIN_FLD_RESULTS ARRAY [0]
	1       PIN_FLD_POID POID [0] NULL
	1       PIN_FLD_INV_INFO      ARRAY [0]
	2               PIN_FLD_INDICATOR       INT [0] 0

	PCM_OP_SEARCH Output:
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 2
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /payinfo/invoice 186030 0
	1     PIN_FLD_INV_INFO      ARRAY [0] allocated 20, used 1
	2         PIN_FLD_INDICATOR       INT [0] 0
	*******************************************************/
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, bi_pdp, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	inv_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_INV_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(inv_flistp, PIN_FLD_INDICATOR, &pay_indicator, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Pay indicator Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Pay indicator Output flist:",o_flistp);

	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_get_pay_indicator error: in calling PCM_OP_SEARCH", ebufp);
		goto CLEANUP;
	}
	res_flistp = PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (res_flistp == NULL)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_get_pay_indicator error: No Pay indicator!!", ebufp);
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, 0, 0, 0);
		goto CLEANUP;
	}
	inv_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_INV_INFO, PIN_ELEMID_ANY, 1, ebufp);
	if (inv_flistp == NULL)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_get_pay_indicator error: No Pay indicator!!", ebufp);
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NOT_FOUND, 0, 0, 0);
		goto CLEANUP;
	}
	*pay_indpp = PIN_FLIST_FLD_TAKE(inv_flistp, PIN_FLD_INDICATOR, 0, ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
	return;
}

/**************************************************************************
* Function:     fm_mso_get_service_prov_details()
* Owner:        Sasikumar
* Decription:   For getting service details of broadband with active provisioing status.
*
**************************************************************************/
void fm_mso_get_service_prov_details(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        int32                   serv_type,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
	pin_flist_t		*telco_flistp = NULL;
        poid_t                  *s_pdp = NULL;
        poid_t                  *serv_pdp = NULL;
        int32                   flags = 256;
        int32                   srvc_status_active  = 10100;
        int32                   prov_status = 2;
        int64                   db = 0;
        int64                   id = -1;
        char                    *s_template = " select X from /service/telco/broadband where F1 = V1 and F2 = V2 and  F3 = V3 and F4 = V4 ";

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_get_service_prov_details", ebufp);
                return;
        }

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_get_service_prov_details");
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "acct_pdp", acct_pdp );

        db = PIN_POID_GET_DB(acct_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        serv_pdp = PIN_POID_CREATE(db, "/service/telco/broadband", id, ebufp);

        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, serv_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &srvc_status_active, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	telco_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp);
	PIN_FLIST_FLD_SET(telco_flistp, PIN_FLD_STATUS, &prov_status, ebufp);        

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Input flist:",s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Output flist:",*o_flistpp);

        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        return;
}
