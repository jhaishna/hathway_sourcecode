/*
 *
 *      Copyright (c) 2004 - 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)%full_filespec: fm_bill_pol_reverse_payment.c;67FP2;071604;120181%";
#endif

#include <stdio.h>
#include <string.h>
#include "pcm.h"
#include "ops/bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_ar.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pinlog.h"
#include "psiu_business_params.h"

EXPORT_OP void
op_bill_pol_reverse_payment(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_bill_pol_reverse_payment(
	pcm_context_t     	*ctxp,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t            *ebufp);

static  void
fm_bill_pol_reverse_payment_writeoff_account(
	pcm_context_t     	*ctxp,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for PCM_OP_BILL_POL_REVERSE_PAYMENT  
 *******************************************************************/
void
op_bill_pol_reverse_payment(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = (pin_flist_t *)NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_BILL_POL_REVERSE_PAYMENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_bill_pol_reverse_payment", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_reverse_payment input flist", in_flistp);
	
	fm_bill_pol_reverse_payment(ctxp, flags, in_flistp, ret_flistpp, ebufp);	

	/***********************************************************
	* Results.
	***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_reverse_payment error", ebufp);
	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_bill_pol_reverse_payment return flist", 
			*ret_flistpp);
	}

	return;
}

/************************************************************************
 * fm_bill_pol_reverse_payment()
 ************************************************************************/
static  void
fm_bill_pol_reverse_payment(
	pcm_context_t      	*ctxp,
	int			flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t       	*ebufp)	
{
	poid_t                  *a_pdp 		= NULL;
	pin_flist_t             *inh_flistp 	= NULL;
	
	if (PIN_ERR_IS_ERR(ebufp)){
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	*ret_flistpp	=  PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Get and set PIN_FLD_POID in the output flist
	 ***********************************************************/
	a_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, (void *)a_pdp, ebufp);

	/***********************************************************
	 * Get INHERITED_INFO from the input flist and
	 * Set in the ouput flist if present
	 ***********************************************************/
	inh_flistp = PIN_FLIST_SUBSTR_GET(in_flistp,
				PIN_FLD_INHERITED_INFO, 1, ebufp);
	if ( inh_flistp ) {
		PIN_FLIST_SUBSTR_SET(*ret_flistpp, inh_flistp, 
				PIN_FLD_INHERITED_INFO, ebufp);
	}

	fm_bill_pol_reverse_payment_writeoff_account(ctxp, flags, in_flistp,
						 ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_bill_pol_reverse_payment error", ebufp);
	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
	}
	return;
}

/************************************************************************
 * fm_bill_pol_reverse_payment_writeoff_account()
 * If the payment on a written off account is reversed, set the appropriate 
 * reason code and write off the account
 * This function can be modified to set the tax flag 
 * to create seperate tax event for write off 
 * ( PIN_FLD_FLAGS = PIN_FLD_FLAGS | PIN_BILL_WRITEOFF_TAX ) 
 ************************************************************************/
static void 
fm_bill_pol_reverse_payment_writeoff_account(
	pcm_context_t     	*ctxp,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_errbuf_t       	*ebufp)	
{
	pin_pymt_result_t	*status_flag 		= NULL;
	char			writeoff_rev_level[8] 	= { "" };
	pin_flist_t		*i_flistp		= NULL;
	pin_flist_t		*r_flistp		= NULL;
	void 			*vp 			= NULL ;
	int			glid_temp		= 0;

	if (PIN_ERR_IS_ERR(ebufp)){
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/******************************************************************
	 * Get Flag from input flist which determines if WOR feature is 
	 * enabled and Payment reversal is for payment applied to 
	 * written-off account
	 ******************************************************************/
	status_flag = (pin_pymt_result_t *) PIN_FLIST_FLD_GET(in_flistp, 
				PIN_FLD_STATUS, 1, ebufp); 

 	/**************************************************************
	 * Verify if write off level set to "account" (account) in 
	 * /config/business_params and Call PCM_OP_AR_ACCOUNT_WRITEOFF
	 **************************************************************/

	psiu_bparams_get_str(ctxp, PSIU_BPARAMS_AR_PARAMS, 
		PSIU_BPARAMS_AR_WRITEOFF_LEVEL, writeoff_rev_level, 8, ebufp); 

	if ( status_flag && ( *status_flag == PIN_PYMT_WRITEOFF_SUCCESS ) && 
		writeoff_rev_level && 
		!strcmp(writeoff_rev_level, PIN_WRITEOFF_REV_LEVEL_ACCOUNT) ){ 

		i_flistp = PIN_FLIST_CREATE(ebufp);

		vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, vp, ebufp);
	
		vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PROGRAM_NAME, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_START_T, 1, ebufp);
		if (vp) {
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_START_T,
							(void *) vp, ebufp);
		}

		vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_END_T, 1, ebufp);
		if (vp) {
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_END_T,
							(void *) vp, ebufp);
		}

		vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DESCR, 1, ebufp);
		if (vp) {
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DESCR,
							(void *) vp, ebufp);
		}

		/**********************************************************
		 * Set default Reason id & Reason domain id. 
		 **********************************************************/
		glid_temp = PIN_WRITEOFF_FOR_PAYMENT_REVERSAL ;

		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_STRING_ID,(void *)&glid_temp, ebufp);

		glid_temp = PIN_REASON_CODES_CREDIT_REASONS_AUTO_WRITEOFF ;

		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_STR_VERSION,(void *)&glid_temp, ebufp);

		/**************************************************************
		 * Call PCM_OP_AR_ACCOUNT_WRITEOFF
		 **************************************************************/

		PCM_OP(ctxp, PCM_OP_AR_ACCOUNT_WRITEOFF, flags, i_flistp,
                                &r_flistp, ebufp);

		/***********************************************************
		 * Clean up.
		 ***********************************************************/
		PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	}

	/**********************************************
	 * Error?
	 **********************************************/

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_reverse_payment_writeoff_account error", 
			ebufp);
	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
	}
	return;
}
