/* continuus file information --- %full_filespec: fm_inv_pol_common.h~5:incl:1 % */
/*
 * @(#)pin_inv_common.h 1 Fri Aug 06 10:59:53 1999
 *
 *      Copyright (c) 1999-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef _FM_INV_POL_COMMON_H_
#define _FM_INV_POL_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************
 * String buffer structure used in adding string to buffer
 *******************************************************************/
typedef struct {
	int32 chunksize;
	char *strp;
	int32 size;
	int32 strsize;
} fm_inv_pol_str_buf_t;

/*******************************************************************
 * Add string to buffer routine
 *******************************************************************/
extern void
fm_inv_pol_add_str_to_buf(
	fm_inv_pol_str_buf_t   *bufp,
	char                   *string,
	pin_errbuf_t           *ebufp );

/*******************************************************************
 * Generate return flist from the string buffer.
 * Used in 
 *     PCM_OP_INV_POL_FORMATE_INVOICE_HTML, 
 *     PCM_OP_INV_POL_FORMATE_INVOICE_DOC1, 
 *     PCM_OP_INV_POL_FORMATE_INVOICE_XML
 *******************************************************************/
extern void
fm_inv_pol_format_invoice_finalize(
	fm_inv_pol_str_buf_t   *dbufp,
        char                   *type,
	pin_flist_t            **ret_flistpp,
	pin_errbuf_t           *ebufp);

/*******************************************************************
 * Get the template
 * Used in 
 *     PCM_OP_INV_POL_FORMATE_INVOICE_HTML, 
 *     PCM_OP_INV_POL_FORMATE_INVOICE_XSLT 
 *******************************************************************/
extern void
fm_inv_pol_format_invoice_get_template(
	pcm_context_t           *ctxp,
	poid_t                  *a_pdp,
	char                    *locale,
	char                    *type_str,
	pin_flist_t             **template_flistpp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Read the cache
 * Used in 
 *     PCM_OP_INV_POL_FORMATE_VIEW_INVOICE
 *******************************************************************/
extern void
fm_inv_pol_lookup_config_cache(
	pcm_context_t           *ctxp,
	poid_t                  *a_pdp,
	pin_flist_t             **c_flistp,
	pin_errbuf_t            *ebufp);


#ifdef __cplusplus
}
#endif

#endif /*_FM_INV_POL_COMMON_H_*/
