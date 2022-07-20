/*******************************************************************
 * File:	fm_mso_cust_set_customer_bit.c
 * Opcode:	MSO_OP_CUST_SET_CUSTOMER_BIT 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist:
 * 0 PIN_FLD_POID                          POID [0] 0.0.0.1  0 0 
 * 0 PIN_FLD_LOGIN                          STR [0] "CATV-00000001" 
 * 0 PIN_FLD_FLAGS                          INT [0] 0
 * 0 PIN_FLD_INHERITED_INFO           SUBSTRUCT [0] allocated 20, used 1
 * 1     MSO_FLD_PERSONAL_BIT_INFO        ARRAY [0] allocated 20, used 2    
 * 2        PIN_FLD_PARAM_NAME              STR [0] "bit_number"            
 * 2        PIN_FLD_PARAM_VALUE             STR [0] "bit_value"             
 * 
 * flags =0 : Initialize/Add
 * flags =1 : Modify Existing
 *
 * Multiple Arrays can be passed during initialization, but onle one array 
 *   to be passed for modification in case the elem_id is not known
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_set_customer_bit.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_prov.h"

#define ADD_CUSTOMER_BIT 0
#define MOD_CUSTOMER_BIT 1



/**************************************
* External Functions start
***************************************/
/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_SET_CUSTOMER_BIT 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_set_customer_bit(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_set_customer_bit(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_SET_CUSTOMER_BIT  
 *******************************************************************/
void 
op_mso_cust_set_customer_bit(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	poid_t			*inp_pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_SET_CUSTOMER_BIT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_set_customer_bit error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_set_customer_bit input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_set_customer_bit(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_set_customer_bit error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_set_customer_bit output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_set_customer_bit(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*pers_bit_info = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*pers_bit_info_passed = NULL;
	pin_flist_t		*inherited_info_passed = NULL;
	pin_flist_t		*update_srvc_iflist = NULL;
	pin_flist_t		*update_srvc_oflist = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*create_prov_iflist = NULL;
	pin_flist_t		*create_prov_oflist = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*srvc_pdp = NULL;

	char			*template = "select X from /service where  F1 = V1 " ;
	char			*param = NULL;
	char			*param_passed = NULL;
	char			*param_val = NULL;

	int32			*flag = NULL;
	int32			set_customer_bit_failure = 1;
	int32			set_customer_bit_success = 0;
	int64			db = -1;
	int32			srch_flag = 512;
	int32			count_pb_info = 0;
	int32			flag_cb_exist = 0;
	int32			elem_id = 0 ;
	int32			prov_flgs =0;
	pin_cookie_t		cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_set_customer_bit input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in calling fm_mso_cust_set_customer_bit", ebufp);
		goto CLEANUP;
	}

	flag = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if (flag && (*flag < 0 || *flag > 1) )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, 
			PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
			"Invalid Flags value, 0: initialize/add, 1:Modify ", ebufp);
		goto CLEANUP;
	}
	
	inherited_info_passed =  PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_INHERITED_INFO, 1, ebufp );
	if (!inherited_info_passed)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, 
			PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
			"Missing PIN_FLD_INHERITED_INFO ", ebufp);
		goto CLEANUP;
	}
	count_pb_info = PIN_FLIST_ELEM_COUNT(inherited_info_passed, MSO_FLD_PERSONAL_BIT_INFO, ebufp);

	if (count_pb_info ==0 )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, 
			PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
			"Missing MSO_FLD_PERSONAL_BIT_INFO ", ebufp);
		goto CLEANUP;
	}
	if (*flag == 1 && count_pb_info >1 )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, 
			PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
			"Only one customer bit info can be modified ", ebufp);
		goto CLEANUP;
	}

	pers_bit_info_passed = PIN_FLIST_ELEM_GET(inherited_info_passed, 
		MSO_FLD_PERSONAL_BIT_INFO, 0, 1, ebufp );
	param_passed = PIN_FLIST_FLD_GET(pers_bit_info_passed, PIN_FLD_PARAM_NAME, 0, ebufp );
        param_val = PIN_FLIST_FLD_GET(pers_bit_info_passed, PIN_FLD_PARAM_VALUE, 0, ebufp );
        if ((strcmp(param_val,"A") == 0) || (strcmp(param_val,"A\n") == 0))
        {
                prov_flgs = CATV_SET_BIT;
        }
        else if ((strcmp(param_val,"D") == 0) || (strcmp(param_val,"D\n") == 0))
        {
                prov_flgs = CATV_UNSET_BIT;
        }
	else
        {
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, 
			PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR,
                        "wrong MSO_FLD_PERSONAL_BIT_INFO ", ebufp);
		 goto CLEANUP;
        }

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ));
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, srvc_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_ELEM_SET(result_flist, NULL, MSO_FLD_PERSONAL_BIT_INFO, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_cust_set_customer_bit SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_cust_set_customer_bit SEARCH output flist", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (result_flist && *flag == MOD_CUSTOMER_BIT )
	{
		if (PIN_FLIST_ELEM_COUNT(result_flist, MSO_FLD_PERSONAL_BIT_INFO, ebufp) == 0 )
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, 
				PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
				"No Customer BIT exist to be modified", ebufp);
			goto CLEANUP;
		}

		while((pers_bit_info = PIN_FLIST_ELEM_GET_NEXT(result_flist, MSO_FLD_PERSONAL_BIT_INFO,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			param = PIN_FLIST_FLD_GET(pers_bit_info, PIN_FLD_PARAM_NAME, 0, ebufp );
			if (strcmp (param_passed, param ) == 0)
			{
				flag_cb_exist =1;
				break;
			}
		}

		if (flag_cb_exist == 0 )
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, 
				PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
				"Matching customer bit param not exist", ebufp);
			goto CLEANUP;
		}
	}
	else if (!result_flist )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, 
			PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Service ", ebufp);
		goto CLEANUP;
	}
	/*****************************************************
	* Prepare input for PCM_OP_CUST_UPDATE_SERVICES
	*****************************************************/
	update_srvc_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(update_srvc_iflist, PIN_FLD_POID, 
		(PIN_FLIST_FLD_GET(result_flist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp)),ebufp);
	PIN_FLIST_FLD_SET(update_srvc_iflist, PIN_FLD_PROGRAM_NAME, "SET_CUSTOMER_BIT_INFO", ebufp); 

	services = PIN_FLIST_ELEM_ADD(update_srvc_iflist, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(services, PIN_FLD_POID, 
		(PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp)),ebufp);
	if (*flag == ADD_CUSTOMER_BIT)
	{
		elem_id = 0;
		cookie = NULL;

		inherited_info = PIN_FLIST_SUBSTR_ADD(services, PIN_FLD_INHERITED_INFO, ebufp);
		while((pers_bit_info_passed = PIN_FLIST_ELEM_GET_NEXT(inherited_info_passed, 
			MSO_FLD_PERSONAL_BIT_INFO, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_FLIST_ELEM_SET(inherited_info, pers_bit_info_passed, 
				MSO_FLD_PERSONAL_BIT_INFO, PIN_ELEMID_ASSIGN, ebufp );
		}
	}
	else if (*flag == MOD_CUSTOMER_BIT)
	{
		inherited_info = PIN_FLIST_SUBSTR_ADD(services, PIN_FLD_INHERITED_INFO, ebufp);
		PIN_FLIST_ELEM_COPY(inherited_info_passed, MSO_FLD_PERSONAL_BIT_INFO, 0, 
			inherited_info, MSO_FLD_PERSONAL_BIT_INFO, elem_id, ebufp );
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_srvc_iflist", update_srvc_iflist);

	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, update_srvc_iflist, &update_srvc_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&update_srvc_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_srvc_oflist", update_srvc_oflist);

	/******************************************************************
	* Call Provisioning APIs
	* Start
	*******************************************************************/
	create_prov_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(create_prov_iflist, PIN_FLD_POID, 
		(PIN_FLIST_FLD_GET(result_flist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp)), ebufp);
	PIN_FLIST_FLD_SET(create_prov_iflist, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(create_prov_iflist, MSO_FLD_REGION_NUMBER , param_passed, ebufp);
	PIN_FLIST_FLD_SET(create_prov_iflist, PIN_FLD_FLAGS , &prov_flgs, ebufp);

	/* Write USERID, PROGRAM_NAME in buffer  - start */
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_USERID,create_prov_iflist,PIN_FLD_USERID,ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,create_prov_iflist,PIN_FLD_PROGRAM_NAME,ebufp);
	/* Write USERID, PROGRAM_NAME in buffer - end */

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "create_prov_iflist", create_prov_iflist);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, create_prov_iflist, &create_prov_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&create_prov_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "create_prov error",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, 
			PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
			"Error Calling MSO_OP_PROV_CREATE_ACTION", ebufp);   
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"create_prov_action input list output flist", create_prov_oflist);

	/******************************************************************
	* Call Provisioning APIs
	* End
	*******************************************************************/	
	/********************************************************* 
	 * Errors..?
	 *********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_set_customer_bit error",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, 
			PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Unknown BRM Error", ebufp);
		goto CLEANUP;
	}
	else
	{
		ret_flistp = PIN_FLIST_COPY(result_flist, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &set_customer_bit_success, ebufp);
		PIN_FLIST_ELEM_COPY(inherited_info_passed, MSO_FLD_PERSONAL_BIT_INFO, 0, 
			ret_flistp, MSO_FLD_PERSONAL_BIT_INFO, elem_id, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, 
			"Customer Bit modified successfully", ebufp);		
	}
CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
 	PIN_FLIST_DESTROY_EX(&update_srvc_oflist, NULL);
	*r_flistpp = ret_flistp;
	return;
}
