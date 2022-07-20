/*
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. 
All rights reserved. 
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_subscription_pol_spec_cancel.c /cgbubrm_7.3.2.rwsmod/1 2009/03/24 07:02:26 amamidi Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_SUBSCRIPTION_POL_SPEC_CANCEL operation. 
 *
 * Also included are subroutines specific to the operation.
 *
 * These routines are generic and work for all account types.
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/bill.h"
#include "ops/subscription.h"
#include "pin_act.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_currency.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"

#define FILE_SOURCE_ID  "fm_subscription_pol_spec_cancel.c(1.3)"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_subscription_pol_spec_cancel(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

void fm_subs_pol_get_cancel_specs();

/*******************************************************************
 * Main routine for the PCM_OP_SUBSCRIPTION_POL_SPEC_CANCEL operation.
 *******************************************************************/
void
op_subscription_pol_spec_cancel(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	pin_cookie_t		cookie = NULL;

	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*flistp = NULL;

	poid_t			*a_pdp = NULL;

	char			*action;
	char			*cp;

	u_int			rec_id;
	u_int			done = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_SUBSCRIPTION_POL_SPEC_CANCEL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_spec_cancel opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_spec_cancel input flist", in_flistp);

	/***********************************************************
	 * Get the policy return flist.
	 ***********************************************************/
	fm_subs_pol_get_cancel_specs(ctxp, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Errors?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_spec_cancel error", ebufp);
		PIN_FLIST_DESTROY(r_flistp, NULL);
		*ret_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*ret_flistpp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_subscription_pol_spec_cancel return flist", 
				*ret_flistpp);
	}

	return;
}


/*******************************************************************
 * fm_subs_pol_get_cancel_specs():
 *
 *	Valid ACTION (Strings) are ( Only one of the following )
 *		1)  PIN_BILL_CANCEL_PRODUCT_ACTION_CANCEL_ONLY
 *		    only if the product needs to be cancelled but
 *		      not deleted from the d/b.
 *		    (if you expect some event batch to come in
 *			after the product is cancelled. )
 *		2)  PIN_BILL_CANCEL_PRODUCT_ACTION_CANCEL_DELETE 
 *		    if the product is to be cancelled and deleted.
 *		    This is what is passed as default to this policy.
 * 		3)  PIN_BILL_CANCEL_PRODUCT_ACTION_DONOT_CANCEL, if 
 *		    the product is not to be cancelled and not to be
 *		    deleted.
 *******************************************************************/
void
fm_subs_pol_get_cancel_specs(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**out_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *p_arrayp = NULL;
        pin_cookie_t    cookie = NULL;
        int             elem_id = 0;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * look for existing provisioning tag in product object
	 */
	s_flistp = PIN_FLIST_CREATE(ebufp);

	p_arrayp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PRODUCTS,
		&elem_id, 0, &cookie, ebufp);

	/*
	 * set product object (from input flist) on search flist
	 */
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID,
		PIN_FLIST_FLD_GET(p_arrayp, PIN_FLD_PRODUCT_OBJ, 0, ebufp),
		ebufp);

	/*
	 * set provisioning tag on search flist to retrieve it
	 */
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PROVISIONING_TAG,
		NULL, ebufp);

	/*
	 * call opcode to read flds from db
	 */
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, s_flistp, &r_flistp, ebufp);

	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PROVISIONING_TAG,
                0, ebufp);

        if (vp && strcmp((char *)vp, "") != 0) {

		/*
		 * Set action not to delete product for all products with a
		 * provisioning tag (status of product needs to be updated by
		 * the provisioning system and so must remain in the table).
		 */

		PIN_FLIST_FLD_SET(p_arrayp, PIN_FLD_ACTION,
			PIN_BILL_CANCEL_PRODUCT_ACTION_CANCEL_ONLY, ebufp);

	} else {

		/*
		 * For all products without a provisioning tag cancel and
		 * delete product from table.
		 */

		if (pin_conf_keep_cancelled_products_or_discounts == 0){
			PIN_FLIST_FLD_SET(p_arrayp, PIN_FLD_ACTION,
			PIN_BILL_CANCEL_PRODUCT_ACTION_CANCEL_DELETE, ebufp);
		} else {
			PIN_FLIST_FLD_SET(p_arrayp, PIN_FLD_ACTION,
                        PIN_BILL_CANCEL_PRODUCT_ACTION_CANCEL_ONLY, ebufp);
		}	

	} /* end else */

	
	/*
	 * Allocate the output flist
	 */
	*out_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	
	/*
	 * Cleanup
	 */
	PIN_FLIST_DESTROY(s_flistp, NULL);
	PIN_FLIST_DESTROY(r_flistp, NULL);

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_subs_pol_get_cancel_specs error", ebufp);

	}

	return;
}
