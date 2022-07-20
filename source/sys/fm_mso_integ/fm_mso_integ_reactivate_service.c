/*******************************************************************
 * File:	fm_mso_integ_reactivate_service.c
 * Opcode:	MSO_OP_INTEG_REACTIVATE_SERVICE 
 * Owner:	Rohini Mogili
 * Created:	21-NOV-2013
 * Description: This opcode is for changing the plan 

	r << xx 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 16
	0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
	0 PIN_FLD_PLAN_LISTS	ARRAY [0]
	1     PIN_FLD_FLAGS           INT [0] 0
	1     PIN_FLD_PLAN_OBJ    POID [0] 0.0.0.1 /plan 41089 8
	1     PIN_FLD_DEALS          ARRAY [0] allocated 5, used 5
	2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 42530 0
	2         PIN_FLD_PACKAGE_ID      INT [0] 22
	1     PIN_FLD_DEALS          ARRAY [1] allocated 5, used 5
	2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 43554 0
	2         PIN_FLD_PACKAGE_ID      INT [0] 23
	0 PIN_FLD_PLAN_LISTS	ARRAY [1]
	1     PIN_FLD_FLAGS           INT [0] 1
	1     PIN_FLD_PLAN_OBJ    POID [0] 0.0.0.1 /plan 41089 8
	1     PIN_FLD_DEALS          ARRAY [0] allocated 5, used 5
	2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 42530 0
	1     PIN_FLD_DEALS          ARRAY [1] allocated 5, used 5
	2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 43554 0
	xx
	xop 11005 0 1

	xop: opcode 11005, flags 0
	# number of field entries allocated 20, used 4
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 16
	0 PIN_FLD_DESCR           STR [0] "ACCOUNT - Service change plan completed successfully"

 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_integ_reactivate_service.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
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

#define ACCT_TYPE_CSR_ADMIN 1
#define ACCT_TYPE_CSR       2 
#define ACCT_TYPE_LCO       3
#define ACCT_TYPE_JV        4
#define ACCT_TYPE_DTR       5
#define ACCT_TYPE_SUB_DTR   6
#define ACCT_TYPE_OPERATION 7
#define ACCT_TYPE_FIELD_SERVICE 8
#define ACCT_TYPE_LOGISTICS 9
#define ACCT_TYPE_COLLECTION_AGENT 10
#define ACCT_TYPE_SALES_PERSON 11
#define ACCT_TYPE_SUBSCRIBER 12

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

extern int 
fm_mso_validate_csr_role(
	pcm_context_t		*ctxp,
	int64			db,
	poid_t		*user_id,
	pin_errbuf_t		*ebufp);

extern int 
fm_mso_reactivate_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);


/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_INTEG_REACTIVATE_SERVICE 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_integ_reactivate_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_integ_reactivate_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_INTEG_REACTIVATE_SERVICE  
 *******************************************************************/
void 
op_mso_integ_reactivate_service(
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
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_INTEG_REACTIVATE_SERVICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_integ_reactivate_service error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_integ_reactivate_service input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_integ_reactivate_service()", ebufp);
		return;
	}*/

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//	if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
        if (!(prog_name && strstr(prog_name,"pin_deferred_act")))
	{
			inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			local = fm_mso_trans_open(ctxp, inp_pdp, 3,ebufp);
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
					PIN_ERRBUF_CLEAR(ebufp);
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
					PIN_ERRBUF_CLEAR(ebufp);
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

	fm_mso_integ_reactivate_service(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
	{		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_integ_reactivate_service error", ebufp);
                if(local == LOCAL_TRANS_OPEN_SUCCESS &&
                        (!(prog_name && strstr(prog_name,"pin_deferred_act")))
                  )
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
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_integ_reactivate_service output flist", *r_flistpp);
	return;
}


/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_integ_reactivate_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*change_status_inflistp = NULL;
	pin_flist_t		*change_status_outflistp = NULL;
	
	poid_t			*user_id = NULL;
	
	char			*account_no = NULL;
	char			*plan_name = NULL;	
	char			search_template_catv[100] = " select X from /service/catv where F1 = V1 and service_t.poid_type = '/service/catv' ";
	char			search_template_bb[100] = " select X from /service/broadband where F1 = V1 and service_t.poid_type = '/service/broadband' ";
	
	int32			status_change_success = 0;
	int32			status_change_failure = 1;
	int64			db = 1;
	int				result_status = 0;
	int				*service_type = NULL;
	int				*action_flags = NULL;
	int				search_flags = 768;
	int				elem_id = 0;

	pin_cookie_t	cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_reactivate_service input flist", i_flistp);	
//
//	account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);	
//
//	if (account_no)
//	{	
//		service_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 1, ebufp);
//		action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
//	}
//	else
//	{
//		ret_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51633", ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account no not passed in input as it is mandatory field", ebufp);
//		goto CLEANUP;
//	}	
//
//	/********************************** Searching the service object based on input **********************************/
//
//	search_inflistp = PIN_FLIST_CREATE(ebufp);
//	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
//	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
//
//	if (*(int*)service_type == 0)
//	{
//		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template_bb, ebufp);
//	}
//	else if (*(int*)service_type == 1)
//	{
//		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template_catv, ebufp);
//	}
//
//	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
//
//	if (*(int*)action_flags == 1)
//	{
//		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_LOGIN, account_no, ebufp);
//	}
//	else if (*(int*)action_flags == 2)
//	{
//		results_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
//		PIN_FLIST_FLD_SET(results_flistp, MSO_FLD_AGREEMENT_NO, account_no, ebufp);
//	}
//
//	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_reactivate_service search service input list", search_inflistp);
//	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
//	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
//		PIN_ERRBUF_CLEAR(ebufp);
//		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
//		ret_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51634", ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_reactivate_service search service error", ebufp);
//		goto CLEANUP;
//	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_reactivate_service search service output flist", search_outflistp);
//
//	results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
//
//	/***********************************Service reactivation flist **********************************************/
//
//	change_status_inflistp = PIN_FLIST_CREATE(ebufp);
//	PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_ACCOUNT_OBJ, change_status_inflistp, PIN_FLD_POID, ebufp);
//	PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, change_status_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, change_status_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, change_status_inflistp, PIN_FLD_USERID, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATUS, change_status_inflistp, PIN_FLD_STATUS, ebufp);
//	PIN_FLIST_FLD_SET(change_status_inflistp, PIN_FLD_DESCR, "Serivre status change", ebufp);
//
//	
//	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_reactivate_service reactivation input flist", change_status_inflistp);
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling reactivate service status ", ebufp);
//		PIN_ERRBUF_CLEAR(ebufp);
//		PIN_FLIST_DESTROY_EX(&change_status_inflistp, NULL);
//		ret_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51635", ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_reactivate_service reactivation error", ebufp);
//		goto CLEANUP;
//	}

	//PCM_OP(ctxp, MSO_OP_CUST_REACTIVATE_SERVICE, 0, change_status_inflistp, &change_status_outflistp, ebufp);

	PCM_OP(ctxp, MSO_OP_CUST_SUSPEND_SERVICE, 0, i_flistp, &ret_flistp, ebufp);

	//result_status = fm_mso_reactivate_service_status(ctxp, change_status_inflistp, &ret_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_REACTIVATE_SERVICE", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51636", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling MSO_OP_CUST_REACTIVATE_SERVICE", ebufp);
		goto CLEANUP;
	}
//
//	result_status  = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 1, ebufp);	
//	if ( result_status == 0)
//	{		
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
//	}
//	else if ( result_status == 1)
//	{		
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service no need of status change");	
//	}
//	else if ( result_status == 2)
//	{		
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service status change successful");	
//	}
//	if ( result_status == 3)
//	{		
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service insufficient data provided");	
//	}
//	if ( result_status == 4)
//	{		
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service no service existing with this service poid");	
//	}
//		

	/*******************************************************************
	* reactivation  - End
	*******************************************************************/

	CLEANUP:	
	*r_flistpp = ret_flistp;
	return;
}






