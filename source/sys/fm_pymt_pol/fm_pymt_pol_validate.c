/*
 *
* Copyright (c) 1999, 2014, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_pymt_pol_validate.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:46:04 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_PYMT_POL_VALIDATE operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_pymt.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_pymt_pol_validate(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_validate_result(
	pin_flist_t	*i_flistp,
	pin_flist_t	*r_flistp,
	pin_errbuf_t	*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_VALIDATE operation.
 *******************************************************************/
void
op_pymt_pol_validate(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;

	void			*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_VALIDATE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_validate opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_validate input flist", i_flistp);

	/***********************************************************
	 * Prep the return flist.
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, vp, ebufp);

	/***********************************************************
	 * Fill in the actual results.
	 ***********************************************************/
	fm_pymt_pol_validate_result(i_flistp, r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_validate error", ebufp);
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
			"op_pymt_pol_validate return flist", r_flistp);

	}

	return;
}

/*******************************************************************
 * fm_pymt_pol_validate_result():
 *
 *	Turn a pin_charge result into a validation result/descr.
 *
 *******************************************************************/
static void
fm_pymt_pol_validate_result(
	pin_flist_t	*i_flistp,
	pin_flist_t	*r_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_charge_result_t	*type = NULL;
	char			*descr = NULL;
	int32			result = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * For now, what we return depends only on pin charge result.
	 ***********************************************************/
	type = (pin_charge_result_t *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_RESULT, 0, ebufp);

	if (type == (pin_charge_result_t *)NULL) {
		/* log? */
		return;
	}

	/***********************************************************
	 * What is the validation result?
	 ***********************************************************/
	switch (*type) {
	case PIN_CHARGE_RES_PASS:
		/* no problem */
		result = PIN_RESULT_PASS;
		descr = "validation successful";
		break;

	case PIN_CHARGE_RES_SRVC_UNAVAIL:
		/* can't do avs - assume good card */
		result = PIN_RESULT_PASS;
		descr = "validation successful";
		break;

	case PIN_CHARGE_RES_FAIL_ADDR_AVS:
		/* generic avs failure */
		result = PIN_RESULT_FAIL;
		descr = "unable to verify address";
		break;

	case PIN_CHARGE_RES_FAIL_ADDR_LOC:
		/* street address failure is acceptable */
		result = PIN_RESULT_PASS;
		descr = "street address not correct";
		break;

	case PIN_CHARGE_RES_FAIL_ADDR_ZIP:
		result = PIN_RESULT_FAIL;
		descr = "zip code not correct";
		break;

	case PIN_CHARGE_RES_FAIL_CARD_BAD:
		/* bogus card */
		result = PIN_RESULT_FAIL;
		descr = "creditcard not valid";
		break;

	case PIN_CHARGE_RES_FAIL_DECL_SOFT:
	case PIN_CHARGE_RES_FAIL_DECL_HARD:
		/* couldn't auth $1 - bogus card? */
		result = PIN_RESULT_FAIL;
		descr = "card failed credit check";
		break;

	case PIN_CHARGE_RES_FAIL_NO_ANS:
		/* temporary failure */
		result = PIN_RESULT_FAIL;
		descr = "unable to validate at this time";
		break;

	case PIN_CHARGE_RES_CHECKPOINT:
		/* uh oh */
		result = PIN_RESULT_FAIL;
		descr = "unable to validate at this time";
		break;

	case PIN_CHARGE_RES_CVV_BAD:
		/* CVV failure */
		result = PIN_RESULT_FAIL;
		descr = "security id check failed";
		break;

	case PIN_CHARGE_RES_FAIL_NO_TOKEN:
		/* Could not generate token */
		result = PIN_CHARGE_RES_FAIL_NO_TOKEN;
		descr = "Failed to generate token";
		break;

	default:
		/* error - I think */
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_RESULT, 0, *type);
		break;
	}

	/***********************************************************
	 * Add the failure info to the result flist.
	 ***********************************************************/
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, (void *)descr, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_validate_result error", ebufp);
	}

	return;
}

