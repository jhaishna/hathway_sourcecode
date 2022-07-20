/*
 *
* Copyright (c) 2004, 2013, Oracle and/or its affiliates. All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_pymt_pol_suspend_payment.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:45:58 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_PYMT_POL_SUSPEND_PAYMENT operation. 
 *******************************************************************/

/*******************************************************************
*******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_rate.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_pymt_pol_suspend_payment(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void 
fm_pymt_pol_suspend_payment(
	pcm_context_t   	*ctxp,
	int 			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_SUSPEND_PAYMENT operation.
 *******************************************************************/
void
op_pymt_pol_suspend_payment(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	void 			*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
	{
		return ;
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_SUSPEND_PAYMENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_suspend_payment opcode error", ebufp);
		return;
	}
	
	/***********************************************************
	 * Debug: The input Flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_suspend_payment input flist", i_flistp);

	/***********************************************************
	 * Prep the return flist.
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Get the poid from the Input Flist.
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, vp, ebufp);
	fm_pymt_pol_suspend_payment(ctxp, flags, i_flistp, r_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		/***************************************************
		 * Log Error Buuffer and return.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_suspend_payment error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;

	} 
	else
	{

		*o_flistpp = r_flistp;
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_pymt_pol_suspend_payment return flist", r_flistp);
	}

	return;
}

/********************************************************************
 * fm_pymt_pol_suspend_payment()
 ********************************************************************/
static void 
fm_pymt_pol_suspend_payment(
	pcm_context_t           *ctxp,
	int			flags,
	pin_flist_t 		*i_flistp,
	pin_flist_t 		*r_flistp,
	pin_errbuf_t            *ebufp)
{
	int             err = PIN_ERR_NONE;
	poid_t		*dummy_pdp = NULL;
	poid_t		*s_pdp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*a_flistp = NULL;
	pin_flist_t	*acc_flistp = NULL;
	int64		database=1;
	char		template[124];
	int32		flag = SRCH_EXACT;
	void		*vp = NULL;
	pin_cookie_t	cookie = NULL;
	int32		elem_id = 0;	

	if (PIN_ERR_IS_ERR(ebufp))
	{
		return ;
	}
        PIN_ERR_CLEAR_ERR(ebufp);

	dummy_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)){
                /***************************************************
                 * Log Error Buffer and return.
                 ***************************************************/
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_pymt_pol_suspend_payment error", ebufp);
        }
	database = PIN_POID_GET_DB(dummy_pdp);

	/******************************************
	 * Create Search Flist
	 ******************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp =	PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	strcpy(template, "");
	strcpy(template, " select X from /config/psm where F1 like V1 " );
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, (void *)template, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *) &flag, ebufp);

	a_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	dummy_pdp = PIN_POID_CREATE(database, "/config/psm", -1, ebufp);
	PIN_FLIST_FLD_PUT(a_flistp, PIN_FLD_POID, (void *) dummy_pdp, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	a_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_ACCOUNTS, -1, ebufp);
	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);	

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	s_flistp = NULL;
	s_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS,
				0, 0, ebufp);
	/*********************************************
	 * Check if no results are returned. Display
	 * Error since no PSM accounts are configure
	 *********************************************/
	if(!s_flistp){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
			"No Suspense Account Configured");
			
	}
	/*********************************************
	 * Add the search result to output flist
	 *********************************************/
	a_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_RESULTS, 0, ebufp);

	/**************************************
	 * Get the Account Poid from the result
	 **************************************/
	while (acc_flistp = PIN_FLIST_ELEM_GET_NEXT(s_flistp, PIN_FLD_ACCOUNTS,
				&elem_id, 1, &cookie, ebufp))
        {
                vp = (poid_t *)PIN_FLIST_FLD_GET(acc_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
                if (database == PIN_POID_GET_DB(vp)) {
                        PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
                        break;
                }
        }

	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)){
		/***************************************************
		 * Log Error Buffer and return.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_suspend_payment error", ebufp);
	}
	return ;
}
