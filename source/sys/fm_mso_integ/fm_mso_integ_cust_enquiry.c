/*******************************************************************
 * File:	fm_mso_integ_cust_enquiry.c
 * Opcode:	MSO_OP_INTEG_CUST_ENQUIRY 
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
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_integ_cust_enquiry.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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

/**************************************
* Internal Functions start
***************************************/

static void
fm_mso_integ_get_last_bill(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_integ_get_outstanding_due(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_integ_get_next_bill_date(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_integ_get_service_details(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_integ_get_plan_details(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_integ_get_receipts(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	int				*service_type,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * MSO_OP_INTEG_CUST_ENQUIRY 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_integ_cust_enquiry(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_integ_cust_enquiry(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_INTEG_CUST_ENQUIRY  
 *******************************************************************/
void 
op_mso_integ_cust_enquiry(
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
	if (opcode != MSO_OP_INTEG_CUST_ENQUIRY) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_integ_cust_enquiry error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_integ_cust_enquiry input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/

	fm_mso_integ_cust_enquiry(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_integ_cust_enquiry error", ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);	
		}			
	}
	else
	{		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_integ_cust_enquiry output flist", *r_flistpp);
	}
	return;
}


/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_integ_cust_enquiry(
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
	pin_flist_t		*bill_details = NULL;
	pin_flist_t		*services_details = NULL;
	
	poid_t			*user_id = NULL;
	poid_t			*balgrp_pdp = NULL;
	
	char			*account_no = NULL;
	char			*plan_name = NULL;	
	char			search_template_catv[100] = " select X from /service/catv where F1 = V1 and service_t.poid_type = '/service/catv' ";
	char			search_template_bb[100] = " select X from /service/broadband where F1 = V1 and service_t.poid_type = '/service/broadband' ";
	char			search_template_acct[300] = " select X from /account where F1 = V1 ";
	int32			cust_enquiry_success = 0;
	int32			cust_enquiry_failure = 1;
	int64			db = 1;
	int				acct_update = 0;
	int				*service_type = NULL;
	int				*action_flags = NULL;
	int				search_flags = 768;
	int				elem_id = 0;
	int				elem_svc = 0;

	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie_svc = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_cust_enquiry input flist", i_flistp);	

	account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);	

	if (account_no)
	{	
		service_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 1, ebufp);
		action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
		if (!action_flags)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51600", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Flags not passed in input as it is mandatory field", ebufp);
			goto CLEANUP;
		}
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51600", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account no not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}	

	/********************************** Searching the service object based on input **********************************/

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);

	if ((*(int*)action_flags == 11) || (*(int*)action_flags == 12) || (*(int*)action_flags == 13) || (*(int*)action_flags == 14) || (*(int*)action_flags == 15) || (*(int*)action_flags == 16))
	{			
		if (*(int*)service_type == 0)
		{
			strcpy(search_template_acct, " select X from /service 1, /account 2 where 2.F1 = V1 and service_t.account_obj_id0 = account_t.poid_id0 and service_t.poid_type = '/service/broadband' ");
		}
		else if (*(int*)service_type == 1)
		{
			strcpy(search_template_acct, " select X from /service 1, /account 2 where 2.F1 = V1 and service_t.account_obj_id0 = account_t.poid_id0 and service_t.poid_type = '/service/catv' ");
		}
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template_acct, ebufp);
	}
	else if (*(int*)service_type == 0)
	{
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template_bb, ebufp);
	}
	else if (*(int*)service_type == 1)
	{
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template_catv, ebufp);
	}
	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);

	if ((*(int*)action_flags == 11) || (*(int*)action_flags == 12) || (*(int*)action_flags == 13) || (*(int*)action_flags == 14) || (*(int*)action_flags == 15) || (*(int*)action_flags == 16))
	{
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	}
	else if ((*(int*)action_flags == 21) || (*(int*)action_flags == 22) || (*(int*)action_flags == 23) || (*(int*)action_flags == 24) || (*(int*)action_flags == 25) || (*(int*)action_flags == 26))
	{
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_LOGIN, account_no, ebufp);
	}
	else if ((*(int*)action_flags == 31) || (*(int*)action_flags == 32) || (*(int*)action_flags == 33) || (*(int*)action_flags == 34) || (*(int*)action_flags == 35) || (*(int*)action_flags == 36))
	{
		results_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		PIN_FLIST_FLD_SET(results_flistp, MSO_FLD_AGREEMENT_NO, account_no, ebufp);
	}

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_cust_enquiry search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51601", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Unkown BRM error", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_cust_enquiry search service output flist", search_outflistp);

	results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (!results_flistp)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51600", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account doesn't exists in BRM", ebufp);
		goto CLEANUP;
	}
	/*********************************** Main functionality **********************************************/

	/*********************************************************
	* Retrieve outstanding due
	**********************************************************/
	if ((*(int*)action_flags == 11) || (*(int*)action_flags == 21) || (*(int*)action_flags == 31))
	{
		balgrp_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
		fm_mso_integ_get_outstanding_due(ctxp, balgrp_pdp, &bill_details, ebufp );
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51602", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_integ_get_outstanding_due:", ebufp);
			goto CLEANUP;
		}
		if (bill_details)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, ret_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_success, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
			PIN_FLIST_FLD_COPY(bill_details, PIN_FLD_PENDING_RECV, ret_flistp, PIN_FLD_AMOUNT, ebufp);
		}
	}

	/*********************************************************
	* Retrieve Last Bill Details 
	**********************************************************/
	if ((*(int*)action_flags == 12) || (*(int*)action_flags == 22) || (*(int*)action_flags == 32))
	{
		balgrp_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
		fm_mso_integ_get_last_bill(ctxp, balgrp_pdp, &bill_details, ebufp );
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51603", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_integ_get_last_bill:", ebufp);
			goto CLEANUP;
		}
		if (bill_details)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, ret_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_success, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
			PIN_FLIST_FLD_COPY(bill_details, PIN_FLD_TOTAL_DUE, ret_flistp, PIN_FLD_AMOUNT, ebufp);
			PIN_FLIST_FLD_COPY(bill_details, PIN_FLD_START_T, ret_flistp, PIN_FLD_CYCLE_START_T, ebufp);
			PIN_FLIST_FLD_COPY(bill_details, PIN_FLD_END_T, ret_flistp, PIN_FLD_CYCLE_END_T, ebufp);
			PIN_FLIST_FLD_COPY(bill_details, PIN_FLD_DUE_T, ret_flistp, PIN_FLD_DUE_DATE_T, ebufp);
		}
	}
 	/*********************************************************
	* Retrieve Next Bill Date
	**********************************************************/
	if ((*(int*)action_flags == 13) || (*(int*)action_flags == 23) || (*(int*)action_flags == 33))
	{
		balgrp_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
		fm_mso_integ_get_next_bill_date(ctxp, balgrp_pdp, &bill_details, ebufp );
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51604", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_integ_get_next_bill_date:", ebufp);
			goto CLEANUP;
		}
		if (bill_details)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, ret_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_success, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
			PIN_FLIST_FLD_COPY(bill_details, PIN_FLD_NEXT_BILL_T, ret_flistp, PIN_FLD_NEXT_BILL_T, ebufp);
		}
	}

	/*******************************************************************
	* Retrieve service/plan details
	*******************************************************************/
	
	if ((*(int*)action_flags == 14) || (*(int*)action_flags == 24) || (*(int*)action_flags == 34))
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, ret_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_success, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);

		while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_svc, 1, &cookie_svc, ebufp)) != NULL)
		{
			balgrp_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
			fm_mso_integ_get_service_details(ctxp, balgrp_pdp, &bill_details, ebufp );
		
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51605", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_integ_get_service_details:", ebufp);
				goto CLEANUP;
			}

			services_details = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_SERVICES, elem_svc, ebufp);
			PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_LOGIN, services_details, PIN_FLD_LOGIN, ebufp);
			PIN_FLIST_CONCAT(services_details, bill_details, ebufp); 
		}
	}
	
	
	/*******************************************************************
	* Retrieve plan details
	*******************************************************************/
	
	if ((*(int*)action_flags == 15) || (*(int*)action_flags == 25) || (*(int*)action_flags == 35))
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, ret_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_success, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);

		while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_svc, 1, &cookie_svc, ebufp)) != NULL)
		{
			balgrp_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
			fm_mso_integ_get_plan_details(ctxp, balgrp_pdp, &bill_details, ebufp );
		
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51606", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_integ_get_plan_details:", ebufp);
				goto CLEANUP;
			}

			services_details = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_SERVICES, elem_svc, ebufp);
			PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_LOGIN, services_details, PIN_FLD_LOGIN, ebufp);
			PIN_FLIST_CONCAT(services_details, bill_details, ebufp); 
		}
	}
	

	/*******************************************************************
	* Retrieve latest payment details
	*******************************************************************/
	
	if ((*(int*)action_flags == 16) || (*(int*)action_flags == 26) || (*(int*)action_flags == 36))
	{
		balgrp_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		fm_mso_integ_get_receipts(ctxp, balgrp_pdp, service_type, &bill_details, ebufp );
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51607", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling fm_mso_integ_get_receipts:", ebufp);
			goto CLEANUP;
		}
		if (bill_details)
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, ret_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cust_enquiry_success, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, ret_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
			PIN_FLIST_FLD_COPY(bill_details, PIN_FLD_AMOUNT, ret_flistp, PIN_FLD_AMOUNT, ebufp);
			PIN_FLIST_FLD_COPY(bill_details, PIN_FLD_RECEIPT_NO, ret_flistp, PIN_FLD_RECEIPT_NO, ebufp);
			PIN_FLIST_FLD_COPY(bill_details, PIN_FLD_CREATED_T, ret_flistp, PIN_FLD_CREATED_T, ebufp);			
		}
	}
	
	/*******************************************************************
	* Add plan  - End
	*******************************************************************/

	CLEANUP:	
	*r_flistpp = ret_flistp;
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);	
	return;
}


/**************************************************
* Function: 	fm_mso_integ_get_outstanding_due()
* Owner:        Rohini Mogili
* Decription:    
***************************************************/
static void
fm_mso_integ_get_outstanding_due(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_balgrp_inflistp = NULL;
	pin_flist_t		*read_balgrp_outflistp = NULL;
	pin_flist_t		*read_billinfo_inflistp = NULL;
	pin_flist_t		*read_billinfo_outflistp = NULL;

	poid_t			*billinfo_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_integ_get_outstanding_due", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_outstanding_due balgrp_pdp", balgrp_pdp);

	/******************************************** Reading bal grp **************************************************/

	read_balgrp_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_balgrp_inflistp, PIN_FLD_POID, balgrp_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_balgrp_inflistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_outstanding_due read bal grp input list", read_balgrp_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_balgrp_inflistp, &read_balgrp_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_balgrp_inflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_outstanding_due read bal grp output list", read_balgrp_outflistp);

	billinfo_pdp = PIN_FLIST_FLD_GET(read_balgrp_outflistp, PIN_FLD_BILLINFO_OBJ, 1, ebufp);

	/******************************************** Reading billinfo **************************************************/

	if (!billinfo_pdp)
	{
		goto cleanup;
	}

	read_billinfo_inflistp = PIN_FLIST_CREATE(ebufp);

	read_billinfo_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_billinfo_inflistp, PIN_FLD_POID, billinfo_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_billinfo_inflistp, PIN_FLD_ACTUAL_LAST_BILL_OBJ, NULL, ebufp);	
	PIN_FLIST_FLD_SET(read_billinfo_inflistp, PIN_FLD_PENDING_RECV, NULL, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_outstanding_due read billinfo input list", read_billinfo_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_billinfo_inflistp, &read_billinfo_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_billinfo_inflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_outstanding_due read billinfo output list", read_billinfo_outflistp);

	*ret_flistp = PIN_FLIST_COPY(read_billinfo_outflistp, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_last_bill ret_flistp", *ret_flistp);	

cleanup:
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_FLIST_DESTROY_EX(ret_flistp, NULL);
	}
	PIN_FLIST_DESTROY_EX(&read_balgrp_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_billinfo_outflistp, NULL);
	
	return;
}

/**************************************************
* Function: 	fm_mso_integ_get_last_bill()
* Owner:        Rohini Mogili
* Decription:    
***************************************************/
static void
fm_mso_integ_get_last_bill(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_balgrp_inflistp = NULL;
	pin_flist_t		*read_balgrp_outflistp = NULL;
	pin_flist_t		*read_billinfo_inflistp = NULL;
	pin_flist_t		*read_billinfo_outflistp = NULL;
	pin_flist_t		*read_bill_iflist = NULL;
	pin_flist_t		*read_bill_oflist = NULL;

	poid_t			*billinfo_pdp = NULL;
	poid_t			*last_bill_obj = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_integ_get_last_bill", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_last_bill balgrp_pdp", balgrp_pdp);

	/******************************************** Reading bal grp **************************************************/

	read_balgrp_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_balgrp_inflistp, PIN_FLD_POID, balgrp_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_balgrp_inflistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_last_bill read bal grp input list", read_balgrp_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_balgrp_inflistp, &read_balgrp_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_balgrp_inflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_last_bill read bal grp output list", read_balgrp_outflistp);

	billinfo_pdp = PIN_FLIST_FLD_GET(read_balgrp_outflistp, PIN_FLD_BILLINFO_OBJ, 1, ebufp);

	/******************************************** Reading billinfo **************************************************/

	if (!billinfo_pdp)
	{
		goto cleanup;
	}

	read_billinfo_inflistp = PIN_FLIST_CREATE(ebufp);

	read_billinfo_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_billinfo_inflistp, PIN_FLD_POID, billinfo_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_billinfo_inflistp, PIN_FLD_ACTUAL_LAST_BILL_OBJ, NULL, ebufp);	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_last_bill read billinfo input list", read_billinfo_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_billinfo_inflistp, &read_billinfo_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_billinfo_inflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_last_bill read billinfo output list", read_billinfo_outflistp);

	last_bill_obj = PIN_FLIST_FLD_GET(read_billinfo_outflistp, PIN_FLD_ACTUAL_LAST_BILL_OBJ, 1, ebufp);

	/******************************************** Reading bill **************************************************/

	if (last_bill_obj)
	{
		
		read_bill_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_POID, last_bill_obj, ebufp );
		PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_TOTAL_DUE, NULL, ebufp );
		PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_START_T, NULL, ebufp );
		PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_END_T, NULL, ebufp );
		PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_DUE_T, NULL, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_last_bill read input list", read_bill_iflist);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_bill_iflist, &read_bill_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&read_bill_iflist, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_last_bill read output list", read_bill_oflist);
		
		*ret_flistp = PIN_FLIST_COPY(read_bill_oflist, ebufp);
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_last_bill ret_flistp", *ret_flistp);
	}	

cleanup:
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_FLIST_DESTROY_EX(ret_flistp, NULL);
	}
	PIN_FLIST_DESTROY_EX(&read_bill_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&read_balgrp_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_billinfo_outflistp, NULL);
	
	return;
}



/**************************************************
* Function: 	fm_mso_integ_get_next_bill_date()
* Owner:        Rohini Mogili
* Decription:    
***************************************************/
static void
fm_mso_integ_get_next_bill_date(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_balgrp_inflistp = NULL;
	pin_flist_t		*read_balgrp_outflistp = NULL;
	pin_flist_t		*read_billinfo_inflistp = NULL;
	pin_flist_t		*read_billinfo_outflistp = NULL;

	poid_t			*billinfo_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_integ_get_next_bill_date", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_next_bill_date balgrp_pdp", balgrp_pdp);

	/******************************************** Reading bal grp **************************************************/

	read_balgrp_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_balgrp_inflistp, PIN_FLD_POID, balgrp_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_balgrp_inflistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_next_bill_date read bal grp input list", read_balgrp_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_balgrp_inflistp, &read_balgrp_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_balgrp_inflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_next_bill_date read bal grp output list", read_balgrp_outflistp);

	billinfo_pdp = PIN_FLIST_FLD_GET(read_balgrp_outflistp, PIN_FLD_BILLINFO_OBJ, 1, ebufp);

	/******************************************** Reading billinfo **************************************************/

	if (!billinfo_pdp)
	{
		goto cleanup;
	}

	read_billinfo_inflistp = PIN_FLIST_CREATE(ebufp);

	read_billinfo_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_billinfo_inflistp, PIN_FLD_POID, billinfo_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_billinfo_inflistp, PIN_FLD_ACTUAL_LAST_BILL_OBJ, NULL, ebufp);	
	PIN_FLIST_FLD_SET(read_billinfo_inflistp, PIN_FLD_NEXT_BILL_T, NULL, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_next_bill_date read billinfo input list", read_billinfo_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_billinfo_inflistp, &read_billinfo_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_billinfo_inflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_next_bill_date read billinfo output list", read_billinfo_outflistp);

	*ret_flistp = PIN_FLIST_COPY(read_billinfo_outflistp, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_next_bill_date ret_flistp", *ret_flistp);	

cleanup:
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_FLIST_DESTROY_EX(ret_flistp, NULL);
	}
	PIN_FLIST_DESTROY_EX(&read_balgrp_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_billinfo_outflistp, NULL);	
	return;
}




/**************************************************
* Function: 	fm_mso_integ_get_service_details()
* Owner:        Rohini Mogili
* Decription:    
***************************************************/
static void
fm_mso_integ_get_service_details(
	pcm_context_t		*ctxp,
	poid_t			*service_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*read_plan_inflistp = NULL;
	pin_flist_t		*read_plan_outflistp = NULL;
	pin_flist_t		*rflistp = NULL;

	poid_t			*plan_pdp = NULL;
	int64			db = 0;
	int				search_flags = 768;
	int				elem_id = 0;
	int32			pp_status_active = 1;

	pin_cookie_t	cookie = NULL;

	char			search_template[100] = " service X from /purchased_product where F1 = V1 and F2 = V2 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_integ_get_service_details", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_service_details service_pdp", service_pdp);

	/******************************************** Reading bal grp **************************************************/

	db = PIN_POID_GET_DB(service_pdp);

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);

	if (strcmp(PIN_POID_GET_TYPE(service_pdp), "/account") == 0)
	{
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, service_pdp, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
	}	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &pp_status_active, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_service_details search purchased_product input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_service_details search purchased_product output list", search_outflistp);	

	rflistp = PIN_FLIST_CREATE(ebufp);

	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		plan_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);

		if (plan_pdp)
		{
			read_plan_inflistp = PIN_FLIST_CREATE(ebufp);

			read_plan_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_plan_inflistp, PIN_FLD_POID, plan_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_plan_inflistp, PIN_FLD_NAME, NULL, ebufp);	
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_service_details read plan input list", read_plan_inflistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_plan_inflistp, &read_plan_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&read_plan_inflistp, NULL);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
				goto cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_service_details read plan output list", read_plan_outflistp);

			args_flistp = PIN_FLIST_ELEM_ADD(rflistp, PIN_FLD_PLAN, elem_id, ebufp);
			PIN_FLIST_FLD_COPY(read_plan_outflistp, PIN_FLD_NAME, args_flistp, PIN_FLD_NAME, ebufp);
			PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_PURCHASE_END_T, args_flistp, PIN_FLD_PURCHASE_END_T, ebufp);

			PIN_FLIST_DESTROY_EX(&read_plan_outflistp, NULL);
		}
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_service_details ret_flistp", rflistp);	

	*ret_flistp = PIN_FLIST_COPY(rflistp, ebufp);

cleanup:
	PIN_FLIST_DESTROY_EX(&rflistp, NULL);	
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	return;
}


/**************************************************
* Function: 	fm_mso_integ_get_plan_details()
* Owner:        Rohini Mogili
* Decription:    
***************************************************/
static void
fm_mso_integ_get_plan_details(
	pcm_context_t		*ctxp,
	poid_t			*service_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*read_plan_inflistp = NULL;
	pin_flist_t		*read_plan_outflistp = NULL;
	pin_flist_t		*rflistp = NULL;

	poid_t			*plan_pdp = NULL;
	int64			db = 0;
	int				search_flags = 768;
	int				elem_id = 0;
	int32			pp_status_active = 1;

	pin_cookie_t	cookie = NULL;

	char			search_template[100] = " service X from /purchased_product where F1 = V1 and F2 = V2 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_integ_get_plan_details", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_plan_details service_pdp", service_pdp);

	/******************************************** Reading bal grp **************************************************/

	db = PIN_POID_GET_DB(service_pdp);

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	if (strcmp(PIN_POID_GET_TYPE(service_pdp), "/account") == 0)
	{
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, service_pdp, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
	}	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &pp_status_active, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_plan_details search purchased_product input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_plan_details search purchased_product output list", search_outflistp);	

	rflistp = PIN_FLIST_CREATE(ebufp);

	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		plan_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);

		if (plan_pdp)
		{
			read_plan_inflistp = PIN_FLIST_CREATE(ebufp);

			read_plan_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_plan_inflistp, PIN_FLD_POID, plan_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_plan_inflistp, PIN_FLD_NAME, NULL, ebufp);
			PIN_FLIST_FLD_SET(read_plan_inflistp, PIN_FLD_DESCR, NULL, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_plan_details read plan input list", read_plan_inflistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_plan_inflistp, &read_plan_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&read_plan_inflistp, NULL);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
				goto cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_plan_details read plan output list", read_plan_outflistp);

			args_flistp = PIN_FLIST_ELEM_ADD(rflistp, PIN_FLD_PLAN, elem_id, ebufp);
			PIN_FLIST_FLD_COPY(read_plan_outflistp, PIN_FLD_NAME, args_flistp, PIN_FLD_NAME, ebufp);
			PIN_FLIST_FLD_COPY(read_plan_outflistp, PIN_FLD_DESCR, args_flistp, PIN_FLD_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_PURCHASE_START_T, args_flistp, PIN_FLD_PURCHASE_START_T, ebufp);
			PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_PURCHASE_END_T, args_flistp, PIN_FLD_PURCHASE_END_T, ebufp);

			PIN_FLIST_DESTROY_EX(&read_plan_outflistp, NULL);
		}
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_plan_details ret_flistp", rflistp);	

	*ret_flistp = PIN_FLIST_COPY(rflistp, ebufp);

cleanup:
	PIN_FLIST_DESTROY_EX(&rflistp, NULL);	
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	return;
}


/**************************************************
* Function: 	fm_mso_integ_get_receipts()
* Owner:        Rohini Mogili
* Decription:    
***************************************************/
static void
fm_mso_integ_get_receipts(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	int				*service_type,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*receipts_inflistp = NULL;
	pin_flist_t		*receipts_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*read_plan_inflistp = NULL;
	pin_flist_t		*read_plan_outflistp = NULL;
	pin_flist_t		*rflistp = NULL;

	poid_t			*plan_pdp = NULL;
	int64			db = 0;
	int				search_flags = 0;
	int				elem_id = 0;
	int32			pp_status_active = 1;

	pin_cookie_t	cookie = NULL;

	char			search_template[100] = " service X from /purchased_product where F1 = V1 and F2 = V2 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_integ_get_receipts", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_receipts acct_pdp", acct_pdp);

	/******************************************** Reading bal grp **************************************************/

	receipts_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(receipts_inflistp, PIN_FLD_POID, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(receipts_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(receipts_inflistp, MSO_FLD_SERVICE_TYPE, service_type, ebufp);		

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_receipts search payment input list", receipts_inflistp);
	PCM_OP(ctxp, MSO_OP_PYMT_GET_RECEIPTS, 0, receipts_inflistp, &receipts_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&receipts_inflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PYMT_GET_RECEIPTS", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_receipts search payment output list", receipts_outflistp);	

	rflistp = PIN_FLIST_ELEM_GET(receipts_outflistp, PIN_FLD_RESULTS, 0, 1 , ebufp);
	
	if (rflistp)
	{	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_get_receipts ret_flistp", rflistp);	
		*ret_flistp = PIN_FLIST_COPY(rflistp, ebufp);
	}
	else 
		*ret_flistp = NULL;

cleanup:
	PIN_FLIST_DESTROY_EX(&receipts_outflistp, NULL);
	return;
}
