/*******************************************************************
 * File:	fm_mso_cust_create_bb_service.c
 * Opcode:	MSO_OP_CUST_CREATE_BB_SERVICE 
 * Owner:	
 * Created:	21-JULY-2014
 * Description: Creates broadband service for an account in 
 *				inactive status.
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_create_bb_service.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "ops/bal.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pin_cust.h"
#include "pin_pymt.h"
#include "pin_inv.h"
#include "mso_ops_flds.h"
#include "mso_prov.h"
#include "mso_ntf.h"
#include "mso_cust.h"
#include "mso_lifecycle_id.h"
#include "mso_utils.h"


#define CORPORATE_PARENT 1
#define CORPORATE_CHILD  2

#define SERVICE_BB    "/service/telco/broadband"
#define SRVC_CATV     "/service/catv"

#define READY_TO_USE 1
#define PREACTIVATED 6
#define REPAIRED_TO_USE 9

#define MSO_FLAG_SRCH_BY_ID 1
#define MSO_FLAG_SRCH_BY_PIN 2
#define MSO_FLAG_SRCH_BY_ACCOUNT 3
#define MSO_FLAG_SRCH_BY_JV 4
#define MSO_FLAG_SRCH_BY_DT 5
#define MSO_FLAG_SRCH_BY_SDT 6
#define MSO_FLAG_SRCH_BY_LCO 7
#define MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER 8

#define WHOLESALE "/profile/wholesale"
#define CUSTOMER_CARE "/profile/customer_care"


#define SUCCESS 0
#define FAILED 1

#define CURRENCY_INR 356
#define PAY_TERM_BB_POSTPAID 102
#define PAY_TERM_BB_PREPAID 103


/**************************************
* External Functions start
***************************************/
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

extern void
fm_mso_get_deals_from_plan(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	**r_deal_flistp,
	pin_flist_t	**r_limit_flistp,
	pin_errbuf_t	*ebufp);

extern void
fm_mso_get_profile_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
void
extern fm_mso_get_device_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
extern void 
fm_mso_utils_sequence_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_read_bal_grp(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	poid_t			*srvc_pdp,
	pin_flist_t		**bal_grp_outflist,
	pin_errbuf_t		*ebufp);
extern void
fm_mso_get_billinfo(
	pcm_context_t		*ctxp,
	poid_t			*bal_grp_obj,
	poid_t			**billinfo,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_deal_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**deals_deposit,
	pin_flist_t		**deals_activation,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_all_billinfo(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	int32			billinfo_type,
	pin_flist_t		**billinfo,
	pin_errbuf_t		*ebufp)	;

extern void
fm_mso_get_err_desc(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_validate_pay_type(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			opcode,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_validate_pay_type_corp(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			opcode,
	pin_flist_t		**ret_flistp,
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

/**************************************
* Local Functions start
***************************************/
static void
fm_mso_validate_acnt_res(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static int32
fm_mso_get_services_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**service_array,
	char			*ser_type,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_update_bb_agr_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_store_bb_plan_details(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);	

static void
fm_mso_update_commision_model(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_populate_billinfo_array(
	pcm_context_t		*ctxp,
	int32			catv_ser_count,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_update_bb_service_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);	
	
static void
fm_mso_set_assoc_bus_profile(
	pcm_context_t		*ctxp,
	poid_t			*billinfo_pdp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);
	
	
static void
fm_mso_cust_bill_supress_profile(
	pcm_context_t	*ctxp, 
	poid_t			*i_flistp,
	pin_errbuf_t	*ebufp);
	
static void
fm_mso_set_bb_login_pass(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_plan_type(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			*customer_type,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);
int
fm_mso_validate_cust_prod(
	pin_decimal_t	*priority,
	int32			*customer_type,
	int32			*acc_pay_type,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_acnt_corp_child_bb(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_populate_billinfo_array_corp(
	pcm_context_t		*ctxp,
	int32			catv_ser_count,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);


int
mso_cust_validate_input_bb_fields(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

void
fm_mso_validate_bb_parent(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);	
/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_CREATE_BB_SERVICE 
 *
 *******************************************************************/
//Input flist
/************************************************************************************
0 PIN_FLD_POID           POID [0] 0.0.0.1 /plan -1 0
0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 325529 0
0 PIN_FLD_BILLINFO      ARRAY [0] allocated 20, used 1
1     PIN_FLD_BILL_WHEN       INT [0] 1
0 PIN_FLD_PAYINFO      ARRAY [0] allocated 20, used 1
1     PIN_FLD_INDICATOR       INT [0] 1
0 PIN_FLD_SERVICES      ARRAY [0] allocated 20, used 4
1     PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband -1 0
1	  PIN_FLD_LOGIN			STR [0] "unique_login"
1     MSO_FLD_BB_INFO    SUBSTRUCT [0] allocated 20, used 3
2         MSO_FLD_SALESMAN_OBJ   POID [0] NULL poid pointer
2         MSO_FLD_TECHNOLOGY   ENUM [0] 1
2         MSO_FLD_NETWORK_ELEMENT   STR [0] "Network Element"
2         MSO_FLD_NETWORK_NODE   ENUM [0] 1
2         MSO_FLD_NETWORK_IP   STR [0] "0.0.0.1"
2		  MSO_FLD_NETWORK_AMPLIFIER STR [0] "Amplifier"
2		  MSO_FLD_IS1CLICK_FLAG 	ENUM [0] 1
2		  PIN_FLD_BILL_WHEN 		INT [0] 3
0 PIN_FLD_PLAN_LIST_CODE SUBSTRUCT [0] allocated 20, used 3
1     PIN_FLD_PLAN          ARRAY [0] allocated 20, used 4
2         PIN_FLD_PURCHASE_END_T TSTAMP [0] (1398796200) Wed Apr 30 00:00:00 2014
2         PIN_FLD_PURCHASE_START_T TSTAMP [0] (1394562600) Wed Mar 12 00:00:00 2014
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 191188 2
2         PIN_FLD_CODE            STR [0] "STB_Zero_Value"
1     PIN_FLD_PLAN          ARRAY [1] allocated 20, used 4
2         PIN_FLD_PURCHASE_END_T TSTAMP [0] (1398796200) Wed Apr 30 00:00:00 2014
2         PIN_FLD_PURCHASE_START_T TSTAMP [0] (1394562600) Wed Mar 12 00:00:00 2014
2         PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 164074 0
2         PIN_FLD_CODE            STR [0] "CATV-Basic Service Tier Package Mumbai"
0 PIN_FLD_BUSINESS_TYPE   ENUM [0] 99011000
*******************************************************************************************/ 
 
 
EXPORT_OP void
op_mso_cust_create_bb_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_create_bb_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);
	

/*******************************************************************
 * MSO_OP_CUST_CREATE_BB_SERVICE  
 *******************************************************************/
void 
op_mso_cust_create_bb_service(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int32			local = LOCAL_TRANS_OPEN_SUCCESS;
	int32			status_uncommitted =2;
	int32			*ret_status = NULL;
	poid_t			*inp_pdp = NULL;
	char			*prog_name = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_CREATE_BB_SERVICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_create_bb_service error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_create_bb_service input flist", i_flistp);
		
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Account obj not found: Error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51412", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Account obj not found: Error", ebufp);
		return;
	}
	local = fm_mso_trans_open(ctxp, inp_pdp, 3,ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
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
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
		return;
	}
		
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/

	 fm_mso_cust_create_bb_service(ctxp, flags, i_flistp, r_flistpp, ebufp);

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	ret_status = (int32*)PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp) || (*(int*)ret_status == 1)) 
	{		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"op_mso_cust_create_bb_service error");
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS  && 
		  !(prog_name && strcmp(prog_name,"pin_deferred_act") ==0)
		  )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
		{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_create_bb_service output flist", *r_flistpp);
	}
	return;
}


/*******************************************************************
 * fm_mso_cust_create_bb_service - Implementation
 *******************************************************************/
static void 
fm_mso_cust_create_bb_service(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp;
	pin_flist_t		*ser_flistp=NULL;
	pin_flist_t		*inher_flistp=NULL;
	pin_flist_t		*bb_info_flistp=NULL;
	pin_flist_t		*mod_in_flistp=NULL;
	pin_flist_t		*mod_out_flistp=NULL;
	pin_flist_t		*ser_out_flistp=NULL;
	pin_flist_t		*tmp_flistp=NULL;
	pin_flist_t		*bi_flistp=NULL;
	pin_flist_t		*payinfo=NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*billinfo_pdp = NULL;

	pin_cookie_t		cookie = NULL;
 	int32			*business_type = NULL;
	int32			*indicator = NULL;
 	int32			bus_type_val = 0;
	int32			account_type = 0;
	int32			account_model = 0;
	int32			subscriber_type = 0;
	int32			service_type = 0;
	int32			errorCode	= 0;
	int32			ret_status	= -1;
	int32			bb_ser_count = 0;
	int32			catv_ser_count = 0;
	

	char			*errorDesc	= NULL;
	char			*errorField	= NULL;
	char			errorMsg[80]	= "";
	char			*prog_name = NULL;
	char			temp[80]	= "";
	char			*bb_service_type = "/service/telco/broadband";
	char			*catv_service_type = "/service/catv";

	int32			srvc_status_active = PIN_STATUS_ACTIVE;
	int32			srvc_status_flags = 0;
	int32			create_service_success = 0;
	int32			create_service_failure = 1;
	int32			prov_action_inactive = 1; 
	int32			prov_action_activate = 2;
	int32			corporate_type = 0;

	int			billing_dom = -1;
	int			elem_id = 0;
	pin_fld_num_t		field_no	= 0;
	char			*order_service_type = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;

	int			rv = 0;
	pin_flist_t		*v_flistp = NULL;

	int64			database = 0;	
	int32			pay_type =0;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*wo_flistp = NULL;
	pin_flist_t		*groupinfo_flistp = NULL;
	pin_flist_t		*pinv_flistp = NULL;
	pin_flist_t		*inh_flistp = NULL;
	
	int			parent_indicator = 0 ;
	char			*s_template = "select X from /payinfo where F1 = V1 ";
	int			srch_flag = SRCH_DISTINCT;
	poid_t			*s_pdp = NULL;
			
	



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_bb_service input flist", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if (!acnt_pdp)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Account Poid", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51701", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing Account Poid", ebufp);
		goto CLEANUP;
	}

	database = PIN_POID_GET_DB(acnt_pdp);	
 	/*******************************************************************
	* Derive account_type,account_model & subscriber_type from business_type 
	*  - Start
	*******************************************************************/
	business_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
	if (business_type)
	{
		bus_type_val = *business_type;
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Business Type", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51702", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing Business Type Value", ebufp);
		goto CLEANUP;
	}
	/***************************************
	 *	First 2 digits: Account Type, 
	 *	3rd and 4th digit: Subscriber Type,
	 *	5th digit : Account Model,
	 *	6th digit : Service Type 
	 *	7th and 8th digit : RESERVED 
	 ***************************************/
	
	account_type = bus_type_val/1000000;
	bus_type_val = bus_type_val%1000000;
	
	subscriber_type = bus_type_val/10000;
	bus_type_val = bus_type_val%10000;
	
	account_model = bus_type_val/1000;
	bus_type_val = bus_type_val%1000;
	
	service_type = bus_type_val/100;
	bus_type_val = bus_type_val%100;

	corporate_type = bus_type_val/10;
	
	/************************************************************************
	* Derive account_type,account_model & subscriber_type from business_type 
	* END
	************************************************************************/
	
	/***********************************************************************
	*  Pavan Bellala : 27/05/2015
	*  Validate input fields before creating service
	***********************************************************************/
	
	rv = mso_cust_validate_input_bb_fields(ctxp,i_flistp,&v_flistp,ebufp);
	if(rv == 1)
	{
		if(PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in validating input", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51702", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in validating input", ebufp);
			goto CLEANUP;
		} else {
			ret_flistp = v_flistp;
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Error in input",ret_flistp);
			goto CLEANUP;
		}

	}

	/* Validate the service type value. Must be ALL(HYBRID) or BB */
	if(service_type!=ALL && service_type!=BB)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid service type.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51704", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service type value must be HYBRID or BB.", ebufp);
		goto CLEANUP;
	}
	/*******************************************************************
	* Activation Validation - Start
	*******************************************************************/
	ret_flistp=NULL;
	fm_mso_validate_acnt_res(ctxp, i_flistp, &ret_flistp, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Validate Account return flist", ret_flistp );
	if (ret_flistp)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == create_service_failure)
		{
			goto CLEANUP;
		}
		else if (ret_status == create_service_success)
		{
			ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
			inher_flistp = PIN_FLIST_SUBSTR_GET(ser_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
			bb_info_flistp = PIN_FLIST_SUBSTR_GET(inher_flistp, MSO_FLD_BB_INFO, 0, ebufp);
			PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_CITY, bb_info_flistp, PIN_FLD_CITY, ebufp );
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		}
		
	} 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist, with CITY", i_flistp );
	/*******************************************************************
	*	Account Status Validation - End
	*******************************************************************/
	
	ret_flistp=NULL;
	//Pawan: Uncommented call to below function since commision 
	//model was not getting populated in service.
	fm_mso_update_commision_model(ctxp, i_flistp, &ret_flistp, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Update Commision model return flist", ret_flistp );
	if (ret_flistp)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == create_service_failure)
		{
			goto CLEANUP;
		}
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	} 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist, with Commision model", i_flistp );

	/*******************************************************************
	* Service Count Validation - Start
	*******************************************************************/
	ret_flistp=NULL;
	bb_ser_count = fm_mso_get_services_info(ctxp, i_flistp, &ret_flistp, bb_service_type, ebufp );
	if(bb_ser_count>0)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Broadband service exist.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51703", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Broadband service already exist.", ebufp);
		goto CLEANUP;
	}
	/*******************************************************************
	* Corporate Child Validation - Start
	*******************************************************************/
	if (account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD )
	{
		fm_mso_validate_acnt_corp_child_bb(ctxp, i_flistp, &ret_flistp, ebufp);
		if (ret_flistp)
		{
			vp = (int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
			if (vp && *(int32*)vp == create_service_failure)
			{
				goto CLEANUP;
			}

		}
	}
	/*************************************************************************
	Pavan Bellala - 12-08-2015
	Added corporate parent validation
	*************************************************************************/
	else if(account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_PARENT )
	{
		ret_flistp = NULL;
		fm_mso_validate_bb_parent(ctxp,i_flistp,&ret_flistp, ebufp);
                if (ret_flistp)
                {
                        vp = (int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
                        if (vp && *(int32*)vp == create_service_failure)
                        {
                                goto CLEANUP;
                        }

                }
	}

	/*******************************************************************
	* Check Plan compatibility with customer type - Start
	*******************************************************************/
	ret_flistp=NULL;
	fm_mso_validate_plan_type(ctxp, i_flistp,&account_type, &ret_flistp, ebufp );
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Validating Plan type.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51733", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Validating Plan type.", ebufp);	
		goto CLEANUP;
	} 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Validate Plan type return flist", ret_flistp );
	if (ret_flistp)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == create_service_failure)
		{
			goto CLEANUP;
		}		
	} 
	
	/*******************************************************************
	* Check Plan compatibility with customer type - END
	*******************************************************************/
	

	catv_ser_count = fm_mso_get_services_info(ctxp, i_flistp, &ret_flistp, catv_service_type, ebufp );
	
	
	/*******************************************************************
	* Populate agreement number.
	*******************************************************************/
	ret_flistp=NULL;
	fm_mso_update_bb_agr_no(ctxp, i_flistp, &ret_flistp, ebufp );
	if (ret_flistp)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", ret_flistp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist, with AGR No.", i_flistp );

	/*******************************************************************
	* Populate service login and password.
	*******************************************************************/
	ret_flistp=NULL;
	fm_mso_set_bb_login_pass(ctxp, i_flistp, &ret_flistp, ebufp );
	if (ret_flistp)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", ret_flistp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist, with login and password.", i_flistp );

	
	/*******************************************************************
	* Populate Statuses array and Telco features array.
	*******************************************************************/
	ret_flistp=NULL;
	fm_mso_update_bb_service_info(ctxp, i_flistp, &ret_flistp, ebufp );
	if (ret_flistp)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", ret_flistp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist, with Status.", i_flistp );	
	
	
	/*************************************************************
	* Validation for PAY Type
	* Start
	*************************************************************/
	payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
	if(!payinfo)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Payinfo/Indicator does not exist", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51723", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Payinfo/Indicator does not exist.", ebufp);
		goto CLEANUP;
	}

 	/*************************************************************
	* Validation for PAY Type
	* End
	*************************************************************/

	/************************************************************************
	Modify balance_group and billinfo
	START
	************************************************************************/
	if(service_type==BB || service_type==ALL)
	{
		if (account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD)
		{
			fm_mso_populate_billinfo_array_corp(ctxp, catv_ser_count,i_flistp, &ret_flistp, ebufp );
			if (ret_flistp)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", ret_flistp);
				goto CLEANUP;
			}			
		}
		else
		{
			//Only Broadband service can be purchased on account. 
			fm_mso_populate_billinfo_array(ctxp, catv_ser_count,i_flistp, &ret_flistp, ebufp );
			if (ret_flistp)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", ret_flistp);
				goto CLEANUP;
			}
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist, with Billinfo.", i_flistp );
	}
	/************************************************************************
	Modify balance_group and billinfo
	END
	************************************************************************/
	
	mod_in_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	
	tmp_flistp = PIN_FLIST_SUBSTR_GET(mod_in_flistp,PIN_FLD_SUPPRESSION_INFO, 1, ebufp);
	if(tmp_flistp) {
		PIN_FLIST_SUBSTR_DROP(mod_in_flistp,PIN_FLD_SUPPRESSION_INFO, ebufp);
	}
	
	tmp_flistp = PIN_FLIST_SUBSTR_GET(mod_in_flistp,MSO_FLD_SUBSCRIPTION_PLANS, 1, ebufp);
	if(tmp_flistp) {
		PIN_FLIST_SUBSTR_DROP(mod_in_flistp,MSO_FLD_SUBSCRIPTION_PLANS, ebufp);
	}
	
	tmp_flistp = PIN_FLIST_SUBSTR_GET(mod_in_flistp,MSO_FLD_INSTALLATION_PLANS, 1, ebufp);
	if(tmp_flistp) {
		PIN_FLIST_SUBSTR_DROP(mod_in_flistp,MSO_FLD_INSTALLATION_PLANS, ebufp);
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CUST_MODIFY_CUSTOMER input flist", mod_in_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_MODIFY_CUSTOMER, 0, mod_in_flistp, &mod_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&mod_in_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CUST_MODIFY_CUSTOMER output flist", mod_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Service Creation.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51724", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Service Creation", ebufp);	
		goto CLEANUP;
	} 
	
	/*Update Associated business profile of billinfo*/
	bi_flistp = PIN_FLIST_ELEM_GET(mod_out_flistp, PIN_FLD_BILLINFO, 0, 0, ebufp);
	billinfo_pdp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_POID, 1, ebufp );
	fm_mso_set_assoc_bus_profile(ctxp, billinfo_pdp, &ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp) ) 
	{	
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in updating associated business profile", ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51719", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating associated business profile.", ebufp);
		goto CLEANUP;
	}
	
	/* Create bill suppression profile for billinfo */
	fm_mso_cust_bill_supress_profile(ctxp, i_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating supresssion_profile object.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51722", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in creating supresssion_profile object.", ebufp);	
		goto CLEANUP;
	} 
	
	/*modify billing dates*/
	
	
	/*Call Manage activation plans to store plans for future purchase*/	
	ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
	ser_out_flistp = PIN_FLIST_ELEM_GET(mod_out_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
	PIN_FLIST_FLD_COPY(ser_out_flistp, PIN_FLD_SERVICE_OBJ, i_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_MANAGE_PLANS_ACTIVATION input flist", i_flistp);
	PCM_OP(ctxp, MSO_OP_CUST_MANAGE_PLANS_ACTIVATION, 0, i_flistp, &ret_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_MANAGE_PLANS_ACTIVATION output flist", ret_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp) ) 
	{	
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in creating Manage Plans Activation object", ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51717", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in creating Manage Plans Activation object.", ebufp);
		goto CLEANUP;
	}
	else 
	{
		ret_status = *(int32 *)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 1, ebufp);
		 PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		if (ret_status == 1)
		goto CLEANUP;
	}

	 PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

	/*********** Pavan Bellala 07-08-2015 ******************************************
	Bug:1062
	Update the indicator field of parent account with that of the child if it has
	no services (MSO_ACCT_ONLY )
	*******************************************************************************/
	if(mod_out_flistp) {
	
		bi_flistp = PIN_FLIST_ELEM_GET(mod_out_flistp, PIN_FLD_BILLINFO, 0, 0, ebufp);
		vp = PIN_FLIST_FLD_GET(bi_flistp,PIN_FLD_PAY_TYPE,1,ebufp);
		if( vp ) pay_type = *(int32 *)vp;

		if(pay_type == PIN_PAY_TYPE_SUBORD) 
		{
			s_flistp = PIN_FLIST_CREATE(ebufp);
			s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);	
		
			PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);	
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	       	 	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template , ebufp);

			groupinfo_flistp = PIN_FLIST_SUBSTR_GET(mod_out_flistp,PIN_FLD_GROUP_INFO,1,ebufp);
			if(groupinfo_flistp != NULL){
				tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS,1,ebufp);
				PIN_FLIST_FLD_COPY(groupinfo_flistp,PIN_FLD_PARENT,tmp_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,ebufp);
				pinv_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp,PIN_FLD_INV_INFO,PIN_ELEMID_ANY,ebufp);
        			PIN_FLIST_FLD_SET(pinv_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
			        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search parent payinfo input", s_flistp);
        			PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &so_flistp, ebufp);

        			PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search parent payinfo output", so_flistp);

        			if(so_flistp && (PIN_FLIST_ELEM_COUNT(so_flistp,PIN_FLD_RESULTS,ebufp)>0))
        			{
                			tmp_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
                			pinv_flistp = PIN_FLIST_ELEM_GET(tmp_flistp,PIN_FLD_INV_INFO,PIN_ELEMID_ANY,1,ebufp);
                			vp = PIN_FLIST_FLD_GET(pinv_flistp,PIN_FLD_INDICATOR,1,ebufp);
                			if(vp != NULL){
                        			parent_indicator = *(int32*)vp;
                        			if(parent_indicator == MSO_ACCT_ONLY )
                        			{
                                			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No indicator set yet for parent.Setting of child's");
							s_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(tmp_flistp,PIN_FLD_POID,s_flistp,PIN_FLD_POID,ebufp);
							pinv_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_INV_INFO,PIN_ELEMID_ANY,ebufp);

							payinfo = PIN_FLIST_ELEM_GET(mod_out_flistp,PIN_FLD_PAYINFO,0,0,ebufp);
							inh_flistp = PIN_FLIST_SUBSTR_GET(payinfo,PIN_FLD_INHERITED_INFO,1,ebufp);
							vp = (pin_flist_t *)PIN_FLIST_ELEM_GET(inh_flistp,PIN_FLD_INV_INFO,0,0,ebufp);
							PIN_FLIST_FLD_COPY(vp, PIN_FLD_INDICATOR, pinv_flistp,PIN_FLD_INDICATOR, ebufp);
							PIN_FLIST_FLD_COPY(payinfo,PIN_FLD_PAYMENT_TERM,s_flistp,PIN_FLD_PAYMENT_TERM,ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "setting parent indicator input",s_flistp);
							PCM_OP(ctxp, PCM_OP_WRITE_FLDS,0,s_flistp,&wo_flistp,ebufp);
							PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "setting parent indicator output",wo_flistp);
							PIN_FLIST_DESTROY_EX(&wo_flistp, NULL);
						} else {
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Nothing to update as parent indicator is not MSO_ACCT_ONLY");
						}
	                                } else {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error in fetching parent indicator");
						goto COMMON_ERR;
                                	}
                        	} else {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error in searching parent indicator");
					goto COMMON_ERR;
				}
				PIN_FLIST_DESTROY_EX(&so_flistp,NULL);
			}
			
		}

	}	

	/*******************************************************************
	* Populate Return Flist for success Case 
	*******************************************************************/
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, ret_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS,&create_service_success, ebufp );
	/*Set Billinfo array */
	bi_flistp = PIN_FLIST_ELEM_GET(mod_out_flistp, PIN_FLD_BILLINFO, 0, 0, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_BILLINFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_BILL_WHEN, tmp_flistp, PIN_FLD_BILL_WHEN, ebufp );
	PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_POID, tmp_flistp, PIN_FLD_POID, ebufp );
	/*Set Services array */
	tmp_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_COPY(ser_out_flistp, PIN_FLD_SERVICE_OBJ, tmp_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
	vp = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(ser_out_flistp, PIN_FLD_INHERITED_INFO, 1, ebufp);
	if (vp)
	{
		vp1 = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(vp, MSO_FLD_BB_INFO, 1, ebufp);
		PIN_FLIST_FLD_COPY(vp1, MSO_FLD_AGREEMENT_NO, tmp_flistp, MSO_FLD_AGREEMENT_NO, ebufp );
	}
	PIN_FLIST_FLD_COPY(ser_out_flistp, PIN_FLD_LOGIN, tmp_flistp, PIN_FLD_LOGIN, ebufp );

	//Lifecycle event creation
	fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_CREATE_BB_SERVICE, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}


	
CLEANUP:
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Final ret_flistp",ret_flistp);
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL); 
	PIN_FLIST_DESTROY_EX(&mod_out_flistp, NULL);
	return;
COMMON_ERR:
                PIN_ERRBUF_RESET(ebufp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in common", ebufp);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51718", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in fetching details.", ebufp);
		*r_flistpp = ret_flistp;
		return;

}


/**************************************************
* Function: fm_mso_validate_acnt_res()
* 
* 
***************************************************/
static void
fm_mso_validate_acnt_res(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_flds_inflist = NULL;
	pin_flist_t		*read_flds_outflist = NULL;
	pin_flist_t		*nameinfo_billing = NULL;
	
	poid_t			*acnt_pdp = NULL;

	int32			activate_customer_failure = 1;
	int32			activate_customer_success = 0;
	int32			*acnt_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_acnt_res", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_acnt_res input flist", in_flistp);	

	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	read_flds_inflist =  PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flds_inflist, PIN_FLD_POID, (void *)acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_flds_inflist, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_inflist, MSO_FLD_AREA, NULL, ebufp);

	nameinfo_billing = PIN_FLIST_ELEM_ADD(read_flds_inflist, PIN_FLD_NAMEINFO, 1, ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist", read_flds_inflist);
	
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_inflist, &read_flds_outflist, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_inflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Getting Account details", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51100", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Getting Account details", ebufp);	
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist", read_flds_outflist);
	acnt_status =  PIN_FLIST_FLD_GET(read_flds_outflist, PIN_FLD_STATUS, 1, ebufp);

	if (acnt_status && (*acnt_status != PIN_STATUS_ACTIVE))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Account Status is INACTIVE", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51101", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Account Status is INACTIVE", ebufp);
		goto CLEANUP;
	} 

	*ret_flistp = PIN_FLIST_COPY(read_flds_outflist, ebufp);
	nameinfo_billing = PIN_FLIST_ELEM_GET(read_flds_outflist, PIN_FLD_NAMEINFO, 1, 0, ebufp);
	PIN_FLIST_FLD_COPY(nameinfo_billing, PIN_FLD_CITY, *ret_flistp, PIN_FLD_CITY, ebufp);
	PIN_FLIST_ELEM_DROP(*ret_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_success, ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&read_flds_outflist, NULL);
	return;
}

/******************************************************
* Function: fm_mso_get_services_info()
* Description: Return the count and services array
*			for active and inactive service for an 
* 			account. Service type specified as input.
*******************************************************/

static int32
fm_mso_get_services_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**service_array,
	char			*ser_type,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	
	int64			db = -1;
	int32			srch_flag = 256;
	int32			count_bb = 0;
	int				service_status_active = PIN_STATUS_ACTIVE;
	int				service_status_inactive = PIN_STATUS_INACTIVE;

	poid_t			*srch_pdp = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*ser_pdp = NULL;
	char			*template = "select X from /service where F1.id = V1 and ( F2 = V2 or F3 = V3 ) and F4.type like V4 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_services_info", ebufp);
	}
	
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"acnt_pdp", acnt_pdp);
	db = PIN_POID_GET_DB(acnt_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &service_status_active, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATUS, &service_status_inactive, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
	ser_pdp = PIN_POID_CREATE(db, ser_type, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, ser_pdp, ebufp);
	PIN_POID_DESTROY(ser_pdp,NULL);
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BB Service Search input flist", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, service_array, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BB Service Search output flist", *service_array);

	count_bb = PIN_FLIST_ELEM_COUNT(*service_array, PIN_FLD_RESULTS, ebufp);
	
	CLEANUP:
	return count_bb;
}


/**************************************************
* Function: fm_mso_update_bb_agr_no()
* 
* 
***************************************************/
static void
fm_mso_update_bb_agr_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*gen_seq_iflist = NULL;
	pin_flist_t		*gen_seq_oflist = NULL;
	pin_flist_t		*services_array = NULL;
	pin_flist_t		*inher_flistp = NULL;
	pin_flist_t		*srvc_bb_info = NULL;

	poid_t			*acnt_pdp = NULL;
	int32			seq = 0;
	int32			activate_customer_failure = 1;
	char			agr_no[20];
	char			*err_code = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_bb_agr_no", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_agr_no input flist", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	/**********************************************************************************
	*  Populate Agreement Number -Start
	***********************************************************************************/
	gen_seq_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(gen_seq_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(gen_seq_iflist, PIN_FLD_NAME, "MSO_SEQUENCE_TYPE_AGREEMENT", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_agr_no gen_seq_iflist", gen_seq_iflist);

	fm_mso_utils_sequence_no(ctxp, gen_seq_iflist, &gen_seq_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&gen_seq_iflist, NULL);
	
	err_code = (char*)PIN_FLIST_FLD_GET(gen_seq_oflist, PIN_FLD_ERROR_CODE, 1, ebufp);
	if ( err_code )
	{
		*ret_flistp = PIN_FLIST_COPY(gen_seq_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&gen_seq_oflist, NULL);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_agr_no gen_seq_oflist", gen_seq_oflist);
	
	seq = *(int32*)PIN_FLIST_FLD_GET(gen_seq_oflist, PIN_FLD_HEADER_NUM, 1, ebufp);
	PIN_FLIST_DESTROY_EX(&gen_seq_oflist, NULL);

	sprintf(agr_no, "%s%s%010d", "BB", "-",seq);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AGR_NO:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, agr_no);

	services_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 0, ebufp); 
	inher_flistp = PIN_FLIST_SUBSTR_GET(services_array, PIN_FLD_INHERITED_INFO, 0, ebufp);
	srvc_bb_info = PIN_FLIST_SUBSTR_GET(inher_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(srvc_bb_info, MSO_FLD_AGREEMENT_NO, agr_no, ebufp);
	
	// Temporarily set login to Agreement no.
	//PIN_FLIST_FLD_SET(services_array, PIN_FLD_LOGIN, agr_no, ebufp);
  	/**********************************************************************************
	*  Populate Agreement Number -End
	***********************************************************************************/
	
	CLEANUP:
	return;
}

/**********************************************************************************
*  Function: fm_mso_store_bb_plan_details
*  Description: Creates an object of type /mso_account_plan_activation
*				to store all the plan details to be purchased
*				after service creation
***********************************************************************************/
static void
fm_mso_store_bb_plan_details(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	int64			db = -1;
	pin_flist_t		*obj_in_flistp = NULL;
	pin_flist_t		*obj_out_flistp = NULL;
	pin_flist_t		*plan_array = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*plan_list_flistp  = NULL;
	pin_flist_t		*plan_flistp  = NULL;

	poid_t			*acnt_pdp = NULL;
	poid_t			*mso_act_pdp = NULL;
	int32			activate_customer_failure = 1;
	int			elem_id = 0;
	int			status_new = 0;
	pin_cookie_t		cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_store_bb_plan_details", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_store_bb_plan_details input flist", i_flistp);

	
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acnt_pdp);
	obj_in_flistp = PIN_FLIST_CREATE(ebufp);
	mso_act_pdp = PIN_POID_CREATE(db, "/mso_account_plan_activation", (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(obj_in_flistp, PIN_FLD_POID, mso_act_pdp,ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_ACCOUNT_OBJ, obj_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	ser_flistp = PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_SERVICES,0,1,ebufp);
	PIN_FLIST_FLD_COPY(ser_flistp,PIN_FLD_SERVICE_OBJ, obj_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_STATUS, (void *)status_new, ebufp);
	
	plan_list_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PLAN_LIST_CODE, 0, ebufp);
	while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_list_flistp, PIN_FLD_PLAN,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_FLIST_ELEM_SET(obj_in_flistp, plan_flistp, PIN_FLD_PLAN, elem_id, ebufp);
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ: mso_account_plan_activation input flist", obj_in_flistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, obj_in_flistp, &obj_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ output flist", obj_out_flistp);
	PIN_FLIST_DESTROY_EX(&obj_in_flistp, NULL);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Can't create /mso_account_plan_activation object ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51705", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Can't create /mso_account_plan_activation object", ebufp);
		goto CLEANUP;
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&obj_out_flistp, NULL);
	return;
}	

/**********************************************************************************
*  Function: fm_mso_update_commision_model
*  Description: Adds the commision model value for a customer 
*		based on the LCO.
*
***********************************************************************************/
static void
fm_mso_update_commision_model(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	int64			db = -1;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*ws_info = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*inher_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	poid_t			*acc_pdp = NULL;
	int32			activate_customer_failure = 1;
	int32			activate_customer_success = 0;
	int32 profile_srch_type = MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_commision_model", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_commision_model input flist", i_flistp);
	
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_POID, acc_pdp, ebufp );
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_PROFILE_DESCR, WHOLESALE, ebufp );
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp );
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_OBJECT, acc_pdp, ebufp );

	fm_mso_get_profile_info(ctxp, srch_in_flistp, &srch_out_flistp, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_commision_model output flist", srch_out_flistp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (!srch_out_flistp)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Wholesale profile not found in BRM", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51706", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Wholesale profile not found for Subscriber LCO", ebufp);
		goto CLEANUP;
	}	
	else 
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_success, ebufp);
		ws_info=PIN_FLIST_SUBSTR_GET(srch_out_flistp, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
		if(ws_info)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "wholesale profile flist", ws_info);
			ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
			inher_flistp = PIN_FLIST_SUBSTR_GET(ser_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
			bb_info_flistp = PIN_FLIST_SUBSTR_GET(inher_flistp, MSO_FLD_BB_INFO, 0, ebufp);
			PIN_FLIST_FLD_COPY(ws_info, MSO_FLD_COMMISION_MODEL, bb_info_flistp, MSO_FLD_COMMISION_MODEL, ebufp);
		}
	}
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}	

/**********************************************************************************
*  Function: fm_mso_populate_billinfo_array
*  Description: Update/populates the billinfo and balgroup
*				array in input flist.
*
***********************************************************************************/
static void
fm_mso_populate_billinfo_array(
	pcm_context_t		*ctxp,
	int32			catv_ser_count,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	int64			db = -1;
	pin_flist_t		*bg_out_flist = NULL;
	pin_flist_t		*bi_out_flist = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*billinfo = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*inher_info = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*read_in_flistp = NULL;
	pin_flist_t		*read_out_flistp = NULL;
	
	poid_t			*acc_pdp = NULL;
	poid_t			*bg_pdp = NULL;
	poid_t			*bi_pdp = NULL;
	int32			activate_customer_failure = 1;
	int32			activate_customer_success = 0;
	int32			pay_type = PIN_PAY_TYPE_INVOICE;
	int32			*indicator = NULL;
	int32			ind_val = 0;
	int32			pay_term = 0;
	char			*bi_id=0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_populate_billinfo_array", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_billinfo_array input flist", i_flistp);
	
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp);
	/* Fetch the existing billinfo poid */
	fm_mso_read_bal_grp(ctxp, acc_pdp, NULL, &bg_out_flist, ebufp);
	bg_pdp = PIN_FLIST_FLD_GET(bg_out_flist, PIN_FLD_POID, 0, ebufp );
	fm_mso_get_billinfo(ctxp, bg_pdp, &bi_pdp, ebufp );
	fm_mso_read_billinfo(ctxp, bi_pdp, &bi_out_flist, ebufp );
	PIN_FLIST_DESTROY_EX(&bg_out_flist, NULL);
	//PIN_POID_DESTROY(bg_pdp,ebufp);
	if(catv_ser_count==0)
	{
		/* If no service exist then set existing billinfo poid in input flist */
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_POID, bi_pdp, ebufp);
	}
	else if(catv_ser_count>0)
	{
		/* Service already exists. Create new billinfo */
		PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);
	}
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "BB", ebufp);
	/* Copy payinfo poid in BILLINFO array */
	PIN_FLIST_FLD_COPY(bi_out_flist, PIN_FLD_PAYINFO_OBJ, billinfo, PIN_FLD_PAYINFO_OBJ, ebufp);

	/* Set the BAL_INFO array with elem id as 0 to associate 
		service level balgroup  to this billinfo*/
	PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);	
	
	/* Read Payinfo object */
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(bi_out_flist, PIN_FLD_PAYINFO_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_OBJ output flist", read_out_flistp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	
	/* Copy payinfo poid in PAYINFO array */
	payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
	indicator = PIN_FLIST_FLD_TAKE(payinfo, PIN_FLD_INDICATOR, 1, ebufp);
	PIN_FLIST_FLD_COPY(bi_out_flist, PIN_FLD_PAYINFO_OBJ, payinfo, PIN_FLD_POID, ebufp);
	if(*indicator == TYPE_BB_PREPAID)
	{
		pay_term = PAY_TERM_BB_PREPAID;
	}
	else
	{
		pay_term = PAY_TERM_BB_POSTPAID;
	}
	PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAYMENT_TERM, &pay_term, ebufp);
	PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
	inher_info = PIN_FLIST_SUBSTR_ADD(payinfo,PIN_FLD_INHERITED_INFO, ebufp);
	inv_info = PIN_FLIST_ELEM_ADD(inher_info,PIN_FLD_INV_INFO, 0, ebufp);
	ind_val = *indicator;
	PIN_FLIST_FLD_PUT(inv_info, PIN_FLD_INDICATOR, indicator, ebufp);
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	
	/* Add BAL_INFO array in services array with ELEM ID 0 */
	ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	PIN_FLIST_ELEM_SET(ser_flistp, NULL,PIN_FLD_BAL_INFO, 0, ebufp );
	
	/* Add Indicator in BB INFO */
	inher_info = PIN_FLIST_SUBSTR_GET(ser_flistp,PIN_FLD_INHERITED_INFO, 1, ebufp);
	inv_info = PIN_FLIST_SUBSTR_GET(inher_info,MSO_FLD_BB_INFO, 1, ebufp);
	if(inv_info) {
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INDICATOR, &ind_val, ebufp);
	}
	/*Pawan:03-03-2015 Added logic to fetch the existing BB Balance group */
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, bi_pdp, ebufp );
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_OBJ output flist", read_out_flistp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	bi_id = (char *)PIN_FLIST_FLD_GET(read_out_flistp, PIN_FLD_BILLINFO_ID, 1, ebufp);
	if(bi_id && strcmp(bi_id,"BB")==0) {
		bg_pdp = PIN_FLIST_FLD_TAKE(read_out_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
	}
	
	/* Add BAL_INFO array in input flist with ELEM ID 0 */
	bal_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
	if(bi_id && strcmp(bi_id,"BB")==0) {
		PIN_FLIST_FLD_PUT(bal_flistp, PIN_FLD_POID, bg_pdp, ebufp);
	} else {
		PIN_FLIST_FLD_PUT(bal_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
	}
	
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_NAME, "BB Service Balance Group", ebufp);
	PIN_FLIST_ELEM_SET(bal_flistp, NULL, PIN_FLD_BILLINFO, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in populating Billinfo and Bal group", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51707", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in populating Billinfo and Bal group", ebufp);
		goto CLEANUP;
	}	

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&bi_out_flist, NULL);
	return;
}

/**********************************************************************************
*  Function: fm_mso_populate_billinfo_array_corp
*  Description: Update/populates the billinfo and balgroup
*				array in input flist.
*
***********************************************************************************/
static void
fm_mso_populate_billinfo_array_corp(
	pcm_context_t		*ctxp,
	int32			catv_ser_count,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	int64			db = -1;
	pin_flist_t		*bg_out_flist = NULL;
	pin_flist_t		*bi_out_flist = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*billinfo = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*inher_info = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*read_in_flistp = NULL;
	pin_flist_t		*read_out_flistp = NULL;
	pin_flist_t		*parent_billinfo_details = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*group_info = NULL;
	pin_flist_t		*all_billinfo = NULL;
	
	poid_t			*acc_pdp = NULL;
	poid_t			*bg_pdp = NULL;
	poid_t			*bi_pdp = NULL;
	poid_t			*corp_parent_acnt = NULL;
	poid_t			*corp_parent_billinfo_pdp = NULL;

	int32			activate_customer_failure = 1;
	int32			activate_customer_success = 0;
	//int32			pay_type = PIN_PAY_TYPE_INVOICE;
	int32			*pay_type = NULL;
	int32			*indicator = NULL;
	int32			ind_val = 0;  
	int32			billinfo_type = BB;
	int32			flag = 1;
	int32			pay_term = 0;
	char			*bi_id=0;
	char			msg[100];

	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_populate_billinfo_array_corp", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_billinfo_array_corp input flist", i_flistp);

//	corp_child_acnt = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	group_info = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 0, ebufp);
	corp_parent_acnt = PIN_FLIST_FLD_GET(group_info, PIN_FLD_PARENT, 0, ebufp);
	
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp);
	pay_type = PIN_FLIST_FLD_GET(billinfo, PIN_FLD_PAY_TYPE, 1, ebufp);

	/* Fetch the existing billinfo poid */
	/*fm_mso_read_bal_grp(ctxp, acc_pdp, NULL, &bg_out_flist, ebufp);
	bg_pdp = PIN_FLIST_FLD_GET(bg_out_flist, PIN_FLD_POID, 0, ebufp );
	fm_mso_get_billinfo(ctxp, bg_pdp, &bi_pdp, ebufp );
	fm_mso_read_billinfo(ctxp, bi_pdp, &bi_out_flist, ebufp );
	PIN_FLIST_DESTROY_EX(&bg_out_flist, NULL);*/
	fm_mso_get_all_billinfo(ctxp, acc_pdp, BB, &all_billinfo, ebufp);
	if (all_billinfo)
	{
		bi_out_flist = PIN_FLIST_ELEM_GET(all_billinfo, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if (bi_out_flist)
		{
			bg_pdp =  PIN_FLIST_FLD_GET(bi_out_flist, PIN_FLD_BAL_GRP_OBJ, 1, ebufp );
			bi_id = PIN_FLIST_FLD_GET(bi_out_flist, PIN_FLD_BILLINFO_ID, 1, ebufp);
		}
	}
	//PIN_POID_DESTROY(bg_pdp,ebufp);
	if(catv_ser_count==0)
	{
		/* If no service exist then set existing billinfo poid in input flist */
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_POID, bi_pdp, ebufp);
	}
	else if(catv_ser_count>0)
	{
		/* Service already exists. Create new billinfo */
		PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp), ebufp);
	}
	if(pay_type && *pay_type == PIN_PAY_TYPE_SUBORD )  //PIN_PAY_TYPE_SUBORD = 10007
	{
		flag = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Non paying child encountered");
		fm_mso_get_all_billinfo(ctxp, corp_parent_acnt, billinfo_type, &parent_billinfo_details, ebufp); 

		result_flist = PIN_FLIST_ELEM_GET(parent_billinfo_details, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (result_flist)
		{
			//parent_billinfo_id = (char *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_BILLINFO_ID, 0, ebufp );
			corp_parent_billinfo_pdp = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp);
		}
		else
		{
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51065", ebufp);
			strcpy(msg, "Parent not allowed to activate child for '" );
			//strcat(msg,billinfo_id);
			strcat(msg,"' service. Proceed after modifying parent");
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,msg, ebufp);
			PIN_ERRBUF_RESET(ebufp);
			goto CLEANUP;
		}		
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PARENT_BILLINFO_OBJ, corp_parent_billinfo_pdp, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_AR_BILLINFO_OBJ, corp_parent_billinfo_pdp, ebufp);

		/** Set payinfo poid instead of account poid **********/
		//PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAYINFO_OBJ, corp_parent_acnt, ebufp);		
		PIN_FLIST_FLD_COPY(bi_out_flist,PIN_FLD_PAYINFO_OBJ,billinfo, PIN_FLD_PAYINFO_OBJ, ebufp);

		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, pay_type, ebufp);

		vp = PIN_FLIST_FLD_GET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, 1, ebufp);
		if (vp)
		{
			PIN_FLIST_FLD_DROP(billinfo, PIN_FLD_ACTG_FUTURE_DOM,  ebufp);
		}
		
		vp = PIN_FLIST_FLD_GET(billinfo, PIN_FLD_BILL_WHEN, 1, ebufp);
		if (vp)
		{
			PIN_FLIST_FLD_DROP(billinfo, PIN_FLD_BILL_WHEN,  ebufp);
		}

		/****** Pavan Bellala 07-08-2015 ********
		Commented to have payinfo array
		****************************************/
		/*
		payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp );
		if (payinfo)
		{
			PIN_FLIST_ELEM_DROP(i_flistp, PIN_FLD_PAYINFO, 0, ebufp);
		}
		*/
		PIN_FLIST_FLD_COPY(bi_out_flist, PIN_FLD_PAYINFO_OBJ, billinfo, PIN_FLD_PAYINFO_OBJ, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "BB", ebufp);
	
		/* Set the BAL_INFO array with elem id as 0 to associate 
			service level balgroup  to this billinfo*/
		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);	
	}
	else
	{
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, pay_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "BB", ebufp);
		/* Copy payinfo poid in BILLINFO array */
		PIN_FLIST_FLD_COPY(bi_out_flist, PIN_FLD_PAYINFO_OBJ, billinfo, PIN_FLD_PAYINFO_OBJ, ebufp);

		/* Set the BAL_INFO array with elem id as 0 to associate 
			service level balgroup  to this billinfo*/
		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_BAL_INFO, 0, ebufp);	
	}

	
	/* Read Payinfo object */
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(bi_out_flist, PIN_FLD_PAYINFO_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_OBJ output flist", read_out_flistp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	
	/* Copy payinfo poid in PAYINFO array */
	//Pavan Bellala - Commented flag to set indicator for all accounts 
	//if (flag)
	//{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Input flist here ",i_flistp);
		payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
		indicator = PIN_FLIST_FLD_TAKE(payinfo, PIN_FLD_INDICATOR, 1, ebufp);
		PIN_FLIST_FLD_COPY(read_out_flistp, PIN_FLD_POID, payinfo, PIN_FLD_POID, ebufp);
		if(*indicator == TYPE_BB_PREPAID)
		{
			pay_term = PAY_TERM_BB_PREPAID;
		}
		else
		{
			pay_term = PAY_TERM_BB_POSTPAID;
		}
		PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAYMENT_TERM, &pay_term, ebufp);
		//PIN_FLIST_FLD_COPY(read_out_flistp, PIN_FLD_PAYMENT_TERM, payinfo, PIN_FLD_PAYMENT_TERM, ebufp);
		PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAY_TYPE, pay_type, ebufp);
		inher_info = PIN_FLIST_SUBSTR_ADD(payinfo,PIN_FLD_INHERITED_INFO, ebufp);
		inv_info = PIN_FLIST_ELEM_ADD(inher_info,PIN_FLD_INV_INFO, 0, ebufp);
		ind_val = *indicator;
		PIN_FLIST_FLD_PUT(inv_info, PIN_FLD_INDICATOR, indicator, ebufp);
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	//}
	
	/* Add BAL_INFO array in services array with ELEM ID 0 */
	ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	PIN_FLIST_ELEM_SET(ser_flistp, NULL,PIN_FLD_BAL_INFO, 0, ebufp );
	
	/* Add Indicator in BB INFO */
	inher_info = PIN_FLIST_SUBSTR_GET(ser_flistp,PIN_FLD_INHERITED_INFO, 1, ebufp);
	inv_info = PIN_FLIST_SUBSTR_GET(inher_info,MSO_FLD_BB_INFO, 1, ebufp);
	if(inv_info) {
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INDICATOR, &ind_val, ebufp);
	}
	/*Pawan:03-03-2015 Added logic to fetch the existing BB Balance group */
	/*read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, bi_pdp, ebufp );
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_OBJ output flist", read_out_flistp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	bi_id = (char *)PIN_FLIST_FLD_GET(read_out_flistp, PIN_FLD_BILLINFO_ID, 1, ebufp);

	if(bi_id && strcmp(bi_id,"BB")==0) {
		bg_pdp = PIN_FLIST_FLD_TAKE(read_out_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
	}  */
	
	/* Add BAL_INFO array in input flist with ELEM ID 0 */
	bal_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
	if(bi_id && strcmp(bi_id,"BB")==0) {
		PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_POID, bg_pdp, ebufp);
	} else {
		PIN_FLIST_FLD_PUT(bal_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
	}
	
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_NAME, "BB Service Balance Group", ebufp);
	PIN_FLIST_ELEM_SET(bal_flistp, NULL, PIN_FLD_BILLINFO, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in populating Billinfo and Bal group", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51707", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in populating Billinfo and Bal group", ebufp);
		goto CLEANUP;
	}	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_billinfo_array_corp modified input flist at end of function", i_flistp);
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&all_billinfo, NULL);
	PIN_FLIST_DESTROY_EX(&parent_billinfo_details, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_populate_billinfo_array_corp return flist",*ret_flistp);
	return;
}	


/**************************************************
* Function: fm_mso_update_bb_service_info()
* Description: This function does following:
* 		1. Add STATUSES array
*		2. Add Telco features array
***************************************************/
static void
fm_mso_update_bb_service_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*inher_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	pin_flist_t		*status_flistp = NULL;
	pin_flist_t		*telco_flistp = NULL;
	poid_t			*acc_pdp = NULL;

	int32			activate_customer_failure = 1;
	int32			service_status_inactive = PIN_STATUS_INACTIVE;
	int32			service_status_flags = 0;
	int32			prov_status_new = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_bb_service_info", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_service_info input flist", i_flistp);
	
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	/* Set service status in PIN_FLD_STATUSES */
	ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp); 
	status_flistp = PIN_FLIST_ELEM_ADD(ser_flistp, PIN_FLD_STATUSES, 0, ebufp);
	PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS, &service_status_inactive, ebufp);
	PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS_FLAGS,  &service_status_flags, ebufp);
	
	/* Set provisioning status in PIN_FLD_TELCO_FEATURES */
	inher_flistp = PIN_FLIST_SUBSTR_GET(ser_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
	telco_flistp = PIN_FLIST_ELEM_ADD(inher_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp);
	PIN_FLIST_FLD_SET(telco_flistp, PIN_FLD_NAME, "Provisioning Status", ebufp);
	PIN_FLIST_FLD_SET(telco_flistp, PIN_FLD_STATUS,  &prov_status_new, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in populating Status array.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51708", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in populating Status array.", ebufp);
	}
	return;
}

/*********************************************************
* Function: fm_mso_set_assoc_bus_profile()
* Description: This function changes the associated 
* 			business profile of billinfo. The new profile
*			is evaluated as per rules in iscript.
**********************************************************/
static void
fm_mso_set_assoc_bus_profile(
	pcm_context_t		*ctxp,
	poid_t			*billinfo_pdp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*bus_prof_pdp = NULL;
	
	int64			db = -1;
	int32			srch_flag = 256;
	int32			one = 1;
	char			*template = "select X from /config/business_profile where F1.type like V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_set_assoc_bus_profile", ebufp);
	}
	
	if(!billinfo_pdp)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Billinfo poid is NULL.", ebufp);
		return;
	}
	
	db = PIN_POID_GET_DB(billinfo_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	
	bus_prof_pdp = PIN_POID_CREATE(db, "/config/business_profile", -1, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, bus_prof_pdp, ebufp);
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Config bus profile Search input", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Config bus profile Search output", srch_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	/***************************************************************************
	* Take any valid /config/business_profile poid to pass 
	*  in SET_ASSOCIATED_BUS_PROFILE opcode. The input value is ignored
	*  and the billinfo is evaluated as per logic defind in Bus profile iScript.
	*****************************************************************************/
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp );
	/* Prepare flist for PCM_OP_CUST_SET_ASSOCIATED_BUS_PROFILE */
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, srch_in_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &one, ebufp);
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILLINFO_OBJ, billinfo_pdp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	/* Call PCM_OP_CUST_SET_ASSOCIATED_BUS_PROFILE */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set Associated Bus Profile input", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_SET_ASSOCIATED_BUS_PROFILE, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set Associated Bus Profile output", srch_out_flistp);	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SET_ASSOCIATED_BUS_PROFILE", ebufp);
		goto CLEANUP;
	}
	CLEANUP:
	PIN_POID_DESTROY(bus_prof_pdp,NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
	}


/**************************************************
 * Function: fm_mso_cust_bill_supress_profile()
 * Description:
 *		Creates the /mso_bill_suppression_profile
 * 		for each billinfo.
 ***************************************************/
static void
fm_mso_cust_bill_supress_profile(
	pcm_context_t	*ctxp, 
	poid_t			*i_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*bi_flistp = NULL;
	pin_flist_t		*billinfo = NULL;
	pin_flist_t		*obj_in_flistp = NULL;
	pin_flist_t		*obj_out_flistp = NULL;
	pin_cookie_t	cookie=NULL;
	poid_t			*pdp = NULL;
	poid_t			*acc_pdp = NULL;

	int64			db=0;
	int32			flag_zero=0;
	int				found=0;
	int				elem_id=0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_cust_bill_supress_profile", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_bill_supress_profile input flist.", i_flistp);
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	
	/* Loop though each billinfo in input flist */ 	
	//while((bi_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BILLINFO,
	//&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	//{
	
	bi_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_SUPPRESSION_INFO, 1, ebufp);
	billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 0, ebufp);
	obj_in_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/mso_bill_suppression_profile", -1, ebufp);
	PIN_FLIST_FLD_PUT(obj_in_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, obj_in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_POID, obj_in_flistp, PIN_FLD_BILLINFO_OBJ, ebufp);
	
	found = PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_FLAGS, obj_in_flistp, PIN_FLD_FLAGS, ebufp);
	// If PIN_FLD_FLAG does not exist then set to zero
	if(!found) { 
		PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_FLAGS, (void *)&flag_zero, ebufp);
	}
	
	found = 0;
	found = PIN_FLIST_FLD_COPY(bi_flistp, PIN_FLD_REASON, obj_in_flistp, PIN_FLD_REASON, ebufp);
	// If PIN_FLD_REASON does not exist then set to zero
	if(!found) {
		PIN_FLIST_FLD_SET(obj_in_flistp, PIN_FLD_REASON, (void *)&flag_zero, ebufp);
	}
	
	// Create /mso_bill_suppression_profile Object for billinfo
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Create /mso_bill_suppression_profile Object input flist.", obj_in_flistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, obj_in_flistp, &obj_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&obj_in_flistp, NULL);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Create /mso_bill_suppression_profile Object output flist.", obj_out_flistp);
	PIN_FLIST_DESTROY_EX(&obj_out_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_bill_supress_profile: Error in creating suppression profile object", ebufp);
		goto cleanup;
	}
	//}

	cleanup:
		PIN_FLIST_DESTROY_EX(&obj_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&obj_out_flistp, NULL);
	return;
}

/*********************************************************
* Function: fm_mso_set_bb_login_pass
* Description: This function set the login and password
* 			for broadband service.
*			
**********************************************************/
static void
fm_mso_set_bb_login_pass(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	pin_flist_t		*inh_flistp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*acc_pdp = NULL;
	int32			activate_customer_failure = 1;
	
	int64			db = -1;
	int32			srch_flag = 256;
	int32			one = 1;
	char			*template = "select X from /mso_customer_credential where F1 = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_set_bb_login_pass", ebufp);
	}
	

	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_customer_credential Search input", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_customer_credential Search output", srch_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp );

	ser_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	//PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_LOGIN, ser_flistp, PIN_FLD_LOGIN, ebufp);
	PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PASSWD, ser_flistp, PIN_FLD_PASSWD_CLEAR, ebufp);
	PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PASSWD_STATUS, ser_flistp, PIN_FLD_PASSWD_STATUS, ebufp);
	inh_flistp = PIN_FLIST_SUBSTR_GET(ser_flistp, PIN_FLD_INHERITED_INFO, 1, ebufp);
	bb_info_flistp = PIN_FLIST_SUBSTR_GET(inh_flistp, MSO_FLD_BB_INFO, 1, ebufp);
	PIN_FLIST_FLD_MOVE(ser_flistp, MSO_FLD_PASSWORD, bb_info_flistp, MSO_FLD_PASSWORD, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in populating Login and password.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51731", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in populating Login and password.", ebufp);
	}
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}


/*********************************************************
* Function: fm_mso_validate_plan_type
* Description: This function validates the product to be
* 			purchased against the customer type.
*			
**********************************************************/
static void
fm_mso_validate_plan_type(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			*customer_type,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*subs_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*pay_flistp = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_decimal_t	*priority = NULL;
	int32			*indicator = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*acc_pdp = NULL;	
	int64			db = -1;
	int32			srch_flag = 256;
	int				failure = 1;
	int				ret_val = -1;
	char			*template = "select X from /product 1, /deal 2, /plan 3 where 3.F1 = V1 and 2.F2 = 3.F3 and 2.F4 = 1.F5 order by 1.F6 asc ";
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_plan_type", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_plan_type input flist.", i_flistp);
	
	subs_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_SUBSCRIPTION_PLANS, 1, ebufp);
	plan_flistp = PIN_FLIST_ELEM_GET(subs_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	
	db = PIN_POID_GET_DB(acc_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_PLAN_OBJ, arg_flistp, PIN_FLD_POID, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
	ser_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 4, ebufp );
	ser_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_PRODUCTS, 0, ebufp );
	PIN_FLIST_FLD_SET(ser_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 5, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 6, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);
	
	result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Product Search input", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Product Search input", srch_out_flistp);
	result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp );
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No products found for this plan", ebufp);
		return;
	}
	priority = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRIORITY, 0, ebufp);
	pay_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 0, ebufp);
	indicator = PIN_FLIST_FLD_GET(pay_flistp, PIN_FLD_INDICATOR, 0, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Test");
	
	ret_val = fm_mso_validate_cust_prod(priority,customer_type,indicator,ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in product priority validation", ebufp);
		return;
	}
	if (ret_val == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Plan type does not match customer type.");
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acc_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51732", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plan type does not match customer type.", ebufp);
	}
}


/*********************************************************
* Function: fm_mso_validate_cust_prod
* Description: This function validates the priority of 
*			product to be purchased against the customer 
*			type and payinfo indicator.
*			
**********************************************************/
int
fm_mso_validate_cust_prod(
	pin_decimal_t	*priority,
	int32			*customer_type,
	int32			*acc_pay_type,
	pin_errbuf_t		*ebufp)
{

	char			*tmp_str = NULL;
	char			msg[200]="";
	int32			priority_int=0;
	int32			temp_pri=-1;
	int32			new_plan_type=-1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_cust_prod", ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Test");
	tmp_str = pbo_decimal_to_str(priority, ebufp);
	priority_int = atoi(tmp_str);
	sprintf(msg , " Priority is: %d  and cust type is %d " , priority_int,*(int32 *)customer_type);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	/* Check if the product priority falls with range specified for each cust type. */
	if ( *(int32 *)customer_type == ACCT_TYPE_CORP_SUBS_BB )
	{	
		if ( priority_int < BB_SUB_COR_PRI_START || priority_int > BB_SUB_COR_PRI_END )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_CORP");
			return 1;
		}
	}
	/**** Commented cyber cafe ************
	else if ( *(int32 *)customer_type == ACCT_TYPE_CYBER_CAFE_BB )
	{
		if ( priority_int < BB_SUB_CYB_PRI_START || priority_int > BB_SUB_RET_PRI_END )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_CYB");
			return 1;
		}
	}
	***************************************/
	else if ( *(int32 *)customer_type == ACCT_TYPE_RETAIL_BB )
	{
		if ( priority_int < BB_SUB_RET_PRI_START || priority_int > BB_SUB_RET_PRI_END )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_RET");
			return 1;
		}
	}
	else if ( *(int32 *)customer_type == ACCT_TYPE_SME_SUBS_BB )
	{
		if ( priority_int < BB_SUB_SME_PRI_START || priority_int > BB_SUB_SME_PRI_END )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_SME");
			return 1;
		}
	}

	/* Find the New plan type: Prepaid/Postpaid */
	temp_pri=priority_int%1000;
	if(temp_pri < 500) // New plan type if prepaid.
	{
		new_plan_type = TYPE_BB_PREPAID;
	} 
	else // temp_pri >=500; New plan type is postpaid.
	{
		new_plan_type = TYPE_BB_POSTPAID;
	}
	
	/* Match the payinfo indicator with plan type*/
	if(acc_pay_type && *acc_pay_type != new_plan_type)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Cust type NOT equal to Plan Type");		
		if(new_plan_type == TYPE_BB_PREPAID)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "New Plan is Prepaid and account is postpaid");
			return 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "New Plan is Postpaid and account is prrepaid");
			return 1;
		}
	}
	
	return 0;
}
		


/**************************************************
* Function: fm_mso_validate_acnt_corp_child_bb()
* 
* 
***************************************************/
static void
fm_mso_validate_acnt_corp_child_bb(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_inflist = NULL;
	pin_flist_t		*srch_outflist = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*group_info = NULL;
	pin_flist_t		*nameinfo_billing = NULL;
	
	poid_t			*srch_pdp = NULL;
	poid_t			*corp_child_acnt = NULL;
	poid_t			*corp_parent_acnt = NULL;
	poid_t			*result_acnt_pdp = NULL;

	int32			activate_customer_failure = 1;
	int32			activate_customer_success = 0;
	int32			*acnt_status = NULL;
	int32			srch_flags = 768;
	int32			no_of_accounts = 0;
	int32			elem_id =0;
	int32			acnt_is_child =0;

	char			*template = "select X from /account where F1.id = V1 or F2.id = V2 ";
	pin_cookie_t		cookie = NULL;

	int64			db = -1;
	
	char			*pay_templatep = "select X from /payinfo where F1 = V1 ";
	void 			*vp = NULL;
	char			msg[100];
	int			parent_indicator = -2;
	int			child_indicator = -2;
	int			paytype = 0;
	pin_flist_t		*payinfo_flistp = NULL;
	pin_flist_t		*parent_flistp = NULL;
	pin_flist_t		*billinfo_flistp = NULL;
	pin_flist_t		*arg1_flistp= NULL;

	


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_acnt_corp_child_bb", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_acnt_corp_child_bb input flist", in_flistp);

	corp_child_acnt = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	group_info = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_GROUP_INFO, 0, ebufp);
	corp_parent_acnt = PIN_FLIST_FLD_GET(group_info, PIN_FLD_PARENT, 0, ebufp);

	db = PIN_POID_GET_DB(corp_child_acnt);

	srch_inflist =  PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_inflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, corp_parent_acnt, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, corp_child_acnt, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_AREA, NULL, ebufp);

	nameinfo_billing = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, 1, ebufp);
	PIN_FLIST_FLD_SET(nameinfo_billing, PIN_FLD_CITY, NULL, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_acnt_corp_child_bb search input", srch_inflist);
	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_inflist, &srch_outflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_inflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: fm_mso_validate_acnt_corp_child_bb()", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, corp_child_acnt, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51102", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: fm_mso_validate_acnt_corp_child_bb()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_acnt_corp_child_bb search output", srch_outflist);

	no_of_accounts = PIN_FLIST_ELEM_COUNT(srch_outflist, PIN_FLD_RESULTS, ebufp);
	if (no_of_accounts !=2 )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Either Child or Parent does not exist", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, corp_child_acnt, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51103", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Either Child or Parent does not exist", ebufp);
		goto CLEANUP;
	}

	while((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_outflist, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		result_acnt_pdp = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp);
		if (PIN_POID_COMPARE(result_acnt_pdp, corp_child_acnt, 0, ebufp) == 0)
		{
			acnt_is_child =1;
			*ret_flistp = PIN_FLIST_COPY(result_flist, ebufp);
			nameinfo_billing = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_NAMEINFO, 1,0, ebufp);
			PIN_FLIST_FLD_COPY(nameinfo_billing, PIN_FLD_CITY, *ret_flistp, PIN_FLD_CITY, ebufp);
			PIN_FLIST_ELEM_DROP(*ret_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_success, ebufp);

			acnt_status =  PIN_FLIST_FLD_GET(result_flist, PIN_FLD_STATUS, 0, ebufp);
			if (acnt_status && (*acnt_status != PIN_STATUS_ACTIVE) && acnt_is_child)
			{
				PIN_FLIST_DESTROY_EX(ret_flistp, NULL);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Child Account Status is INACTIVE", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, corp_child_acnt, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51104", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Child Account Status is INACTIVE", ebufp);
				goto CLEANUP;
			}

		} else {
			//Parent account
			acnt_status =  PIN_FLIST_FLD_GET(result_flist, PIN_FLD_STATUS, 0, ebufp);	
			if (acnt_status && (*acnt_status != PIN_STATUS_ACTIVE) )
			{
				PIN_FLIST_DESTROY_EX(ret_flistp, NULL);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Parent Account Status is INACTIVE", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, corp_parent_acnt, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51105", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Parent Account Status is INACTIVE", ebufp);
				goto CLEANUP;
			}

		}
	}
	PIN_FLIST_DESTROY_EX(&srch_outflist,NULL);
	/******** Pavan Bellala 07-08-2015 ***********************************************
	Bug: 1062
	Add functionality to check if parent/child hierarchy is valid
	or not.
	|---------------------------------------------------------------------------------|
	| Parent     |           Child 1               |	   Child 2                |
	|---------------------------------------------------------------------------------|
	| No service |          Postpaid               |   No child                       |
	| No service |          Prepaid                |   No child                       |
	| No service |          Postpaid               |   Prepaid (only if paid by self) |
	| Postpaid   |	Prepaid (only if paid by self) |   No child                       |
	| Postpaid   |		Postpaid 	       |   No child                       |   
	| Postpaid   |	 	Postpaid	       |   Prepaid (only if paid by self) |
	| Prepaid    |		  Prepaid    	       |   No child			  |
	| Prepaid    |	Postpaid (only if paid by self)|   No child			  |
	| Prepaid    |	Postpaid (only if paid by self)|   Prepaid			  |
	|---------------------------------------------------------------------------------|
	*************************************************************************************/

        srch_inflist =  PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_inflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
        PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_TEMPLATE, pay_templatep, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, corp_parent_acnt, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_RESULTS, 0, ebufp);
	arg1_flistp = PIN_FLIST_ELEM_ADD(arg_flist,PIN_FLD_INV_INFO,PIN_ELEMID_ANY,ebufp);
        PIN_FLIST_FLD_SET(arg1_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
	
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search parent payinfo input", srch_inflist);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_inflist, &srch_outflist, ebufp);

        PIN_FLIST_DESTROY_EX(&srch_inflist, NULL);	
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search parent payinfo output", srch_outflist);
	
	if(srch_outflist && (PIN_FLIST_ELEM_COUNT(srch_outflist,PIN_FLD_RESULTS,ebufp)>0))
	{
		arg_flist = PIN_FLIST_ELEM_GET(srch_outflist,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
		arg1_flistp = PIN_FLIST_ELEM_GET(arg_flist,PIN_FLD_INV_INFO,PIN_ELEMID_ANY,1,ebufp);
		vp = PIN_FLIST_FLD_GET(arg1_flistp,PIN_FLD_INDICATOR,1,ebufp);
		if(vp != NULL){
			parent_indicator = *(int32*)vp;
			if(parent_indicator == MSO_ACCT_ONLY )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No indicator set yet for parent.Valid for first child");
                		if(*ret_flistp != NULL){
			        	parent_flistp = PIN_FLIST_ELEM_ADD(*ret_flistp,PIN_FLD_PARENT,0,ebufp);
                        		PIN_FLIST_FLD_SET(parent_flistp,PIN_FLD_INDICATOR,&parent_indicator,ebufp);
				}
                        	goto CLEANUP;
			}
		
		}
		PIN_FLIST_DESTROY_EX(&srch_outflist,NULL);
	} else {
		PIN_FLIST_DESTROY_EX(ret_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Parent Account payinfo not found", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, corp_parent_acnt, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51106", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Parent Account payinfo not found", ebufp);
		goto CLEANUP;
        }

	payinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PAYINFO,0,1,ebufp);
	if(payinfo_flistp != NULL)
	{
		vp = PIN_FLIST_FLD_GET(payinfo_flistp, PIN_FLD_INDICATOR,1,ebufp);
		if(vp != NULL){
			child_indicator = *(int32*)vp;
		}		
	} else {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Account payinfo not found in input", ebufp);
		goto COMMON_ERR;
        }
	
	if(parent_indicator == child_indicator)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Parent and Child of same type.Valid");
		goto CLEANUP;
	} else {
		/**Check the pay type of the child - self or paid by parent ***/
		billinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp,PIN_FLD_BILLINFO,PIN_ELEMID_ANY,1,ebufp);
		if(billinfo_flistp != NULL){
			vp = PIN_FLIST_FLD_GET(billinfo_flistp,PIN_FLD_PAY_TYPE,1,ebufp);
			if(vp){
				paytype = *(int32*)vp;
				if(paytype != PIN_BILL_TYPE_SUBORD)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Child is paid by self. Hence valid");
					goto CLEANUP;
				} else {
					sprintf ( msg, "Child paytype is %d", paytype);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
					PIN_FLIST_DESTROY_EX(ret_flistp, NULL);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Paid by parent is not allowed as parent supports different connection", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, corp_parent_acnt, ebufp );
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51108", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR,
							 "Paid by parent is not allowed as parent supports different connection", ebufp);
					goto CLEANUP;
				}	
			} else {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error in paytype");
				goto COMMON_ERR;			
			}
		
		} else {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error in billinfo array");
			goto COMMON_ERR;
		}

	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", *ret_flistp);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_outflist, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_acnt_corp_child_bb end of function input flist", in_flistp);
	return;

COMMON_ERR:
	PIN_FLIST_DESTROY_EX(ret_flistp, NULL);
	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in input", ebufp);
	PIN_ERRBUF_RESET(ebufp);
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, corp_parent_acnt, ebufp );
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in input", ebufp);
	
	PIN_FLIST_DESTROY_EX(&srch_outflist, NULL);
	return;
}



/**********************************************************************************
*  Function: mso_cust_validate_input_bb_fields
*  Description: Validate input fields in input flist
*
***********************************************************************************/

int
mso_cust_validate_input_bb_fields(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t	*svc_flistp = NULL;
	pin_flist_t	*inher_flistp = NULL;
	pin_flist_t	*bb_flistp = NULL;
	pin_flist_t	*ret_flistp = NULL;

	int		rv = 0;
	int32           create_service_failure = 1;

	void            *vp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	return 1;

	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_ACCOUNT_OBJ,ret_flistp,PIN_FLD_POID,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cust_validate_input_bb_fields input flist", i_flistp);

	svc_flistp = PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_SERVICES,PIN_ELEMID_ANY,1,ebufp);
	if(svc_flistp)
	{
		inher_flistp = PIN_FLIST_SUBSTR_GET(svc_flistp,PIN_FLD_INHERITED_INFO,1,ebufp);
		if(inher_flistp)
		{
			bb_flistp = PIN_FLIST_SUBSTR_GET(inher_flistp,MSO_FLD_BB_INFO,1,ebufp);
			if(bb_flistp)
			{
				vp = PIN_FLIST_FLD_GET(bb_flistp,MSO_FLD_NETWORK_ELEMENT,1,ebufp);
				if( vp == NULL || (strcmp(vp,"")==0))
				{
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure,ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51702", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "CMTS ID value cannot be NULL", ebufp);
					rv = 1;
				} 
				
				/***** Pavan Bellala 14-08-2015 *********************
				 Commented section as Amplifier and Node are optional
				****************************************************/
				/*
				else if( rv == 0 ){
					vp = PIN_FLIST_FLD_GET(bb_flistp,MSO_FLD_NETWORK_AMPLIFIER, 1, ebufp);		
					if(!vp || (strcmp(vp,"")==0))
					{
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure,ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51702", ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Amplifier ID value cannot be NULL", ebufp);
						rv = 1;						
					} else if(rv == 0){
						vp = PIN_FLIST_FLD_GET(bb_flistp,MSO_FLD_NETWORK_NODE, 1, ebufp);
						if(!vp || (strcmp(vp,"")==0))
						{
							PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure,ebufp);
							PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51702", ebufp);
							PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Node ID value cannot be NULL", ebufp);
							rv = 1;
						}
					}
				
				}*/
			} else {
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure,ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51702", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Broadband service details cannot be NULL", ebufp);				
				rv = 1;
			}

		} else {
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure,ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51702", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Broadband service details cannot be NULL", ebufp);				
			rv = 1;
		}

	} else {
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure,ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51702", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid input during service creation", ebufp);				
		rv = 1;
	}

	*r_flistpp = ret_flistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cust_validate_input_bb_fields return flist", *r_flistpp);
	
	return rv;

}

void
fm_mso_validate_bb_parent(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{

	pin_flist_t	*srch_inflist = NULL;
	pin_flist_t	*srch_outflist = NULL;
	pin_flist_t	*arg_flist = NULL;
	pin_flist_t	*result_flistp = NULL;
	pin_flist_t	*ret_flistp = NULL;
	
	void		*vp = NULL;
	int		input_indicator = 0;
	int		parent_indicator = 0;
	int64		db = 1;
	int		srch_flags = SRCH_DISTINCT;
	int32           create_service_failure = 1;
	char		*template = "select X from /payinfo where F1 = V1 ";

	poid_t		*acct_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_bb_parent", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_bb_parent input flist", in_flistp);

	acct_pdp = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_ACCOUNT_OBJ,1,ebufp);
	db = PIN_POID_GET_DB(acct_pdp);
	srch_inflist =  PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_inflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
        PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_TEMPLATE, template, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_bb_parent search input", srch_inflist);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_inflist, &srch_outflist, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_inflist, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: fm_mso_validate_bb_parent()", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acct_pdp, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51109", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in validating parent search", ebufp);
		*o_flistpp = ret_flistp;
		return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_bb_parent search output", srch_outflist);
	result_flistp = PIN_FLIST_ELEM_GET(srch_outflist,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	if(result_flistp!=NULL)
	{
		arg_flist = PIN_FLIST_ELEM_GET(result_flistp,PIN_FLD_INV_INFO,0,1,ebufp);
		if(arg_flist != NULL)
		{
			vp = PIN_FLIST_FLD_GET(arg_flist,PIN_FLD_INDICATOR,1,ebufp);
			if(vp) parent_indicator = *(int*)vp;
		}

	}
	PIN_FLIST_DESTROY_EX(&srch_outflist, NULL);
	arg_flist = PIN_FLIST_ELEM_GET(in_flistp,PIN_FLD_PAYINFO,0,1,ebufp);
	if(arg_flist != NULL)
	{
		vp = PIN_FLIST_FLD_GET(arg_flist,PIN_FLD_INDICATOR,1,ebufp);
		if(vp) input_indicator = *(int*)vp;
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in validating parent",ebufp);
		PIN_ERRBUF_RESET(ebufp);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acct_pdp, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51110", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in validating parent", ebufp);
		*o_flistpp = ret_flistp;
		return;
	}
	if((input_indicator == parent_indicator) || (parent_indicator == MSO_ACCT_ONLY))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Validation for parent passed");
	}else{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Validation failed.Parent supporting child of another connection type");
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acct_pdp, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_service_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51111", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Validation failed.Parent supporting child of another connection type", ebufp);
		*o_flistpp = ret_flistp;
		return;		
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_bb_parent : ret_flistp", ret_flistp);

}
