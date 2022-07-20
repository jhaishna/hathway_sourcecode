/*******************************************************************************
* Copyright (c) 2008, 2012, Oracle and/or its affiliates. All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 ********************************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)$Id: fm_collections_pol_custom_config.c /cgbubrm_1.0.0.collections/6 2012/01/17 08:53:14 praghura Exp $";
#endif

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#include "cm_fm.h"
#include "ops/collections.h"
#include "mso_ops_flds.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_collections_pol_custom_config_func();
#endif

/*******************************************************************
 * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
 * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
 * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
 *******************************************************************/

struct cm_fm_config fm_collections_pol_custom_config[] = {
	/* opcode as a int32, function name (as a string) */

 	{ PCM_OP_COLLECTIONS_POL_SELECT_PROFILE, 
		"op_collections_pol_select_profile", CM_FM_OP_OVERRIDABLE }, 

	{ PCM_OP_COLLECTIONS_POL_EXEC_POLICY_ACTION, 
		"op_collections_pol_exec_policy_action", CM_FM_OP_OVERRIDABLE }, 

/*	{ PCM_OP_COLLECTIONS_POL_PREP_DUNNING_DATA, 
		"op_collections_pol_prep_dunning_data" }, 

	{ PCM_OP_COLLECTIONS_POL_EXIT_SCENARIO, 
			"op_collections_pol_exit_scenario" }, 
*/
	{ PCM_OP_COLLECTIONS_POL_APPLY_LATE_FEES, 
		"op_collections_pol_apply_late_fees", CM_FM_OP_OVERRIDABLE }, 
/*
	{ PCM_OP_COLLECTIONS_POL_ASSIGN_AGENT, 
		"op_collections_pol_assign_agent" }, 

	{ PCM_OP_COLLECTIONS_POL_APPLY_FINANCE_CHARGES, 
		"op_collections_pol_apply_finance_charges" }, 

	{ PCM_OP_COLLECTIONS_POL_CALC_DUE_DATE,
			"op_collections_pol_calc_due_date" },

        { PCM_OP_COLLECTIONS_POL_PROCESS_BILLINFO,
                        "op_collections_pol_process_billinfo" }, 
*/
        { MSO_OP_COLLECTIONS_POL_APPLY_LATE_FEES,
                "op_mso_collections_pol_apply_late_fees" },
	{ 0,	(char *)0 }
};

#ifdef MSDOS
void *
fm_collections_pol_custom_config_func()
{
  return ((void *) (fm_collections_pol_custom_config));
}
#endif
