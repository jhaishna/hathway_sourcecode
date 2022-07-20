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
 * preparing the Input FLIST for MSO_OP_PYMT_BULK_COLLECT opcode and payment receipt creation
 * for bulk payments from LCO through OAP
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_lco_bulk.c  /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
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
#include "mso/mso_flds.h"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_pymt_bulk_collect(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

void mso_retrieve_account(
        pcm_context_t     	*ctxp,
        char	            *acct_no,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);


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
	pin_flist_t		*r_flistp = NULL;
	int			*cmdp = NULL;
	pin_flist_t		*a_flistp = NULL;
	pin_cookie_t    cookie = NULL;
	int32           rec_id = 0;
	pin_flist_t		*blk_i_flistp = NULL;
	pin_flist_t		*blk_o_flistp = NULL;
	pin_flist_t		*blk_p_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t		*res_a_pdp = NULL;
	poid_t		*blk_pdp = NULL;
	poid_t		*lco_pdp = NULL;
	pin_decimal_t	*amt_total = NULL;
	pin_decimal_t	*amt_prsd = NULL;
	pin_decimal_t	*amt = NULL;
	void			*status = NULL;
	int32			p_status = 0;
	char			*acct_no;
	int32			elem = 0;
	time_t          time_now = NULL;

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
	
	r_flistp = PIN_FLIST_CREATE(ebufp);
	blk_o_flistp = PIN_FLIST_CREATE(ebufp);
	blk_i_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_bulk_collect input flist", i_flistp);

	//Retrieving the required params from the i_flistp
	blk_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	lco_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	amt_total = (pin_decimal_t *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AMOUNT_TOTAL, 0, ebufp);
	status = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 0, ebufp);
	amt_prsd = pbo_decimal_from_str("0.0",ebufp);

	//Setting the parameters to the bulk payment object creation input flist 
	PIN_FLIST_FLD_SET(blk_i_flistp, PIN_FLD_POID, (void *)blk_pdp, ebufp);
	PIN_FLIST_FLD_SET(blk_i_flistp, MSO_FLD_LCO_OBJ, (void *)lco_pdp, ebufp);
	PIN_FLIST_FLD_SET(blk_i_flistp, MSO_FLD_AMOUNT_TOTAL, (pin_decimal_t *)amt_total, ebufp);
	PIN_FLIST_FLD_SET(blk_i_flistp, MSO_FLD_AMOUNT_PROCESSED, (pin_decimal_t *)amt_prsd, ebufp);
	PIN_FLIST_FLD_SET(blk_i_flistp, PIN_FLD_STATUS, (void *)status, ebufp);
	
	while((a_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, MSO_FLD_PAYMENT_DATA, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
	{
		acct_no = (void *)PIN_FLIST_FLD_GET(a_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(a_flistp, PIN_FLD_AMOUNT, 1, ebufp);
		mso_retrieve_account(ctxp, acct_no, &r_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Account Retrieval Error flist", r_flistp);
            return;
        }
		res_flistp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_RESULTS, 0, ebufp);
		res_a_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);

		blk_p_flistp = PIN_FLIST_ELEM_ADD(blk_i_flistp, MSO_FLD_PAYMENT_DATA, (int32 )rec_id, ebufp);
		PIN_FLIST_FLD_SET(blk_p_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)res_a_pdp, ebufp);

		time_now = pin_virtual_time(NULL);
		PIN_FLIST_FLD_SET(blk_p_flistp, MSO_FLD_PYMT_COLLECTION_T, &time_now, ebufp);
		PIN_FLIST_FLD_SET(blk_p_flistp, MSO_FLD_PYMT_STATUS, (void *)&p_status, ebufp);
		PIN_FLIST_FLD_SET(blk_p_flistp, PIN_FLD_AMOUNT, (pin_decimal_t *)amt, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Bulk Payment Object Creation input flist", blk_i_flistp);
	
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, blk_i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_pymt_bulk_collect error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;

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
			"mso_pymt_bulk_collect return flist", *o_flistpp);
	}
	return;
}

void mso_retrieve_account(
        pcm_context_t     	*ctxp,
        char	             *acct_no,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{
	u_int64         id = (u_int64)-1;
        pin_flist_t     *s_flistp = NULL;
	pin_flist_t	*flistp = NULL;
        int32           s_flags = 256;
	poid_t          *pdp = NULL;
	int64           db= 1;

        s_flistp = PIN_FLIST_CREATE(ebufp);
        pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
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
return;
}
