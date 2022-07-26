/*******************************************************************
 * File:	fm_mso_cust_terminate_service.c
 * Opcode:	MSO_OP_CUST_TERMINATE_SERVICE 
 * Owner:	Rohini Mogili
 * Created:	08-NOV-2013
 * Description: Caled for changing the status of the service.

	r << xx 1
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 9
	0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
	0 PIN_FLD_DESCR           STR [0] "[11 Nov, 2013 5:44 PM  Bill not paid]\n"
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /service/admin_client 43523 10
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 9
	0 PIN_FLD_STATUS         ENUM [0] 10103
	xx
	xop 11009 0 1


	nap(13252)> xop 11008 0 1
	xop: opcode 11008, flags 0
	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41343 9
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 9
	0 PIN_FLD_DESCR           STR [0] "ACCOUNT - Service status change completed successfully"

 *
 * Modification History: 
 * Modified By: Jyothirmayi Kachiraju
 * Date: 12-FEB-2020
 * Modification details 
 * Description: Code added to store the Termination Reason Type
 * 				and Sub Type in the Life Cycle Events when the
 *				CATV service is getting terminated. Existing
 *				Auto GRV Process for CATV PP Customers is removed.
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_terminate_service.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "ops/device.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_lifecycle_id.h"

PIN_IMPORT  char    *dvbip_wh_code;
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
fm_mso_get_account_info(
    pcm_context_t       *ctxp,
    poid_t          *acnt_pdp,
    pin_flist_t     **ret_flistp,
    pin_errbuf_t        *ebufp);

extern void 
fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);
	
extern void 
fm_mso_cust_get_pp_type(
		pcm_context_t		*ctxp,
        poid_t              *acct_pdp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

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
fm_mso_terminate_service_status(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

extern void 
fm_mso_update_ncr_validty(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT void 
fm_mso_terminate_service_device_deassociate(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp);

static int 
fm_mso_terminate_service_validate(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_errbuf_t		*ebufp);

//static void
//fm_mso_cust_term_lc_event(
//	pcm_context_t		*ctxp,
//	pin_flist_t		*i_flistp,
//	pin_flist_t		*actv_out_flistp,
//	pin_errbuf_t		*ebufp);

static void
fm_mso_get_associated_plans(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**srch_oflistp,
	int32			srvc_type,
	pin_errbuf_t		*ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_TERMINATE_SERVICE 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_terminate_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_terminate_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

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
get_evt_lifecycle_template(
	pcm_context_t			*ctxp,
	int64				db,
	int32				string_id, 
	int32				string_version,
	char				**lc_template, 
	pin_errbuf_t			*ebufp);

/*******************************************************************
 * MSO_OP_CUST_TERMINATE_SERVICE  
 *******************************************************************/
void 
op_mso_cust_terminate_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	
	int					local = LOCAL_TRANS_OPEN_SUCCESS;
	int					*status = NULL;
	int32				status_uncommitted =2;
	
	poid_t				*inp_pdp = NULL;
	
	char            	*prog_name = NULL;
	
	*r_flistpp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_TERMINATE_SERVICE) 
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_terminate_service error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_terminate_service input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_terminate_service()", ebufp);
		return;
	}*/

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//	if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
    if (!(prog_name && (strstr(prog_name,"pin_deferred_act") || strstr(prog_name, "Transfer Subscription"))))
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

	fm_mso_cust_terminate_service(ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp) || (status && *(int*)status == 1)) 
	{	
		if((prog_name && strstr(prog_name,"pin_deferred_act")))
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
							   PIN_ERRCLASS_SYSTEM_DETERMINATE,
							   PIN_ERR_BAD_VALUE, 
							   PIN_FLD_STATUS, 0, 0);
			return;
        	}	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"fm_mso_cust_terminate_service in flistp", i_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_cust_terminate_service error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS &&
				(!(prog_name && (strstr(prog_name,"pin_deferred_act") || strstr(prog_name, "Transfer Subscription")))))
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
                        	if (!(prog_name && strstr(prog_name, "Transfer Subscription")))
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_terminate_service output flist", *r_flistpp);
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_terminate_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*plans_flist = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*user_id = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*service_pdp_before_term = NULL;
	
	//char			*account_no_str = NULL;
	//char			account_no_str[10];
	char			*reason_type = NULL;
	char			*reason_subtype = NULL;
	char			*term_reason_descr = NULL;
	char			*term_reason_descr_before_term = NULL;
	char			*token = NULL;
	int32			status_change_success = 0;
	int32			status_change_failure = 1;
	int64			db = -1;
	int				csr_access = 0;
	int				acct_update = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_terminate_service input flist", i_flistp);	
	
	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
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
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51437", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	if (!account_obj)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51438", ebufp);
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
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51439", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CSR does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}
		else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_terminate_service CSR haas permission to change account information");	
		}
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51440", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	
	/*******************************************************************
	* Validation for PRICING_ADMIN roles  - End
	*******************************************************************/

	/*******************************************************************
	* Service status change - Start
	*******************************************************************/

	service_pdp_before_term = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

         if (service_pdp_before_term && strcmp((char*)PIN_POID_GET_TYPE(service_pdp_before_term), MSO_CATV) == 0)
         {
               	term_reason_descr_before_term = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);

               	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "term_reason_descr_before_term >> ");
               	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, term_reason_descr_before_term);

               	if (term_reason_descr_before_term && strlen(term_reason_descr_before_term) != 0)
               	{
                  	if(strlen(term_reason_descr_before_term) > 120)
                        {
                           	ret_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
                                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51437", ebufp);
                               	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Termination Reason Type-SubType-Remarks cannot have more than 120 Characters", ebufp);
                                goto CLEANUP;
                         }
                }
        }

	acct_update = fm_mso_terminate_service_status(ctxp, i_flistp, &ret_flistp, ebufp);

	if ( acct_update == 0)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_terminate_service no need of status change");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_terminate_service status change successful");	

		/********************************************************************
		 * Changes for adding Termination reason type and subtype for CATV
		 ********************************************************************/
		service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		
		if (service_pdp && strcmp((char*)PIN_POID_GET_TYPE(service_pdp), MSO_CATV) == 0)
		{
			term_reason_descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "term_reason_descr >> ");	
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, term_reason_descr);	
		
			if (term_reason_descr && strlen(term_reason_descr) != 0)
			{
				if(strlen(term_reason_descr) > 120)
				{
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51437", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Termination Reason Type-SubType-Remarks cannot have more than 120 Characters", ebufp);
					goto CLEANUP;
				}
				else if (strlen(term_reason_descr) <= 120)
				{
				   PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_DESCR, ebufp);
				   PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DESCR, term_reason_descr, ebufp);
				   
				   // Returns first token - Temination Reason Type
					reason_type = strtok(term_reason_descr, "-"); 	
					
					if (reason_type && strlen(reason_type) != 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "reason_type >> ");	
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, reason_type);
						PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_TYPE_STR, reason_type, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Input Flist after setting Termination Reason Type >> ");
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status : Reason Type ",i_flistp);
					}
				
					while (reason_type != NULL) 
					{ 
						reason_type = strtok(NULL, "-"); 
						
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "reason_subtype before  >> ");	
						if(reason_type != NULL && strlen(reason_type) != 0)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "reason_subtype >> ");	
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, reason_type);
							PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_VALUE, reason_type, ebufp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Input Flist after setting Termination Sub Reason Type >> ");
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status : Sub Reason Type ",i_flistp);
							break;
						}
					}
				}
			}
		}
		
		/**************************************************************
		 * Calling Lifecycle event creation function
		 **************************************************************/
		fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_TERMINATE_SERVICE, ebufp);
	}
	if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_terminate_service insufficient data provided");	
	}
	if ( acct_update == 4)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_terminate_service no service existing with this service poid");	
	}

	/*******************************************************************
	* Service status change  - End
	*******************************************************************/	

	CLEANUP:	
	*r_flistpp = ret_flistp;
	return;
}


/**************************************************
* Function: fm_mso_terminate_service_status()
* 
* 
***************************************************/
EXPORT_OP int 
fm_mso_terminate_service_status(
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
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*search_flistp = NULL;
	pin_flist_t		*search_res_flistp = NULL;
	pin_flist_t		*ncr_validity_oflist = NULL;
	pin_flist_t		*plans_flist  = NULL;
	pin_flist_t		*pp_flistp  = NULL;


	poid_t		*service_obj = NULL;
	poid_t		*account_obj = NULL;
	poid_t		*search_pdp = NULL;
	poid_t		*mso_pp_obj = NULL;
	poid_t		*mso_bb_bp_obj = NULL;
	poid_t		*mso_bb_obj = NULL;

	int		status_flags = PIN_STATUS_FLAG_MANUAL;
	int		*status = NULL;
	int		*old_status = NULL;
	int		status_change_success = 0;
	int		status_change_failure = 1;
	int		validate = 0;
	int		terminate_status = 0;
	int		*conn_type = NULL;
	int32		*association_flag = NULL;
	int32		db = -1;
	int		search_flags = 512;
	int		prov_status = MSO_PROV_IN_PROGRESS;
    int32   business_type = 0;
    int32   arr_business_type[8];
	char		bp_str[10] = "base plan";
	char            search_template[100] = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
	char		*descr = NULL;
	char		*agreement_no = NULL;
	char		*login = NULL;
	char		login_terminate[255] = "";
	char		*login_orig = NULL;
	char		*stb_idp = NULL;
	time_t           current_t = 0;

	int		*mpp_statusp = NULL;
	poid_t		*mpp_pdp = NULL;
	char		*plan_typep = NULL;
	int		b_elem_id = 0;
	pin_cookie_t	b_cookie  = NULL;
	int		active_bp_count =0;
	int32		*prov_call = NULL;
	int32           *pp_type = NULL;
    int32           *customer_typep = NULL;
	pin_flist_t     *profile_flistp  = NULL;
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status :input flist",in_flist);	

        account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	stb_idp = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_STB_ID, 1, ebufp );
	status = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_STATUS, 1, ebufp );
	association_flag = PIN_FLIST_FLD_GET(in_flist , PIN_FLD_FLAGS, 1, ebufp);
	db = PIN_POID_GET_DB(account_obj);
	if (stb_idp)
	{
		search_flags = 122;
		search_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, search_flistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_SET(search_flistp, MSO_FLD_SERIAL_NO, stb_idp, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, search_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);

		PCM_OP(ctxp, MSO_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&search_flistp, NULL);

		r_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (r_flistp)
		{
			services_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
			if (services_flistp)
			{
				PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_ACCOUNT_OBJ, in_flist, PIN_FLD_POID, ebufp);
                account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
				service_obj = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);	
				PIN_FLIST_FLD_COPY(services_flistp, PIN_FLD_SERVICE_OBJ, in_flist, PIN_FLD_SERVICE_OBJ, ebufp);
			}
		}
		else
		{
			PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Device serial number not found");
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51430", ebufp);	
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Device serial number not found", ebufp);
			*ret_flistp = r_flistp;
			return 3;
		}
	}

	if (!service_obj || !status)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51441", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service terminate status - Service object or status is missing in input", ebufp);
		
		*ret_flistp = r_flistp;
		return 3;
	}

	readsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_POID, service_obj, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_LOGIN, NULL, ebufp);	
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_STATUS, NULL, ebufp);
	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
        {
		args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, MSO_FLD_CATV_INFO, 0, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CONN_TYPE, NULL, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
	}
	else if (service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0)
	{
		args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, MSO_FLD_BB_INFO, 0, ebufp );
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status read service input list", readsvc_inflistp);
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
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51442", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service terminate status - PCM_OP_READ_FLDS of service poid error", ebufp);
		
		*ret_flistp = r_flistp;
		return 4;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status read service output flist", readsvc_outflistp);
	old_status = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_STATUS, 1, ebufp );
	login = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_LOGIN, 1, ebufp );
	login_orig = strdup(login);
	current_t = pin_virtual_time(NULL);
	memset(login_terminate,sizeof(login_terminate),'\0');
	sprintf(login_terminate,"%s-%d",login,current_t);
	
	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
	{
		args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, MSO_FLD_CATV_INFO, 0, 1, ebufp );
		conn_type = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_CONN_TYPE, 1, ebufp );
		agreement_no = strdup((char*)PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp ));
	}
	else if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, MSO_FLD_BB_INFO, 0, 1, ebufp );
		agreement_no = strdup((char*)PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp ));
	}

	if (*(int*)status == *(int*)old_status)
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51443", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service terminate status - There is no need of service termination", ebufp);
		
		*ret_flistp = r_flistp;
		return 1;		
	}

	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		goto TERMINATE;
	}
	if (*(int*)conn_type == 0)
	{
		validate = fm_mso_terminate_service_validate(ctxp, in_flist, ebufp);
		if (validate == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ACCOUNT - Service terminate status - Service object terminate not valid as the child service is still active :");	
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51444", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service terminate status - Service object terminate not valid as the child service is still active", ebufp);
			
			*ret_flistp = r_flistp;
			return 3;
		}
	}

	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
	{
		fm_mso_get_associated_plans(ctxp, in_flist, &plans_flist, CATV, ebufp );

		fm_mso_get_purchased_prod_active(ctxp, in_flist, &pp_flistp, ebufp);
	}



TERMINATE:

	updsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, updsvc_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, updsvc_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, updsvc_inflistp, PIN_FLD_DESCR, ebufp);
	
	//PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_SERVICES, 0, updsvc_inflistp, PIN_FLD_SERVICES, 0, ebufp);
	services_flistp = PIN_FLIST_ELEM_ADD(updsvc_inflistp, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);		
	PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_STATUS, status, ebufp);
	PIN_FLIST_FLD_SET(services_flistp, PIN_FLD_LOGIN, login_terminate, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, services_flistp, PIN_FLD_POID, ebufp);
	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		inher_flistp = PIN_FLIST_SUBSTR_ADD(services_flistp, PIN_FLD_INHERITED_INFO, ebufp);	
		args_flistp = PIN_FLIST_ELEM_ADD(inher_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
		// terminate_status = MSO_CANCEL_STATUS;	
		terminate_status = MSO_PROV_IN_PROGRESS;
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &terminate_status, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status input list", updsvc_inflistp);
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
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51445", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service terminate status - PCM_OP_CUST_UPDATE_SERVICES of service poid error", ebufp);
		
		PIN_FLIST_DESTROY_EX(&plans_flist, NULL);

		*ret_flistp = r_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status output flist", updsvc_outflistp);
	PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);

	/*************************** Updating custom class status *************************************/
	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		
        	db = PIN_POID_GET_DB(service_obj);

	        search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	        search_flistp = PIN_FLIST_CREATE(ebufp);
	        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
	        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

	        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
	        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
		// Pawan-24-10-14: Added below conditions
		args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prov_status, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, bp_str, ebufp);

		result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, (char *)NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, NULL,ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);

        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service search mso purchased plan input list", search_flistp);
	        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service search mso purchased plan output list", search_res_flistp);

	        if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching MSO Purchased plan", ebufp);
			PIN_ERRBUF_RESET(ebufp);
        	        PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
			PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51626", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in searching MSO Purchased Plan class", ebufp);

			*ret_flistp = r_flistp;
			return 0;
	        }
		/***** Pavan Bellala 25-Nov-2015 ***************************
		Update all mso_purchased_plan to terminate except for
		Base plan. Base plan is updated after prov. response
		************************************************************/
		b_elem_id =0; 
		b_cookie = NULL; 
		active_bp_count = 0;
		
		while(( result_flistp = PIN_FLIST_ELEM_GET_NEXT(search_res_flistp,PIN_FLD_RESULTS,
							&b_elem_id, 1, &b_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_pur_plan result flist",result_flistp);
			mpp_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp);
			mpp_statusp = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_STATUS,0,ebufp);
			plan_typep = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_DESCR, 0,ebufp);
			if(mpp_statusp && (*mpp_statusp != MSO_PROV_STATE_TERMINATED))
			{
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"mso_pur_plan is not terminated",mso_pp_obj);
				if( plan_typep == NULL || (plan_typep && (!strstr(plan_typep,MSO_BB_BP))))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Not base plan poid.Set status to terminated");
					terminate_status = MSO_PROV_STATE_TERMINATED;
				} 
				else 
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Base plan poid.Set status to in-progress");
					terminate_status = MSO_PROV_IN_PROGRESS;
					mso_pp_obj = PIN_POID_COPY(mpp_pdp,ebufp);
					if(*mpp_statusp == MSO_PROV_STATE_ACTIVE)
					{
						active_bp_count++;
					}
				}				
				/*** Double check if active Base plans is more than 1 --Invalid condition ****/
				if(active_bp_count > 1 )
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"More than 1 active base plan-Invalid",search_res_flistp);
					PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
					r_flistp = PIN_FLIST_CREATE(ebufp);	
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51628", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error:More than one active base plan encountered", ebufp);
					
					*ret_flistp = r_flistp;
					return 0;
				}

				updsvc_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(updsvc_inflistp, PIN_FLD_POID, mpp_pdp, ebufp);
				PIN_FLIST_FLD_SET(updsvc_inflistp, PIN_FLD_STATUS, &terminate_status, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
					"fm_mso_terminate_service mso purchase plan status update input list", updsvc_inflistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, updsvc_inflistp, &updsvc_outflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
					"fm_mso_terminate_service mso purchase plan status update out flist", updsvc_outflistp);
				PIN_FLIST_DESTROY_EX(&updsvc_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51627", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in updating MSO Purchased Plan class", ebufp);
					
					*ret_flistp = r_flistp;
					return 0;
				}
			}
		}	

        	if (PIN_FLIST_ELEM_COUNT(search_res_flistp,PIN_FLD_RESULTS,ebufp)<=0)
        	{
        	        PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
			goto PROV;
		} 
		else 
		{
			result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp,PIN_FLD_RESULTS,0,0,ebufp);
		}
		/*******************************************************
		 Update NON currecy resource validity -start
		*******************************************************/
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PROGRAM_NAME, "set_validity_by_terminate_service", ebufp);

		fm_mso_update_ncr_validty(ctxp, result_flistp, &ncr_validity_oflist, ebufp);
		if (ncr_validity_oflist)
		{
			descr = PIN_FLIST_FLD_GET(ncr_validity_oflist, PIN_FLD_DESCR, 1, ebufp);
			if (descr && strcmp(descr ,"success") != 0)
			{
				return 0;
			}
		}
		/*******************************************************
		 Update NON currecy resource validity -end
		*******************************************************/
        	PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);

		//Get associated plans before destroying input flist
		fm_mso_get_associated_plans(ctxp, search_flistp, &plans_flist, BB, ebufp );
		PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
		
	}
PROV:	
	/*************************** Updating custom class status *************************************/

	/*************************** Provisoning calling *************************************/
	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		status_flags = DOC_BB_END_SUBS;
	} 
	else 
	{
		status_flags = CATV_TERMINATE;
	}
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_DELETE_STATUS, old_status, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_LOGIN, login_orig, ebufp);
	//Pawan 24-10-2014: commented below block to reuse the value from mso_pp from above 
        /*fm_mso_cust_get_bp_obj( ctxp, account_obj, service_obj, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
        if ( mso_bb_obj && !PIN_POID_IS_NULL(mso_bb_obj))
        {
                PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_bb_obj, ebufp);
        } */
	PIN_FLIST_FLD_PUT(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_pp_obj, ebufp);

	if (status_flags == CATV_TERMINATE && pp_flistp)
	{
		b_elem_id =0; b_cookie = NULL; active_bp_count = 0;
		search_flags = 1;
		while ((result_flistp = PIN_FLIST_ELEM_GET_NEXT(pp_flistp, PIN_FLD_RESULTS,
			&b_elem_id, 1, &b_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			args_flistp = PIN_FLIST_ELEM_ADD(provaction_inflistp, PIN_FLD_PRODUCTS, active_bp_count, ebufp);
			PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS_FLAGS, &search_flags, ebufp);
			active_bp_count++;
		}
	}

	/* Write USERID, PROGRAM_NAME in buffer - start */
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
	/* Write USERID, PROGRAM_NAME in buffer - end */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status provisioning input list", provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);

	if(provaction_outflistp)
        {
               prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
        }
	if (PIN_ERRBUF_IS_ERR(ebufp) || (prov_call && *prov_call == 1))
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION in flistp", provaction_inflistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51446", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service terminate status - MSO_OP_PROV_CREATE_ACTION error", ebufp);

		PIN_FLIST_DESTROY_EX(&plans_flist, NULL);
		
		*ret_flistp = r_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

	/******************************************* notification flist ***********************************************/
	status_flags = 9;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status notification input list", provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51441", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service terminate status - MSO_OP_NTF_CREATE_NOTIFICATION error", ebufp);
		
		PIN_FLIST_DESTROY_EX(&plans_flist, NULL);
		*ret_flistp = r_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_status notification output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);	
	if (pp_flistp) PIN_FLIST_DESTROY_EX(&pp_flistp, NULL);	

	/*************************** Device deassociation calling *************************************/
	/*** Pavan Bellala 21-Nov-2015 Commented this section as device should be released only 
	 ***** through GRV for BB*/
	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
	{
		if((association_flag) && (*association_flag == 1))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " No need of device association ");
		}
		else
		{	
			/************************************************************************
			 * CATV GRV Process AD == Removed Auto GRV for PP Customers.
			 * From now on, the Device should be released only after manual GRV
			 * for CATV PP Customers
			 ***********************************************************************/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Before calling fm_mso_cust_get_pp_type ");
			fm_mso_cust_get_pp_type(ctxp, account_obj, &profile_flistp, ebufp);
			
			if(profile_flistp != NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Printing profile_flistp... ");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pp_type profile_flistp", profile_flistp);
				pp_type = PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_PP_TYPE, 1, ebufp);
                customer_typep = PIN_FLIST_FLD_GET(profile_flistp, PIN_FLD_CUSTOMER_TYPE, 0, ebufp);
                PIN_FLIST_FLD_COPY(profile_flistp, PIN_FLD_CUSTOMER_TYPE, in_flist, PIN_FLD_CUSTOMER_TYPE, ebufp);

                fm_mso_get_account_info(ctxp, account_obj, &pp_flistp, ebufp);   
                business_type = *(int32 *)PIN_FLIST_FLD_GET(pp_flistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
                num_to_array(business_type, arr_business_type);

				if(pp_type != NULL && (*pp_type == PP_TYPE_SECONDARY || *customer_typep == 48 || arr_business_type[7] > 1))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " CATV Secondary Point Customer, Disassociate Device as per existing process ");
					// For CATV Secondary Point Customers, Auto GRV is applicable as per existing process.
					fm_mso_terminate_service_device_deassociate(ctxp, in_flist, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling device de-associate ", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						r_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_failure, ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51441", ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service terminate status - fm_mso_terminate_service_device_deassociate error", ebufp);
						PIN_FLIST_DESTROY_EX(&plans_flist, NULL);
						
						*ret_flistp = r_flistp;
						return 0;
					}
				}

	            if (pp_flistp) PIN_FLIST_DESTROY_EX(&pp_flistp, NULL);	

				if(pp_type != NULL && *pp_type == PP_TYPE_PRIMARY )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Primary Point CATV Customer, therefore Manual GRV is applicable... ");
				}
			}
		}
	}

	/* delete AMC deferred Actions Start */
	fm_mso_cust_bb_hw_delete_deferred_actions(ctxp, account_obj, ebufp);
	/* delete AMC deferred Actions End */

	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_change_success, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "ACCOUNT - Service terminate status completed successfully", ebufp);

	PIN_FLIST_CONCAT(r_flistp, plans_flist, ebufp);
	PIN_FLIST_DESTROY_EX(&plans_flist, NULL);
	PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
	
	*ret_flistp = r_flistp;
	
	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	return 2;
}

/**************************************************
* Function: fm_mso_terminate_service_device_deassociate()
* 
* 
***************************************************/
void
fm_mso_terminate_service_device_deassociate(
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

	int			search_flags = 768;
	int			apply_fee = 0;
	int			elem_id = 0;
	int64			db = -1;
	char			*param_val = NULL;
    char            *customer_typep = NULL;
	char			search_template[100] = " select X from /device where F1 = V1 ";

	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_device_deassociate :");	

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );

	if (!service_obj)
	{
		return;
	}

	db = PIN_POID_GET_DB(service_obj);

    customer_typep = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_CUSTOMER_TYPE, 1, ebufp );
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);

	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, 1, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_device_deassociate search device input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_device_deassociate search device output flist", search_outflistp);
	
	search_flags = 0; 
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		device_das_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, device_das_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(device_das_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, device_das_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, device_das_inflistp, PIN_FLD_DESCR, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(device_das_inflistp, PIN_FLD_SERVICES, 0, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, args_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_device_deassociate device remove input list", device_das_inflistp);
		PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE , 0, device_das_inflistp, &device_das_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&device_das_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_DEVICE_ASSOCIATE device remove", ebufp);
			PIN_FLIST_DESTROY_EX(&device_das_outflistp, NULL);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_device_deassociate device remove output flist", device_das_outflistp);
		PIN_FLIST_DESTROY_EX(&device_das_outflistp, NULL);

        if (customer_typep && *customer_typep == 48)
        {
            device_das_inflistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, device_das_inflistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_DEVICE_ID, device_das_inflistp, PIN_FLD_DEVICE_ID, ebufp);
            PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, device_das_inflistp, PIN_FLD_USERID, ebufp);
            PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, device_das_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
            PIN_FLIST_FLD_SET(device_das_inflistp, PIN_FLD_ACCOUNT_NO, dvbip_wh_code, ebufp);

            PCM_OP(ctxp, MSO_OP_DEVICE_SET_ATTR, 0, device_das_inflistp, &device_das_outflistp, ebufp);
            PIN_FLIST_DESTROY_EX(&device_das_inflistp, NULL);
            if (PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in MSO_OP_DEVICE_SET_ATTR device move", ebufp);
                break;
            }
            PIN_FLIST_DESTROY_EX(&device_das_outflistp, NULL);
        }

	}
	
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	return;
}

/**************************************************
* Function: fm_mso_terminate_service_validate()
* 
* 
***************************************************/

static int 
fm_mso_terminate_service_validate(
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
	int64			db = -1;
	char			*param_val = NULL;
	char			search_template[100] = " select X from /service where F1 = V1 and F2.type = V2 ";

	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_validate :");	

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
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, service_obj, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_validate search service input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_terminate_service_validate search service output flist", search_outflistp);
	
	search_flags = 0; 
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		list_service_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
		if (PIN_POID_COMPARE(service_obj,list_service_obj,0,ebufp) != 0)
		{
			status = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_STATUS, 1, ebufp);
			if (*(int*)status != PIN_STATUS_CLOSED)
			{
				return 0;
			}
		}		
	}
	
	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	return 1;
}


//static void
//fm_mso_cust_term_lc_event(
//	pcm_context_t		*ctxp,
//	pin_flist_t		*in_flistp,
//	pin_flist_t		*out_flistp,
//	pin_errbuf_t		*ebufp)
//{
//	pin_flist_t		*lc_iflistp = NULL;
//	pin_flist_t		*lc_temp_flistp = NULL;
//	pin_flist_t		*acnt_info = NULL;
//	pin_flist_t		*plan_list = NULL;
//	pin_flist_t		*plans = NULL;
//
//	poid_t			*acc_pdp = NULL;
//	poid_t			*service_pdp = NULL;
//		
//	int64			db = 0;
//	int32			elem_id = 0;
//	int32			srvc_type = 0;
//	int32			srch_flag = 256;
//	int			string_id = ID_TERMINATE_SERVICE;
//
//	char			*event = "/event/activity/mso_lifecycle/terminate_service";
//	char			*action_name = "SERVICE_TERMINATION";
//	char			*lc_template = NULL;
//	char			msg[255]="";
//	char			srvc_type_str[20]="";
//	char			*prog_name = NULL;
//	char			*reason = NULL;
//	char			*account_no = NULL;
//	char			*agreement_no = NULL;
//
//	void			*vp = NULL;
//
//	pin_cookie_t		cookie = NULL;
//	pin_cookie_t		previous_cookie = NULL;
//
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//		return;
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_term_lc_event", in_flistp );
//	/* Get Lifecycle event template */
//
//	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
//	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
//	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//	agreement_no = PIN_FLIST_FLD_GET(out_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
//	reason =  PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_DESCR, 1, ebufp);
//
//	if (service_pdp && strcmp((char*)PIN_POID_GET_TYPE(service_pdp),MSO_CATV)==0 )
//	{
//		srvc_type = CATV;
//		strcpy(srvc_type_str,"CATV");
//	}
//	else
//	{
//		srvc_type = BB;
//		strcpy(srvc_type_str,"BROADBAND");
//	}
//
//	db = PIN_POID_GET_DB(acc_pdp);
//	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
//
//	if (acc_pdp)
//	{
//		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
//		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
//	}
//
//	if (srvc_type == CATV )
//	{
//	}
//
//	sprintf(msg,lc_template,srvc_type_str, agreement_no, reason, account_no);
//	lc_iflistp = PIN_FLIST_CREATE(ebufp);
//	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_SERVICE_TERMINATE, 0, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR,       reason, ebufp);
//
//	 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "0000");
//
//	while((plans = PIN_FLIST_ELEM_GET_NEXT(out_flistp, PIN_FLD_PLAN,
//		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
//	{
//		PIN_FLIST_FLD_COPY(plans, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_PLAN_OBJ, ebufp);
//		PIN_FLIST_FLD_COPY(plans, PIN_FLD_CODE, lc_temp_flistp, PIN_FLD_CODE, ebufp);
//
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_term_lc_event gen_lifecycle_evt Flist:", lc_iflistp);
//
//		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
//			msg, action_name, event, lc_iflistp, ebufp);
//
//		PIN_FLIST_ELEM_DROP(out_flistp, PIN_FLD_PLAN, elem_id, ebufp );
//		cookie = previous_cookie;
//
//		previous_cookie = cookie;
//	}
//	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
//	return;
//}


static void
fm_mso_get_associated_plans(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**srch_oflistp,
	int32			srvc_type,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*service_pdp = NULL;
	int64			db =1;
		

	int32			status_actve = 1;
	int32			status_inactve = 2;
	int32			srch_flag = 256;
	char			*template_catv = "select X from /plan 1, /purchased_product  2 where  1.F1 = 2.F2 and 2.F3.id = V3 and ( 2.F4= V4 or 2.F5 = V5 ) ";
	char			*template_bb =   "select X from /plan 1, /mso_purchased_plan 2 where  1.F5 = 2.F6 and 2.F1 = V1 and 2.F2 = V2 and 2.F3 = V3 and 2.F4 = V4 order by mso_purchased_plan_t.created_t desc";


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_associated_plans", in_flistp );
	/* Get Lifecycle event template */

	if (srvc_type == CATV )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV");
		service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
		/*Search Plans from purchased product*/
		srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_catv , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PLAN_OBJ, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &status_actve, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &status_inactve, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CODE, NULL, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_associated_plans SEARCH input flist", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, srch_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_associated_plans SEARCH output flist", *srch_oflistp);
		PIN_FLIST_FLD_DROP(*srch_oflistp,  PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_RENAME(*srch_oflistp,  PIN_FLD_RESULTS, PIN_FLD_PLAN, ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_associated_plans SEARCH output flist", *srch_oflistp);
   	}
	else if (srvc_type == BB)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "BB");
		arg_flist = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_ARGS, 6, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PLAN_OBJ, NULL, ebufp);

		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TEMPLATE, template_bb , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_RESULTS, 0, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CODE, NULL, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_associated_plans SEARCH input flist", in_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, in_flistp, srch_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_associated_plans SEARCH output flist", *srch_oflistp);
		PIN_FLIST_FLD_DROP(*srch_oflistp,  PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_RENAME(*srch_oflistp,  PIN_FLD_RESULTS, PIN_FLD_PLAN, ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_associated_plans SEARCH output flist", *srch_oflistp);
	}
	return;
}

