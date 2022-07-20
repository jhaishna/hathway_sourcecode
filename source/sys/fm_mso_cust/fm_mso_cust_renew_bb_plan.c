/*******************************************************************
 * File:        fm_mso_cust_renew_bb_plan.c
 * Opcode:      MSO_OP_CUST_RENEW_BB_PLAN 
 * Owner:       Leela Pratyush
 * Created:     05-OCT-2014
 * Description: This opcode is invoked for doing renew of existing prepaid plan before expiry date

        r << xx 1
        0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 2431536 0
        0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 2431536 0
        0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 980936 8
        0 PIN_FLD_PROGRAM_NAME    STR [0] "top_up_plan"
        0 PIN_FLD_INDICATOR             INT [0] 2
        0 MSO_FLD_TECHNOLOGY    ENUM [0] 2
        0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 2427945 0
        0 PIN_FLD_PLAN          ARRAY [0] allocated 20, used 5
        1     PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 2427337 2
        1     MSO_FLD_PLAN_NAME       STR [0] "STB_Zero_Value"
        1     PIN_FLD_ACTION          STR [0] "ADD"
        1     PIN_FLD_PRODUCTS      ARRAY [0] allocated 20, used 5
        2         PIN_FLD_PRODUCT_OBJ    POID [0] 0.0.0.1 /product 2420378 2
        2         PIN_FLD_PURCHASE_END_T TSTAMP [0] (1414657000) Wed Apr 30 00:00:00 2014
        2         PIN_FLD_PURCHASE_START_T TSTAMP [0] (1411965861) Wed Mar 12 00:00:00 2014
        2         MSO_FLD_DISC_AMOUNT  DECIMAL [0] NULL
        2         MSO_FLD_DISC_PERCENT DECIMAL [0] NULL
        xx
        xop 11210 0 1

        xop: opcode 11210, flags 0
        # number of field entries allocated 20, used 4
        0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
        0 PIN_FLD_STATUS         ENUM [0] 0
        0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 12345 1
        0 PIN_FLD_DESCR           STR [0] "Renew plan completed successfully"

 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_renew_bb_plan.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bal.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "fm_bal.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_ntf.h"
#include "mso_lifecycle_id.h"


#define CORPORATE_PARENT 1
#define CORPORATE_CHILD  2
#define MSO_ACTIVE_STATUS    10100
#define MSO_INACTIVE_STATUS  10102
#define MSO_PREPAID           2
#define MSO_POSTPAID          1

/**************************************
* External Functions start
***************************************/
extern int32
fm_mso_trans_open(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        int32           flag,
        pin_errbuf_t    *ebufp);

extern void 
fm_mso_trans_commit(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

extern void  
fm_mso_trans_abort(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

extern int 
fm_mso_validate_csr_role(
        pcm_context_t           *ctxp,
        int64                   db,
        poid_t                  *user_id,
        pin_errbuf_t            *ebufp);

extern
void fm_mso_create_lifecycle_evt(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *out_flistp,
        int32                   action,
        pin_errbuf_t            *ebufp);

static int
fm_mso_bb_renew(
        cm_nap_connection_t     *connp,
        pin_flist_t             *in_flist,
        pin_flist_t             **bb_ret_flistp,
        pin_flist_t             *lc_out_flist,
        pin_errbuf_t            *ebufp);

static int fm_mso_cust_call_prov(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp);

static int fm_mso_cust_call_notif(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp);
                
int fm_mso_update_service_status(
                pcm_context_t           *ctxp,
                pin_flist_t             *in_flist,
                int32                                   status,
                pin_errbuf_t            *ebufp);

extern int
fm_quota_check(
        pcm_context_t                   *ctxp,
        char                            *plan_name,
        char                            *prov_tag,
        int                             *tech,
        int                             *plan_typ,
        pin_decimal_t                   **initial_amount,
        int                             *error_codep,
        pin_errbuf_t                    *ebufp);

/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/
EXPORT_OP int 
fm_mso_add_deal(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_RENEW_BB_PLAN 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_renew_bb_plan(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_cust_renew_bb_plan(
        cm_nap_connection_t     *connp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

/*********
 * Functions added for Plan renewal 
 *********/
extern void
fm_mso_reactivate_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_purchase_bb_plans(
        cm_nap_connection_t     *connp,
        pin_flist_t             *mso_act_flistp,
        char                    *prov_tag,
        int                     flags,
	int			grace_flag,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

extern int
fm_mso_update_mso_purplan(
        pcm_context_t           *ctxp,
        poid_t                  *mso_pp_obj,
        int                     new_status,
        char                    *descr,
        pin_errbuf_t            *ebufp);

extern int
fm_mso_update_ser_prov_status(
        pcm_context_t           *ctxp,
        pin_flist_t                     *in_flistp,
        int                             prov_status,
        pin_errbuf_t            *ebufp);

static
int32
get_diff_time(
        pcm_context_t           *ctxp,
        time_t          timeStamp,
        time_t          timeStamp2,
        pin_errbuf_t            *ebufp);

static int 
fm_mso_renew_amc_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp );

extern int32
fm_mso_utils_discount_validation(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp);
/*******************************************************************
 * MSO_OP_CUST_RENEW_BB_PLAN  
 *******************************************************************/
void 
op_mso_cust_renew_bb_plan(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;

        *r_flistpp              = NULL;
        int                     local = LOCAL_TRANS_OPEN_SUCCESS;
        int                     *status = NULL;
        int32                   *renew_flag = NULL;
        int32                   status_uncommitted =2;
        poid_t                  *inp_pdp = NULL;
        char            *prog_name = NULL;
		void			*vp = NULL;
        
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /*******************************************************************
         * Insanity Check 
         *******************************************************************/
        if (opcode != MSO_OP_CUST_RENEW_BB_PLAN) {
                pin_set_err(ebufp, PIN_ERRLOC_FM, 
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_cust_renew_bb_plan error",
                        ebufp);
                return;
        }

        /*******************************************************************
         * Debug: Input flist 
         *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
                "op_mso_cust_renew_bb_plan input flist", i_flistp);
        
        prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
        renew_flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
//        if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
        if (!(prog_name && strstr(prog_name,"pin_deferred_act")))
        {
                inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
                local = fm_mso_trans_open(ctxp, inp_pdp, 3,ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
                        return;
                }
                if ( local == LOCAL_TRANS_OPEN_FAIL )
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
                        return;
                }
        }
        else
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'pin_deferred_act' so transaction will not be handled at API level");
        }
        /*
         * If flag then it is renewal call
         */
        if((renew_flag) && (*renew_flag == 1))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " This is renewal plan ");
                fm_mso_reactivate_plan(ctxp, i_flistp, r_flistpp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Return flist final is ", *r_flistpp);
        }
        else
        {
                fm_mso_cust_renew_bb_plan(connp, flags, i_flistp, r_flistpp, ebufp);
        }

        if (*r_flistpp)
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Renewal completed ");
            status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
        }
        else
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_renew_bb_plan: input flist", i_flistp);
        }
       

        /***********************************************************
         * Results.
         ***********************************************************/
        if (PIN_ERRBUF_IS_ERR(ebufp) || *r_flistpp == NULL || (status && *(int*)status == 1)) 
        {   
            if((prog_name && strstr(prog_name,"pin_deferred_act")))
            {
                     pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);
			        return;

        	}        
            if (PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_renew_bb_plan error", ebufp);
                PIN_ERRBUF_RESET(ebufp);
            }
		
            if(local == LOCAL_TRANS_OPEN_SUCCESS 
                    && (!(prog_name && strstr(prog_name,"pin_deferred_act"))))
            {
                fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
            }
            else
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT aborted");
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		    }
        }
        else
        {
                if(local == LOCAL_TRANS_OPEN_SUCCESS && 
					(!(prog_name && strstr(prog_name,"pin_deferred_act")))
                  )
                {
						/* If PIN_FLD_MODE (Calc Only flag) is 1 then abort the transaction.*/
						vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 1, ebufp );
						if(vp && *(int32 *)vp == 1) {
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Success and Price Calc Flag = 1. Abort the Trans.");
							fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
						}
						else {
							fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
						}
                }
                else
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
                }               
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_cust_renew_bb_plan output flist", *r_flistpp);
        return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_renew_bb_plan(
        cm_nap_connection_t     *connp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *success_flistp = NULL;
        pin_flist_t             *acnt_info = NULL;
        pin_flist_t             *lc_out_flist = NULL;

        poid_t                  *routing_poid = NULL;
        poid_t                  *account_obj = NULL;
        poid_t                  *service_obj = NULL;
        poid_t                  *user_id = NULL;
        char			bb_service_type[]="/service/telco/broadband"; 
        int32                   account_type      = -1;
        int32                   account_model     = -1;
        int32                   subscriber_type   = -1;
        int32                   add_plan_success = 0;
        int32                   add_plan_failure = 1;
        int32                   *over_flagp = NULL;
        int32                   override_flag = 1;
	int32			topup_failure_t = 1;
        int64                   db = -1;
        int                     csr_access = 0;
        int                     acct_update = 0;
        int                     ret_status = 0;
	pcm_context_t           *ctxp = connp->dm_ctx;
        
        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_renew_bb_plan input flist", i_flistp);
        
        routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
        service_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
        if (routing_poid)
        {       
                db = PIN_POID_GET_DB(routing_poid);
                account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
                user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp); 
        }
        else
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51412", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
                goto CLEANUP;
        }
        if (!account_obj || !service_obj )
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51413", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account POID value not passed in input as it is mandatory field", ebufp);
                goto CLEANUP;
        }

        if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
        {
                goto ROLE;              
        }
        //subscriber_type = fm_mso_get_parent_info(ctxp, i_flistp, account_obj, &acnt_info, ebufp);
        if (ret_flistp)
        {
                ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
                if (ret_status == add_plan_failure)
                {
                        goto CLEANUP;
                }
        }

        /*******************************************************************
        * Validation for PRICING_ADMIN roles - Start
        *******************************************************************/
ROLE:
        if ( user_id && user_id == account_obj )
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Topup from Self Care"); 
        }
        else if (user_id)
        {
                csr_access = fm_mso_validate_csr_role(ctxp, db, user_id, ebufp);

                if ( csr_access == 0)
                {               
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51414", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ROLE passed in input does not have permissions to change the service status", ebufp);
                        goto CLEANUP;
                }else 
                {               
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR has permission to change account information");    
                }
        }
        else
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51415", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
                goto CLEANUP;
        }

        /*******************************************************************
        * Validation for CSR roles  - End
        *******************************************************************/
       over_flagp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OVERRIDE_FLAGS, 1, ebufp);
       if (over_flagp && *over_flagp == 1)
       {
            override_flag = 0;
       }
	/******************************************************************
	* INITIAL CHECK TO AVOID TRIAL PLAN REACTIVATION
	******************************************************************/
	if (override_flag && strncmp(PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp)),bb_service_type,strlen(bb_service_type))==0)
	{
	char  		    *event_typ=NULL;
	time_t 	    *exact_created_t =NULL;
	poid_t  	    *plan_pdp=NULL;
	poid_t		    *service_pdp=NULL;
	poid_t		    *act_pdp=NULL;
	pin_flist_t	    *event_type_flistp = NULL;
	pin_flist_t	    *subscr_prods_flistp = NULL;
	pin_flist_t	    *usage_flistp = NULL;
	pin_flist_t	    *last_plan_flistp = NULL;
	time_t		current_t = pin_virtual_time(NULL);
	int32		    *days_threshold = NULL;
	int32		    err = 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ENTERING TRIAL PLAN VALIDATION FOR RENEWAL");
	pin_conf("fm_mso_cust", "days_threshold", PIN_FLDT_INT, (caddr_t*)&days_threshold, &err );
	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);		
	fm_mso_get_event_from_plan(ctxp,plan_pdp,&event_type_flistp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "NEW PLAN EVENT TYPE FOR VALIDATION OF TRIAL IN RENEWAL", event_type_flistp);
	if (event_type_flistp && event_type_flistp != NULL && PIN_FLIST_ELEM_COUNT(event_type_flistp, PIN_FLD_RESULTS, ebufp) > 0)
	{
		last_plan_flistp = PIN_FLIST_ELEM_GET(event_type_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		usage_flistp = PIN_FLIST_ELEM_GET(last_plan_flistp, PIN_FLD_USAGE_MAP, 0, 1, ebufp);
		event_typ = PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
	}	
	if (event_typ && strstr(event_typ, "trial")!=NULL && (strlen(strstr(event_typ, "trial"))>0))
	{	
		fm_mso_get_subscr_purchased_products(ctxp, act_pdp, service_pdp, &subscr_prods_flistp, ebufp);
		if (subscr_prods_flistp && subscr_prods_flistp != NULL && PIN_FLIST_ELEM_COUNT(subscr_prods_flistp, PIN_FLD_RESULTS, ebufp) > 0)
              {
			last_plan_flistp = PIN_FLIST_ELEM_GET(subscr_prods_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			exact_created_t = PIN_FLIST_FLD_GET(last_plan_flistp, PIN_FLD_CYCLE_START_T, 1, ebufp);
		if(((fm_utils_time_round_to_midnight(current_t) - *exact_created_t)/60/60/24) < *(int32*)days_threshold)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validation Successful.");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &topup_failure_t, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51490", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Sorry Cannot Renew Trial Plan.", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ERROR FLIST DEFINITION", ret_flistp);
			*r_flistpp = ret_flistp;
			return;
		}
		else
 			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Renewal is for a Trial Plan and 30 days have crossed hence Avoiding Validation.");
		}
		else
 			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Old Subscription Plan Found in Renewal hence Avoiding Validation.");
	}
	else 
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Renewal is Not for a Trial Plan hence Avoiding Validation.");
	
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Some Unhandled Data Exception has Occured in Initial Validation for BB Customer");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Initial Validations", ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &topup_failure_t, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51491", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Unhandled Data Exception in Initial Validation.", ebufp);
		*r_flistpp = ret_flistp;
		return;
		}
	}
 	/***********************************************************************
	* END OF INITIAL CHECK TO AVOID TRIAL PLAN REACTIVATION
	************************************************************************/

	/*******************************************************************
        * Topup plan - Start
        *******************************************************************/

        lc_out_flist = PIN_FLIST_CREATE(ebufp);
        acct_update = fm_mso_bb_renew(connp, i_flistp, &ret_flistp, lc_out_flist, ebufp);

        if ( acct_update == 0)
        {               
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Topup failed");
                *r_flistpp = ret_flistp;
                goto CLEANUP;
        }
        else if ( acct_update == 1)
        {               
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_renew_bb_plan add plan successful");
                fm_mso_create_lifecycle_evt(ctxp, i_flistp, lc_out_flist, ID_RENEW_PREPAID, ebufp );
        }
        else if ( acct_update == 2)
        {               
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_renew_bb_plan insufficient data provided");   
                *r_flistpp = ret_flistp;
                goto CLEANUP;
        }
		else if ( acct_update == 3)
		{		
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_renew_bb_plan CALC ONLY successful");	
				*r_flistpp = ret_flistp;
				goto CLEANUP;
		}
        
        /*******************************************************************
        * Topup plan  - End
        *******************************************************************/
        ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, ret_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_success, ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "Topup/renewal completed", ebufp);
                //*r_flistpp = success_flistp;
        //PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
CLEANUP:
        PIN_FLIST_DESTROY_EX(&lc_out_flist, NULL);
        *r_flistpp = ret_flistp;
        return;
}

static int
fm_mso_bb_renew(
	      cm_nap_connection_t     *connp,
        pin_flist_t             *in_flist,
        pin_flist_t             **bb_ret_flistp,
        pin_flist_t             *lc_out_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *dealinfo_flistp = NULL;
        pcm_context_t           *ctxp = connp->dm_ctx;
        pin_flist_t             *deals_flistp = NULL;
        pin_flist_t             *products_flistp = NULL;
        pin_flist_t             *statuschange_inflistp = NULL;
        pin_flist_t             *statuschange_outflistp = NULL;
        pin_flist_t             *read_flistp = NULL;
        pin_flist_t             *read_res_flistp = NULL;
        pin_flist_t             *status_flistp = NULL;
        pin_flist_t             *provaction_inflistp = NULL;
        pin_flist_t             *provaction_outflistp = NULL;
        pin_flist_t             *plan_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *results_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *return_flistp = NULL;
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *prov_res_flistp = NULL;
        pin_flist_t             *plan_in_flistp = NULL;
        pin_flist_t             *prov_in_flistp = NULL;
        pin_flist_t             *pur_return_flistp = NULL;
        pin_flist_t             *read_bal_flistp = NULL;
        pin_flist_t             *read_bal_res_flistp = NULL;
        pin_flist_t             *bal_flistp = NULL;
        pin_flist_t             *set_in_flistp = NULL;
        pin_flist_t             *set_res_flistp = NULL;
        pin_flist_t             *sub_bal_flistp = NULL;
        pin_flist_t             *temp_in_flistp = NULL;
        pin_flist_t             *read_pp_flistp = NULL;
        pin_flist_t             *read_pp_res_flistp = NULL;
        pin_flist_t             *lc_sub_bal_array = NULL;
	pin_flist_t             *mso_pp_flistp  = NULL;
	pin_flist_t             *read_in_flistp  = NULL;
	pin_flist_t             *read_out_flistp  = NULL;
	pin_flist_t             *tmp_flistp  = NULL;
        
        poid_t                  *service_obj = NULL;
        poid_t                  *bal_grp_obj = NULL;
        poid_t                  *plan_obj = NULL;
        poid_t                  *mso_pp_obj = NULL;
        poid_t                  *mso_plan_obj = NULL;
        poid_t                  *account_obj = NULL;
        poid_t                  *product_obj = NULL;
        poid_t                  *offering_obj = NULL;
        poid_t                  *pp_obj = NULL;
	poid_t			*deact_mso_pp_obj = NULL;

        char                     msg[512] ;
        char                     *tmp_str = NULL ;
        char                     *prov_tag = NULL ;
	char			 *temp_tag = NULL ;

        char                     *search_template = "select X from /mso_customer_credential where F1 = V1 ";
	char			*search_prt_template1 = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 order by F4 desc ";
        char                     *search_prt_template2 = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3";
	char			*search_prt_template3 = "select X from /mso_purchased_plan where F1 = V1 ";
        
        int                     res_id = 0;
        int                     bal_flags = 0;
        int                     counter = 0;
        int                     elem_id = 0;
        int                     elem_id2 = 0;
        int                     elem_base = 0;
        int                     bal_elem_id = 0;
        int                     sub_bal_id = 0;
        int                     prty = 0;
        int                     cntr = 0;
        int                     plan_status = 0;
        int                     prov_result = 0;
        int                     notif_result = 0;
        int64                   diff = 0;
        int32                   *prov_status = NULL;
        int                     set_err = 0;
        int                     plan_count = 0;
        int                     renew_extend = 0;
        int                     prov_counter = 0;
        int                     status_change_failure = 1;
        int32                   srch_flag = 512;
        int32                   diff2 = 0;
        int32                   db = 1;
        int32                  *customer_type = NULL;
        int32                  *indicator = NULL;
        int                     *ret_status = NULL;
        int32                   *status = NULL;
        int32                   *cancel_status = NULL;
        int32                   ret_val = -1;

        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie2 = NULL;
        pin_cookie_t            res_cookie = NULL;
        pin_cookie_t            bal_cookie = NULL;
        pin_cookie_t            sub_cookie = NULL;
        pin_cookie_t            cookie_base = NULL;

        pin_decimal_t          *priority = NULL;
        pin_decimal_t          *plan_priority = NULL;
        pin_decimal_t          *plan_closed = NULL;

        time_t                  *pp_end_t = NULL;
        time_t                  current_t = 0;
        time_t                  *plan_clsd_t = NULL;
        time_t                  *valid_end_t = NULL;
		time_t			*pp_cyc_end_t = NULL;
        void                    *vp = NULL;
	int32			price_calc_flag = 0;
	int32			subs_plan_type = 1;
	int32			add_plan_success = 0;
	int		        elem_id_r = 0;
        pin_cookie_t		cookie_r = NULL;
        pin_flist_t    	*bal_imp = NULL;
        int32          	*bal_res = NULL;
	char		*plan_name = NULL;
	int		*tech = NULL;
	int		plan_type = 0;
	int		opcode = MSO_OP_CUST_RENEW_BB_PLAN;
	pin_decimal_t	*initial_amount = NULL;
	int32               error_codep = 0;
	pin_flist_t     *read_mso_plan_oflistp = NULL;
        pin_flist_t     *read_mso_plan_flistp = NULL;
        pin_flist_t     *prod_flistp = NULL;
	int		rec_id_p = 0; 	
	pin_cookie_t	cookie_p = NULL;
	pin_flist_t     *rfld_iflistp = NULL;
        pin_flist_t     *rfld_oflistp = NULL;
        pin_flist_t     *temp_flistp = NULL;
	int		*fup_status = NULL;
	char             tmp[200]="";
	int32                   ret_vall = 0;
        if (PIN_ERRBUF_IS_ERR(ebufp))
	{
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_bb_renew error", ebufp);
                return 0;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan :");
        service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
        account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );     
        
	if ( !service_obj || !account_obj || (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) != 0 )
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51413", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Incorrect POID value passed in input", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
		/* Get the value of PIN_FLD_MODE (Calc Only flag) */
		vp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_MODE, 1, ebufp );
		if(vp && *(int32 *)vp == 1) {
			price_calc_flag = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
		}
		
		/* Read service status */
        read_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, service_obj, ebufp);
        PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_STATUS_FLAGS, NULL, ebufp);
        PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(read_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_renew read service input list", read_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - PCM_OP_READ_FLDS of service poid error", ebufp);

                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_renew read service output flist", read_res_flistp);

        bal_grp_obj = PIN_FLIST_FLD_TAKE(read_res_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
        status = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_STATUS, 1, ebufp );
        args_flistp = PIN_FLIST_ELEM_GET(read_res_flistp, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp );
        prov_status = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATUS, 1, ebufp );
	
	/* Validate service status */
        if ( *(int32 *)status != MSO_INACTIVE_STATUS || *(int32 *)prov_status == MSO_PROV_ACTIVE )
        {
                PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51616", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service is not active or Provisioning status is still active - Cannot renew plan", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
	
	/* Update service status to active - Existing status is Inactive  */
        ret_val = fm_mso_update_service_status(ctxp, in_flist, MSO_ACTIVE_STATUS, ebufp);
        if(ret_val!=0)
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51617", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service Status update failed - Cannot renew plan", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }


    PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
	
	/* Validate that plan poid exists and is not NULL */
	plan_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PLAN_OBJ, 1, ebufp);	
        if ( !plan_obj || plan_obj == (poid_t *)NULL )
        {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No plan to purchase in input flist");
                PIN_ERRBUF_RESET(ebufp);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51633", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Plan Poid missing in input flist", ebufp);
                *bb_ret_flistp = ret_flistp;
		return 0;
        }

	/* Check if there is any Active plan for plan poid passed in input.
		It must not return any active plan since renew is only done when 
		the plan is de-active due to Validity Expiry or Quota exhaust */
        plan_status = 2;
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_prt_template2, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &plan_status, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRIORITY, (int32 *)NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "product type search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp) )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51645", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching /mso_purchased_plan for this account", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plan search output flist", srch_out_flistp);    
        results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

	/* If there is an active plan then return. Renew cannot be done */
        if ( results_flistp )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "This plan is already active for the customer", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51646", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is already active for the customer", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	/* Search all the /mso_purchased_plan with following:
		1) plan poid passed in input (to be renewed)
		2) status = 4 (which are in deactive state)
		3) order by created_t in descending order */
		
        plan_status = 4;
        db = PIN_POID_GET_DB(service_obj);
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_prt_template1, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &plan_status, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EFFECTIVE_T, (time_t *)NULL, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_MOD_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRIORITY, (int32 *)NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Getting latest closed date for the plan to be renewed", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp) )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51645", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching /mso_purchased_plan for this account", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Last updated Effective date for the plan is :", srch_out_flistp);       
        results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if ( !results_flistp )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "This plan does not exist for the customer", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51636", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "This plan does not exist for the customer", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Verifying dates");
	mso_pp_flistp = PIN_FLIST_COPY(results_flistp, ebufp);
	// Added below to fetch the plan end date from product instead of considering
	//  modified date from mso_purchased_plan
	//plan_clsd_t = PIN_FLIST_FLD_TAKE(results_flistp, PIN_FLD_MOD_T, 1, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(tmp_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_CYCLE_END_T, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased product read input list", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp) || !read_out_flistp )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51629", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in reading Purchased product for this account", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased product read output flist", read_out_flistp);
	plan_clsd_t = PIN_FLIST_FLD_TAKE(read_out_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	//Added till here

	deact_mso_pp_obj = PIN_FLIST_FLD_TAKE(results_flistp, PIN_FLD_POID, 1, ebufp);
        /*if (!pbo_decimal_is_null(plan_closed, ebufp)) {
                *plan_clsd_t = pbo_decimal_to_double(plan_closed, ebufp);
        }
        if ( plan_clsd_t )
        {
                *(time_t *)plan_clsd_t = fm_utils_time_round_to_midnight(*(time_t *)plan_clsd_t);
        }*/

	/*added changes for fup carry forward*/
	/*read flds the service to check weather it is fup or not*/
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, service_obj, ebufp);
        //Add the BB Info SUBSTR
        temp_flistp = PIN_FLIST_SUBSTR_ADD(read_in_flistp, MSO_FLD_BB_INFO, ebufp);
		
        PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_FUP_STATUS, (void *)0, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_renew: Read Service In flist", read_in_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_renew: Read Service out flist", read_out_flistp);
	
	temp_flistp = PIN_FLIST_SUBSTR_GET(read_out_flistp, MSO_FLD_BB_INFO, 0, ebufp);
        fup_status = (int *)PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_FUP_STATUS, 0, ebufp);	
	sprintf(tmp,"fup_flag %d",*fup_status);	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
        *(time_t *)plan_clsd_t = fm_utils_time_round_to_midnight(*(time_t *)plan_clsd_t);
        current_t = pin_virtual_time(NULL);
        current_t = fm_utils_time_round_to_midnight(current_t);
        if ( current_t && plan_clsd_t )
        {
                if (!( current_t  >= *plan_clsd_t ))  
                        renew_extend = 0;
                if ( ( current_t - (*plan_clsd_t) ) < 604800 ) 
                {
			if(fup_status && *fup_status == 0)
			{
                        	renew_extend = 1;                       
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Renew done with in 7 days");
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"This is an fup plan so there is no carry forward");
			}
                }
        }       
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	/* 
        plan_status = 4;
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_prt_template3, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, deact_mso_pp_obj, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRIORITY, (int32 *)NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso purchased plan search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp) )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51645", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching /mso_purchased_plan for this account", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plan search output flist", srch_out_flistp);    
        results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	*/
        /*if ( results_flistp )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "This plan is already active for the customer", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51646", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is already active for the customer", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }*/

	plan_priority = PIN_FLIST_FLD_GET(mso_pp_flistp, PIN_FLD_PRIORITY, 1, ebufp);
        if ( !plan_priority || plan_priority == (pin_decimal_t *)NULL )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan type is not updated in /mso_purchased_plan for this plan", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51635", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Plan type is not updated in /mso_purchased_plan for this plan", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        
	args_flistp = PIN_FLIST_ELEM_GET(mso_pp_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);
        if ( args_flistp )
        {
                pp_obj = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 1, ebufp);       
        }
        if ( !pp_obj || PIN_POID_IS_NULL(pp_obj) != 0 )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No purchased product is associated for this plan", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51638", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No purchased product is associated for this plan", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
	/* Read the Cycle end date for purchased product */ 
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, pp_obj, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_CYCLE_END_T, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased product read input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp) || !srch_out_flistp )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51629", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in reading Purchased product for this account", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased product read output flist", srch_out_flistp);
	pp_cyc_end_t = PIN_FLIST_FLD_TAKE(srch_out_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	/* Read /mso_customer_credential to find the customer type */
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CUSTOMER_TYPE, (int32 *)NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer type search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp) || !srch_out_flistp )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51629", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching  customer type for this account", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer type search output flist", srch_out_flistp);
        results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        customer_type = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CUSTOMER_TYPE, 1, ebufp);
        if ( !customer_type || customer_type == (int32 *)NULL )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Customer type is not updated for this account", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51630", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Customer type is not updated for this account", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        tmp_str = pbo_decimal_to_str(plan_priority, ebufp);
        prty = atoi(tmp_str);   
        sprintf(msg,"Product priority is : %d",prty);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
        if ( *(int32 *)customer_type == ACCT_TYPE_CORP_SUBS_BB )
        {
                if ( prty < BB_COR_PREPAID_START || prty > BB_COR_PREPAID_END )
                {
                        set_err = 1;
                        goto NOMATCH;
                }
        } 
	/******* Pavan Bellala ***********************
	Commneted cyber cafe

        else if ( *(int32 *)customer_type == ACCT_TYPE_CYBER_CAFE_BB )
        {
                if ( prty < BB_CYB_PREPAID_START || prty > BB_CYB_PREPAID_END )
                {
                        set_err = 1;
                        goto NOMATCH;
                }
        } 
	***********************************************/
        else if ( *(int32 *)customer_type == ACCT_TYPE_RETAIL_BB )
        {
                if ( prty < BB_RET_PREPAID_START || prty > BB_RET_PREPAID_END )
                {
                        set_err = 1;
                        goto NOMATCH;
                }
        } 
        else if ( *(int32 *)customer_type == ACCT_TYPE_SME_SUBS_BB )
        {
                if ( prty < BB_SME_PREPAID_START || prty > BB_SME_PREPAID_END )
                {
                        set_err = 1;
                        goto NOMATCH;
                }
        } 
NOMATCH:               
        if ( set_err == 1 )
        {                       
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Renew input",in_flist);                
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Only Subscription type plan can be renewed", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51637", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Only Subscription type plan can be renewed", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        
        temp_in_flistp = PIN_FLIST_COPY(in_flist, ebufp);
                elem_id = 0;
                cookie = NULL;  
	PIN_FLIST_FLD_SET(mso_pp_flistp, PIN_FLD_TYPE, &subs_plan_type, ebufp);
	mso_plan_obj = PIN_FLIST_FLD_GET(mso_pp_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
	while ((products_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_pp_flistp, PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) != NULL)
                {
                        product_obj = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
                        if ( !product_obj || PIN_POID_IS_NULL(product_obj) != 0 )
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Product poid is not present inside products array ", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
                                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
                                ret_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51640", ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Product poid is not present inside products array ", ebufp);
                                *bb_ret_flistp = ret_flistp;
                                return 0;
                        }
                        read_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, product_obj, ebufp);
                        PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_PRIORITY, (pin_decimal_t *)NULL, ebufp);
                        PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_PROVISIONING_TAG, (pin_decimal_t *)NULL, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_renew read product input list", read_flistp);
                        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
                        PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS for product", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
                                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
                                ret_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Renew Plan: PCM_OP_READ_FLDS of product poid error", ebufp);
                                *bb_ret_flistp = ret_flistp;
                                return 0;
                        }
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan read product output flist", read_res_flistp);
		temp_tag = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
                        PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_PURCHASE_START_T, &current_t, ebufp);
                        PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_CYCLE_START_T, &current_t, ebufp);
                        PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_USAGE_START_T, &current_t, ebufp);
                        //PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_PURCHASE_END_T, &diff2, ebufp);
                        //PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_CYCLE_END_T, &diff2, ebufp);
                        //PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_USAGE_END_T, &diff2, ebufp);

		/* If product has provisioning tag then it is a subscription products
			 so set the discount amount and discount percent passed in input */
		if(temp_tag && strlen((char *)temp_tag)>0)
                {
			prov_tag = strdup(temp_tag);
			ret_vall = fm_mso_utils_discount_validation(ctxp, in_flist, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_utils_discount_validation", ebufp);
				
                                PIN_ERRBUF_RESET(ebufp);
                                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
                                ret_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error after returning from discount validation", ebufp);
				*bb_ret_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                                return 0;
			}
			else if(ret_vall == 1)
			{
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
                                ret_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in dicount validation where either discount amount or percentage is wrong", ebufp);
                                *bb_ret_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
                                PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                                return 0;
			}
			PIN_FLIST_FLD_COPY(temp_in_flistp, MSO_FLD_DISC_PERCENT, products_flistp,MSO_FLD_DISC_PERCENT, ebufp);
			PIN_FLIST_FLD_COPY(temp_in_flistp, MSO_FLD_DISC_AMOUNT, products_flistp,MSO_FLD_DISC_AMOUNT, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Provisioning tag is :" );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, prov_tag );
        }

	}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PP flist.", mso_pp_flistp);
		PIN_FLIST_ELEM_SET(temp_in_flistp,mso_pp_flistp, PIN_FLD_PLAN, 0, ebufp);
		PIN_FLIST_FLD_SET(temp_in_flistp, PIN_FLD_OPCODE, &opcode, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_renew_plan calling purchase plans.", temp_in_flistp);
		fm_mso_purchase_bb_plans ( connp, temp_in_flistp, prov_tag, 1, 1, &return_flistp, ebufp);
        //fm_mso_purchase_bb_plans ( ctxp, temp_in_flistp, &return_flistp, ebufp);
        ret_status = PIN_FLIST_FLD_GET( return_flistp, PIN_FLD_STATUS, 1, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp) || ( ret_status && *(int32 *)ret_status == 1) )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in purchasing plan ", ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
                *bb_ret_flistp = return_flistp;
                 return 0;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_renew_plan: purchase plans return flist", return_flistp);
        /*If Renew is called in calc only mode then return from here. */
		if(price_calc_flag == 1)
		{
			goto CALC_ONLY;
		}
		
        elem_id = 0;
        cookie = NULL;
        while ((products_flistp = PIN_FLIST_ELEM_GET_NEXT(return_flistp, PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                offering_obj = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
                if ( offering_obj )
                {
                        read_pp_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_POID, offering_obj, ebufp);
                        PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_PURCHASE_END_T, (time_t *)NULL, ebufp);
                        PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_CYCLE_END_T, (time_t *)NULL, ebufp);
                        PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_USAGE_END_T, (time_t *)NULL, ebufp);
                        PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "Reading Purchased product end dates for this plan input", read_pp_flistp);
                        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_pp_flistp, &read_pp_res_flistp, ebufp);
                        PIN_FLIST_DESTROY_EX(&read_pp_flistp, NULL);
                        if (PIN_ERRBUF_IS_ERR(ebufp) )
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No purchased product is associated for this plan", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
                                PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
                                ret_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51638", ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No purchased product is associated for this plan", ebufp);
                                *bb_ret_flistp = ret_flistp;
                                return 0;
                        }
                        PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "Reading Purchased product end dates for this plan output", read_pp_res_flistp);
                        pp_end_t = PIN_FLIST_FLD_GET(read_pp_res_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
                }
        }

/**************************************************************/
	if(renew_extend == 1)
	{
        diff = get_diff_time( ctxp, current_t, *pp_end_t, ebufp);
        diff2 = (diff)*86400;
        read_bal_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
        PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_POID, account_obj, ebufp);
		/* Subtract 1 second from purchase product cycle end date and pass this
			in bal get balances so that the last expired sub balance is also retunred */
		*pp_cyc_end_t = *pp_cyc_end_t - 1;
		PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_END_T, pp_cyc_end_t, ebufp);
        bal_flags = 1;
        PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_FLAGS, &bal_flags, ebufp);
        PIN_FLIST_ELEM_SET(read_bal_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity read balance group input list", read_bal_flistp);
        PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES , 0, read_bal_flistp, &read_bal_res_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&read_bal_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in reading Balance group info", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51621", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in reading Balance group info", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        bal_elem_id = 0;
        bal_cookie = NULL;
        while ((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(read_bal_res_flistp, PIN_FLD_BALANCES, &bal_elem_id, 1, &bal_cookie, ebufp)) != NULL)
        {
                if (  bal_elem_id != MSO_FREE_MB && bal_elem_id != MSO_FUP_TRACK )
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Currency resource\n");
                        continue;
                }
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Non Currency resource\n");
                set_in_flistp = PIN_FLIST_CREATE(ebufp);
                counter = PIN_FLIST_ELEM_COUNT(bal_flistp, PIN_FLD_SUB_BALANCES, ebufp);
                if ( counter < 1 )
                        continue;
                sub_bal_id = 0;
                sub_cookie = NULL;
                while ((sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_SUB_BALANCES, &sub_bal_id, 1, &sub_cookie, ebufp )) != NULL)
                {
                        vp = (pin_flist_t*)PIN_FLIST_ELEM_ADD(lc_out_flistp, PIN_FLD_BALANCES, bal_elem_id, ebufp );

                        valid_end_t = NULL;
                        valid_end_t = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 1, ebufp );
                        //*valid_end_t = *valid_end_t + diff2;
                        valid_end_t = pp_end_t;
                        args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
                        PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
                        PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_TO, args_flistp, PIN_FLD_VALID_TO, ebufp );
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);

                        cntr = cntr + 1;
                        args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
                        PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, (time_t *)valid_end_t, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
                        cntr = cntr + 1;

                        lc_sub_bal_array = PIN_FLIST_COPY(args_flistp, ebufp);
                        PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_CURRENT_BAL, lc_sub_bal_array, PIN_FLD_CURRENT_BAL, ebufp );
                        PIN_FLIST_ELEM_PUT(vp, lc_sub_bal_array, PIN_FLD_SUB_BALANCES, cntr, ebufp);

                }
                if ( !args_flistp )
                        continue;

                PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_POID, account_obj, ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, set_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
                PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity bal grp change validity input list", set_in_flistp);
                PCM_OP(ctxp, PCM_OP_BAL_CHANGE_VALIDITY , 0, set_in_flistp, &set_res_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
        }
	}
        /* Update service provisioning status */
         ret_val = fm_mso_update_ser_prov_status(ctxp, in_flist, MSO_PROV_IN_PROGRESS, ebufp );
        if(ret_val)
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51754", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error while updating service provisioning status.", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;                               
        } 
        /* Pawan:13-03-2015 Updated mso_purchased_plan description with "base plan" */
        ret_val = -1;
        mso_pp_obj = PIN_FLIST_FLD_GET(return_flistp,MSO_FLD_PURCHASED_PLAN_OBJ,0, ebufp);
        ret_val = fm_mso_update_mso_purplan(ctxp, mso_pp_obj, MSO_PROV_IN_PROGRESS,"base plan", ebufp );
        if(ret_val)
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51754", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error while updating MSO purchased plan status.", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;                               
        } 
/****************************************************************/
        
        /*   Call Provisioning and Notification */
        res_id = 0;
        res_cookie = NULL;
        prov_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_POID, account_obj, ebufp);    
        PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);     
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, prov_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
        PIN_FLIST_FLD_COPY(return_flistp,MSO_FLD_PURCHASED_PLAN_OBJ,prov_in_flistp,MSO_FLD_PURCHASED_PLAN_OBJ,ebufp);
	//PIN_FLIST_ELEM_COPY(return_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, prov_in_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, ebufp);
	
	read_mso_plan_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(return_flistp,MSO_FLD_PURCHASED_PLAN_OBJ,read_mso_plan_flistp, PIN_FLD_POID,ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_renew mso_plan: Read input flist", read_mso_plan_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_mso_plan_flistp, &read_mso_plan_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_mso_plan_flistp, NULL);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_renew mso_plan: Read output flist", read_mso_plan_oflistp);
	
        while ( (prod_flistp = PIN_FLIST_ELEM_GET_NEXT (read_mso_plan_oflistp,
                       PIN_FLD_PRODUCTS, &rec_id_p, 1,&cookie_p, ebufp)) != (pin_flist_t *)NULL )
        {
                prov_tag = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
                if ( prov_tag && strlen(prov_tag) > 0)
                {
                    break;
                }
        }
	
	rfld_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID, (void *)service_obj, ebufp);
        //Add the BB Info SUBSTR
        temp_flistp = PIN_FLIST_SUBSTR_ADD(rfld_iflistp, MSO_FLD_BB_INFO, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_TECHNOLOGY, (void *)0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_renew: Read Service In flist", rfld_iflistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp, &rfld_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_renew: Read Service Out flist", rfld_oflistp);
	temp_flistp = PIN_FLIST_SUBSTR_GET(rfld_oflistp, MSO_FLD_BB_INFO, 0, ebufp);
        tech = (int *)PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);
	
	fm_quota_check(ctxp, plan_name, prov_tag, tech, &plan_type, &initial_amount, &error_codep, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_quota_check: Error in Checking quota details", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_mso_plan_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Renewal Plan - PLAN QUOTA details error", ebufp);
        
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
	
	while ((bal_imp = PIN_FLIST_ELEM_GET_NEXT(return_flistp, PIN_FLD_SUB_BAL_IMPACTS, &elem_id_r, 1, &cookie_r, ebufp)) != NULL)	
	{
		bal_res = PIN_FLIST_FLD_GET(bal_imp, PIN_FLD_RESOURCE_ID, 1, ebufp);
		if(plan_type == 1 && (bal_res && (*bal_res == MSO_FREE_MB)))
		{	
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " This is limited plan ");
			break;
		}
		else if(plan_type == 2 && (bal_res && (*bal_res == MSO_FUP_TRACK)))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " This is fup plan ");
			break;
		}
	}
	PIN_FLIST_ELEM_SET(prov_in_flistp,bal_imp,PIN_FLD_SUB_BAL_IMPACTS, elem_id_r,ebufp);	
        PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,prov_in_flistp,PIN_FLD_USERID,ebufp);
        while ((pur_return_flistp = PIN_FLIST_ELEM_GET_NEXT(return_flistp, PIN_FLD_PRODUCTS, &res_id, 1, &res_cookie, ebufp )) != NULL)
        {
                PIN_FLIST_ELEM_COPY(pur_return_flistp, PIN_FLD_PRODUCTS, res_id, prov_in_flistp, PIN_FLD_PRODUCTS, res_id, ebufp);
        }
	PIN_FLIST_FLD_COPY(return_flistp, MSO_FLD_PROVISIONABLE_FLAG, prov_in_flistp, MSO_FLD_PROVISIONABLE_FLAG, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_create_action: provisioning input list bell", prov_in_flistp);
        prov_result = fm_mso_cust_call_prov( ctxp, prov_in_flistp, ebufp);       
        PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp) || prov_result == 10 )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_mso_plan_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);
        
                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        prov_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_POID, account_obj, ebufp);    
        PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);     
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, prov_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
        PIN_FLIST_FLD_COPY(return_flistp,MSO_FLD_PURCHASED_PLAN_OBJ,prov_in_flistp,MSO_FLD_PURCHASED_PLAN_OBJ,ebufp);
    PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,prov_in_flistp,PIN_FLD_USERID,ebufp);
        //notif_result = fm_mso_cust_call_notif( ctxp, prov_in_flistp, ebufp);     
        PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp) || notif_result == 10 )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&read_mso_plan_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - MSO_OP_NTF_CREATE_NOTIFICATION error", ebufp);

                *bb_ret_flistp = ret_flistp;
                return 0;
        }
        /*   Call Provisioning and Notification */

CALC_ONLY:
	//Call AMC function
	ret_val==0;
	ret_val = fm_mso_renew_amc_plan(ctxp, in_flist, &return_flistp, ebufp );
	if(ret_val==1)
	{
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51781", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in associating AMC plan", ebufp);
                *bb_ret_flistp = ret_flistp;
                return 0;
	}

        *bb_ret_flistp = return_flistp;
        PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_mso_plan_oflistp, NULL);
        PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
		if(read_res_flistp)
		PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		if(read_pp_res_flistp)
		PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
		if(read_bal_res_flistp)
		PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);
		if(set_res_flistp)
		PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
		pbo_decimal_destroy(&initial_amount);
		
		
		if(price_calc_flag == 1)
		{
			PIN_FLIST_FLD_SET(*bb_ret_flistp, PIN_FLD_STATUS, &add_plan_success, ebufp);
			return 3;
		}
        return 1;
}

static int fm_mso_cust_call_prov(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp)
{       
        pin_flist_t     *provaction_inflistp = NULL;
        pin_flist_t     *provaction_outflistp = NULL;
        pin_flist_t     *res_flistp = NULL;

        int             status_flags = 0;       
        int             elem_id = 0;
        int             *prov_call = NULL;

        pin_cookie_t    cookie = NULL;  

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return ;
        status_flags = DOC_BB_RENEW_AEXPIRY_PRE;
        provaction_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_PROVISIONABLE_FLAG, provaction_inflistp, MSO_FLD_PROVISIONABLE_FLAG, ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(in_flist,MSO_FLD_PURCHASED_PLAN_OBJ,provaction_inflistp,MSO_FLD_PURCHASED_PLAN_OBJ,ebufp);
	PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_SUB_BAL_IMPACTS, PIN_ELEMID_ANY, provaction_inflistp, PIN_FLD_SUB_BAL_IMPACTS, 0, ebufp);
        PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

        /* Write USERID, PROGRAM_NAME in buffer - start */
        PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
        PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
        /* Write USERID, PROGRAM_NAME in buffer - end */
        while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL)
        {
                PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_RESULTS, elem_id, provaction_inflistp, PIN_FLD_RESULTS, elem_id, ebufp); 
        } 
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_create_action: provisioning input list", provaction_inflistp);
        PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
                if(provaction_outflistp)
                {
                        prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
                        if(prov_call && (*prov_call == 1))
                        {
                                //*ret_flistp = PIN_FLIST_COPY(provaction_outflistp, ebufp);
                                return 10;
                        }
                }
                else
                {
                        return 10;
                }               
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
                return 10;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_create_action: provisioning output flist", provaction_outflistp);
        PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
        return 1;
}
       
static int fm_mso_cust_call_notif(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t     *provaction_inflistp = NULL;
        pin_flist_t     *provaction_outflistp = NULL;

        int             status_flags = 0;
        int             *prov_call = NULL;      
        
        if (PIN_ERRBUF_IS_ERR(ebufp))
                return ;
        status_flags = NTF_BB_AFTER_EXPIRY;
        provaction_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
                PIN_FLIST_FLD_COPY(in_flist,MSO_FLD_PURCHASED_PLAN_OBJ,provaction_inflistp,MSO_FLD_PURCHASED_PLAN_OBJ,ebufp);
        PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_call_notif notification input list", provaction_inflistp);
        PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
                if(provaction_outflistp)
                {
                        prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
                        if(prov_call && (*prov_call == 1))
                        {
                                //*ret_flistp = PIN_FLIST_COPY(provaction_outflistp, ebufp);
                                return 10;
                        }
                }
                else
                {
                        return 10;
                }               
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
                return 10;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_call_notif notification output flist", provaction_outflistp);
        PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
        return 1;
}

static int dater(x)
{ int y=0;
    switch(x)
    {
        case 0: y=0; break;
        case 1: y=31; break;
        case 2: y=59; break;
        case 3: y=90; break;
        case 4: y=120;break;
        case 5: y=151; break;
        case 6: y=181; break;
        case 7: y=212; break;
        case 8: y=243; break;
        case 9:y=273; break;
        case 10:y=304; break;
        case 11:y=334; break;
        default: printf("Invalid Input\n\n");
    }
    return(y);
}

static int32 
get_diff_time(
        pcm_context_t   *ctxp,
        time_t          timeStamp,
        time_t          timeStamp2,
        pin_errbuf_t            *ebufp)
{
        struct tm tmStruct;
        struct tm tmStruct2;
        int32 abc = 0;
        int ref,dd1,dd2,i;
        char msg[100];

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_diff \n");

        localtime_r((const time_t *)&timeStamp, &tmStruct);
        localtime_r((const time_t *)&timeStamp2, &tmStruct2);
        if ( tmStruct2.tm_year == tmStruct.tm_year )
        {
                if ( tmStruct.tm_mon == tmStruct2.tm_mon )
                {
                        if ( tmStruct2.tm_mday < tmStruct.tm_mday )
                                return -1;
                        else if ( tmStruct2.tm_mday > tmStruct.tm_mday )
                        {
                                abc = tmStruct2.tm_mday - tmStruct.tm_mday;
                        }
                        return abc;
                }
                else if ( tmStruct.tm_mon > tmStruct2.tm_mon )
                {
                        return -1;
                }
                else if ( tmStruct2.tm_mon > tmStruct.tm_mon )
                {
                        ref = tmStruct.tm_year;
                        dd1 = 0;
                        dd2 = 0;
                        dd1 = dater(tmStruct.tm_mon);
                        if( (tmStruct2.tm_year)%4 == 0 && tmStruct.tm_mon < 2 && tmStruct2.tm_mon > 2 )
                        {
                                sprintf ( msg," i mod 4 is : %d \n",(tmStruct2.tm_year)%4 );
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Leap year ");
                                dd2 = dd2 + 1;
                        }
                        dd1 = dd1 + tmStruct.tm_mday + ((tmStruct.tm_year-ref)*365);
                        dd2 = dater(tmStruct2.tm_mon) + dd2 + tmStruct2.tm_mday + ((tmStruct2.tm_year-ref)*365);
                        abc = abs(dd2-dd1);
                        return abc;
                }
        }
        else if ( tmStruct2.tm_year < tmStruct.tm_year )
        {
                return -1;
        }
        else if ( tmStruct2.tm_year > tmStruct.tm_year )
        {
                ref = tmStruct.tm_year;
                dd1 = 0;
                dd1 = dater(tmStruct.tm_mon);
                dd2 = 0;
                dd2 = dater(tmStruct2.tm_mon);
                for( i = ref; i<tmStruct2.tm_year; i++ )
                {
                        if( i%4 == 0 )
                        {
                                sprintf ( msg," i mod 4 is : %d \n",i%4 );
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Leap year ");
                                dd2 = dd2 + 1;
                        }
                }
                sprintf ( msg," abc mod 4 is : %d %d %d \n",tmStruct.tm_mday, dd1, ((tmStruct.tm_year)-ref)*365 );
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                dd1 = dd1 + tmStruct.tm_mday + ((tmStruct.tm_year)-ref)*365;
                sprintf ( msg," abc mod 5 is : %d %d \n",tmStruct2.tm_mday, dd2 );
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

                if( (tmStruct2.tm_year)%4 == 0 && tmStruct2.tm_mon > 2 )
                {
                        sprintf ( msg," i mod 4 is : %d \n",(tmStruct2.tm_year)%4 );
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Leap year ");
                        dd2 = dd2 + 1;
                }
                sprintf ( msg," abc mod 6 is : %d %d %d \n",tmStruct2.tm_mday, dd2, (tmStruct2.tm_year-ref)*365 );
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                dd2 = dd2 + tmStruct2.tm_mday + ((tmStruct2.tm_year-ref)*365);
                abc = abs(dd2-dd1);
                return abc;
        }

        return abc;
}

/******************************************************************
* function : fm_mso_activate_service
* Change Service/Purchased_product status from inactive to active
* 
*******************************************************************/
int fm_mso_update_service_status(
                pcm_context_t           *ctxp,
                pin_flist_t             *in_flist,
                int32                                   status,
                pin_errbuf_t            *ebufp)
{               

        pin_flist_t                     *svc_in_flistp = NULL;
        pin_flist_t                     *svc_out_flistp = NULL;
        pin_flist_t                     *statuses_array = NULL;
        int32                           status_flags = 1;
        //int32                         status = MSO_ACTIVE_STATUS;
        svc_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, svc_in_flistp, PIN_FLD_POID, ebufp );
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, svc_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp );;
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, svc_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp );


        statuses_array = PIN_FLIST_ELEM_ADD(svc_in_flistp, PIN_FLD_STATUSES, 0, ebufp);
        PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS, &status, ebufp);
        PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Update Service Status input flist", svc_in_flistp);
        PCM_OP(ctxp, PCM_OP_CUST_SET_STATUS, 0, svc_in_flistp, &svc_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&svc_in_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating service status", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                return 1;
        }
        PIN_FLIST_DESTROY_EX(&svc_out_flistp, NULL);
        return 0;
}

static int 
fm_mso_renew_amc_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t	*amc_flistp = NULL;
	pin_flist_t	*amc_out_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t     *ppf_flistp  = NULL;
        pin_flist_t     *pr_ret_flistp  = NULL;
	pin_flist_t     *plan_srch_flistpp = NULL;
        pin_flist_t     *tal_pln_details = NULL;

	poid_t		*acc_obj = NULL;
	poid_t		*svc_obj = NULL;
	poid_t		*user_obj = NULL;
	poid_t          *pr_poid  = NULL;
        poid_t          *plan_pp_poid  = NULL;

	int32		amc_flag = AMC_ON_PREPAID_RENEWAL;
	int32		mode = 0;
	void		*vp = NULL;
	int32   count =0;
	int32		elem_id = 0;
	pin_cookie_t	cookie = NULL;
	char		*event_type = NULL;
	pin_decimal_t                   *pd_priority = NULL;
        char                            *strp = NULL;
        int32           prty =0;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 1;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_renew_amc_plan input flist", in_flist);
	acc_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp );
	svc_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 0, ebufp );
	user_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_USERID, 0, ebufp );
	vp =  PIN_FLIST_FLD_GET(in_flist, PIN_FLD_MODE, 1, ebufp );
	if(vp) {
		mode = *((int32 *)vp);
	}

	//Call AMC opcode
	amc_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_POID, acc_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_SERVICE_OBJ, svc_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_USERID, user_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_FLAGS, &amc_flag, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_MODE, &mode, ebufp);

	while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(*r_flistpp, PIN_FLD_RESULTS, 
			&elem_id, 1, &cookie, ebufp)) != NULL)
	{	
		  //Dont copy Tal IP  plan details for AMC
                ppf_flistp = PIN_FLIST_SUBSTR_GET(tmp_flistp, PIN_FLD_PRODUCT, 1,ebufp);
                if(ppf_flistp) {
                        pr_poid = PIN_FLIST_FLD_GET(ppf_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp );
                        fm_mso_get_poid_details(ctxp,pr_poid,&pr_ret_flistp,ebufp);
                        pd_priority = PIN_FLIST_FLD_GET(pr_ret_flistp, PIN_FLD_PRIORITY, 1, ebufp );
                        strp = pbo_decimal_to_str(pd_priority, ebufp);
                        prty = atoi(strp);
                }
                if (!( prty > 5000 && prty < 6000 )) {

		PIN_FLIST_ELEM_SET(amc_flistp, tmp_flistp, PIN_FLD_RESULTS, elem_id, ebufp);
		}
	}

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC input", amc_flistp);
        PCM_OP(ctxp, MSO_OP_CUST_BB_HW_AMC, 0, amc_flistp, &amc_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_BB_HW_AMC", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC output", amc_out_flistp);

	// If renew plan is called by price calculator then copy the results array 
	// from AMC return flist to return flist of function.
        if(mode==1)
	{
		count = PIN_FLIST_ELEM_COUNT(*r_flistpp, PIN_FLD_RESULTS, ebufp);
		cookie = NULL;
		while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(amc_out_flistp, PIN_FLD_RESULTS, 
				&elem_id, 1, &cookie, ebufp)) != NULL)
		{
			event_type = (char *) PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 1, ebufp ));
                        if(event_type && strstr(event_type, "mso_hw_amc"))
			{
				//PIN_FLIST_ELEM_SET(*r_flistpp, tmp_flistp, PIN_FLD_RESULTS, count, ebufp);
				PIN_FLIST_ELEM_COPY(amc_out_flistp,PIN_FLD_RESULTS,elem_id,*r_flistpp,PIN_FLD_RESULTS, count, ebufp);
				count++;
			}
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_renew_amc_plan return flist", *r_flistpp);
	PIN_FLIST_DESTROY_EX(&amc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&amc_out_flistp, NULL);
	return 0;
}
