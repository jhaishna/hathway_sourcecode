/******************************************************************************
 *
 * Copyright (c) 2019 Hathway. All rights reserved.
 *
 * This material is the confidential property of Hathway 
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid Hathway license or sublicense agreement.
 *
 *****************************************************************************/
/*******************************************************************************
 * Maintentance Log:
 *
 * Date: 20190508
 * Author: Tata Consultancy Services 
 * Identifier: Multiple Children Discount 
 * Notes: Registraction of child connections of a customer 
 ***********************************************************************/
/*******************************************************************************
 * File History
 *
 * DATE     |  CHANGE                                           |  Author
 ***********|***************************************************|****************
 |08/05/2019|Initial vesion					| Rama Puppala
 |**********|***************************************************|****************
 ********************************************************************************/

#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_device_child_registration.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
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


#define FILE_LOGNAME "fm_mso_device_child_registration.c"
#define GROUP_MSO_HIER_POID_TYPE "/group/mso_hierarchy" 


/*******************************************************************************
 * Functions Defined outside this source file
 ******************************************************************************/

PIN_IMPORT void
fm_mso_get_device_details(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	char		*device_id,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void
fm_mso_get_base_pdts(
	pcm_context_t	*ctxp,
	poid_t		*acct_pdp,
	poid_t		*srvc_pdp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT time_t 
fm_mso_base_pack_validation(
	pcm_context_t   *ctxp,
	pin_flist_t     *alc_addon_pdts_flistp,
	int32           is_base_pack,
	pin_errbuf_t    *ebufp);

PIN_IMPORT void
fm_mso_is_child_service(
	pcm_context_t	*ctxp,
	poid_t		*serv_pdp,
	int32		parent_info_flag,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT int32
fm_mso_trans_open(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        int32           flag,
        pin_errbuf_t    *ebufp);

PIN_IMPORT void
fm_mso_trans_commit(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT void
fm_mso_trans_abort(
        pcm_context_t   *ctxp,
        poid_t          *pdp,
        pin_errbuf_t    *ebufp);

/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_device_child_registration(
    cm_nap_connection_t    *connp,
    int32            opcode,
    int32            flags,
    pin_flist_t        *i_flistp,
    pin_flist_t        **o_flistpp,
    pin_errbuf_t        *ebufp);

static void
fm_mso_device_child_registration(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int32		flags,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

/*******************************************************************************

 ******************************************************************************/
void
op_mso_device_child_registration(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	int32			*status = NULL;
	int32			error_set = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
        	return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*************************************************
	 * Insanity check.
	 ************************************************/
	if (opcode != MSO_OP_DEVICE_CHILD_REGISTRATION) 
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_mso_device_child_registration", ebufp);
        	return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_device_child_registration input flist", i_flistp);

	fm_mso_device_child_registration(ctxp, i_flistp, flags, o_flistpp, ebufp);

	if (*o_flistpp)
	{
		status = PIN_FLIST_FLD_GET(*o_flistpp, PIN_FLD_STATUS, 1, ebufp);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp) && (status && (*(int*)status == 0))) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_device_child_registration error", ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
		error_set = 1;
		*o_flistpp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *o_flistpp, PIN_FLD_POID, ebufp);
        	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &error_set, ebufp);
    	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_device_child_registration return flist", *o_flistpp);

    return;
}


/*******************************************************************************
 * fm_mso_device_child_registration()
 *
 * Inputs: flist with devicep array, each element including
 *         a STB_ID/VC_ID and a connection type; flist with customer contacts 
 * Outputs: void; ebuf set if errors encountered
 *
 * Description:
 * This function validates the input devices belong to the same LCO or not 
 * and create a group /group/mso_hierarchy object upon successful registration 
 ******************************************************************************/
static void
fm_mso_device_child_registration(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int32		flags,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*device_r_flistp = NULL;
	pin_flist_t	*grp_crt_flistp = NULL;
	pin_flist_t	*grp_crt_o_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*serv_flistp = NULL;
	pin_flist_t	*serv_in_flistp = NULL;
	pin_flist_t	*serv_out_flistp = NULL;
	pin_flist_t	*memb_flistp = NULL;
	pin_flist_t	*inh_flistp = NULL;
	pin_flist_t	*acc_flistp = NULL;
	pin_flist_t	*disc_flistp = NULL;
	pin_flist_t	*disc_oflistp = NULL;
	pin_flist_t	*alc_addon_pdts_main_flistp = NULL;
	pin_flist_t	*catv_infop = NULL;
	pin_cookie_t	cookie = NULL;
	poid_t         	*acc_pdp = NULL;
	poid_t		*curr_acc_pdp = NULL;
	poid_t          *serv_pdp = NULL;
	poid_t          *pdp = NULL;
	poid_t          *lco_pdp = NULL;
	poid_t          *lco_current_pdp = NULL;
	pin_decimal_t	*per_discp = NULL;
	char		*program_namep = NULL;
	char		*devicep = NULL;
	char		*acnt_no = NULL;
	time_t		disc_end_t = 1893436200;
	int64		db = -1;
	int32		*conn_typep = NULL;
	int32		elem_id = 0;
    	int32		grp_status_active = 10100;
	int32		ret_status = 0;	
	int32		pkg_type = 3;
	int32		disc_type = 0;
	int32		*disc_status = NULL;
	int32		serv_type = 0;
	int32		failure = 1;
	int32		child_conn_type = 2;
	int32		t_status = -1;
	int32		*flagsp = NULL;
	int32		action_flag = 0;
	int32		acc_no_val = -1;
	time_t		bp_valid_flag = 0;

	void		*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/*
	 * Validate the input flist if required fields are present
	 */
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(pdp);

	acnt_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
	if (acnt_no && (strlen(acnt_no) != 10 || sscanf(acnt_no, "%d", &acc_no_val) != 1))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Account Number Format");
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "51100", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Account Number", ebufp);
		goto CLEANUP;
	}

	if ((acc_no_val != -1) && (acc_no_val < 1000000000 || acc_no_val > 2000000000))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Account Number") ;
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "51101", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Account Number", ebufp);
		goto CLEANUP;
        }

	if (acnt_no && strcmp(acnt_no, "") !=0 )
	{
		fm_mso_get_acnt_from_acntno(ctxp, acnt_no, &acc_flistp, ebufp);
		if (acc_flistp == NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account Number not in our system");
			*ret_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, pdp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "51102", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Account Number", ebufp);
			goto CLEANUP;	
		}

		acc_pdp = PIN_FLIST_FLD_GET(acc_flistp, PIN_FLD_POID, 0, ebufp);

		/****************************************************************
		 * Get service details of an account
		****************************************************************/
		fm_mso_utils_get_service_details(ctxp, acc_pdp, 0, &serv_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp) || serv_flistp == NULL)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error gettig the service details !!");
			*ret_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, pdp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "51103", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "There are no active services for your account", ebufp);
			goto CLEANUP;	
                }

	}
	

	per_discp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CYCLE_DISCOUNT, 1, ebufp);
	flagsp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (flagsp)
	{
		action_flag = *(int32 *) flagsp;
	}

	grp_crt_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(grp_crt_flistp, PIN_FLD_POID, 
		PIN_POID_CREATE(db, GROUP_MSO_HIER_POID_TYPE, (int64)-1, ebufp), ebufp);	
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, grp_crt_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(grp_crt_flistp, PIN_FLD_NAME, "CATV Parent-Child Hierarchy", ebufp);
	PIN_FLIST_FLD_SET(grp_crt_flistp, PIN_FLD_TYPE_STR, "Parent-Child Hierarchy", ebufp);
	PIN_FLIST_FLD_SET(grp_crt_flistp, PIN_FLD_STATUS, &grp_status_active, ebufp);
	inh_flistp = PIN_FLIST_SUBSTR_ADD(grp_crt_flistp, PIN_FLD_INHERITED_INFO, ebufp);

	while (action_flag && (res_flistp = PIN_FLIST_ELEM_GET_NEXT(serv_flistp, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		serv_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);		
		memb_flistp = PIN_FLIST_ELEM_ADD(grp_crt_flistp, PIN_FLD_MEMBERS, elem_id, ebufp);
		PIN_FLIST_FLD_SET(memb_flistp, PIN_FLD_OBJECT, serv_pdp, ebufp);
		PIN_FLIST_FLD_SET(memb_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
		PIN_FLIST_FLD_SET(memb_flistp, PIN_FLD_CONN_TYPE, conn_typep, ebufp);
		
		tmp_flistp = PIN_FLIST_SUBSTR_GET(res_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
		conn_typep = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CONN_TYPE, 0, ebufp);
		if (conn_typep && *conn_typep == 0)
		{
			PIN_FLIST_FLD_SET(grp_crt_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
			PIN_FLIST_FLD_SET(grp_crt_flistp, PIN_FLD_PARENT, acc_pdp, ebufp);
		}
		else
		{
			/*****************************************************************
			 * Update connection type in service object
			 ****************************************************************/
			if (t_status == -1)
			{	
				t_status = fm_mso_trans_open(ctxp, acc_pdp, 1, ebufp);
			}

			serv_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(serv_in_flistp, PIN_FLD_POID, serv_pdp, ebufp);
			catv_infop = PIN_FLIST_SUBSTR_ADD(serv_in_flistp, MSO_FLD_CATV_INFO, ebufp);	
			PIN_FLIST_FLD_SET(catv_infop, PIN_FLD_CONN_TYPE, &child_conn_type, ebufp);

			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, serv_in_flistp, &serv_out_flistp, ebufp);

			PIN_FLIST_DESTROY_EX(&serv_in_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&serv_out_flistp, NULL);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Error in updating the service", ebufp);
				ret_status = 5;
				break;
			}

			tmp_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_ALIAS_LIST, PIN_ELEMID_ANY, 1, ebufp);
			if (tmp_flistp)
			{
				devicep = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_NAME, 0, ebufp);
			}

			disc_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_POID,
				PIN_POID_CREATE(db, "/service", (int64)-1, ebufp), ebufp);      
			PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_ACCOUNT_NO, acnt_no, ebufp);
			PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_DEVICE_ID, devicep, ebufp);	
			PIN_FLIST_FLD_SET(disc_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
			PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_DISCOUNT_FLAGS, &disc_type, ebufp);
			PIN_FLIST_FLD_SET(disc_flistp, MSO_FLD_SERVICE_TYPE, &serv_type, ebufp);
			PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_END_T, &disc_end_t, ebufp);
			if (per_discp)
			{
				PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_DISCOUNT, per_discp, ebufp);
			}
			else
			{
                               	PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_DISCOUNT,
                              		pbo_decimal_from_str("0.6", ebufp), ebufp);      
			}
					
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Discount config create input", disc_flistp);	

			PCM_OP(ctxp, MSO_OP_CUST_CATV_DISC_CONFIG, flags,
					disc_flistp, &disc_oflistp, ebufp);

			PIN_FLIST_DESTROY_EX(&disc_flistp, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Discount config create output", disc_oflistp);	

			disc_status = (int32 *)PIN_FLIST_FLD_GET(disc_oflistp, PIN_FLD_STATUS, 1, ebufp);
			if ((disc_status && *disc_status == failure) || PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_FLIST_DESTROY_EX(&disc_oflistp, ebufp);
				ret_status = 3;
				pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_DUPLICATE,
				PIN_FLD_DISCOUNT, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Discount already configured", ebufp);
				break;
			}
		}
		PIN_FLIST_ELEM_SET(inh_flistp, memb_flistp, PIN_FLD_MEMBERS, elem_id, ebufp);
	}

	elem_id = 0;
	cookie = NULL;
	while (!action_flag && (tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_DEVICES,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		conn_typep = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CONN_TYPE, 0, ebufp);
		devicep = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_STB_ID, 1, ebufp);
		if (devicep)
		{
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_TYPE, "/device/stb", ebufp);
		}
		else
		{
			devicep = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_VC_ID, 1, ebufp);
			if (devicep)
			{
				PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DEVICE_TYPE, "/device/vc", ebufp);
			}
		}
		if (devicep)
		{
			fm_mso_get_device_details(ctxp, i_flistp, devicep, &device_r_flistp, ebufp);
			if (device_r_flistp)
			{
				res_flistp = PIN_FLIST_ELEM_GET(device_r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
				if (res_flistp)
				{
					if (lco_pdp == NULL)
					{
						lco_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
					}
					else
					{
						lco_current_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
					}

					if (lco_current_pdp && PIN_POID_COMPARE(lco_pdp, lco_current_pdp, 0, ebufp) != 0)
					{
						ret_status = 2;
						pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MATCH,
							MSO_FLD_LCO_OBJ, 0, 0);
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Registering STB LCO not matching", ebufp);
						break;
					}
					serv_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_SERVICES,
						PIN_ELEMID_ANY, 1, ebufp);
					if (serv_flistp)
					{
						/*******************************************
						 * Comment this logic till NTO2 rollout
						if (acc_pdp == NULL)
						{
							acc_pdp = PIN_FLIST_FLD_GET(serv_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
						}
						else
						{
							curr_acc_pdp = PIN_FLIST_FLD_GET(serv_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
						}

						if (curr_acc_pdp && PIN_POID_COMPARE(acc_pdp, curr_acc_pdp, 0, ebufp) != 0)
						{
							ret_status = 11;
							pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MATCH,
								MSO_FLD_LCO_OBJ, 0, 0);
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Registering STB Account not matching", ebufp);
							break;
						}
						*******************************************/
						acc_pdp = PIN_FLIST_FLD_GET(serv_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
						serv_pdp = PIN_FLIST_FLD_GET(serv_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);		
					
						memb_flistp = PIN_FLIST_ELEM_ADD(grp_crt_flistp, PIN_FLD_MEMBERS, elem_id, ebufp);
						PIN_FLIST_FLD_SET(memb_flistp, PIN_FLD_OBJECT, serv_pdp, ebufp);
						PIN_FLIST_FLD_SET(memb_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
						PIN_FLIST_FLD_SET(memb_flistp, PIN_FLD_CONN_TYPE, conn_typep, ebufp);
						/*************************************************************
			 		 	 * If parent connection, get account_obj_id0 to set as parent
			 		 	 ************************************************************/
						if (conn_typep && *conn_typep == 0)
						{
							t_status = fm_mso_trans_open(ctxp, acc_pdp, 1, ebufp);

							fm_mso_get_base_pdts(ctxp, acc_pdp, serv_pdp, &alc_addon_pdts_main_flistp, ebufp);

							if (alc_addon_pdts_main_flistp && alc_addon_pdts_main_flistp != NULL)
							{
								ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp,
									PIN_FLD_STATUS, 0, ebufp);
								if (ret_status == failure)
								{
									ret_status = 4;
									pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
										PIN_ERR_NOT_FOUND,
										PIN_FLD_PRODUCT, 0, 0);
									PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Base Package of main conn is Inactive.", ebufp);
									break;
								}
							}
			
							bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 0, ebufp);
							if (bp_valid_flag == 0)
							{
								ret_status = 4;
								pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
									PIN_ERR_NOT_FOUND,
									PIN_FLD_PRODUCT, 0, 0);
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Base Pack of main conn is Inactive.", ebufp);
								break;
							}
							PIN_FLIST_FLD_SET(grp_crt_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
							PIN_FLIST_FLD_SET(grp_crt_flistp, PIN_FLD_PARENT, acc_pdp, ebufp);
							PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NAME, memb_flistp, PIN_FLD_NAME, ebufp);
							PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_RMN, memb_flistp, MSO_FLD_RMN, ebufp);
							PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_RMAIL, memb_flistp, MSO_FLD_RMAIL, ebufp);
						}
						else
						{
							/*****************************************************************
							 * Update connection type in service object
							 ****************************************************************/

							serv_in_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(serv_in_flistp, PIN_FLD_POID, serv_pdp, ebufp);
							catv_infop = PIN_FLIST_SUBSTR_ADD(serv_in_flistp, MSO_FLD_CATV_INFO, ebufp);	
							PIN_FLIST_FLD_SET(catv_infop, PIN_FLD_CONN_TYPE, &child_conn_type, ebufp);

							PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, serv_in_flistp, &serv_out_flistp, ebufp);

							PIN_FLIST_DESTROY_EX(&serv_in_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&serv_out_flistp, NULL);

							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
									"Error in updating the service", ebufp);
								ret_status = 5;
								break;
							}

							fm_mso_get_account_info(ctxp, acc_pdp, &acc_flistp, ebufp);
							vp = PIN_FLIST_FLD_TAKE(acc_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
							PIN_FLIST_DESTROY_EX(&acc_flistp, ebufp);

							disc_flistp = PIN_FLIST_CREATE(ebufp);
                                                        PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_POID,
                                                        	PIN_POID_CREATE(db, "/service", (int64)-1, ebufp), ebufp);      
                                                        PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_ACCOUNT_NO, vp, ebufp);
                                                        PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_DEVICE_ID, devicep, ebufp);	
							PIN_FLIST_FLD_SET(disc_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
							PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_DISCOUNT_FLAGS, &disc_type, ebufp);
							PIN_FLIST_FLD_SET(disc_flistp, MSO_FLD_SERVICE_TYPE, &serv_type, ebufp);
							PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_END_T, &disc_end_t, ebufp);
							if (per_discp)
							{
								PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_DISCOUNT, per_discp, ebufp);
							}
							else
							{
                                                        	PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_DISCOUNT,
                                                        		pbo_decimal_from_str("0.6", ebufp), ebufp);      
							}
					
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
								"Discount config create input", disc_flistp);	

							PCM_OP(ctxp, MSO_OP_CUST_CATV_DISC_CONFIG, flags,
								disc_flistp, &disc_oflistp, ebufp);

							PIN_FLIST_DESTROY_EX(&disc_flistp, ebufp);

							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
								"Discount config create output", disc_oflistp);	

							disc_status = (int32 *)PIN_FLIST_FLD_GET(disc_oflistp, PIN_FLD_STATUS, 1, ebufp);
							if ((disc_status && *disc_status == failure) || PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_FLIST_DESTROY_EX(&disc_oflistp, ebufp);
								ret_status = 3;
								pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_DUPLICATE,
									PIN_FLD_DISCOUNT, 0, 0);
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Discount already configured", ebufp);
								break;
							}
							PIN_FLIST_DESTROY_EX(&disc_oflistp, ebufp);
						}
						PIN_FLIST_ELEM_SET(inh_flistp, memb_flistp, PIN_FLD_MEMBERS, elem_id, ebufp);
					}
				}
				else
				{
					ret_status = 10;
					pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND,
						MSO_FLD_STB_ID, 0, 0);
				}
			}
		}
		else
		{
			ret_status = 1;
			pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND,
				MSO_FLD_STB_ID, 0, 0);
			break;
		}
	}

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_device_child_registration: Mandatory fields missing in flist", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
		if (ret_status == 1)
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "72001", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Mandatory fields missing in the input !!!", ebufp);
		}
		else if (ret_status == 2)
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "72002", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Registering STB not belong to same LCO !!!", ebufp);
		}
		else if (ret_status == 3)
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "72003", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Discount already configured for child box!!!", ebufp);
		}
		else if (ret_status == 4)
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "72004", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Base Pack of main connection is Inactive!!!", ebufp);
		}
		else if (ret_status == 5)
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "72005", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Child connection type update failed!!!", ebufp);
		}
		else if (ret_status == 10)
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "72010", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "STB not in our system !!!", ebufp);
		}
		else if (ret_status == 11)
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "72011", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Registering STB not belong to same Account !!!", ebufp);
		}
		goto CLEANUP;
	}


	/*********************************************************************
	 * Call PCM_OP_GROUP_CREATE_GROUP opcode to create the group
	 *******************************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_device_child_registration group create input flist ", grp_crt_flistp);
	PCM_OP(ctxp, PCM_OP_GROUP_CREATE_GROUP, flags, grp_crt_flistp,
		&grp_crt_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_device_child_registration group create output flist", grp_crt_o_flistp);

    	PIN_FLIST_DESTROY_EX(&grp_crt_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		ret_status = 3;
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "72003", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Hierarchy of STB creation failed", ebufp);
		goto CLEANUP;
	}

	*ret_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
       	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
       	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_DESCR, "Registration of multiple connections successful", ebufp);
    

CLEANUP:
	PIN_FLIST_DESTROY_EX(&grp_crt_flistp, NULL);
    	PIN_FLIST_DESTROY_EX(&grp_crt_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);

	if ((PIN_ERRBUF_IS_ERR(ebufp) || ret_status != 0 ) && t_status == 0 )
	{
		fm_mso_trans_abort(ctxp, acc_pdp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"abort transaction");
	}
	else if (( ! PIN_ERRBUF_IS_ERR(ebufp) && ret_status == 0 ) && t_status == 0)
	{
		fm_mso_trans_commit(ctxp, acc_pdp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"commit transaction");
	}
	return;
}

