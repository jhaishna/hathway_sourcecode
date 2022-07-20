/*******************************************************************
 * File:	fm_mso_cust_blk_register_customer.c
 * Opcode:	MSO_OP_CUST_BLK_REGISTER_CUSTOMER 
 * Created:	11-DEC-2015
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 *

0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 7732920 0
0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 7732920 0
0 PIN_FLD_PROGRAM_NAME    STR [0] "BULK|OAP|LCO1125122926"
0 PIN_FLD_STATUS         ENUM [0] 0
0 PIN_FLD_FIRST_NAME      STR [0] "ALPHA"
0 PIN_FLD_LAST_NAME       STR [0] "ALPHA"
0 PIN_FLD_MIDDLE_NAME       STR [0] "ALPHA"
0 PIN_FLD_ADDRESS         STR [0] "21"
0 MSO_FLD_AREA_NAME       STR [0] "SILLOD"
0 MSO_FLD_AREA            STR [0] "MH|MH-D366-C1725|MH-D366-C1725-A19891"
0 MSO_FLD_BUILDING_NAME   STR [0] "SILLOD"
0 MSO_FLD_STREET_NAME     STR [0] "SILLOD"
0 MSO_FLD_LOCATION_NAME   STR [0] "SILLOD"
0 PIN_FLD_ZIP             STR [0] "400002"
0 PIN_FLD_CITY            STR [0] "MUMBAI"
0 PIN_FLD_STATE           STR [0] "MAHARASHTRA"
0 PIN_FLD_COUNTRY         STR [0] "INDIA"
0 MSO_FLD_REGION         ENUM [0] 1
0 MSO_FLD_STB_ID          STR [0] "4840110000001106"
0 MSO_FLD_VC_ID           STR [0] "000007001159"
0 MSO_FLD_ENTITYID        STR [0] "1000028208"
0 MSO_FLD_SUBSCRIBER_CODE STR [0] "RESIDENTIAL"
0 PIN_FLD_BUSINESS_TYPE  ENUM [0] 99271100
0 PIN_FLD_INDICATOR       INT [0] 1
0 MSO_FLD_RMN             STR [0] "9898989898"
0 MSO_FLD_RMAIL           STR [0] "abcd@hathway.net"
0 MSO_FLD_PLAN_NAME       STR [0] "DD NATIONAL PREPAIRING PACK 2M"

********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_get_invoice.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_utils.h"

extern int32 *subs_plan_poid;
extern char *subs_plan_code;
extern int *validity_days;

#define CATV_PREPAID 1
#define CATV_POSTPAID 0

/**************************************
* External Functions start
***************************************/
extern void
fm_mso_utils_sequence_no(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);

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

/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_BLK_REGISTER_CUSTOMER 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_blk_reg_customer(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_blk_reg_customer(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

void
fm_mso_search_blk_lco_account(
	pcm_context_t	*ctxp,
	char			*acct_no,
	pin_flist_t		*ret_flistp,
	pin_errbuf_t	*ebufp);

void
fm_mso_search_plan_detail(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t **r_flistpp,
	pin_errbuf_t	*ebufp);

/*******************************************************************
 * MSO_OP_CUST_BLK_REGISTER_CUSTOMER  
 *******************************************************************/
void 
op_mso_cust_blk_reg_customer(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	
	int32			local = LOCAL_TRANS_OPEN_SUCCESS;
	int32			*ret_status = NULL;
	int32			status_uncommitted = 2;
	poid_t			*inp_pdp = NULL;
	
	*r_flistpp		= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_BLK_REGISTER_CUSTOMER) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_blk_reg_customer error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_blk_reg_customer input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	 inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Opening the Transaction for BULK Action!!!");
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

	fm_mso_cust_blk_reg_customer(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/
	ret_status = (int32*)PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_blk_reg_customer error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else if (ret_status && *ret_status == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"op_mso_cust_blk_reg_customer ret_status=1");
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
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_blk_reg_customer output flist", *r_flistpp);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_RESET(ebufp);

	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_blk_reg_customer(
	pcm_context_t	*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*lco_flistp = NULL;
	pin_flist_t		*seq_in_flistp = NULL;
	pin_flist_t		*seq_out_flistp = NULL;
	pin_flist_t		*reg_cust_in_flistp = NULL;
	pin_flist_t		*reg_cust_out_flistp = NULL;
	pin_flist_t		*reg_cust_billinfo_flistp = NULL;
	pin_flist_t		*act_cust_in_flistp = NULL;
	pin_flist_t		*act_cust_out_flistp = NULL;
	pin_flist_t		*balinfo_flistp = NULL;
	pin_flist_t		*acctinfo_flistp = NULL;
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*profile_flistp = NULL;
	pin_flist_t		*inherited_flistp = NULL;
	pin_flist_t		*cc_info_flistp = NULL;
	pin_flist_t		*phones_flistp = NULL;
	pin_flist_t		*billinfo_flistp = NULL;
	pin_flist_t		*plan_list_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*payinfo_flistp = NULL;
	pin_flist_t		*invinfo_flistp = NULL;
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*catvinfo_flistp = NULL;
	pin_flist_t		*devices_flistp = NULL;
	pin_flist_t		*plan_detail_in_flistp = NULL;
	pin_flist_t		*plan_detail_out_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;

	poid_t			*user_id = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*lco_pdp = NULL;
	poid_t			*hw_plan_pdp = NULL;
	poid_t			*subs_plan_pdp = NULL;

	char			*zip = NULL;
	char			*area_name = NULL;
	char			*first_name = NULL;
	char			*middle_name = NULL;
	char			*last_name = NULL;
	char			*address = NULL;
	char			*rmn_value = NULL;
	char			*rmail_value = NULL;
	char			*plan_name = NULL;

	char			*stb_id = NULL;
	char			*vc_id = NULL;
	char			*acct_no = NULL;
	char			d_login[100];
	char			d_fname[100];
	char			d_mname[100];
	char			d_lname[100];
	char			*lco_fname = NULL;
	char			*lco_mname = NULL;
	char			*lco_lname = NULL;
	char			*lco_addr = NULL;
	char			*seq_no = NULL;
//	char			*hw_plan_code = HW_PLAN_CODE;
//	char			*subs_plan_code = SUBS_PLAN_CODE;
	char			*poi = NULL;
	char			*poa = NULL;
	char                    *aac_pkg = NULL;

	int32			status_failure = 1;
	int32			status_success = 0;
	int32			srch_flag = 768;
	int32			elem_count = 0;
	int				svc_type = 0;
	int				*pay_type = NULL;
	int				p_type_value = CATV_PREPAID;
	int				rmn = 5 ;// For RMN Phone Element Array
	int				currency = CURRENCY; // For INR = 356 
	int				*status = NULL;
	int				hp_poid = 0;
	int32			sp_poid = 0;
	int				valid_day = 0;
	int32			elem_id = 0;

	int64			db = -1;
	time_t			now = 0;
	time_t			validity_timestamp = 0;
	time_t          new_eff_t = 0;
	time_t          *eff_t = NULL;
	struct tm       *timeeff = NULL;
	struct tm       *twlmonths = NULL;
	//int		*validity_days = 30;
	pin_cookie_t	cookie = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_blk_reg_customer input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_blk_reg_customer", ebufp);
		goto CLEANUP;
	}

	user_id =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID , 1, ebufp );
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ); 
	acct_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ENTITYID, 1, ebufp ); 
	zip = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ZIP, 1, ebufp ); 
	area_name = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AREA_NAME, 1, ebufp );
	first_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FIRST_NAME, 1, ebufp );
	last_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LAST_NAME, 1, ebufp );
	middle_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MIDDLE_NAME, 1, ebufp );
	address = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ADDRESS, 1, ebufp );
	rmn_value = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_RMN, 1, ebufp );
	rmail_value = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_RMAIL, 1, ebufp );
	plan_name = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PLAN_NAME, 1, ebufp );
	
	stb_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_STB_ID, 1, ebufp ); 
	vc_id = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_VC_ID, 1, ebufp ); 
	pay_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_INDICATOR, 1, ebufp ); 

	db = PIN_POID_GET_DB(user_id);

  	/*******************************************************************
	* Mandatory fields validation
	*******************************************************************/
	if (!user_id || !zip || !area_name || !stb_id || !acct_no)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"MISSING MANDATORY PARAMETERS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51190", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "MISSING MANDATORY PARAMETERS", ebufp);
		goto CLEANUP;
	}
	memset(d_login,'\0',sizeof(d_login));
	memset(d_fname,'\0',sizeof(d_fname));
	memset(d_mname,'\0',sizeof(d_mname));
	memset(d_lname,'\0',sizeof(d_lname));
	
	/*******************************************************************
	* Sequence for LOGIN ID & Other Name Params, based on availability
	*******************************************************************/

	seq_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(seq_in_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(seq_in_flistp, PIN_FLD_NAME, "MSO_BLK_ACNT_CREATION_SEQUENCE", ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Sequence input flist", seq_in_flistp);
	fm_mso_utils_sequence_no(ctxp, seq_in_flistp, &seq_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Sequence output flist", seq_out_flistp);
	seq_no = PIN_FLIST_FLD_GET(seq_out_flistp, PIN_FLD_STRING, 1, ebufp);

	if (!seq_no)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Sequence Object Details Not Found", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51190", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Sequence Object Details Not Found", ebufp);
		goto CLEANUP;
	}

	reg_cust_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(reg_cust_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/plan", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(reg_cust_in_flistp, PIN_FLD_USERID, user_id, ebufp);
	PIN_FLIST_FLD_SET(reg_cust_in_flistp, PIN_FLD_PASSWD_CLEAR, "Hathway123", ebufp);
	PIN_FLIST_FLD_SET(reg_cust_in_flistp, PIN_FLD_DELIVERY_PREFER, 0, ebufp);
	PIN_FLIST_FLD_SET(reg_cust_in_flistp, PIN_FLD_PARENT_FLAGS, 0, ebufp);
	strcpy(d_login, "LOGIN_");
	strcat(d_login, seq_no);
	PIN_FLIST_FLD_SET(reg_cust_in_flistp, PIN_FLD_LOGIN, d_login, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, reg_cust_in_flistp, PIN_FLD_PROGRAM_NAME , ebufp);

	PIN_FLIST_FLD_SET(reg_cust_in_flistp, PIN_FLD_FLAGS, 0, ebufp);
	svc_type = CATV;
	PIN_FLIST_FLD_SET(reg_cust_in_flistp, PIN_FLD_TYPE_OF_SERVICE, &svc_type, ebufp);

	profile_flistp = PIN_FLIST_ELEM_ADD(reg_cust_in_flistp, PIN_FLD_PROFILES, 0, ebufp);
	inherited_flistp = PIN_FLIST_SUBSTR_ADD(profile_flistp, PIN_FLD_INHERITED_INFO, ebufp); 
	cc_info_flistp = PIN_FLIST_SUBSTR_ADD(inherited_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp );

	PIN_FLIST_FLD_SET(cc_info_flistp, MSO_FLD_SALESMAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(cc_info_flistp, MSO_FLD_SALES_CLOSE_TYPE, 0, ebufp);
		
	lco_flistp = PIN_FLIST_CREATE(ebufp);
	fm_mso_search_blk_lco_account(ctxp, acct_no, lco_flistp, ebufp);
	lco_pdp = PIN_FLIST_FLD_GET(lco_flistp, PIN_FLD_POID, 1, ebufp);
	lco_fname = PIN_FLIST_FLD_GET(lco_flistp, PIN_FLD_FIRST_NAME, 1, ebufp);
	lco_mname = PIN_FLIST_FLD_GET(lco_flistp, PIN_FLD_MIDDLE_NAME, 1, ebufp);
	lco_lname = PIN_FLIST_FLD_GET(lco_flistp, PIN_FLD_LAST_NAME, 1, ebufp);
	lco_addr = PIN_FLIST_FLD_GET(lco_flistp, PIN_FLD_ADDRESS, 1, ebufp);
	if (!lco_pdp || !lco_fname || !lco_lname)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"LCO Account Name Details Not Found", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51190", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "LCO Account Name Details Not Found", ebufp);
		goto CLEANUP;
	}
	
	PIN_FLIST_FLD_SET(cc_info_flistp, PIN_FLD_PARENT, lco_pdp, ebufp);

	PIN_FLIST_FLD_PUT(profile_flistp, PIN_FLD_PROFILE_OBJ, PIN_POID_CREATE(db, "/profile/customer_care", -1, ebufp), ebufp);

	strcpy(d_fname, lco_fname);
	strcat(d_fname, "_");
	strcat(d_fname, seq_no);

	if (lco_mname && (strcmp(lco_mname,"")!=0))
	{
		strcpy(d_mname, lco_mname);
		strcat(d_mname, "_");
		strcat(d_mname, seq_no);
	}
	else
	{
		strcpy(d_mname, " ");
	}
	
	strcpy(d_lname, lco_lname);
	strcat(d_lname, "_");
	strcat(d_lname, seq_no);

	/********************************************************************************
	* Validating Names, if not passed from Input, using LCO names with Sequence ID
	********************************************************************************/
	nameinfo_flistp = PIN_FLIST_ELEM_ADD(reg_cust_in_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
	if (first_name && (strcmp(first_name,"")!=0) )
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_FIRST_NAME, first_name, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_FIRST_NAME, d_fname, ebufp);
	}
	
	
	if (middle_name && (strcmp(middle_name,"")!=0) )
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_MIDDLE_NAME, middle_name, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Name info if -test 1");		
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Name info else -test 2");
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_MIDDLE_NAME, "", ebufp);
	}

	if (last_name && (strcmp(last_name,"")!=0) )
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_LAST_NAME, last_name, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_LAST_NAME, d_lname, ebufp);
	}

	PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_SALUTATION, "Mr.", ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_BUILDING_NAME, nameinfo_flistp, MSO_FLD_BUILDING_NAME , ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_STREET_NAME, nameinfo_flistp, MSO_FLD_STREET_NAME , ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LOCATION_NAME, nameinfo_flistp, MSO_FLD_LOCATION_NAME , ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LANDMARK, nameinfo_flistp, MSO_FLD_LANDMARK , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_flistp, MSO_FLD_AREA_NAME, area_name, ebufp);

	phones_flistp = PIN_FLIST_ELEM_ADD(nameinfo_flistp, PIN_FLD_PHONES, 5, ebufp);

	if (rmn_value && (strcmp(rmn_value,"")!=0) )
	{
		PIN_FLIST_FLD_SET(phones_flistp, PIN_FLD_PHONE, rmn_value, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(phones_flistp, PIN_FLD_PHONE, "1234567890", ebufp);
	}
	
	PIN_FLIST_FLD_SET(phones_flistp, PIN_FLD_TYPE, &rmn, ebufp);
	
	if (rmail_value && (strcmp(rmail_value,"")!=0) )
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_EMAIL_ADDR, rmail_value, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_EMAIL_ADDR, "dummy_email@hathway.net", ebufp);
	}
	
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, nameinfo_flistp, PIN_FLD_COUNTRY , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_ZIP, zip, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATE, nameinfo_flistp, PIN_FLD_STATE , ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CITY, nameinfo_flistp, PIN_FLD_CITY , ebufp);
	if (address && (strcmp(address,"")!=0) )
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_ADDRESS, address, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_ADDRESS, lco_addr, ebufp);
	}

	nameinfo_flistp = PIN_FLIST_ELEM_ADD(reg_cust_in_flistp, PIN_FLD_NAMEINFO, 2, ebufp);
	if (first_name && (strcmp(first_name,"")!=0) )
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_FIRST_NAME, first_name, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_FIRST_NAME, d_fname, ebufp);
	}
	
	if (middle_name && (strcmp(middle_name,"")!=0) )
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_MIDDLE_NAME, middle_name, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Name info 1 if -test 1");
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Name info 1 else -test 2");
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_MIDDLE_NAME, "", ebufp);
	}
	
	if (last_name && (strcmp(last_name,"")!=0) )
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_LAST_NAME, last_name, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_LAST_NAME, d_lname, ebufp);
	}

	PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_SALUTATION, "Mr.", ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_BUILDING_NAME, nameinfo_flistp, MSO_FLD_BUILDING_NAME , ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_STREET_NAME, nameinfo_flistp, MSO_FLD_STREET_NAME , ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LOCATION_NAME, nameinfo_flistp, MSO_FLD_LOCATION_NAME , ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LANDMARK, nameinfo_flistp, MSO_FLD_LANDMARK , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_flistp, MSO_FLD_AREA_NAME, area_name, ebufp);

	phones_flistp = PIN_FLIST_ELEM_ADD(nameinfo_flistp, PIN_FLD_PHONES, 5, ebufp);
	if (rmn_value && (strcmp(rmn_value,"")!=0) )
	{
		PIN_FLIST_FLD_SET(phones_flistp, PIN_FLD_PHONE, rmn_value, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(phones_flistp, PIN_FLD_PHONE, "1234567890", ebufp);
	}
	
	PIN_FLIST_FLD_SET(phones_flistp, PIN_FLD_TYPE, &rmn, ebufp);
	
	if (rmail_value && (strcmp(rmail_value,"")!=0) )
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_EMAIL_ADDR, rmail_value, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_EMAIL_ADDR, "dummy_email@hathway.net", ebufp);
	}
	
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_COUNTRY, nameinfo_flistp, PIN_FLD_COUNTRY , ebufp);
	PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_ZIP, zip, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATE, nameinfo_flistp, PIN_FLD_STATE , ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CITY, nameinfo_flistp, PIN_FLD_CITY , ebufp);
	if (address && (strcmp(address,"")!=0) )
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_ADDRESS, address, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_ADDRESS, lco_addr, ebufp);
	}

	PIN_FLIST_FLD_SET(reg_cust_in_flistp, MSO_FLD_ROLES, "SELF CARE", ebufp);

	acctinfo_flistp = PIN_FLIST_ELEM_ADD(reg_cust_in_flistp, PIN_FLD_ACCTINFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AREA, acctinfo_flistp, MSO_FLD_AREA , ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_REGION, acctinfo_flistp, MSO_FLD_REGION , ebufp);
	PIN_FLIST_FLD_PUT(acctinfo_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/account", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_CURRENCY, &currency, ebufp);

	if (rmail_value && (strcmp(rmail_value,"")!=0) )
	{
		PIN_FLIST_FLD_SET(acctinfo_flistp, MSO_FLD_RMAIL, rmail_value, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(acctinfo_flistp, MSO_FLD_RMAIL, "dummy_email@hathway.net", ebufp);
	}

	
	if (rmn_value && (strcmp(rmn_value,"")!=0) )
	{
		PIN_FLIST_FLD_SET(acctinfo_flistp, MSO_FLD_RMN, rmn_value, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(acctinfo_flistp, MSO_FLD_RMN, "1234567890", ebufp);
	}

	
	PIN_FLIST_FLD_SET(acctinfo_flistp, MSO_FLD_CONTACT_PREF, 0, ebufp);
	balinfo_flistp = PIN_FLIST_ELEM_ADD(acctinfo_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_AAC_SOURCE, "OAP_BULK_ACT", ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_AAC_VENDOR, "OAP", ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BUSINESS_TYPE, acctinfo_flistp, PIN_FLD_BUSINESS_TYPE, ebufp);
	
	poi = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCESS_CODE1, 1, ebufp);
	if (poi)
	{
		PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_ACCESS_CODE1, poi, ebufp);	
	}

        aac_pkg = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AAC_PACKAGE, 1, ebufp);
        if (aac_pkg)
        {
                PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_AAC_PACKAGE, aac_pkg, ebufp);
        }


        poa = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCESS_CODE2, 1, ebufp);
        if (poa)
        {
                PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_ACCESS_CODE2, poa, ebufp);
        }


	while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, MSO_FLD_GST_INFO, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
		PIN_FLIST_ELEM_SET(reg_cust_in_flistp, tmp_flistp, MSO_FLD_GST_INFO, elem_id, ebufp);
	}
	// Calling Register Customer API
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_blk_reg_customer registration input flist", reg_cust_in_flistp);

	PCM_OP(ctxp, MSO_OP_CUST_REGISTER_CUSTOMER, 0, reg_cust_in_flistp, &reg_cust_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_REGISTER_CUSTOMER", ebufp);
			PIN_ERRBUF_RESET(ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_blk_reg_customer registration output flist", reg_cust_out_flistp);
	if (!reg_cust_out_flistp)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Customer Registration Failed", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51190", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Customer Registration Failed", ebufp);
		goto CLEANUP;
	}
	
	status = PIN_FLIST_FLD_GET(reg_cust_out_flistp, PIN_FLD_STATUS, 1, ebufp);
	if (status && (*status == 1))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Customer Registration API Error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_COPY(reg_cust_out_flistp, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		goto CLEANUP;
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Customer Registration Successful!!!");
		act_cust_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(act_cust_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/plan", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(act_cust_in_flistp, PIN_FLD_USERID, user_id, ebufp);
		PIN_FLIST_FLD_COPY(reg_cust_out_flistp, PIN_FLD_POID, act_cust_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(reg_cust_out_flistp, PIN_FLD_ACCOUNT_NO, act_cust_in_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
		PIN_FLIST_FLD_COPY(reg_cust_out_flistp, PIN_FLD_BUSINESS_TYPE, act_cust_in_flistp, PIN_FLD_BUSINESS_TYPE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, act_cust_in_flistp, PIN_FLD_PROGRAM_NAME , ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PURCHASE_END_T, act_cust_in_flistp, PIN_FLD_PURCHASE_END_T, ebufp);
		reg_cust_billinfo_flistp = PIN_FLIST_ELEM_GET(reg_cust_out_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp);
		if (!reg_cust_billinfo_flistp)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Billinfo Creation Error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51190", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Billinfo Creation Error", ebufp);
			goto CLEANUP;
		}

		billinfo_flistp = PIN_FLIST_ELEM_ADD(act_cust_in_flistp, PIN_FLD_BILLINFO, 0, ebufp);
		PIN_FLIST_FLD_COPY(reg_cust_billinfo_flistp, PIN_FLD_BILL_WHEN, billinfo_flistp, PIN_FLD_BILL_WHEN , ebufp);
		
		plan_list_flistp = PIN_FLIST_SUBSTR_ADD(act_cust_in_flistp, PIN_FLD_PLAN_LIST_CODE, ebufp);
		plan_flistp = PIN_FLIST_ELEM_ADD(plan_list_flistp, PIN_FLD_PLAN, 0, ebufp);

	/***************************************************************************************
	* Validating Plan Name from Input, if not passed using the default from CM's pi.conf
	***************************************************************************************/
		if (plan_name && (strcmp(plan_name,"")!=0))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Plan Name passed in INPUT");
			plan_detail_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(plan_detail_in_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
			PIN_FLIST_FLD_SET(plan_detail_in_flistp, PIN_FLD_NAME, plan_name, ebufp);
			fm_mso_search_plan_detail(ctxp, plan_detail_in_flistp, &plan_detail_out_flistp, ebufp);
			if (!plan_detail_out_flistp)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"PLAN DETAILS NOT AVAILABLE", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51190", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plan Details Not Available in DB", ebufp);
				goto CLEANUP;
			}
			
			PIN_FLIST_FLD_COPY(plan_detail_out_flistp, PIN_FLD_POID, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(plan_detail_out_flistp, PIN_FLD_CODE, plan_flistp, PIN_FLD_CODE, ebufp);

		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Plan Name Not passed in INPUT");
			if (!subs_plan_poid || !subs_plan_code || !validity_days)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"FTA PLAN DETAILS NOT CONFIGURED IN CONFIG FILE", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51190", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "FTA PLAN DETAILS NOT CONFIGURED IN CONFIG FILE", ebufp);
				goto CLEANUP;
			}
			
			sp_poid = *subs_plan_poid;
			subs_plan_pdp = PIN_POID_CREATE(db, "/plan", sp_poid, ebufp);

			PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_PLAN_OBJ, subs_plan_pdp, ebufp);
			PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_CODE, subs_plan_code, ebufp);
		}
		now = pin_virtual_time((time_t *)NULL);
		valid_day = *validity_days;
		valid_day = valid_day * 86400;
		validity_timestamp =  now + valid_day;

		PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_PURCHASE_START_T, &now, ebufp);
		PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_PURCHASE_END_T, &validity_timestamp, ebufp);


		if (pay_type && (*pay_type == 0 || *pay_type == 1))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PAY_TYPE passed from Input");
			p_type_value = (int )*pay_type;
		}
		else 
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PAY_TYPE NOT passed from Input,Hence Defaulting to PREPAID", act_cust_in_flistp);
			p_type_value = CATV_PREPAID;
		}

		payinfo_flistp = PIN_FLIST_ELEM_ADD(act_cust_in_flistp, PIN_FLD_PAYINFO, 0, ebufp);
		inherited_flistp = PIN_FLIST_SUBSTR_ADD(payinfo_flistp, PIN_FLD_INHERITED_INFO, ebufp); 
		invinfo_flistp = PIN_FLIST_ELEM_ADD(inherited_flistp, PIN_FLD_INV_INFO, 0, ebufp );
		PIN_FLIST_FLD_SET(invinfo_flistp, PIN_FLD_INDICATOR, &p_type_value, ebufp);

		services_flistp = PIN_FLIST_ELEM_ADD(act_cust_in_flistp, PIN_FLD_SERVICES, 0, ebufp);
		PIN_FLIST_FLD_PUT(services_flistp, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, MSO_CATV, -1, ebufp), ebufp);
		catvinfo_flistp = PIN_FLIST_SUBSTR_ADD(services_flistp, MSO_FLD_CATV_INFO, ebufp); 
		PIN_FLIST_FLD_SET(catvinfo_flistp, MSO_FLD_SALESMAN_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(catvinfo_flistp, MSO_FLD_CARF_RECEIVED, 0, ebufp);
		PIN_FLIST_FLD_SET(catvinfo_flistp, MSO_FLD_BOUQUET_ID, "bouquet_id", ebufp);

		devices_flistp = PIN_FLIST_ELEM_ADD(services_flistp, PIN_FLD_DEVICES, 0, ebufp);
		PIN_FLIST_FLD_SET(devices_flistp, MSO_FLD_STB_ID, stb_id, ebufp);
	
		if (vc_id && (strcmp(vc_id,"")!=0) )
		{
			devices_flistp = PIN_FLIST_ELEM_ADD(services_flistp, PIN_FLD_DEVICES, 1, ebufp);
			PIN_FLIST_FLD_SET(devices_flistp, MSO_FLD_VC_ID, vc_id, ebufp);
		}
		

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_blk_reg_customer activation input flist", act_cust_in_flistp);

		PCM_OP(ctxp, MSO_OP_CUST_ACTIVATE_CUSTOMER, 0, act_cust_in_flistp, &act_cust_out_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_ACTIVATE_CUSTOMER", ebufp);
				PIN_ERRBUF_RESET(ebufp);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_blk_reg_customer activation output flist", act_cust_out_flistp);
		if (!act_cust_out_flistp)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Customer Activation Failed", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51190", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Customer Activation Failed", ebufp);
			goto CLEANUP;
		}
	
		status = PIN_FLIST_FLD_GET(act_cust_out_flistp, PIN_FLD_STATUS, 1, ebufp);
		if (status && (*status != 0))
		{
			//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Customer Activation API Error", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Customer Activation API Error");
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_COPY(act_cust_out_flistp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			goto CLEANUP;
		}
		else
		{
			//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,"Customer Activation Successful!!!", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Customer Activation Successful!!!");
			*ret_flistp = PIN_FLIST_COPY(act_cust_out_flistp, ebufp);
			PIN_FLIST_FLD_COPY(reg_cust_out_flistp, PIN_FLD_ACCOUNT_NO, *ret_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_success, ebufp);
		}

	}

	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&lco_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&seq_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&seq_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&reg_cust_in_flistp, NULL);
	//PIN_FLIST_DESTROY_EX(&reg_cust_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&act_cust_in_flistp, NULL);
//	PIN_FLIST_DESTROY_EX(&act_cust_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_detail_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_detail_out_flistp, NULL);
	
	
	return;
}

void
fm_mso_search_blk_lco_account(
	pcm_context_t	*ctxp,
	char			*acct_no,
	pin_flist_t		*ret_flistp,
	pin_errbuf_t	*ebufp)
{
		
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*nameinfo_flistp = NULL;
	
	char		*device_id = NULL;
	char		search_template[100] = " select X from /account where F1 = V1 ";
//	char		*acct_no = NULL;
	char		*fname = NULL;
	char		*lname = NULL;
	char		*addr = NULL;

	int			search_flags = 768;
	int64		db = 1;
	//int64		db = 0;
	poid_t		*pdp = NULL;
	poid_t		*lco_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_account input flist", i_flistp);

//	acct_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO , 1, ebufp);

	/*************
	 * search flist to search device poid
	 ************/
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_NO, acct_no, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_account search input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_account search output flist", search_outflistp);

	if(PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) > 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Account object found");

		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		lco_pdp = PIN_FLIST_FLD_GET(results_flistp , PIN_FLD_POID , 1, ebufp);
		nameinfo_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 1, ebufp);
		fname = PIN_FLIST_FLD_GET(nameinfo_flistp , PIN_FLD_FIRST_NAME , 1, ebufp);
		lname = PIN_FLIST_FLD_GET(nameinfo_flistp , PIN_FLD_LAST_NAME , 1, ebufp);
		addr = PIN_FLIST_FLD_GET(nameinfo_flistp , PIN_FLD_ADDRESS , 1, ebufp);

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
		}

		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID , lco_pdp , ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ACCOUNT_NO , acct_no , ebufp);
		PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_FIRST_NAME, ret_flistp, PIN_FLD_FIRST_NAME , ebufp);
		PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_MIDDLE_NAME, ret_flistp, PIN_FLD_MIDDLE_NAME , ebufp);
		PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_LAST_NAME, ret_flistp, PIN_FLD_LAST_NAME , ebufp);
		PIN_FLIST_FLD_COPY(nameinfo_flistp, PIN_FLD_ADDRESS, ret_flistp, PIN_FLD_ADDRESS , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_account return flist", ret_flistp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Account object not found");
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_FIRST_NAME , NULL , ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_MIDDLE_NAME , NULL , ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_LAST_NAME , NULL , ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ADDRESS , NULL , ebufp);
	}

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
}


void
fm_mso_search_plan_detail(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t **r_flistpp,
	pin_errbuf_t	*ebufp)
{
		
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	
	char		*plan_name = NULL;
	char		search_template[100] = " select X from /plan where F1 = V1 ";
	char		*manufacturer = NULL;
	int			state_id = 0;
	int			search_flags = 768;
	int64		db = 1;
	poid_t		*pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_plan_detail input flist", i_flistp);

	plan_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME , 1, ebufp);

	/*************
	 * search flist to search plan poid
	 ************/
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, plan_name, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_plan_detail search plan input list", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_plan_detail search plan output flist", search_outflistp);

	if(PIN_FLIST_ELEM_COUNT(search_outflistp , PIN_FLD_RESULTS , ebufp) > 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Plan object found");

		results_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
		}
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID , ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_CODE, *r_flistpp, PIN_FLD_CODE , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_plan_detail search return flist", *r_flistpp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Plan object not found");
	}

	PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
}

