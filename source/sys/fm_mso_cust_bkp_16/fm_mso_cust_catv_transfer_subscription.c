/*******************************************************************
 * File:	fm_mso_cust_catv_transfer_subscription.c
 * Opcode:	MSO_OP_CUST_CATV_TRANSFER_SUBSCRIPTION 
 * Owner:	SREEKANTH Y	
 * Created:	20-OCT-2016	
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * This opcode is for implementing the transfer subscription from one account
 * to another account and will allow only child connection.
 * 
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_catv_transfer_subscription.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_lifecycle_id.h"
#include "mso_device.h"
#include "mso_utils.h"
#include "mso_errs.h"

/*******************************************************************
 * MSO_OP_CUST_CATV_TRANSFER_SUBSCRIPTION 
 *******************************************************************/

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


EXPORT_OP void
op_mso_cust_catv_transfer_subscription(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_catv_transfer_subscription(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static int32 
fm_mso_perform_lco_check(
	pcm_context_t		*ctxp,
	poid_t			*act_pdp,
	poid_t			*new_act_pdp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_perform_subscription_transfer(
	pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
	poid_t                  *act_pdp,
	poid_t                  *new_act_pdp,
	poid_t                  *srvc_pdp,
	char                    *device_id,
	char                    *vc_id,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp);

static void
fm_mso_prepare_activation_data(
        pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
        poid_t                  *act_pdp,
        poid_t                  *new_act_pdp,
        poid_t                  *new_srvc_pdp,
        char			*device_id,
        char			*vc_id,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_CUST_CATV_TRANSFER_SUBSCRIPTION  
 *******************************************************************/
void 
op_mso_cust_catv_transfer_subscription(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	poid_t			*inp_pdp = NULL;
	int32 			status_uncommitted = 1;
	int32			*status = NULL;
	int                     local = LOCAL_TRANS_OPEN_SUCCESS;
	

	*r_flistpp		= NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_CATV_TRANSFER_SUBSCRIPTION ) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_catv_transfer_subscription error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_catv_transfer_subscription input flist", i_flistp);

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
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_catv_transfer_subscription(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_catv_transfer_subscription error", ebufp);
		if( local == LOCAL_TRANS_OPEN_SUCCESS)
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "50095", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Transfer is failed due to BRM error", ebufp);
		return;
	}
	else
	{
		if ( r_flistpp != NULL){
			status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 0, ebufp);
			if( local == LOCAL_TRANS_OPEN_SUCCESS  && (status != NULL && *status == 0))
			{
				fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
			}
			else{
				fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
				if (status && *status == 1){
					
				}
			} 
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_catv_transfer_subscription output flist", *r_flistpp);
	}
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_catv_transfer_subscription(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*device_iflistp = NULL;
	pin_flist_t		*device_oflistp = NULL;
	pin_flist_t		*dvc_srvc_flistp = NULL;
	pin_flist_t		*dvc_stb_flistp = NULL;
	pin_flist_t		*prod_iflistp = NULL;
	pin_flist_t		*prod_oflistp = NULL;
	pin_flist_t		*result_flistp= NULL;
	pin_flist_t		*read_iflistp= NULL;
	pin_flist_t		*read_oflistp= NULL;
	pin_flist_t		*srvc_flistp= NULL;
	pin_flist_t		*acct_flistp = NULL;

	poid_t			*act_pdp = NULL;
	poid_t			*new_act_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*d_act_pdp = NULL;
	poid_t			*d_srvc_pdp = NULL;
	poid_t			*dvc_pdp = NULL;
	poid_t			*vc_pdp = NULL;

	int32			*act_status = NULL; 
	int32                   device_search_type = MSO_FLAG_SRCH_BY_ID;
	int32                   transfer_failure = 1;
	int32                   conn_type = -1;
	int32			lco_status = 0; 
	int32			elem_id = 0;
	int			transfer_flag = 0;

	char			*device_id = NULL;
	char			*acct_no = NULL;
	char			*vc_id = NULL;
	char			error_code[10];
	char			error_msg[250];

	pin_cookie_t		cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_catv_transfer_subscription input flist", i_flistp);
	

	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
	acct_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);

	if (!act_pdp || !acct_no || !device_id){
		strcpy(error_code, "50000");
		if (!act_pdp)
			strcpy(error_msg, "Accoutn OBJ of OLD account is mising in input flist");
		else if (!acct_no)
			strcpy(error_msg, "New Account No is missing in input flist");
		else if (!device_id)
			strcpy(error_msg, "Child STB is missing in input flist");

		fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
		*r_flistpp = ret_flistp;
		return;
	}
	// Get new account details from account no provided
	fm_mso_get_acnt_from_acntno(ctxp, acct_no, &acct_flistp, ebufp);
	if (acct_flistp){
		new_act_pdp = PIN_FLIST_FLD_GET(acct_flistp, PIN_FLD_POID, 1,ebufp);
	}
	// Perform LCO check whether both accounts belongs to same LCO
	lco_status = fm_mso_perform_lco_check(ctxp, act_pdp, new_act_pdp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in getting LCO details", ebufp);
		PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
		return;
	}
	if ( !lco_status){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"LCO is not matching");
		strcpy(error_code, "60000");
		strcpy(error_msg, "LCO details are not matching");
		fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
		*r_flistpp = ret_flistp;
		PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
		return;
	}
	else{
		// Get device details and perform package and connection details 
		/*fm_mso_utils_get_catv_children(ctxp, act_pdp, &srvc_flistp, ebufp);
		if (srvc_flistp){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Account has child connections", ebufp);
			strcpy(error_code, "51088");
			strcpy(error_msg, "Device is parent STB, please do swap and retry");
			fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
			*r_flistpp = ret_flistp;
			PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
			return;
		}*/
		device_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(device_iflistp, PIN_FLD_POID, act_pdp, ebufp );
		PIN_FLIST_FLD_SET(device_iflistp, PIN_FLD_DEVICE_ID, device_id, ebufp );
		PIN_FLIST_FLD_SET(device_iflistp, PIN_FLD_SEARCH_TYPE, &device_search_type, ebufp );

		fm_mso_get_device_info(ctxp, device_iflistp, &device_oflistp, ebufp );
		PIN_FLIST_DESTROY_EX(&device_iflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in getting the STB details", ebufp);
			PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
			return;
		}
		if (!device_oflistp)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"STB details not present in BRM", ebufp);
			strcpy(error_code, "51090");
			strcpy(error_msg, "STB details not present in BRM");
			fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
			*r_flistpp = ret_flistp;
			PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
			return;
		}
		else{
			dvc_pdp = PIN_FLIST_FLD_GET(device_oflistp, PIN_FLD_POID, 1, ebufp);
			if ( dvc_pdp && strcmp(PIN_POID_GET_TYPE(dvc_pdp), "/device/stb") != 0 ){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Provide device id is  not a STB");
				strcpy(error_code, "51088");
				strcpy(error_msg, "Provided  DEVICE ID is not a STB device, please enter valid STB");
				fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
				*r_flistpp = ret_flistp;
				PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
				PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
				return;
			}
			dvc_srvc_flistp = PIN_FLIST_ELEM_GET(device_oflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
			dvc_stb_flistp = PIN_FLIST_ELEM_GET(device_oflistp, MSO_FLD_STB_DATA, PIN_ELEMID_ANY, 1, ebufp);
			if(!dvc_srvc_flistp){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"STB is not associated with any account");
				strcpy(error_code, "51088");
				strcpy(error_msg, "STB is not associated with any account");
				fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
				*r_flistpp = ret_flistp;
				PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
				PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
				return;
			}
			else{ 
				d_act_pdp = PIN_FLIST_FLD_GET(dvc_srvc_flistp,  PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
				d_srvc_pdp = PIN_FLIST_FLD_GET(dvc_srvc_flistp,  PIN_FLD_SERVICE_OBJ, 1, ebufp);
				if ((act_pdp && d_act_pdp ) && PIN_POID_COMPARE(act_pdp, d_act_pdp, 0, ebufp) != 0 ){
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"STB doesn't belong to same account, no transfer allowed");
					strcpy(error_code, "51088");
					strcpy(error_msg, "STB doesn't belong to same account, no transfer allowed");
					fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
					*r_flistpp = ret_flistp;
					PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
					return;
				}
				conn_type = fm_mso_catv_conn_type(ctxp, d_srvc_pdp, ebufp);
				fm_mso_utils_get_catv_children(ctxp, act_pdp, &srvc_flistp, ebufp);
				if (srvc_flistp && PIN_FLIST_ELEM_COUNT(srvc_flistp, PIN_FLD_RESULTS, ebufp) > 0 ){
					if (conn_type == 0){
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Account has child connections, so parent connection will not be allowed");
						strcpy(error_code, "51088");
						strcpy(error_msg, "Transfer is not allowed for main connections, please perform SWAP stagging and then retry");
						fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
						*r_flistpp = ret_flistp;
						PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
						return;
					}
					else //Aif (conn_type == 1){ // proceed to perform transfer
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"STB is child connection, can transfer");
						transfer_flag = 1;
					}
				}
				else{ // if no child connections, allow to transfer main connection
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account has no child connections, so main connection transfer can be allowed");
					transfer_flag = 1; // Set Transfer flag
				}
				if (transfer_flag){
					//Get VC details to pair
					if (dvc_stb_flistp){
						vc_pdp = PIN_FLIST_FLD_GET(dvc_stb_flistp, MSO_FLD_VC_OBJ, 1, ebufp);
						if (vc_pdp && strcmp(PIN_POID_GET_TYPE(vc_pdp), "/device/vc") == 0){
							read_iflistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, vc_pdp, ebufp );
							PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_DEVICE_ID, NULL, ebufp );
							PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_iflistp, &read_oflistp, ebufp);
							PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);
							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
									"Error in getting the /device/vc details", ebufp);
								PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
								PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
								return;
							}
							if (read_oflistp != NULL){
								vc_id = PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_DEVICE_ID, 1, ebufp);	
							}
						}
					}
					/*
					else if(!dvc_stb_flistp && !vc_id){
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"VC is not paired with given STB");
						strcpy(error_code, "51094");
						strcpy(error_msg, "VC details not found with given STB, no transferr allowed");
						fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
						*r_flistpp = ret_flistp;
						PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
						return;
					}*/
					fm_mso_perform_subscription_transfer(ctxp, i_flistp, act_pdp, new_act_pdp, d_srvc_pdp, device_id, vc_id, &ret_flistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
							"Error in Subscription Transfer", ebufp);
						PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
						return;
					}
					if( ret_flistp ){
						act_status = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 1, ebufp);
						if ( act_status && *act_status == 0){
							fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_SUBSCRIPTION_TRANSFER, ebufp);
							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
									"Error in Life Cycle creation", ebufp);
								PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &transfer_failure, ebufp);
								PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
								PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
								return;
							}
						}
					}
				}
			}
		}
	}

	*r_flistpp = ret_flistp;
	PIN_FLIST_DESTROY_EX(&device_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
	return;
}

static void
fm_mso_perform_subscription_transfer(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        poid_t                  *act_pdp,
        poid_t                  *new_act_pdp,
        poid_t                  *srvc_pdp,
        char			*device_id,
        char			*vc_id,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srvc_iflistp = NULL;
	pin_flist_t		*srvc_oflistp = NULL;
	pin_flist_t		*srvc_arry_flistp = NULL;
	pin_flist_t		*prod_iflistp = NULL;
	pin_flist_t		*prod_oflistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*srvc_flistp = NULL;
	pin_flist_t		*activate_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*planlist_flistp = NULL;

	poid_t			*new_srvc_pdp = NULL;
	poid_t			*prod_pdp = NULL;

	int32			*flag = NULL;
	int32			*status = NULL;
	int32			*act_status = NULL;
	int			grace_flag = 0;
	int			elem_id  = 0;
	int			status_closed = 10103;
	int			transfer_failure = 1;

	char			error_code[10];
	char 			error_msg[200];

	pin_cookie_t		cookie = NULL;
	pin_flist_t		*term_srvc_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	// Check the products in grace period
	prod_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(prod_iflistp, PIN_FLD_POID, act_pdp, ebufp );
	PIN_FLIST_FLD_SET(prod_iflistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
	fm_mso_get_purchased_prod_active(ctxp, prod_iflistp, &prod_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&prod_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in getting the product details", ebufp);
		return;
	}
	if (prod_oflistp != NULL && (PIN_FLIST_ELEM_COUNT(prod_oflistp, PIN_FLD_RESULTS, ebufp) > 0 )){
		while ((result_flistp = PIN_FLIST_ELEM_GET_NEXT(prod_oflistp, PIN_FLD_RESULTS, &elem_id, 1,                                                                                                                      &cookie, ebufp)) != NULL ){
			prod_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
			// Check for BASE PACK GRACE period
			if (prod_pdp && !fm_mso_catv_pt_pkg_type(ctxp, prod_pdp, ebufp)){
				flag = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp);	
				if ( flag && (*flag == 65535 || *flag == 4096 || *flag == 4095)){
					grace_flag = 1; //Grace period is on	
					break;
				}
			}
		}
	}
	if ( grace_flag == 1){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Package is in grace period, transfer is not allowed");
		strcpy(error_code, "51094");
		strcpy(error_msg, "Package is in grace period, transfer is not allowed");
		fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
		*r_flistpp = ret_flistp;
		PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
		return;
	}
	else{// Proceed to terminate the service
		srvc_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(srvc_iflistp, PIN_FLD_POID, act_pdp, ebufp );
		PIN_FLIST_FLD_SET(srvc_iflistp, PIN_FLD_PROGRAM_NAME, "Transfer Subscription", ebufp);
		PIN_FLIST_FLD_SET(srvc_iflistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
		PIN_FLIST_FLD_SET(srvc_iflistp, PIN_FLD_STATUS, &status_closed, ebufp );
		PIN_FLIST_FLD_SET(srvc_iflistp, PIN_FLD_DESCR, "Transfer Subscription to other account", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, srvc_iflistp, PIN_FLD_USERID, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_catv_transfer_subscription terminate service in flist", srvc_iflistp);
		PCM_OP(ctxp, MSO_OP_CUST_TERMINATE_SERVICE, 0, srvc_iflistp, &srvc_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srvc_iflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in terminating the service", ebufp);
			PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_catv_transfer_subscription terminate service out flist", srvc_oflistp);
		if (srvc_oflistp != NULL){
			status = PIN_FLIST_FLD_GET(srvc_oflistp, PIN_FLD_STATUS, 1, ebufp);
			if (status && *status == 0){//Proceed to transfer the STB to new account
				fm_mso_utils_get_catv_main(ctxp, new_act_pdp, &srvc_flistp, ebufp);
				if (srvc_flistp && PIN_FLIST_ELEM_COUNT(srvc_flistp, PIN_FLD_RESULTS, ebufp) > 0){
					tmp_flistp = PIN_FLIST_ELEM_GET(srvc_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
					new_srvc_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 1, ebufp);
					fm_mso_prepare_activation_data(ctxp, i_flistp, act_pdp, new_act_pdp, new_srvc_pdp, device_id, vc_id, &activate_flistp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prepare_activation_data in flist", activate_flistp);
					if(activate_flistp){
						planlist_flistp = PIN_FLIST_SUBSTR_GET(activate_flistp, PIN_FLD_PLAN_LIST_CODE, 1, ebufp);
						if (!planlist_flistp){
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Additional PLAN details not found");
							strcpy(error_code, "51094");
							// First check for active product on new accounts found
							act_status = PIN_FLIST_FLD_GET(activate_flistp, PIN_FLD_STATUS, 1, ebufp);
							if (act_status && *act_status == 2){
								strcpy(error_msg, "No active packages found on new account's main connection. Transfer failed");
							}
							else if (act_status && *act_status == 1){
								strcpy(error_msg, "No DEMO pack is found for new account's state. Transfer failed");
							}
							fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
							*r_flistpp = ret_flistp;
							PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&srvc_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&activate_flistp, NULL);
							return;
						}
					}
					PCM_OP(ctxp, MSO_OP_CUST_ACTIVATE_CUSTOMER, 0, activate_flistp, &ret_flistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
							"Error in activation on new account", ebufp);
						PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&srvc_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&activate_flistp, NULL);
						return;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_transfer_activation_data out flist", ret_flistp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_transfer_activation_data terminated flist", srvc_oflistp);
					if (ret_flistp){
						// Add old account service details to the output
						act_status = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 1, ebufp);
						if (act_status && *act_status == 0){
							term_srvc_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, MSO_FLD_SERVICE_TERMINATE, 0, ebufp);
							PIN_FLIST_FLD_COPY(srvc_oflistp, PIN_FLD_POID, term_srvc_flistp, PIN_FLD_POID, ebufp);
							PIN_FLIST_FLD_COPY(srvc_oflistp, MSO_FLD_AGREEMENT_NO, term_srvc_flistp, MSO_FLD_AGREEMENT_NO, ebufp);
							PIN_FLIST_FLD_COPY(srvc_oflistp, PIN_FLD_SERVICE_OBJ, term_srvc_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
						}
					}
				}
				else{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "New account doesn't have MAIN connection");
					strcpy(error_code, "51094");
					strcpy(error_msg, "Transfer is not allowed as New Account doesn't have MAIN connection");
					fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
					*r_flistpp = ret_flistp;
					PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&srvc_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
					return;
				}
			}
			else{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service Termination is failed");
				strcpy(error_code, "51094");
				strcpy(error_msg, "Subscription Transfer is failed due to package cancellation issue on old account");
				fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &ret_flistp, error_code, error_msg, ebufp);
				*r_flistpp = ret_flistp;
				PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
				PIN_FLIST_DESTROY_EX(&srvc_oflistp, NULL);
				return;
			}
		}
	}
	*r_flistpp  = ret_flistp;
	
	PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&activate_flistp, NULL);
	return;
}

static void
fm_mso_prepare_activation_data(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        poid_t                  *act_pdp,
        poid_t                  *new_act_pdp,
        poid_t                  *new_srvc_pdp,
        char			*device_id,
        char			*vc_id,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srvc_iflistp = NULL;
	pin_flist_t		*srvc_oflistp = NULL;
	pin_flist_t		*prod_iflistp = NULL;
	pin_flist_t		*prod_oflistp = NULL;
	pin_flist_t		*read_iflistp = NULL;
	pin_flist_t		*read_oflistp = NULL;
	pin_flist_t		*srvc_array_flistp = NULL;
	pin_flist_t		*dvc_array_flistp = NULL;
	pin_flist_t		*plan_list_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*addl_plan_flistp = NULL;
	pin_flist_t		*catv_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;

	poid_t			*plan_pdp = NULL;
	poid_t			*prod_pdp = NULL;
	
	char			*code = NULL;

	int32			*status = NULL;
	int			elem_id  = 0;
	int			p_status = 2; // Main Connection No plan found status
	int			db = 1;
	int			is_base_pack = -1;

	time_t			end_t = 0;
	time_t			start_t = fm_utils_time_round_to_midnight(pin_virtual_time(NULL));

	pin_cookie_t		cookie = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	ret_flistp = PIN_FLIST_CREATE(ebufp);

		
	// Build Input flist for new activation	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "In Prep activation data");
	PIN_FLIST_FLD_PUT(ret_flistp, PIN_FLD_POID, PIN_POID_CREATE(db,"/plan", -1, ebufp), ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ACCOUNT_OBJ, new_act_pdp, ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_PROGRAM_NAME, "Transfer Subscription", ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, ret_flistp, PIN_FLD_USERID, ebufp);
	
	// Check for new account's MAIN PACK has active plans
	prod_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(prod_iflistp, PIN_FLD_POID, new_act_pdp, ebufp );
	PIN_FLIST_FLD_SET(prod_iflistp, PIN_FLD_SERVICE_OBJ, new_srvc_pdp, ebufp );
	fm_mso_get_purchased_prod_active(ctxp, prod_iflistp, &prod_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&prod_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in getting the product details", ebufp);
		return;
	}
	if (prod_oflistp != NULL && (PIN_FLIST_ELEM_COUNT(prod_oflistp, PIN_FLD_RESULTS, ebufp) > 0 )){

		/*****************************************************************************************
		* Validate the Parent and Child STB and make sure to mirror the new account's MAIN package
		* and build the input flist for CATV activation and add the PLAN details.
		* This will check the DEMO plan that is mapped for new accoun't state  in 
		* /mso_cfg_catv_state_demo_pack table and will get the DEMO PLAN.
		*****************************************************************************************/
		//fm_mso_validate_parent_child_pckg_mirror(ctxp, i_flistp, new_act_pdp, &addl_plan_flistp, ebufp);
		fm_mso_catv_add_demo_pack(ctxp, i_flistp, new_act_pdp, &addl_plan_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Additional Plan Details", addl_plan_flistp);
		
		if(addl_plan_flistp && PIN_FLIST_ELEM_COUNT(addl_plan_flistp, PIN_FLD_PLAN, ebufp) > 0){
			plan_list_flistp = PIN_FLIST_SUBSTR_ADD(ret_flistp, PIN_FLD_PLAN_LIST_CODE, ebufp);
			while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(addl_plan_flistp, PIN_FLD_PLAN, &elem_id, 1,                                                                                                                      &cookie, ebufp)) != NULL ){
				plan_flistp = PIN_FLIST_ELEM_ADD(plan_list_flistp, PIN_FLD_PLAN, elem_id, ebufp);
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_PURCHASE_END_T, &end_t, ebufp);
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_PURCHASE_START_T, &start_t, ebufp);
				PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_CODE, plan_flistp, PIN_FLD_CODE, ebufp);
			}
		}
		else{
			if (addl_plan_flistp)
				status = PIN_FLIST_FLD_GET(addl_plan_flistp, PIN_FLD_STATUS, 1, ebufp);
			if(status && *status == 1 ){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_prep_activation_data: Additional Plans not found");
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, status, ebufp);
				*r_flistpp = ret_flistp;
				PIN_FLIST_DESTROY_EX(&prod_iflistp, NULL);
				PIN_FLIST_DESTROY_EX(&addl_plan_flistp, NULL);
				return;
			}
			//As no additional plan details found, return
			return;
		}
	}
	else{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_prep_activation_data: New Account Main connection doesn't have active plans");
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &p_status, ebufp);
		*r_flistpp = ret_flistp;
		return;
	}

	// Add SERVICES array
	srvc_array_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(srvc_array_flistp, PIN_FLD_SERVICE_OBJ, new_srvc_pdp, ebufp);
	catv_flistp = PIN_FLIST_SUBSTR_ADD(srvc_array_flistp, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_FLD_SET(catv_flistp, MSO_FLD_SALESMAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(catv_flistp, MSO_FLD_BOUQUET_ID, "bouquet_id", ebufp);
	// Add DEVICES array
	dvc_array_flistp = PIN_FLIST_ELEM_ADD(srvc_array_flistp, PIN_FLD_DEVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(dvc_array_flistp, MSO_FLD_STB_ID, device_id, ebufp);
	if (vc_id)
	{
		dvc_array_flistp = PIN_FLIST_ELEM_ADD(srvc_array_flistp, PIN_FLD_DEVICES, 1, ebufp);
		PIN_FLIST_FLD_SET(dvc_array_flistp, MSO_FLD_VC_ID, vc_id, ebufp);
	}

	*r_flistpp = ret_flistp;
	PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&addl_plan_flistp, NULL);
	return;
		
}

static int32 
fm_mso_perform_lco_check(
        pcm_context_t           *ctxp,
        poid_t                  *act_pdp,
        poid_t                  *new_act_pdp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lco_flistp = NULL;
	pin_flist_t             *ret_flistp = NULL;

	poid_t			*lco_pdp = NULL;
	poid_t			*new_lco_pdp  = NULL;

	int32			status = 0;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_RESET(ebufp);
	
	// Get LCO of old account
	fm_mso_util_get_customer_lco_info(ctxp, act_pdp, &ret_flistp, ebufp);
	if (ret_flistp != NULL ){
		lco_pdp =  PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	} 
	else{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "LCO details not found - 1");
	}
	// Get LCO of new account
	fm_mso_util_get_customer_lco_info(ctxp, new_act_pdp, &lco_flistp, ebufp);
	if (lco_flistp != NULL ){
		new_lco_pdp =  PIN_FLIST_FLD_GET(lco_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	} 
	else{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "LCO details not found - 2");
	}
	if (lco_pdp && new_lco_pdp && PIN_POID_COMPARE(lco_pdp, new_lco_pdp, 0, ebufp) == 0){
		status = 1;//SUCCESS
	}
	else{
		status = 0; //Not matching
	}
	PIN_FLIST_DESTROY_EX(&ret_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&lco_flistp,NULL);
	
	return status;	
}
