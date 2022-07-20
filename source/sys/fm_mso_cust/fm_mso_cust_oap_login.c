/*******************************************************************
 * File:	fm_mso_cust_oap_login.c
 * Opcode:	MSO_OP_CUST_REGISTER_CUSTOMER 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_oap_login.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"

#define PWD_MATCHES 1


/**************************************
* External Functions start
***************************************/
/**************************************
* External Functions end
***************************************/
static int32
fm_mso_compare_passwd(
	pcm_context_t		*ctxp,
	poid_t			*service_pdp,
	char			*pwd_entered,
	char			*pwd_stored,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_OAP_LOGIN 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_oap_login(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_oap_login(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_OAP_LOGIN  
 *******************************************************************/
void 
op_mso_cust_oap_login(
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
	if (opcode != MSO_OP_CUST_OAP_LOGIN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_oap_login error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_oap_login input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_oap_login(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_oap_login error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_oap_login output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_oap_login(
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

	poid_t			*service_pdp = NULL;
	poid_t			*srch_pdp = NULL;
//	poid_t			*cust_cred_pdp = NULL;
	char			*login = NULL;
	char			*pwd = NULL;
	char			*pwd_stored = NULL;
	char			*template_1 = "select X from /service where  F1 = V1 and F2.type = V2 " ;
	char			*template_2 = "select X from /mso_customer_credential where  F1 = V1 " ;
//	char			*roles = NULL;
//	int32			*login_type = NULL;
	int32			*customer_type = NULL;
	int64			db = -1;
	int32			srch_flag = 256;
	int32			pwd_compare_result = 0;
	int32			login_failure = 1;
	int32			login_success = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_oap_login input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_oap_login", ebufp);
		goto CLEANUP;
	}

	service_pdp =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
	db = PIN_POID_GET_DB(service_pdp);
	login = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 1, ebufp ); 
	pwd = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PASSWD, 1, ebufp ); 
//	login_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LOGIN_AS, 1, ebufp ); 

	/*******************************************************************
	* Mandatory fields validation
	*******************************************************************/
	if (!login || !pwd )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"MISSING LOGIN, PWD", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, service_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp); 
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "MISSING LOGIN, PWD", ebufp);
		goto CLEANUP;
	}

	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_2 , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_LOGIN, login, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_USERID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PASSWD, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_ROLES, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CUSTOMER_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_DATA_ACCESS, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_oap_login SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_oap_login SESRCH output flist", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (result_flist)
	{

		/*******************************************************************
		* Compare MSO_FLD_LOGIN_AS with PIN_FLD_CUSTOMER_TYPE - Start
		*******************************************************************/
//		customer_type = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_CUSTOMER_TYPE, 1, ebufp );
//		if (!(customer_type && (*customer_type == *login_type )))
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "invalid login type") ;
//			ret_flistp = PIN_FLIST_CREATE(ebufp);
//			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, service_pdp, ebufp );
//			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp);
//			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &login_failure, ebufp); 
//			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
//			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "PLEASE LOGIN AFTER CHANGING 'login as'", ebufp);
//			goto CLEANUP;
//		}
		/*******************************************************************
		* Compare MSO_FLD_LOGIN_AS with PIN_FLD_CUSTOMER_TYPE - End
		*******************************************************************/

		/*******************************************************************
		* Compare_Password - Start
		*******************************************************************/
		pwd_stored =  (char *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PASSWD, 0, ebufp );
		pwd_compare_result = fm_mso_compare_passwd(ctxp, service_pdp, pwd, pwd_stored, ebufp);

		if (pwd_compare_result == PWD_MATCHES )
		{
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(result_flist, PIN_FLD_USERID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_ROLES, PIN_FLIST_FLD_GET(result_flist, MSO_FLD_ROLES, 0, ebufp ), ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_DATA_ACCESS, PIN_FLIST_FLD_GET(result_flist, MSO_FLD_DATA_ACCESS, 0, ebufp ), ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &login_success, ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_ACCOUNT_OBJ, ret_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_CUSTOMER_TYPE, PIN_FLIST_FLD_GET(result_flist, PIN_FLD_CUSTOMER_TYPE, 1, ebufp ), ebufp);
			
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"INCORRECT PASSWORD", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, service_pdp, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp); 
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &login_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51002", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "INCORRECT PASSWORD", ebufp);
			goto CLEANUP;
		}
		/*******************************************************************
		* Compare_Password - End
		*******************************************************************/
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"INVALID LOGIN", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, service_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp); 
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51001", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &login_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "INVALID LOGIN", ebufp);
		goto CLEANUP;
	}
	/*******************************************************************
	* Root Login Validation - End
	*******************************************************************/
	
	
	

	/********************************************************* 
	 * Errors..?
	 *********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_cust_oap_login error",ebufp);
	}
	

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	*r_flistpp = ret_flistp;
	return;
}

/**************************************************
* Function: fm_mso_compare_passwd()
* 
* 
***************************************************/
static int32
fm_mso_compare_passwd(
	pcm_context_t		*ctxp,
	poid_t			*service_pdp,
	char			*pwd_entered,
	char			*pwd_stored,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*compare_pwd_input = NULL;
	pin_flist_t		*compare_pwd_output = NULL;

	int32			ret_val = 0;


	compare_pwd_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(compare_pwd_input, PIN_FLD_POID, service_pdp, ebufp);
	PIN_FLIST_FLD_SET(compare_pwd_input, PIN_FLD_PASSWD_CLEAR, pwd_entered, ebufp);
	PIN_FLIST_FLD_SET(compare_pwd_input, PIN_FLD_PASSWD, pwd_stored, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_compare_passwd input list", compare_pwd_input);
	PCM_OP(ctxp, PCM_OP_CUST_POL_COMPARE_PASSWD, 0, compare_pwd_input, &compare_pwd_output, ebufp);
	PIN_FLIST_DESTROY_EX(&compare_pwd_input, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_compare_passwd", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_compare_passwd output flist", compare_pwd_output);
	ret_val = *(int32*)PIN_FLIST_FLD_GET(compare_pwd_output, PIN_FLD_BOOLEAN, 0, ebufp );

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&compare_pwd_output, NULL);
	return ret_val;
}
