/*
 * @(#)%Portal Version: fm_subscription_pol_custom_config.c:CUPmod7.3PatchInt:1:2006-Dec-14 00:38:03 %	
 *
* Copyright (c) 2006, 2012, Oracle and/or its affiliates. All rights reserved. 
 *      
 *	This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_subscription_pol_custom_config.c:CUPmod7.3PatchInt:1:2006-Dec-14 00:38:03 % ";
#endif

#include <stdio.h>      /* for FILE * in pcm.h */
#include "ops/bill.h"
#include "ops/subscription.h"
#include "pcm.h"
#include "cm_fm.h"

#ifndef PCM_OP_SUBSCRIPTION_POL_CONFIG_EET
#define PCM_OP_SUBSCRIPTION_POL_CONFIG_EET 426
#endif

#ifdef MSDOS
__declspec(dllexport) void * fm_subscription_pol_custom_config_func();
#endif

struct cm_fm_config fm_subscription_pol_custom_config[] = {
        /* opcode as a u_int, function name (as a string) */

	/*****************************************************************************
	 * If you want to customize any of the op-codes commented below, you need to *
	 * uncomment it. The op-codes that have the op-code name "XXXXX" need to be  *
	 * writen if you want to customize those op-codes.			     *
	 *****************************************************************************/

	{ PCM_OP_SUBSCRIPTION_POL_PRE_FOLD,             
		(char *)"op_subscription_pol_pre_fold" },
	{ PCM_OP_SUBSCRIPTION_POL_PREP_FOLD,             
		(char *)"op_subscription_pol_prep_fold" },
	{ PCM_OP_SUBSCRIPTION_POL_PURCHASE_PROD_PROVISIONING,
		(char *)"op_subscription_pol_purchase_prod_provisioning" },
	{ PCM_OP_SUBSCRIPTION_POL_CANCEL_PROD_PROVISIONING,
		(char *)"op_subscription_pol_cancel_prod_provisioning" },
	{ PCM_OP_SUBSCRIPTION_POL_GET_PROD_PROVISIONING_TAGS,
		(char *)"op_subscription_pol_get_prod_provisioning_tags" },
	{ PCM_OP_SUBSCRIPTION_POL_SPEC_CYCLE_FEE_INTERVAL,
		(char *)"op_subscription_pol_spec_cycle_fee_interval" },
	{ PCM_OP_SUBSCRIPTION_POL_SPEC_CANCEL, 
		(char *)"op_subscription_pol_spec_cancel" },
	{ PCM_OP_SUBSCRIPTION_POL_SPEC_CANCEL_DISCOUNT, 
		(char *)"op_subscription_pol_spec_cancel_discount" },
	{ PCM_OP_SUBSCRIPTION_POL_GET_SPONSORS, 
		(char *)"op_subscription_pol_get_sponsors" },
	{ PCM_OP_SUBSCRIPTION_POL_SPEC_FOLD, 
		(char *)"op_subscription_pol_spec_fold" },
	{ PCM_OP_SUBSCRIPTION_POL_SNOWBALL_DISCOUNT, 
		(char *)"op_subscription_pol_snowball_discount" },
	{ PCM_OP_SUBSCRIPTION_POL_SPEC_RERATE,
		(char *)"op_subscription_pol_spec_rerate" },
	{ PCM_OP_SUBSCRIPTION_POL_CONFIG_EET,
		(char *)"op_subscription_pol_config_eet" },
	{ PCM_OP_SUBSCRIPTION_POL_PRE_TRANSITION_DEAL,
		(char *)"op_subscription_pol_pre_transition_deal" },
	{ PCM_OP_SUBSCRIPTION_POL_PRE_TRANSITION_PLAN,
		(char *)"op_subscription_pol_pre_transition_plan" },
	{ PCM_OP_SUBSCRIPTION_POL_PREP_MEMBERS,
		(char *)"op_subscription_pol_prep_members" },
	{ PCM_OP_SUBSCRIPTION_POL_COUNT_LINES,
		(char *)"op_subscription_pol_count_lines" },
	{ PCM_OP_SUBSCRIPTION_POL_NOTIFY_AGGREGATION,
		(char *)"op_subscription_pol_notify_aggregation" },
	{ PCM_OP_SUBSCRIPTION_POL_UPDATE_CDC,
		(char *)"op_subscription_pol_update_cdc" },
/*	{ PCM_OP_SUBSCRIPTION_POL_AUTO_SUBSCRIBE_MEMBERS,
		(char *)"XXXXX" },
	{ PCM_OP_SUBSCRIPTION_POL_AUTO_SUBSCRIBE_SERVICE,
		(char *)"XXXXX" },
	{ PCM_OP_SUBSCRIPTION_POL_GENERATE_RERATE_REQUEST,
		(char *)"XXXXX" },
        { PCM_OP_SUBSCRIPTION_POL_POST_TRANSFER_SUBSCRIPTION,
                (char *)"op_subscription_pol_post_transfer_subscription" },  */
	{ 0,    (char *)0 } 	
};

#ifdef MSDOS
void *
fm_subscription_pol_custom_config_func()
{
  return ((void *) (fm_subscription_pol_custom_config));
}
#endif

