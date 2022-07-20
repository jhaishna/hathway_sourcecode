/*******************************************************************
 *
 *      Copyright (c) 1999-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_valid_status.c:BillingVelocityInt:4:2006-Sep-05 04:32:49 %";
#endif

#include <stdio.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

/***********************************************************************
 * The following routines are defined elsewhere
 ***********************************************************************/
extern pin_flist_t* 
fm_cust_pol_valid_add_fail(
        pin_flist_t     *r_flistp,
        u_int           field,
        u_int           elemid,
        u_int           result,
        char            *descr,
        void            *val,
        pin_errbuf_t    *ebufp);

/*******************************************************************
 * Functions contained within.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_valid_status(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_valid_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);


 
/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_STATUS operation.
 *******************************************************************/
void
op_cust_pol_valid_status(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;


	/*
	 * Null out results until we have some.
	 */
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != PCM_OP_CUST_POL_VALID_STATUS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_valid_status", ebufp);
		return; 
	}

	/*
	 * Debug: what did we get?
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_valid_status input flist", i_flistp);

	/*
	 * Call main function to do it
	 */
	fm_cust_pol_valid_status(ctxp, i_flistp, &r_flistp, ebufp);

	/*
	 * Results.
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_valid_status error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_status return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_status()
 *
 *	Validate the status for an account/service.
 *
 *	XXX NOOP - STUBBED ONLY - ALWAYS RETURN PASS XXX
 *
 *******************************************************************/
static void
fm_cust_pol_valid_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp)
{
	void			*vp;
        poid_t                  *pdp;
        const char 		*poid_type = NULL;
	u_int			result = PIN_CUST_VERIFY_PASSED;
	u_int			status;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Create outgoing flist
	 */
	*out_flistpp = PIN_FLIST_CREATE(ebufp);


	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	/*
	 * Try to change root account?
	 */
        poid_type = PIN_POID_GET_TYPE(pdp);
	if (poid_type && strcasecmp(poid_type, "/account") == 0) {
 		if (PIN_POID_GET_ID(pdp) == 1) {
 
			PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_WARNING,
				"op_cust_pol_valid_status warning: root account status can't be changed");

   			status = PIN_STATUS_ACTIVE;

   			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_STATUS, &status, ebufp);
   			result =  PIN_CUST_VERIFY_FAILED;


			fm_cust_pol_valid_add_fail(
        			*out_flistpp,
        			PIN_FLD_STATUS, /* u_int field */
        			0,		/* u_int elemid */
        			result,
        			"root account status can't be changed",	/* char *descr */
         			PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 0, ebufp),  /* void  *val */
        			ebufp);

  		}
 	}
        else {
                vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS, 0, ebufp);
                if (vp) {
                        switch (*(int32 *)vp) {
                        case PIN_STATUS_CLOSED:
                        case PIN_STATUS_INACTIVE:
                        case PIN_STATUS_ACTIVE:
                                break;
                        default:
                                result = PIN_CUST_VERIFY_FAILED;
                                fm_cust_pol_valid_add_fail(*out_flistpp,
                                               PIN_FLD_STATUS, 0, result, 
                                               "invalid status value",
                                               vp, ebufp);
                        }
                }
        }
        if (result == PIN_CUST_VERIFY_PASSED) {
		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, pdp,
                                  ebufp);
                vp = (void *)&result;
                PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_RESULT, vp, ebufp);
        }

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_status error", ebufp);
	}

	return;
}

