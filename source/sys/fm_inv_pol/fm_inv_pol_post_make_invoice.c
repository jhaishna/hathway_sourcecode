/*******************************************************************
 *
 *      Copyright (c) 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static	char	Sccs_id[] = "@(#)%Portal Version: fm_inv_pol_post_make_invoice.c:BillingVelocityInt:2:2006-Sep-05 04:27:08 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_INV_POL_POST_MAKE_INVOICE operation. 
 *******************************************************************/


#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/inv.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

extern char		*service_type;
/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_inv_pol_post_make_invoice(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_inv_pol_post_make_invoice(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
mso_inv_prepare_input_for_doc_reference(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_INV_POL_POST_MAKE_INVOICE operation.
 *******************************************************************/
void
op_inv_pol_post_make_invoice(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;


	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Invalid call
	 ***********************************************************/
	if (opcode != PCM_OP_INV_POL_POST_MAKE_INVOICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_post_make_invoice opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_inv_pol_post_make_invoice input flist", i_flistp);

	/***********************************************************
	 * Implementation
	 ***********************************************************/
	fm_inv_pol_post_make_invoice(ctxp, flags, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log errors
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_post_make_invoice error", ebufp);

		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

		*o_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		*o_flistpp = r_flistp;

		/***************************************************
		 * Return flist log
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_inv_pol_post_make_invoice return flist", r_flistp);

	}

	return;
}

/*******************************************************************
 * fm_inv_pol_post_make_invoice():
 *
 *******************************************************************/
static void
fm_inv_pol_post_make_invoice(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Prepare the return flist.
	 ***********************************************************/
	*ret_flistpp = (pin_flist_t *)NULL;

	r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	if (service_type != NULL && strcmp(service_type, "BB") == 0){
		mso_inv_prepare_input_for_doc_reference(ctxp, i_flistp, ebufp);
	}

	/***********************************************************
	 * Empty policy
	 ***********************************************************/

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_inv_pol_post_make_invoice: error", ebufp);
	}else {
		*ret_flistpp = r_flistp;
	}

	return;
}


static void
mso_inv_prepare_input_for_doc_reference(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp)
{

	void			*i_pdp = NULL;
	char			*poid_type = NULL;



	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_inv_prepare_input_for_doc_reference start");

	
	i_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	poid_type = (char *)PIN_POID_GET_TYPE(i_pdp);		
	if (poid_type && (strcmp(poid_type, "/invoice") != 0)){
		goto cleanup;
	}	
	mso_op_create_inv_doc_reference(ctxp, in_flistp, ebufp);
	

cleanup:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_inv_prepare_input_for_doc_reference: end");	

}

