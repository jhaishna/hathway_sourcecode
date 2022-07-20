/*******************************************************************
 * File:        fm_mso_cust_renew_netflix_plan.c
 * Opcode:      MSO_OP_CUST_RENEW_NETFLIX 
 * Owner:       Kaliprasad Mahunta
 * Created:     04-SEP-2018
 * Description: This opcode is invoked for doing renew of existing prepaid plan before expiry date

        INPUT :
        0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 2431536 0
        0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 2431536 0
        0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/netflix 2427945 0
        0 MSO_FLD_PLAN_NAME       STR [0] "Netflix_Test_Plan"
        0 PIN_FLD_AMOUNT  DECIMAL [0] NULL

        OUTPUT :
        # number of field entries allocated 20, used 4
        0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
        0 PIN_FLD_STATUS         ENUM [0] 0
        0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/netflix 12345 1
        0 PIN_FLD_DESCR           STR [0] "Renew plan completed successfully"

 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_renew_netflix_plan.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bal.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "fm_bal.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_lifecycle_id.h"


/**************************************
* External Functions start
***************************************/
extern int32
fm_mso_trans_open(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        int32           flag,
        pin_errbuf_t    *ebufp);

extern void 
fm_mso_trans_commit(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

extern void  
fm_mso_trans_abort(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

extern
void fm_mso_create_lifecycle_evt(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *out_flistp,
        int32                   action,
        pin_errbuf_t            *ebufp);

extern int32
get_diff_time(
        pcm_context_t   *ctxp,
        time_t          timeStamp,
        time_t          timeStamp2,
        pin_errbuf_t    *ebufp);

/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/
void 
fm_mso_renew_netflix_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_RENEW_NETFLIX 
 *
 *******************************************************************/

EXPORT_OP void
op_mso_cust_netflix_renew(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


/**static void
fm_mso_cust_renew_netflix_plan(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);**/
/*******************************************************************
 * MSO_OP_CUST_RENEW_NETFLIX_PLAN  
 *******************************************************************/
void 
op_mso_cust_netflix_renew(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;

        *r_flistpp              = NULL;
	pin_flist_t		*r_flistp = NULL;
        int                     local = LOCAL_TRANS_OPEN_SUCCESS;
        int                     *status = NULL;
        int                     netflix_status = 0;
        int32                   *renew_flag = NULL;
        int32                   status_uncommitted =2;
        poid_t                  *account_pdp = NULL;
        poid_t                  *service_pdp = NULL;
	poid_t			*inp_pdp = NULL;
        char            	*prog_name = NULL;
        char            	*plan_name = NULL;
	void			*vp = NULL;
        
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /*******************************************************************
         * Insanity Check 
         *******************************************************************/
        if (opcode != MSO_OP_CUST_NETFLIX_RENEW) {
                pin_set_err(ebufp, PIN_ERRLOC_FM, 
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_cust_renew_bb_plan error",
                        ebufp);
                return;
        }

        /*******************************************************************
         * Debug: Input flist 
         *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
                "op_mso_cust_renew_bb_plan input flist", i_flistp);
        
        /*******************************************************************
         * Validation: Mandatory field validation
         *******************************************************************/

        account_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

        if(!account_pdp) {
                //Account poid missing return error with message
                netflix_status = 2;
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &netflix_status, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Missing : Account POID", ebufp);
        }

        service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);

        if(!service_pdp) {
                //Service poid missing
                netflix_status = 3;
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &netflix_status, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_DESCR, "Missing : Service POID", ebufp);
        }

        plan_name =(char *) PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_NAME, 0, ebufp);

        if(!plan_name) {
                //Plan Name missing get current plan from DB
        }

        prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
        renew_flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
//        if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
        if (!(prog_name && strstr(prog_name,"pin_deferred_act")))
        {
                inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
                local = fm_mso_trans_open(ctxp, inp_pdp, 3,ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
                        return;
                }
                if ( local == LOCAL_TRANS_OPEN_FAIL )
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
                        return;
                }
        }
        else
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'pin_deferred_act' so transaction will not be handled at API level");
        }
//        fm_mso_renew_netflix_service(ctxp, i_flistp, &r_flistp, ebufp);
	fm_mso_renew_netflix_service(ctxp, i_flistp, r_flistpp, ebufp);

//        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_cust_renew_bb_plan netflix output flist", r_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_cust_renew_bb_plan netflix output flist", *r_flistpp);
	//status = PIN_FLIST_FLD_GET(r_flistp,PIN_FLD_STATUS,0,ebufp);


        /***********************************************************
         * Results.
         ***********************************************************/
        if (PIN_ERRBUF_IS_ERR(ebufp) || (status && *(int*)status == 1)) 
        {       if((prog_name && strstr(prog_name,"pin_deferred_act")))
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
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_renew_bb_plan error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS &&
			(!(prog_name && strstr(prog_name,"pin_deferred_act")))
		  )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT aborted");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}
        }
        else
        {
                if(local == LOCAL_TRANS_OPEN_SUCCESS && 
					(!(prog_name && strstr(prog_name,"pin_deferred_act")))
                  )
                {
						/* If PIN_FLD_MODE (Calc Only flag) is 1 then abort the transaction.*/
						vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 1, ebufp );
						if(vp && *(int32 *)vp == 1) {
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Success and Price Calc Flag = 1. Abort the Trans.");
							fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
						}
						else {
							fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
						}
                }
                else
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
                }               
        }

//        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_cust_renew_bb_plan output flist", *r_flistpp);
//	fm_mso_renew_netflix_service(ctxp, i_flistp, r_flistpp, ebufp);

//        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_cust_renew_bb_plan netflix output flist", r_flistp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_cust_renew_bb_plan netflix output flist", *r_flistpp);
        return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
void fm_mso_renew_netflix_service(
                pcm_context_t           *ctxp,
                pin_flist_t             *in_flist,
        	pin_flist_t             **ret_flistpp,
                pin_errbuf_t            *ebufp)
{               
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *srch_flistp = NULL;
        pin_flist_t     *srch_out_flistp = NULL;
	pin_flist_t     *srch_svc_o_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *result_srch_flistp = NULL;
	pin_flist_t	*dl_flistp = NULL;
	pin_flist_t	*dl_o_flistp = NULL;
	pin_flist_t     *srvarr_flistp = NULL;
	pin_flist_t	*lc_out_flist = NULL;
       	pin_flist_t     *ret_flistp;
	pin_flist_t     *pln_srch_o_flistp = NULL;
	pin_flist_t     *pdarr_flistp = NULL;
        pin_flist_t     *rslt_srch_flistp = NULL;
	pin_flist_t	*prdarr_flistp  = NULL;
	pin_flist_t     *rd_flds_flistp = NULL;
	pin_flist_t     *rd_flds_o_flistp = NULL;
	pin_flist_t     *netflix_prod_list = NULL;
	pin_flist_t     *pur_prod_flistp = NULL;
	pin_flist_t     *netflix_srch_flistp = NULL;
	poid_t          *account_obj = NULL;
	poid_t		*srvc_pdp = NULL;
	poid_t          *dl_pdp = NULL;
        poid_t          *pd_pdp = NULL;
	poid_t 		*plan_pdp = NULL;
	poid_t 		*pur_prod_poid = NULL;
	poid_t 		*prod_poid = NULL;
	int32          	srch_flag = 256;
	int32		elem_id = 0;
	int32		ret_status = 1;
	int32		*prod_status = NULL;
	int32		active_product_found = 1;
	int32		day = 0;
	int32           err = 0;
	int32		grace_flag = 0;
	int32		topup_flag = 0;
	int32		end_unit = 0;
	int32		end_offset = 0;
	int32		start_unit = 0;
	int32		start_offset = 0;
	int32		diff_time = 0;
	int             *netflix_grace_days = NULL;
	int64		db = 0;
	pin_cookie_t	cookie = NULL;
        char            *offer_descr = NULL;
        char            *status_descr = NULL;
        char            *search_template = "select X from /profile_cust_offer where F1 = V1 ";
	char            *srch_svc_temp = "select X from /service/netflix where F1 like V1 AND F2.id = V2 ";
	char            *pln_srch_template = "select X from /deal 1, /plan 2  where  2.F1 = V1 and 2.F2 = 1.F3 ";
        char            *plan_name = NULL;
	char            *planname  = NULL;
        char            *prog_name = "Netflix Renew";
	char            msg[512];

	time_t          *cycle_end_t = NULL;
	time_t          *new_cycle_end_t = NULL;
	time_t          new_end_t = 0;
	time_t          start_t = 0;
	time_t          new_start_t = 0;
	time_t          current_time = 0;
	time_t          expiry = 0;

	struct tm       *timeeff;

	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );
        db = PIN_POID_GET_DB(account_obj);
        srch_flistp = PIN_FLIST_CREATE(ebufp);
     	 *ret_flistpp = PIN_FLIST_CREATE(ebufp);
	 PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_POID, account_obj, ebufp );
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
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &ret_status, ebufp);
		*ret_flistpp = ret_flistp;
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                PIN_ERRBUF_RESET(ebufp);
		goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer profile search output flist", srch_out_flistp);
        while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                offer_descr = PIN_FLIST_FLD_GET(results_flistp, MSO_FLD_OFFER_DESCR, 1, ebufp);
		if (offer_descr && strcmp(offer_descr,"Netflix") ==0 )
                {
			srch_flistp = PIN_FLIST_CREATE(ebufp);
        		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, srch_svc_temp, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
                        PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/netflix", -1, ebufp), ebufp);
        		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
 		        results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);

		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix service search input list", srch_flistp);
		        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_svc_o_flistp, ebufp);
        		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		        if (PIN_ERRBUF_IS_ERR(ebufp) || !srch_svc_o_flistp ) {
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &ret_status, ebufp);
				*ret_flistpp = ret_flistp;
		                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                		PIN_ERRBUF_RESET(ebufp);
				goto cleanup;
		        }
		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix service search output flist", srch_svc_o_flistp);

			result_srch_flistp = PIN_FLIST_ELEM_GET(srch_svc_o_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix service search result flist", result_srch_flistp);
			if (result_srch_flistp) {
				srvc_pdp = PIN_FLIST_FLD_GET(result_srch_flistp, PIN_FLD_POID, 1, ebufp);
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Netflix service POID", srvc_pdp);
        		}
			//Get Netflix product from DB Kali	
			netflix_srch_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(netflix_srch_flistp, PIN_FLD_POID, account_obj, ebufp);
			PIN_FLIST_FLD_SET(netflix_srch_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
                       	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix active product search");
		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix purchased product active input flist", netflix_srch_flistp);
			fm_mso_get_purchased_prod_all(ctxp,netflix_srch_flistp,&netflix_prod_list,ebufp);	
		        if (netflix_prod_list != NULL && (PIN_FLIST_ELEM_COUNT(netflix_prod_list, PIN_FLD_RESULTS, ebufp) > 0 )){
                       	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix active product out");
		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix purchased product active output flist", netflix_prod_list);
				elem_id =0;
				cookie = NULL;
                		while ((pur_prod_flistp = PIN_FLIST_ELEM_GET_NEXT(netflix_prod_list, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL ){
                       	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix active product");
		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix active product flist", pur_prod_flistp);
                        		new_cycle_end_t = PIN_FLIST_FLD_GET(pur_prod_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
					if(new_cycle_end_t)
					{
                       	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix cycle_end_t found");
						if((cycle_end_t) && (*cycle_end_t > *new_cycle_end_t))
						{
			sprintf(msg,"new_cycle_end_t time %d",*new_cycle_end_t);
                       	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
							continue;	
						}
						else
						{
			sprintf(msg,"new_cycle_end_t time %d",*new_cycle_end_t);
                       	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
							cycle_end_t = new_cycle_end_t;
			sprintf(msg,"cycle_end_t time %d",*cycle_end_t);
                       	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
                                			prod_status = (int *) PIN_FLIST_FLD_GET(pur_prod_flistp, PIN_FLD_STATUS, 1, ebufp);
                                			pur_prod_poid = PIN_FLIST_FLD_GET(pur_prod_flistp, PIN_FLD_POID, 1, ebufp);
                                			prod_poid = PIN_FLIST_FLD_GET(pur_prod_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
							if(prod_status && *prod_status == 1)
							{
								active_product_found = 1;
                       	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Kali Netflix active product found");
							}
						}
					}
					else
					{
						continue;
					}
                		}
        		}

			pin_conf("fm_mso_cust", "netflix_grace_period", PIN_FLDT_INT, (caddr_t*)&netflix_grace_days, &err );
			current_time = pin_virtual_time(NULL);
			sprintf(msg,"current time %d",current_time);
                       	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			if(netflix_grace_days)
			{
				sprintf(msg,"Grace days %d",*netflix_grace_days);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}
			if(cycle_end_t)
			{
				sprintf(msg,"Cycle End t %d",*cycle_end_t);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				if(netflix_grace_days)
				{
					expiry = *cycle_end_t + (*netflix_grace_days * 86400);
				}
				else
				{
					expiry = *cycle_end_t;
				}
				if((*cycle_end_t < current_time) && (current_time <= expiry))
				{
					grace_flag = 1;
				}
				if(*cycle_end_t > current_time)
				{
					topup_flag = 1;
				}
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Cycle End time 0");
			}
			sprintf(msg,"expiry time %d",expiry);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

			end_offset = 1;
			end_unit = 5;
			if(topup_flag == 1)
			{
				//Topup case calculate start date
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Topup Case");
				start_unit = 4;
				//diff_time = get_diff_time(ctxp,*cycle_end_t, current_time,ebufp);
				diff_time = *cycle_end_t - current_time;

				sprintf(msg,"difference time : %d",diff_time);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

				day = diff_time / 86400;

				sprintf(msg,"difference days : %d",day);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				start_offset = day + 1;

			}	
			else if(grace_flag == 1)
			{
				//Grace period is on. Back dated purchase
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Grace Period Case");
				start_t =  *cycle_end_t + 86400;
				new_start_t = fm_utils_time_round_to_midnight(start_t);
				sprintf(msg,"Backdate date : %d",new_start_t);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

			}

			else
			{
				//Other cases. Start time and end time shoud not be set
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Other case purchase on same date");
			}
			//find out Netflix deal and product details using plan poid
			plan_name =(char *) PIN_FLIST_FLD_GET(in_flist, MSO_FLD_PLAN_NAME, 1, ebufp);
			if(plan_name){
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
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Deal and Product search Input Flist");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix Deal and Product search input flist", srch_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &pln_srch_o_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp) || !pln_srch_o_flistp) {
				PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_DESCR, "Not able to find Netflix Deal or product ", ebufp);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Netflix Deal and Product  SEARCH", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Deal and Product search Output Flist");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix Deal and Product search output flist", pln_srch_o_flistp);
			rslt_srch_flistp= PIN_FLIST_ELEM_GET(pln_srch_o_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix Deal and Product search result flist", rslt_srch_flistp);
			if (rslt_srch_flistp) {
				dl_pdp = PIN_FLIST_FLD_GET(rslt_srch_flistp, PIN_FLD_POID, 1, ebufp);
				pdarr_flistp = PIN_FLIST_ELEM_GET(rslt_srch_flistp,PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);
				pd_pdp = PIN_FLIST_FLD_GET(pdarr_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);

			//Call for netflix purchased deal
			dl_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_DEAL_OBJ, dl_pdp, ebufp);
			PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_PRODUCT_OBJ, pd_pdp, ebufp);
			PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_POID, account_obj, ebufp);
			PIN_FLIST_FLD_SET(dl_flistp,PIN_FLD_PLAN_OBJ,PIN_POID_FROM_STR(plan_name, NULL, ebufp), ebufp);
			PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
			PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
			PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_CYCLE_END_OFFSET, &end_offset, ebufp);
			PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_CYCLE_END_UNIT, &end_unit, ebufp);
			if(grace_flag == 1)
			{
				//Back dated purchased set end_t
				PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_END_T, &new_start_t, ebufp);
			}

			if(start_unit == 4)
			{
				PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_CYCLE_START_OFFSET, &start_offset, ebufp);
				PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_CYCLE_START_UNIT, &start_unit, ebufp);
			}

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Purchase deal Input Flist");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix purchase deal input flist", dl_flistp);
			fm_mso_purchase_deal(ctxp, dl_flistp, &dl_o_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Purchase deal output Flist");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix subscription output flist", dl_o_flistp);
			if (dl_o_flistp) {
				ret_status = 0;
				*ret_flistpp= NULL;
				*ret_flistpp= PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, account_obj, ebufp);

				//PIN_FLIST_FLD_SET(dl_o_flistp, PIN_FLD_STATUS, &ret_status, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_DESCR, "Success : Netflix Service Renewed Succesfully", ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix subscription completed", dl_o_flistp);
				//*ret_flistpp = PIN_FLIST_COPY(dl_o_flistp, ebufp);
				//find out the netflix plan name using plan poid
				/*rd_flds_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(rd_flds_flistp,PIN_FLD_POID,PIN_POID_FROM_STR(plan_name, NULL, ebufp), ebufp);
				PIN_FLIST_FLD_SET(rd_flds_flistp,PIN_FLD_NAME,NULL, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix plan name search inputflist", rd_flds_flistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rd_flds_flistp, &rd_flds_o_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&rd_flds_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp) || !rd_flds_o_flistp) {
					PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistpp , PIN_FLD_DESCR, "Not able to find Netflix plan name ", ebufp);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in finding Netflix plan name", ebufp);
					PIN_ERRBUF_RESET(ebufp);
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix plan name search outflist", rd_flds_o_flistp);
				planname = PIN_FLIST_FLD_GET(rd_flds_o_flistp, PIN_FLD_NAME, 0, ebufp);
				*/
				prog_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 1, ebufp );
				if (!prog_name) {
					prog_name = "Netflix Renew";
				}


				//Lifecycle Kali Add plan pdp on input 
				PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_PLAN_OBJ, PIN_POID_FROM_STR(plan_name, NULL, ebufp), ebufp);
				PIN_FLIST_FLD_SET(dl_flistp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
				lc_out_flist = PIN_FLIST_CREATE(ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return flist ", *ret_flistpp);
				fm_mso_create_lifecycle_evt(ctxp, dl_flistp, lc_out_flist, ID_RENEW_PREPAID, ebufp );
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside Netflix renewal");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return flist after lifecycle event", *ret_flistpp);

			}
			}
			}else{
				PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_DESCR, "Not able to find Netflix Deal or product ", ebufp);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Netflix Deal and Product  SEARCH", ebufp);
				goto cleanup;
			}
		}
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		ret_status = 0;
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &ret_status, ebufp);
		*ret_flistpp = ret_flistp;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in renew netflix service", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return flist after lifecycle event 2", *ret_flistpp);
cleanup :
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&dl_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&dl_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&lc_out_flist, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return flist after lifecycle event 3", *ret_flistpp);
}
