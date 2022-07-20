/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 * 
 * Version 1.0: 05-12-2013: Vilva Sabarikanth: Added the MSO Customization for
 * searching the receipts using MSO_OP_AR_GET_ADJUSTMENTS with account poid as input
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_op_ar_get_adjustments.c  /cgbubrm_7.5.0. 2013/11/04 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_AR_GET_ADJUSTMENTS operation. 
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

#define CATV_PROFILE "CATV"
#define BB_PROFILE "BB"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_op_ar_get_adjustments(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

void mso_retrieve_adjustments(
	pcm_context_t     	*ctxp,
	pin_flist_t			*in_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_AR_GET_ADJUSTMENTS operation.
 *******************************************************************/
void
mso_op_ar_get_adjustments(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	poid_t			*acct_pdp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int32			status = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_AR_GET_ADJUSTMENTS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_ar_get_adjustments opcode error", ebufp);
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_op_ar_get_adjustments input flist", i_flistp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	//Retrieving the account poid from the i_flistp
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	
	mso_retrieve_adjustments(ctxp, i_flistp, &r_flistp, ebufp);

	flistp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_RESULTS, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Adjustment Items Not Found");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53100", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Adjustment Items Not Found", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
	}
	else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERRBUF_CLEAR(ebufp);
		*o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &status, ebufp);
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_ar_get_adjustments return flist", *o_flistpp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	}
	return;

CLEANUP:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
        return;
}

void mso_retrieve_adjustments(
        pcm_context_t     	*ctxp,
        pin_flist_t	        *i_flistp,
        pin_flist_t         **r_flistpp,
        pin_errbuf_t        *ebufp)
{
	u_int64         id = (u_int64)-1;
    pin_flist_t     *s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
    int32           s_flags = 512;
	int32			*flag = NULL;
	int32			*srvc_type = NULL;
	int32			status = 0;
	int64           db = 0;
	poid_t          *pdp = NULL;
	poid_t          *acct_pdp = NULL;
	poid_t          *item_pdp = NULL;
	char			*billinfo = NULL;

		acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
		srvc_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp);
		if (*srvc_type == 1)
		{
			billinfo = CATV_PROFILE;
		}
		else if (*srvc_type == 0)
		{
			billinfo = BB_PROFILE;
		}

		db = PIN_POID_GET_DB(acct_pdp);

        s_flistp = PIN_FLIST_CREATE(ebufp);
        pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
		item_pdp = PIN_POID_CREATE(db, "/item/adjustment", 1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, item_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_ID, billinfo, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATUS, flistp, PIN_FLD_STATUS,ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Searching Adjustments");
		if (*flag == 1)
		{
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /item 1, /billinfo 2 where 1.F1 = V1 and 1.F2.type = V2 and 1.F3 = V3 and 1.F4 = 2.F5 and 2.F6 = V6 and 1.F7 = V7 and ( 1.F8 > V8 and 1.F9 < V9 )", ebufp);
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, flistp, PIN_FLD_CREATED_T,ebufp);
			flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, flistp, PIN_FLD_CREATED_T,ebufp);
		}
		else if (*flag == 0)
		{
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /item 1, /billinfo 2 where 1.F1 = V1 and 1.F2.type = V2 and 1.F3 = V3 and 1.F4 = 2.F5 and 2.F6 = V6  and 1.F7 = V7", ebufp);
		}
		else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Invalid Flag Value in the Input");
		}
		res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting Results");
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_NAME, NULL, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_DUE, NULL, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ITEM_TOTAL, NULL, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ITEM_NO, NULL, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_RECVD, NULL, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, NULL, ebufp);
    		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Adjustment search input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Adjustment search output flist",*r_flistpp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Adjustment search Error",*r_flistpp);
            return;
        }
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		return;
}
