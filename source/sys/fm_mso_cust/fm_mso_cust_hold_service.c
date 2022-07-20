/*******************************************************************
 * File:	fm_mso_cust_hold_service.c
 * Opcode:	MSO_OP_CUST_HOLD_SERVICE 
 * Owner:	Leela Pratyush
 * Created:	11-AUG-2014
 * Description: This opcode will be invoked incase of customer requested suspension for a temporary period

	r << xx 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 50541 0
	0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
	0 PIN_FLD_DESCR           STR [0] "Bill not paid"
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 50541 10
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 2302241 0
	0 MSO_FLD_TECHNOLOGY     ENUM [0] 0
	0 PIN_FLD_STATUS         ENUM [0] 10102
	0 PIN_FLD_INDICATOR       INT [0] 2
	0 PIN_FLD_VALIDITY_FLAGS    INT [0] 1
	xx
	xop 11008 0 1

	nap(13252)> xop 11008 0 1
	xop: opcode 11008, flags 0
	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41343 9
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 9
	0 PIN_FLD_DESCR           STR [0] "ACCOUNT - Service status change completed successfully"

 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_hold_service.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "ops/bal.h"
#include "ops/subscription.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "psiu_business_params.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_ntf.h"
#include "mso_lifecycle_id.h"


#define MSO_PREPAID           2
#define MSO_INR_CUR           356
#define MSO_EXTEND_VALIDITY   1
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

extern void
fm_mso_cust_gen_event(
        pcm_context_t   *ctxp,
        poid_t          *acct_pdp,
        poid_t          *serv_pdp,
        char            *program,
        char            *descr,
        char            *event_type,
        int		flags,
	pin_errbuf_t    *ebufp);

EXPORT_OP int 
fm_mso_hold_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_HOLD_SERVICE 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_hold_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_hold_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_cust_get_bp_obj(
	pcm_context_t           *ctxp,
	poid_t			*account_obj,
	poid_t                  *service_obj,
	int						mso_status,
	poid_t                  **bp_obj,
	poid_t                  **mso_obj,
	pin_errbuf_t            *ebufp);

extern void
fm_mso_set_bp_status(
	pcm_context_t           *ctxp,
	poid_t                  *service_obj,
	poid_t                  *bp_obj,
	int32					old_status,
	pin_errbuf_t            *ebufp);
/*******************************************************************
 * MSO_OP_CUST_HOLD_SERVICE  
 *******************************************************************/
void 
op_mso_cust_hold_service(
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
	int32			status_uncommitted =1;
	poid_t			*inp_pdp = NULL;
	char            *prog_name = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_HOLD_SERVICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_hold_service error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_hold_service input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_hold_service()", ebufp);
		return;
	}*/

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	//if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
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

	fm_mso_cust_hold_service(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1)) 
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
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_hold_service error", ebufp);
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
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}		
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_hold_service output flist", *r_flistpp);
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_hold_service(
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
	
	//char			*account_no_str = NULL;
	//char			account_no_str[10];
	int32			status_change_success = 0;
	int32			status_change_failure = 1;
	int64			db = -1;
	int				csr_access = 0;
	int				acct_update = 0;
	int32			*ind_chk = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hold_service input flist", i_flistp);	
	
	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	ind_chk = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_INDICATOR, 1, ebufp);
	if(ind_chk && *ind_chk == MSO_PREPAID)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51650", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Can't perform Hold action for Prepaid Customer", ebufp);
                goto CLEANUP;
	}
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
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51651", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	if (!account_obj)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51652", ebufp);
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
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51653", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CSR does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hold_service CSR haas permission to change account information");	
		}
	}else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51654", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	
	/*******************************************************************
	* Validation for PRICING_ADMIN roles  - End
	*******************************************************************/

	/*******************************************************************
	* Service status change - Start
	*******************************************************************/

	acct_update = fm_mso_hold_service_status(ctxp, i_flistp, &ret_flistp, ebufp);

	if ( acct_update == 0)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hold_service no need of status change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hold_service status change successful");
		fm_mso_create_lifecycle_evt(ctxp, i_flistp, NULL, ID_HOLD_SERVICE, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}
	if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hold_service insufficient data provided");	
	}
	if ( acct_update == 4)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_hold_service no service existing with this service poid");	
	}

	/*******************************************************************
	* Service status change  - End
	*******************************************************************/

	CLEANUP:	
	*r_flistpp = ret_flistp;
	return;
}

/**************************************************
* Function: fm_mso_hold_service_status()
* 
* 
***************************************************/
EXPORT_OP int 
fm_mso_hold_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*readsvc_inflistp = NULL;
	pin_flist_t		*readsvc_outflistp = NULL;
	pin_flist_t		*updsvc_inflistp = NULL;
	pin_flist_t		*updsvc_outflistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*inher_flistp = NULL;
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*read_flistp = NULL;
	pin_flist_t		*read_res_flistp = NULL;
	pin_flist_t		*read_bal_flistp = NULL;
	pin_flist_t		*read_bal_res_flistp = NULL;
	pin_flist_t		*set_in_flistp = NULL;
	pin_flist_t		*set_res_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*products_flistp = NULL;
	pin_flist_t		*sub_bal_flistp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*bal_grp_obj = NULL;
	poid_t			*product_obj = NULL;
	poid_t			*offering_obj = NULL;
	poid_t			*mso_bb_bp_obj = NULL;
	poid_t			*mso_bb_obj = NULL;

	int				status_flags = PIN_STATUS_FLAG_MANUAL;
	int				*technology = NULL;
	int				*status = NULL;
	int				*old_status = NULL;
	int				*old_prov_status = NULL;
	int				new_prov_status = 0;
	int				apply_fee = 0;
	int				bal_flags = 0;
	int64				db = 0;
	int				elem_id = 0;
	int				bal_elem_id = 0;
	int				sub_bal_id = 0;
	int				flg = 0;
	int				counter = 0;
	int32				*indicator = NULL;
	int				status_change_success = 0;
	int				status_change_failure = 1;
	int				validate = 0;
	int				prty = 0;
	int32				*extend = NULL;
	int					*prov_call = NULL;
	int32					curr_status = 0;
	char				*msg = NULL;
	char				*strp = NULL;
	char				*event = NULL;
	char				*program = NULL;
	char                    	*search_template = "select X from /purchased_product where F1 = V1 and F2 != V2 ";

	time_t				*end_t = NULL;

	pin_decimal_t			*priority = NULL;	
	
	pin_cookie_t 			cookie = NULL;
	pin_cookie_t 			bal_cookie = NULL;
	pin_cookie_t 			sub_cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status :");	

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	status = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_STATUS, 1, ebufp );
	indicator = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_INDICATOR, 1, ebufp );
	extend = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_VALIDITY_FLAGS, 1, ebufp );

	if ( !service_obj || !status || !indicator )
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51655", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service hold status - Service object or status is missing in input", ebufp);
		*ret_flistp = r_flistp;
		return 3;
	}

	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) != 0 )
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51656", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service hold status - Service hold is applicable only for BB service", ebufp);
		*ret_flistp = r_flistp;
		return 3;
	}
	technology = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_TECHNOLOGY, 1, ebufp );
	if (!technology )
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51657", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service Technology is mandatory for bb service hold/unhold", ebufp);
		*ret_flistp = r_flistp;
		return 3;
	}

	readsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_POID, service_obj, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_CREATED_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status read service input list", readsvc_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readsvc_inflistp, &readsvc_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&readsvc_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51658", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service hold status - PCM_OP_READ_FLDS of service poid error", ebufp);
		
		*ret_flistp = r_flistp;
		return 4;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status read service output flist", readsvc_outflistp);

	bal_grp_obj = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp );
	old_status = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_STATUS, 1, ebufp );
	args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp );
	old_prov_status = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATUS, 1, ebufp );
	if ( *(int *)status == *(int *)old_status || *(int *)status != MSO_SUSPEND_STATUS )
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51659", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service already has holded status ", ebufp);
	
		*ret_flistp = r_flistp;
		return 1;		
	}
	if ( *(int*)old_prov_status != MSO_PROV_ACTIVE )
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51660", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Provisioning status for this service is not active - Cannot hold", ebufp);
		*ret_flistp = r_flistp;
		return 1;		
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to update service status");
	updsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updsvc_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updsvc_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, updsvc_inflistp, PIN_FLD_DESCR, ebufp);
	services_flistp = PIN_FLIST_ELEM_ADD(updsvc_inflistp, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, services_flistp, PIN_FLD_POID, ebufp);
	
	PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);		
	PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS, status, ebufp);
	new_prov_status = MSO_PROV_IN_PROGRESS;
	inher_flistp = PIN_FLIST_SUBSTR_ADD(services_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(inher_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &new_prov_status, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status input list", updsvc_inflistp);
	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, updsvc_inflistp, &updsvc_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&updsvc_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_SERVICES", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51661", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service hold status - PCM_OP_CUST_UPDATE_SERVICES of service poid error", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status output flist", updsvc_outflistp);
	PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);

	acct_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	db = PIN_POID_GET_DB(service_obj);
	/*************************** Plan Validity Extension *************************************/
	if ( indicator && *(int32 *)indicator == MSO_PREPAID )
	{
		end_t = PIN_FLIST_FLD_GET(in_flist,PIN_FLD_END_T, 1, ebufp);
		if ( end_t != (time_t *)NULL && *(int32 *)extend == MSO_EXTEND_VALIDITY )
		{
			status_flags = 768;
			search_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, args_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

			status_flags = 3;

			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status_flags, ebufp);

			results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status search purchased product input list", search_inflistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51662", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - PCM_OP_SEARCH of purchased product error", ebufp);

				*ret_flistp = r_flistp;
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status search purchased product output flist", search_outflistp);

			results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );						
			if (!results_flistp)
			{
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				goto PROV;	
			}	
			while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
			{
				offering_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp );
				product_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp );
				read_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, product_obj, ebufp);
				PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_PRIORITY, (pin_decimal_t *)NULL, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status read product info input list", read_flistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating Product information", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51663", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Error in reading Product information", ebufp);
	
					*ret_flistp = r_flistp;
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status read product info result list", read_res_flistp);
				priority = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_PRIORITY, 1, ebufp);
				if ( !priority )
				{
					PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
					goto PROV;	
				}
				strp = pbo_decimal_to_str(priority, ebufp);
				prty = atoi(strp);
				if ( !( prty > BB_SME_PREPAID_START && prty < BB_SME_PREPAID_END ) &&
					!( prty > BB_RET_PREPAID_START && prty < BB_RET_PREPAID_END ) &&  
					!( prty > BB_CYB_PREPAID_START && prty < BB_RET_PREPAID_END ) &&  
					!( prty > BB_COR_PREPAID_START && prty < BB_COR_PREPAID_END ))
				{
					continue;	
				}   

				set_in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, set_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
				PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_DESCR, MSO_MANUAL_SUSPEND, ebufp);
				products_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp );
				PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_PRODUCT_OBJ, product_obj, ebufp);
				PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_OFFERING_OBJ, offering_obj, ebufp);
				PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_PURCHASE_END_T, end_t, ebufp);
				PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_USAGE_END_T, end_t, ebufp);
				PIN_FLIST_FLD_SET(products_flistp, PIN_FLD_CYCLE_END_T, end_t, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status set product info input list", set_in_flistp);
				PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODINFO, 0, set_in_flistp, &set_res_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating Product information", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51664", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Error in setting product information", ebufp);
	
					*ret_flistp = r_flistp;
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status set product info result list", set_res_flistp);
				PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
			}
		
			/*************************** Plan Validity Extension *************************************/
			/*************************** Resource Validity Extension *************************************/
			if ( !bal_grp_obj )
			{
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				goto PROV;	
			}
			read_bal_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
			PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_FLAGS, &bal_flags, ebufp);
			PIN_FLIST_ELEM_SET(read_bal_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status read balance group input list", read_bal_flistp);
			PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES , 0, read_bal_flistp, &read_bal_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&read_bal_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in reading Balance group information", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51665", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Error in reading Balance group information", ebufp);
	
				*ret_flistp = r_flistp;
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status read balance group result list", read_bal_res_flistp);
	
			bal_elem_id = 0;
			bal_cookie = NULL;	
			while ((bal_flistp = PIN_FLIST_ELEM_GET_NEXT(read_bal_res_flistp, PIN_FLD_BALANCES, &bal_elem_id, 1, &bal_cookie, ebufp)) != NULL)
			{	
				if ( bal_elem_id == MSO_INR_CUR )
					continue;
				set_in_flistp = PIN_FLIST_CREATE(ebufp);
				counter = PIN_FLIST_ELEM_COUNT(bal_flistp, PIN_FLD_SUB_BALANCES, ebufp);
				if ( counter == 0 || counter > 1 )
					continue;
				sub_bal_id = 0;
				sub_cookie = NULL;	
				sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_SUB_BALANCES, &sub_bal_id, 1, &sub_cookie, ebufp );	
				if ( !sub_bal_flistp )
					continue;
				
				PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, set_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
				PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
				PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_ELEMENT_ID, &sub_bal_id, ebufp);
				PIN_FLIST_FLD_SET(set_in_flistp, PIN_FLD_RESOURCE_ID, &bal_elem_id, ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, 0, ebufp);
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_TO, args_flistp, PIN_FLD_VALID_TO, ebufp );
				args_flistp = PIN_FLIST_ELEM_ADD(set_in_flistp, PIN_FLD_SUB_BALANCES, 1, ebufp);
				PIN_FLIST_FLD_COPY(sub_bal_flistp, PIN_FLD_VALID_FROM, args_flistp, PIN_FLD_VALID_FROM, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, (time_t *)end_t, ebufp);
	
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status bal grp change validity input list", set_in_flistp);
				PCM_OP(ctxp, PCM_OP_BAL_CHANGE_VALIDITY , 0, set_in_flistp, &set_res_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&set_in_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in reading Balance group information", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
					PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51666", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Prepaid plan extension - Error in reading Balance group information", ebufp);
		
					*ret_flistp = r_flistp;
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_service_status bal grp change validity result list", set_res_flistp);
			}
			PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&set_res_flistp, NULL);
		}	
	}
	PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
	/*************************** Resource Validity Extension *************************************/
	/*************************** Provisoning calling *************************************/
PROV:
	status_flags = DOC_BB_HOLD_SERV;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

	fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, -1, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
	if ( mso_bb_obj && !PIN_POID_IS_NULL(mso_bb_obj))
	{
		PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_bb_obj, ebufp);
	}
	else if ( PIN_POID_IS_NULL(mso_bb_obj))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting base plan object for Provisioning", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51667", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Hold service - Error in getting base plan object for Provisioning", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}
	/* Write USERID, PROGRAM_NAME in buffer - start */
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
	/* Write USERID, PROGRAM_NAME in buffer - end */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity provisioning input list", provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
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
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51668", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning action.", ebufp);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

	/******************************************* notification flist ***********************************************/
	status_flags = NTF_SERVICE_SUSPENSION;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity notification input list", provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
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
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51669", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning notification.", ebufp);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

	/* Pawan: 08-DEC-2014: Added below to Update MSO_PURCHASED_PLAN status*/
	curr_status = MSO_PROV_STATE_ACTIVE;
	fm_mso_set_bp_status(ctxp, service_obj, mso_bb_bp_obj,curr_status, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in Updating purchased plan status in BRM.");
		PIN_ERRBUF_RESET(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51670", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in Updating purchased plan status in BRM.", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}
	/*******************************************************************
	* Create Lifecycle event - Start
	*******************************************************************/
	program = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	msg = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_DESCR, 1, ebufp );
	event = MSO_LC_EVENT;
	flg = 5;
	//fm_mso_cust_gen_event(ctxp, acct_pdp, service_obj, program, msg, event, flg,ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating Life cycle event in BRM ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51670", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in creating Life cycle event in BRM", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}

	/*******************************************************************
	* Create Lifecycle event - End
	*******************************************************************/
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_success, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	if(*(int32 *)extend == MSO_EXTEND_VALIDITY) {
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Service hold with Validity extn completed successfully", ebufp);
	} else {
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Service hold completed successfully", ebufp);
	}
	
	*ret_flistp = r_flistp;
	
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	return 2;
}
/************************************************************
 * Function: fm_mso_cust_get_bp_obj()
 *		Search Base plan in mso_purchased_plan
 *		If mso_status is -1 then search plan with active(2)
 *		status. Otherwise use mso_status value.
 ***********************************************************/
extern void
fm_mso_cust_get_bp_obj(
	pcm_context_t           *ctxp,
	poid_t			*account_obj,
	poid_t                  *service_obj,
	int						mso_status,
	poid_t                  **bp_obj,
	poid_t                  **mso_obj,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        pin_flist_t             *search_flistp = NULL;
        pin_flist_t             *search_res_flistp = NULL;

	char            search_template[100] = " select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 and F4 = V4 ";

	int32           db = -1;
	int             search_flags = 512;
	int32		status = -1;
	int			res_cnt = 0;

	poid_t          *search_pdp = NULL;
	poid_t          *mso_bb_bp_obj = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	if(mso_status==-1)
		status = MSO_PROV_ACTIVE;
	else
		status = mso_status;
	db = PIN_POID_GET_DB(account_obj);
	
	search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	search_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, MSO_BB_BP, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_bp_obj: search input flist",search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_get_bp_obj:Error in searcing base plan object", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_bp_obj: search result flist",search_res_flistp);
	res_cnt = PIN_FLIST_ELEM_COUNT(search_res_flistp, PIN_FLD_RESULTS, ebufp);
	if ( res_cnt != 1 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_bp_obj:Error in searcing base plan object");
        PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return;
	}	
	result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	mso_bb_bp_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Got base plan object");	
	if ( mso_bb_bp_obj && !PIN_POID_IS_NULL(mso_bb_bp_obj) )
	{
		*bp_obj = PIN_POID_COPY(mso_bb_bp_obj, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Copied base plan object");	
        	mso_bb_bp_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Got mso plan object");	
		if ( !PIN_POID_IS_NULL(mso_bb_bp_obj) )
		{
			*mso_obj = PIN_POID_COPY(mso_bb_bp_obj,ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Copied mso plan object");	
		}
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Got the base plan object");
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return;	
	}
	PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	return;
}

