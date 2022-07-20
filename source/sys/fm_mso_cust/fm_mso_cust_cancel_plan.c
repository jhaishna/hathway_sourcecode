/*******************************************************************
o     PIN_FLD_POID           POID [0] 0.0.0.1 /billinfo 265170974 100
 * File:	fm_mso_cust_cancel_plan.c
 * Opcode:	MSO_OP_CUST_CANCEL_PLAN 
 * Owner:	Rohini Mogili
 * Created:	18-NOV-2013
 * Description:  This opcode is for cancel the plan

	r << xx 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 44029 10
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 16
	0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
	0 PIN_FLD_PLAN_LISTS	ARRAY [0]
	1     PIN_FLD_PLAN_OBJ    POID [0] 0.0.0.1 /plan 41089 8
	1     PIN_FLD_DEALS          ARRAY [0] allocated 5, used 5
	2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 42530 0
	2         PIN_FLD_PACKAGE_ID      INT [0] 20
	1     PIN_FLD_DEALS          ARRAY [1] allocated 5, used 5
	2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 43554 0
	2         PIN_FLD_PACKAGE_ID      INT [0] 21
	xx
	xop 11006 0 1

	xop: opcode 11006, flags 0
	# number of field entries allocated 20, used 4
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 16
	0 PIN_FLD_DESCR           STR [0] "ACCOUNT - Service cancel plan completed successfully"

 *
 * Modification History:
 * Modified By: Leela Pratyush
 * Date:
 * Modification details
 *
 * Modification History:
 * Modified By: Pavan Bellala
 * Date: 08/July/2015
 * Modification details : Added changes for disassociating device for H/W plan
 *			  Call deposit refund opcode to refund deposit charges
 *
 *************************************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_cancel_plan.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_cust.h"
#include "mso_ops_flds.h"
#include "mso_prov.h"
#include "mso_lifecycle_id.h"


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

extern void 
fm_mso_update_ncr_validty(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

//extern void
//get_evt_lifecycle_template(
//	pcm_context_t			*ctxp,
//	int64				db,
//	int32				string_id, 
//	int32				string_version,
//	char				**lc_template, 
//	pin_errbuf_t			*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_mso_get_account_info(
    pcm_context_t   *ctxp,
    poid_t      *acnt_pdp,
    pin_flist_t **ret_flistp,
    pin_errbuf_t    *ebufp);

/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/
extern int 
fm_mso_validate_csr_role(
	pcm_context_t		*ctxp,
	int64			db,
	poid_t		*user_id,
	pin_errbuf_t		*ebufp);

EXPORT_OP int 
fm_mso_cancel_deal(
	pcm_context_t		*ctxp,
	int32			flag,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);  

PIN_IMPORT void
mso_cancel_alc_addon_pdts(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void 
fm_mso_validate_fdp_et_align(
	pcm_context_t		*ctxp,
	u_int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);


static void
fm_mso_cust_cancel_plan_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_srvc_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistp,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_validate_pkg_id(
	pcm_context_t	*ctxp,
	poid_t		*svc_obj,
	poid_t		*deal_obj,
	int32		pkg_id,
	pin_errbuf_t	*ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_CANCEL_PLAN 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_cancel_plan(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_cancel_plan(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_bb_update_status(
	pcm_context_t		*ctxp,
	poid_t 	*account_obj,
	poid_t 	*service_obj,
	poid_t	*plan_obj,
	poid_t	**mso_plan_obj,
	int		*bp_flag,
	char            *prog_name,
	pin_flist_t	*user_id,
	pin_errbuf_t            *ebufp); 

extern void
fm_mso_commission_error_processing (
        pcm_context_t           *ctxp,
        int                     error_no,
        char                    *error_in,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

int
mso_cancel_plan_refund_hw_deposit(
        pcm_context_t   *ctxp,
        poid_t          *account_pdp,
        poid_t          *service_pdp,
        poid_t          *device_pdp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_cust_cancel_amc_plan(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        poid_t                  *service_obj,
        pin_flist_t             *user_id,
        poid_t                  **mso_plan_obj,
        pin_errbuf_t            *ebufp);

		
static	int
fm_mso_compare_ca_ids(
	pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t            *ebufp);
	
static	void
fm_mso_get_ca_id_from_plan(
	pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
	poid_t                  *mso_plan_obj,
	char                  	*mso_node,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t            *ebufp);

static	void
fm_mso_get_cust_active_plans(
	pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t            *ebufp);		

/*******************************************************************
 * MSO_OP_CUST_CANCEL_PLAN  
 *******************************************************************/
void 
op_mso_cust_cancel_plan(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
    pin_flist_t         *services_flistp = NULL;
	*r_flistpp		= NULL;
	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	int			*status = NULL;
	int32			status_uncommitted =2;
	poid_t			*inp_pdp = NULL;
    poid_t          *account_obj = NULL;
	char            *prog_name = NULL;
	char            *descr = NULL;
    char            *stb_idp = NULL;
	char            msg[100];
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_CANCEL_PLAN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_cancel_plan error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_cancel_plan input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_cancel_plan()", ebufp);
		return;
	}*/

    stb_idp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp);
    if (stb_idp)
    {
        fm_mso_cust_get_acc_from_sno(ctxp, i_flistp, &services_flistp, ebufp);
        if (services_flistp)
        {
            PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_SERVICE_OBJ, i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
            PIN_FLIST_DESTROY_EX(&services_flistp, NULL);
        }
        else
        {
            PIN_FLIST_DESTROY_EX(&services_flistp, NULL);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Device serial number not found");
            *r_flistpp = PIN_FLIST_CREATE(ebufp);
		    inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, inp_pdp, ebufp );
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51430", ebufp);
            PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Device serial number not found", ebufp);
            return;
        }
    }
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	sprintf( msg,"Program name is : %s",prog_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_NAME, 1, ebufp);
	
	if((descr && (strcmp(descr, "Bulk_cancel_plan") == 0)) ||
	(descr && (strcmp(descr, "top_up_plan") == 0)) ||
	(descr && (strcmp(descr, "deactivate_bb_service") == 0)) ||
	(descr && (strcmp(descr, "swap_bb_device") == 0)) )
	{
	  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by special operations so transaction will not be handled at API level");
	  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,descr);
	}
	else
	{	
	if ( !(prog_name && strstr(prog_name,"pin_deferred_act")) )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Program name is not Topup");
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
	}
	fm_mso_cust_cancel_plan(ctxp, flags, i_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Cancel PLan result flist:",*r_flistpp);	
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 **********************************************************/
	if((descr && (strcmp(descr, "Bulk_cancel_plan") == 0)) ||
        (descr && (strcmp(descr, "top_up_plan") == 0)) ||
        (descr && (strcmp(descr, "deactivate_bb_service") == 0)) ||
        (descr && (strcmp(descr, "swap_bb_device") == 0)) )
        {
	  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'Bulk_cancel_plan' so transaction will not be handled at API level");
	}
	else
	{	 
	if (PIN_ERRBUF_IS_ERR(ebufp) || (status && *(int*)status == 1)) 
	{		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"op_mso_cust_cancel_plan in flistp", i_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_cancel_plan error", ebufp);
		if( local == LOCAL_TRANS_OPEN_SUCCESS && 
		   ( !(prog_name && strstr(prog_name,"pin_deferred_act")) ))
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS &&
		   ( !(prog_name && strstr(prog_name,"pin_deferred_act")) ))
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else if(local == LOCAL_TRANS_OPEN_SUCCESS &&
	           ((prog_name && strcmp(prog_name,"top_up_plan") == 0 )) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
		}
		else
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}		
	}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_cancel_plan output flist", *r_flistpp);
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_cancel_plan(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*user_id = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*s_pdp = NULL;
	pin_flist_t             *planlists_flistp = NULL;	
	pin_flist_t             *tmp_flistp = NULL;	
	pin_flist_t             *search_i_flistp = NULL;	
	pin_flist_t             *out_flistpp = NULL;	
	pin_flist_t             *results_flist = NULL;	
	pin_flist_t             *planlist_flistp = NULL;	
	pin_flist_t             *r_flistp = NULL;
	//char			*account_no_str = NULL;
	//char			account_no_str[10];
	int32			cancel_plan_success = 0;
	int32			cancel_plan_failure = 1;
	int32               	s_flags;
	int32               	*status = NULL;
	int64			db = -1;
	int				csr_access = 0;
	int				acct_update = 0;
	char 			*template = " select X from /purchased_product where  F1 = V1  AND F2 = V2 AND F3 = V3 ";
	char			*descr = NULL;
	char			*action_name = NULL;
	pin_decimal_t		*ref_amt =NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan input flist", i_flistp);	
	
	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	
	descr 	= PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);	
	
	action_name   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_NAME, 1, ebufp);	

	if (routing_poid)
	{	
		db = PIN_POID_GET_DB(routing_poid);
		account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		//account_no_str = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp); 
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51417", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	if (!account_obj)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51418", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account POID value not passed in input as it is mandatory field", ebufp);
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
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51419", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ROLE passed in input does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR haas permission to change account information");	
		}
	}else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51420", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}

	/*******************************************************************
	* Validation for PRICING_ADMIN roles  - End
	*******************************************************************/

	/*******************************************************************
	* Cancel plan - Start
	*******************************************************************/
	
	/*********************
	 * Check for cancelled product is already available or not
	 ******/
	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if(descr && (strcmp(descr, "Bulk_cancel_plan") == 0))
        {

		db = PIN_POID_GET_DB(routing_poid);
	
		planlist_flistp = PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_PLAN_LISTS,PIN_ELEMID_ANY, 1, ebufp);

		plan_pdp = PIN_FLIST_FLD_GET(planlist_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
	
		search_i_flistp = PIN_FLIST_CREATE(ebufp);
    		s_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);
    		PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    		s_flags = SRCH_DISTINCT ;
    		PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
    		PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_INDEX_NAME, "I_PURCHASED_PRODUCT_ACT__ID", ebufp);

       	 	tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    		PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);
    		PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, NULL, ebufp);
	
		PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);
        	tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, routing_poid, ebufp);
        	tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
        	tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
        	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        		"fm_mso_cancel_plan search purchased product input flist ", search_i_flistp);

		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        			&out_flistpp, ebufp);

    		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        		"fm_mso_cancel_plan search purchased product output flist", out_flistpp);

		PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
	
		if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
               	 	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH for finding the Purchased product", ebufp);
                	PIN_FLIST_DESTROY_EX(&out_flistpp, NULL);
			PIN_ERRBUF_RESET(ebufp);
                	ret_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                	PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_ERROR_CODE, "51624", ebufp);
                	PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_ERROR_DESCR,"fm_mso_cust_cancel_plan search Purchased product error",ebufp);
                	goto CLEANUP;
	        }

		if(PIN_FLIST_ELEM_COUNT(out_flistpp , PIN_FLD_RESULTS , ebufp) == 1)	
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Only One Purchased product found ");
		
			results_flist = PIN_FLIST_ELEM_GET(out_flistpp, PIN_FLD_RESULTS , PIN_ELEMID_ANY, 1, ebufp);
		
			status = PIN_FLIST_FLD_GET(results_flist, PIN_FLD_STATUS , 1, ebufp);

			if ( *status == 3)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Already the product is cancelled ");
				ret_flistp = PIN_FLIST_CREATE(ebufp);
               	 		PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                		PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
                		PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_ERROR_CODE, "51624", ebufp);
                		PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_ERROR_DESCR,"The Plan is Already in Cancelled state hence Cant Cancel again ",ebufp);
                		goto CLEANUP;

			}		
			else
			{
				acct_update = fm_mso_cancel_deal(ctxp, flags, i_flistp, &ret_flistp, ebufp);

                		if ( acct_update == 0)
                		{
                        		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
                		}
                		else if ( acct_update == 1)
                		{
                        		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan no need of cancel plan");
                		}
                		else if ( acct_update == 2)
                		{
                        		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan cancel plan successful");
					/****************************************************
					 * Calling Validate for FDP pack and ET refund 
					 ****************************************************
					commenting as cancellation ET refund not required 27-DEC-2016
                			if (service_pdp && strstr(PIN_POID_GET_TYPE(service_pdp), "catv"))
					{    
						fm_mso_validate_fdp_et_align(ctxp, flags, i_flistp, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan ET Refund successful");
					}    */
					if (service_pdp && strstr(PIN_POID_GET_TYPE(service_pdp), "catv") && (flags == PCM_OPFLG_CALC_ONLY))
					{
						if (!PIN_ERR_IS_ERR(ebufp) ){
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan ET Refund successful");
							if(r_flistp){
								ref_amt = PIN_FLIST_FLD_GET(r_flistp, MSO_FLD_REFUND_AMOUNT, 1, ebufp);
								if(!pbo_decimal_is_null(ref_amt, ebufp)){
									PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
								}
								else{
									ref_amt = pbo_decimal_from_str("0", ebufp);
									PIN_FLIST_FLD_PUT(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
								}
							}else{
								ref_amt = pbo_decimal_from_str("0", ebufp);
								PIN_FLIST_FLD_PUT(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
							}
							PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cancel_plan_success, ebufp);
							*r_flistpp = ret_flistp;
						}
						else {
							PIN_ERR_BUF_CLEAR(ebufp);
							*r_flistpp =  PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
							PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ, *r_flistpp, PIN_FLD_SERVICE_OBJ, ebufp );
							PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
							PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_ERROR_CODE, "34003", ebufp);
							PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_ERROR_DESCR,"ET Refund failed",ebufp);
							if(ret_flistp) PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
						}
						PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
						return;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan ET Refund successful");
					}
					fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_REMOVE_PLAN, ebufp);
                		}
                		else if ( acct_update == 3)
                		{
                        	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan insufficient data provided");
                		}

			}

		}
		else if (PIN_FLIST_ELEM_COUNT(out_flistpp , PIN_FLD_RESULTS , ebufp) == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No Purchase product exist");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                        PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_ERROR_CODE, "51624", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp,PIN_FLD_ERROR_DESCR,"No Purchased Product Found for the given Plan Hence Cancellation is not Possible ",ebufp);
                        goto CLEANUP;

		}
		else if (PIN_FLIST_ELEM_COUNT(out_flistpp , PIN_FLD_RESULTS , ebufp) > 1)
                {
	                acct_update = fm_mso_cancel_deal(ctxp, flags, i_flistp, &ret_flistp, ebufp);

	                if ( acct_update == 0)
	                {		
		              PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
	                }
	                else if ( acct_update == 1)
	                {		
		               PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan no need of cancel plan");	
	                }
	                else if ( acct_update == 2)
	                {		
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan cancel plan successful");	
				/****************************************************
				 * Calling Validate for FDP pack and ET refund 
				 ****************************************************
				if (service_pdp && strstr(PIN_POID_GET_TYPE(service_pdp), "catv"))
				{    
					fm_mso_validate_fdp_et_align(ctxp, flags, i_flistp, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan ET Refund successful");
				}
				//fm_mso_cust_cancel_plan_lc_event(ctxp, i_flistp, ret_flistp, ebufp);*/
				if (service_pdp && strstr(PIN_POID_GET_TYPE(service_pdp), "catv") && flags == PCM_OPFLG_CALC_ONLY)
				{    
					fm_mso_validate_fdp_et_align(ctxp, flags, i_flistp, &r_flistp, ebufp);
					if (!PIN_ERR_IS_ERR(ebufp)){
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan ET Refund successful");
						if(r_flistp){
							ref_amt = PIN_FLIST_FLD_GET(r_flistp, MSO_FLD_REFUND_AMOUNT, 1, ebufp);
							if(!pbo_decimal_is_null(ref_amt, ebufp)){
								PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
							}
							else{
								ref_amt = pbo_decimal_from_str("0", ebufp);
								PIN_FLIST_FLD_PUT(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
							}
						}
						else{
							ref_amt = pbo_decimal_from_str("0", ebufp);
							PIN_FLIST_FLD_PUT(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
						}
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cancel_plan_success, ebufp);
						*r_flistpp = ret_flistp;
					}
					else{
						*r_flistpp =  PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ, *r_flistpp, PIN_FLD_SERVICE_OBJ, ebufp );
						PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_ERROR_CODE, "34003", ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_ERROR_DESCR,"ET Refund failed",ebufp);
						if(ret_flistp) PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
					}
					PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
					return;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan ET Refund successful");
				}
				fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_REMOVE_PLAN, ebufp);
	                }
	                else if ( acct_update == 3)
	                {		
		              PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan insufficient data provided");	
	                }

		}		
	}
	else
	{	
		acct_update = fm_mso_cancel_deal(ctxp, flags, i_flistp, &ret_flistp, ebufp);

		if ( acct_update == 0)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
		}
		else if ( acct_update == 1)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan no need of cancel plan");	
		}
		else if ( acct_update == 2)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan cancel plan successful");
			/****************************************************
			 * Calling Validate for FDP pack and ET refund 
			 ****************************************************/
			if (service_pdp && strstr(PIN_POID_GET_TYPE(service_pdp), "catv") && flags == PCM_OPFLG_CALC_ONLY)
			{    
				//fm_mso_validate_fdp_et_align(ctxp, flags, i_flistp, ebufp);
				//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan ET Refund successful");
				fm_mso_validate_fdp_et_align(ctxp, flags, i_flistp, &r_flistp,ebufp);
				if (!PIN_ERR_IS_ERR(ebufp) ){
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan ET Refund successful");
					if(r_flistp){
						ref_amt = PIN_FLIST_FLD_GET(r_flistp, MSO_FLD_REFUND_AMOUNT, 1, ebufp);
						if(!pbo_decimal_is_null(ref_amt, ebufp)){
							PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
						}
						else{
							ref_amt = pbo_decimal_from_str("0", ebufp);
							PIN_FLIST_FLD_PUT(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
						}
					}
					else{
						ref_amt = pbo_decimal_from_str("0", ebufp);
						PIN_FLIST_FLD_PUT(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
					}
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &cancel_plan_success, ebufp);
					*r_flistpp = ret_flistp;
				}
				else{
					if(PIN_ERR_IS_ERR(ebufp))
						PIN_ERR_CLEAR_BUF(ebufp);
					*r_flistpp =  PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ, *r_flistpp, PIN_FLD_SERVICE_OBJ, ebufp );
					PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_ERROR_CODE, "34003", ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_ERROR_DESCR,"ET Refund failed",ebufp);
					if(ret_flistp) PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
				}
				PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
				return;
				//fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_REMOVE_PLAN, ebufp);
			}
			if(action_name && (strcmp(action_name,"deactivate_bb_service") != 0))
                        {
                                fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_REMOVE_PLAN, ebufp);
                        }
			//New Tariff - Transaction mapping Start*/
                        if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0 )
                        {
				fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_REMOVE_PLAN, ebufp);	
			}
			//New Tariff - Transaction mapping End*/
		}
		else if ( acct_update == 3)
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan insufficient data provided");	
		}
	}

	/*******************************************************************
	* Cancel plan  - End
	*******************************************************************/

	CLEANUP:	
	*r_flistpp = ret_flistp;
	return;
}

/**************************************************
* Function: fm_mso_cancel_deal()
* 
* 
***************************************************/
EXPORT_OP int 
fm_mso_cancel_deal(
	pcm_context_t		*ctxp,
	int32			flag,
	pin_flist_t		*in_flist,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*planlists_flistp = NULL;
	pin_flist_t		*canceldeal_inflistp = NULL;
	pin_flist_t		*canceldeal_outflistp = NULL;
	pin_flist_t		*dealinfo_flistp = NULL;
	pin_flist_t		*deals_flistp = NULL;
	pin_flist_t		*readplan_inflistp = NULL;
	pin_flist_t		*readplan_outflistp = NULL;
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
    pin_flist_t     *plan_detail_in_flistp = NULL;
    pin_flist_t     *plan_detail_out_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*read_iflist = NULL;
	pin_flist_t		*read_oflist = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*total_flistp = NULL;
    pin_flist_t     *acnt_info    = NULL;
    pin_flist_t     *serv_flistp = NULL;
    pin_flist_t     *event_local_flistp1 = NULL;
    pin_flist_t     *args_flistp1 = NULL;
    pin_flist_t     *bal_impt2 = NULL;
    pin_flist_t     *args_flistp2 = NULL;
    pin_flist_t     *args_flistp3 = NULL;
    pin_flist_t     *results_flistp1 = NULL;
    pin_flist_t     *search_outflistp1 = NULL;
    pin_flist_t     *results_flistp2 = NULL;
    pin_flist_t     *results_flistp3 = NULL;
    pin_flist_t     *results_flistp4 = NULL;
    pin_flist_t     *update_serv_ipflist = NULL;
    pin_flist_t     *event_ipflist = NULL;
    pin_flist_t     *update_serv_opflist = NULL;
    pin_flist_t     *total_amtp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*offering_obj = NULL;
	poid_t			*read_deal_obj = NULL;
	poid_t			*get_deal_obj = NULL;
	poid_t			*product_obj = NULL;
	poid_t			*mso_pp_obj = NULL;
	poid_t			*event_pdp = NULL;
    poid_t          *evt_pdp = NULL;
	int			elem_id = 0;
	int			elem_base = 0;
	int			*package_id = NULL;
	int			pkg_id = 0;
	int			elem_pp = 0;
	int			cancel_plan_success = 0;
	int			cancel_plan_failure = 1;
	int			c_elem_id = 0;
	pin_cookie_t		cookie_base = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie_pp = NULL;
	pin_cookie_t		cookie_countdeals = NULL;
	pin_cookie_t		c_cookie = NULL;

	int			bb_service = 0;
	int			update_status = 0;
	int			elem_countplan = 0;
	int			elem_countdeals = 0;	
	int			flags = 768;
	int			cnt = 0;
	int64		db = 0;
	int			hardware = 0;
	int32		count = 0;
	int			*prov_call = NULL;
	int			bp_flag = 0;
	int			ret_val = -1;
	int32			add_prov = -1;
	int32			is_base_pack = -1;
	int32           bus_type = 0;
    int32           flag1 = 0;
    int32           dpo_flags = 0;
    int             days = 0;
    int             extra_days = 0;
    int             plan_days = 0;
    double          charge_days = 0;
    double          extra_charge = 0;
    double          plandays = 1;
    time_t          *sus_date = NULL;
    time_t          *start_tp = NULL;
    time_t          *end_tp = NULL;
    time_t          *product_end_tp = NULL;
    time_t          pvt = 0;

    pin_decimal_t		*hundred = NULL;
	pin_decimal_t		*thousand = NULL;
	pin_decimal_t		*priority = NULL;
	pin_decimal_t		*ref_amt = NULL;
    pin_decimal_t   *charge_amount_1 = NULL;
    pin_decimal_t   *charge_amount = pbo_decimal_from_str("0.0", ebufp);
    pin_decimal_t   *refund_amtp = pbo_decimal_from_str("0.0", ebufp);
    pin_decimal_t   *neg_amountp = pbo_decimal_from_str("-1.0", ebufp);

	char			search_template[100] = " select X from /purchased_product where F1 = V1 and F2 = V2 ";
    char            search_template1[100] = " select X from /event where F1 = V1 and F2 = V2 and F3 = V3 and F4 > V4";
	char			*plan_name = NULL;
	char			*prog_name = NULL;
	char			msg[100];
	char			*prov_tag = NULL;
	char            *code = NULL;
    char            tmp[200]="";
    pin_flist_t     *lco_err_flistp = NULL;
    int64           lco_err_code = 0;
	pin_flist_t		*user_id = NULL;
	int32 			comp_flag = 0;
	int32 			*status_flags_new = NULL;
    void            *vp = NULL;
        //New Tariff - Transaction mapping Start/
        pin_flist_t             *lifecycle_flistp = NULL;
        pin_flist_t             *lc_plan_list_flistp = NULL;
        pin_flist_t             *eflistp = NULL;
        int32                   offer_count = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal :");	

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );
	//prog_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACTION_NAME, 1, ebufp);
	flags = 4;

	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		bb_service = 1;
		flags = DOC_BB_END_SUBS;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"BB service type");
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &flags, ebufp);
	}
	else 
	{
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &flags, ebufp);
	}

    if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
    {
                //New Tariff - Transaction mapping Start*/
                if(lifecycle_flistp == NULL) {
                        lifecycle_flistp = PIN_FLIST_CREATE(ebufp);
                }
                //New Tariff - Transaction mapping End*/
        plan_name = PIN_FLIST_FLD_TAKE(in_flist, MSO_FLD_PLAN_NAME, 1, ebufp );
        if (plan_name)
        {
            plan_detail_in_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, plan_detail_in_flistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_PUT(plan_detail_in_flistp, PIN_FLD_NAME, plan_name, ebufp);

            fm_mso_search_plan_detail(ctxp, plan_detail_in_flistp, &plan_detail_out_flistp, ebufp);

            if (!plan_detail_out_flistp)
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"PLAN DETAILS NOT AVAILABLE");
			    r_flistp = PIN_FLIST_CREATE(ebufp);
			    PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
			    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51429", ebufp);
			    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - plan provided not available", ebufp);
			
			    *ret_flistp = r_flistp;
			    return 3;
            }
      	    planlists_flistp = PIN_FLIST_ELEM_ADD(in_flist, PIN_FLD_PLAN_LISTS, 0, ebufp);
            PIN_FLIST_FLD_COPY(plan_detail_out_flistp, PIN_FLD_POID, planlists_flistp, PIN_FLD_PLAN_OBJ, ebufp);
        }
	}
	plan_flistp = PIN_FLIST_CREATE(ebufp);

	hundred = pbo_decimal_from_str("90.0",ebufp);
	thousand = pbo_decimal_from_str("1000.0",ebufp);
	while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, &elem_base, 1, &cookie_base, ebufp )) != NULL)
	{	
		plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);		
		if (!service_obj || !plan_obj)
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - Service object or plan object is missing in input", ebufp);
			
			*ret_flistp = r_flistp;
			return 3;
		}
                //New Tariff - Transaction mapping added the below line
                if(lifecycle_flistp != NULL) {
                        lc_plan_list_flistp = PIN_FLIST_ELEM_ADD(lifecycle_flistp, PIN_FLD_PLAN_LISTS, elem_base, ebufp);
		        PIN_FLIST_FLD_COPY(planlists_flistp, PIN_FLD_PLAN_OBJ, lc_plan_list_flistp, PIN_FLD_PLAN_OBJ, ebufp);
			offer_count = 0;
                }

		db = PIN_POID_GET_DB(plan_obj);

		readplan_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_POID, plan_obj, ebufp );
		PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_NAME, NULL, ebufp );
		PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_CODE, NULL, ebufp );
		deals_flistp = PIN_FLIST_ELEM_ADD(readplan_inflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(deals_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal read plan input list", readplan_inflistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readplan_inflistp, &readplan_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&readplan_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);

			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - PCM_OP_READ_FLDS of plan poid error", ebufp);
			
			*ret_flistp = r_flistp;
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal read plan output flist", readplan_outflistp);


		/***************************************************************************
		* Validation for not allowing to cancel 
		****************************************************************************/
		plan_name = (char*)PIN_FLIST_FLD_GET(readplan_outflistp, PIN_FLD_NAME, 1, ebufp);
		PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_CODE, planlists_flistp, PIN_FLD_CODE, ebufp );
		if (plan_name && strstr(plan_name, "STB Zero Value" ))
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "STB Zero Value/STB Zero Value ADV Plans can not be cancelled", ebufp);
			
			*ret_flistp = r_flistp;
			return 0;

		}

		args_flistp = PIN_FLIST_ELEM_ADD(plan_flistp, PIN_FLD_PLAN, elem_base, ebufp);
		PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_NAME, args_flistp, PIN_FLD_NAME, ebufp);

		elem_id = 0;
		cookie = NULL;

		while ((deals_flistp = PIN_FLIST_ELEM_GET_NEXT(readplan_outflistp, PIN_FLD_SERVICES, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			canceldeal_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, canceldeal_inflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, canceldeal_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, canceldeal_inflistp, PIN_FLD_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, canceldeal_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
			dealinfo_flistp = PIN_FLIST_ELEM_ADD(canceldeal_inflistp, PIN_FLD_DEAL_INFO, 0, ebufp );
			PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, dealinfo_flistp, PIN_FLD_DEAL_OBJ, ebufp);
			//PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_PACKAGE_ID, dealinfo_flistp, PIN_FLD_PACKAGE_ID, ebufp);

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
			if ( !package_id || (package_id && *(int32 *)package_id == 0))
			{
				package_id = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PACKAGE_ID, 1, ebufp);
			}

            if (package_id)
            {
                pkg_id = *(int32 *)package_id;
            }
			// Added below function to validate the package id
			ret_val = fm_mso_validate_pkg_id(ctxp, service_obj, read_deal_obj , pkg_id, ebufp);
			if(ret_val==1)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in sarching purchased product with pkg id");
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51652", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in sarching purchased product with pkg id", ebufp);
				*ret_flistp = r_flistp;
				return 0;
			} 
			if (ret_val==1 && (!package_id || (package_id && *(int32 *)package_id == 0)))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_CANCEL_DEAL", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);

				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51641", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Package id is not sent or zero in input flist", ebufp);
				
				*ret_flistp = r_flistp;
				return 0;
			}
			// Added below function to validate the package id
            if (ret_val > 1 && !package_id)
            {
                PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_PACKAGE_ID, &ret_val, ebufp);
                pkg_id = ret_val;
            }
            else
            {
			    PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_PACKAGE_ID, package_id, ebufp);
            }
	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal input list", canceldeal_inflistp);
			PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_CANCEL_DEAL, flag, canceldeal_inflistp, &canceldeal_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&canceldeal_inflistp, NULL);
			
			switch (ebufp->pin_err) {
				case 100002:
				case 100004:
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "lco error set");
					lco_err_code = ebufp->pin_err;
					break;
				default: 
					lco_err_code = 0;
			}
			
			if (lco_err_code != 0) {
				PIN_ERRBUF_RESET(ebufp);
				lco_err_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_POID,lco_err_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "lco error flistp", lco_err_flistp);
			}

			if (PIN_ERRBUF_IS_ERR(ebufp) || lco_err_code != 0 )
			{
				/*LCO Error Handling Start*/
                                if (lco_err_code != 0) {
                                	fm_mso_commission_error_processing(ctxp, lco_err_code,
                                		"Error in fm_mso_cust_activate_customer", lco_err_flistp, &r_flistp, ebufp);
                                }
                                /*LCO Error Handling End*/
				else {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_CANCEL_DEAL", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);

				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - PCM_OP_SUBSCRIPTION_CANCEL_DEAL error", ebufp);
				}
				*ret_flistp = r_flistp;
				return 0;
			}
			PIN_FLIST_DESTROY_EX(&lco_err_flistp, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal output flist", canceldeal_outflistp);
			// To return the refund amount when this opcode is called in CALC_ONLY mode			
			if ((service_obj && strcmp(PIN_POID_GET_TYPE(service_obj),"/service/catv") == 0) && flag == PCM_OPFLG_CALC_ONLY )
			{
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				c_elem_id = 0;
				c_cookie = NULL;
				while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT( canceldeal_outflistp,																PIN_FLD_RESULTS, &c_elem_id, 1, &c_cookie, ebufp )) != NULL ){
					event_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 1, ebufp);
					if ( strstr(PIN_POID_GET_TYPE(event_pdp),"/event/billing/product/fee/cycle")){
						total_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_TOTAL, 356, 1, ebufp);
						ref_amt = PIN_FLIST_FLD_GET(total_flistp, PIN_FLD_AMOUNT, 1, ebufp);
						//PIN_FLIST_FLD_COPY(total_flistp, PIN_FLD_AMOUNT, r_flistp, PIN_FLD_AMOUNT, ebufp );
					}
				}
				if ( ref_amt ) {
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_AMOUNT, ref_amt, ebufp);
				}
				else{//if no refund, set to zero
					PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_AMOUNT, pbo_decimal_from_str("0", ebufp), ebufp);
				}
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal CALCONLY ouput", r_flistp);
				*ret_flistp = r_flistp;
				PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
				return 2;
				
			}
			PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);

			/* If Cancel plan if called from Deactivate service then Goto SKIP */
			prog_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACTION_NAME, 1, ebufp);
			if(prog_name && strcmp(prog_name,"deactivate_bb_service") == 0 )
			{
				goto SKIP;
			}

			if ( bb_service == 1 )
			{
				bp_flag=0;
				user_id = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_USERID, 1, ebufp);	
				update_status = fm_mso_bb_update_status( ctxp, account_obj,service_obj, plan_obj,&mso_pp_obj,&bp_flag, prog_name, user_id, ebufp);
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"MSO_PP_POID",mso_pp_obj);
				if ( update_status == 1 )
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "Error in calling PCM_OP_READ_FLDS", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);
		
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51647", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in updating cancel plan status for service", ebufp);
					
					*ret_flistp = r_flistp;
					return 0;
				}
				/* If cancelled plan is NOT a base plan (bp_flag=0)then provisioning 
						action is not required.*/
				if(bp_flag == 0)
				{
					goto SKIP;
				}
			}
			/* If Cancel plan if called from BB TOPUP then provisioning call
				is not required. Goto SKIP */
			if((prog_name && strcmp(prog_name,"top_up_plan") == 0 ) ||
			(prog_name && strcmp(prog_name,"deactivate_bb_service") == 0 ))
			{
				goto SKIP;
			}
			/*************************** Searching purchased product *************************************/
			flags = 768;

			search_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &flags, ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, args_flistp, PIN_FLD_DEAL_OBJ, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp );
			//PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_PACKAGE_ID, args_flistp, PIN_FLD_PACKAGE_ID, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PACKAGE_ID, &pkg_id, ebufp);

			results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp );
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal search purchased product input list", search_inflistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - Error in fetching PIN_FLD_OFFERING_OBJ from return flist of PCM_OP_SEARCH", ebufp);
				
				*ret_flistp = r_flistp;
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal search purchased product output flist", search_outflistp);
		    fm_mso_get_account_info(ctxp, (poid_t *)account_obj, &acnt_info, ebufp);
            vp = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
            if(vp)
            {
                bus_type = *(int32 *)vp;
            }
            PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
            acnt_info = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_SET(acnt_info, PIN_FLD_POID, service_obj, ebufp);
            fm_mso_get_srvc_info(ctxp, acnt_info, &serv_flistp, ebufp);
            vp = PIN_FLIST_FLD_GET(serv_flistp, PIN_FLD_STATUS, 1, ebufp );
            if(vp)
            {
                flag1 = *(int32 *)vp;
            }
            sus_date = PIN_FLIST_FLD_GET(serv_flistp, PIN_FLD_LAST_STATUS_T, 1, ebufp);
	
			elem_pp = 0;
			cookie_pp = NULL;
            pvt = fm_utils_time_round_to_midnight(pin_virtual_time((time_t *)NULL));			
			while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_pp, 1, &cookie_pp, ebufp )) != NULL)
			{
				product_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ , 0 , ebufp);
                product_end_tp = (time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PURCHASE_END_T , 1, ebufp);
                if(bus_type && flag1 && bus_type != 0 && (bus_type == 99481100) && (flag1 == 10102))
                {

                        // searching for event details
                        event_local_flistp1 = PIN_FLIST_CREATE(ebufp);
                        flags = 256;
                        PIN_FLIST_FLD_SET(event_local_flistp1, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
                        PIN_FLIST_FLD_SET(event_local_flistp1, PIN_FLD_FLAGS, &flags, ebufp);
                        PIN_FLIST_FLD_SET(event_local_flistp1, PIN_FLD_TEMPLATE, search_template1, ebufp);
                        args_flistp = PIN_FLIST_ELEM_ADD(event_local_flistp1, PIN_FLD_ARGS, 1, ebufp );
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
                        args_flistp1 = PIN_FLIST_ELEM_ADD(event_local_flistp1, PIN_FLD_ARGS, 2, ebufp );
                        PIN_FLIST_FLD_SET(args_flistp1, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
                        args_flistp2 = PIN_FLIST_ELEM_ADD(event_local_flistp1, PIN_FLD_ARGS, 3, ebufp );
                        bal_impt2 = PIN_FLIST_ELEM_ADD(args_flistp2, PIN_FLD_BAL_IMPACTS, 1, ebufp );
                        PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, bal_impt2, PIN_FLD_OFFERING_OBJ, ebufp);
                        args_flistp3 = PIN_FLIST_ELEM_ADD(event_local_flistp1, PIN_FLD_ARGS, 4, ebufp );
                        PIN_FLIST_FLD_SET(args_flistp3, PIN_FLD_EARNED_END_T, sus_date, ebufp);
                        results_flistp1 = PIN_FLIST_ELEM_ADD(event_local_flistp1, PIN_FLD_RESULTS, 0, ebufp );
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal search event input flist", event_local_flistp1);
                        PCM_OP(ctxp, PCM_OP_SEARCH, 0, event_local_flistp1, &search_outflistp1, ebufp);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
                            PIN_ERRBUF_RESET(ebufp);
                            PIN_FLIST_DESTROY_EX(&search_outflistp1, NULL);
                        }
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal search event output flist", search_outflistp1);
                        if(search_outflistp1)
                        {
                            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "refund opcode");
                            results_flistp2 = PIN_FLIST_ELEM_GET(search_outflistp1, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                            if(results_flistp2)
                            {
                                results_flistp3 = PIN_FLIST_ELEM_GET(results_flistp2, PIN_FLD_SUB_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp);
                            }
                            if(results_flistp3)
                            {
                                results_flistp4 = PIN_FLIST_ELEM_GET(results_flistp3, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, 1, ebufp);
                            }
                            if(results_flistp4)
                            {
                                charge_amount_1 = pbo_decimal_from_str("0.0", ebufp);
                                charge_amount_1 = PIN_FLIST_FLD_GET(results_flistp4, PIN_FLD_AMOUNT, 1, ebufp);
                                charge_amount = pbo_decimal_add(charge_amount, charge_amount_1, ebufp);
                            }
                            start_tp = (time_t *) PIN_FLIST_FLD_GET(results_flistp2, PIN_FLD_EARNED_START_T, 1, ebufp);
                            end_tp = (time_t *)PIN_FLIST_FLD_GET(results_flistp2, PIN_FLD_EARNED_END_T, 1, ebufp);
                            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan amount");
                            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(charge_amount, ebufp));
                            results_flistp4 = NULL;
                        }
                }

				if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV) == 0) && is_base_pack != 0)
				{
					is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, product_obj, ebufp);
				}
                                //New Tariff - Transaction mapping Start */
                                if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV) == 0))
				{
                                         eflistp = PIN_FLIST_ELEM_ADD(lc_plan_list_flistp, PIN_FLD_OFFERINGS, offer_count++, ebufp);
                                         PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, eflistp, PIN_FLD_OFFERING_OBJ, ebufp);
                                         PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_PACKAGE_ID, eflistp, PIN_FLD_PACKAGE_ID, ebufp);
				}
                                //New Tariff - Transaction mapping End */

				/***** read the product to get the priority *****/
				read_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_POID, product_obj , ebufp);
				PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_PRIORITY , (void *) NULL , ebufp);
				PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal Read product input flist", read_iflist);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_iflist, &read_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&read_iflist, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal Read product output flist", read_oflist);
				priority = PIN_FLIST_FLD_GET(read_oflist,PIN_FLD_PRIORITY , 0, ebufp);
				prov_tag = PIN_FLIST_FLD_GET(read_oflist, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
				//Check to add provisioning only for products having provisioning tag 
				if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
                                {
					if(strcmp(prov_tag, "")!=0)
					{
						add_prov = fm_validate_prov_ca_id(ctxp, service_obj, prov_tag, ebufp);
						if(!add_prov || add_prov == 2)
						{
							hardware =1;
						}

						/*********************************
						 * Base pack product
						 ********************************/
						if (add_prov == 2)
						{
							add_prov = 0;
							PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PKG_TYPE, &add_prov, ebufp);
						}

						if (add_prov == 0)
						{
							hardware = 0;
						}
					}
					else
					{
						hardware = 1;
					}
				}

				if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
			        {
					if ( pbo_decimal_compare(priority, thousand, ebufp) == -1 )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " this is a cancel of Hardware product");
						hardware = 1;
					} 			
				}
				else
				{
					if (pbo_decimal_compare(priority, hundred, ebufp)==-1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " this is a cancel of Hardware product");
						hardware = 1;
					}
				}
				status_flags_new = PIN_FLIST_FLD_GET (results_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp);
				if ( status_flags_new && *status_flags_new != 0xFFF && hardware == 0 )
				{
					args_flistp = PIN_FLIST_ELEM_ADD(provaction_inflistp, PIN_FLD_PRODUCTS, cnt++, ebufp );
					PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);
				}
				hardware = 0;
				PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
			}			
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		}

		PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);
	}
    if(sus_date && end_tp && start_tp && charge_amount && *end_tp <= pvt)
     {

        //for refund
        *(time_t *)sus_date = fm_utils_time_round_to_midnight(*(time_t *)sus_date);
        if(end_tp && product_end_tp && (*end_tp < *product_end_tp))
        {
           extra_days = (int)(*product_end_tp - *end_tp)/86400;

        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(charge_amount, ebufp));
        days = (int)((*end_tp - *sus_date) - 86400)/86400;
        plan_days = (int)(*end_tp - *start_tp)/86400;
        plandays = plan_days;
        sprintf(msg, "days: %d", days);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
        extra_charge = extra_days;
        sprintf(msg, "plan_days: %d", plan_days);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
        charge_days = days;
        charge_days = (double)(extra_charge + charge_days);
        sprintf(msg, "extra_days: %d", extra_days);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
        charge_amount = pbo_decimal_divide(charge_amount, pbo_decimal_from_double(plandays, ebufp), ebufp);
        refund_amtp = pbo_decimal_multiply(charge_amount,
                                  pbo_decimal_from_double(charge_days, ebufp), ebufp);
        refund_amtp = pbo_decimal_multiply(refund_amtp, neg_amountp, ebufp);
        update_serv_ipflist = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(update_serv_ipflist, PIN_FLD_POID, account_obj, ebufp);
        evt_pdp = PIN_POID_CREATE(db, "/event/billing/credit_charges", -1, ebufp);
        event_ipflist = PIN_FLIST_SUBSTR_ADD(update_serv_ipflist, PIN_FLD_EVENT, ebufp);
        PIN_FLIST_FLD_PUT(event_ipflist, PIN_FLD_POID, evt_pdp, ebufp);
        PIN_FLIST_FLD_SET(event_ipflist, PIN_FLD_PROGRAM_NAME, "CREDIT CHARGES", ebufp);
        PIN_FLIST_FLD_SET(event_ipflist, PIN_FLD_DESCR, "Credit Charges", ebufp);
        PIN_FLIST_FLD_SET(event_ipflist, PIN_FLD_NAME, "NCR", ebufp);
        PIN_FLIST_FLD_SET(event_ipflist, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
        PIN_FLIST_FLD_SET(event_ipflist, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
        PIN_FLIST_FLD_SET(event_ipflist, PIN_FLD_SYS_DESCR, "Billing time: Credit Charges", ebufp);
        PIN_FLIST_FLD_SET(event_ipflist, PIN_FLD_START_T, &pvt, ebufp);
        PIN_FLIST_FLD_SET(event_ipflist, PIN_FLD_END_T, &pvt, ebufp);
        PIN_FLIST_FLD_SET(event_ipflist, PIN_FLD_EFFECTIVE_T, &pvt, ebufp);
        total_amtp = PIN_FLIST_ELEM_ADD(event_ipflist, PIN_FLD_TOTAL, 356, ebufp);
        PIN_FLIST_FLD_SET(total_amtp, PIN_FLD_AMOUNT, refund_amtp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Credit Cycle Charges input flist", update_serv_ipflist);
        PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, update_serv_ipflist, &update_serv_opflist, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Credit Cycle Charges output flist", update_serv_ipflist);

     }

	pbo_decimal_destroy(&hundred);
	pbo_decimal_destroy(&thousand);
	/* Write USERID, PROGRAM_NAME in buffer - start */
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_DESCR,provaction_inflistp,PIN_FLD_DESCR,ebufp);
	//Added if condition to add MSO_FLD_PURCHASED_PLAN_OBJ only for Broadband service.
	if(bb_service) {
		PIN_FLIST_FLD_PUT(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_pp_obj,ebufp);
	}
	/* Write USERID, PROGRAM_NAME in buffer - end */

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal provisioning input list", provaction_inflistp);
	count = PIN_FLIST_ELEM_COUNT(provaction_inflistp, PIN_FLD_PRODUCTS , ebufp);
	if (count == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Products hence no provisioning action ");
		goto SKIP;
	}
	else if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
        {
		comp_flag = fm_mso_compare_ca_ids(ctxp, in_flist,ret_flistp, ebufp);
		if (!comp_flag)
		{
			PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No  Need for Provisioning");
			goto SKIP;
		}
	}
	else
	{
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);	
	}
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}

	if(provaction_outflistp)
	{
		prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
		if(prov_call && (*prov_call == 1))
		{
			*ret_flistp = PIN_FLIST_COPY(provaction_outflistp, ebufp);
			return 0;
		}
	}
	else
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51751", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning action.", ebufp);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

	if (is_base_pack == 0)
	{
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, in_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);
		mso_cancel_alc_addon_pdts(ctxp, in_flist, ebufp);
	}
	/******************************************* notification flist ***********************************************/
/*	flags = 12;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &flags, ebufp);

	PIN_FLIST_CONCAT(provaction_inflistp, plan_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal provisioning input list", provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - MSO_OP_NTF_CREATE_NOTIFICATION error", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}

	if(provaction_outflistp)
	{
		prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
		if(prov_call && (*prov_call == 1))
		{
			*ret_flistp = PIN_FLIST_COPY(provaction_outflistp, ebufp);
			return 0;
		}
	}
	else
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &cancel_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51752", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning notification.", ebufp);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cancel_deal provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
*/
	PIN_FLIST_DESTROY_EX(&plan_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&plan_detail_in_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&plan_detail_out_flistp, NULL);

	/**************************************************************************
	* Create Output flist - Start
	**************************************************************************/
SKIP:	
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &cancel_plan_success, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "ACCOUNT - Service cancel plan completed successfully", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Cancel plan output flist:",r_flistp);	
	*ret_flistp = r_flistp;
	
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
        if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
        {
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Cancel plan lifecycle_flistp:", lifecycle_flistp);
        	fm_mso_create_lifecycle_evt(ctxp, in_flist, lifecycle_flistp, ID_REMOVE_PLAN, ebufp);
	}
        PIN_FLIST_DESTROY_EX(&lifecycle_flistp, NULL);

	return 2;
}

static int 
fm_mso_bb_update_status(
	pcm_context_t	*ctxp,
	poid_t 		*account_obj,
	poid_t 		*service_obj,
	poid_t		*plan_obj,
	poid_t		**mso_plan_obj,
	int		*bp_flag,
	char		*prog_name,
	pin_flist_t	*user_id,
	pin_errbuf_t	*ebufp) 
{
	
	pin_flist_t		*writeflds_flistp = NULL;	
	pin_flist_t		*writeflds_outflistp = NULL;	
	pin_flist_t		*readsvc_inflistp = NULL;	
	pin_flist_t		*readsvc_outflistp = NULL;	
	pin_flist_t		*srch_flistp = NULL;	
	pin_flist_t		*srch_out_flistp = NULL;	
	pin_flist_t		*cust_srch_flistp = NULL;	
	pin_flist_t		*cust_srch_out_flistp = NULL;	
	pin_flist_t		*args_flistp = NULL;	
	pin_flist_t		*results_flistp = NULL;	 
	pin_flist_t		*ncr_validity_oflist = NULL;

	/****** Pavan Bellala -08/07/2015 ******************************
	Disassociate device changes 
	****** Pavan Bellala - 08/07/2015  ****************************/
	pin_flist_t		*do_flistp = NULL;	
	pin_flist_t		*del_flistp = NULL;	
	pin_flist_t		*tmp_flistp = NULL;
	//char			*device_template  = "select X from /device where F1 = V1 and F2 = V2 and F3 = V3 ";
	char                  *device_template  = "select X from /device where F1 = V1 and F2 = V2 ";
	int			rv = 0;
	poid_t			*device_pdp  = NULL;


	int32			*status = NULL;
	int32			plan_status = 0;
	int32			db = 0;
	int32			*customer_type = NULL;
	int32			srch_flag = 512;
	int			subsc_type = 0;
	int			prty = 0;
	int			tmp = 0;
	char			msg[100];
	char			*search_template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3";
	//char                  *search_template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
	char			*search_template2 = "select X from /mso_customer_credential where F1 = V1 ";
	char			*tmp_str = NULL ;
	char			*plan_type = NULL;
	char			*descr;
	//char			*prog_name = NULL;
	pin_decimal_t		*plan_priority = NULL;
	pin_flist_t		*dev_flistp = NULL;
	pin_cookie_t            cookie_amc = NULL;
	int                     elem_amc = 0;	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Entered into fm_mso_bb_update_status function");
		plan_status = 2;
        db = PIN_POID_GET_DB(service_obj);
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &plan_status, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
        
	results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PRIORITY, (int32 *)NULL, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_DESCR, (char *)NULL, ebufp);
        
	args_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "product type search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp) )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching /mso_purchased_plans PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			return 1;
		}	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "product type search output list", srch_out_flistp);
		results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if ( !results_flistp )
        {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "This plan does not exist for the customer", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			return 1;
		}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_PP results list", results_flistp);
	plan_type = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_DESCR, 1, ebufp);

	plan_priority = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRIORITY, 1, ebufp);
	tmp_str = pbo_decimal_to_str(plan_priority, ebufp);
        prty = atoi(tmp_str);
        sprintf(msg,"Plan priority is : %d",prty);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	/*******************************************************
	 Update NON currecy resource validity -start
	*******************************************************/
	//Added OR condition below to avoid changing validity for hardware plan cancellation
	if (prog_name && strcmp(prog_name, "top_up_plan") !=0 
		&& prty >=BB_SUBS_PRI_START)
	{
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PROGRAM_NAME, "set_validity_by_cancel_plan", ebufp);

		fm_mso_update_ncr_validty(ctxp, results_flistp, &ncr_validity_oflist, ebufp);
		if (ncr_validity_oflist)
		{
			descr = PIN_FLIST_FLD_GET(ncr_validity_oflist, PIN_FLD_DESCR, 1, ebufp);
			if (descr && strcmp(descr ,"success") != 0)
			{
				return 0;
			}
		}
	}
 	/*******************************************************
	 Update NON currecy resource validity -start
	*******************************************************/



	/*IF cancelled plan is Base plan then set bp_flag to 1*/
	if(plan_type && strcmp(plan_type,"base plan")==0) {
		*bp_flag = 1;
	} else {
		*bp_flag = 0;
	}
	*mso_plan_obj = PIN_FLIST_FLD_TAKE(results_flistp, PIN_FLD_POID, 1, ebufp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"MSO_PP_POID",*mso_plan_obj);
        cust_srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(cust_srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(cust_srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(cust_srch_flistp, PIN_FLD_TEMPLATE, search_template2, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(cust_srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(cust_srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_CUSTOMER_TYPE, (int32 *)NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer type search input list", cust_srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, cust_srch_flistp, &cust_srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&cust_srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp) || !srch_out_flistp )
        {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&cust_srch_out_flistp, NULL);
			return 1;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer type search output flist", cust_srch_out_flistp);
        results_flistp = PIN_FLIST_ELEM_GET(cust_srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        customer_type = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CUSTOMER_TYPE, 1, ebufp);
        if ( !customer_type || customer_type == (int32 *)NULL )
        {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Customer type is not updated for this account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&cust_srch_out_flistp, NULL);		
		return 1;
	}

	/* Check the priority to find if the cancalled plan is subscription plan*/
        if ( *(int32 *)customer_type == ACCT_TYPE_CORP_SUBS_BB )
        {
                if ( ( prty > BB_COR_PREPAID_START && prty < BB_COR_PREPAID_END ) || 
			( prty > BB_COR_POSTPAID_START && prty < BB_COR_POSTPAID_END ) )
                {
                        subsc_type = 1;
                }
        }
	/**********
        else if ( *(int32 *)customer_type == ACCT_TYPE_CYBER_CAFE_BB )
        {
                if ( ( prty > BB_CYB_PREPAID_START && prty < BB_CYB_PREPAID_END ) &&
			( prty > BB_CYB_POSTPAID_START && prty < BB_CYB_POSTPAID_END ))
                {
                        subsc_type = 1;
                }
        }
	**********/
        else if ( *(int32 *)customer_type == ACCT_TYPE_RETAIL_BB )
        {
                if ( ( prty > BB_RET_PREPAID_START && prty < BB_RET_PREPAID_END ) ||
			( prty > BB_RET_POSTPAID_START && prty < BB_RET_POSTPAID_END ))
                {
                        subsc_type = 1;
                }
        }
        else if ( *(int32 *)customer_type == ACCT_TYPE_SME_SUBS_BB )
        {
                if ( ( prty > BB_SME_PREPAID_START && prty < BB_SME_PREPAID_END ) ||
			(  prty > BB_SME_POSTPAID_START && prty < BB_SME_POSTPAID_END ))
                {
                        subsc_type = 1;
                }
        }
		/*If plan type is subscription then update the MSO Purchased plan status 
			to MSO_PROV_IN_PROGRESS else update the status to MSO_CANCEL_STATUS*/
		//prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
		
		// Commented the below part to set the status to Terminated always since
		// the status of cancelled plan is not updated by prov response.
		/*if(subsc_type == 1 && prog_name && strcmp(prog_name,"top_up_plan") != 0) {
			tmp = MSO_PROV_IN_PROGRESS;	
		} else { */
			//tmp = MSO_PROV_STATE_TERMINATED;
		//}

	if(prty == BB_HW_CABLE_MODEM_RENT_PRIO || prty == BB_HW_DCM_RENT_PRIO)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Removing AMC Hardware Plan");
		fm_mso_cust_cancel_amc_plan(ctxp, account_obj, service_obj, user_id, mso_plan_obj, ebufp);
	
	 	if (PIN_ERRBUF_IS_ERR(ebufp))
	 	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling AMC opcode for AMC product cancellation", ebufp);
	 	}
	}	
	tmp = MSO_PROV_STATE_TERMINATED;
		
	writeflds_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(writeflds_flistp, PIN_FLD_POID, *mso_plan_obj, ebufp);
        PIN_FLIST_FLD_SET(writeflds_flistp, PIN_FLD_STATUS, &tmp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_update_status write service flds input list", writeflds_flistp);
        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, writeflds_flistp, &writeflds_outflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&writeflds_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
                PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&cust_srch_out_flistp, NULL);		
                PIN_FLIST_DESTROY_EX(&writeflds_outflistp, NULL);
                return 1;
        }
        PIN_FLIST_DESTROY_EX(&writeflds_outflistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_update_status write service flds output list", writeflds_outflistp);

	readsvc_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_POID, service_obj, ebufp);
        PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_STATUS, NULL, ebufp);
        PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_CREATED_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_update_status read service input list", readsvc_inflistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readsvc_inflistp, &readsvc_outflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&readsvc_inflistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&cust_srch_out_flistp, NULL);		
		PIN_FLIST_DESTROY_EX(&writeflds_outflistp, NULL);
		return 1;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_update_status read service output flist", readsvc_outflistp);

	args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp);
	status = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATUS, 1, ebufp);
	if ( status && *(int32 *)status != 2 && *(int32 *)status != 1 )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "Error in provisioning status of service ", ebufp);
		return 1;
	}
	
	if ( status && subsc_type == 1 )
	{
		tmp = MSO_PROV_IN_PROGRESS;	
		writeflds_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(writeflds_flistp, PIN_FLD_POID, service_obj, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(writeflds_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &tmp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_update_status write service flds input list", writeflds_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, writeflds_flistp, &writeflds_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&writeflds_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating service provisioning status ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&cust_srch_out_flistp, NULL);		
			PIN_FLIST_DESTROY_EX(&writeflds_outflistp, NULL);
			return 1;
		}
		PIN_FLIST_DESTROY_EX(&writeflds_outflistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_update_status write service flds output list", writeflds_outflistp);
	}

	/****** Pavan Bellala -08/07/2015 ******************************
	Changes added: If the cancelled plan is Hardware plan, then
	the associated device should also be removed from /device class
	****** Pavan Bellala - 08/07/2015  ****************************/
	if( prty >= BB_HW_PRI_START && prty <= BB_HW_PRI_END )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Hardware plan cancelled. Disassociate device");
        	srch_flistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, device_template, ebufp);
        	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_SERVICES,0,ebufp);
        	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ,account_obj , ebufp);
        	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_SERVICES,0,ebufp);
        	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ,service_obj , ebufp);
        	//args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		//tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN,0,ebufp);
        	//PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_PURCHASED_PLAN_OBJ,*mso_plan_obj, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,ebufp);
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_update_status:input for device search",srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, srch_flistp, &do_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_update_status:output for device search",do_flistp);
		
		PIN_FLIST_DESTROY_EX(&srch_flistp,NULL);
		if(do_flistp && PIN_FLIST_ELEM_COUNT(do_flistp,PIN_FLD_RESULTS,ebufp)>0)
		{
			args_flistp = PIN_FLIST_ELEM_GET(do_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
			if(args_flistp)
			{
				//if((tmp_flistp = PIN_FLIST_ELEM_GET(args_flistp,PIN_FLD_PLAN,0,1,ebufp))!= NULL)
				srch_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(args_flistp,PIN_FLD_POID,srch_flistp,PIN_FLD_POID,ebufp);
				while ((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(args_flistp, PIN_FLD_PLAN, &elem_amc, 1, &cookie_amc, ebufp )) != NULL)
				{
				  if(dev_flistp)
				   {
					//srch_flistp = PIN_FLIST_CREATE(ebufp);
					//PIN_FLIST_FLD_COPY(args_flistp,PIN_FLD_POID,srch_flistp,PIN_FLD_POID,ebufp);
					//PIN_FLIST_ELEM_SET(srch_flistp,tmp_flistp,PIN_FLD_PLAN,0,ebufp);
					PIN_FLIST_ELEM_SET(srch_flistp,dev_flistp,PIN_FLD_PLAN, elem_amc,ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_update_status:drop plan object in IN LOOP",srch_flistp);
				   }
				 }
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_update_status:drop plan object in",srch_flistp);
					PCM_OP(ctxp, PCM_OP_DELETE_FLDS,0,srch_flistp,&del_flistp,ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_update_status:drop plan object out ",del_flistp);
					PIN_FLIST_DESTROY_EX(&srch_flistp,NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in disassociating device", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&do_flistp,NULL);
						PIN_FLIST_DESTROY_EX(&del_flistp,NULL);
						return 1;
					}
			
					/****************************************************
					Check and refund deposit
					***************************************************/
					device_pdp = PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_POID,1,ebufp);
					if(!PIN_POID_IS_NULL(device_pdp))
					{
						rv = mso_cancel_plan_refund_hw_deposit(ctxp,account_obj,service_obj,device_pdp,ebufp);				
						if(PIN_ERRBUF_IS_ERR(ebufp)|| rv !=0)
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in disassociating device", ebufp);
							PIN_ERRBUF_RESET(ebufp);
							PIN_FLIST_DESTROY_EX(&do_flistp,NULL);
                                               		PIN_FLIST_DESTROY_EX(&del_flistp,NULL);
                                                	return 1;
                                        	}
							
					}
				     
				
			}
	
		}
		PIN_FLIST_DESTROY_EX(&do_flistp,NULL);
		PIN_FLIST_DESTROY_EX(&del_flistp,NULL);
		
	}

	PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&cust_srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&writeflds_outflistp, NULL);
	return 2;
}

static int 
fm_mso_validate_pkg_id(
	pcm_context_t	*ctxp,
	poid_t		*svc_obj,
	poid_t		*deal_obj,
	int32		pkg_id,
	pin_errbuf_t	*ebufp)
{
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	pin_flist_t		*srch_flistp = NULL;	
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	int32			db = 0;
	int32			srch_flag = 256;
	int			subsc_type = 0;
	int			status_active = 1;
	char			*search_template = "select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 ";

        db = PIN_POID_GET_DB(svc_obj);
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEAL_OBJ, deal_obj, ebufp);

        if (pkg_id)
        {
            args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
            PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PACKAGE_ID, &pkg_id, ebufp);
        }
        else
        {
            args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
            PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status_active, ebufp);
        }

	    args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, svc_obj, ebufp);

	    results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PACKAGE_ID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Purchased prod input fist", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Searching purchased product.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Purchased prod output fist", srch_out_flistp);
	if(!srch_out_flistp || (PIN_FLIST_ELEM_COUNT(srch_out_flistp , PIN_FLD_RESULTS , ebufp) == 0))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active Purchased product NOT found");
		return 1;
	}
    results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
    if (results_flistp)
    {
        pkg_id = *(int32 *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp);
        return pkg_id;
    }

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active Purchased product found");
	return 0;
}


int
mso_cancel_plan_refund_hw_deposit(
        pcm_context_t   *ctxp,
        poid_t          *account_pdp,
        poid_t          *service_pdp,
        poid_t          *device_pdp,
        pin_errbuf_t    *ebufp)
{

        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *results_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
	pin_flist_t		*refund_iflistp = NULL;
	pin_flist_t		*refund_oflistp = NULL;

        int32                   db = 0;
        int32                   srch_flag = 256;
        int                     rv = 0;
        char                    *search_template = "select X from /mso_deposit where F1 = V1 and F2 = V2 and F3 = V3 ";


        if (PIN_ERRBUF_IS_ERR(ebufp))
                return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_cancel_plan_refund_hw_deposit function call");

        db = PIN_POID_GET_DB(account_pdp);
        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_OBJ, device_pdp, ebufp);

        results_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search deposit input fist", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp) || (!srch_out_flistp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Searching deposit.", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                return 1;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search deposit output fist", srch_out_flistp);
        if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Deposit NOT found");
                return 0;
        } else {
		results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
		if(results_flistp){
			 refund_iflistp = PIN_FLIST_CREATE(ebufp);
			 PIN_FLIST_FLD_SET(refund_iflistp, PIN_FLD_POID, account_pdp, ebufp);
			 args_flistp = PIN_FLIST_ELEM_ADD(refund_iflistp, PIN_FLD_DEPOSITS, 0, ebufp);
			 PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
			 PIN_FLIST_FLD_COPY(results_flistp, MSO_FLD_DEPOSIT_NO,args_flistp, MSO_FLD_DEPOSIT_NO, ebufp);
			 PIN_FLIST_FLD_SET(refund_iflistp, PIN_FLD_PROGRAM_NAME,"Cancel HW Plan",ebufp);
			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				 "fm_mso_cust_cancel_plan deposit refund input list:", refund_iflistp);
			 PCM_OP(ctxp,MSO_OP_PYMT_DEPOSIT_REFUND,0 , refund_iflistp , &refund_oflistp , ebufp);
			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				 "fm_mso_cust_cancel_plan deposit refund output list:", refund_oflistp);			 
		 	 PIN_FLIST_DESTROY_EX(&refund_iflistp, NULL);
			 if (PIN_ERRBUF_IS_ERR(ebufp))
			 {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Searching deposit.", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&refund_oflistp, NULL);
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
				return 1;
			 }
			 PIN_FLIST_DESTROY_EX(&refund_oflistp, NULL);
		} else {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No results found");
			rv = 1;
		}	
	}
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return rv;

}

static void
fm_mso_cust_cancel_amc_plan(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        poid_t                  *service_obj,
        pin_flist_t             *user_id,
        poid_t                  **mso_plan_obj,
        pin_errbuf_t            *ebufp)

{

        pin_flist_t     *read_in_flistp = NULL;
        pin_flist_t     *read_out_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
	int32           amc_flag = AMC_ON_POSTPAID_TERMINATION;
        int32           mode = 0;
	int32           *indicator = NULL;
        pin_flist_t	*amc_flistp = NULL;
	pin_flist_t	*amc_out_flistp = NULL;
	 
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, service_obj, ebufp);        
	args_flistp = PIN_FLIST_SUBSTR_ADD(read_in_flistp, MSO_FLD_BB_INFO, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read service input", read_in_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read service output", read_out_flistp);
        args_flistp = PIN_FLIST_SUBSTR_GET(read_out_flistp, MSO_FLD_BB_INFO, 0, ebufp );
        indicator = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_INDICATOR, 0, ebufp);


        // AMC plan should only be associated for Postpaid
        if(!indicator || *indicator!=PAYINFO_BB_POSTPAID)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan is Not Postpaid. AMC not applicable.");
                PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
                return;
        }

        PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan is Postpaid.");
        amc_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_POID, account_obj, ebufp);
        PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
        PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_USERID, user_id, ebufp);
        PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_FLAGS, &amc_flag, ebufp);
        PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_MODE, &mode, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC input", amc_flistp);
        PCM_OP(ctxp, MSO_OP_CUST_BB_HW_AMC, 0, amc_flistp, &amc_out_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                  PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_BB_HW_AMC", ebufp);
                  PIN_ERRBUF_RESET(ebufp);
        }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC output", amc_out_flistp);
	PIN_FLIST_DESTROY_EX(&amc_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&amc_out_flistp, NULL);
        return;
}

static	int
fm_mso_compare_ca_ids(
	pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*from_plan_flistp = NULL;
	pin_flist_t		*to_plan_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*flistp_ca_id = NULL;
	pin_flist_t		*old_flistp = NULL;
	pin_flist_t		*old_flistp_ca_id = NULL;
	pin_flist_t 		*read_o_flistp = NULL;
	pin_flist_t 		*read_flistp = NULL;
	pin_flist_t 		*r_flistp = NULL;
	pin_flist_t 		*rflistp = NULL;
	pin_flist_t 		*pp_out_flistp = NULL;
	
	void			*vp = NULL;
	char			msg[1024];
	char			func_name[] = "fm_mso_compare_ca_ids";
	char			*network_node = NULL;
	char			*new_network_node = NULL;
	char			*ca_id = NULL;
	char			*old_ca_id = NULL;
	char			*node = NULL;

	int32			compare_flag = 0;
	int32			rec_id = 0;	
	int32			rec_id1 = 0;
	int32                   rec_id2 = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t            cookie2 = NULL;
	int32			old_prd_count = 0;
	int32			new_prd_count = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing");	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		goto CLEANUP;
	}
	sprintf(msg,"%s: Compare CA_IDs", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_compare_ca_ids input list", i_flistp);
	
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1 ,ebufp);
	if (vp){
		read_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, (poid_t *)vp, ebufp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_o_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			sprintf(msg,"%s: Error before processing");	
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
			goto CLEANUP;
		}
		if (read_o_flistp){
			rflistp = PIN_FLIST_SUBSTR_GET(read_o_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
			node = PIN_FLIST_FLD_GET(rflistp, MSO_FLD_NETWORK_NODE, 1 ,ebufp);
		}
			
	}
	
	//getting details of added plan
	tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS, PIN_ELEMID_ANY, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PLAN_OBJ, 1 ,ebufp);
	
	
	if (vp){
	    fm_mso_get_ca_id_from_plan(ctxp, i_flistp, (poid_t *)vp, node, &from_plan_flistp, ebufp);
	}
	
	sprintf(msg,"%s: Added Plan flist", func_name);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, from_plan_flistp);
		
	old_prd_count = PIN_FLIST_ELEM_COUNT(from_plan_flistp, PIN_FLD_RESULTS,ebufp);
	fm_mso_get_cust_active_plans (ctxp, i_flistp, &pp_out_flistp, ebufp);	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pp_out_flistp: ", pp_out_flistp);
	while ((r_flistp = PIN_FLIST_ELEM_GET_NEXT(pp_out_flistp,
	        PIN_FLD_RESULTS, &rec_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PP IN");
		vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		
		if  (vp)
			fm_mso_get_ca_id_from_plan(ctxp, i_flistp, (poid_t *)vp, node, &to_plan_flistp, ebufp);
			
		sprintf(msg,"%s: Active Plan flist", func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, to_plan_flistp);
		new_prd_count = PIN_FLIST_ELEM_COUNT(to_plan_flistp, PIN_FLD_RESULTS,ebufp);
	
		if (old_prd_count > 0 && new_prd_count >0 && old_prd_count >= new_prd_count)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 1");
			
			/********* VERIMATRIX CHANGES - Enhanced condition to check for Verimatrix network node ***/
			if ( node )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 1.1");
				rec_id = 0;
				cookie = NULL;
				while ((flistp = PIN_FLIST_ELEM_GET_NEXT(to_plan_flistp,
					PIN_FLD_RESULTS, &rec_id, 1, &cookie,
					ebufp)) != (pin_flist_t *)NULL)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 2");
					compare_flag = 0;
					flistp_ca_id = PIN_FLIST_ELEM_GET(flistp, MSO_FLD_CATV_PT_DATA, PIN_ELEMID_ANY, 0, ebufp);
					if (flistp_ca_id != NULL)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 3");
						ca_id = PIN_FLIST_FLD_GET(flistp_ca_id, MSO_FLD_CA_ID, 1 ,ebufp);
						sprintf(msg,"%s: ca_id", ca_id);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
						rec_id1 = 0;
						cookie1 = NULL;
						while ((old_flistp = PIN_FLIST_ELEM_GET_NEXT(from_plan_flistp,
							PIN_FLD_RESULTS, &rec_id1, 1, &cookie1,
							ebufp)) != (pin_flist_t *)NULL)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 4");
							old_flistp_ca_id = PIN_FLIST_ELEM_GET(old_flistp, MSO_FLD_CATV_PT_DATA, PIN_ELEMID_ANY, 0, ebufp);
							if (old_flistp_ca_id != NULL)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 5");
								old_ca_id = PIN_FLIST_FLD_GET(old_flistp_ca_id, MSO_FLD_CA_ID, 1 ,ebufp);
								sprintf(msg,"%s: old_ca_id", old_ca_id);
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
								sprintf(msg,"%s: new_ca_id", ca_id);
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
								/******************************************************
								 * NTO: Handle NA CA_ID
								 *****************************************************/
								if (strcmp(old_ca_id, "NA") != 0 && old_ca_id && ca_id && strcmp(ca_id,old_ca_id) == 0)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 6");
									compare_flag = 1;
									break;
								}
							}
						}
					}
					/*if (compare_flag == 1){
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 7");
						break;
					}*/
				}
			}
		}
		if (compare_flag == 1){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 8");
			break;
		}
	}		
	
	CLEANUP:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"I am in Cleanup");
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_o_flistp, NULL);
	return compare_flag;
}

static	void
fm_mso_get_ca_id_from_plan(
	pcm_context_t           *ctxp,
	pin_flist_t				*i_flistp,
	poid_t                  *mso_plan_obj,
	char                  	*mso_node,
	pin_flist_t				**ret_flistp,
	pin_errbuf_t            *ebufp)
{
	
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*tmp_flp = NULL;
	pin_flist_t		*out_flistp = NULL;
	poid_t			*pdp = NULL;
	
	char			msg[1024];
	int32			srch_flag = 768;
	char            template1 [] = "select X from /mso_cfg_catv_pt 1 , /product 2, /deal 3, /plan 4 where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = 4.F6 and 4.F7 = V7 and 1.F8 = V8 ";
	char			func_name[] = "fm_mso_get_ca_id_from_plan";
	int32			args_cnt = 1;
	int32			search_fail = 1;
	int64           db = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing");	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;
	}
	
	sprintf(msg,"%s: Prepare search flist to search CA_ID using PLAN", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1 ,ebufp);
	db = PIN_POID_GET_DB(pdp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);

    // Add ARGS array
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PROVISIONING_TAG,"",ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PROVISIONING_TAG,"",ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID,NULL,ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,PIN_FLD_PRODUCTS,PIN_ELEMID_ANY,ebufp);
	PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID,NULL,ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,PIN_FLD_SERVICES,PIN_ELEMID_ANY,ebufp);
	PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID, mso_plan_obj,ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,MSO_FLD_CATV_PT_DATA,PIN_ELEMID_ANY,ebufp);
	PIN_FLIST_FLD_SET(tmp_flp, MSO_FLD_NETWORK_NODE, mso_node, ebufp);

    	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

	sprintf(msg, "%s: PCM_OP_SEARCH input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, srch_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
			goto CLEANUP;
	}
	sprintf(msg,"%s: PCM_OP_SEARCH return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, srch_out_flistp);
	*ret_flistp = PIN_FLIST_COPY(srch_out_flistp, ebufp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
}

static	void
fm_mso_get_cust_active_plans(
	pcm_context_t           *ctxp,
 	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	
	poid_t 			*serv_pdp = NULL;
	
	char			msg[1024];
	int32			srch_flags = 768;
	char            	template [] = "select X from /purchased_product where  F1 = V1 and F2 = V2 and F3 = V3 ";
	char			func_name[] = "fm_mso_get_cust_active_plans";
	int32			prod_status = 1;
	int32			status_failure =-1;
	int64           	db = 1;
	
	void			*vp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing");	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;
	}
	
	serv_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1 ,ebufp);
	
	if (!serv_pdp)
	{
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service Poid Not Found");
           *ret_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
           goto CLEANUP;
	}

	sprintf(msg,"%s: Prepare search flist to search active purchased plan", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp );
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp );
	
        args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prod_status, ebufp );

	result_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch purchased product poids iflist:", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
	   PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()");
	   *ret_flistp = PIN_FLIST_CREATE(ebufp);
	   PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
  	   PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
	   PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
	   PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
	   goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch purchased product poids oflist:", srch_oflistp);
	*ret_flistp = PIN_FLIST_COPY(srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_active_plans return flist", *ret_flistp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
}
void
fm_mso_get_srvc_info(
    pcm_context_t       *ctxp,
    pin_flist_t     *in_flistp,
    pin_flist_t     **out_flistp,
    pin_errbuf_t        *ebufp)


 {
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"serv_info:: input flist::", in_flistp);
        poid_t        *vp = NULL;
        pin_flist_t *robj_iflistp = NULL;
        pin_flist_t *robj_rflistp = NULL;
        char        msg[100];
        if (PIN_ERRBUF_IS_ERR(ebufp)){
                sprintf(msg,"%s: fm_mso_user_bill_info::ERROR");
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
        }

         vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0 ,ebufp);
         robj_iflistp = PIN_FLIST_CREATE(ebufp);
         PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID, vp, ebufp);

         PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, robj_iflistp, &robj_rflistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
         {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&robj_rflistp, NULL);
                return;
         }
         PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"serv_info:: output flist::", robj_rflistp);
         *out_flistp=PIN_FLIST_COPY(robj_rflistp,ebufp);
         PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
         PIN_FLIST_DESTROY_EX(&robj_rflistp, NULL);
         return;
}