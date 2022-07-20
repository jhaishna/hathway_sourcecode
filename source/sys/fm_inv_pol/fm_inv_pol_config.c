/*******************************************************************
 *
* Copyright (c) 1999, 2014, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_inv_pol_custom_config.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:40:50 nishahan Exp $";
#endif

#include <stdio.h>	/* for FILE * in pcm.h */
#include "ops/inv.h"
#include "pcm.h"
#include "cm_fm.h"


#ifdef WIN32
__declspec(dllexport) void * fm_inv_pol_custom_config_func();
#endif


/*******************************************************************
 *******************************************************************/
struct cm_fm_config fm_inv_pol_custom_config[] = {
	/* opcode as a u_int, function name (as a string) */
	{ PCM_OP_INV_POL_PREP_INVOICE,
				"op_inv_pol_prep_invoice" },
	{ PCM_OP_INV_POL_FORMAT_INVOICE,
				"op_inv_pol_format_invoice" },
	{ PCM_OP_INV_POL_FORMAT_VIEW_INVOICE,
				"op_inv_pol_format_view_invoice" },
	{ PCM_OP_INV_POL_FORMAT_INVOICE_HTML,
				"op_inv_pol_format_invoice_html" },
	{ PCM_OP_INV_POL_FORMAT_INVOICE_DOC1,
				"op_inv_pol_format_invoice_doc1" },
	{ PCM_OP_INV_POL_FORMAT_INVOICE_XML,
				"op_inv_pol_format_invoice_xml" },
	{ PCM_OP_INV_POL_FORMAT_INVOICE_XSLT,
				"op_inv_pol_format_invoice_xslt" },
	{ PCM_OP_INV_POL_SELECT,
				"op_inv_pol_select" },
	{ PCM_OP_INV_POL_POST_MAKE_INVOICE,
				"op_inv_pol_post_make_invoice" },
	{ 0,(char *)0 }
};

#ifdef WIN32
void *
fm_inv_pol_custom_config_func()
{
	return ((void *) (fm_inv_pol_custom_config));
}
#endif
