#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_operations_action.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_operations.h"
#include "mso_lifecycle_id.h"



extern void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);


/**************************************
* Local Functions start
***************************************/
static int
fm_mso_action(
        pcm_context_t           *ctxp,
	int32			action_flag,
        pin_flist_t             *in_flist,
        pin_flist_t             **out_flist,
        pin_errbuf_t            *ebufp);

/*******************************************************************************
Function to filter the dd object's specification by getting the below fields
   PIN_FLD_FIELD_NAME
   PIN_FLD_FIELD_TYPE 
********************************************************************************/
void filter_class_definition (
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp, 
	pin_flist_t		**r_flistpp, 
	pin_errbuf_t	*ebufp);

/*******************************************************************************
Function processes the dd object's specification recursively and prepares 
   conslidated input flist 
********************************************************************************/
void process_class_definition ( 
	pcm_context_t	*ctxp,
	int64		database,
	char		*class_name,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp );

/*******************************************************************************
Function parses the input class name and calls the process class definition 
   for base class and further sub classes to consolidate the
   configuration object specification 
********************************************************************************/
void parse_storable_class (
	pcm_context_t	*ctxp,
	int64		database,
	char 		*class_name,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

/*******************************************************************************
Function to complare the configuration object specification and the input
   Returns 1 in case of success else it returns 0 
********************************************************************************/
int compare_object (
        pcm_context_t   *ctxp,
        pin_flist_t     *object_spec_flistp,
        pin_flist_t     *object_flistp,
	char		*str,
        pin_errbuf_t    *ebufp);

/********************************************************************************
Function to navigate through the input flist of the configuration object to 
   be created and compares against the configuration specification. 
   Returns 1 in case of success else it returns 0 
********************************************************************************/
int navigate_input_flist (
    	pcm_context_t   *ctxp,
    	pin_flist_t     *i_flistp,
    	pin_flist_t     *obj_spec_flistp,
    	char            *path,
        pin_flist_t	**err_flistp,
    	pin_errbuf_t    *ebufp) ;



static int32 handle_cmts_cc_mapping(pcm_context_t   *ctxp,
                                pin_flist_t *input_flistp,
                                pin_flist_t **ret_flistp,
                                int32   action,
                                pin_errbuf_t    *ebufp);

static int32 handle_credit_limit(pcm_context_t   *ctxp,
                                pin_flist_t *input_flistp,
                                int32   action,
                                pin_errbuf_t    *ebufp);

static int32 add_cities_in_credit_limit(pcm_context_t   *ctxp,
                                pin_flist_t *input_flistp,
                                int32   action,
                                pin_flist_t **ret_flistp,
                                pin_errbuf_t    *ebufp);

static void
fm_mso_end_config(
        pcm_context_t   *ctxp,
        pin_flist_t     *input_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp);

#define ADD_CITIES_ENTRY  6
/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_OPERATIONS_ACTION
 *
 * This policy is used to manage the configuration objects through OAP
 * And it supports the below actions 
 * 1. Creating Configuration Object 
 * 2. Updating Configuration Object 
 * 3. Deleting Configuration Object Array
 * 4. Deleting Configuration Object 
 * 5. Search Configuration Object 
 *******************************************************************/

EXPORT_OP void
op_mso_operations_action(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_operations_action(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_OPERATIONS_ACTION
 *******************************************************************/

void
op_mso_operations_action(
        cm_nap_connection_t     *connp,
        int32                   opcode,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;

        *r_flistpp              = NULL;
        int                     *status = NULL;
        int32                   status_uncommitted =2;
        poid_t                  *inp_pdp = NULL;
        char            *prog_name = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        /*******************************************************************
         * Insanity Check
         *******************************************************************/
        if (opcode != MSO_OP_OPERATIONS_ACTION) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_operations_action error",
                        ebufp);
                return;
        }

        /*******************************************************************
         * Debug: Input flist
         *******************************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_mso_operations_action input flist", i_flistp);

        fm_mso_operations_action(ctxp, flags, i_flistp, r_flistpp, ebufp);
        status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

        /***********************************************************
         * Results.
         ***********************************************************/
        if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1))
        {
               PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_operations_action error", ebufp);
        }
        else
        {
               PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_operations_action output flist", *r_flistpp);
        }
        return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/

static void
fm_mso_operations_action(
        pcm_context_t           *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *ret_flistp = NULL;
        poid_t                  *routing_poid = NULL;
        poid_t                  *user_id = NULL;

        int			action_success = 0;
        int			action_failure = 1;
        int			acct_update = 0;
        int			csr_access = 0;
        int			*flag_ptr = NULL;
        int			*ops_task_ptr = NULL;
        pin_flist_t		*input_flistp = NULL;
        pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*lc_info = NULL;
        int64                   db = -1;
	int 			valid_action = 0; 
	char			msg[256];
	char			error_descr[256];
	char			cfg_obj_type[256];
	pin_flist_t		*cfg_obj_spec_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	poid_t			*cfg_poid = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_operations_action input flist", i_flistp);

        routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
        if (routing_poid)
        {
                db = PIN_POID_GET_DB(routing_poid);
                user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
                flag_ptr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_MODE, 1, ebufp);
                ops_task_ptr = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_OPERATION_TASK, 1, ebufp);
		input_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
		if (input_flistp) {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_operation_action data array input flist", input_flistp);
		}
		else {
                	ret_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &action_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51503", ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Config object input flist not passed in input", ebufp);
			goto CLEANUP;
		}
        }
        else
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &action_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51504", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
                goto CLEANUP;
        }

        /*******************************************************************
        * Validation for PRICING_ADMIN roles - Start
        *******************************************************************/

        if (user_id)
        {
                csr_access = fm_mso_validate_csr_role(ctxp, db, user_id, ebufp);

                if ( csr_access == 0)
                {
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &action_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51505", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ROLE passed in input does not have permissions to change the service status", ebufp);
                        goto CLEANUP;
                }
		else
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR has permission to change account information");
                }
        }else
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &action_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51506", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
                goto CLEANUP;
        }

        /*******************************************************************
        * Validation for PRICING_ADMIN roles  - End
        *******************************************************************/

	/******************************************************************
	* Configuration Selection - Start
	******************************************************************/
	lc_info = PIN_FLIST_ELEM_ADD(i_flistp, MSO_FLD_OPS_CREATE_CONFIG, 1, ebufp);
	switch (*ops_task_ptr) {
		case CMTS_MASTER_CONF:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CMTS_MASTER_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_cmts_master");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "CMTS MASTER CONFIG",  ebufp);
			break; 
		case BB_CLIENT_CLASS_MASTER_CONF:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "BB_CLIENT_CLASS_MASTER_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_client_class_type");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "BB CLIENT CLASS MASTER CONFIG",  ebufp);
			break; 
		case BB_CLIENT_CLASS_CMTS_MASTER_CONF:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "BB_CLIENT_CLASS_CMTS_MASTER_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_cmts_cc_mapping");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "BB CLIENT CLASS CMTS MASTER CONFIG",  ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," before break");
			break; 
		case AREA_LEVEL_BOUQUET_CONF:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AREA_LEVEL_BOUQUET_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_bouquet_area_map");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "AREA LEVEL BOUQUET CONFIG",  ebufp);
			break; 
		case CITY_LEVEL_BOUQUET_CONF:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CITY_LEVEL_BOUQUET_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_bouquet_city_map");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "CITY LEVEL BOUQUET CONFIG",  ebufp);
			break; 
		case CATV_CHANNEL_LIST_CONF:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV_CHANNEL_LIST_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_catv_channel_master");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "CATV CHANNEL LIST CONFIG",  ebufp);
			break; 
		case CATV_PROVISIONING_TAG_CONF:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV_PROVISIONING_TAG_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_catv_pt");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "CATV PROVISIONING TAG CONFIG",  ebufp);
			break; 
		case CATV_PT_CHANNEL_MAPPING_CONF:
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV_PT_CHANNEL_MAPPING_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_product_channel_map");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "CATV PT CHANNEL MAPPING CONFIG",  ebufp);
            fm_mso_end_config(ctxp, input_flistp, &ret_flistp, ebufp);
			break; 
		case CATV_PLAN_PRODUCT_MAPPING_CONF:
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV_PLAN_PRODUCT_MAPPING_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_plan_products_map");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "CATV PLAN PRODUCTS MAPPING CONFIG",  ebufp);
            fm_mso_end_config(ctxp, input_flistp, &ret_flistp, ebufp);
			break; 
		case BB_PROVISIONING_TAG:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "BB_PROVISIONING_TAG");
			strcpy(cfg_obj_type,"/mso_config_bb_pt");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "BB PROVISIONING TAG CONFIG",  ebufp);
			break; 
		case BB_CREDIT_LIMIIT_MANAGEMENT_CONF:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "BB_CREDIT_LIMIIT_MANAGEMENT_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_credit_limit");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "BB CREDIT LIMIIT MANAGEMENT CONFIG",  ebufp);
			handle_credit_limit(ctxp, input_flistp, *flag_ptr, ebufp);
			break; 
		case INV_TEMPLATE_VER_MASTER:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "INV_TEMPLATE_VER_MASTER");
			strcpy(cfg_obj_type,"/mso_cfg_inv_tmpl_ver");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "INV TEMPLATE VER MASTER CONFIG",  ebufp);
			break; 
		case ITEM_KNOCKOFF_PRIORITY_MASTER_CONF:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ITEM_KNOCKOFF_PRIORITY_MASTER_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_pymt_priority");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "ITEM KNOCKOFF PRIORITY MASTER CONFIG",  ebufp);
			break; 
		case REFUND_RULE_CONF:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "REFUND_RULE_CONF");
			strcpy(cfg_obj_type,"/mso_cfg_refund_rule");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "REFUND RULE CONFIG",  ebufp);
			break; 
		case DSA_COMM_RULE:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DSA_COMM_RULE");
			strcpy(cfg_obj_type,"/mso_dsa_comm_rule");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "DSA COMM RULE CONFIG",  ebufp);
			break; 
		case DSA_COMM_CRITERIA:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DSA_COMM_CRITERIA");
			strcpy(cfg_obj_type,"/mso_dsa_comm_criteria");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "DSA COMM CRITERIA CONFIG",  ebufp);
			break; 
		case DSA_COMM_RULE_ASSOCIATE:
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DSA_COMM_RULE_ASSOCIATE");
			strcpy(cfg_obj_type,"/mso_dsa_rule_associate");
			PIN_FLIST_FLD_SET(lc_info, PIN_FLD_OBJ_TYPE, "DSA COMM RULE ASSOCIATE CONFIG",  ebufp);
			break; 
		default :
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid action task option");
			break; 
	}
	/******************************************************************
	* Configuration Selection - End
	******************************************************************/

        /*******************************************************************
        * Action - Start
        *******************************************************************/
	cfg_poid = PIN_FLIST_FLD_GET(input_flistp, PIN_FLD_POID, 0, ebufp);
	if (strcmp(PIN_POID_GET_TYPE(cfg_poid),cfg_obj_type)!=0) {
		valid_action = VALIDATION_FALSE;	
		sprintf(error_descr,"Configuration Object Type should be %s instead of %s",cfg_obj_type, PIN_POID_GET_TYPE(cfg_poid));
	}
	else {
		/* 
		*********** COMMENTING THIS PART OF THE CODE FOR A DETAILED ANALYSIS ***
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Before add conf");
	    if ( *flag_ptr == ADD_CONF_OBJ ) {
	        parse_storable_class (ctxp, db, cfg_obj_type, &cfg_obj_spec_flistp, ebufp);
	        strcpy(msg,"");
	        if ( compare_object(ctxp,cfg_obj_spec_flistp, input_flistp, msg, ebufp) == 1) {
		    PIN_ERR_LOG_MSG (PIN_ERR_LEVEL_DEBUG, "compare_object success");
		    valid_action = VALIDATION_TRUE;
	        }
	        else {
		    valid_action = VALIDATION_FALSE;
		    sprintf(error_descr,"%sROOT is missing", msg);
		    PIN_ERR_LOG_MSG (PIN_ERR_LEVEL_DEBUG, "compare_object failed");
	        }
	    }
	    if ( (*flag_ptr == ADD_CONF_OBJ && valid_action == VALIDATION_TRUE ) || valid_action != VALIDATION_TRUE) {
		if (*flag_ptr != ADD_CONF_OBJ) 
		    parse_storable_class (ctxp, db, cfg_obj_type, &cfg_obj_spec_flistp, ebufp);
    	        strcpy(msg,"");
		err_flistp = PIN_FLIST_CREATE(ebufp);
       	        if ( navigate_input_flist(ctxp, input_flistp, cfg_obj_spec_flistp, msg, &err_flistp, ebufp) == 1) {
                    PIN_ERR_LOG_MSG (PIN_ERR_LEVEL_DEBUG, "navigate_input_flist success");
		    valid_action = VALIDATION_TRUE;
                }
      	        else {
               	    valid_action = VALIDATION_FALSE;
		    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "navigate err flistp", err_flistp);
                    sprintf(error_descr,"%s is incorrect", 
		    PIN_FLIST_FLD_GET(err_flistp, PIN_FLD_DESCR, 0, ebufp));
                    PIN_ERR_LOG_MSG (PIN_ERR_LEVEL_DEBUG, "navigate_input_flist failed");
                }
	    }
	    */
	}

	//if (valid_action == VALIDATION_TRUE) {
	if (1) {
	    if(*ops_task_ptr == BB_CLIENT_CLASS_CMTS_MASTER_CONF) {
		acct_update = handle_cmts_cc_mapping(ctxp, input_flistp, &ret_flistp, *flag_ptr, ebufp);
	    } else if((*ops_task_ptr == BB_CREDIT_LIMIIT_MANAGEMENT_CONF) && (*flag_ptr == ADD_CITIES_ENTRY)) {
	        acct_update = add_cities_in_credit_limit(ctxp, input_flistp, *flag_ptr, &ret_flistp, ebufp);
	    } else {
                acct_update = fm_mso_action(ctxp, *flag_ptr, input_flistp, &ret_flistp, ebufp);
	    }
            //acct_update = fm_mso_action(ctxp, *ops_task_ptr, input_flistp, &ret_flistp, ebufp);
	}
	else {
		PIN_ERRBUF_CLEAR(ebufp);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &action_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "54200", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
                goto CLEANUP;
	}

        if ( acct_update == VALIDATION_FALSE)
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &action_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "54201", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Master data setup for given input failed", ebufp);
                goto CLEANUP;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Action completed successfully", ret_flistp);
	//lifecycle 
	//fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_OPERATION_ADD_ACTION, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}
        /*******************************************************************
        * Action  - End
        *******************************************************************/

CLEANUP:
	PIN_FLIST_DESTROY_EX(&cfg_obj_spec_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
        *r_flistpp = ret_flistp;
        return;
}


/*******************************************************************
 * Function: fm_mso_action
 *
 * This function is used to execute the respective opcode and return 
 * the required flist back to the function callee.
 *
 * Arguments: 
 *      1. Context Pointer
 *      2. Action Flag (Refer the action macros defined above)
 *      3. Input Flist
 *      4. Output Flist
 *      5. Error Buffer
 * Return value:
 * 	Function returns an integer value 
 *		 0 indicates failure
 *		 1 indicates success
 *******************************************************************/

static int
fm_mso_action(
        pcm_context_t           *ctxp,
        int32                   action_flag,
        pin_flist_t             *in_flist,
        pin_flist_t             **out_flist,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *search_inflistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *results_flistp = NULL;
        pin_flist_t             *return_flistp = NULL;
        pin_flist_t             *result_flist = NULL;
        int                     action_success = 0;
        int                     action_failure = 1;
        char                    opcode[80];
        char                    action[80];
        char                    msg[255];
        char                    search_template[255];
        int64                   db=0;
        int32                   search_flags=256;
        int32                   *ops_task_ptr = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return 0;

        ops_task_ptr = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_OPERATION_TASK, 1, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_action input list", in_flist);
	switch (action_flag) {
		case ADD_CONF_OBJ: 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Configuration Object - Create");
        		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, in_flist, &return_flistp, ebufp);
			strcpy(opcode,"PCM_OP_CREATE_OBJ");
			strcpy(action,"ACTION - Create new object completed successfully");
		break;
		case UPDATE_CONF_OBJ: 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Configuration Object - Update");
        		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, in_flist, &return_flistp, ebufp);
			strcpy(opcode,"PCM_OP_WRITE_FLDS");
			strcpy(action,"ACTION - Updating existing object completed successfully");
		break;
		case DELETE_CONF_OBJ_FLD: 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Configuration Object - Delete Array");
			strcpy(opcode,"PCM_OP_DELETE_FLDS");
        		PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 32, in_flist, &return_flistp, ebufp);
			strcpy(action,"ACTION - Deleting existing object's array completed successfully");
		break;
		case DELETE_CONF_OBJ: 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Configuration Object - Delete");
        		PCM_OP(ctxp, PCM_OP_DELETE_OBJ, 0, in_flist, &return_flistp, ebufp);
			strcpy(opcode,"PCM_OP_DELETE_OBJ");
			strcpy(action,"ACTION - Deleting existing object completed successfully");
		break;
		case SEARCH_CONF_OBJ: 
			sprintf(search_template, "select X from %s where F1.type = V1 ",PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp)));
			db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));
			
			search_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);
			results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Configuration Object - Search");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_action search template", search_inflistp);
        		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &return_flistp, ebufp);
			strcpy(opcode,"PCM_OP_SEARCH");
			strcpy(action,"ACTION - Search object completed successfully");
		break;
		default:
			if(*ops_task_ptr == BB_CREDIT_LIMIIT_MANAGEMENT_CONF) {
			}
		break;
	}

        sprintf(msg, "fm_mso_action %s output flist", opcode);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, return_flistp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                sprintf(msg, "Error in calling %s", opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
                return 0;
        }

	if(action_flag!=SEARCH_CONF_OBJ){
            *out_flist = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_POID, *out_flist, PIN_FLD_POID, ebufp );
            PIN_FLIST_FLD_SET(*out_flist, PIN_FLD_STATUS, &action_success, ebufp);
            PIN_FLIST_FLD_SET(*out_flist, PIN_FLD_DESCR, action, ebufp);
            PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
	}
	else {
	    PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_STATUS, &action_success, ebufp);
            PIN_FLIST_FLD_SET(return_flistp, PIN_FLD_DESCR, action, ebufp);
	    *out_flist = return_flistp;
            PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	}
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_object output flist", *out_flist);

        return 1;
}


/*******************************************************************************
Function Name : 
	parse_storable_class

Function Description : 
	It parses the input class name and calls the process class definition for 
	base class and further sub classes to consolidate 
	the configuration object specification
 
Function Arguments:
	1. PCM Context Pointer
	2. Database Number 
	3. Storable Class Name 
	4. Return Flist pointer to return the consolidated configuration object specification
	5. Error buffer

Return Value:
	None
********************************************************************************/
void parse_storable_class (
	pcm_context_t	*ctxp,
	int64		database,
	char 		*class_name,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp) {

	char 		*token="";
	char 		tmp_str[256]="";
	char 		tmp_class_name[256]="";
	pin_flist_t	*cfg_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	char		msg[256]="";

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
	
	sprintf(msg, "parse_storable_class input storable class name : %s", class_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

	cfg_flistp = PIN_FLIST_CREATE (ebufp);
	token = strtok(class_name,"/");
	do {
		sprintf(tmp_str,"/%s",token);
		strcat(tmp_class_name, tmp_str);

		/*Call the opcode to get the object_spec*/
		process_class_definition (ctxp, database, tmp_class_name, &r_flistp, ebufp);
		if (r_flistp != NULL)
			PIN_FLIST_CONCAT(cfg_flistp, r_flistp, ebufp);
		PIN_FLIST_DESTROY_EX (&r_flistp, NULL);

	} while (token=strtok(NULL,"/"));
	*r_flistpp = cfg_flistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "parse_storable_class return flist", *r_flistpp);
}

/*******************************************************************************
Function Name : 
	process_class_definition

Function Description : 
	It processes the dd object's specification recursively and prepares
	conslidated input flist
 
Function Arguments:
	1. PCM Context Pointer
	2. Database Number 
	3. Storable Class Name 
	4. Return Flist pointer to return the configuration object specification
	5. Error buffer

Return Value:
	None
*******************************************************************************/
void process_class_definition ( 
	pcm_context_t	*ctxp,
	int64		database,
	char		*class_name,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp ) { 

	poid_t		*dd_obj_pdp = NULL;
	pin_flist_t	*dd_flistp = NULL;
	pin_flist_t	*dd_r_flistp = NULL;
	pin_flist_t	*flistp0 = NULL;
	pin_flist_t	*tmp_obj_spec_flistp = NULL;
	pin_flist_t	*cfg_obj_spec_flistp = NULL;
	char		msg[256]="";

	int32           elem_obj_id=0, elem_obj_fld_id=0, elem_fld_id=0;
	pin_cookie_t    cookie_obj=NULL, cookie_obj_fld=NULL, cookie_fld=NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return ;

	sprintf(msg, "process_class_definition input storable class name : %s", class_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

	dd_flistp = PIN_FLIST_CREATE(ebufp);
	dd_obj_pdp = PIN_POID_CREATE(database, "/dd/objects", -1, ebufp);
	PIN_FLIST_FLD_PUT(dd_flistp,PIN_FLD_POID, dd_obj_pdp, ebufp);
	flistp0 = PIN_FLIST_ELEM_ADD(dd_flistp, PIN_FLD_OBJ_DESC, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp0,PIN_FLD_NAME, class_name, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "process_class_definition dd_objects input flist", dd_flistp);

	if ( PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting dd_object spec process_class_definition ", ebufp);
		goto cleanup;
	}
	PCM_OP(ctxp, PCM_OP_SDK_GET_OBJ_SPECS, 0, dd_flistp, &dd_r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "process_class_definition dd_objects return flist", dd_r_flistp);

	if (dd_r_flistp != NULL) {
		cfg_obj_spec_flistp = PIN_FLIST_CREATE(ebufp);
		while ((tmp_obj_spec_flistp = 
	  		PIN_FLIST_ELEM_GET_NEXT(dd_r_flistp, PIN_FLD_OBJ_DESC, &elem_obj_id, 1, &cookie_obj, ebufp)) != NULL ) {
	  		filter_class_definition ( ctxp, tmp_obj_spec_flistp, &cfg_obj_spec_flistp, ebufp);
		}
		*r_flistpp = cfg_obj_spec_flistp;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "process_class_definition return flist", *r_flistpp);
cleanup:
	PIN_FLIST_DESTROY_EX(&dd_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&dd_r_flistp, NULL);
}

/*******************************************************************************
Function Name : 
	filter_class_definition

Function Description : 
 	Function to filter the dd object's specification by getting the below fields
	PIN_FLD_FIELD_NAME, PIN_FLD_FIELD_TYPE
Function Arguments:
	1. PCM Context Pointer
	2. Input Flist  pointer to the configuration object specification
	3. Return Flist pointer to the filtered configuration object specification
	4. Return Flist pointer to return the configuration object specification
	5. Error buffer

Return Value:
	None
*******************************************************************************/
void filter_class_definition (
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp, 
	pin_flist_t	**r_flistpp, 
	pin_errbuf_t	*ebufp) {
	
	char            	*fields_str = "PIN_FLD_CREATED_TPIN_FLD_MOD_TPIN_FLD_READ_ACCESSPIN_FLD_WRITE_ACCESSPIN_FLD_ACCOUNT_OBJ";
	char			*field_name;
	char			*fld_name;
	int32			*field_type = NULL;
	pin_flist_t		*tmp_obj_flistp=NULL;
	pin_flist_t		*tmp_r_flistp=NULL;
	pin_flist_t		*tmp_flistp=NULL;
	pin_flist_t		*tmp_flistp1=NULL;
	pin_flist_t		*dd_fld_flistp=NULL;
	pin_flist_t		*dd_r_fld_flistp=NULL;
	pin_flist_t		*tmp_fld_flistp=NULL;
	poid_t			*tmp_pdp0=NULL;

	int32           	elem_obj_id=0, elem_obj_fld_id=0, elem_fld_id=0;
	pin_cookie_t    	cookie_obj=NULL, cookie_obj_fld=NULL, cookie_fld=NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "filter_process_definition input flist", i_flistp);

	tmp_r_flistp = PIN_FLIST_CREATE(ebufp);
	elem_obj_id = 0;
	cookie_obj = NULL;
	
	while ( (tmp_obj_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_OBJ_ELEM, &elem_obj_fld_id, 1, &cookie_obj_fld, ebufp)) != NULL ) {
		field_name=(void *)PIN_FLIST_FLD_GET(tmp_obj_flistp,PIN_FLD_FIELD_NAME,0,ebufp);
		field_type=(void *)PIN_FLIST_FLD_GET(tmp_obj_flistp,PIN_FLD_FIELD_TYPE,0,ebufp);
		if ( strstr( fields_str, field_name ) == NULL ) {
			tmp_flistp = PIN_FLIST_ELEM_ADD(tmp_r_flistp, PIN_FLD_OBJ_ELEM, elem_obj_fld_id, ebufp);
			PIN_FLIST_FLD_COPY(tmp_obj_flistp, PIN_FLD_FIELD_NAME, tmp_flistp, PIN_FLD_FIELD_NAME, ebufp);
			PIN_FLIST_FLD_COPY(tmp_obj_flistp, PIN_FLD_FIELD_TYPE, tmp_flistp, PIN_FLD_FIELD_TYPE, ebufp);
		}
		if ( *field_type == PIN_FLDT_ARRAY || *field_type == PIN_FLDT_SUBSTRUCT) {
			filter_class_definition (ctxp, tmp_obj_flistp, r_flistpp, ebufp);
			tmp_flistp1 = PIN_FLIST_ELEM_ADD (tmp_r_flistp, PIN_FLD_OBJ_ELEM, elem_obj_fld_id, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp1, PIN_FLD_FIELD_NAME, field_name, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp1, PIN_FLD_FIELD_TYPE, field_type, ebufp);
			PIN_FLIST_CONCAT (tmp_flistp1, *r_flistpp, ebufp);
		}
	}
	*r_flistpp = tmp_r_flistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "filter_process_definition return flist", *r_flistpp);
}


/*******************************************************************************
Function Name : 
	compare_object

Function Description : 
	It compare the configuration object specification and the input flist
	
Function Arguments:
	1. PCM Context Pointer
	2. Input Flist pointer to the configuration object specification
	3. Input Flist pointer to the configuration object input flist
	4. Message String
	5. Error buffer

Return Value:
	0 - Failure 
	1 - Success

Algorithm:
	1. Loop through the object specification input flist 
	2. Read the Field Name and Type from the current read element
	3. if the field type is ARRAY or SUBSTRUCT then
	3.1 Loop through the object input flist to read its fields 
	3.1.1 call the function (recursive) again if the flag is not equal to 1 (EQUAL)
	3.1.2 store the return value of the function into flag after multiplying by 1. 
              [To keep track of the object comparition status whether its EQUAL or NOT EQUAL]
	3.1.3 if the flag is equal to 1 (EQUAL) then clear the error buffer
	3.1.4 else concatenate the current field into the msg variable and return OBJECT_NOT_EQUAL value
	4. else read the field from the object input flist into the generic pointer
	5. if the error is set due to unavailability of the field in the object input flist 
	5.1 then concatenate the current field into the msg variable and return OBJECT_NOT_EQUAL value
	5.2 else continue the loop (1)
	5.3 return 1(EQUAL) if there is no error 
*******************************************************************************/
int compare_object (
	pcm_context_t	*ctxp,
	pin_flist_t	*object_spec_flistp,
	pin_flist_t	*object_flistp,
	char		*msg,
	pin_errbuf_t    *ebufp) {

	pin_flist_t	*tmp_obj_flistp=NULL;
	char		*field_name_obj_spec=NULL;
	int32		*field_type_obj_spec=NULL;
	int32           elem_obj_spec_id=0;
	int32           elem_obj_id=0;
	int32           tmp_elem_id=0;
	pin_cookie_t    cookie_obj_spec=NULL;
	pin_cookie_t    cookie_obj=NULL;
	pin_cookie_t    tmp_cookie=NULL;
	void		*obj_fld=NULL;
	void		*vp=NULL;
	pin_fld_num_t	field = 0;
	pin_flist_t	*tmp_flistp = 0;
	static int	flag = OBJECT_EQUAL;
	int32		count = 0;
	static char	tmp_str[256] = "";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "compare_object input object_spec_flistp", object_spec_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "compare_object input object_flistp", object_flistp);

	while ( (tmp_obj_flistp = PIN_FLIST_ELEM_GET_NEXT(object_spec_flistp, 
		PIN_FLD_OBJ_ELEM, &elem_obj_spec_id, 1, &cookie_obj_spec, ebufp)) != NULL ) {
		field_name_obj_spec=(void *)PIN_FLIST_FLD_GET(tmp_obj_flistp,PIN_FLD_FIELD_NAME,0,ebufp);
		field_type_obj_spec=(void *)PIN_FLIST_FLD_GET(tmp_obj_flistp,PIN_FLD_FIELD_TYPE,0,ebufp);
		field = pin_field_of_name(field_name_obj_spec);
		tmp_elem_id = 0;
		tmp_cookie = NULL;

		if ( *field_type_obj_spec == PIN_FLDT_ARRAY || *field_type_obj_spec == PIN_FLDT_SUBSTRUCT) {
			tmp_elem_id = 0;
			tmp_cookie = NULL;
			while ( (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(object_flistp, field, &tmp_elem_id, 0, &tmp_cookie, ebufp)) != NULL ){
				if (flag == OBJECT_EQUAL) {
				   flag = OBJECT_EQUAL * compare_object (ctxp, tmp_obj_flistp, tmp_flistp, msg, ebufp);			
				}
			}
			if (flag == OBJECT_EQUAL) {
			   PIN_ERRBUF_CLEAR(ebufp);
			}
			else  {
			   sprintf(tmp_str, "%s->", field_name_obj_spec);
			   strcat(msg, tmp_str);
			   PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			   PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"compare_object failed");
			   return OBJECT_NOT_EQUAL;
			}
		}
		else {
			vp = PIN_FLIST_FLD_GET(object_flistp, field, 0, ebufp);
		}
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			sprintf(tmp_str, "%s->", field_name_obj_spec);
			strcat(msg, tmp_str);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"compare_object failed");
			PIN_ERRBUF_CLEAR(ebufp);
			return OBJECT_NOT_EQUAL;
		}
	}
	PIN_ERRBUF_CLEAR(ebufp);
	return OBJECT_EQUAL;
}


/*******************************************************************************
Function Name : 
	navigate_input_flist

Function Description : 
  	It to navigate through the input flist of the configuration object to
   	be created and compares against the configuration specification.
	
Function Arguments:
	1. PCM Context Pointer
	2. Input Flist pointer to the configuration object specification
	3. Input Flist pointer to the configuration object input flist
	4. Path String to track the field
	5. Error buffer

Return Value:
	0 - Failure 
	1 - Success

Algorithm:
	1. check whether the error flag is set or not. return if it is set.
	2. copy the string "MAIN" into path string variable if its length is < 1.
	   This will be executed when the function enters first time.
	3. Loop through input flist (the actual flist used for object manipulation)
	3.1 Read the field from the input flist
	3.2 if the field is of type ARRAY or SUBSTRUCT then 
	3.2.1 compare the field against the object specification 
	3.2.1.1 if field found then 
	3.2.1.1.1 copy the current path into previous_path variable string 
	3.2.1.1.2 concatenate the current field into the path string 
	3.2.1.1.3 call the function (recursive) again to process its fields
	3.2.1.1.4 copy back the previous_path variable string into path string
	3.2.1.2 else store the element into error string to indicate missing field
	3.3 if the field is not of type ARRAY or SUBSTRUCT then 
	3.3.1 continue the loop (1) if the field is equal to the below fields
	      PIN_FLD_CREATED_T
	      PIN_FLD_MOD_T
	      PIN_FLD_READ_ACCESS
	      PIN_FLD_WRITE_ACCESS
	      PIN_FLD_ACCOUNT_OBJ
	3.3.1 compare the field against the object specification 
	3.3.1.1 if field found then continue the loop (1) 
	3.3.1.2 else store the element into error string to indicate missing field
*******************************************************************************/
int navigate_input_flist (
    	pcm_context_t   *ctxp,
    	pin_flist_t     *i_flistp,
    	pin_flist_t     *obj_spec_flistp,
    	char            *path,
    	pin_flist_t	    **err_flistp,
    	pin_errbuf_t    *ebufp) {

	char            *fields_str = "PIN_FLD_CREATED_TPIN_FLD_MOD_TPIN_FLD_READ_ACCESSPIN_FLD_WRITE_ACCESSPIN_FLD_ACCOUNT_OBJ";
        pin_fld_num_t	field = 0;
        int32		rec_id = 0, elem_obj_spec_id = 0, elem_obj_spec_elm_id = 0;
        pin_cookie_t	cookie = NULL, cookie_obj_spec=NULL, cookie_obj_spec_elm=NULL;
        pin_errbuf_t	any_field_ebuf;
        char		*field_name=NULL;
        void		*vp=NULL;
        pin_flist_t	*tmp_obj_elm_flistp = NULL;
        pin_flist_t	*tmp_obj_flistp = NULL;

        static int	err_flag = OBJECT_EQUAL;
        char		prev_path [3000]="";
        char		msg [3000]="";

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return ;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERRBUF_CLEAR(&any_field_ebuf);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "navigate input object_spec_flistp", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "navigate input object_flistp", obj_spec_flistp);

	sprintf(msg, "Entering navigate : err_flag : %d", err_flag);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	
	if (err_flag == OBJECT_NOT_EQUAL) return err_flag;
	if (strlen (path) < 1 ) strcpy (path, "MAIN");

	while ((vp = PIN_FLIST_ANY_GET_NEXT(i_flistp, &field, &rec_id, &cookie, &any_field_ebuf)) != NULL && err_flag != 0) {
	    switch ( pin_type_of_field( field )) {
		case PIN_FLDT_ARRAY:
		case PIN_FLDT_SUBSTRUCT:
		    elem_obj_spec_elm_id = 0;
		    cookie_obj_spec_elm = NULL;
		    err_flag = OBJECT_NOT_EQUAL;
		    sprintf(msg, "\nField ID : %d         Field Type : %d    Field Name : %s\n", field, PIN_FIELD_GET_TYPE( field ));
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		    while ( (tmp_obj_elm_flistp = PIN_FLIST_ELEM_GET_NEXT(obj_spec_flistp, 
		    	PIN_FLD_OBJ_ELEM, &elem_obj_spec_elm_id, 1, &cookie_obj_spec_elm, ebufp)) != NULL ) {
			field_name=(void *)PIN_FLIST_FLD_GET(tmp_obj_elm_flistp,PIN_FLD_FIELD_NAME,0,ebufp);
			if (pin_field_of_name(field_name) == field) {
			    printf("Found Element : %s\n", field_name);
			    err_flag = OBJECT_EQUAL;
			    break;
			}
		    }
		    sprintf(msg, "path : %s  prev_path : %s   field : %s", 
		    path, prev_path, pin_name_of_field (field));
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		    strcpy(prev_path,path);
		    strcat(path, "->");
		    strcat(path, pin_name_of_field (field));
		    if (err_flag == OBJECT_NOT_EQUAL)  {
		        printf("Array/Substruct Field : %s is invalid ", path);
		        PIN_FLIST_FLD_SET(*err_flistp, PIN_FLD_DESCR, path, ebufp);
		        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, path);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "navigate_input_flist return fail");
		        return err_flag;
		    }
		    navigate_input_flist (ctxp, (pin_flist_t*)vp, tmp_obj_elm_flistp, path, err_flistp, ebufp);
		    strcpy(path, prev_path);
		    break;
		default:
		    elem_obj_spec_id = 0;
		    cookie_obj_spec = NULL;
		    sprintf(msg, "\nField ID : %d         Field Type : %d    field Name : %s\n", field, PIN_FIELD_GET_TYPE( field), pin_name_of_field (field) );
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		    if ( strstr (fields_str, pin_name_of_field (field))!=NULL) continue;
		    err_flag = OBJECT_NOT_EQUAL;
		    while ( (tmp_obj_flistp = PIN_FLIST_ELEM_GET_NEXT(obj_spec_flistp, PIN_FLD_OBJ_ELEM,
		        &elem_obj_spec_id, 1, &cookie_obj_spec, ebufp)) != NULL ) {
		        field_name=(void *)PIN_FLIST_FLD_GET(tmp_obj_flistp,PIN_FLD_FIELD_NAME, 0,ebufp);
		        if (pin_field_of_name(field_name) == field) {
		            printf("Found Field : %s\n", field_name);
		            err_flag = OBJECT_EQUAL;
		            break;
		        }
		    }
		    if (err_flag == OBJECT_NOT_EQUAL) {
		        strcat(path, "->");
		        strcat(path, pin_name_of_field (field));
		        printf("Field : %s is invalid ", path );
		        PIN_FLIST_FLD_SET(*err_flistp, PIN_FLD_DESCR, path, ebufp);	
		        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, path);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "navigate_input_flist return fail");
		        return err_flag;
		    }
	    }

	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "navigate_input_flist return success");
	return err_flag;
}


static int32 handle_cmts_cc_mapping(pcm_context_t   *ctxp,
				pin_flist_t *input_flistp,
                                pin_flist_t **ret_flistp,
				int32	action,
				pin_errbuf_t	*ebufp) {

    char *cmts_id = NULL;
    int64	db = 1;
    pin_flist_t *srch_in_flistp = NULL;
    pin_flist_t *srch_out_flistp = NULL;
    pin_flist_t *return_flistp = NULL;
    pin_flist_t *args_flistp = NULL;
    pin_flist_t *results_flistp = NULL;
    pin_flist_t *cc_mapping_flistp = NULL;
    pin_flist_t *new_ip_flistp = NULL;
    poid_t	*cc_mapping_pdp = NULL;
    char *cmts_template = "select X from /mso_cfg_cmts_cc_mapping where F1 = V1 ";
    int32	search_flags = 256;
    int32           elem_obj_id=0;
    pin_cookie_t    cookie_obj=NULL;
    int			action_success = 0;
    int			action_failure = 1;
    int32 	status = 1;
    char	action_str[100];

    cmts_id = PIN_FLIST_FLD_GET(input_flistp, MSO_FLD_CMTS_ID, 0, ebufp);

    srch_in_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
    PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
    PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, cmts_template, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_CMTS_ID, cmts_id, ebufp);
    results_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);

    PIN_FLIST_FLD_SET(results_flistp, MSO_FLD_CMTS_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Search input");
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_action cmts search template input flist", srch_in_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_action cmts search template output flist", srch_out_flistp);

    results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
    if(results_flistp == NULL) {
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Result flist null... must be a new one.. ");
    } else {
        cc_mapping_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
    }
    new_ip_flistp = NULL;
			
    switch(action) {
	case ADD_CONF_OBJ:
	    while(( cc_mapping_flistp =
                        PIN_FLIST_ELEM_GET_NEXT(input_flistp, MSO_FLD_CC_MAPPING, &elem_obj_id, 1, &cookie_obj, ebufp)) != NULL
		) {
		new_ip_flistp = PIN_FLIST_CREATE(ebufp);
	        if(cc_mapping_pdp == NULL) {
    		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No /mso_cfg_cmts_cc_mapping exists. creating a new one");
		    PIN_FLIST_FLD_SET(new_ip_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/mso_cfg_cmts_cc_mapping", -1, ebufp), ebufp);
		    PIN_FLIST_FLD_SET(new_ip_flistp, MSO_FLD_CMTS_ID, cmts_id, ebufp);
		    PIN_FLIST_ELEM_COPY(input_flistp, MSO_FLD_CC_MAPPING,elem_obj_id, new_ip_flistp, MSO_FLD_CC_MAPPING, 0, ebufp);
    		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"New object input is ::",new_ip_flistp);
		    PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, new_ip_flistp, &return_flistp, ebufp);
    		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"New object created is ::",return_flistp);
    		    cc_mapping_pdp = PIN_FLIST_FLD_TAKE(return_flistp, PIN_FLD_POID, 1, ebufp);
	        } else {
    		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"/mso_cfg_cmts_cc_mapping exists. updating the entry");
		    PIN_FLIST_FLD_SET(new_ip_flistp, PIN_FLD_POID, cc_mapping_pdp, ebufp);
		    PIN_FLIST_ELEM_COPY(input_flistp, MSO_FLD_CC_MAPPING,elem_obj_id, new_ip_flistp, MSO_FLD_CC_MAPPING, PIN_ELEMID_ASSIGN, ebufp);
    		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Update object input is ::",new_ip_flistp);
	    	    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, new_ip_flistp, &return_flistp, ebufp);
    	    	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Update object output is ::",return_flistp);
	        }
		PIN_FLIST_DESTROY_EX(&new_ip_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
		    status = action_failure;
		} else {
		    strcpy(action_str,"ACTION - Create new object completed successfully");
		    status = action_success;
		}
	    }
	    break;
	case UPDATE_CONF_OBJ:
	        if(cc_mapping_pdp == NULL) {
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No CC Mapping object found. Update operation can not be done");
		    status = action_failure;
		} else {
		    PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_POID, cc_mapping_pdp, ebufp);
    		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Update operation input is ::",input_flistp);
                    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, input_flistp, &return_flistp, ebufp);
                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Update object output is ::",return_flistp);
		    if (PIN_ERRBUF_IS_ERR(ebufp)) {
		        status = action_failure;
		    } else {
		        strcpy(action_str,"ACTION - Update object completed successfully");
		        status = action_success;
		    }
		}
	    break;
	case DELETE_CONF_OBJ_FLD:
	        if(cc_mapping_pdp == NULL) {
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No CC Mapping object found. Delete operation can not be done");
		    status = action_failure;
		} else {
		    PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_POID, cc_mapping_pdp, ebufp);
		    PIN_FLIST_FLD_DROP(input_flistp, MSO_FLD_CMTS_ID, ebufp);
		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Delete Fields operation input is ::",input_flistp);
                    PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 32, input_flistp, &return_flistp, ebufp);
                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Delete Fields Ouptut flist is ::",return_flistp);
		    if (PIN_ERRBUF_IS_ERR(ebufp)) {
		        status = action_failure;
		    } else {
		        strcpy(action_str,"ACTION - Delete fields completed successfully");
		        status = action_success;
		    }
		}
	    break;
	case DELETE_CONF_OBJ:
	    break;
    }	
    PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
            *ret_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status, ebufp);
            PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_DESCR, action_str, ebufp);
            PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
    return VALIDATION_TRUE;
}

static int32 handle_credit_limit(pcm_context_t   *ctxp,
                                pin_flist_t *input_flistp,
                                int32   action,
                                pin_errbuf_t    *ebufp)
{
    char *plan_name = NULL;
    int64	db = 1;
    pin_flist_t *srch_in_flistp = NULL;
    pin_flist_t *srch_out_flistp = NULL;
    pin_flist_t *return_flistp = NULL;
    pin_flist_t *args_flistp = NULL;
    pin_flist_t *results_flistp = NULL;
    pin_flist_t *cl_flistp = NULL;
    pin_flist_t *new_ip_flistp = NULL;
    poid_t	*cl_pdp = NULL;
    char *cmts_template = "select X from /mso_cfg_credit_limit where F1 = V1 ";
    int32	search_flags = 256;
    int32           elem_obj_id=0;
    pin_cookie_t    cookie_obj=NULL;
    int			action_success = 0;
    int			action_failure = 1;
    int32 	status = 1;
    char	action_str[100];

    if(action == ADD_CONF_OBJ){
 	return;
    }

    plan_name = PIN_FLIST_FLD_GET(input_flistp, MSO_FLD_PLAN_NAME, 0, ebufp);
    srch_in_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
    PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
    PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, cmts_template, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PLAN_NAME, plan_name, ebufp);
    results_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);

    PIN_FLIST_FLD_SET(results_flistp, MSO_FLD_PLAN_NAME, NULL, ebufp);
    PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Search input");
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_action cfg_credit_limit search template input flist", srch_in_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_action cfg_credit_limit search template output flist", srch_out_flistp);

    results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
    if(results_flistp == NULL) {
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Result flist null.. Doing nothing");
	    return;
    } else {
	PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, input_flistp, PIN_FLD_POID, ebufp);
    }

    if(action == DELETE_CONF_OBJ_FLD || action == DELETE_CONF_OBJ) {
	PIN_FLIST_FLD_DROP(input_flistp, MSO_FLD_PLAN_NAME, ebufp);
	PIN_FLIST_FLD_DROP(input_flistp, PIN_FLD_PLAN_OBJ, ebufp);
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_action cfg_credit_limit modified input flist", input_flistp);
}

static int32 add_cities_in_credit_limit(pcm_context_t   *ctxp,
                                pin_flist_t *input_flistp,
                                int32   action,
                                pin_flist_t **ret_flistp,
                                pin_errbuf_t    *ebufp)
{

	pin_flist_t	*cities_flistp = NULL;
	pin_flist_t	*new_ip_flistp = NULL;
	pin_flist_t	*return_flistp = NULL;
	int32           elem_obj_id=0;
    	pin_cookie_t    cookie_obj=NULL;
    	int                 action_success = 0;
    	int                 action_failure = 1;
    	int32       status = 1;
    	char        action_str[100];


        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Update object input is ::",input_flistp);
	if(action == ADD_CITIES_ENTRY) {

	while(( cities_flistp =
                        PIN_FLIST_ELEM_GET_NEXT(input_flistp, MSO_FLD_CITIES, &elem_obj_id, 1, &cookie_obj, ebufp)) != NULL) {

		    new_ip_flistp = PIN_FLIST_CREATE(ebufp);
    		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"/mso_cfg_credit_limit exists. updating the entry");
		    PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_POID, new_ip_flistp, PIN_FLD_POID, ebufp);
		    PIN_FLIST_ELEM_COPY(input_flistp, MSO_FLD_CITIES,elem_obj_id, new_ip_flistp, MSO_FLD_CITIES, PIN_ELEMID_ASSIGN, ebufp);
    		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Update object input is ::",new_ip_flistp);
	    	    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, new_ip_flistp, &return_flistp, ebufp);
    	    	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Update object output is ::",return_flistp);
		    PIN_FLIST_DESTROY_EX(&new_ip_flistp, NULL);
		    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"setting status to failure");
		        status = action_failure;
			break;
		    } else {
        		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"setting status to success");
		        strcpy(action_str,"ACTION - Update of object completed successfully");
		        status = action_success;
		    }
	    }

	}
            *ret_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(return_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status, ebufp);
            PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_DESCR, action_str, ebufp);
            PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Output flist is ::",*ret_flistp);

    return VALIDATION_TRUE;
}

/**********************************************************************
 * fm_mso_end_config() : End configuration validity with current date
 *********************************************************************/
static void
fm_mso_end_config(
        pcm_context_t   *ctxp,
        pin_flist_t     *input_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp)
{
    
    pin_flist_t *search_flistp = NULL;
    pin_flist_t *search_res_flistp = NULL;
    pin_flist_t *args_flistp = NULL;
    pin_flist_t *result_flistp = NULL;
    pin_flist_t *updsvc_inflistp = NULL;
    pin_flist_t *updsvc_outflistp = NULL;

    poid_t      *search_pdp = NULL;
    poid_t      *pdp = NULL;
    time_t      valid_t = 0;
    time_t      end_time = pin_virtual_time((time_t *)NULL);
    char        search_template[512] = "";
    int32       db = -1;
    int         search_flags = 256;

    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
    PIN_ERRBUF_CLEAR(ebufp);
 
    pdp = PIN_FLIST_FLD_GET(input_flistp, PIN_FLD_POID, 0, ebufp);
    PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_END_T, &valid_t, ebufp);
 
    db = PIN_POID_GET_DB(pdp); 
    search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    search_flistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
    PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
    sprintf(search_template, "select X from %s where F1.type = V1 and F2 = V2 and F3 = V3 ", PIN_POID_GET_TYPE(pdp));
    PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
    pdp = PIN_FLIST_FLD_GET(input_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
    if (pdp)
    {
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, pdp, ebufp);
    }
    else
    {
        pdp = PIN_FLIST_FLD_GET(input_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
        if (pdp)
        {
            PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, pdp, ebufp);
        }
        else
        {
            PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
            return;
        }
    }

    args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, &valid_t, ebufp);

    result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_END_T, (char *)NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_end_config search input list", search_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
    PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching fm_mso_end_config", ebufp);
        PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
        return;
    }

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_end_config search output list", search_res_flistp);

    result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
    if ( !result_flistp )
    {
        PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
        return;
    }
    
    pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp);
    updsvc_inflistp = PIN_FLIST_CREATE(ebufp);
    PIN_FLIST_FLD_SET(updsvc_inflistp, PIN_FLD_POID, pdp, ebufp);
    PIN_FLIST_FLD_SET(updsvc_inflistp, PIN_FLD_END_T, &end_time, ebufp);

    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, updsvc_inflistp, &updsvc_outflistp, ebufp);

    PIN_FLIST_DESTROY_EX(&updsvc_inflistp, NULL);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
        PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
        return;
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_end_config config update output list", updsvc_outflistp);
    PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
    return;
}
