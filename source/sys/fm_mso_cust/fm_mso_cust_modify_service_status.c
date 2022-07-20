/*******************************************************************
 * File:	fm_mso_cust_modify_service_status.c
 * Opcode:	MSO_OP_CUST_MODIFY_SERVICE_STATUS 
 * Owner:	Rohini Mogili
 * Created:	08-NOV-2013
 * Description: Caled for changing the status of the service.

	r << xx 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 9
	0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
	0 PIN_FLD_DESCR           STR [0] "[11 Nov, 2013 5:44 PM  Bill not paid]\n"
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /service/admin_client 43523 10
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 9
	0 PIN_FLD_STATUS         ENUM [0] 1
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
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_modify_service_status.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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

#define READONLY 0
#define READWRITE 1
#define LOCK_OBJ 2

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1


#define ACCT_TYPE_CSR_ADMIN         1
#define ACCT_TYPE_CSR               2
#define ACCT_TYPE_LCO               3
#define ACCT_TYPE_JV                4
#define ACCT_TYPE_DTR               5
#define ACCT_TYPE_SUB_DTR           6
#define ACCT_TYPE_OPERATION         7
#define ACCT_TYPE_FIELD_SERVICE     8
#define ACCT_TYPE_LOGISTICS         9
#define ACCT_TYPE_COLLECTION_AGENT 10
#define ACCT_TYPE_SALES_PERSON     11
#define ACCT_TYPE_HTW              13
#define ACCT_TYPE_SUBSCRIBER       99

#define NAMEINFO_BILLING 1
#define NAMEINFO_INSTALLATION 2

#define ACCOUNT_MODEL_UNKNOWN 0
#define ACCOUNT_MODEL_RES 1
#define ACCOUNT_MODEL_CORP 2
#define ACCOUNT_MODEL_MDU 3



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

static int 
fm_mso_update_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_MODIFY_SERVICE_STATUS 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_modify_service_status(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_modify_service_status(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_MODIFY_SERVICE_STATUS  
 *******************************************************************/
void 
op_mso_cust_modify_service_status(
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
	poid_t			*inp_pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_MODIFY_SERVICE_STATUS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_modify_service_status error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_modify_service_status input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, 3,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_modify_service_status()", ebufp);
		return;
	}
	fm_mso_cust_modify_service_status(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_modify_service_status error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_modify_service_status output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_modify_service_status(
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
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status input flist", i_flistp);	
	
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
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	if (!account_obj)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
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
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ROLE passed in input does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR haas permission to change account information");	
		}
	}else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	
	/*******************************************************************
	* Validation for PRICING_ADMIN roles  - End
	*******************************************************************/

	/*******************************************************************
	* Service status change - Start
	*******************************************************************/

	acct_update = fm_mso_update_service_status(ctxp, i_flistp, ebufp);

	if ( acct_update == 0)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51010", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account information update failed", ebufp);
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status no need of status change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status status change successful");	
	}
	if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status insufficient data provided");	
	}

	/*******************************************************************
	* Service status change  - End
	*******************************************************************/

	
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	//account_no_int = PIN_POID_GET_ID(account_obj);
	//USE PIN_POID_TO_STR  to avoid large size poids
	//sprintf(account_no_str, "%ld", account_no_int );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp );
	//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ACCOUNT_NO, account_no_str, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_success, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, ret_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	if ( acct_update == 1)
	{		
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "ACCOUNT - Service status change - no need of status change", ebufp);
	}
	else if ( acct_update == 2)
	{		
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "ACCOUNT - Service status change completed successfully", ebufp);
	}
	else if ( acct_update == 3)
	{		
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "ACCOUNT - Service status change - insufficient data provided", ebufp);
	}
	//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "ACCOUNT - Service status change completed successfully", ebufp);
	
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/

	CLEANUP:	
	*r_flistpp = ret_flistp;
	return;
}

/**************************************************
* Function: fm_mso_update_service_status()
* 
* 
***************************************************/
static int 
fm_mso_update_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*readsvc_inflistp = NULL;
	pin_flist_t		*readsvc_outflistp = NULL;
	pin_flist_t		*updsvc_inflistp = NULL;
	pin_flist_t		*updsvc_outflistp = NULL;

	poid_t			*service_obj = NULL;

	int				status_flags = PIN_STATUS_FLAG_MANUAL;
	int				*status = NULL;
	int				*old_status = NULL;
	int				status_brm = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_status :");	

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	status = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_STATUS, 1, ebufp );

	if (!service_obj || !status)
	{
		return 3;
	}

	readsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_POID, service_obj, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_STATUS, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_status read service input list", readsvc_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readsvc_inflistp, &readsvc_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&readsvc_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_status read service output flist", readsvc_outflistp);
	old_status = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_STATUS, 1, ebufp );
	
	if (*status == 0)
	{
		status_brm = PIN_STATUS_ACTIVE;
	}
	else if (*status == 1)
	{
		status_brm = PIN_STATUS_INACTIVE;
	}
	else if (*status == 2)
	{
		status_brm = PIN_STATUS_CLOSED;
	}
	
	if (status_brm == *(int*)old_status)
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		return 1;		
	}
	PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);

	updsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updsvc_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updsvc_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, updsvc_inflistp, PIN_FLD_DESCR, ebufp);
	
	//PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_SERVICES, 0, updsvc_inflistp, PIN_FLD_SERVICES, 0, ebufp);
	services_flistp = PIN_FLIST_ELEM_ADD(updsvc_inflistp, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);		
	PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS, &status_brm, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, services_flistp, PIN_FLD_POID, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_status input list", updsvc_inflistp);
	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, updsvc_inflistp, &updsvc_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&updsvc_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_SERVICES", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_status output flist", updsvc_outflistp);
	PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
	return 2;
}

