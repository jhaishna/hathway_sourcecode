
/*******************************************************************
 * File:        fm_mso_commission_manage_agreement.c
 * Opcode:      MSO_OP_COMMISSION_MANAGE_AGREEMENT
 * Description: This policy opcode facilitate to add, change and cancel agreement for LCO account. 
 				Where the agreement is nothing but a pricing plan which has commission rates
				configured for different commission rules.
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_commission_manage_agreement.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_commission.h"


/*******************************************************************
 * MSO_OP_COMMISSION_MANAGE_AGREEMENT
 *
 * This policy is used to add, change, remove agreement (plan) for 
 * an LCO account. 
 *******************************************************************/

/*************************************
External subroutines- Start
**************************************/
extern int
fm_mso_validate_csr_role(
        pcm_context_t           *ctxp,
        int64                   db,
        poid_t          	*user_id,
        pin_errbuf_t            *ebufp);

/*************************************
External subroutines- End
**************************************/


EXPORT_OP void
op_mso_commission_manage_agreement (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


static void
fm_mso_commission_manage_agreement (
        pcm_context_t            *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_manage_agreement(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *in_flist,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

static int fm_mso_commission_get_lco_agreement(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flist,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_utils_get_services_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        char                    *ser_type,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_utils_prepare_status_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        int                     flag,
        char                    *status_descr,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_utils_prepare_error_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        char                    *error_code,
        char                    *error_descr,
        pin_errbuf_t            *ebufp);

void
op_mso_commission_manage_agreement (
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

        pcm_context_t           *ctxp = connp->dm_ctx;
        *r_flistpp              = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != MSO_OP_COMMISSION_MANAGE_AGREEMENT) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_commission_manage_agreement error",ebufp);
                return;
        }
        /*******************************************************************

        /*******************************************************************
         * Debug: Input flist
         *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_mso_commission_manage_agreement input flist", i_flistp);
        fm_mso_commission_manage_agreement(ctxp, flags, i_flistp, r_flistpp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_commission_manage_agreement output flist", *r_flistpp);
        return;
}

/*******************************************************************
 * Function: fm_mso_commission_manage_agreement
 *
 * This function is used to add, change, remove the agreement for 
 * an LCO accounts. Function expects the below things:
 *      1. Action Mode (1-Add, 2-Change, 3-Remove Agreement)
 *      2. LCO Account POID
 *      3. Plan List which includes agreement (plan) to be added
 *
 * Arguments:
 *      1. Context Pointer
 *      2. Flags
 *      3. Input Flist
 *      4. Output Flist  [Used to return the status of the action]
 *      5. Error Buffer
 *
 * Return value: None
 *******************************************************************/

static void
fm_mso_commission_manage_agreement(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t             *ret_flistp = NULL;
        poid_t                  *routing_poid = NULL;
        poid_t                  *account_obj = NULL;
        poid_t                  *user_id = NULL;

        int64                   db = -1;
        int                     csr_access = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_manage_agreement input flist", i_flistp);

        routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
        if (routing_poid) {
                db = PIN_POID_GET_DB(routing_poid);
                account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
                user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
        }
        else {
		fm_mso_utils_prepare_error_flist(ctxp,i_flistp,&ret_flistp,		
			"51412","POID value not passed in input as it is mandatory field", ebufp);
                goto CLEANUP;
        }

        if (!account_obj) {
                fm_mso_utils_prepare_error_flist(ctxp,i_flistp,&ret_flistp,
                        "51413","Account POID value not passed in input as it is mandatory field", ebufp);
                goto CLEANUP;
        }

        /*******************************************************************
        * Validation for PRICING_ADMIN roles - Start
        *******************************************************************/
        if (user_id)
        {
                csr_access = fm_mso_validate_csr_role(ctxp, db, user_id, ebufp);
                if ( csr_access == 0) {
                	fm_mso_utils_prepare_error_flist(ctxp,i_flistp,&ret_flistp,
                        	"51414","ROLE passed in input does not have permissions to change the service status", ebufp);
                        goto CLEANUP;
                }	
		else {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR haas permission to change account information");
                }
        }
		else {
                fm_mso_utils_prepare_error_flist(ctxp,i_flistp,&ret_flistp,
                        "51415","User ID value not passed in input as it is mandatory field", ebufp);
                goto CLEANUP;
        }

        /*******************************************************************
        * Validation for PRICING_ADMIN roles  - End
        *******************************************************************/

        fm_mso_manage_agreement(ctxp, 0, i_flistp, &ret_flistp, ebufp);

CLEANUP:
        *r_flistpp = ret_flistp;
        return;
}


/*******************************************************************
 * Function: fm_mso_manage_agreement
 *
 * This function is used to add, change, remove agreement (plan) to
 * an LCO account. 
 *
 * Arguments:
 *      1. Context Pointer
 *      2. Action Mode (1-Add, 2-Change, 3-Remove Agreement)
 *      3. Input Flist
 *      4. Output Flist (Used to return the status)
 *      5. Error Buffer
 *
 * Return value: None
 *******************************************************************/

static void
fm_mso_manage_agreement(
        pcm_context_t           *ctxp,
        int32           		flags,
        pin_flist_t             *in_flist,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp) {

        pin_flist_t             *planlists_flistp = NULL;
        pin_flist_t             *purchasedeal_inflistp = NULL;
        pin_flist_t             *purchasedeal_outflistp = NULL;
        pin_flist_t             *dealinfo_flistp = NULL;
        pin_flist_t             *deals_flistp = NULL;
        pin_flist_t             *readplan_inflistp = NULL;
        pin_flist_t             *readplan_outflistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *serv_info_flistp = NULL;
        pin_flist_t             *canceldeal_inflistp = NULL;  
        pin_flist_t             *canceldeal_outflistp = NULL;  
  
        poid_t                  *read_deal_obj = NULL;  
        poid_t                  *plan_obj = NULL;
        poid_t                  *get_deal_obj = NULL;

        char 					msg[512] ;
        int                     elem_id = 0;
        int                     elem_base = -1;
        int                     elem_countdeals = 0;       
        int                     *package_id = 0;         
 
        pin_cookie_t            cookie_countdeals = NULL;    

        int                     *action_mode = NULL;
        //int32                   failure = 1;

        pin_cookie_t            cookie = NULL;
        pin_cookie_t            cookie_base = NULL;
        poid_t                  *serv_pdp= NULL;
        pin_flist_t             *ret_flist= NULL;
        int32                   result_count = 0;
        int32                   *cancel_flagp = NULL;
        int32                   cancel_flag = 0;
        int32                   err_flag = 0;


        if (PIN_ERRBUF_IS_ERR(ebufp))
			return ;

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_manage_agreement input flist", in_flist);

		action_mode = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACTION_MODE, 1, ebufp);
		if (!action_mode) {
			fm_mso_utils_prepare_error_flist(ctxp,in_flist,&r_flistp,
				"51416","MANAGE AGREEMENT - ACTION_MODE is missing in input", ebufp);
			*ret_flistp = r_flistp;
			return;
		}

		while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, &elem_base, 1, &cookie_base, ebufp )) != NULL)
		{
			elem_countdeals = 0;
			plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
			cancel_flagp = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_FLAGS, 1, ebufp );
			elem_countdeals=PIN_FLIST_ELEM_COUNT(planlists_flistp, PIN_FLD_DEALS, ebufp);


			/*Check whether the PIN_FLD_PLAN_LISTS array for cancel is placed first or not*/
			if (*action_mode==2 && (elem_countdeals == 0 && cancel_flag != 1)) {
				fm_mso_utils_prepare_error_flist(ctxp,in_flist,&r_flistp,
					"51417","MANAGE AGREEMENT - Plan list should be contain the deal to be cancel first", ebufp);
				*ret_flistp = r_flistp;
				return;
			}

			/*Check whether the plan object present in the input flist or not*/
			if (!plan_obj) {
				fm_mso_utils_prepare_error_flist(ctxp,in_flist,&r_flistp,
					"51418","MANAGE AGREEMENT - Plan object is missing in input", ebufp);
					*ret_flistp = r_flistp;
					return;
			}

			/*Read the Plan to get the services and deals Start*/
			readplan_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_POID, plan_obj, ebufp );
			PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_NAME, NULL, ebufp );
			deals_flistp = PIN_FLIST_ELEM_ADD(readplan_inflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_SET(deals_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp );
			PIN_FLIST_FLD_SET(deals_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_manage_agreement read plan input list", readplan_inflistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readplan_inflistp, &readplan_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&readplan_inflistp, NULL);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);

				fm_mso_utils_prepare_error_flist(ctxp,in_flist,&r_flistp,
					"51419","MANAGE AGREEMENT - PCM_OP_READ_FLDS of plan poid error", ebufp);
				*ret_flistp = r_flistp;
				goto cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_manage_agreement read plan output flist", readplan_outflistp);
			/*Read the Plan to get the services and deals End*/

			/*Get and Set the LCO Service Outcollect Object Start*/
			fm_mso_utils_get_services_info(ctxp, in_flist, &serv_info_flistp, "/service/settlement/ws/outcollect", ebufp);
			if (serv_info_flistp != NULL) {
				ret_flist = PIN_FLIST_FLD_GET(serv_info_flistp, PIN_FLD_RESULTS, 0, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "flist", ret_flist);
				if (ret_flist == NULL ) {
				    PIN_ERRBUF_CLEAR(ebufp);
                                    fm_mso_utils_prepare_error_flist(ctxp,in_flist,&r_flistp,
                                        "51420","MANAGE AGREEMENT - Wholesale Settlement Services were not found", ebufp);
                                    *ret_flistp = r_flistp;
                                    return;
				}
				serv_pdp = PIN_FLIST_FLD_GET(ret_flist, PIN_FLD_POID, 0, ebufp);
			}
			/*Get and Set the LCO Service Outcollect Object End*/

			/*According to the flag purchase or cancel the deal Start*/
			elem_id = 0;
			cookie = NULL;
			while ((deals_flistp = PIN_FLIST_ELEM_GET_NEXT(readplan_outflistp, PIN_FLD_SERVICES, &elem_id, 1, &cookie, ebufp)) != NULL)
			{
				/*Get and Set the LCO Service Outcollect Object Start*/
				fm_mso_utils_get_services_info(ctxp, in_flist, &serv_info_flistp, 
				   (char*)PIN_POID_GET_TYPE (PIN_FLIST_FLD_GET(deals_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp)), ebufp);

				if (serv_info_flistp != NULL) {
					ret_flist = PIN_FLIST_FLD_GET(serv_info_flistp, PIN_FLD_RESULTS, 0, ebufp);
                                	if (ret_flist == NULL) {
					    PIN_ERRBUF_CLEAR(ebufp);
                                    	    fm_mso_utils_prepare_error_flist(ctxp,in_flist,&r_flistp,
                                        	"51420","MANAGE AGREEMENT - Wholesale Settlement Services were not found", ebufp);
                                    	    *ret_flistp = r_flistp;
                                    	    return;
                                	}
					serv_pdp = PIN_FLIST_FLD_GET(ret_flist, PIN_FLD_POID, 0, ebufp);
				}
				/*Get and Set the LCO Service Outcollect Object End*/

				/* Perform cancel deal if action_mode not equal to NULL*/
				if (*action_mode != 1 && cancel_flag != 1) 
				{                      
					canceldeal_inflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, canceldeal_inflistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, canceldeal_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
					dealinfo_flistp = PIN_FLIST_ELEM_ADD(canceldeal_inflistp, PIN_FLD_DEAL_INFO, 0, ebufp );
					PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, dealinfo_flistp, PIN_FLD_DEAL_OBJ, ebufp);
					
					read_deal_obj = PIN_FLIST_FLD_GET(deals_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
					
					elem_countdeals = 0;
					cookie_countdeals = NULL;
						
					while ((args_flistp = PIN_FLIST_ELEM_GET_NEXT(planlists_flistp, PIN_FLD_DEALS, &elem_countdeals, 1, &cookie_countdeals, ebufp)) != NULL)
					{
						get_deal_obj = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
						if ((PIN_POID_COMPARE(read_deal_obj, get_deal_obj, 0, ebufp)) == 0)
						{
							package_id = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_PACKAGE_ID, 1, ebufp);
							break;
						}
					}
					
					PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_PACKAGE_ID, package_id, ebufp);
					PIN_FLIST_FLD_SET(canceldeal_inflistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
					
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_agreement input list", canceldeal_inflistp);
					PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_CANCEL_DEAL, 0, canceldeal_inflistp, &canceldeal_outflistp, ebufp);
					PIN_FLIST_DESTROY_EX(&canceldeal_inflistp, NULL);

					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_CANCEL_DEAL", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
						
						if(*action_mode == 2) {
							fm_mso_utils_prepare_error_flist(ctxp, in_flist, &r_flistp, 
								"51420","MANAGE AGREEMENT - Change Agreement - PCM_OP_SUBSCRIPTION_CANCEL_DEAL error", ebufp);
						}
						else if(*action_mode = 3) {
							fm_mso_utils_prepare_error_flist(ctxp, in_flist, &r_flistp, 
								"51421","MANAGE AGREEMENT - Cancel Agreement - PCM_OP_SUBSCRIPTION_CANCEL_DEAL error", ebufp);						
						}
						*ret_flistp = r_flistp;
						goto cleanup;
					}
					cancel_flag = 1;
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_agreement output flist", canceldeal_outflistp);

					result_count=PIN_FLIST_ELEM_COUNT(canceldeal_outflistp,PIN_FLD_RESULTS,ebufp);
					if(result_count==0) {
						fm_mso_utils_prepare_error_flist(ctxp, in_flist, &r_flistp,
							"51422","MANAGE AGREEMENT - No plan to cancel",ebufp);
						*ret_flistp = r_flistp;
						goto cleanup;
					}

					if(result_count==1) {
						fm_mso_utils_prepare_error_flist(ctxp, in_flist, &r_flistp,
							"51423","MANAGE AGREEMENT - Already an agreement is cancelled",ebufp);
						*ret_flistp = r_flistp;
						goto cleanup;
					}
					PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
				}
				else {
					result_count = fm_mso_commission_get_lco_agreement(ctxp, in_flist, ebufp);
					if(result_count > 0) {
						fm_mso_utils_prepare_error_flist(ctxp,in_flist,&r_flistp,
							"51424","MANAGE AGREEMENT - Already Active Agreement is available", ebufp);
						*ret_flistp = r_flistp;
						goto cleanup;
					}
					purchasedeal_inflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, purchasedeal_inflistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, purchasedeal_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
					
					dealinfo_flistp = PIN_FLIST_ELEM_ADD(purchasedeal_inflistp, PIN_FLD_DEAL_INFO, 0, ebufp );
					PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, dealinfo_flistp, PIN_FLD_DEAL_OBJ, ebufp);
					PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
					
					PIN_FLIST_FLD_SET(purchasedeal_inflistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_manage add deal input list", purchasedeal_inflistp);
					PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, 0, purchasedeal_inflistp, &purchasedeal_outflistp, ebufp);
					PIN_FLIST_DESTROY_EX(&purchasedeal_inflistp, NULL);

					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);
						
						if(*action_mode == 1) {
							fm_mso_utils_prepare_error_flist(ctxp, in_flist, &r_flistp, 
								"51425","MANAGE AGREEMENT - Add Agreement - PCM_OP_SUBSCRIPTION_PURCHASE_DEAL error", ebufp);
						}
						else if(*action_mode = 2) {
							fm_mso_utils_prepare_error_flist(ctxp, in_flist, &r_flistp, 
								"51426","MANAGE AGREEMENT - Change Agreement - PCM_OP_SUBSCRIPTION_PURCHASE_DEAL error", ebufp);						
						}						
						*ret_flistp = r_flistp;
						goto cleanup;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_manage add deal output flist", purchasedeal_outflistp);
					PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);
				}

			}
			/*According to the flag purchase or cancel the deal End*/
		} 
		if( planlists_flistp == NULL && elem_base == -1) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"error");
			fm_mso_utils_prepare_error_flist(ctxp, in_flist, &r_flistp,
				"51427", "MANAGE AGREEMENT - PIN_FLD_PLAN_LISTS is NULL",ebufp);
			*ret_flistp = r_flistp;
		}

		/*Setting the success response message */
		if(err_flag!=1) {
			switch (*(int*)action_mode) {
				case 1: strcpy(msg, "MANAGE AGREEMENT - Add Agreement completed successfully"); break;
				case 2: strcpy(msg, "MANAGE AGREEMENT - Change Agreement completed successfully"); break;
				case 3: strcpy(msg, "MANAGE AGREEMENT - Cancel Agreement completed successfully"); break;
			}         
			fm_mso_utils_prepare_status_flist(ctxp, in_flist, &r_flistp, PIN_FLD_ACCOUNT_OBJ, msg, ebufp);
			*ret_flistp = r_flistp;
		}
cleanup:
		PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&serv_info_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);
		return;
}


static int fm_mso_commission_get_lco_agreement(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flist,
	pin_errbuf_t            *ebufp) {
	
	/* Variable Declaration Start */
	pin_flist_t             *flistp=NULL;
	pin_flist_t             *flistp0=NULL;
	pin_flist_t             *ret_flistp=NULL;
	poid_t                  *tmp_pdp0=NULL;
	int32                   tmp_enum0=256;
	char                    *tmp_str0="select X from /purchased_product 1, /plan 2 where 1.F1 = V1 and 1.F2.type = V2 and 1.F3 = 2.F4 and purchased_product_t.status = 1 ";
	poid_t                  *tmp_pdp1=NULL;
	poid_t                  *tmp_pdp2=NULL;
	//poid_t                  *tmp_pdp3=NULL;
	//poid_t                  *tmp_pdp4=NULL;
	int32                   result_count=0;
	pin_cookie_t            cookie=NULL;
	int64                   db=1;
	char			msg [800];
	/* Variable Declaration End */
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return ;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_lco_agreement input flist", in_flist);

	flistp = PIN_FLIST_CREATE(ebufp);
	tmp_pdp0 = PIN_POID_CREATE(PIN_POID_GET_DB(PCM_GET_USERID(ctxp)), "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(flistp,PIN_FLD_POID, tmp_pdp0, ebufp);
	PIN_FLIST_FLD_SET(flistp,PIN_FLD_FLAGS, (void*)&tmp_enum0, ebufp);
	PIN_FLIST_FLD_SET(flistp,PIN_FLD_TEMPLATE, tmp_str0, ebufp);
	flistp0 = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 1, ebufp);
	tmp_pdp1 = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp0,PIN_FLD_ACCOUNT_OBJ, tmp_pdp1, ebufp);
	flistp0 = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 2, ebufp);
	tmp_pdp2 = PIN_POID_CREATE(db, "/service/settlement/ws/outcollect", -1, ebufp);
	PIN_FLIST_FLD_PUT(flistp0,PIN_FLD_SERVICE_OBJ, tmp_pdp2, ebufp);
	flistp0 = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_PUT(flistp0,PIN_FLD_PLAN_OBJ, NULL, ebufp);
	flistp0 = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_PUT(flistp0,PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_lco_agreement search input flist", flistp);
	if ( PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fm_mso_commission_get_lco_agreement search", ebufp);
		goto cleanup;
	}
	
	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_COUNT_ONLY, flistp, &ret_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_lco_agreement search output flist", ret_flistp);
	
	flistp0 = PIN_FLIST_ELEM_GET_NEXT(ret_flistp, PIN_FLD_RESULTS, &result_count, 1, &cookie, ebufp);
	
	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_commission_get_lco_agreement error in Purchased Plan Count ",ebufp);
	}
	sprintf(msg, "Number of Active plans : %d", result_count); 
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	
cleanup:
	/* Destroy Section Start */
	PIN_FLIST_DESTROY_EX(&flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	/* Destroy Section End */
	
	return result_count;
	
}
