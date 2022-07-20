/*******************************************************************************
* Copyright (c) 2008, 2013, Oracle and/or its affiliates. All rights reserved.
 *
 *	This material is the confidential property of Oracle Corporation
 *	or its licensors and may be used, reproduced, stored or transmitted
 *	only in accordance with a valid Oracle license or sublicense agreement.
 ********************************************************************************/

#ifndef lint
static  char  Sccs_id[] = "@(#)$Id: fm_pymt_pol_custom_config.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:45:32 nishahan Exp $";
#endif

#include <stdio.h>      /* for FILE * in pcm.h */
#include "ops/pymt.h"
#include "pcm.h"
#include "cm_fm.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_pymt_pol_custom_config_func();
#endif

/*******************************************************************
 * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
 * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
 * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
 *******************************************************************/

struct cm_fm_config fm_pymt_pol_custom_config[] = {
	/* opcode as a u_int, function name (as a string) */

/*	{ PCM_OP_PYMT_POL_VALID_VOUCHER,	"op_pymt_pol_valid_voucher" }, */
/*	{ PCM_OP_PYMT_POL_PURCHASE_DEAL,	"op_pymt_pol_purchase_deal" }, */
/*	{ PCM_OP_PYMT_POL_PRE_COLLECT,		"op_pymt_pol_pre_collect" }, */ 
/*	{ PCM_OP_PYMT_POL_COLLECT,		"op_pymt_pol_collect" }, */
/*	{ PCM_OP_PYMT_POL_VALIDATE,		"op_pymt_pol_validate" }, */
	{ PCM_OP_PYMT_POL_UNDER_PAYMENT,	"op_pymt_pol_under_payment" },
	{ PCM_OP_PYMT_POL_OVER_PAYMENT,		"op_pymt_pol_over_payment" }, 
/*	{ PCM_OP_PYMT_POL_SPEC_COLLECT,		"op_pymt_pol_spec_collect" }, 
	{ PCM_OP_PYMT_POL_SPEC_VALIDATE,	"op_pymt_pol_spec_validate" }, 
	{ PCM_OP_PYMT_POL_GRANT_INCENTIVE,	"op_pymt_pol_grant_incentive" }, 
	{ PCM_OP_PYMT_POL_APPLY_FEE,		"op_pymt_pol_apply_fee" }, 
	{ PCM_OP_PYMT_POL_CHARGE,		"op_pymt_pol_charge" }, 
	{ PCM_OP_PYMT_POL_VALIDATE_PAYMENT,	"op_pymt_pol_validate_payment" },
	{ PCM_OP_PYMT_POL_PROVISION_INCENTIVE,	"op_pymt_pol_provision_incentive" },
	{ PCM_OP_PYMT_POL_SUSPEND_PAYMENT,	"op_pymt_pol_suspend_payment" }, 
	{ PCM_OP_PYMT_POL_MBI_DISTRIBUTE,	"op_pymt_pol_mbi_distribute" }, 
     { PCM_OP_PYMT_POL_PRE_REFUND,       "op_pymt_pol_pre_refund" }, 
   { PCM_OP_PYMT_POL_POST_REFUND,       "op_pymt_pol_post_refund" }, */
	{ 0,    (char *)0 }
};

#ifdef MSDOS
void *
fm_pymt_pol_custom_config_func()
{
  return ((void *) (fm_pymt_pol_custom_config));
}
#endif
