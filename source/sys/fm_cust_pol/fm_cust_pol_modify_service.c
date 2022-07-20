/*******************************************************************
 *
 *      Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_cust_pol_modify_service.c:RWSmod7.3.1Int:1:2007-Jun-28 05:36:22 %";
#endif

#include <stdio.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pcm.h"
#include "pinlog.h"
#include "fm_utils.h"

EXPORT_OP void
op_cust_pol_modify_service(
	cm_nap_connection_t	*connp,
	u_int			opcode,
	u_int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_modify_service(
	pcm_context_t		*ctxp,
	u_int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*
 * Change the service info for the service object
 */
void
op_cust_pol_modify_service(
	cm_nap_connection_t	*connp,
	u_int			opcode,
	u_int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t           *ctxp = connp->dm_ctx;
	
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Null out results until we have some.
	 */
	*r_flistpp = NULL;

	/*
	 * Insanity check.
	 */
	if (opcode != PCM_OP_CUST_POL_MODIFY_SERVICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_modify_service", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			  "op_cust_pol_modify_service input flist:",
			  i_flistp);

	/*
	 * Call the main function.
	 */
	fm_cust_pol_modify_service(ctxp, flags, i_flistp, r_flistpp, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			  "op_cust_pol_modify_service output flist:",
			  *r_flistpp);


	if (PIN_ERR_IS_ERR(ebufp))
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				 "op_cust_pol_modify_service flist error",
				 ebufp);
	
	return;
}

static void
fm_cust_pol_modify_service(
	pcm_context_t		*ctxp,
	u_int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	if (*r_flistpp == NULL) {
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
	}
	
	return;
}
