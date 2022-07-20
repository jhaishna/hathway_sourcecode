/*******************************************************************
 * File:	fm_mso_cust_deactivate_bb_service.c
 * Opcode:	MSO_OP_CUST_DEACTIVATE_BB_SERVICE 
 * Owner:	Nagaraj
 * Created:	04-NOV-2014
 * Description: Called for de-activating the provisioing status and cancel plan in various scenarios

	nap(13252)> xop 11008 0 1
	xop: opcode 11008, flags 0
	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41343 9
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 37393 9
	0 PIN_FLD_MODE			 ENUM [0] 0
	0 PIN_FLD_DESCR           STR [0] "ACCOUNT - Service status change completed successfully"
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_deactivate_bb_service.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "psiu_business_params.h"
#include "mso_cust.h"
#include "mso_ntf.h"
#include "mso_prov.h"
#include "mso_errs.h"
#include "mso_lifecycle_id.h"

#define SUCCESS 0
#define FAILURE 1


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
fm_mso_deactivate_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**ret_flistp,
	pin_flist_t		*lc_in_flist,
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

EXPORT_OP 
void create_return_flist(
	pin_flist_t	*r_flistp,
	int32		status,
	poid_t		*acc_pdp,
	poid_t		*svc_pdp,
	char		*error_code,
	char		*error_descr,
	pin_errbuf_t	*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

int32
fm_cust_deac_act_purchased(
        pcm_context_t           *ctxp,
	poid_t			*acc_pdp,
	int32			flag,
	pin_errbuf_t            *ebufp);
	
/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_DEACTIVATE_SERVICE 
 *
 *******************************************************************/

EXPORT_OP void
op_mso_cust_deactivate_bb_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_deactivate_bb_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_SUSPEND_SERVICE  
 *******************************************************************/
void 
op_mso_cust_deactivate_bb_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int			*status = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_DEACTIVATE_BB_SERVICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_deactivate_service error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_deactivate_service input flist", i_flistp);
	
	fm_mso_cust_deactivate_bb_service(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
	{		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
		    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_deactivate_service error", ebufp);
		}
	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_deactivate_service output flist", *r_flistpp);
	return;
}

/*******************************************************************
 * This is the default implementation for this opcode
 *******************************************************************/
static void 
fm_mso_cust_deactivate_bb_service	(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*lc_in_flist = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*user_id = NULL;
	
	int32			status_change_failure = 1;
	int32			status_change_success = 0;
	int64			db = -1;
	int				csr_access = 0;
	int				acct_update = 0;
	int32			*mode = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_deactivate_bb_service input flist", i_flistp);	
	
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if (routing_poid)
	{	
		db = PIN_POID_GET_DB(routing_poid);
		account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(ret_flistp,status_change_failure,PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ),
				NULL, CUST_BB_DEACTIVATE_INV_STATUS,"POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}

	mode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 0, ebufp);
	if(mode == NULL) {
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(ret_flistp,status_change_failure,PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ),
                                NULL, CUST_BB_DEACTIVATE_INV_STATUS,"MODE value not passed in input as it is mandatory field", ebufp);
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
		create_return_flist(ret_flistp,status_change_failure,PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ),
                                NULL, CUST_BB_DEACTIVATE_INV_STATUS,"CSR does not have permissions to change the service status", ebufp);
				   goto CLEANUP;
		   }else
		   {
				   PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service CSR haas permission to change account information");
		   }
	}else
	{
		   ret_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(ret_flistp,status_change_failure,PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ),
                                NULL, CUST_BB_DEACTIVATE_INV_STATUS,"User ID value not passed in input as it is mandatory field", ebufp);
		   goto CLEANUP;
	}
	
	/*****************************************************************
	* Checking for the subscription product active to deactivate the 
	  service
	*****************************************************************/
	if((mode && *mode == VALIDITY_EXPIRY) && fm_cust_deac_act_purchased(ctxp, account_obj,1, ebufp))
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
                create_return_flist(ret_flistp, status_change_failure, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ),
                                NULL, CUST_BB_DEACTIVATE_HAVE_SUBSCRIPTION,"Already having the subscription purchased_product or error in search", ebufp);
                   goto CLEANUP;
	}		

	/*******************************************************************
	* Service status change - Start
	*******************************************************************/

	lc_in_flist = PIN_FLIST_COPY(i_flistp, ebufp );
	acct_update = fm_mso_deactivate_service_status(ctxp, i_flistp, &ret_flistp, lc_in_flist, ebufp);

	if ( acct_update == 0)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_deactivate_bb_service no need of status change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_deactivate_bb_service status change successful");	
		fm_mso_create_lifecycle_evt(ctxp, lc_in_flist, NULL, ID_DEACTIVATE_VALIDITY_EXPIRY, ebufp);
 	}
	if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_deactivate_bb_service insufficient data provided");	
	}
	if ( acct_update == 4)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_deactivate_bb_service no service existing with this service poid");	
	}
	if ( acct_update == 5)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_deactivate_bb_service failure in cancelling the plan");	
	}

	/*******************************************************************
	* Service status change  - End
	*******************************************************************/

	CLEANUP:	
		*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&lc_in_flist, NULL);
	return;
}

/**************************************************
* Function: fm_mso_suspend_service_status()
* 
* 
***************************************************/
EXPORT_OP int 
fm_mso_deactivate_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**ret_flistp,
	pin_flist_t		*lc_in_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*readsvc_inflistp = NULL;
	pin_flist_t		*readsvc_outflistp = NULL;
	pin_flist_t		*updsvc_inflistp = NULL;
	pin_flist_t		*updsvc_outflistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*inher_flistp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*mso_bb_bp_obj = NULL;
	poid_t			*mso_bb_obj = NULL;

	int				status_flags = PIN_STATUS_FLAG_MANUAL;
	int				*status = NULL;
	int				*old_status = NULL;
	int				*old_prov_status = NULL;
	int				*status_val = NULL;
	int				new_prov_status = MSO_PROV_IN_PROGRESS;
	int				inactive_status = PIN_STATUS_INACTIVE;
	int				status_flag = 1;
	int				flg = 0;
	int				status_change_success = 0;
	int				status_change_failure = 1;
	int				ret_val = 0;

	char				*msg = NULL;
	char				*event = NULL;
	char				*program = NULL;

	int32			mode = 0;
	time_t			now_t = 0;
	char			*err_desc = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_deactivate_bb_service_status :");	

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	acct_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID,0,ebufp);
	status = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_STATUS, 1, ebufp );

	if (!service_obj || !status)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
                                NULL, CUST_BB_DEACTIVATE_INV_STATUS,"ACCOUNT - Service Deactivate status - Service object or status is missing in input", ebufp);
		*ret_flistp = r_flistp;
		return 3;
	}



	readsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_POID, service_obj, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_CREATED_T, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_deactivate_service_status read service input list", readsvc_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readsvc_inflistp, &readsvc_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&readsvc_inflistp, NULL);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
                                service_obj, CUST_BB_DEACTIVATE_INV_STATUS,"ACCOUNT - Service deactivate status - PCM_OP_READ_FLDS of service poid error", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service deactivate status - PCM_OP_READ_FLDS of service poid error", ebufp);
		*ret_flistp = r_flistp;
		return 4;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_deactivate_service_status read service output flist", readsvc_outflistp);
	

	old_status = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_STATUS, 1, ebufp );
	args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp );
	old_prov_status = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATUS, 1, ebufp );
	if (*(int *)status == *(int *)old_status)
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
                                service_obj, CUST_BB_DEACTIVATE_INV_STATUS,"Service is already in deactivated state ", ebufp);
		*ret_flistp = r_flistp;
		return 1;		
	}
	if((*(int*)old_prov_status == MSO_PROV_IN_PROGRESS)  || (*(int*)old_prov_status == MSO_PROV_STATE_DEACTIVE))
	{
	        PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                r_flistp = PIN_FLIST_CREATE(ebufp);
                create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
                                service_obj, CUST_BB_DEACTIVATE_ALREADY_IN_PROGRESS,"Service is already in deactivated state ", ebufp);
                *ret_flistp = r_flistp;
                return 1;
	}
	else if ( *(int*)old_prov_status != MSO_PROV_SUSPEND && *(int*)old_prov_status != MSO_PROV_ACTIVE )
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
                                service_obj, CUST_BB_DEACTIVATE_INV_STATUS,"Provisioning status for this service is not active - Cannot deactivate", ebufp);
		*ret_flistp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Returning....", *ret_flistp);
		return 1;		
	}


	PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to update service status");
	mode = *(int32 *)PIN_FLIST_FLD_GET(in_flist, PIN_FLD_MODE, 0, ebufp);

	if(mode == VALIDITY_EXPIRY || mode == QUOTA_EXHAUSTION) {	// Call the same set of actions for both quota exhaustion and validity expiry	
		
		updsvc_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, updsvc_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updsvc_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, updsvc_inflistp, PIN_FLD_DESCR, ebufp);
		services_flistp = PIN_FLIST_ELEM_ADD(updsvc_inflistp, PIN_FLD_SERVICES, 0, ebufp );
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, services_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS, &inactive_status, ebufp);
		PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS_FLAGS, &status_flag, ebufp);
		inher_flistp = PIN_FLIST_SUBSTR_ADD(services_flistp, PIN_FLD_INHERITED_INFO, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(inher_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &new_prov_status, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_deactivate_service_status input list", updsvc_inflistp);
		PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, updsvc_inflistp, &updsvc_outflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_deactivate_service_status output list", updsvc_outflistp);

		PIN_FLIST_DESTROY_EX(&updsvc_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_SERVICES", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
                                service_obj, CUST_BB_DEACTIVATE_INV_STATUS,"ACCOUNT - Service suspend status - PCM_OP_CUST_UPDATE_SERVICES of service poid error", ebufp);
			*ret_flistp = r_flistp;
			return 0;
		}
		PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);

		/* Pawan: Added below block to Cancel the plan in case of Quota Exhaust */
		if (mode == QUOTA_EXHAUSTION)
		{
			PIN_FLIST_FLD_SET(in_flist, PIN_FLD_ACTION_NAME, "deactivate_bb_service", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_CANCEL_PLAN input list", in_flist);
			PCM_OP(ctxp, MSO_OP_CUST_CANCEL_PLAN, 0, in_flist, &updsvc_outflistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_CANCEL_PLAN", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
					service_obj, CUST_BB_DEACTIVATE_INV_STATUS,"ACCOUNT - Service suspend status - PCM_OP_CUST_UPDATE_SERVICES of service poid error", ebufp);
				*ret_flistp = r_flistp;
				return 0;
			}
			else
			{
				status_val = PIN_FLIST_FLD_GET(updsvc_outflistp, PIN_FLD_STATUS, 0, ebufp );
				if(status_val && *status_val == 1)
				{
					*ret_flistp = updsvc_outflistp;
					return 0;
				}
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_CANCEL_PLAN output list", updsvc_outflistp);
			PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
		}

		/*************************** Provisoning calling *************************************/	
		acct_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
		if (mode == QUOTA_EXHAUSTION)
		{
		   status_flags = DOC_BB_DEACTIVATE_PQUOTA_EXP;
		} 
		else if (mode == VALIDITY_EXPIRY) 
		{
			status_flags = DOC_BB_DEACTIVATE_PKG_EXP;
		}
		provaction_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

			fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, -1, &mso_bb_bp_obj, &mso_bb_obj,ebufp);  //update
			//PIN_FLIST_FLD_SET(lc_in_flist, PIN_FLD_PLAN_OBJ, mso_bb_bp_obj, ebufp);
			
			if ( mso_bb_obj && !PIN_POID_IS_NULL(mso_bb_obj))
			{
				PIN_FLIST_FLD_SET(lc_in_flist, PIN_FLD_PLAN_OBJ, mso_bb_bp_obj, ebufp);   
				PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_bb_obj, ebufp);
			}
			else
			{
				fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, MSO_PROV_STATE_SUSPENDED, &mso_bb_bp_obj, &mso_bb_obj,ebufp);
				if(mso_bb_obj && !PIN_POID_IS_NULL(mso_bb_obj))
				{
					PIN_FLIST_FLD_SET(lc_in_flist, PIN_FLD_PLAN_OBJ, mso_bb_bp_obj, ebufp);
                                	PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_bb_obj, ebufp);
				}
				else
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting base plan object for Provisioning", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					r_flistp = PIN_FLIST_CREATE(ebufp);
					create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
                                	service_obj, CUST_BB_DEACTIVATE_INV_STATUS,"Suspend service - Error in getting base plan object for Provisioning", ebufp);
					*ret_flistp = r_flistp;
					return 0;
				}
			}
		/* Pawan:20-01-2015: Update /mso_purchased_plan status to 1(In-progress) */	
		ret_val = fm_mso_update_mso_purplan_status(ctxp, mso_bb_obj, new_prov_status, ebufp );
			
		/* Write USERID, PROGRAM_NAME in buffer - start */
		PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
		/* Write USERID, PROGRAM_NAME in buffer - end */
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_deactivate_service_status provisioning input list", provaction_inflistp);
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
		if(provaction_outflistp)
			status = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp) || !provaction_outflistp || (status && *status == 1))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
                                service_obj, CUST_BB_DEACTIVATE_INV_STATUS,"ACCOUNT - Service suspend status - MSO_OP_PROV_CREATE_ACTION error", ebufp);
			*ret_flistp = r_flistp;
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_deactivate_service_status provisioning output flist", provaction_outflistp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

		/******************************************* notification flist ***********************************************/
		status_flags = NTF_BB_ON_EXPIRY;
		provaction_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
		now_t = pin_virtual_time((time_t *)NULL);
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_EFFECTIVE_T, &now_t, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_deactivate_service_status notification input list", provaction_inflistp);
		PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
                                service_obj, CUST_BB_DEACTIVATE_INV_STATUS,"ACCOUNT - Service suspend status - MSO_OP_PROV_CREATE_ACTION error", ebufp);
			*ret_flistp = r_flistp;
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_deactivate_service_status notification output flist", provaction_outflistp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

		/*******************************************************************
		* Create Lifecycle event - Start
		*******************************************************************/
		program = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 1, ebufp );
		msg = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_DESCR, 1, ebufp );
		event = MSO_LC_EVENT;
		flg = 1;
		//fm_mso_cust_gen_event(ctxp, acct_pdp, service_obj, program, msg, event, flg,ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating Life cycle event in BRM ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
		create_return_flist(r_flistp,status_change_failure,PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ),
                                service_obj, CUST_BB_DEACTIVATE_INV_STATUS,"Error in creating Life cycle event in BRM", ebufp);
			*ret_flistp = r_flistp;
			return 0;
		}

		/*******************************************************************
		* Create Lifecycle event - End
		*******************************************************************/
}
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_success, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "ACCOUNT - Service suspend status completed successfully", ebufp);
	
	*ret_flistp = r_flistp;
	
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	return 2;
}


void create_return_flist(pin_flist_t *r_flistp,
			int32 status,
			poid_t	*acc_pdp,
			poid_t	*svc_pdp,
			char	*error_code,
			char	*error_descr,
			pin_errbuf_t            *ebufp) 
{

	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, acc_pdp, ebufp );
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
	if(svc_pdp) {
            PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_SERVICE_OBJ,svc_pdp, ebufp);
	}
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, error_descr, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "create_return_flist return flist", r_flistp);
}

/*****************************************
Added function to check the weather the 
deactivate request is going on even
subscription product is active
*****************************************/
int32
fm_cust_deac_act_purchased(
        pcm_context_t                   *ctxp,
        poid_t                          *acc_pdp,
	int32				flag,
        pin_errbuf_t                    *ebufp)
{
        pin_flist_t                     *s_iflistp = NULL;
        pin_flist_t                     *s_oflistp = NULL;
        pin_flist_t                     *args_flistp = NULL;
        pin_flist_t                     *tmp_flistp = NULL;
        poid_t                          *s_pdp = NULL;
        int64                           db_no = 0;
        int32                           s_flags = SRCH_DISTINCT;
	char                            *pre_tmpl = "select X from /purchased_product 1, /product 2 where 1.F1 = V1 and 1.F2 = 2.F3 and (1.F4 = V4 or 1.F8 = V8) and 2.F5 is not null and 2.F6 <> V6 and 2.F7 <> V7 and 2.F9 <> V9 ";
	int				act_status = 1;
	int				count = 0;
   	pin_decimal_t			*fup_top_rent_pri = pbo_decimal_from_str("2900",ebufp);
	pin_decimal_t                   *add_mb_sub_pri = pbo_decimal_from_str("2930",ebufp);
	pin_decimal_t                   *fup_top_sme_pri = pbo_decimal_from_str("1900",ebufp);
	int				act_status_inac = 2;
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }

                db_no = PIN_POID_GET_DB(acc_pdp);
                s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
                s_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(s_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
                PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
                PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, (void *)pre_tmpl, ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acc_pdp, ebufp);
                
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 3, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
                
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 2, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, (void *)NULL, ebufp);
                
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 4, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status, ebufp);
                
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 5, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);
               	
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 6, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, fup_top_rent_pri, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 7, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, add_mb_sub_pri, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 8, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status_inac, ebufp);		
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 9, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, fup_top_sme_pri, ebufp);
 
		args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
                
		if(PIN_ERRBUF_IS_ERR(ebufp)){
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_deac_act_purchased: Error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&s_iflistp,NULL);
			return 1;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_deac_act_purchased: Search Input Flist ", s_iflistp);
                //Call the PCM_OP_SEARCH
                PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp)){
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_deac_act_purchased: after Search Error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&s_oflistp,NULL);
			return 1;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_deac_act_purchased: Search Output Flist ", s_oflistp);
		count = PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp);
		PIN_FLIST_DESTROY_EX(&s_iflistp,NULL);
		PIN_FLIST_DESTROY_EX(&s_oflistp,NULL);
		if (pbo_decimal_is_null(fup_top_rent_pri, ebufp))
                	pbo_decimal_destroy(&fup_top_rent_pri);
		if (pbo_decimal_is_null(add_mb_sub_pri, ebufp))
                        pbo_decimal_destroy(&add_mb_sub_pri);
		if (pbo_decimal_is_null(fup_top_sme_pri, ebufp))
                        pbo_decimal_destroy(&fup_top_sme_pri);
		if(count >1 && flag == 0)
			return 1;
		else if(count >=1 && flag == 1)
			return 1;	
		else
			return 0;
			
}
