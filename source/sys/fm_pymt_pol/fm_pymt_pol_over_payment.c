/*
 *
 * Copyright (c) 2004, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_pymt_pol_over_payment.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:45:39 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_PYMT_POL_OVER_PAYMENT operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_pymt_pol_over_payment(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_OVER_PAYMENT operation.
 * Looks like we got some extra money. By default, let the money
 * sit in the "payment" item itself until somebody move this money
 * some place else. 
 *******************************************************************/
void
op_pymt_pol_over_payment(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	int			*actg_typep = NULL;
	void			*vp = NULL;

	const char		*item_type = NULL;
	pin_decimal_t 		*duep = NULL;
	int			result = 0;
	int			status = PIN_SELECT_STATUS_OVER_PAYMENT;
	int32			rec_id = 0;

	// MSO Customization Start
	pin_flist_t			*item_in_flistp = NULL;
	pin_flist_t			*item_out_flistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*misc_flistp = NULL;
	pin_flist_t			*non_misc_flistp = NULL;
	poid_t				*item_pdp = NULL;
	poid_t				*svc_pdp = NULL;
	char				*item_type1 = NULL;
	char				*svc_type = NULL;
	int32				stack_opcode = 0;
	int32				rec_id1 = 0;
	int32				rec_id2 = 0;
	int32				rec_id3 = 0;
	int32				elem_count = 0;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t		cookie2 = NULL;
	pin_cookie_t		cookie3 = NULL;
	int32				count = 0;
	int32				misc_elem = 0;
	int32				non_misc_elem = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_OVER_PAYMENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_over_payment opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_over_payment input flist", i_flistp);

	/***********************************************************
	 * Just a straight copy of the input flist. Let the vendor
	 * figure out, what they want to with the extra money.
	 * Filter out any refund items with some credit. 
	 * (This can have some credit and would be picked up by the 
	 * refund application). If there is some debit sitting on
	 * these refund items, we need to consider them for any
	 * allocation of future payments.
	 ***********************************************************/
	r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	// MSO Customization Start

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside Customization");
	misc_flistp = PIN_FLIST_CREATE(ebufp);
	non_misc_flistp = PIN_FLIST_CREATE(ebufp);

	temp_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	elem_count = PIN_FLIST_ELEM_COUNT(temp_flistp, PIN_FLD_ITEMS, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1");
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(temp_flistp, PIN_FLD_ITEMS,
		&rec_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL) {
		item_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_POID, 1, ebufp);
		item_type1 = (char*) PIN_POID_GET_TYPE(item_pdp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "2");
		if ( item_type1 && (strcmp((char*)item_type1, "/item/misc") == 0) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "3");
			svc_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
			svc_type = (char *) PIN_POID_GET_TYPE(svc_pdp);
			if (svc_type && (strcmp((char*)svc_type, "") == 0) )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "4");
				PIN_FLIST_ELEM_SET(misc_flistp, flistp, PIN_FLD_ITEMS, misc_elem, ebufp);
				misc_elem = misc_elem + 1;
			}
			else
			{
				PIN_FLIST_ELEM_SET(non_misc_flistp, flistp, PIN_FLD_ITEMS, non_misc_elem, ebufp);
				non_misc_elem = non_misc_elem + 1;
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "5");
		}
		else
		{
			PIN_FLIST_ELEM_SET(non_misc_flistp, flistp, PIN_FLD_ITEMS, non_misc_elem, ebufp);
			non_misc_elem = non_misc_elem + 1;
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Misc Items updated input flist", misc_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Non-Misc Items updated input flist",non_misc_flistp);
	
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(misc_flistp, PIN_FLD_ITEMS,
		&rec_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL) {
		PIN_FLIST_ELEM_SET(temp_flistp, flistp, PIN_FLD_ITEMS, count, ebufp);
		count = count + 1;
	}

	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(non_misc_flistp, PIN_FLD_ITEMS,
		&rec_id3, 1, &cookie3, ebufp)) != (pin_flist_t *)NULL) {
		PIN_FLIST_ELEM_SET(temp_flistp, flistp, PIN_FLD_ITEMS, count, ebufp);
		count = count + 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Updated temp flist",temp_flistp);
	i_flistp = PIN_FLIST_COPY(temp_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Updated input flist",i_flistp);

	// MSO Customization End
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ITEMS,
			&rec_id, 1, &cookie, ebufp)) != NULL) {

		duep = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DUE, 0, ebufp);
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_POID, 0, ebufp);
		item_type = PIN_POID_GET_TYPE((poid_t *)vp);
		if ( (item_type) && (!strcmp(item_type, PIN_OBJ_TYPE_ITEM_REFUND))
			&& (pbo_decimal_sign(duep, ebufp) < 0) ) {

			PIN_FLIST_ELEM_DROP(r_flistp, PIN_FLD_ITEMS, rec_id,
					ebufp);
		}
	}

	/***********************************************************
	 * Check the accounting type. If the accounting type is
	 * balance forward then just set the result to pass.
	 * If the accounting type is open item and flags passed is 
	 * PIN_BILLFLG_SELECT_FINAL then set the result to pass. 
	 ***********************************************************/
	actg_typep = (int *)PIN_FLIST_FLD_GET(r_flistp, 
			PIN_FLD_ACTG_TYPE, 0, ebufp);

	if (actg_typep && ((*actg_typep == PIN_ACTG_TYPE_BALANCE_FORWARD) ||
		((*actg_typep == PIN_ACTG_TYPE_OPEN_ITEMS) &&
		(flags & PIN_BILLFLG_SELECT_FINAL)))){

		result = PIN_SELECT_RESULT_PASS;
	} else {
		result = PIN_SELECT_RESULT_FAIL;
		status |= PIN_SELECT_STATUS_MANUAL_ALLOC;
	}

	/***********************************************************
	 * Set the result & result status of select items.
	 ***********************************************************/
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_SELECT_RESULT, 
			(void *)&result, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_SELECT_STATUS, 
			(void *)&status, ebufp);

	/***********************************************************
	 * Cleanup
	 ***********************************************************/

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_over_payment error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*o_flistpp = r_flistp;

		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_pymt_pol_over_payment return flist", *o_flistpp);

	}
	PIN_FLIST_DESTROY_EX(&temp_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&misc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&non_misc_flistp, NULL);
	return;
}
