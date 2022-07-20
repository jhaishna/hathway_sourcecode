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
* 0.1    |25/01/2014 |Satya Prakash     |  Wholesale Rating Implementation
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_rate_settlement_charges.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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
#include "mso_prov.h"


#define  FIXED 0
#define  PERCENT 1

#define  TAX_INCLUSIVE 0
#define  TAX_EXCLUSIVE 1
#define  TAX_SHARE 2



/*******************************************************************************
 * Functions Defined outside this source file
 ******************************************************************************/
extern int32
fm_mso_trans_open(
    pcm_context_t    *ctxp,
    poid_t        *pdp,
    int32        flag,
    pin_errbuf_t    *ebufp);

extern void 
fm_mso_trans_commit(
    pcm_context_t    *ctxp,
    poid_t        *pdp,
    pin_errbuf_t    *ebufp);

extern void 
fm_mso_trans_abort(
    pcm_context_t    *ctxp, 
    poid_t        *pdp,
    pin_errbuf_t    *ebufp);

/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_rate_settlement_charges(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *in_flistp,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t    *ebufp);

static void
fm_mso_rate_settlement_charges(
    pcm_context_t           *ctxp,
    pin_flist_t        *in_flistp,
    int32            flags,
    pin_flist_t        **out_flistpp,
    pin_errbuf_t    *ebufp);

static void
fm_mso_rate_get_customer_plan_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *offering_obj,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_rate_get_customer_lco_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *cust_pdp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_rate_get_lco_agreement_for_plan(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *lco_poidp,
    poid_t                  *plan_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_rate_get_lco_et_agreement(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *lco_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);


static void
update_settlement_charge_status(
    pcm_context_t           *ctxp,
    poid_t                  *lco_poidp,
    int32                    status,
    pin_errbuf_t            *ebufp);

static void
fm_mso_rate_get_lco_svc_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *lco_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_rate_settlement_charges command
 *******************************************************************/
void
op_mso_rate_settlement_charges(
        cm_nap_connection_t    *connp,
        int32            opcode,
        int32            flags,
        pin_flist_t        *in_flistp,
        pin_flist_t        **ret_flistpp,
        pin_errbuf_t    *ebufp)
{
    pcm_context_t       *ctxp = connp->dm_ctx;
    poid_t              *pdp = NULL;
    int32               t_status;
    int32               status = 2;


    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_RATE_SETTLEMENT_CHARGES) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_rate_settlement_charges", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_rate_settlement_charges input flist", in_flistp);


    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
    t_status = fm_mso_trans_open(ctxp, pdp, 3, ebufp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_rate_settlement_charges(ctxp, in_flistp, flags, ret_flistpp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_rate_settlement_charges error", ebufp);

        PIN_ERRBUF_CLEAR(ebufp);
        status = 2;  //Error
        update_settlement_charge_status(ctxp, pdp, status, ebufp);

        if (t_status == 0)
        {
            fm_mso_trans_commit(ctxp, pdp, ebufp);
        }

        *ret_flistpp = (pin_flist_t *)NULL;

        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_VALUE, 0, 0, 0);
    } 
    else 
    {
        if (t_status == 0)
        {
            fm_mso_trans_commit(ctxp, pdp, ebufp);
        }

        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_rate_settlement_charges return flist", *ret_flistpp);
    }

    return;
}

/*******************************************************************
 * fm_mso_rate_settlement_charges()
 *
 * Perfomrs following action:
 *******************************************************************/
static void
fm_mso_rate_settlement_charges(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flistp,
    int32                   flags,
    pin_flist_t             **out_flistpp,
    pin_errbuf_t            *ebufp)
{
    int32                    status;
    poid_t                  *pdp = NULL;
    int64                   db_no;
    pin_flist_t             *plan_obj_r_flistp = NULL;
    pin_flist_t             *aggrement_r_flistp = NULL;
    pin_flist_t             *lco_obj_r_flistp = NULL;
    pin_flist_t             *lco_svc_r_flistp = NULL;
    pin_flist_t             *s_flistp = NULL;
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *sub_flistp= NULL;
    pin_flist_t             *tmp_flistp= NULL;
    pin_flist_t             *tmp1_flistp= NULL;
    poid_t                  *customer_obj = NULL;
    poid_t                  *offering_obj = NULL;
    poid_t                  *lco_objp = NULL;
    poid_t                  *plan_objp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *lco_svc_objp = NULL;
    pin_decimal_t           *fix_amountp = NULL;
    pin_decimal_t           *percent_sharep = NULL;
    int32                   *st_criteriap = NULL;
    int32                   *et_criteriap = NULL;
    int32                   *vat_criteriap = NULL;
    int32                   *setlmnt_type = NULL;
    pin_decimal_t           *lco_amount = NULL;
    pin_decimal_t           *lco_st = NULL;
    pin_decimal_t           *lco_vat = NULL;
    pin_decimal_t           *lco_et = NULL;
    pin_decimal_t           *cust_amount = NULL;
    pin_decimal_t           *cust_st = NULL;
    pin_decimal_t           *cust_vat = NULL;
    pin_decimal_t           *cust_et = NULL;
    pin_decimal_t           *zero = NULL;
    pin_decimal_t           *hundred = NULL;
    char                    *program_name = "Wholesale_Rating";
    int32                   prerated = 2;
    int32                   resource = 356;
    time_t                  now;
    int32                   count =0;
    int                     i = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_settlement_charges input flist", in_flistp);


    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    customer_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
    offering_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
    cust_amount = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_AMOUNT, 0, ebufp);
    cust_et = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_ET_AMOUNT, 0, ebufp);
    cust_st = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_ST_AMOUNT, 0, ebufp);
    cust_vat = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_VAT_AMOUNT, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp) ) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_rate_settlement_charges error: required field missing in input flist", ebufp);
        return;
    }

    hundred = pbo_decimal_from_str("100.0",ebufp);
    zero = pbo_decimal_from_str("0.0",ebufp);
    lco_st = pbo_decimal_from_str("0.0",ebufp);
    lco_et = pbo_decimal_from_str("0.0",ebufp);
    lco_vat = pbo_decimal_from_str("0.0",ebufp);
    lco_amount = pbo_decimal_from_str("0.0",ebufp);

    /*
     * Get LCO account poid
     */
    fm_mso_rate_get_customer_lco_poid(ctxp, in_flistp, customer_obj, &lco_obj_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(lco_obj_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count == 1)
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(lco_obj_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
        tmp1_flistp = PIN_FLIST_SUBSTR_GET (tmp_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
        lco_objp = PIN_FLIST_FLD_GET(tmp1_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
    }
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, MSO_FLD_LCO_OBJ, 0, 0);
        goto CLEANUP;
    }

    /*
     * Get LCO account poid
     */
    fm_mso_rate_get_lco_svc_poid(ctxp, in_flistp, lco_objp, &lco_svc_r_flistp, ebufp);
    count = PIN_FLIST_ELEM_COUNT(lco_svc_r_flistp, PIN_FLD_RESULTS, ebufp);
    if (count == 1)
    {
        tmp_flistp = PIN_FLIST_ELEM_GET(lco_svc_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
        lco_svc_objp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);
    }
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, PIN_FLD_SERVICE_OBJ, 0, 0);
        goto CLEANUP;
    }

    if (PIN_POID_GET_ID(offering_obj) != 0 )
    {
        /*
         * Read /purchased_product object to get /plan poid
         */
        fm_mso_rate_get_customer_plan_poid(ctxp, in_flistp, offering_obj, &plan_obj_r_flistp, ebufp);
        plan_objp = PIN_FLIST_FLD_GET(plan_obj_r_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp) ) 
        {
            goto CLEANUP;
        }

        /*
         * Get LCO agreement for the plan
         */
        fm_mso_rate_get_lco_agreement_for_plan(ctxp, in_flistp, lco_objp, plan_objp, &aggrement_r_flistp, ebufp);

        count = PIN_FLIST_ELEM_COUNT(aggrement_r_flistp, PIN_FLD_RESULTS, ebufp);
        if (count != 1)
        {
            /*
             * //Not processed as LCO does not have any agreement for this plan
             * May not neccessarily an error 
             */
            status = 3;
            *out_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
            update_settlement_charge_status(ctxp, pdp, status, ebufp);
            goto CLEANUP;
             
        }

        tmp1_flistp = PIN_FLIST_ELEM_GET(aggrement_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 0, ebufp);
        setlmnt_type = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SETTLEMENT_TYPE, 0, ebufp);

        if (*setlmnt_type == FIXED)
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"fixed");
            fix_amountp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_FIXED_AMOUNT, 0, ebufp);
            lco_amount = fix_amountp;
        }
        else
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"percent");
            percent_sharep = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PERCENT, 0, ebufp);
            lco_amount = pbo_decimal_multiply(cust_amount, percent_sharep, ebufp);
            pbo_decimal_divide_assign(lco_amount, hundred, ebufp);
        }

        st_criteriap = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ST_CRITERIA, 0, ebufp);
        et_criteriap = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ET_CRITERIA, 0, ebufp);
        vat_criteriap = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_VAT_CRITERIA, 0, ebufp);

        /*
         * Calculate LCO_ST_TAX share
         */
        if (*st_criteriap == TAX_INCLUSIVE )
        {
            lco_st = zero;
        }
        else if (*st_criteriap == TAX_EXCLUSIVE)
        {
            lco_st = cust_st;
        }
        else if (*st_criteriap == TAX_SHARE)
        {
            lco_st = pbo_decimal_multiply(cust_st, lco_amount, ebufp);
            pbo_decimal_divide_assign(lco_st, cust_amount, ebufp);
        }

        /*
         * Calculate LCO_VAT_TAX share
         */
        if (*vat_criteriap == TAX_INCLUSIVE )
        {
            lco_vat = zero;
        }
        else if (*vat_criteriap == TAX_EXCLUSIVE)
        {
            lco_vat = cust_vat;
        }
        else if (*vat_criteriap == TAX_SHARE)
        {
            lco_vat = pbo_decimal_multiply(cust_vat, lco_amount, ebufp);
            pbo_decimal_divide_assign(lco_vat, cust_amount, ebufp);
        }

        /*
         * Calculate LCO_ET_TAX share
         */
        if (*et_criteriap == TAX_INCLUSIVE )
        {
            lco_et = zero;
        }
        else if (*et_criteriap == TAX_EXCLUSIVE)
        {
            lco_et = cust_et;
        }
        else if (*et_criteriap == TAX_SHARE)
        {
            lco_et = pbo_decimal_multiply(cust_et, lco_amount, ebufp);
            pbo_decimal_divide_assign(lco_et, cust_amount, ebufp);
        }
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"at 8");
    }
    else
    {
        fm_mso_rate_get_lco_et_agreement(ctxp, in_flistp, lco_objp, &aggrement_r_flistp, ebufp);
        count = PIN_FLIST_ELEM_COUNT(aggrement_r_flistp, PIN_FLD_RESULTS, ebufp);
        if (count != 1)
        {
            /*
             * //Not processed as LCO does not have any agreement for this plan
             * May not neccessarily an error 
             */
            status = 3;
            *out_flistpp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
            update_settlement_charge_status(ctxp, pdp, status, ebufp);
            goto CLEANUP;
             
        }

        tmp1_flistp = PIN_FLIST_ELEM_GET(aggrement_r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_GET(tmp1_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 0, ebufp);
        et_criteriap = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ET_CRITERIA, 0, ebufp);

        if (*et_criteriap == TAX_INCLUSIVE )
        {
            lco_et = zero;
        }
        else if (*et_criteriap == TAX_EXCLUSIVE)
        {
            lco_et = cust_et;
        }
        else if (*et_criteriap == TAX_SHARE)
        {
            percent_sharep = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PERCENT, 0, ebufp);
            lco_et = pbo_decimal_multiply(cust_et, percent_sharep, ebufp);
            pbo_decimal_divide_assign(lco_et, hundred, ebufp);
        }
    }


    /*
     * if atleast one impact is non-zero, 
     *      create balance impact for LCO
     */
    if (!pbo_decimal_is_zero(lco_amount, ebufp) || 
        !pbo_decimal_is_zero(lco_et, ebufp) ||
        !pbo_decimal_is_zero(lco_st, ebufp) ||
        !pbo_decimal_is_zero(lco_vat, ebufp) )
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Create Impact for LCO");
        /***********************************************************
         * Generate Pre-rated Event
         ***********************************************************/
        db_no = PIN_POID_GET_DB(pdp); 

        si_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/event/billing/settlement/ws", -1, ebufp);

        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, si_flistp, PIN_FLD_POID, ebufp);
        sub_flistp = PIN_FLIST_ELEM_ADD(si_flistp, PIN_FLD_EVENT, 0, ebufp);
        PIN_FLIST_FLD_PUT(sub_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ACCOUNT_OBJ, lco_objp, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SERVICE_OBJ, lco_svc_objp, ebufp);

        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SYS_DESCR, program_name, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_NAME, program_name, ebufp);

        now = pin_virtual_time(NULL);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_START_T, &now, ebufp);
        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_END_T, &now, ebufp);

        if (!pbo_decimal_is_zero(lco_amount, ebufp))
        {
            tmp_flistp = PIN_FLIST_ELEM_ADD(sub_flistp, PIN_FLD_BAL_IMPACTS, i++, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_IMPACT_TYPE, &prerated, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RESOURCE_ID, &resource, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, lco_objp, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_AMOUNT, lco_amount, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RATE_TAG, "WS", ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TAX_CODE, "WS_MAIN", ebufp);
        }

        if (!pbo_decimal_is_zero(lco_st, ebufp))
        {
            tmp_flistp = PIN_FLIST_ELEM_ADD(sub_flistp, PIN_FLD_BAL_IMPACTS, i++, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_IMPACT_TYPE, &prerated, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RESOURCE_ID, &resource, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, lco_objp, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_AMOUNT, lco_st, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RATE_TAG, "WS", ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TAX_CODE, "WS_ST", ebufp);
        }

        if (!pbo_decimal_is_zero(lco_et, ebufp))
        {
            tmp_flistp = PIN_FLIST_ELEM_ADD(sub_flistp, PIN_FLD_BAL_IMPACTS, i++, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_IMPACT_TYPE, &prerated, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RESOURCE_ID, &resource, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, lco_objp, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_AMOUNT, lco_et, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RATE_TAG, "WS", ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TAX_CODE, "WS_ET", ebufp);
        }

        if (!pbo_decimal_is_zero(lco_vat, ebufp))
        {
            tmp_flistp = PIN_FLIST_ELEM_ADD(sub_flistp, PIN_FLD_BAL_IMPACTS, i, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_IMPACT_TYPE, &prerated, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RESOURCE_ID, &resource, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, lco_objp, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_AMOUNT, lco_vat, ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RATE_TAG, "WS", ebufp);
            PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TAX_CODE, "WS_VAT", ebufp);
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_rate_settlement_charges act_usage input flist ", si_flistp);
        PCM_OP(ctxp, PCM_OP_ACT_USAGE, PCM_OPFLG_CACHEABLE, si_flistp,
            out_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "fm_mso_rate_settlement_charges act_usage output flist", *out_flistpp);
    }
    else
    {
        *out_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *out_flistpp, PIN_FLD_POID, ebufp);
    }

    if (PIN_ERRBUF_IS_ERR(ebufp) ) 
    {
        goto CLEANUP;
    }
    else
    {
        status = 0;  //Processed
        update_settlement_charge_status(ctxp, pdp, status, ebufp);
    }

CLEANUP:
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&lco_obj_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&plan_obj_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&aggrement_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&lco_svc_r_flistp, NULL);

    return;
}

/*******************************************************************************
 * fm_mso_rate_get_customer_lco_poid()
 *
 * Description:
 * Search LCO account no  from /account object
 ******************************************************************************/
void
fm_mso_rate_get_customer_lco_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *cust_pdp,
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
    pin_flist_t             *tmp1_flistp= NULL;

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

    template = "select X from /profile/customer_care where F1 = V1 and profile_t.poid_type='/profile/customer_care' ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, cust_pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    tmp1_flistp = PIN_FLIST_SUBSTR_ADD (tmp_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, MSO_FLD_LCO_OBJ, NULL, ebufp);

//    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_customer_lco_poid search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_customer_lco_poid search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}

/*******************************************************************************
 * fm_mso_rate_get_customer_plan_poid()
 *
 * Description:
 * Search Plan name from /plan object
 ******************************************************************************/
void
fm_mso_rate_get_customer_plan_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *offering_obj,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_POID, offering_obj, ebufp);
    PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_customer_plan_poid PCM_OP_READ_FLDS input flist", si_flistp);
    PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_customer_plan_poid PCM_OP_READ_FLDS output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);

    return;
}


/*******************************************************************************
 * fm_mso_rate_get_lco_et_agreement()
 *
 * Description:
 * Search LCO poid from /mso_lco_agreement object
 ******************************************************************************/
void
fm_mso_rate_get_lco_et_agreement(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *lco_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
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

    s_flags = 768 ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /mso_lco_agreement where F1 = V1 and F2 = V2 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_LCO_OBJ, lco_poidp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_NAME, "Entertainment Tax", ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
//    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_lco_et_agreement search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_lco_et_agreement search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}

/*******************************************************************************
 * fm_mso_rate_get_lco_agreement_for_plan()
 *
 * Description:
 * Search LCO poid from /mso_lco_agreement object
 ******************************************************************************/
void
fm_mso_rate_get_lco_agreement_for_plan(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *lco_poidp,
    poid_t                  *plan_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
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

    s_flags = 768 ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /mso_lco_agreement where F1 = V1 and F2 = V2 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_LCO_OBJ, lco_poidp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_PLAN_OBJ, plan_poidp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
//    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_lco_agreement_for_plan search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_lco_agreement_for_plan search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}

void
update_settlement_charge_status(
    pcm_context_t           *ctxp,
    poid_t                  *pdp,
    int32                    status,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *si_flistp = NULL;
    pin_flist_t             *so_flistp = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }

    si_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
    PIN_FLIST_FLD_SET(si_flistp, PIN_FLD_STATUS, &status, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "update_settlement_charge_status WRITE_FLDS input flist ", si_flistp);
    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, si_flistp,
        &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "update_settlement_charge_status WRITE_FLDS output flist", so_flistp);

    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);

    return;
}



/*******************************************************************************
 * fm_mso_rate_get_lco_svc_poid()
 *
 * Description:
 * Search LCO poid from /mso_lco_agreement object
 ******************************************************************************/
void
fm_mso_rate_get_lco_svc_poid(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    poid_t                  *lco_poidp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *tmp1_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *svc_pdp = NULL;
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

    s_flags = 768 ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /service where F1 = V1 and F2.type = V2 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, lco_poidp, ebufp);

    svc_pdp = (poid_t *)PIN_POID_CREATE(database, "/service/settlement/ws", -1, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_POID, (void *)svc_pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_lco_svc_poid search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_rate_get_lco_svc_poid search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}
