/*******************************************************************
 *
 *      Copyright (c) 2000-2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/
#ifndef lint
static char *_csrc = "@(#) %filespec: fm_inv_pol_format_invoice_xml.c~6 %  (%full_filespec: fm_inv_pol_format_invoice_xml.c~6:csrc:1 %)";
#endif

#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "ops/inv.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#include "pin_inv.h"

#include "fm_inv_pol_common.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_inv_pol_format_invoice_xml();

static void
fm_inv_pol_format_invoice_xml(
	pcm_context_t           *ctxp,
	int32                     flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_INV_POL_FORMAT_INVOICE_XML operation.
 *
 * NOTE:
 * This opcode will become OBSOLETE in future Portal versions.
 *******************************************************************/
void
op_inv_pol_format_invoice_xml(connp, opcode, flags, in_flistp, 
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
	if (opcode != PCM_OP_INV_POL_FORMAT_INVOICE_XML) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_invoice_xml opcode error", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_inv_pol_format_invoice_xml input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_inv_pol_format_invoice_xml(ctxp, flags, in_flistp, 
				       ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_invoice_xml error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_inv_pol_format_invoice_xml return flist", 
				  *ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_inv_pol_format_invoice_xml():
 *
 * This function converts the input flist into the default Portal
 * invoicing output for the XML format.
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
fm_inv_pol_format_invoice_xml(
	pcm_context_t           *ctxp,
	int32                     flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	fm_inv_pol_str_buf_t	dbuf;
	char		str[BUFSIZ];

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	if( in_flistp == NULL ) {
		return;
	}

	/***********************************************************
	 * Reset ret_flistpp.
	 ***********************************************************/
	*ret_flistpp = NULL;

        /*******************************************************
         * Initialize dbuf - will hold the xml invoice
         *******************************************************/

        dbuf.strp = NULL;
        dbuf.strsize = 0;

	/***********************************************************
	 * Debug what we have got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_format_invoice_xml: input flist",
		 in_flistp);

	/***********************************************************
	 * Convert the flist to XML 
	 ***********************************************************/
	PIN_FLIST_TO_XML( in_flistp, PIN_XML_BY_SHORT_NAME, 0,
		&dbuf.strp, &dbuf.strsize, "invoice", ebufp );

	dbuf.strsize -= 2;
	sprintf(str, "Buffer size: %d\n", dbuf.strsize);
	PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG, str );
	/***********************************************************
	 * Now create the return flist and put the buffer onto it.
	 ***********************************************************/
	fm_inv_pol_format_invoice_finalize(&dbuf, 
		FM_INV_FORMAT_TYPE_XML, ret_flistpp, ebufp);

	if( dbuf.strp ) {
		pin_free( dbuf.strp );
	}
	return;
}
