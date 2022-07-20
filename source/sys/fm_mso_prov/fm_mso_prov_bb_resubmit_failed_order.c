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
* 0.1    |15/10/2014 |Someshwar         |  CATV Failover Management
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/


#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_prov_bb_resubmit_failed_order.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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
#include "cm_cache.h"
#include "mso_lifecycle_id.h"


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

void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_prov_bb_resubmit_failed_order(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

static void
fm_mso_prov_bb_resubmit_failed_order(
        pcm_context_t       *ctxp,
        pin_flist_t         *in_flistp,
        int32               flags,
        pin_flist_t         **out_flistpp,
        pin_errbuf_t        *ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_PROV_RESUBMIT_FAILED_ORDER command
 *******************************************************************/
void
op_mso_prov_bb_resubmit_failed_order(
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
    if (opcode != MSO_OP_PROV_BB_RESUBMIT_FAILED_ORDER) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_prov_bb_resubmit_failed_order", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_prov_bb_resubmit_failed_order input flist", in_flistp);

    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    t_status = fm_mso_trans_open(ctxp, pdp, 1, ebufp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_prov_bb_resubmit_failed_order(ctxp, in_flistp, flags, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_prov_bb_resubmit_failed_order error", ebufp);

        PIN_ERRBUF_CLEAR(ebufp);

        if (t_status == 0)
        {
            fm_mso_trans_abort(ctxp, pdp, ebufp);
        }

        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "Fail", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Order Resubmission Failed", ebufp);


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
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "Success", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Order Successfully Resubmitted", ebufp);
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_CLEAR(ebufp);
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_prov_bb_resubmit_failed_order return flist", *ret_flistpp);

    return;
}

/*******************************************************************
 * fm_mso_prov_bb_resubmit_failed_order()
 *
 * Perfomrs following action:
 * 1. 
 *******************************************************************/
static void
fm_mso_prov_bb_resubmit_failed_order(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    int64               db_no;
    poid_t              *service_poidp = NULL;
    poid_t              *acc_poidp = NULL;
    pin_flist_t         *payload_flistp = NULL;
    pin_flist_t         *si_flistp = NULL;
    pin_flist_t         *so_flistp = NULL;
    pin_flist_t         *w_i_flistp = NULL;
    pin_flist_t         *w_o_flistp = NULL;
    pin_flist_t         *tmp_flistp= NULL;
    pin_flist_t		*lc_in_flist = NULL;

    char                *flistbuf = NULL;
    int                 flistlen=0;
    int                 status = MSO_PROV_STATE_NEW;
    int32               *failed_task_id = NULL;
	int32               *tech = NULL;
    int32               i = 1;
    //int32               priority = 1;
    pin_buf_t           *nota_buf     = NULL;
    pin_flist_t         *order_i_flistp     = NULL;
    pin_flist_t         *task_flistp     = NULL;
    char                *order_id = NULL;
    time_t              now;
    poid_t              *objp = NULL;
    char                new_order_id[128];
    char                *action = NULL;
    int32               task_count;
    int32               rankone = 1;
    char                *prefix = NULL;
    char                *seq = NULL;
    char                *resumbmit_count = NULL;
    int32               already_submited;
    char                *cas_subs_id = NULL;
    void		*vp = NULL;
    pin_flist_t		*tmpp_flistp= NULL;
    pin_flist_t         *create_bal_flist = NULL;
    char                *initial_amt = NULL;
    int                 task_id_p = 0;
    pin_cookie_t        cookie_p = NULL;
    int                 task_id_c = 0;
    pin_cookie_t        cookie_c = NULL;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_resubmit_failed_order input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/
    failed_task_id = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_TASK_FAILED, 0, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    service_poidp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
    acc_poidp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);
    order_id = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ORDER_ID, 0, ebufp);
    action = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACTION, 0, ebufp);
    task_count = PIN_FLIST_ELEM_COUNT(tmp_flistp, MSO_FLD_TASK, ebufp);
    tech = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp) || task_count == 0 ) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_prov_bb_resubmit_failed_order error: required field missing in input flist", ebufp);
        return;
    }
    /***Added condition to check the initial amount in the buffer flist**/ 
    while ( (tmpp_flistp = PIN_FLIST_ELEM_GET_NEXT (tmp_flistp,
                                MSO_FLD_TASK, &task_id_p, 1,&cookie_p, ebufp))
                                                          != (pin_flist_t *)NULL )
    {
        while ( (create_bal_flist = PIN_FLIST_ELEM_GET_NEXT (tmpp_flistp,
                                MSO_FLD_CREATEBALANCE, &task_id_c, 1,&cookie_c, ebufp))
                                                          != (pin_flist_t *)NULL )
        {
                initial_amt = PIN_FLIST_FLD_GET(create_bal_flist, MSO_FLD_INITIALAMOUNT, 1, ebufp);
        }
    }

    payload_flistp = PIN_FLIST_CREATE(ebufp);
    payload_flistp = PIN_FLIST_COPY(tmp_flistp, ebufp);

    for (i = 1;i < *failed_task_id ;i++ )
    {
        task_flistp = PIN_FLIST_ELEM_GET(payload_flistp, MSO_FLD_TASK, i , 1, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_prov_bb_resubmit_failed_order input flist", payload_flistp);
        if (task_flistp)
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "dropping task element");
            PIN_FLIST_ELEM_DROP(payload_flistp, MSO_FLD_TASK, i, ebufp);
        }
    }

    /*
     * Return if validation and enrichment returns error
     */
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
        return;
    }


    /*
     * Generate New Order Id and set it in payload
     */
    prefix = strtok((char *)order_id,"-");
    seq = strtok (NULL, "-");
    resumbmit_count = strtok (NULL, "-");

    if (!resumbmit_count)
    {
        sprintf(new_order_id, "%s-%s-1",prefix,seq);
    }
    else
    {
        already_submited = atoi((char *)resumbmit_count);
        sprintf(new_order_id, "%s-%s-%d",prefix,seq,++already_submited);
    }

    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, new_order_id, ebufp); 

    /*
     * Convert flist to Buffer. it will be used by Provising system.
     */
    PIN_FLIST_FLD_SET(payload_flistp, PIN_FLD_ORDER_ID, new_order_id, ebufp); 

//    if (strcmp(action,"ACTIVATE_SUBSCRIBER_NDS") == 0 && *failed_task_id > 1 )
//    {
//        si_flistp = PIN_FLIST_CREATE(ebufp);
//        PIN_FLIST_FLD_SET( si_flistp, PIN_FLD_POID, service_poidp, ebufp);
//        tmp_flistp = PIN_FLIST_SUBSTR_ADD(si_flistp, MSO_FLD_CATV_INFO, ebufp);
//        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
//
//        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
//            "prepare_catv_add_package_flist PCM_OP_READ_OBJ input flist", si_flistp);
//        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, si_flistp, &so_flistp, ebufp);
//        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
//            "prepare_catv_add_package_flist PCM_OP_READ_FLDS output flist", so_flistp);
//
//        tmp_flistp = PIN_FLIST_SUBSTR_GET(so_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
//        cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp); 
//
//        if ( cas_subs_id && strlen(cas_subs_id) > 0)
//        {
//                //add cas subscriber id 
//            cas_subs_id = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, 0, ebufp);
//        }
//        else
//        {
//            pin_set_err(ebufp, PIN_ERRLOC_FM,
//                PIN_ERRCLASS_SYSTEM_DETERMINATE,
//                    PIN_ERR_BAD_VALUE, MSO_FLD_CAS_SUBSCRIBER_ID, 0, 0);
//            goto CLEANUP;
//        }
//
//        PIN_FLIST_FLD_SET(payload_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, cas_subs_id, ebufp); 
//    }

    /*
     * Convert flist to Buffer. it will be used by Provising system.
     */
    PIN_FLIST_TO_STR(payload_flistp,&flistbuf,&flistlen,ebufp );


    /*
     * Create flist for order creation
     */
    db_no = PIN_POID_GET_DB(service_poidp); 
    objp = PIN_POID_CREATE(db_no, "/mso_prov_bb_order", -1, ebufp);

    order_i_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_POID, objp, ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_SERVICE_OBJ, service_poidp, ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_ACCOUNT_OBJ, acc_poidp, ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_ORDER_ID, new_order_id, ebufp); 
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_STATUS, &status, ebufp); //PROVISIONING
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_ACTION, action, ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, MSO_FLD_TECHNOLOGY, tech, ebufp);
    if(initial_amt != NULL)
    {
    	PIN_FLIST_FLD_SET(order_i_flistp, MSO_FLD_INITIALAMOUNT, initial_amt, ebufp);	
    }
    //PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_RANK, PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_RANK, 0, ebufp), ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_RANK, &rankone, ebufp);
    nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
    if ( nota_buf == NULL ) 
    {
             pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0 ,0 );
             PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate nemory for nota_buf", ebufp ); 
             goto CLEANUP;
    }   

    nota_buf->flag   = 0;
    nota_buf->size   = flistlen - 2;
    nota_buf->offset = 0;
    nota_buf->data   = ( caddr_t)flistbuf;
    nota_buf->xbuf_file = ( char *) NULL;

    PIN_FLIST_FLD_PUT( order_i_flistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_resubmit_failed_order create input flist", order_i_flistp);
    PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, order_i_flistp, out_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_resubmit_failed_order order create output flist", *out_flistpp);

    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
		"Failed create_obj of order creation in fm_mso_prov_bb_resubmit_failed_order ", ebufp);
        PIN_ERRBUF_RESET(ebufp);
	goto CLEANUP;
    }
    
    PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ORDER_ID, new_order_id, ebufp);

    
    /*
     * Update previous order status as Correced.
     */
    status = MSO_PROV_STATE_CORRECTED;
    w_i_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, w_i_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_STATUS, &status, ebufp); //CORRECTED

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_resubmit_failed_order update previous order status input flist", w_i_flistp);
    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, w_i_flistp, &w_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_resubmit_failed_order update previous order status output flist", w_o_flistp);
	
	lc_in_flist  = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(lc_in_flist, PIN_FLD_ACCOUNT_OBJ, acc_poidp, ebufp );
	PIN_FLIST_FLD_SET(lc_in_flist, PIN_FLD_ORDER_ID, order_id, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, lc_in_flist, MSO_FLD_PROV_ORDER_OBJ, ebufp );
	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (vp)
	{
	   PIN_FLIST_FLD_COPY(vp, PIN_FLD_PROGRAM_NAME, lc_in_flist, PIN_FLD_PROGRAM_NAME, ebufp );
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer access information change successful");
	fm_mso_create_lifecycle_evt(ctxp, lc_in_flist, NULL, ID_PROV_ORDER_RESUBMIT, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}


CLEANUP:
    PIN_FLIST_DESTROY_EX(&payload_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&order_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&si_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
    
    return;
}


