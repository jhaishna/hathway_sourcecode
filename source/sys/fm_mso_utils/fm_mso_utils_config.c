/*******************************************************************************
 * Copyright (c) 2008, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *	This material is the confidential property of Oracle Corporation
 *	or its licensors and may be used, reproduced, stored or transmitted
 *	only in accordance with a valid Oracle license or sublicense agreement.
 ********************************************************************************/

#ifndef lint
static  char  Sccs_id[] = "@(#)$Id: fm_mso_utils_custom_config.c /cgbubrm_7.3.2.rwsmod/2 2009/11/03 05:14:06 sdganta Exp $";
#endif

#include <stdio.h>      /* for FILE * in pcm.h */
#include "pcm.h"
#include "cm_fm.h"
#include "mso_ops_flds.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_mso_utils_custom_config_func();
#endif

/*******************************************************************
 * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
 * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
 * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
 *******************************************************************/

struct cm_fm_config fm_mso_utils_custom_config[] = {
	/* opcode as a u_int, function name (as a string) */

	{ MSO_OP_UTILS_ADD_HINT_BILLING,    "op_mso_utils_add_hint_billing" },
	{ MSO_OP_UTILS_MTA_CONFIG_BILLING,    "op_mso_utils_mta_config_billing" },
	{ MSO_OP_UTILS_CREATE_LIFECYCLE_EVENT, "op_mso_utils_create_lifecycle_event" },
	{ 0,    (char *)0 }
};

#ifdef MSDOS
void *
fm_mso_utils_custom_config_func()
{
  return ((void *) (fm_mso_utils_custom_config));
}
#endif
