/*******************************************************************
 *
 *      Copyright (c) 2000-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#) %full_filespec: fm_inv_pol_format_invoice_xslt.c;Apollo;081103;54884 %";
#endif

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "ops/inv.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "cm_cache.h"

#include "pin_inv.h"
#include "fm_inv_pol_common.h"

#define FILE_LOGNAME "fm_inv_pol_format_invoice_xslt.c(1)"

/*******************************************************************
 * Global cache pointers (from fm_inv_pol_init)
 *******************************************************************/
extern int32 perf_features_flags;

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_inv_pol_format_invoice_xslt();

static void
fm_inv_pol_format_invoice_xslt(
	pcm_context_t           *ctxp,
	int32                   flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_INV_POL_FORMAT_INVOICE_XSLT operation.
 *******************************************************************/
void
op_inv_pol_format_invoice_xslt(connp, opcode, flags, in_flistp, 
			       ret_flistpp, ebufp)
        cm_nap_connection_t	*connp;
	int32			opcode;
        int32			flags;
        pin_flist_t		*in_flistp;
        pin_flist_t		**ret_flistpp;
        pin_errbuf_t		*ebufp;
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_INV_POL_FORMAT_INVOICE_XSLT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_invoice_xslt opcode error", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_inv_pol_format_invoice_xslt input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_inv_pol_format_invoice_xslt(ctxp, flags, in_flistp, 
				       ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_invoice_xslt error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_inv_pol_format_invoice_xslt return flist", 
				  *ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_inv_pol_format_invoice_xslt():
 *
 *
 * NOTE:
 * By default this function is called from either of the following
 * policy opcode:
 *             PCM_OP_INV_POL_FORMAT_INVOICE
 *             PCM_OP_INV_POL_VIEW_INVOICE
 *
 * Description:
 *******************************************************************/
static void
fm_inv_pol_format_invoice_xslt(
	pcm_context_t           *ctxp,
	int32                     flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t 		*template_flistp = NULL; 
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*inv_flistp = NULL;
	void			*vp = NULL;
	poid_t			*a_pdp = NULL;
	char			portal_locale[BUFSIZ];
	char			type_str[BUFSIZ]; 

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
	
	*ret_flistpp = NULL;

	/* Get the template */
	flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_ACCTINFO, 0, 0, ebufp);
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_ACCOUNT_OBJ, 
		0, ebufp);

	vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_LOCALE, 1, ebufp);
	if (vp == NULL) { /* Set default locale */
		strcpy(portal_locale, "en_US");
	} else {
		strcpy(portal_locale, (char *)vp);
	}

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TYPE_STR, 1, ebufp);
	if (vp == NULL) { /* Set default type string */
		strcpy(type_str, "en_US");
	} else {
		strcpy(type_str, (char *)vp);
	}

	fm_inv_pol_format_invoice_get_template(ctxp, a_pdp, portal_locale,
		type_str, &template_flistp, ebufp);

	if (template_flistp == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
			"No template flist found!");
		goto ErrOut;
	}

	/* First, call PCM_OP_INV_POL_FORMAT_INVOICE_XML to get xml buffer*/
	if (perf_features_flags & INV_PERF_FEAT_USE_OPREF) {
		PCM_OPREF(ctxp, PCM_OP_INV_POL_FORMAT_INVOICE_XML, 0,
			in_flistp, &r_flistp, ebufp);
	} else {
		PCM_OP(ctxp, PCM_OP_INV_POL_FORMAT_INVOICE_XML, 0,
			in_flistp, &r_flistp, ebufp);
	}

	if ( PIN_ERR_IS_ERR( ebufp )) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Call PCM_OP_INV_POL_FORMAT_INVOICE_XML error", ebufp);
		goto ErrOut;
	}

	/* Now create the invoice flist */
	inv_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(inv_flistp, PIN_FLD_POID, vp, ebufp);

	/* Add the xml buffer on the flist */
	flistp = PIN_FLIST_ELEM_TAKE(r_flistp, PIN_FLD_FORMATS, 0, 0, ebufp);
	PIN_FLIST_ELEM_PUT(inv_flistp, flistp, PIN_FLD_INVOICES, 0, ebufp);

	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	/* Add the style sheet buffer on the flist */
	vp = PIN_FLIST_FLD_TAKE(template_flistp, PIN_FLD_NAME, 0, ebufp);
	PIN_FLIST_FLD_PUT(template_flistp, PIN_FLD_TYPE_STR, vp, ebufp);

	PIN_FLIST_ELEM_PUT(inv_flistp, template_flistp, PIN_FLD_FORMATS,
		0, ebufp);

	template_flistp = NULL;

	/* Now call the java opcode PCM_OP_INV_FORMAT_INVOICE */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"PCM_OP_INV_FORMAT_INVOICE: input flist",
		 inv_flistp);

	if (perf_features_flags & INV_PERF_FEAT_USE_OPREF) {
		PCM_OPREF(ctxp, PCM_OP_INV_FORMAT_INVOICE, 0, inv_flistp, 
			&r_flistp, ebufp);
	} else {
		PCM_OP(ctxp, PCM_OP_INV_FORMAT_INVOICE, 0, inv_flistp, 
			&r_flistp, ebufp);
	}

	if ( PIN_ERR_IS_ERR( ebufp )) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Call PCM_OP_INV_FORMAT_INVOICE error", ebufp);
		goto ErrOut;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"PCM_OP_INV_FORMAT_INVOICE: return flist",
		 r_flistp);

	/* Now retrieve the buffer and contruct the return flist */
	flistp = PIN_FLIST_ELEM_TAKE(r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);

	vp = PIN_FLIST_FLD_TAKE(flistp, PIN_FLD_RESULT, 0, ebufp);
	PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_RESULT, vp, ebufp);

	PIN_FLIST_ELEM_PUT(r_flistp, flistp, PIN_FLD_FORMATS, 0, ebufp);
	

	PIN_FLIST_DESTROY_EX(&inv_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&template_flistp, NULL);
	*ret_flistpp = r_flistp;
	return;
ErrOut:
	PIN_FLIST_DESTROY_EX(&inv_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&template_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
}
