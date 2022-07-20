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
static  char Sccs_id[] = "@(#)%Portal Version: fm_pymt_pol_provision_incentive.c:BillingVelocityInt:2:2006-Sep-05 21:52:50 %";
#endif

#include "pcm.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_cust.h"
#include "ops/pymt.h"
#include "psiu_round.h"

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/

EXPORT_OP void
op_pymt_pol_provision_incentive(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);


static void
fm_pymt_pol_provision_incentive(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

extern time_t
psiu_time_round_to_midnight_apply_offset(
	time_t	a_time,
	int	offset);

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_PROVISION_INCENTIVE command
 *******************************************************************/

void
op_pymt_pol_provision_incentive(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp		= connp->dm_ctx;
	pin_flist_t	*r_flistp	= NULL;


	/***********************************************************
	* Null out results until we have some.
	***********************************************************/

	*o_flistpp	= NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);


	/***********************************************************
        * Error out in case opcode is not PCM_OP_PYMT_POL_PROVISION_INCENTIVE
	***********************************************************/

	if (opcode != PCM_OP_PYMT_POL_PROVISION_INCENTIVE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_pymt_pol_provsion_incentive", ebufp);
		return;
	}


	/***********************************************************
        * Debug what we got.
        ***********************************************************/

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_pymt_pol_provision_incentive input flist", in_flistp);

	/***********************************************************
	 * Calling fm_pymt_pol_provision_incentive function.
	 ***********************************************************/

	fm_pymt_pol_provision_incentive(ctxp, flags, in_flistp, 
						&r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERR_IS_ERR(ebufp)) {
		*o_flistpp	= (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_pymt_pol_provision_incentive error", ebufp);
	} else {
		*o_flistpp	= r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		    "op_pymt_pol_provision_incentive return flist", r_flistp);
	}

	return;
}



/*******************************************************************
 * fm_pymt_pol_provision_incentive()
 * Gets the End_T from event object and returns it as effective_t
 *******************************************************************/

static void
fm_pymt_pol_provision_incentive(
	pcm_context_t	*ctxp,
	int32		flags,
	pin_flist_t	*in_flistp,
	pin_flist_t	**out_flistpp,
	pin_errbuf_t	*ebufp)
{
	/**********************************************************
	 * Local Variables	  					
	 *********************************************************/

	pin_flist_t	*s_flistp	= NULL;
	pin_flist_t	*r_flistp	= NULL;
	
	void 		*vp		= NULL;
	time_t		*effective_tp	= NULL;

	if (PIN_ERR_IS_ERR(ebufp)) { 
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/**********************************************************
	 * Creating the Output Flist		               
	 *********************************************************/

	if (*out_flistpp == (pin_flist_t *)NULL) {
		*out_flistpp = PIN_FLIST_CREATE(ebufp);
	}

	/**********************************************************
	 * Getting the Poid from input flist and setting it in 
	 * output flist.
	 *********************************************************/

	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID,
						0, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, vp, ebufp);

	/**********************************************************
	 * Checking if Effective_t exists in the input flist.
	 * 	1. If it exists then setting the same effective_t
	 * 	   in output flist.
	 * 	2. Else Reading the Event Object and setting the 
	 *	   effective_t of event object in the output flist.
	 *********************************************************/

	effective_tp = (time_t *) PIN_FLIST_FLD_GET(in_flistp,
					 PIN_FLD_EFFECTIVE_T, 1, ebufp);

	if (effective_tp) {

		*effective_tp =
		     psiu_time_round_to_midnight_apply_offset(*effective_tp, 0);

		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_EFFECTIVE_T,
					 (void *) effective_tp, ebufp);
	} else {
		vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PAYMENT_EVENT_OBJ,
							0, ebufp);

		s_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, vp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_END_T, NULL, ebufp);

		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, s_flistp, &r_flistp, ebufp);

		effective_tp = (time_t *) PIN_FLIST_FLD_GET(r_flistp, 
						PIN_FLD_END_T, 0, ebufp);

		if (effective_tp) {
			*effective_tp = 
				 psiu_time_round_to_midnight_apply_offset(
					*effective_tp, 0);
		}

		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_EFFECTIVE_T, 
					(void *) effective_tp, ebufp);

		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/

 	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_provision_incentive error", ebufp);
	}

	return;
}
