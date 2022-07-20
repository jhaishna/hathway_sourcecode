/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 * Version 1.0: 08-10-2013: HarishKulkarni: Added the MSO Customization of
 * Refund Processing using MSO_OP_UTILS_CREATE_LIFECYCLE_EVENT opcode and refund object creation
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_utils_create_lifecycle_event.c /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_UTILS_CREATE_LIFECYCLE_EVENT operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
#include <time.h>
#include <math.h>
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_rate.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_ar.h"
#include "ops/bill.h"
#include "ops/bal.h"
#include "mso_ops_flds.h" 
#include "pin_currency.h"
#include "mso_lifecycle_id.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_mso_utils_create_lifecycle_event(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

static void fm_mso_utils_create_lifecycle_event(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);


extern void
fm_mso_utils_gen_lifecycle_evt(
        pcm_context_t   *ctxp,
        poid_t          *acct_pdp,
        poid_t          *serv_pdp,
        char            *program,
        char            *descr,
        char            *name,
        char            *event_type,
        pin_flist_t     *in_flistp,
        pin_errbuf_t    *ebufp);



/*******************************************************************
 * Main routine for the MSO_OP_UTILS_CREATE_LIFECYCLE_EVENT operation.
 *******************************************************************/
void
op_mso_utils_create_lifecycle_event(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	int			status = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_UTILS_CREATE_LIFECYCLE_EVENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_utils_create_lifecycle_event opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_utils_create_lifecycle_event input flist", i_flistp);

	/***********************************************************
	 * Call the routine to process refund.
	 ***********************************************************/
	fm_mso_utils_create_lifecycle_event(ctxp, i_flistp, &r_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unknown Error occured in MSO_OP_UTILS_CREATE_LIFECYCLE_EVENT_INITIATE.");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"Unknown Error occured in MSO_OP_UTILS_CREATE_LIFECYCLE_EVENT_INITIATE.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	*o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_utils_create_lifecycle_event return flist", *o_flistpp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	return;
}

void fm_mso_utils_create_lifecycle_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*lc_in_flist = NULL;

	char                    *event = "/event/activity/mso_lifecycle/misc";

	poid_t			*acct_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*b_pdp = NULL;
	char			*msg = NULL;
	int			status = 0;
	pin_decimal_t		*amount = NULL;
	char			*action_name= NULL;
	char			*prog_name = NULL;

	PIN_ERR_LOG_MSG(3,"entered 1");

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Unknown input error!!");
		PIN_ERR_LOG_MSG(3,"entered 1");
		goto CLEANUP;
	}

	PIN_ERR_LOG_MSG(3,"entered 3");
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_utils_create_lifecycle_event input",i_flistp);

	lc_in_flist = PIN_FLIST_CREATE(ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,lc_in_flist, PIN_FLD_POID, ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ,lc_in_flist, PIN_FLD_SERVICE_OBJ, ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, lc_in_flist, PIN_FLD_USERID, ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, lc_in_flist, PIN_FLD_PROGRAM_NAME, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Calling lifecycle event create",lc_in_flist);

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_POID,1,ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_SERVICE_OBJ,1,ebufp);
	action_name = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_ACTION,1,ebufp);
	msg = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_DESCR,1,ebufp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_PROGRAM_NAME,1,ebufp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name, msg, action_name, event, lc_in_flist, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;
	}

	sprintf(msg, "Lifecycle event creation done!!");
	err_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, msg, ebufp);
	*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return flist is:",*r_flistpp);

	CLEANUP:
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			status = 1;
			PIN_ERRBUF_CLEAR(ebufp);
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, msg);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53022", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		}
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&lc_in_flist, NULL);   
		
	
	return;
}
