/*******************************************************************
 *
 * Copyright (c) 1999, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

 /***************************************************************************************************************
*VERSION |   DATE    |    Author        |               DESCRIPTION                                            *
*--------------------------------------------------------------------------------------------------------------*
* 0.1    |23/01/2014 |Satya Prakash     |  CATV Wholesale rating Agreement Management
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_rate_manage_ws_contract.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/timeb.h>
#include "pin_sys.h"
#include "pin_os.h"
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"

#define FIXED               0
#define PERCENT             1

#define CRETAE_AGREEMENT    1
#define ADD_AGREEMENT       2
#define DEL_AGREEMENT       3
#define MODIFY_AGREEMENT    4
#define VIEW_AGREEMENT      5



/*******************************************************************
 * Functions Defined outside this source file
 *******************************************************************/
extern int32
fm_mso_trans_open(
    pcm_context_t       *ctxp,
    poid_t              *pdp,
    int32               flag,
    pin_errbuf_t        *ebufp);

extern void 
fm_mso_trans_commit(
    pcm_context_t       *ctxp,
    poid_t              *pdp,
    pin_errbuf_t        *ebufp);

extern void 
fm_mso_trans_abort(
    pcm_context_t       *ctxp,
    poid_t              *pdp,
    pin_errbuf_t        *ebufp);

/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_rate_manage_ws_contract(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

static void
fm_mso_rate_manage_ws_contract(
        pcm_context_t       *ctxp,
        pin_flist_t         *in_flistp,
        int32               flags,
        pin_flist_t         **out_flistpp,
        pin_errbuf_t        *ebufp);

static void
fm_mso_rate_create_ws_agreement(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_rate_add_ws_agreement(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_rate_delete_ws_agreement(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_rate_modify_ws_agreement(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_rate_view_ws_agreement(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_rate_get_lco_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *lco_account,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_rate_get_check_lco_agreement_exist(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *lco_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_rate_get_plan_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *lco_account,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

     void
fm_mso_rate_get_ws_product_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *product_name,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_rate_get_lco_agreement_for_plan(
    pcm_context_t           *ctxp,
    poid_t                  *plan_poidp,
    poid_t                  *lco_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_rate_create_ws_contract command
 *******************************************************************/
void
op_mso_rate_manage_ws_contract(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp)
{
    pin_flist_t             *r_flistp = NULL;
    pcm_context_t           *ctxp = connp->dm_ctx;
    poid_t                  *pdp = NULL;
    int32                   t_status;
    int32                   oap_failure = 1;
    int32                   oap_success = 0;

    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_RATE_MANAGE_WS_CONTRACT) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_rate_manage_ws_contract", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_rate_manage_ws_contract input flist", in_flistp);

    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    t_status = fm_mso_trans_open(ctxp, pdp, 1, ebufp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_rate_manage_ws_contract(ctxp, in_flistp, flags, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_rate_manage_ws_contract error", ebufp);

        PIN_ERRBUF_CLEAR(ebufp);

        if (t_status == 0)
        {
            fm_mso_trans_abort(ctxp, pdp, ebufp);
        }

        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);

        if(r_flistp)
            PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    } 
    else 
    {
        if (t_status == 0)
        {
            fm_mso_trans_commit(ctxp, pdp, ebufp);
        }

        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &oap_success, ebufp);
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_CLEAR(ebufp);
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_rate_manage_ws_contract return flist", *ret_flistpp);

    return;
}

/*******************************************************************
 * fm_mso_rate_manage_ws_contract()
 *
 * Perfomrs following action:
 *  Calls flag specific action
 *    Flag = 1, Create LCO agreement
 *    Flag = 2, Add New plan to agreement
 *    Flag = 3, Delete Plan from  agreement
 *    Flag = 4, Modify details for a specitic 
 *                  Plan of the agreement
 *    Flag = 5, View LCO agreement
 ********************************************************************/
 void
fm_mso_rate_manage_ws_contract(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    int32               *flag = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_manage_ws_contract input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp) ) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_rate_manage_ws_contract error: required field missing in input flist", ebufp);
        return;
    }

    if (*flag == CRETAE_AGREEMENT )
    {
        fm_mso_rate_create_ws_agreement(ctxp, in_flistp, flags, out_flistpp, ebufp);
    }
    else if (*flag == ADD_AGREEMENT )
    {
        fm_mso_rate_add_ws_agreement(ctxp, in_flistp, flags, out_flistpp, ebufp);
    }
    else if (*flag == DEL_AGREEMENT )
    {
        fm_mso_rate_delete_ws_agreement(ctxp, in_flistp, flags, out_flistpp, ebufp);
    }
    else if (*flag == MODIFY_AGREEMENT )
    {
        fm_mso_rate_modify_ws_agreement(ctxp, in_flistp, flags, out_flistpp, ebufp);
    }
    else if (*flag == VIEW_AGREEMENT )
    {
        fm_mso_rate_view_ws_agreement(ctxp, in_flistp, flags, out_flistpp, ebufp);
    }

    return;
}


/*******************************************************************
 * fm_mso_rate_create_ws_contract()
 *
 * Perfomrs following action:
 * 1. 
 *******************************************************************/
void
fm_mso_rate_create_ws_agreement(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    int64               db_no;
    pin_flist_t         *lco_obj_r_flistp = NULL;
    pin_flist_t         *aggrement_r_flistp = NULL;
    pin_flist_t         *s_i_flistp = NULL;
    pin_flist_t         *ws_i_flistp = NULL;
    pin_flist_t         *ws_o_flistp = NULL;
    pin_flist_t         *plan_obj_r_flistp = NULL;
    pin_flist_t         *ws_product_obj_r_flistp = NULL;
    pin_flist_t         *plan_flistp = NULL;
    pin_flist_t         *tmp_flistp= NULL;
    pin_flist_t         *tmp1_flistp= NULL;
    pin_flist_t         *sub_flistp= NULL;
    poid_t              *lco_poidp = NULL;
    poid_t              *lco_bal_grp_obj = NULL;
    poid_t              *plan_poidp = NULL;
    poid_t              *deal_obj = NULL;
    poid_t              *ws_product_poidp = NULL;
    poid_t              *ws_svc_obj = NULL;
    poid_t              *pdp = NULL;
    char                *plan_name = NULL;
    char                *lco_account = NULL;
    char                ws_login[32];
    int32               count ;
    int32               bill_cycle = 1 ;
    int64               database;
    pin_cookie_t        cookie = NULL;
    int32               rec_id = 0;
    int32               oap_failure = 1;
    pin_decimal_t       *override_amt = NULL;
    pin_decimal_t       *fix_amountp = NULL;
    pin_decimal_t       *percent_sharep = NULL;
    pin_decimal_t       *quantity = NULL;
    int32               *setlmnt_type = NULL;




    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_create_ws_agreement input flist", in_flistp);

    /***********************************************************
     * Sample Input flist
     ***********************************************************
    *0 PIN_FLD_POID           POID [0] 0.0.0.1 /mso_lco_agreement -1 0
    *0 PIN_FLD_FLAGS           INT [0] 1
    *0 PIN_FLD_ACCOUNT_NO      STR [0] "670791"
    *0 PIN_FLD_PLAN          ARRAY [0] allocated 20, used 7
    *1     PIN_FLD_NAME            STR [0] "Package-Popular-Mumbai"
    *1     PIN_FLD_SETTLEMENT_TYPE   ENUM [0] 1   // 0 Fixed Amount, 1 Percenatage
    *1     PIN_FLD_FIXED_AMOUNT DECIMAL [0] 0
    *1     PIN_FLD_PERCENT      DECIMAL [0] 10
    *1     MSO_FLD_ST_CRITERIA   ENUM [0] 2   // 0 - No Hathway share, 1 - Full share to Hathway, 2 - Shared
    *1     MSO_FLD_ET_CRITERIA   ENUM [0] 0   // 0 - No Hathway share, 1 - Full share to Hathway, 2 - Shared
    *1     MSO_FLD_VAT_CRITERIA  ENUM [0] 1   // 0 - No Hathway share, 1 - Full share to Hathway, 2 - Shared
     ***********************************************************/

    override_amt = pbo_decimal_from_str("0.0",ebufp);
    quantity = pbo_decimal_from_str("1.0",ebufp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    lco_account = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
//    ws_product_name = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PLAN_NAME, 0, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, 0, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp) ) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_rate_create_ws_agreement error: required field missing in input flist", ebufp);
        PIN_ERRBUF_RESET(ebufp);
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52210", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Required parameter missing in input", ebufp);
        return;
    }

    /*
     * Get LCO account poid
     */
    fm_mso_rate_get_lco_poid(ctxp, in_flistp, lco_account, &lco_obj_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(lco_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count != 1)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52201", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid LCO account", ebufp);
        goto CLEANUP;
    }

    tmp_flistp = PIN_FLIST_ELEM_GET(lco_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    lco_poidp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);
    lco_bal_grp_obj = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);

    /*
     * Check LCO agreement exist?
     */
    fm_mso_rate_get_check_lco_agreement_exist(ctxp, in_flistp, lco_poidp, &aggrement_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(aggrement_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count != 0)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52202", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Agreement Already exist for LCO", ebufp);
        goto CLEANUP;
    }

    /*
     * Prepare flist for agreement creation
     */
    s_i_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, s_i_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(s_i_flistp, MSO_FLD_LCO_OBJ, lco_poidp, ebufp);

    /*
     * Loop through PLAN arrary and append it in agreemnet list
     */
    while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (in_flistp,
            PIN_FLD_PLAN, &rec_id, 1,&cookie, ebufp))
                                      != (pin_flist_t *)NULL )
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inside loop"); 
        /*
         * Validate agreement details
         */
        setlmnt_type = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SETTLEMENT_TYPE, 0, ebufp);
        if (!setlmnt_type || !(*setlmnt_type == FIXED || *setlmnt_type == PERCENT ))
        {
            *out_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52204", ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid SETTLEMENT_TYPE", ebufp);
            PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
            goto CLEANUP;
        }

        fix_amountp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_FIXED_AMOUNT, 1, ebufp);
        percent_sharep = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PERCENT, 1, ebufp);

        if ( *setlmnt_type == FIXED && pbo_decimal_is_null(fix_amountp, ebufp) )
        {
            *out_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52205", ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid FIXED_AMOUNT", ebufp);
            PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
            goto CLEANUP;
        }
        else if (*setlmnt_type == PERCENT && pbo_decimal_is_null(percent_sharep,ebufp))
        {
            *out_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52206", ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid PERCENT share", ebufp);
            PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
            goto CLEANUP;
        }

        /*
         * append plan detail in agreemnet list
         */
        PIN_FLIST_ELEM_SET(s_i_flistp, tmp_flistp, PIN_FLD_PLAN, rec_id, ebufp);
        sub_flistp = PIN_FLIST_ELEM_GET(s_i_flistp, PIN_FLD_PLAN, rec_id, 0, ebufp);

        plan_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
        if (strcmp(plan_name,"Entertainment Tax") != 0  )
        {
            /*
             * Get Plan Poid
             */
            fm_mso_rate_get_plan_poid(ctxp, in_flistp, plan_name, &plan_obj_r_flistp, ebufp);
            count = PIN_FLIST_ELEM_COUNT(plan_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
            if (count != 1)
            {
                *out_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
                PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52203", ebufp);
                PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Plan Name", ebufp);
                PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
                goto CLEANUP;
            }
            plan_flistp = PIN_FLIST_ELEM_GET(plan_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
            plan_poidp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PLAN_OBJ, plan_poidp, ebufp);
        }

        PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
    }

    /*
     * Check if errror
     */
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        goto CLEANUP;
    }


    /*
     * Create Agreement Object
     */
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_create_ws_agreement create input flist", s_i_flistp);
    PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, s_i_flistp, out_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_create_ws_agreement  create output flist", *out_flistpp);


//     fm_mso_rate_get_ws_product_poid(ctxp, in_flistp, ws_product_name, &ws_product_obj_r_flistp, ebufp);
//     count = PIN_FLIST_ELEM_COUNT(ws_product_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
//     if (count != 1)
//     {
//         *out_flistpp = PIN_FLIST_CREATE(ebufp);
//         PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
//         PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
//         PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52203", ebufp);
//         PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Plan Name", ebufp);
//         PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
//         goto CLEANUP;
//     }
//     plan_flistp = PIN_FLIST_ELEM_GET(ws_product_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
//     ws_product_poidp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_POID, 0, ebufp);

    /*
     * Create Wholesale service for LCO
     */
    database = PIN_POID_GET_DB(lco_poidp); 
    ws_svc_obj = (poid_t *)PIN_POID_CREATE(database, "/service/settlement/ws", -1, ebufp);
    pdp = (poid_t *)PIN_POID_CREATE(database, "/plan", -1, ebufp);
//    deal_obj = (poid_t *)PIN_POID_CREATE(database, "/deal", -1, ebufp);

    sprintf(ws_login,"OUT-%s", lco_account);

    ws_i_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_PUT(ws_i_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
    PIN_FLIST_FLD_SET(ws_i_flistp, PIN_FLD_ACCOUNT_OBJ, lco_poidp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(ws_i_flistp, PIN_FLD_SERVICES, 0, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_LOGIN, ws_login, ebufp); 
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_BAL_GRP_OBJ, (void *)lco_bal_grp_obj, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PASSWD_CLEAR, "XXXX", ebufp);
    PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_SERVICE_OBJ, (void *)ws_svc_obj, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(ws_i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, (void *)lco_bal_grp_obj, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_create_ws_agreement modify customer input flist", ws_i_flistp);
    PCM_OP(ctxp, PCM_OP_CUST_MODIFY_CUSTOMER, 0, ws_i_flistp, &ws_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_create_ws_agreement  modify customer output flist", ws_o_flistp);




CLEANUP:
    PIN_FLIST_DESTROY_EX(&lco_obj_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&aggrement_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&s_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ws_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&ws_o_flistp, NULL);
    
    return;
}

/*******************************************************************
 * fm_mso_rate_add_ws_agreement()
 *
 * Perfomrs following action:
 * 1. 
 *******************************************************************/
void
fm_mso_rate_add_ws_agreement(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    int64               db_no;
    pin_flist_t         *lco_obj_r_flistp = NULL;
    pin_flist_t         *aggrement_r_flistp = NULL;
    pin_flist_t         *s_i_flistp = NULL;
    pin_flist_t         *plan_obj_r_flistp = NULL;
    pin_flist_t         *plan_flistp = NULL;
    pin_flist_t         *tmp_flistp= NULL;
    pin_flist_t         *tmp1_flistp= NULL;
    pin_flist_t         *sub_flistp= NULL;
    pin_flist_t         *agreement_flistp= NULL;
    poid_t              *lco_poidp = NULL;
    poid_t              *plan_poidp = NULL;
    poid_t              *agreemement_poidp = NULL;
    poid_t              *r_plan_objp = NULL;
    char                *plan_name = NULL;
    char                *r_plan_name = NULL;
    char                *lco_account = NULL;
    int32               count ;
    pin_cookie_t        cookie = NULL;
    int32               rec_id = 0;
    int32               oap_failure = 1;
    pin_decimal_t       *override_amt = NULL;
    pin_decimal_t       *fix_amountp = NULL;
    pin_decimal_t       *percent_sharep = NULL;
    int32               *setlmnt_type = NULL;
    int32               agreement_exist = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_add_ws_agreement input flist", in_flistp);

    /***********************************************************
     * Sample Input flist
     ***********************************************************
    *0 PIN_FLD_POID           POID [0] 0.0.0.1 /mso_lco_agreement -1 0
    *0 PIN_FLD_ACCOUNT_NO      STR [0] "0.0.0.1-101269"
    *0 PIN_FLD_FLAGS           INT [0] 2
    *0 PIN_FLD_PLAN          ARRAY [0] allocated 20, used 7
    *1     PIN_FLD_NAME            STR [0] "Package Delhi Popular"
    *1     PIN_FLD_SETTLEMENT_TYPE   ENUM [0] 0
    *1     PIN_FLD_FIXED_AMOUNT DECIMAL [0] 100
    *1     PIN_FLD_PERCENT      DECIMAL [0] 0
    *1     MSO_FLD_ST_PERCENT   DECIMAL [0] 10.12
    *1     MSO_FLD_ET_PERCENT   DECIMAL [0] 10.12
    *1     MSO_FLD_VAT_PERCENT  DECIMAL [0] 10.12
     ***********************************************************/

    override_amt = pbo_decimal_from_str("0.0",ebufp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    lco_account = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, 0, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp) ) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_rate_add_ws_agreement error: required field missing in input flist", ebufp);
        PIN_ERRBUF_RESET(ebufp);
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52210", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Required parameter missing in input", ebufp);
        return;
    }

    /*
     * Get LCO account poid
     */
    fm_mso_rate_get_lco_poid(ctxp, in_flistp, lco_account, &lco_obj_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(lco_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count != 1)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52201", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid LCO account", ebufp);
        goto CLEANUP;
    }

    tmp_flistp = PIN_FLIST_ELEM_GET(lco_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    lco_poidp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);

    /*
     *
     */
    tmp_flistp = PIN_FLIST_ELEM_GET (in_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
    plan_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

    /*
     * Validate agreement details
     */
    setlmnt_type = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SETTLEMENT_TYPE, 0, ebufp);
    if (!setlmnt_type || !(*setlmnt_type == FIXED || *setlmnt_type == PERCENT ))
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52204", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid SETTLEMENT_TYPE", ebufp);
        PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
        goto CLEANUP;
    }

    fix_amountp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_FIXED_AMOUNT, 1, ebufp);
    percent_sharep = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PERCENT, 1, ebufp);

    if ( *setlmnt_type == FIXED && pbo_decimal_is_null(fix_amountp, ebufp) )
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52205", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid FIXED_AMOUNT", ebufp);
        PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
        goto CLEANUP;
    }
    else if (*setlmnt_type == PERCENT && pbo_decimal_is_null(percent_sharep,ebufp))
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52206", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid PERCENT share", ebufp);
        PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
        goto CLEANUP;
    }

    /*
     * Get Plan Poid
     */
    if (strcmp(plan_name,"Entertainment Tax") != 0  )
    {
        fm_mso_rate_get_plan_poid(ctxp, in_flistp, plan_name, &plan_obj_r_flistp, ebufp);
        count = PIN_FLIST_ELEM_COUNT(plan_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
        if (count != 1)
        {
            *out_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52203", ebufp);
            PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Plan Name", ebufp);
            PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
            goto CLEANUP;
        }

        plan_flistp = PIN_FLIST_ELEM_GET(plan_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
        plan_poidp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_POID, 0, ebufp);
    }

    /*
     * Check LCO agreement exist for plan?
     */
    fm_mso_rate_get_check_lco_agreement_exist(ctxp, in_flistp, lco_poidp, &aggrement_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(aggrement_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count == 0)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52207", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Agreement Does Not exist for LCO", ebufp);
        goto CLEANUP;
    }

    tmp1_flistp = PIN_FLIST_ELEM_GET(aggrement_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    agreemement_poidp = PIN_FLIST_FLD_GET(tmp1_flistp, PIN_FLD_POID, 0, ebufp);

    while ( (agreement_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp1_flistp,
            PIN_FLD_PLAN, &rec_id, 1,&cookie, ebufp))
                                      != (pin_flist_t *)NULL )
    {
        if (strcmp(plan_name,"Entertainment Tax") != 0)
        {
            r_plan_objp = PIN_FLIST_FLD_GET(agreement_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
            if (!PIN_POID_COMPARE(r_plan_objp, plan_poidp, 0, ebufp))
            {
                agreement_exist = 1;
                break;
            }
        }
        else
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entertainment Tax Plan"); 
            r_plan_name = PIN_FLIST_FLD_GET(agreement_flistp, PIN_FLD_NAME, 0, ebufp);
            if (!strcmp(r_plan_name, plan_name))
            {
                agreement_exist = 1;
                break;
            }
        }
    }

    if (agreement_exist)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52208", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Agreement Already exist for the Plan", ebufp);
        goto CLEANUP;
    }

    /*
     * Prepare flist for agreement creation
     */
    s_i_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(s_i_flistp, PIN_FLD_POID, (void *)agreemement_poidp, ebufp);

    /*
     * append plan detail in agreemnet list
     */
    PIN_FLIST_ELEM_SET(s_i_flistp, tmp_flistp, PIN_FLD_PLAN, PIN_ELEMID_ASSIGN, ebufp);
    sub_flistp = PIN_FLIST_ELEM_GET(s_i_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 0, ebufp);
    PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PLAN_OBJ, plan_poidp, ebufp);

    /*
     * Check if errror
     */
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        goto CLEANUP;
    }

    /*
     * Create Agreement Object
     */
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_add_ws_agreement modify input flist", s_i_flistp);
    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_ADD_ENTRY, s_i_flistp, out_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_add_ws_agreement modify output flist", *out_flistpp);


CLEANUP:
    PIN_FLIST_DESTROY_EX(&lco_obj_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&aggrement_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&s_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);

    return;
}


/*******************************************************************
 * fm_mso_rate_delete_ws_agreement()
 *
 * Perfomrs following action:
 * 1. 
 *******************************************************************/
void
fm_mso_rate_delete_ws_agreement(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    int64               db_no;
    pin_flist_t         *lco_obj_r_flistp = NULL;
    pin_flist_t         *aggrement_r_flistp = NULL;
    pin_flist_t         *s_i_flistp = NULL;
    pin_flist_t         *plan_obj_r_flistp = NULL;
    pin_flist_t         *plan_flistp = NULL;
    pin_flist_t         *tmp_flistp= NULL;
    pin_flist_t         *tmp1_flistp= NULL;
    pin_flist_t         *sub_flistp= NULL;
    pin_flist_t         *agreement_flistp= NULL;
    poid_t              *lco_poidp = NULL;
    poid_t              *plan_poidp = NULL;
    poid_t              *agreemement_poidp = NULL;
    poid_t              *r_plan_objp = NULL;
    char                *plan_name = NULL;
    char                *r_plan_name = NULL;
    char                *lco_account = NULL;
    int32               count ;
    pin_cookie_t        cookie = NULL;
    int32               rec_id = 0;
    int32               oap_failure = 1;
    pin_decimal_t       *override_amt = NULL;
    pin_decimal_t       *fix_amountp = NULL;
    pin_decimal_t       *percent_sharep = NULL;
    int32               *setlmnt_type = NULL;
    int32               agreement_exist = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_delete_ws_agreement input flist", in_flistp);

    /***********************************************************
     * Sample Input flist
     ***********************************************************
    *0 PIN_FLD_POID           POID [0] 0.0.0.1 /mso_lco_agreement -1 0
    *0 PIN_FLD_ACCOUNT_NO      STR [0] "0.0.0.1-101269"
    *0 PIN_FLD_FLAGS           INT [0] 3
    *0 PIN_FLD_PLAN          ARRAY [0] allocated 20, used 7
    *1     PIN_FLD_NAME            STR [0] "Package Delhi Popular"
     ***********************************************************/

    override_amt = pbo_decimal_from_str("0.0",ebufp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    lco_account = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
    plan_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp) ) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_rate_modify_ws_agreement error: required field missing in input flist", ebufp);
        PIN_ERRBUF_RESET(ebufp);
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52210", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Required parameter missing in input", ebufp);
        return;
    }

    /*
     * Get LCO account poid
     */
    fm_mso_rate_get_lco_poid(ctxp, in_flistp, lco_account, &lco_obj_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(lco_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count != 1)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52201", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid LCO account", ebufp);
        goto CLEANUP;
    }

    tmp_flistp = PIN_FLIST_ELEM_GET(lco_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    lco_poidp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);


//    /*
//     * Get Plan Poid
//     */
//    fm_mso_rate_get_plan_poid(ctxp, in_flistp, plan_name, &plan_obj_r_flistp, ebufp);
//    count = PIN_FLIST_ELEM_COUNT(plan_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
//    if (count != 1)
//    {
//        *out_flistpp = PIN_FLIST_CREATE(ebufp);
//        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
//        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
//        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "42007", ebufp);
//        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Plan Name", ebufp);
//        PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
//        goto CLEANUP;
//    }
//
//    plan_flistp = PIN_FLIST_ELEM_GET(plan_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
//    plan_poidp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_POID, 0, ebufp);


    /*
     * Check LCO agreement exist for plan?
     */
    fm_mso_rate_get_check_lco_agreement_exist(ctxp, in_flistp, lco_poidp, &aggrement_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(aggrement_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count == 0)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52207", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Agreement Does Not exist for LCO", ebufp);
        goto CLEANUP;
    }

    tmp1_flistp = PIN_FLIST_ELEM_GET(aggrement_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    agreemement_poidp = PIN_FLIST_FLD_GET(tmp1_flistp, PIN_FLD_POID, 0, ebufp);

    while ( (agreement_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp1_flistp,
            PIN_FLD_PLAN, &rec_id, 1,&cookie, ebufp))
                                      != (pin_flist_t *)NULL )
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inside loop"); 
        r_plan_name = PIN_FLIST_FLD_GET(agreement_flistp, PIN_FLD_NAME, 0, ebufp);
        if (!strcmp(r_plan_name, plan_name))
        {
            agreement_exist = 1;
            break;
        }
    }

    if (!agreement_exist)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52209", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Agreement does not exist for the Plan", ebufp);
        goto CLEANUP;
    }

    /*
     * Prepare flist for agreement creation
     */
    s_i_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(s_i_flistp, PIN_FLD_POID, (void *)agreemement_poidp, ebufp);
    PIN_FLIST_ELEM_SET(s_i_flistp, NULL, PIN_FLD_PLAN, rec_id, ebufp);

    /*
     * Check if errror
     */
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        goto CLEANUP;
    }

    /*
     * Create Agreement Object
     */
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_modify_ws_agreement delete input flist", s_i_flistp);
    PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, s_i_flistp, out_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_modify_ws_agreement delete output flist", *out_flistpp);


CLEANUP:
    PIN_FLIST_DESTROY_EX(&lco_obj_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&aggrement_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&s_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);

    return;
}

/*******************************************************************
 * fm_mso_rate_modify_ws_agreement()
 *
 * Perfomrs following action:
 * 1. 
 *******************************************************************/
void
fm_mso_rate_modify_ws_agreement(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    int64               db_no;
    pin_flist_t         *lco_obj_r_flistp = NULL;
    pin_flist_t         *aggrement_r_flistp = NULL;
    pin_flist_t         *s_i_flistp = NULL;
    pin_flist_t         *plan_obj_r_flistp = NULL;
    pin_flist_t         *plan_flistp = NULL;
    pin_flist_t         *tmp_flistp= NULL;
    pin_flist_t         *tmp1_flistp= NULL;
    pin_flist_t         *sub_flistp= NULL;
    pin_flist_t         *agreement_flistp= NULL;
    poid_t              *lco_poidp = NULL;
    poid_t              *plan_poidp = NULL;
    poid_t              *agreemement_poidp = NULL;
    poid_t              *r_plan_objp = NULL;
    char                *plan_name = NULL;
    char                *r_plan_name = NULL;
    char                *lco_account = NULL;
    int32               count ;
    pin_cookie_t        cookie = NULL;
    int32               rec_id = 0;
    int32               oap_failure = 1;
    pin_decimal_t       *override_amt = NULL;
    pin_decimal_t       *fix_amountp = NULL;
    pin_decimal_t       *percent_sharep = NULL;
    int32               *setlmnt_type = NULL;
    int32               agreement_exist = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_modify_ws_agreement input flist", in_flistp);

    /***********************************************************
     * Sample Input flist
     ***********************************************************
    *0 PIN_FLD_POID           POID [0] 0.0.0.1 /mso_lco_agreement -1 0
    *0 PIN_FLD_ACCOUNT_NO      STR [0] "0.0.0.1-101269"
    *0 PIN_FLD_FLAGS           INT [0] 4
    *0 PIN_FLD_PLAN          ARRAY [0] allocated 20, used 7
    *1     PIN_FLD_NAME            STR [0] "Package Delhi Popular"
    *1     PIN_FLD_SETTLEMENT_TYPE   ENUM [0] 0
    *1     PIN_FLD_FIXED_AMOUNT DECIMAL [0] 100
    *1     PIN_FLD_PERCENT      DECIMAL [0] 0
    *1     MSO_FLD_ST_PERCENT   DECIMAL [0] 10.12
    *1     MSO_FLD_ET_PERCENT   DECIMAL [0] 10.12
    *1     MSO_FLD_VAT_PERCENT  DECIMAL [0] 10.12
     ***********************************************************/

    override_amt = pbo_decimal_from_str("0.0",ebufp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    lco_account = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
    plan_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp) ) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_rate_modify_ws_agreement error: required field missing in input flist", ebufp);
        PIN_ERRBUF_RESET(ebufp);
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52210", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Required parameter missing in input", ebufp);
        return;
    }

    /*
     * Get LCO account poid
     */
    fm_mso_rate_get_lco_poid(ctxp, in_flistp, lco_account, &lco_obj_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(lco_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count != 1)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52201", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid LCO account", ebufp);
        goto CLEANUP;
    }

    tmp_flistp = PIN_FLIST_ELEM_GET(lco_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    lco_poidp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);

    /*
     * Validate agreement details
     */
    tmp_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
    setlmnt_type = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SETTLEMENT_TYPE, 0, ebufp);
    if (!setlmnt_type || !(*setlmnt_type == FIXED || *setlmnt_type == PERCENT ))
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "42007", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid SETTLEMENT_TYPE", ebufp);
        PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
        goto CLEANUP;
    }

    fix_amountp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_FIXED_AMOUNT, 1, ebufp);
    percent_sharep = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PERCENT, 1, ebufp);

    if ( *setlmnt_type == FIXED && pbo_decimal_is_null(fix_amountp, ebufp) )
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52205", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid FIXED_AMOUNT", ebufp);
        PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
        goto CLEANUP;
    }
    else if (*setlmnt_type == PERCENT && pbo_decimal_is_null(percent_sharep,ebufp))
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52206", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid PERCENT share", ebufp);
        PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
        goto CLEANUP;
    }

//    /*
//     * Get Plan Poid
//     */
//    fm_mso_rate_get_plan_poid(ctxp, in_flistp, plan_name, &plan_obj_r_flistp, ebufp);
//    count = PIN_FLIST_ELEM_COUNT(plan_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
//    if (count != 1)
//    {
//        *out_flistpp = PIN_FLIST_CREATE(ebufp);
//        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
//        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
//        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "42007", ebufp);
//        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Plan Name", ebufp);
//        PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
//        goto CLEANUP;
//    }
//
//    plan_flistp = PIN_FLIST_ELEM_GET(plan_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
//    plan_poidp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_POID, 0, ebufp);


    /*
     * Check LCO agreement exist for plan?
     */
    fm_mso_rate_get_check_lco_agreement_exist(ctxp, in_flistp, lco_poidp, &aggrement_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(aggrement_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count == 0)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52207", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Agreement Does Not exist for LCO", ebufp);
        goto CLEANUP;
    }

    tmp1_flistp = PIN_FLIST_ELEM_GET(aggrement_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    agreemement_poidp = PIN_FLIST_FLD_GET(tmp1_flistp, PIN_FLD_POID, 0, ebufp);

    while ( (agreement_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp1_flistp,
            PIN_FLD_PLAN, &rec_id, 1,&cookie, ebufp))
                                      != (pin_flist_t *)NULL )
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inside loop"); 
        r_plan_name = PIN_FLIST_FLD_GET(agreement_flistp, PIN_FLD_NAME, 0, ebufp);
        if (!strcmp(r_plan_name, plan_name))
        {
            agreement_exist = 1;
            break;
        }
    }

    if (!agreement_exist)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52209", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Agreement does not exist for the Plan", ebufp);
        goto CLEANUP;
    }

    /*
     * Prepare flist for agreement creation
     */
    s_i_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(s_i_flistp, PIN_FLD_POID, (void *)agreemement_poidp, ebufp);

    /*
     * append plan detail in agreemnet list
     */
    PIN_FLIST_ELEM_SET(s_i_flistp, tmp_flistp, PIN_FLD_PLAN, rec_id, ebufp);
    sub_flistp = PIN_FLIST_ELEM_GET(s_i_flistp, PIN_FLD_PLAN, rec_id, 0, ebufp);
    PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PLAN_OBJ, plan_poidp, ebufp);

    /*
     * Check if errror
     */
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        goto CLEANUP;
    }

    /*
     * Create Agreement Object
     */
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_modify_ws_agreement modify input flist", s_i_flistp);
    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, s_i_flistp, out_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_modify_ws_agreement modify output flist", *out_flistpp);


CLEANUP:
    PIN_FLIST_DESTROY_EX(&lco_obj_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&aggrement_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&s_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);

    return;
}

/*******************************************************************
 * fm_mso_rate_view_ws_agreement()
 *
 * Perfomrs following action:
 * 1. 
 *******************************************************************/
void 
fm_mso_rate_view_ws_agreement(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    pin_flist_t         *lco_obj_r_flistp = NULL;
    pin_flist_t         *tmp_flistp= NULL;
    pin_flist_t         *tmp1_flistp= NULL;
    poid_t              *lco_poidp = NULL;
    char                *lco_account = NULL;
    int32               count ;
    int32               oap_failure = 1;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_view_ws_agreement input flist", in_flistp);

    /***********************************************************
     * Sample Input flist
     ***********************************************************
    *0 PIN_FLD_POID           POID [0] 0.0.0.1 /mso_lco_agreement -1 0
    *0 PIN_FLD_ACCOUNT_NO      STR [0] "0.0.0.1-101269"
    *0 PIN_FLD_FLAGS           INT [0] 5
     ***********************************************************/


    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    lco_account = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp) ) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_rate_modify_ws_agreement error: required field missing in input flist", ebufp);
        PIN_ERRBUF_RESET(ebufp);
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52210", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Required parameter missing in input", ebufp);
        return;
    }

    /*
     * Get LCO account poid
     */
    fm_mso_rate_get_lco_poid(ctxp, in_flistp, lco_account, &lco_obj_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(lco_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count != 1)
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52201", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Invalid LCO account", ebufp);
        goto CLEANUP;
    }

    tmp_flistp = PIN_FLIST_ELEM_GET(lco_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    lco_poidp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);


    /*
     * Check LCO agreement exist for plan?
     */
    fm_mso_rate_get_check_lco_agreement_exist(ctxp, in_flistp, lco_poidp, out_flistpp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(*out_flistpp, PIN_FLD_RESULTS, ebufp);
    if (count == 0)
    {
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_CODE, "52207", ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ERROR_DESCR, "Agreement Does Not exist for LCO", ebufp);
        goto CLEANUP;
    }


    /*
     * Check if errror
     */
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        goto CLEANUP;
    }


CLEANUP:
    PIN_FLIST_DESTROY_EX(&lco_obj_r_flistp, NULL);

    return;
}


/*******************************************************************************
 * fm_mso_rate_get_lco_poid()
 *
 * Description:
 * Search LCO account no  from /account object
 ******************************************************************************/
void
fm_mso_rate_get_lco_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *lco_account,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    char                    *template =  NULL;
    int32                   s_flags = 0;
    int64                   database = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /account where F1 = V1 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_NO, lco_account, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_lco_poid search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_lco_poid search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}



/*******************************************************************************
 * fm_mso_rate_get_plan_poid()
 *
 * Description:
 * Search Plan name from /plan object
 ******************************************************************************/
void
fm_mso_rate_get_plan_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *plan_name,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    char                    *template =  NULL;
    int32                   s_flags = 0;
    int64                   database = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /plan where F1 = V1 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, plan_name, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_plan_poid search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_plan_poid search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}

/*******************************************************************************
 * fm_mso_rate_get_ws_product_poid()
 *
 * Description:
 * Search Plan name from /plan object
 ******************************************************************************/
void
fm_mso_rate_get_ws_product_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *product_name,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    char                    *template =  NULL;
    int32                   s_flags = 0;
    int64                   database = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /product where F1 = V1 and F2 = V2 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, product_name, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PERMITTED, "/service/settlement/ws", ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_ws_product_poid search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_ws_product_poid search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}

/*******************************************************************************
 * fm_mso_rate_get_check_lco_agreement_exist()
 *
 * Description:
 * Search LCO poid from /mso_lco_agreement object
 ******************************************************************************/
void
fm_mso_rate_get_check_lco_agreement_exist(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *lco_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    char                    *template =  NULL;
    int32                   s_flags = 0;
    int64                   database = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /mso_lco_agreement where F1 = V1 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_LCO_OBJ, lco_poidp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_check_lco_agreement_exist search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_check_lco_agreement_exist search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}

/*******************************************************************************
 * fm_mso_rate_get_check_lco_agreement_exist()
 *
 * Description:
 * Search LCO poid from /mso_lco_agreement object
 ******************************************************************************/
void
fm_mso_rate_get_lco_agreement_for_plan(
    pcm_context_t           *ctxp,
    poid_t                  *plan_poidp,
    poid_t                  *lco_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *sub_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    char                    *template =  NULL;
    int32                   s_flags = 0;
    int64                   database = 0;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    database = PIN_POID_GET_DB(lco_poidp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /mso_lco_agreement where F1 = V1 and F2 = V2 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_LCO_OBJ, lco_poidp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    sub_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PLAN_OBJ, plan_poidp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_lco_agreement_for_plan search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_lco_agreement_for_plan search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;

}

