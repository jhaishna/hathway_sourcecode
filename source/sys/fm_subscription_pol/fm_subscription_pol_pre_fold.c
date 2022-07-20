/*******************************************************************
 *
 * Copyright (c) 1996, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_subscription_pol_pre_fold.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 06:17:17 nishahan Exp $";
#endif

#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "ops/bill.h"
#include "ops/subscription.h"
#include "cm_fm.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_rate.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"

#define FILE_SOURCE_ID  "fm_subscription_pol_pre_fold.c(4)"

EXPORT_OP void
op_subscription_pol_pre_fold(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_subs_pol_pf_cancel_products();


/*******************************************************************
 * Main routine for the PCM_OP_SUBSCRIPTION_POL_PRE_FOLD  command
 *******************************************************************/
void
op_subscription_pol_pre_fold(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)

{
	pcm_context_t		*ctxp = connp->dm_ctx;
	char			*pgname = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = (pin_flist_t *)NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_SUBSCRIPTION_POL_PRE_FOLD) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_subscription_pol_pre_fold", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_pre_fold input flist", in_flistp);


            
	/***************************************************
 	 * Call PCM_OP_SUBSCRIPTION_CANCEL_PRODUCT
 	 ***************************************************/
	pgname = (char *)PIN_FLIST_FLD_GET(in_flistp, 
				PIN_FLD_PROGRAM_NAME,0,ebufp);
	if(pgname&&strcmp(pgname,"pin_cycle")){
		fm_subs_pol_pf_cancel_products(ctxp, flags, in_flistp, ebufp);
	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_pre_fold error", ebufp);
	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_subscription_pol_pre_fold return flist", 
			*ret_flistpp);
	}

	return;
}

static void
fm_subs_pol_pf_cancel_products(ctxp, flags, in_flistp, ebufp)
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
	int32		opcode = PCM_OP_SUBSCRIPTION_CANCEL_PRODUCT;
	int32		rec_id = 0;
	int32		opflag = 0;
	int32		counter = 0;
	char		err_msg[PCM_MAX_POID_TYPE + 128] = "";

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

		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_OFFERING_OBJ, 0,
			ebufp);
		PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_OFFERING_OBJ, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_QUANTITY, 0,ebufp);
		PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_QUANTITY, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if (vp) {
			PIN_FLIST_FLD_SET(i_can_flistp, PIN_FLD_SERVICE_OBJ, vp,
				ebufp);
		}

		/**********************************************************
	 	 * Call the PCM_OP_SUBSCRIPTION_CANCEL_PRODUCT opcode
	 	 **********************************************************/

		PCM_OP(ctxp, opcode, opflag, i_can_flistp,&r_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&i_can_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

		if (PIN_ERR_IS_ERR(ebufp))
			counter++;
		PIN_ERR_CLEAR_ERR(ebufp);
	} /* while() */

	if (counter > 0) {
		sprintf(err_msg, "Cancellation failed for %d products", counter);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, err_msg, ebufp);
	}

	PIN_ERR_CLEAR_ERR(ebufp);
	return;
}

