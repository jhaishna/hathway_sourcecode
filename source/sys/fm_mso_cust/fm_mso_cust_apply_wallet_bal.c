/*******************************************************************
 * File:	fm_mso_cust_apply_wallet_bal.c
 * Opcode:	MSO_OP_CUST_APPLY_WALLET_BAL 
 * Created:	19-DEC-2018	
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * This opcode is to retrieve bouquet ids for the give city (area wise if applicable)
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_apply_wallet_bal.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"

/*******************************************************************
 * MSO_OP_CUST_APPLY_WALLET_BAL 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_apply_wallet_bal(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_apply_wallet_bal(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);


static void
fm_mso_cust_get_acct_bal_grp_poid(
        pcm_context_t   *ctxp,
        char            *account_no,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp);

/*******************************************************************
 * MSO_OP_CUST_APPLY_WALLET_BAL  
 *******************************************************************/
void 
op_mso_cust_apply_wallet_bal(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_APPLY_WALLET_BAL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_apply_wallet_bal error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_apply_wallet_bal input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_apply_wallet_bal(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_apply_wallet_bal error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_apply_wallet_bal output flist", *r_flistpp);
	}
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_apply_wallet_bal(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*bill_debit_flistp = NULL;
	pin_flist_t		*bill_debit_r_flistp = NULL;
	pin_flist_t		*acct_bal_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*debit_flistp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*bal_grp_pdp = NULL;
	char			*account_no = NULL;
	char			*program_name = NULL;
	int32			*currency_p = NULL;
	pin_decimal_t		*amount_p = NULL;
	int32			success = 0;
	int32			set_apply_ncr_bal_failure = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_apply_wallet_bal input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in calling fm_mso_cust_apply_wallet_bal", ebufp);
	}

	account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	currency_p = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CURRENCY, 0, ebufp);
	amount_p = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_apply_wallet_bal error",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_apply_ncr_bal_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84001", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "One/more input data is missing", ebufp);
		goto cleanup;
	}


	if (account_no != NULL && strlen(account_no)>0) {

		fm_mso_cust_get_acct_bal_grp_poid(ctxp, account_no, &acct_bal_flistp, ebufp);
		res_flistp = PIN_FLIST_ELEM_GET(acct_bal_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_apply_wallet_bal error",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_apply_ncr_bal_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84001", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Search Error while getting account and balance group", ebufp);
			goto cleanup;
		}

		if (res_flistp == NULL ) {
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                	ret_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_apply_ncr_bal_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "84000", ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "AccountNo not found", ebufp);
			goto cleanup;
		}
		else {
			acct_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);
			bal_grp_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
		
        		bill_debit_flistp = PIN_FLIST_CREATE(ebufp);
        		PIN_FLIST_FLD_SET(bill_debit_flistp, PIN_FLD_POID, acct_pdp, ebufp);
        		PIN_FLIST_FLD_SET(bill_debit_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
        		PIN_FLIST_FLD_SET(bill_debit_flistp, PIN_FLD_DESCR, program_name, ebufp);
        		PIN_FLIST_FLD_SET(bill_debit_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_pdp, ebufp);
        		debit_flistp = PIN_FLIST_ELEM_ADD(bill_debit_flistp, PIN_FLD_DEBIT, *currency_p, ebufp);
        		PIN_FLIST_FLD_SET(debit_flistp, PIN_FLD_BAL_OPERAND, amount_p, ebufp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_apply_wallet_bal bill debit input flist", bill_debit_flistp);
        		PCM_OP(ctxp, PCM_OP_BILL_DEBIT, 0, bill_debit_flistp, &bill_debit_r_flistp, ebufp);
        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_apply_wallet_bal bill debit output flist", bill_debit_r_flistp);

			if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_apply_wallet_bal bill debit error",ebufp);
                        	PIN_ERRBUF_RESET(ebufp);
			}
			else {
                		ret_flistp = PIN_FLIST_CREATE(ebufp);
                		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, ret_flistp, PIN_FLD_ACCOUNT_NO,ebufp);
                		PIN_FLIST_FLD_COPY(bill_debit_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
                		PIN_FLIST_FLD_COPY(bill_debit_flistp, PIN_FLD_BAL_GRP_OBJ, ret_flistp, PIN_FLD_BAL_GRP_OBJ,ebufp);
                		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &success, ebufp);
                		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS_MSG, "Wallet Balance Applied successfully", ebufp);
			}

		}
	}
	else{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_apply_ncr_bal_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "94002", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account No is null", ebufp);
		goto cleanup;	
	}


cleanup:
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&acct_bal_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bill_debit_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bill_debit_r_flistp, NULL);

	return;
}


static void
fm_mso_cust_get_acct_bal_grp_poid(
        pcm_context_t   *ctxp,
        char           	*account_no,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *srch_iflistp=NULL;
        pin_flist_t     *srch_oflistp=NULL;
        pin_flist_t     *arg_flistp=NULL;
        pin_flist_t     *res_flistp=NULL;

        char    *template="select X from /account where F1 = V1 ";
        int64   db = 1;
        int32   srch_flag = 256;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fm_mso_cust_get_acct_bal_grp_poid ", ebufp);
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_acct_bal_grp_poid entered ");

        srch_iflistp= PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
        
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_acct_bal_grp_poid search input flist is : ", srch_iflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_acct_bal_grp_poid search output flist is : ", srch_oflistp);

	*ret_flistp = PIN_FLIST_COPY(srch_oflistp, ebufp);

        PIN_FLIST_DESTROY_EX(&srch_iflistp,NULL);
        PIN_FLIST_DESTROY_EX(&srch_oflistp,NULL);
        return;
}

