/*******************************************************************
 *
 * Copyright (c) 1996, 2009, Oracle and/or its affiliates. All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_act_pol_spec_vrfy.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:16:00 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_ACT_POL_SPEC_VERIFY operation. 
 *
 * Also included are subroutines specific to the operation.
 *
 * These routines are generic and work for all account types.
 *******************************************************************/

/*******************************************************************
 *******************************************************************/
	/***********************************************************
	 ***********************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/act.h"
#include "pin_act.h"
#include "pin_cust.h"
#include "pin_currency.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#define FILE_SOURCE_ID  "fm_act_pol_spec_vrfy.c(26)"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_act_pol_spec_verify(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_act_pol_get_specs(
	pin_flist_t	**p_flistpp,
	pin_errbuf_t	*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_ACT_POL_SPEC_VERIFY operation.
 *******************************************************************/
void
op_act_pol_spec_verify(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*p_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*flistp = NULL;

	poid_t			*a_pdp = NULL;
	char			*action;
	char			*cp;

	int32			rec_id;
	u_int			done = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_ACT_POL_SPEC_VERIFY) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_spec_verify opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_act_pol_spec_verify input flist", in_flistp);

	/***********************************************************
	 * Get the required input fields.
	 ***********************************************************/
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	action = (char *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACTION, 0, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_spec_verify error", ebufp);
		*ret_flistpp = NULL;
	}

	/***********************************************************
	 * Get the policy flist.
	 ***********************************************************/
	fm_act_pol_get_specs(&p_flistp, ebufp);

	/***********************************************************
	 * Construct the list of checks for this action.
	 ***********************************************************/
	done = 0;
	while (!done) {

		flistp = PIN_FLIST_ELEM_GET_NEXT(p_flistp, PIN_FLD_ACTIVITIES,
			&rec_id, 1, &cookie, ebufp);

		if (flistp == (pin_flist_t *)NULL) {
			done = 1;
			continue;
			/*******/
		}

		cp = (char *)PIN_FLIST_FLD_GET(flistp,
			PIN_FLD_ACTION, 0, ebufp);

		if (cp && action && !strcmp(cp, action)) {

			r_flistp = PIN_FLIST_COPY(flistp, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID,
				(void *)a_pdp, ebufp);
			done = 1;

		}

	}

	/***********************************************************
	 * Did we find the desired action? If not, DEFAULT.
	 ***********************************************************/
	if (r_flistp == (pin_flist_t *)NULL) {
		pinlog(FILE_SOURCE_ID, __LINE__, LOG_FLAG_WARNING,
		    "unknown action (%s) in op_act_pol_spec_verify", action);

		done = 0;
		cookie = NULL;
		while (!done) {
			flistp = PIN_FLIST_ELEM_GET_NEXT(p_flistp,
			    PIN_FLD_ACTIVITIES, &rec_id, 1, &cookie, ebufp);

			if (flistp == (pin_flist_t *)NULL) {
				done = 1;
				continue;
				/*******/
			}

			cp = (char *)PIN_FLIST_FLD_GET(flistp,
				PIN_FLD_ACTION, 0, ebufp);

			if (cp && !strcmp(cp, "DEFAULT")) {
				r_flistp = PIN_FLIST_COPY(flistp, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID,
					(void *)a_pdp, ebufp);
				done = 1;
			}
		}
	}

	/***********************************************************
	 * Clean up.
	 ***********************************************************/
	/* Free local memory */
	PIN_FLIST_DESTROY_EX(&p_flistp, NULL);

	/* Error? */
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_spec_verify error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*ret_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*ret_flistpp = r_flistp;

		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_act_pol_spec_verify return flist", r_flistp);

	}

	return;
}


/*******************************************************************
 * fm_act_pol_get_specs():
 *
 *	Routine to retrieve the configured list of actions
 *	and the checks that are associated with each of them.
 *
 *	For now, we hardcode this list to handle a limited set
 *	of actions needed for the system to function. The easiest
 *	way to tweak the configuration is to either change the
 *	'checks' for the actions already defined or to add an
 *	activities array for a new action.
 *
 *	The actions added by default are:
 *		default -
 *			service status = active
 *			service passwd
 *		mail_deliv -
 *			service status = active
 *		mail_login
 *			service status = active
 *			service passwd
 *		act_login (generic)
 *			service status = active
 *			service passwd
 *			credit avail >= 0
 *		term_ip_login
 *			service status = active
 *			service passwd
 *			credit avail >= 0
 *		term_ip_authenticate
 *			service status = active
 *			service passwd
 *			credit avail >= 0
 *		term_ip_authenticate/chap
 *			service status = active
 *			credit avail >= 0
 *		term_ipt_authenticate
 *			service status = active
 *			service passwd
 *			credit avail >= 0
 *			dupe session
 *		term_ipt_authenticate/authenticate_only
 *                      <none>
 *		term_ipt_authenticate/no_dupe_check
 *			service status = active
 *			service passwd
 *			credit avail >= 0
 *		term_ipt_authenticate/chap
 *			service status = active
 *			credit avail >= 0
 *			dupe session
 *		term_ipt_authenticate/chap/ericsson
 *			service status = active
 *			credit avail >= 0
 *			dupe session
 *		term_ipt_authenticate/chap/no_dupe_check
 *			service status = active
 *			credit avail >= 0
 *		gprs_authenticate
 *			service status = active
 *			service passwd
 *		gprs_authenticate/chap
 *			service status = active
 *		gprs_authenticate_anon
 *			service status = active
 *		gprs_authorize
 *			service status = active
 *			credit avail >= 0
 *		sms_authenticate
 *			service status = active
 *			service passwd
 *		sms_authenticate/chap
 *			service status = active
 *		sms_authorize
 *			service status = active
 *			credit avail >= 0
 *
 *	Note, by convention, action strings are the opcode strings.
 *
 *  For each action, the position of each check in the PIN_FLD_CHECKS
 *  array is the number defined for the check itself.  This avoids
 *  overwriting elements in the array, since it guarantees each check
 *  has a unique array position.
 *
 *******************************************************************/
static void
fm_act_pol_get_specs(
	pin_flist_t	**p_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*a_flistp = NULL;
	pin_flist_t	*c_flistp = NULL;

	char		*action;
	char		*event;
	u_int		type;
	u_int		status;
	u_int32		scope_snapshot = PIN_ACT_VERIFY_SCOPE_SNAPSHOT;
	u_int32		scope_master = PIN_ACT_VERIFY_SCOPE_MASTER; 
	u_int		bal_id;
	u_int		scope;
	pin_decimal_t	*amount;
	int32		act_id = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	amount = pbo_decimal_from_str("0.0", ebufp);
	/***********************************************************
	 * Allocate the flist.
	 ***********************************************************/
	flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Someday this flist should have a poid.
	 ***********************************************************/

	/***********************************************************
	 * Define the valid actions
	 ***********************************************************/
	/***********************************************************
	 * action = "PCM_OP_MAIL_DELIV_VERIFY"
	 ***********************************************************/
	action = "PCM_OP_MAIL_DELIV_VERIFY";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
					PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	/***********************************************************
	 * action = "PCM_OP_MAIL_LOGIN_VERIFY" (same plus passwd)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_MAIL_LOGIN_VERIFY";
	PIN_FLIST_ELEM_SET(flistp, a_flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	a_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACTIVITIES, act_id, 0,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_PASSWD;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
					PIN_ACT_CHECK_SRVC_PASSWD, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);

	/***********************************************************
	 * action = "PCM_OP_ACT_LOGIN"
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_ACT_LOGIN";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	type = PIN_ACT_CHECK_SRVC_PASSWD;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_PASSWD, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);

	type = PIN_ACT_CHECK_CREDIT_AVAIL;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_CREDIT_AVAIL, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_master, 
				ebufp);
	bal_id = PIN_CURRENCY_DOLLARS;
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_ELEMENT_ID, (void *)&bal_id, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_AMOUNT, (void *)amount, ebufp);

	/***********************************************************
	 * action = "PCM_OP_TERM_IP_DIALUP_LOGIN" (same checks)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_TERM_IP_DIALUP_LOGIN";
	PIN_FLIST_ELEM_SET(flistp, a_flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	a_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACTIVITIES, act_id, 0,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	/***********************************************************
	 * action = "PCM_OP_TERM_IP_DIALUP_AUTHENTICATE" (same checks)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_TERM_IP_DIALUP_AUTHENTICATE";
	PIN_FLIST_ELEM_SET(flistp, a_flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	a_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACTIVITIES, act_id, 0,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	/***********************************************************
	 * action = "PCM_OP_IPT_AUTHENTICATE/NO_DUPE_CHECK" (same checks)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_IPT_AUTHENTICATE/NO_DUPE_CHECK";
	PIN_FLIST_ELEM_SET(flistp, a_flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	a_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACTIVITIES, act_id, 0,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	/***********************************************************
	 * action = "PCM_OP_IPT_AUTHENTICATE" (same + dupe session)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_IPT_AUTHENTICATE";
	PIN_FLIST_ELEM_SET(flistp, a_flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	a_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACTIVITIES, act_id, 0,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_DUPE_SESSION;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_DUPE_SESSION, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_master, 
				ebufp);
	event = "/ipt/call";
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_OBJ_TYPE, (void *)event, ebufp);
	/*
	 * If PIN_FLD_COUNT is not present, the value in 
	 * PIN_FLD_MAX_ACTIVE_CALLS in the /service object will be used.
	 * This is the default behavior.
	 */
	scope = PIN_ACT_SCOPE_SRVC_OBJ;
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SUBTYPE, (void *)&scope, ebufp);

	/***********************************************************
	 * action = "PCM_OP_TERM_IP_DIALUP_AUTHENTICATE/CHAP"
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_TERM_IP_DIALUP_AUTHENTICATE/CHAP";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_master, 
			ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	type = PIN_ACT_CHECK_CREDIT_AVAIL;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_CREDIT_AVAIL, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_master, 
				ebufp);
	bal_id = PIN_CURRENCY_DOLLARS;
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_ELEMENT_ID, (void *)&bal_id, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_AMOUNT, (void *)amount, ebufp);

	/***********************************************************
	 * action = "PCM_OP_IPT_AUTHENTICATE/CHAP/NO_DUPE_CHECK" (same checks)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_IPT_AUTHENTICATE/CHAP/NO_DUPE_CHECK";
	PIN_FLIST_ELEM_SET(flistp, a_flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	a_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACTIVITIES, act_id, 0,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	/***********************************************************
	 * action = "PCM_OP_IPT_AUTHENTICATE/CHAP" (same + dupe session)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_IPT_AUTHENTICATE/CHAP";
	PIN_FLIST_ELEM_SET(flistp, a_flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	a_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACTIVITIES, act_id, 0,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_DUPE_SESSION;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_DUPE_SESSION, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_master, 
				ebufp);
	event = "/ipt/call";
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_OBJ_TYPE, (void *)event, ebufp);
	/*
	 * If PIN_FLD_COUNT is not present, the value in 
	 * PIN_FLD_MAX_ACTIVE_CALLS in the /service object will be used.
	 * This is the default behavior.
	 */
	scope = PIN_ACT_SCOPE_SRVC_OBJ;
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SUBTYPE, (void *)&scope, ebufp);

	/***********************************************************
	 * action = "PCM_OP_IPT_AUTHENTICATE/CHAP/ERICSSON" (same checks)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_IPT_AUTHENTICATE/CHAP/ERICSSON";
	PIN_FLIST_ELEM_SET(flistp, a_flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	a_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACTIVITIES, act_id, 0,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	/***********************************************************
	 * action = "PCM_OP_IPT_AUTHENTICATE/AUTHENTICATE_ONLY" (no checks)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_IPT_AUTHENTICATE/AUTHENTICATE_ONLY";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	/***********************************************************
	 * action = "PCM_OP_IPT_AUTHENTICATE/CHAP/AUTHENTICATE_ONLY"
	 *(no checks)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_IPT_AUTHENTICATE/CHAP/AUTHENTICATE_ONLY";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	/***********************************************************
	 * action = "PCM_OP_WAP_AUTHENTICATE" 
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_WAP_AUTHENTICATE";
	PIN_FLIST_ELEM_SET(flistp, a_flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	a_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACTIVITIES, act_id, 0,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);
		  
	type = PIN_ACT_CHECK_SRVC_PASSWD;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
	PIN_ACT_CHECK_SRVC_PASSWD, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot,
			  ebufp);

	/***********************************************************
	* action = "PCM_OP_WAP_AUTHENTICATE/CHAP" (*NOT* the same!!!)
	***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_WAP_AUTHENTICATE/CHAP";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
			PIN_ACT_CHECK_SRVC_STATUS, ebufp);

	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot,
					 ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	/***********************************************************
	 * action = "PCM_OP_CONTENT_AUTHENTICATE" 
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_CONTENT_AUTHENTICATE";
	PIN_FLIST_ELEM_SET(flistp, a_flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	a_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACTIVITIES, act_id, 0,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);
		  
	type = PIN_ACT_CHECK_SRVC_PASSWD;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
	PIN_ACT_CHECK_SRVC_PASSWD, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot,
			  ebufp);

	/***********************************************************
	* action = "PCM_OP_CONTENT_AUTHENTICATE/CHAP" (*NOT* the same!!!)
	***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_CONTENT_AUTHENTICATE/CHAP";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
			PIN_ACT_CHECK_SRVC_STATUS, ebufp);

	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot,
					 ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);


	/***********************************************************
	 * action = "PCM_OP_GPRS_AUTHENTICATE" (*NOT* the same!!!)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_GPRS_AUTHENTICATE";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id, 
			ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	type = PIN_ACT_CHECK_SRVC_PASSWD;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_PASSWD, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);

	/***********************************************************
	 * action = "PCM_OP_GPRS_AUTHENTICATE/CHAP" (*NOT* the same!!!)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_GPRS_AUTHENTICATE/CHAP";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id, 
			ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	/***********************************************************
	 * action = "PCM_OP_GPRS_AUTHENTICATE_ANON"
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_GPRS_AUTHENTICATE_ANON";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id, 
			ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	/***********************************************************
	 * action = "PCM_OP_GPRS_AUTHORIZE"
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_GPRS_AUTHORIZE";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id, 
			ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	type = PIN_ACT_CHECK_CREDIT_AVAIL;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_CREDIT_AVAIL, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_master, 
				ebufp);
	bal_id = PIN_CURRENCY_DOLLARS;
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_ELEMENT_ID, (void *)&bal_id, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_AMOUNT, (void *)amount, ebufp);

	/***********************************************************
	 * action = "PCM_OP_SMS_AUTHENTICATE"
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_SMS_AUTHENTICATE";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id, 
			ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	type = PIN_ACT_CHECK_SRVC_PASSWD;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_PASSWD, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);

	/***********************************************************
	 * action = "PCM_OP_SMS_AUTHORIZE"
	 ***********************************************************/
	act_id = act_id + 1;
	action = "PCM_OP_SMS_AUTHORIZE";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	type = PIN_ACT_CHECK_CREDIT_AVAIL;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_CREDIT_AVAIL, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_master, 
				ebufp);
	bal_id = PIN_CURRENCY_DOLLARS;
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_ELEMENT_ID, (void *)&bal_id, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_AMOUNT, (void *)amount, ebufp);


	/***********************************************************
	 * action = "DEFAULT" (for things that don't have ops yet)
	 ***********************************************************/
	act_id = act_id + 1;
	action = "DEFAULT";
	a_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ACTIVITIES, act_id,
		ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACTION, (void *)action, ebufp);

	type = PIN_ACT_CHECK_SRVC_STATUS;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_STATUS, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);
	status = PIN_STATUS_ACTIVE;
	c_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_CHOICES, 0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	type = PIN_ACT_CHECK_SRVC_PASSWD;
	c_flistp = PIN_FLIST_ELEM_ADD(a_flistp, PIN_FLD_CHECKS,
				PIN_ACT_CHECK_SRVC_PASSWD, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_TYPE, (void *)&type, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_SCOPE, (void *)&scope_snapshot, 
				ebufp);


	pbo_decimal_destroy(&amount);
	/***********************************************************
	 * action = ?
	 ***********************************************************/

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_act_pol_get_specs error", ebufp);
		PIN_FLIST_DESTROY_EX(&flistp, NULL);
		*p_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*p_flistpp = flistp;
	}

	return;
}

