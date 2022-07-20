/**********************************************************************
*
*	C Source:		fm_bill_pol_spec_cycle_fee_interval.c
*	Instance:		1
*	Description:	
*	%created_by:	kbhasin %
*	%date_created:	Fri Feb 02 11:33:13 2001 %
*
**********************************************************************
*
*      Copyright (c) 1999 - 2006 Oracle. All rights reserved.
*
*      This material is the confidential property of Oracle Corporation or its
*      licensors and may be used, reproduced, stored or transmitted only in
*      accordance with a valid Oracle license or sublicense agreement.
*
**********************************************************************/
#ifndef lint
static char *_csrc = "@(#) %filespec: fm_bill_pol_spec_cycle_fee_interval.c~5 %  (%full_filespec: fm_bill_pol_spec_cycle_fee_interval.c~5:csrc:1 %)";
#endif
/*******************************************************************
 * Contains the PCM_OP_BILL_POL_SPEC_CYCLE_FEE_INTERVAL operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "pin_rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_bill_pol_spec_cycle_fee_interval(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_BILL_POL_SPEC_CYCLE_FEE_INTERVAL operation.
 *******************************************************************/
void
op_bill_pol_spec_cycle_fee_interval(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;
	int32			*scopep = NULL;
	int32			*mmc_typep = NULL;

	*r_flistpp = NULL;
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != PCM_OP_BILL_POL_SPEC_CYCLE_FEE_INTERVAL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_spec_cycle_fee_interval bad opcode error", 
			ebufp);
		return;
	}

	/*
	 * Debug: What we got.
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_spec_cycle_fee_interval input flist", i_flistp);

	scopep = (int32 *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SCOPE, 0, ebufp);
	if (!scopep) {
		goto cleanup;
	}

	mmc_typep = (int32 *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MMC_TYPE, 0, 
		ebufp);
	if (!mmc_typep) {
		goto cleanup;
	}

	/*
	 * Prepare return flist.
	 */
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, vp, ebufp);

	switch (*scopep) {
	case PIN_RATE_SCOPE_CYCLE_FORWARD:
		if (*mmc_typep == 0) { /* This is single-month fee */
			vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SMC_START_T,
				0, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SMC_START_T, 
				vp, ebufp);
			vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SMC_END_T,
				0, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SMC_END_T, 
				vp, ebufp);
		} else { /* This is multi-month fee */
			vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MMC_START_T,
				0, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_MMC_START_T, 
				vp, ebufp);
			vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MMC_END_T,
				0, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_MMC_END_T, 
				vp, ebufp);
		}
		break;
	case PIN_RATE_SCOPE_CYCLE_ARREARS:
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SMC_START_T, 0, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SMC_START_T, vp, ebufp);
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SMC_END_T, 0, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SMC_END_T, vp, ebufp);
		break;
	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_SCOPE, 0, *scopep);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Bad value of the scope field", ebufp);
		goto cleanup;
	}

cleanup:
	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"op_bill_pol_spec_cycle_fee_interval error", ebufp);
	} else {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_spec_cycle_fee_interval output flist", *r_flistpp);
	}
	return;
}
