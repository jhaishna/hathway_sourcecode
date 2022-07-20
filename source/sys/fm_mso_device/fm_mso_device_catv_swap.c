/*
 * (#)$Id: fm_mso_device_catv_swap.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $
 *
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates. All rights reserved.
 *
 * This material is the confidential property of Oracle Corporation or its
 * licensors and may be used, reproduced, stored or transmitted only in
 * accordance with a valid Oracle license or sublicense agreement.
 *
 */
/***************************************************************************************************************
*VERSION |   DATE    |    Author        |               DESCRIPTION                                            *
*--------------------------------------------------------------------------------------------------------------*
* 0.1    |12/12/2013 |Satya Prakash     | Device swap of STB and Smart Card for CATV service
*
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/


#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_device_catv_swap.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <pcm.h>
#include <pinlog.h>
#include "ops/device.h"
#include "pin_device.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "mso_device.h"
#include "mso_prov.h"
#include "mso_ops_flds.h"
#include "mso_lifecycle_id.h"


#define FILE_LOGNAME "fm_mso_device_catv_swap.c"
#define READY_TO_USE 1
#define MSO_DEVICE_TYPE_SD  0
#define MSO_DEVICE_TYPE_HD  1
#define CUSTOMER_CARE "/profile/customer_care"
#define MSO_FLAG_SRCH_BY_SELF_ACCOUNT 9

/*******************************************************************************
 * Functions Defined outside this source file
 ******************************************************************************/
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

extern int32
fm_mso_is_den_sp(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_mso_populate_bouquet_id(
        pcm_context_t	*ctxp,
        pin_flist_t	*i_flistp,
        int32		mso_device_type,
        int32		city_only,
        pin_flist_t	**ret_flistp,
        pin_errbuf_t	*ebufp);

PIN_IMPORT void
fm_mso_get_account_info(
        pcm_context_t   *ctxp,
        poid_t          *acnt_pdp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT void
fm_mso_get_base_pdts(
	pcm_context_t	*ctxp,
	poid_t		*acct_pdp,
	poid_t		*srvc_pdp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp);


void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_device_catv_swap(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_catv_swap(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    int32               *error_flag,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_update_device(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    pin_errbuf_t        *ebufp);

PIN_EXPORT void
fm_mso_get_device_details(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *device_id,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp);

static void
fm_mso_device_swap_create_provisioning_request(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    pin_errbuf_t         *ebufp);

/*******************************************************************************

 ******************************************************************************/
void
op_mso_device_catv_swap(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp)
{
    pcm_context_t        *ctxp = connp->dm_ctx;
//    int32                *status = NULL;
    poid_t               *pdp = NULL;
    int32                error_set = 0;

    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    /*
     * Insanity check.
     */
    if (opcode != MSO_OP_DEVICE_CATV_SWAP) 
    {

        pin_set_err(ebufp, PIN_ERRLOC_FM,
            PIN_ERRCLASS_SYSTEM_DETERMINATE,
            PIN_ERR_BAD_OPCODE, 0, 0, opcode);

        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "bad opcode in op_mso_device_catv_swap", ebufp);

        *o_flistpp = NULL;
        return;
    }


    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "op_mso_device_catv_swap input flist", i_flistp);


    fm_mso_device_catv_swap(ctxp, i_flistp, &error_set, o_flistpp, ebufp);

    if (PIN_ERRBUF_IS_ERR(ebufp)  &&  error_set == 0) 
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
            "op_mso_device_catv_swap error", ebufp);
        //unhandled error -- Ideally should not 
        error_set = 1;
        PIN_ERRBUF_CLEAR(ebufp);
        *o_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *o_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &error_set, ebufp);
    }
    else if ( ! PIN_ERRBUF_IS_ERR(ebufp) &&   error_set == 0)
    {
        PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &error_set, ebufp);
    }

     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "op_mso_device_catv_swap return flist",
            *o_flistpp);

    return;
}


/*******************************************************************************
 * fm_mso_device_catv_swap()
 *
 * Inputs: flist with device poid and services array, each element including
 *         a service poid and an account poid; flist with device attributes
 * Outputs: void; ebuf set if errors encountered
 *
 * Description:
 * This function validates the input services against the given device, and
 * ensures the state of the device is Assigned if successful.
 ******************************************************************************/
static void
fm_mso_device_catv_swap(
    pcm_context_t        *ctxp,
    pin_flist_t        *i_flistp,
    int32              *error_flag,
    pin_flist_t        **ret_flistpp,
    pin_errbuf_t       *ebufp)
{
	pin_flist_t		*current_device_r_flistp = NULL;
	pin_flist_t		*new_device_r_flistp = NULL;
	pin_flist_t		*assoc_device_flistp = NULL;
	pin_flist_t		*assoc_device_o_flistp = NULL;
	pin_flist_t		*disassoc_device_flistp = NULL;
	pin_flist_t		*disassoc_device_o_flistp = NULL;
	pin_flist_t		*prov_flistp = NULL;
	pin_flist_t		*prov_o_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*pair_flistp = NULL;
	pin_flist_t		*depair_flistp = NULL;
	pin_flist_t		*depair_oflistp = NULL;
	pin_flist_t		*depair_oflistp1 = NULL;
	pin_flist_t		*pair_oflistp = NULL;
	pin_flist_t		*lc_in_flistp = NULL;
	pin_flist_t		*read_iflistp = NULL;
	pin_flist_t		*read_oflistp = NULL;
	pin_flist_t		*prod_oflistp = NULL;
	poid_t			*new_device_poidp = NULL;
	poid_t			*current_poidp = NULL;
	poid_t			*lco_poidp = NULL;
	pin_cookie_t		cookie = NULL;
	char			*program_namep = NULL;
	char			prov_prog_name[128] = "";
	char			*new_device_type = NULL;
	char			*old_device_type = NULL;
	char			*old_dev_makep = NULL;
	char			*new_dev_makep = NULL;
	char			*current_device = NULL;
	char			*curr_paired_dev = NULL;
	char			*new_paired_dev = NULL;	
	
	char                    *new_device = NULL;
	char                    *device_type = NULL;
	char                    *acc_nop = NULL;
	char			*areap = NULL;
	char			*bouquet_id = NULL;
	int32                   disassoc_flag;
	int32                   assoc_flag;
	int32                   prov_flag;
	int32			elem_id = 0;
	poid_t                  *acc_poidp = NULL;
	poid_t                  *service_poidp = NULL;
	int32                   *flags = NULL;
	poid_t                  *pdp = NULL;
	poid_t                  *lco_current_pdp = NULL;
	poid_t                  *lco_new_pdp = NULL;
	int32			prov_action_activate = 2;
	int32			count = 0;
	int32			flag_old = 0;
	int32			swap_flag = 0;
	int32			flag_new = 1;
    	int32			t_status = -1;
	int32			mso_device_type = MSO_DEVICE_TYPE_SD;
	int32			prov_call = 0;
	int32			profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;
	void		    *vp = NULL;
	void		    *vp1 = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*
	 * Validate the input flist if required fields are present
	 */
	current_device = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OLD_VALUE, 0, ebufp);
	new_device =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NEW_VALUE, 0, ebufp);
	flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	device_type = (char *)PIN_POID_GET_TYPE(pdp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_device_catv_swap error: required field missing in input flist", ebufp);
		return;
	}

	program_namep = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	lc_in_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

	/*
	 * Get Current device and associated service poid and account poid
	 */
	fm_mso_get_device_details(ctxp, i_flistp, current_device, &current_device_r_flistp, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_GET(current_device_r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (tmp_flistp)
	{
		current_poidp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);
		lco_current_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		acc_nop = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SOURCE, 0, ebufp);
		old_dev_makep = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_MANUFACTURER, 0, ebufp);
		old_device_type = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_DEVICE_TYPE, 0, ebufp);
		current_device = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_OLD_VALUE, current_device, ebufp);
		//sub_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
		sub_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_SERVICES,PCM_RECID_ALL, 0, ebufp);
		service_poidp = PIN_FLIST_FLD_GET(sub_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		acc_poidp = PIN_FLIST_FLD_GET(sub_flistp , PIN_FLD_ACCOUNT_OBJ, 0, ebufp);		
		if(PIN_FLIST_ELEM_COUNT(tmp_flistp, PIN_FLD_GROUP_DETAILS, ebufp) > 0)
		{
			pair_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_GROUP_DETAILS,PCM_RECID_ALL, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "group_detail_array", pair_flistp);
			curr_paired_dev = PIN_FLIST_FLD_GET(pair_flistp, PIN_FLD_NAME,0, ebufp);
		}
	}
	if (PIN_ERRBUF_IS_ERR(ebufp) || tmp_flistp == NULL)
	{
		*error_flag = 1;
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52011", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Current Device id doesn't exist !!!", ebufp);
		goto CLEANUP;
	}

	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ, acc_poidp, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SERVICE_OBJ, service_poidp, ebufp);
	/*
	 * Get new device poid
	 */
	fm_mso_get_device_details(ctxp, i_flistp, new_device, &new_device_r_flistp, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_GET(new_device_r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (tmp_flistp)
	{
		new_device_poidp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 0, ebufp);
		new_device_type = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_DEVICE_TYPE, 0, ebufp);
		lco_new_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		new_device = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_DEVICE_ID, 0, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_NEW_VALUE, new_device, ebufp);
		if (new_device_type && strstr(new_device_type, "HD"))
		{
			mso_device_type = MSO_DEVICE_TYPE_HD;
		}
		new_dev_makep = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_MANUFACTURER, 0, ebufp);
		assoc_flag = *(int32 *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATE_ID, 0, ebufp);
		if(PIN_FLIST_ELEM_COUNT(tmp_flistp, PIN_FLD_GROUP_DETAILS, ebufp) > 0)
		{			
			pair_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_GROUP_DETAILS,PCM_RECID_ALL, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "group_detail_array", pair_flistp);
			new_paired_dev = PIN_FLIST_FLD_GET(pair_flistp, PIN_FLD_NAME,0, ebufp);
		}
	}
	if (PIN_ERRBUF_IS_ERR(ebufp) || tmp_flistp == NULL)
	{
		*error_flag = 1;
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52012", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "New Device id doesn't exist !!!", ebufp);
		goto CLEANUP;
	}

	if (assoc_flag != MSO_STB_STATE_GOOD && assoc_flag != MSO_STB_STATE_REPAIRED)
	{
		*error_flag = 1;
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52010", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "New Device id is not ready to use", ebufp);
		goto CLEANUP;
	}

	if (*flags == MSO_DEVICE_CHANGE_FAULTY_DEVICE )
	{
		disassoc_flag = MSO_DEVICE_DEACTIVATION_REASON_FAULTY;
		assoc_flag = MSO_DEVICE_ACTIVATION_REPLACEMENT;
	}
	else if ( *flags == MSO_DEVICE_CHANGE_TEMPORARY_DEVICE )
	{
		disassoc_flag = MSO_DEVICE_DEACTIVATION_REASON_TEMPORARY_DEVICE;
		assoc_flag = MSO_DEVICE_ACTIVATION_REPLACEMENT;
	}
	else
	{
		PIN_ERRBUF_CLEAR(ebufp);
		*error_flag = 1;
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52003", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Reason of Device Swap", ebufp);
		goto CLEANUP;
	}

    	t_status = fm_mso_trans_open(ctxp, acc_poidp, 1, ebufp);
	/******************************************************
	 * This is technology change option
	 ******************************************************/
	if (strcmp(old_dev_makep, new_dev_makep) != 0 || 
		(strstr(old_dev_makep, "CISCO") && strstr(new_dev_makep, "CISCO")) ||
		(strstr(old_dev_makep, "GOSPELL") && strstr(new_dev_makep, "GOSPELL")) ||
		(strstr(old_dev_makep, "NAGRA") && strstr(new_dev_makep, "NAGRA")))
	{
		assoc_flag = MSO_DEVICE_ACTIVATION_NEW;
		disassoc_flag = MSO_DEVICE_DEACTIVATION_REASON_TEMPORARY_DEVICE;
	}

	/*******************************************************
	 * NDS1 P2 to P1 swap is like terminate and activate
	 ******************************************************/
	if ((strstr(old_dev_makep, "NDS1") && strstr(new_dev_makep, "NDS1")) && 
		strstr(new_device_type, "P1") && strstr(old_device_type, "P2"))
	{
		assoc_flag = MSO_DEVICE_ACTIVATION_NEW;
		disassoc_flag = MSO_DEVICE_DEACTIVATION_REASON_TEMPORARY_DEVICE;
	}
	/*
	 * Disassociate existing device 
	 */

	disassoc_device_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(disassoc_device_flistp, PIN_FLD_POID, (void *) current_poidp, ebufp);
	PIN_FLIST_FLD_SET(lc_in_flistp, MSO_FLD_OLD_DEVICE_OBJ, (void *) current_poidp, ebufp);
	if (program_namep)
	{
		PIN_FLIST_FLD_SET(disassoc_device_flistp, PIN_FLD_PROGRAM_NAME, program_namep, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(disassoc_device_flistp, PIN_FLD_PROGRAM_NAME, "MSO_Device_Swap", ebufp);
	}
	PIN_FLIST_FLD_SET(disassoc_device_flistp, PIN_FLD_FLAGS, &disassoc_flag, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(disassoc_device_flistp, PIN_FLD_SERVICES, 0,ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, (void *) acc_poidp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, (void *) service_poidp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_device_catv_swap disassociate device input flist ", disassoc_device_flistp);
	PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, disassoc_device_flistp,
		&disassoc_device_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_device_catv_swap disassociate device output flist", disassoc_device_o_flistp);


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		*error_flag = 1;
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52004", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Disassociation Failed for Current Device id", ebufp);
		goto CLEANUP;
	}

	if (strstr(old_dev_makep, "NDS1") && strstr(new_dev_makep, "NDS1") )
	{
		depair_flistp = PIN_FLIST_CREATE(ebufp);
		if (curr_paired_dev && strlen(curr_paired_dev) > 0 ) 
		{			
			if (strcmp(device_type, MSO_OBJ_TYPE_DEVICE_STB) == 0)
			{
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_VC_ID, (void *) curr_paired_dev, ebufp);
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_STB_ID, (void *) current_device, ebufp);
			}
			else if (strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC) == 0)
			{
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_VC_ID, (void *) current_device, ebufp);
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_STB_ID, (void *) curr_paired_dev, ebufp);
			}
			swap_flag = 3;
			PIN_FLIST_FLD_SET(depair_flistp, PIN_FLD_FLAGS, &swap_flag, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap depair device input flist ", depair_flistp);
			PCM_OP(ctxp, MSO_OP_DEVICE_PAIR, 0, depair_flistp,
				&depair_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap depair device output flist", depair_oflistp);
			PIN_FLIST_DESTROY_EX(&depair_flistp, NULL);
		}

		if (new_paired_dev && strlen(new_paired_dev) > 0 ) 
		{
			depair_flistp = PIN_FLIST_CREATE(ebufp);
			if (strcmp(device_type, MSO_OBJ_TYPE_DEVICE_STB) == 0)
			{
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_VC_ID, (void *) new_paired_dev, ebufp);
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_STB_ID, (void *) new_device, ebufp);
			}
			else if (strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC) == 0)
			{
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_VC_ID, (void *) new_device, ebufp);
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_STB_ID, (void *) new_paired_dev, ebufp);
			}
			swap_flag = 3;
			PIN_FLIST_FLD_SET(depair_flistp, PIN_FLD_FLAGS, &swap_flag, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap depair new device input flist ", depair_flistp);
			PCM_OP(ctxp, MSO_OP_DEVICE_PAIR, 0, depair_flistp,
				&depair_oflistp1, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap depair device output flist", depair_oflistp1);
			PIN_FLIST_DESTROY_EX(&depair_flistp, NULL);
		}

		
		depair_flistp = PIN_FLIST_CREATE(ebufp);
			if (strcmp(device_type, MSO_OBJ_TYPE_DEVICE_STB) == 0)
			{
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_VC_ID, (void *) curr_paired_dev, ebufp);
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_STB_ID, (void *) new_device, ebufp);
			}
			else if (strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC) == 0)
			{
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_VC_ID, (void *) new_device, ebufp);
				PIN_FLIST_FLD_SET(depair_flistp, MSO_FLD_STB_ID, (void *) curr_paired_dev, ebufp);
			}
			swap_flag = 2;
			PIN_FLIST_FLD_SET(depair_flistp, PIN_FLD_FLAGS, &swap_flag, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap pair new device input flist ", depair_flistp);
			PCM_OP(ctxp, MSO_OP_DEVICE_PAIR, 0, depair_flistp,
				&pair_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap pair device output flist", pair_oflistp);


	}



	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		*error_flag = 1;
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52004", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Pairing/Depairing Failed for Current/New Device id", ebufp);
		goto CLEANUP;
	}

	if (strcmp(device_type, MSO_OBJ_TYPE_DEVICE_STB) == 0 && strstr(old_dev_makep, "NDS") &&
		!strstr(new_dev_makep, "NDS"))
	{
		fm_mso_terminate_service_device_deassociate(ctxp, i_flistp, ebufp);
	}	
	/*
	 * Associate New device 
	 */
	assoc_device_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(assoc_device_flistp, PIN_FLD_POID, (void *)new_device_poidp, ebufp);
	PIN_FLIST_FLD_SET(lc_in_flistp, MSO_FLD_NEW_DEVICE_OBJ, (void *) new_device_poidp, ebufp);
	if (program_namep)
	{
		PIN_FLIST_FLD_SET(assoc_device_flistp, PIN_FLD_PROGRAM_NAME, program_namep, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(assoc_device_flistp, PIN_FLD_PROGRAM_NAME, "MSO_Device_Swap", ebufp);
	}
	PIN_FLIST_FLD_SET(assoc_device_flistp, PIN_FLD_FLAGS, &assoc_flag, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(assoc_device_flistp, PIN_FLD_SERVICES, 0,ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, (void *) acc_poidp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, (void *) service_poidp, ebufp);
	/* Add previous STB device poid in extended info for non-NDS devices */
	if(!strstr(old_dev_makep, "NDS")){
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(assoc_device_flistp, PIN_FLD_EXTENDED_INFO,ebufp);
		//tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, MSO_FLD_STB_DATA, 0,ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_VC_OBJ, (void *) current_poidp, ebufp);

	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_device_catv_swap associate device input flist ", assoc_device_flistp);
	PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE, 0, assoc_device_flistp,
			&assoc_device_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_device_catv_swap associate device output flist", assoc_device_o_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		*error_flag = 1;
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52005", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Association Failed for New Device id", ebufp);
		goto CLEANUP;
	}

	/*
	 * Generate provisioning request
	 */
	prov_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_POID, (void *) acc_poidp, ebufp);
	PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_SERVICE_OBJ, (void *) service_poidp, ebufp);
	
	if (t_status == 0)
	{
		PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_TRANS_DONE, &t_status, ebufp);
	}

	read_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, acc_poidp, ebufp);
	PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_OBJECT, acc_poidp, ebufp);
	PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_PROFILE_DESCR, CUSTOMER_CARE, ebufp);
	PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);
	fm_mso_get_profile_info(ctxp, read_iflistp, ret_flistpp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);

	tmp_flistp = PIN_FLIST_SUBSTR_GET(*ret_flistpp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
	lco_poidp = PIN_FLIST_FLD_TAKE(tmp_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
	PIN_FLIST_DESTROY_EX(ret_flistpp, NULL);
	
	PIN_FLIST_FLD_SET(prov_flistp, MSO_FLD_LCO_OBJ, lco_poidp, ebufp);

	/*if ((strstr(old_dev_makep, "NDS") && strstr(new_dev_makep, "NDS")) ||
		(strstr(old_dev_makep, "VM") && strstr(new_dev_makep, "VM"))) */
	if (assoc_flag != MSO_DEVICE_ACTIVATION_NEW)
	{
		/***********************************************************
		 * This is for the same technology
		 ***********************************************************/

		if (strcmp(device_type,MSO_OBJ_TYPE_DEVICE_VC) == 0)
		{
			prov_flag = CATV_CHANGE_VC;
			PIN_FLIST_FLD_SET(prov_flistp, MSO_FLD_VC_ID, new_device, ebufp);
		}
		else if ( strcmp(device_type, MSO_OBJ_TYPE_DEVICE_STB) == 0)
		{
			prov_flag = CATV_CHANGE_STB;
			if (strstr(new_dev_makep, "VM"))
			{
				PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_OLD_VALUE, current_device, ebufp);
			}
			PIN_FLIST_FLD_SET(prov_flistp, MSO_FLD_STB_ID, new_device, ebufp);
		}

		PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_FLAGS, &prov_flag, ebufp); 
		PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_DEVICE_TYPE, new_device_type, ebufp);

		/* Write USERID, PROGRAM_NAME in buffer - start */
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_USERID,prov_flistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,prov_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OLD_VALUE, prov_flistp, PIN_FLD_OLD_VALUE, ebufp);
		/* Write USERID, PROGRAM_NAME in buffer - end */
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap provisioning request input flist ", prov_flistp);
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, prov_flistp, &prov_o_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap provisioning request output flist", prov_o_flistp);
	}
	else
	{
		/*****************************************************
		 * Terminate old service
		 *****************************************************/
		read_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, service_poidp, ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_LOGIN, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_STATUS, NULL, ebufp);

		PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, read_iflistp, &read_oflistp, ebufp);	

		PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);

		PIN_FLIST_FLD_COPY(read_oflistp, PIN_FLD_LOGIN, prov_flistp, PIN_FLD_LOGIN, ebufp);

		prov_flag = CATV_TERMINATE;	
		PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_FLAGS, &prov_flag, ebufp); 

		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, prov_flistp, PIN_FLD_USERID, ebufp);
		strcpy(prov_prog_name, "SWAP|");
		strcat(prov_prog_name, program_namep);	
		PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_PROGRAM_NAME, prov_prog_name, ebufp);
		PIN_FLIST_FLD_SET(prov_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OLD_VALUE, prov_flistp, PIN_FLD_OLD_VALUE, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap prov service terminate in_flist ", prov_flistp);
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, prov_flistp, &prov_o_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap prov service terminate out_flist", prov_o_flistp);

		if (prov_o_flistp)	
		{
			vp = PIN_FLIST_FLD_GET(prov_o_flistp, PIN_FLD_STATUS, 1, ebufp);
			if (vp && *(int32 *)vp == 1)
			{
				prov_call = 1;
			}
		}

		if (PIN_ERRBUF_IS_ERR(ebufp) || prov_o_flistp == NULL || prov_call == 1)
		{	
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}
			*error_flag = 1;
			*ret_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52006", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Error in terminate device swap provisioning Request", ebufp);
		}

		fm_mso_get_account_info(ctxp, acc_poidp, ret_flistpp, ebufp);

		areap = PIN_FLIST_FLD_TAKE(*ret_flistpp, MSO_FLD_AREA, 0, ebufp);
		PIN_FLIST_DESTROY_EX(ret_flistpp, NULL);
		
		read_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_ACCOUNT_OBJ, acc_poidp, ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, MSO_FLD_NETWORK_NODE, new_dev_makep, ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, MSO_FLD_DEVICE_TYPE, &mso_device_type, ebufp);
		PIN_FLIST_FLD_PUT(read_iflistp, MSO_FLD_AREA, areap, ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, MSO_FLD_LCO_OBJ, lco_poidp, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(read_iflistp, PIN_FLD_SERVICES, 0, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(tmp_flistp, MSO_FLD_CATV_INFO, ebufp);

		PIN_FLIST_DESTROY_EX(ret_flistpp, NULL);
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_device_catv_swap get bouquet id in_flist", read_iflistp);

		fm_mso_populate_bouquet_id(ctxp, read_iflistp, mso_device_type, 0, ret_flistpp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_device_catv_swap get bouquet id out_flist", read_iflistp);

		tmp_flistp = PIN_FLIST_ELEM_GET(read_iflistp, PIN_FLD_SERVICES, 0, 0, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_GET(tmp_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		bouquet_id = PIN_FLIST_FLD_TAKE(tmp_flistp, MSO_FLD_BOUQUET_ID, 0, ebufp);
	
		PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);

		read_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, service_poidp, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(read_iflistp, MSO_FLD_CATV_INFO, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_NETWORK_NODE, new_dev_makep, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
		PIN_FLIST_FLD_PUT(tmp_flistp, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);

		prov_flag = CATV_ACTIVATION;	
		PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_FLAGS, &prov_flag, ebufp); 
		if (t_status == 0)
		{
			PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_TRANS_DONE, &t_status, ebufp);
		}

		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, read_iflistp, ret_flistpp, ebufp);

		PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(ret_flistpp, NULL);
		/********************************************************
		 * Get active purchased products
		 *******************************************************/
		fm_mso_get_purchased_prod_active(ctxp, prov_flistp, &prod_oflistp, ebufp);

		while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(prod_oflistp, PIN_FLD_RESULTS,
						&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			sub_flistp = PIN_FLIST_ELEM_ADD(prov_flistp, PIN_FLD_PRODUCTS, count, ebufp); 
			PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_POID, sub_flistp, PIN_FLD_POID, ebufp);	
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS_FLAGS, &prov_action_activate, ebufp);
			count++;
		}
		
		PIN_FLIST_FLD_DROP(prov_flistp, PIN_FLD_OLD_VALUE, ebufp);
		PIN_FLIST_FLD_DROP(prov_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, ebufp);
		PIN_FLIST_FLD_SET(prov_flistp, MSO_FLD_DEVICE_TYPE, &mso_device_type, ebufp);

    		PIN_FLIST_DESTROY_EX(&prov_o_flistp, NULL);

		/****************************************************
                 * Get base pack plan POID
                 ****************************************************/
                fm_mso_get_base_pdts(ctxp, acc_poidp, service_poidp, &prov_o_flistp, ebufp);

		if (prov_o_flistp)
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(prov_o_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			if (tmp_flistp)
			{
				PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_PLAN_OBJ, prov_flistp, PIN_FLD_PLAN_OBJ, ebufp);
			}
			PIN_FLIST_DESTROY_EX(&prov_o_flistp, NULL);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap prov service activate in_flist ", prov_flistp);
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, prov_flistp, &prov_o_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"fm_mso_device_catv_swap prov service activate out_flist", prov_o_flistp);

		PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
	}

	if (prov_o_flistp)	
	{
		vp = PIN_FLIST_FLD_GET(prov_o_flistp, PIN_FLD_STATUS, 1, ebufp);
		if (vp && *(int32 *)vp == 1)
		{
			prov_call = 1;
		}
	}

	if (PIN_ERRBUF_IS_ERR(ebufp) || prov_o_flistp == NULL || prov_call == 1)
	{
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}
		*error_flag = 1;
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, error_flag, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "52007", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Error in activatate device swap provisioning Request", ebufp);
	}

	/***********************************************************
	 * Lifecycle events
	 ***********************************************************/
	if (/*action_flag = DOC_BB_MODEM_CHANGE &&*/ !(PIN_ERRBUF_IS_ERR(ebufp)) )
	{
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(disassoc_device_flistp, PIN_FLD_SERVICES, 0, 1, ebufp ) ;
		if (vp)
		{
			PIN_FLIST_FLD_COPY(vp, PIN_FLD_ACCOUNT_OBJ, lc_in_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(vp, PIN_FLD_SERVICE_OBJ, lc_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(vp, PIN_FLD_POID, lc_in_flistp, MSO_FLD_OLD_DEVICE_OBJ, ebufp);

			vp1 = PIN_FLIST_ELEM_ADD(lc_in_flistp, PIN_FLD_DEVICES, 0, ebufp);
			PIN_FLIST_FLD_COPY(lc_in_flistp, PIN_FLD_OLD_VALUE, vp1, PIN_FLD_DEVICE_ID, ebufp);
			PIN_FLIST_FLD_RENAME(lc_in_flistp, PIN_FLD_FLAGS, PIN_FLD_REASON_ID, ebufp)
				PIN_FLIST_FLD_SET(vp1, PIN_FLD_FLAGS, &flag_old, ebufp );
		}

		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(assoc_device_flistp, PIN_FLD_SERVICES, 0, 1, ebufp ) ;
		if (vp)
		{
			PIN_FLIST_FLD_COPY(vp, PIN_FLD_POID, lc_in_flistp, MSO_FLD_NEW_DEVICE_OBJ, ebufp);

			vp1 = PIN_FLIST_ELEM_ADD(lc_in_flistp, PIN_FLD_DEVICES, 1, ebufp);
			PIN_FLIST_FLD_COPY(lc_in_flistp, PIN_FLD_NEW_VALUE, vp1, PIN_FLD_DEVICE_ID, ebufp);
			PIN_FLIST_FLD_SET(vp1, PIN_FLD_FLAGS, &flag_new, ebufp );
		}

		PIN_FLIST_FLD_DROP(lc_in_flistp, PIN_FLD_OLD_VALUE ,ebufp)
		PIN_FLIST_FLD_DROP(lc_in_flistp, PIN_FLD_NEW_VALUE ,ebufp)
		
		fm_mso_create_lifecycle_evt(ctxp, lc_in_flistp, NULL, ID_SWAP_DEVICE, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Lifecycle event creation", ebufp);
			PIN_ERRBUF_CLEAR(ebufp);
			goto CLEANUP;
		}
	}

	/***********************************************************
	 * Check current device LCO and new device LCO accounts
	 * If not matching, then do movement
	 **********************************************************/
	if (PIN_POID_COMPARE(lco_new_pdp, lco_current_pdp, 0, ebufp) != 0)
	{
		read_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, read_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, read_iflistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, read_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_DEVICE_TYPE, new_device_type, ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_ACCOUNT_NO, acc_nop, ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_OVERRIDE_FLAGS, &flag_new, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NEW_VALUE, read_iflistp, PIN_FLD_DEVICE_ID, ebufp);

		PCM_OP(ctxp, MSO_OP_DEVICE_SET_ATTR, 0, read_iflistp, ret_flistpp, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in location update", ebufp);
			PIN_ERRBUF_CLEAR(ebufp);
			goto CLEANUP;
		}
		PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(ret_flistpp, NULL);
	}
    if (program_namep && !strstr(program_namep, "MyJIO"))
    {
        swap_flag = 32;
        prov_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, prov_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, prov_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERIAL_NO, prov_flistp, MSO_FLD_SERIAL_NO, ebufp);
        PIN_FLIST_FLD_SET(prov_flistp, PIN_FLD_FLAGS, &swap_flag, ebufp);

        PIN_FLIST_DESTROY_EX(&prov_o_flistp, NULL);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_swap_service_status notification input list", prov_flistp);

        PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, prov_flistp, &prov_o_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&prov_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in MSO_OP_NTF_CREATE_NOTIFICATION", ebufp);
            PIN_ERRBUF_CLEAR(ebufp);
            goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_swap_service_status notification output flist", prov_o_flistp);
    }

	*ret_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, acc_poidp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OLD_VALUE, *ret_flistpp, PIN_FLD_OLD_VALUE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NEW_VALUE, *ret_flistpp, PIN_FLD_NEW_VALUE, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp, MSO_FLD_LCO_OBJ, lco_poidp, ebufp); 

CLEANUP:
    PIN_FLIST_DESTROY_EX(&disassoc_device_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&disassoc_device_o_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&assoc_device_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&assoc_device_o_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&current_device_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&new_device_r_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&prov_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&prov_o_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&lc_in_flistp, NULL);
    if (lco_poidp) PIN_POID_DESTROY(lco_poidp, NULL);

    if ((PIN_ERRBUF_IS_ERR(ebufp) || *error_flag == 1 ) && t_status == 0 ) 
    {
        fm_mso_trans_abort(ctxp, acc_poidp, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"abort transaction");
    }
    else if ( ! PIN_ERRBUF_IS_ERR(ebufp) && *error_flag == 0 && t_status == 0)
    {
        fm_mso_trans_commit(ctxp, acc_poidp, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"commit transaction");
    }
    
    return;
}

/*******************************************************************************
 * fm_mso_get_device_details()
 *
 * Description:
 * Search device from /device object
 ******************************************************************************/
void
fm_mso_get_device_details(
    pcm_context_t           *ctxp,
    pin_flist_t             *i_flistp,
    char                    *device_id,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *device_pdp = NULL;
    char                    template[500] = "select X from /device where (F1 = V1 or F3 = V3) and F2.type = V2 "; 
    int32                   s_flags = 0;
    int64                   database = 0;
    char                    *device_type = NULL;
	int32					den_sp = 0;
	char			*den_nwp = "NDS1";
	char			*hw_nwp = "NDS";


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
    database = PIN_POID_GET_DB(pdp); 

    device_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_TYPE, 1, ebufp);
    if (device_type == NULL)
    {
    	device_type = (char *)PIN_POID_GET_TYPE(pdp);
    }

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);



	den_sp = fm_mso_is_den_sp(ctxp, i_flistp, ebufp);

	if (device_type && strcmp(device_type, "/device/vc") == 0)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_details search input flist ", search_i_flistp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 4, ebufp);
		if (den_sp == 1)
		{
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, den_nwp, ebufp);
		}
		else
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_details search input flist ", search_i_flistp);
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_MANUFACTURER, hw_nwp, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_details search input flist ", search_i_flistp);
		
		strcpy(template, "select X from /device where (F1 = V1 or F3 = V3) and F2.type = V2 and F4 = V4 ") ;
	}

	PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);   
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_details search input flist ", search_i_flistp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);

//    if (strcmp(device_type,"VC") == 0 )
//    {
//          device_pdp = (poid_t *)PIN_POID_CREATE(database, "/device/vc", -1, ebufp);
//    }
//    else if(strcmp(device_type,"STB") == 0 )
//    {
//          device_pdp = (poid_t *)PIN_POID_CREATE(database, "/device/stb", -1, ebufp);
//    }

    device_pdp = (poid_t *)PIN_POID_CREATE(database, device_type, -1, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_POID, (void *)device_pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_SERIAL_NO, device_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_DEVICE_TYPE, NULL, ebufp);
    /* Sachid: Adding PIN_FLD_MANUFACTURER for Cisco device swap */
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_MANUFACTURER, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATE_ID, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SOURCE, NULL, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DEVICE_ID, NULL, ebufp);
    PIN_FLIST_ELEM_SET(tmp_flistp, NULL, PIN_FLD_SERVICES, PCM_RECID_ALL, ebufp);
	PIN_FLIST_ELEM_SET(tmp_flistp, NULL, PIN_FLD_GROUP_DETAILS, PCM_RECID_ALL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_details search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        r_flistpp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_get_device_details search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);

    return;
}

