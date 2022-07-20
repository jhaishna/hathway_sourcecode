/*******************************************************************
 * File:	fm_mso_cust_reactivate_service.c
 * Opcode:	MSO_OP_CUST_REACTIVATE_SERVICE 
 * Owner:	Rohini Mogili
 * Created:	08-NOV-2013
 * Description: Caled for changing the status of the service.

	r << xx 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 9
	0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
	0 PIN_FLD_DESCR           STR [0] "[11 Nov, 2013 5:44 PM  Bill not paid]\n"
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /service/admin_client 43523 10
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 9
	0 PIN_FLD_STATUS         ENUM [0] 1
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
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_reactivate_service.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
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
#include "mso_prov.h"
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

extern void
fm_mso_cust_get_bp_obj(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        poid_t                  *service_obj,
	int			mso_status,
        poid_t                  **bp_obj,
        poid_t                  **mso_obj,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_commission_error_processing (
        pcm_context_t           *ctxp,
        int                     error_no,
        char                    *error_in,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	int32		action,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_set_bp_status(
	pcm_context_t           *ctxp,
	poid_t                  *service_obj,
	poid_t                  *bp_obj,
	int32					old_status,
	pin_errbuf_t            *ebufp);

PIN_IMPORT void
fm_mso_cust_get_acc_from_sno(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **ret_flistpp,
        pin_errbuf_t    *ebufp);


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
fm_mso_reactivate_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_reactive_service_reactivation_fee(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp);

extern int
fm_mso_apply_reactivation_fee(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
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

extern void
get_evt_lifecycle_template(
	pcm_context_t			*ctxp,
	int64				db,
	int32				string_id, 
	int32				string_version,
	char				**lc_template, 
	pin_errbuf_t			*ebufp);

static int32 
fm_mso_reactive_service_validate(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp);


static int32 
fm_mso_validate_base_pkg(
	pcm_context_t	*ctxp,
	pin_flist_t	* in_flist, 
	pin_errbuf_t	*ebufp);


//static void
//fm_mso_cust_react_lc_event(
//	pcm_context_t		*ctxp,
//	pin_flist_t		*i_flistp,
//	pin_flist_t		*r_flistp,
//	pin_errbuf_t		*ebufp);


/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_REACTIVATE_SERVICE 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_reactivate_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_reactivate_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_REACTIVATE_SERVICE  
 *******************************************************************/
void 
op_mso_cust_reactivate_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int			local = -1;
	int			*status = NULL;
    int32       *t_openp = NULL;
	int32			status_uncommitted =2;
	poid_t			*inp_pdp = NULL;
	char            	*prog_name = NULL;
	int32                           curr_status= 0;
	poid_t			*service_obj = NULL;
	poid_t			*plan_pdp = NULL;	
	pin_flist_t		*r_flistp = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_REACTIVATE_SERVICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_reactivate_service error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_reactivate_service input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_reactivate_service()", ebufp);
		return;
	}*/

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
    fm_mso_cust_reactivate_service(ctxp, flags, i_flistp, r_flistpp, ebufp);
    status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
    t_openp = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_TRANS_DONE, 1, ebufp);
    if (t_openp)
    {
        local = *(int32 *)t_openp;
    }
    inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (status && *(int*)status == 1)) 
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
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"op_mso_cust_reactivate_service error", i_flistp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		service_obj = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
		if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
		{
			plan_pdp = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_PLAN_OBJ, 0, ebufp );
			curr_status = MSO_PROV_STATE_SUSPENDED;
			if(plan_pdp)
                	   fm_mso_set_bp_status ( ctxp, service_obj, plan_pdp,curr_status, ebufp);
                   if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_POID_IS_NULL(plan_pdp) != 0 )
                   {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in setting base plan status", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
                  }
		}
		if(local == LOCAL_TRANS_OPEN_SUCCESS && 
                        (!(prog_name && (strstr(prog_name,"pin_deferred_act") || strstr(prog_name,"Service Re-Activation after Payment"))))
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
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_reactivate_service output flist", *r_flistpp);
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_reactivate_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*search_flistp = NULL;
	pin_flist_t		*search_res_flistp = NULL;
	pin_flist_t		*services_flistp = NULL;
    pin_flist_t     *service_info_flistp = NULL;
    pin_flist_t     *catv_info_flistp = NULL;
    pin_cookie_t    cookie = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*user_id = NULL;
	poid_t			*parent_service_pdp = NULL;
	char			*stb_idp = NULL;	
	char			*account_no_str = NULL;
    char            *prog_name = NULL;
	//char			account_no_str[10];
	int32			search_flags = 0;
	int32			status_change_success = 0;
	int32			status_change_failure = 1;
    int32           srvc_status_inactive = PIN_STATUS_INACTIVE;
    int32           srvc_status = 0;
    int32           conn_type = 0;
	int64			db = -1;
	int				csr_access = 0;
	int				acct_update = -1;
    int32           service_flags = 6;
    int32           elem_id = 0;
    int32           local = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service input flist", i_flistp);	
	
	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if (routing_poid)
	{	
		db = PIN_POID_GET_DB(routing_poid);
		account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		account_no_str = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, 1, ebufp);
		user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp); 
		stb_idp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp);
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51432", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
    if (account_no_str)
    {
        search_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, search_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, search_flistp, PIN_FLD_USERID, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, search_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, search_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &service_flags, ebufp);

        PCM_OP(ctxp, MSO_OP_CUST_GET_CUSTOMER_INFO, 0, search_flistp, &search_res_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_suspend_service get_subscriber flist", search_res_flistp);

        service_info_flistp = PIN_FLIST_SUBSTR_GET(search_res_flistp, PIN_FLD_SERVICE_INFO, 1, ebufp);

        if (service_info_flistp == NULL)
        {
            ret_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51437", ebufp);
            PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Jio customer ID passing not having any service", ebufp);
            goto CLEANUP;
        }

        PIN_FLIST_FLD_COPY(search_res_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
        account_obj = PIN_FLIST_FLD_GET(search_res_flistp, PIN_FLD_POID, 1, ebufp);
    }
	else if (stb_idp)
    {
		fm_mso_cust_get_acc_from_sno(ctxp, i_flistp, &services_flistp, ebufp);

                if (services_flistp)
                {
			account_obj = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
			PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_SERVICE_OBJ, i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_DESTROY_EX(&services_flistp, NULL);
                }
                else
                {
                        PIN_FLIST_DESTROY_EX(&services_flistp, NULL);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Device serial number not found");
                        ret_flistp= PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51430", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Device serial number not found", ebufp);
                        goto CLEANUP;
                }
        }
	if (!account_obj)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51433", ebufp);
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
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51434", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CSR does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service CSR haas permission to change account information");	
		}
	}else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51435", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	
	/*******************************************************************
	* Validation for PRICING_ADMIN roles  - End
	*******************************************************************/
   prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//  if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
        if (!(prog_name && (strstr(prog_name,"pin_deferred_act") || strstr(prog_name,"Service Re-Activation after Payment"))))
    {
            local = fm_mso_trans_open(ctxp, account_obj, 3, ebufp);
            if(PIN_ERRBUF_IS_ERR(ebufp))
            {
                    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
                    PIN_ERRBUF_RESET(ebufp);
                    *r_flistpp = PIN_FLIST_CREATE(ebufp);
                    PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, account_obj, ebufp );
                    PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_change_failure, ebufp);
                    PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
                    PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
                    return;
            }
            if ( local == LOCAL_TRANS_OPEN_FAIL )
            {
                    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
                    PIN_ERRBUF_RESET(ebufp);
                    *r_flistpp = PIN_FLIST_CREATE(ebufp);
                    PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, account_obj, ebufp );
                    PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_change_failure, ebufp);
                    PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
                    PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
                    return;
            }
    }
    else
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'pin_deferred_act or payment' so transaction will not be handled at API level");
    }
 
	/*******************************************************************
	* Service status change - Start
	*******************************************************************/

    while (account_no_str && (services_flistp = PIN_FLIST_ELEM_GET_NEXT(service_info_flistp, PIN_FLD_SERVICES,
                                &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
            srvc_status = *(int32 *)PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_STATUS, 0, ebufp);
            catv_info_flistp = PIN_FLIST_SUBSTR_GET(services_flistp, MSO_FLD_CATV_INFO, 1, ebufp);
            if (srvc_status == srvc_status_inactive && catv_info_flistp)
            {
                conn_type = *(int32 *)PIN_FLIST_FLD_GET(catv_info_flistp, PIN_FLD_CONN_TYPE, 0, ebufp);
                if (conn_type == 0)
                {
                    parent_service_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_POID, 0, ebufp);
                }
                else
                {
                    PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_POID, i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_cust_suspend_service status", i_flistp);
	                acct_update = fm_mso_reactivate_service_status(ctxp, i_flistp, &ret_flistp, ebufp);
                    if (acct_update == 0)
                    {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Account information update failed");
                        break;
                    }
                    else if ( acct_update == 2)
                    {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_suspend_service child status change successful");
                        fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_SUSPEND_SERVICE, ebufp );
                        acct_update = -1;
                    }
                    PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                }
            }
        }

    if (acct_update == -1)
    {
        if (parent_service_pdp)
        {
            PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SERVICE_OBJ, parent_service_pdp, ebufp);
        }
        acct_update = fm_mso_reactivate_service_status(ctxp, i_flistp, &ret_flistp, ebufp);
    }

	if ( acct_update == 0)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service no need of status change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service status change successful");
		/*============================================================
		Calling LIFECYCLE event creation function
		============================================================*/
		//fm_mso_cust_react_lc_event(ctxp, i_flistp, ret_flistp, ebufp );
		fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_REACTIVATE_SERVICE ,ebufp );
	}
	if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service insufficient data provided");	
	}
	if ( acct_update == 4)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service no service existing with this service poid");	
	}
        if ( acct_update == 5)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_reactivate_service service already terminated");
        }


	/*******************************************************************
	* Service status change  - End
	*******************************************************************/
    if (ret_flistp)
    {
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_TRANS_DONE, &local, ebufp);
    }

	CLEANUP:	
	*r_flistpp = ret_flistp;
	return;
}

/**************************************************
* Function: fm_mso_reactivate_service_status()
* 
* 
***************************************************/
EXPORT_OP int 
fm_mso_reactivate_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*readsvc_inflistp = NULL;
	pin_flist_t		*readsvc_outflistp = NULL;
	pin_flist_t		*updsvc_inflistp = NULL;
	pin_flist_t		*updsvc_outflistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t     *inher_flistp = NULL;
    pin_flist_t     *plan_flistpp = NULL;
    pin_flist_t     *charge_flistp = NULL;
    pin_flist_t     *temp_flistp = NULL;
    pin_flist_t     *result_flistp = NULL;
    pin_flist_t     *adjust_iflistp = NULL;
    pin_flist_t     *adjust_oflistp = NULL;
    pin_flist_t     *product_flistp = NULL;
    pin_flist_t     *cycle_fee_flistp = NULL;
    pin_flist_t     *acnt_info = NULL;

	poid_t			*offering_obj = NULL;
	poid_t			*service_obj = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*bp_obj = NULL;
    poid_t          *mso_bb_bp_obj = NULL;
    poid_t          *mso_bb_obj = NULL;
    poid_t          *plan_pdp = NULL;
    poid_t          *plan_pdp1 = NULL;
    poid_t          *deal_pdp = NULL;
    poid_t          *product_pdp = NULL;
    time_t          *sus_datep = NULL;
    time_t          *end_datep = NULL;
    time_t          *strt_datep = NULL;
    time_t          pvt = 0;
    time_t          noofdays = 0;
    pin_decimal_t   *charge_amount = NULL;
    double          charge_days = 1;

	int				status_flags = PIN_STATUS_FLAG_MANUAL;
	int				*status = NULL;
	int				*old_status = NULL;
	int				*technology = NULL;
	int				*old_prov_status = NULL;
	int				new_prov_status = 0;
	int				elem_id = 0;
	int				catv_service = 0;
	int				bb_service = 0;
	int				flg = 0;
	int				apply_fee = 0;
	int				status_change_success = 0;
	int				status_change_failure = 1;
	int				validate = 0;
	int				*conn_type = NULL;
	int				*prov_call = NULL;
    int             plan_days = 0;
    int             end_datepp = 0;
    int32           flags = 3;
    int32           bus_type = 0;
	int32			curr_status= 0;
	int32           grace_flags = 0xFFF;
	int64			db = 0;
	int32			indicator = -1;
	pin_cookie_t	cookie = NULL;
	int32			base_valid = 0;
    struct          tm *current_time;
//	char			search_template[100] = " select X from /purchased_product where F1 = V1 and F2 != V2 ";
    char		    search_template[100] = " select X from /purchased_product where F1 = V1 and F2 != V2 and F3 != V3";
	char				*program = NULL; 
	char				*msg = NULL; 
	char				*event = NULL; 
	char				*agreement_no = NULL;
	void				*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status :", in_flist);	

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	acct_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	status = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_STATUS, 1, ebufp );

	if (!service_obj || !status)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51436", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service reactivate status - Service object or status is missing in input", ebufp);
		
		*ret_flistp = r_flistp;
		return 3;
	}

	db = PIN_POID_GET_DB(service_obj);

        if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
        {
                technology = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_TECHNOLOGY, 1, ebufp );
/*                if (!technology )
                {
                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51617", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service Technology is mandatory for bb service suspension/reconnect", ebufp);
                        *ret_flistp = r_flistp;
                        return 3;
                }  */

                bb_service = 1;
        }
        else if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
        {
                catv_service = 1;
        }

	readsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_POID, service_obj, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_STATUS, NULL, ebufp);
	/* Added for suspension reactication issue for cisco boxes 21-JAN-2016*/
	//PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_LAST_STATUS_T, NULL, ebufp);
        if ( catv_service == 1 )
        {
            args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, MSO_FLD_CATV_INFO, 0, ebufp );
            PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CONN_TYPE, NULL, ebufp);
		    PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
		    PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_LAST_STATUS_T, NULL, ebufp);
        /* Added for suspension reactication issue for cisco boxes 21-JAN-2016*/
		//PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
        }
        if ( bb_service == 1 )
        {
                args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, MSO_FLD_BB_INFO, 0, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
		//Added for suspension/reactivation for postpaid
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status read service input list", readsvc_inflistp);
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
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51436", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service reactivate status - PCM_OP_READ_FLDS of service poid error", ebufp);
		
		*ret_flistp = r_flistp;
		return 4;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status read service output flist", readsvc_outflistp);
	
	if ( catv_service == 1 )
	{
		old_status = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_STATUS, 1, ebufp );
		sus_datep = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_LAST_STATUS_T, 1, ebufp );
        args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, MSO_FLD_CATV_INFO, 0, 1, ebufp );
		conn_type = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_CONN_TYPE, 1, ebufp );	
		vp = (char*)PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp );
		if (vp)
		{
			agreement_no = strdup(vp); 
		}
                if (old_status && *(int*)old_status == 10103)
                {

                        PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51436", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service is in Terminated status", ebufp);

                        *ret_flistp = r_flistp;
                        return 5;
                }

		if (*(int*)status == *(int*)old_status)
		{
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51436", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service reactivate status - There is no need of service reactivation", ebufp);
		
			*ret_flistp = r_flistp;
			return 1;		
		}
		if (*(int*)conn_type == 2)
		{
			validate = fm_mso_reactive_service_validate(ctxp, in_flist, ebufp);
			if (validate == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service object reactive not valid as the parent service is still not active:");	
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51441", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service object reactive not valid as the parent service is still not active", ebufp);
			
				*ret_flistp = r_flistp;
				return 3;
			}

		}
	
	}
        else if ( bb_service == 1 )
        {
                old_status = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_STATUS, 1, ebufp );
                args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp );
                old_prov_status = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATUS, 1, ebufp );

		args_flistp = PIN_FLIST_SUBSTR_GET(readsvc_outflistp, MSO_FLD_BB_INFO, 1, ebufp );
		vp = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp );
		if (vp)
		{
			agreement_no = strdup(vp); 
		}
		vp = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_INDICATOR, 1, ebufp);
		if (vp)
		{
			indicator = *(int32 *)vp;
		}
                if ( *(int*)old_prov_status != MSO_PROV_SUSPEND )
                {
                        PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51616", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Provisioning status for this service is not suspended - Cannot reactivate", ebufp);
                        *ret_flistp = r_flistp;
                        return 1;
                }
		
        }
	/* Added for suspension reactication issue for cisco boxes 21-JAN-2016*/

	/*if ( catv_service == 1 )
        {
                char            *net_elem = NULL;
                time_t          last_status_t = 0;
                pin_flist_t     *srv_catv_flistp = NULL;
                pin_flist_t     *tmp_prov_iflistp = NULL;
                pin_flist_t     *tmp_prov_oflistp = NULL;
                status_flags = CATV_SUSPEND;
                last_status_t = *(time_t*)PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_LAST_STATUS_T,0,ebufp);
                srv_catv_flistp = PIN_FLIST_SUBSTR_GET(readsvc_outflistp, MSO_FLD_CATV_INFO, 0, ebufp);
                net_elem = PIN_FLIST_FLD_GET(srv_catv_flistp,MSO_FLD_NETWORK_NODE,0,ebufp);
                if(net_elem  && strstr(net_elem,"CISCO") &&(last_status_t < 1451413800))
                {
                        PIN_ERR_LOG_MSG(3,"Service is suspended before 30-DEC-2015 and eligible for resuspension");
                        tmp_prov_iflistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_POID,tmp_prov_iflistp,PIN_FLD_POID,ebufp);
                        PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_SERVICE_OBJ,tmp_prov_iflistp,PIN_FLD_SERVICE_OBJ,ebufp);
                        PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,tmp_prov_iflistp,PIN_FLD_USERID,ebufp);
                        PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,tmp_prov_iflistp,PIN_FLD_PROGRAM_NAME,ebufp);
                        PIN_FLIST_FLD_SET(tmp_prov_iflistp,PIN_FLD_FLAGS,&status_flags,ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status creating suspension prov input list", tmp_prov_iflistp);
                        PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, tmp_prov_iflistp, &tmp_prov_oflistp, ebufp);

                        PIN_FLIST_DESTROY_EX(&tmp_prov_iflistp, NULL);
                        if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION cisco suspensio order", ebufp);
                                PIN_ERRBUF_RESET(ebufp);
                                PIN_FLIST_DESTROY_EX(&tmp_prov_oflistp, NULL);
                                r_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51436", ebufp);
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service reactivate status - MSO_OP_PROV_CREATE_ACTION error", ebufp);

                                *ret_flistp = r_flistp;
                                 return 0;
                        }
                        PIN_FLIST_DESTROY_EX(&tmp_prov_oflistp, NULL);
			sleep(5);
                }
	}*/
	//PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);

	updsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updsvc_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updsvc_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, updsvc_inflistp, PIN_FLD_DESCR, ebufp);
	services_flistp = PIN_FLIST_ELEM_ADD(updsvc_inflistp, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, services_flistp, PIN_FLD_POID, ebufp);
	
	if ( catv_service == 1 )
        {
		PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);		
		PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS, status, ebufp);
	}
        else if ( bb_service == 1 )
        {
		//Added for suspension/reactivation fix BB
                if(indicator == PAYINFO_BB_POSTPAID)
		{
		        PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);
	                PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS, status, ebufp);
		}
		new_prov_status = MSO_PROV_IN_PROGRESS;
                inher_flistp = PIN_FLIST_SUBSTR_ADD(services_flistp, PIN_FLD_INHERITED_INFO, ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(inher_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &new_prov_status, ebufp);
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status input list", updsvc_inflistp);
	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, updsvc_inflistp, &updsvc_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&updsvc_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_SERVICES", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51436", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service reactivate status - PCM_OP_CUST_UPDATE_SERVICES of service poid error", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status output flist", updsvc_outflistp);
	PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);

/*	fm_mso_cust_get_bp_obj ( ctxp, acct_pdp, service_obj, bp_obj, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_POID_IS_NULL(bp_obj) != 0 )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting base plan object", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51645", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in searching /mso_purchased_plan for this account", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}
	fm_mso_set_bp_status ( ctxp, service_obj, bp_obj, ebufp);	
	if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_POID_IS_NULL(bp_obj) != 0 )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in setting base plan status", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51645", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in setting /mso_purchased_plan status for this account", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	} */

	/*************************** Searching purchased product *************************************/
	if(catv_service)
	{
		base_valid = fm_mso_validate_base_pkg(ctxp, in_flist, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
	                PIN_ERRBUF_RESET(ebufp);
        	        PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
                	r_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                	PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
               		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51436", ebufp);
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service reactivate status - Error during base pack validation", ebufp);

                	*ret_flistp = r_flistp;
                	return 0;
        	}
	}
	// Added to prevent prov reactivation of packs when base pack is not available
	if(bb_service == 1 || base_valid == 1)
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

	    	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS_FLAGS, &grace_flags, ebufp);

		results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp );
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status search purchased product input list", search_inflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51436", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service reactivate status - PCM_OP_SEARCH of purchased product error", ebufp);
			
			*ret_flistp = r_flistp;
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status search purchased product output flist", search_outflistp);	
        fm_mso_get_account_info(ctxp, (poid_t *)acct_pdp, &acnt_info, ebufp);
        vp = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
        if(vp)
        {
            bus_type = *(int32 *)vp;
        }
        if(bus_type && bus_type != 0  && (bus_type == 99481100) && search_outflistp)
        {
            cookie = NULL;
            elem_id = 0;
            charge_flistp = PIN_FLIST_CREATE(ebufp);
        //  pvt = pin_virtual_time((time_t *)NULL);
            pvt = fm_utils_time_round_to_midnight(pin_virtual_time((time_t *)NULL));
            while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL)
            {
                if(result_flistp)
                {
                    plan_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
                    deal_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
                    offering_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp);
                    product_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
                    strt_datep = (time_t *)PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PURCHASE_START_T, 1, ebufp);
                    end_datep = (time_t *)PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
                    if(sus_datep && end_datep && *end_datep > pvt)
                    {
                        //for extend validity
                        noofdays = pvt - *sus_datep;
                        end_datepp = *end_datep;
                        noofdays = end_datepp + (noofdays + 86400);
                        current_time = localtime(&noofdays);
                        noofdays = mktime(current_time);
                        noofdays = fm_utils_time_round_to_midnight((time_t *)noofdays);
                        if(plan_pdp)
                        {
                            adjust_iflistp = PIN_FLIST_CREATE(ebufp);
                            PIN_FLIST_FLD_SET(adjust_iflistp, PIN_FLD_POID, offering_obj, ebufp);
                            PIN_FLIST_FLD_SET(adjust_iflistp, PIN_FLD_USAGE_END_T, &noofdays, ebufp);
                            PIN_FLIST_FLD_SET(adjust_iflistp, PIN_FLD_CYCLE_END_T, &noofdays, ebufp);
                            PIN_FLIST_FLD_SET(adjust_iflistp, PIN_FLD_PURCHASE_END_T, &noofdays, ebufp);
                            cycle_fee_flistp = PIN_FLIST_ELEM_ADD(adjust_iflistp, PIN_FLD_CYCLE_FEES, 1, ebufp);
                            PIN_FLIST_FLD_SET(cycle_fee_flistp, PIN_FLD_CHARGED_TO_T, &noofdays, ebufp);
                            PIN_FLIST_FLD_SET(cycle_fee_flistp, PIN_FLD_CYCLE_FEE_END_T, &noofdays, ebufp)
                            PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, adjust_iflistp, &adjust_oflistp, ebufp);
                            if (PIN_ERRBUF_IS_ERR(ebufp))
                            {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling get plan details", ebufp);
                            }
                            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Extend validity output flistp", adjust_oflistp);
                        }
                    }
                }
            }
            if(charge_flistp)
            {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plan charge amounts::", charge_flistp);
            }
        }
		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	}
	status_flags = CATV_REACTIVATE;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PLAN_OBJ, provaction_inflistp, PIN_FLD_PLAN_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
	if (results_flistp)
	{
        cookie = NULL;
        elem_id = 0;
		while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
		{	
			offering_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp );
			services_flistp = PIN_FLIST_ELEM_ADD(provaction_inflistp, PIN_FLD_PRODUCTS, elem_id, ebufp );
			PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_POID, offering_obj, ebufp);
		}
	} 
	if (elem_id > 200)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51466", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "You can't activate more than 200 channels in one transaction", ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
		*ret_flistp = r_flistp;
		return 3;
	}

	// Pawan: 09-DEC-2014: Added below condition (bb_service == 1) to avoid this 
	//			flow for CATV service.
	if(bb_service == 1)
	{
		status_flags = DOC_BB_REACTIVATE_SERV;
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
		fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj,MSO_PROV_SUSPEND, &mso_bb_bp_obj, &mso_bb_obj,ebufp);
		if ( mso_bb_obj && !PIN_POID_IS_NULL(mso_bb_obj))
		{
			 PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_bb_obj, ebufp);
		}
		else if ( PIN_POID_IS_NULL(mso_bb_obj) || PIN_POID_IS_NULL(mso_bb_bp_obj))
		{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting base plan object for Provisioning", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51644", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Suspend service - Error in getting base plan object for Provisioning", ebufp);
				*ret_flistp = r_flistp;
				return 0;
		}
	}

	/* Write USERID, PROGRAM_NAME in buffer - start */
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);

	/* Write USERID, PROGRAM_NAME in buffer - end */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status provisioning input list", provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51436", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service reactivate status - MSO_OP_PROV_CREATE_ACTION error", ebufp);
		
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
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_PUT(*ret_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51751", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning action.", ebufp);
		return 0;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

	/******************************************* notification flist ***********************************************/
/*	status_flags = 8;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status provisioning input list", provaction_inflistp);
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
		PIN_FLIST_FLD_PUT(*ret_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51752", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning notification.", ebufp);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_service_status provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
*/
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

	/* Pawan 08-DEC-2014: Added below to Update MSO_PURCHASED_PLAN status */
	/*if ( bb_service == 1 )
	{	
		curr_status = MSO_PROV_STATE_SUSPENDED;
		fm_mso_set_bp_status ( ctxp, service_obj, mso_bb_bp_obj,curr_status, ebufp);	
		if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_POID_IS_NULL(mso_bb_bp_obj) != 0 )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in setting base plan status", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51645", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in setting /mso_purchased_plan status for this account", ebufp);
			*ret_flistp = r_flistp;
			return 0;
		}
	}*/

	/******************************************* reaction fee flist ***********************************************/

	//commented as  no reactivation fee applied for CATV for now
       // if ( catv_service == 1 )
      //  {
      //          apply_fee = fm_mso_apply_reactivation_fee(ctxp, in_flist, ebufp);

        //        if (apply_fee)
          //      {
                        //fm_mso_suspend_service_reactivation_fee(ctxp, in_flist, apply_fee, ebufp);
	//		fm_mso_reactive_service_reactivation_fee(ctxp, in_flist, ebufp);
         //       }
       // }

        /*******************************************************************
        * Create Lifecycle event - Start
        *******************************************************************/
        program = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 1, ebufp );
        msg = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_DESCR, 1, ebufp );
        event = MSO_LC_EVENT;
	flg = 2;
        //fm_mso_cust_gen_event(ctxp, acct_pdp, service_obj, program, msg, event, flg,  ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating Life cycle event in BRM ", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51618", ebufp);
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
	PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	if(mso_bb_bp_obj)
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_PLAN_OBJ, mso_bb_bp_obj, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "ACCOUNT - Service reactivate status completed successfully", ebufp);
	
	*ret_flistp = r_flistp;
	
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	return 2;
}

/**************************************************
* Function: fm_mso_reactive_service_reactivation_fee()
* 
* 
***************************************************/
static void
fm_mso_reactive_service_reactivation_fee(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*results2_flistp = NULL;
	pin_flist_t		*reactive_inflistp = NULL;
	pin_flist_t		*reactive_outflistp = NULL;
	pin_flist_t		*dealinfo_flistp = NULL;
	pin_flist_t		*purchasedeal_inflistp = NULL;
	pin_flist_t		*purchasedeal_outflistp = NULL;
	pin_flist_t 		*ret_flistp = NULL;

	//struct tm * localtime (const time_t * timer);

	struct tm		*timeeff;
	struct tm		*timenew;

	poid_t			*service_obj = NULL;
	time_t			pvt = 0;
	time_t			*effective_t = NULL;
	time_t			*last_inactive_t = NULL;
	time_t			new_effective_t = 0;

	int				*prev_days = NULL;
	int				days = 0;
	int				quantity = 0;
	int				new_days = 0;
	int				search_flags = 768;
	int				*old_status = NULL;
	int32			errp = 0;
	int64			db = -1;
	char			*valpp = NULL;

	char			search_template[100] = " select X from /mso_srvc_reactivation_fee where F1 = V1 ";
	char			search_template2[100] = " select X from /plan where F1 = V1 ";
	
	pin_flist_t             *lco_err_flistp = NULL;
	 int64                   lco_err_code = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee :");	

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );

	if (!service_obj)
	{
		return;
	}

	db = PIN_POID_GET_DB(service_obj);

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 1, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee search service output flist", search_outflistp);
	
	results2_flistp = PIN_FLIST_ELEM_TAKE(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

	pvt = pin_virtual_time((time_t *)NULL);

	if (!results2_flistp)
	{
		return;
	}
	last_inactive_t = PIN_FLIST_FLD_GET(results2_flistp, MSO_FLD_LAST_INACTIVE_T, 1, ebufp);
	prev_days = PIN_FLIST_FLD_GET(results2_flistp, PIN_FLD_DAYS, 1, ebufp);
	effective_t = PIN_FLIST_FLD_GET(results2_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);		

	timeeff = localtime(effective_t);
	timenew = localtime(effective_t);
	
	timenew->tm_year = timeeff->tm_year + 1;

	new_effective_t = mktime (timenew);

	reactive_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(results2_flistp, PIN_FLD_POID, reactive_inflistp, PIN_FLD_POID, ebufp);

	if (new_effective_t <= pvt)
	{
		days = (new_effective_t - *last_inactive_t)/86400;
		days = days + *(int*)prev_days;

		new_days = (pvt - new_effective_t)/86400;

		if (new_days >= 90)
		{
			PIN_FLIST_FLD_SET(reactive_inflistp, PIN_FLD_DAYS, &quantity, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(reactive_inflistp, PIN_FLD_DAYS, &new_days, ebufp);
		}
		
		PIN_FLIST_FLD_SET(reactive_inflistp, PIN_FLD_EFFECTIVE_T, &new_effective_t, ebufp);
	}
	else
	{
		days = (pvt - *last_inactive_t)/86400;
		days = days + *(int*)prev_days;	
		
		if (days >= 90)
		{
			PIN_FLIST_FLD_SET(reactive_inflistp, PIN_FLD_DAYS, &quantity, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(reactive_inflistp, PIN_FLD_DAYS, &days, ebufp);
		}			
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee write flds input list", reactive_inflistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, reactive_inflistp, &reactive_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&reactive_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&reactive_outflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee write flds output flist", reactive_outflistp);
	PIN_FLIST_DESTROY_EX(&reactive_outflistp, NULL);

	pin_conf("fm_mso_cust","reactivation_plancode", PIN_FLDT_STR, &valpp, &errp); 

	if (valpp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, valpp);
	}

	if (days >= 90 || new_days >= 90)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee the days are more than 90");
		quantity = 1;
		if (days >= 90 && new_days >= 90)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee the new_days are more than 90");
			quantity = 2;
		}

		/********************************* purchasing deal**********************************/
		search_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
		PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template2, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CODE, valpp, ebufp);

		results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 1, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee search plan input list", search_inflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee search plan output flist", search_outflistp);

		results_flistp = PIN_FLIST_ELEM_TAKE(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (results_flistp)
		args_flistp = PIN_FLIST_ELEM_TAKE(results_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);

		if (args_flistp)
		{
		/********************************* purchasing deal**********************************/
		while (quantity > 0)
		{		
			purchasedeal_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, purchasedeal_inflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, purchasedeal_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, purchasedeal_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);

			dealinfo_flistp = PIN_FLIST_ELEM_ADD(purchasedeal_inflistp, PIN_FLD_DEAL_INFO, 0, ebufp );
			PIN_FLIST_FLD_COPY(args_flistp, PIN_FLD_DEAL_OBJ, dealinfo_flistp, PIN_FLD_DEAL_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, dealinfo_flistp, PIN_FLD_PLAN_OBJ, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee input list", purchasedeal_inflistp);
			PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, 0, purchasedeal_inflistp, &purchasedeal_outflistp, ebufp);
							
			switch (ebufp->pin_err) {
                        case 100002:
                        case 100004:
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "lco error set");
                                lco_err_code = ebufp->pin_err;
                                break;
                        default: lco_err_code = 0;
                	}

                	if (lco_err_code != 0) {
                        	PIN_ERRBUF_RESET(ebufp);
                        	lco_err_flistp = PIN_FLIST_CREATE(ebufp);
                        	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_POID,lco_err_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
                        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "lco error flistp", lco_err_flistp);
                	}

			PIN_FLIST_DESTROY_EX(&purchasedeal_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp) || lco_err_code != 0)
                	{
                        	/*LCO Error Handling Start*/
                        	if (lco_err_code != 0) {
                                	fm_mso_commission_error_processing(ctxp, lco_err_code,
                                        "Error in fm_mso_cust_activate_customer", lco_err_flistp, &ret_flistp, ebufp);
                        	}
                        	/*LCO Error Handling End*/
                        	else {
				//if (PIN_ERRBUF_IS_ERR(ebufp))
				//{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);
				return;
			  }
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_reactivation_fee output flist", purchasedeal_outflistp);
			
			PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);
			quantity = quantity - 1;
		}
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	}
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);		
	}
	PIN_FLIST_DESTROY_EX(&results2_flistp, NULL);
	if (valpp)
	{
		free(valpp);
	}		
	return;
}

/**************************************************
* Function: fm_mso_reactive_service_validate()
* 
* Return 1 for parent service active otherwise 0 
***************************************************/


static int32 
fm_mso_reactive_service_validate(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*account_obj = NULL;

	int			search_flags = 768;
	int32			s_status = PIN_STATUS_ACTIVE;
	int			elem_id = 0;
	int32			conn_type = 0;
	int64			db = -1;
	int32			is_parent_active= 0;
	char			*param_val = NULL;
	char			search_template[100] = " select X from /service/catv where F1 = V1 and F2 = V2 and F3 = V3 ";

	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return is_parent_active;
	PIN_ERRBUF_CLEAR(ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_validate :");	

	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp );
	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );

	if (!service_obj)
	{
		return is_parent_active;
	}

	db = PIN_POID_GET_DB(service_obj);

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &s_status, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
	args_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CONN_TYPE, &conn_type, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_validate search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactive_service_validate search service output flist", search_outflistp);
	
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		is_parent_active = 1;
		break;	
	}

CLEANUP:	
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	return is_parent_active;
}


static int32
fm_mso_validate_base_pkg(
        pcm_context_t   *ctxp,
        pin_flist_t     *in_flist,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t	*srch_i_flistp = NULL;
	pin_flist_t     *srch_o_flistp = NULL;
	pin_flist_t	*args_flistp   = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*srch_res_flistp =NULL;
	pin_flist_t	*write_i_flistp = NULL;
	pin_flist_t	*write_o_flistp = NULL;
	poid_t		*srvc_pdp = NULL;
	int32		srch_flags = 256;
	int32		elemid = 0;
	int32           status_flags = 0xFFF;		
        int32		prod_status = PIN_PRODUCT_STATUS_CANCELLED;
	int32		base_pack = 0;	
	int32		valid = 0;
	int64		db = 1;
	char		srch_template1[200] = "select X from /purchased_product 1, /product 2, /mso_cfg_catv_pt 3 where  1.F1 = V1 and 1.F2 != V2 and 1.F3 = 2.F4 and 2.F5 = 3.F6 and 3.F7 = V7 ";
	char		srch_template2[100] = "select X from /purchased_product where F1 = V1 and F2 != V2 and F3 != V3";
	pin_cookie_t    cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(3, "validate base pkg inflistp", in_flist);

	srvc_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 0, ebufp);

	srch_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	
        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prod_status, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, srvc_pdp, ebufp);
	
        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, srvc_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, srvc_pdp, ebufp);
 
        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 6, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, srvc_pdp, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PKG_TYPE, &base_pack, ebufp);
	
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, srch_template1, ebufp);
	
	results_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(3, "base pack search flistp", srch_i_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp, &srch_o_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
	
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
                PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);
                return;
        }
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Purchase Products output flist", srch_o_flistp);
	
	srch_res_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	
	if(srch_res_flistp)
	{
		srvc_pdp = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		PIN_FLIST_FLD_SET(in_flist, PIN_FLD_PLAN_OBJ, srvc_pdp, ebufp);
		PIN_ERR_LOG_MSG(3, "base pack is available");
		valid = 1;
		goto CLEANUP;
	}
	else 
	{
		
		valid = 0;		
		PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);
		
		srch_i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
		
	       	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);

        	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp);
	        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prod_status, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 3, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status_flags, ebufp);		

		PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, srch_template2, ebufp);

	        results_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
	        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
		
		PIN_ERR_LOG_FLIST(3, "purchased_product_srch" , srch_i_flistp);
		
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp, &srch_o_flistp, ebufp);
	        PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);

        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
	                PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);
        	        return;
        	}

	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Purchase Products output flist", srch_o_flistp);
		
		while((srch_res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_o_flistp, PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp)) != NULL)
		{
			write_i_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_POID, write_i_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(write_i_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);
			PIN_ERR_LOG_FLIST(3, "Deactivation writeflds flist", write_i_flistp);

			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_i_flistp, &write_o_flistp, ebufp);
		
			PIN_ERR_LOG_FLIST(3, "Deactivation writeflds flist", write_o_flistp);	
			PIN_FLIST_DESTROY_EX(&write_i_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&write_o_flistp, ebufp);
		}
	}

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_i_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_o_flistp, NULL);
		return valid;

}