/*
 *
 * Copyright (c) 1996, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_bill_pol_pre_fold.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:33:10 nishahan Exp $";
#endif

#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "ops/netflow.h"
#include "ops/bill.h"
#include "cm_fm.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_rate.h"
#include "pinlog.h"
#include "fm_utils.h"

#define FILE_SOURCE_ID  "fm_bill_pol_pre_fold.c(1.7)"

EXPORT_OP void
op_bill_pol_pre_fold(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_bill_pol_pf_cancel_products();

static void
fm_bill_pol_pre_fold_netflow_rate(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_BILL_POL_PRE_FOLD  command
 *******************************************************************/
void
op_bill_pol_pre_fold(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)

{
	pcm_context_t		*ctxp = connp->dm_ctx;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = (pin_flist_t *)NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_BILL_POL_PRE_FOLD) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_bill_pol_pre_fold", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_pre_fold input flist", in_flistp);

	/***************************************************
 	 * Call PCM_OP_BILL_CANCEL_PRODUCT
 	 ***************************************************/
	fm_bill_pol_pf_cancel_products(ctxp, flags, in_flistp, ebufp);

	/***********************************************************
	 * Do netflow rating only if netflow is setup
	 ***********************************************************/
	if (pin_conf_has_netflow) {
			fm_bill_pol_pre_fold_netflow_rate(ctxp, flags,
							  in_flistp, ebufp);
	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_pre_fold error", ebufp);
	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_bill_pol_pre_fold return flist", 
			*ret_flistpp);
	}

	return;
}

static void
fm_bill_pol_pf_cancel_products(ctxp, flags, in_flistp, ebufp)
	pcm_context_t	*ctxp;
	int32		flags;
	pin_flist_t	*in_flistp; /* product list */
	pin_errbuf_t	*ebufp;
{
	pin_flist_t	*p_flistp = NULL; /* a single account product */
	pin_flist_t	*i_can_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*t_flistp = NULL;
	pin_cookie_t	cookie = NULL;
	void		*vp = NULL;
	time_t		*p_end_tp = NULL;
	time_t		*cur_tp = NULL;
	int32		*statusp = NULL;
	int32		opcode = PCM_OP_BILL_CANCEL_PRODUCT;
	int32		rec_id;
	int32		opflag = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Walk thru the product list and cancel each product, if necessary
	 ***********************************************************/
	while (p_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PRODUCTS,
		&rec_id, 0, &cookie, ebufp)) {

		/*********************************************************
		 * Check the product status
		 *********************************************************/
		statusp = (int32 *)PIN_FLIST_FLD_GET(p_flistp,
			PIN_FLD_STATUS, 0, ebufp);
		if (statusp && (*statusp == PIN_PRODUCT_STATUS_CANCELLED)) {
			continue;
		}

		/*********************************************************
		 * Make sure the purchase_end_time has expired for this product
		 *********************************************************/
		p_end_tp = (time_t *)PIN_FLIST_FLD_GET(p_flistp,
			PIN_FLD_PURCHASE_END_T, 1, ebufp);
		cur_tp = (time_t *)PIN_FLIST_FLD_GET(in_flistp,
			PIN_FLD_END_T, 1, ebufp);
		if (!p_end_tp || !*p_end_tp || !cur_tp || !*cur_tp ||
			 (*p_end_tp >= *cur_tp)) {
			continue;
		}

		/**********************************************************
	 	 * Initialize the input flist for the cancel_product opcode
	 	 **********************************************************/

		i_can_flistp = PIN_FLIST_CREATE(ebufp);
		vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
		PIN_FLIST_FLD_SET(i_can_flistp, PIN_FLD_POID, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME,0,ebufp);
		PIN_FLIST_FLD_SET(i_can_flistp, PIN_FLD_PROGRAM_NAME, vp, 
			ebufp);

		PIN_FLIST_FLD_SET(i_can_flistp, PIN_FLD_END_T, (void *)cur_tp, 
				ebufp);

		t_flistp = PIN_FLIST_ELEM_ADD(i_can_flistp, PIN_FLD_PRODUCTS, 
			rec_id, ebufp);
		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_PRODUCT_OBJ, 0,ebufp);
		PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_PRODUCT_OBJ, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_NODE_LOCATION, 0,
			ebufp);
		PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_NODE_LOCATION, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_QUANTITY, 0,ebufp);
		PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_QUANTITY, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if (vp) {
			PIN_FLIST_FLD_SET(i_can_flistp, PIN_FLD_SERVICE_OBJ, vp,
				ebufp);
		}

		/**********************************************************
	 	 * Call the PCM_OP_BILL_CANCEL_PRODUCT opcode
	 	 **********************************************************/
		PCM_OP(ctxp, opcode, opflag, i_can_flistp, &r_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&i_can_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_CLEAR_ERR(ebufp);
	} /* while() */

	PIN_ERR_CLEAR_ERR(ebufp);
	return;
}

static void
fm_bill_pol_pre_fold_netflow_rate(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*rate_flistp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*serv_pdp = NULL;
	poid_t			*search_pdp = NULL;
	int64			db_no = 0;
	int32			elem_id = 0;
	pin_cookie_t		cookie = NULL;

	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	db_no = PIN_POID_GET_DB(acct_pdp);
	serv_pdp = PIN_POID_CREATE(db_no, "/service/broadband", -1, ebufp);
	
	/*
	 * Search for all the broadband service the account has
	 */
	search_flistp = PIN_FLIST_CREATE(ebufp);
	search_pdp = PIN_POID_CREATE(db_no, "/search", 237, ebufp);
	PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);

	/*
	 * Set argument array
	 */
	arg_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_POID, serv_pdp, ebufp);

	/*
	 * Set result array
	 */
	res_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0,
					ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);

	/*
	 * Call PCM_OP_SEARCH
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search input flist:",
			  search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, flags, search_flistp, &ret_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search output flist:",
			  ret_flistp);

	while ((res_flistp = PIN_FLIST_ELEM_TAKE_NEXT(ret_flistp,
		PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL) {
		pin_flist_t	*r_flistp = NULL;
			
		serv_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID,
					     0, ebufp);
		rate_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(rate_flistp, PIN_FLD_POID,
				       serv_pdp, ebufp);
		PCM_OP(ctxp, PCM_OP_NETFLOW_RATE_USAGE, flags, rate_flistp,
		       &r_flistp, ebufp);

		if (rate_flistp) {
			PIN_FLIST_DESTROY(rate_flistp, NULL);
			rate_flistp = NULL;
		}
		if (r_flistp) {
			PIN_FLIST_DESTROY(r_flistp, NULL);
		}
	}

	if (search_flistp) {
		PIN_FLIST_DESTROY(search_flistp, NULL);
	}
}
