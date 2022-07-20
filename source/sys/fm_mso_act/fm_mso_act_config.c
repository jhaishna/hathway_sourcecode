/*******************************************************************
 *
 * Copyright (c) 2007, 2009, Oracle and/or its affiliates.All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_act_config.c:CUPmod7.3PatchInt:1:2007-Jan-10 20:07:40 %";
#endif

#include <stdio.h>	/* for FILE * in pcm.h */
#include "ops/act.h"
#include "pcm.h"
#include "cm_fm.h"
#include "mso_ops_flds.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_mso_act_config_func();
#endif


/*******************************************************************
 *******************************************************************/

    /*
     * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
     * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
     * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
     */

struct cm_fm_config fm_mso_act_config[] = {
	/* opcode as a u_int, function name (as a string) */
	{ MSO_OP_ACT_RATE_BB_EVENT,			"op_mso_act_rate_bb_event" },
	{ 0,	(char *)0 }
};

#ifdef MSDOS
void *
fm_mso_act_config_func()
{
  return ((void *) (fm_mso_act_config));
}
#endif

