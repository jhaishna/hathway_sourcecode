/*******************************************************************
 *
 *      Copyright (c) 1996 - 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_rate_pol_tax_loc.c:ServerIDCVelocityInt:3:2006-Sep-06 16:40:39 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_RATE_POL_TAX_LOC operation. 
 *
 * Used to provide the relevant address information for use
 * in taxing a particular event. The addresses provided are
 * used to determine jusridiction of the tax calculated.
 *
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#define FILE_SOURCE_ID  "fm_rate_pol_tax_loc.c"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_rate_pol_tax_loc(
        cm_nap_connection_t	*connp,
	u_int32			opcode,
        u_int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_rate_pol_tax_loc(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * Main routine for the PCM_OP_RATE_POL_TAX_LOC operation.
 *******************************************************************/
void
op_rate_pol_tax_loc(
        cm_nap_connection_t	*connp,
	u_int32			opcode,
        u_int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_RATE_POL_TAX_LOC) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_tax_loc opcode error", ebufp);
		return;
		/*****/
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_rate_pol_tax_loc input flist", i_flistp);

	/***********************************************************
	 * Do the actual op in a sub.
	 ***********************************************************/
	fm_rate_pol_tax_loc(ctxp, i_flistp, o_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_tax_loc error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_rate_pol_tax_loc return flist", *o_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_rate_pol_tax_loc():
 *
 *    This policy could do several things to prepare the input
 *    flist for the tax calculation.  It could locate the locale's
 *    involved. 
 *
 *    The return flist should look like this:
 *	0 PIN_FLD_POID           POID [0] 0.0.0.1 /event/session -1 0
 *	0 PIN_FLD_ORDER_ACCEPT    STR [0] "Cupertino;CA;95014;US"
 *	0 PIN_FLD_ORDER_ORIGIN    STR [0] "Cupertino;CA;95014;US"
 *	0 PIN_FLD_SHIP_FROM       STR [0] "Cupertino;CA;95014;US"
 *	0 PIN_FLD_SHIP_TO         STR [0] "Cupertino ;CA ;95014 ;USA"
 *
 *    The default implementation is just to return the POID only. 
 *
 *******************************************************************/
static void
fm_rate_pol_tax_loc(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) 
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	*o_flistpp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	if (vp) {
		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_POID, (void *)vp, ebufp);
	}

	return;
}


