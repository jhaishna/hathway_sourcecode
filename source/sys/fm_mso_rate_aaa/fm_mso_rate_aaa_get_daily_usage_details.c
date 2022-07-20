/*******************************************************************
 * File:	fm_mso_rate_aaa_search_susp_event.c
 * Opcode:	MSO_OP_RATE_AAA_GET_DAILY_USAGE_DETAILS 
 * Owner:	Nagaraja V
 * Created:	
 * Description: This opcode is used to search the daily usage details for a given account and a given date range.
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 *
1. Search Based on Session ID
        0 PIN_FLD_POID               POID   [0] 0.0.0.1 /account 1234 0
        0 PIN_FLD_START_T            TSTAMP [0] (123445564)
        0 PIN_FLD_END_T	             TSTAMP [0] (123446564)

********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_rate_aaa_search_susp_event.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_errs.h"
#include "mso_cust.h"

/*******************************************************************
 * MSO_OP_RATE_AAA_GET_DAILY_USAGE_DETAILS
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_rate_aaa_get_daily_usage_details(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_rate_aaa_get_daily_usage_details(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_cust_get_bp_obj(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        poid_t                  *service_obj,
	int			mso_status,
        poid_t                  **bp_obj,
        poid_t                  **mso_obj,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_CUST_SEARCH_BILL  
 *******************************************************************/
void 
op_mso_rate_aaa_get_daily_usage_details(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	poid_t			*inp_pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_RATE_AAA_GET_DAILY_USAGE_DETAILS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_rate_aaa_get_daily_usage_details error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_rate_aaa_get_daily_usage_details input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_rate_aaa_get_daily_usage_details(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
	    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_rate_aaa_get_daily_usage_details error", ebufp);
	}
	else
	{
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"op_mso_rate_aaa_get_daily_usage_details output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_rate_aaa_get_daily_usage_details(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*srch_pp_in_flistp = NULL;
	pin_flist_t		*srch_pp_out_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*sort_fld_flistp = NULL;
	void			*srch_pdp = NULL;
	poid_t			*acc_pdp = NULL;
	time_t			*pp_eff_t = NULL;
	time_t			*start_day = NULL;
	time_t			*end_day = NULL;
	int32			elem_count = 0;
	int32			srch_flag = 0;
	int64			db = 1;
	char            error_code[5] = { '\0' };
    char            error_desc[250] = { '\0' };
	char			*template = "select X from /mso_bb_daily_usage where F1 = V1 and F2 >= V2 and F3 <= V3 " ;
	char			*pp_template = "select X from /mso_purchased_plan where F1 = V1 " ;
	char			debug_msg[250];
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_daily_usage_details input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_rate_aaa_get_daily_usage_details", ebufp);
		return;
	}
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	start_day = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 0, ebufp);
	end_day = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);

	sprintf(debug_msg, "Start time is %d", *start_day);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
	sprintf(debug_msg, "End time is %d", *end_day);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);


	if(*start_day > *end_day) {
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid date input");
	    *r_flistp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistp, PIN_FLD_POID, ebufp);
	    memset(error_code, 0, 5);
            memset(error_desc, 0, 250);
	    strcpy(error_code,MSO_ERR_DAILY_USAGE_DETAIL_INPUT_ERR);
            strcpy(error_desc, MSO_ERR_DAILY_USAGE_DETAIL_INPUT_ERR_MSG);
	    PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
            PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_ERROR_DESCR, error_desc, ebufp);
	    return;
	}

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
/*
	srch_pp_in_flistp = PIN_FLIST_CREATE(ebufp);
    	PIN_FLIST_FLD_SET(srch_pp_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);	
    	PIN_FLIST_FLD_SET(srch_pp_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
    	PIN_FLIST_FLD_SET(srch_pp_in_flistp, PIN_FLD_TEMPLATE, pp_template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_pp_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_pp_in_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DESCR, MSO_BB_BP, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_pp_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_EFFECTIVE_T, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search base plan input flist", srch_pp_in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_pp_in_flistp, &srch_pp_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search base plan output flist", srch_pp_out_flistp);
        PIN_FLIST_DESTROY_EX(&srch_pp_in_flistp, NULL);

	if(PIN_FLIST_ELEM_COUNT(srch_pp_out_flistp, PIN_FLD_RESULTS, ebufp) != 0) {
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base plan entry found. Checking if the purchase time is later than start day");
	    result_flist = PIN_FLIST_ELEM_GET(srch_pp_out_flistp, PIN_FLD_RESULTS,0, 0, ebufp);
	    pp_eff_t = (time_t *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_EFFECTIVE_T, 0 , ebufp);
	    if(*pp_eff_t > *start_day)
	    {
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Purchase time is later than start day. Using purchase date as start day");
		start_day = pp_eff_t;
	    } else {
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Purchase time is earlier than start day. Doing nothing..");
	    }	
	} else {
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base plan entry not found. Will go ahead with passed start day");
	}
*/

	srch_flistp = PIN_FLIST_CREATE(ebufp);
    	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);	
    	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
    	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After template");
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After arg1");

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_EFFECTIVE_T, start_day, ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After arg2");

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_EFFECTIVE_T, end_day, ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After arg2");

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_EFFECTIVE_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BYTES_UPLINK, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BYTES_DOWNLINK, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_TOTAL_DATA, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DURATION, NULL, ebufp);
	
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_login input flist", srch_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
    PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&srch_pp_out_flistp, NULL);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
        return;
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_daily_usage_details search output flist", srch_out_flistp);

	/* Pawan: Added below to sort result flist on date as requested by OAP */
	sort_fld_flistp = PIN_FLIST_CREATE(ebufp);
	result_flist = PIN_FLIST_ELEM_ADD(sort_fld_flistp,PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_EFFECTIVE_T, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Sort Fld flist", sort_fld_flistp);
	PIN_FLIST_SORT(srch_out_flistp,sort_fld_flistp,1,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Flist after sorting", srch_out_flistp);
	
	elem_count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
	if ( elem_count == 0)
	{
		*r_flistp = PIN_FLIST_CREATE(ebufp);
	/*	pin_errbuf_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                     MSO_ERR_DAILY_USAGE_DETAIL_NOT_FOUND, 0, 0, 0, PIN_DOMAIN_ERRORS,
                                     MSO_ERR_DAILY_USAGE_DETAIL_NOT_FOUND, 1, 0, NULL);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Could not find the daily usage for the given account and date", ebufp);
	*/
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Could not find the daily usage for the given account and date");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID,*r_flistp, PIN_FLD_POID, ebufp);
	} else {
		PIN_FLIST_FLD_SET(srch_out_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
	    *r_flistp = srch_out_flistp;
	} 
	return;
}

