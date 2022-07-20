/******************************************************************************
 *
 * Copyright (c) 2017 Hathway. All rights reserved.
 *
 * This material is the confidential property of Hathway 
 * or its licensors and may be used, reproduced, stored or transmitted
 * only in accordance with a valid DexYP license or sublicense agreement.
 *
 *****************************************************************************/
/*******************************************************************************
 * Maintentance Log:
 *
 * Date: 20190508
 * Author: Tata Consultancy Services 
 * Identifier: LCO account details
 * Notes: Getting LCO details of a customer 
 ***********************************************************************/
/*******************************************************************************
 * File History
 *
 * DATE     |  CHANGE                                           |  Author
 ***********|***************************************************|****************
 |03/06/2019|Initial vesion					| Rama Puppala
 |**********|***************************************************|****************
 ********************************************************************************/

#ifndef lint
static const char Sccs_id[] = "(#)$Id: fm_mso_cust_get_lco_info.c st_cgbubrm_gsivakum_bug-8453030/1 2009/05/05 23:37:48 gsivakum Exp $";
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


#define FILE_LOGNAME "fm_mso_cust_get_lco_info.c"


/*******************************************************************************
 * Functions Defined outside this source file
 ******************************************************************************/

PIN_IMPORT void
fm_mso_get_acnt_from_acntno(
	pcm_context_t	*ctxp,
	char		*acnt_no,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void
fm_mso_get_lco_city(
	pcm_context_t	*ctxp,
	poid_t		*account_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void fm_mso_utils_get_service_details(
	pcm_context_t	*ctxp,
	poid_t		*acct_pdp,
	int32		serv_type,
	pin_flist_t	**o_flistpp,
	pin_errbuf_t	*ebufp);

/*******************************************************************************
 * Functions contained within this source file
 ******************************************************************************/
EXPORT_OP void
op_mso_cust_get_lco_info(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_get_lco_info(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int32		flags,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

/*******************************************************************************

 ******************************************************************************/
void
op_mso_cust_get_lco_info(
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
	if (opcode != MSO_OP_CUST_GET_LCO_INFO) 
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_mso_cust_get_lco_info", ebufp);
        	return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_cust_get_lco_info input flist", i_flistp);

	fm_mso_cust_get_lco_info(ctxp, i_flistp, flags, o_flistpp, ebufp);

	if (*o_flistpp)
	{
		status = PIN_FLIST_FLD_GET(*o_flistpp, PIN_FLD_STATUS, 1, ebufp);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp) && (status && (*(int*)status != 0))) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_get_lco_info error", ebufp);
    	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_cust_get_lco_info return flist", *o_flistpp);

    return;
}


/*******************************************************************************
 * fm_mso_cust_get_lco_info()
 *
 * Inputs: flist with POID and ACCOUNT_NO or MSO_FLD_LEGACY_ACCOUNT_NO i
 * as mandatory fields 
 * Outputs: void; ebuf set if errors encountered
 *
 * Description:
 * This function retrieves account poid of the given account number
 * and finds LCO details of the given account 
 ******************************************************************************/
static void
fm_mso_cust_get_lco_info(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	int32		flags,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*acnt_infop = NULL;
	pin_flist_t	*lco_infop = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*ph_flistp = NULL;
	pin_flist_t	*srch_flistp = NULL;	
	pin_flist_t	*srch_out_flistp = NULL;	
	pin_flist_t	*serv_flistp = NULL;
	pin_flist_t	*serv_res_flistp = NULL;
	poid_t         	*acc_pdp = NULL;
	poid_t         	*pdp = NULL;
	pin_cookie_t	cookie = NULL;
	char		*acc_nop = NULL;
	char		*legacy_acc_nop = NULL;
	char		*templatep = "select X from /account where F1 = V1 ";
	char		*device_idp = NULL;
	int64		db = -1;
	int32		*phone_typep = NULL;
	int32		srch_flag = 768;
	int32		elem_id = 0;
	int32		ret_status = 0;	

	void		*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/*
	 * Validate the input flist if required fields are present
	 */
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	acc_nop = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	legacy_acc_nop = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, 1, ebufp);

	if (acc_nop && strcmp(acc_nop, "") != 0)
	{
		fm_mso_get_acnt_from_acntno(ctxp, acc_nop, &acnt_infop, ebufp);
	}
	else if (legacy_acc_nop && strcmp(legacy_acc_nop, "") != 0)
	{
		db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp));
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, templatep, ebufp);
	
		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, tmp_flistp,
			MSO_FLD_LEGACY_ACCOUNT_NO, ebufp);

		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_RMN, NULL, ebufp);

		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

		acnt_infop = PIN_FLIST_ELEM_TAKE(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	}
	else
	{
		ret_status = 1;
		pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND,
			PIN_FLD_ACCOUNT_NO, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Account number not found in input FLIST", ebufp);
		goto CLEANUP;
	}
	
	if (acnt_infop)
	{
		acc_pdp = PIN_FLIST_FLD_GET(acnt_infop, PIN_FLD_POID, 0, ebufp);	
		fm_mso_get_lco_city(ctxp, acc_pdp, &lco_infop, ebufp);

		/****************************************************************
		 * Get service details of an account
		 ****************************************************************/
		fm_mso_utils_get_service_details(ctxp, acc_pdp, 0, &serv_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp) || serv_flistp == NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error gettig the service details !!");
		}

		serv_res_flistp = PIN_FLIST_ELEM_GET(serv_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if (serv_res_flistp == NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No services are attached to the account !!");
		}
		tmp_flistp = PIN_FLIST_ELEM_GET(serv_res_flistp, PIN_FLD_ALIAS_LIST, 1, 1, ebufp);
		if (tmp_flistp == NULL)
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(serv_res_flistp, PIN_FLD_ALIAS_LIST, 0, 1, ebufp);
		}
		if (tmp_flistp == NULL)
		{
			tmp_flistp = PIN_FLIST_ELEM_GET(serv_res_flistp, PIN_FLD_ALIAS_LIST, 2, 1, ebufp);
		}
	
		if (tmp_flistp)
		{
			device_idp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_NAME, 0, ebufp);	
		}
    	
		PIN_FLIST_DESTROY_EX(&serv_flistp, NULL);
	}

	*ret_flistpp = PIN_FLIST_CREATE(ebufp);
	if (lco_infop)
	{
		vp = PIN_FLIST_FLD_TAKE(lco_infop, PIN_FLD_POID, 0, ebufp);
		PIN_FLIST_FLD_PUT(*ret_flistpp, PIN_FLD_POID, vp, ebufp);

		vp = PIN_FLIST_FLD_TAKE(lco_infop, MSO_FLD_RMN, 0, ebufp);
		PIN_FLIST_FLD_PUT(*ret_flistpp, MSO_FLD_RMN, vp, ebufp);

		vp = PIN_FLIST_FLD_TAKE(lco_infop, MSO_FLD_RMAIL, 0, ebufp);
		PIN_FLIST_FLD_PUT(*ret_flistpp, MSO_FLD_RMAIL, vp, ebufp);

		vp = PIN_FLIST_FLD_TAKE(lco_infop, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		PIN_FLIST_FLD_PUT(*ret_flistpp, MSO_FLD_LCO_ACCT_NO, vp, ebufp);

		if (device_idp)
		{
			PIN_FLIST_FLD_PUT(*ret_flistpp, PIN_FLD_DEVICE_ID, device_idp, ebufp);
		}

		tmp_flistp = PIN_FLIST_ELEM_GET(lco_infop, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 1, ebufp);
		if (tmp_flistp)
		{
			vp = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_COMPANY, 0, ebufp);
			PIN_FLIST_FLD_PUT(*ret_flistpp, PIN_FLD_NAME, vp, ebufp);
		}
		while((ph_flistp = PIN_FLIST_ELEM_GET_NEXT(tmp_flistp, PIN_FLD_PHONES,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			phone_typep = PIN_FLIST_FLD_GET(ph_flistp, PIN_FLD_TYPE, 0, ebufp);
			if (phone_typep && *(int32 *) phone_typep == 2)
			{
				vp = PIN_FLIST_FLD_TAKE(ph_flistp, PIN_FLD_PHONE, 0, ebufp);
				PIN_FLIST_FLD_PUT(*ret_flistpp, PIN_FLD_OFFICE_PHONE, vp, ebufp);
				break;
			}

		}
	}
	else
	{
		ret_status = 2;
	}

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_cust_get_lco_info: Mandatory fields missing in flist", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
		if (ret_status == 1)
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "82001", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "Mandatory field ACCOUNT_NO missing in the input !!!", ebufp);
		}
		else if (ret_status == 2)
		{
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_CODE, "82002", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_ERROR_DESCR, "LCO account not found !!!", ebufp);
		}
	}

       	PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_STATUS, &ret_status, ebufp);
    
CLEANUP:
	PIN_FLIST_DESTROY_EX(&lco_infop, NULL);
    	PIN_FLIST_DESTROY_EX(&acnt_infop, NULL);
	return;
}

