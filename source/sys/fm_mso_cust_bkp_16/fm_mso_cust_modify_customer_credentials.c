/*******************************************************************
 * File:	fm_mso_cust_modify_customer_credentials.c
 * Opcode:	MSO_OP_CUST_MODIFY_CUSTOMER_CREDENTIALS 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 *
	0 PIN_FLD_USERID          POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_POID            POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_ACCOUNT_NO       STR [0] "988766"
	0 PIN_FLD_LOGIN            STR [0] ""
	0 PIN_FLD_PASSWD_CLEAR     STR [0] ""
********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_modify_customer_credentials.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_cust.h"
#include "mso_lifecycle_id.h"


/**************************************
* External Functions start
***************************************/
extern char*
fm_mso_encrypt_passwd(
	pcm_context_t		*ctxp,
	poid_t			*service_pdp,
	char			*non_encrypted_pwd,
	pin_errbuf_t		*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_MODIFY_CUSTOMER_CREDENTIALS 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_modify_customer_credentials(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_modify_customer_credentials(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_MODIFY_CUSTOMER_CREDENTIALS  
 *******************************************************************/
void 
op_mso_cust_modify_customer_credentials(
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
	if (opcode != MSO_OP_CUST_MODIFY_CUSTOMER_CREDENTIALS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_modify_customer_credentials error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_modify_customer_credentials input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_modify_customer_credentials(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_modify_customer_credentials error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_modify_customer_credentials output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_modify_customer_credentials(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*acnt_result_flist = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*update_ssa_iflist = NULL;
	pin_flist_t		*update_ssa_oflist = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;


	poid_t			*user_id = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*ssa_pdp = NULL;
	poid_t			*service_pdp = NULL;

	char			*acnt_no = NULL;
	char			*login = NULL;
	char			*pwd = NULL;
	char			*prog_name = NULL;
	char			*encrypted_pwd = NULL;
	char			*existing_login = NULL;
	char			*template = "select X from /mso_customer_credential where  F1.id = V1 or  F2.id = V2 order by customer_type " ;
	// as the value of customer type of CSR/CSR_ADMIN is 2 and that of end customer is 12 so directly ELEM_GET with elem id can be used
	int32			flg_access_self = 0;
	int32			ssa_customer_type = -1;
	int32			*customer_type = NULL;

	int64			db = -1;
	int32			srch_flag = 512;
	int32			modify_ssa_failure = 1;
	int32			modify_ssa_success = 0;
	int32			elem_id = 0;
	int32			caller = MSO_OP_CUST_MODIFY_CUSTOMER_CREDENTIALS;

	pin_cookie_t		cookie = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;
	void			*vp2 = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer_credentials input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_modify_customer_credentials", ebufp);
		goto CLEANUP;
	}

	user_id =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID , 1, ebufp );
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ); 
	acnt_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp ); 
	login = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 1, ebufp );
	pwd = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PASSWD_CLEAR, 1, ebufp ); 
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	db = PIN_POID_GET_DB(user_id);
	if (PIN_POID_COMPARE(user_id, acnt_pdp,0, ebufp) == 0 )
	{
		flg_access_self=1;
	}

  	/*******************************************************************
	* Mandatory fields validation
	*******************************************************************/
	if (!user_id || !acnt_pdp || !acnt_no || !login || !pwd || !prog_name)
	{
		//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing userid, account no, login or  pwd or program name", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp); 
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_ssa_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51180", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing userid, account no, login or pwd or program name", ebufp);
		goto CLEANUP;
	}

	/*******************************************************************
	* Root Login Validation - Start
	*******************************************************************/
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, user_id, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer_credentials SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_customer_credentials SESRCH output flist", srch_out_flistp);

	while((vp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{

		vp1 = PIN_FLIST_FLD_GET((pin_flist_t*)vp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		if (vp1 && PIN_POID_COMPARE(user_id, (poid_t*)vp1 , 0, ebufp) == 0 ) //CRS's profile
		{
			result_flist =  PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, elem_id, 1, ebufp);
		}

		vp2 = PIN_FLIST_FLD_GET((pin_flist_t*)vp, PIN_FLD_LOGIN, 1, ebufp);
		if (vp2 && strcmp ((char*)vp2, login) == 0 ) //Searched account's profile
		{
			acnt_result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, elem_id, 1, ebufp);
		}
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"CSR Profile", result_flist );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Searched account profile", acnt_result_flist );


	//result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (!result_flist)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"USERID or Account does not exist", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp); 
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_ssa_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51181", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "USERID or Account does not exist", ebufp);
		goto CLEANUP;
	}
	if (result_flist && !flg_access_self )
	{
		customer_type = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_CUSTOMER_TYPE, 1, ebufp);
		if (customer_type && (*customer_type == ACCT_TYPE_CSR || *customer_type == ACCT_TYPE_CSR_ADMIN ))
		{
			//acnt_result_flist =  PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 1, 1, ebufp);
			if (!acnt_result_flist)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Account Passed", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp); 
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_ssa_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51182", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Account Passed", ebufp);
				goto CLEANUP;	
			}
			existing_login = (char* )PIN_FLIST_FLD_GET(acnt_result_flist, PIN_FLD_LOGIN, 0, ebufp );
			ssa_pdp = PIN_POID_COPY( PIN_FLIST_FLD_GET(acnt_result_flist, PIN_FLD_POID, 0, ebufp ),ebufp);
			ssa_customer_type = *(int32*)PIN_FLIST_FLD_GET(acnt_result_flist, PIN_FLD_CUSTOMER_TYPE, 1, ebufp);
			//if (ssa_customer_type == ACCT_TYPE_CSR || ssa_customer_type == ACCT_TYPE_CSR_ADMIN )
			if (  *customer_type == ACCT_TYPE_CSR && 
			    ( ssa_customer_type == ACCT_TYPE_CSR || ssa_customer_type == ACCT_TYPE_CSR_ADMIN) 
			   )
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"CSR can not change password of another CSR", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_ssa_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51183", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CSR can not change password of another CSR", ebufp);
				goto CLEANUP;
			}
			if (strcmp(existing_login, login) !=0 )
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Login", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_ssa_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51184", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Login", ebufp);
				goto CLEANUP;
			}
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"CREDENTIAL change Prevelige not given to this account", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, user_id, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp); 
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_ssa_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51185", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CREDENTIAL change Prevelige not given to this account", ebufp);
			goto CLEANUP;
		}
		
	}
	else if (result_flist && flg_access_self)
	{
		existing_login = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_LOGIN, 0, ebufp );
		ssa_pdp = PIN_POID_COPY( PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp ), ebufp);
		ssa_customer_type = *(int32*)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_CUSTOMER_TYPE, 1, ebufp);
		service_pdp = PIN_POID_COPY(PIN_FLIST_FLD_GET(result_flist, PIN_FLD_USERID, 1, ebufp), ebufp);

		if (strcmp(existing_login, login) !=0 )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Login", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_ssa_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51186", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Login", ebufp);
			goto CLEANUP;
		}
	}
	encrypted_pwd = fm_mso_encrypt_passwd(ctxp, acnt_pdp, pwd, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	/********************************************************* 
	 * Update /mso_customer_credentials
	 *********************************************************/
	update_ssa_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(update_ssa_iflist, PIN_FLD_POID, ssa_pdp, ebufp );
	PIN_FLIST_FLD_SET(update_ssa_iflist, PIN_FLD_PASSWD, encrypted_pwd, ebufp );
	
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_ssa_iflist, &update_ssa_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&update_ssa_iflist, NULL);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_FLIST_DESTROY_EX(&update_ssa_oflist, NULL)
	/********************************************************* 
	 * Mofify the password for /service/admin_client
	 *********************************************************/	
//	if (ssa_customer_type != ACCT_TYPE_SUBSCRIBER )
//	{
//		update_ssa_iflist = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_PUT(update_ssa_iflist, PIN_FLD_POID, service_pdp, ebufp );
//		PIN_FLIST_FLD_PUT(update_ssa_iflist, PIN_FLD_PASSWD, encrypted_pwd, ebufp );
//		
//		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_ssa_iflist, &update_ssa_oflist, ebufp);
//		PIN_FLIST_DESTROY_EX(&update_ssa_iflist, NULL);
//		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
//		if (PIN_ERRBUF_IS_ERR(ebufp))
//		{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
//			return;
//		}
//		PIN_FLIST_DESTROY_EX(&update_ssa_oflist, NULL)	
//	}

	/********************************************************* 
	 * Errors..?
	 *********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_cust_modify_customer_credentials error",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp); 
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_ssa_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51187", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "fm_mso_cust_modify_customer_credentials error", ebufp);
		goto CLEANUP;
	}
	
  	/********************************************************* 
	 * Prepare Output Flist for success case
	 *********************************************************/
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_ssa_success, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp); 
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "Password Changed Successfully.", ebufp);

	//lifecycle
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACTION_MODE, &caller, ebufp);
	fm_mso_create_lifecycle_evt(ctxp, i_flistp, NULL, ID_MODIFY_CUSTOMER_CHANGE_USERNAME_PASSWORD, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&update_ssa_oflist, NULL);
	*r_flistpp = ret_flistp;
	return;
}

