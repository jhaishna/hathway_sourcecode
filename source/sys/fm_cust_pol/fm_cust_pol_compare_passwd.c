/*******************************************************************
 *
* Copyright (c) 1999, 2013, Oracle and/or its affiliates. All rights reserved. 
 * 
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_compare_passwd.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 04:28:54 nishahan Exp $";
#endif

/*******************************************************************
 * This file contains the PCM_OP_CUST_POL_COMPARE_PASSWD operation. 
 *
 * The given clear text passwd is encrypted (by _ENCRYPT_PASSWD)
 * and compared the the given encrypted string.
 *
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_compare_passwd(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_pwd_boolean_result(	
	pin_flist_t		**out_flistpp,
	poid_t			*obj_pdp,
	u_int			bool_value,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_COMPARE_PASSWD operation.
 *******************************************************************/
void
op_cust_pol_compare_passwd(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;

	poid_t			*obj_pdp = NULL ;
	char			*pwd_given = NULL;
	char			*pwd_derived = NULL;

	u_int			boolean_value = PIN_BOOLEAN_FALSE;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_COMPARE_PASSWD) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_compare_passwd", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_compare_passwd input flist", in_flistp);

	/***********************************************************
	 * Get the obj poid from the input. (for results).
	 ***********************************************************/
	obj_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp,
		PIN_FLD_POID, 0, ebufp);

	/***********************************************************
	 * Get the encrypted string from the input.
	 ***********************************************************/
	pwd_given = (char *)PIN_FLIST_FLD_GET(in_flistp,
		PIN_FLD_PASSWD, 0, ebufp);

	/***********************************************************
	 * Set Flag value to 1, when called from compare  
	 **********************************************************/
	flags = 1;

	/***********************************************************
	 * Call _ENCRYPT_PASSWD to encrypt the given clear text.
	 ***********************************************************/
	opcode = PCM_OP_CUST_POL_ENCRYPT_PASSWD;
	PCM_OP(connp->dm_ctx, opcode, flags, in_flistp,	&r_flistp, ebufp);

	/***********************************************************
	 * Get the resulting enctrypted string.
	 ***********************************************************/
	pwd_derived = (char *)PIN_FLIST_FLD_GET(r_flistp,
		PIN_FLD_PASSWD, 0, ebufp);

	/***********************************************************
	 * Compare the strings.
	 ***********************************************************/
	if ((pwd_given != (char *)NULL) && (pwd_derived != (char *)NULL)) {

		if (strcmp(pwd_given, pwd_derived) == 0) {
			/* passwds match */
			boolean_value = PIN_BOOLEAN_TRUE;

		}

	}

	/***********************************************************
	 * Construct the results.
	 ***********************************************************/
	fm_pwd_boolean_result(ret_flistpp, obj_pdp, boolean_value, ebufp);

	/***********************************************************
	 * Clean up.
	 ***********************************************************/
	/* Free local memory. */
	PIN_FLIST_DESTROY(r_flistp, NULL);

	/* Error? */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_compare_passwd error", ebufp);
	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_compare_passwd ret flist", *ret_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_pwd_boolean_result():
 *
 *	Create a boolean return flist containing the given
 *	poid and boolean result. We don't actually look at
 *	the boolean value, but assume the caller knows
 *	what it is doing.
 *
 *******************************************************************/
static void
fm_pwd_boolean_result(
	pin_flist_t	**out_flistpp,
	poid_t		*obj_pdp,
	u_int		bool_value,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*flistp = NULL;


	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*out_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Alloc the flist.
	 ***********************************************************/
	flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Add the poid.
	 ***********************************************************/
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)obj_pdp, ebufp);

	/***********************************************************
	 * Add the boolean result.
	 ***********************************************************/
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_BOOLEAN, (void *)&bool_value, ebufp);

	/***********************************************************
	 * Check the error.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_LOG_ERR("bad set in fm_pwd_boolean_result", ebufp);
		PIN_FLIST_DESTROY(flistp, NULL);
		return;
	}

	/***********************************************************
	 * Set the results.
	 ***********************************************************/
	*out_flistpp = flistp;

	/***********************************************************
	 * No errors.
	 ***********************************************************/
	PIN_ERR_CLEAR_ERR(ebufp);

	return;
}

