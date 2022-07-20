/*******************************************************************
 *
 * Copyright (c) 1999, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_prep_login.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/timeb.h>
#include "pin_sys.h"
#include "pin_os.h"
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"

EXPORT_OP void
op_cust_pol_prep_login(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_prep_login(
        cm_nap_connection_t	*connp,
	pin_flist_t		*in_flistp,
        int32			flags,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_PREP_LOGIN  command
 *******************************************************************/
void
op_cust_pol_prep_login(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_PREP_LOGIN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_prep_login", ebufp);
		return;
	}

	/***********************************************************
	 * We will not open any transactions with Policy FM
	 * since policies should NEVER modify the database.
	 ***********************************************************/

	/***********************************************************
	 * Log input flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_prep_login input flist", in_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_prep_login(connp, in_flistp, flags, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_prep_login error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_prep_login return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_prep_login()
 *
 *	Prep the login to be ready for on-line registration.
 *
 *	XXX HARDCODE - STUBBED PROTOTYPE ONLY XXX
 *
 *******************************************************************/
static void
fm_cust_pol_prep_login(
        cm_nap_connection_t	*connp,
	pin_flist_t		*in_flistp,
	int32                   flags,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	poid_t			*o_pdp = NULL;
	char			*domain = NULL;
	char			*login = NULL;
	const char		*type = NULL;
	char			*name = NULL;
	char			logout[512];
	char                    generated_login[255];
        pin_cookie_t            cookie = NULL;
        pin_flist_t             *flistp = NULL;
        int32                   elemid;
	int32			err;
	int32			*flag = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Prep outgoing flist
	 ***********************************************************/
	*out_flistpp = PIN_FLIST_CREATE(ebufp);

	o_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, (void *)o_pdp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_login error", ebufp);
		return;
	}

	/***********************************************************
	 * Now for the login.
	 ***********************************************************/
	login = (char *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_LOGIN, 1, ebufp);
	
	type = PIN_POID_GET_TYPE(o_pdp);

	/* Just keep what we have */
	if ( login ) {
		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_LOGIN,
				(void *)login, ebufp);
	}

	/**************************************************************
	 * Check for NULL login. If login is not found in the input,
	 * then set it as an empty string so that it will fail at
	 * PCM_OP_CUST_POL_VALID_LOGIN opcode. We can't have a service 
	 * with NULL or empty login as it may cause security breach. 
	 **************************************************************/
//	if (!login && fm_utils_op_is_ancestor(
//                connp->coip, PCM_OP_CUST_CREATE_SERVICE)) {
//		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_LOGIN,
//				(void *)"", ebufp);
//	}
	
	cookie = NULL;
	for( 
	    flistp = PIN_FLIST_ELEM_GET_NEXT( in_flistp, 
		PIN_FLD_ALIAS_LIST, &elemid, 0, &cookie, ebufp );
	    ebufp->pin_err == PIN_ERR_NONE;
	    flistp = PIN_FLIST_ELEM_GET_NEXT( in_flistp, 
		PIN_FLD_ALIAS_LIST, &elemid, 0, &cookie, ebufp ) 
	) {

		PIN_FLIST_ELEM_SET( *out_flistpp, flistp, 
				PIN_FLD_ALIAS_LIST, elemid, ebufp );
	}
	if ( ebufp->pin_err == PIN_ERR_NOT_FOUND ) {
		PIN_ERRBUF_CLEAR( ebufp );
	}


	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_login error", ebufp);
	}

	return;
}



