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
 * searching the deposits using MSO_OP_PYMT_GET_DEPOSITS with account poid as input
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_get_deposits.c  /cgbubrm_7.5.0. 2013/11/04 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_GET_DEPOSITS operation. 
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


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_pymt_get_deposits(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

EXPORT_OP static void mso_retrieve_deposits(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_PYMT_GET_DEPOSITS operation.
 *******************************************************************/
void
mso_pymt_get_deposits(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	poid_t			*acct_pdp = NULL;
	int32			status = 0;
		
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_GET_DEPOSITS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_pymt_get_deposits opcode error", ebufp);
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_get_deposits input flist", i_flistp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	mso_retrieve_deposits(ctxp, i_flistp, &r_flistp, ebufp);

	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status, ebufp);
	flistp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_RESULTS, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{	
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Deposit Objects(/mso_deposit) Found");
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53060", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No Deposits Found", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
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
			"mso_pymt_get_deposits error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERRBUF_CLEAR(ebufp);
		*o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
//		*o_flistpp = r_flistp;

		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_pymt_get_deposits return flist", *o_flistpp);
	}
	Cleanup:
		PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);
	return;
}

EXPORT_OP static void mso_retrieve_deposits(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	poid_t			*pdp = NULL;
	poid_t			*acct_pdp = NULL;
	int64			db = 0;
	int32			*d_status = NULL;
	int32			s_flags = 512;
	u_int64			id = (u_int64)-1;

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	d_status = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 1, ebufp);

	db = PIN_POID_GET_DB(acct_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	//Pawan: Changed the value in below condition from 0 to 2; since 2 is considered ALL
	if (d_status && *d_status != 2)
	{
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, d_status, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_deposit where F1 = V1 and F2 = V2 and F3 = V3", ebufp);
	}
	else 
	{
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_deposit where F1 = V1 and F2 = V2", ebufp);
	}
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acct_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, MSO_FLD_DEPOSIT_AMOUNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, MSO_FLD_DEPOSIT_DATE, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, MSO_FLD_DEPOSIT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, MSO_FLD_REFUND_RULE, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ITEM_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_USERID, NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DEVICE_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Deposit search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Deposit search output flist",*r_flistpp);
      
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);

return;
}
