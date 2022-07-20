/* continuus file information --- %full_filespec: fm_act_pol_validate_schedule.c~6:csrc:1 % */
/**********************************************************************
*
*	C Source:		fm_act_pol_validate_schedule.c
*	Instance:		1
*	Description:	
*	%created_by:	gmartin %
*	%date_created:	Fri Jul 21 16:54:55 2000 %
*
**********************************************************************/
#ifndef lint
static char *_csrc = "@(#) %filespec: fm_act_pol_validate_schedule.c~6 %  (%full_filespec: fm_act_pol_validate_schedule.c~6:csrc:1 %)";
#endif
/*******************************************************************
 *
 *  @(#) %full_filespec: fm_act_pol_validate_schedule.c~6:csrc:1 %
 *
 *      Copyright (c) 1999 - 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

/*******************************************************************
 * Contains the PCM_OP_ACT_POL_VALIDATE_SCHEDULE operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/act.h"
#include "pin_act.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#define FILE_SOURCE_ID	"fm_act_pol_validate_schedule.c (%I)"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_act_pol_validate_schedule(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_ACT_POL_VALIDATE_SCHEDULE operation.
 *
 * Currently this is just a stub.
 *
 *******************************************************************/
void
op_act_pol_validate_schedule(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*e_flistp = NULL;
	poid_t			*e_pdp = NULL;
	char			*event = NULL;
	void			*vp = NULL;

	*r_flistpp = NULL;
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != PCM_OP_ACT_POL_VALIDATE_SCHEDULE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_validate_schedule bad opcode error", 
			ebufp);
		return;
	}

	/*
	 * Debug: What we got.
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_act_pol_validate_schedule input flist", i_flistp);

	/*
	 * Prepare return flist.
	 */
	if (!PIN_ERR_IS_ERR(ebufp)) {
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
	}

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"op_act_pol_validate_schedule error", ebufp);
	} else {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_act_pol_validate_schedule output flist", *r_flistpp);
	}

	return;
}
