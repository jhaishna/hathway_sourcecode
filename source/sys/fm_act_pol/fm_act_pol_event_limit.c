/**
 *
 * Copyright (c) 1999, 2009, Oracle and/or its affiliates. All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 **/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_act_pol_event_limit.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:15:26 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_ACT_POL_EVENT_LIMIT operation. 
 *******************************************************************/

/*******************************************************************
 *******************************************************************/
	/***********************************************************
	 ***********************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/cust.h"
#include "ops/act.h"
#include "pin_act.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#define FILE_SOURCE_ID  "fm_act_pol_event_limit.c(1.4)"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_act_pol_event_limit(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp);

void fm_act_pol_handle_po_limits(
	pcm_context_t		*ctxp,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp);


/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_ACT_POL_EVENT_LIMIT operation.
 *******************************************************************/
void
op_act_pol_event_limit(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	poid_t			*e_pdp = NULL;
	const char			*poid_type = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_ACT_POL_EVENT_LIMIT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_event_limit opcode error", ebufp);
		return;
	}
	/***********************************************************
	 * Debut what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_act_pol_event_limit input flist", i_flistp);

	/***********************************************************
	 * Action depends on the type of event.  List events here
	 * and then branch to appropriate handler.
	 ***********************************************************/
	e_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	poid_type = PIN_POID_GET_TYPE(e_pdp);

	/***********************************************************
	 * FIX THIS: To really do this right, we should qualify it
	 *	with a resource id (100001 for POs).
	 ***********************************************************/
	if ((strcmp(poid_type, PIN_OBJ_TYPE_EVENT_CREDIT_LIMIT) == 0) ||
	    (strcmp(poid_type, PIN_OBJ_TYPE_EVENT_DATE_EXPIRE) == 0)) {
		fm_act_pol_handle_po_limits(ctxp, flags, i_flistp, 
			r_flistpp, ebufp);
	}
	
	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		*r_flistpp = (pin_flist_t *)NULL;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_event_limit error", ebufp);

	} else {

	/***********************************************************
	 * Output flist.
	 ***********************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_act_pol_event_limit output flist", *r_flistpp);

	}
	return;
}

/*******************************************************************
 * fm_act_pol_handle_po_limits()
 *	
 * 	Routine that calls PCM_OP_CUST_SET_STATUS if options
 *	is to inactivate account.  PCM_OP_ACT_POL_EVENT_NOTIFY
 *	is called to notify account of action.
 *******************************************************************/
void
fm_act_pol_handle_po_limits(
	pcm_context_t		*ctxp,
	u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp)
{
        pin_flist_t		*flistp = NULL;
        pin_flist_t		*e_flistp = NULL;
        pin_flist_t		*in_flistp = NULL;
        pin_flist_t		*inh_flistp = NULL;
        pin_flist_t		*r_flistp = NULL;
	u_int			*flagsp = 0;
	u_int			dummy = 0;
	void			*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Find out if we inactivating the account.
	 ***********************************************************/
	inh_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_INHERITED_INFO,
		0, ebufp);
	flagsp = (poid_t *)PIN_FLIST_FLD_GET(inh_flistp, 
		PIN_FLD_PO_OPTIONS, 0, ebufp);

	/***********************************************************
	 * Prepare to inactivate the account if answer was yes
	 ***********************************************************/
	if (flagsp && *flagsp) {
		in_flistp = PIN_FLIST_CREATE(ebufp);

		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, vp, ebufp);

		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 
			0, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, vp, ebufp);

		e_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_STATUSES,
			0, ebufp);

		dummy = PIN_STATUS_INACTIVE;
		PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_STATUS, (void *)&dummy, 
			ebufp);
		
		dummy = PIN_STATUS_FLAG_PO_EXHAUSTED;
		PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_STATUS_FLAGS, 
			(void *)&dummy, ebufp);

		PCM_OP(ctxp, PCM_OP_CUST_SET_STATUS, flags, in_flistp,
                        &r_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	} 

	/***********************************************************
	 * For now, don't do anything with the return flists.
	 ***********************************************************/
	PCM_OP(ctxp, PCM_OP_ACT_POL_EVENT_NOTIFY, flags, i_flistp,
                        &r_flistp, ebufp);
	
	/***********************************************************
	 * Prepare to notify account owner.
	 ***********************************************************/
	*r_flistpp = r_flistp;

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		*r_flistpp = (pin_flist_t *)NULL;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_act_pol_handle_po_limits error", ebufp);
	}
	return;
}
