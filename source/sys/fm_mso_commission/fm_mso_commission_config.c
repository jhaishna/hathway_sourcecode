/*******************************************************************
 *
 *
* Copyright (c) 2007, 2011, Oracle and/or its affiliates. All rights reserved. 
 * 
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
//static  char Sccs_Id[] = "@(#)$Id: fm_mso_commission_config.c /cgbubrm_main.rwsmod/1 2011/07/26 04:22:50 dbangalo Exp $";
#endif

#include <stdio.h>			/* for FILE * in pcm.h */
#include "ops/cust.h"
#include "pcm.h"
#include "cm_fm.h"
#include "mso_ops_flds.h"
#include "mso_commission.h"


PIN_EXPORT void * fm_mso_commission_config_func();

/*******************************************************************
 *******************************************************************/

struct cm_fm_config fm_mso_commission_config[] = {
	/* opcode as a u_int, function name (as a string) */

        /*****************************************************************************
         * If you want to customize any of the op-codes commented below, you need to *
         * uncomment it.
         *****************************************************************************/

	{ MSO_OP_COMMISSION_GET_LCO_PLANS,			"op_mso_commission_get_lco_plans" },
	{ MSO_OP_COMMISSION_MANAGE_AGREEMENT,			"op_mso_commission_manage_agreement" },
	{ MSO_OP_COMMISSION_PROCESS_COMMISSION,			"op_mso_commission_process_commission" },
	{ MSO_OP_COMMISSION_APPLY_COMMISSION,			"op_mso_commission_apply_commission" },
	{ MSO_OP_COMMISSION_LCO_BAL_TRANSFER,			"op_mso_commission_lco_bal_transfer" },
	{ MSO_OP_COMMISSION_LCO_BAL_IMPACT,			"op_mso_commission_lco_bal_impact" },
	{ MSO_OP_COMMISSION_RECTIFY_LCO_WRONG_TAGGING,		"op_mso_commission_rectify_lco_wrong_tagging" },
	{ MSO_OP_COMMISSION_RECTIFY_SDT_DT_WRONG_TAGGING,	"op_mso_commission_rectify_sdt_dt_wrong_tagging" },
	{ MSO_OP_COMMISSION_PROCESS_DSA_COMMISSION,		"op_mso_commission_process_dsa_commission" },
	{ 0,	(char *)0 }
};

void *
fm_mso_commission_config_func()
{
  return ((void *) (fm_mso_commission_config));
}

