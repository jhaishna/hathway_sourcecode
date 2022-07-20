/*******************************************************************
 * File:	fm_mso_cust_update_gst_info.c
 * Opcode:	MSO_OP_CUST_UPDATE_GST_INFO 
 * Owner:	Kunal Bhardwaj
 * Created:	08-Mar-2017
 * Description: 
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_update_gst_info.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_ntf.h"
#include "ops/device.h"
#include "mso_lifecycle_id.h"
#include "mso_device.h"
#include "ops/bal.h"

/*******************************************************************
 * MSO_OP_CUST_UPDATE_GST_INFO 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_update_gst_info(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_update_gst_info(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	int32			is_resgister_customer,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

int
fm_mso_validate_csr_role(
        pcm_context_t		*ctxp,
        int64        		db,
        poid_t        		*user_id,
        pin_errbuf_t  		*ebufp);

void
fm_mso_create_gst_info_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT int
fm_mso_cust_get_gst_profile(
        pcm_context_t 		*ctxp,
        poid_t      		*account_pdp,
        pin_flist_t   		**r_flistpp,
        pin_errbuf_t  	  	*ebufp);

/*******************************************************************
 * MSO_OP_CUST_UPDATE_GST_INFO
 *******************************************************************/
void 
op_mso_cust_update_gst_info(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	poid_t			*inp_pdp = NULL;
	int                     local = LOCAL_TRANS_OPEN_SUCCESS;
	int32			failure = 1;
	int32			success = 0;
	int32			*status = NULL;
	int32                   stack_opcode = 0;
	int32			is_register_customer = 0;

	cm_op_info_t            *opstackp = connp->coip;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_UPDATE_GST_INFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_update_gst_info error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_update_gst_info input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	opstackp = connp->coip;
	while(opstackp != NULL)
	{
		stack_opcode = opstackp->opcode;
		if(stack_opcode == MSO_OP_CUST_REGISTER_CUSTOMER)
            	{
			is_register_customer  = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction will be handled at Register Customer API");
			break;
		}
		opstackp = opstackp->next;
	}

	if (!is_register_customer)
	{
		local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "50001", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
			return;
		}
		if ( local == LOCAL_TRANS_OPEN_FAIL )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "50002", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
			return;
		}	
	}
	fm_mso_cust_update_gst_info(ctxp, flags, i_flistp, is_register_customer, r_flistpp, ebufp);
	/***********************************************************
	 * Results.
	 ***********************************************************/
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp) || (status && *(int*)status == 1))
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_update_gst_info in flist", i_flistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_update_gst_info error", ebufp);
		if(!is_register_customer && local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else if (!is_register_customer)
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS)
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		}
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_update_gst_info output flist", *r_flistpp);
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_update_gst_info(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	int32			is_register_customer,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*profile_in_flistp = NULL;
	pin_flist_t		*profile_out_flistp = NULL;
	pin_flist_t		*profile_flistp = NULL;
	pin_flist_t		*gst_info_flistp = NULL;
	pin_flist_t		*inherited_flistp = NULL;
	pin_flist_t		*existing_profile_flistp = NULL;
	pin_flist_t		*upd_profile_in_flistp = NULL;
	pin_flist_t             *upd_profile_out_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	
	
	poid_t			*profile_pdp = NULL;
	poid_t			*existing_profile_pdp = NULL;
	poid_t			*account_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*user_id = NULL;

	char			*template = "select X from /account where  F1 = V1 " ;
	char			*profile_name = "GST_INFO";
	char			*account_no = NULL;
	char			*state = NULL;
	char			*gstin = NULL;
	char			*bu_code = NULL;
	char			*city = NULL;
	char			*existing_state = NULL;
	char			*pgm_name = NULL;
	int64			db = 1;
	int32			srch_flag = 256;
	time_t			current_t = 0;
	int32			failure = 1;
	int32			success = 0;
	int32			csr_access = 0;
	int32			ret_flag = 0;
	int32			elem_id = 0;
	pin_cookie_t		cookie = NULL;
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_update_gst_info input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_update_gst_info", ebufp);
		goto CLEANUP;
	}

	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
	db = PIN_POID_GET_DB(user_id);
        account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	pgm_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
        profile_pdp = PIN_POID_CREATE(db, "/profile/gst_info", -1, ebufp);
        current_t = pin_virtual_time(NULL);
	*r_flistpp = PIN_FLIST_CREATE(ebufp);


        /*******************************************************************
        * Validation for CSR roles
        *******************************************************************/

	if (!is_register_customer)
	{
        	if (user_id)
       	 	{
                	csr_access = fm_mso_validate_csr_role(ctxp, db, user_id, ebufp);
                	if ( csr_access == 0)
                	{
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                        	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                        	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51006", ebufp);
                        	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,"CSR does not have permissions",ebufp);
                        	goto CLEANUP;
                	}
			else
                	{
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR haas permission to change account information");
                	}
        	}
		else
        	{
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51005", ebufp);
                	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "User ID Missing", ebufp);
                	goto CLEANUP;
        	}
	}
	

	/*******************************************************************
	* Mandatory fields validation
	*******************************************************************/
	if (!account_no )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Account Number", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Missing Account Number", ebufp);
		goto CLEANUP;
	}
	
        if (!pgm_name )
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Mandatory Information", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51007", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Missing Mandatory Information", ebufp);
                goto CLEANUP;
        }


	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_update_gst_info search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error searching account");
                PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51001", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error searching account", ebufp);
                goto CLEANUP;

	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_update_gst_info search output flist", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		account_pdp = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
		/*
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /profile/gst_info -1 4
		0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 1290705 0
		0 PIN_FLD_NAME            STR [0] "GST_INFO"
		0 PIN_FLD_PROFILES ARRAY [0] allocated 20, used 18
		1 PIN_FLD_PROFILE_OBJ    POID [0] 0.0.0.1 /profile/gst_info -1 4
		1 PIN_FLD_INHERITED_INFO SUBSTRUCT [0] allocated 20, used 18
		2 MSO_FLD_GST_INFO ARRAY [0] allocated 20, used 18
		3     PIN_FLD_ACCOUNT_NO        STR [0] "GST NUMBER"
		3     PIN_FLD_ACCOUNT_CODE        STR [0] "BU CODE"
		3     PIN_FLD_ADDRESS        STR [0] " ADDRESS"
		3     PIN_FLD_CITY        STR [0] "MUMBAI"
		3     PIN_FLD_STATE        STR [0] "MAHARASHTRA"
		3     PIN_FLD_COUNTRY        STR [0] "INDIA"
		3     PIN_FLD_ZIP        STR [0] "400057"
		3     PIN_FLD_IMAGE_REF_NO        STR [0] "//202.88.131.226/Documents/GST path"
		3     PIN_FLD_FILENAME        STR [0] "GST DOC"
		*/
		gst_info_flistp = PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_GST_INFO, PIN_ELEMID_ANY, 1, ebufp);
		state = PIN_FLIST_FLD_GET(gst_info_flistp, PIN_FLD_STATE, 1, ebufp);
		gstin = PIN_FLIST_FLD_GET(gst_info_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		bu_code = PIN_FLIST_FLD_GET(gst_info_flistp, PIN_FLD_ACCOUNT_CODE, 1, ebufp);
		city = PIN_FLIST_FLD_GET(gst_info_flistp, PIN_FLD_CITY, 1, ebufp);
		if(!state || !gstin || !bu_code || !city)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Missing Mandatory Info");
                        PIN_ERRBUF_RESET(ebufp);
                        ret_flag = 0;
                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51007", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Missing Mandatory Info", ebufp);
                        goto CLEANUP;
		}

		//getting existing gst profile
		ret_flag = fm_mso_cust_get_gst_profile(ctxp, account_pdp, &existing_profile_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error searching account");
			PIN_ERRBUF_RESET(ebufp);
			ret_flag = 0;
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51001", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error searching profile", ebufp);
			goto CLEANUP;
		}

		if(existing_profile_flistp)
		{
			while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(existing_profile_flistp, MSO_FLD_GST_INFO, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				existing_state = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATE, 1, ebufp);
				if (existing_state && state && strcmp(existing_state, state) == 0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"GST Configurationg already present for state");
					PIN_ERRBUF_RESET(ebufp);
					ret_flag = 0;
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51001", ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "GST Configurationg already present for state", ebufp);
					goto CLEANUP;
				}
			}
		}
		if(ret_flag == 0 && existing_profile_flistp == NULL)	
		{
			profile_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(profile_in_flistp, PIN_FLD_POID, profile_pdp, ebufp);
			PIN_FLIST_FLD_SET(profile_in_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
			PIN_FLIST_FLD_SET(profile_in_flistp, PIN_FLD_NAME, profile_name, ebufp);
			PIN_FLIST_FLD_SET(profile_in_flistp, PIN_FLD_EFFECTIVE_T, &current_t, ebufp);
			profile_flistp = PIN_FLIST_ELEM_ADD(profile_in_flistp, PIN_FLD_PROFILES, 0, ebufp);
			PIN_FLIST_FLD_SET(profile_flistp, PIN_FLD_PROFILE_OBJ, profile_pdp, ebufp);
			inherited_flistp = PIN_FLIST_SUBSTR_ADD(profile_flistp, PIN_FLD_INHERITED_INFO, ebufp);
			PIN_FLIST_ELEM_SET(inherited_flistp, gst_info_flistp, MSO_FLD_GST_INFO, 0, ebufp);
			/*gst_info_flistp = PIN_FLIST_ELEM_ADD(inherited_flistp, MSO_FLD_GST_INFO, 0, ebufp);
			PIN_FLIST_FLD_SET(gst_info_flistp, PIN_FLD_ACCOUNT_NO, gst_number, ebufp);
			PIN_FLIST_FLD_SET(gst_info_flistp, PIN_FLD_ACCOUNT_CODE, bu_code, ebufp);
			PIN_FLIST_FLD_SET(gst_info_flistp, PIN_FLD_ADDRESS, address, ebufp);
			PIN_FLIST_FLD_SET(gst_info_flistp, PIN_FLD_CITY, city, ebufp);
			PIN_FLIST_FLD_SET(gst_info_flistp, PIN_FLD_STATE, state, ebufp);
			PIN_FLIST_FLD_SET(gst_info_flistp, PIN_FLD_COUNTRY, country, ebufp);
			PIN_FLIST_FLD_SET(gst_info_flistp, PIN_FLD_ZIP, zip, ebufp);
			PIN_FLIST_FLD_SET(gst_info_flistp, PIN_FLD_IMAGE_REF_NO, image_path, ebufp);
			PIN_FLIST_FLD_SET(gst_info_flistp, PIN_FLD_FILENAME, file_name, ebufp);*/
		
			PIN_ERR_LOG_FLIST(3, "Create Profile Input Flist: ", profile_in_flistp);
			PCM_OP(ctxp, PCM_OP_CUST_CREATE_PROFILE, 0, profile_in_flistp, &profile_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&profile_in_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error in creating GST profile");
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51003", ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in creating GST profile", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(3, "Create Profile Output Flist: ", profile_out_flistp);
			//*r_flistpp = PIN_FLIST_COPY(profile_out_flistp, ebufp);
			result_flist = PIN_FLIST_ELEM_GET(profile_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );

		}
		else if (existing_profile_flistp)
		{	
			existing_profile_pdp = PIN_FLIST_FLD_GET(existing_profile_flistp, PIN_FLD_POID, 0, ebufp);
			upd_profile_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(upd_profile_in_flistp, PIN_FLD_POID, existing_profile_pdp, ebufp );
			PIN_FLIST_ELEM_SET(upd_profile_in_flistp, gst_info_flistp, MSO_FLD_GST_INFO, PIN_ELEMID_ASSIGN, ebufp);	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Update Profile Input Flistp", upd_profile_in_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, upd_profile_in_flistp, &upd_profile_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&upd_profile_in_flistp, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Update Profile Output Flistp", upd_profile_out_flistp);
                       	if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error in creating GST profile");
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51003", ebufp);
                                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in creating GST profile", ebufp);
                                goto CLEANUP;
                        }
			PIN_FLIST_FLD_COPY(upd_profile_out_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
		}	
		fm_mso_create_gst_info_lc_event(ctxp, i_flistp, profile_out_flistp, ebufp); 
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error in creating lifecycle event");
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51004", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in creating lifecylei event", ebufp);
                        goto CLEANUP;
                }

	 	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &success, ebufp);		
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "00", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Success", ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Invalid Account Number");
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51002", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Account Number", ebufp);
		goto CLEANUP;
	}

	CLEANUP:
	if (profile_pdp && !PIN_POID_IS_NULL(profile_pdp))
	{
		PIN_POID_DESTROY(profile_pdp, ebufp);
	}
	
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&profile_out_flistp, NULL);
	
	return;
}

PIN_EXPORT int
fm_mso_cust_get_gst_profile(
	pcm_context_t		*ctxp,
	poid_t			*account_pdp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;

	poid_t                  *profile_pdp = NULL;
	poid_t                  *srch_pdp = NULL;

	char                    *template = "select X from /profile/gst_info where  F1 = V1 and F2.type = V2 " ;
	int64                   db = 1;
	int32                   srch_flag = 256;
	int32			ret_flag = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_cust_get_gst_profile");
		return;
	}
	db = PIN_POID_GET_DB(account_pdp);
	profile_pdp = PIN_POID_CREATE(db, "/profile/gst_info", 1, ebufp);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, profile_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_gst_profile search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error searching Profile");
		PIN_ERRBUF_RESET(ebufp);
		ret_flag = 0;
		*r_flistpp = NULL;
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_gst_profile search output flist", srch_out_flistp);
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		ret_flag = 1;
		*r_flistpp = PIN_FLIST_COPY(result_flist, ebufp);
	}
	else
	{
		ret_flag = 0;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_gst_profile output flist", *r_flistpp);

CLEANUP:

	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return ret_flag;
}

