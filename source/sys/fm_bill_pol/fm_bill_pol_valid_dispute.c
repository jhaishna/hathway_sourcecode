/*
 *
 *      Copyright (c) 1999 - 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_bill_pol_valid_dispute.c:BillingVelocityInt:2:2006-Sep-05 21:55:39 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_BILL_POL_VALID_DISPUTE operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#define FILE_SOURCE_ID  "fm_bill_pol_valid_dispute.c(1.7)"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_bill_pol_valid_dispute(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

void
fm_bill_pol_validate_item_fields(
        cm_nap_connection_t	*connp,
        pin_flist_t		*i_flistp,
        u_int			*resultp,
        pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_BILL_POL_VALID_DISPUTE operation.
 *******************************************************************/
void
op_bill_pol_valid_dispute(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*ret_flistp = NULL;
	char			*descrp = NULL;
	void			*vp = NULL;

	u_int			result = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_BILL_POL_VALID_DISPUTE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_valid_dispute opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_valid_dispute input flist", i_flistp);

	/***********************************************************
	 * Validate the item fields.
	 ***********************************************************/
	fm_bill_pol_validate_item_fields(connp, i_flistp, &result, ebufp);

	/***********************************************************
	 * prepare the return flist. 
	 ***********************************************************/
	ret_flistp = PIN_FLIST_CREATE(ebufp);
        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, vp, ebufp);

	/***********************************************************
	 * Set the validation result field (pass or fail).
	 ***********************************************************/
	result |= PIN_BOOLEAN_TRUE;
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);

	/***********************************************************
	 * Set the Description about the validation.
	 ***********************************************************/
	descrp = (char *)"Succeeded";
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, (void *)descrp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_valid_dispute error", ebufp);
		PIN_FLIST_DESTROY(ret_flistp, NULL);
		*o_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*o_flistpp = ret_flistp;

		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_bill_pol_valid_dispute return flist", *o_flistpp);

	}

	return;
}

/*******************************************************************
 * fm_bill_pol_validate_item_fields()
 *******************************************************************/
void
fm_bill_pol_validate_item_fields(connp, i_flistp, resultp, ebufp)
        cm_nap_connection_t	*connp;
	pin_flist_t	*i_flistp;
	u_int		*resultp;
	pin_errbuf_t	*ebufp;
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*robj_flistp = NULL;
	u_int		*statusp = NULL;
	pin_decimal_t	*dp = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * prepare the flist to read the item object fields.
	 ***********************************************************/
	robj_flistp = PIN_FLIST_CREATE(ebufp);

        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(robj_flistp, PIN_FLD_POID, vp, ebufp);
	PIN_FLIST_FLD_SET(robj_flistp, PIN_FLD_ITEM_TOTAL, NULL, ebufp);
	PIN_FLIST_FLD_SET(robj_flistp, PIN_FLD_DUE, NULL, ebufp);
	PIN_FLIST_FLD_SET(robj_flistp, PIN_FLD_DISPUTED, NULL, ebufp);
	PIN_FLIST_FLD_SET(robj_flistp, PIN_FLD_STATUS, NULL, ebufp);

	/***********************************************************
	 * Read the item object fields.
	 ***********************************************************/
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_flistp, &r_flistp, ebufp);
	PIN_FLIST_DESTROY(robj_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_validate_item_fields rflds error", ebufp);
		PIN_FLIST_DESTROY(r_flistp, NULL);
		return;
	} 

	/***********************************************************
	 * Check the status of the item.
	 ***********************************************************/
	statusp = (u_int *)PIN_FLIST_FLD_GET(r_flistp, 
			PIN_FLD_STATUS, 0, ebufp);

	if (statusp && (*statusp == PIN_ITEM_STATUS_PENDING)) {
		*resultp |= PIN_VALID_ITEM_STATUS_PENDING;
	} 

	if (statusp && (*statusp == PIN_ITEM_STATUS_CLOSED)) {
		*resultp |= PIN_VALID_ITEM_STATUS_CLOSED;
	} 

	/***********************************************************
	 * Check to see whether the item had any total at all.
	 ***********************************************************/
	dp = (pin_decimal_t *)PIN_FLIST_FLD_GET(r_flistp, 
			PIN_FLD_ITEM_TOTAL, 0, ebufp);

	if (dp && (pbo_decimal_sign(dp, ebufp) == 0)) {
		*resultp |= PIN_VALID_ITEM_NO_TOTAL;
	} 

	/***********************************************************
	 * If this function is called during settlement check to
	 * see whether this item is in dispute.
	 ***********************************************************/
	if (fm_utils_op_is_ancestor(connp->coip, 
			PCM_OP_AR_ITEM_SETTLEMENT)) {

		dp = (pin_decimal_t *)PIN_FLIST_FLD_GET(r_flistp, 
				PIN_FLD_DISPUTED, 0, ebufp);

		if (dp && (pbo_decimal_sign(dp, ebufp) == 0)) {
			*resultp |= PIN_VALID_ITEM_NOT_IN_DISPUTE;
		} 
	}

	/***********************************************************
	 * Errors?.
	 ***********************************************************/
	PIN_FLIST_DESTROY(r_flistp, NULL);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_validate_item_fields error", ebufp);
	}

	return;
}
