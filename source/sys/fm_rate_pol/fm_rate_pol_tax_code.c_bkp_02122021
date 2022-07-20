/*******************************************************************
 *
* Copyright (c) 2003, 2016, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_rate_pol_tax_code.c /cgbubrm_7.5.0.portalbase/2 2016/04/18 23:15:05 mmukeshk Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_RATE_POL_GET_TAXCODE operation. 
 *
 * Used to provide the information about available taxcodes.
 * Used in the Price Tool to help to define product <-> taxcode
 * connection.
 *
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"

#define FILE_SOURCE_ID  "fm_rate_pol_get_taxcode.c"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_rate_pol_get_taxcode(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);


static void
fm_rate_pol_get_taxcode(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_RATE_POL_GET_TAXCODE operation.
 *******************************************************************/
void
op_rate_pol_get_taxcode(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_RATE_POL_GET_TAXCODE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_get_taxcode opcode error", ebufp);
		return;
		/*****/
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_rate_pol_get_taxcode input flist", i_flistp);

	/***********************************************************
	 * Do the actual op in a sub.
	 ***********************************************************/
	fm_rate_pol_get_taxcode(ctxp, i_flistp, o_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_get_taxcode error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		  "op_rate_pol_get_taxcode return flist", *o_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_rate_pol_get_taxcode():
 *
 *    This policy will return the cached list of taxcodes
 *    in an flist.
 *
 *******************************************************************/
static void
fm_rate_pol_get_taxcode(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;
	pin_flist_t             *flistp = NULL;
	char            *taxCode = NULL;
	u_int32                 glid = 0;
	pin_cookie_t    cookie = NULL;
	int32           elemid = 0;
	int32		flags = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
		/*****/
	}

	PIN_ERR_CLEAR_ERR(ebufp);
	*o_flistpp = NULL;

	/*
	 * set something to return
	 */
	*o_flistpp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_POID, (void *)vp, ebufp);

	/*
	 * populate the return flist with cached list of taxcodes
         * 
         * Flag is passed in to determine the type of returned taxcodes.
         * Possible flag values are:
         *    0 (default) - Return all taxcodes.
         *    1 - Return taxcodes that are associated with a tax package only.
         *    2 - Return taxcodes that are user-defined only.
	 */
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (vp) {
		flags = *(int32*)vp;
	} 
	fm_utils_tax_get_taxcodes(ctxp, *o_flistpp, flags, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_GL_ID, 1, ebufp);
        if (vp) {
		while ((flistp = PIN_FLIST_ELEM_GET_NEXT(*o_flistpp, PIN_FLD_RESULTS,
			&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

			/* Get the tax code from the results array */
		        taxCode = (char*)PIN_FLIST_FLD_GET(flistp,PIN_FLD_TAX_CODE, 0, ebufp);
			fm_utils_taxcode_to_glid(ctxp, taxCode, &glid, ebufp);
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_GL_ID, (void *)&glid, ebufp);
		}

	}

	return;
}

