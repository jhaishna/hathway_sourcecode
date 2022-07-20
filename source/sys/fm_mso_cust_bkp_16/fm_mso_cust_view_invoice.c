/*******************************************************************
 * File:	fm_mso_cust_view_invoice.c
 * Opcode:	MSO_OP_CUST_VIEW_INVOICE 
 * Owner:	Sangappa Bellanki
 * Created:	07-AUG-2014
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 *
	0 PIN_FLD_USERID	POID [0] 0.0.0.1 /account 128831
	0 PIN_FLD_POID		POID [0] 0.0.0.1 /account 128831
	0 PIN_FLD_BILL_NO	STR [0] "B1-4"
	0 PIN_FLD_FLAGS		INT  [0] 0	
********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_view_invoice.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "pin_inv.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"


/**************************************
* External Functions start
***************************************/

/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_VIEW_INVOICE 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_view_invoice(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_view_invoice(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_create_template_report(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_VIEW_INVOICE  
 *******************************************************************/
void 
op_mso_cust_view_invoice(
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

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "op_mso_cust_view_invoice: start.");

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_VIEW_INVOICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_view_invoice error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_view_invoice input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_view_invoice(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_view_invoice error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_view_invoice output flist", *r_flistpp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "op_mso_cust_view_invoice: end.");
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_view_invoice(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*update_ssa_iflist = NULL;
	pin_flist_t		*update_ssa_oflist = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;


	poid_t			*user_id = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;

	char			*bill_no = NULL;

	char			*template = "select X from /invoice where F1 = V1 and F2 = V2 ";

	char			search_inv_failure[8] = "FAILURE";
	char			search_inv_success[8] = "SUCCESS";
	int32			srch_flag = 768;
	int32			elem_count = 0;
	int32			success    = 0;
	int32			fail       = 1;

	int64			db = -1;
	u_int			*type = NULL;	
	int32			inv_status = -1;
	int32			*inv_flag = NULL;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_view_invoice: start.");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_view_invoice input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_view_invoice", ebufp);
		goto cleanup;
	}

	user_id =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID , 0, ebufp );
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ); 
	bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO, 0, ebufp ); 
	inv_flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp ); 

	db = PIN_POID_GET_DB(user_id);

  	/*******************************************************************
	* Mandatory fields validation
	*******************************************************************/
	if (!user_id || !acnt_pdp || !bill_no || !inv_flag)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"MISSING ACCOUNT_NO or BILL_NUMBER or FLAG", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &search_inv_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51190", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "MISSING ACCOUNT_NO or BILL_NUMBER OR FLAG ", ebufp);
		goto cleanup;
	}

	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILL_NO, bill_no, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	//PIN_FLIST_FLD_SET(result_flist, PIN_FLD_INVOICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_TEMPLATE_NAME, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_REPORT_NAME, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp);
	if((inv_flag != NULL) && (*inv_flag) == 1)
	{
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PATH, NULL, ebufp);	
	}


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_view_invoice SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_view_invoice SESRCH output flist", srch_out_flistp);

	elem_count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp );
	if (elem_count == 0)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invoice does not exist", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS_MSG, search_inv_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &fail, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51191", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Invoice does not exist", ebufp);
		goto cleanup;
	}
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, elem_count-1, 1, ebufp);
	if (result_flist)
	{
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, result_flist, PIN_FLD_INVOICE_OBJ, ebufp);
		PIN_FLIST_FLD_RENAME(result_flist, PIN_FLD_STATUS, PIN_FLD_BILLING_STATUS, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, &success, ebufp);
	}
	
	//PIN_FLIST_FLD_RENAME(result_flist, PIN_FLD_POID, PIN_FLD_INVOICE_OBJ, ebufp );
/*	type = (u_int *) PIN_FLIST_FLD_GET(result_flist, PIN_FLD_STATUS, 1, ebufp);
	switch( *type )
	{
		case PIN_INV_STATUS_PENDING:
			inv_status = PIN_INV_STATUS_PENDING; 		
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, (void *)&inv_status, ebufp);
			break;
		case PIN_INV_STATUS_GENERATED:
			inv_status = PIN_INV_STATUS_GENERATED; 		
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, (void *)&inv_status, ebufp);
			break;
		case PIN_INV_STATUS_REGENERATED:
			inv_status = PIN_INV_STATUS_REGENERATED; 		
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, (void *)&inv_status, ebufp);
			break;
		case PIN_INV_STATUS_SCHEDULED:
			inv_status = PIN_INV_STATUS_SCHEDULED; 		
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, (void *)&inv_status, ebufp);
			break;
		case PIN_INV_STATUS_ERRORED:
			inv_status = PIN_INV_STATUS_ERRORED; 		
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, (void *)&inv_status, ebufp);
			break;
		case PIN_INV_STATUS_DUPLICATED:
			inv_status = PIN_INV_STATUS_DUPLICATED; 		
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, (void *)&inv_status, ebufp);
			break;
		default:
			PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, (void *)&inv_status, ebufp);
	}
*/
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, acnt_pdp, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS_MSG, search_inv_success, ebufp);
	*ret_flistp = PIN_FLIST_COPY(result_flist, ebufp);
	//fm_mso_cust_create_template_report(ctxp, result_flist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	
cleanup:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_view_invoice: end.");
	return;
}

//void
//fm_mso_cust_create_template_report(
//	pcm_context_t	*ctxp,
//	pin_flist_t	*i_flistp,
//	pin_errbuf_t	*ebufp)
//{
//	pin_flist_t	*create_obj_i_flist = NULL; 
//	pin_flist_t	*ret_flistp = NULL; 
//	poid_t		*report_poidp = NULL;
//
//
//	if(PIN_ERRBUF_IS_ERR(ebufp)) { return;}
//
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_template_report: start.");
//	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_template_report input", i_flistp);
//
//
//	create_obj_i_flist = PIN_FLIST_CREATE(ebufp);
//	report_poidp = PIN_POID_CREATE(1, "/mso_cfg_inv_tmpl_ver", -1, ebufp);
//	PIN_FLIST_FLD_PUT( create_obj_i_flist, PIN_FLD_POID, report_poidp, ebufp );
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, create_obj_i_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, create_obj_i_flist, PIN_FLD_START_T, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, create_obj_i_flist, PIN_FLD_END_T, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TEMPLATE_NAME, create_obj_i_flist, PIN_FLD_TEMPLATE_NAME, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REPORT_NAME, create_obj_i_flist, PIN_FLD_REPORT_NAME, ebufp);
//
//	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_template_report: create obj flist ", create_obj_i_flist);
//	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_obj_i_flist, &ret_flistp, ebufp);
//	if (PIN_ERRBUF_IS_ERR(ebufp)) {
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//			"fm_mso_cust_create_template_report: "
//			"CREATE OBJ error",
//			ebufp);
//		goto cleanup;
//	}
//	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_template_report ", ret_flistp);
//
//cleanup:
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_template_report: end.");
//	PIN_FLIST_DESTROY_EX(&create_obj_i_flist, ebufp);
//	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
//
//}
