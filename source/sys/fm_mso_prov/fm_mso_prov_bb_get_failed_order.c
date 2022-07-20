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
* 0.1    |15/10/2014 |Someshwar         |  BB Provisioning : Failover management 
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_prov_bb_get_failed_orders.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
#endif

#define _GNU_SOURCE 1


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


/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_prov_bb_get_failed_orders(
        cm_nap_connection_t    *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_prov_bb_get_failed_orders(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        int32                   flags,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PROV_BB_GET_FAILED_ORDERS command
 *******************************************************************/
void
op_mso_prov_bb_get_failed_orders(
        cm_nap_connection_t    *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
    pin_flist_t                 *r_flistp = NULL;
    pcm_context_t               *ctxp = connp->dm_ctx;

    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_PROV_BB_GET_FAILED_ORDERS) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_prov_bb_get_failed_orders", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_prov_bb_get_failed_orders input flist", in_flistp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_prov_bb_get_failed_orders(ctxp, in_flistp, flags, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        *ret_flistpp = (pin_flist_t *)NULL;
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_prov_bb_get_failed_orders error", ebufp);
    } 
    else 
    {
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_prov_bb_get_failed_orders return flist", *ret_flistpp);
    }

    return;
}

/*******************************************************************
 * fm_mso_prov_get_failed_orders()
 *
 * Perfomrs following action:
 * 1. call action specific function to enrich the input from service
 *        and package detail
 * 2. get provisioning sequence 
 * 3. create provisioning order /mso_prov_order
 *******************************************************************/
static void
fm_mso_prov_bb_get_failed_orders(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flistp,
    int32                   flags,
    pin_flist_t             **out_flistpp,
    pin_errbuf_t            *ebufp)
{
    int32               *search_flag = NULL;
    char                *action = NULL;
    char                *error_code = NULL;
    char                *error_descr = NULL;
    char                *order_id = NULL;
    time_t              start_t;
    time_t              end_t;
    char                *template = NULL;
    int32               s_flags;
    int64               db_no;
    poid_t              *s_pdp = NULL;
    poid_t              *pdp = NULL;
    poid_t              *acnt_pdp = NULL;
    pin_flist_t         *search_i_flistp = NULL;
    pin_flist_t         *tmp_flistp= NULL;
    char                *start_time = NULL;
    char                *end_time = NULL;
    struct tm           str_time;
    char                *year = NULL;
    char                *month = NULL;
    char                *day = NULL;
    int32               *status = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_get_failed_orders input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    search_flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_prov_bb_get_failed_orders error: required field missing in input flist", ebufp);
        return;
    }

    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    db_no = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);


    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ORDER_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ACTION, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, MSO_FLD_TASK_FAILED, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ERROR_DESCR, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ERROR_CODE, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_CREATED_T, NULL, ebufp);


    /***********************************************************
     * Prepare search input based on the search key 
     ***********************************************************/
    if ( *search_flag ==  SEARCH_BY_ORDER_ID )
    {
        template = "select X from /mso_prov_bb_order where F1 = V1 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        order_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_ID, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, order_id, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_STATUS )
    {
        template = "select X from /mso_prov_bb_order where F1 = V1 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        status = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_STATUS, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, status, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ERROR_CODE )
    {
        template = "select X from /mso_prov_bb_order where F1 = V1 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        error_code = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ERROR_CODE, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ERROR_DESCR )
    {
        template = "select X from /mso_prov_bb_order where F1 like V1 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        error_descr = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ERROR_DESCR, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_PROVISIONING_ACTION )
    {
        template = "select X from /mso_prov_bb_order where F1 = V1 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        action = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACTION, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACTION, action, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ORDER_CREATION_DATE )
    {
        template = "select X from /mso_prov_bb_order where F1 >= V1 and F2 <= V2 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

        start_time = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_START_CREATION_DATE, 0, ebufp);
        end_time = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_END_CREATION_DATE, 0, ebufp);

        /*
         * Convert start time
         */
        year =  strndup(start_time + 0, 4);
        month = strndup(start_time + 4, 2);
        day =   strndup(start_time + 6, 2);

        str_time.tm_year = atoi(year) - 1900 ;
        str_time.tm_mon = atoi(month) - 1;
        str_time.tm_mday = atoi(day);
        str_time.tm_hour = 0;
        str_time.tm_min = 0;
        str_time.tm_sec = 0;
        start_t = mktime(&str_time);

        /*
         * Convert end time
         */
        year =  strndup(end_time + 0, 4);
        month = strndup(end_time + 4, 2);
        day =   strndup(end_time + 6, 2);

        str_time.tm_year = atoi(year) - 1900;
        str_time.tm_mon = atoi(month) - 1;
        str_time.tm_mday = atoi(day);
        str_time.tm_hour = 23;
        str_time.tm_min = 59;
        str_time.tm_sec = 59;
        end_t = mktime(&str_time);

        /*
         * Set start time and end time in search argument
         */
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CREATED_T, &start_t, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CREATED_T, &end_t, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ACCNT_ORDER_ID )
    {
        template = "select X from /mso_prov_bb_order where F1 = V1 and F2 = V2 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        order_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_ID, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, order_id, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ACCNT_STATUS )
    {
        template = "select X from /mso_prov_bb_order where F1 = V1 and F2 = V2";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        status = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_STATUS, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, status, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ACCNT_ERROR_CODE )
    {
        template = "select X from /mso_prov_bb_order where F1 = V1 and F2 = V2 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        error_code = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ERROR_CODE, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ACCNT_ERROR_DESCR )
    {
        template = "select X from /mso_prov_bb_order where F1 like V1 and F2 = V2 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        error_descr = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ERROR_DESCR, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ACCNT_PROVISIONING_ACTION )
    {
        template = "select X from /mso_prov_bb_order where F1 = V1 and F2 = V2 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        action = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACTION, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACTION, action, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ACCNT_ORDER_CREATION_DATE )
    {
        template = "select X from /mso_prov_bb_order where F1 >= V1 and F2 <= V2 and F3 = V3 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

        start_time = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_START_CREATION_DATE, 0, ebufp);
        end_time = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_END_CREATION_DATE, 0, ebufp);

        /*
         * Convert start time
         */
        year =  strndup(start_time + 0, 4);
        month = strndup(start_time + 4, 2);
        day =   strndup(start_time + 6, 2);

        str_time.tm_year = atoi(year) - 1900 ;
        str_time.tm_mon = atoi(month) - 1;
        str_time.tm_mday = atoi(day);
        str_time.tm_hour = 0;
        str_time.tm_min = 0;
        str_time.tm_sec = 0;
        start_t = mktime(&str_time);

        /*
         * Convert end time
         */
        year =  strndup(end_time + 0, 4);
        month = strndup(end_time + 4, 2);
        day =   strndup(end_time + 6, 2);

        str_time.tm_year = atoi(year) - 1900;
        str_time.tm_mon = atoi(month) - 1;
        str_time.tm_mday = atoi(day);
        str_time.tm_hour = 23;
        str_time.tm_min = 59;
        str_time.tm_sec = 59;
        end_t = mktime(&str_time);

        /*
         * Set start time and end time in search argument
         */
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CREATED_T, &start_t, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CREATED_T, &end_t, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
    }
    else
    {
        //set error
        pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
           PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Invalid Search code", ebufp);
        return;
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_get_failed_orders search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        out_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_get_failed_orders search output flist", *out_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}


