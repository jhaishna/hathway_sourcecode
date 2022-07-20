/*
 * (#)$Id: fm_mso_device_set_attr.c  $
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
* 0.1    |23/02/2015 |Harish Kulkarni     | Set the Device Attributes
*
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/


#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_device_set_attr.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
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

#define FILE_LOGNAME "fm_mso_device_set_attr.c"

#define WHOLESALE "/profile/wholesale"
#define MSO_FLAG_SRCH_BY_SELF_ACCOUNT 9

// Functions contained within this source file

EXPORT_OP void
op_mso_device_set_attr(
	cm_nap_connection_t    *connp,
	int32            opcode,
	int32            flags,
	pin_flist_t        *i_flistp,
	pin_flist_t        **o_flistpp,
	pin_errbuf_t        *ebufp);

static void fm_mso_device_set_attr(
	pcm_context_t   *ctxp,
	pin_flist_t     *task_flistp,
	pin_flist_t     **ret_flistp,
	pin_errbuf_t    *ebufp);


// Functions called from outside
extern void
fm_mso_search_lco_account(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_errbuf_t    *ebufp);

extern void
fm_mso_search_device_object(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_errbuf_t    *ebufp);

extern void
fm_mso_get_profile_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

/*******************************************************************************
 * Main function which does the device Set Attribute:
 ******************************************************************************/

void
op_mso_device_set_attr(
	cm_nap_connection_t    *connp,
	int32            opcode,
	int32            flags,
	pin_flist_t        *i_flistp,
	pin_flist_t        **o_flistpp,
	pin_errbuf_t        *ebufp)
{
	pcm_context_t        *ctxp = connp->dm_ctx;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != MSO_OP_DEVICE_SET_ATTR)
	{

		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_OPCODE, 0, 0, opcode);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"bad opcode in op_mso_device_set_attr", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
				"op_mso_device_set_attr input flist error", i_flistp);

		*o_flistpp = NULL;
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_device_set_attr input flist", i_flistp);

	// Call the function to do the Set Attribute:
	fm_mso_device_set_attr(ctxp, i_flistp, o_flistpp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_mso_device_set_attr error", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
		"op_mso_device_set_attr error - input flist", i_flistp);

		*o_flistpp = NULL;
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_mso_device_set_attr return flist",
				*o_flistpp);
	}

	return;

}

static void fm_mso_device_set_attr(
	pcm_context_t   *ctxp,
	pin_flist_t     *task_flistp,
	pin_flist_t     **ret_flistpp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t     *job_iflistp = NULL;
	pin_flist_t     *write_iflistp = NULL;
	pin_flist_t     *write_oflistp = NULL;
	pin_flist_t     *ipdata_flistp = NULL;
	pin_flist_t	*srch_prof_iflistp = NULL;
	pin_flist_t	*srch_prof_oflistp = NULL;
	pin_flist_t	*wholesale_flistp = NULL;
	pin_flist_t	*ui_flistp = NULL;
	pin_flist_t	*uo_flistp = NULL;
	poid_t          *device_pdp = NULL;
	poid_t          *lco_account_pdp = NULL;
	poid_t          *stock_account_pdp = NULL;
	poid_t          *current_sp_pdp = NULL;
	poid_t          *new_sp_pdp = NULL;
	char            *device_id = NULL;
	char            *account_no = NULL;
	char            *device_obj_type = NULL;
	int32		*business_typep = NULL;
	int32           status_sucess = 0;
	int32           status_fail = 1;
	int32           *upd_flagsp = NULL;
	int32           over_flag = 0;
	int32           state_id = 0;
	int32		customer_type = 0;
	int32		profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;

    /*repai counter update*/

    int32       *rep_flagp = NULL;
    int32       *repair_cntp = NULL;
    time_t      *rep_warranty_endp = NULL;
    char         repair_cntp_msg[256];
    char        rep_warranty_endp_msg[256];
    char        repair_flagp_msg[256];

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " fm_mso_device_set_attr input flist is " , task_flistp);

    rep_flagp = PIN_FLIST_FLD_TAKE(task_flistp, PIN_FLD_FLAGS, 1, ebufp);

    if(rep_flagp)
    {

        repair_cntp = PIN_FLIST_FLD_TAKE(task_flistp, PIN_FLD_COUNT, 1, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " repair_cntp is");
        sprintf(repair_cntp_msg,"repair_cntp is %d",*repair_cntp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, repair_cntp_msg);

        rep_warranty_endp = PIN_FLIST_FLD_TAKE(task_flistp, MSO_FLD_WARRANTY_END, 1, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " rep_warranty_endp is");
        sprintf(rep_warranty_endp_msg,"rep_warranty_endp is %d", rep_warranty_endp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, rep_warranty_endp_msg);


    }

	upd_flagsp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_OVERRIDE_FLAGS, 1, ebufp);
	if (upd_flagsp)
	{
		over_flag = *(int32 *)upd_flagsp;
	}
	device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 0, ebufp);
	if(device_id)
	{
		fm_mso_search_device_object(ctxp,task_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " fm_mso_device_set_attr input flist after device_search is " , task_flistp);
        device_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_OBJ, 1, ebufp);
        device_obj_type = (char *)PIN_POID_GET_TYPE(device_pdp);
        
		if ((PIN_POID_IS_NULL(device_pdp)) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Device not found ");
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_DEVICE_ID, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Invalid Device number", ebufp);
			goto CLEANUP;
		}

		stock_account_pdp = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_STPK_PLAN_OBJ, 0, ebufp);
		state_id = *(int32 *)PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_OLD_STATE, 0, ebufp);
	}
	if (state_id == 2 && over_flag != 1) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
			"Error in updating the device object for location update as its assigned " );
		
		job_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51039" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "Device is already assigned" , ebufp);
		*ret_flistpp = PIN_FLIST_COPY(job_iflistp,ebufp);
		goto CLEANUP;
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
			"Error in updating the device object for location update " );
		PIN_ERRBUF_CLEAR(ebufp);
		/********
		 * Update the status of the mso_mta_job to 2 to indiacte the failure record
		 ******/
		job_iflistp     = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, " Device Id not found " , ebufp);
		*ret_flistpp = PIN_FLIST_COPY(job_iflistp,ebufp);
		goto CLEANUP;
	}

	account_no = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_NO , 1, ebufp);

	if(account_no != (char *)NULL && strcmp(account_no,"") != 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "It's a Location Update");
		fm_mso_search_lco_account(ctxp, task_flistp , ebufp);
		lco_account_pdp = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_ACCOUNT_OBJ,1, ebufp);
		if((PIN_POID_IS_NULL(lco_account_pdp)))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO account object not found ");
			pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Invalid account number", ebufp);
            goto CLEANUP;
		}

		/****************************************************************************
		 * Check current account and new account belongs to same Service Provider
		 ***************************************************************************/


		if (over_flag != 2)
		{
		srch_prof_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(srch_prof_iflistp, PIN_FLD_POID, stock_account_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_prof_iflistp, PIN_FLD_PROFILE_DESCR, WHOLESALE, ebufp);
		PIN_FLIST_FLD_SET(srch_prof_iflistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);
		PIN_FLIST_FLD_SET(srch_prof_iflistp, PIN_FLD_OBJECT, stock_account_pdp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_device_set_attr get_profile input", srch_prof_iflistp);
		fm_mso_get_profile_info(ctxp, srch_prof_iflistp, &srch_prof_oflistp, ebufp );


		if (!srch_prof_oflistp)
		{            
            if(device_obj_type && (strstr(device_obj_type, "/device/vc") || strstr(device_obj_type, "/device/stb") ))
            {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Profile object not found ");
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51050" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Profile object not found " , ebufp);
			*ret_flistpp = PIN_FLIST_COPY(job_iflistp,ebufp);
			goto CLEANUP;
            }

            goto UPDATE;
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_device_set_attr get_profile output", srch_prof_oflistp);

		wholesale_flistp = PIN_FLIST_SUBSTR_GET(srch_prof_oflistp, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
		if (wholesale_flistp)
		{
			current_sp_pdp = PIN_FLIST_FLD_TAKE(wholesale_flistp, MSO_FLD_COMPANY_OBJ, 1, ebufp);
			customer_type = *(int32 *) PIN_FLIST_FLD_TAKE(wholesale_flistp,PIN_FLD_CUSTOMER_TYPE,1,ebufp);
		}
		PIN_FLIST_DESTROY_EX(&srch_prof_oflistp, NULL);

		PIN_FLIST_FLD_SET(srch_prof_iflistp, PIN_FLD_POID, lco_account_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_prof_iflistp, PIN_FLD_OBJECT, lco_account_pdp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , "fm_mso_device_set_attr get_profile input", srch_prof_iflistp);

		fm_mso_get_profile_info(ctxp, srch_prof_iflistp, &srch_prof_oflistp, ebufp );
		PIN_FLIST_DESTROY_EX(&srch_prof_iflistp, NULL);
		if (!srch_prof_oflistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Profile object not found ");
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51051" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Profile object not found " , ebufp);
			*ret_flistpp = PIN_FLIST_COPY(job_iflistp,ebufp);
			PIN_FLIST_DESTROY_EX(&srch_prof_oflistp, NULL);
			goto CLEANUP;
		}

		wholesale_flistp = PIN_FLIST_SUBSTR_GET(srch_prof_oflistp, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
		if (wholesale_flistp)
		{
			new_sp_pdp = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_COMPANY_OBJ, 1, ebufp);
		}
		
		if (current_sp_pdp && new_sp_pdp && 
			PIN_POID_COMPARE(current_sp_pdp, new_sp_pdp, 0, ebufp) != 0 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Stock points not belong to same service provider");
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51052" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Stock points not belong to same service provider" , ebufp);
			*ret_flistpp = PIN_FLIST_COPY(job_iflistp,ebufp);
			PIN_FLIST_DESTROY_EX(&srch_prof_oflistp, NULL);
			goto CLEANUP;
		}
		else
		{
			PIN_FLIST_DESTROY_EX(&srch_prof_oflistp, NULL);
		}
		}
		business_typep = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
				"Error in updating the LCO Account object for location update " );
			PIN_ERRBUF_CLEAR(ebufp);
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp     = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "LCO Account not found " , ebufp);
			*ret_flistpp = PIN_FLIST_COPY(job_iflistp,ebufp);
			goto CLEANUP;
		}

		if (*business_typep == 30000000 && state_id != MSO_STB_STATE_FAULTY)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
				"Error in moving the device to the reparing vendor as its state is not faulty " );
		
			job_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51030" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "Device is not faulty" , ebufp);
			*ret_flistpp = PIN_FLIST_COPY(job_iflistp,ebufp);
			goto CLEANUP;
		} 
		else if (*business_typep == 30000000 && state_id == MSO_STB_STATE_FAULTY)
		{
			/************************************************************
			 * This is faulty device and hence change state to reparing.
			 ************************************************************/
			state_id = MSO_STB_STATE_REPAIRING;
		 	write_iflistp = PIN_FLIST_CREATE(ebufp);
		 	PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
			PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_STATE_ID, &state_id, ebufp);
		 	PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_PROGRAM_NAME, write_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		 	PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_DEVICE_TYPE, write_iflistp, PIN_FLD_DEVICE_TYPE, ebufp);
		 	PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, write_iflistp, PIN_FLD_POID, ebufp);
		 	PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_USERID, write_iflistp, PIN_FLD_USERID, ebufp);
		 	PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_ORDER_OBJ, write_iflistp, MSO_FLD_ORDER_OBJ, ebufp);
		 
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device Set State input flist is  ", write_iflistp);

		 	PCM_OP(ctxp, MSO_OP_DEVICE_SET_STATE, 0, write_iflistp, &write_oflistp, ebufp);
	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device Set State output flist is  ", write_oflistp);
			PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
		
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
				"Error in updating the state_id to repairing for location update " );
				PIN_ERRBUF_CLEAR(ebufp);
				job_iflistp     = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51041" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "Set State 4 failed " , ebufp);
				*ret_flistpp = PIN_FLIST_COPY(job_iflistp,ebufp);
				goto CLEANUP;
			}
		}
		else if ( customer_type  == 30 && state_id == 9){
			ui_flistp = PIN_FLIST_CREATE(ebufp);	
			PIN_FLIST_FLD_SET(ui_flistp , PIN_FLD_POID , device_pdp , ebufp);
			PIN_FLIST_FLD_SET(ui_flistp,PIN_FLD_RECEIPT_NO,"",ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device update challan input flist is  " ,ui_flistp);
                 	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, ui_flistp , &uo_flistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device update challan input flist is  " ,uo_flistp);
			PIN_FLIST_DESTROY_EX(&ui_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&uo_flistp, NULL);
		}	
		else
		{
		}

UPDATE:
		/*****
		 * Prepare flist to set the device attribute
		 *****/
		 write_iflistp = PIN_FLIST_CREATE(ebufp);
		 PIN_FLIST_FLD_SET(write_iflistp , PIN_FLD_POID , device_pdp , ebufp);
		 PIN_FLIST_FLD_SET(write_iflistp , PIN_FLD_ACCOUNT_OBJ , lco_account_pdp , ebufp);
		 PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_ACCOUNT_NO, write_iflistp, PIN_FLD_SOURCE, ebufp);
		 PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME, write_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		 PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_RECEIPT_NO, write_iflistp, PIN_FLD_RECEIPT_NO, ebufp);
		 PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_ORDER_ID, write_iflistp, PIN_FLD_ORDER_ID, ebufp);
		 PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_REFERENCE_ID, write_iflistp, PIN_FLD_REFERENCE_ID, ebufp);
		 PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_TRANSACTION_ID, write_iflistp, MSO_FLD_TRANSACTION_ID, ebufp);	
		 PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_STATE, write_iflistp, PIN_FLD_STATE, ebufp);	 
		 //Inventory changes //
		 PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_CHANNEL_NAME, write_iflistp, MSO_FLD_CHANNEL_NAME, ebufp);
		 PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_VALID_TO, write_iflistp, PIN_FLD_VALID_TO, ebufp);
		 PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_DELIVERY_STATUS, write_iflistp, PIN_FLD_DELIVERY_STATUS, ebufp);
 		 PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_VENDOR_WARRANTY_END, write_iflistp, MSO_FLD_VENDOR_WARRANTY_END,  ebufp);
 		 PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_MANUFACTURER, write_iflistp, PIN_FLD_MANUFACTURER,  ebufp);
		 if(rep_flagp)
         {
             PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Repair flag found");
             PIN_FLIST_FLD_SET(write_iflistp, MSO_FLD_WARRANTY_END, rep_warranty_endp, ebufp);
         }
         if (customer_type == 13)
         {
             PIN_FLIST_FLD_SET(write_iflistp, MSO_FLD_ORDER_TYPE, "REVERSE" , ebufp);
         }
         else if (customer_type == 19)
         {
             PIN_FLIST_FLD_SET(write_iflistp, MSO_FLD_ORDER_TYPE, "REGULAR", ebufp);
         }


		 //
		 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device Set Attr input flist is  " ,write_iflistp);
		 PCM_OP(ctxp, PCM_OP_DEVICE_SET_ATTR, 0, write_iflistp , &write_oflistp , ebufp);
	
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
				"Error in updating the LCO Account object for location update " );
			PIN_ERRBUF_CLEAR(ebufp);
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp     = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "Set Attr failed " , ebufp);
			*ret_flistpp = PIN_FLIST_COPY(job_iflistp,ebufp);
			goto CLEANUP;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "It's a Attribute Update");

		write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_POID , device_pdp , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_PROGRAM_NAME, write_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_DEVICE_TYPE , write_iflistp , PIN_FLD_DEVICE_TYPE , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_SERIAL_NO , write_iflistp , MSO_FLD_SERIAL_NO , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_MANUFACTURER , write_iflistp , PIN_FLD_MANUFACTURER , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_MODEL , write_iflistp , PIN_FLD_MODEL , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_MAKE , write_iflistp , MSO_FLD_MAKE , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_CATEGORY , write_iflistp , PIN_FLD_CATEGORY , ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_PROVISIONABLE_FLAG , write_iflistp , MSO_FLD_PROVISIONABLE_FLAG , ebufp);
		
		// added fix
	
		 //Inventory changes //
		PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_CHANNEL_NAME, write_iflistp, MSO_FLD_CHANNEL_NAME, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_VALID_TO, write_iflistp, PIN_FLD_VALID_TO, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_DELIVERY_STATUS, write_iflistp, PIN_FLD_DELIVERY_STATUS, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_ORDER_ID, write_iflistp, PIN_FLD_ORDER_ID, ebufp);
        PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_ORDER_TYPE, write_iflistp, MSO_FLD_ORDER_TYPE, ebufp);
        PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_RECEIPT_NO, write_iflistp, PIN_FLD_RECEIPT_NO, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_VENDOR_WARRANTY_END, write_iflistp, MSO_FLD_VENDOR_WARRANTY_END, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_WARRANTY_END_OFFSET, write_iflistp, MSO_FLD_WARRANTY_END_OFFSET, ebufp);
        if(rep_flagp)
        {
            PIN_FLIST_FLD_SET(write_iflistp, MSO_FLD_WARRANTY_END, rep_warranty_endp, ebufp);
            PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_COUNT, repair_cntp, ebufp);
        }
        else{
		
            PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_WARRANTY_END, write_iflistp, MSO_FLD_WARRANTY_END, ebufp);
            PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_COUNT, write_iflistp, PIN_FLD_COUNT,  ebufp);
        }

		if(strstr(device_obj_type, "/ip/"))
		{
			/***added changes for ip attribute update****/
			//pin_flist_t *ipdata_flistp = PIN_FLIST_ELEM_ADD(write_iflistp, MSO_FLD_IP_DATA, 0, ebufp );
			//PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_IP_VERSION , ipdata_flistp , MSO_FLD_IP_VERSION , ebufp);
			//PIN_FLIST_FLD_COPY(task_flistp, MSO_FLD_IP_POOL_NAME , ipdata_flistp , MSO_FLD_IP_POOL_NAME , ebufp);
			PIN_FLIST_FLD_COPY(task_flistp , MSO_FLD_IP_DATA, write_iflistp, MSO_FLD_IP_DATA, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device Set Attr input flist is  " ,write_iflistp);
		PCM_OP(ctxp,PCM_OP_DEVICE_SET_ATTR,0 , write_iflistp , &write_oflistp , ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG ,
					"Error in updating the device attributes" );
			PIN_ERRBUF_CLEAR(ebufp);
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp     = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE , "51040" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, "Set Attr failed " , ebufp);
			*ret_flistpp = PIN_FLIST_COPY(job_iflistp,ebufp);
			goto CLEANUP;
		}
	}

	*ret_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS , &status_sucess, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device Set Attr output flist is", *ret_flistpp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&write_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&write_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&job_iflistp, NULL);
	if (current_sp_pdp) PIN_POID_DESTROY(current_sp_pdp, NULL);
	return;

}

