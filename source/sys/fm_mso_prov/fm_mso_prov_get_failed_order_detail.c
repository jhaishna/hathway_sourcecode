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
* 0.1    |03/12/2013 |Satya Prakash     |  CATV Provisioning : Fallover management 
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_prov_get_failed_order_detail.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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


/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_prov_get_failed_order_detail(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_prov_get_failed_order_detail(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        int32                   flags,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_prov_get_oap_result(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PROV_GET_FAILED_ORDERS command
 *******************************************************************/
void
op_mso_prov_get_failed_order_detail(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *r_flistp = NULL;
        pcm_context_t           *ctxp = connp->dm_ctx;
        poid_t                  *pdp = NULL;
        int32                   oap_failure = 10;

    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_PROV_GET_FAILED_ORDER_DETAIL) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_prov_get_failed_order_detail", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_prov_get_failed_order_detail input flist", in_flistp);


    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_prov_get_failed_order_detail(ctxp, in_flistp, flags, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_prov_get_failed_order_detail error", ebufp);

        PIN_ERRBUF_RESET(ebufp);
        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);

        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    } 
    else 
    {
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_CLEAR(ebufp);
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
         "op_mso_prov_get_failed_order_detail return flist", *ret_flistpp);

    return;
}

/*******************************************************************
 * fm_mso_prov_get_failed_order_detail()
 *
 * Perfomrs following action:
 * 1. 
 *******************************************************************/
static void
fm_mso_prov_get_failed_order_detail(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flistp,
    int32                   flags,
    pin_flist_t             **out_flistpp,
    pin_errbuf_t            *ebufp)
{

    pin_flist_t             *read_i_flist = NULL;
    pin_flist_t             *read_o_flist = NULL;
    pin_flist_t             *tmp_flistp= NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_get_failed_order_detail input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/


    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_prov_get_failed_order_detail error: required field missing in input flist", ebufp);
        return;
    }

    read_i_flist = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, read_i_flist, PIN_FLD_POID, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_get_failed_order_detail search input flist ", read_i_flist);
    PCM_OP(ctxp, PCM_OP_READ_OBJ, PCM_OPFLG_CACHEABLE, read_i_flist,
        &read_o_flist, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_get_failed_order_detail search output flist", read_o_flist);

    PIN_FLIST_DESTROY_EX(&read_i_flist, NULL);

    /*
     * Convert the BLOB data to flist and return as outuput
     */
    fm_mso_prov_get_oap_result(ctxp, read_o_flist, out_flistpp, ebufp);
    if(read_o_flist)
    	PIN_FLIST_DESTROY_EX(&read_o_flist, NULL);

    return;
}

/*******************************************************************
 * fm_mso_prov_get_oap_result()
 *
 * 1. 
 *******************************************************************/
static void
fm_mso_prov_get_oap_result(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_flist_t             **ret_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *tmp_flistp = NULL;
    pin_flist_t             *r_flistp = NULL;
    pin_flist_t             *sub_flistp = NULL;
    pin_buf_t               *pin_bufp     = NULL;
    pin_cookie_t            cookie = NULL;
    int32                   rec_id = 0;
    void                    *vp;

   /******************************************************
     * Null out results until we have some.
     ******************************************************/
    if ( PIN_ERRBUF_IS_ERR(ebufp) )
            return;
    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_get_oap_result input flist", i_flistp);

    *ret_flistpp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATUS, *ret_flistpp, PIN_FLD_STATUS, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_TASK_FAILED, *ret_flistpp, MSO_FLD_TASK_FAILED, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ERROR_CODE, *ret_flistpp, PIN_FLD_ERROR_CODE, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ERROR_DESCR, *ret_flistpp, PIN_FLD_ERROR_DESCR, ebufp);

    pin_bufp = (pin_buf_t *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_INPUT_FLIST, 0, ebufp );
    if (pin_bufp) 
    { 
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pin_bufp->data);
        /* Convert the buffer to a flist */
        pin_str_to_flist((char *)pin_bufp->data, 1, &r_flistp, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_ADD(*ret_flistpp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_CONCAT(tmp_flistp, r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_prov_get_oap_result return flist", *ret_flistpp);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    }
    return;
}

