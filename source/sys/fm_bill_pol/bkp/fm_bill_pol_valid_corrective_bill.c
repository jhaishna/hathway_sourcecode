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
static char Sccs_id[] = "@(#)%full_filespec: fm_bill_pol_valid_corrective_bill.c;67FP2;071604;120181%";
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
#include "mso_ops_flds.h"

EXPORT_OP void
op_bill_pol_valid_corrective_bill(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_bill_pol_valid_corrective_bill(
	pcm_context_t     	*ctxp,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for PCM_OP_BILL_POL_VALID_CORRECTIVE_BILL  
 *******************************************************************/
void
op_bill_pol_valid_corrective_bill(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = (pin_flist_t *)NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_BILL_POL_VALID_CORRECTIVE_BILL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_bill_pol_valid_corrective_bill", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_valid_corrective_bill input flist", in_flistp);
	
	fm_bill_pol_valid_corrective_bill(ctxp, flags, in_flistp, ret_flistpp, ebufp);	

	/***********************************************************
	* Results.
	***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_valid_corrective_bill error", ebufp);
	} else {
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_bill_pol_valid_corrective_bill return flist", 
			*ret_flistpp);
	}

	return;
}

/************************************************************************
 * fm_bill_pol_valid_corrective_bill()
 ************************************************************************/
static  void
fm_bill_pol_valid_corrective_bill(
	pcm_context_t      	*ctxp,
	int			flags,
	pin_flist_t             *in_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t       	*ebufp)	
{
	poid_t                  *a_pdp 		= NULL;
	pin_flist_t             *inh_flistp 	= NULL;
	u_int                   result = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	*ret_flistpp	=  PIN_FLIST_CREATE(ebufp);

	*ret_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
	result |= PIN_BOOLEAN_TRUE;
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_RESULT, (void *)&result, ebufp);
	return;
}

