/*******************************************************************
 * File:	fm_mso_cust_get_invoice.c
 * Opcode:	MSO_OP_CUST_GET_INVOICE 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 *
	0 PIN_FLD_USERID          POID [0] 0.0.0.1 /account 128831
	0 PIN_FLD_POID            POID [0] 0.0.0.1 /account 128831
	0 PIN_FLD_BILL_NO          STR [0] "B1-4"
********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_invoice.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_bill.h"
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
 * MSO_OP_CUST_GET_INVOICE 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_get_invoice(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_invoice(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_GET_INVOICE  
 *******************************************************************/
void 
op_mso_cust_get_invoice(
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

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_GET_INVOICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_get_invoice error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_get_invoice input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_get_invoice(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_get_invoice error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_get_invoice output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_get_invoice(
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
	pin_flist_t		*formats = NULL;


	poid_t			*user_id = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;

	char			*bill_no = NULL;

	char			*template = "select X from /invoice where F1 = V1 and F2.id = V2 " ;

	int32			search_inv_failure = 1;
	int32			search_inv_success = 0;
	int32			srch_flag = 768;
	int32			elem_count = 0;

	int64			db = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_invoice input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_get_invoice", ebufp);
		goto CLEANUP;
	}

	user_id =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID , 1, ebufp );
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ); 
	bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO, 1, ebufp ); 

	db = PIN_POID_GET_DB(user_id);

  	/*******************************************************************
	* Mandatory fields validation
	*******************************************************************/
	if (!user_id || !acnt_pdp || !bill_no )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"MISSING ACCOUNT_NO or BILL_NUMBER", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &search_inv_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51190", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "MISSING ACCOUNT_NO or BILL_NUMBER", ebufp);
		goto CLEANUP;
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
	formats = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_FORMATS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(formats, PIN_FLD_BUFFER, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_invoice SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_invoice SESRCH output flist", srch_out_flistp);

	elem_count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp );
	if (elem_count == 0)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invoice does not exist", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &search_inv_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51191", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Invoice does not exist", ebufp);
		goto CLEANUP;
	}

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, elem_count-1, 1, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, acnt_pdp, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, &search_inv_success, ebufp );
	*ret_flistp = PIN_FLIST_COPY(result_flist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

