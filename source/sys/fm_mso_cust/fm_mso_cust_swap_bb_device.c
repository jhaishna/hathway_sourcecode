/*******************************************************************
 * File:	fm_mso_cust_swap_bb_device.c
 * Opcode:	MSO_OP_CUST_SWAP_BB_DEVICE 
 * Owner:	Pawan
 * Created:	07-NOV-2014
 * Description: This opcode is used to swap the device. 

	r << xx 1
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 2474113 0
		0 PIN_FLD_USERID           POID [0] 0.0.0.1 /account 44029 0
		0 PIN_FLD_PROGRAM_NAME    STR [0] "Automatic Account creation"
		0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 2476353 0
		0 PIN_FLD_REASON_ID         INT [0] 1
		0 PIN_FLD_DEVICES       ARRAY [0] allocated 20, used 1
		1    PIN_FLD_DEVICE_ID       STR [0] "18:59:33:5c:93:4a"
		1    PIN_FLD_FLAGS         INT [0] 0
		0 PIN_FLD_DEVICES       ARRAY [1] allocated 20, used 1
		1    PIN_FLD_DEVICE_ID       STR [0] "18:59:33:5c:93:4b"
		1    PIN_FLD_FLAGS         INT [0] 1
	xx
	xop XXXX 0 1

	nap(13252)> xop XXXX 0 1
	xop: opcode XXXX, flags 0
	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 2474113 0
	0 PIN_FLD_STATUS         STR [0] 0
	0 PIN_FLD_DESCR         STR [0] "Device changed successfully."

 *
 * Modification History:
 * Modified By: Jyothirmayi Kachiraju
 * Date: 12th Dec 2019
 * Modification details : Added a new function to validate the Device Types for Swapping Devices
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_swap_bb_device.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/device.h"
#include "ops/act.h"
#include "ops/cust.h"
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
#include "mso_device.h"
#include "mso_lifecycle_id.h"

#define MSO_FLAG_SRCH_BY_ID 1

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

void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

extern int
validate_device_type(
	pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *device_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

extern int
fm_mso_update_mso_purplan_status(
	pcm_context_t           *ctxp,
	poid_t                  *mso_purplan_obj,
	int			status,
	pin_errbuf_t            *ebufp);

extern int
fm_mso_update_ser_prov_status(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	int 			prov_status,
	pin_errbuf_t            *ebufp);

/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/
static void
fm_mso_cust_validate_device_service(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**ret_flistp,
	pin_flist_t		**new_device_flistp,
	pin_flist_t		**old_device_flistp,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_cust_swap_bb_ott(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/* ***********************************************************************
 * Changes for JIO-CPS Integration-ISP Enhancement
 * This function takes the information of two devices and validates
 * whether the device types of both the devices are of same type or not.
 * If same type, returns 0, otherwise returns 1.
 ************************************************************************/
int 
fm_mso_cust_validate_device_types(
	pcm_context_t	*ctxp, 
	pin_flist_t		*old_device_flistp,
	pin_flist_t		*new_device_flistp,	
	pin_errbuf_t	*ebufp);

/**************************************
* Local Functions end
***************************************/
EXPORT_OP void
op_mso_cust_swap_bb_device(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_swap_bb_device(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void 
cleanup_old_device_traces(
	pcm_context_t		*ctxp,
	pin_flist_t		*old_device_flistp,
	pin_flist_t		*in_flistp,
	int32			old_plan_type,
	int32			new_plan_type,
	pin_flist_t		*new_device_flistp,
	pin_errbuf_t		*ebufp );

static int 
fm_mso_cust_swap_dev_amc_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	int32			old_plan_type,
	int32			new_plan_type,
	pin_errbuf_t		*ebufp );

PIN_IMPORT int
fm_mso_chk_device_warehouse(
        pcm_context_t           *ctxp,
        char                    *lco_code,
		char                    *cmts,
        char                    *city,
		pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * MSO_OP_CUST_SWAP_BB_DEVICE  
 *******************************************************************/
void 
op_mso_cust_swap_bb_device(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	int			*status = NULL;
	int32			status_uncommitted =2;
	poid_t			*inp_pdp = NULL;
	char            *prog_name = NULL;
	
	poid_t			*poid_p = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	*r_flistpp = NULL;

	/* Insanity Check */
	if (opcode != MSO_OP_CUST_SWAP_BB_DEVICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_mso_swap_bb_device error",
			ebufp);
		return;
	}

	/* Debug Input flist */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_swap_bb_device input flist", i_flistp);
	
	/*** Pavan Bellala 22-07-2015 ****************************
	Bug id 1216
	During Bulk swap CPE, the PIN_FLD_POID is mta task poid.
	But, the opcode expects ACCOUNT_POID in PIN_FLD_POID. Hence,
	modifiying input file.
	*** Pavan Bellala 22-07-2015 ****************************/
	
	poid_p = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if(strcmp(PIN_POID_GET_TYPE(poid_p),"/account")!=0)
	{
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,i_flistp,PIN_FLD_JOB_OBJ,ebufp);	
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_ACCOUNT_OBJ,i_flistp,PIN_FLD_POID,ebufp);	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_cust_swap_bb_device modified input flist",i_flistp);
	}

		prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
		inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		local = fm_mso_trans_open(ctxp, inp_pdp, 3,ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51761", ebufp);
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
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51762", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
			return;
		}
	
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEFAULT_FLAG, 1, ebufp) != NULL)
	{
		fm_mso_cust_swap_bb_ott(ctxp, flags, i_flistp, r_flistpp, ebufp);
	}
	else
	{
		fm_mso_cust_swap_bb_device(ctxp, flags, i_flistp, r_flistpp, ebufp);
	}

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
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"op_mso_cust_swap_bb_device error");
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS)
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}		
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_swap_bb_device output flist", *r_flistpp);
	return;
}


/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_swap_bb_device(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*new_device_flistp = NULL;
	pin_flist_t		*old_device_flistp = NULL;
	pin_flist_t		*device_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*device_flistp = NULL;
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*prov_out_flistp = NULL;
	pin_flist_t		*lc_in_flistp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*user_id = NULL;
	poid_t			*bp_obj = NULL;
	poid_t			*mso_pp_obj = NULL;
	poid_t			*dev_pdp = NULL;
	int32			action_flag = 0;
	int32			device_change_success = 0;
	int32			device_change_failure = 1;
	int32			*reason_id = NULL;
	int32			*prov_flag = NULL;
	int64			db = -1;
	char			*program_name = NULL;
	int				csr_access = 0;
	int				acct_update = 0;
	int				disas_flag = 0;
	int				assoc_flag = 0;
	int				*result = NULL;
	int				*prov_call = NULL;
	int				ret_val = 0;
	int32			old_plan_type = 0; // start as customer device
	int32			new_plan_type = 0; // start as customer device
	pin_flist_t		*plan_flistp = NULL;
	int32			flag = 0;
	int32			elem_id = 0;
	pin_cookie_t	cookie = NULL;
	void			*vp = NULL;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_bb_device input flist", i_flistp);

	lc_in_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	
	account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	service_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	reason_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 1, ebufp);
	program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	
	while ((device_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_DEVICES, &elem_id, 1, &cookie, ebufp )) != NULL)
	{
		flag = *(int32 *)PIN_FLIST_FLD_GET(device_flistp, PIN_FLD_FLAGS, 0, ebufp);
		if (flag)
		{
			if(flag == 1) { // for new device only.
				vp = PIN_FLIST_FLD_GET(device_flistp, PIN_FLD_TYPE, 1, ebufp);
				if(vp == NULL) {
					*r_flistpp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51763", ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Type for new device plan not passed in input as it is mandatory field", ebufp);
					return;
				}
				new_plan_type = *(int32 *)vp;
			}
		} 

	}

	if (account_obj)
	{	
		db = PIN_POID_GET_DB(account_obj);
		//user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp); 
	}
	else
	{
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51763", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		return;
	}

	if ( !service_obj)
	{
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51764", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Service poid is missing in input.", ebufp);
		return;
	}	

	if ( !reason_id)
	{
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51765", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Device Swap Reason ID missing in input.", ebufp);
		return;
	}
	/* Device swap validations rules */
	fm_mso_cust_validate_device_service(ctxp, i_flistp, &ret_flistp, &new_device_flistp, &old_device_flistp, ebufp);
	if ( !ret_flistp)
	{
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51766", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Device validation failed - Cannot swap device", ebufp);

		PIN_FLIST_DESTROY_EX(&new_device_flistp,NULL);
		PIN_FLIST_DESTROY_EX(&old_device_flistp,NULL);

		return;
	}	
	result = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 1, ebufp);
	if(result && *result ==1)
	{
		*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&new_device_flistp,NULL);
		PIN_FLIST_DESTROY_EX(&old_device_flistp,NULL);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);	
	/* Get Reason ID and set associate and dis-associate flags */
	reason_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 0, ebufp);
	if (*reason_id == MSO_DEVICE_CHANGE_FAULTY_DEVICE )
	{
	        disas_flag = MSO_DEVICE_DEACTIVATION_REASON_FAULTY;
	        //assoc_flag = MSO_DEVICE_ACTIVATION_REPLACEMENT; //Issue in fm_mso_device_bb_associate.c 
		assoc_flag = MSO_DEVICE_ACTIVATION_NEW;
	}
	else if ( *reason_id == MSO_DEVICE_CHANGE_TEMPORARY_DEVICE )
	{
		disas_flag = MSO_DEVICE_DEACTIVATION_REASON_TEMPORARY_DEVICE;
		//assoc_flag = MSO_DEVICE_ACTIVATION_REPLACEMENT; //Issue in fm_mso_device_bb_associate.c 
		assoc_flag = MSO_DEVICE_ACTIVATION_NEW;
	}
    else if (*reason_id == MSO_DEVICE_CHANGE_TECH_DEVICE)
    {
        disas_flag = MSO_DEVICE_DEACTIVATION_REASON_CHANGE_TECH;
        assoc_flag = MSO_DEVICE_ACTIVATION_NEW;
    }
	else
	{
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51767", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Reason ID for swap device.", ebufp);
		PIN_FLIST_DESTROY_EX(&new_device_flistp,NULL);
		PIN_FLIST_DESTROY_EX(&old_device_flistp,NULL);
		return;		
	}
	
	/* Get the mso_purchased_plan obj*/
	fm_mso_cust_get_bp_obj(ctxp, account_obj, service_obj,-1, &bp_obj, &mso_pp_obj, ebufp);

	/* Copy the purchased plan flist for this device for later use 
	dev_plan_flistp = NULL;
	plan_flistp = PIN_FLIST_ELEM_GET(old_device_flistp, PIN_FLD_PLAN, 0, 1, ebufp);
	if(plan_flistp != NULL) {
		dev_plan_flistp = PIN_FLIST_COPY(plan_flistp, ebufp);
	}*/

	plan_flistp = PIN_FLIST_ELEM_GET(old_device_flistp, PIN_FLD_PLAN, 0, 1, ebufp);
	if(plan_flistp) {
		old_plan_type = *(int32 *)PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_TYPE, 0, ebufp);
	}

	// Call function to execute AMC functionality
	ret_val=0;
	ret_val = fm_mso_cust_swap_dev_amc_plan(ctxp, i_flistp, old_plan_type, new_plan_type, ebufp );
	if(ret_val==1)
	{
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51781", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in associating AMC plan", ebufp);
		PIN_FLIST_DESTROY_EX(&new_device_flistp,NULL);
		PIN_FLIST_DESTROY_EX(&old_device_flistp,NULL);
		goto CLEANUP;
	}
	
	// Cleanup old device trace (plan array)
	if(old_plan_type == RENTAL_DEPOSIT_PLAN || old_plan_type == OUTRIGHT_PLAN) {
	    cleanup_old_device_traces(ctxp, old_device_flistp, i_flistp, old_plan_type, new_plan_type, new_device_flistp, ebufp );
	}

	/* Disassociate existing device */
	device_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(old_device_flistp, PIN_FLD_POID, device_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(old_device_flistp, PIN_FLD_POID, lc_in_flistp, MSO_FLD_OLD_DEVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_FLAGS, &disas_flag, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_SERVICES, 0,ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, (void *) account_obj, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SERVICE_OBJ, (void *) service_obj, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_bb_device disassociate input flist ", device_flistp);
	PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, device_flistp, &device_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_cust_swap_bb_device disassociate output flist", device_out_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51779", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Disassociation Failed for Existing Device.", ebufp);
		PIN_FLIST_DESTROY_EX(&new_device_flistp,NULL);
		PIN_FLIST_DESTROY_EX(&old_device_flistp,NULL);
		goto CLEANUP;
	}
	PIN_FLIST_DESTROY_EX(&device_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&device_out_flistp, NULL);
		
	plan_flistp = PIN_FLIST_ELEM_GET(old_device_flistp, PIN_FLD_PLAN, 0, 1, ebufp);
	if(plan_flistp) {
		old_plan_type = *(int32 *)PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_TYPE, 0, ebufp);
	}
	if(old_plan_type == RENTAL_DEPOSIT_PLAN || old_plan_type == OUTRIGHT_PLAN) {
	    cleanup_old_device_traces(ctxp, old_device_flistp, i_flistp, old_plan_type, new_plan_type, new_device_flistp, ebufp );
	}

	 /* Associate New device */
	device_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(new_device_flistp, PIN_FLD_POID, device_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(new_device_flistp, PIN_FLD_POID, lc_in_flistp, MSO_FLD_NEW_DEVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_FLAGS, &assoc_flag, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_SERVICES, 0,ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, (void *) account_obj, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SERVICE_OBJ, (void *) service_obj, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_cust_swap_bb_device associate input flist ", device_flistp);
	PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, device_flistp, &device_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"fm_mso_cust_swap_bb_device associate output flist", device_out_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside IS ERR condition");
                if(((ebufp)->pin_err == PIN_ERR_IM_CONNECT_FAILED) || (ebufp)->pin_err == PIN_ERR_DM_CONNECT_FAILED ||
                        (ebufp)->rec_id == 9999 )
                {
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,"Device not associated to Locator.First move the device to Locator and then assign.",ebufp);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Set Device not associated message");
                }
                else
                {
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Association Failed for New Device.", ebufp);
                }
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51780", ebufp);

                PIN_FLIST_DESTROY_EX(&new_device_flistp,NULL);
                PIN_FLIST_DESTROY_EX(&old_device_flistp,NULL);

		goto CLEANUP;
	}

	// Call Provisioning action.
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "New Device flist", new_device_flistp);
	prov_flag = (int32 *)PIN_FLIST_FLD_GET(new_device_flistp, MSO_FLD_PROVISIONABLE_FLAG, 1, ebufp);
	if(prov_flag && *prov_flag == 1)
	{
		prov_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, prov_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, prov_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(old_device_flistp, PIN_FLD_DEVICE_ID, prov_in_flistp, PIN_FLD_LOGIN, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, prov_in_flistp, PIN_FLD_REASON_ID, ebufp);
		dev_pdp = PIN_FLIST_FLD_GET(old_device_flistp, PIN_FLD_POID, 0, ebufp);
		if(dev_pdp) {
			if(strstr(PIN_POID_GET_TYPE(dev_pdp),"/device/modem")) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Device Type is Modem");
				action_flag = DOC_BB_MODEM_CHANGE;
			} else if(strstr(PIN_POID_GET_TYPE(dev_pdp),"/device/ip/")) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Device Type is IP");
				//Added this field to avoid existing ips being added in QPS
				PIN_FLIST_FLD_COPY(new_device_flistp, PIN_FLD_DEVICE_ID, prov_in_flistp, PIN_FLD_DEVICE_ID, ebufp);
				action_flag = DOC_BB_IP_CHANGE; 
			}
		}
		
		PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &action_flag, ebufp);
		if ( mso_pp_obj )
		PIN_FLIST_FLD_SET(prov_in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_pp_obj, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_bb_device provisioning input list", prov_in_flistp);
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, prov_in_flistp, &prov_out_flistp, ebufp);
		//PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
		if(prov_out_flistp)
		{
			prov_call = PIN_FLIST_FLD_GET(prov_out_flistp, PIN_FLD_STATUS, 1, ebufp);
			if(prov_call && (*prov_call == 1))
			{
				*r_flistpp = PIN_FLIST_COPY(prov_out_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
				return;
			}
		}
		else
		{
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51751", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in Provisioning action.", ebufp);
			PIN_FLIST_DESTROY_EX(&new_device_flistp,NULL);
			PIN_FLIST_DESTROY_EX(&old_device_flistp,NULL);
			PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_bb_device provisioning return list", prov_out_flistp);
		//Pawan: added below to update the service prov status to in-progress
		ret_val = fm_mso_update_ser_prov_status(ctxp, prov_in_flistp, MSO_PROV_IN_PROGRESS, ebufp );
		if(ret_val)
		{
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51767", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error while updating service provisioning status.", ebufp);
			PIN_FLIST_DESTROY_EX(&new_device_flistp,NULL);
			PIN_FLIST_DESTROY_EX(&old_device_flistp,NULL);
			PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
			return;
		}

		// Pawan: Added below to update the status of mso_pur_plan to in-progress 
		if(fm_mso_update_mso_purplan_status(ctxp,mso_pp_obj, MSO_PROV_IN_PROGRESS, ebufp ) == 0)
		{
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &device_change_failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51754", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error while updating mso_purhcased_plan status.", ebufp);
			PIN_FLIST_DESTROY_EX(&new_device_flistp,NULL);
			PIN_FLIST_DESTROY_EX(&old_device_flistp,NULL);
			PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
			return;
		}
		PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
	}

	/***********************************************************
	* Lifecycle events
	***********************************************************/
	if (/*action_flag = DOC_BB_MODEM_CHANGE &&*/ !(PIN_ERRBUF_IS_ERR(ebufp)) )
	{
		fm_mso_create_lifecycle_evt(ctxp, lc_in_flistp, NULL, ID_SWAP_DEVICE, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Error in Lifecycle event creation",lc_in_flistp);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Lifecycle event creation", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}
	
	/* Create Output flist */
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &device_change_success, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "Device changed successfully.", ebufp);
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_bb_device output flist", *r_flistpp);

CLEANUP:
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_bb_device output flist", *r_flistpp);
	PIN_FLIST_DESTROY_EX(&device_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&device_out_flistp, NULL);	
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&lc_in_flistp, NULL);
	return;
}

/**************************************************
* Function: fm_mso_cust_validate_device_service()
* 
* 
***************************************************/
static void
fm_mso_cust_validate_device_service(
	pcm_context_t		*ctxp,
	pin_flist_t			*in_flist,
	pin_flist_t			**ret_flistp,
	pin_flist_t			**new_device_flistp,
	pin_flist_t			**old_device_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*readsvc_inflistp = NULL;
	pin_flist_t		*readsvc_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*device_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	//pin_flist_t		*old_device_flistp = NULL;
	//pin_flist_t		*new_device_flistp = NULL;
	
	poid_t			*service_obj = NULL;
	poid_t			*device_svc = NULL;
	poid_t			*new_device_pdp = NULL;
	poid_t			*old_device_pdp = NULL;
	int				*status = NULL;
	int				*prov_status = NULL;
	int				elem_id = 0;
	int				device_change_success = 0;
	int				device_change_failure = 1;
	int				*flag =  NULL;
	int				remove_found = 0;
	int				add_found = 0;
	int				DEVICE_STATE_FREE = 1l;
	int        	 	DEVICE_STATE_REP = 9;
	int64			db = 0;	
	int32			device_search_type = MSO_FLAG_SRCH_BY_ID;	
	pin_cookie_t 	cookie = NULL;
	char			*device_id = NULL;
	char			remove_device_id[64];
	char			add_device_id[64];
	char			*old_type = NULL;
	char			*new_type = NULL;
	char			*lco_code = NULL;
	char			*cmts_id = NULL;
	char			*city = NULL;

	int				v_result = PIN_BOOLEAN_TRUE;
	int32			is_valid_warehouse_type = 0;
	pin_flist_t		*v_flistp = NULL;
	int 			device_type_mismatch = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_validate_device_service input flist", in_flist);

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	readsvc_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, readsvc_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_CREATED_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(readsvc_inflistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(readsvc_inflistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);
	args_flistp = PIN_FLIST_SUBSTR_ADD(readsvc_inflistp, MSO_FLD_BB_INFO, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_TECHNOLOGY, NULL, ebufp);	
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_NETWORK_ELEMENT, NULL, ebufp);	
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, NULL, ebufp);	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_validate_device_service read service input", readsvc_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readsvc_inflistp, &readsvc_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&readsvc_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51768", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Swap Device error - Error in Reading service.", ebufp);
		*ret_flistp = r_flistp;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_validate_device_service read service  output", readsvc_outflistp);

	/*Check Service status and service provisioning status */
	status = PIN_FLIST_FLD_GET(readsvc_outflistp, PIN_FLD_STATUS, 1, ebufp );
	args_flistp = PIN_FLIST_ELEM_GET(readsvc_outflistp, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp );
	prov_status = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATUS, 1, ebufp );
	
	if ( *(int *)status != PIN_STATUS_ACTIVE)
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51769", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service status is not active - Cannot Swap device. ", ebufp);
		*ret_flistp = r_flistp;
		return;		
	}
	if ( *(int *)prov_status != MSO_PROV_ACTIVE )
	{
		PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51770", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service Provisioning status is not active - Cannot Swap device.", ebufp);
		*ret_flistp = r_flistp;
		return;		
	}
	//PIN_FLIST_DESTROY_EX(&readsvc_outflistp, NULL);
	
	/*Check if Add and Remove devices exists in input flist */
	while ((device_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_DEVICES, &elem_id, 1, &cookie, ebufp )) != NULL)
	{
		device_id = NULL;
		device_id = PIN_FLIST_FLD_GET(device_flistp, PIN_FLD_DEVICE_ID, 1, ebufp );
		if(!device_id || strlen(device_id)==0)
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51771", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Device Id missing in input - Cannot Swap device.", ebufp);
			*ret_flistp = r_flistp;
			return;
		}
		flag = PIN_FLIST_FLD_GET(device_flistp, PIN_FLD_FLAGS, 1, ebufp );
		if (flag && *(int*)flag == 0)
		{
			remove_found = 1;
			strcpy(remove_device_id,device_id);
		}
		else if(flag && *(int*)flag == 1)
		{
			add_found = 1;
			strcpy(add_device_id,device_id);
		}
	}

	if(!remove_found)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51771", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Device to be removed, not found.", ebufp);
		*ret_flistp = r_flistp;
		return;
	}

	if(!add_found)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51772", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Device to be added, not found.", ebufp);
		*ret_flistp = r_flistp;
		return;
	}		
	
	/* Get Device information from Old Device Id */
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, srch_in_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_DEVICE_ID, remove_device_id, ebufp );
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_SEARCH_TYPE, &device_search_type, ebufp );
	fm_mso_get_device_info(ctxp, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_info Old device flist", srch_out_flistp);
	if(!srch_out_flistp)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51773", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Existing device details not found.", ebufp);
		*ret_flistp = r_flistp;
		return;
	}
	*old_device_flistp = PIN_FLIST_COPY(srch_out_flistp, ebufp );
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	
	/* Get Device information from New Device Id */
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, srch_in_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_DEVICE_ID, add_device_id, ebufp );
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_SEARCH_TYPE, &device_search_type, ebufp );
	fm_mso_get_device_info(ctxp, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_info New device flist", srch_out_flistp);
	if(!srch_out_flistp)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51774", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "New Device Id specified in input not found.", ebufp);
		*ret_flistp = r_flistp;
		return;
	}	
	*new_device_flistp = PIN_FLIST_COPY(srch_out_flistp, ebufp );
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	// Added the below code for JIO-CPS Integration-ISP Enhancement 
	/* Validation to check the Device Type of the Devices to be swapped */
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validating Device Types for Swapping Devices");
	
	if(*old_device_flistp != NULL && *new_device_flistp != NULL)
	{
		device_type_mismatch = fm_mso_cust_validate_device_types(ctxp, *old_device_flistp, *new_device_flistp, ebufp);
	
		if(device_type_mismatch != 0)
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51777", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "New Device Type does not match Old Device Type.", ebufp);
			*ret_flistp = r_flistp;
			return;
		}
	}
	
	/* Verify is existing device is associated with service or not*/
	services_flistp = PIN_FLIST_ELEM_GET(*old_device_flistp, PIN_FLD_SERVICES, 0, 1, ebufp );
	/***** Pavan Bellala 18-11-2015 *************
	Error catch to handle issue in migrated data
	*********************************************/
	if(services_flistp == NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Old_device flist",*old_device_flistp);
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51779", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Data incorrect/missing. Cannot Swap device.", ebufp);
                *ret_flistp = r_flistp;

                return;
	}
	device_svc = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	if(PIN_POID_COMPARE(device_svc, service_obj, 0, ebufp) != 0)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51775", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Device Id is not associated to this service.", ebufp);
		*ret_flistp = r_flistp;
		return;
	}

	/* Verify the State ID of new device */
	status = PIN_FLIST_FLD_GET(*new_device_flistp, PIN_FLD_STATE_ID, 1, ebufp );
	if (( *(int *)status != DEVICE_STATE_FREE)&&( *(int *)status != DEVICE_STATE_REP))
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51776", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "New Device is not free to allocate - Cannot Swap device.", ebufp);
		*ret_flistp = r_flistp;
		return;		
	}
	
	/* Compare the device type of both devices */
	new_device_pdp = PIN_FLIST_FLD_GET(*new_device_flistp, PIN_FLD_POID, 1, ebufp );
	old_device_pdp = PIN_FLIST_FLD_GET(*old_device_flistp, PIN_FLD_POID, 1, ebufp );
	if(new_device_pdp && old_device_pdp)
	{
		new_type = (char *)PIN_POID_GET_TYPE(new_device_pdp);
		old_type = (char *)PIN_POID_GET_TYPE(old_device_pdp);
		if(strcmp(new_type,old_type))
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51777", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "New device type DOES NOT MATCH old device type.", ebufp);
			*ret_flistp = r_flistp;
			return;
		}
	}
	else
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51778", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in getting device POID - Cannot Swap device.", ebufp);
		*ret_flistp = r_flistp;
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info flist", readsvc_outflistp);

	args_flistp = PIN_FLIST_SUBSTR_GET(readsvc_outflistp, MSO_FLD_BB_INFO, 1, ebufp );

	cmts_id = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_NETWORK_ELEMENT, 1, ebufp);
	city = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_CITY, 1, ebufp);
	   lco_code = PIN_FLIST_FLD_GET(*new_device_flistp, PIN_FLD_SOURCE, 1, ebufp);

	is_valid_warehouse_type = fm_mso_chk_device_warehouse(ctxp, lco_code, cmts_id, city, ret_flistp, ebufp);

	if(is_valid_warehouse_type == 0 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Validation for device source failed. returning.. ");
		return;
	}



	/*** Pavan Bellala  31072015 ******************************
	Bug id : 1294 Device validation added
	********** Pavan Bellala 31072015  ***********************/ 
	v_result = validate_device_type(ctxp, in_flist, *new_device_flistp, &v_flistp,ebufp);
	if( v_result  == PIN_BOOLEAN_FALSE )
	{
		*ret_flistp = v_flistp;
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		return;
	}
		
	
	/* Create Output flist */
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &device_change_success, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Device Validation success.", ebufp);
	*ret_flistp = r_flistp;


	PIN_FLIST_DESTROY_EX(&v_flistp,NULL);

	return;	
	
}



static void cleanup_old_device_traces(
	pcm_context_t		*ctxp,
	pin_flist_t		*old_device_flistp,
	pin_flist_t		*in_flistp,
	int32			old_plan_type,
	int32			new_plan_type,
	pin_flist_t		*new_device_flistp,
	pin_errbuf_t		*ebufp )
{

	pin_flist_t	*cancel_plan_flistp = NULL;
	pin_flist_t	*srch_prt_out_flistp = NULL;
	pin_flist_t	*arg_flistp = NULL;
	pin_flist_t	*read_pp_res_flistp = NULL;
	pin_flist_t	*cancel_plan_ret_flistp = NULL;
	pin_flist_t	*plan_flistp = NULL;
	pin_flist_t	*read_mso_iflistp = NULL;
	pin_flist_t	*read_mso_oflistp = NULL;
	pin_flist_t	*srch_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	pin_flist_t	*srch_out_flistp = NULL;
	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*in_flist = NULL;
	pin_flist_t	*bb_ret_flistp = NULL;
	pin_flist_t	*read_pp_flistp = NULL;
	pin_flist_t	*svc_flistp = NULL;
	pin_flist_t	*prod_flistp = NULL;
	pin_flist_t	*del_flistp = NULL;
	pin_flist_t	*del_out_flistp = NULL;
	pin_flist_t	*refund_iflistp = NULL;
	pin_flist_t	*refund_oflistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	int64		db = 1;
	int32		srch_flag = 0;
	int32		plan_status = 0;
	int32		status_change_failure = 1;
	poid_t		*service_obj = NULL;
	poid_t		*plan_obj = NULL;
	poid_t		*pp_obj = NULL;
	char		*search_prt_template = "";
	char		*descr = "swap_bb_device";
	int32		search_flags = 0;
	int32		is_same_plan_type = 0;
	int32		deposit_status = 0;
	int32		elem_id = 0;
	pin_cookie_t	cookie = NULL;
	char		search_dep_template[256] = "select X from /mso_deposit where F1 = V1 and F2 = V2 and F3 = V3 ";

	/* cancel the plan
	if rental, check for deposit and call liquidation opcode */
	
	/* cancel the plan */
	
	// 1. Read mso_purchased_plan entry for the device

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cleanup_old_device_traces old_device_flistp", old_device_flistp);

	if (old_plan_type == new_plan_type) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old and New plan types are same.");
		is_same_plan_type = 1 ;
	}

	if(is_same_plan_type) {
		// move the plans from old device to new device
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Moving the plans from old device to new.");
		del_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(old_device_flistp, PIN_FLD_POID, del_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_ELEM_COPY(old_device_flistp, PIN_FLD_PLAN, 0, del_flistp, PIN_FLD_PLAN, 0, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"delete plan input flist ", del_flistp);
		PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, del_flistp, &del_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"delete plan output flist", del_out_flistp);
		
		PIN_FLIST_DESTROY_EX(&del_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&del_out_flistp, NULL);

		del_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(new_device_flistp, PIN_FLD_POID, del_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_ELEM_COPY(old_device_flistp, PIN_FLD_PLAN, 0, del_flistp, PIN_FLD_PLAN, 0, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"add plan input flist ", del_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, del_flistp, &del_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"add plan output flist", del_out_flistp);

		PIN_FLIST_DESTROY_EX(&del_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&del_out_flistp, NULL);

	} else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old and New plan types are NOT same.");

		plan_flistp = PIN_FLIST_ELEM_GET(old_device_flistp, PIN_FLD_PLAN, 0, 0, ebufp);	
		svc_flistp = PIN_FLIST_ELEM_GET(old_device_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
		service_obj = PIN_FLIST_FLD_GET(svc_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);

		read_mso_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(plan_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, read_mso_iflistp, PIN_FLD_POID, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_purchased_plan read flds input list", read_mso_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_mso_iflistp, &read_mso_oflistp, ebufp);
		// Memory management
		PIN_FLIST_DESTROY_EX(&read_mso_iflistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_purchased_plan read flds output list", read_mso_oflistp);

		plan_obj = PIN_FLIST_FLD_GET(read_mso_oflistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		prod_flistp = PIN_FLIST_ELEM_GET(read_mso_oflistp, PIN_FLD_PRODUCTS, 0, 0, ebufp);
		pp_obj = PIN_FLIST_FLD_GET(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 0, ebufp);	 

		//3. Read package id and deal info for the plan

		read_pp_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, read_pp_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(read_pp_flistp, PIN_FLD_PACKAGE_ID, (time_t *)NULL, ebufp); 
		PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "Reading Purchased product end dates for this plan input", read_pp_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_pp_flistp, &read_pp_res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_pp_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp) )
		{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No purchased product is associated for this plan", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&srch_prt_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_pp_res_flistp, NULL);			
				return;
		}
		PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "Reading Purchased product end dates for this plan output", read_pp_res_flistp);

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Preparing Plan flist for calling cancel plan opcode");
		cancel_plan_flistp = PIN_FLIST_CREATE(ebufp);
		//if ( *(int32 *)customer_type == ACCT_TYPE_CORP_SUBS_BB )
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, cancel_plan_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, cancel_plan_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
		//PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_DESCR, descr, ebufp);
		PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_ACTION_NAME, descr, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, cancel_plan_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, cancel_plan_flistp, PIN_FLD_USERID, ebufp);

		arg_flistp = PIN_FLIST_ELEM_ADD(cancel_plan_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
		PIN_FLIST_CONCAT(arg_flistp, read_mso_oflistp, ebufp);
		PIN_FLIST_FLD_COPY(read_pp_res_flistp, PIN_FLD_PACKAGE_ID, arg_flistp, PIN_FLD_PACKAGE_ID, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_CANCEL_PLAN input flist", cancel_plan_flistp);
		PCM_OP(ctxp, MSO_OP_CUST_CANCEL_PLAN, 0, cancel_plan_flistp, &cancel_plan_ret_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_CANCEL_PLAN output flist", cancel_plan_ret_flistp);
		PIN_FLIST_DESTROY_EX(&cancel_plan_flistp, NULL);
		

		// remove plan from device
		del_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(old_device_flistp, PIN_FLD_POID, del_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_ELEM_COPY(old_device_flistp, PIN_FLD_PLAN, 0, del_flistp, PIN_FLD_PLAN, 0, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"delete plan input flist ", del_flistp);
		PCM_OP(ctxp, PCM_OP_DELETE_FLDS, 0, del_flistp, &del_out_flistp, ebufp);
		// Memory management
		PIN_FLIST_DESTROY_EX(&del_flistp, NULL);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"delete plan output flist", del_out_flistp);
		PIN_FLIST_DESTROY_EX(&del_out_flistp, NULL);

		if(old_plan_type == RENTAL_DEPOSIT_PLAN) {	
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old plan type is rental and deposit. Checking if there are any deposits for this device");
			// Call deposit refund opcode to liquidate the deposits for the old device.
			search_inflistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
            PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
            PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_dep_template, ebufp);
            args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
            PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
            args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
            PIN_FLIST_FLD_COPY(old_device_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
            args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
            PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &deposit_status, ebufp);
            results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object_with_plan search device input list", search_inflistp);
            PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

            pin_flist_t    *result_info = NULL;
            while ((result_info = PIN_FLIST_ELEM_GET_NEXT(search_outflistp,PIN_FLD_RESULTS,
                     &elem_id, 1, &cookie, ebufp))!= NULL)
            {
				refund_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, refund_iflistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, refund_iflistp, PIN_FLD_USERID, ebufp);
                
                pin_flist_t     *deposit_flistp = NULL;
                deposit_flistp = PIN_FLIST_ELEM_ADD(refund_iflistp, PIN_FLD_DEPOSITS, 0, ebufp);
                PIN_FLIST_FLD_SET(deposit_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
                PIN_FLIST_FLD_COPY(result_info, MSO_FLD_DEPOSIT_NO, deposit_flistp , MSO_FLD_DEPOSIT_NO , ebufp);

                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_integ_create_grv_uploader_job refund. input list:", refund_iflistp);
                PCM_OP(ctxp,MSO_OP_PYMT_DEPOSIT_REFUND,0 , refund_iflistp , &refund_oflistp , ebufp);

                PIN_FLIST_DESTROY_EX(&refund_iflistp, NULL);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PYMT_DEPOSIT_REFUND", ebufp);
                    PIN_FLIST_DESTROY_EX(&refund_oflistp, NULL);
                    return;
                }
                else
                {
                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                            "fm_mso_integ_create_grv_uploader_job refund. output list:" ,refund_oflistp);                    
                    PIN_FLIST_DESTROY_EX(&refund_oflistp , NULL);
                    return;
                }
            }
            PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
            PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);			
		}		
	}
}

static int 
fm_mso_cust_swap_dev_amc_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	int32			old_plan_type,
	int32			new_plan_type,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t	*plan_flistp = NULL;
	pin_flist_t	*read_in_flistp = NULL;
	pin_flist_t	*read_out_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*amc_flistp = NULL;
	pin_flist_t	*amc_out_flistp = NULL;

	poid_t		*acc_obj = NULL;
	poid_t		*svc_obj = NULL;
	poid_t		*user_obj = NULL;
	int32		*indicator = NULL;
	int32		amc_flag = -1;
	int32		mode = 0;
	char		msg[256] = {"\0"};

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 1;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_dev_amc_plan input flist", in_flist);

	acc_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp );
	svc_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 0, ebufp );
	user_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_USERID, 0, ebufp );
	//Read service indicator.
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
	args_flistp = PIN_FLIST_SUBSTR_ADD(read_in_flistp, MSO_FLD_BB_INFO, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read service input", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read service output", read_out_flistp);
	args_flistp = PIN_FLIST_SUBSTR_GET(read_out_flistp, MSO_FLD_BB_INFO, 0, ebufp );
	indicator = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_INDICATOR, 0, ebufp);
	sprintf(msg, "old_plan_type=%d and new_plan_type=%d",old_plan_type,new_plan_type);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, msg);

	// Based on indicator value and device type find the amc_flag value
	if(indicator && *indicator==PAYINFO_BB_PREPAID)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan is Prepaid");
		if(old_plan_type==RENTAL_DEPOSIT_PLAN && new_plan_type==OUTRIGHT_PLAN) {
			amc_flag = AMC_ON_PREPAID_SWAP_R_O;
		}
		// Commented below as this scenrio is not required.
		//else if(old_plan_type==OUTRIGHT_PLAN && new_plan_type==RENTAL_DEPOSIT_PLAN) {
		//	amc_flag = AMC_ON_PREPAID_SWAP_O_R;
		//}
		else if(old_plan_type==RENTAL_DEPOSIT_PLAN && new_plan_type==RENTAL_DEPOSIT_PLAN) {
			amc_flag = AMC_ON_PREPAID_SWAP_R_R;
		}
		sprintf(msg, "AMC flag = %d",amc_flag);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	}
	else if(indicator && *indicator==PAYINFO_BB_POSTPAID)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan is Postpaid");
		if(old_plan_type==RENTAL_DEPOSIT_PLAN && new_plan_type==OUTRIGHT_PLAN) {
			amc_flag = AMC_ON_POSTPAID_SWAP_R_O;
		}
		//Commented below as this scenrio will be applicable in Add hardware plan
		//else if(old_plan_type==OUTRIGHT_PLAN && new_plan_type==RENTAL_DEPOSIT_PLAN) {
		//	amc_flag = AMC_ON_POSTPAID_SWAP_O_R;
		//}
		else if(old_plan_type==RENTAL_DEPOSIT_PLAN && new_plan_type==RENTAL_DEPOSIT_PLAN) {
			amc_flag = AMC_ON_POSTPAID_SWAP_R_R;
		}
		sprintf(msg, "AMC flag = %d",amc_flag);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	}

	//If amc_flag valus is -1 then AMC call is not required.
	if(amc_flag == -1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Does not match AMC plan change criteria.");
		return 0;
	}

	//Call AMC opcode
	amc_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_POID, acc_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_SERVICE_OBJ, svc_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_USERID, user_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_FLAGS, &amc_flag, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_MODE, &mode, ebufp);
		
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC input", amc_flistp);
        PCM_OP(ctxp, MSO_OP_CUST_BB_HW_AMC, 0, amc_flistp, &amc_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_BB_HW_AMC", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC output", amc_out_flistp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&amc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&amc_out_flistp, NULL);
	return 0;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_swap_bb_ott(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	int			status = 0;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*ip_flistp = NULL;
	pin_flist_t		*dev_flistp = NULL;
	pin_flist_t		*device_flistp = NULL;
	pin_flist_t		*lc_in_flistp = NULL;
	pin_flist_t             *device_out_flistp=NULL;
	int32			*reason_id = NULL;
	int 			disas_flag;
	int 			assoc_flag;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_bb_ott", i_flistp);
	
	lc_in_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

	//SET REASON ID AS PER INPUT

	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 1, ebufp) != NULL)
	{
	 	reason_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 1, ebufp);
		 if (*reason_id == MSO_DEVICE_CHANGE_FAULTY_DEVICE )
		 {
	         	disas_flag = MSO_DEVICE_DEACTIVATION_REASON_FAULTY;
  	         	assoc_flag = MSO_DEVICE_ACTIVATION_NEW;
	 	}
	 	else if ( *reason_id == MSO_DEVICE_CHANGE_TEMPORARY_DEVICE )
	 	{
	 	       disas_flag = MSO_DEVICE_DEACTIVATION_REASON_TEMPORARY_DEVICE;
	       	assoc_flag = MSO_DEVICE_ACTIVATION_NEW;
	 	}
	}
	else
	{
		assoc_flag = MSO_DEVICE_ACTIVATION_NEW;
		disas_flag = MSO_DEVICE_DEACTIVATION_REASON_SERVICE_CLOSURE;
	}

	ip_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ip_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, ip_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, ip_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OLD_VALUE, ip_flistp, MSO_FLD_SERIAL_NO, ebufp);

	// CHECK OLD DEVICE EXISTENCE AND TYPE
	fm_mso_get_device_det_reg(ctxp, ip_flistp, &dev_flistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(dev_flistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		status=2;
		goto CLEANUP_ERROR;
	}

	res_flistp = PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	if (strcmp(PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_DEVICE_TYPE,1, ebufp),OTT_MODEM)!=0)
	{
		status=4;
		goto CLEANUP_ERROR;
	}

	//DEASSOCIATE OLD OTT BOX

	device_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, device_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, lc_in_flistp, MSO_FLD_OLD_DEVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, device_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_FLAGS, &disas_flag, ebufp);
	res_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_SERVICES, 0,ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, res_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, res_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device DisAssociation input Flist is : ", device_flistp);
	PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, device_flistp, &device_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device DisAssociation input Flist is : ", device_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
	     PIN_ERRBUF_RESET(ebufp);
	     status = 5;
	     goto CLEANUP_ERROR;
	}
	PIN_FLIST_DESTROY_EX(&device_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&device_out_flistp, ebufp);

	// CHECK NEW DEVICE EXISTENCE AND TYPE

	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NEW_VALUE, ip_flistp, MSO_FLD_SERIAL_NO, ebufp);
	
	fm_mso_get_device_det_reg(ctxp, ip_flistp, &dev_flistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(dev_flistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		status=3;
		goto CLEANUP_ERROR;
	}

	res_flistp = PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	if (strcmp(PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_DEVICE_TYPE,1, ebufp),OTT_MODEM)!=0)
	{
		status=4;
		goto CLEANUP_ERROR;
	}
	if (*(int *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_STATE_ID,1, ebufp)!= MSO_MODEM_STATE_GOOD)
	{
		status=7;
		goto CLEANUP_ERROR;
	}


	//ASSOCIATE NEW OTT BOX

	device_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, device_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, lc_in_flistp, MSO_FLD_NEW_DEVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, device_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(device_flistp, PIN_FLD_FLAGS, &assoc_flag, ebufp);
	res_flistp = PIN_FLIST_ELEM_ADD(device_flistp, PIN_FLD_SERVICES, 0,ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, res_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, res_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device DisAssociation input Flist is : ", device_flistp);
	PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, device_flistp, &device_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device DisAssociation input Flist is : ", device_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
	     PIN_ERRBUF_RESET(ebufp);
	     status = 6;
	     goto CLEANUP_ERROR;
	}
	PIN_FLIST_DESTROY_EX(&device_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&device_out_flistp, ebufp);

	err_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, err_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, err_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OLD_VALUE, err_flistp, PIN_FLD_OLD_VALUE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NEW_VALUE, err_flistp, PIN_FLD_NEW_VALUE, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, "SWAP OTT SUCCESS", ebufp);

	// LIFECYCLE CREATION
	if (!(PIN_ERRBUF_IS_ERR(ebufp)))
	{
		fm_mso_create_lifecycle_evt(ctxp, lc_in_flistp, NULL, ID_SWAP_OTT, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Error in Lifecycle event creation",lc_in_flistp);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Lifecycle event creation", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
	}


CLEANUP_ERROR : 
	if(status > 1)
	{
		PIN_ERRBUF_RESET(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, err_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, err_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OLD_VALUE, err_flistp, PIN_FLD_OLD_VALUE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NEW_VALUE, err_flistp, PIN_FLD_NEW_VALUE, ebufp);
		
		if(status == 2)
		{
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55882",ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Old Device Search Failed.", ebufp);
		}
		if(status == 3)
		{
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55883",ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"New Device Search Failed.", ebufp);
		}
		if(status == 4)
		{
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55884",ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Old or New Device is not OTT,Check Device type.", ebufp);
		}
		if(status == 5)
		{
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55885",ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Disassociation Failed for Old OTT Box.", ebufp);
		}
		if(status == 6)
		{
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55886",ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Association Failed for New OTT Box.", ebufp);
		}
		if(status == 7)
		{
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"55887",ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"New OTT Box is not in Free State.", ebufp);
		}	
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, "SWAP OTT FAILURE", ebufp);
	}
	
	*r_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST AFTER SWAP OTT DEVICE", *r_flistpp);

	// MEMORY MANAGEMENT
	PIN_FLIST_DESTROY_EX(&ip_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&dev_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);	
	PIN_FLIST_DESTROY_EX(&lc_in_flistp, ebufp);
			
    return;
}

/* ************************************************************************
 * Jyothirmayi Kachiraju === Changes for JIO-CPS Integration-ISP AD
 * This function takes the information of two devices and validates
 * whether the device types of both the devices are of same type or not.
 * If same type, returns 0, otherwise returns 1.
 ************************************************************************/
int 
fm_mso_cust_validate_device_types(
	pcm_context_t	*ctxp, 
	pin_flist_t		*old_device_flistp,
	pin_flist_t		*new_device_flistp,	
	pin_errbuf_t	*ebufp)
{
	int 			device_type_mismatch = 0;
	int 			old_device_type = 0;
	int 			new_device_type = 0;	
	char 			*old_device_type_str = NULL;
	char 			*new_device_type_str = NULL;
	
	if(old_device_flistp != NULL && new_device_flistp !=NULL)
	{
		old_device_type_str = (char *)PIN_FLIST_FLD_GET(old_device_flistp, PIN_FLD_DEVICE_TYPE, 1, ebufp);
		new_device_type_str = (char *)PIN_FLIST_FLD_GET(new_device_flistp, PIN_FLD_DEVICE_TYPE, 1, ebufp);
		
		if(old_device_type_str != NULL && new_device_type_str != NULL)
		{	
			old_device_type = atoi(old_device_type_str);	
			new_device_type = atoi(new_device_type_str);
			
			// If Old Device is JIO (35) and New Device is Non JIO (any), then throw error
			if( strcmp(old_device_type_str, JIO_GPON_MODEM) == 0 && strcmp(new_device_type_str, JIO_GPON_MODEM) != 0 )
			{
				device_type_mismatch = 1;
			}
			// If Old Device is Non JIO GPON or DOCSIS 2 or DOCSIS 3 Device (30 / 31 / 10 / 20) 
			// and New Device is JIO (35), then throw error
			else if( ( strcmp(old_device_type_str, HW_GPON_MODEM) == 0 ||
					   strcmp(old_device_type_str, HW_ONU_GPON_MODEM) == 0 ||
					   old_device_type == DOCSIS2_CABLE_MODEM ||
					   old_device_type == DOCSIS3_CABLE_MODEM ) && 
					  strcmp(new_device_type_str, JIO_GPON_MODEM) == 0 )
			{
				device_type_mismatch = 1;
			}
			// If Old Device is Non JIO DOCSIS 3 Device (20) and 
			// New Device is JIO GPON (35) or Hathway GPON (30 / 31) or DOCSIS 2 (10), then throw error
			else if( ( old_device_type == DOCSIS3_CABLE_MODEM ) && 
					 ( strcmp(new_device_type_str, HW_GPON_MODEM) == 0 ||
					   strcmp(new_device_type_str, HW_ONU_GPON_MODEM) == 0 ||
						   old_device_type == DOCSIS2_CABLE_MODEM ) )
			{
				device_type_mismatch = 1;
			}
			// If Old Device is Non JIO DOCSIS 2 Device (10) and 
			// New Device is Hathway GPON (30 / 31) or JIO GPON (35), then throw error
			else if( ( old_device_type == DOCSIS2_CABLE_MODEM ) && 
					 ( strcmp(new_device_type_str, HW_GPON_MODEM) == 0 ||
					   strcmp(new_device_type_str, HW_ONU_GPON_MODEM) == 0 ||
					   strcmp(new_device_type_str, JIO_GPON_MODEM) == 0) )
			{
				device_type_mismatch = 1;
			}			
		}
	}
	
	return device_type_mismatch;
}
