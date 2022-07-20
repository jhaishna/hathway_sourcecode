/*******************************************************************************
 * Copyright (c) 2008, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *	This material is the confidential property of Oracle Corporation
 *	or its licensors and may be used, reproduced, stored or transmitted
 *	only in accordance with a valid Oracle license or sublicense agreement.
 ********************************************************************************/

#ifndef lint
static  char  Sccs_id[] = "@(#)$Id: fm_mso_billnow_config.c /cgbubrm_7.5 2013/10/20 05:14:06 vsabarik Exp $";
#endif

#include <stdio.h>      /* for FILE * in pcm.h */
#include "pcm.h"
#include "cm_fm.h"
#include "mso_ops_flds.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_mso_billnow_custom_config_func();
#endif

/*******************************************************************
 * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
 * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
 * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
 *******************************************************************/

struct cm_fm_config fm_mso_billnow_custom_config[] = {
	/* opcode as a u_int, function name (as a string) */

	{ MSO_OP_BILL_MAKE_BILL_NOW,   "mso_op_bill_make_bill_now", CM_FM_OP_OVERRIDABLE }, 
	{ 0,    (char *)0 }
};

#ifdef MSDOS
void *
fm_mso_billnow_custom_config_func()
{
  return ((void *) (fm_mso_billnow_custom_config));
}
#endif
