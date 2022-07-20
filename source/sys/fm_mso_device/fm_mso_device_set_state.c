/*
 * (#)$Id: fm_mso_device_set_state.c  $
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
* 0.1    |23/02/2015 |Harish Kulkarni     | Set the Device State
*
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/


#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_device_set_state.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
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

#define FILE_LOGNAME "fm_mso_device_set_state.c"

// Functions contained within this source file

EXPORT_OP void
op_mso_device_set_state(
	cm_nap_connection_t    *connp,
	int32            opcode,
	int32            flags,
	pin_flist_t        *i_flistp,
	pin_flist_t        **o_flistpp,
	pin_errbuf_t        *ebufp);

static void fm_mso_device_set_state(
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

/*******************************************************************************
 * Main function which does the device Set State:
 ******************************************************************************/

void
op_mso_device_set_state(
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
	if (opcode != MSO_OP_DEVICE_SET_STATE)
	{

		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_OPCODE, 0, 0, opcode);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"bad opcode in op_mso_device_set_state", ebufp);

		*o_flistpp = NULL;
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_device_set_state input flist", i_flistp);

	// Call the function to do the Set State:
	fm_mso_device_set_state(ctxp, i_flistp, o_flistpp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_mso_device_set_state error", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
		"op_mso_device_set_state error - input flist", i_flistp);

		*o_flistpp = NULL;
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"op_mso_device_set_state return flist",
				*o_flistpp);
	}

	return;

}

static void fm_mso_device_set_state(
	pcm_context_t   *ctxp,
	pin_flist_t     *task_flistp,
	pin_flist_t     **ret_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t     *job_iflistp = NULL;
	pin_flist_t     *write_iflistp = NULL;
	pin_flist_t     *write_oflistp = NULL;
	poid_t          *device_pdp = NULL;
	char            *device_id = NULL;
	char		*acc_nop = NULL;
	char		*warranty_strp = NULL;
	time_t		*warranty_endp = NULL;
	time_t		extend_warranty = 0;
	time_t		current_t = pin_virtual_time(NULL);
	time_t		current_date = 0;
	int32           status_sucess = 0;
	int32           status_fail = 1;
	int32		*state_old = NULL;
	int32		*state_new = NULL;
	int32		*repair_cntp = NULL;
	int32		repair_flag = 0;
	int32		warranty_days = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " fm_mso_device_set_state input flist is " , task_flistp);

	device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 0, ebufp);
	state_new = PIN_FLIST_FLD_GET(task_flistp , PIN_FLD_STATE_ID,0, ebufp);
	if(device_id)
	{
		fm_mso_search_device_object(ctxp,task_flistp, ebufp);
		device_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_OBJ,1, ebufp);
		state_old = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_OLD_STATE,1, ebufp);
	}
	if((PIN_POID_IS_NULL(device_pdp)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Device not found ");
		pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"Invalid Device number", ebufp);
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
		*ret_flistp = PIN_FLIST_COPY(job_iflistp,ebufp);
		goto CLEANUP;
	}

	if((state_old) && (*state_old == 3))
	{
		if( (*state_new) && (*state_new == 4))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
				"Correct new state " );
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
				"Incorrect new state " );
			PIN_ERRBUF_CLEAR(ebufp);
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp     = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51042" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "Exisitng state id to new state id change not allowed" , ebufp);
			*ret_flistp = PIN_FLIST_COPY(job_iflistp,ebufp);
			goto CLEANUP;			
		}
	}
	else if ((state_old) && (*state_old == 4  ))
	{
		                /*Repaired state Change condition also added*/
		if((*state_new) && ((*state_new == 9) || (*state_new == 5))) // VERIMATRIX - Repaired device state added
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
				"Correct new state " );		
			if (*state_new == 9)
			{
				repair_flag = 1;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG ," repair flag is set");
			}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
				"Incorrect new state " );
			PIN_ERRBUF_CLEAR(ebufp);
			/********
			 * Update the status of the mso_mta_job to 2 to indiacte the failure record
			 ******/
			job_iflistp     = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51042" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "Exisitng state id to new state id change not allowed" , ebufp);
			*ret_flistp = PIN_FLIST_COPY(job_iflistp,ebufp);
			goto CLEANUP;
		}
	}
	else
	{
		if( !((state_old) && (*state_old == 1) && (state_new) && (*state_new == 3) ))
		{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
			"Device having incorrect state id to change " );
		PIN_ERRBUF_CLEAR(ebufp);
		/********
		 * Update the status of the mso_mta_job to 2 to indiacte the failure record
		 ******/
		job_iflistp     = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51041" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "Exisitng state id not allowed for state change" , ebufp);
		*ret_flistp = PIN_FLIST_COPY(job_iflistp,ebufp);
		goto CLEANUP;		
		}
	}

	/*****
	 * Prepare flist to set the device State
	 *****/
	 write_iflistp = PIN_FLIST_CREATE(ebufp);
	 PIN_FLIST_FLD_SET(write_iflistp , PIN_FLD_POID , device_pdp , ebufp);
	 PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_STATE_ID, write_iflistp, PIN_FLD_NEW_STATE, ebufp);
	 PIN_FLIST_FLD_COPY(task_flistp , PIN_FLD_PROGRAM_NAME, write_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Device Set State input flist is  " ,write_iflistp);
	 PCM_OP(ctxp,PCM_OP_DEVICE_SET_STATE,0 , write_iflistp , &write_oflistp , ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , 
			"Error in Device Set State" );
		PIN_ERRBUF_CLEAR(ebufp);
		/********
		 * Update the status of the mso_mta_job to 2 to indiacte the failure record
		 ******/
		job_iflistp     = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
		PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "Set State failed " , ebufp);
		*ret_flistp = PIN_FLIST_COPY(job_iflistp,ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Device Set State output flist is  " ,write_oflistp);
	PIN_FLIST_FLD_SET(write_oflistp , PIN_FLD_STATUS , &status_sucess , ebufp);
	*ret_flistp = PIN_FLIST_COPY(write_oflistp,ebufp);

	if (repair_flag)
	{
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG ,"Entered into repair flag implementation ");
		/**************************************************************
		 * This device is being repaired, so warranty should be updated
		 **************************************************************/
		PIN_FLIST_DESTROY_EX(&write_iflistp , NULL);
		PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "task_flistp in repair counter update" , task_flistp);
		acc_nop = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SOURCE, 1, ebufp);
		repair_cntp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_COUNT, 1, ebufp);
		if (acc_nop != (char *)NULL && strcmp(acc_nop, "") != 0)
		{
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "repair count update1");
			fm_mso_search_lco_account(ctxp, task_flistp, ebufp);
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_lco_account repai counter" , task_flistp);
			warranty_strp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_AAC_PROMO_CODE, 1, ebufp);
			if (warranty_strp && strcmp(warranty_strp, "") != 0)
			{
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "repair count update2");
				current_date = fm_utils_time_round_to_midnight(current_t);
				warranty_endp = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_WARRANTY_END, 1, ebufp);
				if (warranty_endp && current_date >= *warranty_endp)
				{
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "repair count update3");
					warranty_days = atoi(warranty_strp);
					extend_warranty = warranty_days * 86400 + current_date;
				}
			}
		}

		if (repair_cntp)
		{
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "updating count");
			*repair_cntp = *repair_cntp + 1;	
		}
	 	write_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, write_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_USERID, write_iflistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_PROGRAM_NAME, write_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_DEVICE_TYPE, write_iflistp, PIN_FLD_DEVICE_TYPE, ebufp);
		PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_DEVICE_ID, write_iflistp, PIN_FLD_DEVICE_ID, ebufp);
		if (repair_cntp)
		{
			PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_COUNT, repair_cntp, ebufp);
            PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_FLAGS, &repair_flag, ebufp);
		}
		else
		{
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "updating repairing flag as no count found ");
			PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_COUNT, &repair_flag, ebufp);
           // PIN_FLIST_FLD_SET(write_iflistp, PIN_FLD_FLAGS, &repair_flag, ebufp);
		}

		if (extend_warranty != 0)
		{
			PIN_FLIST_FLD_SET(write_iflistp, MSO_FLD_WARRANTY_END, &extend_warranty, ebufp);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_DEVICE_SET_ATTR input flist is", write_iflistp);

		PCM_OP(ctxp, MSO_OP_DEVICE_SET_ATTR, 0, write_iflistp, &write_oflistp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_DEVICE_SET_ATTR output flist is", write_oflistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"Error in updating the device attr to from repaired state" );
			PIN_ERRBUF_CLEAR(ebufp);
			
			job_iflistp     = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, job_iflistp , PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51043" , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, "Set Attr failed " , ebufp);
			*ret_flistp = PIN_FLIST_COPY(job_iflistp,ebufp);
		}
	}


	CLEANUP:
	PIN_FLIST_DESTROY_EX(&write_iflistp , NULL);
	PIN_FLIST_DESTROY_EX(&write_oflistp , NULL);
	PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);

	return;

}


/*void
fm_mso_search_device_object(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{
		
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;
	
	char		*device_id = NULL;		
	char		search_template[100] = " select X from /device where F1 = V1 ";
	int		search_flags = 768;
	int32		*state_old = NULL;
	int64		db = 1;

	poid_t		*device_pdp = NULL;
	poid_t		*service_pdp = NULL;
	poid_t		*account_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object input flist", i_flistp);

	device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);

	*************
	 * search flist to search account poid
	 ************
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search device input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search device output flist", search_outflistp);

	if(PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) > 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object found");

		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		device_pdp = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_POID , 1, ebufp);
		state_old = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_STATE_ID , 1, ebufp);

		services_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_SERVICES , PIN_ELEMID_ANY, 1, ebufp);
		if(services_flistp)
		{
		  service_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		  account_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		}
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
		}

		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_OBJ , device_pdp , ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SERVICE_OBJ , service_pdp , ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ , account_pdp , ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_OLD_STATE , state_old , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search return flist", i_flistp);
	}
	else
	{
		// In case of ETHERNET, we will pass "AGREEMENT NO" in place of DEVICE ID so get the service details by AGREEMENT
		serv_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(serv_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
		
		fm_mso_get_bb_service_agreement_no(ctxp, device_id, &serv_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object search agreement flist", serv_flistp);
		service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if(PIN_POID_IS_NULL(service_pdp)){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object not found");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " 1");
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_OBJ , NULL , ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " 2");
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SERVICE_OBJ , NULL , ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " 3");
		}
	}

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
}
*/
