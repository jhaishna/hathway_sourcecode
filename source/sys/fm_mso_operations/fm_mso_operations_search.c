/*******************************************************************
 * File:	fm_mso_operations_search.c
 * Opcode:	MSO_OP_OPERATIONS_SEARCH 
 * Owner:	Rohini Mogili
 * Created:	08-NOV-2013
 * Description: Caled for changing the status of the service.

	r << xx 1
	0 PIN_FLD_POID                      POID [0] 0.0.0.1 /config/business_params 10830 4
	0 PIN_FLD_USERID		POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_PARAMS                   ARRAY [32] allocated 4, used 4
	1     PIN_FLD_DESCR                  STR [0] "This parameter is for applying the reactivation fee if the reactivation period is more than 90 days in a single year"
	1     PIN_FLD_PARAM_NAME             STR [0] "apply_reactivation_fee"
	1     PIN_FLD_PARAM_TYPE             INT [0] 1
	1     PIN_FLD_PARAM_VALUE            STR [0] "0"
	xx
	xop 11060 2 1 

	xop: opcode 11060, flags 2
	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /config/business_params 10830 4
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_DESCR           STR [0] "ACTION - Updating the object completed successfully"



	r << xx 1
	0 PIN_FLD_POID                      POID [0] 0.0.0.1 /config/business_params 10830 4
	0 PIN_FLD_USERID		POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_PARAMS                   ARRAY [32] allocated 4, used 4
	1     PIN_FLD_DESCR                  STR [0] "This parameter is for applying the reactivation fee if the reactivation period is more than 90 days in a single year"
	1     PIN_FLD_PARAM_NAME             STR [0] "apply_reactivation_fee"
	1     PIN_FLD_PARAM_TYPE             INT [0] 1
	1     PIN_FLD_PARAM_VALUE            STR [0] "0"
	xx
	xop 11060 3 1

	xop: opcode 11060, flags 3
	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /config/business_params 10830 6
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_DESCR           STR [0] "ACTION - Deleting few fields completed successfully"


 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_operations_search.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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

#define READONLY 0
#define READWRITE 1
#define LOCK_OBJ 2

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1

#define NAMEINFO_BILLING 1
#define NAMEINFO_INSTALLATION 2

#define ACCOUNT_MODEL_UNKNOWN 0
#define ACCOUNT_MODEL_RES 1
#define ACCOUNT_MODEL_CORP 2
#define ACCOUNT_MODEL_MDU 3



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

extern int 
fm_mso_validate_csr_role(
	pcm_context_t		*ctxp,
	int64			db,
	poid_t		*user_id,
	pin_errbuf_t		*ebufp);
/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/
static int 
fm_mso_refund_rule(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_lco_settlement_products(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_state_tax_zones(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_city_tax_zones(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_catv_channel_master(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_catv_pt(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_catv_pt_channel_map(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_bb_pt(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_catv_pt_channel_map_exact(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_catv_prod_pt(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_OPERATIONS_SEARCH 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_operations_search(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_operations_search(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_OPERATIONS_SEARCH  
 *******************************************************************/
void 
op_mso_operations_search(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	int			*status = NULL;
	int32			status_uncommitted =2;
	poid_t			*inp_pdp = NULL;
	char            *prog_name = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_OPERATIONS_SEARCH) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_operations_search error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_operations_search input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_operations_search()", ebufp);
		return;
	}*/

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//	if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
        if (!(prog_name && (strstr(prog_name,"pin_deferred_act") || strstr(prog_name,"fm_mso_pdt_channels_validation"))))
	{
			inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
			local = fm_mso_trans_open(ctxp, inp_pdp, 1,ebufp);
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

	fm_mso_operations_search(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
	{		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_operations_search error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS &&
			(!(prog_name && (strstr(prog_name,"pin_deferred_act") || strstr(prog_name,"fm_mso_pdt_channels_validation"))))
	 	  )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_operations_search output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_operations_search(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*action_inflistp = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*user_id = NULL;
	
	int				action_success = 0;
	int				action_failure = 1;
	int				acct_update = 0;
	int				csr_access = 0;
	int				*action_flags = NULL;
	int64			db = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_operations_search input flist", i_flistp);	
	
	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if (routing_poid)
	{	
		db = PIN_POID_GET_DB(routing_poid);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_operations_search poid value", routing_poid);	
		user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp); 
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &action_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51052", ebufp);
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
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51053", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ROLE passed in input does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR has permission to change account information");	
		}
	}else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &action_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51054", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}

	/*******************************************************************
	* Validation for PRICING_ADMIN roles  - End
	*******************************************************************/

	action_inflistp = PIN_FLIST_COPY(i_flistp, ebufp);
	PIN_FLIST_FLD_DROP(action_inflistp, PIN_FLD_USERID, ebufp); 	

	/*******************************************************************
	* Action - Start
	*******************************************************************/

	switch (flags)
	{
		case 1 :
			acct_update = fm_mso_refund_rule(ctxp, action_inflistp, &ret_flistp, ebufp);
			break;
		case 2 :
			acct_update = fm_mso_lco_settlement_products(ctxp, action_inflistp, &ret_flistp, ebufp);
			break;
		case 3 :
			acct_update = fm_mso_state_tax_zones(ctxp, action_inflistp, &ret_flistp, ebufp);
			break;
		case 4 :
			acct_update = fm_mso_city_tax_zones(ctxp, action_inflistp, &ret_flistp, ebufp);
			break;
		case 5 :
			acct_update = fm_mso_catv_channel_master(ctxp, action_inflistp, &ret_flistp, ebufp);
			break;
		case 6 :
			acct_update = fm_mso_catv_pt(ctxp, action_inflistp, &ret_flistp, ebufp);
			break;
		case 7 :
			acct_update = fm_mso_catv_pt_channel_map(ctxp, action_inflistp, &ret_flistp, ebufp);
			break;
		case 8 :
			acct_update = fm_mso_bb_pt(ctxp, action_inflistp, &ret_flistp, ebufp);
			break;
		default :
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid option");
			break;	
	}

	action_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (action_flags && (*action_flags == 9))
	{
		acct_update = fm_mso_catv_pt_channel_map_exact(ctxp, action_inflistp, &ret_flistp, ebufp);
	}
	else if (action_flags && (*action_flags == 10))
	{
		acct_update = fm_mso_catv_prod_pt(ctxp, action_inflistp, &ret_flistp, ebufp);
	}


	if ( acct_update == 0)
	{		
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &action_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51052", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Master data setup for given input failed", ebufp);
		goto CLEANUP;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Action completed successfully", ret_flistp);
	PIN_FLIST_DESTROY_EX(&action_inflistp, NULL);

	/*******************************************************************
	* Action  - End
	*******************************************************************/	
	
	CLEANUP:	
	*r_flistpp = ret_flistp;
	return;
}

/**************************************************
* Function: fm_mso_refund_rule()
* 
* 
***************************************************/
static int 
fm_mso_refund_rule(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			search_template[100] = " select X from /mso_cfg_refund_rule where F1.type = V1 ";

	int				search_flags = 768;
	int			action_success = 0;
	int			action_failure = 1;	

	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_refund_rule :");		

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_refund_rule input list", in_flist);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_refund_rule search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_refund_rule search output flist", search_outflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}

	PIN_FLIST_FLD_SET(search_outflistp, PIN_FLD_STATUS, &action_success, ebufp);

	/******************************************* reaction fee flist ***********************************************/

	*out_flist = search_outflistp;	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_refund_rule final output flist", *out_flist);

	return 2;
}

/**************************************************
* Function: fm_mso_lco_settlement_products()
* 
* 
***************************************************/
static int 
fm_mso_lco_settlement_products(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			search_template[100] = " select X from /mso_cfg_lco_settlement_products where F1.type = V1 ";

	int				search_flags = 768;

	int			action_success = 0;
	int			action_failure = 1;	

	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_lco_settlement_products :");		

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_lco_settlement_products input list", in_flist);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_lco_settlement_products search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_lco_settlement_products search output flist", search_outflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}

	PIN_FLIST_FLD_SET(search_outflistp, PIN_FLD_STATUS, &action_success, ebufp);

	/******************************************* reaction fee flist ***********************************************/

	*out_flist = search_outflistp;	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_lco_settlement_products final output flist", *out_flist);

	return 2;
}

/**************************************************
* Function: fm_mso_state_tax_zones()
* 
* 
***************************************************/
static int 
fm_mso_state_tax_zones(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			search_template[100] = " select X from /mso_cfg_state_tax_zones where F1.type = V1 ";

	int				search_flags = 768;

	int			action_success = 0;
	int			action_failure = 1;	

	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_state_tax_zones :");		

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_state_tax_zones input list", in_flist);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_state_tax_zones search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_state_tax_zones search output flist", search_outflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}

	PIN_FLIST_FLD_SET(search_outflistp, PIN_FLD_STATUS, &action_success, ebufp);

	/******************************************* reaction fee flist ***********************************************/

	*out_flist = search_outflistp;	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_state_tax_zones final output flist", *out_flist);

	return 2;
}

/**************************************************
* Function: fm_mso_city_tax_zones()
* 
* 
***************************************************/
static int 
fm_mso_city_tax_zones(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			search_template[100] = " select X from /mso_cfg_city_tax_zones where F1.type = V1 ";

	int				search_flags = 768;

	int			action_success = 0;
	int			action_failure = 1;	

	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_city_tax_zones :");		

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_city_tax_zones input list", in_flist);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_city_tax_zones search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_city_tax_zones search output flist", search_outflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}

	PIN_FLIST_FLD_SET(search_outflistp, PIN_FLD_STATUS, &action_success, ebufp);

	/******************************************* reaction fee flist ***********************************************/

	*out_flist = search_outflistp;	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_city_tax_zones final output flist", *out_flist);

	return 2;
}


/**************************************************
* Function: fm_mso_catv_channel_master()
* 
* 
***************************************************/
static int 
fm_mso_catv_channel_master(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			search_template[100] = " select X from /mso_cfg_catv_channel_master where F1.type = V1 ";

	int				search_flags = 768;

	int			action_success = 0;
	int			action_failure = 1;	

	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_channel_master :");		

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_channel_master input list", in_flist);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_channel_master search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_channel_master search output flist", search_outflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}

	PIN_FLIST_FLD_SET(search_outflistp, PIN_FLD_STATUS, &action_success, ebufp);

	/******************************************* reaction fee flist ***********************************************/

	*out_flist = search_outflistp;	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_channel_master final output flist", *out_flist);

	return 2;
}

/**************************************************
* Function: fm_mso_catv_pt()
* 
* 
***************************************************/
static int 
fm_mso_catv_pt(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			search_template[100] = " select X from /mso_cfg_catv_pt where F1.type = V1 ";

	int				search_flags = 768;

	int			action_success = 0;
	int			action_failure = 1;	

	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt :");		

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt input list", in_flist);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt search output flist", search_outflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}

	PIN_FLIST_FLD_SET(search_outflistp, PIN_FLD_STATUS, &action_success, ebufp);

	/******************************************* reaction fee flist ***********************************************/

	*out_flist = search_outflistp;	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt final output flist", *out_flist);

	return 2;
}

/**************************************************
* Function: fm_mso_catv_pt_channel_map()
* 
* 
***************************************************/
static int 
fm_mso_catv_pt_channel_map(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			search_template[100] = " select X from /mso_cfg_catv_pt_channel_map where F1.type = V1 ";

	int				search_flags = 768;

	int			action_success = 0;
	int			action_failure = 1;	

	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt_channel_map :");		

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt_channel_map input list", in_flist);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt_channel_map search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt_channel_map search output flist", search_outflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}

	PIN_FLIST_FLD_SET(search_outflistp, PIN_FLD_STATUS, &action_success, ebufp);

	/******************************************* reaction fee flist ***********************************************/

	*out_flist = search_outflistp;	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt_channel_map final output flist", *out_flist);

	return 2;
}

/**************************************************
* Function: fm_mso_bb_pt()
* 
* 
***************************************************/
static int 
fm_mso_bb_pt(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			search_template[100] = " select X from /mso_config_bb_pt where F1.type = V1 ";

	int				search_flags = 768;

	int			action_success = 0;
	int			action_failure = 1;	

	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_pt :");		

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_pt input list", in_flist);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_pt search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_pt search output flist", search_outflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}

	PIN_FLIST_FLD_SET(search_outflistp, PIN_FLD_STATUS, &action_success, ebufp);

	/******************************************* reaction fee flist ***********************************************/

	*out_flist = search_outflistp;	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_pt final output flist", *out_flist);

	return 2;
}

/**************************************************
* Function: fm_mso_catv_pt_channel_map_exact()
* Retrieves all the channels mapped to the product poid
* 
***************************************************/
static int 
fm_mso_catv_pt_channel_map_exact(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			search_template[100] = "select X from /mso_cfg_catv_pt_channel_map 1, /product 2 where 2.F1 = V1 and 2.F2 = 1.F3";
	int				search_flags = 768;
	int			action_success = 0;
	int			action_failure = 1;	
	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt_channel_map_exact :");		

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt_channel_map_exact input list", in_flist);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);	

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, MSO_FLD_PKG_TYPE, 0, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(results_flistp, MSO_FLD_CATV_CHANNELS, PIN_ELEMID_ANY, ebufp);
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt_channel_map_exact search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt_channel_map_exact search output flist", search_outflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}

	PIN_FLIST_FLD_SET(search_outflistp, PIN_FLD_STATUS, &action_success, ebufp);

	*out_flist = search_outflistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_pt_channel_map_exact final output flist", *out_flist);

	return 2;
}


/********************************************************
* Function: fm_mso_catv_prod_pt()
* Retrieves provisioning tag mapped to the product poid
* 
********************************************************/
static int 
fm_mso_catv_prod_pt(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	char			search_template[100] = "select X from /product  where F1 = V1";
	int				search_flags = 768;
	int				action_success = 0;
	int				action_failure = 1;
	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_prod_pt input list", in_flist);

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp));

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_NAME, 0, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_CODE, 0, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PRIORITY, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_prod_pt search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_prod_pt search output flist", search_outflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}

	PIN_FLIST_FLD_SET(search_outflistp, PIN_FLD_STATUS, &action_success, ebufp);

	*out_flist = search_outflistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_prod_pt final output flist", *out_flist);

	return 2;
}
