/*
 *
* Copyright (c) 2003, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_pymt_pol_collect.c /cgbubrm_mainbrm.portalbase/2 2019/02/19 22:42:38 agangrad Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_PYMT_POL_COLLECT operation. 
 *******************************************************************/


#include <stdio.h> 
#include <string.h> 
 
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
#include "fm_bill_utils.h"
#include "ops/ar.h"
#include "psiu_business_params.h"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_pymt_pol_collect(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_collect_result(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_collect_result_pass(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_collect_result_hard(
	pcm_context_t		*ctxp,
	package_proceeding_type_t contract_proceeding,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_collect_result_soft(
	pcm_context_t		*ctxp,
	package_proceeding_type_t contract_proceeding,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_collect_config(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_pymt_pol_collect_clear_pending(
	pin_flist_t	*r_flistp,
	pin_decimal_t	*amount,
	char		*descr,
	pin_errbuf_t	*ebufp);

static void
fm_pymt_pol_collect_set_status(
	pin_flist_t		*r_flistp,
	pin_status_t		status,
	int			flags,
	char			*descr,
	pin_errbuf_t		*ebufp);


static void
fm_pymt_pol_collect_writeoff_account(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int			flags,
	pin_errbuf_t		*ebufp);
	
static void
fm_pymt_pol_collect_command_none(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int		flags,
	pin_flist_t	**r_flistp,	
	pin_errbuf_t	*ebufp);	

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_COLLECT operation.
 *******************************************************************/
void
op_pymt_pol_collect(
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
	pin_flist_t             *e_flistp = NULL;
        pin_cookie_t            cookie = NULL;
        int32                   elemid = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_COLLECT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_collect opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_collect input flist", i_flistp);

	/***********************************************************
	 * Prep the return flist.
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);

	/******************************************************
         * Addition of PIN_FLD_EVENTS in output flist
         ******************************************************/

        while ((e_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,
                PIN_FLD_EVENTS, &elemid, 1, &cookie,
                ebufp)) != (pin_flist_t *)NULL) {
                PIN_FLIST_ELEM_SET(r_flistp, e_flistp, PIN_FLD_EVENTS,
                                                        elemid, ebufp);
        }

	cmdp = (int *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_COMMAND, 0, ebufp); 
			 	
	if (cmdp && *cmdp == PIN_CHARGE_CMD_NONE){
		
		/***********************************************************
		 * Processing for PIN_CHARGE_CMD_NONE
		 ***********************************************************/
		fm_pymt_pol_collect_command_none(ctxp, i_flistp, flags,
							 	&r_flistp, ebufp);

	} else {

		/***********************************************************
		 * Fill in the results.
		 ***********************************************************/
		fm_pymt_pol_collect_result(ctxp, i_flistp, r_flistp, ebufp);

	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_collect error", ebufp);
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
			"op_pymt_pol_collect return flist", r_flistp);

	}

	return;
}

/*******************************************************************
 * fm_pymt_pol_collect_result():
 *
 *	Turn a pin_charge result into a collect result.
 *
 *******************************************************************/
static void
fm_pymt_pol_collect_result(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_charge_result_t	*type = NULL;
	int			result = 0;
	void			*vp = NULL;
	pin_flist_t             *res_flistp = NULL;
	poid_t 			*a_pdp = NULL;
	pin_flist_t 		*temp_flistp = NULL;
	package_proceeding_type_t contract_proceeding = PACKAGE_NOT_CONTRACTABLE;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	res_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if(res_flistp){
                vp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 1, ebufp);

                if (vp){
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, vp, ebufp);
                }
                else{
                        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, vp, ebufp);
                }
        }
        else {
                vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, vp, ebufp);
        }
	/***********************************************************
	 * Our behavoir depends primarily on the pin_result.
	 ***********************************************************/
	type = (pin_charge_result_t *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_RESULT, 0, ebufp);

	if (type == (pin_charge_result_t *)NULL) {
		/* log? */
		return;
	}

	/***********************************************************
	 * What is the pin_result?
	 ***********************************************************/
	if (*type >= PIN_CHARGE_RES_AUTH_PENDING){
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, 
			(void *)type, ebufp);
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 
			1, ebufp);
		if (vp) {
			PIN_FLIST_FLD_SET(r_flistp, 
				PIN_FLD_DESCR, vp, ebufp);
		}
	} else {	
		/* need to check the contract situation for given scope */
        	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

        	temp_flistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_POID, (void *) a_pdp, ebufp);
                PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_SCOPE_OBJ, (void *) a_pdp, ebufp);
        	contract_proceeding = fm_bill_utils_get_subscriber_contracts(ctxp,
                               		0, temp_flistp, NULL, ebufp);
        	PIN_FLIST_DESTROY_EX(&temp_flistp, NULL);

		switch (*type) {
		case PIN_CHARGE_RES_PASS:
			fm_pymt_pol_collect_result_pass(ctxp,
				i_flistp, r_flistp, ebufp);
			break;

		case PIN_CHARGE_RES_FAIL_CARD_BAD:
		case PIN_CHARGE_RES_FAIL_DECL_HARD:
			fm_pymt_pol_collect_result_hard(ctxp,
				contract_proceeding, i_flistp, r_flistp, ebufp);
			break;

		case PIN_CHARGE_RES_CVV_BAD:
		case PIN_CHARGE_RES_FAIL_DECL_SOFT:
		case PIN_CHARGE_RES_FAIL_ADDR_AVS:
		case PIN_CHARGE_RES_FAIL_ADDR_LOC:
		case PIN_CHARGE_RES_FAIL_ADDR_ZIP:
		case PIN_CHARGE_RES_FAIL_NO_ANS:
		case PIN_CHARGE_RES_SRVC_UNAVAIL:
		case PIN_CHARGE_RES_FAIL_FORMAT_ERROR:
		case PIN_CHARGE_RES_FAIL_LOGICAL_PROBLEM:
		case PIN_CHARGE_RES_FAIL_INVALID_CONTENT:
		case PIN_CHARGE_RES_FAIL_TECHNICAL_PROBLEM :
			fm_pymt_pol_collect_result_soft(ctxp,
				contract_proceeding, i_flistp, r_flistp, ebufp);
			break;

		case PIN_CHARGE_RES_FAIL_NO_MIN:
		case PIN_CHARGE_RES_FAIL_SELECT_ITEMS:
		case PIN_CHARGE_RES_NO_CREDIT_BALANCE:
		case PIN_CHARGE_RES_INVALID_CMD:
		
			result = PIN_RESULT_FAIL;
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, 
				(void *)&result, ebufp);
			vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR,
							1, ebufp);
			if (vp){
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR,
							vp, ebufp);
			}
			break;

		default:
			/* error - I think */
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_RESULT, 0, *type);
			break;
		}
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_collect_result error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_pymt_pol_collect_result_pass():
 *
 *	Create the collect result for a PASS pin result.
 *
 *******************************************************************/
static void
fm_pymt_pol_collect_result_pass(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*b_flistp = NULL;
	pin_charge_cmd_t	*cmd;
	pin_decimal_t		*amount = NULL;
	pin_decimal_t		*pend = NULL;
	pin_decimal_t		*dp = NULL; /*temp*/
	char			*descr;
	pin_status_t		*status;
	int			*flags;
	int			result;
	pin_flist_t 		*vp;
	pin_flist_t		*inh_flistp;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * All passes get the PASS result.
	 ***********************************************************/
	result = PIN_RESULT_PASS;
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);

	/***********************************************************
	 * The rest depends on the command.
	 ***********************************************************/
	cmd = (pin_charge_cmd_t *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_COMMAND, 0, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_collect_result_pass error - no command in input flist", ebufp);
		return;
	}

	/***********************************************************
	 * Calc and add the description.
	 ***********************************************************/
	if(cmd){
	switch (*cmd) {
	case PIN_CHARGE_CMD_AUTH_ONLY:
		descr = "authorization successful";
		break;
	case PIN_CHARGE_CMD_CONDITION:
		descr = "authorization & deposit successful";
		break;
	case PIN_CHARGE_CMD_DEPOSIT:
		descr = "deposit successful";
		break;
	case PIN_CHARGE_CMD_REFUND:
		descr = "refund successful";
		break;
	default:
		/* error - I think */
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_COMMAND, 0, *cmd);
		goto cleanup;
	}
	}
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, (void *)descr, ebufp);
        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
        if(vp){
                inh_flistp = PIN_FLIST_SUBSTR_ADD(r_flistp, PIN_FLD_INHERITED_INFO, ebufp);
                PIN_FLIST_FLD_SET(inh_flistp, PIN_FLD_AMOUNT, vp, ebufp);
        }


	/***********************************************************
	 * Read the config info.
	 ***********************************************************/
	fm_pymt_pol_collect_config(ctxp, i_flistp, &b_flistp, ebufp);

	/***********************************************************
	 * Add any needed actions.
	 ***********************************************************/
	switch (*cmd) {
	case PIN_CHARGE_CMD_AUTH_ONLY:
	case PIN_CHARGE_CMD_CONDITION:
	case PIN_CHARGE_CMD_REFUND:
		amount = (pin_decimal_t *)PIN_FLIST_FLD_GET(i_flistp,
			PIN_FLD_AMOUNT, 0, ebufp);
		pend = (pin_decimal_t *)PIN_FLIST_FLD_GET(b_flistp,
			PIN_FLD_PENDING_RECV, 0, ebufp);
		status = (pin_status_t *)PIN_FLIST_FLD_GET(b_flistp,
			PIN_FLD_STATUS, 0, ebufp);
		flags = (int *)PIN_FLIST_FLD_GET(b_flistp,
			PIN_FLD_STATUS_FLAGS, 0, ebufp);

		if (PIN_ERR_IS_ERR(ebufp))
			break;

		if (pbo_decimal_compare(amount, pend, ebufp) >= 0) {
			dp = pbo_decimal_negate(pend, ebufp);

			fm_pymt_pol_collect_clear_pending(r_flistp, dp,
				"credit toward outstanding bill", ebufp);
			if ((status && (*status == PIN_STATUS_INACTIVE)) &&
				(flags && (*flags & PIN_STATUS_FLAG_DEBT))) {
				fm_pymt_pol_collect_set_status(r_flistp,
					PIN_STATUS_ACTIVE, PIN_STATUS_FLAG_DEBT,
					"cleared outstanding debt", ebufp);
			}
		} else {
			dp = pbo_decimal_negate(amount, ebufp);
			fm_pymt_pol_collect_clear_pending(r_flistp, dp,
				"credit toward outstanding bill", ebufp);
		}
		pbo_decimal_destroy(&dp);
		break;
	case PIN_CHARGE_CMD_DEPOSIT:
		break;
	default:
		/* error - I think */
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_COMMAND, 0, *cmd);
		goto cleanup;
	}

	/***********************************************************
         * Cleanup
         ***********************************************************/

cleanup :

		PIN_FLIST_DESTROY_EX(&b_flistp, NULL);
		/***********************************************************
	 	 * Error?
		 ***********************************************************/
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_pymt_pol_collect_result_pass error", ebufp);
		}

		return;
}

/*******************************************************************
 * fm_pymt_pol_collect_result_hard():
 *
 *	Create the collect result for 'hard' pin declines.
 *
 *	All hard declines are suject to the same rules.
 *
 *******************************************************************/
static void
fm_pymt_pol_collect_result_hard(
	pcm_context_t		*ctxp,
	package_proceeding_type_t contract_proceeding,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*b_flistp = NULL;

	int			result;
	char			*descr;
	pin_status_t		*status;
	int			*flags;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * All get the FAIL result and default descr.
	 ***********************************************************/
	result = PIN_RESULT_FAIL;
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);
	descr = "creditcard operation declined";
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, (void *)descr, ebufp);

	/***********************************************************
	 * Read the config info.
	 ***********************************************************/
	fm_pymt_pol_collect_config(ctxp, i_flistp, &b_flistp, ebufp);

	/***********************************************************
	 * If not already inactive for debt/closed, suspend.
	 ***********************************************************/
	status = (pin_status_t *)PIN_FLIST_FLD_GET(b_flistp,
		PIN_FLD_STATUS, 0, ebufp);
	flags = (int *)PIN_FLIST_FLD_GET(b_flistp,
		PIN_FLD_STATUS_FLAGS, 0, ebufp);

	if (status) {
		switch (*status) {
		case PIN_STATUS_INACTIVE:
			if (flags && (*flags & PIN_STATUS_FLAG_DEBT))
				break;
			/* intentional fall-thru */
		case PIN_STATUS_ACTIVE:
			 /* In OMC1.2 with contracts service inactivation is not supported */
			if (contract_proceeding != PACKAGE_NOT_CONTRACTABLE &&
	                      contract_proceeding != PACKAGE_NOT_COMMITTED_CONTRACT) {
				PIN_ERR_LOG_MSG (PIN_ERR_LEVEL_DEBUG,
				    "Inactivation is not supported with contracts"
					"Skipping...");
			} else {
				fm_pymt_pol_collect_set_status(r_flistp, PIN_STATUS_INACTIVE,
					PIN_STATUS_FLAG_DEBT, "credit card failure", ebufp);
			}
			break;
		case PIN_STATUS_CLOSED:
			break;
                default:
                       break;  
		}
	}

	/***********************************************************
	 * Someday check for age of debt and close after so long.
	 ***********************************************************/

	/***********************************************************
	 * Cleanup.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&b_flistp, NULL);

	/* Error? - caught/logged by caller */

	return;
}

/*******************************************************************
 * fm_pymt_pol_collect_result_soft():
 *
 *	Create the collect result for 'soft' pin declines.
 *
 *	All soft declines are subject to the same rules, but
 *	we may need/want to treat each command differently.
 *
 *******************************************************************/
static void
fm_pymt_pol_collect_result_soft(
	pcm_context_t		*ctxp,
	package_proceeding_type_t contract_proceeding,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*a_flistp = NULL;
	pin_flist_t		*b_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_cookie_t		cookie = NULL;

	int			result;
	int			rec_id;
	char			*descr = NULL;
	pin_status_t		*status = NULL;
	int			*flags = NULL;
	poid_t			*a_pdp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*b_pdp = NULL;
	void			*vp = NULL;
	char			*templatep;
	int32			srch_flags = SRCH_DISTINCT;

	int			item_status = PIN_ITEM_STATUS_OPEN;
	pin_decimal_t		*item_due = NULL;
	time_t			start_t = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	item_due = pbo_decimal_from_str("0.01", ebufp);

	/***********************************************************
	 * All get the FAIL result and default descr.
	 ***********************************************************/
	result = PIN_RESULT_FAIL;
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);
	descr = "creditcard operation declined";
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, (void *)descr, ebufp);

	/***********************************************************
	 * Read the config info.
	 ***********************************************************/
	fm_pymt_pol_collect_config(ctxp, i_flistp, &b_flistp, ebufp);

	/***********************************************************
	 * If not already inactive for debt/closed, suspend.
	 ***********************************************************/
	status = (pin_status_t *)PIN_FLIST_FLD_GET(b_flistp,
		PIN_FLD_STATUS, 0, ebufp);
	flags = (int *)PIN_FLIST_FLD_GET(b_flistp,
		PIN_FLD_STATUS_FLAGS, 0, ebufp);
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(b_flistp, 
		PIN_FLD_POID, 0, ebufp);
	b_pdp = (poid_t *)PIN_FLIST_FLD_GET(b_flistp, 
		PIN_FLD_LAST_BILL_OBJ, 0, ebufp);

	/***********************************************************
	 * Nothing to do, if there was no last bill object.
	 ***********************************************************/
	if (PIN_POID_IS_NULL(b_pdp)) {
		PIN_FLIST_DESTROY_EX(&b_flistp, NULL);
		pbo_decimal_destroy(&item_due);
		return;
	}

	/***********************************************************
	 * Read the bill object for the start of the billing cycle.
	 ***********************************************************/
	vp = (void *)NULL;
	a_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_POID, (void *)b_pdp, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_START_T, vp, ebufp);

	/***********************************************************
	 * Read the info from the last bill object.
	 ***********************************************************/
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, a_flistp, &flistp, ebufp);
	vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_START_T, 0,ebufp);
	if (vp) start_t = *((time_t *)vp);

	PIN_FLIST_DESTROY_EX(&a_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&flistp, NULL);
	
	/***********************************************************
	 * Allocate the flist for search the item objects.
	 ***********************************************************/
	a_flistp = PIN_FLIST_CREATE(ebufp);

	/**************************************************************
	 * Construct the search poid.
	 **************************************************************/
	s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp), "/search/pin", 
		-1, ebufp);
	PIN_FLIST_FLD_PUT(a_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);

	/**************************************************************
	 * Specify the search arguments.
	 **************************************************************/
	flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)a_pdp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&item_status, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DUE, (void *)item_due, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_EFFECTIVE_T, (void *)&start_t, ebufp);

	templatep = "select X from /item/$1 "
		" where F1 = V1 "
		" and F2 = V2 "
		" and F3 >= V3 "
		" and F4 <= V4 ";
 
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_TEMPLATE, (void *)templatep, 
		ebufp);
	/**************************************************************
	 * Set-up the results array.
	 **************************************************************/
	vp = (void *)NULL;
	flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, vp, ebufp);

	/**************************************************************
	 * Call the PCM_OP_SEARCH opcode to do the actual search.
	 **************************************************************/
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, a_flistp, &flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&a_flistp, NULL);

	/**************************************************************
	 * Did we find any items with 30 days due.
	 **************************************************************/
	a_flistp = PIN_FLIST_ELEM_GET_NEXT(flistp, PIN_FLD_RESULTS,
                &rec_id, 1, &cookie, ebufp);

	if ((a_flistp != (pin_flist_t *)NULL) && (status)) {
		switch (*status) {
		case PIN_STATUS_INACTIVE:
			if (flags && (*flags & PIN_STATUS_FLAG_DEBT))
				break;
			/* intentional fall-thru */
		case PIN_STATUS_ACTIVE:
			/* In OMC1.2 with contracts service inactivation is not supported */
                        if (contract_proceeding != PACKAGE_NOT_CONTRACTABLE &&
	                      contract_proceeding != PACKAGE_NOT_COMMITTED_CONTRACT) {

                                PIN_ERR_LOG_MSG (PIN_ERR_LEVEL_DEBUG,
                                    "Inactivation is not supported with contracts feature"
                                        "Skipping...");
                        } else {
                                fm_pymt_pol_collect_set_status(r_flistp,
                                    PIN_STATUS_INACTIVE, PIN_STATUS_FLAG_DEBT,
                                            "credit card pastdue", ebufp);
                        }
			break;
		case PIN_STATUS_CLOSED:
			break;
                default:
                       break;  
		}
	}

	/***********************************************************
	 * Someday check for age of debt and close after so long.
	 ***********************************************************/

	/***********************************************************
	 * Cleanup.
	 ***********************************************************/

	PIN_FLIST_DESTROY_EX(&b_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&flistp, NULL);
	pbo_decimal_destroy(&item_due);

	/* Error? - caught/logged by caller */

	return;
}

/*******************************************************************
 * fm_pymt_pol_collect_config():
 *
 *	Performs the initial data gathering needed for figuring
 *	out what policy action is needed. We read some info from
 *	the account object and then also read the last bill object
 *	(as pointed to by the account).
 *
 *******************************************************************/
static void
fm_pymt_pol_collect_config(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*a_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	poid_t		*a_pdp = NULL;
	poid_t		*b_pdp = NULL;
	void		*vp;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Null the results until we have them.
	 ***********************************************************/
	*o_flistpp = (pin_flist_t *)NULL;

	/***********************************************************
	 * Get the input info.
	 ***********************************************************/
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	/***********************************************************
	 * Create the flist for reading the account.
	 ***********************************************************/
	a_flistp = PIN_FLIST_CREATE(ebufp);

	vp = (void *)a_pdp;
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_POID, vp, ebufp);

	vp = (void *)NULL;
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_STATUS, vp, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_STATUS_FLAGS, vp, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_PENDING_RECV, vp, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_LAST_BILL_OBJ, vp, ebufp);

	/***********************************************************
	 * Read the info from the account.
	 ***********************************************************/
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, a_flistp, o_flistpp, ebufp);

	/***********************************************************
	 * Clean up.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&a_flistp, NULL);

	b_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, 
					PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	
	/***********************************************************
	 * Create the flist for reading the billinfo.
	 ***********************************************************/
	a_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_POID, b_pdp, ebufp);

	vp = (void *)NULL;
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_PENDING_RECV, vp, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_LAST_BILL_OBJ, vp, ebufp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, a_flistp, &r_flistp, ebufp);

	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PENDING_RECV, 0, ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_PENDING_RECV, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_LAST_BILL_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_LAST_BILL_OBJ, vp, ebufp);

	PIN_FLIST_DESTROY_EX(&a_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_collect_config", ebufp);
	}

	return;
}

/*******************************************************************
 * Routines for adding an activity array element to an flist.
 * The actions we understand are:
 *	- "clear_pending"
 *	- "set_status"
 *	- "cease_billing"
 * These are literal strings coded into PCM_OP_BILL_COLLECT.
 *******************************************************************/
/*******************************************************************
 * fm_pymt_pol_collect_clear_pending():
 *******************************************************************/
static void
fm_pymt_pol_collect_clear_pending(
	pin_flist_t	*r_flistp,
	pin_decimal_t	*amount,
	char		*descr,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*flistp = NULL;
	char		*action;
	int		count;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Don't bother with zero amounts.
	 ***********************************************************/
	if (pbo_decimal_sign(amount, ebufp) == 0) {
		return;
		/*****/
	}

	/***********************************************************
	 * What elemid to add.
	 ***********************************************************/
	count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_ACTIVITIES, ebufp);

	/***********************************************************
	 * Add our element.
	 ***********************************************************/
	flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_ACTIVITIES, count, ebufp);

	/***********************************************************
	 * Fill in our values.
	 ***********************************************************/
	action = "clear_pending";
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACTION, (void *)action, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_AMOUNT, (void *)amount, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, (void *)descr, ebufp);

	return;
}

/*******************************************************************
 * fm_pymt_pol_collect_set_status():
 *******************************************************************/
static void
fm_pymt_pol_collect_set_status(
	pin_flist_t		*r_flistp,
	pin_status_t		status,
	int			flags,
	char			*descr,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*flistp = NULL;
	char			*action;
	int			count;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * What elemid to add.
	 ***********************************************************/
	count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_ACTIVITIES, ebufp);

	/***********************************************************
	 * Add our element.
	 ***********************************************************/
	flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_ACTIVITIES, count, ebufp);

	/***********************************************************
	 * Fill in our values.
	 ***********************************************************/
	action = "set_status";
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACTION, (void *)action, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&status, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS_FLAGS, (void *)&flags, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, (void *)descr, ebufp);

	return;
}


/*******************************************************************
 * fm_pymt_pol_collect_writeoff_account():
 * This function checks for the event type .
 * If event type is /event/billing/writeoff_reversal
 * then it writesoff the account/billinfo based on LINK_OBJ
 *******************************************************************/
static void
fm_pymt_pol_collect_writeoff_account(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int			flags,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*flistp              = NULL;
	pin_flist_t		*wa_flistp           = NULL;
	pin_flist_t		*r_flistp            = NULL;
	pin_flist_t		*read_flistp         = NULL;
	pin_flist_t		*read_ret_flistp     = NULL;
	poid_t                  *wo_acct_flistp      = NULL;
	poid_t                  *wor_pdp             = NULL;
	char			*writeoff_event_type = { "" };
	int			result_flag          = 0;
	void			*vp                  = NULL;
	int32			rec_id               = 0;
	pin_cookie_t		cookie               = NULL;
	int			temp                 = 0;
	u_int			count                = 0;
	int32			opcode               = PCM_OP_AR_BILLINFO_WRITEOFF;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Check for the event type /event/billing/writeoff_reversal
	 ***********************************************************/
	cookie = NULL;
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_EVENTS,
		&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)0){
		
			vp = PIN_FLIST_FLD_GET( flistp, PIN_FLD_POID, 0, ebufp);

			if (vp != NULL)
				writeoff_event_type = 
					(char *)PIN_POID_GET_TYPE((poid_t *) vp );

			if( writeoff_event_type && strcmp( writeoff_event_type, 
					PIN_OBJ_TYPE_EVENT_WRITEOFF_REVERSAL) == 0 ){
				read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(flistp, PIN_FLD_POID, read_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_LINK_OBJ, NULL, ebufp);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_ret_flistp, ebufp);

				/********************************************************************
				 * LINK_OBJ determines if the WOR was done for account or billinfo.
				 * This decides if we need to re-writeoff the account or billinfo.
				 ********************************************************************/
				wor_pdp = PIN_FLIST_FLD_GET(read_ret_flistp, PIN_FLD_LINK_OBJ, 1, ebufp);

				vp = PIN_FLIST_FLD_GET( flistp, PIN_FLD_RESULT, 0, ebufp);
				if ( vp != NULL ) {
					result_flag = *(int *)vp;
				}

				if( result_flag == PIN_REVERSE_RES_PASS ) {
		
					/********************************************************************
					 * If the Writeoff reversal was done on the billinfo, re-writeoff 
					 * the billinfo. Else, re-writeoff the account. 
					 * Writeoff the Account/Billifo by calling appropriate opcode 
					 ********************************************************************/
					wa_flistp = PIN_FLIST_CREATE(ebufp);

					if (wor_pdp) {
						vp = wor_pdp;
					} else {
						vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
					}
	
					PIN_FLIST_FLD_SET(wa_flistp, PIN_FLD_POID, vp, ebufp);

					vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
					PIN_FLIST_FLD_SET(wa_flistp, PIN_FLD_PROGRAM_NAME, vp, ebufp);
					/*************************************************************
					* Add the String ID and String version for the GL
					*************************************************************/
					temp = PIN_WRITEOFF_FOR_AUTO_WRITEOFF_REVERSAL;
					PIN_FLIST_FLD_SET(wa_flistp, PIN_FLD_STRING_ID, &temp, ebufp);
					temp = PIN_REASON_CODES_CREDIT_REASONS;
					PIN_FLIST_FLD_SET(wa_flistp, PIN_FLD_STR_VERSION, &temp, ebufp);

					/*************************************************************
					* Perform Account writeoff in the end since it will writeoff
					* everything.
					*************************************************************/
					if (strcmp((const char*)PIN_POID_GET_TYPE(wor_pdp), "/account") == 0) {
						if (!count) {
							wo_acct_flistp = PIN_FLIST_COPY(wa_flistp, ebufp);
							count = 1;
						}
					} else {
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
							"fm_pymt_pol_collect_writeoff input flist", wa_flistp);
						PCM_OP(ctxp, opcode, flags, wa_flistp, &r_flistp, ebufp);

						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
							"fm_pymt_pol_collect_writeoff output flist", r_flistp);
					}

					if (PIN_ERR_IS_ERR(ebufp)) {
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"fm_pymt_pol_collect_writeoff", ebufp);
					}
		
					/***********************************************************
					 * Clean up.
					 ***********************************************************/
					PIN_FLIST_DESTROY_EX(&wa_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
				}

				/***********************************************************
				 * Clean up.
				 ***********************************************************/
				PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_ret_flistp, NULL);
			}
	}

	/***********************************************************
	 * Call Account writeoff, if it was reversed.
	 ***********************************************************/
	if (wo_acct_flistp) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_pymt_pol_collect_writeoff_account input flist", wo_acct_flistp);
		PCM_OP(ctxp, PCM_OP_AR_ACCOUNT_WRITEOFF, flags, wo_acct_flistp, &r_flistp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_pymt_pol_collect_writeoff_account output flist", r_flistp);

		/***********************************************************
		 * Clean up.
		 ***********************************************************/
		PIN_FLIST_DESTROY_EX(&wo_acct_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_collect_writeoff_account error", ebufp);
	}
	
	return;
}

static void
fm_pymt_pol_collect_command_none(
	pcm_context_t	*ctxp,
	pin_flist_t	*flistp,
	int		flags,
	pin_flist_t	**r_flistp,	
	pin_errbuf_t	*ebufp)
{
	void		*vp = NULL;
	int		dummy = PIN_RESULT_FAIL;
	char 		writeoff_rev_level[9] = { "" };
	int 		writeoff_rev_flag = 0;	
	pin_flist_t     *res_flistp = NULL;
	char            *pdp = NULL;
	
	res_flistp = PIN_FLIST_ELEM_TAKE(flistp, 
		PIN_FLD_RESULTS, 0, 1, ebufp);
	
	/******************************************
	 * Appending Fields from PIN_FLD_RESULTS
	 * into output flist
	 ******************************************/

	if (res_flistp) {
		PIN_FLIST_CONCAT(*r_flistp, res_flistp, ebufp); 
	}

	PIN_FLIST_DESTROY_EX(&res_flistp, NULL)
	/*******************************************
	 * Set the result to pass.
	 *******************************************/
	vp = PIN_FLIST_FLD_GET(*r_flistp, PIN_FLD_POID,
		1, ebufp);	
	if(!vp){
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_POID,
						0, ebufp);
		PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_POID,
						vp, ebufp);	
	}

	pdp = (char *)PIN_POID_GET_TYPE((poid_t *)vp);

	/*******************************************
	 * If the poid is /event/billing/payment or
	 * /event/billing/refund then only consider
	 * payment or refund as successful.
	 *******************************************/
	if (pdp && (strstr(pdp,"/event/billing/"))) {
		dummy = PIN_RESULT_PASS;
	} else {
		dummy = PIN_RESULT_FAIL;
	}
	PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_RESULT, 
		(void *)&dummy, ebufp);

	dummy = 0;
	PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_TYPE, 
		(void *)&dummy, ebufp);

	/***********************************************************
	 * Write-off account action happens only in case when Automatic 
	 * WOR feature is enabled and the writeoff level is set to 
	 * "account" (account) or "bilinfo" in /config/business_params
	 ***********************************************************/
	writeoff_rev_flag = psiu_bparams_get_int(ctxp, PSIU_BPARAMS_AR_PARAMS, 
					PSIU_BPARAMS_AR_AUTO_WO_REV_ENABLE, ebufp);
	
	if (writeoff_rev_flag && psiu_bparams_get_str(ctxp, PSIU_BPARAMS_AR_PARAMS, 
		PSIU_BPARAMS_AR_WRITEOFF_LEVEL, writeoff_rev_level, 9, ebufp)) 
		{
	
		if ( writeoff_rev_level && ((strcmp(writeoff_rev_level, PIN_WRITEOFF_REV_LEVEL_ACCOUNT) == 0)
					|| (strcmp(writeoff_rev_level, PIN_WRITEOFF_REV_LEVEL_BILLINFO) == 0)) ){
			fm_pymt_pol_collect_writeoff_account(ctxp, flistp, flags, ebufp);							
		}		
			
	}	 
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_collect_command_none error", ebufp);
	}
	
	return;	
	
}
