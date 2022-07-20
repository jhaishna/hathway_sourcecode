/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 * 
 * Version 1.0: 04-11-2013: Vilva Sabarikanth: Added the MSO Customization for
 * searching the receipts using MSO_OP_DEVICE_VIEW_CHALLAN with account poid as input
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_device_generate_challan.c  /cgbubrm_7.5.0. 2013/11/04 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_DEVICE_VIEW_CHALLAN operation. 
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

#define SEQ_TYPE_REPAIR_CHALLAN "MSO_SEQUENCE_TYPE_REPAIR_CHALLAN"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_mso_device_generate_challan(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

static void mso_create_challan(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_DEVICE_VIEW_CHALLAN operation.
 *******************************************************************/
void
op_mso_device_generate_challan(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t			*r_flistp = NULL;
	pin_flist_t			*err_flistp = NULL;
	int32				status = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_DEVICE_GENERATE_CHALLAN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_device_generate_challan opcode error", ebufp);
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_device_generate_challan input flist", i_flistp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	//Retrieving the account poid from the i_flistp
	
	mso_create_challan(ctxp, i_flistp, &r_flistp, ebufp);

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Challan creation failed");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53010", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error while creating challan for vendor", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,err_flistp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,err_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_USERID,err_flistp,PIN_FLD_USERID,ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		goto CLEANUP;
	}
	else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERRBUF_CLEAR(ebufp);
		*o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,*o_flistpp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,*o_flistpp,PIN_FLD_PROGRAM_NAME,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_USERID,*o_flistpp,PIN_FLD_USERID,ebufp);
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_device_generate_challan return flist", *o_flistpp);
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_device_generate_challan return flist", *o_flistpp);
	return;
}

static void mso_create_challan(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)-1;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*seq_flistp = NULL;
	pin_flist_t		*cr_flistp = NULL;
	poid_t			*acct_pdp = NULL;
	int64			db=0;
	int32           	rec_id = 0;
        pin_cookie_t    	cookie = NULL;
	pin_flist_t		*wi_flistp = NULL;
	pin_flist_t		*wo_flistp = NULL;
	pin_flist_t		*d_flistp = NULL;

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	
	in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_NAME, SEQ_TYPE_REPAIR_CHALLAN, ebufp);
       	fm_mso_utils_sequence_no(ctxp, in_flistp, &seq_flistp, ebufp);

	cr_flistp = PIN_FLIST_COPY(i_flistp,ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,cr_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
	PIN_FLIST_FLD_COPY(seq_flistp,PIN_FLD_STRING,cr_flistp,PIN_FLD_RECEIPT_NO,ebufp);
	db = PIN_POID_GET_DB(acct_pdp);
	PIN_FLIST_FLD_SET(cr_flistp,PIN_FLD_POID,PIN_POID_CREATE(db,"/mso_repair_challan",id,ebufp),ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Challan creation input flist", cr_flistp);
        PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, cr_flistp, r_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Challan creation output flist",*r_flistpp);
	PIN_FLIST_FLD_COPY(seq_flistp,PIN_FLD_STRING,*r_flistpp,PIN_FLD_RECEIPT_NO,ebufp);

	while((d_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_DEVICES, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL){

		wi_flistp = PIN_FLIST_CREATE(ebufp);	
		PIN_FLIST_FLD_COPY(d_flistp,PIN_FLD_DEVICE_OBJ,wi_flistp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_COPY(seq_flistp,PIN_FLD_STRING,wi_flistp,PIN_FLD_RECEIPT_NO,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update device with challan input flist", wi_flistp);
        	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wi_flistp, &wo_flistp, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update device with challan output flist",wo_flistp);
		PIN_FLIST_DESTROY_EX(&wi_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&wo_flistp, NULL);

	}

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Challan search Error",*r_flistpp);
	return;
	}

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Challan creation return flist",*r_flistpp);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&seq_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&cr_flistp, NULL);
	return;
}
