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
* 0.1    |16/04/2014 |Phani Jandhyala   |  Searching all the Bulk job orders 
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_integ_search_job.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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
op_mso_integ_search_jobs(
        cm_nap_connection_t    *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_integ_search_job(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_INTEG_SEARCH_JOB command
 *******************************************************************/
void
op_mso_integ_search_jobs(
        cm_nap_connection_t    *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
    pin_flist_t		*r_flistp = NULL;
    pcm_context_t	*ctxp = connp->dm_ctx;
	int32			order_sucess = 0;

    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);
	
     /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_INTEG_SEARCH_JOB) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_integ_search_jobs", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_integ_search_jobs input flist", in_flistp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_integ_search_job(ctxp, in_flistp,&r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        *ret_flistpp = (pin_flist_t *)NULL;
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_integ_search_jobs error", ebufp);
    }
    else
    {
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp,PIN_FLD_STATUS,&order_sucess, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_integ_search_jobs return flist", *ret_flistpp);
    }

    return;
}

/*******************************************************************
 * fm_mso_integ_search_job()
 *
 * Perfomrs following action:
 * 1. call action specific function to enrich the input 
 * 2. get the details of all the jobs against a task
 *******************************************************************/
static void
fm_mso_integ_search_job(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flistp,
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
	int32				status_value = 3;
    int32 				*status = NULL;
    int64               db_no;
    poid_t              *s_pdp = NULL;
    poid_t              *pdp = NULL;
    poid_t              *acnt_pdp = NULL;
    poid_t              *order_pdp = NULL;
    pin_flist_t         *search_i_flistp = NULL;
    pin_flist_t         *tmp_flistp= NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_integ_search_job input flist", in_flistp);

	//search_flag = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 0, ebufp);

	pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	
	order_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_OBJ, 1, ebufp);

	order_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_ID, 0, ebufp);

	status = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_STATUS ,1 , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_integ_search_job error: required field missing in input flist", ebufp);
        return;
    }

    db_no = PIN_POID_GET_DB(pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_OPCODE, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, NULL, ebufp);
    //PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_INPUT_FLIST, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ERROR_DESCR, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ERROR_CODE, NULL, ebufp);

	/***********************************************************
     * Prepare search input based on the search key 
     ***********************************************************/
	if(status && *status != 4)
	{
		template = "select X from /mso_mta_job where F1 = V1 and F2 = V2 and F3 != V3 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, status, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, order_id, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &status_value, ebufp);

	}
	else if(status && *status == 4)
	{
		template = "select X from /mso_mta_job where F1 = V1 and F2 != V2 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, order_id, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &status_value, ebufp);

	}
	else
	{
		template = "select X from /mso_mta_job where F1 = V1 and F2 != V2 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, order_id, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &status_value, ebufp);
	}
    /*if ( *search_flag ==  SEARCH_BY_ORDER_ID )
    {
        template = "select X from /mso_mta_job where F1 = V1 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        order_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_ID, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, order_id, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_STATUS )
    {
        template = "select X from /mso_mta_job where F1 = V1 and F2 = V2 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        status = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_STATUS, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, status, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, order_id, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ERROR_CODE )
    {
        template = "select X from /mso_mta_job where F1 = V1 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        error_code = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ERROR_CODE, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
    }
    else if ( *search_flag ==  SEARCH_BY_ERROR_DESCR )
    {
        template = "select X from /mso_mta_job where F1 like V1 ";
        PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        error_descr = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ERROR_DESCR, 0, ebufp);
        tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
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
    }*/

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_integ_search_job search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        out_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_integ_search_job search output flist", *out_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}
