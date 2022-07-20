/*******************************************************************
 * File:	fm_mso_cust_suspend_service.c
 * Opcode:	MSO_OP_CUST_SUSPEND_SERVICE 
 * Owner:	Rohini Mogili
 * Created:	08-NOV-2013
 * Description: Caled for changing the status of the service.

	r << xx 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 9
	0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
	0 PIN_FLD_DESCR           STR [0] "[11 Nov, 2013 5:44 PM  Bill not paid]\n"
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /service/admin_client 43523 10
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 9
	0 PIN_FLD_STATUS         ENUM [0] 10102
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
 * Owner:	Leela Pratyush
 * Created:	08-NOV-2013
 * Description: Called for changing the status of the service.
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_suspend_service.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "psiu_business_params.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_lifecycle_id.h"


#define PSIU_BPARAMS_BILLING_REACTIVATION_FEE       "apply_reactivation_fee"

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
fm_mso_set_bp_status(
	pcm_context_t           *ctxp,
	poid_t                  *service_obj,
	poid_t                  *bp_obj,
	int32					old_status,
	pin_errbuf_t            *ebufp);		

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


extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	int32		action,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void
fm_mso_cust_get_acc_from_sno(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_flist_t     **ret_flistpp,
        pin_errbuf_t    *ebufp);
/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/

EXPORT_OP int 
fm_mso_suspend_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_suspend_service_reactivation_fee(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	int					apply_fee,
	pin_errbuf_t		*ebufp);

EXPORT_OP int
fm_mso_apply_reactivation_fee(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_suspend_service_validate(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_susp_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp);

void fm_mso_pre_suspend_updates(
        pcm_context_t   *ctxp,
        poid_t   	*service_pdp,
        pin_errbuf_t    *ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_SUSPEND_SERVICE 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_suspend_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_suspend_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_SUSPEND_SERVICE  
 *******************************************************************/
void 
op_mso_cust_suspend_service(
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
    int32           *t_openp = NULL;
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
	if (opcode != MSO_OP_CUST_SUSPEND_SERVICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_suspend_service error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_suspend_service input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_suspend_service()", ebufp);
		return;
	}*/

	fm_mso_cust_suspend_service(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
    t_openp = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_TRANS_DONE, 1, ebufp);
    if (t_openp)
    {
        local = *(int32 *)t_openp;
    }
    inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
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
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"op_mso_cust_suspend_service error", i_flistp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
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
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_suspend_service output flist", *r_flistpp);
    PIN_FLIST_FLD_DROP(*r_flistpp, PIN_FLD_TRANS_DONE, ebufp);
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_suspend_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*search_flistp = NULL;
	pin_flist_t		*search_res_flistp = NULL;
	pin_flist_t		*catv_info_flistp = NULL;
	pin_flist_t		*services_flistp = NULL;
    pin_flist_t     *service_info_flistp = NULL;
    pin_cookie_t    cookie = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*user_id = NULL;
	poid_t			*parent_service_pdp = NULL;
	char			*stb_idp = NULL;	
	char			*account_no_str = NULL;
    char            *prog_name = NULL;
	int32			status_change_success = 0;
	int32			status_change_failure = 1;
    int32           srvc_status_active = PIN_STATUS_ACTIVE;
    int32           srvc_status = 0;
    int32           conn_type = 0;
    int32           local = -1;
	int64			db = -1;
    int32           service_flags = 6;
    int32           elem_id = 0;
	int				csr_access = 0;
	int				acct_update = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_suspend_service input flist", i_flistp);	
	
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
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51427", ebufp);
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
	
		PIN_FLIST_FLD_COPY(search_res_flistp, PIN_FLD_POID, i_flistp, PIN_FLD_POID, ebufp);
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
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51428", ebufp);
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
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51429", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CSR does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_suspend_service CSR haas permission to change account information");	
		}
	}else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51430", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	
	/*******************************************************************
	* Validation for PRICING_ADMIN roles  - End
	*******************************************************************/
    prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
    //if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
        if (!(prog_name && strstr(prog_name,"pin_deferred_act")))
    {    
        local = fm_mso_trans_open(ctxp, account_obj, 3,ebufp);
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
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'pin_deferred_act' so transaction will not be handled at API level");
    } 
	/*******************************************************************
	* Service status change - Start
	*******************************************************************/

        while (account_no_str && (services_flistp = PIN_FLIST_ELEM_GET_NEXT(service_info_flistp, PIN_FLD_SERVICES,
                                &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
            srvc_status = *(int32 *)PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_STATUS, 0, ebufp);
            catv_info_flistp = PIN_FLIST_SUBSTR_GET(services_flistp, MSO_FLD_CATV_INFO, 1, ebufp);
            if (srvc_status == srvc_status_active && catv_info_flistp)
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
	                acct_update = fm_mso_suspend_service_status(ctxp, i_flistp, &ret_flistp, ebufp);
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
	    acct_update = fm_mso_suspend_service_status(ctxp, i_flistp, &ret_flistp, ebufp);
    }

	if ( acct_update == 0)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_suspend_service no need of status change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_suspend_service status change successful");	
		/*============================================================
		Calling LIFECYCLE event creation function
		============================================================*/
		//fm_mso_cust_susp_lc_event(ctxp, i_flistp, ret_flistp, ebufp ); 
		fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_SUSPEND_SERVICE, ebufp );

	}
    else if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_suspend_service insufficient data provided");	
	}
    else if ( acct_update == 4)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_suspend_service no service existing with this service poid");	
	}
    else if ( acct_update == 5)
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_suspend_service service already terminated");
    }

    
	/*******************************************************************
	* Service status change  - End
	*******************************************************************/
    if (ret_flistp)
    {
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_TRANS_DONE, &local, ebufp);
    }
	CLEANUP:	
    PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	*r_flistpp = ret_flistp;
	return;
}

/**************************************************
* Function: fm_mso_suspend_service_status()
* 
* 
***************************************************/
EXPORT_OP int 
fm_mso_suspend_service_status(
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
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*inher_flistp = NULL;
	pin_flist_t		*srch_i_flistp = NULL;
	pin_flist_t		*srch_o_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*srch_res_flistp = NULL;
    pin_flist_t     *service_write_flistp = NULL;
    pin_flist_t     *catv_info_flistp = NULL;
    pin_flist_t     *service_write_oflistp = NULL;
	poid_t			*service_obj = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*mso_bb_bp_obj = NULL;
	poid_t			*mso_bb_obj = NULL;
	poid_t			*srch_pdp = NULL;

	int				status_flags = PIN_STATUS_FLAG_MANUAL;
	int				*status = NULL;
	int				*technology = NULL;
	int				*old_status = NULL;
	int				*old_prov_status = NULL;
	int				new_prov_status = 0;
	int				apply_fee = 0;
	int				flg = 0;
	int				bb_service = 0;
	int				catv_service = 0;
	int				status_change_success = 0;
	int				status_change_failure = 1;
	int				validate = 0;
	int				*conn_type = NULL;
	int				*prov_call = NULL;
	int32				curr_status = 0;
	int32				srch_flags = 256;
	int32				grace_flags = 0xFFF;
	int32				prod_status = 1;
	int32				base_pack = 0;
	int32				indicator = -1;
    int                 reason_id = 0;
    int                 *reason_id1 = NULL;
    int                 domain_reason_id = 0;
    int                 *domain_reason_id1  = NULL;

	char				*msg = NULL;
	char				*event = NULL;
	char				*program = NULL;
	char				*agreement_no = NULL;
	char 				srch_tmpl[200] = "select X from /purchased_product 1, /product 2 , /mso_cfg_catv_pt 3 where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3 and 1.F4 = 2.F5 and 2.F6 = 3.F7 and 3.F8 = V8 " ;
	void				*vp = NULL;
	char				dummy_char[] = "";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_status :");	

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	status = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_STATUS, 1, ebufp );

	if (!service_obj || !status)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service suspend status - Service object or status is missing in input", ebufp);
		*ret_flistp = r_flistp;
		return 3;
	}

	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		technology = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_TECHNOLOGY, 1, ebufp );
	/*	if (!technology )
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
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_CREATED_T, NULL, ebufp);
	if ( catv_service == 1 )
	{
		args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, MSO_FLD_CATV_INFO, 0, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CONN_TYPE, NULL, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
	}
	if ( bb_service == 1 )
	{
		args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, MSO_FLD_BB_INFO, 0, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_status read service input list", readsvc_inflistp);
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
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service suspend status - PCM_OP_READ_FLDS of service poid error", ebufp);
		
		*ret_flistp = r_flistp;
		return 4;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_status read service output flist", readsvc_outflistp);
/*	old_status = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_STATUS, 1, ebufp );
	args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, MSO_FLD_CATV_INFO, 0, 1, ebufp );
	conn_type = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_CONN_TYPE, 1, ebufp );
*/	

	PIN_FLIST_FLD_COPY(readsvc_outflistp, PIN_FLD_CREATED_T, in_flist, PIN_FLD_EFFECTIVE_T, ebufp);
	if ( catv_service == 1 )
	{	
		// Added to prevent suspension when base pack is in 4095 
		srch_i_flistp = PIN_FLIST_CREATE(ebufp);
		srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(service_obj), "/search", -1, ebufp);
		PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS,	&srch_flags, ebufp);
		PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, srch_tmpl, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prod_status, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 3, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS_FLAGS, &grace_flags, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 4, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 5, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 6, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, dummy_char, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 7, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, dummy_char, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 8, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PKG_TYPE, &base_pack, ebufp);	
		res_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp ,PIN_FLD_RESULTS ,0 , ebufp);
		PIN_FLIST_FLD_SET(res_flistp ,PIN_FLD_POID ,NULL ,ebufp);
	
		PIN_ERR_LOG_FLIST(3, "search input flist", srch_i_flistp);
		
		PCM_OP(ctxp ,PCM_OP_SEARCH , 0, srch_i_flistp, &srch_o_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_i_flistp, ebufp);
	        if (PIN_ERRBUF_IS_ERR(ebufp))
	        {
		 	PIN_ERR_LOG_MSG(3, "ERROR DURING SEARCH FOR GRACE");
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&srch_o_flistp, ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
        	        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                  	PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51432", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                                        "ERROR -  DURING SEARCH FOR GRACE", ebufp);
                        *ret_flistp = r_flistp;
                        return 5;
		}
		srch_res_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		
		if(srch_res_flistp)
		{
                     PIN_ERR_LOG_MSG(3, "Base pack is in grace so no suspension");
		     PIN_ERRBUF_RESET(ebufp);
  		     PIN_FLIST_DESTROY_EX(&srch_o_flistp, ebufp);
                     r_flistp = PIN_FLIST_CREATE(ebufp);
                     PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                     PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                     PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
                     PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51432", ebufp);
                     PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                                        "BASE PACK is in grace suspension not allowed", ebufp);
                        *ret_flistp = r_flistp;
                        return 5;
                }
		old_status = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_STATUS, 1, ebufp );
		args_flistp = PIN_FLIST_SUBSTR_GET(readsvc_outflistp, MSO_FLD_CATV_INFO, 1, ebufp );
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
                        PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, 
					"ACCOUNT - Service already Terminated", ebufp);
                        *ret_flistp = r_flistp;
                        return 5;
                }

		if (*(int*)status == *(int*)old_status)
		{
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service suspend status - There is no need of service suspension", ebufp);
		
			*ret_flistp = r_flistp;
			return 1;		
		}
		if (*(int*)conn_type == 0)
		{
			validate = fm_mso_suspend_service_validate(ctxp, in_flist, ebufp);
			if (validate == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service object suspend not valid as the child service is still active :");	
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
				PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51441", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service suspend not valid as the child service is still active", ebufp);
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
		if(vp)
		{
			indicator = *(int32 *)vp;
		}
		if (*(int *)status == *(int *)old_status)
		{
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service already has suspended status ", ebufp);
		
			*ret_flistp = r_flistp;
			return 1;		
		}
		if ( *(int*)old_prov_status != MSO_PROV_ACTIVE )
		{
			PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51616", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Provisioning status for this service is not active - Cannot suspend", ebufp);
			*ret_flistp = r_flistp;
			return 1;		
		}
	}

	PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to update service status");
	updsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updsvc_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updsvc_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, updsvc_inflistp, PIN_FLD_DESCR, ebufp);
	services_flistp = PIN_FLIST_ELEM_ADD(updsvc_inflistp, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, services_flistp, PIN_FLD_POID, ebufp);
	if(indicator !=2)
	{
		PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);		
		PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS, status, ebufp);

		fm_mso_pre_suspend_updates(ctxp, service_obj, ebufp);	
		if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling Pre_suspend updates", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51430", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - error in Pre_suspend updates ", ebufp);

			*ret_flistp = r_flistp;
			return 0;
		}
	}

//	if ( catv_service == 1 )
//	{
//		PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);		
//		PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS, status, ebufp);
//		// FDP suspend function to update prod status 
//		//fm_mso_fdp_suspend(ctxp, service_obj, ebufp);
//	}
	
	if ( bb_service == 1 )
	{
		new_prov_status = MSO_PROV_IN_PROGRESS;
		inher_flistp = PIN_FLIST_SUBSTR_ADD(services_flistp, PIN_FLD_INHERITED_INFO, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(inher_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &new_prov_status, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_status input list", updsvc_inflistp);
	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, updsvc_inflistp, &updsvc_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&updsvc_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_SERVICES", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service suspend status - PCM_OP_CUST_UPDATE_SERVICES of service poid error", ebufp);
		
		*ret_flistp = r_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_status output flist", updsvc_outflistp);
	PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
    reason_id1 = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_REASON_ID, 1, ebufp);
    domain_reason_id1 = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_REASON_DOMAIN_ID, 1, ebufp);

    if(reason_id1 && domain_reason_id1)
    {
        reason_id = *(int *)reason_id1;
        domain_reason_id = *(int *)domain_reason_id1;
    }
    if ( catv_service == 1 && reason_id && domain_reason_id && reason_id != 0 && domain_reason_id != 0)
    {
        service_write_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, service_write_flistp, PIN_FLD_POID, ebufp);
        catv_info_flistp = PIN_FLIST_SUBSTR_ADD(service_write_flistp, MSO_FLD_CATV_INFO, ebufp );
        PIN_FLIST_FLD_SET(catv_info_flistp, PIN_FLD_REASON_ID, &reason_id, ebufp);
        PIN_FLIST_FLD_SET(catv_info_flistp, PIN_FLD_REASON_DOMAIN_ID, &domain_reason_id, ebufp);
        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, service_write_flistp, &service_write_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling service CATV info updation", ebufp);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_status write fields output list", service_write_oflistp);
    }


	/*************************** Provisoning calling *************************************/
	acct_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	status_flags = CATV_SUSPEND;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
	if ( bb_service == 1 )
	{
		status_flags = DOC_BB_SUSPEND;
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
		fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj,-1, &mso_bb_bp_obj, &mso_bb_obj,ebufp);
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
				PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
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
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_status provisioning input list", provaction_inflistp);
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
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51751", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning action.", ebufp);
		return 0;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_status provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

	/******************************************* notification flist ***********************************************/
/*	status_flags = 7;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_status notification input list", provaction_inflistp);
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
		PIN_FLIST_FLD_PUT(*ret_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51752", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning notification.", ebufp);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_status provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
*/
	/* Pawan: 08-DEC-2014: Added below to Update MSO_PURCHASED_PLAN status*/
	if ( bb_service == 1 )
	{
		curr_status = MSO_PROV_STATE_ACTIVE;
		fm_mso_set_bp_status(ctxp, service_obj, mso_bb_bp_obj, curr_status, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in Updating purchased plan status in BRM.");
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_PUT(*ret_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51670", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Updating purchased plan status in BRM.", ebufp);
			return 0;
		}
	}
	
	/******************************************* reaction fee flist ***********************************************/

	if ( catv_service == 1 )
	{
		apply_fee = fm_mso_apply_reactivation_fee(ctxp, in_flist, ebufp);

		if (apply_fee)
		{
			fm_mso_suspend_service_reactivation_fee(ctxp, in_flist, apply_fee, ebufp);
		}
	}
	
	/*******************************************************************
	* Create Lifecycle event - Start
	*******************************************************************/
	program = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	msg = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_DESCR, 1, ebufp );
	event = MSO_LC_EVENT;
	flg = 1;

	/*******************************************************************
	* Create Lifecycle event - End
	*******************************************************************/
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_success, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "ACCOUNT - Service suspend status completed successfully", ebufp);
	
	*ret_flistp = r_flistp;
	
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	return 2;
}


/**************************************************
* Function: fm_mso_suspend_service_reactivation_fee()
* 
* 
***************************************************/
static void
fm_mso_suspend_service_reactivation_fee(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	int					apply_fee,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*reactive_inflistp = NULL;
	pin_flist_t		*reactive_outflistp = NULL;

	poid_t			*service_obj = NULL;
	time_t			pvt = 0;
	time_t			*effective_t = NULL;

	struct tm		*timeeff;

	int				days = 0;
	int				search_flags = 768;
	int				*old_status = NULL;
	int64			db = -1;

	char			search_template[100] = " select X from /mso_srvc_reactivation_fee where F1 = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_reactivation_fee :", in_flist);	

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

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_reactivation_fee search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_reactivation_fee search service output flist", search_outflistp);
	
	results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	pvt = pin_virtual_time((time_t *)NULL);

	effective_t = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_EFFECTIVE_T, 1, ebufp);
	timeeff = localtime(effective_t);

	if (apply_fee == 2)
	{
		timeeff->tm_mon = 3;
		timeeff->tm_mday = 1;
		timeeff->tm_hour = 0;
		timeeff->tm_min = 0;
		timeeff->tm_sec = 0;
	}
	else if (apply_fee == 3)
	{
		timeeff->tm_mon = 0;
		timeeff->tm_mday = 1;
		timeeff->tm_hour = 0;
		timeeff->tm_min = 0;
		timeeff->tm_sec = 0;
	}
	*effective_t = mktime (timeeff);

	if (results_flistp)
	{
		reactive_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, reactive_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(reactive_inflistp, MSO_FLD_LAST_INACTIVE_T, &pvt, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_reactivation_fee write flds input list", reactive_inflistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, reactive_inflistp, &reactive_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&reactive_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&reactive_outflistp, NULL);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_reactivation_fee write flds output flist", reactive_outflistp);
	}
	else
	{
		reactive_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(reactive_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/mso_srvc_reactivation_fee", -1, ebufp), ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, reactive_inflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, reactive_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(reactive_inflistp, PIN_FLD_EFFECTIVE_T, effective_t, ebufp);
		PIN_FLIST_FLD_SET(reactive_inflistp, MSO_FLD_LAST_INACTIVE_T, &pvt, ebufp);
		PIN_FLIST_FLD_SET(reactive_inflistp, PIN_FLD_DAYS, &days, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_reactivation_fee create input list", reactive_inflistp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, reactive_inflistp, &reactive_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&reactive_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CREATE_OBJ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&reactive_outflistp, NULL);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_reactivation_fee create output flist", reactive_outflistp);
	}
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);		
	}
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&reactive_outflistp, NULL);
	return;
}


/**************************************************
* Function: fm_mso_apply_reactivation_fee()
* 
* 
***************************************************/
EXPORT_OP int
fm_mso_apply_reactivation_fee(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	
	poid_t			*service_obj = NULL;

	int				search_flags = 768;
	int				apply_fee = 0;
	int64			db = -1;
	char			*param_val = NULL;
	char			search_template[100] = " select X from /config/business_params where F1 = V1 and F2 = V2 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return apply_fee;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_reactivation_fee :");	

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );

	if (!service_obj)
	{
		return apply_fee;
	}

	db = PIN_POID_GET_DB(service_obj);

	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, PSIU_BPARAMS_BILLING_PARAMS, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_PARAMS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PARAM_NAME, PSIU_BPARAMS_BILLING_REACTIVATION_FEE, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_reactivation_fee search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return apply_fee;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_reactivation_fee search service output flist", search_outflistp);
	
	results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, 0, 1, ebufp);

	if (results_flistp)
	{	
		args_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_PARAMS, PIN_ELEMID_ANY, 1, ebufp);
		if (args_flistp)
		{
			param_val = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
			apply_fee = 1 + atoi(param_val);
		}		
	}
	else
	{
		apply_fee = 0;
	}
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	return apply_fee;
}

/**************************************************
* Function: fm_mso_suspend_service_validate()
* 
* 
***************************************************/


static int 
fm_mso_suspend_service_validate(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*device_das_inflistp = NULL;
	pin_flist_t		*device_das_outflistp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*list_service_obj = NULL;

	int				search_flags = 768;
	int				*status = NULL;
	int				elem_id = 0;
	int32			child_con_type = 2;
	int32			child_con_type1 = 1;
	int64			db = -1;
	char			*param_val = NULL;
	char			search_template[100] = " select X from /service/catv where F1 = V1 and ( F2 = V2 or F3 = V3 ) ";

	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_validate :");	

	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );
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
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	args_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CONN_TYPE, &child_con_type, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
	args_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CONN_TYPE, &child_con_type1, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_validate search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_suspend_service_validate search service output flist", search_outflistp);
	
	search_flags = 0; 
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		list_service_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
		if (PIN_POID_COMPARE(service_obj,list_service_obj,0,ebufp) != 0)
		{
			status = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_STATUS, 1, ebufp);
			if (*(int*)status == PIN_STATUS_ACTIVE)
			{
				return 0;
			}
		}		
	}
	
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	return 1;
}


/*search for FDP packs and update status to 99(temp value)
so that these cases dont enter spec_cycle_fee customizations */

void fm_mso_pre_suspend_updates(
        pcm_context_t *ctxp,
        poid_t   *service_pdp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *srch_i_flistp = NULL;
        pin_flist_t     *srch_o_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *usage_flistp = NULL;
        pin_flist_t     *wrt_flistp = NULL;
        pin_flist_t     *wrt_oflistp = NULL;
        poid_t          *srch_poid = NULL;
        char            *template = "select X from /purchased_product 1, /product 2 where 1.F1=V1 and 1.F2=V2 and 1.F3= 2.F4 and 2.F5 like V5";
	char		*template_bb = "select X from /purchased_product 1 where 1.F1 = V1 and 1.F2 = V2";
	char 		*service_type = NULL;
        int32           srch_flag = 256;
	int32		bb_service = 0;
        int32           active =1;
        int32           elem_id =0;
        int32           temp_status =99;
        int32           status_flag =0;
        int32           susp_status = 9;
        pin_cookie_t    cookie = NULL;


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(1, "Error before search");
                PIN_ERRBUF_RESET(ebufp);
        }
	
	//Get service type
	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp), MSO_BB) == 0 ))
	{	
		bb_service =1;
	}

	
        PIN_ERR_LOG_MSG(3, "Entering fm_mso_fdp_suspend");
        srch_i_flistp = PIN_FLIST_CREATE(ebufp);
        srch_poid = PIN_POID_CREATE(PIN_POID_GET_DB(service_pdp), "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_poid, ebufp);
        PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        res_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS_FLAGS, NULL, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
        args_flistp =  PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &active, ebufp);
	if(!bb_service)
	{	
		PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);	
	        args_flistp =  PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 3, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	        args_flistp =  PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 4, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 5, ebufp);
        	usage_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_USAGE_MAP, 0, ebufp);
        	PIN_FLIST_FLD_SET(usage_flistp, PIN_FLD_EVENT_TYPE, "%fdp%", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, template_bb, ebufp);
	}

        PIN_ERR_LOG_FLIST(3, "Search input flist", srch_i_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp, &srch_o_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(1, "Error during search", srch_o_flistp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(3, "Search output flist", srch_o_flistp);
        results_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if(results_flistp)
        {
                PIN_ERR_LOG_MSG(3, "Active products found");
                while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_o_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
                {
                        status_flag = *(int32 *) PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_STATUS_FLAGS, 0, ebufp);
                        wrt_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, wrt_flistp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(wrt_flistp, PIN_FLD_STATUS, &temp_status, ebufp);
                        if(status_flag == 1 || bb_service)
                        {
                                PIN_FLIST_FLD_SET(wrt_flistp, PIN_FLD_STATUS_FLAGS, &susp_status, ebufp);
                        }
                        PIN_ERR_LOG_FLIST(3, "write flds input flist", wrt_flistp);
                        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wrt_flistp, &wrt_oflistp, ebufp);
	  	        if (PIN_ERRBUF_IS_ERR(ebufp))
        		{
		                PIN_ERR_LOG_MSG(1, "Error during write fields");
                		goto CLEANUP;
        		}

                        PIN_FLIST_DESTROY_EX(&wrt_flistp, NULL);
                        PIN_FLIST_DESTROY_EX(&wrt_oflistp, NULL);
                }
        }

        CLEANUP:
                PIN_FLIST_DESTROY_EX(&wrt_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&wrt_oflistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);
                return;

}
