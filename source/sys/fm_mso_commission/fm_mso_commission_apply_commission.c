/*******************************************************************
 * File:        fm_mso_commission_apply_commission.c
 * Opcode:      MSO_OP_COMMISSION_APPLY_COMMISSION
 * Owner:       
 * Created:     29-JUL-2014
 * Description: This opcode is for applying and impacting the commission
 *******************************************************************/
  
  
  
#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_commission_apply_commission.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_rate.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_commission.h"
  
EXPORT_OP void
op_mso_commission_apply_commission (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_commission_apply_commission (
        pcm_context_t     	 *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
  
static void
fm_mso_negative_qty_modify(
        pcm_context_t     	 *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


void
op_mso_commission_apply_commission (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) { 
        pcm_context_t           *ctxp = connp->dm_ctx;
        *r_flistpp              = NULL;
 
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);
 
        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != MSO_OP_COMMISSION_APPLY_COMMISSION) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_commission_apply_commission error",ebufp);
                return;
        }
        /*******************************************************************
 
        /*******************************************************************
         * Debug: Input flist
         *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_mso_commission_apply_commission input flist", i_flistp);
	fm_mso_commission_apply_commission(ctxp, flags, i_flistp, r_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_commission_apply_commission output flist", *r_flistpp);
        return;
}
 
 
/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void
fm_mso_commission_apply_commission (
        pcm_context_t           *ctxp,
        int32             	flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

	pin_flist_t		*flistp = NULL;
	pin_flist_t		*comm_flistp = NULL;
	pin_flist_t		*comm_event_flistp = NULL;
	//pin_flist_t		*plan_info_flistp = NULL;
	pin_flist_t		*charge_info_flistp = NULL;
	pin_flist_t		*custom_info_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	//pin_flist_t		*res_flistp = NULL;
	//pin_flist_t		*serv_info_flistp = NULL;
	//pin_flist_t		*lco_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	poid_t			*event_objp = NULL;
	//poid_t			*poid_objp = NULL;
	//int32			flag = 0;
	int64			db = -1;
	int32			*commission_rule = NULL;
	//pin_flist_t		*charge_head_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*r_comm_flistp = NULL;
	pin_flist_t		*dup_i_flistp = NULL;

	//int32			*count = NULL;
	int32			*rating_status = NULL;
	int32			rating_success = 0;
	int32			*commission_for = NULL;
	//int32			rating_failed = 1;
	//poid_t			*object_pdp = NULL;
	poid_t			*lco_objp = NULL;
	poid_t			*dtr_objp = NULL;
	poid_t			*sdt_objp = NULL;

	char			*descr = NULL;
//	char			*service_type = NULL;
	char			err_descr[255]="";
	//void			*vp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_apply_commission input flist", i_flistp);

	/* Read the required data from the input flist - Start */
	db = PIN_POID_GET_DB(PCM_GET_USERID(ctxp));
	commission_rule  = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_COMMISSION_RULE, 0, ebufp);
	lco_objp         = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LCO_OBJ,         1, ebufp);
	dtr_objp         = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DT_OBJ,          1, ebufp);
	sdt_objp         = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SDT_OBJ,         1, ebufp);
	commission_for   = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_COMMISSION_FOR,  1, ebufp);

//	service_type = (char*)PIN_POID_GET_TYPE (PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LCO_OUTCOLLECT_SERV_OBJ, 1, ebufp));
//	if (service_type == NULL) {
//		service_type = (char*)PIN_POID_GET_TYPE (PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, 1, ebufp));
//	}
//        if (service_type == NULL) {
//		service_type = (char*)PIN_POID_GET_TYPE (PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DSA_SERV_OBJ, 1, ebufp));
//        }
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, service_type);
	/* Read the required data from the input flist - End */

	/* Prepare the input flist to apply the commission charge - Start */
	comm_flistp = PIN_FLIST_CREATE(ebufp);
	comm_event_flistp = PIN_FLIST_SUBSTR_ADD(comm_flistp, PIN_FLD_EVENT, ebufp);

	/* Creating duplicate of input flist*/
	dup_i_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	
	if (commission_rule != NULL) 
	{
	     switch(*commission_rule) 
	      {
		case ACTIVE_CUST_COUNT_BASED_COMMISSION:
			event_objp = PIN_POID_CREATE(db, "/event/billing/settlement/ws/outcollect/active_cust_count_based",-1,ebufp);
			custom_info_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_CUSTOM_INFO, 0, ebufp);
			PIN_FLIST_SUBSTR_SET(comm_event_flistp, custom_info_flistp, PIN_FLD_CUSTOM_INFO, ebufp);
		break;
		case CHARGE_HEAD_BASED_COMMISSION:
			event_objp = PIN_POID_CREATE(db, 
				"/event/billing/settlement/ws/outcollect/charge_head_based", -1, ebufp);
			charge_info_flistp = PIN_FLIST_SUBSTR_GET(dup_i_flistp, MSO_FLD_CHARGE_HEAD_INFO, 0, ebufp);
			if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_SERVICE_OBJ, 1,ebufp))
			    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		//	if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_ACCOUNT_OBJ, 1,ebufp))
		//	    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_ITEM_TYPE, 1, ebufp))
			    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_ITEM_TYPE, ebufp);
			if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_SESSION_OBJ, 1, ebufp))
			    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_SESSION_OBJ, ebufp);
			if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp))
			    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
			if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_TOTALS, 1, ebufp))
			    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_TOTALS, ebufp);
			if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_TAX, 1, ebufp))
			    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_TAX, ebufp);
			if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_AMOUNT, 1, ebufp))
			    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_AMOUNT, ebufp);
		//	if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_POID, 1, ebufp))
		//	    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_POID, ebufp);
			if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_ITEM_TOTAL, 1, ebufp))
			    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_ITEM_TOTAL, ebufp);
			if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_DUE, 1, ebufp))
			    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_DUE, ebufp);
			if(PIN_FLIST_FLD_GET(charge_info_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp))
			    PIN_FLIST_FLD_DROP(charge_info_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
			PIN_FLIST_SUBSTR_SET(comm_event_flistp, charge_info_flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp);
		break;
             }
             PIN_FLIST_FLD_SET(comm_flistp, PIN_FLD_POID, event_objp, ebufp);
             PIN_FLIST_FLD_PUT(comm_event_flistp, PIN_FLD_POID, event_objp, ebufp);
        }

	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "debug4");
	if ( commission_for && *commission_for == ACCT_TYPE_LCO ) {
        	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_OBJ, comm_event_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_OUTCOLLECT_SERV_OBJ, comm_event_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		descr = "LCO Commission";
		strcpy(err_descr, "LCO ");
	}
        else if ( commission_for && *commission_for == ACCT_TYPE_DTR ) {
        	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DT_OBJ, comm_event_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, comm_event_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		descr = "DTR Commission";
		strcpy(err_descr, "DTR ");
	}
        else if ( commission_for && *commission_for == ACCT_TYPE_SUB_DTR ) {
        	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SDT_OBJ, comm_event_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SDT_OUTCOLLECT_SERV_OBJ, comm_event_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		descr = "SDT Commission";
		strcpy(err_descr, "SDT ");
	}
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SESSION_OBJ, comm_event_flistp, PIN_FLD_SESSION_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EFFECTIVE_T, comm_event_flistp, PIN_FLD_START_T, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EFFECTIVE_T, comm_event_flistp, PIN_FLD_END_T, ebufp);

        //PIN_FLIST_FLD_SET(comm_event_flistp,PIN_FLD_PROGRAM_NAME, descr, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, comm_event_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
        PIN_FLIST_FLD_SET(comm_event_flistp,PIN_FLD_SYS_DESCR, descr, ebufp);
        PIN_FLIST_FLD_SET(comm_event_flistp,PIN_FLD_DESCR, descr, ebufp);
        PIN_FLIST_FLD_SET(comm_event_flistp,PIN_FLD_NAME, descr, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "debug5");
	/* Prepare the input flist to apply the commission charge - End */

	fm_mso_negative_qty_modify(ctxp, comm_flistp, NULL, ebufp);
	/* Call the ACT_USAGE to apply the commission - Start */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_apply commission act_usage input flist", comm_flistp);
	PCM_OP(ctxp, PCM_OP_ACT_USAGE, flags, comm_flistp, &r_comm_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_apply commission act_usage output flist", r_comm_flistp);

	//Prepare the result flist 
	PIN_ERRBUF_CLEAR(ebufp);
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(r_comm_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);

	result_flistp = PIN_FLIST_ELEM_GET(r_comm_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_EVENT_OBJ, ebufp);
	rating_status = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_RATING_STATUS, 1, ebufp);

	if(rating_status != NULL) {
	    switch (*rating_status) {
		case SUCCESSFUL_RATING: 
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_RATING_STATUS, &rating_success, ebufp);
			bal_flistp = PIN_FLIST_ELEM_GET(result_flistp, PIN_FLD_BAL_IMPACTS, 0, 0, ebufp);
			PIN_FLIST_FLD_COPY(bal_flistp, PIN_FLD_AMOUNT, *r_flistpp, PIN_FLD_AMOUNT, ebufp);
			if (flags != 0) {
			    strcat(err_descr, "COMMISSION - Commission calculated successfully");
			    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, err_descr, ebufp);
			}
			else 
			{
			    strcat(err_descr, "COMMISSION - Commission applied successfully");
			    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, err_descr, ebufp);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, err_descr);
		break;
		case NO_INITIAL_PRODUCTS:
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_RATING_STATUS, rating_status, ebufp);
		        strcat(err_descr, "COMMISSION - Commission Products were not available this period");
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, err_descr, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, err_descr);
		break;
		case NO_MATCHING_SELECTOR_DATA: 
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_RATING_STATUS, rating_status, ebufp);
			strcat(err_descr, "COMMISSION - Charge Head is not applicable for commission");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, err_descr, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, err_descr);
			
		break;
                default:
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_RATING_STATUS, rating_status, ebufp);
			strcat(err_descr, "");
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Commission Failed", ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, err_descr);
	    }
	}
	/* Call the ACT_USAGE to apply the commission - End */

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_apply_commission output flist", *r_flistpp);

cleanup:
	PIN_FLIST_DESTROY_EX(&flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&comm_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_comm_flistp, NULL);
        return;
}




/*******************************************************************
 * This function will update the -ve quantity to +ve quantity
 *******************************************************************/
static void
fm_mso_negative_qty_modify (
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)	
{

	pin_flist_t		*charge_head_info = NULL;
	pin_flist_t		*event_flist = NULL;

	//int32			flag = 0;
	int64			db =1;
	int32			*comm_ref_flagp = NULL;

	poid_t			*session_obj = NULL;

	pin_decimal_t		*collection_amt = NULL;
	pin_decimal_t		*negative_collection_amt = NULL;
//	pin_decimal_t		*decimal_zero = pbo_decimal_from_str("0.0",ebufp);

	char			*session_obj_type = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	        return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_negative_qty_modify input flist", i_flistp);

	/* Read the required data from the input flist - Start */
	db = PIN_POID_GET_DB(PCM_GET_USERID(ctxp));

	event_flist = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EVENT, 1, ebufp);
	if (event_flist)
	{
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "000");
		session_obj      = PIN_FLIST_FLD_GET(event_flist, PIN_FLD_SESSION_OBJ, 1, ebufp);
		charge_head_info = PIN_FLIST_SUBSTR_GET(event_flist, MSO_FLD_CHARGE_HEAD_INFO, 1, ebufp);
		// Check FLAGS field for commission reversal in case of cycle refund
		comm_ref_flagp = PIN_FLIST_FLD_GET(charge_head_info, PIN_FLD_FLAGS, 1, ebufp);
		if (charge_head_info)
		{
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "111");
			collection_amt = PIN_FLIST_FLD_GET(charge_head_info, MSO_FLD_COLLECTION_AMT, 1, ebufp);
			if (session_obj)
			{
				//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "222");
				session_obj_type = (char*)PIN_POID_GET_TYPE(session_obj);
			}
			if ((session_obj_type && strcmp(session_obj_type, "/event/billing/batch/reversal")==0) ||															(comm_ref_flagp != NULL && *comm_ref_flagp == 1))
			{
				//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "charge_head_info", charge_head_info);
				if ( collection_amt && 
				     !pbo_decimal_is_null(collection_amt,ebufp) && 
				     pbo_decimal_sign(collection_amt, ebufp) < 0
				   )
				{
					//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "444");
					negative_collection_amt = pbo_decimal_negate(collection_amt, ebufp);
					PIN_FLIST_FLD_SET(charge_head_info, MSO_FLD_COLLECTION_AMT, negative_collection_amt, ebufp);
					if (!pbo_decimal_is_null(collection_amt,ebufp))
					{
						pbo_decimal_destroy(&negative_collection_amt);
					}
					
//					if (!pbo_decimal_is_null(decimal_zero,ebufp))
//					{
//						pbo_decimal_destroy(&decimal_zero);
//					}
				}
			}
		}
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_negative_qty_modify final flist", i_flistp);

cleanup:
	return;
}
