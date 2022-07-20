/*******************************************************************
 * File:	fm_mso_cust_manage_plans_activation.c
 * Opcode:	MSO_OP_CUST_MANAGE_PLANS_ACTIVATION 
 * Owner:	Pawan	
 * Created:	21-JULY-2014
 * Description: Manages (Add, deletes, updates) plans before  
 *				the plans are purchased for account.
 *
 * Modification History:
 * Modified By: Leela Pratyush
 * Date:
 * Modification details :  Updated plan descr for base plan 
 *
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_manage_plans_activation.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "pin_pymt.h"
#include "pin_inv.h"
#include "mso_ops_flds.h"
#include "mso_prov.h"
#include "mso_ntf.h"
#include "mso_cust.h"
#include "mso_lifecycle_id.h"

/**************************************
* External Functions start
***************************************/
extern int32
fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

extern void  
fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_bb_get_payinfo_inv(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

extern void
fm_cust_chng_plan_validate_hierarchy(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        //int                     *in_indicatorp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

/* Local Functions */

static void
fm_mso_cust_create_plan_activation(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);	

static void
fm_mso_cust_update_plan_activation(
	pcm_context_t		*ctxp,
	pin_flist_t		*mso_plan_flistp,
	pin_flist_t		*i_flistp,
	double			priority,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_cust_add_products_array(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	double			*priority,
	pin_errbuf_t		*ebufp);

static int
fm_mso_update_bill_when(
	pcm_context_t		*ctxp,
	poid_t			*acc_obj,
	poid_t			*ser_obj,
	int			bill_when,
	double			priority,
	char			*prog_name,
	pin_errbuf_t		*ebufp);

static int
fm_mso_update_bb_payinfo_indi(
	pcm_context_t		*ctxp,
	poid_t			*acct_obj,
	int			indicator,
	char			*prog_name,
	pin_errbuf_t		*ebufp);

/***************************  Input flist  ********************************************
0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 325529 0
0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 555555 0
0 PIN_FLD_STATUS		ENUM [0] 0
0 MSO_FLD_SUBSCRIPTION_PLANS SUBSTRUCT [0] allocated 20, used 3
1     PIN_FLD_PLAN          ARRAY [0] allocated 20, used 4
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 191188 2
2         MSO_FLD_PLAN_NAME            STR [0] "STB_Zero_Value"
2		  PIN_FLD_ACTION			   STR [0] "ADD"
2		  PIN_FLD_PRODUCTS          	ARRAY [0] allocated 20, used 4
3         	PIN_FLD_PRODUCT_OBJ       	POID [0] 0.0.0.1 /product 191188 2
3         	PIN_FLD_PURCHASE_END_T 		TSTAMP [0] (1398796200) Wed Apr 30 00:00:00 2014
3         	PIN_FLD_PURCHASE_START_T 	TSTAMP [0] (1394562600) Wed Mar 12 00:00:00 2014
3			MSO_FLD_DISC_AMOUNT			DECIMAL [0] NULL
3			MSO_FLD_DISC_PERCENT		DECIMAL [0] NULL
1     PIN_FLD_PLAN          ARRAY [0] allocated 20, used 4
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 191188 2
2         MSO_FLD_PLAN_NAME            STR [0] "STB_Zero_Value"
2		  PIN_FLD_ACTION			   STR [0] "ADD"
2		  PIN_FLD_PRODUCTS          	ARRAY [0] allocated 20, used 4
3         	PIN_FLD_PRODUCT_OBJ       	POID [0] 0.0.0.1 /product 191188 2
3         	PIN_FLD_PURCHASE_END_T 		TSTAMP [0] (1398796200) Wed Apr 30 00:00:00 2014
3         	PIN_FLD_PURCHASE_START_T 	TSTAMP [0] (1394562600) Wed Mar 12 00:00:00 2014
3			MSO_FLD_DISC_AMOUNT			DECIMAL [0] NULL
3			MSO_FLD_DISC_PERCENT		DECIMAL [0] NULL
0 MSO_FLD_INSTALLATION_PLANS SUBSTRUCT [0] allocated 20, used 3
1     PIN_FLD_PLAN          ARRAY [0] allocated 20, used 4
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 191188 2
2         MSO_FLD_PLAN_NAME            STR [0] "STB_Zero_Value"
2		  PIN_FLD_ACTION			   STR [0] "ADD"
2		  PIN_FLD_PRODUCTS          	ARRAY [0] allocated 20, used 4
3         	PIN_FLD_PRODUCT_OBJ       	POID [0] 0.0.0.1 /product 191188 2
3         	PIN_FLD_PURCHASE_END_T 		TSTAMP [0] (1398796200) Wed Apr 30 00:00:00 2014
3         	PIN_FLD_PURCHASE_START_T 	TSTAMP [0] (1394562600) Wed Mar 12 00:00:00 2014
3			MSO_FLD_DISC_AMOUNT			DECIMAL [0] NULL
3			MSO_FLD_DISC_PERCENT		DECIMAL [0] NULL
0 MSO_FLD_HARDWARE_PLANS SUBSTRUCT [0] allocated 20, used 3
1     PIN_FLD_PLAN          ARRAY [0] allocated 20, used 4
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 191188 2
2         MSO_FLD_PLAN_NAME            STR [0] "STB_Zero_Value"
2		  PIN_FLD_ACTION			   STR [0] "ADD"
2		  PIN_FLD_PRODUCTS          	ARRAY [0] allocated 20, used 4
3         	PIN_FLD_PRODUCT_OBJ       	POID [0] 0.0.0.1 /product 191188 2
3         	PIN_FLD_PURCHASE_END_T 		TSTAMP [0] (1398796200) Wed Apr 30 00:00:00 2014
3         	PIN_FLD_PURCHASE_START_T 	TSTAMP [0] (1394562600) Wed Mar 12 00:00:00 2014
3			MSO_FLD_DISC_AMOUNT			DECIMAL [0] NULL
3			MSO_FLD_DISC_PERCENT		DECIMAL [0] NULL
2     	  PIN_FLD_DEVICES       ARRAY [0] allocated 20, used 1
3         	PIN_FLD_DEVICE_ID          STR [0] "0000060000000141"
*******************************************************************************************/ 
 
 
EXPORT_OP void
op_mso_cust_manage_plans_activation(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_manage_plans_activation(
	cm_nap_connection_t	*connp,
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);
	
extern int32
fm_mso_utils_discount_validation(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_CUST_MANAGE_PLANS_ACTIVATION  
 *******************************************************************/
void 
op_mso_cust_manage_plans_activation(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int32			local = LOCAL_TRANS_OPEN_SUCCESS;
	int32			status_uncommitted =2;
	int32			*ret_status = NULL;
	poid_t			*inp_pdp = NULL;
	char			*prog_name = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_MANAGE_PLANS_ACTIVATION) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_manage_plans_activation error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_manage_plans_activation input flist", i_flistp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	/* Open Transaction */
/*	if (!(prog_name && strstr(prog_name,"pin_deferred_act")) &&
	     (!(prog_name && strstr(prog_name,"create_service")) ))
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
	}	*/	
	fm_mso_cust_manage_plans_activation(connp, ctxp, flags, i_flistp, r_flistpp, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	ret_status = (int32*)PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
/*	if (PIN_ERRBUF_IS_ERR(ebufp) || (ret_status && *(int*)ret_status == 1)) 
	{		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_topup_bb_plan error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS && 
		   !(prog_name && strstr(prog_name,"pin_deferred_act")) &&
	     (!(prog_name && strstr(prog_name,"create_service") ))
       		  )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}
	}*/
	
	if (PIN_ERRBUF_IS_ERR(ebufp) ) 
	{	
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_manage_plans_activation error", ebufp);
	}
	else if (ret_status && *ret_status == 0)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_manage_plans_activation success output flist", *r_flistpp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_manage_plans_activation failed: output flist", *r_flistpp);
	}
	return;
}


static void 
fm_mso_cust_manage_plans_activation(
	cm_nap_connection_t	*connp,
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	poid_t			*acc_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	char			*template = "select X from /mso_plans_activation where F1.id = V1 and F2.id = V2 ";
	int64			db=0;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*ser_flistp = NULL;

	int32			create_service_success = 0;
	int32			create_service_failure = 1;
	int32			count = 0;
	int32			*status = NULL;
	int32			srch_flag = 0;
	double			priority = 0.0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_manage_plans_activation input flist", i_flistp);

	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if (!acc_pdp)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Account Poid", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51701", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account Poid is missing in input.", ebufp);
		goto CLEANUP;
	}

	if (!srvc_pdp)
	{
		ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
		if(ser_flistp)
		{
				srvc_pdp = PIN_FLIST_FLD_GET(ser_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
				PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_SERVICE_OBJ, i_flistp, PIN_FLD_SERVICE_OBJ,ebufp);
		}
		if (!srvc_pdp)
		{		
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Service Poid", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51709", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service Poid is missing in input.", ebufp);
			goto CLEANUP;
		}
		else
		{
			
		}
	}
	/* Add products array in input flist */
	fm_mso_cust_add_products_array(ctxp, i_flistp, &priority, ebufp );
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while searching products for plans or wrong discount amount", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51713", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error while searching products for plans or wrong discount amount", ebufp);
		goto CLEANUP;
	}
	
	db = PIN_POID_GET_DB(acc_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Activation search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Activation search output flist", srch_out_flistp);

	count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
	if(count == 1)
	{
		result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
		status = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_STATUS, 1, ebufp);
		if (status && *((int32 *)status)!= 0)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Plans is already active. Try Using Change Plan.", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51710", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Plans is already active. Try Using Change Plan.", ebufp);
			goto CLEANUP;
		}
	}
	else if(count > 1)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"More than one instance of Plan activation.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51711", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "More than one instance of Plan activation.", ebufp);
		goto CLEANUP;
	}
	
	
	
	if(count == 0)
	{
		/* *****************************************************
		* Object of type /mso_plans_activation DOES NOT EXIST 
		*	for this account and service. Create a new one.
		*********************************************************/
		fm_mso_cust_create_plan_activation(ctxp, i_flistp, &ret_flistp, ebufp );
		if (ret_flistp)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_plan_activation return flist", ret_flistp);
			goto CLEANUP;
		} 
	}
	else if(count == 1)
	{
		/* *****************************************************
		* Object of type /mso_plans_activation ALREADY EXISTS 
		* for this account. Update it for each plan action.
		*********************************************************/
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"--> Object Exists for this account.Modifying it.");
		result_flist=PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
		fm_mso_cust_update_plan_activation(ctxp, result_flist,i_flistp, priority, &ret_flistp, ebufp );
		if(!fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_ALLOCATE_HARDWARE) && 
				!fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_CREATE_BB_SERVICE)){
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_MODIFY_ACTIVATION_PLAN_LIST, ebufp);
		}
		if(ret_flistp)
		{
			goto CLEANUP;
		}
	}

	//lifecycle
	//fm_mso_create_lifecycle_evt(ctxp, i_flistp, NULL, ID_MODIFY_ACTIVATION_PLAN_LIST, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}
	
	//Populate Return Flist for success Case
	/*******************************************************************
	* Create Output flist - End   
	*******************************************************************/
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_success, ebufp);
	if(count == 0) {
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "Plans activation object created Successfully", ebufp);
	} else {
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "Plan updated successfully", ebufp);
	}
	CLEANUP:
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Final ret_flistp",ret_flistp);
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL); 
	return;
}



/**********************************************************************************
*  Function: fm_mso_cust_create_plan_activation
*  Description: Creates an object of type /mso_plan_activation
*				to store all the plan details to be purchased
*				after service creation
***********************************************************************************/
static void
fm_mso_cust_create_plan_activation(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	int64			db = -1;
	pin_flist_t		*obj_in_flistp = NULL;
	pin_flist_t		*obj_out_flistp = NULL;
	pin_flist_t		*plan_list_flistp  = NULL;
	pin_flist_t		*plan_flistp  = NULL;
	pin_flist_t		*device_flistp  = NULL;
	
	poid_t			*acnt_pdp = NULL;
	poid_t			*mso_act_pdp = NULL;
	int32			activate_customer_failure = 1;
	int			elem_id = 0;
	int			d_elem_id = 0;
	int			elem_ctr = 0;
	int			status_new = 0;
	int			type = 0;
	char			*action = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		d_cookie = NULL;
	int32			base_plan = 0;
	char			*descr = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_cust_create_plan_activation", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_plan_activation input flist", i_flistp);

	
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acnt_pdp);
	obj_in_flistp = PIN_FLIST_CREATE(ebufp);
	mso_act_pdp = PIN_POID_CREATE(db, "/mso_plans_activation", (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(obj_in_flistp, PIN_FLD_POID, mso_act_pdp,ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_ACCOUNT_OBJ, obj_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ, obj_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_STATUS, (void *)status_new, ebufp);
	
	plan_list_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_SUBSCRIPTION_PLANS, 1, ebufp);
	if(plan_list_flistp)
	{
		while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_list_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Subscription Plan flist is :", plan_flistp );
			action = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_ACTION, 1, ebufp);
			if(action && strcmp(action,"ADD") == 0)
			{
				type = 1;
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_DESCR, MSO_BB_BP, ebufp);
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TYPE, &type, ebufp);
			}

			PIN_FLIST_FLD_DROP(plan_flistp, PIN_FLD_ACTION, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Plan flist after modifcation is :",plan_flistp); 
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Object flist before modif is :",obj_in_flistp); 
			PIN_FLIST_ELEM_SET(obj_in_flistp, plan_flistp, PIN_FLD_PLAN, elem_ctr, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Object flist is :",obj_in_flistp); 
			elem_ctr++;
		}
	}
	elem_id=0;
	cookie=NULL;
	plan_list_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_HARDWARE_PLANS, 1, ebufp);
	if(plan_list_flistp)
	{
		while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_list_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_FLIST_FLD_DROP(plan_flistp, PIN_FLD_ACTION, ebufp);
			type=2;
			PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TYPE, &type, ebufp);	
			d_elem_id=0;
			d_cookie=NULL;
			while((device_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_DEVICES,
				&d_elem_id, 1, &d_cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_FLIST_FLD_DROP(device_flistp, PIN_FLD_TYPE, ebufp);
			}
			PIN_FLIST_ELEM_SET(obj_in_flistp, plan_flistp, PIN_FLD_PLAN, elem_ctr, ebufp);
			elem_ctr++;
		}
	}
	elem_id=0;
	cookie=NULL;
	plan_list_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_INSTALLATION_PLANS, 1, ebufp);
	if(plan_list_flistp)
	{
		while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_list_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_FLIST_FLD_DROP(plan_flistp, PIN_FLD_ACTION, ebufp);
			type=3;
			PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TYPE, &type, ebufp);
			PIN_FLIST_ELEM_SET(obj_in_flistp, plan_flistp, PIN_FLD_PLAN, elem_ctr, ebufp);
			elem_ctr++;
		}
	}

	/**added condition to throw an error when two base_plan's are added in the flist***/
	cookie = NULL;
	while(( plan_list_flistp = PIN_FLIST_ELEM_GET_NEXT(obj_in_flistp, PIN_FLD_PLAN,
                        &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {

		descr = PIN_FLIST_FLD_GET(plan_list_flistp, PIN_FLD_DESCR, 1, ebufp);	
		if(descr && strcmp(descr,"base plan") ==0)
		{
			if(base_plan == 1)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Two base plan's are in flist ", ebufp);
                		PIN_ERRBUF_RESET(ebufp);
                		*ret_flistp = PIN_FLIST_CREATE(ebufp);
                		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
                		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
                		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51705", ebufp);
                		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Two base plan's are there in Create Object", ebufp);
                		goto CLEANUP;
			}
			else
			{
				base_plan=1;
			}
		}
        }	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ: mso_account_plan_activation input flist", obj_in_flistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, obj_in_flistp, &obj_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ output flist", obj_out_flistp);
	PIN_FLIST_DESTROY_EX(&obj_in_flistp, NULL);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Can't create /mso_plans_activation object ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51705", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Can't create /mso_plans_activation object", ebufp);
		goto CLEANUP;
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&obj_out_flistp, NULL);
	return;
}	

/**********************************************************************************
*  Function: fm_mso_cust_update_plan_activation
*  Description: Updates /mso_plan_activation object for that account
*				to add/update/delete plans as per action specified 
*				in input flist.
***********************************************************************************/
static void
fm_mso_cust_update_plan_activation(
	pcm_context_t		*ctxp,
	pin_flist_t		*mso_plan_flistp,
	pin_flist_t		*i_flistp,
	double			priority,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	int64			db = -1;
	pin_flist_t		*obj_in_flistp = NULL;
	pin_flist_t		*obj_out_flistp = NULL;
	pin_flist_t		*plan_list_flistp  = NULL;
	pin_flist_t		*plan_flistp  = NULL;
	pin_flist_t		*tmp_plan_flistp  = NULL;
	pin_flist_t		*device_flistp  = NULL;
	pin_flist_t		*del_in_flistp  = NULL;
	pin_flist_t		*del_out_flistp  = NULL;
	
	poid_t			*acc_pdp = NULL;
	poid_t			*svc_pdp = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*m_plan_obj = NULL;
	int32			activate_customer_failure = 1;
	int				elem_id = 0;
	int				d_elem_id = 0;
	int				m_elem_id = 0;
	int				type = 0;
	int				res_val = -1;
	int				is_sub = 0;
	int32			*bill_when = NULL;
	char			*action = NULL;
	char			*prog_name = NULL;
	void			*vp = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		d_cookie = NULL;
	pin_cookie_t		m_cookie = NULL;
	char			*descr = NULL;
	int32			base_plan = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_cust_update_plan_activation", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_update_plan_activation input flist", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_update_plan_activation MSO plans flist", mso_plan_flistp);

	acc_pdp = PIN_FLIST_FLD_GET(mso_plan_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	svc_pdp = PIN_FLIST_FLD_GET(mso_plan_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	obj_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(mso_plan_flistp,PIN_FLD_POID, obj_in_flistp, PIN_FLD_POID, ebufp);
	del_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(mso_plan_flistp,PIN_FLD_POID, del_in_flistp, PIN_FLD_POID, ebufp);
	
	plan_list_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_SUBSCRIPTION_PLANS, 1, ebufp);
	if( plan_list_flistp && PIN_FLIST_COUNT(plan_list_flistp, ebufp)>0)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Subscription Plan List flist is :", plan_list_flistp );
		while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_list_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Subscription Plan flist is :", plan_flistp );
			action = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_ACTION, 1, ebufp);
			if(action && strcmp(action,"ADD") == 0)
			{
				//PIN_FLIST_FLD_DROP(plan_flistp, PIN_FLD_ACTION, ebufp);
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_DESCR, MSO_BB_BP, ebufp);
				type=1;
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TYPE, &type, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Plan flist after modifcation is :",plan_list_flistp); 
				PIN_FLIST_ELEM_SET(obj_in_flistp, plan_flistp, PIN_FLD_PLAN, PIN_ELEMID_ASSIGN, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Object flist is :",obj_in_flistp); 
			}
			if(action && strcmp(action,"DELETE") == 0)
			{
				plan_obj = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
				m_cookie = NULL;
				m_elem_id = 0;
				while((tmp_plan_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_plan_flistp, PIN_FLD_PLAN,
						&m_elem_id, 1, &m_cookie, ebufp)) != (pin_flist_t *)NULL)
				{
					m_plan_obj = PIN_FLIST_FLD_GET(tmp_plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Compare poid:");
					PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Delete Plan obj", plan_obj);
					PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "MSO Plan obj", m_plan_obj);
					if(PIN_POID_COMPARE(plan_obj, m_plan_obj, 0, ebufp ) == 0)
					{
						//If poid matches then add this elem id in delete fields flist.
						//Only elem id is sufficient to delete.
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matched: Setting elem id.");
						PIN_FLIST_ELEM_SET(del_in_flistp, NULL, PIN_FLD_PLAN, m_elem_id, ebufp );
					}
				}
			}
		}
		is_sub = 1;
	}
	elem_id=0;
	cookie=NULL;
	plan_list_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_HARDWARE_PLANS, 1, ebufp);
	if(plan_list_flistp && PIN_FLIST_COUNT(plan_list_flistp, ebufp)>0)
	{
		while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_list_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			action = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_ACTION, 1, ebufp);
			if(action && strcmp(action,"ADD") == 0)
			{
				//PIN_FLIST_FLD_DROP(plan_flistp, PIN_FLD_ACTION, ebufp);
				type=2;
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TYPE, &type, ebufp);
				d_elem_id=0;
				d_cookie=NULL;
				while((device_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_DEVICES,
					&d_elem_id, 1, &d_cookie, ebufp)) != (pin_flist_t *)NULL)
				{
					vp = PIN_FLIST_FLD_GET(device_flistp,PIN_FLD_TYPE, 1, ebufp);
					if(vp)
						PIN_FLIST_FLD_DROP(device_flistp, PIN_FLD_TYPE, ebufp);
				}
				PIN_FLIST_ELEM_SET(obj_in_flistp, plan_flistp, PIN_FLD_PLAN, PIN_ELEMID_ASSIGN, ebufp);
			}
			if(action && strcmp(action,"DELETE") == 0)
			{
				plan_obj = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
				m_cookie = NULL;
				m_elem_id = 0;
				while((tmp_plan_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_plan_flistp, PIN_FLD_PLAN,
						&m_elem_id, 1, &m_cookie, ebufp)) != (pin_flist_t *)NULL)
				{
					m_plan_obj = PIN_FLIST_FLD_GET(tmp_plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
					if(PIN_POID_COMPARE(plan_obj, m_plan_obj, 0, ebufp ) == 0)
					{
						//If poid matches then add this elem id in delete fields flist.
						//Only elem id is sufficient to delete.
						PIN_FLIST_ELEM_SET(del_in_flistp, NULL, PIN_FLD_PLAN, m_elem_id, ebufp );
					}
				}
			}
		}
	}
	elem_id=0;
	cookie=NULL;
	plan_list_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_INSTALLATION_PLANS, 1, ebufp);
		if(plan_list_flistp && PIN_FLIST_COUNT(plan_list_flistp, ebufp)>0)
	{
		while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_list_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			action = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_ACTION, 1, ebufp);
			if(action && strcmp(action,"ADD") == 0)
			{
				//PIN_FLIST_FLD_DROP(plan_flistp, PIN_FLD_ACTION, ebufp);
				type=3;
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TYPE, &type, ebufp);
				PIN_FLIST_ELEM_SET(obj_in_flistp, plan_flistp, PIN_FLD_PLAN, PIN_ELEMID_ASSIGN, ebufp);
			}
			if(action && strcmp(action,"DELETE") == 0)
			{
				plan_obj = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
				m_cookie = NULL;
				m_elem_id = 0;
				while((tmp_plan_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_plan_flistp, PIN_FLD_PLAN,
						&m_elem_id, 1, &m_cookie, ebufp)) != (pin_flist_t *)NULL)
				{
					m_plan_obj = PIN_FLIST_FLD_GET(tmp_plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
					if(PIN_POID_COMPARE(plan_obj, m_plan_obj, 0, ebufp ) == 0)
					{
						//If poid matches then add this elem id in delete fields flist.
						//Only elem id is sufficient to delete.
						PIN_FLIST_ELEM_SET(del_in_flistp, NULL, PIN_FLD_PLAN, m_elem_id, ebufp );
					}
				}
			}
		}
	}
	
	m_cookie = NULL;
        while(( tmp_plan_flistp = PIN_FLIST_ELEM_GET_NEXT(obj_in_flistp, PIN_FLD_PLAN,
                        &m_elem_id, 1, &m_cookie, ebufp)) != (pin_flist_t *)NULL)
        {
		action = PIN_FLIST_FLD_GET(tmp_plan_flistp, PIN_FLD_ACTION, 1, ebufp);
		if(action && strcmp(action,"ADD") == 0){
			PIN_FLIST_FLD_DROP(tmp_plan_flistp, PIN_FLD_ACTION, ebufp);
		}
                descr = PIN_FLIST_FLD_GET(tmp_plan_flistp, PIN_FLD_DESCR, 1, ebufp);
                if(descr && strcmp(descr,"base plan") ==0)
                {
                        if(base_plan == 1)
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Two base plan's are in flist ", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
                                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
                                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
                                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51712", ebufp);
                                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Two base plan's are there in Write flds Object", ebufp);
                                goto CLEANUP;
                        }
                        else
                        {
                                base_plan=1;
                        }
                }
        }
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS input flist", obj_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_ADD_ENTRY, obj_in_flistp, &obj_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS output flist", obj_out_flistp);
	PIN_FLIST_DESTROY_EX(&obj_in_flistp, NULL);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Can't update /mso_plans_activation object ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51712", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Can't update /mso_plans_activation object", ebufp);
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_DELETE_FLDS input flist", del_in_flistp);
	PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, del_in_flistp, &del_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_DELETE_FLDS output flist", del_out_flistp);
	PIN_FLIST_DESTROY_EX(&del_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Can't delete entry from /mso_plans_activation object ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51713", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Can't delete entry from /mso_plans_activation object", ebufp);
		goto CLEANUP;
	}

	/* Update the BILL_WHEN in MSO_FLD_BB_INFO */
	bill_when = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_WHEN, 1, ebufp);
	if(bill_when && is_sub)
	{
		res_val = fm_mso_update_bill_when(ctxp, acc_pdp, svc_pdp, *bill_when, priority, prog_name,  ebufp);
		if(res_val == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in Updating Service->Bill When.");
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51714", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Updating Service->Bill When.", ebufp);
			goto CLEANUP;		
		}
		/*** Pavan Bellala 12-08-2015 *************
		Added new return value for validation error
		*******************************************/ 
		else if(res_val == 2 ){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Validation error: Not allowed to change plan type");
			PIN_ERRBUF_RESET(ebufp);
                        *ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51715", ebufp);
                        PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Validation error: Not allowed to change plan type",ebufp);
                        goto CLEANUP;

		}
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&obj_out_flistp, NULL);
	return;
}

/**********************************************************************************
*  Function: fm_mso_cust_add_products_array
*  Description: Adds the products array under plans array since products array
*				is not sent by OAP (as per latest changes) and 
***********************************************************************************/
static void
fm_mso_cust_add_products_array(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	double			*priority,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*subs_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*ins_flistp = NULL;
	pin_flist_t		*hw_flistp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*acc_pdp = NULL;	
	int64			db = -1;
	int32			srch_flag = 256;
	char			*template = "select X from /product 1, /deal 2, /plan 3 where 3.F1 = V1 and 2.F2 = 3.F3 and 2.F4 = 1.F5 ";
	char			*prov_tag = NULL;
	char			*action = NULL;
	int			elem_id = 0;
	int			p_elem_id = 0;
	void		*vp = NULL;
	pin_decimal_t		*prod_pri = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		p_cookie = NULL;
	int32			ret_vall = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_cust_add_products_array", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_products_array input flist", i_flistp);

	subs_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_SUBSCRIPTION_PLANS, 1, ebufp);
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	
	db = PIN_POID_GET_DB(acc_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
	ser_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 4, ebufp );
	ser_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_PRODUCTS, 0, ebufp );
	PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 5, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);
	
	result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
	if(subs_flistp && PIN_FLIST_COUNT(subs_flistp, ebufp)>0) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Adding Subs plan");
		p_cookie = NULL;
		while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(subs_flistp, PIN_FLD_PLAN,
			&p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			//Not required to populate product array for Delete
			action = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_ACTION, 1, ebufp);
			if(action && strcmp(action,"DELETE") == 0)
				continue;
			//plan_flistp = PIN_FLIST_ELEM_GET(subs_flistp, PIN_FLD_PLAN, 0, 1, ebufp);
			arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_PLAN_OBJ, arg_flistp, PIN_FLD_POID, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Subs. Product Search input", srch_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
			//PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Subs. Product Search output", srch_out_flistp);
			result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp );
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No Subs products found for this plan", ebufp);
				goto CLEANUP;
			}
			cookie = NULL;
			while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_END_T, 1, ebufp);
				if(vp) {
					PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_PURCHASE_END_T, result_flistp, PIN_FLD_PURCHASE_END_T, ebufp);
				} else {
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
				}
				PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_PURCHASE_START_T, result_flistp, PIN_FLD_PURCHASE_START_T, ebufp);
				prov_tag = NULL;
				prov_tag = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
				if(prov_tag && strlen(prov_tag)>1) {
					/*If product has prov tag then it means it is a subs product. Discount should be 
						applied only on subscription product and not on grant product	*/
					PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_ACCOUNT_OBJ, plan_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                                        PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ, plan_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
                        		ret_vall = fm_mso_utils_discount_validation(ctxp, plan_flistp, ebufp);
					PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_ACCOUNT_OBJ, ebufp);
                                        PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_SERVICE_OBJ, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_utils_discount_validation", ebufp);
						goto CLEANUP;
					}
					else if(ret_vall == 1)
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in dicount validation where either discount amount or percentage is wrong", ebufp);
						pin_set_err(ebufp, PIN_ERRLOC_FM,
                                 		PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                 		MSO_FLD_DISC_PERCENT, 0, 0, 0);
						goto CLEANUP;	
					}
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting discount for Subs product");
					vp  = PIN_FLIST_FLD_TAKE(plan_flistp, MSO_FLD_DISC_AMOUNT, 1, ebufp);
					if(vp && !pbo_decimal_is_null(vp, ebufp))
						PIN_FLIST_FLD_PUT(result_flistp, MSO_FLD_DISC_AMOUNT, vp, ebufp);
					vp  = PIN_FLIST_FLD_TAKE(plan_flistp, MSO_FLD_DISC_PERCENT, 1, ebufp);
					if(vp && !pbo_decimal_is_null(vp, ebufp))
						PIN_FLIST_FLD_PUT(result_flistp, MSO_FLD_DISC_PERCENT,vp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Product flist.", result_flistp);
					prod_pri = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRIORITY, 1, ebufp);
					if(prod_pri) {
						*priority = pbo_decimal_to_double(prod_pri, ebufp);
					}
				}
				PIN_FLIST_FLD_RENAME(result_flistp, PIN_FLD_POID, PIN_FLD_PRODUCT_OBJ, ebufp);
				PIN_FLIST_FLD_DROP(result_flistp,PIN_FLD_PROVISIONING_TAG, ebufp);
				PIN_FLIST_FLD_DROP(result_flistp,PIN_FLD_PRIORITY, ebufp);
				PIN_FLIST_ELEM_SET(plan_flistp, result_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
				
			}
			vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_END_T, 1, ebufp);
			if(vp)
				PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_PURCHASE_END_T, ebufp);
				
			vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_START_T, 1, ebufp);
			if(vp)
				PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_PURCHASE_START_T, ebufp);
				
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Flist after Subscription plan.", i_flistp);
		}
	}
	
	result_flistp = PIN_FLIST_ELEM_GET(srch_in_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	PIN_FLIST_FLD_DROP(result_flistp,PIN_FLD_PROVISIONING_TAG, ebufp);
	PIN_FLIST_FLD_DROP(result_flistp,PIN_FLD_PRIORITY, ebufp);
	/* Execute the above search for installation plan also.. 
		Change the plan obj in existing search input flist and call search. */ 
	ins_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_INSTALLATION_PLANS, 1, ebufp);
	if(ins_flistp && PIN_FLIST_COUNT(ins_flistp, ebufp)>0)
	{
		/* change the plan obj in existing search input flist */
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Adding INS plan");
		p_elem_id = 0;
		p_cookie = NULL;
		while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(ins_flistp, PIN_FLD_PLAN,
			&p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			action = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_ACTION, 1, ebufp);
			if(action && strcmp(action,"DELETE") == 0)
				continue;
			arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_PLAN_OBJ, arg_flistp, PIN_FLD_POID, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Ins. Product Search input", srch_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
			//PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Ins. Product Search input", srch_out_flistp);
			result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp );
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No Ins. product found for this plan", ebufp);
				goto CLEANUP;
			}
			
			elem_id=0;
			cookie=NULL;	
			while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_END_T, 1, ebufp);
				if(vp) {
					PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_PURCHASE_END_T, result_flistp, PIN_FLD_PURCHASE_END_T, ebufp);
				} else {
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
				}
				PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_ACCOUNT_OBJ, plan_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ, plan_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				ret_vall = fm_mso_utils_discount_validation(ctxp, plan_flistp, ebufp);
				PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_ACCOUNT_OBJ, ebufp);
				PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_SERVICE_OBJ, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_utils_discount_validation", ebufp);
					goto CLEANUP;
				}
				else if(ret_vall == 1)
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in dicount validation where either discount amount or percentage is wrong", ebufp);
					 pin_set_err(ebufp, PIN_ERRLOC_FM,
					 PIN_ERRCLASS_SYSTEM_DETERMINATE,
					 MSO_FLD_DISC_PERCENT, 0, 0, 0);
					goto CLEANUP;
				}
				PIN_FLIST_FLD_RENAME(result_flistp, PIN_FLD_POID, PIN_FLD_PRODUCT_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_PURCHASE_START_T, result_flistp, PIN_FLD_PURCHASE_START_T, ebufp);
				PIN_FLIST_FLD_COPY(plan_flistp,MSO_FLD_DISC_AMOUNT, result_flistp, MSO_FLD_DISC_AMOUNT, ebufp);
				PIN_FLIST_FLD_COPY(plan_flistp,MSO_FLD_DISC_PERCENT, result_flistp, MSO_FLD_DISC_PERCENT, ebufp);
				PIN_FLIST_ELEM_SET(plan_flistp, result_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
			}
			vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_END_T, 1, ebufp);
			if(vp)
				PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_PURCHASE_END_T, ebufp);
				
			vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_START_T, 1, ebufp);
			if(vp)
				PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_PURCHASE_START_T, ebufp);

				vp = PIN_FLIST_FLD_GET(plan_flistp,MSO_FLD_DISC_AMOUNT, 1, ebufp);
			if(vp)
				PIN_FLIST_FLD_DROP(plan_flistp,MSO_FLD_DISC_AMOUNT, ebufp);
				
			vp = PIN_FLIST_FLD_GET(plan_flistp,MSO_FLD_DISC_PERCENT, 1, ebufp);
			if(vp)
				PIN_FLIST_FLD_DROP(plan_flistp,MSO_FLD_DISC_PERCENT, ebufp);
		}
	}
	
	/* Execute the above search for Hardware plan also.. 
		Change the plan obj in existing search input flist and call search. */ 
	hw_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_HARDWARE_PLANS, 1, ebufp);
	if(hw_flistp && PIN_FLIST_COUNT(hw_flistp, ebufp)>0)
	{
		/* change the plan obj in existing search input flist */
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Adding HW plan");
		p_cookie = NULL;
		while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(hw_flistp, PIN_FLD_PLAN,
			&p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			//plan_flistp = PIN_FLIST_ELEM_GET(hw_flistp, PIN_FLD_PLAN, 0, 1, ebufp);
			arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_PLAN_OBJ, arg_flistp, PIN_FLD_POID, ebufp);
			result_flistp = PIN_FLIST_ELEM_GET(srch_in_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
			//PIN_FLIST_FLD_DROP(result_flistp,PIN_FLD_PROVISIONING_TAG, ebufp);
			arg_flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_USAGE_MAP, 0, ebufp );
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_EVENT_TYPE, NULL, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "HW. Product Search input", srch_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
			//PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "HW. Product Search output", srch_out_flistp);
			result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp );
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No HW product found for this plan", ebufp);
				goto CLEANUP;
			}
			
			elem_id=0;
			cookie=NULL;	
			while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_END_T, 1, ebufp);
				if(vp) {
					PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_PURCHASE_END_T, result_flistp, PIN_FLD_PURCHASE_END_T, ebufp);
				} else {
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
				}
				PIN_FLIST_FLD_RENAME(result_flistp, PIN_FLD_POID, PIN_FLD_PRODUCT_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_PURCHASE_START_T, result_flistp, PIN_FLD_PURCHASE_START_T, ebufp);
				arg_flistp = PIN_FLIST_ELEM_GET(result_flistp, PIN_FLD_USAGE_MAP, 0,0, ebufp );
				vp = NULL;
				vp = PIN_FLIST_FLD_GET(arg_flistp,PIN_FLD_EVENT_TYPE, 0, ebufp);
				if(vp && strstr(vp,"/event/billing/product/fee/cycle/cycle_forward"))
				{
					PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_ACCOUNT_OBJ, plan_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
					PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ, plan_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
                                        ret_vall = fm_mso_utils_discount_validation(ctxp, plan_flistp, ebufp);
					PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_ACCOUNT_OBJ, ebufp);
                                        PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_SERVICE_OBJ, ebufp);
                                        if (PIN_ERRBUF_IS_ERR(ebufp))
                                        {
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_utils_discount_validation", ebufp);
                                                goto CLEANUP;
                                        }
                                        else if(ret_vall == 1)
                                        {
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in dicount validation where either discount amount or percentage is wrong", ebufp);
						pin_set_err(ebufp, PIN_ERRLOC_FM,
                                 		PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                 		MSO_FLD_DISC_PERCENT, 0, 0, 0);
                                                goto CLEANUP;
                                        }

					PIN_FLIST_FLD_COPY(plan_flistp,MSO_FLD_DISC_AMOUNT, result_flistp, MSO_FLD_DISC_AMOUNT, ebufp);
					PIN_FLIST_FLD_COPY(plan_flistp,MSO_FLD_DISC_PERCENT, result_flistp, MSO_FLD_DISC_PERCENT, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adding discount to Product array", result_flistp);
				}
				PIN_FLIST_ELEM_DROP(result_flistp,PIN_FLD_USAGE_MAP,0, ebufp);
				PIN_FLIST_ELEM_SET(plan_flistp, result_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
			}
			vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_END_T, 1, ebufp);
			if(vp)
				PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_PURCHASE_END_T, ebufp);
				
			vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_START_T, 1, ebufp);
			if(vp)
				PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_PURCHASE_START_T, ebufp);

				vp = PIN_FLIST_FLD_GET(plan_flistp,MSO_FLD_DISC_AMOUNT, 1, ebufp);
			if(vp)
				PIN_FLIST_FLD_DROP(plan_flistp,MSO_FLD_DISC_AMOUNT, ebufp);
				
			vp = PIN_FLIST_FLD_GET(plan_flistp,MSO_FLD_DISC_PERCENT, 1, ebufp);
			if(vp)
				PIN_FLIST_FLD_DROP(plan_flistp,MSO_FLD_DISC_PERCENT, ebufp);
		}
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_products_array return flist", i_flistp);
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}	

/*******************************************************
* Function: fm_mso_update_bill_when()
*	Updates /service with new value of BILL_WHEN 
*	 and indicator field.
*******************************************************/
static int
fm_mso_update_bill_when(
	pcm_context_t		*ctxp,
	poid_t			*acc_obj,
	poid_t			*ser_obj,
	int			bill_when,
	double			priority,
	char			*prog_name,
	pin_errbuf_t            *ebufp)
{

	pin_flist_t *ser_in_flistp = NULL;
	pin_flist_t *ser_out_flistp = NULL;
	pin_flist_t *tmp_flistp = NULL;
	int		indicator = 0;


	pin_flist_t	*v_flistp = NULL;
	pin_flist_t	*vo_flistp = NULL;
	pin_flist_t	*payinfo_flistp = NULL;
	poid_t		*payinfo_pdp = NULL;
	int		is_change_valid = PIN_BOOLEAN_TRUE;
	void		*vp = NULL;
	int		old_indicator = 0;
	poid_t		*a_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 1;

	ser_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(ser_in_flistp, PIN_FLD_POID, ser_obj, ebufp);
	tmp_flistp = PIN_FLIST_SUBSTR_ADD(ser_in_flistp, MSO_FLD_BB_INFO, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_BILL_WHEN, &bill_when, ebufp);
	if(priority>0)
	{	priority = ((int)priority%1000);
		if(priority<500)	{
			indicator = TYPE_BB_PREPAID;
		} else {
			indicator = TYPE_BB_POSTPAID;
		}
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_INDICATOR, &indicator, ebufp);

		/****** Pavan Bellala 12-08-2015 *************************
		Bug id :1062  Validate hierarchy for changing plan type
		Validate before allowing change plan before activation
		****** Pavan Bellala 12-08-2015 ************************/
		/** Fetch payinfo ***/
		fm_mso_bb_get_payinfo_inv(ctxp,acc_obj,&payinfo_flistp,ebufp);
                if(payinfo_flistp != NULL)
                {
                        payinfo_pdp = PIN_FLIST_FLD_TAKE(payinfo_flistp,PIN_FLD_POID,1,ebufp);
                        tmp_flistp = PIN_FLIST_ELEM_GET(payinfo_flistp,PIN_FLD_INV_INFO,0,1,ebufp);
                        if(tmp_flistp!= NULL){
                                vp = PIN_FLIST_FLD_GET(tmp_flistp,PIN_FLD_INDICATOR,1,ebufp);
                                if(vp) old_indicator = *(int*)vp;
                        }

                        PIN_FLIST_DESTROY_EX(&payinfo_flistp,NULL);
                }
                if(old_indicator != indicator){
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Change of connection type to occur.Validate!!!");

			v_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(v_flistp,PIN_FLD_POID,payinfo_pdp,ebufp);
			PIN_FLIST_FLD_SET(v_flistp,PIN_FLD_ACCOUNT_OBJ,acc_obj,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bill_when:input for validate",v_flistp);
			fm_cust_chng_plan_validate_hierarchy(ctxp,v_flistp,&vo_flistp,ebufp);
			if(vo_flistp != NULL){
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_update_bill_when:validation output",vo_flistp);
				vp = PIN_FLIST_FLD_GET(vo_flistp,PIN_FLD_RESULT_FLAGS,1,ebufp);
				is_change_valid = *(int*)vp;
				if(is_change_valid == PIN_BOOLEAN_FALSE)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"fm_mso_update_bill_when:validation not successful");
					return 2;
				}else if(is_change_valid == PIN_BOOLEAN_TRUE){

					a_pdp = PIN_FLIST_FLD_GET(vo_flistp,PIN_FLD_ACCOUNT_OBJ,1,ebufp);
					if(!PIN_POID_IS_NULL(a_pdp)){
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Start updating parent indicator");
						fm_mso_update_bb_payinfo_indi(ctxp,a_pdp,indicator,prog_name,ebufp);			
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Return from updating parent indicator");
					}
				}
			}
		} 
		fm_mso_update_bb_payinfo_indi(ctxp, acc_obj, indicator, prog_name, ebufp);
		
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bill_when: write input", ser_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, ser_in_flistp, &ser_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ser_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
			PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
			return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bill_when: write output", ser_out_flistp);
	PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
	return 0;
}


/*******************************************************
* Function: fm_mso_update_bb_payinfo_indi()
*	Updates payinfo with new values of PAY_TERM
*	  and INDICATOR. 
*
*******************************************************/
static int
fm_mso_update_bb_payinfo_indi(
	pcm_context_t		*ctxp,
	poid_t			*acct_obj,
	int			indicator,
	char			*prog_name,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*pay_in_listp = NULL;
	pin_flist_t		*pay_out_listp = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*inher_info = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*in_flistp = NULL;
	int			pay_term=0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_update_bb_payinfo_indi", ebufp);
                return 1;
        }
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_payinfo_indi ", in_flistp);
	fm_mso_bb_get_payinfo_inv(ctxp, acct_obj, &in_flistp,ebufp);
	if(indicator == PAYINFO_BB_PREPAID) {
		pay_term = PAY_TERM_BB_PREPAID;
	} else {
		pay_term = PAY_TERM_BB_POSTPAID;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Payinfo flist ", in_flistp);

	pay_in_listp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY( in_flistp, PIN_FLD_ACCOUNT_OBJ,  pay_in_listp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(pay_in_listp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp );

	payinfo = PIN_FLIST_ELEM_ADD(pay_in_listp, PIN_FLD_PAYINFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID,  payinfo, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PAY_TYPE,  payinfo, PIN_FLD_PAY_TYPE, ebufp );
	PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAYMENT_TERM,&pay_term, ebufp );

	inher_info = PIN_FLIST_SUBSTR_ADD(payinfo, PIN_FLD_INHERITED_INFO, ebufp);
	inv_info = PIN_FLIST_ELEM_ADD(inher_info, PIN_FLD_INV_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INDICATOR, &indicator, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set Payinfo Input flist ", pay_in_listp);
	PCM_OP(ctxp, PCM_OP_CUST_SET_PAYINFO, 0, pay_in_listp, &pay_out_listp, ebufp);
	PIN_FLIST_DESTROY_EX(&pay_in_listp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating payinfo invoice", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&pay_out_listp, NULL);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set Payinfo output flist ", pay_out_listp);
	PIN_FLIST_DESTROY_EX(&pay_out_listp, NULL);
	return 0;
}
