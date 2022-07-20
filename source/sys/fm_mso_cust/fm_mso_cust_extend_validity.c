/*******************************************************************
 * File:	fm_mso_cust_extend_validity.c
 * Opcode:	MSO_OP_CUST_EXTEND_VALIDITY 
 * Owner:	Rohini Mogili
 * Created:	08-NOV-2013
 * Description: This opcode will be invoked for extending the validity of prepaid service incase of service disruption

	r << xx 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 9
	0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
	0 PIN_FLD_DESCR           STR [0] "[11 Nov, 2013 5:44 PM  Bill not paid]\n"
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /service/admin_client 43523 10
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 9
	0 PIN_FLD_STATUS         ENUM [0] 10102
	xx
	xop 11008 0 1

	nap(13252)> xop 11008 0 1
	xop: opcode 11008, flags 0
	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41343 9
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 9
	0 PIN_FLD_DESCR           STR [0] "ACCOUNT - Service status change completed successfully"

 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_extend_validity.c:CUPmod7.5PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "ops/bal.h"
#include "ops/subscription.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
//#include "fm_utils.h"
//#include "fm_bal.h"
#include "mso_ops_flds.h"
//#include "psiu_business_params.h"
#include "mso_cust.h"
#include "mso_lifecycle_id.h"
#include "mso_prov.h"

#define MSO_PROV_ACTIVE       2
#define MSO_INR_CUR           356
#define MSO_ACTIVE_STATUS     10100
#define MSO_EXTEND_VALIDITY   "Extending the validity up on user request"
#define MSO_BB_TYPE           "BB"
#define MSO_CATV_TYPE         "CATV"
#define MSO_BB_PREPAID        2
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

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_cust_get_bp_obj(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        poid_t                  *service_obj,
        int                     mso_status,
        poid_t                  **bp_obj,
        poid_t                  **mso_obj,
        pin_errbuf_t            *ebufp);

/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/
extern int 
fm_mso_validate_csr_role(
	pcm_context_t		*ctxp,
	int64			db,
	poid_t		*user_id,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_cust_gen_event(
        pcm_context_t   *ctxp,
        poid_t          *acct_pdp,
        poid_t          *serv_pdp,
        char            *program,
        char            *descr,
        char            *event_type,
        int		flags,
	pin_errbuf_t    *ebufp);

EXPORT_OP int 
fm_mso_extend_validity(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_poid_details(
	pcm_context_t           *ctxp,
        poid_t                  *poid_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_search_event_map(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_EXTEND_VALIDITY 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_extend_validity(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_extend_validity(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_EXTEND_VALIDITY  
 *******************************************************************/
void 
op_mso_cust_extend_validity(
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
	int32			status_uncommitted =2;
	poid_t			*inp_pdp = NULL;
	char            *prog_name = NULL;
	time_t			pvt = 0;
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_EXTEND_VALIDITY) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_extend_validity error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_extend_validity input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_extend_validity()", ebufp);
		return;
	}*/

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//	if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
        if (!(prog_name && strstr(prog_name,"pin_deferred_act")))
	{
		inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
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

	fm_mso_cust_extend_validity(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
	{		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_extend_validity error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
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
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_extend_validity output flist", *r_flistpp);
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_extend_validity(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*user_id = NULL;
	char			bb_service_type[]="/service/telco/broadband";	
	int32                   validity_failure_t = 1;
	//char			*account_no_str = NULL;
	//char			account_no_str[10];
	int32			status_change_success = 0;
	int32			status_change_failure = 1;
	int64			db = -1;
	int				csr_access = 0;
	int				acct_update = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity input flist", i_flistp);	
	
	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if (routing_poid)
	{	
		db = PIN_POID_GET_DB(routing_poid);
		account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		//account_no_str = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp); 
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51427", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	if (!account_obj)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51428", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}

	/*******************************************************************
	* Validation for PRICING_ADMIN roles - Start
	*******************************************************************/

	if (user_id)
	{
		csr_access = fm_mso_validate_csr_role(ctxp, db, user_id, ebufp);

		if ( csr_access == 0)
		{		
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51429", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CSR does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity CSR haas permission to change account information");	
		}
	} 
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51430", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	
	/*******************************************************************
	* Validation for PRICING_ADMIN roles  - End
	*******************************************************************/

	/******************************************************************
	* INITIAL CHECK TO AVOID TRIAL PLAN VALIDITY EXTENSION
	******************************************************************/
	if (strncmp(PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp)),bb_service_type,strlen(bb_service_type))==0)
	{
	char  		    *event_typ=NULL;
	poid_t  	    *prod_pdp=NULL;
	poid_t		    *service_pdp=NULL;
	poid_t		    *act_pdp=NULL;
	pin_flist_t	    *event_type_flistp = NULL;
	pin_flist_t	    *subscr_prods_flistp = NULL;
	pin_flist_t	    *usage_flistp = NULL;
	pin_flist_t	    *last_plan_flistp = NULL;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ENTERING TRIAL PLAN VALIDATION FOR VALIDITY EXTENSION"); 
	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	fm_mso_get_subscr_purchased_products(ctxp, act_pdp, service_pdp, &subscr_prods_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SUBSCRIPTION PLAN FOR VALIDATION OF TRIAL", subscr_prods_flistp);
	
	if (subscr_prods_flistp && subscr_prods_flistp != NULL && PIN_FLIST_ELEM_COUNT(subscr_prods_flistp, PIN_FLD_RESULTS, ebufp) > 0)
       {	
		last_plan_flistp = PIN_FLIST_ELEM_GET(subscr_prods_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		prod_pdp = PIN_FLIST_FLD_GET(last_plan_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
		fm_mso_get_event_type_base(ctxp,prod_pdp,&event_type_flistp,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PLAN EVENT TYPE FOR VALIDATION OF TRIAL", event_type_flistp);
		if (event_type_flistp && event_type_flistp != NULL && PIN_FLIST_ELEM_COUNT(event_type_flistp, PIN_FLD_RESULTS, ebufp) > 0)
		{
			last_plan_flistp = PIN_FLIST_ELEM_GET(event_type_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			event_typ = PIN_FLIST_FLD_GET(last_plan_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
		}
		
		if (event_typ && event_typ != NULL && strstr(event_typ, "trial")!=NULL && (strlen(strstr(event_typ, "trial"))>0))
              {
		   	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validation Successful.");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validity_failure_t, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51490", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Sorry Cannot Extend Validity of Trial Plan.", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ERROR FLIST DEFINITION", ret_flistp);
			*r_flistpp = ret_flistp;
			return;
		}
		else
                 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan is Not a Trial Plan hence Avoiding Validation.");
	}
	else 
	     PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Old Subscription Plan Found in Account hence Avoiding Validation.");
	
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Some Unhandled Data Exception has Occured in Initial Validation for BB Customer");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Initial Trial Validations", ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validity_failure_t, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51491", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Unhandled Data Exception in Initial Trial Validation.", ebufp);
		*r_flistpp = ret_flistp;
		return;
		}
	}
 	/***********************************************************************
	* END OF INITIAL CHECK TO AVOID TRIAL PLAN VALIDITY EXTENSION
	************************************************************************/


	/*******************************************************************
	* Service status change - Start
	*******************************************************************/

	acct_update = fm_mso_extend_validity(ctxp, i_flistp, &ret_flistp, ebufp);

	if ( acct_update == 0)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity status change successful");
		fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_VALIDITY_EXTENTION_FOR_A_SERVICE_PLAN, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}
	if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity insufficient data provided");	
	}
	if ( acct_update == 4)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity no service existing with this service poid");	
	}

	/*******************************************************************
	* Service status change  - End
	*******************************************************************/

	CLEANUP:	
	*r_flistpp = ret_flistp;
	return;
}

/**************************************************
* Function: fm_mso_extend_validity()
* 
* 
***************************************************/
EXPORT_OP int 
fm_mso_extend_validity(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*readsvc_inflistp = NULL;
	pin_flist_t		*readsvc_outflistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*inher_flistp = NULL;
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*read_flistp = NULL;
	pin_flist_t		*read_res_flistp = NULL;
	pin_flist_t		*read_bal_flistp = NULL;
	pin_flist_t		*read_bal_res_flistp = NULL;
	pin_flist_t		*set_in_flistp = NULL;
	pin_flist_t		*set_res_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*bal_args = NULL;
	pin_flist_t		*products_flistp = NULL;
	pin_flist_t		*sub_bal_flistp = NULL;
	pin_flist_t		*pay_search_flistp = NULL;
	pin_flist_t		*pay_search_res_flistp = NULL;
	pin_flist_t		*cycle_fees_flistp = NULL;
	
	poid_t			*service_obj = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*bal_grp_obj = NULL;
	poid_t			*product_obj = NULL;
	poid_t			*offering_obj = NULL;

	int				*status = NULL;
	int				*prov_status = NULL;
	int				bal_flags = 0;
	int				status_flags = 0;
	int64				db = 0;
	int				elem_id = 0;
	int				bal_elem_id = 0;
	int				sub_bal_id = 0;
	int				flg = 0;
	int				counter = 0;
	int				status_change_success = 0;
	int				status_change_failure = 1;
	int				prty = 0;
	int				cntr = 0;
	int				c_elem_id = 0;
	int32				*indicator = NULL;
	int32				old_end_t_copied = 0;

	char				*msg = NULL;
	char				*strp = NULL;
	char				*event = NULL;
	char				*reason = NULL;
	char				*program = NULL;
	//char                    	*search_template = "select X from /purchased_product where F1 = V1 and F2 != V2 ";
	char                    	*search_template = "select X from /purchased_product where F1 = V1 and F2 = V2 ";
	char                    	*pay_srch_template = "select X from /payinfo 1,/billinfo 2 where 2.F1 = V1 and 1.F2 = 2.F3 and 2.F4 = V4 ";

	
	time_t				*end_t = NULL;
        time_t          		*bal_end_t   = NULL;
        time_t          		temp_t       = 0;

	pin_decimal_t			*priority = NULL;	
	
	pin_cookie_t 			cookie = NULL;
	pin_cookie_t 			bal_cookie = NULL;
	pin_cookie_t 			sub_cookie = NULL;
	pin_cookie_t 			c_cookie = NULL;
	poid_t                          *mso_pp_objp = NULL;
	poid_t				*bp_obj = NULL;
        pin_flist_t     		*set_out_flistp = NULL;
        pin_flist_t     		*sub_aud = NULL;
        pin_flist_t     		*sub_bal = NULL;
	pin_flist_t			*set_inp_flistp = NULL;	
	pin_flist_t			*ball_grp_obj = NULL;
	pin_flist_t			*usage_flist = NULL;
	char				*event_type = NULL;
	int32				*fup_check = NULL;
	pin_decimal_t			*disc_amount = NULL;
	char				*err_desc = NULL;
	pin_flist_t			*ret_flistpp = NULL;
	int32				base_plan = 0;
	char				*prod_prov_tag = NULL;
	char				*prov_tag = NULL;
	int                             elem_id_p = 0;
	pin_cookie_t                    cookie_p = NULL;
	poid_t				*base_plan_pdp = NULL;	
	pin_flist_t			*product_flistp = NULL;
	int32				*count = NULL;
	int32				*unit = NULL;
	time_t				*chrg_to = NULL;
	int32           		mod_prty=0;
	int32				fup_plan = 0;
	pin_flist_t			*write_flist = NULL;
	time_t				*purch_end_t = NULL;
	int32				fup_bal = 0;
	int32				flag = 1;
	int32                           *a_count = NULL;
        int32                           *a_unit = NULL;
	pin_flist_t			*event_oflistp = NULL;
	int32				fup_add = 0;
	int32				fup_grant = 0;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity :");	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity",in_flist);
	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	reason = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_REASON_CODE, 1, ebufp );

	if ( !service_obj || !reason )
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Extend Validity - Service object or status is missing in input", ebufp);
		*ret_flistp = r_flistp;
		return 3;
	}

	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) != 0 )
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51618", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Extend Validity - Service hold is applicable only for BB service", ebufp);
		*ret_flistp = r_flistp;
		return 3;
	}

	readsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_POID, service_obj, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_CREATED_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, MSO_FLD_BB_INFO, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_FUP_STATUS, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity read service input list", readsvc_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readsvc_inflistp, &readsvc_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&readsvc_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Extend Validity - PCM_OP_READ_FLDS of service poid error", ebufp);
		
		*ret_flistp = r_flistp;
		return 4;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity read service output flist", readsvc_outflistp);

	bal_grp_obj = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp );
	status = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_STATUS, 1, ebufp );
	args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp );
	prov_status = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATUS, 1, ebufp );
	if ( *(int *)status !=  MSO_ACTIVE_STATUS || *(int*)prov_status != MSO_PROV_ACTIVE )
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51623", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Either Service or Provisioning status is not active ", ebufp);
	
		*ret_flistp = r_flistp;
		return 0;		
	}


	acct_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	/* RAVI get mso_purchased_plan object */
	fm_mso_cust_get_bp_obj(ctxp,acct_pdp,service_obj,-1,&bp_obj,&mso_pp_objp,ebufp);
	if(!mso_pp_objp)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51623", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base plan is not present for this account ", ebufp);

                *ret_flistp = r_flistp;
                return 0;
	}
	db = PIN_POID_GET_DB(service_obj);
	/*************************** Pay type Validation *************************************/
        pay_search_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_PUT(pay_search_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(pay_search_flistp, PIN_FLD_FLAGS, &status_flags, ebufp);
        PIN_FLIST_FLD_SET(pay_search_flistp, PIN_FLD_TEMPLATE, pay_srch_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(pay_search_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(pay_search_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (poid_t *)NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(pay_search_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PAYINFO_OBJ, (poid_t *)NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(pay_search_flistp, PIN_FLD_ARGS, 4, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILLINFO_ID, (char *)MSO_BB_TYPE, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(pay_search_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, (poid_t *)NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_INV_INFO, PIN_ELEMID_ANY, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_INDICATOR, (int32 *)NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo search pay type input list", pay_search_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, pay_search_flistp, &pay_search_res_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&pay_search_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
        	PIN_FLIST_DESTROY_EX(&pay_search_res_flistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51624", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in reading pay type", ebufp);
	
		*ret_flistp = r_flistp;
		return 0;		
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_custinfo search pay type output flist", pay_search_res_flistp);
	results_flistp = PIN_FLIST_ELEM_GET(pay_search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	if ( !results_flistp )
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
        	PIN_FLIST_DESTROY_EX(&pay_search_res_flistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51624", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in reading pay type", ebufp);
	
		*ret_flistp = r_flistp;
		return 0;		
	}
	args_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_INV_INFO, 0, 1, ebufp );
	indicator = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_INDICATOR, 1, ebufp );
	if ( !indicator || *(int *)indicator !=  MSO_BB_PREPAID )
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
        	PIN_FLIST_DESTROY_EX(&pay_search_res_flistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51625", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Extend validity can be done only for prepaid service ", ebufp);
	
		*ret_flistp = r_flistp;
		return 0;		
	}
        PIN_FLIST_DESTROY_EX(&pay_search_res_flistp, NULL);

	/*************************** Pay type Validation End*************************************/
	
	/*************************** Plan Validity Extension *************************************/
	end_t = PIN_FLIST_FLD_GET(in_flist,PIN_FLD_END_T, 1, ebufp);
	if ( end_t != (time_t *)NULL )
	{
	
		status_flags = 512;
		search_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, args_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

		status_flags = 1;

		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status_flags, ebufp);

		results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity search purchased product input list", search_inflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51619", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - PCM_OP_SEARCH of purchased product error", ebufp);

			*ret_flistp = r_flistp;
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity search purchased product output flist", search_outflistp);

		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
		if (!results_flistp)
		{
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			goto PROV;	
		}
		while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			offering_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp );
			product_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp );
			read_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, product_obj, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity read product info input list", read_flistp);
			PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating Product information", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51622", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Error in reading Product information", ebufp);

				*ret_flistp = r_flistp;
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity read product info result list", read_res_flistp);
			priority = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_PRIORITY, 1, ebufp);
			if ( !priority )
			{
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
				goto PROV;	
			}
			prod_prov_tag = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
			usage_flist = PIN_FLIST_ELEM_GET(read_res_flistp,PIN_FLD_USAGE_MAP, 0, 1, ebufp );
			event_type = PIN_FLIST_FLD_GET(usage_flist, PIN_FLD_EVENT_TYPE, 1, ebufp); 
			strp = pbo_decimal_to_str(priority, ebufp);
			prty = atoi(strp);
			mod_prty = prty%1000;
			/**check for the fup_topup product and return***/
			if (prty > 1000 && ((mod_prty >= BB_FUP_START && mod_prty <= BB_FUP_END) || (mod_prty >= BB_ADD_MB_START && mod_prty <= BB_ADD_MB_END)))
			{
				
				purch_end_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
				if(purch_end_t && end_t && *purch_end_t > *end_t)
				{
					fup_add = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Fuptopup plan need to be extended");
				}	
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No extension required");
					continue;
				}
			}
			
			
			if ( !( prty >= BB_SME_PREPAID_START && prty < BB_SME_PREPAID_END ) &&
				!( prty >= BB_RET_PREPAID_START && prty < BB_RET_PREPAID_END ) &&  
				!( prty >= BB_CYB_PREPAID_START && prty < BB_RET_PREPAID_END ) &&  
				!( prty >= BB_COR_PREPAID_START && prty < BB_COR_PREPAID_END ) &&
				!( prty > BB_SME_FUP_START && prty < BB_SME_FUP_END ) &&
				!( prty > BB_RET_FUP_START && prty < BB_RET_FUP_END ) &&
				!( prty > BB_CYB_FUP_START && prty < BB_CYB_FUP_END ) &&
				!( prty > 5000 && prty < 6000 ) &&
				!( prty > BB_COR_FUP_START && prty < BB_COR_FUP_END ) && fup_add == 0)
			{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not a subscription/fup product");
					continue;		
			}   
			/***added to check for the fup_plan**/
                        if(mod_prty >= BB_UNLIMITED_FUP_RANGE_START
                                && mod_prty <= BB_UNLIMITED_FUP_RANGE_END)
                        {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FUP Plan");
                                fup_plan = 1;
                        }			
	
			/**Added changes to check the subscription product is matching with the active products*************/
			fm_mso_get_poid_details(ctxp, mso_pp_objp, &ret_flistpp, ebufp);
			cookie_p = NULL;
                	while((product_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistpp, PIN_FLD_PRODUCTS,
                        	&elem_id_p, 1, &cookie_p, ebufp)) != (pin_flist_t *)NULL)
                	{
                        	prov_tag = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
                        	if(prov_tag && prod_prov_tag && (strlen(prov_tag) > 0) && (strlen(prod_prov_tag) > 0))
                        	{
                                	base_plan_pdp = PIN_FLIST_FLD_GET(product_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 1, ebufp);
                                	if (PIN_POID_COMPARE(base_plan_pdp, offering_obj, 0, ebufp) == 0)
					{
						prov_tag = NULL;
						break;
                                	}
					else
					{
						r_flistp = PIN_FLIST_CREATE(ebufp);
                                		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                          		        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51620", ebufp);
                                		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Base plan is not matching", ebufp);

                                		*ret_flistp = r_flistp;
                                		return 0;		
					}
                        	}
                	}
			PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);

			args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, MSO_FLD_BB_INFO, PIN_ELEMID_ANY, 0, ebufp );
                        fup_check = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_FUP_STATUS, 1, ebufp );

			set_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
                        PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
                        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, set_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
                        if ( reason )
                                PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_DESCR, reason, ebufp);
                        products_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp );
                        PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_PRODUCT_OBJ, product_obj, ebufp);
                        PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_OFFERING_OBJ, offering_obj, ebufp);

			fup_add = 0;
			PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_PURCHASE_END_T, end_t, ebufp);
                        PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_USAGE_END_T, end_t, ebufp);
                        PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_CYCLE_END_T, end_t, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity set product info input list", set_in_flistp);
                        PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODINFO, 0, set_in_flistp, &set_res_flistp, ebufp);
                        PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating Product information", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
                                PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                                PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                                r_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51620", ebufp);
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Error in setting product information", ebufp);

                                *ret_flistp = r_flistp;
                                return 0;
                        }
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity set product info result list", set_res_flistp);
                        PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
			
			if (!old_end_t_copied &&  prod_prov_tag && (strlen(prod_prov_tag) > 0))
			{
				PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_USAGE_END_T, in_flist, MSO_FLD_OLD_VALIDITY_END_T, ebufp);
				old_end_t_copied=1;
			}

			/***avoid extending charged dates for fup plan*****/	
			c_cookie = NULL;
			while ((cycle_fees_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_CYCLE_FEES,
				&c_elem_id, 1, &c_cookie, ebufp)) != NULL)
			{
				if(set_in_flistp == NULL)	
				{
					set_in_flistp = PIN_FLIST_CREATE(ebufp);
				}
				if(fup_plan == 0 || (fup_plan == 1 && event_type && !strstr (event_type,"grant")))
				{
					PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_POID, offering_obj, ebufp);
					//products_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_CYCLE_FEES, 1, ebufp );
					products_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_CYCLE_FEES, c_elem_id, ebufp );
					PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_CHARGED_TO_T, end_t, ebufp);
					PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_CYCLE_FEE_END_T, end_t, ebufp);
				}
				else if(event_type && strstr (event_type,"grant"))
				{
					PIN_FLIST_FLD_SET(read_res_flistp , PIN_FLD_FLAGS , &flag , ebufp);
					fm_mso_search_event_map(ctxp, read_res_flistp, &event_oflistp, ebufp);
					count = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_COUNT, 1, ebufp);
					unit = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_UNIT, 1, ebufp);
					a_count = PIN_FLIST_FLD_GET(event_oflistp, PIN_FLD_COUNT, 1, ebufp);
                                        a_unit = PIN_FLIST_FLD_GET(event_oflistp, PIN_FLD_UNIT, 1, ebufp);
					if(count && unit && a_unit && a_count && *count == *a_count && *unit == *a_unit)
					{
						fup_grant = 1;
						chrg_to = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);	
						if(end_t && chrg_to && *end_t < *chrg_to)
						{	
							PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_POID, offering_obj, ebufp);
							products_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_CYCLE_FEES, c_elem_id, ebufp );
							PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_CHARGED_TO_T, end_t, ebufp);
							PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_CYCLE_FEE_END_T, end_t, ebufp);
							fup_bal = 1;	
						}
						else
						{
							PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_POID, offering_obj, ebufp);
							PIN_FLIST_ELEM_SET(set_in_flistp, cycle_fees_flistp, PIN_FLD_CYCLE_FEES, c_elem_id, ebufp );
							break;	
						}
					}
					PIN_FLIST_DESTROY_EX(&event_oflistp, NULL);
				}

			}
			if(fup_plan == 1 && fup_grant == 0 && event_type && strstr (event_type,"grant"))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting valid cycle array for fup grant product", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51620", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Wrong cycle array for FUP grant", ebufp);
				*ret_flistp = r_flistp;
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
				return 0;
			}
			fup_grant = 0;
			fup_plan = 0;
			if(set_in_flistp != NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity WriteFlds input flist", set_in_flistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, set_in_flistp, &set_res_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
			}
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in write flds in updating Product information", ebufp);
                                        PIN_ERRBUF_RESET(ebufp);
                                        PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
                                        r_flistp = PIN_FLIST_CREATE(ebufp);
                                        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51620", ebufp);
                                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Error in setting product information", ebufp);
                                        *ret_flistp = r_flistp;
					PIN_FLIST_DESTROY_EX(&event_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
                                        return 0;
                        }
			PIN_FLIST_DESTROY_EX(&event_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		}
		/*************************** Plan Validity Extension *************************************/
		/*************************** Resource Validity Extension *************************************/
		if ( !bal_grp_obj )
		{
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			goto PROV;	
		}
	
		read_bal_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
		PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		bal_flags = 1;
		PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_FLAGS, &bal_flags, ebufp);
		PIN_FLIST_ELEM_SET(read_bal_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity read balance group input list", read_bal_flistp);
		PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES , 0, read_bal_flistp, &read_bal_res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_bal_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in reading Balance group information", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51621", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Error in reading Balance group information", ebufp);

			*ret_flistp = r_flistp;
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity read balance group result list", read_bal_res_flistp);

		bal_elem_id = 0;
		bal_cookie = NULL;	
		while ((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(read_bal_res_flistp, PIN_FLD_BALANCES, &bal_elem_id, 1, &bal_cookie, ebufp)) != NULL)
		{
			if(fup_bal == 1 && bal_elem_id == MSO_FUP_TRACK)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FUP Balance update\n");
			}
			else if ( bal_elem_id != MSO_FREE_MB)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Currency resource\n");
				continue;
			}
			set_in_flistp = PIN_FLIST_CREATE(ebufp);
			counter = PIN_FLIST_ELEM_COUNT(bal_flistp, PIN_FLD_SUB_BALANCES, ebufp);
			/****Pavan Bellala 30/09/2015 ********************
			Bug id : 1465 -- Fix for Wrong check in 
				         validity extension		
			*************************************************/	
			//if ( counter == 0 || counter > 1 )
			if(counter == 0) 
			{
				continue;
			}
			PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, set_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
			PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
			//PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
			//PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
			sub_bal_id = 0;
			sub_cookie = NULL;	
			cntr = 0;
			while ((sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_SUB_BALANCES,
									 &sub_bal_id, 1, &sub_cookie, ebufp )) != NULL)
			{	
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_TO, args_flistp, PIN_FLD_VALID_TO, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
				cntr = cntr + 1;
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, cntr, ebufp);
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, (time_t *)end_t, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
				cntr = cntr + 1;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity bal grp change validity input list", set_in_flistp);
			PCM_OP(ctxp, PCM_OP_BAL_CHANGE_VALIDITY , 0, set_in_flistp, &set_res_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity bal grp change validity input list", set_res_flistp);
			PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in reading Balance group information", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51621", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Error in reading Balance group information", ebufp);
				*ret_flistp = r_flistp;
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity bal grp change validity result list", set_res_flistp);
		}
		PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

		PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
	}	
	//PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
	
	/*************************** Resource Validity Extension *************************************/
	/*************************** Provisoning calling *************************************/
PROV:
	
	/* changes have ended and one more line added below*/	
	
	status_flags = DOC_BB_PLAN_VALIDITY_EXTENSION;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_pp_objp, ebufp);
		/**Ravi getting bal_grp poid and setting it in the provision flist**/
	PIN_FLIST_FLD_SET(provaction_inflistp,PIN_FLD_BAL_GRP_OBJ,bal_grp_obj,ebufp);
	
	
	/* Write USERID, PROGRAM_NAME in buffer - start */
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
	/* Write USERID, PROGRAM_NAME in buffer - end */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity provisioning input list", provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);

	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Extend Validity - MSO_OP_PROV_CREATE_ACTION error", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

	/******************************************* notification flist ***********************************************/
	status_flags = 7;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity notification input list", provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Extend Validity - MSO_OP_NTF_CREATE_NOTIFICATION error", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_extend_validity provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

	/******************************************* reaction fee flist ***********************************************/
	/*******************************************************************
	* Create Lifecycle event - Start
	*******************************************************************/
	program = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	msg = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_DESCR, 1, ebufp );
	event = MSO_LC_EVENT;
	flg = 3;
//	fm_mso_cust_gen_event(ctxp, acct_pdp, service_obj, program, msg, event, flg,ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating Life cycle event in BRM ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51618", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in creating Life cycle event in BRM", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}

	/*******************************************************************
	* Create Lifecycle event - End
	*******************************************************************/
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_success, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "ACCOUNT - Extend Validity completed successfully", ebufp);
	
	*ret_flistp = r_flistp;
	
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	return 2;
}
