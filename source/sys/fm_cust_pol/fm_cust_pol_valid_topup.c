/*******************************************************************
 *
* Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
    static  char    Sccs_id[] = "@(#)%Portal Version: fm_cust_pol_valid_topup.c:BillingVelocityInt:4:2006-Sep-05 04:27:31 %";
#endif

#include <stdio.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "pcm.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pbo_decimal.h"

EXPORT_OP void
op_cust_pol_valid_topup(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void fm_cust_pol_valid_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void fm_cust_validate_sponsored_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *input_flistp,
        pin_errbuf_t            *ebufp);

static void fm_cust_validate_standard_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *input_flistp, 
        pin_errbuf_t            *ebufp);

double 
pbo_decimal_to_double(
    const pin_decimal_t *pdp,
    pin_errbuf_t *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_TOPUP operation
 *******************************************************************/
void
op_cust_pol_valid_topup(
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
        if (opcode != PCM_OP_CUST_POL_VALID_TOPUP) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_cust_valid_topup", ebufp);
                return;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                          "op_cust_pol_valid_topup input flist:",
                          i_flistp);

        /*
         * Call the main function.
         */
        fm_cust_pol_valid_topup(ctxp, flags, i_flistp, r_flistpp, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                          "op_cust_pol_valid_topup output flist:",
                          *r_flistpp);

        if (PIN_ERR_IS_ERR(ebufp))
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                 "op_cust_pol_valid_topup flist error",
                                 ebufp);
        return;
}

/****************************************************************
 * Funtion : fm_cust_pol_valid_topup
 * Description : This function will check whether to validate
 *               for standard topup or sponsored topup.
 ****************************************************************/
static void fm_cust_pol_valid_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{

    void              *vp = NULL;
    int32             result = PIN_RESULT_PASS ;
    pin_flist_t       *topup_flistp;
    poid_t            *pdp;


    if (PIN_ERR_IS_ERR(ebufp))
        return;
    PIN_ERR_CLEAR_ERR(ebufp);

    /****************************************************************
     * Copy the input flist to the output flist.
     ****************************************************************/
    *r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);

    /****************************************************************
     * get the TOPUP_INFO array from the input flist.
     ****************************************************************/
    topup_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOPUP_INFO,PIN_ELEMID_ANY,0, ebufp);


    /****************************************************************
     * Get the parent the poid from the input flist.
     * If the parent poid is present in the input flist, then it is
     * sponsored topup or it is standard topup.
     ****************************************************************/
    pdp = PIN_FLIST_FLD_GET(topup_flistp, PIN_FLD_PARENT, 1, ebufp);

    if( !pdp )
    {
        fm_cust_validate_standard_topup(ctxp, flags, i_flistp, ebufp);
    }
    else  
    {
        fm_cust_validate_sponsored_topup(ctxp, flags, i_flistp, ebufp);
    }
 

    /****************************************************************
     * If there is no error buf is set then then the validation is
     * success, then append the reust as PASS in to the output flist
     * else apped result field into the output flist as fail.
     ****************************************************************/

    if (PIN_ERR_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_pol_valid_topup error", ebufp);
        PIN_ERR_CLEAR_ERR(ebufp);
        result = PIN_RESULT_FAIL;
    }
    vp = (void *)&result;
    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_RESULT, vp, ebufp);
        
    return;
}


/****************************************************************
 * Function : fm_cust_validate_standard_topup
 * Description : This function will validate the input data for
 *               standard topup.
 ****************************************************************/
static void fm_cust_validate_standard_topup(
        pcm_context_t *ctxp,
        int32         flags,
        pin_flist_t   *input_flistp,
        pin_errbuf_t  *ebufp)
{
    pin_flist_t       *in_flistp  = NULL;
    poid_t            *parent_pdp = NULL;
    poid_t            *topup_pdp  = NULL;
    pin_decimal_t     *topup_amt  = NULL;
    pin_decimal_t     *topup_cap  = NULL;
    int32             optional    = 1;

    if (PIN_ERR_IS_ERR(ebufp))    
        return;        
    PIN_ERR_CLEAR_ERR(ebufp); 

    /****************************************************************
     * Get the topup amount and the topup capacity and just do the
     * sanity check.
     ****************************************************************/
    in_flistp = PIN_FLIST_ELEM_GET(input_flistp,PIN_FLD_TOPUP_INFO,PIN_ELEMID_ANY,  0, ebufp);

    topup_pdp = (poid_t *) PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);


    /****************************************************************
     * If the call come from CRETATE_TOPUP, then the TOPUP_AMT is
     * mandatory or if the call is from MODIFY_TOPUP then this field
     * optional.
     ****************************************************************/

    if( PIN_POID_IS_TYPE_ONLY(topup_pdp))
    {
        optional = 0;
    }

    topup_amt = (pin_decimal_t*) PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TOPUP_AMT, optional, ebufp);
    topup_cap = (pin_decimal_t*) PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TOPUP_CAP, 1, ebufp);


    if( topup_amt )
    {
        if (!PIN_ERR_IS_ERR(ebufp))
        {
            if( pbo_decimal_sign(topup_amt,ebufp) <= 0 )     
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                (char *)"bad topup amt in the fm_cust_validate_standard_topup");
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                            PIN_ERR_BAD_VALUE, PIN_FLD_TOPUP_AMT, 0, 0);
            }
        }
    }
    if( topup_cap )
    {
        if (!PIN_ERR_IS_ERR(ebufp))
        {
            if(  pbo_decimal_sign(topup_cap,ebufp) < 0 )
            {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                            PIN_ERR_BAD_VALUE, PIN_FLD_TOPUP_CAP, 0, 0);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                            "topup cap is less than zero", ebufp);
          }
        }
    }

    if( topup_amt && topup_cap )
    {
        if (!PIN_ERR_IS_ERR(ebufp))
        {
            if( pbo_decimal_compare(topup_amt, topup_cap,ebufp) == 1 ){
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                            PIN_ERR_BAD_VALUE, PIN_FLD_TOPUP_CAP, 0, 0); 
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,        
                            " topup amt exceeds topup cap ",       
                            ebufp);
            }
        }   
    }
    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_cust_validate_standard_topup error", ebufp);
    }
    return;
}

/****************************************************************
 * Function : fm_cust_validate_sponsored_topup
 * Description : This function will validate the input data for
 *               sponsored topup.
 ****************************************************************/

static void fm_cust_validate_sponsored_topup(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *input_flistp,
        pin_errbuf_t            *ebufp)
{
    pin_flist_t     *topupinfo_flistp    = NULL;
    pin_flist_t     *grptopupinfo_flistp = NULL;
    pin_flist_t     *grptopuplmt_flistp  = NULL;
    pin_flist_t     *grptopupmem_flistp  = NULL;
    pin_flist_t     *tmp_flistp          = NULL;
    pin_flist_t     *args1_flistp        = NULL;
    pin_flist_t     *args2_flistp        = NULL;
    pin_flist_t     *member_flistp       = NULL;
    pin_flist_t     *reslt_flistp        = NULL;
    pin_flist_t     *s_flistp            = NULL;
    pin_flist_t     *r_flistp            = NULL;

    poid_t          *parent_pdp          = NULL;
    poid_t          *topup_pdp           = NULL;
    poid_t          *acct_pdp            = NULL;
    poid_t          *grpobj_pdp          = NULL;
    poid_t          *s_pdp               = NULL;
    poid_t          *grp_pdp             = NULL;
    poid_t          *balgrp_pdp          = NULL;

    int32           *status              = NULL;
    int32           *topup_int           = NULL;
    int32           search_flag          = SRCH_DISTINCT;
    int64           db                   = 0;
    char            *s_tmplt             = NULL;
    char            *in_group_name       = NULL;
    char            *db_group_name       = NULL;
    
    pin_decimal_t   *topup_amt           = NULL;
    pin_decimal_t   *topup_cap           = NULL;

    if (PIN_ERR_IS_ERR(ebufp))    
        return;        
    PIN_ERR_CLEAR_ERR(ebufp); 

    topupinfo_flistp = PIN_FLIST_ELEM_GET(input_flistp, PIN_FLD_TOPUP_INFO, 0,
                    0, ebufp);
    grptopupinfo_flistp = PIN_FLIST_ELEM_GET(topupinfo_flistp, PIN_FLD_GROUP_TOPUP_INFO, 0,
                    0, ebufp);
    grptopuplmt_flistp = PIN_FLIST_ELEM_GET(grptopupinfo_flistp, PIN_FLD_GROUP_TOPUP_LIMITS, 0,
                    0, ebufp);
    grptopupmem_flistp = PIN_FLIST_ELEM_GET(grptopupinfo_flistp, PIN_FLD_GROUP_TOPUP_MEMBERS, 0,
                    0, ebufp);

    parent_pdp = PIN_FLIST_FLD_GET(topupinfo_flistp, PIN_FLD_PARENT, 0, ebufp);
    topup_pdp = PIN_FLIST_FLD_GET(topupinfo_flistp, PIN_FLD_POID, 0, ebufp);
    acct_pdp = PIN_FLIST_FLD_GET(topupinfo_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
    db = PIN_POID_GET_DB(parent_pdp);

    /****************************************************************
     * Read the Parent account object.
     ****************************************************************/
    tmp_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, parent_pdp, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, NULL, ebufp);

    PCM_OP(ctxp, PCM_OP_READ_FLDS, flags, tmp_flistp, &r_flistp, ebufp);

    /****************************************************************
     * Check the payer's account status.
     ****************************************************************/
    status = (int32 *) PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STATUS, 0, ebufp);
    if( status && (*status == PIN_STATUS_CLOSED ))
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                    (char *)"payers account is close");
        pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);
    }
    PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&tmp_flistp, NULL);

    /****************************************************************
     * Read the member account's account object.
     ****************************************************************/
    tmp_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, acct_pdp, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, NULL, ebufp);

    PCM_OP(ctxp, PCM_OP_READ_FLDS, flags, tmp_flistp, &r_flistp, ebufp);

    /****************************************************************
     * Check the member account's account status.
     ****************************************************************/
    status =(int32 *) PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STATUS, 0, ebufp);
    if (!PIN_ERR_IS_ERR(ebufp))
    {
        if( status && *status == PIN_STATUS_CLOSED )
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                    (char *)"members account's account is close");
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);
        }
    }
    PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&tmp_flistp, NULL);

    topup_amt = (pin_decimal_t*) PIN_FLIST_FLD_GET(grptopuplmt_flistp, PIN_FLD_TOPUP_AMT, 0, ebufp);
    topup_cap = (pin_decimal_t*) PIN_FLIST_FLD_GET(grptopuplmt_flistp, PIN_FLD_TOPUP_CAP, 0, ebufp);
    topup_int = (int32 *) PIN_FLIST_FLD_GET(grptopupmem_flistp, PIN_FLD_TOPUP_INTERVAL, 0, ebufp);

    /****************************************************************
     * Do sanity check on payer account can not sponser itself
     ****************************************************************/
    if(PIN_POID_COMPARE(parent_pdp, acct_pdp, 0, ebufp) == 0)
    {
        if (!PIN_ERR_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                      (char *)"Payer should not add himself to the his group");
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_PARENT, 0, 0);
        }
    }

    /****************************************************************
     * Do sanity check on amount ,cap and topup interval
     ****************************************************************/
    if( pbo_decimal_sign(topup_amt,ebufp) <= 0 )
    {
        if (!PIN_ERR_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                         (char *)"Topup amount is less than zero");
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, PIN_FLD_TOPUP_AMT, 0, 0);
        }
    }
    if( pbo_decimal_sign(topup_cap,ebufp) <= 0)
    {
        if (!PIN_ERR_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                    (char *)"Topup cap is less than zero");
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, PIN_FLD_TOPUP_AMT, 0, 0);
        }
    }
    if( pbo_decimal_compare(topup_amt,topup_cap,ebufp) == 1)
    {
        if (!PIN_ERR_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                    (char *)"Topup amount is greater than cap amount");
            pin_set_err(ebufp, PIN_ERRLOC_FM,
                    PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, PIN_FLD_TOPUP_AMT, 0, 0);
        }
    }
    if(topup_int)
    {
        if( *topup_int <= 0)
        {
            if (!PIN_ERR_IS_ERR(ebufp))
            {
               PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                         (char *)"Topup interval is negative or zero");
               pin_set_err(ebufp, PIN_ERRLOC_FM,
                         PIN_ERRCLASS_SYSTEM_DETERMINATE,
                         PIN_ERR_BAD_VALUE, PIN_FLD_TOPUP_INTERVAL, 0, 0);
            }
        }
    }

    /****************************************************************
     * If any of the above sanity check fails, return
     ****************************************************************/

    if (!PIN_ERR_IS_ERR(ebufp))    
    {
        grp_pdp = PIN_FLIST_FLD_GET(grptopupinfo_flistp, PIN_FLD_POID, 0,ebufp);
        if( PIN_POID_IS_TYPE_ONLY(topup_pdp) || PIN_POID_IS_TYPE_ONLY(grp_pdp) )
        {
          /***************************************************
           * The call is from PCM_OP_CUST_CREATE_TOPUP
           **************************************************/


          /****************************************************************
           * Find out whether the member account is already have the 
           * sponsored topup.
           ****************************************************************/
         
          s_pdp = PIN_POID_CREATE(db, "/search", 0, ebufp);
          s_flistp = PIN_FLIST_CREATE(ebufp);
          s_tmplt = " select X from /topup where F1 = V1 ";
          PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
          PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_tmplt, ebufp);
          PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &search_flag, ebufp);

          args1_flistp = PIN_FLIST_CREATE(ebufp);
          PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
        
          reslt_flistp = PIN_FLIST_CREATE(ebufp);
          PIN_FLIST_FLD_SET(reslt_flistp, PIN_FLD_GROUP_OBJ, NULL, ebufp);

          PIN_FLIST_ELEM_PUT(s_flistp, args1_flistp, PIN_FLD_ARGS,  1, ebufp);
          PIN_FLIST_ELEM_PUT(s_flistp, reslt_flistp, PIN_FLD_RESULTS, 0, ebufp);        

          PCM_OP(ctxp, PCM_OP_SEARCH, flags, s_flistp, &r_flistp, ebufp);


          tmp_flistp = PIN_FLIST_ELEM_TAKE(r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
          PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
          PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
          if( tmp_flistp != NULL )
          {
              grpobj_pdp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_GROUP_OBJ, 1, ebufp);
              PIN_FLIST_DESTROY_EX(&tmp_flistp, NULL);
              if( !PIN_POID_IS_NULL(grpobj_pdp) )
              {
                  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                      (char *)"Member is already in some other sponsored topup group");
                  pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                    PIN_ERR_BAD_VALUE, PIN_FLD_ACCOUNT_OBJ, 0, 0);
              }
              PIN_POID_DESTROY(grpobj_pdp, NULL);
          }
        }
        else
        {
          /***************************************************
           * The call is from PCM_OP_CUST_MODIFY_TOPUP
           **************************************************/
          /****************************************************************
           * Check the status of member account's sponsore topup status.
           * If it is close then it is any error. Also check if the group
           * name in DB and input matches
           ****************************************************************/
        
          tmp_flistp = PIN_FLIST_ELEM_GET(grptopupinfo_flistp, PIN_FLD_GROUP_TOPUP_MEMBERS, PIN_ELEMID_ANY, 0, ebufp);
          balgrp_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
          in_group_name= PIN_FLIST_FLD_GET(grptopupinfo_flistp, PIN_FLD_NAME, 0, ebufp);

          s_pdp = PIN_POID_CREATE(db, "/search", 0, ebufp);
          s_flistp = PIN_FLIST_CREATE(ebufp);
          s_tmplt = " select X from /group/topup where F1 = V1 and F2 = V2 ";
          PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
          PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_tmplt, ebufp);
          PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &search_flag, ebufp);

          args1_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
          PIN_FLIST_FLD_SET(args1_flistp, PIN_FLD_POID, grp_pdp, ebufp);

          args2_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
          member_flistp = PIN_FLIST_ELEM_ADD(args2_flistp, PIN_FLD_GROUP_TOPUP_MEMBERS, PIN_ELEMID_ANY, ebufp);
          PIN_FLIST_FLD_SET(member_flistp, PIN_FLD_BAL_GRP_OBJ, balgrp_pdp, ebufp);

          reslt_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
          PIN_FLIST_FLD_SET(reslt_flistp, PIN_FLD_NAME, NULL, ebufp);
          tmp_flistp =  PIN_FLIST_ELEM_ADD(reslt_flistp, PIN_FLD_GROUP_TOPUP_MEMBERS, PIN_ELEMID_ANY, ebufp);
          PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, NULL, ebufp);

          PCM_OP(ctxp, PCM_OP_SEARCH, flags, s_flistp, &r_flistp, ebufp);

          PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
       
          reslt_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
          db_group_name = PIN_FLIST_FLD_GET(reslt_flistp, PIN_FLD_NAME, 0, ebufp);
        
          if( db_group_name && in_group_name )
          {
              if( strcmp(db_group_name, in_group_name)!= 0) 
              {
                  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                              (char *)"group name in Input Flist and Database does not match");
                  pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_BAD_VALUE, PIN_FLD_NAME, 0, 0);
              }
          }
            
          member_flistp = PIN_FLIST_ELEM_GET(reslt_flistp, PIN_FLD_GROUP_TOPUP_MEMBERS, PIN_ELEMID_ANY, 0,ebufp);
          status = PIN_FLIST_FLD_GET(member_flistp, PIN_FLD_STATUS, 0, ebufp);
          if( status != NULL )
          {
              if( *status == PIN_STATUS_CLOSED )
              {
                  PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
                  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                                (char *)"member account group topup status is close");
                  pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);
              }
          }
         PIN_FLIST_DESTROY_EX(&r_flistp, NULL); 
       }
    }
    if (PIN_ERR_IS_ERR(ebufp))
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_cust_validate_sponsored_topup error",
                        ebufp);
    return;
}
