/*
 *
 *      Copyright (c) 2004 - 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_bill_pol_config_billing_cycle.c:BillingVelocityInt:2:2006-Sep-05 21:52:18 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_BILL_POL_CONFIG_BILLING_CYCLE  operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/bill.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#define FILE_SOURCE_ID  "fm_bill_pol_config_billing_cycle.c(1)"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_bill_pol_config_billing_cycle(
        cm_nap_connection_t	*connp,
		u_int				opcode,
        u_int				flags,
        pin_flist_t			*i_flistp,
        pin_flist_t			**o_flistpp,
        pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_BILL_POL_CONFIG_BILLING_CYCLE  operation.
 *******************************************************************/
void
op_bill_pol_config_billing_cycle(
        cm_nap_connection_t	*connp,
		u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_BILL_POL_CONFIG_BILLING_CYCLE ) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_bill_pol_config_billing_cycle opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_bill_pol_config_billing_cycle input flist", i_flistp);

	/***********************************************************
	 * Call corresponding fm_act_pol opcode
	 ***********************************************************/
	PCM_OP(ctxp, PCM_OP_ACT_POL_CONFIG_BILLING_CYCLE, flags, i_flistp,
			o_flistpp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_bill_pol_config_billing_cycle error", ebufp);

	} else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_bill_pol_config_billing_cycle return flist", *o_flistpp);
	}

	return;
}
