/*******************************************************************
 *
 * Copyright (c) 2004, 2009, Oracle and/or its affiliates.All rights reserved. 
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_subscription_pol_prep_members.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 06:17:26 nishahan Exp $";
#endif

#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "cm_fm.h"
#include "pinlog.h"
#include "pin_subscription.h"
#include "ops/subscription.h"
#include "ops/monitor.h"

#define PIN_FLAG_OWNER_OBJ 	0
#define PIN_FLAG_AR_OBJ 	1

EXPORT_OP void
op_subscription_pol_prep_members(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_SUBSCRIPTION_POL_PREP_MEMBERS command.
 * This is called by PCM_OP_SUBSCRIPTION_GROUP_SHARING_CREATE and 
 * PCM_OP_SUBSCRIPTION_GROUP_SHARING_MODIFY to validate the members 
 * based on the monitor type passed.
 *******************************************************************/
void
op_subscription_pol_prep_members(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)

{
	pcm_context_t	*ctxp = connp->dm_ctx;
	pin_flist_t	*ret_flistp	 = NULL;

	poid_t	*grp_poidp = NULL;

        PIN_ERR_CLEAR_ERR(ebufp);
	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_SUBSCRIPTION_POL_PREP_MEMBERS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_subscription_pol_prep_members", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get in input?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_prep_members input flist", in_flistp);

	grp_poidp = (poid_t *) PIN_FLIST_FLD_GET(in_flistp, 
		PIN_FLD_GROUP_OBJ, 1, ebufp);

	if (grp_poidp && !strcmp(PIN_POID_GET_TYPE(grp_poidp),
		GROUP_SHARING_PROFILES_POID_TYPE)) {
		ret_flistp = PIN_FLIST_COPY(in_flistp, ebufp);
	} else {
		/************************************************************
		 * Call the validation opcode to validate the members
		 *************************************************************/

		PCM_OP(ctxp, PCM_OP_MONITOR_VALIDATE_MONITOR_MEMBERS, flags, 
				in_flistp, &ret_flistp, ebufp);
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	
	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_prep_members output flist", ret_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                    "op_subscription_pol_prep_members error", ebufp);
	} else {
		*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	}
	
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}
