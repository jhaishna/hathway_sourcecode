/*******************************************************************
 * File:	fm_mso_cust_topup_bb_plan.c
 * Opcode:	MSO_OP_CUST_TOP_UP_BB_PLAN 
 * Owner:	Leela Pratyush
 * Created:	26-AUG-2014
 * Description: This opcode is invoked for doing top-up of existing prepaid plan before expiry date

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

	xop: opcode 11006, flags 0
	# number of field entries allocated 20, used 4
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 12345 1
	0 PIN_FLD_DESCR           STR [0] "Topup plan completed successfully"

 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_topup_bb_plan.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#define CORPORATE_CHILD 2
#define MSO_ACTIVE_STATUS    10100
#define MSO_PREPAID           2
#define MSO_POSTPAID          1

/**************************************
* External Functions start
***************************************/
extern int32
fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

extern void  
fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

extern int 
fm_mso_validate_csr_role(
	pcm_context_t		*ctxp,
	int64			db,
	poid_t			*user_id,
	pin_errbuf_t		*ebufp);
	
extern int
fm_mso_update_mso_purplan(
	pcm_context_t		*ctxp,
	poid_t				*mso_pp_obj,
	int					new_status,
	char 				*descr,
	pin_errbuf_t            *ebufp);

void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

extern void mso_get_tal_products(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);
static int
fm_mso_bb_topup(
       cm_nap_connection_t		*connp,
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
		
static void
fm_mso_extend_prod_validity(
        pcm_context_t           *ctxp,
    	time_t			        new_end_t,
        time_t                  new_start_t,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp);
		
static void
fm_mso_get_curr_fup_valid_end_date(
        pcm_context_t           *ctxp,
        pin_flist_t             *pp_flistp,
	time_t			*ncr_end_t,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_get_mso_purplan(
        pcm_context_t           *ctxp,
        poid_t                  *acc_pdp,
        poid_t                  *plan_obj,
        int                     status,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/
EXPORT_OP int 
fm_mso_add_deal(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_TOP_UP_BB_PLAN 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_topup_bb_plan(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_topup_bb_plan(
	cm_nap_connection_t	*connp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

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
        char			*prov_tag,
	int			flags,
	int                     grace_flag,
	pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

extern int32
get_diff_time(
        pcm_context_t           *ctxp,
        time_t          timeStamp,
        time_t          timeStamp2,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_cust_get_bp_obj(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        poid_t                  *service_obj,
	int			mso_status,
        poid_t                  **bp_obj,
        poid_t                  **mso_obj,
        pin_errbuf_t            *ebufp);

static int 
fm_mso_topup_amc_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	time_t			new_start_t,
	time_t			new_end_t,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp );

extern void
fm_mso_get_balances(
        pcm_context_t           *ctxp,
        poid_t                  *account_pdp,
        time_t                  bal_srch_t,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_get_poid_details(
        pcm_context_t           *ctxp,
        poid_t                  *poid_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void fm_mso_cancel_tal_products(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_CUST_TOP_UP_BB_PLAN  
 *******************************************************************/
void 
op_mso_cust_topup_bb_plan(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	int			*status = NULL;
	int32			*renew_flag = NULL;
	int32			status_uncommitted =2;
	int32			price_calc_flag = 0;
	poid_t			*inp_pdp = NULL;
	char            *prog_name = NULL;
	void			*vp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_TOP_UP_BB_PLAN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_topup_bb_plan error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_topup_bb_plan input flist", i_flistp);
	
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	renew_flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	/* Fetch the value of CALC ONLY flag*/
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 1, ebufp );
	if(vp && *(int32 *)vp == 1) {
		price_calc_flag = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
	}
//	if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
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
		fm_mso_cust_topup_bb_plan(connp, flags, i_flistp, r_flistpp, ebufp);
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Topup/renewal completed ");
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (status && *(int*)status == 1) || price_calc_flag ==1)
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
			PIN_ERRBUF_RESET(ebufp);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_topup_bb_plan error", ebufp);
		}
		if(price_calc_flag ==1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Rollback due to Price Calc.");
		}
		if(local == LOCAL_TRANS_OPEN_SUCCESS &&
			(!(prog_name && strstr(prog_name,"pin_deferred_act")))
		  )
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
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}		
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_topup_bb_plan output flist", *r_flistpp);
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_topup_bb_plan(
	cm_nap_connection_t		*connp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp=connp->dm_ctx;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*success_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*lc_out_flist = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*service_obj = NULL;
	poid_t			*user_id = NULL;
        char			bb_service_type[]="/service/telco/broadband";	
	int32			topup_failure_t = 1;
	int32			account_type      = -1;
	int32			account_model     = -1;
	int32			subscriber_type   = -1;
	int32			topup_success = 0;
	int32			topup_failure = 1;
    int32           *over_flagp = NULL;
    int32           override_flag = 1; 
	int64			db = -1;
	int			csr_access = 0;
	int			acct_update = 0;
	int			ret_status = 0;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_topup_bb_plan input flist", i_flistp);	
	
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
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &topup_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51412", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	if (!account_obj || !service_obj )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &topup_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51413", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	/*******************************************************************
	* Validation product rules - Start
	*******************************************************************/
//	fm_mso_get_account_info(ctxp, account_obj, &acnt_info, ebufp);
//	fm_mso_get_business_type_values(ctxp, *(int32*)PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp), 
//	                        &account_type, &account_model, &subscriber_type, ebufp );

	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		goto ROLE;		
	}
	//subscriber_type = fm_mso_get_parent_info(ctxp, i_flistp, account_obj, &acnt_info, ebufp);
	if (ret_flistp)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == topup_failure)
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
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &topup_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51414", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ROLE passed in input does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR haas permission to change account information");	
		}
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &topup_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51415", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}

    over_flagp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OVERRIDE_FLAGS, 1, ebufp);
    if (over_flagp && *over_flagp == 1)
    {
        override_flag = 0;
    }
	/*******************************************************************
	* Validation for CSR roles  - End
	*******************************************************************/

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
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ENTERING TRIAL PLAN VALIDATION FOR TOPUP");
	pin_conf("fm_mso_cust", "days_threshold", PIN_FLDT_INT, (caddr_t*)&days_threshold, &err ); 
	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);		
	fm_mso_get_event_from_plan(ctxp,plan_pdp,&event_type_flistp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "NEW PLAN EVENT TYPE FOR VALIDATION OF TRIAL IN TOPUP", event_type_flistp);
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
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Sorry Cannot Topup Trial Plan.", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ERROR FLIST DEFINITION", ret_flistp);
			*r_flistpp = ret_flistp;
			return;
		}
		else
 			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Topup is for a Trial Plan and 30 days have crossed hence Avoiding Validation.");
		}
		else
 			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Old Subscription Plan Found in Topup hence Avoiding Validation.");
	}
	else 
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Topup is Not for a Trial Plan hence Avoiding Validation.");
	
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
	acct_update = fm_mso_bb_topup(connp, i_flistp, &ret_flistp, lc_out_flist, ebufp);

	if ( acct_update == 0)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Topup failed");
		*r_flistpp = ret_flistp;
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_topup_bb_plan add plan successful");
		fm_mso_create_lifecycle_evt(ctxp, i_flistp, lc_out_flist, ID_TOPUP_PREPAID, ebufp );
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_topup_bb_plan insufficient data provided");	
		*r_flistpp = ret_flistp;
		goto CLEANUP;
	}
	else if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_topup_bb_plan CALC ONLY successful");	
		*r_flistpp = ret_flistp;
		goto CLEANUP;
	}
	/*******************************************************************
	* Topup plan  - End
	*******************************************************************/
	success_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, success_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, success_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
	PIN_FLIST_FLD_SET(success_flistp, PIN_FLD_STATUS, &topup_success, ebufp);
	PIN_FLIST_FLD_SET(success_flistp, PIN_FLD_DESCR, "Topup/renewal completed", ebufp);
	*r_flistpp = success_flistp;
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&lc_out_flist, NULL);
	return;
}

static int
fm_mso_bb_topup(
        cm_nap_connection_t		*connp,
        pin_flist_t             *in_flist,
        pin_flist_t             **bb_ret_flistp,
	pin_flist_t		*lc_out_flistp,
        pin_errbuf_t            *ebufp)
{
	
	pcm_context_t           *ctxp=connp->dm_ctx;
	pin_flist_t		*dealinfo_flistp = NULL;
	pin_flist_t		*deals_flistp = NULL;
	pin_flist_t		*products_flistp = NULL;
	pin_flist_t		*statuschange_inflistp = NULL;
	pin_flist_t		*statuschange_outflistp = NULL;
	pin_flist_t		*read_flistp = NULL;
	pin_flist_t		*read_res_flistp = NULL;
	pin_flist_t		*status_flistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*return_flistp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*prov_res_flistp = NULL;
	pin_flist_t		*plan_in_flistp = NULL;
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*pur_return_flistp = NULL;
	pin_flist_t		*srch_prt_out_flistp = NULL;
	pin_flist_t		*read_pp_flistp = NULL;
	pin_flist_t		*read_pp_res_flistp = NULL;
	pin_flist_t		*cancel_plan_flistp = NULL;
	pin_flist_t		*cancel_plan_ret_flistp = NULL;
	pin_flist_t		*read_bal_flistp = NULL;
	pin_flist_t		*read_bal_res_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*set_in_flistp = NULL;
	pin_flist_t		*set_res_flistp = NULL;
	pin_flist_t		*sub_bal_flistp = NULL;
	pin_flist_t		*temp_in_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*lc_sub_bal_array = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*bal_grp_obj = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*mso_pp_obj = NULL;
	poid_t			*mso_plan_obj = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*product_obj = NULL;
	poid_t			*offering_obj = NULL;
	poid_t			*pp_obj = NULL;
	char 			*plan_type = "base plan";
		
	char			 msg[512] ;
	char			 *tmp_str = NULL ;
	//char			 *top_up = NULL;
	char			 *prov_tag = NULL ;
	char			 *temp_tag = NULL;
	char                     *search_template = "select X from /mso_customer_credential where F1 = V1 ";
	char                     *search_prt_template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3";
	
	int			res_id = 0;
	int			bal_flags = 0;
	int			counter = 0;
	int			elem_id = 0;
	int			elem_id2 = 0;
	int			elem_base = 0;
	int			bal_elem_id = 0;
	int			sub_bal_id = 0;
	int			prty = 0;
	int			cntr = 0;
	int			plan_status = 0;
	int			prov_result = 0;
	int			notif_result = 0;
	int64			diff = 0;
	int32			*prov_status = NULL;
	int			set_err = 0;
	int			plan_count = 0;
	int			prov_counter = 0;
	int			topup_failure = 1;
	int			status_change_failure = 1;
	int			opcode = MSO_OP_CUST_TOP_UP_BB_PLAN;
	int			ret_val = 0;
	int32			srch_flag = 512;
	int32			diff2 = 0;
	int32			db = 0;
	int32                  *customer_type = NULL;
	//int32                  *indicator = NULL;
	int				*ret_status = NULL;
	int32			*status = NULL;
	int32			*cancel_status = NULL;
	int32			price_calc_flag = 0;
	int32			subs_plan_type = 1;
	int32			*bill_when = NULL;
	int32			mm = 0;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie2 = NULL;
	pin_cookie_t		res_cookie = NULL;
	pin_cookie_t		bal_cookie = NULL;
	pin_cookie_t		sub_cookie = NULL;
	pin_cookie_t		cookie_base = NULL;

	pin_decimal_t          *priority = NULL;
	pin_decimal_t          *plan_priority = NULL;

	time_t			*pp_end_t = NULL;
//	time_t			*strt_t = NULL;
	time_t			current_t = 0;
	time_t			*plan_crtd_t = NULL;
	time_t			valid_end_t = 0;
	time_t			opcode_start_t = 0;
	time_t			ncr_end_t = 0;
	time_t          *topup_t = NULL;
	struct tm	*timeeff;
	struct tm	*tmp_time;
	time_t		new_end_t = 0;
	void 			*vp = NULL;
	int32			*fup_status = NULL;
	time_t                  curr_time = 0;	
	int32                   cycle_end_det = 0;
        int32                   unit_var = 0;
        int32                   offset_var = 0;
	pin_validity_modes_t    mode_variable = 0;
	poid_t			*deal_pdp = NULL;
	pin_flist_t		*ret_flistpp = NULL;
	pin_flist_t		*tal_cancel_details = NULL;
	int32                   day = 0;
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_topp_plan :");
	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );	
	//indicator = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_INDICATOR, 1, ebufp );	
	opcode_start_t = pin_virtual_time(NULL);
	if ( !service_obj || !account_obj || (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) != 0 )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &topup_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51413", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Incorrect POID value passed in input", ebufp);
               	*bb_ret_flistp = ret_flistp;
	        return 0;
	}
	vp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_MODE, 1, ebufp );
	if(vp && *(int32 *)vp == 1) {
		price_calc_flag = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
	}

	read_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, service_obj, ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_SUBSTR_ADD(read_flistp, MSO_FLD_BB_INFO, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_FUP_STATUS, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(read_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_topup read service input list", read_flistp);
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
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_topup read service output flist", read_res_flistp);

	bal_grp_obj = PIN_FLIST_FLD_TAKE(read_res_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
	status = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_STATUS, 1, ebufp );
	args_flistp = PIN_FLIST_SUBSTR_GET(read_res_flistp, MSO_FLD_BB_INFO, 1, ebufp );
	bill_when = PIN_FLIST_FLD_TAKE(args_flistp, PIN_FLD_BILL_WHEN, 1, ebufp );
	fup_status = PIN_FLIST_FLD_TAKE(args_flistp, MSO_FLD_FUP_STATUS, 1, ebufp );
	args_flistp = PIN_FLIST_ELEM_GET(read_res_flistp, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp );
	prov_status = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATUS, 1, ebufp );
	if ( *(int32 *)status != MSO_ACTIVE_STATUS || *(int32 *)prov_status != MSO_PROV_ACTIVE )
	{
		PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51616", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service or Provisioning status for this service is not active - Cannot add plan", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
	}
	PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
	
	/* Validate that plan poid exists and is not NULL */
	plan_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PLAN_OBJ, 1, ebufp);
	if ( !plan_obj || plan_obj == (poid_t *)NULL )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No plans to purchase in input flist");
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51633", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Either no plans to topup or more than one plan is given in input flist", ebufp);
		*bb_ret_flistp = ret_flistp;
	}

	plan_status = 2;
	db = PIN_POID_GET_DB(service_obj);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_prt_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &plan_status, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	/*PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRIORITY, (int32 *)NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);*/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "product type search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_prt_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp) )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51629", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching product type for this account", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "product type search output flist", srch_prt_out_flistp);	
	results_flistp = PIN_FLIST_ELEM_GET(srch_prt_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
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
	plan_priority = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRIORITY, 1, ebufp);	
	plan_crtd_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CREATED_T, 1, ebufp);
	if ( !plan_priority || plan_priority == (pin_decimal_t *)NULL )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan type is not updated in /mso_purchased_plan for this plan", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51635", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Plan type is not updated in /mso_purchased_plan for this plan", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
	}
	
	

	/*******************************************************************
	* Topup Plan validation Starts - Biju.J
	*******************************************************************/
	 if (price_calc_flag != 1 ) {
	 if(!fm_utils_op_is_ancestor(connp->coip,  MSO_OP_CUST_GET_CUSTOMER_INFO)) {
		int32 		 *validation_flag = NULL;
		char		 msg_c[1000];
		sprintf(msg_c, "PIN_FLD_BOOLEAN are 1 %ld",validation_flag );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg_c);
		
		validation_flag = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_BOOLEAN, 1, ebufp );
		
		sprintf(msg_c, "PIN_FLD_BOOLEAN are %ld",validation_flag );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg_c);
		if ((!validation_flag) || (*validation_flag == 0))
		{
			int  		 topup_status = 1;
			int32 		 pay_term = 1;
			int32 		 *validity_days = NULL;
			pin_flist_t  *srch_topup_flistp = NULL;
			pin_flist_t  *srch_topup_out_flistp = NULL;
			pin_flist_t  *s_payterm_flistp = NULL;
			pin_flist_t  *s_payterm_out_flistp = NULL;
			pin_flist_t  *service_bb_flistp = NULL;
			pin_flist_t  *res_flistp_pay = NULL;
			pin_flist_t  *res_flistp_top = NULL;
			
			time_t 		 *days_cmptd = NULL;	
			time_t		 *remaing_days = NULL;
			int64           r_days = 0;
			//int32 		 *remaing_days = NULL;
			char		 topup_str[30] = "%SUBSCRIPTION%";
			char		 err_str[50]={'\0'};
			int32		 err = 0;
			char		 msg_v[1000];
			time_t		future_t = 0;
			
			int32 cur_t_num = 0;
			int32 topup_t_num = 0;
			int32 days_cmptd_num = 0;
			char 		 *search_topup_template1 = "select X from /purchased_product where F1 = V1 and F2 = V2 and F3 like V3 AND F4 = V4 and rownum = 1";
			char 		 *search_topup_template = "select X from /service where F1 = V1 and rownum = 1";
			//Get Pay term of the current plan
			db = PIN_POID_GET_DB(service_obj);
			s_payterm_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(s_payterm_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(s_payterm_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(s_payterm_flistp, PIN_FLD_TEMPLATE, search_topup_template, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(s_payterm_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, service_obj, ebufp);
			result_flistp = PIN_FLIST_ELEM_ADD(s_payterm_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Pay Term search input list", s_payterm_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_payterm_flistp, &s_payterm_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Pay Term search output list", s_payterm_out_flistp);
			PIN_FLIST_DESTROY_EX(&s_payterm_flistp, NULL);

			res_flistp_pay = PIN_FLIST_ELEM_GET(s_payterm_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Pay Term search output list -1 ", res_flistp_pay);
			//res_flistp_pay = PIN_FLIST_SUBSTR_GET(s_payterm_out_flistp, PIN_FLD_RESULTS, 0, ebufp);
			service_bb_flistp = PIN_FLIST_SUBSTR_GET(res_flistp_pay, MSO_FLD_BB_INFO, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Pay Term search output list -2", service_bb_flistp);
			pay_term = *(int32 *)PIN_FLIST_FLD_GET(service_bb_flistp, PIN_FLD_BILL_WHEN, 0, ebufp); //doub
			
			sprintf(err_str,"pay_term is : %ld", pay_term);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,err_str);
			
			if (pay_term == 1)   //doub
			{
				pin_conf("fm_mso_cust", "monthly", PIN_FLDT_INT, (caddr_t*)&validity_days, &err ); 
			}
			else if (pay_term == 2)
			{
				pin_conf("fm_mso_cust", "bimonthly", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			else if (pay_term == 3)
			{
				pin_conf("fm_mso_cust", "quarterly", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			else if (pay_term == 4)
			{
				pin_conf("fm_mso_cust", "fourmonth", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			else if (pay_term == 5)
			{
				pin_conf("fm_mso_cust", "fivemonth", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			else if (pay_term == 6)
			{
				pin_conf("fm_mso_cust", "halfyearly", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			else if (pay_term == 7)
			{
				pin_conf("fm_mso_cust", "sevenmonth", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			else if (pay_term == 8)
			{
				pin_conf("fm_mso_cust", "eightmonth", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			else if (pay_term == 9)
			{
				pin_conf("fm_mso_cust", "ninemonth", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			else if (pay_term == 10)
			{
				pin_conf("fm_mso_cust", "tenmonth", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			else if (pay_term == 11)
			{
				pin_conf("fm_mso_cust", "elevenmonth", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			else if (pay_term == 12)
			{
				pin_conf("fm_mso_cust", "yearly", PIN_FLDT_INT, (caddr_t*)&validity_days, &err );
			}
			
			sprintf(msg_v, "validity_days are %ld",*validity_days );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg_v);
			
			//Get cycle start date of Topup plan
			db = PIN_POID_GET_DB(service_obj);
			srch_topup_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(srch_topup_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(srch_topup_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(srch_topup_flistp, PIN_FLD_TEMPLATE, search_topup_template1, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_topup_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_topup_flistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &topup_status, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_topup_flistp, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, topup_str, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_topup_flistp, PIN_FLD_ARGS, 4, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
			result_flistp = PIN_FLIST_ELEM_ADD(srch_topup_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Topup Plan search input list", srch_topup_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_topup_flistp, &srch_topup_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Topup Plan search output list", srch_topup_out_flistp);
			PIN_FLIST_DESTROY_EX(&srch_topup_flistp, NULL);
			
			
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&srch_topup_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51629", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching product type for this account", ebufp);
				*bb_ret_flistp = ret_flistp;
				return 0;
			}
			
			res_flistp_top = PIN_FLIST_ELEM_GET(srch_topup_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			
			if (res_flistp_top != NULL)
			{
				topup_t = PIN_FLIST_FLD_GET(res_flistp_top, PIN_FLD_CYCLE_END_T, 1, ebufp);

				sprintf(msg_v,"topup_t is  : %ld",*topup_t);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);
				
				current_t = pin_virtual_time(NULL);
				current_t = fm_utils_time_round_to_midnight(current_t);
				current_t = current_t + 86400;
				
				sprintf(msg_v,"days remaining is  : %ld", ((*topup_t - fm_utils_time_round_to_midnight(current_t))/60/60/24));
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg_v);
				//if (*days_cmptd <= *(int*)validity_days )
				if(((*topup_t - fm_utils_time_round_to_midnight(current_t))/60/60/24) >= *(int32*)validity_days)
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Your plan is not due for renewal", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51642", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Kindly note your plan is not due for renewal", ebufp);
					//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_CYCLE_START_T, &future_t, ebufp);  //current_t = current_t + remaining days
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_CYCLE_START_T, PIN_FLIST_FLD_GET(res_flistp_top, PIN_FLD_CYCLE_END_T, 1, ebufp ), ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
					*bb_ret_flistp = ret_flistp;
					return 0;
				}
				
			}
		}
	
	/*******************************************************************
	* Topup plan validation Ends - Biju.J
	*******************************************************************/
	
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Verifying dates");

		
		if ( plan_crtd_t )
		{
			*(time_t *)plan_crtd_t = fm_utils_time_round_to_midnight(*(time_t *)plan_crtd_t);
			
		}
		
		if ( plan_crtd_t && *(time_t *)plan_crtd_t == current_t )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "This plan is activated today or already top up today", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51642", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is activated today or already top up today", ebufp);
			*bb_ret_flistp = ret_flistp;
			return 0;
		}
	}
	}
	args_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);
	if ( args_flistp )
	{
		pp_obj = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 1, ebufp);	
	}
	if ( !pp_obj || PIN_POID_IS_NULL(pp_obj) != 0 )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No purchased product is associated for this plan", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51638", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No purchased product is associated for this plan", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
	}
	read_pp_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_POID, pp_obj, ebufp);
	PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_PURCHASE_END_T, (time_t *)NULL, ebufp); 
	PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_CYCLE_END_T, (time_t *)NULL, ebufp); 
	PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_USAGE_END_T, (time_t *)NULL, ebufp); 
	PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_PACKAGE_ID, (time_t *)NULL, ebufp); 
	PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_DEAL_OBJ, (time_t *)NULL, ebufp);
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "Reading Purchased product end dates for this plan input", read_pp_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_pp_flistp, &read_pp_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_pp_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp) )
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No purchased product is associated for this plan", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
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
	deal_pdp = PIN_FLIST_FLD_GET(read_pp_res_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
    if (!topup_t)
    {
        topup_t = PIN_FLIST_FLD_GET(read_pp_res_flistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
    }
	/****************added condition to check the product's validity******************/
	fm_mso_get_poid_details(ctxp, deal_pdp, &ret_flistpp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp) )
        {
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from fucntion fm_mso_get_poid_details", ebufp);
		goto ERR;
        }
	arg_flistp = PIN_FLIST_ELEM_GET(ret_flistpp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 0, ebufp);	
	cycle_end_det = *(int32*)PIN_FLIST_FLD_GET(arg_flistp, PIN_FLD_CYCLE_END_DETAILS, 0, ebufp);
	PIN_VALIDITY_DECODE_FIELD(cycle_end_det, mode_variable, unit_var, offset_var);
	sprintf(msg,"mode_variable = %d", mode_variable);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	sprintf(msg,"unit_var = %d", unit_var);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	sprintf(msg,"offset_var = %d", offset_var);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	pp_end_t = PIN_FLIST_FLD_GET(read_pp_res_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
	ERR:
	if ( !pp_end_t || pp_end_t == (time_t *)NULL || PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "End date is not set for purchased products in this plan", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51639", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "End date is not set for purchased products in this plan", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
	}

	// Fetch the existing charged_to date for grant product(or valid_to for existing NCR)
	// it will be used if the plan is FUP.
	if(fup_status && *fup_status == 0)
	{
		fm_mso_get_curr_fup_valid_end_date(ctxp, results_flistp, &ncr_end_t , ebufp);
	}
	else
	{
		curr_time = pin_virtual_time(NULL);
		timeeff = localtime(&curr_time);
		timeeff->tm_mon = timeeff->tm_mon + 1;
                ncr_end_t = mktime (timeeff);

	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fetching grant product information", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51629", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching  customer type for this account", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
	}

	sprintf(msg,"Grant Product charged_to is : %d",ncr_end_t);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	if(fup_status && *fup_status == 0)
	{
	   tmp_time = localtime(&ncr_end_t);
		mm = tmp_time->tm_mon;
		mm = mm + 1;
		// value of tt_mon could be 1 to 11
		if(mm <= 11) {
			tmp_time->tm_mon = mm;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " New month in same year");
		} else {
			tmp_time->tm_mon = mm-12;
			tmp_time->tm_year = tmp_time->tm_year + 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " New month in next year");
		}
	  ncr_end_t = mktime(tmp_time);
	  sprintf(msg,"New Valid to for FUP is : %d",ncr_end_t);
	  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	}
	//Added till here for FUP
	
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
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
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
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
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
	/***** Commented as cyber cafe removed  
	else if ( *(int32 *)customer_type == ACCT_TYPE_CYBER_CAFE_BB )
	{
		if ( prty < BB_CYB_PREPAID_START || prty > BB_CYB_PREPAID_END )
		{
       			set_err = 1;
			goto NOMATCH;
		}
	} 
	*****************************************/
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
	//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "input for top up", in_flist);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "input for top up", in_flist);
	if ( set_err == 1 )
	{                			
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Only Subscription type plan can be top up", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51637", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Only Subscription type plan can be top up ", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Preparing Plan flist for calling cancel plan opcode");
	//Cancel If tal plan exists
	fm_mso_cancel_tal_products(ctxp,in_flist,&tal_cancel_details,ebufp);

	cancel_plan_flistp = PIN_FLIST_CREATE(ebufp);
	//if ( *(int32 *)customer_type == ACCT_TYPE_CORP_SUBS_BB )
	PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_POID, account_obj, ebufp);
	PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
	PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
	PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_ACTION_NAME, "top_up_plan", ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, cancel_plan_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, cancel_plan_flistp, PIN_FLD_USERID, ebufp);
	//PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_PLAN, elem_id, cancel_plan_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
	PIN_FLIST_ELEM_COPY(srch_prt_out_flistp, PIN_FLD_RESULTS, 0, cancel_plan_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
	arg_flistp = PIN_FLIST_ELEM_GET(cancel_plan_flistp, PIN_FLD_PLAN_LISTS, 0, 0, ebufp);
	PIN_FLIST_FLD_COPY(read_pp_res_flistp, PIN_FLD_PACKAGE_ID, arg_flistp, PIN_FLD_PACKAGE_ID, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_CANCEL_PLAN input flist", cancel_plan_flistp);
	PCM_OP(ctxp, MSO_OP_CUST_CANCEL_PLAN, 0, cancel_plan_flistp, &cancel_plan_ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&cancel_plan_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp) )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling CANCEL PLAN opcode", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&cancel_plan_ret_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "MSO_OP_CUST_CANCEL_PLAN error", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cancel plan output flist", cancel_plan_ret_flistp);
	cancel_status = PIN_FLIST_FLD_GET(cancel_plan_ret_flistp, PIN_FLD_STATUS, 1, ebufp);
	if ( !cancel_status || *(int32 *)cancel_status != 0 )
	{
		//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling CANCEL PLAN opcode", ebufp);
		//PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
		*bb_ret_flistp = cancel_plan_ret_flistp;
		//PIN_FLIST_DESTROY_EX(&cancel_plan_ret_flistp, NULL);
		return 0;
	}
	PIN_FLIST_DESTROY_EX(&cancel_plan_ret_flistp, NULL);
	
	/*Removed change validity from here*/
	diff = get_diff_time( ctxp, current_t, *pp_end_t, ebufp);
	diff2 = (diff)*86400;
	sprintf(msg,"Current time is %d, pp_end_t is %d and diff is %d",current_t,*pp_end_t,diff);

	
	temp_in_flistp = PIN_FLIST_COPY(in_flist, ebufp);
	elem_id = 0;
	cookie = NULL;	
	results_flistp = PIN_FLIST_ELEM_GET(srch_prt_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_TYPE, &subs_plan_type, ebufp);
	mso_plan_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp); //srch_prt_out_flistp
	while ((products_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		product_obj = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
		if ( !product_obj || PIN_POID_IS_NULL(product_obj) != 0 )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Product poid is not present inside products array ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
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
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bb_topup read product input list", read_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS for product", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - PCM_OP_READ_FLDS of product poid error", ebufp);
		*bb_ret_flistp = ret_flistp;
		return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_topup_plan read product output flist", read_res_flistp);
		temp_tag = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
		PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_PURCHASE_START_T, 0, ebufp);
		PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_CYCLE_START_T, 0, ebufp);
		PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_USAGE_START_T, 0, ebufp);
		/* Pawan: Commenting below since topup is done as per validity specified in product
		  config and then prod validity is extended by function fm_mso_extend_prod_validity */
		//PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_PURCHASE_END_T, &diff2, ebufp);
		//PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_CYCLE_END_T, &diff2, ebufp);
		//PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_USAGE_END_T, &diff2, ebufp);
		
		/* If product has provisioning tag then it is a subscription products
			 so set the discount amount and discount percent passed in input */
		if(temp_tag && strlen((char *)temp_tag)>0)
		{
			prov_tag = strdup(temp_tag);
			ret_val = fm_mso_utils_discount_validation(ctxp, in_flist, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_utils_discount_validation", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in discount validation function", ebufp);
				*bb_ret_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
                                PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
				return 0;
			}
			else if(ret_val == 1)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in dicount validation", ebufp);
				PIN_ERRBUF_RESET(ebufp);
                                PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
                                ret_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
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
		//PIN_FLIST_ELEM_SET(temp_in_flistp, products_flistp,PIN_FLD_PRODUCTS, elem_id, ebufp);
	}
		PIN_FLIST_FLD_DROP(results_flistp, PIN_FLD_DESCR, ebufp); // Drop this field before calling pur plan.
		PIN_FLIST_ELEM_COPY(srch_prt_out_flistp, PIN_FLD_RESULTS, 0, temp_in_flistp, PIN_FLD_PLAN, 0, ebufp);
		// Added flag for DISCOUNT OFFER check - AD- 26-04-2017
		PIN_FLIST_FLD_SET(temp_in_flistp, PIN_FLD_OPCODE, &opcode, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_bb_plans input flist", temp_in_flistp);
		fm_mso_purchase_bb_plans ( connp, temp_in_flistp, prov_tag, 1, 0, &return_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_bb_plans return flist", return_flistp);
        //fm_mso_purchase_bb_plans ( ctxp, temp_in_flistp, &return_flistp, ebufp);
        ret_status = PIN_FLIST_FLD_GET( return_flistp, PIN_FLD_STATUS, 1, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp) || ( ret_status && *(int32 *)ret_status == 0) )
        {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in purchasing plan ", ebufp);
			PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
			*bb_ret_flistp = return_flistp;
			 return 0;
        }
	
		mso_pp_obj = PIN_FLIST_FLD_GET( return_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp);
		ret_val = fm_mso_update_mso_purplan(ctxp,mso_pp_obj, MSO_PROV_IN_PROGRESS, plan_type, ebufp );
        if (ret_val)
        {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in purchasing plan ", ebufp);
			PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
			*bb_ret_flistp = return_flistp;
			 return 0;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_topup_plan: purchase plans return flist", return_flistp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, return_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
	// Moved below condition to after date calculation since new_end_t is required 
	// for when AMC plan is called in calc_only mode.
	//if(price_calc_flag != 1) {
		// Calculate the new end date. 
		// It will be Existing End date + Bill When 
		timeeff = localtime(pp_end_t);
		if(bill_when) {
			sprintf(msg,"Bill when is %d, pp_end_t is %d ",*bill_when,*pp_end_t);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			/********added condition to check products validity**************/
			if(unit_var && unit_var == 5)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Defined in Months");
				mm = timeeff->tm_mon;
				mm = mm + *bill_when;
			}
			else if(unit_var && unit_var == 4)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Defined in Days");
				day = timeeff->tm_mday;
				mm = timeeff->tm_mon;
				sprintf(msg,"Before day change : %d",day);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				sprintf(msg,"Before month change : %d",mm);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				day = day + offset_var;
				mm = timeeff->tm_mon;
				sprintf(msg,"after month change : %d",mm);	
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				sprintf(msg,"After day change %d",day);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				timeeff->tm_mday = day;
				new_end_t = mktime (timeeff);
                		sprintf(msg,"New end is %d",new_end_t);
                		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				mm = timeeff->tm_mon;
			}
			else
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Product is defined otherthan the Month's and Days", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
                                PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
                                ret_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Product is defined otherthan the Month's and Days", ebufp);
                                *bb_ret_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
                                PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
                                return 0;	
			}
			// value of tt_mon could be 1 to 11
			if(mm <= 11) {
				timeeff->tm_mon = mm;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " New month in same year");
			} else {
				timeeff->tm_mon = mm-12;
				timeeff->tm_year = timeeff->tm_year + 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " New month in next year");
			}
			PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
		}
		new_end_t = mktime (timeeff);
		sprintf(msg,"New end is %d",new_end_t);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

	if(price_calc_flag != 1) {
		//fm_mso_extend_prod_validity( ctxp, diff2, return_flistp, &valid_end_t, ebufp);
		fm_mso_extend_prod_validity( ctxp, new_end_t, *topup_t, return_flistp, ebufp);
//	
	

	/* Change validity*/
	//current_t=pin_virtual_time ((time_t *) NULL);
	/*PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,msg);
	read_bal_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
	PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_POID, account_obj, ebufp);*/
	//bal_flags = 1;
	/*bal_flags = 4;
	PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_FLAGS, &bal_flags, ebufp);*/
	opcode_start_t = opcode_start_t - 10;
	//PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_END_T, &opcode_start_t, ebufp);
	/*PIN_FLIST_ELEM_SET(read_bal_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity read balance group input list", read_bal_flistp);
	PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES , 0, read_bal_flistp, &read_bal_res_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity read bal group output list", read_bal_res_flistp);
	PIN_FLIST_DESTROY_EX(&read_bal_flistp, NULL);*/
	fm_mso_get_balances(ctxp, account_obj, opcode_start_t, &read_bal_res_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity read bal group output list", read_bal_res_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in reading Balance group info", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&cancel_plan_ret_flistp, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "MSO_OP_CUST_TOPUP error", ebufp);
			*bb_ret_flistp = ret_flistp;
			return 0;
	}
	bal_elem_id = 0;
	bal_cookie = NULL;
	while ((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(read_bal_res_flistp, PIN_FLD_BALANCES, &bal_elem_id, 1, &bal_cookie, ebufp)) != NULL)
	{
			vp = (pin_flist_t*)PIN_FLIST_ELEM_ADD(lc_out_flistp, PIN_FLD_BALANCES, bal_elem_id, ebufp );
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
				/*valid_end_t = NULL;
				valid_end_t = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 1, ebufp );
				*valid_end_t = *valid_end_t + diff2; */ 
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_TO, args_flistp, PIN_FLD_VALID_TO, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
				cntr = cntr + 1;
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, &new_end_t, ebufp);
				// added below to set the correct valid_to for FUP resource.
				if (  bal_elem_id == MSO_FUP_TRACK )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting valid_to for FUP");
					PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, &ncr_end_t, ebufp);
				}
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
		//    PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
		//    PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity bal grp change validity input list", set_in_flistp);
			PCM_OP(ctxp, PCM_OP_BAL_CHANGE_VALIDITY , 0, set_in_flistp, &set_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
	}
	
	
	
	/*   Call Provisioning and Notification */
	res_id = 0;
	res_cookie = NULL;
	prov_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_POID, account_obj, ebufp);	
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);	
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, prov_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);	
       	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,prov_in_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(return_flistp,MSO_FLD_PURCHASED_PLAN_OBJ,prov_in_flistp,MSO_FLD_PURCHASED_PLAN_OBJ,ebufp);
		PIN_FLIST_ELEM_COPY(return_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, prov_in_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, ebufp);
	while ((pur_return_flistp = PIN_FLIST_ELEM_GET_NEXT(return_flistp, PIN_FLD_PRODUCTS, &res_id, 1, &res_cookie, ebufp )) != NULL)
	{
		PIN_FLIST_ELEM_COPY(pur_return_flistp, PIN_FLD_PRODUCTS, res_id, prov_in_flistp, PIN_FLD_PRODUCTS, res_id, ebufp);
	}
	prov_result = fm_mso_cust_call_prov( ctxp, prov_in_flistp, ebufp); 	 
        PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp) || prov_result == 10 )
	{
             	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
               	PIN_ERRBUF_RESET(ebufp);
               	PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
       		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
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
               	PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
               	PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - MSO_OP_NTF_CREATE_NOTIFICATION error", ebufp);

                *bb_ret_flistp = ret_flistp;
                return 0;
        }
	/*   Call Provisioning and Notification - END */
	

	} 
	// this block is added to prevent function calls during calc- only
	

	//Call AMC function
	ret_val==0;
    if (topup_t)
    {
        ret_val = fm_mso_topup_amc_plan(ctxp, in_flist, *topup_t, new_end_t, &return_flistp, ebufp );
    }
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


	CALC_ONLY:
		*bb_ret_flistp = return_flistp;
		PIN_FLIST_DESTROY_EX(&temp_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);
		if (set_res_flistp) 
		PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
	if(price_calc_flag == 1)
	{
		/* Return 3 for Price Calc Only mode */
		return 3;
	}	
	return 1;
}

static int fm_mso_cust_call_prov(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp)
{	
	pin_flist_t	*provaction_inflistp = NULL;
	pin_flist_t	*provaction_outflistp = NULL;
	pin_flist_t	*res_flistp = NULL;

	int		status_flags = 0;	
	int		elem_id = 0;
	int		*prov_call = NULL;

/*	poid_t		*account_obj = NULL;
	poid_t		*service_obj = NULL;
	poid_t		*mso_bb_bp_obj = NULL;
	poid_t		*mso_bb_obj = NULL; */

	pin_cookie_t	cookie = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return ;
        status_flags = DOC_BB_TOPUP_BEXPIRY_NPC;
        provaction_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flist,MSO_FLD_PURCHASED_PLAN_OBJ,provaction_inflistp,MSO_FLD_PURCHASED_PLAN_OBJ,ebufp);
		PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_SUB_BAL_IMPACTS, 0, provaction_inflistp, PIN_FLD_SUB_BAL_IMPACTS, 0, ebufp);
        PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

        /* Write USERID, PROGRAM_NAME in buffer - start */
        PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
        PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
        /* Write USERID, PROGRAM_NAME in buffer - end */
	while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL)
	{
      		PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_RESULTS, elem_id, provaction_inflistp, PIN_FLD_RESULTS, elem_id, ebufp); 
	} 
	/* 
	account_obj = PIN_FLIST_FLD_GET( in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	service_obj = PIN_FLIST_FLD_GET( in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp); 
	fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
	*/
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
	pin_flist_t	*provaction_inflistp = NULL;
	pin_flist_t	*provaction_outflistp = NULL;

	int		status_flags = 0;	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return ;
	status_flags = NTF_BEFORE_EXPIRY;
        provaction_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flist,MSO_FLD_PURCHASED_PLAN_OBJ,provaction_inflistp,MSO_FLD_PURCHASED_PLAN_OBJ,ebufp);
        PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_call_notif notification input list", provaction_inflistp);
        PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
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


int dater(x)
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

extern int32
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


static void
fm_mso_extend_prod_validity(
        pcm_context_t           *ctxp,
    	time_t			new_end_t,
        time_t          new_start_t,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp)
{

	pin_flist_t             *prod_flistp = NULL;
	pin_flist_t             *arg_flistp = NULL;
	pin_flist_t             *read_in_flistp = NULL;
	pin_flist_t             *read_out_flistp = NULL;
	pin_flist_t             *prod_out_flistp = NULL;
	poid_t					*acnt_pdp = NULL;
	poid_t					*srch_pdp = NULL;
	int32					flags = 768;
	int32					*pkg_id = NULL;
	int64					db = -1;
	time_t					*end_t = NULL;
	char					*template = "select X from /purchased_product where F1 = V1 and F2 = V2 ";
    char                    *descp = NULL;
	int				elem_id = 0;
	pin_cookie_t	cookie = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
		return;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_prod_validity input flist", in_flistp);
	prod_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 0, ebufp);
	pkg_id = PIN_FLIST_FLD_GET(prod_flistp,PIN_FLD_PACKAGE_ID,0,ebufp);

	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(acnt_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	read_in_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(read_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, arg_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PACKAGE_ID, pkg_id, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Pur prod search input flist", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Pur prod search output flist", read_out_flistp);

	
	while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(read_out_flistp, PIN_FLD_RESULTS, 
				&elem_id, 1, &cookie, ebufp)) != NULL)
	{
        descp = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_DESCR, 0, ebufp);
		read_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, read_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, read_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_PRODUCTS, 0, ebufp );
		PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PRODUCT_OBJ, arg_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_POID, arg_flistp, PIN_FLD_OFFERING_OBJ, ebufp);
		//end_t = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
		//*end_t = *end_t + diff;
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PURCHASE_END_T, &new_end_t, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CYCLE_END_T, &new_end_t, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_USAGE_END_T, &new_end_t, ebufp);
        if (descp && strstr(descp, "-SUBSCRIPTION"))
        {  
		    PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_USAGE_START_T, &new_start_t, ebufp);
        }    
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PROD SETINFO input flist", read_in_flistp);
		PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODINFO, 0, read_in_flistp, &prod_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PROD SETINFO output flist", prod_out_flistp);
		PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);
	}
	//*valid_end_t = *(time_t *)end_t;
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	return;
}

static int 
fm_mso_topup_amc_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	time_t			new_start_t,
	time_t			new_end_t,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t     *amc_flistp = NULL;
	pin_flist_t     *amc_out_flistp = NULL;
	pin_flist_t     *tmp_flistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *ret_flistp = NULL;
	pin_flist_t     *ppf_flistp  = NULL;
	pin_flist_t     *pr_ret_flistp  = NULL;
	pin_flist_t     *plan_srch_flistpp = NULL;
	pin_flist_t     *tal_pln_details = NULL;

	poid_t          *acc_obj = NULL;
	poid_t		*svc_obj = NULL;
	poid_t		*user_obj = NULL;
	poid_t          *pr_poid  = NULL;
	poid_t          *plan_pp_poid  = NULL;
	
	int32		amc_flag = AMC_ON_PREPAID_TOPUP;
	pin_decimal_t                   *pd_priority = NULL;
	char                            *strp = NULL;
	int32		prty =0;
	int32		mode = 0;
	char		msg[256] = {"\0"};
	void		*vp = NULL;
	int32   count =0;
	int32		elem_id = 0;
	pin_cookie_t	cookie = NULL;
	char		*event_type = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 1;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_topup_amc_plan input flist", in_flist);
	ret_flistp = PIN_FLIST_COPY(*r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_topup_amc_plan ret_input flist", ret_flistp);
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
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_USAGE_START_T, &new_start_t, ebufp);

	while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistp, PIN_FLD_RESULTS, 
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

			args_flistp = PIN_FLIST_SUBSTR_GET(tmp_flistp, PIN_FLD_CYCLE_INFO, 1,ebufp);
			if(args_flistp)
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CYCLE_END_T, &new_end_t, ebufp);
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

	// If topup plan is called by price calculator then copy the results array 
	// from AMC return flist to return flist of function.
        if(mode==1)
	{
		//args_flistp = PIN_FLIST_ELEM_GET(*r_flistpp,PIN_FLD_DATA_ARRAY,0,1,ebufp);
		//if(args_flistp) {
			count = PIN_FLIST_ELEM_COUNT(*r_flistpp, PIN_FLD_RESULTS, ebufp);
			cookie = NULL;
			while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(amc_out_flistp, PIN_FLD_RESULTS, 
					&elem_id, 1, &cookie, ebufp)) != NULL)
			{
				event_type = (char *) PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 1, ebufp ));
				if(event_type && strstr(event_type, "mso_hw_amc"))
				{
					PIN_FLIST_ELEM_SET(*r_flistpp, tmp_flistp, PIN_FLD_RESULTS, count, ebufp);
					count++;
				}
			}
		//}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_topup_amc_plan return flist", *r_flistpp);
	PIN_FLIST_DESTROY_EX(&amc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&amc_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return 0;
}

static void
fm_mso_get_curr_fup_valid_end_date(
        pcm_context_t           *ctxp,
        pin_flist_t             *pp_flistp,
	time_t			*ncr_end_t,
        pin_errbuf_t            *ebufp)
{

	pin_flist_t             *prod_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *read_in_flistp = NULL;
	pin_flist_t             *read_out_flistp = NULL;
	pin_flist_t             *pp_in_flistp = NULL;
	pin_flist_t             *pp_out_flistp = NULL;
	time_t			*charged_to = NULL;
	int			elem_id = 0;
	int			grant_prod_found=0;
	char			*event_type = NULL;

	pin_cookie_t		cookie = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
		return;


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_curr_fup_valid_end_date input flist", pp_flistp);
	
	while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(pp_flistp, PIN_FLD_PRODUCTS, 
				&elem_id, 1, &cookie, ebufp)) != NULL)
	{
		read_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PRODUCT_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EVENT_TYPE, NULL, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read product input list", read_in_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);	
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read product output flist", read_out_flistp);
		args_flistp = PIN_FLIST_ELEM_GET(read_out_flistp, PIN_FLD_USAGE_MAP, 0, 1, ebufp );
		if (args_flistp)
		{
			event_type = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp );
		}
		if(event_type && (strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_grant") ||
			strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward_thirty_days/mso_grant"))) 
		{
			grant_prod_found = 1;
			pp_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, pp_in_flistp, PIN_FLD_POID, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(pp_in_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CHARGED_TO_T, NULL, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CYCLE_FEE_END_T, NULL, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read product input list", pp_in_flistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, pp_in_flistp, &pp_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&pp_in_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&pp_out_flistp, NULL);	
				return;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read product output flist", pp_out_flistp);
			args_flistp = PIN_FLIST_ELEM_GET(pp_out_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp );
			if (args_flistp)
			{
				charged_to = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_CYCLE_FEE_END_T, 1, ebufp );
				if(charged_to) {
					*ncr_end_t = *charged_to;
				}
			}
			PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&pp_out_flistp, NULL);
		}
		if(grant_prod_found)
			break;
	}
	return;
}

void fm_mso_cancel_tal_products(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp) {
	
	 pin_flist_t             *tal_pp = NULL;
	pin_flist_t             *mso_plan_flistp = NULL;
	pin_flist_t             *cancel_plan_flistp = NULL;
	 pin_flist_t		*plan_list_flistp= NULL;
	pin_flist_t            *cancel_plan_ret_flistp= NULL;
	pin_flist_t		*deal_arr= NULL;
	pin_flist_t		*arg_flistp= NULL;
	poid_t 			*service_obj = NULL;
	poid_t                  *account_obj = NULL;
	poid_t                  *cancel_plan_obj = NULL;
	int32 			*vp = NULL;
	int32                   price_calc_flag = 0;
	if (PIN_ERRBUF_IS_ERR(ebufp))
                return ;
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_tal_products");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "in_flistp", in_flistp);
        service_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
        account_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp );
	
	mso_get_tal_products(ctxp,in_flistp,&tal_pp,ebufp);
	 if(tal_pp)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"tal_plan_flist");
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "tal_plan_flist", tal_pp);
        }
        else
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"There is no active plan for cancellation");
                return;
        }
	
	 /* Fetch the value of CALC ONLY flag*/
        vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_MODE, 1, ebufp );
        if(vp && *(int32 *)vp == 1) {
                price_calc_flag = 1;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
        }
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "before calling fm_mso_get_mso_purplan");	

	cancel_plan_obj = PIN_FLIST_FLD_GET(tal_pp, PIN_FLD_PLAN_OBJ, 1, ebufp );
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After calling fm_mso_get_mso_purplan 1.1");
	deal_arr = PIN_FLIST_ELEM_GET(tal_pp,PIN_FLD_DEALS,0,0,ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After calling fm_mso_get_mso_purplan 1.2");
	fm_mso_get_mso_purplan(ctxp,account_obj, cancel_plan_obj,MSO_PROV_ACTIVE,&mso_plan_flistp,ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After calling fm_mso_get_mso_purplan");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After calling fm_mso_get_mso_purplan ", mso_plan_flistp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Preparing flist for static ip plan for calling cancel plan opcode");
	PIN_FLIST_FLD_SET(mso_plan_flistp, PIN_FLD_PLAN_OBJ, cancel_plan_obj, ebufp);
        cancel_plan_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_POID, account_obj, ebufp);
        PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
        PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
        PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_ACTION_NAME, "top_up_plan", ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, cancel_plan_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, cancel_plan_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_ELEM_SET(cancel_plan_flistp,mso_plan_flistp,PIN_FLD_PLAN_LISTS,0,ebufp);
//      plan_list_flistp=PIN_FLIST_ELEM_ADD(cancel_plan_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
	//plan_list_flistp = PIN_FLIST_COPY(mso_plan_flistp, ebufp);
        arg_flistp = PIN_FLIST_ELEM_GET(cancel_plan_flistp, PIN_FLD_PLAN_LISTS, 0, 0, ebufp);
        PIN_FLIST_FLD_COPY(deal_arr, PIN_FLD_PACKAGE_ID, arg_flistp, PIN_FLD_PACKAGE_ID, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Call MSO_OP_CUST_CANCEL_PLAN");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_CANCEL_PLAN input flist", cancel_plan_flistp);
        PCM_OP(ctxp, MSO_OP_CUST_CANCEL_PLAN, 0, cancel_plan_flistp, &cancel_plan_ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&cancel_plan_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp) )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling CANCEL PLAN opcode", ebufp);
                PIN_ERRBUF_RESET(ebufp);
        //        ret_flistp = PIN_FLIST_CREATE(ebufp);
          //      PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
           //     PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
             //   PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
	}




return;
}
