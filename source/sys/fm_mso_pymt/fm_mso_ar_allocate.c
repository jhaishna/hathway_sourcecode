/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 * Version 1.0: 27-09-2014: Harish Kulkarni: Added the customization for broadband
 * customers for allocating the AR charg headwise per bill.
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_ar_allocate.c /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_AR_ALLOCATE operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
#include <time.h>
#include <math.h>
 
#include "pcm.h"
#include "ops/pymt.h"
#include "ops/ar.h"
#include "pin_rate.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_ar.h"
#include "ops/bill.h"
#include "mso_ops_flds.h" 


#define CURRENCY 356

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_ar_allocate(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

void mso_retrieve_ar_item(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void fm_mso_sort_items(
	pin_flist_t		*sort_in_flistp,
	pin_flist_t		**sort_out_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT 
void fm_mso_utils_get_pending_bill_items(
	pcm_context_t		*ctxp,
	poid_t			*billinfo_pdp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

EXPORT_OP void mso_retrieve_items(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_AR_ALLOCATE operation.
 *******************************************************************/
void
mso_ar_allocate(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*get_bal_in_flistp = NULL;
	pin_flist_t		*get_bal_out_flistp = NULL;
	pin_flist_t		*ar_sel_out_flistp = NULL;
	pin_flist_t		*arr_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*b_in_flistp = NULL;
	pin_flist_t		*b_out_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	poid_t			*err_pdp = NULL;
	poid_t			*billinfo_pdp = NULL;
	int				children = 0;
	int				currency = CURRENCY;
	int32			status = 1;
	int32			rec_id = 0;
	int32			rec_id1 = 0;
	u_int64			id = (u_int64)-1;
	int64			db = 0;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie1 = NULL;
	pin_decimal_t	*bill_due = NULL;
	pin_decimal_t	*zero = NULL;
	pin_decimal_t	*unapplied_amount = NULL;
	pin_decimal_t	*item_amt = NULL;
	pin_decimal_t	*temp = NULL;
//	char			msg[200];
//	char			*amt_temp = NULL;
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_AR_ALLOCATE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_pymt_allocate_payment opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_op_pymt_allocate_payment input flist", i_flistp);

	/*************************************************************
	 * Identifying Deposit Objects 
	 *************************************************************/
	get_bal_in_flistp = PIN_FLIST_CREATE(ebufp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	err_flistp = PIN_FLIST_CREATE(ebufp);
	err_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	billinfo_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ,get_bal_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(get_bal_in_flistp, PIN_FLD_INCLUDE_CHILDREN, &children, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Get Balances Input Flist", get_bal_in_flistp);
	PCM_OP(ctxp, PCM_OP_AR_GET_ACCT_BAL_SUMMARY, 0, get_bal_in_flistp, &get_bal_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Get Balances Output Flist", get_bal_out_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Get Balances Error",get_bal_out_flistp);
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_FLD_PUT(err_flistp, PIN_FLD_POID, err_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "52199", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Get Balances Error", ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto Cleanup;
	}

	bill_due = pbo_decimal_from_str("0.0", ebufp);
	zero = pbo_decimal_from_str("0.0", ebufp);
	unapplied_amount = pbo_decimal_from_str("0.0", ebufp);
	item_amt = pbo_decimal_from_str("0.0", ebufp);
//	temp = pbo_decimal_from_str("0.0", ebufp);

	bill_due = PIN_FLIST_FLD_GET(get_bal_out_flistp, PIN_FLD_OPENBILL_DUE, 0, ebufp);
	unapplied_amount = PIN_FLIST_FLD_GET(get_bal_out_flistp, PIN_FLD_UNAPPLIED_AMOUNT, 0, ebufp);

	if (pbo_decimal_compare(zero, unapplied_amount, ebufp) == 1)
	{
		children = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Unapplied amount lesser than Zero");
		if (pbo_decimal_compare(bill_due, zero, ebufp) == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Bill Due amount greater than Zero");
			
			mso_retrieve_ar_item(ctxp, i_flistp, &r_flistp, ebufp);
			rs_flistp =  PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Select AR Items Error",rs_flistp);
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_FLD_PUT(err_flistp, PIN_FLD_POID, err_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "52199", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Select AR Items Error", ebufp);
				*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
				goto Cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "AR Items Results",r_flistp);
			
			while((rs_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS, &rec_id1, 1, &cookie1, ebufp))!=(pin_flist_t*)NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Item processed for allocating:", rs_flistp);
				unapplied_amount = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_DUE, 0, ebufp);
				b_in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, b_in_flistp, PIN_FLD_POID, ebufp);

				PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_POID, b_in_flistp, PIN_FLD_ITEM_OBJ, ebufp);
				PIN_FLIST_FLD_SET(b_in_flistp, PIN_FLD_PROGRAM_NAME,"Automatic AR Allocation After Billing", ebufp);

				fm_mso_utils_get_pending_bill_items(ctxp, billinfo_pdp, &ar_sel_out_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Select Open Items Error",ar_sel_out_flistp);
					PIN_ERRBUF_CLEAR(ebufp);
					PIN_FLIST_FLD_PUT(err_flistp, PIN_FLD_POID, err_pdp, ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "52198", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Select Open Items Error", ebufp);
					*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
					goto Cleanup;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Items flist for allocation:", ar_sel_out_flistp);

				cookie = NULL;
				rec_id = 0;
				while((item_flistp = PIN_FLIST_ELEM_GET_NEXT(ar_sel_out_flistp, PIN_FLD_ITEMS, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
				{
//					amt_temp = (char *)pbo_decimal_to_str(unapplied_amount, ebufp);
//					sprintf(msg, "Amount = %s ",amt_temp);
//					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Item array processed:", item_flistp);
					item_amt = PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_AMOUNT, 0, ebufp);
					if(pbo_decimal_sign(item_amt,ebufp) == 1) // We need not allocate to items with negative due
						continue;
					if (pbo_decimal_compare(item_amt, unapplied_amount, ebufp) == 1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Item Amount Lesser than Unapplied Amount");
						temp = (pin_decimal_t *)pbo_decimal_negate(item_amt, ebufp);
						unapplied_amount = (pin_decimal_t *)pbo_decimal_add(unapplied_amount, temp, ebufp);
						PIN_FLIST_ELEM_SET(b_in_flistp, item_flistp, PIN_FLD_ITEMS, rec_id, ebufp);
					}
					else if (pbo_decimal_compare(item_amt, unapplied_amount, ebufp) == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Item Amount Equal to Unapplied Amount");
						temp = (pin_decimal_t *)pbo_decimal_negate(item_amt, ebufp);
						unapplied_amount = (pin_decimal_t *)pbo_decimal_add(unapplied_amount, temp, ebufp);
						PIN_FLIST_ELEM_SET(b_in_flistp, item_flistp, PIN_FLD_ITEMS, rec_id, ebufp);
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Item Amount greater than Unapplied Amount");
						PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_AMOUNT, unapplied_amount, ebufp);
						PIN_FLIST_ELEM_SET(b_in_flistp, item_flistp, PIN_FLD_ITEMS, rec_id, ebufp);
						break;
					}
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill Item transfer Input Flist", b_in_flistp);
				PCM_OP(ctxp, PCM_OP_BILL_ITEM_TRANSFER , 0, b_in_flistp, &b_out_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill Item transfer Output Flist", b_out_flistp);

				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Bill Item transfer Error",b_in_flistp);
					PIN_ERRBUF_CLEAR(ebufp);
					PIN_FLIST_FLD_PUT(err_flistp, PIN_FLD_POID, err_pdp, ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "52198", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Bill Item transfer Error", ebufp);
					*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
					goto Cleanup;
				}
				PIN_FLIST_DESTROY_EX(&ar_sel_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&b_out_flistp, NULL);
				b_out_flistp = NULL;
				ar_sel_out_flistp = NULL;
				
			}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Bill Due amount not greater than Zero");
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_FLD_PUT(err_flistp, PIN_FLD_POID, err_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "52197", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Bill Due amount not greater than Zero", ebufp);
			*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			goto Cleanup;
		}
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Unapplied amount not lesser than Zero");
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_FLD_PUT(err_flistp, PIN_FLD_POID, err_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "52196", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Unapplied amount not lesser than Zero", ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto Cleanup;
	}

	status = 0;
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ,err_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, "AR Allocation Successful !", ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
	*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_op_pymt_allocate_payment output flist", *o_flistpp);

Cleanup:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ar_sel_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&get_bal_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&b_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&b_out_flistp, NULL);

/*
	if (!pbo_decimal_is_null(bill_due, ebufp))
	{
		pbo_decimal_destroy(&bill_due);
	}
	
	if (!pbo_decimal_is_null(zero, ebufp))
	{
		pbo_decimal_destroy(&zero);
	}
	
	if (!pbo_decimal_is_null(unapplied_amount, ebufp))
	{
		pbo_decimal_destroy(&unapplied_amount);
	}
	*/
	return;
	
}

void mso_retrieve_ar_item(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)-1;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_decimal_t	*zero = NULL;
    int32			s_flags = 512;
	int64			db = 0;
	poid_t			*pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*binfo_pdp = NULL;
	int32			open_items = 2;
	char			*template = "select X from /item where F1 = V1 and F2 = V2 "
					"and F3 = V3 and F4 < V4 and item_t.poid_type in"
					" ('/item/payment','/item/adjustment') order by F5 desc";


	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	binfo_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(acct_pdp);
	zero = pbo_decimal_from_str("0.0", ebufp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Searching Credit(Negative Balance) Items");
	// Identifying the Payment Item or migrated item having some negative balance
//	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /item where F1 = V1 and F2 = V2 and (F3.type = V3 or F4.type = V4) and F5 = V5 order by F6 desc", ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, flistp, PIN_FLD_BILLINFO_OBJ,ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&open_items, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DUE, zero, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_CREATED_T, 0, ebufp);
	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, res_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_DUE, zero, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Credit Item search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Credit Item search output flist",*r_flistpp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Credit Item search Error",*r_flistpp);
		return;
	}
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

	return;
}

