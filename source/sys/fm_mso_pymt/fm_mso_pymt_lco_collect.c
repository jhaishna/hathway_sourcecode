/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 * 
 * Version 1.0: 01-10-2013: Vilva Sabarikanth: Added the MSO Customization of
 * preparing the Payment Input FLIST for MSO_OP_PYMT_LCO_COLLECT opcode and payment 
 * posting and receipt creation for bulk payments from LCO through OAP
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_lco_collect.c  /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_LCO_COLLECT operation. 
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

#define CATV_PROFILE "CATV"
#define BB_PROFILE "BB"

#define PAY_TYPE_CASH   10011
#define PAY_TYPE_CHECK  10012
#define PAY_TYPE_ONLINE 10013
#define PAY_TYPE_TDS    10018
#define PAY_TYPE_ECS 10020
#define PAY_TYPE_PORTAL_TRANSFER 10021

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
void
mso_pymt_lco_collect(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
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

void mso_retrieve_account_poid(
        pcm_context_t     	*ctxp,
        char	            *acct_no,
		pin_flist_t			**o_flistpp,
        pin_errbuf_t        *ebufp);

int mso_confirm_service_type(
        pcm_context_t     	*ctxp,
        int32	            *srvc_type,
		poid_t				*acct_pdp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PYMT_LCO_COLLECT operation.
 *******************************************************************/
void
mso_pymt_lco_collect(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	int					*cmdp = NULL;
	int					local = LOCAL_TRANS_OPEN_SUCCESS;
	int					flag = 0;
	poid_t				*lco_pymt_pdp = NULL;
	poid_t				*acct_pdp = NULL;
	poid_t				*rcpt_pdp = NULL;
	pin_flist_t			*in_flistp = NULL;
	pin_flist_t			*r_flistp = NULL;
	pin_flist_t			*rs_flistp = NULL;
	pin_flist_t			*pymt_flistp = NULL;
	pin_flist_t			*ch_flistp = NULL;
	pin_flist_t			*p_flistp = NULL;
	pin_flist_t			*inh_flistp = NULL;
	pin_flist_t			*c_flistp = NULL;
	pin_flist_t			*ro_flistp = NULL;
	int32				*srvc_type = NULL;
	int32				*pay_type = NULL;
	int32				p_status=0;
	int32				sw_hw = 0;
	int32				status = 0;
	int32				p_channel=3;
	int32				command=0;
	int32				*flag1 = NULL;
	char				*acct_no;
	time_t				coll_time = (time_t)0;
	int32				new_serv_type_bb = 1;
	int32				new_serv_type_catv = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_LCO_COLLECT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_pymt_lco_collect opcode error", ebufp);
		return;
	}
	
	in_flistp = PIN_FLIST_CREATE(ebufp);
	pymt_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_lco_collect input flist", i_flistp);
	//Reading the Input Params
	lco_pymt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	acct_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	srvc_type = (int32 *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp);

	mso_retrieve_account_poid(ctxp, acct_no, &r_flistp, ebufp);

	rs_flistp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_RESULTS, 0, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_POID, 0, ebufp);
    if(!PIN_ERRBUF_IS_ERR(ebufp))
    {
		flag = mso_confirm_service_type(ctxp, srvc_type, acct_pdp, ebufp);
		if (flag == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service Matched");
			PIN_FLIST_FLD_SET(pymt_flistp, PIN_FLD_POID, acct_pdp , ebufp);
			//PIN_FLIST_FLD_SET(pymt_flistp, PIN_FLD_PROGRAM_NAME, "MSO_OP_PYMT_LCO_COLLECT", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, pymt_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
			PIN_FLIST_FLD_SET(pymt_flistp, MSO_FLD_PYMT_CHANNEL, &p_channel, ebufp);
			if(*srvc_type == 0)
			{
				PIN_FLIST_FLD_SET(pymt_flistp, MSO_FLD_SERVICE_TYPE, &new_serv_type_catv, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(pymt_flistp, MSO_FLD_SERVICE_TYPE, &new_serv_type_bb, ebufp);
			}
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, pymt_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, pymt_flistp, PIN_FLD_START_T, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, pymt_flistp, PIN_FLD_END_T, ebufp);
			PIN_FLIST_FLD_SET(pymt_flistp, PIN_FLD_TYPE, &sw_hw, ebufp);
			//PIN_FLIST_FLD_SET(pymt_flistp, PIN_FLD_DESCR, "LCO Bulk Payment", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, pymt_flistp, PIN_FLD_DESCR, ebufp);
			ch_flistp = PIN_FLIST_ELEM_ADD(pymt_flistp, PIN_FLD_CHARGES, 0, ebufp);
			PIN_FLIST_FLD_SET(ch_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(ch_flistp, PIN_FLD_COMMAND, &command, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PAY_TYPE, ch_flistp, PIN_FLD_PAY_TYPE, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, ch_flistp, PIN_FLD_AMOUNT, ebufp);
			p_flistp = PIN_FLIST_SUBSTR_ADD(ch_flistp, PIN_FLD_PAYMENT, ebufp);
			inh_flistp = PIN_FLIST_SUBSTR_ADD(p_flistp, PIN_FLD_INHERITED_INFO, ebufp);
			pay_type = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PAY_TYPE, 0, ebufp);
			if (*pay_type == PAY_TYPE_CASH || *pay_type == PAY_TYPE_TDS || *pay_type == PAY_TYPE_PORTAL_TRANSFER)
			{
				c_flistp = PIN_FLIST_SUBSTR_ADD(inh_flistp, PIN_FLD_CASH_INFO, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REFERENCE_ID, c_flistp, PIN_FLD_RECEIPT_NO, ebufp);
			}
			else if (*pay_type == PAY_TYPE_CHECK)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "3");
				c_flistp = PIN_FLIST_SUBSTR_ADD(inh_flistp, PIN_FLD_CHECK_INFO, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REFERENCE_ID, c_flistp, PIN_FLD_RECEIPT_NO, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHECK_NO, c_flistp, PIN_FLD_CHECK_NO, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_NAME, c_flistp, PIN_FLD_BANK_NAME, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BRANCH_NO, c_flistp, PIN_FLD_BRANCH_NO, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_CODE, c_flistp, PIN_FLD_BANK_CODE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_ACCOUNT_NO, c_flistp, PIN_FLD_BANK_ACCOUNT_NO, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_PYMT_COLLECTION_T, c_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
			}
			else if (*pay_type == PAY_TYPE_ONLINE || *pay_type == PAY_TYPE_ECS)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "4");
				c_flistp = PIN_FLIST_SUBSTR_ADD(inh_flistp, PIN_FLD_WIRE_TRANSFER_INFO , ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REFERENCE_ID,      c_flistp, PIN_FLD_WIRE_TRANSFER_ID, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHECK_NO,          c_flistp, PIN_FLD_CHECK_NO, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_NAME,         c_flistp, PIN_FLD_BANK_NAME, ebufp);
				//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BRANCH_NO,         c_flistp, PIN_FLD_BRANCH_NO, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_CODE,         c_flistp, PIN_FLD_BANK_CODE, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_ACCOUNT_NO,   c_flistp, PIN_FLD_BANK_ACCOUNT_NO, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_PYMT_COLLECTION_T, c_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BRANCH_NO,         c_flistp, PIN_FLD_BRANCH_NO, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_IP_ADDRESS,         c_flistp, PIN_FLD_IP_ADDRESS, ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "4");
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Payment Type Mismatch", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				status = 2;
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, lco_pymt_pdp, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_PYMT_STATUS, &status, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ERROR_CODE, "53034", ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ERROR_DESCR, "Payment Type Mismatch", ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Error Update input flist", in_flistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, in_flistp, &ro_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Error Update output flist", ro_flistp);
				PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
				p_status=1;
				*o_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
				goto Cleanup;
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_PYMT_COLLECT calling input flist", pymt_flistp);
			PCM_OP(ctxp, MSO_OP_PYMT_COLLECT, 0, pymt_flistp, &r_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_PYMT_COLLECT calling output flist", r_flistp);

			rcpt_pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 0, ebufp);
			flag1 = (int32 *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STATUS, 0, ebufp);
			if (*flag1 == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Payment Posted Successfully");
				PIN_ERRBUF_CLEAR(ebufp);
				status = 1;
				coll_time = pin_virtual_time((time_t *)NULL);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, lco_pymt_pdp, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_PYMT_STATUS, &status, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_PYMT_PROCESSING_T, &coll_time, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_RECEIPT_OBJ, rcpt_pdp, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ERROR_CODE, "", ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ERROR_DESCR, "", ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Success Update input flist", in_flistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, in_flistp, &ro_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Success Update output flist", ro_flistp);
				PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
			}
			else
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Payment Posting Failed", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				status = 2;
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, lco_pymt_pdp, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_PYMT_STATUS, &status, ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ERROR_CODE, "53035", ebufp);
				PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ERROR_DESCR, "Payment Posting Failed", ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Error Update input flist", in_flistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, in_flistp, &ro_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Error Update output flist", ro_flistp);
				PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
				p_status=1;
				*o_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
				goto Cleanup;
			}
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Service Mismatch", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			status = 2;
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, lco_pymt_pdp, ebufp);
			PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_PYMT_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ERROR_CODE, "53033", ebufp);
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ERROR_DESCR, "Service Mismatch", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Error Update input flist", in_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, in_flistp, &ro_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Error Update output flist", ro_flistp);
			PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
			p_status=1;
			*o_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
			goto Cleanup;
		}
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Account Number Mismatch", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 2;
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, lco_pymt_pdp, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_PYMT_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ERROR_CODE, "53032", ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ERROR_DESCR, "Account Number Mismatch", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Error Update input flist", in_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, in_flistp, &ro_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Error Update output flist", ro_flistp);
		PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
		p_status=1;
		*o_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
		goto Cleanup;
	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_pymt_lco_collect error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;
		p_status=1;
	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERRBUF_CLEAR(ebufp);
		*o_flistpp = r_flistp;
		
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_pymt_lco_collect return flist", *o_flistpp);
		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_DESCR, "Payment Posted Successfully", ebufp);
	}

Cleanup:
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &p_status, ebufp);
	PIN_FLIST_DESTROY_EX(&pymt_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	return;
}

void mso_retrieve_account_poid(
        pcm_context_t     	*ctxp,
        char	            *acct_no,
		pin_flist_t			**o_flistpp,
        pin_errbuf_t            *ebufp)
{
	u_int64         id = (u_int64)-1;
    pin_flist_t     *s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
    int32           s_flags = 256;
	poid_t          *pdp = NULL;
	int64           db = 1;

        s_flistp = PIN_FLIST_CREATE(ebufp);
        pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /account where F1 = V1 ", ebufp);
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_NO, (void *)acct_no, ebufp);
    	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Account search input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Account search output flist",*o_flistpp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Account search Error",*o_flistpp);
            return;
        }
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}


int mso_confirm_service_type(
        pcm_context_t     	*ctxp,
        int32	            *srvc_type,
		poid_t				*acct_pdp,
        pin_errbuf_t            *ebufp)
{
	u_int64         id = (u_int64)-1;
    pin_flist_t     *s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
    int32           s_flags = 256;
	poid_t          *pdp = NULL;
	poid_t          *billinfo_pdp = NULL;
	int64           db = 0;

        s_flistp = PIN_FLIST_CREATE(ebufp);
		db = PIN_POID_GET_DB(acct_pdp);
        pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /billinfo where F1 = V1 and F2 = V2", ebufp);
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acct_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		if (*srvc_type == 0)
		{
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_ID, CATV_PROFILE, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_ID, BB_PROFILE, ebufp);
		}

    	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Billinfo search input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Billinfo search output flist",r_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Billinfo search Error",r_flistp);
            return 0;
        }
		rs_flistp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_RESULTS, 0, ebufp);
		billinfo_pdp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_POID, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Service Billinfo Not Found ",r_flistp);
            return 0;
        }
		else
		{
			return 1;
		}
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
}

