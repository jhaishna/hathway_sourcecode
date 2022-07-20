/**
 * Copyright (c) 1999, 2009, Oracle and/or its affiliates. All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 **/


#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_act_pol_spec_rates.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:15:58 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_ACT_POL_SPEC_RATES operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/cust.h"
#include "ops/act.h"
#include "pin_act.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "fm_utils.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_term.h"

#define FILE_SOURCE_ID	"fm_act_pol_spec_rates.c(1.16)"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_act_pol_spec_rates(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp);


/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
extern u_int fm_utils_op_is_ancestor();

/*******************************************************************
 * Main routine for the PCM_OP_ACT_POL_SPEC_RATES operation.
 *******************************************************************/
void
op_act_pol_spec_rates(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*e_flistp = NULL;
	poid_t			*e_pdp = NULL;
	const char			*event = NULL;
	char			*rate = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*******************************************************************
	 * Insanity check.
 	 *******************************************************************/
	if (opcode != PCM_OP_ACT_POL_SPEC_RATES) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_spec_rates bad opcode error", ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: What we got.
 	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_act_pol_spec_rates input flist", i_flistp);

	/*******************************************************************
	 * Our action will depend on the type of event
	 * Get the event substruct to get all the needed data
 	 *******************************************************************/
	e_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EVENT, 0, ebufp);
	e_pdp = (poid_t *)PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_POID, 0, ebufp);
	event = PIN_POID_GET_TYPE(e_pdp);
	*r_flistpp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, e_pdp, ebufp);

	/*******************************************************************
	 * Here is a sample rating for changing passwd. Basically checks
	 * the event type and the opcode stack. If we are called from
	 * CREATE_SERVICE don't bother charging.
 	 *******************************************************************/
	if (event && (strcmp(event, PIN_OBJ_TYPE_EVENT_PASSWORD) == 0) &&
		(fm_utils_op_is_ancestor(connp->coip, 
			PCM_OP_CUST_CREATE_SERVICE) == 0)) {

		rate = PIN_OBJ_TYPE_EVENT_PASSWORD;
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_RATE_PLAN_NAME,
				  rate, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_IMPACT_CATEGORY,
				  "default", ebufp);
	}

	/*******************************************************************
	 * Error?
 	 *******************************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(r_flistpp, NULL);
		*r_flistpp = (pin_flist_t *)NULL;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_spec_rates error", ebufp);
	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_act_pol_spec_rates return flist", *r_flistpp);
	}

	return;
}
