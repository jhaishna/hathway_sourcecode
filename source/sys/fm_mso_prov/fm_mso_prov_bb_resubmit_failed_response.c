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
* 0.1    |03/07/2015 |Pavan Bellala     |  BRM objects are updated after Provisioning for Broadband.In case of 
					|  failure during updating this opcode is called to resubmit response 
*--------------------------------------------------------------------------------------------------------------*
*
****************************************************************************************************************/
#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_mso_prov_bb_resubmit_failed_response.c /cgbubrm_7.3.2.rwsmod/1 2009/08/04 23:37:19 pjegan Exp $";
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



/*******************************************************************
 * Fuctions defined in this code 
 *******************************************************************/

EXPORT_OP void
op_mso_prov_bb_resubmit_failed_response(
        cm_nap_connection_t    *connp,
        int32               opcode,
        int32               flags,
        pin_flist_t         *in_flistp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

static void
fm_mso_prov_bb_resubmit_failed_response(
        pcm_context_t       *ctxp,
        pin_flist_t         *in_flistp,
        int32               flags,
        pin_flist_t         **out_flistpp,
        pin_errbuf_t        *ebufp);

int
mso_prov_failed_response_update_status(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PROV_RESUBMIT_FAILED_RESPONSE command
 *******************************************************************/
void
op_mso_prov_bb_resubmit_failed_response(
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
    if (opcode != MSO_OP_PROV_BB_RESUBMIT_FAILED_RESPONSE) {
        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
                PIN_ERR_BAD_OPCODE, 0, 0, opcode);
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_prov_bb_resubmit_failed_response", ebufp);
        return;
    }

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_prov_bb_resubmit_failed_response input flist", in_flistp);

    pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    t_status = fm_mso_trans_open(ctxp, pdp, 1, ebufp);

    /***********************************************************
     * Call main function to do it
     ***********************************************************/
    fm_mso_prov_bb_resubmit_failed_response(ctxp, in_flistp, flags, &r_flistp, ebufp);

    /***********************************************************
     * Results.
     ***********************************************************/
    if (PIN_ERRBUF_IS_ERR(ebufp)) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_prov_bb_resubmit_failed_response error", ebufp);

        PIN_ERRBUF_CLEAR(ebufp);

        if (t_status == 0)
        {
            fm_mso_trans_abort(ctxp, pdp, ebufp);
        }

        *ret_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &oap_failure, ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "Fail", ebufp);
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Response Resubmission Failed", ebufp);


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
        PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Response Successfully Resubmitted", ebufp);
        *ret_flistpp = r_flistp;
        PIN_ERRBUF_CLEAR(ebufp);
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_prov_bb_resubmit_failed_response return flist", *ret_flistpp);

    return;
}

/*******************************************************************
 * fm_mso_prov_bb_resubmit_failed_response()
 *
 * Perfomrs following action:
 * 1. 
 *******************************************************************/
static void
fm_mso_prov_bb_resubmit_failed_response(
    pcm_context_t       *ctxp,
    pin_flist_t         *in_flistp,
    int32               flags,
    pin_flist_t         **out_flistpp,
    pin_errbuf_t        *ebufp)
{
    int64               db_no;
    poid_t              *service_pdp = NULL;
    poid_t              *acc_pdp = NULL;
    poid_t              *s_pdp = NULL;

    pin_flist_t         *search_flistp = NULL;
    pin_flist_t         *so_flistp = NULL;
    pin_flist_t		*results_flistp = NULL;
    pin_flist_t		*resubmit_flistp = NULL;
    pin_flist_t		*out_resubmit_flistp = NULL;
    pin_flist_t         *tmp_flistp= NULL;

    char                *order_idp = NULL;
    char                *template = "select X from /mso_prov_bb_order where F1 = V1 and F2 = V2 and F3 = V3 ";
    int					rv = 0;
    int32                 s_flags = SRCH_DISTINCT;
    void		*vp = NULL;
    int			*failed_task_idp = NULL;
    int			prov_status = MSO_PROV_STATE_SUCCESS;
	


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    /***********************************************************
     * Log input flist
     ***********************************************************/
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_resubmit_failed_response input flist", in_flistp);

    *out_flistpp = PIN_FLIST_COPY(in_flistp,ebufp);
    PIN_FLIST_FLD_DROP(*out_flistpp,PIN_FLD_STATUS,ebufp);
    
    /***********************************************************
     * Check mandatory fields are available
     ***********************************************************/

    failed_task_idp = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_TASK_FAILED, 0, ebufp);
    service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
    acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
    order_idp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ORDER_ID, 0, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "fm_mso_prov_bb_resubmit_failed_response error: required field missing in input flist", ebufp);
        return;
    }

	/**********************************************************
	Fetch the failed response from database
	***********************************************************/

	db_no = PIN_POID_GET_DB(acc_pdp); 

    search_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
    tmp_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID, order_idp, ebufp);

    PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_resubmit_failed_response search input flist ", search_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_flistp, &so_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_bb_resubmit_failed_response search output flist", so_flistp);

	PIN_FLIST_DESTROY_EX(&search_flistp,NULL);
	
	if(so_flistp && PIN_FLIST_ELEM_COUNT(so_flistp,PIN_FLD_RESULTS,ebufp)> 0 )
	{

		/**************************************************
		Prepare input flist for resubmit
		***************************************************/
		resubmit_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(resubmit_flistp,PIN_FLD_POID,acc_pdp,ebufp);
		PIN_FLIST_FLD_SET(resubmit_flistp,PIN_FLD_SERVICE_OBJ,service_pdp,ebufp);
		PIN_FLIST_FLD_SET(resubmit_flistp,PIN_FLD_ORDER_ID,order_idp,ebufp);

		results_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_RESULTS,0,1,ebufp);
		if(results_flistp)
		{
			PIN_FLIST_FLD_COPY(results_flistp,MSO_FLD_TASK_FAILED,resubmit_flistp,MSO_FLD_TASK_FAILED,ebufp);
			PIN_FLIST_FLD_COPY(results_flistp,PIN_FLD_BUFFER,resubmit_flistp,PIN_FLD_INPUT_FLIST,ebufp);
		
			tmp_flistp = PIN_FLIST_ELEM_ADD(resubmit_flistp,MSO_FLD_TASK,0,ebufp);
			PIN_FLIST_FLD_SET (tmp_flistp,PIN_FLD_STATUS,&prov_status,ebufp);

			if( resubmit_flistp)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"Calling process response opcode input", resubmit_flistp);
				PCM_OP(ctxp, MSO_OP_PROV_BB_PROCESS_RESPONSE, 0, resubmit_flistp, &out_resubmit_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"Calling process response opcode output flist", out_resubmit_flistp);
			}

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in return of "
								"MSO_OP_PROV_BB_PROCESS_RESPONSE",ebufp);
				goto CLEANUP;

			} else {
				rv = mso_prov_failed_response_update_status(ctxp,in_flistp,ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"status updated in DB");
				if(rv == 0){
					PIN_FLIST_FLD_SET(*out_flistpp,PIN_FLD_STATUS_MSG,"Update:Successful",ebufp);
				} else if(rv == 1){
					PIN_FLIST_FLD_SET(*out_flistpp,PIN_FLD_STATUS_MSG,
								"Update:Failure(No failed reponse object found)",ebufp);
				} else if(rv == 2){
					PIN_FLIST_FLD_SET(*out_flistpp,PIN_FLD_STATUS_MSG,
								"Update:Failure(Internal error during update)",ebufp);
				}
			}
		
		}

	}	
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"fm_mso_prov_bb_resubmit_failed_response no results");
		PIN_FLIST_FLD_SET(*out_flistpp,PIN_FLD_STATUS_MSG,"Update:Failure(No Provisioning order results found)",ebufp);
		PIN_FLIST_DESTROY_EX(&so_flistp,NULL);
		return;
	}


CLEANUP:
	PIN_FLIST_DESTROY_EX(&resubmit_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&out_resubmit_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&so_flistp, NULL);   
    return;
}




/**************************************************
mso_prov_failed_response_update_status()
***************************************************/

int
mso_prov_failed_response_update_status(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t         *w_i_flistp = NULL;
        pin_flist_t         *w_o_flistp = NULL;
	pin_flist_t         *search_flistp = NULL;
	pin_flist_t         *so_flistp = NULL;
	pin_flist_t	    *tmp_flistp = NULL;
	char                *template1 = "select X from /mso_failed_prov_response 1, /mso_prov_bb_order 2 where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3 and 1.F4 = 2.F5 and 2.F6 = V6 and 2.F6 = V6 ";
	//char                *template1 = "select X from /mso_failed_prov_response 1 where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3 ";

	poid_t              *s_pdp = NULL;
	poid_t              *pdp = NULL;
	int32                s_flags = SRCH_DISTINCT;
	int	            return_value = 0;
	int32               status = 0;
	int		    st = MSO_PROV_STATE_SUCCESS;
	int64		    db_no;
	 /*
         * Update status as resubmitted.
        */
	
    	pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	db_no = PIN_POID_GET_DB(pdp);
	status = MSO_PROV_RESPONSE_RESUBMIT;
	search_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

	s_flags = SRCH_DISTINCT ;
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, (void*)template1, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD (search_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ,tmp_flistp, PIN_FLD_SERVICE_OBJ,ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, tmp_flistp, PIN_FLD_ACCOUNT_OBJ,ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_ORDER_ID,tmp_flistp, PIN_FLD_ORDER_ID,ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID,"",ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ORDER_ID,"",ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS,&st,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"search failed response input flist ", search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_flistp,&so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"search failed response output flist", so_flistp);
	PIN_FLIST_DESTROY_EX(&search_flistp,NULL);
	if(so_flistp && PIN_FLIST_ELEM_COUNT(so_flistp,PIN_FLD_RESULTS,ebufp)>0)
	{
		tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
		if(tmp_flistp)
		{
			w_i_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(tmp_flistp,PIN_FLD_POID,w_i_flistp,PIN_FLD_POID,ebufp);
			PIN_FLIST_FLD_SET(w_i_flistp, PIN_FLD_STATUS, &status, ebufp); 

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"mso_prov_failed_response_update_status update status input flist", w_i_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, w_i_flistp, &w_o_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"mso_prov_failed_response_update_status update status output flist", w_o_flistp);
			
			    /* lc_in_flist  = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(lc_in_flist, PIN_FLD_ACCOUNT_OBJ, acc_poidp, ebufp );
			PIN_FLIST_FLD_SET(lc_in_flist, PIN_FLD_ORDER_ID, order_id, ebufp );
			PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, lc_in_flist, MSO_FLD_PROV_ORDER_OBJ, ebufp );
			vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			if (vp)
			{
				PIN_FLIST_FLD_COPY(vp, PIN_FLD_PROGRAM_NAME, lc_in_flist, PIN_FLD_PROGRAM_NAME, ebufp );
			}

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer access information change successful");
			fm_mso_create_lifecycle_evt(ctxp, lc_in_flist, NULL, ID_PROV_ORDER_RESUBMIT, ebufp); */
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{	
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				return_value = 2;
			}

			PIN_FLIST_DESTROY_EX(&w_i_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&w_o_flistp, NULL);
		
			return_value = 0;
		}

	} else {
			return_value = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Data correction is required");
	}


PIN_FLIST_DESTROY_EX(&so_flistp, NULL);
return return_value;

}
