/*******************************************************************
 *
* Copyright (c) 1996, 2016, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_rate_pol_tax_supplier.c /cgbubrm_7.5.0.portalbase/2 2016/04/19 13:26:50 vivilin Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_RATE_POL_GET_TAX_SUPPLIER operation. 
 *
 * Used to provide the information about available tax suppliers.
 * Used in the Price Tool to help to define product <-> tax_supplier
 * connection...
 *
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/rate.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"

#define FILE_SOURCE_ID  "fm_rate_pol_tax_supplier.c"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_rate_pol_get_tax_supplier(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);


static void
fm_rate_pol_get_tax_supplier(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_RATE_POL_GET_TAX_SUPPLIER operation.
 *******************************************************************/
void
op_rate_pol_get_tax_supplier(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
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
	if (opcode != PCM_OP_RATE_POL_GET_TAX_SUPPLIER) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_get_tax_supplier opcode error", ebufp);
		return;
		/*****/
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_rate_pol_get_tax_supplier input flist", i_flistp);

	/***********************************************************
	 * Do the actual op in a sub.
	 ***********************************************************/
	fm_rate_pol_get_tax_supplier(ctxp, i_flistp, o_flistpp, ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_get_tax_supplier error", ebufp);
	} else {
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		  "op_rate_pol_get_tax_supplier return flist", *o_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_rate_pol_get_tax_supplier():
 *
 *    This policy will search through cached list of tax suppliers
 *    and return findings...
 *
 *******************************************************************/
static void
fm_rate_pol_get_tax_supplier(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	poid_t			*pdp = NULL;
	int32 			element_id = 0;
	pin_cookie_t		cookie = NULL;
	int64			pd_db = 0;
	int64			pd_id = 0;
	char			*pd_type = NULL;
	int			*tax_pkg = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) 
		return;
		/*****/

	PIN_ERR_CLEAR_ERR(ebufp);
	*o_flistpp = NULL;

	/******************************************************************
	 * Use the input flist poid to get the database number
	 ******************************************************************/
	pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_POID_IS_NULL(pdp) || PIN_ERR_IS_ERR(ebufp)) {
		goto ErrOut;
		/**********/
	}
	pd_db = PIN_POID_GET_DB(pdp);

	/*
	 * set something to return
	 */
	*o_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_POID, (void *)pdp, ebufp);

        fm_utils_init_tax_supplier_objects_flistp = fm_bill_utils_cfg_get_tax_supplier(ctxp, ebufp);
	if (!fm_utils_init_tax_supplier_objects_flistp) {
		goto ErrOut;
		/**********/
	}

	pdp = (poid_t*)PIN_FLIST_FLD_GET(
			fm_utils_init_tax_supplier_objects_flistp,
			PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	pd_type = (char *) PIN_POID_GET_TYPE(pdp);
	pd_id = PIN_POID_GET_ID(pdp);

	/******************************************************************
	 * Loop through list of cached tax suppliers
	 ******************************************************************/
	while( (flistp = PIN_FLIST_ELEM_GET_NEXT(
		fm_utils_init_tax_supplier_objects_flistp, 
		PIN_FLD_TAX_SUPPLIERS,
		&element_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL ) {
		
		r_flistp = PIN_FLIST_ELEM_ADD(*o_flistpp, PIN_FLD_RESULTS,
							 element_id, ebufp);

		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_TAX_SUPPLIER, &element_id,
			ebufp);
        	PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_ACCOUNT_OBJ,
			PIN_POID_CREATE(pd_db, pd_type, pd_id, ebufp), ebufp);

		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_NAME, 0, ebufp);
        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_NAME, vp,ebufp);

		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DESCR, 0, ebufp);
        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_ADDRESS, 0, ebufp);
        	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ADDRESS, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_GEOCODE, 1, ebufp);
		if(vp != NULL){
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_GEOCODE, vp, ebufp);

			tax_pkg = (int*)PIN_FLIST_FLD_GET(flistp, PIN_FLD_TAXPKG_TYPE, 1, ebufp);
			if(tax_pkg != NULL){
				PIN_FLIST_FLD_SET(r_flistp,PIN_FLD_TAXPKG_TYPE,tax_pkg,ebufp);
			}
		}
	}

ErrOut:

	return;
}
