/*******************************************************************
 *
 *      Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_prep_profile.c:RWSmod7.3.1Int:1:2007-Jun-28 05:36:31 %";
#endif

#include <stdio.h>
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

EXPORT_OP void
op_cust_pol_prep_profile(
        cm_nap_connection_t     *connp,
        u_int                   opcode,
        u_int                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * fm_cust_pol_prep_profile()
 *
 *      Prep the profile info to be ready for storage.
 *
 *      XXX STUBBED ONLY XXX
 *
 *******************************************************************/
static void
fm_cust_pol_prep_profile(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *flistp = NULL;
        poid_t                  *s_pdp = NULL;
 
        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);
 
        /***********************************************************
         * Create outgoing flist
         ***********************************************************/
        *out_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
 
#ifdef EXAMPLE_USE

        /***********************************************************
         * Dummy up the ip inherited info in none provided
         ***********************************************************/
        s_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
 
        /*
         * XXX - this is the place where any customer sites could plug
         * in any preperatory profile specific information. Note that the
         * POID at this point is:
         *
         *      * A "type only" poid if the profile object is about to
         *        be created.
         *
         *      * A "real" poid if the profile object is being modified.
         */
 
        /*
         * One prep example might be to name the object that is
         * being created. Do this only on create.
         */
        if(PIN_POID_IS_TYPE_ONLY(s_pdp)) {
                pin_flist_t     *sublistp;
 
                sublistp = PIN_FLIST_SUBSTR_GET(*out_flistpp,
                        PIN_FLD_INHERITED_INFO, 1, ebufp);
                if(sublistp == (pin_flist_t *)NULL) {
                        sublistp = PIN_FLIST_SUBSTR_ADD(*out_flistpp,
                                PIN_FLD_INHERITED_INFO, ebufp);
                }
 
                PIN_FLIST_FLD_SET(sublistp, PIN_FLD_NAME,
                        (void *)"Example Name", ebufp);
        }
#endif

        /***********************************************************
         * Error?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_cust_pol_prep_profile error", ebufp);
        }
 
        return;
}

/*******************************************************************
* Main routine for the PCM_OP_CUST_POL_PREP_PROFILE command
*******************************************************************/
void
op_cust_pol_prep_profile(
        cm_nap_connection_t     *connp,
        u_int                   opcode,
        u_int                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;
        pin_flist_t             *r_flistp = NULL;
 
        /***********************************************************
        * Null out results until we have some.
        ***********************************************************/
        *ret_flistpp = NULL;
        PIN_ERR_CLEAR_ERR(ebufp);
 
        /***********************************************************
        * Insanity check.
        ***********************************************************/
        if (opcode != PCM_OP_CUST_POL_PREP_PROFILE) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_cust_pol_prep_profile", ebufp);
                return;
        }

        /***********************************************************
         * We will not open any transactions with Policy FM
         * since policies should NEVER modify the database.
         ***********************************************************/
 
        /***********************************************************
         * Call main function to do it
         ***********************************************************/
        fm_cust_pol_prep_profile(ctxp, in_flistp, &r_flistp, ebufp);
 
        /***********************************************************
         * Results.
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                *ret_flistpp = (pin_flist_t *)NULL;
                PIN_FLIST_DESTROY(r_flistp, NULL);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_cust_pol_prep_profile error", ebufp);
        } else {
                *ret_flistpp = r_flistp;
                PIN_ERR_CLEAR_ERR(ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_cust_pol_prep_profile return flist", r_flistp);
        }
 
        return;
}
