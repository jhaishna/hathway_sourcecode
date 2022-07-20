/*******************************************************************
 *
 *  @(#) %full_filespec: fm_cust_pol_prep_topup.c~8:6.7FP2;070604;119686%
 *
* Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 * 
 *******************************************************************/

#ifndef lint
    static  char    Sccs_id[] = "@(#)%full_filespec: fm_cust_pol_prep_topup.c~8:6.7FP2;070604;119686%";
#endif

#include <stdio.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pcm.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_cust.h"

EXPORT_OP void
op_cust_pol_prep_topup(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


static void fm_cust_pol_prep_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void fm_cust_pol_prep_modify_standard_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void fm_cust_pol_prep_modify_sponsored_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void fm_cust_pol_prep_create_sponsored_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void fm_cust_pol_prep_create_standard_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp); 


/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_PREP_TOPUP operation
 *******************************************************************/
void
op_cust_pol_prep_topup(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;

        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        /*
         * Null out results until we have some.
         */
        *r_flistpp = NULL;

        /*
         * Insanity check.
         */

        if (opcode != PCM_OP_CUST_POL_PREP_TOPUP) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_cust_pol_prep_topup", ebufp);
                return;
        }


        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                          "op_cust_pol_prep_topup input flist:",
                          i_flistp);

        /*
         * Call the main function.
         */
        fm_cust_pol_prep_topup(ctxp, flags, i_flistp, r_flistpp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                          "op_cust_pol_prep_topup output flist:",
                          *r_flistpp);


        if (PIN_ERR_IS_ERR(ebufp))
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                 "op_cust_pol_prep_topup flist error",
                                 ebufp);

        return;
}

/********************************************************************
 * Function : fm_cust_pol_prep_topup
 * Description : This function will prepare the input flist for the
 *               Standards as well as the Sponsored topup.
 ********************************************************************/
static void fm_cust_pol_prep_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
    int    createonly =    0;
    int    status     =    0;
    int    index      =    0;
    int    sponsore_topup = 0;
    void    *vp       =    NULL;

    
    pin_flist_t    *intopup_flistp        =    NULL;
    pin_flist_t    *outtopup_flistp       =    NULL;
    pin_flist_t    *ingrptopupmem_flistp  =    NULL;
    pin_flist_t    *outgrptopupmem_flistp =    NULL;
    pin_flist_t    *grptopup_flistp       =    NULL;
    pin_flist_t    *ingrptopup_flistp     =    NULL;
    pin_flist_t    *outgrptopup_flistp    =    NULL;
    pin_flist_t    *r_flistp              =    NULL;

    poid_t    *intopup_pdp   =    NULL;
    poid_t    *inparent_pdp  =    NULL;
    poid_t    *initiator_pdp =    NULL;
    poid_t    *grp_pdp       =    NULL;
    poid_t    *tmp_pdp       =    NULL;
    
    
    if (PIN_ERR_IS_ERR(ebufp))
            return;
    PIN_ERR_CLEAR_ERR(ebufp);
    /****************************************************************
     * Get the PIN_FLD_TOPUP_INFO from the input flist.
     * Value available in the input flist this function construct the 
     * output flist.
     *****************************************************************/
    intopup_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOPUP_INFO,
                                                         PIN_ELEMID_ANY, 0, ebufp); 

    /****************************************************************
     * Just copy the input flist to the retflist.If any change is required
     * for the flist we will manipulate the output flist.
     ****************************************************************/

    *r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
        
    /****************************************************************
     * First check requires the POID and PARENT poid from the input 
     * flist. From this we check whether the call is to
     * create the standard topup or the sponsored topup.
     ****************************************************************/
    intopup_pdp = (poid_t *)PIN_FLIST_FLD_GET(intopup_flistp, PIN_FLD_POID, 0, ebufp);
    inparent_pdp = (poid_t *)PIN_FLIST_FLD_GET(intopup_flistp,  PIN_FLD_PARENT, 1, ebufp);
        
    /****************************************************************
     * check the topup poid is just type only or not.
     ****************************************************************/
    if( PIN_POID_IS_TYPE_ONLY(intopup_pdp) )
    { 
        /****************************************************************
         * topup poid is just type only i,e. 0.0.0.x /topup -1 1
	 ****************************************************************/
        if( PIN_POID_IS_NULL(inparent_pdp) )
        {
            fm_cust_pol_prep_create_standard_topup(ctxp, flags, i_flistp, r_flistpp, ebufp);
            return;
        }
        else
        {
            createonly = 1;
            sponsore_topup = 1;
            fm_cust_pol_prep_create_sponsored_topup(ctxp, flags, i_flistp, r_flistpp, ebufp);
        }
    }
    else
    {
        /****************************************************************
         * The input flist contains the valid topup poid.
         ****************************************************************/

        /****************************************************************
         * Now read PIN_FLD_GROUP_OBJ and PIN_FLD_GROUP_INDEX from 
         * the /topup object 
         ****************************************************************/

        grptopup_flistp = PIN_FLIST_CREATE(ebufp);
        tmp_pdp = PIN_POID_COPY( intopup_pdp, ebufp);
        PIN_FLIST_FLD_PUT(grptopup_flistp, PIN_FLD_POID, tmp_pdp, ebufp);
        /****************************************************************
         * setting the group dummy group poid and group index
         ****************************************************************/
        PIN_FLIST_FLD_SET(grptopup_flistp, PIN_FLD_GROUP_OBJ, tmp_pdp, ebufp);
        PIN_FLIST_FLD_SET(grptopup_flistp, PIN_FLD_GROUP_INDEX, (void *) &index, ebufp);

        PCM_OP(ctxp, PCM_OP_READ_FLDS, flags, grptopup_flistp, &r_flistp, ebufp);


        /****************************************************************
         * Now get the group object from the return flist.
         ****************************************************************/
        grp_pdp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_GROUP_OBJ, 1, ebufp);

        /****************************************************************
         * check whethere the poid is null or not.
         ****************************************************************/
        if( PIN_POID_IS_NULL(grp_pdp) )
        {
            if( PIN_POID_IS_NULL(inparent_pdp) )  
            {
                /****************************************************************
                 * Since the input flist does not contains the PARENT and
                 * specefied the topup object does not have the GROUP_OBJ
                 * it hase to prepare the flist for the modification
                 * standard topup.
                 ****************************************************************/
                fm_cust_pol_prep_modify_standard_topup(ctxp, flags, i_flistp, r_flistpp, ebufp);
                return ;
            }
            else
            {
                /****************************************************************
                 * Input flist have the PARENT poid and the topup object does not  have the
                 * valid group poid, hence prepare the flist to create the 
                 * sponsored topup.
                 * This case if for a account holder having the standard topup
                 * is being added to PARENts some group.
                 ****************************************************************/

                createonly = 1;
                sponsore_topup = 1;
                fm_cust_pol_prep_create_sponsored_topup(ctxp, flags, i_flistp, r_flistpp, ebufp);
            }
        }
        else
        {
            /****************************************************************
             * The topup object is having the  valid group poid.
             * Hence prepare the flistp for modifing the sponsored topup.
             ****************************************************************/
            sponsore_topup = 1;
            fm_cust_pol_prep_modify_sponsored_topup(ctxp, flags, i_flistp, r_flistpp, ebufp);
        }
        PIN_FLIST_DESTROY_EX(&grptopup_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    }

    /****************************************************************
     * Check whethere we are preparing the flist for standard or
     * sponsored topup. Becuse we have to continue exution for 
     * for sponsored topup.
     ****************************************************************/
    if( sponsore_topup )
    {
        initiator_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        outtopup_flistp = PIN_FLIST_ELEM_GET(*r_flistpp, PIN_FLD_TOPUP_INFO,
                                                 PIN_ELEMID_ANY, 0, ebufp);

        outgrptopup_flistp = PIN_FLIST_ELEM_GET(outtopup_flistp, PIN_FLD_GROUP_TOPUP_INFO,
                                                 PIN_ELEMID_ANY, 0, ebufp);
        outgrptopupmem_flistp = PIN_FLIST_ELEM_GET(outgrptopup_flistp, PIN_FLD_GROUP_TOPUP_MEMBERS,
                                                PIN_ELEMID_ANY, 0, ebufp);

        /****************************************************************
         * Check who is the intiatore of the operation.
         ****************************************************************/
        if( !PIN_POID_COMPARE( inparent_pdp, initiator_pdp , 0, ebufp) )
        {
            /****************************************************************
             * Initiatore is the payers.
             ****************************************************************/
            ingrptopup_flistp = PIN_FLIST_ELEM_GET(intopup_flistp, PIN_FLD_GROUP_TOPUP_INFO,
                                            PIN_ELEMID_ANY, 0, ebufp);
            ingrptopupmem_flistp = PIN_FLIST_ELEM_GET(ingrptopup_flistp, PIN_FLD_GROUP_TOPUP_MEMBERS,
                                            PIN_ELEMID_ANY, 0, ebufp);

            /****************************************************************
             * If the STATUS is not set set it to ACTIVE.
             ****************************************************************/
            vp = PIN_FLIST_FLD_GET(ingrptopupmem_flistp, PIN_FLD_STATUS, 1, ebufp);

            if( vp == NULL )
            {
                status = PIN_STATUS_ACTIVE;
                PIN_FLIST_FLD_SET(outgrptopupmem_flistp, PIN_FLD_STATUS, &status, ebufp);
            }

            /****************************************************************
             * If the opcode is creating the sponsored topup and the group 
             * name is not set it to default.
             ****************************************************************/
            if( createonly )
            {
                vp = PIN_FLIST_FLD_GET(ingrptopup_flistp, PIN_FLD_NAME, 1, ebufp);
                if( vp == NULL )
                {
                    PIN_FLIST_FLD_SET(outgrptopup_flistp, PIN_FLD_NAME, (void *)"default", ebufp);
                }
            }
        }
        else
        {
            /****************************************************************
             * Initiatore is member account. Set the PIN to NULL and
             * STATUS is INACTIVE.
             ****************************************************************/
            status = PIN_STATUS_INACTIVE;
            PIN_FLIST_FLD_SET(outgrptopupmem_flistp, PIN_FLD_STATUS, &status, ebufp);
            PIN_FLIST_FLD_SET(outgrptopupmem_flistp, PIN_FLD_PIN, (void *)"", ebufp);
        }
    }
    if (PIN_ERR_IS_ERR(ebufp))
              PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                               "fm_cust_pol_prep_topup error",
                                 ebufp);

    return;
}

/****************************************************************
 * Function : fm_cust_pol_prep_modify_standard_topup
 * Description : Noting to prepare. late custer can add.
 ****************************************************************/
static void fm_cust_pol_prep_modify_standard_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
    if (PIN_ERR_IS_ERR(ebufp))
            return;    
    PIN_ERR_CLEAR_ERR(ebufp); 
    return;
}

/****************************************************************
 * Function : fm_cust_pol_prep_create_standard_topup
 * Description : Noting to prepare. late custer can add.
 ****************************************************************/
static void fm_cust_pol_prep_create_standard_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
    if (PIN_ERR_IS_ERR(ebufp))
            return;    
    PIN_ERR_CLEAR_ERR(ebufp); 
    return;
}

/****************************************************************
 * Function : fm_cust_pol_prep_modify_sponsored_topup
 * Description : Noting to prepare. late custer can add.
 ****************************************************************/
static void fm_cust_pol_prep_modify_sponsored_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
    if (PIN_ERR_IS_ERR(ebufp))
            return;    
    PIN_ERR_CLEAR_ERR(ebufp); 
    return;
}

/****************************************************************
 * Function : fm_cust_pol_prep_create_sponsored_topup
 * Description : This function will prepare the flist for
 *               creating the sponsored topup operation.
 ****************************************************************/
static void fm_cust_pol_prep_create_sponsored_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
    poid_t    *initiator_pdp =    NULL;
    poid_t    *parent_pdp    =    NULL;
    poid_t    *balgrp_pdp    =    NULL;
    poid_t    *s_pdp         =    NULL;
    poid_t    *grp_pdp       =    NULL;

    pin_flist_t    *topup_flistp      =    NULL;
    pin_flist_t    *ingrptopup_flistp =    NULL;
    pin_flist_t    *ingrplmt_flistp   =    NULL;
    pin_flist_t    *ingrpmem_flistp   =    NULL;
    pin_flist_t    *tmp_flistp        =    NULL;
    pin_flist_t    *args1_flistp      =    NULL;
    pin_flist_t    *args2_flistp      =    NULL;
    pin_flist_t    *args3_flistp      =    NULL;
    pin_flist_t    *outtopup_flistp   =    NULL;
    pin_flist_t    *outgrpmem_flistp  =    NULL;
    pin_flist_t    *outgrptopup_flistp=    NULL;
    pin_flist_t    *reslt_flistp      =    NULL;
    pin_flist_t    *f_flistp          =    NULL;
    pin_flist_t    *s_flistp          =    NULL;
    pin_flist_t    *r_flistp          =    NULL;
    
    char   *grp_name   = NULL;
    char   *s_tmplt    = NULL;
    int    *res_id     = NULL;
    int    *topup_interval = NULL;
    int64  db_no       = 0;
    int    search_flag = 256;
    time_t nexttopup;
    time_t now;
    void   *vp;


    
    if (PIN_ERR_IS_ERR(ebufp))
            return;    
    PIN_ERR_CLEAR_ERR(ebufp); 

    initiator_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

    topup_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOPUP_INFO,
                                                        PIN_ELEMID_ANY, 0, ebufp);
    ingrptopup_flistp = PIN_FLIST_ELEM_GET(topup_flistp, PIN_FLD_GROUP_TOPUP_INFO,
                                                        PIN_ELEMID_ANY, 0, ebufp);
    ingrpmem_flistp = PIN_FLIST_ELEM_GET(ingrptopup_flistp, PIN_FLD_GROUP_TOPUP_MEMBERS,
                                                        PIN_ELEMID_ANY, 0, ebufp);
    ingrplmt_flistp = PIN_FLIST_ELEM_GET(ingrptopup_flistp, PIN_FLD_GROUP_TOPUP_LIMITS,
                                                        PIN_ELEMID_ANY, 0, ebufp);

    /****************************************************************
     * Update the NEXT_TOPUP_T in the out put flist.
     ****************************************************************/

    outtopup_flistp = PIN_FLIST_ELEM_GET(*r_flistpp, PIN_FLD_TOPUP_INFO,
                                                        PIN_ELEMID_ANY, 0, ebufp);
    outgrptopup_flistp = PIN_FLIST_ELEM_GET(outtopup_flistp, PIN_FLD_GROUP_TOPUP_INFO,
                                                        PIN_ELEMID_ANY, 0, ebufp);
    outgrpmem_flistp = PIN_FLIST_ELEM_GET(outgrptopup_flistp, PIN_FLD_GROUP_TOPUP_MEMBERS,
                                                        PIN_ELEMID_ANY, 0, ebufp);
    topup_interval = PIN_FLIST_FLD_GET(outgrpmem_flistp, PIN_FLD_TOPUP_INTERVAL, 0, ebufp);

    if (PIN_ERR_IS_ERR(ebufp))
            return;

    vp = PIN_FLIST_FLD_GET(ingrpmem_flistp , PIN_FLD_NEXT_TOPUP_T, 1, ebufp);

    if(vp){
        PIN_FLIST_FLD_SET(outgrpmem_flistp, PIN_FLD_NEXT_TOPUP_T,  vp, ebufp);
    }else{
        now = pin_virtual_time((time_t *) NULL);
	if(topup_interval){
        	nexttopup = now + ((*topup_interval) * 24 * 60 * 60 );
	}
        PIN_FLIST_FLD_SET(outgrpmem_flistp, PIN_FLD_NEXT_TOPUP_T, &nexttopup, ebufp);
    }

    grp_name = PIN_FLIST_FLD_GET(ingrptopup_flistp, PIN_FLD_NAME, 1, ebufp);
    grp_pdp = PIN_FLIST_FLD_GET(ingrptopup_flistp, PIN_FLD_POID, 0, ebufp);

    /****************************************************************
     * check if the group poid is type only or not and also
     * it has the name or not.
     ****************************************************************/

    if( PIN_POID_IS_TYPE_ONLY(grp_pdp) || !grp_name )
    {
        /****************************************************************
         * If the group name or the group poid is not defind in the
         * input flist then search /group/topup for the particular
         * parent and set the name and the group poid.
         ****************************************************************/
        parent_pdp = PIN_FLIST_FLD_GET(ingrptopup_flistp, PIN_FLD_PARENT, 0, ebufp);
        balgrp_pdp = PIN_FLIST_FLD_GET(ingrptopup_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
        res_id = PIN_FLIST_FLD_GET(ingrplmt_flistp, PIN_FLD_RESOURCE_ID, 0, ebufp);

        db_no = PIN_POID_GET_DB(initiator_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db_no, "/search", -1, ebufp);
        s_tmplt = " select X from /group/topup where F1 = V1 and F2 = V2 and F3 = V3 ";
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_tmplt, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &search_flag, ebufp);
        
        tmp_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PARENT, parent_pdp, ebufp);
        
        args1_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_BAL_GRP_OBJ, balgrp_pdp, ebufp);
        f_flistp = PIN_FLIST_CREATE(ebufp);
        if( grp_name == NULL )
        {
            /****************************************************************
             * if the group name is null then the search argument will contain
             * the resource id.
             ****************************************************************/
            args3_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_SET(f_flistp, PIN_FLD_RESOURCE_ID, res_id, ebufp);
            PIN_FLIST_ELEM_PUT(args3_flistp, f_flistp, PIN_FLD_GROUP_TOPUP_LIMITS, PCM_RECID_ALL, ebufp);
            PIN_FLIST_ELEM_PUT(s_flistp, args3_flistp, PIN_FLD_ARGS,  3, ebufp);
        }
        else
        {
            PIN_FLIST_FLD_SET(f_flistp, PIN_FLD_NAME, grp_name, ebufp);
            PIN_FLIST_ELEM_PUT(s_flistp, f_flistp, PIN_FLD_ARGS,  3, ebufp);
        }


        PIN_FLIST_ELEM_PUT(s_flistp, tmp_flistp, PIN_FLD_ARGS,  1, ebufp);
        PIN_FLIST_ELEM_PUT(s_flistp, args1_flistp, PIN_FLD_ARGS,  2, ebufp);

        reslt_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(reslt_flistp, PIN_FLD_NAME, "",ebufp);

        PIN_FLIST_ELEM_PUT(s_flistp, reslt_flistp, PIN_FLD_RESULTS,  0, ebufp);

        PCM_OP(ctxp, PCM_OP_SEARCH, flags, s_flistp, &r_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

        tmp_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        /****************************************************************
         * If the results array of search contains any record then select the
         * finst element of the results array and set the group poid and name
         * else don't set it.
         ****************************************************************/
        if( tmp_flistp != NULL )
        {
            grp_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
            grp_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);
             
            f_flistp = PIN_FLIST_ELEM_GET(outtopup_flistp, PIN_FLD_GROUP_TOPUP_INFO,
                                                        PIN_ELEMID_ANY, 0, ebufp);
            PIN_FLIST_FLD_SET(f_flistp, PIN_FLD_NAME, grp_name, ebufp);
            PIN_FLIST_FLD_SET(f_flistp, PIN_FLD_POID, grp_pdp, ebufp);
        }
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    }
    if (PIN_ERR_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                 "fm_cust_pol_prep_create_sponsored_topup error",
                                 ebufp);
    }
    return;
}
