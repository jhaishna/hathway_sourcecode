/*******************************************************************************
 * Copyright (c) 2008, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *	This material is the confidential property of Oracle Corporation
 *	or its licensors and may be used, reproduced, stored or transmitted
 *	only in accordance with a valid Oracle license or sublicense agreement.
 ********************************************************************************/

#ifndef lint
static  char  Sccs_id[] = "@(#)$Id: fm_mso_pymt_custom_config.c /cgbubrm_7.5 2013/10/20 05:14:06 vsabarik Exp $";
#endif

#include <stdio.h>      /* for FILE * in pcm.h */
#include "ops/pymt.h"
#include "pcm.h"
#include "cm_fm.h"
#include "mso_ops_flds.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_mso_pymt_custom_config_func();
#endif

/*******************************************************************
 * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
 * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
 * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
 *******************************************************************/

struct cm_fm_config fm_mso_pymt_custom_config[] = {
	/* opcode as a u_int, function name (as a string) */

	{ MSO_OP_PYMT_REVERSE_PAYMENT,		"mso_pymt_reverse_pymt", CM_FM_OP_OVERRIDABLE }, 
	{ MSO_OP_PYMT_COLLECT,	        	"mso_pymt_collect", CM_FM_OP_OVERRIDABLE }, 
	{ MSO_OP_PYMT_BULK_COLLECT, 		"mso_pymt_bulk_collect", CM_FM_OP_OVERRIDABLE },
	{ MSO_OP_PYMT_CREATE_DEPOSIT,		"mso_pymt_create_deposit", CM_FM_OP_OVERRIDABLE },
	{ MSO_OP_PYMT_DEPOSIT_REFUND,		"mso_pymt_deposit_refund", CM_FM_OP_OVERRIDABLE },
	{ MSO_OP_PYMT_GET_RECEIPTS,		"mso_pymt_get_receipts", CM_FM_OP_OVERRIDABLE },
	{ MSO_OP_PYMT_GET_DEPOSITS,             "mso_pymt_get_deposits", CM_FM_OP_OVERRIDABLE },
	{ MSO_OP_PYMT_LCO_COLLECT, 		"mso_pymt_lco_collect", CM_FM_OP_OVERRIDABLE },
	{ MSO_OP_PYMT_BULK_GET_PAYMENTS,	"mso_pymt_blk_get_payments", CM_FM_OP_OVERRIDABLE },
	{ MSO_OP_PYMT_LCO_COLLECT_UPDATE,	"mso_pymt_lco_collect_update", CM_FM_OP_OVERRIDABLE },
	{ MSO_OP_PYMT_ALLOCATE_PAYMENT,		"mso_pymt_allocate_payment", CM_FM_OP_OVERRIDABLE },
	{ 0,    (char *)0 }
};

#ifdef MSDOS
void *
fm_mso_pymt_custom_config_func()
{
  return ((void *) (fm_mso_pymt_custom_config));
}
#endif
