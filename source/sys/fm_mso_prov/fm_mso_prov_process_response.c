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
* 0.1    |02/12/2013 |Satya Prakash     |  CATV Provisioning - Response Processing
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_prov_process_response.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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
op_mso_prov_process_response(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *in_flistp,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t    *ebufp);

static void
fm_mso_prov_process_response(
    pcm_context_t           *ctxp,
    pin_flist_t        *in_flistp,
    int32            flags,
    pin_flist_t        **out_flistpp,
    pin_errbuf_t    *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PROV_PROCESS_RESPONSE command
 *******************************************************************/
void
op_mso_prov_process_response(
        cm_nap_connection_t    *connp,
        int32            opcode,
        int32            flags,
        pin_flist_t        *in_flistp,
        pin_flist_t        **ret_flistpp,
        pin_errbuf_t    *ebufp)
{
    pin_flist_t         *r_flistp = NULL;
    pcm_context_t       *ctxp = connp->dm_ctx;
    poid_t              *pdp = NULL;
    int32               t_status;


    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_PROV_PROCESS_RESPONSE) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_prov_process_response", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_prov_process_response input flist", in_flistp);


    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    t_status = fm_mso_trans_open(ctxp, pdp, 1, ebufp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_prov_process_response(ctxp, in_flistp, flags, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        if (t_status == 0)
        {
            fm_mso_trans_abort(ctxp, pdp, ebufp);
        }

        *ret_flistpp = (pin_flist_t *)NULL;
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_prov_process_response error", ebufp);
    } 
    else 
    {
        if (t_status == 0)
        {
            fm_mso_trans_commit(ctxp, pdp, ebufp);
        }

        *ret_flistpp = r_flistp;
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_prov_process_response return flist", *ret_flistpp);
    }

    return;
}

/*******************************************************************
 * fm_mso_prov_process_response()
 *
 * Perfomrs following action:
 * 1. Search order_id in /mso_prov_order to get action and service_obj
 *        and package detail
 * 2. loop through the TASK array to check the STATUS of the order
 * 3. if all TASK element has STATUS successful, update order as 
 *     successful. else failed. 
 * 4. for CATV activation, additionally updates CAS_SUBSCRIBER_ID
 *      in service class
 *******************************************************************/
static void
fm_mso_prov_process_response(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flistp,
    int32                   flags,
    pin_flist_t             **out_flistpp,
    pin_errbuf_t            *ebufp)
{
    int32                   *action_flag = NULL;
    int                     *status = NULL;
    int                     *activation_status = NULL;
    char                    *order_id = NULL;
    char                    *error_code = NULL;
    char                    *error_descr = NULL;
    char                    *cas_id = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *service_obj = NULL;
    poid_t                  *order_poid = NULL;
    char                    *service_type = NULL;
    int32                   s_flags;
    int64                   db_no;
    char                    *template = NULL;
    poid_t                  *s_pdp = NULL;
    int32                   count;
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *w_i_flistp = NULL;
    pin_flist_t             *w_o_flistp = NULL;
    pin_flist_t             *u_i_flistp = NULL;
    pin_flist_t             *u_o_flistp = NULL;
    pin_flist_t             *tmp_flistp= NULL;
    pin_flist_t             *task_flistp= NULL;
    char                    *action = NULL;
    int32                   task_id = 0;
    pin_cookie_t            cookie = NULL;
    int32                   prov_status  = MSO_PROV_STATE_SUCCESS;


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_process_response input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    order_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_ID, 0, ebufp);
    count = PIN_FLIST_ELEM_COUNT(in_flistp, MSO_FLD_TASK, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp) ) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_prov_process_response error: required field missing in input flist", ebufp);
        return;
    }

    if (count == 0)
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_NOT_FOUND, MSO_FLD_TASK, 0, 0);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_prov_process_response error: required field missing in input flist", ebufp);
        return;
    }

    /***********************************************************
     * Search provisioning order object
     ***********************************************************/
    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    db_no = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    template = "select X from /mso_prov_order where F1 = V1 ";
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, order_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ACTION, NULL, ebufp);
//    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, NULL, ebufp);


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_process_response search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        out_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_process_response search output flist", *out_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    count = PIN_FLIST_ELEM_COUNT(*out_flistpp, PIN_FLD_RESULTS, ebufp);


    /***********************************************************
     * There should be exactly on matching result
     ***********************************************************/
    if (count == 1)
    {
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "order found");
         status = &prov_status;

        while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT (in_flistp,
                    MSO_FLD_TASK, &task_id, 1,&cookie, ebufp))
                                              != (pin_flist_t *)NULL ) 
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inside TASK loop");
            status = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATUS, 0, ebufp);
            if (*status != MSO_PROV_STATE_SUCCESS)
            {
                error_code = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ERROR_CODE, 0, ebufp);
                error_descr = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ERROR_DESCR, 0, ebufp);
                break;
            }
        }

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "fm_mso_prov_process_response error: required field missing in input flist", ebufp);
            return;
        }

        tmp_flistp = PIN_FLIST_ELEM_GET(*out_flistpp, PIN_FLD_RESULTS, 0, 0, ebufp);

        if (*status == MSO_PROV_STATE_SUCCESS)
        {
            prov_status = MSO_PROV_STATE_SUCCESS;
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Successful at NE");
            service_obj = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp); 
            action = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACTION, 0, ebufp);
            order_poid = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);

            /***********************************************************
             * Update /mso_service_order class with status succesfful
             ***********************************************************/
            w_i_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, order_poid, ebufp);
            PIN_FLIST_FLD_SET( w_i_flistp, PIN_FLD_STATUS, &prov_status, ebufp);

            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_prov_process_response write status input flist ", w_i_flistp);
            PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_CACHEABLE, w_i_flistp,
                &w_o_flistp, ebufp);
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_prov_process_response write status output flist", w_o_flistp);

            PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);

            /***********************************************************
             * Update MSO_FLD_CAS_SUBSCRBER_ID for CATV activation 
             * and CATV preactivation scenario
             ***********************************************************/
            if (strstr(action,"ACTIVATE_SUBSCRIBER_NDS") || strcmp(action, "ACTIVATE_SUBSCRIBER_NGR") == 0)
            {
                task_flistp = PIN_FLIST_ELEM_GET (in_flistp, MSO_FLD_TASK, 1, 1, ebufp);
                if (task_flistp)
                {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "activate subscriber scenario");
                    cas_id = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

                    u_i_flistp = PIN_FLIST_CREATE(ebufp);
                    PIN_FLIST_FLD_SET(u_i_flistp, PIN_FLD_POID, service_obj, ebufp);

                    service_type = (char *) PIN_POID_GET_TYPE ( service_obj);
                    if (strcmp(service_type,"/service/catv")== 0)
                    {
                        tmp_flistp = PIN_FLIST_ELEM_ADD (u_i_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
                        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_id, ebufp);
                    }
		    else
                    {
                        PIN_FLIST_FLD_SET(u_i_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_id, ebufp);
                    }


                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_prov_process_response update cas input flist ", u_i_flistp);
                    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_CACHEABLE, u_i_flistp,
                        &u_o_flistp, ebufp);
                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_prov_process_response update cas output flist", u_o_flistp);

                    PIN_FLIST_DESTROY_EX(&u_i_flistp, NULL);
                    PIN_FLIST_DESTROY_EX(&u_o_flistp, NULL);
                }
                
            }
        }
        else
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Failed at NE");
            prov_status = MSO_PROV_STATE_FAILED_PROVISIONING;
            service_obj = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp); 
            action = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACTION, 0, ebufp);
            order_poid = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);

            /***********************************************************
             * Update /mso_service_order class with status failed
             *  and error_descr
             ***********************************************************/
            w_i_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_POID, order_poid, ebufp);
            PIN_FLIST_FLD_SET( w_i_flistp, PIN_FLD_STATUS, &prov_status, ebufp);
            PIN_FLIST_FLD_SET( w_i_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
            PIN_FLIST_FLD_SET( w_i_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
            PIN_FLIST_FLD_SET( w_i_flistp, MSO_FLD_TASK_FAILED, &task_id, ebufp);

            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_prov_process_response write status input flist ", w_i_flistp);
            PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_CACHEABLE, w_i_flistp,
                &w_o_flistp, ebufp);
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_prov_process_response write status output flist", w_o_flistp);

            PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);

            /***********************************************************
             * Update MSO_FLD_CAS_SUBSCRBER_ID for CATV activation 
             * and CATV preactivation scenario
             ***********************************************************/
            if (  strstr(action,"ACTIVATE_SUBSCRIBER_NDS"))
            {
                task_flistp = PIN_FLIST_ELEM_GET (in_flistp, MSO_FLD_TASK, 1, 1, ebufp);
                if (task_flistp)
                {
                    activation_status = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_STATUS, 0, ebufp);

                    if (*activation_status == MSO_PROV_STATE_SUCCESS)
                    {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "however activate subscriber successful");
                        cas_id = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);

                        u_i_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(u_i_flistp, PIN_FLD_POID, service_obj, ebufp);

                        service_type = (char *) PIN_POID_GET_TYPE ( service_obj);
                        if (strcmp(service_type,"/service/catv")== 0)
                        {
                            tmp_flistp = PIN_FLIST_ELEM_ADD (u_i_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
                            PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_id, ebufp);
                        }
		        else
                        {
                            PIN_FLIST_FLD_SET(u_i_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_id, ebufp);
                        }   

                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                            "fm_mso_prov_process_response update cas input flist ", u_i_flistp);
                        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_CACHEABLE, u_i_flistp,
                            &u_o_flistp, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                            "fm_mso_prov_process_response update cas output flist", u_o_flistp);

                        PIN_FLIST_DESTROY_EX(&u_i_flistp, NULL);
                        PIN_FLIST_DESTROY_EX(&u_o_flistp, NULL);
                    }
                }
            }
        }
    }
    else
    {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_ORDER_ID, 0, 0);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Invalid Order_Id", ebufp);
    }

    return;
}
