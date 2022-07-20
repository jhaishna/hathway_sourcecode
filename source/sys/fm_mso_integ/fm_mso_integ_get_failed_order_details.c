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
* 0.1    |08/07/2014 |Phani Jandhyala   |  This opcode will give the detailed information of all failed jobs under an order
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_integ_get_failed_order_details.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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
op_mso_integ_get_failed_order_details(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_integ_get_failed_order_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_INTEG_GET_FAILED_ORDERS command
 *******************************************************************/
void
op_mso_integ_get_failed_order_details(
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
        int32                   failure = 1;
	int32			sucess = 0;

    /***********************************************************
     * Null out results until we have some.
     ***********************************************************/
    *ret_flistpp = NULL;
    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Insanity check.
     ***********************************************************/
    if (opcode != MSO_OP_INTEG_GET_FAILED_ORDER_DETAILS) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_integ_get_failed_order_details", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        " op_mso_integ_get_failed_order_details input flist", in_flistp);


    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_integ_get_failed_order_details(ctxp, in_flistp,&r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            " op_mso_integ_get_failed_order_details error", ebufp);

        PIN_ERRBUF_CLEAR(ebufp);
        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &failure, ebufp);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
    } 
    else 
    {
        *ret_flistpp = r_flistp;
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &sucess, ebufp);
        PIN_ERRBUF_CLEAR(ebufp);
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
         " op_mso_integ_get_failed_order_details return flist", *ret_flistpp);

    return;
}

/*******************************************************************
 * fm_mso_integ_get_failed_order_details()
 *
 * Perfomrs following action:
 * 1. 
 *******************************************************************/
static void
fm_mso_integ_get_failed_order_details(
    pcm_context_t           *ctxp,
    pin_flist_t             *in_flistp,
    pin_flist_t             **out_flistpp,
    pin_errbuf_t            *ebufp)
{

	poid_t	*task_poidp = NULL;
	poid_t	*order_pdp = NULL;
	poid_t	*job_poid = NULL;

	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	
	pin_flist_t	*job_iflistp = NULL;
	pin_flist_t	*job_oflistp = NULL;
	pin_flist_t	*task_flistp = NULL;
	pin_flist_t	*job_detail_flist = NULL;

	char	search_template[100] = " select X from /mso_mta_job where  F1 = V1 and F2 = V2 ";
	char	search_template1[100] = " select X from /mso_mta_job where  F1 like V1 ESCAPE V3 and F2 = V2 group by F1 order by F1 ";
	int	search_flags = 768;
	int	search_flags1 = 1024;
	int 	failed_status = 2;
	int 	order_status_sucess = 0;
	int	elemid_1 = 0;
	int	elem_id = 0;
	int32	*flags = NULL;
	int64	db = 1;
	char	*order_id = NULL;
	char	*err_desc = NULL;
	pin_cookie_t	cookie = NULL;
	char		task_id[30];
	char		escape[30];

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_integ_get_failed_job_details input flist", in_flistp);

    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/

	order_pdp = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_POID , 1, ebufp);
	order_id = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_ORDER_ID, 1, ebufp);
	flags   = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_FLAGS , 1 , ebufp);

	db = PIN_POID_GET_DB(order_pdp);
	if((flags != NULL) && (*flags == 1))
	{
		strcpy(task_id, order_id);
		strcat(task_id, "\\_");
		strcat(task_id, "%");
		strcpy(escape, "\\");

		*out_flistpp = PIN_FLIST_CREATE(ebufp);

		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_POID , order_pdp , ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_ORDER_ID , order_id , ebufp);

		search_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags1, ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template1, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ORDER_ID, task_id, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &failed_status, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ORDER_ID, escape, ebufp);
	
		results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
		//PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
		//PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_ERROR_DESCR, NULL, ebufp);
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_ORDER_ID, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search input list", search_inflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_search_device_object search output flist", search_outflistp);
		/*******
                 * Prepare the return flist and sent the order deatils
                 ******/
                 *out_flistpp = PIN_FLIST_COPY(search_outflistp, ebufp);
		
		PIN_FLIST_DESTROY_EX(&search_inflistp , NULL);
                PIN_FLIST_DESTROY_EX(&search_outflistp , NULL);
	
		return;
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " In the else part for download link");
		*out_flistpp = PIN_FLIST_CREATE(ebufp);

                PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_POID , order_pdp , ebufp);
                PIN_FLIST_FLD_SET(*out_flistpp , PIN_FLD_ORDER_ID , order_id , ebufp);

                search_inflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
                PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
                PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ORDER_ID, order_id, ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &failed_status, ebufp);

                results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
                PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
                PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_ERROR_DESCR, NULL, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search input list", search_inflistp);
                PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

	while((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp,PIN_FLD_RESULTS,&elem_id, 1, &cookie, ebufp))!= NULL)
	{
			job_poid = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_POID , 1, ebufp);
			err_desc = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_ERROR_DESCR , 1, ebufp);

			/**** call get failed job details to get the details *****/

				job_iflistp = PIN_FLIST_CREATE(ebufp);

				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ORDER_OBJ , job_poid , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID , PIN_POID_CREATE(db , "/account", -1 , ebufp), ebufp);

				PCM_OP(ctxp, MSO_OP_INTEG_GET_FAILED_JOB_DETAILS , 0, job_iflistp , &job_oflistp , ebufp);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_INTEG_GET_FAILED_JOB_DETAILS", ebufp);
					PIN_ERRBUF_CLEAR(ebufp);
					PIN_FLIST_DESTROY_EX(&job_oflistp, NULL);
				}

				job_detail_flist = PIN_FLIST_ELEM_GET(job_oflistp , MSO_FLD_TASK , PIN_ELEMID_ANY, 1, ebufp);

				task_flistp = PIN_FLIST_ELEM_ADD(*out_flistpp , MSO_FLD_TASK , elemid_1 , ebufp);

				PIN_FLIST_CONCAT(task_flistp , job_detail_flist , ebufp);

				PIN_FLIST_FLD_SET(task_flistp , PIN_FLD_ERROR_DESCR , err_desc , ebufp);

				elemid_1 = elemid_1 + 1;

				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&job_oflistp , NULL);
	}

	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
    	return;
	}
}

