/*
 * (#)$Id: fm_mso_grv_upload.c  $
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
* 0.1    |16/02/2015 |Harish Kulkarni     		| GRV Upload - Initial Version
* 0.2    |17/02/2020 |Jyothirmayi Kachiraju   		| Manual GRV Process for CATV
*--------------------------------------------------------------------------------------------------------------*
****************************************************************************************************************/


#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_grv_upload.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <pcm.h>
#include <pinlog.h>
#include "ops/device.h"
#include "pin_device.h"
#include "cm_fm.h"
#include "pin_bill.h"
#include "pin_errs.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "mso_device.h"
#include "mso_prov.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_lifecycle_id.h"


#define FILE_LOGNAME "fm_mso_device_catv_preactivation.c"

/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/

EXPORT_OP void
op_mso_grv_upload(
	cm_nap_connection_t    *connp,
	int32            opcode,
	int32            flags,
	pin_flist_t        *i_flistp,
	pin_flist_t        **o_flistpp,
	pin_errbuf_t        *ebufp);

static void fm_mso_op_grv_upload(
	pcm_context_t   *ctxp,
	pin_flist_t     *task_flistp,
	pin_flist_t     **ret_flistp,
	pin_errbuf_t    *ebufp);

//static void
static int
fm_mso_search_device_object_with_plan(
	pcm_context_t   *ctxp,
	pin_flist_t     *i_flistp,
	pin_errbuf_t    *ebufp);

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

static void 
fm_validate_sp_code(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

extern void 
fm_mso_cust_get_pp_type(
	pcm_context_t	    *ctxp,
        poid_t              *acct_pdp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);
		
extern void
fm_mso_utils_sequence_no(
        pcm_context_t  *ctxp,
        pin_flist_t    *i_flistp,
        pin_flist_t    **r_flistpp,
        pin_errbuf_t   *ebufp);
		
/*******************************************************************************
 * Main function which does the GRV Upload: 
 ******************************************************************************/

void
op_mso_grv_upload(
	cm_nap_connection_t    *connp,
	int32            opcode,
	int32            flags,
	pin_flist_t        *i_flistp,
	pin_flist_t        **o_flistpp,
	pin_errbuf_t        *ebufp)
{
	pcm_context_t        *ctxp = connp->dm_ctx;
        int                     local = LOCAL_TRANS_OPEN_SUCCESS;
        int                     *status = NULL;
	int32                     status_success = 0;
        int32                   status_uncommitted =2;
        poid_t                  *inp_pdp = NULL;
        char            *prog_name = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != MSO_OP_GRV_UPLOAD)
	{

		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_mso_grv_upload", ebufp);

		*o_flistpp = NULL;
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_grv_upload input flist", i_flistp);

        inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *o_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*o_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                PIN_FLIST_FLD_SET(*o_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                PIN_FLIST_FLD_SET(*o_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
                PIN_FLIST_FLD_SET(*o_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
                return;
        }
        if ( local == LOCAL_TRANS_OPEN_FAIL )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *o_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*o_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
                PIN_FLIST_FLD_SET(*o_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
                PIN_FLIST_FLD_SET(*o_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
                PIN_FLIST_FLD_SET(*o_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
                return;
        }

	// Call the function to do the GRV Upload:
	fm_mso_op_grv_upload(ctxp, i_flistp, o_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_grv_upload output flist", *o_flistpp);
    	status = PIN_FLIST_FLD_GET(*o_flistpp, PIN_FLD_STATUS, 1, ebufp);

	/***********************************************************
         * Results.
         ***********************************************************/
        if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)status == 1))
        {
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERRBUF_CLEAR(ebufp);
                }
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_grv_upload error", ebufp);
                if(local == LOCAL_TRANS_OPEN_SUCCESS )
                {
                        fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
                }
        }
        else
        {
                if(local == LOCAL_TRANS_OPEN_SUCCESS  &&
                  !(prog_name && strstr(prog_name,"pin_deferred_act"))
                  )
                {
                        fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
                }
                else
                {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
                                PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
                }
		

        }

	return;

}

static void fm_mso_op_grv_upload(
	pcm_context_t		*ctxp,
	pin_flist_t		*task_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*refund_oflistp = NULL;
	pin_flist_t		*refund_iflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*job_oflistp = NULL;
	pin_flist_t		*job_iflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*grv_upload_create_inflistp = NULL;
	pin_flist_t		*grv_upload_create_outflistp = NULL;
	pin_flist_t		*set_attribute_inflistp = NULL;
	pin_flist_t		*set_attribute_outflistp = NULL;
	pin_flist_t		*sp_oflistp = NULL;
	pin_flist_t		*lc_in_flist = NULL;
	pin_flist_t		*deposit_flistp = NULL;
	pin_flist_t		*profile_flistp = NULL;
	pin_flist_t		*seq_flistp = NULL;
	pin_flist_t		*seq_o_flistp = NULL;

	poid_t			*dummy_pdp = NULL;
	poid_t			*device_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*account_pdp = NULL;
	
	char			*grv_no = NULL;
	char			*device_id = NULL;
	char			*service_obj_type = NULL;
	char			search_template[256] = " select X from /mso_deposit where F1 = V1 and F2 = V2 and F3 = V3 ";
	
	int64			db = 1;
	int32			status = 0;
	int32			status_sucess = 0;
	int32			status_fail = 1;
	int32			deposit_status = 0;
	int32			*pp_type = NULL;
	
	int			search_flags = 512;
	int			result_status = 0;
	int			function_return = 0;
	int     		elem_id = 0;
	
	void			*vp = NULL;

	pin_cookie_t    	cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG , " fm_mso_op_grv_upload input flist is " , task_flistp);

	dummy_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_POID , 0, ebufp);
	device_id = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);
	if(device_id)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Searching Device" );
		//fm_mso_search_device_object_with_plan(ctxp,task_flistp, ebufp);
		function_return = fm_mso_search_device_object_with_plan(ctxp,task_flistp, ebufp);
		device_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_DEVICE_OBJ,1, ebufp);
		account_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_ACCOUNT_OBJ,1, ebufp);
		service_pdp = PIN_FLIST_FLD_GET(task_flistp, PIN_FLD_SERVICE_OBJ,1, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the device object for grv upload " );
			PIN_ERRBUF_CLEAR(ebufp);

			job_iflistp     = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
			status_fail = 1;
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
	
			if(function_return == 1)
			{
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, 
							"Error in getting device details", ebufp);
			} 
			else if(function_return == 2)
			{
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, 
							"Error in getting account details", ebufp);
			} 
			else if(function_return == 3)
			{
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, 
							"Error in getting service details", ebufp);
			} 
			else if(function_return == 4)
			{
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, 
							"Error - Invalid service/provisioning status", ebufp);
			} 
			else 
			{
				PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR,
						"Error in device or service or acct details", ebufp);
			}
			*ret_flistp = job_iflistp;
			//PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "fm_mso_op_grv_upload out flist is",*ret_flistp);
			return;

		}

		if(PIN_POID_IS_NULL(device_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object not found ");

			pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Invalid Device Id", ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the device object for grv upload " );
				PIN_ERRBUF_CLEAR(ebufp);

				job_iflistp     = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
				status_fail = 1;
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51040" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, 
					" Error in updating the device object for grv upload " , ebufp);
				*ret_flistp = job_iflistp;
				//PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				return;

			}
		}
		else if(PIN_POID_IS_NULL(account_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Account object not found ");

			pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"No Account linked to this device", ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the device object for grv upload " );
				PIN_ERRBUF_CLEAR(ebufp);

				job_iflistp     = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
				status_fail = 1;
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51041" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, 
					" Error in updating the device object for grv upload " , ebufp);
				*ret_flistp = job_iflistp;
				//PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				return;
			}
		}
		else if(PIN_POID_IS_NULL(service_pdp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Service object not found ");

			pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, PIN_FLD_FLAGS, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"No Service linked to this device", ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Error in updating the device object for grv upload " );
				PIN_ERRBUF_CLEAR(ebufp);

				job_iflistp     = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
				status_fail = 1;
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51042" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, 
					" Error in updating the device object for grv upload " , ebufp);
				*ret_flistp = job_iflistp;
				//PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				return;
			}
		}
		else
		{
			/********************************
			 * Read service object
			 ********************************/
			pin_flist_t    *read_svc_inflistp = NULL;
			pin_flist_t    *read_svc_outflistp = NULL;

			 read_svc_inflistp = PIN_FLIST_CREATE(ebufp);
			 PIN_FLIST_FLD_SET(read_svc_inflistp, PIN_FLD_POID, service_pdp, ebufp);

			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				 "fm_mso_op_grv_upload read service object. input list:", read_svc_inflistp);
			 PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_svc_inflistp, &read_svc_outflistp, ebufp);

			 PIN_FLIST_DESTROY_EX(&read_svc_inflistp, NULL);
			 if (PIN_ERRBUF_IS_ERR(ebufp))
			 {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
				PIN_ERRBUF_CLEAR(ebufp);

				job_iflistp     = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
				status_fail = 1;
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51042" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, " Error in calling PCM_OP_SEARCH " , ebufp);
				*ret_flistp = job_iflistp;
				//PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&read_svc_outflistp, NULL);
				return;
			 }
			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				 "fm_mso_op_grv_upload read service object. output list:", read_svc_outflistp);

			account_pdp = PIN_FLIST_FLD_TAKE(read_svc_outflistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
			PIN_FLIST_DESTROY_EX(&read_svc_outflistp, NULL);
			
			/************************************************************************************
			 * CATV GRV Process AD == Device should be released for BB & CATV PP Customers Only.
			 * Throw error message, if the Account is a CATV Secondary Point Customer
			 ************************************************************************************/
			if (service_pdp && strcmp((char*)PIN_POID_GET_TYPE(service_pdp), MSO_CATV) == 0)
			{
				fm_mso_cust_get_pp_type(ctxp, account_pdp, &profile_flistp, ebufp);
				
				if(profile_flistp)
				{
					pp_type = PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_PP_TYPE, 1, ebufp);

					if( *pp_type == PP_TYPE_SECONDARY )
					{
						// For CATV Secondary Point Customers, Auto GRV is applicable as per existing process.
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV Account is Secondary Point Account, Manual GRV is not applicable...");
							status_fail = 1;
							
							job_iflistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_POID, dummy_pdp, ebufp);							
							PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_STATUS, &status_fail, ebufp);
							PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_CODE, "51042", ebufp);
							PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, 
								"CATV Account is Secondary Point Account, Manual GRV is not applicable...", ebufp);
							*ret_flistp = job_iflistp;
							PIN_FLIST_DESTROY_EX(&profile_flistp, NULL);
							return;
						}
					}
				}
			}
			
			/********************************
			 * Disassociate device
			 ********************************/
			 pin_flist_t   *device_das_inflistp = NULL;
			 pin_flist_t   *device_das_outflistp = NULL;

			 device_das_inflistp = PIN_FLIST_CREATE(ebufp);
			 PIN_FLIST_FLD_SET(device_das_inflistp, PIN_FLD_POID, device_pdp, ebufp);

			 int    i_flags = 0;
			 PIN_FLIST_FLD_SET(device_das_inflistp, PIN_FLD_FLAGS, &i_flags, ebufp);
			 PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_PROGRAM_NAME, device_das_inflistp, PIN_FLD_PROGRAM_NAME , ebufp);

			 pin_flist_t    *args_flistp = NULL;
			 args_flistp = PIN_FLIST_ELEM_ADD(device_das_inflistp, PIN_FLD_SERVICES, 0, ebufp);

			 PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
			 PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);

			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				 "fm_mso_op_grv_upload device disassociation. input list:", device_das_inflistp);
			 PCM_OP(ctxp, PCM_OP_DEVICE_ASSOCIATE , 0, device_das_inflistp, &device_das_outflistp, ebufp);
			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_op_grv_upload device disassociation. out list:", device_das_outflistp);
			 PIN_FLIST_DESTROY_EX(&device_das_inflistp, NULL);
			 if (PIN_ERRBUF_IS_ERR(ebufp))
			 {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_DEVICE_ASSOCIATE", ebufp);
				PIN_ERRBUF_RESET(ebufp);

				job_iflistp     = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
				status_fail = 1;
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51042" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, 
					"Error in calling PCM_OP_DEVICE_ASSOCIATE " , ebufp);
				*ret_flistp = job_iflistp;
				//PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&device_das_outflistp, NULL);
				return;
			 }
			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				 "fm_mso_integ_create_grv_uploader_job device disassociation. output list:", device_das_outflistp);
			 PIN_FLIST_DESTROY_EX(&device_das_outflistp, NULL);


			/*************
			 * search deposit
			 ************/
			search_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
			PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_OBJ, device_pdp, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &deposit_status, ebufp);
			results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object_with_plan search device input list", search_inflistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

			if ((PIN_ERRBUF_IS_ERR(ebufp)))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling Searching deposit objects", ebufp);	
				PIN_ERRBUF_RESET(ebufp);

				job_iflistp     = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
				status_fail = 1;
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51043" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, 
					" Error in calling Searching deposit objects " , ebufp);
				*ret_flistp = job_iflistp;
				//PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				return;
			}

	//		 read_deposit_inflistp = PIN_FLIST_CREATE(ebufp);
	//		 PIN_FLIST_FLD_SET(read_deposit_inflistp, PIN_FLD_POID, account_pdp, ebufp);
	//		 PIN_FLIST_FLD_SET(read_deposit_inflistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
	//		 PIN_FLIST_FLD_SET(read_deposit_inflistp, PIN_FLD_STATUS, &status, ebufp);
	//
	//		 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_integ_create_grv_uploader_job get deposit objects. input list:", read_deposit_inflistp);
	//		 PCM_OP(ctxp, MSO_OP_PYMT_GET_DEPOSITS, 0, read_deposit_inflistp, &read_deposit_outflistp, ebufp);

	//		 if (PIN_ERRBUF_IS_ERR(ebufp))
	//		 {
	//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PYMT_DEPOSIT_REFUND to get deposit objects", ebufp);
	//			PIN_ERRBUF_RESET(ebufp);
	//
	//			job_iflistp     = PIN_FLIST_CREATE(ebufp);
	//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
	//			status_fail = 1;
	//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
	//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51042" , ebufp);
	//			PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, 
	//				" Error in calling MSO_OP_PYMT_DEPOSIT_REFUND to get deposit objects " , ebufp);
	//			*ret_flistp = PIN_FLIST_COPY(job_iflistp, ebufp );
	//			PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
	//			PIN_FLIST_DESTROY_EX(&read_deposit_inflistp, NULL);
	//			PIN_FLIST_DESTROY_EX(&read_deposit_outflistp, NULL);
	//			return;
	//		 }
			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				 "fm_mso_integ_create_grv_uploader_job get deposit objects. output list:", 
				 search_outflistp);

			pin_flist_t    *result_info = NULL;
			while ((result_info = PIN_FLIST_ELEM_GET_NEXT(search_outflistp,PIN_FLD_RESULTS, 
				 &elem_id, 1, &cookie, ebufp))!= NULL)
			{
				refund_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(refund_iflistp, PIN_FLD_POID, account_pdp, ebufp);
				PIN_FLIST_FLD_SET(refund_iflistp, PIN_FLD_USERID, dummy_pdp, ebufp);
				PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_PROGRAM_NAME, refund_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);

				deposit_flistp = PIN_FLIST_ELEM_ADD(refund_iflistp, PIN_FLD_DEPOSITS, 0, ebufp);
				PIN_FLIST_FLD_SET(deposit_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
				PIN_FLIST_FLD_COPY(result_info, MSO_FLD_DEPOSIT_NO, deposit_flistp , MSO_FLD_DEPOSIT_NO , ebufp);

				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
					"fm_mso_integ_create_grv_uploader_job refund. input list:", refund_iflistp);
				PCM_OP(ctxp,MSO_OP_PYMT_DEPOSIT_REFUND,0 , refund_iflistp , &refund_oflistp , ebufp);

				//PIN_FLIST_DESTROY_EX(&refund_iflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PYMT_DEPOSIT_REFUND", ebufp);
					PIN_FLIST_DESTROY_EX(&refund_iflistp, NULL);
					PIN_FLIST_DESTROY_EX(&refund_oflistp, NULL);
					PIN_ERRBUF_RESET(ebufp);
					//
					// Update the status of the mso_mta_job to 2 to indiacte the failure record
					//
					job_iflistp     = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE, "51042" , ebufp);
					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, " Location update failed " , ebufp);
					
					*ret_flistp = job_iflistp;
					//PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
					return;
				}
//				else
//				{
//					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Device grv uploadd sucessfully ");
//					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
//						"fm_mso_integ_create_grv_uploader_job refund. output list:" ,refund_oflistp);
//					job_iflistp     = PIN_FLIST_CREATE(ebufp);
//					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
//					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_sucess, ebufp);
//					PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE, "51042" , ebufp);
//					PIN_FLIST_FLD_SET(job_iflistp, PIN_FLD_ERROR_DESCR, 
//						" Device grv uploadd sucessfully!! " , ebufp);					
//					*ret_flistp = job_iflistp;
//					//PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
//					PIN_FLIST_DESTROY_EX(&refund_oflistp , NULL);
//					PIN_FLIST_DESTROY_EX(&refund_iflistp, NULL);
//					return;
//				}
			}
			PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);


			/*************
			 * Create GRV Upload object 
			 **************************************/

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Create grv_object start");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"task_flist",task_flistp);
			grv_upload_create_inflistp  = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(grv_upload_create_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/mso_grv_upload", -1, ebufp), ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_ACCOUNT_OBJ,grv_upload_create_inflistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
			
			grv_no = PIN_FLIST_FLD_GET(task_flistp, MSO_FLD_REF_NO, 0, ebufp);
			if (grv_no && strlen(grv_no) != 0)
			{			
				PIN_FLIST_FLD_COPY(task_flistp,MSO_FLD_REF_NO,grv_upload_create_inflistp,MSO_FLD_REF_NO,ebufp);
				PIN_FLIST_FLD_SET(grv_upload_create_inflistp, PIN_FLD_ACTION, "BULK_GRV",ebufp);				
			}
			else
			{
				/***************************************************************
				 * Manual GRV Upload for Single Account, generate the GRV_No
				 ***************************************************************/
				PIN_FLIST_FLD_SET(grv_upload_create_inflistp, PIN_FLD_ACTION, "SINGLE_ACCOUNT_GRV",ebufp);
				seq_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(task_flistp, PIN_FLD_POID, seq_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(seq_flistp, PIN_FLD_NAME, "MSO_GRV_SEQUENCE",ebufp);
				
				//call the function to read the grv order sequence
				fm_mso_utils_sequence_no(ctxp, seq_flistp, &seq_o_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&seq_flistp, NULL);
				
				PIN_FLIST_FLD_MOVE(seq_o_flistp, PIN_FLD_STRING, task_flistp, MSO_FLD_REF_NO, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, dummy_pdp, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &status_fail, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51044", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, 
								"fm_mso_grv_upload - Generating GRV Reference Number for Device GRV Upload failed", ebufp);
					*ret_flistp = r_flistp;
				}
				PIN_FLIST_FLD_COPY(task_flistp,MSO_FLD_REF_NO,grv_upload_create_inflistp,MSO_FLD_REF_NO,ebufp);
				PIN_FLIST_DESTROY_EX(&seq_o_flistp, NULL);
			}
				
			PIN_FLIST_FLD_COPY(task_flistp,MSO_FLD_DEPOSIT_DATE,grv_upload_create_inflistp,MSO_FLD_DEPOSIT_DATE,ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_DEVICE_ID,grv_upload_create_inflistp,PIN_FLD_DEVICE_ID,ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,MSO_FLD_SERIAL_NO,grv_upload_create_inflistp,MSO_FLD_SERIAL_NO,ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_DESCR,grv_upload_create_inflistp,PIN_FLD_DESCR,ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_REASON,grv_upload_create_inflistp,PIN_FLD_REASON,ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_PROGRAM_NAME,grv_upload_create_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_USER_NAME,grv_upload_create_inflistp,PIN_FLD_USER_NAME,ebufp);
			
			fm_validate_sp_code(ctxp,task_flistp,&sp_oflistp,ebufp);

			vp = PIN_FLIST_FLD_GET(sp_oflistp, PIN_FLD_STATUS, 0, ebufp);
			
			if(vp && *(int32*)vp != 0)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "Error in creating GRV upload object", ebufp);
				PIN_FLIST_DESTROY_EX(&refund_iflistp, NULL);
				*ret_flistp = sp_oflistp;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_op_grv_upload sp validation result flist", *ret_flistp);
				PIN_FLIST_DESTROY_EX(&grv_upload_create_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&grv_upload_create_outflistp, NULL);
				return;			

			}

			PIN_FLIST_FLD_COPY(sp_oflistp,PIN_FLD_POID,grv_upload_create_inflistp,MSO_FLD_LCO_OBJ,ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_op_grv_upload grv_upload_create_inflistp ", grv_upload_create_inflistp);
			PCM_OP(ctxp,PCM_OP_CREATE_OBJ,0 , grv_upload_create_inflistp, &grv_upload_create_outflistp , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_op_grv_upload grv_upload_create_outflistp ", grv_upload_create_outflistp);

			PIN_FLIST_DESTROY_EX(&sp_oflistp, NULL);
			if ((PIN_ERRBUF_IS_ERR(ebufp)))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating GRV upload object", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				job_iflistp     = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
				status_fail = 1;
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51044" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, 
					" Error in creating GRV upload object  " , ebufp);
				*ret_flistp = job_iflistp;
				//PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&grv_upload_create_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&grv_upload_create_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&refund_iflistp, NULL);
				return;
			}
			//PIN_FLIST_DESTROY_EX(&grv_upload_create_inflistp, NULL);
			//PIN_FLIST_DESTROY_EX(&grv_upload_create_outflistp, NULL);

			// update device with Stockpoint ID
			set_attribute_inflistp  = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(set_attribute_inflistp,PIN_FLD_POID,device_pdp,ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,MSO_FLD_STOCK_POINT_CODE,set_attribute_inflistp,PIN_FLD_SOURCE,ebufp);
			PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_PROGRAM_NAME,set_attribute_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
			PCM_OP(ctxp,PCM_OP_DEVICE_SET_ATTR,0 , set_attribute_inflistp , &set_attribute_outflistp , ebufp);
			if ((PIN_ERRBUF_IS_ERR(ebufp)))
			{
				PIN_ERRBUF_RESET(ebufp);
				job_iflistp     = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_POID ,dummy_pdp , ebufp);
				status_fail = 1;
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_STATUS , &status_fail , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_CODE , "51045" , ebufp);
				PIN_FLIST_FLD_SET(job_iflistp , PIN_FLD_ERROR_DESCR, 
					" Error in creating updating stockpoint id in device  " , ebufp);
				*ret_flistp = PIN_FLIST_COPY(job_iflistp, ebufp );
				PIN_FLIST_DESTROY_EX(&job_iflistp , NULL);
				PIN_FLIST_DESTROY_EX(&set_attribute_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&set_attribute_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&refund_iflistp, NULL);
				PIN_FLIST_DESTROY_EX(&grv_upload_create_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&grv_upload_create_inflistp, NULL);
				return;
			}

			PIN_FLIST_DESTROY_EX(&set_attribute_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&set_attribute_outflistp, NULL);

		}
	}

	lc_in_flist = PIN_FLIST_COPY(task_flistp, ebufp);
	if (refund_oflistp)
	{
		PIN_FLIST_FLD_COPY(refund_iflistp, PIN_FLD_AMOUNT, lc_in_flist, PIN_FLD_AMOUNT, ebufp );
		PIN_FLIST_FLD_COPY(refund_iflistp, MSO_FLD_DEPOSIT_NO, lc_in_flist, MSO_FLD_DEPOSIT_NO, ebufp );
	}	
	PIN_FLIST_FLD_COPY(grv_upload_create_inflistp,PIN_FLD_ACTION,lc_in_flist,PIN_FLD_ACTION,ebufp);
	PIN_FLIST_FLD_COPY(grv_upload_create_outflistp, PIN_FLD_POID, lc_in_flist, MSO_FLD_GRV_OBJ, ebufp );
	fm_mso_create_lifecycle_evt(ctxp, lc_in_flist, grv_upload_create_inflistp, ID_GRV_UPLOAD, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}
	

	PIN_FLIST_DESTROY_EX(&refund_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&lc_in_flist, NULL);
	PIN_FLIST_DESTROY_EX(&grv_upload_create_outflistp, NULL); 
	PIN_FLIST_DESTROY_EX(&grv_upload_create_inflistp, NULL);

	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(task_flistp,PIN_FLD_POID,*ret_flistp, PIN_FLD_POID,ebufp );
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_sucess, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_DESCR, "GRV uploaded successfully", ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_grv_upload output flist", *ret_flistp);
	return;
}


//void
int
fm_mso_search_device_object_with_plan(
	pcm_context_t   *ctxp,
	pin_flist_t     *i_flistp,
	pin_errbuf_t    *ebufp)
{

	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *search_inflistp = NULL;
	pin_flist_t     *search_outflistp = NULL;
	pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *services_flistp = NULL;
	pin_flist_t     *purchased_plans_flistp = NULL;

	char            *device_id = NULL;
	char            search_template[100] = " select X from /device where F1 = V1 ";
	int             search_flags = 768;
	int64           db = 1;
	int64           account_obj_id0 = 0;
	char            *agreement_no = NULL;

	poid_t          *device_pdp = NULL;
	poid_t          *account_pdp = NULL;
	poid_t          *account_pdp1 = NULL;	
	poid_t          *service_pdp = NULL;
	poid_t          *purcased_plan_pdp = NULL;
	int32           *status = NULL;

	void		*vp = NULL;
	int		provisionable_flag = 0;
	int		prov_status =0;
	int		*prov_statusp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
			PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object_with_plan input flist", i_flistp);

	device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AGREEMENT_NO , 1, ebufp);

	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_OBJ , NULL , ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SERVICE_OBJ , NULL , ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ , NULL , ebufp);
	//PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_PURCHASED_PLAN_OBJ , NULL , ebufp);

	/*************
	 * search flist to search account poid
	 ************/
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object_with_plan search device input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object_with_plan search device output flist", search_outflistp);

	if(PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) > 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object found");

		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		
		if(results_flistp != NULL)
		{
			device_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
			services_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);

			if(services_flistp != NULL)
			{
				account_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
				service_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
			}
			
			PIN_FLIST_FLD_COPY(results_flistp,MSO_FLD_SERIAL_NO,i_flistp, MSO_FLD_SERIAL_NO,ebufp);
		}

		/******* Pavan Bellala 11-09-2015 *******************************
		Fetch the provisionable flag. The device should be dissociated
		from active service also, if the provisionable flag is 0
		*****************************************************************/
		vp = PIN_FLIST_FLD_GET(results_flistp,MSO_FLD_PROVISIONABLE_FLAG,1,ebufp);
		if(vp) provisionable_flag = *(int32 *)vp;
		
		if(!PIN_POID_IS_NULL(device_pdp))
		{
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_OBJ, device_pdp, ebufp);
		}
		
		if(!PIN_POID_IS_NULL(account_pdp))
		{
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
		}
		
		if(!PIN_POID_IS_NULL(service_pdp))
		{
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
		}

		//
		// get account_obj_id by account_no for validation
		//
		pin_flist_t     *find_account_obj_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(find_account_obj_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		pin_flist_t     *find_account_obj_args_flistp = PIN_FLIST_ELEM_ADD(find_account_obj_inflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, find_account_obj_args_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
		pin_flist_t     *find_account_obj_results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(find_account_obj_results_flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object_with_plan find account input list", find_account_obj_inflistp);

		pin_flist_t     *find_account_obj_outflistp = NULL;
		PCM_OP(ctxp, PCM_OP_CUST_FIND, 0, find_account_obj_inflistp, &find_account_obj_outflistp, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_FIND", ebufp);
			PIN_FLIST_DESTROY_EX(&find_account_obj_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&find_account_obj_outflistp, NULL);

			PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

			return 2;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_search_device_object_with_plan find account output list", find_account_obj_outflistp);

		pin_flist_t     *find_account_obj_result_flistp = PIN_FLIST_ELEM_GET(find_account_obj_outflistp, 
			PIN_FLD_RESULTS , PIN_ELEMID_ANY, 1, ebufp);
		if (find_account_obj_result_flistp != NULL)
		{
				account_pdp1 = PIN_FLIST_FLD_GET(find_account_obj_result_flistp, PIN_FLD_POID, 1, ebufp);
				if(PIN_POID_COMPARE(account_pdp1,account_pdp,0,ebufp)!=0)
				{
					
					pin_set_err(ebufp, PIN_ERRLOC_FM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, PIN_FLD_ACCOUNT_NO, 0, 0);
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"Invalid Account Number", ebufp);
				
					PIN_FLIST_DESTROY_EX(&find_account_obj_inflistp, NULL);
					PIN_FLIST_DESTROY_EX(&find_account_obj_outflistp, NULL);
					return 2;
				}
				PIN_FLIST_DESTROY_EX(&find_account_obj_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&find_account_obj_outflistp, NULL);

		}
		else
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_ACCOUNT_NO, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Invalid Account Number", ebufp);

			PIN_FLIST_DESTROY_EX(&find_account_obj_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&find_account_obj_outflistp, NULL);

			PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

			return 2;
		}
		
		//
		// get agreement no for the service and compare for validation
		//
		pin_flist_t     *find_agreement_no_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(find_agreement_no_inflistp, PIN_FLD_POID , service_pdp , ebufp);
		PIN_FLIST_FLD_SET(find_agreement_no_inflistp, PIN_FLD_STATUS , NULL , ebufp);

		/******* Pavan Bellala 04-11-2015 *********
		Added validation for provisioning status
		*******************************************/
		pin_flist_t	*telco_flistp = NULL;
		
		char *service_obj_type = (char *)PIN_POID_GET_TYPE(service_pdp);
		if(strstr(service_obj_type, "catv"))
		{
			pin_flist_t     *substr_flistp = PIN_FLIST_SUBSTR_ADD(find_agreement_no_inflistp, MSO_FLD_CATV_INFO, ebufp);
			PIN_FLIST_FLD_SET(substr_flistp, MSO_FLD_AGREEMENT_NO , NULL , ebufp);
			
		}
		else if(strstr(service_obj_type, "broadband"))
		{
			telco_flistp = PIN_FLIST_ELEM_ADD(find_agreement_no_inflistp,PIN_FLD_TELCO_FEATURES, 0, ebufp);
			PIN_FLIST_FLD_SET(telco_flistp,PIN_FLD_STATUS,NULL,ebufp);
			pin_flist_t  *substr_flistp = PIN_FLIST_SUBSTR_ADD(find_agreement_no_inflistp, MSO_FLD_BB_INFO, ebufp);
			PIN_FLIST_FLD_SET(substr_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_search_device_object_with_plan find agreement_no_tmp input list", find_agreement_no_inflistp);

		pin_flist_t     *find_agreement_no_outflistp = NULL;
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, find_agreement_no_inflistp, &find_agreement_no_outflistp, ebufp);


		PIN_FLIST_DESTROY_EX(&find_agreement_no_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_FIND", ebufp);
			//PIN_FLIST_DESTROY_EX(&find_agreement_no_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&find_agreement_no_outflistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			return 3;
		}
		// status has to be terminated for GRV upload
		status = PIN_FLIST_FLD_GET(find_agreement_no_outflistp, PIN_FLD_STATUS, 1, ebufp);

		/*********** Pavan Bellala  04-11-2015 *****************************
		Prov status should be terminated for grv 
		*******************************************************************/
		if(strstr(service_obj_type, "broadband"))
		{
			telco_flistp = PIN_FLIST_ELEM_GET(find_agreement_no_outflistp,PIN_FLD_TELCO_FEATURES, PIN_ELEMID_ANY,1, ebufp);
			if(telco_flistp)
			{
				prov_statusp = PIN_FLIST_FLD_GET(telco_flistp,PIN_FLD_STATUS,1,ebufp);
				prov_status = *(int *)prov_statusp;
			}
		}
		
		/*********** Pavan Bellala 11-09-2015 ************************************
		Service status check is not required if the provisionable_flag is 0
		**************************************************************************/
		if(provisionable_flag != 0)
		{
			if(status && *status != 10103)
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);

				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_device_object_with_plan service is not terminated yet!!!", ebufp);
			
				//PIN_FLIST_DESTROY_EX(&find_agreement_no_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&find_agreement_no_outflistp, NULL);

				PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				return 4;
			} 
			else 
			{
				if(strstr(service_obj_type, "broadband"))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "service is terminated. now check for prov status");
					if(prov_status != MSO_PROV_STATE_TERMINATED)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "prov status is not terminated. GRV not allowed");
						pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE,
							PIN_ERR_BAD_VALUE,PIN_FLD_STATUS, 0, 0);
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"fm_mso_search_device_object_with_plan service prov status is not terminated. GRV not allowed",
										ebufp);
						PIN_FLIST_DESTROY_EX(&find_agreement_no_outflistp, NULL);
						PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
						PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
						return 4;
					}
				}	
			}
		}
		
		if(strstr(service_obj_type, "catv"))
		{
			if(status && *status != 10103)
			{
				pin_set_err(ebufp, PIN_ERRLOC_FM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE,
						PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);

				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_device_object_with_plan service is not terminated yet!!!", ebufp);
			
				//PIN_FLIST_DESTROY_EX(&find_agreement_no_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&find_agreement_no_outflistp, NULL);

				PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
				return 4;
			} 
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_search_device_object_with_plan find agreement_no_tmp output list", find_agreement_no_outflistp);
		
		// status has to be terminated for GRV upload
		char    *agreement_no_tmp = NULL;
		if(strstr(service_obj_type, "catv"))
		{
			pin_flist_t     *substr_flistp = PIN_FLIST_SUBSTR_GET(find_agreement_no_outflistp, MSO_FLD_CATV_INFO, 0, ebufp);
			agreement_no_tmp = PIN_FLIST_FLD_GET(substr_flistp, MSO_FLD_AGREEMENT_NO , 1 , ebufp);
		}
		else if(strstr(service_obj_type, "broadband"))
		{
			pin_flist_t     *substr_flistp = PIN_FLIST_SUBSTR_GET(find_agreement_no_outflistp, MSO_FLD_BB_INFO, 0, ebufp);
			agreement_no_tmp = PIN_FLIST_FLD_GET(substr_flistp, MSO_FLD_AGREEMENT_NO , 1 , ebufp);
		}

		if ((agreement_no_tmp && (agreement_no && strcmp(agreement_no ,agreement_no_tmp) != 0)) || !agreement_no)
		{

			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, MSO_FLD_AGREEMENT_NO, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Invalid Agreement Number", ebufp);
			//PIN_FLIST_DESTROY_EX(&find_agreement_no_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&find_agreement_no_outflistp, NULL);
			return 3;
		}
		

		//PIN_FLIST_DESTROY_EX(&find_agreement_no_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&find_agreement_no_outflistp, NULL);

		purchased_plans_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 1, ebufp);
		//if(purchased_plans_flistp)
		//{
		  //purcased_plan_pdp = PIN_FLIST_FLD_GET(purchased_plans_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 1, ebufp);
		  //PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_PURCHASED_PLAN_OBJ , purcased_plan_pdp , ebufp);
		//}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Device object not found");
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_device_object_with_plan search return flist", i_flistp);

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	return 0;
}

static void 
fm_validate_sp_code(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp)
{

	int		db = 1;
	int32		srch_flag = 512;
	pin_flist_t	*arg_flist = NULL;
	pin_flist_t	*result_flist = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*srch_flistp = NULL;
	pin_flist_t	*srch_oflistp = NULL;
	char		*sp_account_no = NULL;
	poid_t		*srch_pdp = NULL;
	poid_t		*service_pdp = NULL;
	int32           order_status_success = 0;
        int32           order_status_failure = 1;
	int32		sp_btype = 19000000;


	char		template[700] = "select X from /account  where F1 = V1 and account_t.business_type in ('19000000','24000000') " ;


	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ,1, ebufp);

	if (service_pdp && strcmp((char*)PIN_POID_GET_TYPE(service_pdp), MSO_CATV) == 0)
	{
		 sprintf(template,"select X from /account  where F1 = V1 ");
	}

	

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_validate_sp_code  input list", i_flistp);

	sp_account_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STOCK_POINT_CODE, 0, ebufp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, sp_account_no, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_validate_sp_code search input list", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp) || (PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS,ebufp ) <1))
        {
		        
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching SP account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID,PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp), ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51049", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS , &order_status_failure , ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in searhing Stock point account", ebufp);
		*r_flistpp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_validate_sp_code output list", *r_flistpp);
		//PIN_FLIST_DESTROY_EX(&r_flistp , NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
                return;
        }

	result_flist = PIN_FLIST_ELEM_GET(srch_oflistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,0,ebufp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(result_flist,PIN_FLD_POID,r_flistp, PIN_FLD_POID,ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS , &order_status_success , ebufp);
	*r_flistpp = r_flistp;
	//*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_validate_sp_code output list", *r_flistpp);
	
	//PIN_FLIST_DESTROY_EX(&r_flistp , NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	return;    

}

