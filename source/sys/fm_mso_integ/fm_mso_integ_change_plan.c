/*******************************************************************
 * File:	fm_mso_integ_change_plan.c
 * Opcode:	MSO_OP_INTEG_CHANGE_PLAN 
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
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_integ_change_plan.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
fm_mso_cancel_deal(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

extern int 
fm_mso_add_deal(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);


/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_INTEG_CHANGE_PLAN 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_integ_change_plan(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_integ_change_plan(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_INTEG_CHANGE_PLAN  
 *******************************************************************/
void 
op_mso_integ_change_plan(
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
	char            *descr = NULL; // VERIMATRIX
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_INTEG_CHANGE_PLAN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_integ_change_plan error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_integ_change_plan input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_integ_change_plan()", ebufp);
		return;
	}*/

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	
	// VERIMATRIX - Checking description for Bulk Change plan to avoid opening new transaction unnecessarily
	if (descr && (strcmp(descr, "Bulk_change_plan") == 0)) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'Bulk_change_plan' so transaction will not be handled at APIlevel");
	}
	else
	{	
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
	}

	fm_mso_integ_change_plan(ctxp, flags, i_flistp, r_flistpp, ebufp);
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
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_integ_change_plan error", ebufp);
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
		else
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}		
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_integ_change_plan output flist", *r_flistpp);
	return;
}


/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_integ_change_plan(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*args_flistp2 = NULL;
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*canceldeal_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*deal_flistp = NULL;
	pin_flist_t		*adddeal_flistp = NULL;
	pin_flist_t		*cancel_deals_flistp = NULL;	
	pin_flist_t		*add_deals_flistp = NULL;
	pin_flist_t		*search_deal_outflistp = NULL;
	pin_flist_t		*results_flistp2 = NULL;
	
	poid_t			*user_id = NULL;
	
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*plan_name = NULL;	
	char			search_template_catv[100] = " select X from /service/catv where F1 = V1 and service_t.poid_type = '/service/catv' ";
	char			search_template_bb[200] = " select X from /service/telco/broadband where F1 = V1 and service_t.poid_type = '/service/telco/broadband' ";
	char			search_template_plan[100] = " select X from /plan where F1 = V1 ";
	char			search_template_deal[100] = " select X from /purchased_product where F1 = V1 and F2 != V2 and F3 = V3 order by created_t  ";
	
	int32			change_plan_success = 0;
	int32			change_plan_failure = 1;
	int32			*serv_status = NULL;
	int32			*prov_status = NULL;
	int32			*indicator = NULL;
	int64			db = 1;
	int				acct_update = 0;
	int				*service_type = NULL;
	int				*action_flags = NULL;
	int				search_flags = 768;
	int				elem_id = 0;
	int				status = 3;
	int				catv_status = 1; // VERIMATRIX
	int				cancel_flag = 0;
	int				add_flag = 1;
	int32				*bill_when = NULL;
	pin_flist_t			*bb_info_flistp = NULL;
	pin_flist_t			*telco_flistp = NULL;

	pin_cookie_t	cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan input flist", i_flistp);	

	account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);	
	agreement_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);	
	bill_when = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_WHEN, 1, ebufp);

	if (account_no)
	{	
		service_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 1, ebufp);
		action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51608", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account no not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}	

	/********************************** Searching the service object based on input **********************************/

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);

	if (*(int*)service_type == 1)
	{
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template_bb, ebufp);
	}
	else if (*(int*)service_type == 0)
	{
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template_catv, ebufp);
	}

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);

	if (*(int*)action_flags == 1)
	{
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_LOGIN, account_no, ebufp);
	}
	else if (*(int*)action_flags == 2)
	{
		if (*(int*)service_type == 0)
		{
			results_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
			PIN_FLIST_FLD_SET(results_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		}
		else if (*(int*)service_type == 1)
		{
			results_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_BB_INFO, 0, ebufp);
			PIN_FLIST_FLD_SET(results_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		}

	}

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51609", ebufp);
		//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_change_plan search service error", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "search service error", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan search service output flist", search_outflistp);

	results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);

	/***********************************Cancel deal flist **********************************************/
	if(results_flistp != NULL)
	{
		
		if (*(int*)service_type == 1) // VERIMATRIX - Search based on service type
	 {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan search service results flist", results_flistp);
		serv_status = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_STATUS, 0, ebufp);
		bb_info_flistp = PIN_FLIST_SUBSTR_GET(results_flistp, MSO_FLD_BB_INFO, 0, ebufp);
		indicator = PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_INDICATOR, 0, ebufp);
		telco_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_TELCO_FEATURES, PIN_ELEMID_ANY, 0, ebufp);
		prov_status = PIN_FLIST_FLD_GET(telco_flistp, PIN_FLD_STATUS, 0, ebufp);
		//Added below to allow the change plan for prepaid plans that are expired/cancelled and prov status is De-Active
		if((serv_status && *serv_status == 10102) && (indicator && *indicator == 2) && (prov_status && *prov_status == 4)){
			// Check for cancelled products for given deal
			strcpy(search_template_deal, " select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 order by created_t ");				
		}
	 }
	 else if (*(int*)service_type == 0) // VERIMATRIX - Search based on service type
	 {
		 strcpy(search_template_deal, " select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 order by created_t ");	
     }
		
		canceldeal_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_ACCOUNT_OBJ, canceldeal_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, canceldeal_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, canceldeal_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, canceldeal_flistp, PIN_FLD_DESCR, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, canceldeal_flistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	}
	else
	{
		   ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51609", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "search service error", ebufp);
                goto CLEANUP;
	}


	/********************************** Searching the PLAN object based on PLAN NAME **********************************/

	plan_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN, 0, 1, ebufp);
	plan_name = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_NAME, 1, ebufp);

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template_plan, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, plan_name, ebufp);	

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan search plan input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51610", ebufp);
		//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_change_plan search plan error", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "change plan search plan error", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan search plan output flist", search_outflistp);

	results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);

	if(results_flistp == NULL)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51610", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "search plan error Incorrect existing plan name given", ebufp);
                goto CLEANUP;
	}

	/***********************************Cancel deal flist **********************************************/

	args_flistp = PIN_FLIST_ELEM_ADD(canceldeal_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_FLAGS,&cancel_flag , ebufp);
	PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_PLAN_OBJ, ebufp);

	while ((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_SERVICES, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		cancel_deals_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_DEALS, elem_id, ebufp);
		PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_DEAL_OBJ, cancel_deals_flistp, PIN_FLD_DEAL_OBJ, ebufp);

		/********************************** Searching the package Id object based on deal object **********************************/
		search_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template_deal, ebufp);	

		args_flistp2 = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_DEAL_OBJ, args_flistp2, PIN_FLD_DEAL_OBJ, ebufp);
		
		if (*(int*)service_type == 1) // VERIMATRIX - Search based on service type
	   {
		args_flistp2 = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp2, PIN_FLD_STATUS, &status, ebufp);
	   }
	   else if (*(int*)service_type == 0) // VERIMATRIX - Search based on service type
	 {
       args_flistp2 = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp2, PIN_FLD_STATUS, &catv_status, ebufp);
	 }
		args_flistp2 = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_COPY(canceldeal_flistp, PIN_FLD_SERVICE_OBJ, args_flistp2, PIN_FLD_SERVICE_OBJ, ebufp);

		results_flistp2 = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan search plan input list", search_inflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_deal_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan search plan output flist", search_deal_outflistp);
		
		results_flistp2 = PIN_FLIST_ELEM_GET(search_deal_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp) || results_flistp2 == NULL)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_deal_outflistp, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51611", ebufp);
			//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_change_plan search plan error", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "search plan error - Incorrect existing plan name given", ebufp);
			goto CLEANUP;
		}
		//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan search plan output flist", search_deal_outflistp);

		//results_flistp2 = PIN_FLIST_ELEM_GET(search_deal_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if(results_flistp2 != NULL)
		{
			PIN_FLIST_FLD_COPY(results_flistp2, PIN_FLD_PACKAGE_ID, cancel_deals_flistp, PIN_FLD_PACKAGE_ID, ebufp);
			PIN_FLIST_DESTROY_EX(&search_deal_outflistp, NULL);
		}
 
	}
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan cancel plan input flist", canceldeal_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling cancel_deal", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51612", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "change_plan cancel plan error", ebufp);
		goto CLEANUP;
	}


	/*******************************************************************
	* Cancel plan - Start
	*******************************************************************/
	
	//PCM_OP(ctxp , MSO_OP_CUST_CANCEL_PLAN , 0 , canceldeal_flistp, &ret_flistp, ebufp);	

	//acct_update = fm_mso_cancel_deal(ctxp, canceldeal_flistp, &ret_flistp, ebufp);

	/*if ( acct_update == 0)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51613", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account information update failed", ebufp);
		goto CLEANUP;
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan no need of cancel plan");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan cancel plan successful");	
	}
	else if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_change_plan insufficient data provided");	
	}*/

	/***********************************Add deal flist **********************************************/

	adddeal_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(canceldeal_flistp, PIN_FLD_POID, adddeal_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(canceldeal_flistp, PIN_FLD_SERVICE_OBJ, adddeal_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, adddeal_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, adddeal_flistp, PIN_FLD_DESCR, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, adddeal_flistp, PIN_FLD_USERID, ebufp);

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

	/********************************** Searching the PLAN object based on PLAN NAME **********************************/

	plan_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN, 1, 1, ebufp);
	plan_name = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_NAME, 1, ebufp);

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template_plan, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, plan_name, ebufp);	

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_add_plan search plan input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51614", ebufp);
		//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_add_plan search plan error", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Add_plan search plan error", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_add_plan search plan output flist", search_outflistp);

	results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);

	if(results_flistp == NULL)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51614", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "search plan error invalid new plan name", ebufp);
                goto CLEANUP;
	}
	else
	{
		/***********************************Add deal flist **********************************************/

		//args_flistp = PIN_FLIST_ELEM_ADD(adddeal_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(adddeal_flistp, PIN_FLD_PLAN_LISTS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_FLAGS,&add_flag , ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		
		elem_id = 0;
		cookie = NULL;

		while ((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_SERVICES, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			add_deals_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_DEALS, elem_id, ebufp);
			PIN_FLIST_FLD_COPY(deal_flistp, PIN_FLD_DEAL_OBJ, add_deals_flistp, PIN_FLD_DEAL_OBJ, ebufp);
		}
		if(bill_when)
		{
			PIN_FLIST_FLD_SET(canceldeal_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);
		}
		
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_add_plan add plan input flist", adddeal_flistp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling add_deal", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51615", ebufp);
			//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_integ_add_plan add plan error", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Add plan error", ebufp);
			goto CLEANUP;
		}

		/*******************************************************************
		* Add plan - Start
		*******************************************************************/
		PIN_FLIST_FLD_DROP(adddeal_flistp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_DROP(adddeal_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_DROP(adddeal_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
		PIN_FLIST_FLD_DROP(adddeal_flistp,PIN_FLD_SERVICE_OBJ,ebufp);
		PIN_FLIST_FLD_DROP(adddeal_flistp,PIN_FLD_DESCR,ebufp);
		
		PIN_FLIST_CONCAT(canceldeal_flistp,adddeal_flistp,ebufp);
		
		//PCM_OP(ctxp,MSO_OP_CUST_ADD_PLAN, 0, adddeal_flistp , &ret_flistp, ebufp);	
		PCM_OP(ctxp,MSO_OP_CUST_CHANGE_PLAN, 0, canceldeal_flistp , &ret_flistp, ebufp);	
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in MSO_OP_CUST_CHANGE_PLAN", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_POID,0,ebufp ),ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51615", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR,"Change plan error " , ebufp);
                        goto CLEANUP;
                }
		

		//acct_update = fm_mso_add_deal(ctxp, adddeal_flistp, &ret_flistp, ebufp);

		/*if ( acct_update == 0)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
		}
		else if ( acct_update == 1)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_add_plan no need of add plan");	
		}
		else if ( acct_update == 2)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_add_plan add plan successful");	
		}
		else if ( acct_update == 3)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_add_plan insufficient data provided");	
		}*/
	}
	
	/*******************************************************************
	* Cancel plan  - End
	*******************************************************************/

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&canceldeal_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&adddeal_flistp, NULL);
	*r_flistpp = ret_flistp;
	return;
}







