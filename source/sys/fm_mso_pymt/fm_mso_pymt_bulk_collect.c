/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 * 
 * Version 1.0: 26-11-2013: Vilva Sabarikanth: Added the MSO Customization of
 * preparing the Payments Input FLIST for MSO_OP_PYMT_BULK_COLLECT opcode 
 * from LCO through OAP. This opcode creates a master object and individual payment
 * objects that are pulled by MTA for individual payment processing.
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_bulk_collect.c  /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_BULK_COLLECT operation. 
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

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1

#define READONLY 0
#define READWRITE 1
#define LOCK_OBJ 2

#define PAY_TYPE_CASH 10011
#define PAY_TYPE_CHECK 10012
#define PAY_TYPE_ONLINE 10013
#define PAY_TYPE_ECS 10020
/*******************************************************************
 * Routines contained within.
 *******************************************************************/
void
mso_pymt_bulk_collect(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static int
fm_mso_check_ref_id_unique(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	pin_flist_t		**o_flistp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT int32 fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PYMT_BULK_COLLECT operation.
 *******************************************************************/
void
mso_pymt_bulk_collect(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	int			*cmdp = NULL;
	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	pin_flist_t		*a_flistp = NULL;
	pin_flist_t		*blk_i_flistp = NULL;
	pin_flist_t		*pymt_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;

	poid_t			*pymt_pdp = NULL;
	poid_t			*blk_pdp = NULL;
	poid_t			*lco_pdp = NULL;
	poid_t			*user_id = NULL;
	pin_cookie_t		cookie = NULL;
	int32			p_status = 0;
	int32			rec_id = 0;
	int32			elem = 0;
	int32			*pay_type = NULL;
	int32			ret_val = 1;
	int64			db = 0;
	u_int64			id = (u_int64)-1;

	char			*acct_no;
	char			gateway[64]="";
	char			msg[128]="";
	char			*ref_id = NULL;

	time_t			*coll_time = NULL;
	time_t			n_coll_time = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_BULK_COLLECT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_pymt_bulk_collect opcode error", ebufp);
		return;
	}
	
	blk_i_flistp = PIN_FLIST_CREATE(ebufp);
	pymt_flistp = PIN_FLIST_CREATE(ebufp);
	err_flistp = PIN_FLIST_CREATE(ebufp);
	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_bulk_collect input flist", i_flistp);
	lco_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	//Local Transaction Management for Bulk Payment Objects 
	// Putting the transaction flag to READWRITE since this is a bulk operation.
	local = fm_mso_trans_open(ctxp, lco_pdp, READWRITE, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error before processing bulk payments", ebufp);
		goto Cleanup;
	}

	/***********************************************************
	 * fm_mso_check_ref_id_unique()
	 check if 'reference id' passed at file level is unique
	 ***********************************************************/
	
	ret_val = fm_mso_check_ref_id_unique(ctxp,i_flistp, &err_flistp, ebufp);
	sprintf(msg, "ret_val='%d'", ret_val);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	if (ret_val !=0 )
	{

		p_status=1;
		ref_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REFERENCE_ID, 1, ebufp);
		sprintf(msg, "Receipt no '%s' already exist.", ref_id);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, msg);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &p_status, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53030", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto Cleanup;
	}


	//Retrieving the required params from the i_flistp
	coll_time = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PYMT_COLLECTION_T, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERRBUF_CLEAR(ebufp);
		n_coll_time = pin_virtual_time((time_t *)NULL);
	}
	else 
	{
		n_coll_time = *coll_time;
	}

	db = PIN_POID_GET_DB(lco_pdp);
	blk_pdp = PIN_POID_CREATE(db, "/mso_lco_blk_payment", id, ebufp);
	pymt_pdp = PIN_POID_CREATE(db, "/mso_lco_payment", id, ebufp);

	//Setting the parameters to the bulk payment object creation input flist 
	PIN_FLIST_FLD_PUT(blk_i_flistp, PIN_FLD_POID, (void *)blk_pdp, ebufp);
	PIN_FLIST_FLD_PUT(blk_i_flistp, MSO_FLD_LCO_OBJ, (void *)lco_pdp, ebufp);
	fm_mso_get_account_info(ctxp, lco_pdp, &acnt_info, ebufp);
	if(acnt_info)
	{
		PIN_FLIST_FLD_COPY(acnt_info, PIN_FLD_ACCOUNT_NO, blk_i_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
	}
	
	PIN_FLIST_FLD_SET(blk_i_flistp, MSO_FLD_PYMT_COLLECTION_T, &n_coll_time, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AMOUNT_TOTAL, blk_i_flistp, MSO_FLD_AMOUNT_TOTAL,  ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REFERENCE_ID, blk_i_flistp, PIN_FLD_REFERENCE_ID,  ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, blk_i_flistp, PIN_FLD_PROGRAM_NAME,  ebufp);
	PIN_FLIST_FLD_SET(blk_i_flistp, PIN_FLD_USERID, (void *)user_id, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Bulk Payment Object Creation input flist", blk_i_flistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, blk_i_flistp, &r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bulk Payment Object Creation output flist", r_flistp);

	blk_pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 0, ebufp);
	while((a_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, MSO_FLD_PAYMENT_DATA, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
	{
		PIN_FLIST_FLD_PUT(pymt_flistp, PIN_FLD_POID, (void *)pymt_pdp, ebufp);
		PIN_FLIST_FLD_SET(pymt_flistp, MSO_FLD_LCO_OBJ, (void *)lco_pdp, ebufp);
		PIN_FLIST_FLD_SET(pymt_flistp, MSO_FLD_LCO_BLK_PYMT_OBJ, (void *)blk_pdp, ebufp);
		PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_ACCOUNT_NO, pymt_flistp, PIN_FLD_ACCOUNT_NO,  ebufp);
		PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_AMOUNT, pymt_flistp, PIN_FLD_AMOUNT,  ebufp);
		PIN_FLIST_FLD_COPY(a_flistp, MSO_FLD_SERVICE_TYPE, pymt_flistp, MSO_FLD_SERVICE_TYPE,  ebufp);
		PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_REFERENCE_ID, pymt_flistp, PIN_FLD_REFERENCE_ID,  ebufp);
		PIN_FLIST_FLD_COPY(a_flistp, MSO_FLD_PYMT_COLLECTION_T, pymt_flistp, MSO_FLD_PYMT_COLLECTION_T,  ebufp);
		PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_START_T, pymt_flistp, PIN_FLD_START_T,  ebufp);
		PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_DESCR, pymt_flistp, PIN_FLD_DESCR,  ebufp);
		PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_END_T, pymt_flistp, PIN_FLD_END_T,  ebufp);
//		PIN_FLIST_FLD_SET(pymt_flistp, MSO_FLD_PYMT_COLLECTION_T, (void *)coll_time, ebufp);
		PIN_FLIST_FLD_SET(pymt_flistp, MSO_FLD_PYMT_STATUS, (void *)&p_status, ebufp);
		pay_type = (int32 *)PIN_FLIST_FLD_GET(a_flistp, PIN_FLD_PAY_TYPE, 0, ebufp);
		if (*pay_type == PAY_TYPE_CHECK)
		{
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_BANK_CODE, pymt_flistp, PIN_FLD_BANK_CODE,  ebufp);
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_BANK_ACCOUNT_NO, pymt_flistp, PIN_FLD_BANK_ACCOUNT_NO,  ebufp);
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_BANK_NAME, pymt_flistp, PIN_FLD_BANK_NAME,  ebufp);
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_BRANCH_NO, pymt_flistp, PIN_FLD_BRANCH_NO,  ebufp);
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_CHECK_NO, pymt_flistp, PIN_FLD_CHECK_NO,  ebufp);
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_EFFECTIVE_T, pymt_flistp, PIN_FLD_EFFECTIVE_T,  ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unknown Account Details for Cheque Payment");
				p_status=1;
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, lco_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &p_status, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53030", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
				*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
				goto Cleanup;
			}
		}
		else if (*pay_type == PAY_TYPE_ONLINE || *pay_type == PAY_TYPE_ECS)
		{
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_BANK_CODE, pymt_flistp, PIN_FLD_BANK_CODE,  ebufp);
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_BANK_ACCOUNT_NO, pymt_flistp, PIN_FLD_BANK_ACCOUNT_NO,  ebufp);

			//sprintf(gateway, "%s:%s", "Gateway", (char*)(PIN_FLIST_FLD_GET(a_flistp, PIN_FLD_BANK_NAME, 1,ebufp)));
			//PIN_FLIST_FLD_SET(pymt_flistp, PIN_FLD_BANK_NAME, gateway, ebufp);
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_BANK_NAME, pymt_flistp, PIN_FLD_BANK_NAME,  ebufp);

			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_IP_ADDRESS, pymt_flistp, PIN_FLD_IP_ADDRESS,  ebufp);
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_BRANCH_NO, pymt_flistp, PIN_FLD_BRANCH_NO,  ebufp);
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_CHECK_NO, pymt_flistp, PIN_FLD_CHECK_NO,  ebufp);
			PIN_FLIST_FLD_COPY(a_flistp, PIN_FLD_EFFECTIVE_T, pymt_flistp, PIN_FLD_EFFECTIVE_T,  ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unknown Account Details for Cheque Payment");
				p_status=1;
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, lco_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &p_status, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53030", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
				*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
				goto Cleanup;
			}
		}
		else
		{
			PIN_FLIST_FLD_SET(pymt_flistp, PIN_FLD_BANK_CODE, NULL, ebufp);
			PIN_FLIST_FLD_SET(pymt_flistp, PIN_FLD_BANK_ACCOUNT_NO, NULL, ebufp);
		}

		PIN_FLIST_FLD_SET(pymt_flistp, PIN_FLD_PAY_TYPE, pay_type, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, pymt_flistp, PIN_FLD_PROGRAM_NAME,  ebufp);
		PIN_FLIST_FLD_SET(pymt_flistp, PIN_FLD_USERID, (void *)user_id, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"LCO Payment Object Creation input flist", pymt_flistp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, pymt_flistp, &r_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO Payment Object Creation output flist", r_flistp);
	}

Cleanup:
	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_pymt_bulk_collect error", ebufp);
		*o_flistpp = NULL;
		PIN_ERRBUF_RESET(ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, lco_pdp, ebufp);
		}
		p_status=1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, lco_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &p_status, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53031", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Bulk Object Creation Failed", ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERRBUF_CLEAR(ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_commit(ctxp, lco_pdp, ebufp);
		}
		if (blk_pdp)
		{
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, blk_pdp, ebufp);
		}
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &p_status, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, "Bulk Object Created Successfully", ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_pymt_bulk_collect return flist", *o_flistpp);
	}

	PIN_FLIST_DESTROY_EX(&blk_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pymt_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	return;
}

static int
fm_mso_check_ref_id_unique(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	pin_flist_t		**o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*s_oflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;

	poid_t			*s_pdp = NULL;

	int32			flags = 256;
	int32			ret_val = 1;
	int64			db = 1;
	int64			id = -1;

	char			*ref_id = NULL;
	char			*s_template = " select X from /mso_lco_blk_payment where F1 = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_utils_get_service_details", ebufp);
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_ref_id_unique", i_flist );
	ref_id = PIN_FLIST_FLD_GET(i_flist, PIN_FLD_REFERENCE_ID, 1, ebufp);

	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_REFERENCE_ID, ref_id, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_ref_id_unique: Search Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_ref_id_unique: Search Output flist:",s_oflistp);

	if (s_oflistp)
	{
		ret_val = PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp);
	}

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
	return ret_val;
}
