/*******************************************************************
 *	@(#)$Id: fm_inv_pol_format_view.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:41:03 nishahan Exp $
 *
 * Copyright (c) 2000, 2009, Oracle and/or its affiliates.All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/
#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_inv_pol_format_view.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:41:03 nishahan Exp $";
#endif

/*******************************************************************
 * This file contains the PCM_OP_INV_POL_FORMAT_VIEW_INVOICE 
 * Also included are subroutines specific to the operation. 
 *******************************************************************/

#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "ops/inv.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "cm_cache.h"
#include "fm_utils.h"

#include "pin_inv.h"

#define PIN_ERR_NOT_PROPER_FORMAT 93

extern cm_cache_t *fm_inv_pol_config_cache_ptr;
extern int32 fm_inv_pol_service_enabled;

/*******************************************************************
 * Routines defined here.
 *******************************************************************/

EXPORT_OP void op_inv_pol_format_view_invoice();
static void fm_inv_pol_format_view_invoice();

static void
fm_inv_pol_format_view_reorder(
	pcm_context_t		*ctxp,
	pin_flist_t		*inv_flistp,
	pin_errbuf_t		*ebufp );

static void
fm_inv_pol_format_view_reorder_items(
	pcm_context_t		*ctxp,
	pin_flist_t		*inv_flistp,
	int32			fld_no,
	pin_errbuf_t		*ebufp );

static void
fm_inv_pol_format_view_get_format(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp );

/**********************************************************
 * Formatting functions contained in other source files
 **********************************************************/
extern void
fm_inv_pol_lookup_config_cache(
	pcm_context_t           *ctxp,
	poid_t                  *a_pdp,
	pin_flist_t             **c_flistp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_INV_POL_FORMAT_VIEW_INVOICE operation.
 *******************************************************************/
void
op_inv_pol_format_view_invoice(connp, opcode, flags, in_flistp, 
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
	if (opcode != PCM_OP_INV_POL_FORMAT_VIEW_INVOICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_view_invoice opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_inv_pol_format_view_invoice input flist", in_flistp);

	/***********************************************************
	 * Do the actual op in a sub. Copy it since we may need to
	 * replace it later.
	 ***********************************************************/

	fm_inv_pol_format_view_invoice(ctxp, flags, in_flistp, 
				       ret_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_view_invoice error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_inv_pol_format_view_invoice return flist", 
				  *ret_flistpp);
	}
	return;
}

/*******************************************************************
 * fm_inv_pol_format_view_invoice()
 *
 * This function will be called when a requested invoice format
 * does not exist in the invoice object.  In this case, attempt
 * to format on the fly by calling this opcode.  By default, this
 * opcode will not do anything.  
 *
 *******************************************************************/
static void
fm_inv_pol_format_view_invoice(ctxp, flags, in_flistp, ret_flistpp, ebufp)
	pcm_context_t		*ctxp;
	int32			flags;
	pin_flist_t		*in_flistp;
	pin_flist_t		**ret_flistpp;
	pin_errbuf_t		*ebufp;
{
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*t_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*inv_flistp = NULL;
	poid_t			*a_pdp = NULL;
	void			*vp = NULL;
	int32			val = 0;
	char			*type = NULL;
	int32			*i_ptr = NULL;
	int32			err = 0;
	int32			flag = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	*ret_flistpp = NULL;

	/* Get the type string from the input flist */
	vp = PIN_FLIST_FLD_GET( in_flistp, PIN_FLD_TYPE_STR, 0, ebufp );
	if( vp ) type = (char *)vp;
	else type = "";

	/**************************************************************
	 * Now get the invoice flist.
	 **************************************************************/
	fm_inv_pol_format_view_get_format( ctxp, in_flistp, 
			  &inv_flistp, ebufp );

	if (inv_flistp == NULL) {
		goto ErrOut;
	}

	if (fm_inv_pol_service_enabled) {
		fm_inv_pol_format_view_reorder(ctxp, inv_flistp, ebufp);
		/* Add a flag in inv_flistp for the formatting opcode */
		PIN_FLIST_FLD_SET(inv_flistp, PIN_FLD_FLAGS, 
			(void *)&fm_inv_pol_service_enabled, ebufp);
	}

	/**************************************************************
	 * Now look into the cache to see if XSLT engine is to be used.
	 **************************************************************/
	tmp_flistp = PIN_FLIST_ELEM_GET(inv_flistp, PIN_FLD_ACCTINFO,
		0, 0, ebufp);
	a_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ,
		0, ebufp);

	fm_inv_pol_lookup_config_cache(ctxp, a_pdp, &r_flistp, ebufp);

	if (r_flistp == NULL) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Config cache lookup failed ", ebufp);
		goto ErrOut;
	}

	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (vp == NULL) {
		flag = 0;
	} else {
		flag = *(int32 *)vp;
	}

	/* Free r_flistp and set it to NULL */
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	if (flag) { /* Use XSLT transformation engine */
		/* Put the type string on the invoice flist */
		/* in order to get the right style sheet */

		PIN_FLIST_FLD_SET(inv_flistp, PIN_FLD_TYPE_STR, (void *)type, 
			ebufp);

		PCM_OP(ctxp, PCM_OP_INV_POL_FORMAT_INVOICE_XSLT, 0, 
			inv_flistp, &r_flistp, ebufp);

	} else { /* Otherwise, call the right policy */

		if( strcasecmp( type, FM_INV_FORMAT_TYPE_XML ) == 0 ) {
		/* text/xml */

			PCM_OP(ctxp, PCM_OP_INV_POL_FORMAT_INVOICE_XML, 0, 
				inv_flistp, &r_flistp, ebufp);
		} else if ( strcasecmp( type, FM_INV_FORMAT_TYPE_DOC1 ) == 0 ) {
		/* text/doc1 */

			PCM_OP(ctxp, PCM_OP_INV_POL_FORMAT_INVOICE_DOC1, 0, 
				inv_flistp, &r_flistp, ebufp);
		} else if (strcasecmp( type, FM_INV_FORMAT_TYPE_HTML ) == 0 ) {
		/* text/html */

			PCM_OP(ctxp, PCM_OP_INV_POL_FORMAT_INVOICE_HTML, 0, 
				inv_flistp, &r_flistp, ebufp);
		}
	}
	if( r_flistp ) {
		vp  = PIN_FLIST_FLD_GET( r_flistp, PIN_FLD_RESULT, 1, ebufp );	
	}

	if( r_flistp == NULL || ( vp != (void *) NULL &&
		(*(int32 *)vp == PIN_RESULT_FAIL)) )  {
	  	/************************************************
		 * Unknown format
		 ************************************************/
		if (!r_flistp) {
	  		r_flistp = PIN_FLIST_CREATE( ebufp );
			vp = PIN_FLIST_FLD_GET( in_flistp, PIN_FLD_POID,
					 0, ebufp );
			PIN_FLIST_FLD_SET( r_flistp, PIN_FLD_POID, vp, ebufp );
			val = PIN_RESULT_FAIL;
			PIN_FLIST_FLD_SET( r_flistp, PIN_FLD_RESULT, 
				   (void *)&val, ebufp );
		}
	}
	else {
		vp = PIN_FLIST_FLD_GET( in_flistp, PIN_FLD_POID, 0, ebufp );
		PIN_FLIST_FLD_SET( r_flistp, PIN_FLD_POID, vp, ebufp );
		val = PIN_RESULT_PASS;
		PIN_FLIST_FLD_SET( r_flistp, PIN_FLD_RESULT, 
				   (void *)&val, ebufp );
	}


	if ( PIN_ERR_IS_ERR( ebufp )) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_format_view_invoice flist error", ebufp);
		goto ErrOut;
	}

	/**************************************************************
	 * Success
	 **************************************************************/
	
	*ret_flistpp = r_flistp;
	PIN_FLIST_DESTROY_EX(&inv_flistp, NULL );
	return;
ErrOut:
	PIN_FLIST_DESTROY_EX( &r_flistp, NULL );
	PIN_FLIST_DESTROY_EX(&inv_flistp, NULL );
	*ret_flistpp = NULL;
	return;
}

/*******************************************************************
 * This function will assume that the database contains at least
 * one format for each invoice which is text/xml format.
 * It receives as input the list of all known formats in the DB.
 * if pin_flist or xml  is in there, it will read that array to obtain 
 * the flist which it will convert to a real flist and pass out.
 *
 */ 
static void
fm_inv_pol_format_view_get_format(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*f_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	void			*vp = NULL;
	pin_buf_t		*pin_bufp = NULL;
	int32			elem = 0;
	pin_cookie_t		cookie = NULL;
	char			*type = NULL;
	int32			found = 0;
	poid_t			*inv_poidp = NULL;

	if( PIN_ERR_IS_ERR( ebufp )) {
		return;
	}
	PIN_ERR_CLEAR_ERR( ebufp );
	*ret_flistpp = NULL;

	/* First loop through the formats that exist in the DB right
	 * now, we're looking for text/pin_flist format to use as base
	 */
	found = -1;
	cookie = NULL;
	while((f_flistp = PIN_FLIST_ELEM_GET_NEXT( in_flistp, PIN_FLD_FORMATS, 
				  &elem, 1, &cookie, ebufp )) != NULL 
	      			&& found < 0 ) {

		vp = PIN_FLIST_FLD_GET( f_flistp, PIN_FLD_TYPE_STR, 
					0, ebufp );
		if(vp) type = (char *)vp;
		else type = "";

		if( strcasecmp( type, FM_INV_FORMAT_TYPE_PIN_FLIST ) == 0 
			|| strcasecmp( type, FM_INV_FORMAT_TYPE_XML) == 0 ) {
			/* this is it */
		  	found = elem;
		}
	}

	/* If text/pin_flist isn't there, then we can't generate anything */
	if( found < 0 ) {
	  	/* not an ebuf error, but output null flist */
		goto ErrOut;
	}

	/**********************************************************
	 * If the text/pin_flist was found, then let's do the 
	 * READ_FLDS to get the BUF,
	 **********************************************************/

	s_flistp = PIN_FLIST_CREATE( ebufp );
	inv_poidp = (poid_t *)PIN_FLIST_FLD_GET( in_flistp, PIN_FLD_POID, 
						 0, ebufp );
	PIN_FLIST_FLD_SET( s_flistp, PIN_FLD_POID, (void *)inv_poidp, ebufp );
	f_flistp = PIN_FLIST_ELEM_ADD( s_flistp, PIN_FLD_FORMATS, 
				       found, ebufp );
	PIN_FLIST_FLD_SET( f_flistp, PIN_FLD_BUFFER, (void *)NULL, ebufp );

	if( PIN_ERR_IS_ERR( ebufp )) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_format_view_invoice flist error", ebufp);
		goto ErrOut;
	}

        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, s_flistp, &r_flistp, ebufp);

	if( PIN_ERR_IS_ERR( ebufp )) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_format_view_invoice pcm op error", ebufp);
		goto ErrOut;
	}

	/*************************************************************
	 * OK, we should now have the pin_flist array available for 
	 * us.  Use the Buffer from there to create a new format for
	 * the output flist.
	 *************************************************************/

	f_flistp = PIN_FLIST_ELEM_GET( r_flistp, PIN_FLD_FORMATS, 
				       found, 0, ebufp );
	pin_bufp = (pin_buf_t *)PIN_FLIST_FLD_GET( f_flistp, PIN_FLD_BUFFER,
						   0, ebufp );
	if ( pin_bufp == NULL || pin_bufp->data == NULL ) {
	  	/* not strictly an error.  The buffer could be null
	  	 * But we'll output a null pointer anyway.
		 */
		 flistp = NULL;
	}
	else {

		/* Warning, undocumented API call */
		pin_str_to_flist( pin_bufp->data, 0, &flistp, ebufp );

		if( PIN_ERR_IS_ERR( ebufp )) {
			pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
				PIN_ERR_BAD_VALUE, PIN_FLD_FORMATS, 0, 0,PIN_DOMAIN_ERRORS,
				PIN_ERR_NOT_PROPER_FORMAT, 1,0, NULL);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error putting buffer onto flist", ebufp);
			goto ErrOut;
		}
	}


	*ret_flistpp = flistp;
	if( s_flistp ) PIN_FLIST_DESTROY( s_flistp, NULL );
	if( r_flistp ) PIN_FLIST_DESTROY( r_flistp, NULL );
	return;
ErrOut:
	if( r_flistp ) PIN_FLIST_DESTROY( r_flistp, NULL );
	if( s_flistp ) PIN_FLIST_DESTROY( s_flistp, NULL );
	if( flistp ) PIN_FLIST_DESTROY( flistp, NULL );
	*ret_flistpp = NULL;
	return;
}

static void
fm_inv_pol_format_view_reorder(
	pcm_context_t		*ctxp,
	pin_flist_t		*inv_flistp,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t		*r_flistp = NULL;

	if( PIN_ERR_IS_ERR( ebufp )) {
		return;
	}
	PIN_ERR_CLEAR_ERR( ebufp );

	fm_inv_pol_format_view_reorder_items(ctxp, inv_flistp, 
		PIN_FLD_AR_ITEMS, ebufp);

	fm_inv_pol_format_view_reorder_items(ctxp, inv_flistp, 
		PIN_FLD_SUB_ITEMS, ebufp);

	fm_inv_pol_format_view_reorder_items(ctxp, inv_flistp, 
		PIN_FLD_OTHER_ITEMS, ebufp);

	return;
}


static void
fm_inv_pol_format_view_reorder_items(
	pcm_context_t		*ctxp,
	pin_flist_t		*inv_flistp,
	int32			fld_no,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*ungrouped_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*service_in_flistp = NULL;
	pin_flist_t		*service_out_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	int32			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	int32			inner_elem_id = 0;
	pin_cookie_t		inner_cookie = NULL;
	int32			ungrouped_elem_id = 0;
	poid_t			*service_pdp = NULL;
	poid_t			*r_service_pdp = NULL;
	int32			items_elem_id = 0;
	int32			last_elem_id = 0;
	int32			found = 0;
	void			*vp = NULL;


	if( PIN_ERR_IS_ERR( ebufp )) {
		return;
	}
	PIN_ERR_CLEAR_ERR( ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_format_view_invoice_reorder_items input flist", 
		inv_flistp);

	/** Construct the input flist to read /service object */
	service_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(service_in_flistp, PIN_FLD_LOGIN, NULL, ebufp);

	/** Walk through the items array and re-group it by service */
	last_elem_id = 0;
	while((flistp = PIN_FLIST_ELEM_TAKE_NEXT( inv_flistp, fld_no,
			  &elem_id, 1, &cookie, ebufp )) != NULL) {
		service_pdp = PIN_FLIST_FLD_GET(flistp, 
				PIN_FLD_SERVICE_OBJ, 0, ebufp);
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_inv_pol_format_view_invoice_reorder_items"
				" error", 
				ebufp);
			goto Cleanup;
		}
		if (PIN_POID_IS_NULL(service_pdp)) {
			if (ungrouped_flistp == NULL) {
				ungrouped_flistp = PIN_FLIST_CREATE(ebufp);
			}
			PIN_FLIST_ELEM_PUT(ungrouped_flistp, flistp, 
				PIN_FLD_ITEMS, ungrouped_elem_id, ebufp);
			ungrouped_elem_id++;
			continue;
		}
		if (r_flistp == NULL) {
			r_flistp = PIN_FLIST_CREATE( ebufp );
		}
		inner_cookie = NULL;
		inner_elem_id = 0;
		found = 0;
		while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT( r_flistp, 
			fld_no, &inner_elem_id, 1, &inner_cookie, ebufp )) 
			!= NULL) {
			r_service_pdp = PIN_FLIST_FLD_GET(tmp_flistp, 
					PIN_FLD_SERVICE_OBJ, 0, ebufp);
			if (!PIN_POID_COMPARE(service_pdp, r_service_pdp, 
				0, ebufp)) {
				found = 1;
				items_elem_id = PIN_FLIST_COUNT(tmp_flistp, 
						ebufp) - 1;
				PIN_FLIST_ELEM_PUT(tmp_flistp, 
						flistp, 
						PIN_FLD_ITEMS, 
						items_elem_id, 
						ebufp);
				flistp = NULL;
				break;
			}
		}
		if (!found) { /* Add a new element in the array */
			/* Get the service info from /service first */
			PIN_FLIST_FLD_SET(service_in_flistp, PIN_FLD_POID,
				(void *)service_pdp, ebufp);

			PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, 
				service_in_flistp, 
				&service_out_flistp, 
				ebufp);

			if (PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_inv_pol_format_view_invoice_"
					"reorder_items READ /service error", 
					ebufp);
				goto Cleanup;
			}

			tmp_flistp = PIN_FLIST_ELEM_ADD(r_flistp, fld_no,
				last_elem_id++, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ,
				(void *)service_pdp, ebufp);

			vp = PIN_FLIST_FLD_TAKE(service_out_flistp, 
				PIN_FLD_LOGIN, 0, ebufp);
			PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_LOGIN, vp, 
				ebufp);
			PIN_FLIST_DESTROY_EX(&service_out_flistp, NULL);

			PIN_FLIST_ELEM_PUT(tmp_flistp, flistp, 
				PIN_FLD_ITEMS, 0, ebufp);
			flistp = NULL;
		}
	} 

	/** Attach the un-grouped items to the return flist */	
	if (ungrouped_flistp != NULL) {
		if (r_flistp == NULL) {
			r_flistp = PIN_FLIST_CREATE(ebufp);
		}
		PIN_FLIST_ELEM_PUT(r_flistp, ungrouped_flistp, 
			fld_no, last_elem_id, ebufp);
		ungrouped_flistp = NULL;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_format_view_invoice_reorder_items result flist", 
		r_flistp);

	/** Put the re-ordered items flist back to the invoice flist */
	PIN_FLIST_CONCAT(inv_flistp, r_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_inv_pol_format_view_invoice_reorder_items return flist", 
		inv_flistp);

Cleanup:
	PIN_FLIST_DESTROY_EX(&flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ungrouped_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&service_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&service_out_flistp, NULL);
	return;
}	

