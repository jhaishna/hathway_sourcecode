/* continuus file information --- %full_filespec:  fm_cust_pol_post_modify_customer.c~2:csrc:1 % */
/*******************************************************************
 *
 *   @(#) %full_filespec: fm_cust_pol_post_modify_customer.c~2:csrc:1 %
 *
 *      Copyright (c) 2000-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *  
 *******************************************************************/

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_post_modify_customer(
        cm_nap_connection_t	*connp,
		int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_POST_MODIFY_CUSTOMER command
 *******************************************************************/
void
op_cust_pol_post_modify_customer(
        cm_nap_connection_t	*connp,
		int32			opcode,
        int32			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	void *vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_POST_MODIFY_CUSTOMER) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_post_modify_customer", ebufp);
		return;
	}

	/***********************************************************
	 * Debug what we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_post_modify_customer input flist", in_flistp);

	*ret_flistpp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, vp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY(*ret_flistpp, NULL);
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_post_modify_customer error", ebufp);
	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_post_modify_customer return flist", 
			*ret_flistpp);
	}

	return;
}

