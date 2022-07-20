/*******************************************************************
 *
 * Copyright (c) 2007, 2009,2014 Oracle and/or its affiliates.All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_rate_aaa_config.c:CUPmod7.3PatchInt:1:2007-Jan-10 20:07:40 %";
#endif

#include <stdio.h>	/* for FILE * in pcm.h */
#include "ops/act.h"
#include "pcm.h"
#include "cm_fm.h"
#include "mso_ops_flds.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_mso_rate_aaa_config_func();
#endif


/*******************************************************************
 *******************************************************************/

    /*
     * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
     * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
     * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
     */

struct cm_fm_config fm_mso_rate_aaa_config[] = {
	/* opcode as a u_int, function name (as a string) */
	{ MSO_OP_RATE_AAA_RATE_BB_EVENT,			"op_mso_rate_aaa_rate_bb_event" },
	{ MSO_OP_RATE_AAA_CREATE_SUSP_EVENT,			"op_mso_rate_aaa_create_susp_event" },
	{ MSO_OP_RATE_AAA_SEARCH_SUSP_EVENT,			"op_mso_rate_aaa_search_susp_event" },
	{ MSO_OP_RATE_AAA_SEARCH_ACCOUNT,			"op_mso_rate_aaa_search_account" },
	{ MSO_OP_RATE_AAA_RESUBMIT_SUSPENSE_EVENT,		"op_mso_rate_aaa_resubmit_suspense_event" },
	{ MSO_OP_RATE_AAA_GET_DAILY_USAGE_DETAILS,		"op_mso_rate_aaa_get_daily_usage_details" },
	{ MSO_OP_RATE_AAA_AUTHORIZE_PREP_INPUT,			"op_mso_rate_aaa_authorize_prep_input" },
	{ MSO_OP_RATE_AAA_REAUTHORIZE_PREP_INPUT,		"op_mso_rate_aaa_reauthorize_prep_input" },
	{ MSO_OP_RATE_AAA_STOP_ACCOUNTING_PREP_INPUT,		"op_mso_rate_aaa_stop_accounting_prep_input" },
	{ 0,	(char *)0 }
};

#ifdef MSDOS
void *
fm_mso_act_config_func()
{
  return ((void *) (fm_mso_rate_aaa_config));
}
#endif

