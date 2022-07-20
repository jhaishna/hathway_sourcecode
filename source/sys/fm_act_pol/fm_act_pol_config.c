/*******************************************************************
 *
* Copyright (c) 2007, 2014, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_act_pol_custom_config.c:CUPmod7.3PatchInt:1:2007-Jan-10 20:07:40 %";
#endif

#include <stdio.h>	/* for FILE * in pcm.h */
#include "ops/act.h"
#include "pcm.h"
#include "cm_fm.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_act_pol_custom_config_func();
#endif


/*******************************************************************
 *******************************************************************/

    /*
     * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
     * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
     * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
     */

struct cm_fm_config fm_act_pol_custom_config[] = {
	/* opcode as a u_int, function name (as a string) */
/*
	{ PCM_OP_ACT_POL_EVENT_LIMIT,	"op_act_pol_event_limit" },
	{ PCM_OP_ACT_POL_EVENT_NOTIFY,	"op_act_pol_event_notify" }, 
	{ PCM_OP_ACT_POL_SPEC_VERIFY,	"op_act_pol_spec_verify" }, 
	{ PCM_OP_ACT_POL_SPEC_RATES,	"op_act_pol_spec_rates" }, 
*/
	{ PCM_OP_ACT_POL_SPEC_EVENT_CACHE, "op_act_pol_spec_event_cache", CM_FM_OP_OVERRIDABLE }, 
	{ PCM_OP_ACT_POL_SPEC_GLID,	"op_act_pol_spec_glid", CM_FM_OP_OVERRIDABLE }, 
/*
	{ PCM_OP_ACT_POL_VALIDATE_SCHEDULE, "op_act_pol_validate_schedule" }, 
	{ PCM_OP_ACT_POL_SPEC_CANDIDATE_RUMS,"op_act_pol_spec_candidate_rums" }, 
	{ PCM_OP_ACT_POL_CONFIG_BILLING_CYCLE, "op_act_pol_config_billing_cycle" },
	{ PCM_OP_ACT_POL_PRE_AUTHORIZE, "op_act_pol_pre_authorize", CM_FM_OP_OVERRIDABLE },
	{ PCM_OP_ACT_POL_POST_AUTHORIZE, "op_act_pol_post_authorize" }, 
	{ PCM_OP_ACT_POL_PRE_REAUTHORIZE, "op_act_pol_pre_reauthorize", CM_FM_OP_OVERRIDABLE }, 
	{ PCM_OP_ACT_POL_POST_REAUTHORIZE, "op_act_pol_post_reauthorize" }, 
	{ PCM_OP_ACT_POL_SCALE_MULTI_RUM_QUANTITIES, "op_act_pol_scale_multi_rum_quantities" },
	{ PCM_OP_ACT_POL_LOCK_SERVICE, "op_act_pol_lock_service" }, 
	{ PCM_OP_ACT_POL_LOG_USER_ACTIVITY, "op_act_pol_log_user_activity" }, 
	{ PCM_OP_ACT_POL_SET_RESOURCE_STATUS, "op_act_pol_set_resource_status" },
*/
	{ 0,	(char *)0 }
};

#ifdef MSDOS
void *
fm_act_pol_custom_config_func()
{
  return ((void *) (fm_act_pol_custom_config));
}
#endif

