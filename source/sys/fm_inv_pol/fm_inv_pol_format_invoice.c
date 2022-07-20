/*******************************************************************
 *
 * Copyright (c) 1999, 2009, Oracle and/or its affiliates.All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_inv_pol_format_invoice.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:40:52 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_INV_POL_FORMAT_INVOICE operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/inv.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#include "pin_inv.h"
#include "pin_bill.h"
#include "psiu_business_params.h"
#include "mso_ops_flds.h"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_inv_pol_format_invoice(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_inv_pol_format_invoice(
	cm_nap_connection_t	*connp,
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_INV_POL_FORMAT_INVOICE operation.
 *******************************************************************/
void
op_inv_pol_format_invoice(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_INV_POL_FORMAT_INVOICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_invoice opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_inv_pol_format_invoice input flist", i_flistp);



	/***********************************************************
	 * Fill in the actual results.
	 ***********************************************************/
	fm_inv_pol_format_invoice(connp, ctxp, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_inv_pol_format_invoice error", ebufp);
		if (r_flistp) PIN_FLIST_DESTROY(r_flistp, NULL);
		*o_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*o_flistpp = r_flistp;

		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_inv_pol_format_invoice return flist", r_flistp);

	}

	return;
}

/*******************************************************************
 * fm_inv_pol_format_invoice():
 *
 * In this function we determine which default formats we want
 * Portal to create in the PCM_OP_INV_MAKE_INVOICE opcode.
 *
 * Also if any customized formats are required, add the code
 * in this opcode. 
 *
 * NOTE: Anything that is on the input flist (i_flistp)
 *	 at this stage will be part of the invoice. The contents
 *	 of the input flist can be modified in the policy opcode
 *	 PCM_OP_INV_POL_PREP_INVOICE.
 *
 *******************************************************************/
static void
fm_inv_pol_format_invoice(
	cm_nap_connection_t	*connp,
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*invoice_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*stack_flistp=NULL;
	cm_op_info_t		*opstackp = connp->coip;
	int32			stack_opcode = 0;
	char			*trial = NULL;
	int32			invoice_storage_type = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Prepare the return flist.
	 ***********************************************************/
	*ret_flistpp = (pin_flist_t *)NULL;


	/***********************************************************
	 * Setup the invoice array flist.
	 * 
	 * This is the flist that will be returned to the opcode
	 * PCM_OP_INV_MAKE_INVOICE. The flist must have the 
	 * following structure:
	 *
	 * 	0 PIN_FLD_FORMATS 	ARRAY[0]
	 *	1   PIN_FLD_TYPE_STR	STR[0]
	 *	1   PIN_FLD_BUFFER	BUF[0]
	 *
	 * The PIN_FLD_BUFFER field is only set if customized 
	 * invoice formats are required.
	 *
	 * For flist format - FLIST (FM_INV_FORMAT_TYPE_PIN_FLIST):
	 * If the default 'stringified' FLIST invoice format is
	 * required, make sure the invoice_flistp is set up as
	 * follows:
	 *	0 PIN_FLD_FORMATS	ARRAY[*]
	 *	1   PIN_FLD_TYPE_STR	STR[0] "text/pin_flist"
	 *
	 * Default format - XML (FM_INV_FORMAT_TYPE_XML):
	 * If the default Portal XML invoice format is required,
	 * make sure there the invoice_flistp is set up as follows:
	 *
	 *	0 PIN_FLD_INVOICES	ARRAY[*]
	 *	1   PIN_FLD_TYPE_STR	STR[0] "text/xml"
	 *
	 * NOTE: The element can have any array position.
	 *
	 * NOTE: Portal only knows how to create the above 
	 * 	 mentioned invoice formats. For any other format 
	 *	 the PIN_FLD_BUF must be filled in this policy
	 * 	 opcode. 
	 ***********************************************************/

	invoice_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * NOTE:
	 * If the input_flistp is to be converted into the default
	 * XML format, add an array element to the invoice flist
	 * (invoice_flistp) with the field PIN_FLD_TYPE_STR set
	 * to "text/xml".
	 *
	 * If the default XML format is required include the code
	 * below that adds the second element. 
	 ***********************************************************/

	invoice_storage_type = psiu_bparams_get_int(ctxp,
				PSIU_BPARAMS_INVOICING_PARAMS,
				PSIU_BPARAMS_INVOICE_STORAGE_TYPE,
				ebufp);

	//Get the Parent opcode and it's input flist
	while(opstackp != NULL)
	{
		stack_opcode = opstackp->opcode;
		stack_flistp = opstackp->in_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_inv_pol_format_invoice stack input flist is ", stack_flistp);
		if(stack_opcode == MSO_OP_BILL_MAKE_BILL_NOW)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_inv_pol_format_invoice:Make Bill Now Input Flist ", stack_flistp);
			trial = (char *)PIN_FLIST_FLD_GET(stack_flistp, MSO_FLD_BILL_TYPE, 0, ebufp);
			if(trial && strcmp(trial, "trial") == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Trial Invoice");
				invoice_storage_type = 0;
			}
			break;
		}
		//counter++;		
		opstackp = opstackp->next;
	}

	if ( invoice_storage_type == 0) {
		tmp_flistp = PIN_FLIST_ELEM_ADD(invoice_flistp, 
				PIN_FLD_FORMATS, 0, ebufp);
	
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TYPE_STR, 
				(void *)FM_INV_FORMAT_TYPE_PIN_FLIST, ebufp);
	} else {
	
		tmp_flistp = PIN_FLIST_ELEM_ADD(invoice_flistp,
				 PIN_FLD_FORMATS, 0, ebufp);

		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TYPE_STR,
				(void *)FM_INV_FORMAT_TYPE_XML, ebufp);

	}


	/***********************************************************
	 * If any other invoice format provided with Portal  
	 * are required, such as DOC1 or HTML, include the function
	 * calls below. 
	 * 
	 * The functions below will add an array element to 
	 * PIN_FLD_FORMATS with the PIN_FLD_TYPE_STR and
	 * PIN_FLD_BUFFER set. 
	 *
	 * NOTE: 
	 * Customisations of the default Portal DOC1 or HTML
	 * formats can be undertaken by changing the underlying
	 * functions, which are located in fm_inv_pol_utils.c . 
	 *
	 * NOTE:
	 * The DOC1 and HTML related functions will become OBSOLETE
	 * in future Portal versions. 
	 *
	 ***********************************************************/


	/***********************************************************
	 * If any customized invoice formats are required add the
	 * function call here to call the function that does the
	 * conversion into the new format.
	 * 
	 * Make sure that if any new format is added onto the 
	 * invoice_flist and has the correct structure (see above).
	 ***********************************************************/


	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_format_invoice: error", ebufp);
	}


	/***********************************************************
	 * Assing the invoice array (invoice_flistp) to ret_flistpp. 
	 * The invoice_flistp MUST have at least one element on the
	 * PIN_FLD_FORMATS array. 
	 ***********************************************************/
	*ret_flistpp = invoice_flistp;

	return;
}


