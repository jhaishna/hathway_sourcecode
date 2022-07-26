/*******************************************************************
 * File:	fm_mso_cust_register_customer.c
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
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_register_customer.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/bal.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_inv.h"
#include "pin_pymt.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "pin_os.h"
#include "pin_os_string.h"
#include "pin_cust.h"
#include "mso_lifecycle_id.h"

#define WHOLESALE "/profile/wholesale"
#define MSO_FLAG_SRCH_BY_SELF_ACCOUNT 9
#define ACCT_TYPE_DVB               55

//#define PCM_OP_CUSTCARE_MOVE_ACCT 1151



/**************************************
* External Functions start
***************************************/
extern int32
fm_mso_trans_open(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	int32			flag,
	pin_errbuf_t		*ebufp);

extern void 
fm_mso_trans_commit(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	pin_errbuf_t		*ebufp);

extern void  
fm_mso_trans_abort(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	pin_errbuf_t		*ebufp);

extern char*
fm_mso_encrypt_passwd(
	pcm_context_t		*ctxp,
	poid_t			*service_pdp,
	char			*non_encrypted_pwd,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_err_desc(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_account_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_state_city_area_code(
	pcm_context_t		*ctxp,
	char			*state_city_area_code,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_bill_segment(
	pcm_context_t		*ctxp,
	char			*city_code,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_create_ar_limit(
	pcm_context_t	*ctxp,
	poid_t		*account_obj,
	pin_flist_t	**ar_limit,
	pin_errbuf_t	*ebufp);

extern void num_to_array( 
	int	num, 
	int	arr[]);

extern void
fm_mso_utils_get_act_bal_grp (
    pcm_context_t               *ctxp,
    poid_t                      *act_pdp,
    poid_t                      **ret_bal_grp_poidp,
    pin_errbuf_t                *ebufp);

/**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/

static void 
fm_mso_validate_pricing_role(
	pcm_context_t		*ctxp,
	int64			db,
	char			*roles,
	pin_flist_t		**srch_role_outflist,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_update_org_structure(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			account_type,
	int32			corporate_type,
	int32			subscriber_type,
	pin_flist_t		**o_flistp,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_update_account_number(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp)	;

static void 
fm_mso_populate_tax_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			account_model,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_populate_billinfo_and_payinfo(
	pcm_context_t		*ctxp,
	int32			account_type,
	int32			subscriber_type,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_populate_billinfo_and_payinfo_corp(
	pcm_context_t		*ctxp,
	int32			account_type,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_create_hierarchy(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_assign_bill_segment_and_bdom(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	char			*city_code,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static int32
fm_mso_get_parent_payinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*group_info,
	pin_flist_t		**parent_inv_info,
	pin_errbuf_t		*ebufp);

static void
fm_mso_create_tax_exempt(
	pcm_context_t	*ctxp, 
	poid_t		*account_obj,
	pin_flist_t	*exemption,
	pin_errbuf_t	*ebufp);


static void
fm_mso_cust_set_payinfo(
	pcm_context_t	*ctxp, 
	poid_t		*account_obj,
	pin_flist_t	*payinfo,
	pin_errbuf_t	*ebufp);

	
static void
fm_mso_update_account_info(
	pcm_context_t	*ctxp, 
	poid_t		*account_obj,
	char		*last_cmnt,
	pin_errbuf_t	*ebufp);

static void
fm_mso_cust_reg_lc_event(
        pcm_context_t           *ctxp,
        poid_t                  *acc_pdp,
        char                    *acc_no,
        char                    *last_stat_cmnt,
	char			*prog_name,
        pin_errbuf_t            *ebufp);

static int32 
fm_mso_validate_cust_city (
        pcm_context_t   *ctxp,
		pin_flist_t     *i_flistp,
        pin_errbuf_t    *ebufp);

void
fm_mso_create_netflix_service (
        pcm_context_t   *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp);

void fm_mso_create_addon_services (
        pcm_context_t   *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp);
		
PIN_IMPORT void 
fm_mso_get_pref_bdom (
        pcm_context_t   *ctxp,
        char            *area,
        pin_flist_t     **r_flistp,
        pin_errbuf_t    *ebufp);		

PIN_IMPORT void
fm_mso_get_billinfo(
    pcm_context_t       *ctxp,
    poid_t          *bal_grp_obj,
    poid_t          **billinfo,
    pin_errbuf_t        *ebufp);

extern void
fm_mso_get_profile_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

static void fm_mso_bq_config(
        pcm_context_t           *ctxp,
		poid_t					*account_obj,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);
/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_REGISTER_CUSTOMER 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_register_customer(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_register_customer(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT int32 fm_mso_is_den_sp(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * MSO_OP_CUST_REGISTER_CUSTOMER  
 *******************************************************************/
void 
op_mso_cust_register_customer(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
        pin_flist_t             *addon_r_flistpp        = NULL;
	int32			local = LOCAL_TRANS_OPEN_FAIL;
	int32			*ret_status = NULL;
	int32			status_uncommitted = 2;
	poid_t			*inp_pdp = NULL;
	char			*prog_name = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_REGISTER_CUSTOMER) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_register_customer error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_register_customer input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//	if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
	if (!(prog_name && (strstr(prog_name,"pin_deferred_act") || strstr(prog_name,"BULK"))))
	{
		inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51010", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
			return;
		}
		if ( local == LOCAL_TRANS_OPEN_FAIL )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51011", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
			return;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'pin_deferred_act' so transaction will not be handled at API level");
	}

	fm_mso_cust_register_customer(ctxp, flags, i_flistp, r_flistpp, ebufp);
        addon_r_flistpp = PIN_FLIST_COPY(*r_flistpp, ebufp);

        /**Check for addon services like netflix **/
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Call to create Netflix service.");
        fm_mso_create_addon_services(ctxp, i_flistp, &addon_r_flistpp, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix service Created");

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	ret_status = (int32*)PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_register_customer error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else if (ret_status && *ret_status == 1)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_register_customer ret_status=1", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_trans_commit() ...");
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_register_customer output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_register_customer(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*cc_ret_flistp = NULL;
	pin_flist_t		*services_array = NULL;
	pin_flist_t		*cust_cred_inflist = NULL;
	pin_flist_t		*cust_cred_outflist = NULL;
	pin_flist_t		*acctinfo_array = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*validated_output = NULL;
	pin_flist_t		*srch_role_outflist = NULL;
	pin_flist_t		*modify_service_input = NULL;
	pin_flist_t		*modify_service_output = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*role_array = NULL;
	pin_flist_t		*srch_login_iflist = NULL;
	pin_flist_t		*srch_login_oflist = NULL;
	pin_flist_t		*created_billinfo = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*create_out_flist = NULL;
	pin_flist_t		*update_acnt_no_iflist = NULL;
	pin_flist_t		*group_info = NULL;
	pin_flist_t		*get_cust_info_iflist = NULL;
	pin_flist_t		*cust_nameinfo = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*exemption = NULL;
	pin_flist_t		*ar_limit = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*corp_parent_info = NULL;
	pin_flist_t		*actinfo_flistp = NULL;
	pin_flist_t		*bdom_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*gst_info_in_flistp = NULL;
	pin_flist_t		*gst_info_out_flistp = NULL;
	pin_flist_t		*gst_flistp = NULL;
	//DIGITAL OFFERS VARIABLES
	pin_flist_t		*profile_offer_flistp = NULL;
	pin_flist_t		*profile_offer_oflistp = NULL;
	pin_flist_t		*rdfld_flistp = NULL;
	pin_flist_t		*rdfld_oflistp = NULL;

	poid_t			*account_obj = NULL;
	poid_t			*cust_cred_pdp = NULL;
	poid_t			*admin_srvc_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*role_service_pdp = NULL;
	poid_t			*srch_login_pdp = NULL;
	poid_t			*corp_parent_pdp = NULL;
	poid_t			*parent_billinfo_pdp = NULL;
	poid_t			*parent_bg_pdp = NULL;

	char			*login = NULL;
	char			*pwd = NULL;
	char			*encrypted_pwd = NULL;
	char			*roles = NULL;
	char			account_no_str[20];
	char			msg[200];
	char			serv_login[256];
	char			*vp = NULL;
	char			*last_cmnt = NULL;
	char			*prog_name = NULL;
	char			*area	= NULL;
	
	int32			*business_type = NULL;
	int32			tmp_business_type = -1;
	int32			account_type = -1;
	int32			account_model = -1;
	int32			subscriber_type = -1;
	int32			*login_as = NULL;
	int32			flag_internal_user = 0;
	int32			register_customer_success = 0;
	int32			register_customer_failure = 1;
	int32			assign_pricing_role = 0;
	int32			create_output_flag = 0;
	int32			htw_acct_type = ACCT_TYPE_HTW;
	int32			elem_id = 0;
	int32			corporate_type=0;
	int32			pref_dom=13;
	int32			gst_profile_success = 1;
	int32			*gst_status = NULL;
	int32			profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;

	int64			db = -1;
	int64			account_no_int = 0;
	int			srch_flag = 1024;
	int			arr_business_type[8];

	time_t			current_t = 0;
	time_t			next_t = 0;
	time_t			last_t = 0;
	time_t			future_t = 0;
	pin_flist_t		*update_profile_iflistp = NULL;
	pin_flist_t		*update_profile_oflistp = NULL;
	pin_flist_t		*profiles_array_flistp = NULL;
	pin_flist_t		*lc_in_flist = NULL;
        pin_flist_t             *inherited_flistp = NULL;
        pin_flist_t             *wholesale_flistp = NULL;

	
	poid_t			*profile_obj = NULL;

	int32			serv_type = 0;
	pin_cookie_t	        cookie = NULL;
	int32			validation_flag = 0;
	int32			*valpp = NULL;
	int32                   errp = 0;
	int32			pp_type = -1;
	int32			segment_flag  = -1;
	int32			child_pay_type = PIN_PAY_TYPE_SUBORD;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_register_customer input flist", i_flistp);
	lc_in_flist = PIN_FLIST_COPY(i_flistp, ebufp);

	payinfo = PIN_FLIST_ELEM_TAKE(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
	ar_limit = PIN_FLIST_ELEM_TAKE(i_flistp, MSO_FLD_AR_INFO, 0, 1, ebufp);
	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(routing_poid);
	login_as = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LOGIN_AS, 1, ebufp);
	login = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 0, ebufp);
	pwd =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PASSWD_CLEAR, 0, ebufp);
	roles = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ROLES, 0, ebufp);
	acctinfo_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 1, ebufp );
	exemption = PIN_FLIST_CREATE(ebufp);
	while ((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, 
			PIN_FLD_EXEMPTIONS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_EXEMPTIONS, elem_id, exemption, PIN_FLD_EXEMPTIONS, elem_id, ebufp);
	}
	if (acctinfo_array)
	{
		business_type = PIN_FLIST_FLD_GET(acctinfo_array, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
		if (!business_type)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"MISSING PIN_FLD_BUSINESS_TYPE in PIN_FLD_ACCTINFO Array", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51012", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "MISSING PIN_FLD_BUSINESS_TYPE in PIN_FLD_ACCTINFO Array", ebufp);
			goto CLEANUP;
		}
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"MISSING PIN_FLD_ACCTINFO Array", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51013", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "MISSING PIN_FLD_ACCTINFO Array", ebufp);
		goto CLEANUP;
	}

 	/*******************************************************************
	* Derive account_type,account_model & subscriber_type from business_type 
	*  - Start
	*******************************************************************/
	tmp_business_type = *business_type;
//	account_type = tmp_business_type/1000000;
//
//	//reserved_bits = tmp_business_type%1000;
//	tmp_business_type = tmp_business_type/1000;
//
//	account_model = tmp_business_type%10;
//
//	tmp_business_type = tmp_business_type/10;
//	subscriber_type =  tmp_business_type%100;
//
	num_to_array(tmp_business_type, arr_business_type);
	account_type    = 10*(arr_business_type[0])+arr_business_type[1];	// First 2 digits
	subscriber_type = 10*(arr_business_type[2])+arr_business_type[3];	// next 2 digits
	account_model   = arr_business_type[4];				// 5th field
	corporate_type  = arr_business_type[6];				// 7th field

	/************************************************************************
	* Derive account_type,account_model & subscriber_type from business_type 
	* END
	************************************************************************/

	/*************************************************
	Added Condition to validate customer and lco city
	*************************************************/
	if (account_type == ACCT_TYPE_SUBSCRIBER)
	{
		validation_flag = fm_mso_validate_cust_city (ctxp, i_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error in Validating LCO and Customer City");
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51014", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Validating LCO and Customer City", ebufp);
			goto CLEANUP;
		}
		if (validation_flag == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"LCO and Customer City Should be same");
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51014", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "LCO and Customer City Should be same", ebufp);
			goto CLEANUP;
		}
	
		profiles_array_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PROFILES, PIN_ELEMID_ANY, 0, ebufp);
		if (profiles_array_flistp)
		{
			inherited_flistp = PIN_FLIST_SUBSTR_GET(profiles_array_flistp, 
					PIN_FLD_INHERITED_INFO, 1, ebufp);
		
			if (inherited_flistp)
			{
				wholesale_flistp = PIN_FLIST_SUBSTR_GET(inherited_flistp,
						 PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
				if (wholesale_flistp)
				{
					account_obj = PIN_FLIST_FLD_GET(wholesale_flistp, PIN_FLD_PARENT, 0, ebufp);

					profile_offer_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(profile_offer_flistp, PIN_FLD_POID, account_obj, ebufp);
					PIN_FLIST_FLD_SET(profile_offer_flistp, PIN_FLD_PROFILE_DESCR, WHOLESALE, ebufp);
					PIN_FLIST_FLD_SET(profile_offer_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);
					PIN_FLIST_FLD_SET(profile_offer_flistp, PIN_FLD_OBJECT, account_obj, ebufp);

					fm_mso_get_profile_info(ctxp, profile_offer_flistp, &profile_offer_oflistp, ebufp );
					PIN_FLIST_DESTROY_EX(&profile_offer_flistp, NULL);
					
					if (!profile_offer_oflistp)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"LCO profile not found");
						PIN_ERRBUF_RESET(ebufp);
						ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51024", ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "LCO profile not found", ebufp);
						goto CLEANUP;
					}	
						
					temp_flistp = PIN_FLIST_SUBSTR_GET(profile_offer_oflistp, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
					if (temp_flistp)
					{
						account_obj = PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_COMPANY_OBJ, 1, ebufp);
						if (!PIN_POID_IS_NULL(account_obj))
						{
						fm_mso_get_account_info(ctxp, account_obj, &cc_ret_flistp, ebufp); 
		
						business_type = PIN_FLIST_FLD_GET(cc_ret_flistp, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
						if (*business_type == 33000000)
						{
							tmp_business_type = tmp_business_type + 2;
							PIN_FLIST_FLD_SET(acctinfo_array, PIN_FLD_BUSINESS_TYPE, &tmp_business_type, ebufp);
						}
                        else if (*business_type == 44000000)
                        {
							tmp_business_type = tmp_business_type + 4;
							PIN_FLIST_FLD_SET(acctinfo_array, PIN_FLD_BUSINESS_TYPE, &tmp_business_type, ebufp);
                        } 
						PIN_FLIST_DESTROY_EX(&cc_ret_flistp, NULL);
						}
						PIN_FLIST_DESTROY_EX(&profile_offer_oflistp, NULL);
					}	
				}
			}
		}
	}
	
	/**** Pavan Bellala 28/09/2015  ***********************
	Added conditions for account type = reserved 
	*******************************************************/
	if(account_type && (account_type == ACCT_TYPE_RESERVED_BB))
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"ACCT_TYPE_RESERVED_BB invalid account type", i_flistp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51020", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "PIN_FLD_ACCOUNT_TYPE invalid account type in input", ebufp);
		goto CLEANUP;
	}
	

	PIN_FLIST_FLD_SET(lc_in_flist, PIN_FLD_ACCOUNT_TYPE, &account_type, ebufp );

	/*******************************************************************
	* Validation for Commercial/Corporate Customer
	* Start
	*******************************************************************/
	if ( account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD )
	{
		group_info = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 1 , ebufp);
		if (group_info)
		{
			corp_parent_pdp = PIN_FLIST_FLD_GET(group_info, PIN_FLD_PARENT, 1, ebufp );
			if (!corp_parent_pdp  || 
			    PIN_POID_GET_ID(corp_parent_pdp) == 0 ||
			    PIN_POID_GET_ID(corp_parent_pdp) == -1 ||
			    PIN_POID_GET_ID(corp_parent_pdp) == 1 ) 
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid Corporate Parent Account", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51014", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Corporate Parent Account", ebufp);
				goto CLEANUP;
			}

			fm_mso_get_account_info(ctxp, corp_parent_pdp, &corp_parent_info, ebufp );
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Corporate Parent Account does not exist", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51014", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Corporate Parent Account does not exist", ebufp);
				goto CLEANUP;
			}
            parent_bg_pdp = PIN_FLIST_FLD_GET(corp_parent_info, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);   
            fm_mso_get_billinfo(ctxp, parent_bg_pdp, &parent_billinfo_pdp, ebufp); 
//			parent_billinfo_pdp = PIN_FLIST_FLD_GET(group_info, PIN_FLD_PARENT_BILLINFO_OBJ, 1, ebufp );
//			if (!parent_billinfo_pdp ||
//			    PIN_POID_GET_ID(corp_parent_pdp) == 0 ||
//			    PIN_POID_GET_ID(corp_parent_pdp) == -1 ||
//			    PIN_POID_GET_ID(corp_parent_pdp) == 1 )
//			{
//				ret_flistp = PIN_FLIST_CREATE(ebufp);
//				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
//				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
//				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51008", ebufp);
//				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Corporate Parent Billinfo", ebufp);
//				goto CLEANUP;
//			}
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"PIN_FLD_GROUP_INFO mandatory for corporate child account", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51015", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "PIN_FLD_GROUP_INFO mandatory for corporate child account", ebufp);
			goto CLEANUP;
		}
	}
 	/*******************************************************************
	* Validation for Commercial/Corporate Customer
	* End
	*******************************************************************/

	/*******************************************************************
	* Login Uniqueness Validation- Start
	*******************************************************************/
	srch_login_pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
	srch_login_iflist = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_login_iflist, PIN_FLD_POID, srch_login_pdp, ebufp );
	PIN_FLIST_FLD_SET(srch_login_iflist, PIN_FLD_FLAGS, &srch_flag, ebufp );
	PIN_FLIST_FLD_SET(srch_login_iflist, PIN_FLD_TEMPLATE, "select X from /mso_customer_credential where F1 = V1" , ebufp );

	arg_flist = PIN_FLIST_ELEM_ADD(srch_login_iflist, PIN_FLD_ARGS, 1, ebufp );
	/*if (account_type == ACCT_TYPE_HTW)
	{
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CUSTOMER_TYPE, &htw_acct_type, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_LOGIN, login, ebufp);
	} */
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_LOGIN, login, ebufp);
	 
	result_flist = PIN_FLIST_ELEM_ADD(srch_login_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LOGIN, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_login_iflist" ,srch_login_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_login_iflist, &srch_login_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_login_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Login search", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51016", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Login search", ebufp);	
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_login_oflist" ,srch_login_oflist);

	result_flist = PIN_FLIST_ELEM_GET(srch_login_oflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51017", ebufp);
		//msg = pin_malloc(200);
		memset(msg,'\0',sizeof(msg));
		/*if (account_type == ACCT_TYPE_HTW)
		{
			strcpy(msg, "Only one Hathway account can be created");
		}
		else
		{  
			strcpy(msg, "Login '");
			strcat(msg, login);
			strcat(msg, "' Already Exist, Try after changing the login");
		} */
		strcpy(msg, "Login '");
		strcat(msg, login);
		strcat(msg, "' Already Exist, Try after changing the login");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,msg, ebufp);

		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
		goto CLEANUP;
	}
	/*******************************************************************
	* Login Uniqueness Validation- end
	*******************************************************************/

	/*******************************************************************
	* Authorisation for  Internal User Creation- Start
	*******************************************************************/

	if ( account_type == ACCT_TYPE_CSR_ADMIN || 
	     account_type == ACCT_TYPE_CSR ||
	     account_type == ACCT_TYPE_LCO ||
	     account_type == ACCT_TYPE_JV ||
	     account_type == ACCT_TYPE_DTR ||
	     account_type == ACCT_TYPE_SUB_DTR ||
	     account_type == ACCT_TYPE_OPERATION ||
	     account_type == ACCT_TYPE_FIELD_SERVICE ||
	     account_type == ACCT_TYPE_LOGISTICS || 
	     account_type == ACCT_TYPE_COLLECTION_AGENT ||
	     account_type == ACCT_TYPE_SALES_PERSON ||
             account_type == ACCT_TYPE_PRE_SALES
	   )
	{
		// OPEN BELOW if-else when Authorisation required to be done at BRM
		//if(  !(*login_as == ACCT_TYPE_CSR_ADMIN))
		//{
		//	/*******************************************************************
		//	* Validation for CSR Creation permission - Start
		//	*******************************************************************/
		//	ret_flistp = PIN_FLIST_CREATE(ebufp);
		//	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		//	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		//	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51004", ebufp);
		//	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "INSUFFICIENT PREVELIGE:Internal Users can be created by CSR ADMIN only", ebufp);
		//	goto CLEANUP;
		//	/*******************************************************************
		//	* Validation for CSR Creation permission - END
		//	*******************************************************************/
		//}
		//else
		//{
		//	/*******************************************************************
		//	* adding /service/admin_client for CSR and LCO - Start
		//	*******************************************************************/
		//	flag_internal_user = 1;
		//	services_array = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_SERVICES, 0, ebufp);
		//	admin_srvc_pdp = PIN_POID_CREATE(db, "/service/admin_client", (int64)-1, ebufp);
		//	PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, admin_srvc_pdp, ebufp );
		//	PIN_FLIST_FLD_SET(services_array, PIN_FLD_LOGIN, login, ebufp );
		//	PIN_FLIST_FLD_SET(services_array, PIN_FLD_PASSWD_CLEAR, pwd, ebufp );
		//	/*******************************************************************
		//	* adding /service/admin_client for CSR and LCO - End
		//	*******************************************************************/
		//}

		/*******************************************************************
		* adding /service/admin_client for CSR and LCO - Start
		*******************************************************************/
		flag_internal_user = 1;
		services_array = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_SERVICES, 0, ebufp);
		admin_srvc_pdp = PIN_POID_CREATE(db, "/service/admin_client", (int64)-1, ebufp);
		PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, admin_srvc_pdp, ebufp );
		PIN_FLIST_FLD_SET(services_array, PIN_FLD_LOGIN, login, ebufp );
		PIN_FLIST_FLD_SET(services_array, PIN_FLD_PASSWD_CLEAR, pwd, ebufp );
 		/*******************************************************************
		* adding /service/admin_client for CSR and LCO - End
		*******************************************************************/

		//Added Distributor TYPE to the condition

		/******* Pavan Bellala - 24/07/2015 Added Distributor sub type ***********************/
		if (account_type == ACCT_TYPE_LCO || account_type == ACCT_TYPE_DTR || account_type == ACCT_TYPE_SUB_DTR ) {
		//if (account_type == ACCT_TYPE_LCO || account_type == ACCT_TYPE_DTR) 
		//if (account_type == ACCT_TYPE_LCO ) 
			/*******************************************************************
			* adding /service/settlement/ws/incollect & outcollect for LCO - Start
			*******************************************************************/
			sprintf(serv_login, "%s-incollect", login);
			services_array = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_SERVICES, 1, ebufp);
			srvc_pdp = PIN_POID_CREATE(db, "/service/settlement/ws/incollect", (int64)-1, ebufp);
			PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
			PIN_FLIST_FLD_SET(services_array, PIN_FLD_LOGIN, serv_login, ebufp );
			PIN_FLIST_FLD_SET(services_array, PIN_FLD_PASSWD_CLEAR, pwd, ebufp );
			PIN_FLIST_ELEM_SET(services_array, NULL, PIN_FLD_BAL_INFO, 1, ebufp);
			//PIN_FLIST_ELEM_SET(services_array, NULL, PIN_FLD_BAL_INFO, 0, ebufp);
	
			services_array = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_SERVICES, 2, ebufp);
			srvc_pdp = PIN_POID_CREATE(db,"/service/settlement/ws/outcollect", (int64)-1, ebufp);
			sprintf(serv_login,"%s-outcollect",login);
			PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
			PIN_FLIST_FLD_SET(services_array, PIN_FLD_LOGIN, serv_login, ebufp );
			PIN_FLIST_FLD_SET(services_array, PIN_FLD_PASSWD_CLEAR, pwd, ebufp );
			PIN_FLIST_ELEM_SET(services_array, NULL, PIN_FLD_BAL_INFO, 2, ebufp);
			//PIN_FLIST_ELEM_SET(services_array, NULL, PIN_FLD_BAL_INFO, 1, ebufp);
			serv_type=WS_SETTLEMENT;
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_TYPE_OF_SERVICE, &serv_type, ebufp );

		        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"create_customer inflist in progress",i_flistp);
			/*******************************************************************
			* adding /service/settlement/ws/incollect & outcollect for LCO - End
			*******************************************************************/
		}
                if (account_type == ACCT_TYPE_SALES_PERSON ) {
                        /*******************************************************************
                        * adding /service/settlement/ws/dsa_commision - Start
                        *******************************************************************/
                        services_array = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_SERVICES, 1, ebufp);
                        srvc_pdp = PIN_POID_CREATE(db, "/service/settlement/ws/dsa", (int64)-1, ebufp);
                        PIN_FLIST_FLD_PUT(services_array, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
                        sprintf(serv_login,"%s-dsa",login);
                        PIN_FLIST_FLD_SET(services_array, PIN_FLD_LOGIN, serv_login, ebufp );
                        PIN_FLIST_FLD_SET(services_array, PIN_FLD_PASSWD_CLEAR, pwd, ebufp );
                        //PIN_FLIST_ELEM_SET(services_array, NULL, PIN_FLD_BAL_INFO, 1, ebufp);
                        PIN_FLIST_ELEM_SET(services_array, NULL, PIN_FLD_BAL_INFO, 0, ebufp);
                        serv_type=DSA_SETTLEMENT;
                        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_TYPE_OF_SERVICE, &serv_type, ebufp );
                        /*******************************************************************
                        * adding /service/settlement/ws/dsa_commision - End
                        *******************************************************************/
                }

	}
	/*******************************************************************
	* Authorisation for  Internal User Creation - End
	*******************************************************************/
	  
	/*******************************************************************
	* Validation for PRICING_ADMIN roles - Start
	*******************************************************************/
	if ( (account_type == ACCT_TYPE_CSR || account_type == ACCT_TYPE_CSR_ADMIN) && (strstr(roles,"PRICING_ADMIN")))
	{
		//srch_role_outflist = PIN_FLIST_CREATE(ebufp);
		fm_mso_validate_pricing_role(ctxp, db, roles, &srch_role_outflist, ebufp);
		validated_output =  PIN_FLIST_ELEM_GET(srch_role_outflist, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (!validated_output)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"ROLE passed in input does not exist, First create the ROLE", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51018", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ROLE passed in input does not exist, First create the ROLE", ebufp);
			goto CLEANUP;
		}
		else
		{
			assign_pricing_role = 1;
			role_service_pdp = PIN_FLIST_FLD_TAKE(validated_output, PIN_FLD_POID, 0, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_role_outflist, NULL);
		}
	}
	/*******************************************************************
	* Validation for PRICING_ADMIN roles  - End
	*******************************************************************/

	/*******************************************************************
	* LCO/DTR/SUB_DTR Registration  - Start
	*******************************************************************/
/*	if ( account_type == ACCT_TYPE_HTW  ||
	     account_type == ACCT_TYPE_CSR  ||
	     account_type == ACCT_TYPE_CSR_ADMIN  ||
	     account_type == ACCT_TYPE_JV  ||
	     account_type == ACCT_TYPE_DTR ||
	     account_type == ACCT_TYPE_SUB_DTR ||
	     account_type == ACCT_TYPE_LCO ||
	     account_type == ACCT_TYPE_SUBSCRIBER ||
	     account_type == ACCT_TYPE_LOGISTICS
	   )
	{
		fm_mso_update_org_structure(ctxp, i_flistp, account_type, subscriber_type, &ret_flistp, ebufp) ;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_org_structure output flist", i_flistp);
		if (ret_flistp)
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
			goto CLEANUP;
		}
	}
*/
	fm_mso_update_org_structure(ctxp, i_flistp, account_type, corporate_type, subscriber_type, &ret_flistp, ebufp) ;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_org_structure output flist", i_flistp);
	if (ret_flistp)
	{
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		goto CLEANUP;
	}
	/*******************************************************************
	* LCO/DTR/SUB_DTR Registration - End
	*******************************************************************/

	//Added Distributor TYPE to the condition
	if (account_type == ACCT_TYPE_SUBSCRIBER || account_type == ACCT_TYPE_RETAIL_BB || 
		account_type == ACCT_TYPE_SME_SUBS_BB || 
		//account_type == ACCT_TYPE_CYBER_CAFE_BB || removed account type
		account_type == ACCT_TYPE_CORP_SUBS_BB || account_type == ACCT_TYPE_LCO || 
		account_type == ACCT_TYPE_DTR || account_type == ACCT_TYPE_SALES_PERSON || account_type == ACCT_TYPE_SUB_DTR )
	{
	    //Added Distributor TYPE to the condition
	    if(account_type != ACCT_TYPE_LCO && account_type != ACCT_TYPE_DTR && account_type != ACCT_TYPE_SALES_PERSON && account_type != ACCT_TYPE_SUB_DTR ) {
	    //if(account_type != ACCT_TYPE_LCO ) 
		/*******************************************************************
		* Call tax code population-Start
		*******************************************************************/
		fm_mso_populate_tax_info(ctxp, i_flistp, account_model, &ret_flistp, ebufp) ;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info input flist", i_flistp);
		if (ret_flistp)
		{
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
			goto CLEANUP;
		}
		/*******************************************************************
		* Call tax code population-End
		*******************************************************************/
	    }
		/*******************************************************************
		* Call billinfo and payinfo population function
		* Start
		*******************************************************************/
		//if ( account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_PARENT || account_type == ACCT_TYPE_LCO )
		//Added Distributor TYPE to the condition
		
		/*****  Pavan Bellala - 24/07/2015 Added Sub distributor type to the condition *******/

		if ( account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_PARENT || account_type == ACCT_TYPE_LCO || account_type == ACCT_TYPE_DTR || account_type == ACCT_TYPE_SALES_PERSON || account_type == ACCT_TYPE_SUB_DTR )
		{
			fm_mso_populate_billinfo_and_payinfo_corp(ctxp, account_type, i_flistp, &ret_flistp, ebufp);
		}
		else
		{
			fm_mso_populate_billinfo_and_payinfo(ctxp, account_type, corporate_type, i_flistp, &ret_flistp, ebufp);
		}

		if (ret_flistp)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in populating billinfo and payinfo", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			//ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
			//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
			//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51037", ebufp);
			//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in populating billinfo and payinfo", ebufp);	
			goto CLEANUP;
		}
		/*******************************************************************
		* Call billinfo and payinfo population function
		* End
		*******************************************************************/
	}
	/* Remove fields from input flist that should not be passed in account creation */
	acctinfo_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 1, ebufp );
	PIN_FLIST_FLD_COPY(acctinfo_array, PIN_FLD_LASTSTAT_CMNT, lc_in_flist, PIN_FLD_LASTSTAT_CMNT, ebufp);
	last_cmnt = PIN_FLIST_FLD_TAKE(acctinfo_array,PIN_FLD_LASTSTAT_CMNT, 1, ebufp);
	
	//Move the custom fields in the input flist to Cust Credential Flist
	cust_cred_inflist = PIN_FLIST_CREATE(ebufp);
	//Get the Nameinfo Array
	cookie = NULL;
	while ((cust_nameinfo = PIN_FLIST_ELEM_GET_NEXT(i_flistp, 
		PIN_FLD_NAMEINFO, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		temp_flistp = PIN_FLIST_ELEM_ADD(cust_cred_inflist,
			PIN_FLD_NAMEINFO, elem_id, ebufp);
		//Take the custom fields
		PIN_FLIST_FLD_MOVE(cust_nameinfo, 
			MSO_FLD_AREA_NAME, temp_flistp, MSO_FLD_AREA_NAME, ebufp);
		PIN_FLIST_FLD_MOVE(cust_nameinfo, 
			MSO_FLD_LOCATION_NAME, temp_flistp, MSO_FLD_LOCATION_NAME, ebufp);
		PIN_FLIST_FLD_MOVE(cust_nameinfo, 
			MSO_FLD_STREET_NAME, temp_flistp, MSO_FLD_STREET_NAME, ebufp);
		PIN_FLIST_FLD_MOVE(cust_nameinfo, 
			MSO_FLD_BUILDING_NAME, temp_flistp, MSO_FLD_BUILDING_NAME, ebufp);
		PIN_FLIST_FLD_MOVE(cust_nameinfo, 
			MSO_FLD_BUILDING_ID, temp_flistp, MSO_FLD_BUILDING_ID, ebufp);
		PIN_FLIST_FLD_MOVE(cust_nameinfo, 
			MSO_FLD_LANDMARK, temp_flistp, MSO_FLD_LANDMARK, ebufp);	
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Cust Cred Nameinfo", cust_cred_inflist);

	
	/*******************************************************************
	* Call PCM_OP_CUST_CREATE_CUSTOMER  - Start
	*******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CUST_CREATE_CUSTOMER input flist", i_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_CREATE_CUSTOMER, 0, i_flistp, &cc_ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_CREATE_CUSTOMER", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51038", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_CUST_CREATE_CUSTOMER", ebufp);	
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CUST_CREATE_CUSTOMER output flist", cc_ret_flistp);
	/*******************************************************************
	* Call PCM_OP_CUST_CREATE_CUSTOMER  - End
	*******************************************************************/
	
        /*******************************************************************
	* Update wholesale info with own account object into specific field in profile
	*******************************************************************/
	account_obj = PIN_FLIST_FLD_GET(cc_ret_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	profiles_array_flistp = PIN_FLIST_ELEM_GET(cc_ret_flistp, PIN_FLD_PROFILES, PIN_ELEMID_ANY, 0, ebufp);
	if (profiles_array_flistp){
		inherited_flistp = PIN_FLIST_SUBSTR_GET(profiles_array_flistp, 
					PIN_FLD_INHERITED_INFO, 1, ebufp);
		if (inherited_flistp){
			wholesale_flistp = PIN_FLIST_SUBSTR_GET(inherited_flistp,
						 MSO_FLD_WHOLESALE_INFO, 1, ebufp);
			if (wholesale_flistp){
				vp = PIN_FLIST_FLD_GET(wholesale_flistp, MSO_FLD_PP_TYPE, 1, ebufp);
				if (vp)
	   				pp_type = *(int32 *)vp;
			}
		}
	}

	profile_obj = PIN_FLIST_FLD_GET(profiles_array_flistp, PIN_FLD_PROFILE_OBJ, 0, ebufp);

	if (account_type == ACCT_TYPE_HTW || account_type == ACCT_TYPE_DEN || account_type == ACCT_TYPE_JV || 
		account_type == ACCT_TYPE_LCO || account_type == ACCT_TYPE_DTR ||
		account_type == ACCT_TYPE_SUB_DTR || account_type == ACCT_TYPE_DSN || account_type == ACCT_TYPE_DVB)
	{
	
		update_profile_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(update_profile_iflistp, PIN_FLD_POID, profile_obj, ebufp);
		//inherited_info = PIN_FLIST_SUBSTR_ADD(update_profile_iflistp, PIN_FLD_INHERITED_INFO, ebufp );
		profiles_array_flistp = PIN_FLIST_SUBSTR_ADD(update_profile_iflistp, MSO_FLD_WHOLESALE_INFO, ebufp );
		
		if (account_type == ACCT_TYPE_HTW || account_type == ACCT_TYPE_DEN || account_type == ACCT_TYPE_DSN || account_type == ACCT_TYPE_DVB)
		{
			PIN_FLIST_FLD_SET(profiles_array_flistp, MSO_FLD_COMPANY_OBJ, account_obj, ebufp);
		}	
		else if (account_type == ACCT_TYPE_JV )
		{
			PIN_FLIST_FLD_SET(profiles_array_flistp, MSO_FLD_JV_OBJ, account_obj, ebufp);
		}	
		else if (account_type == ACCT_TYPE_LCO)
		{
			PIN_FLIST_FLD_SET(profiles_array_flistp, MSO_FLD_LCO_OBJ, account_obj, ebufp);
			fm_mso_bq_config(ctxp, account_obj,i_flistp, ebufp);
		} 
		else if (account_type == ACCT_TYPE_DTR)
		{
			PIN_FLIST_FLD_SET(profiles_array_flistp, MSO_FLD_DT_OBJ, account_obj, ebufp);
		}
		else if (account_type == ACCT_TYPE_SUB_DTR)
		{
			PIN_FLIST_FLD_SET(profiles_array_flistp, MSO_FLD_SDT_OBJ, account_obj, ebufp);
		}
		//getting and setting the PREF_DOM based on state 
		if (pp_type == 1)	//For SP Customer based on Configuration
		{
			actinfo_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 0, ebufp);
			area = PIN_FLIST_FLD_GET(actinfo_flistp, MSO_FLD_AREA, 1, ebufp);
			fm_mso_get_pref_bdom (ctxp, area, &bdom_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BDOM Output ", bdom_flistp);
		
			if (bdom_flistp)
				vp = PIN_FLIST_FLD_GET(bdom_flistp, MSO_FLD_PREF_DOM, 1, ebufp);			
			if (vp)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Got PREF_DOM");
				pref_dom = *(int32 *)vp;
			}
			else
			{	
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"NO PREF_DOM GOT");
				pin_conf("fm_mso_cust","default_bdom", PIN_FLDT_INT, 
					(caddr_t*) &valpp, &errp);	
				if (valpp)
					pref_dom = *(int32 *)valpp;
				else
					pref_dom = 13;
			}
		}
		else                            //1 for PP Customer
		{
			pref_dom = 1; 
		}
		PIN_FLIST_FLD_SET(profiles_array_flistp, MSO_FLD_PREF_DOM, &pref_dom, ebufp);

		segment_flag = 1;
		PIN_FLIST_FLD_SET(profiles_array_flistp, PIN_FLD_CUSTOMER_SEGMENT, 
						&segment_flag, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist for update wholesale profile ", update_profile_iflistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, update_profile_iflistp, &update_profile_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&update_profile_iflistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist for update wholesale profile", cust_cred_outflist);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating mso_customer_credential object", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating wholesale profile", ebufp);
			PIN_FLIST_DESTROY_EX(&update_profile_oflistp, NULL);
			goto CLEANUP;
		}
		PIN_FLIST_DESTROY_EX(&update_profile_oflistp, NULL);
	}

	/* Update LASTSTAT_CMNT in account object */
	account_obj = PIN_FLIST_FLD_GET(cc_ret_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	if(last_cmnt)
	{
		fm_mso_update_account_info(ctxp, account_obj, last_cmnt, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Updating Last Comment", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51045", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Updating Last Comment", ebufp);	
		goto CLEANUP;
	}
	/*******************************************************************
	* Associate Corporate Child/Parent bill unit and account hierarchy
	* Start
	*******************************************************************
	if ( account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD )
	{
		fm_mso_create_hierarchy(ctxp, cc_ret_flistp, &ret_flistp, ebufp);
	} ************/


	/*******************************************************************
	* Populate the Tax Exemption info for the Account
	*******************************************************************/
	if ( account_type == ACCT_TYPE_RETAIL_BB || account_type == ACCT_TYPE_SME_SUBS_BB ||
		//account_type == ACCT_TYPE_CYBER_CAFE_BB || removed cyber cafe
		account_type == ACCT_TYPE_CORP_SUBS_BB) {
		if(exemption){
			fm_mso_create_tax_exempt(ctxp, account_obj, exemption, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating tax exemption", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51043", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in populating tax exemption info", ebufp);	
				goto CLEANUP;
			}
		}
	}
	if ( account_type == ACCT_TYPE_RETAIL_BB || account_type == ACCT_TYPE_SME_SUBS_BB ||
			//account_type == ACCT_TYPE_CYBER_CAFE_BB || removed cyber cafe
			account_type == ACCT_TYPE_CORP_SUBS_BB) {
		if(payinfo) {
			fm_mso_cust_set_payinfo(ctxp, account_obj, payinfo, ebufp);
			PIN_FLIST_DESTROY_EX(&payinfo, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating additional Payinfo", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51044", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in creating additional Payinfo", ebufp);	
				goto CLEANUP;
			}
		}
		
	
	}
	/*******************************************************************
	* Create the /mso_ar_limit object
	*******************************************************************/
	if ( account_type == ACCT_TYPE_CSR_ADMIN || 
		account_type == ACCT_TYPE_CSR || 
		account_type == ACCT_TYPE_COLLECTION_AGENT )
	{
		if(ar_limit){
			fm_mso_create_ar_limit(ctxp, account_obj, &ar_limit, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating ar limit object", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51044", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in creating ar limit object", ebufp);	
				goto CLEANUP;
			}
		}
	}
	/*******************************************************************
	* Associate Corporate Child/Parent bill unit and account hierarchy
	* Start
	*******************************************************************/
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if ( account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD )
	{
		fm_mso_create_hierarchy(ctxp, cc_ret_flistp, &ret_flistp, ebufp);
	    PIN_FLIST_DESTROY_EX(&corp_parent_info, NULL);
	    PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
   
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Going to change pay_type");

        corp_parent_info = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(corp_parent_info, PIN_FLD_POID, account_obj, ebufp);
        PIN_FLIST_FLD_SET(corp_parent_info, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
        PIN_FLIST_FLD_SET(corp_parent_info, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
        temp_flistp = PIN_FLIST_ELEM_GET(cc_ret_flistp, PIN_FLD_BILLINFO, 0, 0, ebufp);
	    created_billinfo = PIN_FLIST_ELEM_ADD(corp_parent_info, PIN_FLD_BILLINFO, 0, ebufp); 
        PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_POID, created_billinfo, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(created_billinfo, PIN_FLD_PARENT_BILLINFO_OBJ, parent_billinfo_pdp, ebufp); 
        PIN_FLIST_FLD_SET(created_billinfo, PIN_FLD_AR_BILLINFO_OBJ, parent_billinfo_pdp, ebufp); 
        PIN_FLIST_FLD_SET(created_billinfo, PIN_FLD_PAY_TYPE, &child_pay_type, ebufp); 
       
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Change pay type of child account input", corp_parent_info);
 
		PCM_OP(ctxp, PCM_OP_CUST_UPDATE_CUSTOMER, flags, corp_parent_info, &ret_flistp, ebufp);
	    PIN_FLIST_DESTROY_EX(&corp_parent_info, NULL);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Change pay type of child account output", ret_flistp);

	    PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	}
	/*******************************************************************
	* Associate Corporate Child/Parent bill unit and account hierarchy
	* End
	*******************************************************************/

	/*******************************************************************
	* Populate Customer Credential - Start
	*******************************************************************/
	account_obj = PIN_FLIST_FLD_GET(cc_ret_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	//cust_cred_inflist = PIN_FLIST_CREATE(ebufp);
	cust_cred_pdp = PIN_POID_CREATE(db, "/mso_customer_credential", (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(cust_cred_inflist, PIN_FLD_POID, cust_cred_pdp,ebufp);
	PIN_FLIST_FLD_SET(cust_cred_inflist, PIN_FLD_ACCOUNT_OBJ, account_obj,ebufp);
	PIN_FLIST_FLD_SET(cust_cred_inflist, PIN_FLD_LOGIN, (void *)login, ebufp);
	encrypted_pwd =	fm_mso_encrypt_passwd(ctxp, account_obj,pwd, ebufp);
	PIN_FLIST_FLD_PUT(cust_cred_inflist, PIN_FLD_PASSWD, encrypted_pwd, ebufp);
	PIN_FLIST_FLD_SET(cust_cred_inflist, MSO_FLD_ROLES, (void *)PIN_FLIST_FLD_GET(cc_ret_flistp, MSO_FLD_ROLES, 0, ebufp),
		ebufp);
	PIN_FLIST_FLD_SET(cust_cred_inflist, MSO_FLD_DATA_ACCESS, (void *)PIN_FLIST_FLD_GET(cc_ret_flistp, MSO_FLD_DATA_ACCESS, 0, ebufp),
		ebufp);
	PIN_FLIST_FLD_SET(cust_cred_inflist, PIN_FLD_CUSTOMER_TYPE, &account_type, ebufp);
	if (flag_internal_user == 1)
	{
		services_array = PIN_FLIST_ELEM_GET(cc_ret_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
		admin_srvc_pdp = PIN_FLIST_FLD_GET(services_array, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		PIN_FLIST_FLD_SET(cust_cred_inflist, PIN_FLD_USERID, admin_srvc_pdp, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ input flist", cust_cred_inflist);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, cust_cred_inflist, &cust_cred_outflist, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ output flist", cust_cred_outflist);
	PIN_FLIST_DESTROY_EX(&cust_cred_inflist, NULL);
	/********************************************************* 
	 * Errors..?
	 *********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating mso_customer_credential object", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51039", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in creating mso_customer_credential object", ebufp);	
		goto CLEANUP;
	}
	/*******************************************************************
	* Populate Customer Credential - End
	*******************************************************************/

	/*******************************************************************
	* Assign Role to Customer - Start
	*******************************************************************/
	if (assign_pricing_role)
	{
		modify_service_input = 	PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(modify_service_input, PIN_FLD_POID, admin_srvc_pdp, ebufp );
		inherited_info = PIN_FLIST_SUBSTR_ADD(modify_service_input, PIN_FLD_INHERITED_INFO, ebufp );
		role_array =  PIN_FLIST_ELEM_ADD(inherited_info, PIN_FLD_ROLE, 0, ebufp );
		PIN_FLIST_FLD_SET(role_array, PIN_FLD_APPLICATION, "Pricing Center", ebufp );
		PIN_FLIST_FLD_PUT(role_array, PIN_FLD_PROFILE_OBJ, role_service_pdp , ebufp );
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "assigning pricing role", modify_service_input);
		PCM_OP(ctxp, PCM_OP_CUST_MODIFY_SERVICE, 0, modify_service_input, &modify_service_output, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in assigning pricing role", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51040", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in assigning pricing role", ebufp);
			goto CLEANUP;
		}
  	}
 	/*******************************************************************
	* Assign Role to Customer - End
	*******************************************************************/

	/*******************************************************************
	* Modify Account Number - Start
	*******************************************************************/

	//USE PIN_POID_TO_STR  to avoid large size poids
	

/* Account_poid and account no are equal

	update_acnt_no_iflist = PIN_FLIST_CREATE(ebufp);
	account_no_int = PIN_POID_GET_ID(account_obj);
	sprintf(account_no_str, "%d", account_no_int );
	PIN_FLIST_FLD_SET(update_acnt_no_iflist, PIN_FLD_POID, account_obj, ebufp );
	PIN_FLIST_FLD_SET(update_acnt_no_iflist, PIN_FLD_ACCOUNT_NO, account_no_str, ebufp);
	
	fm_mso_update_account_number(ctxp, update_acnt_no_iflist, ebufp );
	PIN_FLIST_DESTROY_EX(&update_acnt_no_iflist, NULL);
*/	

/*Account no = CR-XXX*/

	fm_mso_get_account_info(ctxp, account_obj, &update_acnt_no_iflist, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_acnt_no_iflist", update_acnt_no_iflist);
	vp = (char* )PIN_FLIST_FLD_GET(update_acnt_no_iflist, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	memset(account_no_str, '\0', sizeof(account_no_str));
	sprintf(account_no_str,"%s", vp);
	PIN_FLIST_DESTROY_EX(&update_acnt_no_iflist, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
	
	
	/*******************************************************************
	* Modify Account Number - End
	*******************************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in updating account number", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51041", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating account number", ebufp);	
		goto CLEANUP;
	}
	else
	{
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_success, ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LOGIN, login, ebufp);
//		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "ACCOUNT Registration done successfully", ebufp);
	}
	
	/***************************
	* Creating GST Profile
	***************************/
	elem_id = 0;
	cookie = NULL;
	while((gst_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, MSO_FLD_GST_INFO, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		gst_info_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, gst_info_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, gst_info_in_flistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, gst_info_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_SET(gst_info_in_flistp, PIN_FLD_ACCOUNT_NO, account_no_str, ebufp);
		PIN_FLIST_ELEM_SET(gst_info_in_flistp, gst_flistp, MSO_FLD_GST_INFO, elem_id, ebufp);
//		PIN_FLIST_ELEM_COPY(i_flistp, MSO_FLD_GST_INFO, PIN_ELEMID_ANY, gst_info_in_flistp, MSO_FLD_GST_INFO, 0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Gst Profile Input Flist", gst_info_in_flistp);
		PCM_OP(ctxp, MSO_OP_CUST_UPDATE_GST_INFO, 0, gst_info_in_flistp, &gst_info_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&gst_info_in_flistp, NULL);
       		if (PIN_ERRBUF_IS_ERR(ebufp))
       	 	{
			PIN_ERRBUF_RESET(ebufp);
			gst_profile_success = 0;
        	}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Gst Profile Output Flist", gst_info_out_flistp);
		gst_status = PIN_FLIST_FLD_GET(gst_info_out_flistp, PIN_FLD_STATUS, 1, ebufp);
		if (!gst_status || (gst_status && *gst_status != 0))
		{
			gst_profile_success = 0;
		}

		if (gst_profile_success != 1)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Creating GST Profile", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        ret_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51099", ebufp);
                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Creating GST Profile", ebufp);
			PIN_FLIST_DESTROY_EX(&gst_info_out_flistp, NULL);
                        goto CLEANUP;
		}
		PIN_FLIST_DESTROY_EX(&gst_info_out_flistp, NULL);
	}

	/* Generate Lifecycle Event*/
	//fm_mso_cust_reg_lc_event(ctxp, account_obj, account_no_str, prog_name, last_cmnt, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Generating lifecycle event", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51081", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Generating lifecycle event", ebufp);
		goto CLEANUP;
	}

	/*******************************************************************
	* Create Entry in profile_cust_offer_t when Offer is Present in IP
	*******************************************************************/
	if (!PIN_ERRBUF_IS_ERR(ebufp))
	{
		elem_id=0; 
		cookie= NULL;
		char *ofr_desc= NULL;
		char pai[20]="";
		char *act_no;
		while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, MSO_FLD_REGISTER_CUSTOMER,&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			profile_offer_flistp = PIN_FLIST_CREATE(ebufp);
			srch_login_pdp = PIN_POID_CREATE(db, "/profile_cust_offer", (int64)-1, ebufp);
			PIN_FLIST_FLD_PUT(profile_offer_flistp, PIN_FLD_POID, srch_login_pdp, ebufp );
			PIN_FLIST_FLD_SET(profile_offer_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp );
			PIN_FLIST_FLD_SET(profile_offer_flistp, MSO_FLD_OFFER_DESCR, PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_DESCR, 1, ebufp), ebufp );
			PIN_FLIST_FLD_SET(profile_offer_flistp, MSO_FLD_OFFER_VALUE, PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_VALUE, 1, ebufp), ebufp );
			ofr_desc=PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_DESCR, 1, ebufp);
			if(ofr_desc && (strstr(ofr_desc,"Netflix") ||strstr(ofr_desc,"NETFLIX")))
			{
				rdfld_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(rdfld_flistp, PIN_FLD_POID, account_obj, ebufp );	
				PIN_FLIST_FLD_SET(rdfld_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp );
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist for profile_cust_offer creation", rdfld_flistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rdfld_flistp, &rdfld_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist for profile_cust_offer creation", rdfld_oflistp);
				act_no=PIN_FLIST_FLD_GET(rdfld_oflistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
				sprintf(pai,"%s%s","NF",PIN_FLIST_FLD_GET(rdfld_oflistp, PIN_FLD_ACCOUNT_NO, 1, ebufp));	
				PIN_FLIST_FLD_SET(profile_offer_flistp, MSO_FLD_PAI, pai, ebufp );
				PIN_FLIST_DESTROY_EX(&rdfld_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&rdfld_oflistp, NULL);
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ input flist for profile_cust_offer creation", profile_offer_flistp);
			if(PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_VALUE, 1, ebufp) ==(int *)1)
			{
			  PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, profile_offer_flistp, &profile_offer_oflistp, ebufp);
			  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ output flist for profile_cust_offer creation", profile_offer_oflistp);
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ input flist for profile_cust_offer creation", profile_offer_flistp);
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, profile_offer_flistp, &profile_offer_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ output flist for profile_cust_offer creation", profile_offer_oflistp);

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Generating Profile_Cust_offer Obj", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, routing_poid, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51881", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Generating Profile_Cust_offer Obj", ebufp);
				goto CLEANUP;
			}
			PIN_FLIST_DESTROY_EX(&profile_offer_flistp, NULL);
		}
	   	PIN_FLIST_DESTROY_EX(&profile_offer_oflistp, NULL);
	}

	/*******************************************************************
	* Create Output flist - Start
		0 PIN_FLD_POID             POID [0] 0.0.0.0 0 0                   
		0 PIN_FLD_USERID           POID [0] 0.0.0.1 /account 786433 0     
		0 PIN_FLD_ACCOUNT_OBJ      POID [0] 0.0.0.1 /account 67033 0      
		0 PIN_FLD_FLAGS             INT [0] 0                             
	*******************************************************************/

	/*******************************************************************
	* Create Output flist - Start
	*******************************************************************/
	create_out_flist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(create_out_flist, PIN_FLD_POID, account_obj, ebufp );
	PIN_FLIST_FLD_SET(create_out_flist, PIN_FLD_ACCOUNT_NO, account_no_str, ebufp );
	PIN_FLIST_FLD_SET(create_out_flist, PIN_FLD_STATUS, &register_customer_success, ebufp );
	PIN_FLIST_FLD_SET(create_out_flist, PIN_FLD_BUSINESS_TYPE, &tmp_business_type, ebufp );
	PIN_FLIST_FLD_SET(create_out_flist, PIN_FLD_DESCR, "ACCOUNT Registration done successfully", ebufp );
	PIN_FLIST_FLD_SET(create_out_flist, PIN_FLD_LOGIN, login, ebufp );

	created_billinfo = PIN_FLIST_ELEM_ADD(create_out_flist, PIN_FLD_BILLINFO, 0, ebufp); 
	PIN_FLIST_FLD_SET(created_billinfo, PIN_FLD_ACTG_CYCLE_DOM, PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PREF_DOM, 0, ebufp), ebufp );
	PIN_FLIST_FLD_SET(created_billinfo, PIN_FLD_BILL_WHEN, &account_model, ebufp );

	current_t = pin_virtual_time(NULL);
	current_t = fm_utils_time_round_to_midnight(current_t);
	next_t	=  current_t + 30*86400;
	future_t = next_t + 30*86400; 

	PIN_FLIST_FLD_SET(created_billinfo, PIN_FLD_ACTG_LAST_T, &current_t, ebufp );
	PIN_FLIST_FLD_SET(created_billinfo, PIN_FLD_ACTG_NEXT_T, &next_t, ebufp );
	PIN_FLIST_FLD_SET(created_billinfo, PIN_FLD_ACTG_FUTURE_T, &future_t, ebufp );

	if (group_info)
	{
		PIN_FLIST_SUBSTR_SET(create_out_flist, group_info, PIN_FLD_GROUP_INFO, ebufp);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "create_out_flist", ebufp);
		fm_mso_get_err_desc(ctxp, routing_poid, &ret_flistp, ebufp);
		goto CLEANUP;
	}
	else
	{
		ret_flistp = create_out_flist;	
	}

	fm_mso_create_lifecycle_evt(ctxp, lc_in_flist, ret_flistp, ID_REGISTER_CUSTOMER, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}

/*
	get_cust_info_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(get_cust_info_iflist, PIN_FLD_POID, account_obj, ebufp);
	PIN_FLIST_FLD_SET(get_cust_info_iflist, PIN_FLD_USERID, account_obj, ebufp);
	PIN_FLIST_FLD_SET(get_cust_info_iflist, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
	PIN_FLIST_FLD_SET(get_cust_info_iflist, PIN_FLD_FLAGS, &register_customer_success, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Call get_customer_info after registration", get_cust_info_iflist);
	PCM_OP(ctxp, MSO_OP_CUST_GET_CUSTOMER_INFO, 0, get_cust_info_iflist, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&get_cust_info_iflist, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, account_obj, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &register_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error calling get_customer_info after registration:", ebufp);
		goto CLEANUP;
	}
*/



	/*******************************************************************
	* Create Output flist - End
	*******************************************************************/
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&cc_ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&cust_cred_outflist, NULL);
	PIN_FLIST_DESTROY_EX(&srch_role_outflist, NULL); 
	PIN_FLIST_DESTROY_EX(&corp_parent_info, NULL);
	PIN_FLIST_DESTROY_EX(&bdom_flistp, NULL);

	if (role_service_pdp)
	{
		PIN_POID_DESTROY(role_service_pdp, ebufp);
	}
	*r_flistpp = ret_flistp;
	return;
}



/**************************************************
* Function: fm_mso_validate_pricing_role()
* 
* 
***************************************************/
static void 
fm_mso_validate_pricing_role(
	pcm_context_t		*ctxp,
	int64			db,
	char			*roles,
	pin_flist_t		**srch_role_outflist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;

	poid_t			*srch_pdp = NULL;

	int32			srch_flag = 256;

	char			*template = "select X from /service where  F1 = V1 and F2.type = V2 and F3 = V3 ";
	char			pricing_admin_login[200];

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_pricing_role :");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, roles);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	//pricing_admin_login = pin_malloc(200);
	memset(pricing_admin_login,'\0',sizeof(pricing_admin_login));
	strcpy(pricing_admin_login, "ROLE-<Pricing Center>");
	strcat(pricing_admin_login, roles);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_LOGIN, pricing_admin_login, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/admin_client", -1, ebufp), ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_NAME, "Pricing Center", ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LOGIN, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_pricing_role SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH: fm_mso_validate_pricing_role()", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_pricing_role SEARCH output flist", srch_out_flistp);


	*srch_role_outflist = srch_out_flistp;
	return;
}

/**************************************************
* Function: fm_mso_update_org_structure()
* 
* 
***************************************************/
static void 
fm_mso_update_org_structure(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			account_type,
	int32			corporate_type,
	int32			subscriber_type,
	pin_flist_t		**o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*profile_array = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*wholesale_info = NULL;
	pin_flist_t		*cc_info = NULL;
	pin_flist_t		*parent_wholesale_info = NULL; 
	pin_flist_t		*ret_flistp = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*immediate_parent_pdp = NULL;
	poid_t			*sdt_obj = NULL;

	poid_t			*dt_obj = NULL;
	poid_t			*jv_obj = NULL;
	poid_t			*company_obj = NULL;

	int32			srch_flag = 256;
	int32			parent_account_type = -1;
	int32			*pp_type = NULL;
	int32			*delivery_pref = NULL;
	char			*das_type = NULL;

	char			*template = "select X from /profile where  F1 = V1 and F2.type = V2 ";
	int64			db = -1;
	int64			parent_pdp_id = -1;
	int32			*pref_dom = NULL;
    time_t          *lco_agr_start_t= NULL;
    time_t          *lco_agr_end_t= NULL;
    time_t          *lco_renew_start_t = NULL;
    time_t          *lco_renew_end_t= NULL;
    time_t          *lco_dtp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_org_structure :");

	profile_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PROFILES, 0, 1, ebufp );
	if (profile_array)
	{
		inherited_info = PIN_FLIST_SUBSTR_GET(profile_array, PIN_FLD_INHERITED_INFO, 1, ebufp);
		if (inherited_info)
		{
			if ( account_type == ACCT_TYPE_SUBSCRIBER || account_type == ACCT_TYPE_RETAIL_BB || 
				account_type == ACCT_TYPE_SME_SUBS_BB || 
				//account_type == ACCT_TYPE_CYBER_CAFE_BB || removed account type
				account_type == ACCT_TYPE_CORP_SUBS_BB)
			{
			 	cc_info = PIN_FLIST_ELEM_GET(inherited_info, PIN_FLD_CUSTOMER_CARE_INFO, 0, 1, ebufp );
				if (!cc_info)
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing profiles.inherited_info.PIN_FLD_CUSTOMER_CARE_INFO", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51019", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing profiles.inherited_info.PIN_FLD_CUSTOMER_CARE_INFO", ebufp);
					goto CLEANUP;
				}
				immediate_parent_pdp = PIN_FLIST_FLD_GET(cc_info, PIN_FLD_PARENT, 1, ebufp);
				parent_pdp_id = PIN_POID_GET_ID(immediate_parent_pdp);
			}
			else
			{
				wholesale_info = PIN_FLIST_SUBSTR_GET(inherited_info, MSO_FLD_WHOLESALE_INFO, 1, ebufp );
				if (!wholesale_info)
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing profiles.inherited_info.MSO_FLD_WHOLESALE_INFO", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51020", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing profiles.inherited_info.MSO_FLD_WHOLESALE_INFO", ebufp);
					goto CLEANUP;
				}
				immediate_parent_pdp = PIN_FLIST_FLD_GET(wholesale_info, PIN_FLD_PARENT, 1, ebufp);
				parent_pdp_id = PIN_POID_GET_ID(immediate_parent_pdp);
			}
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing profiles.PIN_FLD_INHERITED_INFO", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51021", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing profiles.PIN_FLD_INHERITED_INFO", ebufp);
			goto CLEANUP;
		}
        if(profile_array && inherited_info && wholesale_info)
        {
            lco_agr_start_t   = PIN_FLIST_FLD_GET(wholesale_info, PIN_FLD_CYCLE_START_T, 1, ebufp);
            lco_agr_end_t     = PIN_FLIST_FLD_GET(wholesale_info, PIN_FLD_CYCLE_END_T, 1, ebufp);
            lco_renew_start_t = PIN_FLIST_FLD_GET(wholesale_info, PIN_FLD_USAGE_START_T, 1, ebufp);
            lco_renew_end_t   = PIN_FLIST_FLD_GET(wholesale_info, PIN_FLD_USAGE_END_T, 1, ebufp);

            if(lco_agr_start_t == NULL)
            {
                PIN_FLIST_FLD_SET(wholesale_info, PIN_FLD_CYCLE_START_T, lco_dtp,ebufp);
            }
            if(lco_agr_end_t == NULL)
            {
                PIN_FLIST_FLD_SET(wholesale_info, PIN_FLD_CYCLE_END_T, lco_dtp,ebufp);
            }
            if(lco_renew_start_t == NULL)
            {
                PIN_FLIST_FLD_SET(wholesale_info, PIN_FLD_USAGE_START_T, lco_dtp,ebufp);
            }
            if(lco_renew_end_t == NULL)
            {
                PIN_FLIST_FLD_SET(wholesale_info, PIN_FLD_USAGE_END_T, lco_dtp,ebufp);
            }

        }
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing PIN_FLD_PROFILES", ebufp);
		PIN_ERRBUF_RESET(ebufp);
 		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51022", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing PIN_FLD_PROFILES", ebufp);
		goto CLEANUP;
	}
	/************************************************************************
	* case:1 Immediate Parent not passed
	************************************************************************/
	if (parent_pdp_id == 0) 
	{
		/****************************************************
		* Subscriber should not have parent_pdp_id==0
		****************************************************/
		if (account_type == ACCT_TYPE_SUBSCRIBER || account_type == ACCT_TYPE_RETAIL_BB || 
				account_type == ACCT_TYPE_SME_SUBS_BB || 
				//account_type == ACCT_TYPE_CYBER_CAFE_BB || removed cyber cafe
				account_type == ACCT_TYPE_CORP_SUBS_BB)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Subscriber cannot be registered without LCO", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51023", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Subscriber cannot be registered without LCO", ebufp);
			goto CLEANUP;
		}
		/****************************************************
		* HTW or JV only can have parent_pdp_id==0
		****************************************************/
		if (account_type == ACCT_TYPE_HTW || account_type == ACCT_TYPE_DEN || account_type == ACCT_TYPE_DSN 
                || account_type == ACCT_TYPE_CSR_ADMIN || account_type == ACCT_TYPE_DVB)
		{
			PIN_FLIST_FLD_SET(wholesale_info, PIN_FLD_CUSTOMER_TYPE, &account_type, ebufp);
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Null value PIN_FLD_PARENT in organisation structure", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51024", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Null value PIN_FLD_PARENT in organisation structure", ebufp);
			goto CLEANUP;
		}
	}
	else
	{
	/************************************************************************
	* case:2 Immediate Parent passed
	************************************************************************/
		
		/**********************************************************************
		* Search profile of immediate parent to update SUB_DTR, DTR, JV in 
		* input flist
		***********************************************************************/
		db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp)) ;

		srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, immediate_parent_pdp, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/wholesale", -1, ebufp), ebufp);

		result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_org_structure SEARCH input list", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH: fm_mso_update_org_structure()", ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_org_structure SEARCH output flist", srch_out_flistp);

		result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if (result_flist)
		{
			parent_wholesale_info = PIN_FLIST_SUBSTR_GET(result_flist, MSO_FLD_WHOLESALE_INFO, 0, ebufp );
			sdt_obj = PIN_FLIST_FLD_GET(parent_wholesale_info, MSO_FLD_SDT_OBJ, 0, ebufp) ;
			dt_obj = PIN_FLIST_FLD_GET(parent_wholesale_info, MSO_FLD_DT_OBJ, 0, ebufp) ; 
			jv_obj = PIN_FLIST_FLD_GET(parent_wholesale_info, MSO_FLD_JV_OBJ, 0, ebufp) ; 
			company_obj = PIN_FLIST_FLD_GET(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, 1, ebufp) ; 
			pp_type = PIN_FLIST_FLD_GET(parent_wholesale_info, MSO_FLD_PP_TYPE, 0, ebufp) ;
			das_type = PIN_FLIST_FLD_GET(parent_wholesale_info, MSO_FLD_DAS_TYPE, 0, ebufp) ;
			parent_account_type = *(int32*)PIN_FLIST_FLD_GET(parent_wholesale_info, PIN_FLD_CUSTOMER_TYPE, 0, ebufp) ;

			delivery_pref = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DELIVERY_PREFER, 1, ebufp );
			if (!delivery_pref)
			{
				PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_PP_TYPE, i_flistp, PIN_FLD_DELIVERY_PREFER, ebufp);
			}
			PIN_FLIST_SUBSTR_SET(i_flistp, parent_wholesale_info, MSO_FLD_WHOLESALE_INFO, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "i_flistp", i_flistp);

			/**********************************************************************
			* Validate Hierarchy
			**********************************************************************/
			pref_dom = PIN_FLIST_FLD_GET(parent_wholesale_info, MSO_FLD_PREF_DOM, 1, ebufp) ;
			if (!pref_dom)
			{
			
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Pref BDOM is Mandatory but not passed", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51030", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Pref BDOM is Mandatory but not passed", ebufp);
				goto CLEANUP;
			}
			PIN_FLIST_FLD_SET(parent_wholesale_info, MSO_FLD_PREF_DOM, pref_dom, ebufp);

			PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_PREF_DOM, i_flistp, MSO_FLD_PREF_DOM, ebufp );
		
			if (account_type == ACCT_TYPE_JV)
			{
				PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
			}
			if (account_type == ACCT_TYPE_DTR ) // case 1: Distributor under JV 
			{
				if ( parent_account_type == ACCT_TYPE_JV)
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, immediate_parent_pdp, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
				}
				else
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Account Type 'Distributor' must have immediate parent as 'JV'", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51025", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account Type 'Distributor' must have immediate parent as 'JV' or 'SP'", ebufp);
					goto CLEANUP;
				}
			}
			if (account_type == ACCT_TYPE_SUB_DTR  )  // case 2: Sub-Distributor under Distributor 
			{
				if ( parent_account_type == ACCT_TYPE_DTR || parent_account_type == ACCT_TYPE_LOGISTICS )
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, immediate_parent_pdp, ebufp);
				}
				else
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Account Type 'Sub-Distributor' must have immediate parent as 'Distributor'", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51026", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account Type 'Sub-Distributor' must have immediate parent as 'Distributor'", ebufp);
					goto CLEANUP;
				}
			}
			if (account_type == ACCT_TYPE_LCO) 
			{	
				if ( parent_account_type == ACCT_TYPE_DEN || parent_account_type == ACCT_TYPE_DSN 
                        || parent_account_type == ACCT_TYPE_HTW || parent_account_type == ACCT_TYPE_DVB)  // case 3.1: LCO under HTW/JV 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_COMPANY_OBJ, immediate_parent_pdp, ebufp);
				}
				else if ( parent_account_type == ACCT_TYPE_JV )
				{
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, immediate_parent_pdp, ebufp);
				}
				else if ( parent_account_type == ACCT_TYPE_DTR )  // case 3.2: LCO under Distributor 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, immediate_parent_pdp, ebufp);	
				}
				else if ( parent_account_type == ACCT_TYPE_SUB_DTR ) // case 3.3: LCO under Sub-Distributor 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, dt_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_SDT_OBJ, immediate_parent_pdp, ebufp);
				}
				else
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Account Type 'LCO' must have immediate parent as 'Distributor' or 'Sub-Distributor' or 'JV' or 'Operator'", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51027", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account Type 'LCO' must have immediate parent as 'Distributor' or 'Sub-Distributor' or 'JV' or 'Operator'", ebufp);
					goto CLEANUP;
				}
			}
			if (account_type == ACCT_TYPE_CSR || account_type == ACCT_TYPE_CSR_ADMIN
				|| account_type == ACCT_TYPE_FIELD_SERVICE || account_type == ACCT_TYPE_COLLECTION_AGENT
				|| account_type == ACCT_TYPE_SALES_PERSON || account_type == ACCT_TYPE_PRE_SALES)
			{
				if ( parent_account_type == ACCT_TYPE_DEN || parent_account_type == ACCT_TYPE_HTW ||
                        parent_account_type == ACCT_TYPE_DSN || parent_account_type == ACCT_TYPE_DVB)  // case 4.1: CSR under HTW/JV 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_COMPANY_OBJ, immediate_parent_pdp, ebufp);
				}
				else if (parent_account_type == ACCT_TYPE_JV)
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, immediate_parent_pdp, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
				}
				else if ( parent_account_type == ACCT_TYPE_DTR || parent_account_type == ACCT_TYPE_LOGISTICS )  // case 4.2: CSR under Distributor 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, immediate_parent_pdp, ebufp);	
				}
				else if ( parent_account_type == ACCT_TYPE_SUB_DTR ) // case 4.3: CSR under Sub-Distributor 
				{
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, dt_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_SDT_OBJ, immediate_parent_pdp, ebufp);
				}
				else if ( parent_account_type == ACCT_TYPE_LCO ) // case 4.4: CSR under LCO
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, dt_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_SDT_OBJ, sdt_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_LCO_OBJ, immediate_parent_pdp, ebufp);
					//Changes Added to Keep the LCO and CSR PP_TYPE same
					if  (pp_type)
					 PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_PP_TYPE, pp_type, 
								ebufp);
				}
				else
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "CSR Account Creation Error", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51028", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CSR Account Creation Error", ebufp);
					goto CLEANUP;
				}
			}
			if (account_type == ACCT_TYPE_LOGISTICS )
			{
				if (parent_account_type == ACCT_TYPE_DEN || parent_account_type == ACCT_TYPE_HTW ||
                        parent_account_type == ACCT_TYPE_DSN || parent_account_type == ACCT_TYPE_DVB)  // case 4.1: Warehouse under HTW/JV 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_COMPANY_OBJ, immediate_parent_pdp, ebufp);
				}
				else if (parent_account_type == ACCT_TYPE_JV)
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, immediate_parent_pdp, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
				}
				else if ( parent_account_type == ACCT_TYPE_DTR || parent_account_type == ACCT_TYPE_LOGISTICS )  // case 4.2: Warehouse under Distributor 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, immediate_parent_pdp, ebufp);	
				}
				else if ( parent_account_type == ACCT_TYPE_SUB_DTR ) // case 4.3: Warehouse under Sub-Distributor 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, dt_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_SDT_OBJ, immediate_parent_pdp, ebufp);
				}
				else if ( parent_account_type == ACCT_TYPE_LCO ) // case 4.4: Warehouse under LCO
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, dt_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_SDT_OBJ, sdt_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_LCO_OBJ, immediate_parent_pdp, ebufp);
				}
				else
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Warehouse Account Creation Error", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51029", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Warehouse Account Creation Error", ebufp);
					goto CLEANUP;
				}
			}
			if (account_type == ACCT_TYPE_OPERATION )
			{
				if ( parent_account_type == ACCT_TYPE_DEN || parent_account_type == ACCT_TYPE_HTW ||
                        parent_account_type == ACCT_TYPE_DSN || parent_account_type == ACCT_TYPE_DVB)  // case 4.1: Ops under HTW/JV 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_COMPANY_OBJ, immediate_parent_pdp, ebufp);
				}
				else if (parent_account_type == ACCT_TYPE_JV)
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, immediate_parent_pdp, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
				}
				else if ( parent_account_type == ACCT_TYPE_DTR || parent_account_type == ACCT_TYPE_LOGISTICS )  // case 4.2: Ops under Distributor 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, immediate_parent_pdp, ebufp);	
				}
				else if ( parent_account_type == ACCT_TYPE_SUB_DTR ) // case 4.3: Ops under Sub-Distributor 
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, dt_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_SDT_OBJ, immediate_parent_pdp, ebufp);
				}
				else if ( parent_account_type == ACCT_TYPE_LCO ) // case 4.4: Ops under LCO
				{
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
					PIN_FLIST_FLD_COPY(parent_wholesale_info, MSO_FLD_COMPANY_OBJ, wholesale_info, MSO_FLD_COMPANY_OBJ, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_DT_OBJ, dt_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_SDT_OBJ, sdt_obj, ebufp);
					PIN_FLIST_FLD_SET(wholesale_info, MSO_FLD_LCO_OBJ, immediate_parent_pdp, ebufp);
				}
				else
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Operations Account Creation Error", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51030", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Operations Account Creation Error", ebufp);
					goto CLEANUP;
				}
			}
			/****************************************
			*  For End Customer
			****************************************/
			if (account_type == ACCT_TYPE_SUBSCRIBER || account_type == ACCT_TYPE_RETAIL_BB || 
				account_type == ACCT_TYPE_SME_SUBS_BB || 
				//account_type == ACCT_TYPE_CYBER_CAFE_BB ||  removed cyber cafe
				account_type == ACCT_TYPE_CORP_SUBS_BB)
			{
				PIN_FLIST_FLD_SET(cc_info, MSO_FLD_JV_OBJ, jv_obj, ebufp);
				PIN_FLIST_FLD_SET(cc_info, MSO_FLD_DT_OBJ, dt_obj, ebufp);
				PIN_FLIST_FLD_SET(cc_info, MSO_FLD_SDT_OBJ, sdt_obj, ebufp);
				PIN_FLIST_FLD_SET(cc_info, MSO_FLD_PP_TYPE, pp_type, ebufp); 
				PIN_FLIST_FLD_SET(cc_info, MSO_FLD_LCO_OBJ, immediate_parent_pdp, ebufp);
				PIN_FLIST_FLD_SET(cc_info, PIN_FLD_CUSTOMER_TYPE, &subscriber_type, ebufp);
                PIN_FLIST_FLD_SET(cc_info, MSO_FLD_COMPANY_OBJ, company_obj, ebufp);
				if (account_type == ACCT_TYPE_SUBSCRIBER)
				{
					PIN_FLIST_FLD_SET(cc_info, MSO_FLD_DAS_TYPE, das_type, ebufp); 
				}

			}
			else
			{
				PIN_FLIST_FLD_SET(wholesale_info, PIN_FLD_CUSTOMER_TYPE, &account_type, ebufp);
			}
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Invalid PIN_FLD_PARENT passed in profile", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51031", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid PIN_FLD_PARENT passed in profile", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_org_structure wholesale_info:", wholesale_info);
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	*o_flistp = ret_flistp;
	return;
}

/**************************************************
* Function: fm_mso_update_account_number()
* 
* 
***************************************************/
static void 
fm_mso_update_account_number(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp)
{
 	
	pin_flist_t		*o_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_account_number  input flist :", i_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, i_flistp, &o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_update_account_number", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_org_structure SEARCH output flist", o_flistp);
	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
	return;
 }


/**************************************************
* Function: fm_mso_populate_tax_info()
* 
* 
***************************************************/
static void 
fm_mso_populate_tax_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			account_model,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*acctinfo = NULL;
	pin_flist_t		*profiles = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*et_zones  = NULL;
	pin_flist_t		*vat_zones = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*cc_info = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*code_info = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*srch_pdp_1 = NULL;

	int32			srch_flag = 512;
	int32			parent_account_type = -1;
	int32			*pp_type = NULL;
	int32			et_zone_elem_id = -1;

	char			*state = NULL;
	char			*city = NULL;
	char			*et_zone_code = NULL;
	char			*vat_zone_code = NULL;
	char			*state_city_area_code = NULL;

	char			*template_state = "select X from /mso_cfg_state_tax_zones where  F1 = V1 ";
	char			*template_city =  "select X from /mso_cfg_city_tax_zones  where  F1 = V1 and F2 = V2 ";
	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error before fm_mso_populate_tax_info :");
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info :");

	/**********************************************************************
	* Parse Input Flist to get state and city from Installation address
	* Start
	**********************************************************************/
	acctinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 0, ebufp);
	state_city_area_code = PIN_FLIST_FLD_GET(acctinfo, MSO_FLD_AREA, 1, ebufp);
	
	fm_mso_get_state_city_area_code(ctxp, state_city_area_code, NULL, &code_info, ebufp );
	
	if (code_info && (PIN_FLIST_FLD_GET(code_info, PIN_FLD_ERROR_CODE, 1, ebufp)))
	{
		*ret_flistp = PIN_FLIST_COPY(code_info, ebufp);
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state_city_area_code", code_info);
	state = (char*)PIN_FLIST_FLD_GET(code_info, PIN_FLD_STATE, 1, ebufp);  //State code
	city = (char*)PIN_FLIST_FLD_GET(code_info, MSO_FLD_CITY_CODE, 1, ebufp);
	PIN_FLIST_FLD_COPY(code_info, MSO_FLD_CITY_CODE, i_flistp, MSO_FLD_CITY_CODE, ebufp );

	/**********************************************************************
	* Parse Input Flist to get state and city from Installation address
	* End
	**********************************************************************/

	/**********************************************************************
	* Parse Input Flist to populate element id for MSO_FLD_ET_ZONES array
	* Start
	**********************************************************************/
	profiles = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PROFILES, 0, 1, ebufp);
	if (!profiles)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No PIN_FLD_PROFILES for this account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51032", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No PIN_FLD_PROFILES for this account", ebufp);
		goto CLEANUP;
	}

	inherited_info = PIN_FLIST_SUBSTR_GET(profiles, PIN_FLD_INHERITED_INFO, 1, ebufp);
	if (!inherited_info)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No profiles.PIN_FLD_INHERITED_INFO for this account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51033", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No profiles.PIN_FLD_INHERITED_INFO for this account", ebufp);
		goto CLEANUP;
	}

	cc_info = PIN_FLIST_SUBSTR_GET(inherited_info, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
	if (!cc_info)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No profiles.inherited_info.PIN_FLD_CUSTOMER_CARE_INFO for this account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51034", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No profiles.inherited_info.PIN_FLD_CUSTOMER_CARE_INFO for this account", ebufp);
		goto CLEANUP;
	}

	pp_type = (int32 *)PIN_FLIST_FLD_GET(cc_info, MSO_FLD_PP_TYPE, 0, ebufp );

	/**********************************************************************
	* Populate element id for MSO_FLD_ET_ZONES array as:
	*    ELEM_ID   MEANING
	*    -------   -------
	*    00        pp & residential
	*    01        pp & Commercial
	*    10        sp & residential
	*    11        sp & Commercial
	**********************************************************************/
	if (account_model == ACCOUNT_MODEL_RES  )
	{
		et_zone_elem_id = (*pp_type)*10 + 0;
	}
	else if (account_model == ACCOUNT_MODEL_CORP || account_model == ACCOUNT_MODEL_MDU  )
	{
		et_zone_elem_id = (*pp_type)*10 + 1;
	}
	/**********************************************************************
	* Parse Input Flist to populate element id for MSO_FLD_ET_ZONES array
	* Start
	**********************************************************************/

	/**********************************************************************
	* Search for  State Level Tax
	**********************************************************************/
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp)) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_state , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE, state, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info SEARCH input flist:STATE", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH: fm_mso_populate_tax_info()", ebufp);
		goto CLEANUP;;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info SEARCH output flist", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (!result_flist)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"No Tax configured for this state", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51035", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No Tax configured for this state", ebufp);
		goto CLEANUP;
	}
	et_zones =  PIN_FLIST_ELEM_GET(result_flist, MSO_FLD_ET_ZONES, et_zone_elem_id, 1, ebufp);
	vat_zones =  PIN_FLIST_ELEM_GET(result_flist, MSO_FLD_VAT_ZONES, PIN_ELEMID_ANY, 1, ebufp);

	if (et_zones && vat_zones)
	{
		et_zone_code = PIN_FLIST_FLD_TAKE(et_zones, MSO_FLD_ET_ZONE_CODE, 0, ebufp );
		vat_zone_code = PIN_FLIST_FLD_TAKE(vat_zones, MSO_FLD_VAT_ZONE_CODE, 0, ebufp );
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No MSO_FLD_ET_ZONE_CODE/MSO_FLD_VAT_ZONE_CODE", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51036", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No MSO_FLD_ET_ZONE_CODE/MSO_FLD_VAT_ZONE_CODE", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "et_zones", et_zones);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "vat_zones", vat_zones);

	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	/**********************************************************************
	* Search for City Level Tax
	**********************************************************************/

	//PIN_FLIST_FLD_DROP(srch_flistp, PIN_FLD_TEMPLATE, ebufp );

	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_city , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CITY, city, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info SEARCH input flist: CITY", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH 1: fm_mso_populate_tax_info()", ebufp);
		goto CLEANUP;;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_tax_info SEARCH output flist:CITY", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
//	if (!result_flist)
//	{
//		*ret_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "No Tax configured for this city", ebufp);
//		goto CLEANUP;
//	}
	if (result_flist)
	{
		et_zones =  PIN_FLIST_ELEM_GET(result_flist, MSO_FLD_ET_ZONES, et_zone_elem_id, 1, ebufp);
	
		if (et_zones )
		{
			free(et_zone_code);
			et_zone_code = PIN_FLIST_FLD_TAKE(et_zones, MSO_FLD_ET_ZONE_CODE, 0, ebufp );
		}
	}

	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	//acctinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 1, ebufp); 

	PIN_FLIST_FLD_PUT(acctinfo, MSO_FLD_ET_ZONE, et_zone_code, ebufp);
	PIN_FLIST_FLD_PUT(acctinfo, MSO_FLD_VAT_ZONE, vat_zone_code, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "modified input_flist ", i_flistp);


	CLEANUP:
	PIN_FLIST_DESTROY_EX(&code_info, NULL); 
	return;
}

/**************************************************
* Function: fm_mso_populate_billinfo_and_payinfo()
* 
* 
***************************************************/
static void
fm_mso_populate_billinfo_and_payinfo(
	pcm_context_t		*ctxp,
	int32			account_type,
	int32			corporate_type,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*billinfo = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*nameinfo = NULL;
	pin_flist_t		*parent_inv_info = NULL;
	pin_flist_t		*group_info = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*actinfo_flistp = NULL;
	pin_flist_t		*code_info_flistp = NULL;
	
	int32			error = 1;
	int32			inv_type = PIN_INV_TYPE_SUMMARY;
	int32			pay_type = PIN_PAY_TYPE_INVOICE;
	int32			delivery_pref_email = PIN_INV_EMAIL_DELIVERY;
	int32			delivery_pref_postal = PIN_INV_USP_DELIVERY;
	int32			actg_type = PIN_ACTG_TYPE_BALANCE_FORWARD;
	int32			payment_term = 101;
	int32			billing_dom = 1;
	int32			*pp_type = NULL;
	int32			indicator = 0;
	int32			prepaid = 1;
	int32			*billing_segment_ptr = NULL;
	int32			*billing_dom_ptr = NULL;
	int32			billing_segment = 0;
	int32			*tds_flag = NULL;
	int32			default_tds_flag = 0;
	int32			payment_term_BB = 102; // Added for Broadband
	int64			db = -1;
	char			name[200];
	char			*state_code = NULL;
	char			*area = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_populate_billinfo_and_payinfo", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_billinfo_and_payinfo input flist", i_flistp);	

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp));

	billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 0, ebufp);
	PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);
	
	billing_dom = *(int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PREF_DOM, 0, ebufp);
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, &billing_dom, ebufp);
//	PIN_FLIST_FLD_DROP(i_flistp, MSO_FLD_PREF_DOM, ebufp);

/*	if (subscriber_type == CORPORATE_CHILD)
	{
		pay_type = PIN_PAY_TYPE_SUBORD;
	}
*/
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
//	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "UNASSIGNED", ebufp);
	if (account_type == ACCT_TYPE_SUBSCRIBER)
	{
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "CATV", ebufp);
	}
	else if (account_type == ACCT_TYPE_SME_SUBS_BB ||
		 account_type == ACCT_TYPE_RETAIL_BB ||
		 //account_type == ACCT_TYPE_CYBER_CAFE_BB || removed account type
		 account_type == ACCT_TYPE_CORP_SUBS_BB
	        )
	{
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "BB", ebufp);
	}

	if (account_type == ACCT_TYPE_SUBSCRIBER )
	{	
		//billing_segment = BILLING_SEGMENT_RETAIL;
		//city_code =  (char *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CITY_CODE, 1, ebufp);
		actinfo_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 0, ebufp);
		area = PIN_FLIST_FLD_GET(actinfo_flistp, MSO_FLD_AREA, 1, ebufp);
		fm_mso_get_state_city_area_code(ctxp, area, NULL, &code_info_flistp, ebufp );
		
		if (code_info_flistp && (PIN_FLIST_FLD_GET(code_info_flistp, PIN_FLD_ERROR_CODE, 1, ebufp)))
		{
			*ret_flistp = PIN_FLIST_COPY(code_info_flistp, ebufp);
			goto CLEANUP;
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state_city_area_code", code_info_flistp);
		state_code = (char*)PIN_FLIST_FLD_GET(code_info_flistp, PIN_FLD_STATE, 1, ebufp);
		fm_mso_assign_bill_segment_and_bdom(ctxp, i_flistp, state_code, &r_flistp, ebufp);
		if (r_flistp)
		{
			billing_segment_ptr = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_BILLING_SEGMENT, 1, ebufp);
			//BDOM based on CITY
			billing_dom_ptr = PIN_FLIST_FLD_GET(r_flistp, MSO_FLD_PREF_DOM, 1, ebufp);

			//BDOM based on LCO's PREF DOM
			/*group_info = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_WHOLESALE_INFO, 1 , ebufp);
			if(group_info)
			{
				billing_dom_ptr = PIN_FLIST_FLD_GET(group_info, MSO_FLD_PREF_DOM, 1, ebufp);
			}*/
			if (billing_segment_ptr && billing_dom_ptr )
			{
				billing_segment = *(int32*)billing_segment_ptr;
				billing_dom     = *(int32*)billing_dom_ptr;
			}
			else
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in assigning bill_segment and bdom", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &error, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51042", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in assigning bill_segment and bdom", ebufp);
				goto CLEANUP;
			}
                }
		else
                {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in assigning bill_segment and bdom", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &error, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51042", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in assigning bill_segment and bdom", ebufp);
			goto CLEANUP;
                }
	}
	else if ( account_type == ACCT_TYPE_HTW ||   //Business unit
		  account_type == ACCT_TYPE_JV  ||
		  account_type == ACCT_TYPE_DTR ||
		  account_type == ACCT_TYPE_SUB_DTR ||
		  account_type == ACCT_TYPE_LCO ||
		  account_type == ACCT_TYPE_LOGISTICS
		)
	{
		billing_segment = BILLING_SEGMENT_WHOLESALE;
		/**** Pavan Bellala 18-09-2015 **************************
		For business unit, billinfo_id is SELF
		********************************************************/
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "SELF", ebufp);
	}
	else if ( account_type == ACCT_TYPE_SME_SUBS_BB ||   
		  account_type == ACCT_TYPE_RETAIL_BB  ||
		  //account_type == ACCT_TYPE_CYBER_CAFE_BB || removed cyber cafe
		  account_type == ACCT_TYPE_CORP_SUBS_BB 
		)
	{
		billing_segment = BILLING_SEGMENT_RETAIL; // Bill segment for broadband.
	   	//Pawan: Added below to set BDOM to 1 for BB.
		billing_dom = BB_BILLING_DOM;
	} // END: Added for Broadband account types

	else
	{
		billing_segment = BILLING_SEGMENT_INTERNAL;
	}
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLING_SEGMENT, &billing_segment, ebufp);
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, &billing_dom, ebufp);

	PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);
	PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Billinfo", i_flistp);

	payinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_PAYINFO, 0, ebufp);
	PIN_FLIST_FLD_PUT(payinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/payinfo/invoice", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(payinfo, PIN_FLD_INV_TYPE, &inv_type, ebufp);
	PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
	// START: Added for Broadband account types
	//PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAYMENT_TERM, &payment_term, ebufp);
	if ( account_type == ACCT_TYPE_SME_SUBS_BB ||   
		  account_type == ACCT_TYPE_RETAIL_BB  ||
		  //account_type == ACCT_TYPE_CYBER_CAFE_BB || removed cyber cafe
		  account_type == ACCT_TYPE_CORP_SUBS_BB 
		)
	{
		PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAYMENT_TERM, &payment_term_BB, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAYMENT_TERM, &payment_term, ebufp);
	}
	// END: Added for Broadband account types
	inherited_info = PIN_FLIST_SUBSTR_ADD(payinfo, PIN_FLD_INHERITED_INFO, ebufp);
	inv_info = PIN_FLIST_ELEM_ADD(inherited_info, PIN_FLD_INV_INFO,0, ebufp);


	// Pavan Bellala - 06-08-2015 Set indicator as -1 initially during registration. And update later on creating service.
	/*
	group_info = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 1, ebufp);
	if (group_info)
	{
		indicator = fm_mso_get_parent_payinfo(ctxp, group_info, &parent_inv_info, ebufp );
		PIN_FLIST_FLD_SET(inv_info,  PIN_FLD_INDICATOR, &indicator, ebufp);
	}
	else
	{
		indicator = 1;
		PIN_FLIST_FLD_SET(inv_info,  PIN_FLD_INDICATOR, &indicator, ebufp);
	}
	*/
	indicator =  -1;
	PIN_FLIST_FLD_SET(inv_info,  PIN_FLD_INDICATOR, &indicator, ebufp);

	//Set TDS flag
	tds_flag = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_TDS_RECV, 1, ebufp);
	if (tds_flag)
	{
		PIN_FLIST_FLD_SET(inv_info,  MSO_FLD_TDS_RECV, tds_flag, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(inv_info,  MSO_FLD_TDS_RECV, &default_tds_flag, ebufp);
	}

	pp_type = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DELIVERY_PREFER, 1, ebufp);
	if (pp_type && *pp_type == 1 )
	{
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_PREFER, &delivery_pref_postal, ebufp);
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_DESCR, "Delivery By Post", ebufp);
	}
	else if (pp_type && *pp_type == 3 )
	{
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_PREFER, pp_type, ebufp);
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_DESCR, "N/A", ebufp);
	}	
	else
	{
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_PREFER, &delivery_pref_email, ebufp);
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_DESCR, "Delivery By email", ebufp);
	}

	nameinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp) ;

	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_ADDRESS, inv_info, PIN_FLD_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_CITY, inv_info, PIN_FLD_CITY, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_COUNTRY, inv_info, PIN_FLD_COUNTRY, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_STATE, inv_info, PIN_FLD_STATE, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_ZIP, inv_info, PIN_FLD_ZIP, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_EMAIL_ADDR, inv_info, PIN_FLD_EMAIL_ADDR, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_AREA_NAME, inv_info, MSO_FLD_AREA_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_LOCATION_NAME, inv_info, MSO_FLD_LOCATION_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_STREET_NAME, inv_info, MSO_FLD_STREET_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_BUILDING_NAME, inv_info, MSO_FLD_BUILDING_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_BUILDING_ID, inv_info, MSO_FLD_BUILDING_ID, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_LANDMARK, inv_info, MSO_FLD_LANDMARK_NAME, ebufp);
	strcpy(name, (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_FIRST_NAME, 0, ebufp));
	strcat(name, " ");
	strcat(name, (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_MIDDLE_NAME, 0, ebufp));
	strcat(name, " ");
	strcat(name, (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_LAST_NAME, 0, ebufp));
	PIN_FLIST_FLD_SET(inv_info, PIN_FLD_NAME, name, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Inv info", i_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in populating billinfo_and_payinfo", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &error, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51042", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in populating billinfo_and_payinfo", ebufp);
		goto CLEANUP;
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	return;
}

/**************************************************
* Function: fm_mso_populate_billinfo_and_payinfo_corp()
* 
* 
***************************************************/
static void
fm_mso_populate_billinfo_and_payinfo_corp(
	pcm_context_t		*ctxp,
	int32			account_type,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*billinfo = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*nameinfo = NULL;
	pin_flist_t		*bal_info = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*actinfo_flistp = NULL;
	pin_flist_t		*code_info_flistp = NULL;
	
	int32			error = 1;
	int32			inv_type = PIN_INV_TYPE_SUMMARY;
	int32			pay_type = PIN_PAY_TYPE_INVOICE;
	int32			delivery_pref_email = PIN_INV_EMAIL_DELIVERY;
	int32			delivery_pref_postal = PIN_INV_USP_DELIVERY;
	int32			actg_type = PIN_ACTG_TYPE_BALANCE_FORWARD;
	int32			payment_term = 101;
	int32			*billing_segment_ptr = NULL;
	int32			*billing_dom_ptr = NULL;
	int32			billing_dom = 1;
	int32			billing_segment = 0;

	int32			*service_offer_type = NULL;
	int32			*pp_type = NULL;
	int32			prepaid = 1;
	int32			*tds_flag = NULL;
	int32			default_tds_flag = 0;

	int64			db = -1;
	char			name[200];
	char			*middle_name = NULL;
	char			*state_code = NULL;
	char			*area = NULL;
	int32			payment_term_BB = 102;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_populate_billinfo_and_payinfo_corp", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_billinfo_and_payinfo_corp input flist", i_flistp);	

	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp));

	/****** Pavan Bellala 06-08-2015 ********************
	Changes made to have only one billinfo all the time
	Commented below two lines
	*****************************************************/

//	billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 0, ebufp);
//	PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);
	
//	billing_dom = *(int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PREF_DOM, 0, ebufp);

	//city_code =  (char *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CITY_CODE, 1, ebufp);
	actinfo_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO, 0, 0, ebufp);
	area = PIN_FLIST_FLD_GET(actinfo_flistp, MSO_FLD_AREA, 1, ebufp);
	fm_mso_get_state_city_area_code(ctxp, area, NULL, &code_info_flistp, ebufp );
	
	if (code_info_flistp && (PIN_FLIST_FLD_GET(code_info_flistp, PIN_FLD_ERROR_CODE, 1, ebufp)))
	{
		*ret_flistp = PIN_FLIST_COPY(code_info_flistp, ebufp);
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state_city_area_code", code_info_flistp);
	state_code = (char*)PIN_FLIST_FLD_GET(code_info_flistp, PIN_FLD_STATE, 1, ebufp);
	fm_mso_assign_bill_segment_and_bdom(ctxp, i_flistp, state_code, &r_flistp, ebufp);
	if (r_flistp)
	{
		billing_segment_ptr = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_BILLING_SEGMENT, 1, ebufp);
		billing_dom_ptr = PIN_FLIST_FLD_GET(r_flistp, MSO_FLD_PREF_DOM, 1, ebufp);

		if (billing_segment_ptr && billing_dom_ptr )
		{
			billing_segment = *(int32*)billing_segment_ptr;
			billing_dom     = *(int32*)billing_dom_ptr;
		}
		else
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in assigning bill_segment and bdom", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &error, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51042", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in assigning bill_segment and bdom", ebufp);
			goto CLEANUP;
		}
	}

//Pavan commented :	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, &billing_dom, ebufp);
//Pavan Commented :	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLING_SEGMENT, &billing_segment, ebufp);

//	PIN_FLIST_FLD_DROP(i_flistp, MSO_FLD_PREF_DOM, ebufp);
/***** Pavan Bellala 06-08-2015 ********************
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "SELF", ebufp);

	PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);
	PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);

	bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
	PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
	PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);
	PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "BB Balance Group", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Billinfo 1 ", i_flistp);
*******************************************************/
	payinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_PAYINFO, 0, ebufp);
	PIN_FLIST_FLD_PUT(payinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/payinfo/invoice", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(payinfo, PIN_FLD_INV_TYPE, &inv_type, ebufp);
	PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
	//PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAYMENT_TERM, &payment_term, ebufp);
	if ( account_type == ACCT_TYPE_SME_SUBS_BB ||
                  account_type == ACCT_TYPE_RETAIL_BB  ||
                  account_type == ACCT_TYPE_CORP_SUBS_BB
           )
        {
                PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAYMENT_TERM, &payment_term_BB, ebufp);
        }
        else
        {
                PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAYMENT_TERM, &payment_term, ebufp);
        }

	inherited_info = PIN_FLIST_SUBSTR_ADD(payinfo, PIN_FLD_INHERITED_INFO, ebufp);
	inv_info = PIN_FLIST_ELEM_ADD(inherited_info, PIN_FLD_INV_INFO,0, ebufp);

	pp_type = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DELIVERY_PREFER, 1, ebufp);
	if (pp_type && *pp_type == 1 )
	{
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_PREFER, &delivery_pref_postal, ebufp);
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_DESCR, "Delivery By Post", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_PREFER, &delivery_pref_email, ebufp);
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_DESCR, "Delivery By email", ebufp);
	}

	//Set TDS flag
	tds_flag = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_TDS_RECV, 1, ebufp);
	if (tds_flag)
	{
		PIN_FLIST_FLD_SET(inv_info,  MSO_FLD_TDS_RECV, tds_flag, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(inv_info,  MSO_FLD_TDS_RECV, &default_tds_flag, ebufp);
	}

	nameinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, 1, ebufp) ;

	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_ADDRESS, inv_info, PIN_FLD_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_CITY, inv_info, PIN_FLD_CITY, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_COUNTRY, inv_info, PIN_FLD_COUNTRY, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_STATE, inv_info, PIN_FLD_STATE, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_ZIP, inv_info, PIN_FLD_ZIP, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, PIN_FLD_EMAIL_ADDR, inv_info, PIN_FLD_EMAIL_ADDR, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_AREA_NAME, inv_info, MSO_FLD_AREA_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_LOCATION_NAME, inv_info, MSO_FLD_LOCATION_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_STREET_NAME, inv_info, MSO_FLD_STREET_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_BUILDING_NAME, inv_info, MSO_FLD_BUILDING_NAME, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_BUILDING_ID, inv_info, MSO_FLD_BUILDING_ID, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo, MSO_FLD_LANDMARK, inv_info, MSO_FLD_LANDMARK_NAME, ebufp);
	strcpy(name, (char *)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_FIRST_NAME, 0, ebufp));
	strcat(name, (char *)" ");
	middle_name = PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_MIDDLE_NAME, 1, ebufp);
	if (middle_name && strcmp(middle_name ,"") != 0)
	{
		strcat(name, middle_name);
		strcat(name, (char *)" ");
	}
	strcat(name, (char *)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_LAST_NAME, 0, ebufp));
	PIN_FLIST_FLD_SET(inv_info, PIN_FLD_NAME, name, ebufp );

	/*********** Pavan Bellala - 06/08/2015 - Set indicator as -1 initially *************/
	prepaid = -1;
	PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INDICATOR, &prepaid, ebufp );


	service_offer_type = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE_OF_SERVICE, 0, ebufp ); 
	if (service_offer_type && *service_offer_type == ALL)
	{
		//create CATV bill unit
		//billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 1, ebufp);
		billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 0, ebufp);
		PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);

		//billing_dom = *(int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PREF_DOM, 0, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, &billing_dom, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLING_SEGMENT, &billing_segment, ebufp);

		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "CATV", ebufp);

		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);
		//PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 1, ebufp);
		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Billinfo 2", i_flistp);

		//bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 1, ebufp);
		bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
		PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
		//PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 1, ebufp);
		PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "CATV Balance Group", ebufp);

		//create BB bill unit
		//billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 2, ebufp);
		billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 1, ebufp);
		PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);

		//billing_dom = *(int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PREF_DOM, 0, ebufp);
		//Pawan: Added below to set 1 as BDOM for BB
		billing_dom = BB_BILLING_DOM;
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, &billing_dom, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLING_SEGMENT, &billing_segment, ebufp);

		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "BB", ebufp);

		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);
		//PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 2, ebufp);
		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 1, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Billinfo 2", i_flistp);

		//bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 2, ebufp);
		bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 1, ebufp);
		PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
		//PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 2, ebufp);
		PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 1, ebufp);
		PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "BB Balance Group", ebufp);
	}

	if (service_offer_type && *service_offer_type == CATV)
	{
		//create CATV bill unit
		//billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 1, ebufp);
		billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 0, ebufp);
		PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);

		//billing_dom = *(int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PREF_DOM, 0, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, &billing_dom, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLING_SEGMENT, &billing_segment, ebufp);

		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "CATV", ebufp);

		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);
		//PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 1, ebufp);
		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Billinfo 2", i_flistp);

		//bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 1, ebufp);
		bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
		PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
		//PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 1, ebufp);
		PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "CATV Balance Group", ebufp);

	}
	if (service_offer_type && *service_offer_type == BB )
	{
		//create BB bill unit
		//billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 1, ebufp);
		billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 0, ebufp);
		PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);

		//billing_dom = *(int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PREF_DOM, 0, ebufp);
		//Pawan: Added below to set 1 as BDOM for BB
		billing_dom = BB_BILLING_DOM;
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, &billing_dom, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLING_SEGMENT, &billing_segment, ebufp);

		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "BB", ebufp);

		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);
		//PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 1, ebufp);
		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Billinfo 2", i_flistp);

		//bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 1, ebufp);
		bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
		PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
		//PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 1, ebufp);
		PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "BB Balance Group", ebufp);

		//Added below to change the Billinfo ID for billinfo[0]
		//billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 0, ebufp);
		//PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "BB", ebufp);
	}
 
        if (service_offer_type && *service_offer_type == WS_SETTLEMENT )
	{
		/**** Pavan Bellala 18-09-2015 ********************
		For Business Units, there should be 2 billinfos 
		***************************************************/
                //create BB bill unit
		billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 0, ebufp);
		PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "SELF", ebufp);

		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);
		//PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 1, ebufp);
		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);
	
		bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
		PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
		PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "Default Balance Group", ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Billinfo 1 ", i_flistp);
		/*******Pavan Bellala 18-09-2015 Added billinfo unit *********************/
                billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 1, ebufp);
                //billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 0, ebufp);
                PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);

                billing_dom = *(int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PREF_DOM, 0, ebufp);
                PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, &billing_dom, ebufp);

                PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
                PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
                PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "WS_SETTLEMENT", ebufp);

                PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);
                PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 2, ebufp);
                //PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Billinfo 2", i_flistp);
		
                bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 1, ebufp);
                //bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
                PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
                //PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 1, ebufp);
                PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);
		if (account_type == ACCT_TYPE_LCO) {
                    PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "LCO Balance", ebufp);
		}
		else {
                    PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "DTR Balance", ebufp);
		}

                bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 2, ebufp);
                //bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 1, ebufp);
                PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
                PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 1, ebufp);
                //PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);
                if (account_type == ACCT_TYPE_LCO) {
                    PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "LCO Commission", ebufp);
                }
                else {
                    PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "DTR Commission", ebufp);
                }

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Billinfo 3", i_flistp);
	}
        if (service_offer_type && *service_offer_type == DSA_SETTLEMENT )
        {
                //create BB bill unit
                //billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 1, ebufp);
                billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 0, ebufp);
                PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);

                billing_dom = *(int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PREF_DOM, 0, ebufp);
                PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, &billing_dom, ebufp);

                PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
                PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
                PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "DSA_SETTLEMENT", ebufp);

                PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);
                //PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 1, ebufp);
                PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Billinfo 2", i_flistp);

               	//PIN_FLIST_FLD_DROP(bal_info, PIN_FLD_NAME, ebufp);
                //PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "Default Balance Group", ebufp);

                //bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 1, ebufp);
                bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);


                PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "Default Balance Group", ebufp);

                PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
                //PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 1, ebufp);
                PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);
                PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "DSA Commission", ebufp);

        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding Inv info", i_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in populating billinfo_and_payinfo corporate account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &error, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51037", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in populating billinfo_and_payinfo corporate account", ebufp);
		goto CLEANUP;
	}
	
	CLEANUP:
	return;
}


/**************************************************
* Function: fm_mso_create_hierarchy()
*  Here only account hierarchy is created, billing 
*  hierarchy will be created during activation
* 
***************************************************/
static void
fm_mso_create_hierarchy(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*group_info = NULL;
	pin_flist_t		*move_acct_iflist = NULL;
	pin_flist_t		*move_acct_oflist = NULL;
//	pin_flist_t		*group_move_member = NULL;
//	pin_flist_t		*update_customer = NULL;
//	pin_flist_t		*child_billinfo = NULL;	
//	pin_flist_t		*billinfo = NULL;
	
//	poid_t			*parent_billinfo_pdp = NULL;
	poid_t			*corp_parent_pdp = NULL;
//	poid_t			*child_billinfo_pdp = NULL;
	poid_t			*child_account_obj = NULL;

	int32			pay_type = PIN_PAY_TYPE_SUBORD;

	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_create_hierarchy", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_hierarchy input flist", i_flistp);	

	child_account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(child_account_obj);
 
	group_info  = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 0, ebufp);
	corp_parent_pdp = PIN_FLIST_FLD_GET(group_info, PIN_FLD_PARENT, 0, ebufp);

	move_acct_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(move_acct_iflist, PIN_FLD_POID, child_account_obj, ebufp);
	PIN_FLIST_FLD_SET(move_acct_iflist, PIN_FLD_PARENT, corp_parent_pdp, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "move_acct_iflist", move_acct_iflist);
	PCM_OP(ctxp, PCM_OP_BILL_GROUP_MOVE_MEMBER, 0, move_acct_iflist, &move_acct_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&move_acct_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH:fm_mso_create_hierarchy ()", ebufp);
		goto CLEANUP;;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "move_acct_oflist", move_acct_oflist);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&move_acct_oflist, NULL);
	return;
}

/**************************************************
* Function: fm_mso_assign_bill_segment_and_bdom()
* 
*
* 
***************************************************/
static void
fm_mso_assign_bill_segment_and_bdom(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	char			*state_code,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*bill_segment_array = NULL;
	pin_flist_t		*wholesale_info = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*cfg_pdp = NULL;

	int32			pay_type = PIN_PAY_TYPE_SUBORD;

	int32			pp_bdom = 1;
	int32			pp_bill_segment = 5000;
	int32			bb_bill_segment = 101;
	int32			pp_no_bill_segment = 104;
	int32			sp_no_bill_segment = 104;
	int32			business_type = 0;

	int32			*lco_pp_type_ptr = NULL;
	int32			*lco_bill_segment_ptr = NULL;
	int32			*lco_pref_bdom_ptr = NULL;
	int32			lco_pp_type = -1;
	int32			lco_bill_segment = -1;
	int32			lco_pref_bdom = 1;
	int32			account_type =0;

	int			arr_business_type[8];


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_assign_bill_segment_and_bdom", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_assign_bill_segment_and_bdom input flist", i_flistp);
	arg_flist = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO,PIN_ELEMID_ANY, 1, ebufp); 
	if (arg_flist)
	{
		business_type = *(int32*)PIN_FLIST_FLD_GET(arg_flist, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
			
		num_to_array(business_type, arr_business_type);
		account_type    = 10*(arr_business_type[0])+arr_business_type[1];// First 2 digits
	}

	wholesale_info=PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
	lco_pp_type_ptr = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_PP_TYPE, 1, ebufp);
	if (lco_pp_type_ptr)
	{
		lco_pp_type = *(int32*)lco_pp_type_ptr;
	}
	lco_bill_segment_ptr = PIN_FLIST_FLD_GET(wholesale_info, PIN_FLD_CUSTOMER_SEGMENT, 1, ebufp);
	if (lco_bill_segment_ptr)
	{
		lco_bill_segment = *(int32*)lco_bill_segment_ptr;
	}
	lco_pref_bdom_ptr = PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_PREF_DOM, 1, ebufp);
	if (lco_pref_bdom_ptr)
	{
		lco_pref_bdom = *(int32*)lco_pref_bdom_ptr;
	}

	if (account_type == 99 )
	{

		if (lco_pp_type == PP_TYPE_PRIMARY &&
		    lco_bill_segment == LCO_IN_BILL_SEGMENT
		   )
		{
			//BDOM=1, SEGMENT=5000
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PP and BILL_SEGMENT" );
			*r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistp, MSO_FLD_PREF_DOM, &pp_bdom, ebufp);
			PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_BILLING_SEGMENT, &pp_bill_segment, ebufp);
		}

		if (lco_pp_type == PP_TYPE_PRIMARY &&
		    lco_bill_segment == LCO_IN_NO_BILL_SEGMENT
		   )
		{
			//BDOM=1, SEGMENT=5050	 --PIN_FLD_BILLING_SEGMENT
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PP and NO_BILL_SEGMENT" );
			*r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistp, MSO_FLD_PREF_DOM, &pp_bdom, ebufp);
			PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_BILLING_SEGMENT, &pp_no_bill_segment, ebufp);

		}


		if (lco_pp_type == PP_TYPE_SECONDARY &&
		    lco_bill_segment == LCO_IN_NO_BILL_SEGMENT
		   )
		{
			//BDOM from /mso_cfg_city_billing_dom, SEGMENT=104
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SP and NO_BILL_SEGMENT" );
			*r_flistp = PIN_FLIST_CREATE(ebufp);
			//fm_mso_get_bdom(ctxp, city_code ,r_flistp, ebufp);
			PIN_FLIST_FLD_SET(*r_flistp, MSO_FLD_PREF_DOM, &lco_pref_bdom, ebufp);
			PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_BILLING_SEGMENT, &sp_no_bill_segment, ebufp);
		}

		if (lco_pp_type == PP_TYPE_SECONDARY &&
		    lco_bill_segment == LCO_IN_BILL_SEGMENT
		   )
		{
			//BDOM from /mso_cfg_city_billing_dom, SEGMENT from /config/billing_segment
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SP and BILL_SEGMENT" );
			*r_flistp = PIN_FLIST_CREATE(ebufp);
			//fm_mso_get_bdom(ctxp, city_code ,r_flistp, ebufp);
			fm_mso_get_bill_segment(ctxp, state_code ,r_flistp, ebufp);
			PIN_FLIST_FLD_SET(*r_flistp, MSO_FLD_PREF_DOM, &lco_pref_bdom, ebufp);
		}

		// set NO bill segment for Free type customer
		if (business_type == 99281000 || business_type == 99282000 || business_type == 99281000)
		{
			PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_BILLING_SEGMENT, &sp_no_bill_segment, ebufp);
		}
	}
	else
	{
		//BDOM=1, SEGMENT=5000
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"BROADBAND" );
		*r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistp, MSO_FLD_PREF_DOM, &pp_bdom, ebufp);
		PIN_FLIST_FLD_SET(*r_flistp, PIN_FLD_BILLING_SEGMENT, &bb_bill_segment, ebufp);
	}

	return ;
}

///**************************************************
//* Function: fm_mso_get_bill_segment()
//* 
//*
//* 
//***************************************************/
//static int32
//fm_mso_get_bill_segment(
//	pcm_context_t		*ctxp,
//	pin_flist_t		*i_flistp,
//	char			*city_code,
//	pin_errbuf_t		*ebufp)
//{
//	pin_flist_t		*srch_flistp = NULL;
//	pin_flist_t		*arg_flist = NULL;
//	pin_flist_t		*result_flist = NULL;
//	pin_flist_t		*srch_out_flistp = NULL;
//	pin_flist_t		*bill_segment_array = NULL;
//
//	poid_t			*srch_pdp = NULL;
//	poid_t			*cfg_pdp = NULL;
//
//	int32			pay_type = PIN_PAY_TYPE_SUBORD;
//	int32			srch_flag = 768;
//
//	char			*template = "select X from /config where F1.type = V1 ";
//	char			*descr = NULL;
//	char			*field_1 = NULL;
//
//	int64			db = -1;
//	int32			billing_segment = BILLING_SEGMENT_DEFAULT;
//	int32			elem_id =0;
//	pin_cookie_t		cookie = NULL;
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_bill_segment", ebufp);
//	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_segment input flist", i_flistp);
//
//	/**********************************************************************
//	* Search for  /config/billing_segment
//	**********************************************************************/
//	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp)) ;
//	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
//	srch_flistp = PIN_FLIST_CREATE(ebufp);
//	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
//	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
//	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
//
//	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
//	cfg_pdp = PIN_POID_CREATE(db, "/config/billing_segment", -1, ebufp);
//	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, cfg_pdp, ebufp);
//
//	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_segment SEARCH input flist", srch_flistp);
//	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH: fm_mso_get_bill_segment()", ebufp);
//		goto CLEANUP;;
//	}
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_segment SEARCH output flist", srch_out_flistp);
//	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
//	if (result_flist)
//	{
//		while((bill_segment_array = PIN_FLIST_ELEM_GET_NEXT(result_flist, PIN_FLD_BILLING_SEGMENTS,
//		    &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
//		{
//			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bill_segment_array", bill_segment_array);
//			descr = (char*)PIN_FLIST_FLD_GET(bill_segment_array, PIN_FLD_DESCR, 1, ebufp);
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"descr");
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,descr);
//			if (descr )
//			{
//				field_1 =  strtok(descr,"|");
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"city_code");
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,city_code);
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"field_1");
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,field_1);
//
//				if (city_code && field_1 && strcmp(field_1, city_code)==0 )
//				{
//					billing_segment=elem_id;
//					break;
//				}
//			}
//		}
//	}
//
//	CLEANUP:
//	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
//	return 	billing_segment;
//}
/**************************************************
* Function: fm_mso_get_parent_payinfo()
* 
*
* 
***************************************************/
static int32
fm_mso_get_parent_payinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*group_info,
	pin_flist_t		**parent_inv_info,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 256;
	int32			indicator = 0;

	void			*vp = NULL;

	char			*template = "select X from /payinfo/invoice where F1.id = V1 ";


	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_parent_payinfo", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_parent_payinfo input flist", group_info);

	/**********************************************************************
	* Search for  /config/billing_segment
	**********************************************************************/
	db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(group_info, PIN_FLD_PARENT, 0, ebufp)) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(group_info, PIN_FLD_PARENT, arg_flist, PIN_FLD_ACCOUNT_OBJ, ebufp );

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_parent_payinfo SEARCH input flist", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH: fm_mso_get_parent_payinfo()", ebufp);
		goto CLEANUP;;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_parent_payinfo SEARCH output flist", srch_out_flistp);
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (result_flist)
	{
		payinfo = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_INV_INFO, 0, 1, ebufp);
		vp = PIN_FLIST_FLD_GET(payinfo, PIN_FLD_INDICATOR, 1, ebufp);
		if (vp)
		{
			indicator = *(int32*)vp;
		}
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return 	indicator;
}

/**************************************************
* Function: fm_mso_create_tax_exempt()
* 
*
* 
***************************************************/
static void
fm_mso_create_tax_exempt(
	pcm_context_t	*ctxp, 
	poid_t			*account_obj,
	pin_flist_t		*exemption,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_create_tax_exempt", ebufp);
		return;
	}
	//Create the input flist
	i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(i_flistp, 
		PIN_FLD_POID, (void *)account_obj, ebufp);
	//Set the tax exemption array
	//PIN_FLIST_ELEM_SET(i_flistp,
	//	(void *)exemption, PIN_FLD_EXEMPTIONS, 0, ebufp);
	PIN_FLIST_CONCAT(i_flistp, exemption, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_create_tax_exempt: Error creating input flist", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_create_tax_exempt: Input Flist", i_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_SET_TAXINFO, 
		0, i_flistp, &r_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_create_tax_exempt: Error calling SET_TAXINFO opcode", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_create_tax_exempt: Output Flist", r_flistp);
	
	cleanup:
		PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		return;
}

/**************************************************
 * Function: fm_mso_cust_set_payinfo()
 * Description:
 *                      Sets the additional payinfo of type
 *                      CC or DD if passed in input flist.
 ***************************************************/
static void
fm_mso_cust_set_payinfo(
        pcm_context_t   *ctxp,
        poid_t                  *account_obj,
        pin_flist_t             *payinfo,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t             *i_flistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *inher_info = NULL;
        pin_flist_t             *inv_info = NULL;
        poid_t                  *pi_pdp = NULL;
        char                    *pi_type = NULL;
        void                    *vp = NULL;
        int                             PAYMENT_TERM_BB=102;
        int                             PAY_TYPE_CC=10003;
        int                             PAY_TYPE_DD=10005;
        int                             payinfo_active=1;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_cust_set_payinfo", ebufp);
                return;
        }
        /* Create payinfo input flist */
        i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, (void *)account_obj, ebufp);
        /* Get poid type and set pay type based on its value */
        pi_pdp = PIN_FLIST_FLD_GET(payinfo, PIN_FLD_POID, 0, ebufp);
        if(pi_pdp) {
                inher_info = PIN_FLIST_SUBSTR_GET(payinfo,PIN_FLD_INHERITED_INFO, 0, ebufp);
                pi_type = (char *)PIN_POID_GET_TYPE(pi_pdp);
                if(pi_type && strcmp(pi_type,PIN_OBJ_TYPE_PAYINFO_CC)==0) {
                        PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAY_TYPE, (void *)&PAY_TYPE_CC, ebufp);
                        inv_info = PIN_FLIST_ELEM_GET(inher_info,PIN_FLD_CC_INFO, 0, 0, ebufp);
                        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_STATUS, &payinfo_active, ebufp);
                }
                else if (pi_type && strcmp(pi_type,PIN_OBJ_TYPE_PAYINFO_DD)==0) {
                        PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAY_TYPE, (void *)&PAY_TYPE_DD, ebufp);
                        inv_info = PIN_FLIST_ELEM_GET(inher_info,PIN_FLD_DD_INFO, 0, 0, ebufp);
                        PIN_FLIST_FLD_SET(inv_info, PIN_FLD_STATUS, &payinfo_active, ebufp);
                }
                else {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Incorrect Payinfo TYPE", ebufp);
                        return;
                }
        }
        else {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Payinfo POID missing.", ebufp);
                        return;
        }
        //PIN_FLIST_FLD_SET(*payinfo, PIN_FLD_PAYMENT_TERM, (void *)&PAYMENT_TERM_BB, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PROGRAM_NAME, "Automatic Account Registration", ebufp);
        PIN_FLIST_ELEM_SET(i_flistp, (void *)payinfo, PIN_FLD_PAYINFO, 0, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_cust_set_payinfo: Error creating input flist", ebufp);
                goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_set_payinfo: Input Flist", i_flistp);
        PCM_OP(ctxp, PCM_OP_CUST_SET_PAYINFO, 0, i_flistp, &r_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_cust_set_payinfo: Error calling PCM_OP_CUST_SET_PAYINFO opcode", ebufp);
                goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_cust_set_payinfo: Output Flist", r_flistp);

        cleanup:
                PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
                return;
}

/**************************************************
 * Function: fm_mso_update_account_info()
 * Description:
 *		Update account with following:
 * 		1. LASTSTAT_CMNT.
 ***************************************************/
static void
fm_mso_update_account_info(
	pcm_context_t	*ctxp, 
	poid_t			*account_obj,
	char			*last_cmnt,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*flds_in_flistp = NULL;
	pin_flist_t		*flds_out_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_update_account_info", ebufp);
		return;
	}
	
	/* Create flist and set the fields */
	flds_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(flds_in_flistp, PIN_FLD_POID, (void *)account_obj, ebufp);
	PIN_FLIST_FLD_PUT(flds_in_flistp, PIN_FLD_LASTSTAT_CMNT, (void *)last_cmnt, ebufp);
	
	/* Call write flds opcode */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Write flds input flist.", flds_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, flds_in_flistp, &flds_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&flds_in_flistp, NULL);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Write flds output flist.", flds_out_flistp);
	PIN_FLIST_DESTROY_EX(&flds_out_flistp, NULL);
	
	return;
}

static int32 
fm_mso_validate_cust_city (
        pcm_context_t   *ctxp,
	pin_flist_t     *i_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t             *read_flistp = NULL;
        pin_flist_t             *read_o_flistp = NULL;
	pin_flist_t             *nameinfo_flistp = NULL;
	pin_flist_t             *profile_flistp = NULL;
	pin_flist_t             *inh_flistp = NULL;
	pin_flist_t             *customer_careinfo = NULL;
		
	poid_t			*lco_pdp = NULL;
	int32 			flag = 0;
	int32                   elem_id = 0;
		
	void			*vp = NULL;
	char			*lco_city = NULL;
	char			*cust_city = NULL;
		
	pin_cookie_t            cookie = NULL;
		
		
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_validate_cust_city input flist",i_flistp);
		
	while((nameinfo_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_NAMEINFO,
                    &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
       	{
		vp = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_CITY, 1, ebufp);
		if (vp)	
		{
			cust_city = (char *)vp;
		}
		else
		{
		  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in getting customer city : fm_mso_validate_cust_city");
	          goto CLEANUP;
		}
	}
		
	elem_id = 0;
	cookie = NULL;
	while((profile_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PROFILES,
                   &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
	  inh_flistp = PIN_FLIST_SUBSTR_GET(profile_flistp, PIN_FLD_INHERITED_INFO, 1, ebufp);
  	  customer_careinfo = PIN_FLIST_SUBSTR_GET(inh_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
	  if (customer_careinfo)
	  {
		vp = PIN_FLIST_FLD_GET(customer_careinfo, PIN_FLD_PARENT, 1, ebufp);
		if (vp)
		{
		  lco_pdp = PIN_FLIST_FLD_GET(customer_careinfo, PIN_FLD_PARENT, 1, ebufp);
		}
		else
		{
		  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in getting lco information : fm_mso_validate_cust_city");
		  goto CLEANUP;
		}
	  }
  	  else
	  {
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in getting lco information : fm_mso_validate_cust_city");
	    goto CLEANUP;
	  }
	}
		
	read_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, lco_pdp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_validate_cust_city read obj in flist",read_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
          PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH:fm_mso_validate_cust_city");
          goto CLEANUP;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_validate_cust_city readobj out flist",read_o_flistp);
		
	elem_id = 0;
	cookie = NULL;
	while((nameinfo_flistp = PIN_FLIST_ELEM_GET_NEXT(read_o_flistp, PIN_FLD_NAMEINFO,
               &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {	
		vp = PIN_FLIST_FLD_GET(nameinfo_flistp, PIN_FLD_CITY, 1, ebufp);
		if (vp)	
		{	
			lco_city = (char *)vp;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"LCO CITY");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,lco_city);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Customer City");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,cust_city);
				
			if (strcmp(cust_city,lco_city) == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Match");
				flag = 1;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Not Match");
				flag = 0;				
			}
		}
    	}
		
    CLEANUP:
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
    	PIN_FLIST_DESTROY_EX(&read_o_flistp, NULL);
		
	return flag;
}

/**
**Function to check if any add on services like Netflix has to created
**/
void fm_mso_create_addon_services (
        pcm_context_t   *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp)

{
        int32                   elem_id = 0;
        pin_cookie_t            cookie = NULL;
        pin_flist_t     *temp_flistp = NULL;
        char     *offer_descr=  NULL;
        int     *offer_val =  NULL;

        elem_id=0;
        cookie= NULL;
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Addon Service Create Input Flist");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_addon_services  input list", i_flistp);

        while((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, MSO_FLD_REGISTER_CUSTOMER,&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_addon_services temp_flistp ", temp_flistp);
                offer_descr = PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_DESCR, 1, ebufp );
                offer_val = PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_OFFER_VALUE, 1, ebufp );
                if ((offer_descr && strcmp(offer_descr,"Netflix") == 0) && (*offer_val == 1)) {
                        //Request is to create inactive netflix service.Below we will check if any netflix service already exists
                        fm_mso_create_netflix_service(ctxp, i_flistp,r_flistpp,ebufp);
                }
        }
        return;

}

/****
***Function to create Netflix Addon service
**/
void fm_mso_create_netflix_service (
        pcm_context_t   *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp)

{
        pin_flist_t     *all_billinfo = NULL;
        pin_flist_t     *bi_out_flist = NULL;
        pin_flist_t     *bal_grp_flistp = NULL;
        pin_flist_t     *bal_grp_o_flistp = NULL;
        pin_flist_t     *def_bal_grp_flistp = NULL;
        pin_flist_t     *def_bal_grp_o_flistp = NULL;
        pin_flist_t     *srvc_flistp = NULL;
        pin_flist_t     *srvc_o_flistp = NULL;
        pin_flist_t     *srvc_arrflistp = NULL;
        pin_flist_t     *statuses_array =  NULL;


        poid_t          *account_obj = NULL;
        poid_t          *billinfo_pdp = NULL;
        poid_t          *bal_obj = NULL;
        poid_t          *svc_obj = NULL;
        poid_t          *bal_grp_objp = NULL;
        poid_t          *acc_poid = NULL;

        int64           db = 0;
        int32                  status = 10100;
        int32                   netflix_failed = 1;
        int32                   netflix_success = 0;
        int32           serv_status = PIN_STATUS_INACTIVE;
        int32           status_flags = 0;
        char                    *login = NULL;
        char                    *pwd = NULL;
        char                    *bal_grp_name = "Netflix_BG";
        char                    *def_bal_grp_name = "Default Balance group";

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Service creation Input Flist");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_netflix_service inputflist", i_flistp);


        account_obj = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_POID, 1, ebufp );
        db = PIN_POID_GET_DB(account_obj);
        login = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 0, ebufp);
        pwd =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PASSWD_CLEAR, 0, ebufp);
        //find out the existing billinfo poid
        fm_mso_get_all_billinfo(ctxp, account_obj, ALL, &all_billinfo, ebufp);
        if (all_billinfo)
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_netflix_service:fm_mso_get_all_billinfo outputflist", all_billinfo);
                bi_out_flist = PIN_FLIST_ELEM_GET(all_billinfo, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                if (bi_out_flist)
                {
                        billinfo_pdp = PIN_FLIST_FLD_GET(bi_out_flist, PIN_FLD_POID, 1, ebufp);

                }
        }

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in populating billinfo", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &netflix_failed, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "70000", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in populating billinfo for netflix service", ebufp);
                goto CLEANUP;
        }

        //Check if any balance group is present, if not create one
        acc_poid = PIN_POID_COPY(account_obj,ebufp);
        fm_mso_utils_get_act_bal_grp(ctxp,acc_poid, &bal_grp_objp, ebufp);
        if (bal_grp_objp == NULL)
        {
                //Create new Balance group
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating Default Balance group");
                def_bal_grp_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(def_bal_grp_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
                PIN_FLIST_FLD_SET(def_bal_grp_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
                PIN_FLIST_FLD_SET(def_bal_grp_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_pdp, ebufp);
                PIN_FLIST_FLD_SET(def_bal_grp_flistp, PIN_FLD_NAME, def_bal_grp_name, ebufp);
                //Creating seperate balancegroup for netflix
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Default Balance group Create Input Flist");
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Create inputflist", def_bal_grp_flistp);
                fm_mso_create_balgrp(ctxp, def_bal_grp_flistp, &def_bal_grp_o_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Validate  bal_grp_flistp created", def_bal_grp_o_flistp );
                if (def_bal_grp_o_flistp)
                {
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Default balance group created", def_bal_grp_o_flistp);
                }
        }

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating Netflix Balance group");
        bal_grp_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(bal_grp_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(bal_grp_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
        PIN_FLIST_FLD_SET(bal_grp_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_pdp, ebufp);
        PIN_FLIST_FLD_SET(bal_grp_flistp, PIN_FLD_NAME, bal_grp_name, ebufp);
        //Creating seperate balancegroup for netflix
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Balance group Create Input Flist");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Create inputflist", bal_grp_flistp);
        fm_mso_create_balgrp(ctxp, bal_grp_flistp, &bal_grp_o_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Validate  bal_grp_flistp created", bal_grp_o_flistp );
        if (bal_grp_o_flistp)
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "new balance created for Netflix service", bal_grp_o_flistp);
                bal_obj = PIN_FLIST_FLD_GET(bal_grp_o_flistp, PIN_FLD_POID, 0, ebufp );
                //Creating netflixservice
                srvc_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(srvc_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/netflix", -1, ebufp), ebufp);
                PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
                PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_BAL_GRP_OBJ, bal_obj, ebufp);
       //         srvc_arrflistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_SERVICES, 0, 1, ebufp);
         //       PIN_FLIST_FLD_COPY(srvc_arrflistp, PIN_FLD_LOGIN, srvc_flistp, PIN_FLD_LOGIN, ebufp);
                PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_LOGIN, login, ebufp);
                PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_PASSWD_CLEAR, pwd, ebufp);//remove hardcoding if this is passed in the input flist
                PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_STATUS, &status, ebufp);
                statuses_array = PIN_FLIST_ELEM_ADD(srvc_flistp, PIN_FLD_STATUSES, 0, ebufp);
                PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS, &serv_status, ebufp);

                PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);

                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating Netflix Service");
                fm_mso_create_service(ctxp, srvc_flistp, &srvc_o_flistp, ebufp);
                if (srvc_o_flistp) {
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "new service object  created for Netflix service", srvc_o_flistp);
                        svc_obj = PIN_FLIST_FLD_GET(srvc_o_flistp, PIN_FLD_POID, 0, ebufp );
                }

                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating inactive  for netflix service", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &netflix_failed, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "70001", ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in creating inactive  for netflix service", ebufp);
                        goto CLEANUP;
                } else {

                        *r_flistpp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &netflix_success, ebufp);
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, "SUccessfully created  inactive   netflix service", ebufp);

                }


        }

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating bal_grp_obj for netflix service", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, account_obj, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &netflix_failed, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "70001", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in creating bal_grp_obj for netflix service", ebufp);
                goto CLEANUP;
        }
        CLEANUP:
        PIN_FLIST_DESTROY_EX(&bal_grp_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);

        return;
}

static void fm_mso_bq_config(
        pcm_context_t           *ctxp,
		poid_t					*account_obj,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_in_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
		pin_flist_t				*cust_nameinfo = NULL;
		pin_flist_t				*citybq_info = NULL;
		pin_flist_t				*svc_code = NULL;
		
		pin_flist_t             *create_in_flistp = NULL;
        pin_flist_t             *create_out_flistp = NULL;

		pin_flist_t             *pr_flistp = NULL;
		pin_flist_t             *inh_flistp = NULL;
		pin_flist_t             *mso_info_flistp = NULL;
		
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *result_flist = NULL;
        int32                   srch_flag = 512;
        time_t                  current_t = 0;
        int                     db = 1;
		int						*carrier_id=0;
		int						*nw_cat=0;
		int						*nw_sub_cat=0;
		int						*bq_cat=0;
		int						*bq_sub_cat=0;
		int32           den_sp = 0;
		int32           bq_flag = 0;
		int32           bq_flag_jvm = 0;
        poid_t          *a_pdp = NULL;

		int32			elem_id = 0;
		pin_cookie_t	        cookie = NULL;

		char			*city = NULL;
		char			*bq_id = NULL;
		char			*bq_id_hd = NULL;
		char			*jvm_bq_id = NULL;
		char			*jvm_bq_id_hd = NULL;
		char			*area = NULL;
		char			*pop_id = NULL;
		char			nds_node[20];
		char			vm_node[20];
		char			cisco_node[20];
		char			cisco1_node[20];
		char			jvm_node[20];


		
		

        char                    *template = "select X from /mso_cfg_bouquet_city_map where F1 = V1 ";
		char                    *template1 = "select X from /mso_cfg_bq_master where F1 = V1 ";

/*		pr_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PROFILES, PIN_ELEMID_ANY, 1, ebufp);
		inh_flistp = PIN_FLIST_SUBSTR_GET(pr_flistp, PIN_FLD_INHERITED_INFO, 1, ebufp);
		mso_info_flistp = PIN_FLIST_SUBSTR_GET(inh_flistp, MSO_FLD_WHOLESALE_INFO, 1, ebufp);

		a_pdp = PIN_FLIST_FLD_GET(mso_info_flistp, PIN_FLD_PARENT, 0, ebufp); */

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Error before fm_mso_bq_config: input flist", i_flistp);
				//PIN_ERRBUF_CLEAR(ebufp);
				return;
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Error before fm_mso_bq_config: input flist1", i_flistp);

		den_sp = fm_mso_is_den_sp(ctxp, i_flistp, ebufp);

		if (den_sp == 1)
		{

			while ((svc_code = PIN_FLIST_ELEM_GET_NEXT(i_flistp, 
				PIN_FLD_SERVICE_CODES, &elem_id, 1, &cookie, ebufp)) != NULL)
			{
				if (elem_id==0)
				{
					bq_id = (char*) PIN_FLIST_FLD_GET(svc_code, MSO_FLD_BOUQUET_ID, 0, ebufp);
					
					srch_in_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
					PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);

					arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );			
					PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID, bq_id , ebufp);

					result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bq_config search input flist", srch_in_flistp);
					PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bq_config output flist", srch_out_flistp);

					if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
					{
							PIN_ERRBUF_CLEAR(ebufp);
							PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
							return;
					}
					if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) > 0 )
					{
						elem_id =0 ;
						cookie = NULL;
						
						while ((citybq_info = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, 
							PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
						{
							pop_id = PIN_FLIST_FLD_GET(citybq_info, MSO_FLD_POPULATION_ID, 0, ebufp);
							bq_cat = PIN_FLIST_FLD_GET(citybq_info, PIN_FLD_CATEGORY_ID, 0, ebufp);
							bq_sub_cat = PIN_FLIST_FLD_GET(citybq_info, PIN_FLD_CATEGORY_VERSION, 0, ebufp);
							nw_cat = PIN_FLIST_FLD_GET(citybq_info, PIN_FLD_REASON_ID, 0, ebufp);
							nw_sub_cat = PIN_FLIST_FLD_GET(citybq_info, PIN_FLD_REASON_DOMAIN_ID, 0, ebufp);
							area = PIN_FLIST_FLD_GET(citybq_info, PIN_FLD_SERVICE_AREA_CODE, 0, ebufp);

							bq_id = PIN_FLIST_FLD_GET(citybq_info, MSO_FLD_BOUQUET_ID, 0, ebufp);
							bq_id_hd = PIN_FLIST_FLD_GET(citybq_info, MSO_FLD_BOUQUET_ID_HD, 0, ebufp);

							jvm_bq_id = PIN_FLIST_FLD_GET(citybq_info, PIN_FLD_CODE, 0, ebufp);
							jvm_bq_id_hd = PIN_FLIST_FLD_GET(citybq_info, PIN_FLD_GEOCODE, 0, ebufp);

						}				
					}
				}
			

				/*else if (elem_id==1)
				{
					jvm_bq_id = (char*) PIN_FLIST_FLD_GET(svc_code, MSO_FLD_BOUQUET_ID, 0, ebufp);
					jvm_bq_id_hd = (char*) PIN_FLIST_FLD_GET(svc_code, MSO_FLD_BOUQUET_ID_HD, 0, ebufp);
					pop_id = PIN_FLIST_FLD_GET(svc_code, MSO_FLD_POPULATION_ID, 0, ebufp);
				}*/
			
			}
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Error before fm_mso_bq_config: input flist2", i_flistp);


		if (den_sp == 0 && !bq_id && !jvm_bq_id && !pop_id)
		{
			srch_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

			arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );

			elem_id =0 ;
			cookie = NULL;
			
			while ((cust_nameinfo = PIN_FLIST_ELEM_GET_NEXT(i_flistp, 
				PIN_FLD_NAMEINFO, &elem_id, 1, &cookie, ebufp)) != NULL)
			{
				city = (char*) PIN_FLIST_FLD_GET(cust_nameinfo, PIN_FLD_CITY, 0, ebufp);
				break;

			}
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CITY, city , ebufp);


			result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
			//PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bq_config search input flist", srch_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bq_config output flist", srch_out_flistp);

			if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
			{
					PIN_ERRBUF_CLEAR(ebufp);
					PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
					return;
			}
			if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) >= 2 )
			{
				elem_id =0 ;
				cookie = NULL;
				
				while ((citybq_info = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, 
					PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
				{
					carrier_id = PIN_FLIST_FLD_GET(citybq_info, PIN_FLD_CARRIER_ID, 0, ebufp);
					pop_id = PIN_FLIST_FLD_GET(citybq_info, MSO_FLD_POPULATION_ID, 0, ebufp);

					if ( *carrier_id == 0 && bq_flag == 0)
					{
						bq_id = PIN_FLIST_FLD_GET(citybq_info, MSO_FLD_BOUQUET_ID, 0, ebufp);
						bq_id_hd = PIN_FLIST_FLD_GET(citybq_info, MSO_FLD_BOUQUET_ID_HD, 0, ebufp);
						bq_flag = 1;
					}
					else if (bq_flag_jvm==0)
					{
						jvm_bq_id = PIN_FLIST_FLD_GET(citybq_info, MSO_FLD_BOUQUET_ID, 0, ebufp);
						jvm_bq_id_hd = PIN_FLIST_FLD_GET(citybq_info, MSO_FLD_BOUQUET_ID_HD, 0, ebufp);
						bq_flag_jvm = 1;
					}		
				}				
			}
		}

		if(bq_id && jvm_bq_id && pop_id)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Error before fm_mso_bq_config: input flist3", i_flistp);
			if (den_sp == 0)
			{
				sprintf(nds_node,"NDS");
				sprintf(vm_node,"VM");
				sprintf(cisco_node,"CISCO");
				sprintf(cisco1_node,"CISCO1");
				sprintf(jvm_node,"JVM");
			}
			else if(den_sp == 1)
			{
				sprintf(nds_node,"NDS1");
				sprintf(vm_node,"GOSPELL");
				sprintf(cisco_node,"NAGRA");
				sprintf(cisco1_node,"CISCO1");
				sprintf(jvm_node,"JVM1");
			}
			else
			{
				goto CLEANUP;

			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Error before fm_mso_bq_config: input flist4", i_flistp);

			create_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(create_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/mso_cfg_bouquet_lco_map", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(create_in_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj , ebufp);
			PIN_FLIST_FLD_SET(create_in_flistp, PIN_FLD_SERVICE_AREA_CODE, area , ebufp);
			
			PIN_FLIST_FLD_SET(create_in_flistp, PIN_FLD_PROGRAM_NAME, (char*)"REG-API" , ebufp);


			arg_flistp = PIN_FLIST_ELEM_ADD(create_in_flistp, PIN_FLD_SERVICE_CODES, 0, ebufp );
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID, bq_id , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID_HD, bq_id_hd , ebufp);			
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_NETWORK_NODE, (char*) nds_node , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_POPULATION_ID, pop_id , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CATEGORY_ID, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CATEGORY_VERSION, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_DOMAIN_ID, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_ID, 0 , ebufp);

			arg_flistp = PIN_FLIST_ELEM_ADD(create_in_flistp, PIN_FLD_SERVICE_CODES, 1, ebufp );
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID, bq_id , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID_HD, bq_id_hd , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_NETWORK_NODE, (char*) vm_node , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_POPULATION_ID, pop_id , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CATEGORY_ID, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CATEGORY_VERSION, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_DOMAIN_ID, 0  , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_ID, 0 , ebufp);

			arg_flistp = PIN_FLIST_ELEM_ADD(create_in_flistp, PIN_FLD_SERVICE_CODES, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID, bq_id , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID_HD, bq_id_hd , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_NETWORK_NODE, (char*) cisco_node , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_POPULATION_ID, pop_id , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CATEGORY_ID, bq_cat , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CATEGORY_VERSION, bq_sub_cat , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_DOMAIN_ID, nw_sub_cat  , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_ID, nw_cat , ebufp);

			arg_flistp = PIN_FLIST_ELEM_ADD(create_in_flistp, PIN_FLD_SERVICE_CODES, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID, bq_id , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID_HD, bq_id_hd , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_NETWORK_NODE, (char*)cisco1_node , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_POPULATION_ID, pop_id , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CATEGORY_ID, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CATEGORY_VERSION, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_DOMAIN_ID, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_ID, 0 , ebufp);



			arg_flistp = PIN_FLIST_ELEM_ADD(create_in_flistp, PIN_FLD_SERVICE_CODES, 4, ebufp );
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID, jvm_bq_id , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_BOUQUET_ID_HD, jvm_bq_id_hd , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_NETWORK_NODE, (char*) jvm_node , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_POPULATION_ID, pop_id , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CATEGORY_ID, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_CATEGORY_VERSION, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_DOMAIN_ID, 0 , ebufp);
			PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_ID, 0 , ebufp);


			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bq_config create input flist", create_in_flistp);
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_in_flistp, &create_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bq_config create output flist", create_in_flistp);
		}

CLEANUP:
PIN_ERRBUF_CLEAR(ebufp);
PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
PIN_FLIST_DESTROY_EX(&create_in_flistp, NULL);
PIN_FLIST_DESTROY_EX(&create_out_flistp, NULL);
return;

                                                        
}
