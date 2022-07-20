/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 * Version 1.0: 07-08-2014: Harish Kulkarni: Added the MSO Customization of
 * reversing the payment from incorrect account and posting it to correct account. 
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_corrective_payment.c /cgbubrm_7.5.0 2013/09/26 04:39:34 harishk Exp $";
#endif

/*******************************************************************
* Contains the MSO_OP_PYMT_CORRECTION operation. 
* Input Flist:
* 0 PIN_FLD_POID		    POID [0] 0.0.0.1 /account 1 0	--> Dummy poid
* 0 PIN_FLD_RECEIPT_NO        STR [0] "RECEIPT-660"		--> Original Payment Receipt
* 0 PIN_FLD_ACCOUNT_NO        STR [0] "1000000367"		--> Destination A/C no.
* 0 PIN_FLD_USERID            POID [0] 0.0.0.1 /account 50541 0	--> User ID
*******************************************************************/

#include <stdio.h> 
#include <string.h> 
#include <time.h>
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_rate.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_ar.h"
#include "ops/bill.h"
#include "psiu_business_params.h"
#include "mso_ops_flds.h"

#define PAY_TYPE_CASH 10011
#define PAY_TYPE_CHECK 10012
#define PAY_TYPE_ONLINE 10013
#define PAY_TYPE_ECS 10020
#define LOCAL_TRANS_OPEN_SUCCESS 0
#define READWRITE 1

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_pymt_correction(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
mso_op_pymt_correction(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void mso_find_receipt(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void mso_find_account(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
mso_op_pymt_reverse_pymt(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
mso_op_pymt_make_pymt(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*recp_flistp,
	poid_t			*dest_acctp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT int32 fm_mso_trans_open(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	int32			flag,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_trans_commit(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_trans_abort(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PYMT_CORRECTION operation.
 *******************************************************************/
void
mso_pymt_correction(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	int			status = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_CORRECTION) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_pymt_corrective_payment opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_corrective_payment input flist", i_flistp);

	/***********************************************************
	 * Call the routine to make the payment correction
	 ***********************************************************/
	mso_op_pymt_correction(ctxp, i_flistp, &r_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unknown Error occured in MSO_OP_PYMT_CORRECTION.");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"Unknown Error occured in MSO_OP_PYMT_CORRECTION.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	*o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_corrective_payment return flist", *o_flistpp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	return;
}

static void
mso_op_pymt_correction(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*acct_srchp = NULL;
	pin_flist_t		*acct_resp = NULL;
	pin_flist_t		*pymt_revp = NULL;
	pin_flist_t		*pymt_depp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*recp_flistp = NULL;
	pin_flist_t		*recp_resultp = NULL;
	poid_t			*dest_acctp = NULL;
	poid_t			*a_pdp = NULL;
	int32			status = 0;
	int32			*pymt_status = NULL;
	int			local = 1;
	char			*dest_acct_no = NULL;
	char			*src_acct_no = NULL;
	char			descr[255]="";
	char			*receipt_no = NULL;

	void			*vp = NULL;

	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, a_pdp, READWRITE, ebufp);	
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in opening transactions", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, a_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53004", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in opening transactions", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Opened");

	/* Call the function to get the destination account poid */
	mso_find_account(ctxp, i_flistp, &acct_srchp, ebufp);
	acct_resp = PIN_FLIST_ELEM_GET(acct_srchp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp) || acct_resp == (pin_flist_t *) NULL)
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error retrieving the destination account details.");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"Error retrieving the destination account details.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}
	dest_acctp = PIN_FLIST_FLD_GET(acct_resp, PIN_FLD_POID, 0, ebufp);
	dest_acct_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

	/* Check if the source and destination accounts are same. If so, error out */

	/* Get the /mso_rceipt object details for the given receipt number */
	mso_find_receipt(ctxp,i_flistp,&recp_flistp, ebufp);
	recp_resultp = PIN_FLIST_ELEM_GET(recp_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp) || recp_resultp == (pin_flist_t *) NULL)
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error retrieving the original payment receipt.");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
				"Error retrieving the original payment receipt.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	/* Get the pay_type of the payment */
	src_acct_no = PIN_FLIST_FLD_GET(recp_resultp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	receipt_no  = PIN_FLIST_FLD_GET(recp_resultp, PIN_FLD_RECEIPT_NO, 0, ebufp);

	if(src_acct_no && dest_acct_no && strcmp(src_acct_no, dest_acct_no) == 0)
	{
		status = 1;
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Source and destination accounts are same!!");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
				"Source and destination accounts are same!!", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	/* Call the function to reverse the wrong payment */
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	sprintf(descr, "Corrective Payment: Transferred to A/C: %s.%s", dest_acct_no, (char*)vp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DESCR, descr, ebufp );
	mso_op_pymt_reverse_pymt(ctxp, i_flistp, &pymt_revp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		goto CLEANUP;
	}
	if (pymt_revp != NULL)
	{
		pymt_status = PIN_FLIST_FLD_GET(pymt_revp, PIN_FLD_STATUS, 0, ebufp);
		if (*pymt_status == 1)
		{
			status = 1;
			*o_flistpp = PIN_FLIST_COPY(pymt_revp, ebufp);
			goto CLEANUP;
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Payment Reversal Output:", pymt_revp);

	/* Call the function to make the payment to a correct account */

	//memset(descr, '\0', 255);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	sprintf(descr, "Corrective Payment: Received from A/C: %s and receipt no: %s.%s", src_acct_no, receipt_no, (char*)vp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DESCR, descr, ebufp );
	mso_op_pymt_make_pymt(ctxp, i_flistp, recp_flistp, dest_acctp, &pymt_depp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		goto CLEANUP;
	}
	if (pymt_depp != NULL)
	{
		pymt_status = PIN_FLIST_FLD_GET(pymt_depp, PIN_FLD_STATUS, 0, ebufp);
		if (*pymt_status == 1)
		{
			status = 1;
			*o_flistpp = PIN_FLIST_COPY(pymt_depp, ebufp);
			goto CLEANUP;
		}
		else
		{
			*o_flistpp = PIN_FLIST_COPY(pymt_depp, ebufp);
			goto CLEANUP;
		}
	}

	CLEANUP:
		if (status == 1)
		{
			if(local == LOCAL_TRANS_OPEN_SUCCESS )
			{
				fm_mso_trans_abort(ctxp, a_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Aborted");
			}
		}
		else
		{
			if(local == LOCAL_TRANS_OPEN_SUCCESS )
			{
				fm_mso_trans_commit(ctxp, a_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Committed");
			}
		}
		PIN_FLIST_DESTROY_EX(&acct_srchp, NULL);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&pymt_revp, NULL);
		PIN_FLIST_DESTROY_EX(&pymt_depp, NULL);
		PIN_FLIST_DESTROY_EX(&recp_flistp, NULL);

	return;
}

static void
mso_op_pymt_reverse_pymt(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	poid_t			*user_id = NULL;
	u_int64			id = (u_int64)-1;
	char			*reason_code = "Incorrect Account";
	char			*receipt_no = NULL;
	int32			status = 0;
	int			reasonid = 1031;

	/*******************************************************************
	* MSO_OP_PYMT_REVERSE_PAYMENT Input:
	* 0 PIN_FLD_POID          POID [0] 0.0.0.1 /account 1 0
	* 0 PIN_FLD_REASON_CODE                STR [0] "Incorrect Account"
	* 0 PIN_FLD_REASON_ID                INT [0] 1031
	* 0 PIN_FLD_RECEIPT_NO                 STR [0] "RECEIPT-660"
	* 0 PIN_FLD_USERID        POID [0] 0.0.0.1 /account 50541 0
	*******************************************************************/
	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_REASON_CODE, reason_code, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_REASON_ID, &reasonid, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_RECEIPT_NO, in_flistp, PIN_FLD_RECEIPT_NO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, in_flistp, PIN_FLD_DESCR, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, in_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"MSO_OP_PYMT_REVERSE_PAYMENT Input flist", in_flistp);
	PCM_OP(ctxp, MSO_OP_PYMT_REVERSE_PAYMENT, 0, in_flistp, o_flistpp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"MSO_OP_PYMT_REVERSE_PAYMENT Output flist",*o_flistpp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		status = 1;
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Reversal Error");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Reversal Error", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	return;

}

static void
mso_op_pymt_make_pymt(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*recp_flistp,
	poid_t			*dest_acctp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t	*ebufp)
{
	//pin_flist_t		*recp_flistp = NULL;
	pin_flist_t		*recp_resultp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;

	pin_flist_t		*charge_flistp = NULL;
	pin_flist_t		*pymt_flistp = NULL;
	pin_flist_t		*inh_info_flistp = NULL;
	pin_flist_t		*sub_info_flistp = NULL;

	int32			status = 0;
	int32			*pay_type = NULL;
	char			*pgm_name = NULL;
	char			*pgm_name_chk = "MSO_OP_PYMT_COLLECT|check pymt";
	char			*pgm_name_csh = "MSO_OP_PYMT_COLLECT|cash pymt";
	char			*pgm_name_online = "MSO_OP_PYMT_COLLECT|online pymt";
//	char			*descr = "Corrective Payment";
	char			program_name[60] = "";
	char			*descr_1 = NULL;
	char			*descr_2 = NULL;
	char			descr[255] = "";
	int32			pymt_channel = 0;
	int32			command = 0;

	/* Get the /mso_rceipt object details for the given receipt number */
	//mso_find_receipt(ctxp,i_flistp,&recp_flistp, ebufp);
	recp_resultp = PIN_FLIST_ELEM_GET(recp_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	pgm_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp) || recp_resultp == (pin_flist_t *) NULL)
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error retrieving the original payment receipt.");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"Error retrieving the original payment receipt.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	/* Get the pay_type of the payment */
	pay_type = PIN_FLIST_FLD_GET(recp_resultp, PIN_FLD_PAY_TYPE, 0, ebufp);

	/* Based on the pay_type, construct the input flist for MSO_OP_PYMT_COLLECT */
	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, dest_acctp, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_PYMT_CHANNEL, &pymt_channel, ebufp);
	//PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_DESCR, descr, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, in_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_TYPE, in_flistp, PIN_FLD_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_START_T, in_flistp, PIN_FLD_START_T, ebufp);
	PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_END_T, in_flistp, PIN_FLD_END_T, ebufp);
	PIN_FLIST_FLD_COPY(recp_resultp, MSO_FLD_SERVICE_TYPE, in_flistp, MSO_FLD_SERVICE_TYPE, ebufp);

	descr_1 = PIN_FLIST_FLD_GET(recp_resultp, PIN_FLD_DESCR, 1, ebufp);
	descr_2 = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	sprintf (descr, "%s(%s)",descr_1,descr_2);
	
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_DESCR, descr, ebufp);

	charge_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_CHARGES, 0, ebufp);
	PIN_FLIST_FLD_SET(charge_flistp, PIN_FLD_ACCOUNT_OBJ, dest_acctp, ebufp);
	PIN_FLIST_FLD_SET(charge_flistp, PIN_FLD_COMMAND, &command, ebufp);
	PIN_FLIST_FLD_SET(charge_flistp, PIN_FLD_PAY_TYPE, pay_type, ebufp);
	PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_AMOUNT, charge_flistp, PIN_FLD_AMOUNT, ebufp);

	pymt_flistp = PIN_FLIST_SUBSTR_ADD(charge_flistp, PIN_FLD_PAYMENT, ebufp);
	inh_info_flistp = PIN_FLIST_SUBSTR_ADD(pymt_flistp, PIN_FLD_INHERITED_INFO, ebufp);

	if (*pay_type == PAY_TYPE_CHECK)
	{
		if (pgm_name)
		{
			strcpy(program_name, pgm_name);
			strcat(program_name, "|");
			strcat(program_name, pgm_name_chk);
		}
		else
		{
			strcpy(program_name, pgm_name_chk);
		}

		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
		sub_info_flistp = PIN_FLIST_ELEM_ADD(inh_info_flistp, PIN_FLD_CHECK_INFO, 0, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_CHECK_NO, sub_info_flistp, PIN_FLD_CHECK_NO, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_BANK_CODE, sub_info_flistp, PIN_FLD_BANK_CODE, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_BANK_ACCOUNT_NO, sub_info_flistp, PIN_FLD_BANK_ACCOUNT_NO, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_REFERENCE_ID, sub_info_flistp, PIN_FLD_RECEIPT_NO, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_BANK_NAME, sub_info_flistp, PIN_FLD_BANK_NAME, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_BRANCH_NO, sub_info_flistp, PIN_FLD_BRANCH_NO, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_EFFECTIVE_T, sub_info_flistp, PIN_FLD_EFFECTIVE_T, ebufp);

	}
	else if (*pay_type == PAY_TYPE_CASH)
	{
		if (pgm_name)
		{
			strcpy(program_name, pgm_name);
			strcat(program_name, "|");
			strcat(program_name, pgm_name_csh);
		}
		else
		{
			strcpy(program_name, pgm_name_csh);
		}
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
		sub_info_flistp = PIN_FLIST_ELEM_ADD(inh_info_flistp, PIN_FLD_CASH_INFO, 0, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_REFERENCE_ID, sub_info_flistp, PIN_FLD_RECEIPT_NO, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_EFFECTIVE_T, sub_info_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
	}
	else if (*pay_type == PAY_TYPE_ONLINE || *pay_type == PAY_TYPE_ECS)
	{
		if (pgm_name)
		{
			strcpy(program_name, pgm_name);
			strcat(program_name, "|");
			strcat(program_name, pgm_name_online);
		}
		else
		{
			strcpy(program_name, pgm_name_online);
		}
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
		sub_info_flistp = PIN_FLIST_ELEM_ADD(inh_info_flistp, PIN_FLD_WIRE_TRANSFER_INFO, 0, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_REFERENCE_ID, sub_info_flistp, PIN_FLD_RECEIPT_NO, ebufp);
		PIN_FLIST_FLD_COPY(recp_resultp, PIN_FLD_EFFECTIVE_T, sub_info_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
	}
	else
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Invalid Pay_Type");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"Invalid payment method.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"MSO_OP_PYMT_COLLECT Input flist", in_flistp);
	PCM_OP(ctxp, MSO_OP_PYMT_COLLECT, 0, in_flistp, o_flistpp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"MSO_OP_PYMT_COLLECT Output flist",*o_flistpp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error Making the Payment");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"Error Making the Payment.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	CLEANUP:
		//PIN_FLIST_DESTROY_EX(&recp_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);		
		PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	return;
}

/* This Module will retrieve the Payment details for the Customer 
 * from Receipt No in the /mso_receipt object. */
static void mso_find_receipt(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int32			s_flags = 256;
	int32			status = 0;
	poid_t			*pdp = NULL;
	int64			db = 0;
	poid_t			*user_id = NULL;

	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	db = PIN_POID_GET_DB(user_id);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_receipt where F1 = V1 ", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_RECEIPT_NO, flistp, PIN_FLD_RECEIPT_NO, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_receipt search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_receipt search output flist",*r_flistpp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
}

/* This Module will retrieve the Account details for the Customer 
 * from account number. */
static void mso_find_account(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	poid_t			*user_id = NULL;
	int32			s_flags = 256;
	int32			status = 0;
	poid_t			*pdp = NULL;
	int64			db = 0;
	char			*rcpt_no = NULL;

		
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	db = PIN_POID_GET_DB(user_id);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /account where F1 = V1 ", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, flistp, PIN_FLD_ACCOUNT_NO, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"account search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"account search output flist",*r_flistpp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
}
