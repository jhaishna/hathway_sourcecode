/******************************************************************************
 *
* Copyright (c) 2011, 2013, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 *********************************************************************************/

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_bill_pol_custom_config.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:32:52 nishahan Exp $";
#endif

#include <stdio.h>
#include "ops/bill.h"
#include "pcm.h"
#include "cm_fm.h"

#ifdef WIN32
__declspec(dllexport) void * fm_bill_pol_custom_config_func();
#endif


/********************************************************************
 * If you want to customize any of the op-codes commented below, you
 * need to uncomment it.
 *******************************************************************/
struct cm_fm_config fm_bill_pol_custom_config[] = {
	/* opcode as a u_int, function name (as a string) */

	{ PCM_OP_BILL_POL_VALID_DISPUTE,	"op_bill_pol_valid_dispute" },
	{ PCM_OP_BILL_POL_VALID_SETTLEMENT, 	"op_bill_pol_valid_settlement" },
	{ PCM_OP_BILL_POL_VALID_ADJUSTMENT,	"op_bill_pol_valid_adjustment" },
	{ PCM_OP_BILL_POL_VALID_TRANSFER,	"op_bill_pol_valid_transfer" },
	{ PCM_OP_BILL_POL_VALID_WRITEOFF,	"op_bill_pol_valid_writeoff" },
	{ PCM_OP_BILL_POL_SPEC_BILLNO,		"op_bill_pol_spec_billno" },
	{ PCM_OP_BILL_POL_BILL_PRE_COMMIT,	"op_bill_pol_bill_pre_commit" },
	{ PCM_OP_BILL_POL_SPEC_FUTURE_CYCLE,	"op_bill_pol_spec_future_cycle", CM_FM_OP_OVERRIDABLE },
	{ PCM_OP_BILL_POL_GET_PENDING_ITEMS, "op_bill_pol_get_pending_items" },
	{ PCM_OP_BILL_POL_POST_BILLING, "op_bill_pol_post_billing" },
	{ PCM_OP_BILL_POL_CALC_PYMT_DUE_T, "op_bill_pol_calc_pymt_due_t" },
	{ PCM_OP_BILL_POL_CHECK_SUPPRESSION, "op_bill_pol_check_suppression" },
	{ PCM_OP_BILL_POL_EVENT_SEARCH, "op_bill_pol_event_search" },
	{ PCM_OP_BILL_POL_REVERSE_PAYMENT,	"op_bill_pol_reverse_payment" },
	{ PCM_OP_BILL_POL_CONFIG_BILLING_CYCLE,	"op_bill_pol_config_billing_cycle" },
	{ PCM_OP_BILL_POL_GET_ITEM_TAG,		"op_bill_pol_get_item_tag", CM_FM_OP_OVERRIDABLE },
	{ PCM_OP_BILL_POL_GET_EVENT_SPECIFIC_DETAILS,   "op_bill_pol_get_event_specific_details" },
	{ PCM_OP_BILL_POL_VALID_CORRECTIVE_BILL,   "op_bill_pol_valid_corrective_bill" },
	/*{ PCM_OP_BILL_POL_ITEM_REFUND,   "op_bill_pol_item_refund" },*/
	{ 0,    (char *)0 }
};

#ifdef WIN32
void *
fm_bill_pol_custom_config_func()
{
  return ((void *) (fm_bill_pol_custom_config));
}
#endif

