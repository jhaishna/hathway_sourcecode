/* Continuus file information --- %full_filespec: fm_collections_pol_assign_agent.c~1:csrc:1 % */
/*
 * @(#) %full_filespec: fm_collections_pol_assign_agent.c~1:csrc:1 %
 *
 *      Copyright (c) 2001-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static const char Sccs_id[] = "@(#) %full_filespec: fm_collections_pol_assign_agent.c~1:csrc:1 %";
#endif

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#include "cm_fm.h"
#include "cm_cache.h"
#include "ops/collections.h"
#include "fm_utils.h"
#include "fm_collections.h"

#ifdef MSDOS
#include <WINDOWS.h>
#endif

#define FILE_LOGNAME "fm_collections_pol_assign_agent.c(1)"

/*******************************************************************
 * Routines defined here.
 *******************************************************************/

EXPORT_OP void
op_collections_pol_assign_agent(
	cm_nap_connection_t     *connp,
	int32                   opcode,
	int32                   flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_COLLECTIONS_POL_ASSIGN_AGENT
 *******************************************************************/
void
op_collections_pol_assign_agent(
	cm_nap_connection_t     *connp,
	int32                   opcode,
	int32                   flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	pcm_context_t           *ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_COLLECTIONS_POL_ASSIGN_AGENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_pol_assign_agent opcode error",
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_collections_pol_assign_agent input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_collections_pol_assign_agent error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_collections_pol_assign_agent return flist",
			*ret_flistpp);
	}
	return;
}

