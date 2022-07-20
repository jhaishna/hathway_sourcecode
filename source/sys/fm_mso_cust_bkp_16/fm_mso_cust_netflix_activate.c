/*******************************************************************
 * File:	fm_mso_cust_activate_netflix_service.c
 * Opcode:	MSO_OP_CUST_NETFLIX_ACTIVATE 
 * Owner:	Kaliprasad Mahunta
 * Created:	04-SEPT-2018
 * Description: Activates netflix service for an account and
 *				purchases the plans stored in /mso_plans_activation
 *
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_activate_netflix_service.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "mso_ops_flds.h"
#include "mso_prov.h"
#include "mso_cust.h"
#include "mso_lifecycle_id.h"

#define SERVICE_NETFLIX    "/service/netflix"

#define SUCCESS 0
#define FAILED 1

/**************************************
* External Functions start
***************************************/


/**************************************
* Local Functions start
***************************************/
extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

/**************************************
* Local Functions end
***************************************/
extern int
fm_mso_update_mso_purplan_status(
	pcm_context_t           *ctxp,
	poid_t                  *mso_purplan_obj,
	int						status,
	pin_errbuf_t            *ebufp);

extern	void
fm_mso_create_balgrp(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_create_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_purchase_deal(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);
	
static void
fm_mso_activate_netflix_service(
pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
	pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
/*******************************************************************
 * MSO_OP_CUST_NETFLIX_ACTIVATE 
 *
 *******************************************************************/
//Input flist
/************************************************************************************
0 PIN_FLD_POID           POID [0] 0.0.0.1 /plan -1 0
0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 325529 0
0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/netflix 555557 0
0 MSO_FLD_PLAN_NAME    STR [0] allocated 20, used 3
*******************************************************************************************/ 
 
 
EXPORT_OP void
//op_mso_cust_activate_bb_service(
op_mso_cust_netflix_activate(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_NETFLIX_ACTIVATE
 *******************************************************************/
void 
op_mso_cust_netflix_activate(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*trans_flistp = NULL;
	pin_flist_t		*trans_out_flistp = NULL;
	int32			local = LOCAL_TRANS_OPEN_SUCCESS;
	int32			status_uncommitted =1;
	int                   *status = NULL;
	int32			status_netflix = 1;
	int32			*ret_status = NULL;
	poid_t			*account_pdp = NULL;
	char			*prog_name = NULL;
	char			*plan_name = NULL;
	*r_flistpp		= NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_NETFLIX_ACTIVATE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_netflix_add error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_netflix_add input flist", i_flistp);
	
	/*******************************************************************
	 * Validation: Mandatory field validation
	 *******************************************************************/

       account_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	if(!account_pdp) {
		//Account poid missing return error with message
		status_netflix = 2;
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_netflix, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Missing : Account POID", ebufp);
	}


	plan_name =(char *) PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_NAME, 0, ebufp);

	if(!plan_name) {
		//Plan Name missing
		status_netflix = 4;
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_netflix, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Missing : Plan Name Missing", ebufp);
	}

        local = fm_mso_trans_open(ctxp, account_pdp, 3,ebufp);
        if(PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, account_pdp, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return Flist", *r_flistpp);
                return;
        }
        if ( local == LOCAL_TRANS_OPEN_FAIL ) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, account_pdp, ebufp );
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
                return;
        }
/**     Added Trasnsaction handling   ***/
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/

//	fm_mso_activate_netflix_service(ctxp, flags, i_flistp, &r_flistpp, ebufp);
	fm_mso_activate_netflix_service(ctxp, i_flistp, r_flistpp, ebufp);
	
	if ( r_flistpp )
		status = PIN_FLIST_FLD_GET( *r_flistpp, PIN_FLD_STATUS, 1, ebufp);
/*     Added TRansaction Handling   ***/
        /***********************************************************
         * Results.
         ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return flist after activate service is :", *r_flistpp );       
 	if (PIN_ERRBUF_IS_ERR(ebufp) || (status && (*(int*)status == 1)))
        {
		if((prog_name && strstr(prog_name,"pin_deferred_act")))
        	{
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);
			return;
        	}
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERRBUF_RESET(ebufp);
                }
                //PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_activate_bb_service error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS &&
			(!(prog_name && strstr(prog_name,"pin_deferred_act")))
		  )
		{
			fm_mso_trans_abort(ctxp, account_pdp, ebufp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT aborted");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After Trans Abort");
        }
        else
        {
                if(local == LOCAL_TRANS_OPEN_SUCCESS &&
                   !(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 )
                  )
                {
                        fm_mso_trans_commit(ctxp, account_pdp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return flist for netflix is :", *r_flistpp );
			status_netflix = 0;
                	//PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_netflix, ebufp);
                	PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Success : Netflix Service Activated successfully", ebufp);
                }
                else
                {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
                }
        }
	return;
}

/*Function to activate Netflix service*/

void fm_mso_activate_netflix_service(
                pcm_context_t           *ctxp,
                pin_flist_t             *in_flist,
		pin_flist_t             **r_flistpp,
                pin_errbuf_t            *ebufp)
{
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *srch_flistp = NULL;
        pin_flist_t     *srch_out_flistp = NULL;
        pin_flist_t     *serv_srch_out_flistp = NULL;
	pin_flist_t     *pln_srch_o_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *dl_flistp = NULL;
        pin_flist_t     *dl_o_flistp = NULL;
	pin_flist_t	*all_billinfo = NULL;
	pin_flist_t	*bi_out_flist = NULL;
	pin_flist_t	*srvarr_flistp = NULL;
	pin_flist_t	*prdarr_flistp = NULL;
	pin_flist_t     *pdarr_flistp = NULL;
	pin_flist_t     *rslt_srch_flistp = NULL;
	pin_flist_t     *lc_out_flistp = NULL;
	pin_flist_t     *rd_flds_flistp = NULL;
	pin_flist_t     *rd_flds_o_flistp =  NULL;
	pin_flist_t     *update_srvc_status_input =  NULL;
	pin_flist_t     *update_srvc_status_output =  NULL;
	pin_flist_t     *statuses_array =  NULL;
	pin_flist_t     *prd_srch_flistp = NULL;
        pin_flist_t     *prd_srch_o_flistp = NULL;
	 pin_flist_t     *prd_reslt_flistp = NULL;

        poid_t          *account_obj = NULL;
	poid_t          *svc_obj = NULL;
	poid_t          *dl_pdp = NULL;
	poid_t          *pd_pdp = NULL;
	poid_t          *netflix_serv_obj = NULL;
	poid_t		*prd_pdp = NULL;
	
        int32           srch_flag = 256;
        int32           elem_id = 0;
        int32           serv_elem_id = 0;
	int32		ret_status = 1;
	int32		*serv_status = NULL;
	int32		status_flags = 0;
	int32           status_product = 1;
	int32		active_bb_service_found = 0;
	int32		active_catv_service_found = 0;
	int32		active_netflix_service_found = 5;
	int32		active_other_service_found = 0;
	int32		activate_service_failure = 1;
	int32		end_offset = 1;
	int32		end_unit = 5;
        int64           db = 0;
        pin_cookie_t    cookie = NULL;
        pin_cookie_t    serv_cookie = NULL;
        char            *offer_descr = NULL;
        char            *search_template = "select X from /profile_cust_offer where F1 = V1 ";
	char            *pln_srch_template = "select X from /deal 1, /plan 2  where  2.F1 = V1 and 2.F2 = 1.F3 ";
        char            *serv_search_template = "select X from /service where F1 = V1 ";
	char		*prod_search_template = "select X from /purchased_product where F1 = V1 AND F2 = V2 AND F3 = V3 AND F4 > V4";
	char            *plan_name = NULL;
	char            *planname = NULL;
	char            *prog_name_dflt = "Netflix_service_creation";
	char            *prog_name = NULL;
	time_t                  current_t = 0;
	int32 purchase_flag = 1;

        account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );
        netflix_serv_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
        db = PIN_POID_GET_DB(account_obj);
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        *r_flistpp = PIN_FLIST_CREATE(ebufp);
	 PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, account_obj, ebufp );
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, MSO_FLD_OFFER_DESCR,NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "profile cust offer search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp) || !srch_out_flistp )
        {
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Netflix is not opted by customer", ebufp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer profile search output flist", srch_out_flistp);
        while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                offer_descr = PIN_FLIST_FLD_GET(results_flistp, MSO_FLD_OFFER_DESCR, 1, ebufp);
                if (offer_descr && strcmp(offer_descr,"Netflix") ==0 )
                {
			//Get Services attached to the account	
        		srch_flistp = PIN_FLIST_CREATE(ebufp);
        		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, serv_search_template, ebufp);
        		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
        		res_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
        		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, NULL, ebufp);

        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search input list", srch_flistp);
        		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &serv_srch_out_flistp, ebufp);
        		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        		if (PIN_ERRBUF_IS_ERR(ebufp) || !serv_srch_out_flistp )
        		{
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Getting services error", ebufp);
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling Service SEARCH", ebufp);
                		PIN_ERRBUF_RESET(ebufp);
                		PIN_FLIST_DESTROY_EX(&serv_srch_out_flistp, NULL);
        		}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search output flist", serv_srch_out_flistp);
			
			//Scan Services and get required details	
			serv_elem_id = 0;
			serv_cookie = NULL;
        		while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(serv_srch_out_flistp, PIN_FLD_RESULTS, &serv_elem_id, 1, &serv_cookie, ebufp)) != NULL)
        		{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Result flist", res_flistp);
				svc_obj = PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_POID,0,ebufp);
				serv_status = PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_STATUS,0,ebufp);
				if( svc_obj && (strcmp(PIN_POID_GET_TYPE(svc_obj),MSO_BB)) == 0 )
				{
					serv_status = (int *)PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_STATUS,0,ebufp);
					if(*serv_status == PIN_STATUS_ACTIVE)
					{
						active_bb_service_found = 1;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active BB Service Found");
					}
				}
				else if( svc_obj && (strcmp(PIN_POID_GET_TYPE(svc_obj),MSO_CATV)) == 0 )
				{
					serv_status = PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_STATUS,0,ebufp);
					if(*serv_status == PIN_STATUS_ACTIVE)
					{
						active_catv_service_found = 1;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active CATV Service Found");
					}
				}
				else if( svc_obj && (strcmp(PIN_POID_GET_TYPE(svc_obj),MSO_NETFLIX)) == 0 )
				{
					//serv_status = PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_STATUS,0,ebufp);
					netflix_serv_obj = PIN_POID_COPY(svc_obj,ebufp);
					if(*serv_status == PIN_STATUS_ACTIVE)
					{
						active_netflix_service_found = 1;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active Netflix Service Found");
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "InActive Netflix Service Found");
						active_netflix_service_found = 0;
					}
				}
				else if(svc_obj)
				{
					serv_status = PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_STATUS,0,ebufp);
					if(*serv_status == PIN_STATUS_ACTIVE)
					{
						active_other_service_found = 1;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active CATV Service Found");
					}
				}
			}
			//if(((active_catv_service_found ==1) || (active_bb_service_found ==1) || (active_other_service_found == 1)) && ((active_netflix_service_found == 0) || (active_netflix_service_found == 1)))
			if(((active_netflix_service_found == 0) || (active_netflix_service_found == 1)))
			{

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "BB or CATV service Fond on Account");
				if(active_netflix_service_found == 0)
				{

					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "InActive Netflix Service Case");
					//Update netflix service status to active and purchase deal
                			update_srvc_status_input = PIN_FLIST_CREATE(ebufp);
					*serv_status = PIN_STATUS_ACTIVE;
                			PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_POID, account_obj, ebufp );
                			PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_SERVICE_OBJ, netflix_serv_obj, ebufp );
                			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, update_srvc_status_input, PIN_FLD_PROGRAM_NAME, ebufp );
                			if (!prog_name)
                			{
                        			PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_PROGRAM_NAME, "oap|csradmin", ebufp );
                			}
                			statuses_array = PIN_FLIST_ELEM_ADD(update_srvc_status_input, PIN_FLD_STATUSES, 0, ebufp);
                			PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS, serv_status, ebufp);
                			PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);

					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Activate Netflix service Input Flist");
                			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Update Service Status input flist", update_srvc_status_input);
                			PCM_OP(ctxp, PCM_OP_CUST_SET_STATUS, 0, update_srvc_status_input, &update_srvc_status_output, ebufp);
                			PIN_FLIST_DESTROY_EX(&update_srvc_status_input, NULL);
                			if (PIN_ERRBUF_IS_ERR(ebufp))
                			{
                        			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating product status", ebufp);
                        			PIN_ERRBUF_RESET(ebufp);

                        			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp );
                        			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &activate_service_failure, ebufp);
                        			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51078", ebufp);
                        			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in updating service status", ebufp);
                			}
                			PIN_FLIST_DESTROY_EX(&update_srvc_status_output, NULL);
					purchase_flag=0;
				}

				if(active_netflix_service_found == 1)
				{
					//Search purchased product
					//Check product status and validity
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active Netflix Service Case");
					current_t = pin_virtual_time(NULL);
					prd_srch_flistp = PIN_FLIST_CREATE(ebufp);
                        		PIN_FLIST_FLD_PUT(prd_srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		                        PIN_FLIST_FLD_SET(prd_srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
                		        PIN_FLIST_FLD_SET(prd_srch_flistp, PIN_FLD_TEMPLATE, prod_search_template, ebufp);
                        		args_flistp = PIN_FLIST_ELEM_ADD(prd_srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
					args_flistp = PIN_FLIST_ELEM_ADD(prd_srch_flistp, PIN_FLD_ARGS, 2, ebufp );
                                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, netflix_serv_obj, ebufp);
					args_flistp = PIN_FLIST_ELEM_ADD(prd_srch_flistp, PIN_FLD_ARGS, 3, ebufp );
                                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status_product, ebufp);
					args_flistp = PIN_FLIST_ELEM_ADD(prd_srch_flistp, PIN_FLD_ARGS, 4, ebufp );
                                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CYCLE_END_T, &current_t, ebufp);
		                        prd_reslt_flistp = PIN_FLIST_ELEM_ADD(prd_srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
                        		PIN_FLIST_FLD_SET(prd_reslt_flistp, PIN_FLD_POID, NULL, ebufp);
                        		//PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, NULL, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Product Search Input");
		                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service netflix product input list", prd_srch_flistp);
                        		PCM_OP(ctxp, PCM_OP_SEARCH, 0, prd_srch_flistp, &prd_srch_o_flistp, ebufp);
		                        PIN_FLIST_DESTROY_EX(&prd_srch_flistp, NULL);
                        		if (PIN_ERRBUF_IS_ERR(ebufp) || !prd_srch_o_flistp)
                        		{
                                		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
                                		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Getting Netflix product error", ebufp);
		                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling Netflix product SEARCH", ebufp);
		                                PIN_ERRBUF_RESET(ebufp);
                                		PIN_FLIST_DESTROY_EX(&prd_srch_o_flistp, NULL);
                        		}
					prd_reslt_flistp= PIN_FLIST_ELEM_GET(prd_srch_o_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Product Search Result");
                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix  Product search result flist", prd_reslt_flistp);
                                        if (prd_reslt_flistp) {
                                                prd_pdp = PIN_FLIST_FLD_GET(prd_reslt_flistp, PIN_FLD_POID, 1, ebufp);
						if (prd_pdp)  {
							purchase_flag = 1;
                                			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
							PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Netflix service already active on account", ebufp);
						}
                                        }

				}
				//Purchase Deal
				if (netflix_serv_obj && purchase_flag ==0) {
					//find out Netflix deal and product details using plan poid
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Purchase Deal Case");
					plan_name =(char *) PIN_FLIST_FLD_GET(in_flist, MSO_FLD_PLAN_NAME, 0, ebufp);
					srch_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
                        		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
            			        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, pln_srch_template, ebufp);
                        		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
                        		PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, PIN_POID_FROM_STR(plan_name, NULL, ebufp), ebufp);
		                        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
					srvarr_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, 0, ebufp );
                        		PIN_FLIST_FLD_SET(srvarr_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
					args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
					PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
		                        results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
                        		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
					prdarr_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_PRODUCTS, 0, ebufp );
					PIN_FLIST_FLD_SET(prdarr_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Plan Deal Search Input");
                        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix Deal and Product search input flist", srch_flistp);
		                        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &pln_srch_o_flistp, ebufp);
		                        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
                		        if (PIN_ERRBUF_IS_ERR(ebufp) || !pln_srch_o_flistp) {
                                		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Not able to find Netflix Deal or product ", ebufp);
                                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Netflix Deal and Product  SEARCH", ebufp);
                                		PIN_ERRBUF_RESET(ebufp);
                       			 }
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Plan Deal Search Output");
		                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix Deal and Product search output flist", pln_srch_o_flistp);
                        		rslt_srch_flistp= PIN_FLIST_ELEM_GET(pln_srch_o_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Plan Deal Search Result");
		                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix Deal and Product search result flist", rslt_srch_flistp);
		                        if (rslt_srch_flistp) {
                                		dl_pdp = PIN_FLIST_FLD_GET(rslt_srch_flistp, PIN_FLD_POID, 1, ebufp);
						pdarr_flistp = PIN_FLIST_ELEM_GET(rslt_srch_flistp,PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);
						pd_pdp = PIN_FLIST_FLD_GET(pdarr_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
                        		}
					


                                	//Purchase  netflix Deal
                                	dl_flistp = PIN_FLIST_CREATE(ebufp);
//					PIN_FLIST_FLD_PUT(dl_flistp, PIN_FLD_DEAL_OBJ, PIN_POID_FROM_STR("0.0.0.1 /deal 13488525380", NULL, ebufp), ebufp);
					PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_DEAL_OBJ, dl_pdp, ebufp);
					PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_POID, account_obj, ebufp);
					PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_PRODUCT_OBJ, pd_pdp, ebufp);
                                	PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
					PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_SERVICE_OBJ, netflix_serv_obj, ebufp);
	                                PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_CYCLE_END_OFFSET, &end_offset, ebufp);
	                                PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_CYCLE_END_UNIT, &end_unit, ebufp);
					PIN_FLIST_FLD_SET(dl_flistp,PIN_FLD_PLAN_OBJ,PIN_POID_FROM_STR(plan_name, NULL, ebufp), ebufp);

					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Deal Purchase Input");
		                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix Deal purchase Input flist", dl_flistp);
					fm_mso_purchase_deal(ctxp, dl_flistp, &dl_o_flistp, ebufp);

					if (dl_o_flistp) {
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix subscription completed", dl_o_flistp);
						*r_flistpp = NULL;
						*r_flistpp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SERVICE_OBJ, netflix_serv_obj, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
						ret_status = 0;
						//PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_STATUS, &ret_status, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
						//*r_flistpp = PIN_FLIST_COPY(dl_o_flistp, ebufp);

						//find out the netflix plan name using plan poid
						rd_flds_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(rd_flds_flistp,PIN_FLD_POID,PIN_POID_FROM_STR(plan_name, NULL, ebufp), ebufp);
						PIN_FLIST_FLD_SET(rd_flds_flistp,PIN_FLD_NAME,NULL, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix plan name search inputflist", rd_flds_flistp);
	                                        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rd_flds_flistp, &rd_flds_o_flistp, ebufp);
	                                        PIN_FLIST_DESTROY_EX(&rd_flds_flistp, NULL);
	                                        if (PIN_ERRBUF_IS_ERR(ebufp) || !rd_flds_o_flistp) {
	                                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
        	                                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Not able to find Netflix plan name ", ebufp);
                	                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in finding Netflix plan name", ebufp);
                        	                        PIN_ERRBUF_RESET(ebufp);
                                	         }
	                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix plan name search outflist", rd_flds_o_flistp);
        	                                planname = PIN_FLIST_FLD_GET(rd_flds_o_flistp, PIN_FLD_NAME, 0, ebufp);
						prog_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 1, ebufp );
						
						//Lifecycle event
						PIN_FLIST_FLD_SET(dl_flistp, MSO_FLD_PLAN_NAME, planname, ebufp);
						PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Inputflist for fm_mso_cust_create_netflix_lc_event ", dl_flistp);
						fm_mso_create_lifecycle_evt(ctxp, dl_flistp, lc_out_flistp, ID_ADD_NETFLIX, ebufp );
					}
					if (PIN_ERRBUF_IS_ERR(ebufp) || !dl_o_flistp )
	                                {
        	                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Not able to purchase new Netflix deal", ebufp);
                	                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while purchasing Netflix deal", ebufp);
                        	                PIN_ERRBUF_RESET(ebufp);
                                	        PIN_FLIST_DESTROY_EX(&dl_o_flistp, NULL);
                               		 }

				}
				if (PIN_ERRBUF_IS_ERR(ebufp) || !svc_obj)
	                        {
        	                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Not able to create new Netflix service", ebufp);
                	                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating new netlix service object", ebufp);
                        	        PIN_ERRBUF_RESET(ebufp);
        	                }

			}
			else if((active_catv_service_found == 0) && (active_bb_service_found == 0) && (active_other_service_found == 0))
			{
				//Neither BB nor CATV Service active on account
        	                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "There is no active service(BB or CATV or Other) on account", ebufp);
                	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating new netlix service object", ebufp);
                        	PIN_ERRBUF_RESET(ebufp);
			}
			else
			{
				//Other cases
			}
                }
        }

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Not able to complete Netflix subscrition", ebufp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in activate netflix service", ebufp);
                PIN_ERRBUF_RESET(ebufp);
        }
	else
	{
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "Netflix service activated on account", ebufp);
	}
        PIN_FLIST_DESTROY_EX(&dl_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&lc_out_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
}

