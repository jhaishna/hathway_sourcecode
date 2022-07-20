/*
 *      Copyright (c) 2004 - 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_act_pol_post_authorize.c:BillingVelocityInt:4:2006-Sep-08 04:09:46 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"

#define FILE_LOGNAME "fm_act_pol_post_authorize.c(1)"

/*******************************************************************
 * PCM_OP_ACT_POL_POST_AUTHORIZE 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * after aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_act_pol_post_authorize(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_act_pol_post_authorize(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * PCM_OP_ACT_POL_POST_AUTHORIZE  
 *******************************************************************/
void 
op_act_pol_post_authorize(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp = NULL;
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != PCM_OP_ACT_POL_POST_AUTHORIZE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_post_authorize error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_act_pol_post_authorize input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_act_pol_post_authorize(ctxp, flags, i_flistp, r_flistpp, ebufp);

	/*******************************************************************
	 * Errors? 
	 *******************************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"op_act_pol_post_authorize error", ebufp);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"op_act_pol_post_authorize output flist", 
			*r_flistpp);
	}
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_act_pol_post_authorize(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t	*rate_flistp = NULL;
	int32		elemid = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;

	*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
	/* IF rating result is present in the output flist, drop it.
	 * By default we will drop this flist, to trim the final output flist.
         * If any of the vertical managers need this rating output, they need 
         * to customize the policy.
         */
	rate_flistp = PIN_FLIST_ELEM_GET(*r_flistpp, PIN_FLD_RESULTS,
		elemid, 1, ebufp);
	if (rate_flistp) {
		PIN_FLIST_ELEM_DROP(*r_flistpp, PIN_FLD_RESULTS, elemid, ebufp);
	}


	/********************************************************* 
	 * Errors..?
	 *********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_act_pol_post_authorize error",ebufp);
	}
	
	return;
}
