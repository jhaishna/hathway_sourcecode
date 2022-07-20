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
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_integ_config.c:CUPmod7.3PatchInt:1:2007-Jan-10 20:07:40 %";
#endif

#include <stdio.h>	/* for FILE * in pcm.h */
#include "ops/act.h"
#include "pcm.h"
#include "cm_fm.h"
#include "mso_ops_flds.h"

#ifdef MSDOS
__declspec(dllexport) void * fm_mso_integ_config_func();
#endif


/*******************************************************************
 *******************************************************************/

    /*
     * NOTE THAT THE DISPATCH ENTRIES ARE COMMENTED. WHEN YOU OVERRIDE
     * AN IMPLEMENTATION, UNCOMMENT THE LINE BELOW THAT MATCHES THE
     * OPCODE FOR WHICH YOU HAVE PROVIDED AN ALTERNATE IMPLEMENTATION.
     */

struct cm_fm_config fm_mso_integ_config[] = {
	/* opcode as a u_int, function name (as a string) */
	{ MSO_OP_INTEG_ADD_PLAN,			"op_mso_integ_add_plan" },
	{ MSO_OP_INTEG_CANCEL_PLAN,      		"op_mso_integ_cancel_plan" },
	{ MSO_OP_INTEG_CHANGE_PLAN,      		"op_mso_integ_change_plan" },
	{ MSO_OP_INTEG_REACTIVATE_SERVICE,	        "op_mso_integ_reactivate_service" },
	{ MSO_OP_INTEG_SUSPEND_SERVICE,    	 	"op_mso_integ_suspend_service" },
	{ MSO_OP_INTEG_CUST_ENQUIRY,    	 	"op_mso_integ_cust_enquiry" },
	{ MSO_OP_INTEG_CREATE_JOB,                      "op_mso_integ_create_job" },
	{ MSO_OP_INTEG_SEARCH_JOB,                      "op_mso_integ_search_jobs" },
	{ MSO_OP_INTEG_GET_FAILED_JOB_DETAILS,          "op_mso_integ_get_failed_job_details" },
	{ MSO_OP_INTEG_RESUBMIT_FAILED_JOB,          	"op_mso_integ_resubmit_failed_job" },
	{ MSO_OP_INTEG_GET_FAILED_ORDER_DETAILS,        "op_mso_integ_get_failed_order_details" },
	{ 0,	(char *)0 }
};

#ifdef MSDOS
void *
fm_mso_integ_config_func()
{
  return ((void *) (fm_mso_integ_config));
}
#endif

