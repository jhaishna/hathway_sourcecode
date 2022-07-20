/*******************************************************************
 * File:	fm_mso_cust_activate_customer.c
 * Opcode:	MSO_OP_CUST_REGISTER_CUSTOMER 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 **********
 * Modification History:
 * Modified By: Sachidanand Joshi
 * Date: 22-Jul-2014
 * Modification details: Allowed Starter plan to add one regional
 * south plan during activation
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_activate_customer.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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

#define CORPORATE_PARENT 1
#define CORPORATE_CHILD 2

#define SRVC_BB    "/service/bb"
#define SRVC_CATV  "/service/catv"

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

#define	CATV_MAIN  0
#define	CATV_ADDITIONAL  1

#define MAX_CATV_ALLOWED 3

#define SUCCESS 0
#define FAILED 1

#define CURRENCY_INR 356
#define MSO_DEVICE_TYPE_SD  0
#define MSO_DEVICE_TYPE_HD  1
#define MAX_PLAN_SELECT_LIMIT 100

#define HDW_PROD_PRI_START 0
#define HDW_PROD_PRI_END   60

PIN_IMPORT int32	*resi_max_allowed;
PIN_IMPORT int32	*comm_max_allowed;
PIN_IMPORT int32    *dvbip_max_allowed;

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

extern void
fm_mso_get_device_info(
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

extern void 
fm_mso_et_product_rule_delhi(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	poid_t			*acnt_pdp,
	int32			action,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

extern void
get_evt_lifecycle_template(
	pcm_context_t		*ctxp,
	int64			db,
	int32			string_id, 
	int32			string_ver,
	char			**lc_template, 
	pin_errbuf_t		*ebufp);


extern void
fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
num_to_array(
	int32	num,
	int32	arr[]);

PIN_EXPORT void mso_bill_pol_catv_main(
    pcm_context_t   *ctxp,
    u_int           flags,
    poid_t          *binfo_pdp,
    poid_t          *service_pdp,
    time_t          event_t,
    pin_opcode_t    op_num,
    pin_flist_t     **r_flistpp,
    pin_errbuf_t    *ebufp);

/*Added for demo pack validation 
extern int32
validate_demo_packs(
        pcm_context_t   *ctxp,
        pin_flist_t     *in_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp);

**************************************
* External Functions end
***************************************/

/**************************************
* Local Functions start
***************************************/
static void
fm_mso_validate_plan_and_device(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_populate_billinfo_and_balinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*existing_srvc_flist,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_update_catv_bb_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_populate_payinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_update_service(
	pcm_context_t		*ctxp,
	char			*prog_name,
	poid_t			*acnt_pdp,
	pin_flist_t		*service_array,
	int32			conn_type,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_update_purchased_prod(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
    time_t          purchase_end_t,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static int32
fm_mso_get_service_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**service_array,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_preactivated_svc_info(
	pcm_context_t		*ctxp,
	poid_t			*stb_pdp,
	pin_flist_t		**preactivated_svc_info,
	pin_errbuf_t		*ebufp);

static void
fm_mso_update_preactivated_svc(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_acnt_res(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_acnt_corp_child(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_populate_billinfo_and_balinfo_corp(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	char			*order_service_type,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT void
fm_mso_populate_bouquet_id(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			mso_device_type,
	int32			city_only,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_plan_purchase_rule(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*service_details,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_additional_service_rule(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*service_details,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_same_family_products(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_addl_plan_purchase(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			srvc_catv_count,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_south_plans(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_modify_billing_dates(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_7_days_demo_plan(
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
fm_mso_activation_disc(
	pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp);

/**************************************
* Local Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_ACTIVATE_CUSTOMER 
 *
 *******************************************************************/

EXPORT_OP void
op_mso_cust_activate_customer(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_activate_customer(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_ACTIVATE_CUSTOMER  
 *******************************************************************/
void 
op_mso_cust_activate_customer(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int32			local = LOCAL_TRANS_OPEN_FAIL;
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
	if (opcode != MSO_OP_CUST_ACTIVATE_CUSTOMER) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_activate_customer error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_activate_customer input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);  //CHANGES MADE FOR OPENING TRANSACTION IN ACCOUNTLEVEL LOCK
//	if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
	if (!(prog_name && (strstr(prog_name,"pin_deferred_act") || strstr(prog_name,"BULK") || strstr(prog_name, "Transfer Subscription"))))
	{
		local = fm_mso_trans_open(ctxp, inp_pdp, 3, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51060", ebufp);
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
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51061", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
			return;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'pin_deferred_act' so transaction will not be handled at API level");
	}

	fm_mso_cust_activate_customer(ctxp, flags, i_flistp, r_flistpp, ebufp);
	

 	/***********************************************************
	 * Results.
	 ***********************************************************/
	ret_status = (int32*)PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"op_mso_cust_activate_customer in flistp", *r_flistpp);
	if (PIN_ERRBUF_IS_ERR(ebufp) ) 
	{	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"op_mso_cust_activate_customer in flistp", i_flistp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_activate_customer error", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else if (ret_status && *ret_status == 1)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"op_mso_cust_activate_customer in flistp", i_flistp);
		//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_activate_customer ret_status=1", ebufp);
		//PIN_ERRBUF_RESET(ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS  &&
                        (!(prog_name && (strstr(prog_name,"pin_deferred_act") || strstr(prog_name, "Transfer Subscription"))))
		  )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_activate_customer output flist", *r_flistpp);
	}
	/*if (r_flistpp)
	{	
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);	
	}*/
	

	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_activate_customer(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*mc_ret_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
    pin_flist_t     *et_ret_flistp = NULL;
	pin_flist_t		*deal_array = NULL;
	pin_flist_t		*cr_lmt_array = NULL;
	pin_flist_t		*profiles_array = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*profile_catv_info = NULL;
	pin_flist_t		*org_hierarchy = NULL;
	pin_flist_t		*services_array = NULL;
	pin_flist_t		*update_srvc_status_input = NULL;
	pin_flist_t		*update_srvc_status_output = NULL;
	pin_flist_t		*statuses_array = NULL;
	pin_flist_t		*services_inp = NULL;
	pin_flist_t		*service_details = NULL;
	pin_flist_t		*create_out_flist = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*created_services = NULL;
	pin_flist_t		*created_billinfo = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*deals = NULL;
	pin_flist_t		*plan_array = NULL;
	pin_flist_t		*payment_details = NULL;
	pin_flist_t		*payment_details_out = NULL;
	pin_flist_t		*deposit_plan = NULL;
	pin_flist_t		*deal_info = NULL;
	pin_flist_t		*products = NULL;
	pin_flist_t		*devices_stb = NULL;
	pin_flist_t		*preactivated_svc_info = NULL;
	pin_flist_t		*create_prov_iflist = NULL;
	pin_flist_t		*create_prov_oflist = NULL;
	pin_flist_t		*products_array = NULL;
	pin_flist_t		*billinfo_out_flist = NULL;
	pin_flist_t		*create_deposit_output = NULL;
	pin_flist_t		*group_info = NULL;
	pin_flist_t		*create_notif_iflist = NULL;
	pin_flist_t		*create_notif_oflist = NULL;
	pin_flist_t		*catv_info = NULL;
	pin_flist_t		*copy_iflistp = NULL;
	pin_flist_t		*modify_cust_iflistp = NULL;
	pin_flist_t		*tmp_flist = NULL;
	pin_flist_t		*bi_flistp = NULL;
	pin_flist_t		*payinfo_flistp = NULL;

	pin_flist_t		*read_i_flistp = NULL;
	pin_flist_t		*read_o_flistp = NULL; 
	pin_flist_t		*srch_res_flistp = NULL;
	pin_flist_t		*agrs_flistp	= NULL;
    pin_flist_t     *disc_flistp = NULL;
    pin_flist_t     *disc_oflistp = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*device_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*stb_pdp = NULL;
	poid_t			*billinfo_pdp = NULL;

	int			billing_dom = -1;
	int			elem_id = 0;
	int			elem_id_1 = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie_1 = NULL;
    pin_decimal_t       *per_discp = NULL;
    time_t          disc_end_t = 1893436200;
    time_t          actg_next_t = 0;
    time_t          purchase_end_t = 0;
	int32			srvc_status_active = PIN_STATUS_ACTIVE;
	int32			srvc_status_flags = 0;
	int32			device_state = -1;
	int32			activate_customer_success = 0;
	int32			activate_customer_failure = 1;
	int32			srvc_catv_count = 0;
	int32			*pymt_status = NULL;
	int32			flg_preactivated = CATV_CHANGE_PREACTIVATED_PLAN;
	int32			flg_catv_activation = CATV_ACTIVATION;
	int32			flg_ntf_catv_activation = NTF_SERVICE_ACTIVATION;
	int32			prov_action_inactive = 1; 
	int32			prov_action_activate = 2;
	int32			count = 1;
	int32			*business_type = NULL;
	int32			tmp_business_type = 0;
	int32			account_type = 0;
	int32			account_model = 0;
	int32			subscriber_type = 0;
	int32			*deposit_status = NULL;
	int32			errorCode	= 0;
	int32			ret_status	= -1;
	int32			*payment_term = NULL ;
	int32			hath_payment_term = BILLING_SEGMENT_DEFAULT;
	int32			corporate_type=0;
	int32			sikkim_payment_term = 104;
    int32           pkg_type = 3;
    int32           disc_type = 0;
    int32           serv_type = 0;
    int32           *disc_status = NULL;
    int32           retrack_tp = 0;
	int			arr_business_type[8];

	pin_fld_num_t		field_no	= 0;

	char			*order_service_type = NULL;
	char			*errorDesc	= NULL;
	char			*errorField	= NULL;
	char			errorMsg[80]	= "";
	char			*prog_name = NULL;
	char			*state = NULL;
    char            *acnt_no = NULL;

	void			*vp = NULL;
        //New Tariff - Transaction mapping */
        pin_flist_t             *eflistp = NULL;
        int32                   offer_count = 0;
        int                     elem_id_p = 0;
        pin_cookie_t            cookie_p = NULL;
        int32                   offercheck = 0;
        pin_flist_t             *plan_list_code = NULL;
        pin_flist_t             *inplan_flistp = NULL;
        poid_t                  *plan_pdp_1 = NULL;
        poid_t                  *plan_pdp_2 = NULL;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_activate_customer input flist", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
    acnt_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);

	if (acnt_pdp != NULL && !acnt_pdp )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing Account Poid", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51062", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Missing Account Poid", ebufp);
		return;
	}

	payment_details = PIN_FLIST_ELEM_TAKE(i_flistp, PIN_FLD_PAYMENT, PIN_ELEMID_ANY, 1, ebufp);

 	/*******************************************************************
	* Derive account_type,account_model & subscriber_type from business_type 
	*  - Start
	*******************************************************************/
	business_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
	if (business_type && business_type != NULL)
	{
		tmp_business_type = *business_type;
	}
	
//	account_type = tmp_business_type/1000000;
//
//	//reserved_bits = tmp_business_type%1000;
//	tmp_business_type = tmp_business_type/1000;
//
//	account_model = tmp_business_type%10;
//
//	tmp_business_type = tmp_business_type/10;
//	subscriber_type =  tmp_business_type%100;
	
	num_to_array(tmp_business_type, arr_business_type);
	account_type    = 10*(arr_business_type[0])+arr_business_type[1];// First 2 digits
	subscriber_type = 10*(arr_business_type[2])+arr_business_type[3];// next 2 digits
	account_model   = arr_business_type[4];				 // 5th field
	corporate_type  = arr_business_type[6];				 // 7th field

	/************************************************************************
	* Derive account_type,account_model & subscriber_type from business_type 
	* END
	************************************************************************/

	/*******************************************************************
	* Activation Validation - Start
	*******************************************************************/
	/*******************************************************************
	*	Account Status Validation - Start
	*******************************************************************/
	if ( account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD )
	{
		/*************************************************************
		* Validation for Commercial/Corporate child account activation
		*************************************************************/
		fm_mso_validate_acnt_corp_child(ctxp, i_flistp, &ret_flistp, ebufp );
	}
	else
	{
		/*************************************************************
		* Validation for Residential account activation
		*************************************************************/
		fm_mso_validate_acnt_res(ctxp, i_flistp, &ret_flistp, ebufp );
	}

	if (ret_flistp && ret_flistp !=(pin_flist_t *)NULL)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_customer_failure)
		{
			goto CLEANUP;
		}
		else if (ret_status == activate_customer_success)
		{
			PIN_FLIST_FLD_COPY(ret_flistp, MSO_FLD_AREA, i_flistp, MSO_FLD_AREA, ebufp );
			PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_CITY, i_flistp, PIN_FLD_CITY, ebufp );
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		}
		
	} 

	/*************************************************************
	* Validation for Residential account activation
	*************************************************************/

	/*******************************************************************
	*	Account Status Validation - End
	*******************************************************************/

	/*******************************************************************
	*	Plan & Device Validation - Start
	*******************************************************************/
	fm_mso_validate_plan_and_device(ctxp, i_flistp, &ret_flistp, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
	if (ret_flistp && ret_flistp != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", ret_flistp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DEVICE and PLAN validation Successful");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "i_flistp", i_flistp );
	/*******************************************************************
	*	Plan & Device Validation - End
	*******************************************************************/

	/*******************************************************************
	* Service Count Validation - Start
	*******************************************************************/
	services_inp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	if(services_inp && services_inp != NULL )
	order_service_type = (char*)PIN_POID_GET_TYPE((poid_t*)PIN_FLIST_FLD_GET(services_inp, PIN_FLD_SERVICE_OBJ, 0, ebufp));

	srvc_catv_count = fm_mso_get_service_info(ctxp, i_flistp, &service_details, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fetching existing service information", ebufp);
		PIN_ERRBUF_RESET(ebufp);

		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51063", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in fetching existing service information", ebufp);
		goto CLEANUP;
	}
    if (subscriber_type == 48 && srvc_catv_count >= *dvbip_max_allowed)
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "MAX_CATV_ALLOWED for DVBIP Reached");
        ret_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51064", ebufp);
        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "MAX DVBIP CATV allowed reached", ebufp);
        goto CLEANUP;
    }
    else if (subscriber_type != 48 && account_model == ACCOUNT_MODEL_RES && srvc_catv_count >= *resi_max_allowed)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "MAX_CATV_ALLOWED for Residence Reached");
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51064", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "MAX Residential CATV allowed reached", ebufp);
		goto CLEANUP;
	}
	else if (account_model == ACCOUNT_MODEL_CORP && srvc_catv_count >= *comm_max_allowed)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "MAX_CATV_ALLOWED for Commercial Reached");
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51064", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "MAX Commercial CATV allowed reached", ebufp);
		goto CLEANUP;
	}
	else if (srvc_catv_count >= 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "srvc_catv_count=1");
		srvc_catv_count = 1;
		if(services_inp && services_inp != NULL )
		catv_info = PIN_FLIST_SUBSTR_GET(services_inp, MSO_FLD_CATV_INFO, 1, ebufp);
		if(catv_info && catv_info != NULL){
		PIN_FLIST_FLD_SET(catv_info, PIN_FLD_CONN_TYPE, &srvc_catv_count, ebufp);
		}
	}
	else if (srvc_catv_count == 0)
	{
		if(services_inp && services_inp != NULL )
		catv_info = PIN_FLIST_SUBSTR_GET(services_inp, MSO_FLD_CATV_INFO, 1, ebufp);
		if(catv_info && catv_info != NULL){ 
		PIN_FLIST_FLD_SET(catv_info, PIN_FLD_CONN_TYPE, &srvc_catv_count, ebufp);
		}
	}
    if (catv_info)
    {
        PIN_FLIST_FLD_SET(catv_info, PIN_FLD_LAST_LOADED_T, &retrack_tp, ebufp);
    }

	/*************************************************************
	* Validation for PAY Type
	* Start
	*************************************************************/
	if ( account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD )
	{
		/*************************************************************
		* Validation for Commercial/Corporate child account activation
		*************************************************************/
		fm_mso_validate_pay_type_corp( ctxp, i_flistp, MSO_OP_CUST_ACTIVATE_CUSTOMER, &ret_flistp, ebufp);
		if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
		{
			ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
			if (ret_status == activate_customer_failure)
			{
				goto CLEANUP;
			}
		}
	}
	else
	{
		/*************************************************************
		* Validation for Residential account activation
		*************************************************************/
		fm_mso_validate_pay_type( ctxp, i_flistp, MSO_OP_CUST_ACTIVATE_CUSTOMER, &ret_flistp, ebufp);
		if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
		{
			ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
			if (ret_status == activate_customer_failure)
			{
				goto CLEANUP;
			}
		}
	}
 	/*************************************************************
	* Validation for PAY Type
	* End
	*************************************************************/
	if (srvc_catv_count !=0 ) // Called only for additional CATV 
	{
		fm_mso_et_product_rule_delhi(ctxp, i_flistp, acnt_pdp, MSO_OP_CUST_ACTIVATE_CUSTOMER,&ret_flistp, ebufp);
		if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
		{
			ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
			if (ret_status == activate_customer_failure)
			{
				goto CLEANUP;
			}
		}
	}

	fm_mso_validate_plan_purchase_rule(ctxp, i_flistp, service_details,&ret_flistp, ebufp);
	if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_customer_failure)
		{
			goto CLEANUP;
		}
	}

	fm_mso_validate_additional_service_rule(ctxp, i_flistp, service_details,&ret_flistp, ebufp);
	if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_customer_failure)
		{
			goto CLEANUP;
		}
	}

	fm_mso_validate_addl_plan_purchase(ctxp, i_flistp, srvc_catv_count, &ret_flistp, ebufp);
	if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == activate_customer_failure)
		{
			goto CLEANUP;
		}
	}

	/*******************************************************************
	* Service Count Validation - End
	*******************************************************************/

	/************************************************************************
	Modify balance_group and billinfo
	START
	************************************************************************/
	if ( account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD )
	{
		/*************************************************************
		* For Commercial/Corporate child account activation
		*************************************************************/
		fm_mso_populate_billinfo_and_balinfo_corp(ctxp, i_flistp, order_service_type, &ret_flistp, ebufp );
	}
	else
	{
		/*************************************************************
		* For Residential account activation
		*************************************************************/
		fm_mso_populate_billinfo_and_balinfo(ctxp, i_flistp, service_details, &ret_flistp, ebufp );
	}

	if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", ret_flistp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
	/************************************************************************
	Modify balance_group and billinfo
	END
	************************************************************************/
	//PIN_FLIST_DESTROY_EX(&service_details, NULL);
	
	/************************************************************************
	* Modify the substruct services.MSO_FLD_CATV_INFO for provisioning
	* -START
	************************************************************************/

                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(3,"Error before 1");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Calling MSO_OP_PYMT_COLLECT", ebufp);
                        goto CLEANUP;
                }


	fm_mso_update_catv_bb_info(ctxp, i_flistp, &ret_flistp, ebufp);
	if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", ret_flistp);
		goto CLEANUP;
	}

                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(3,"Error before 2");
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Calling MSO_OP_PYMT_COLLECT", ebufp);
                        goto CLEANUP;
                }

	/***********************************************************************
	* Get deposit info 
	* Start
	***********************************************************************/
	deposit_plan = PIN_FLIST_ELEM_TAKE(i_flistp, PIN_FLD_DEPOSIT_PLANS, 0, 1, ebufp);
	/***********************************************************************
	* Get deposit info 
	* End
	***********************************************************************/
	/************************************************************************
	* Modify the substruct services.MSO_FLD_CATV_INFO for provisioning
	* -END
	************************************************************************/
	
	/* Defect : Payment term is set to 0 if not passed in input flist 18-12-2014  Sasi */
	/* Solution : Setting Payment Term to 101 if it is not present in the input flist */
        if(srvc_catv_count ==0)
        {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "i_flistp", i_flistp);	

		payinfo_flistp = PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_PAYINFO,0,1,ebufp);

		PIN_ERR_LOG_MSG(3,"entry point 1");

		if(payinfo_flistp && payinfo_flistp != NULL )
		payment_term = (int32 *)PIN_FLIST_FLD_GET(payinfo_flistp,PIN_FLD_PAYMENT_TERM,1,ebufp);

                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(3,"Error before 1");
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Calling MSO_OP_PYMT_COLLECT", ebufp);
                        goto CLEANUP;
                }


		// Adding search to set correct payterm
		read_i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_i_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		agrs_flistp = PIN_FLIST_ELEM_ADD(read_i_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
		PIN_FLIST_FLD_SET(agrs_flistp, PIN_FLD_STATE, NULL, ebufp);
		PIN_ERR_LOG_FLIST(3, "Payment term search flist", read_i_flistp);
		
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_i_flistp, &read_o_flistp, ebufp);
	
		if (PIN_ERRBUF_IS_ERR(ebufp))
	        {
			PIN_ERR_LOG_MSG(3,"Error during Search");
			goto CLEANUP;
		}	
		PIN_ERR_LOG_FLIST(3, "Payment term search state out flist", read_o_flistp);
		srch_res_flistp = PIN_FLIST_ELEM_GET(read_o_flistp, PIN_FLD_NAMEINFO, 1, 0, ebufp);
		if(srch_res_flistp && srch_res_flistp != (pin_flist_t *)NULL){ 
		state = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_STATE, 0, ebufp);
		}
		PIN_ERR_LOG_MSG(3, "entry point 2");

		if( payment_term && (*payment_term == 101 || *payment_term == 104) )
			PIN_ERR_LOG_MSG(3,"entry point 3");
		else
		
		{
			PIN_ERR_LOG_MSG(3,"entry point 4");
		
			if(state && strcmp(state, "SIKKIM")==0)
			{
				PIN_FLIST_FLD_SET(payinfo_flistp, PIN_FLD_PAYMENT_TERM, (void *)&sikkim_payment_term, ebufp);
			}
			else
			{
				hath_payment_term = 101;
				PIN_FLIST_FLD_SET(payinfo_flistp, PIN_FLD_PAYMENT_TERM, (void *)&hath_payment_term, ebufp);
			}
		}
	
	}



	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CUST_MODIFY_CUSTOMER input flist", i_flistp);
	/*Call Validation rules*/

	fm_mso_validate_south_plans(ctxp, i_flistp, &ret_flistp, ebufp)	;
	if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
	{
		goto CLEANUP;
	}
	
	fm_mso_validate_same_family_products(ctxp, i_flistp, &ret_flistp, ebufp);
	if (ret_flistp && ret_flistp !=(pin_flist_t *)NULL) 
	{
		goto CLEANUP;
	}

	//modify billing dates
	modify_cust_iflistp = PIN_FLIST_COPY(i_flistp, ebufp);
	fm_mso_modify_billing_dates(ctxp, modify_cust_iflistp, &ret_flistp, ebufp);
	if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
	{
		goto CLEANUP;
	}
	//Adding function to apply discount at product level
	fm_mso_activation_disc(ctxp , modify_cust_iflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(1, "Error during discount enrichment");
		goto CLEANUP;
	}
	
	
	if ( account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD )
	{
		payinfo_flistp = PIN_FLIST_ELEM_GET(modify_cust_iflistp, PIN_FLD_PAYINFO, 0, 1, ebufp );
		if (payinfo_flistp)
		{
			PIN_FLIST_ELEM_DROP(modify_cust_iflistp, PIN_FLD_PAYINFO, 0, ebufp);
		}
    }
	PIN_ERR_LOG_FLIST(3, "Modify customer flist after discout enrichment", modify_cust_iflistp);

	PCM_OP(ctxp, PCM_OP_CUST_MODIFY_CUSTOMER, 0, modify_cust_iflistp, &mc_ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&modify_cust_iflistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CUST_MODIFY_CUSTOMER output flist", mc_ret_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{     
		errorCode = ebufp->pin_err;
		errorDesc = (char *)PIN_PINERR_TO_STR(errorCode);
		field_no = ebufp->field;
		errorField = (char *)PIN_FIELD_GET_NAME(field_no);
		
		memset(errorMsg,'\0',sizeof(errorMsg));
		strcpy(errorMsg,errorField);
		strcat(errorMsg,": ");
		strcat(errorMsg,errorDesc);

		if (errorCode == PIN_ERR_BAD_VALUE)
		{
			strcat(errorMsg, ": PCM_OP_CUST_MODIFY_CUSTOMER failed") ;
		}

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errorMsg, ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51074", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, errorMsg, ebufp);
		goto CLEANUP;
	}
	if (mc_ret_flistp && mc_ret_flistp != (pin_flist_t *)NULL) 
	{
 		/****************************************************************
		*  validate_7_days_demo_plan
		****************************************************************/
		//commented as this validation is not needed during service activation
		/*fm_mso_validate_7_days_demo_plan(ctxp, i_flistp, &ret_flistp, ebufp);
		if (ret_flistp)
		{
			ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
			if (ret_status == activate_customer_failure)
			{
				goto CLEANUP;
			}
		} */
		services_array = PIN_FLIST_ELEM_GET(mc_ret_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
		if(services_array && services_array != NULL )
		srvc_pdp = PIN_FLIST_FLD_GET(services_array, PIN_FLD_SERVICE_OBJ, 0, ebufp );
		/****************************************************************
		*  Update provisioning info in /service/catv
		****************************************************************/
		fm_mso_update_service(ctxp, prog_name, acnt_pdp, services_array, srvc_catv_count, &ret_flistp, ebufp );
		if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
		{
			goto CLEANUP;
		}
		/****************************************************************
		*  Update /purchased_product to contain /plan poid
		****************************************************************/
        vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp );
        if (vp)
        {
            purchase_end_t = *(time_t *)vp;
        }
		fm_mso_update_purchased_prod(ctxp, services_array, purchase_end_t, &ret_flistp, ebufp );
		if (ret_flistp && ret_flistp!=(pin_flist_t *)NULL) 
		{
			goto CLEANUP;
		}
		/****************************************************************
		*  Call Payment API
		*  Start
		****************************************************************/
		if (payment_details && payment_details != NULL) 
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Payment Details passed", payment_details );
			PCM_OP(ctxp, MSO_OP_PYMT_COLLECT, 0, payment_details, &payment_details_out, ebufp);
			PIN_FLIST_DESTROY_EX(&payment_details, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Calling MSO_OP_PYMT_COLLECT", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51077", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Calling MSO_OP_PYMT_COLLECT", ebufp);
				goto CLEANUP;
			}
			if(payment_details_out && payment_details_out != NULL)
			{
				pymt_status = PIN_FLIST_FLD_GET(payment_details_out, PIN_FLD_STATUS, 1, ebufp );
				if (pymt_status && pymt_status != NULL && *pymt_status == 1 ) 
				{
					ret_flistp = payment_details_out;	
					goto CLEANUP;
				}
			}
		}
		/****************************************************************
		*  Call Payment API
		*  End
		****************************************************************/
		/******************************************************************
		* Change Service/Purchased_product status from inactive to active
		* Start
		*******************************************************************/
		
		update_srvc_status_input = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, update_srvc_status_input, PIN_FLD_PROGRAM_NAME, ebufp );
		if (!prog_name)
		{
			PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_PROGRAM_NAME, "oap|csradmin", ebufp );
		}

		statuses_array = PIN_FLIST_ELEM_ADD(update_srvc_status_input, PIN_FLD_STATUSES, 0, ebufp);
		PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS, &srvc_status_active, ebufp);
		PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS_FLAGS, &srvc_status_flags, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Update Service Status input flist", update_srvc_status_input);
		PCM_OP(ctxp, PCM_OP_CUST_SET_STATUS, 0, update_srvc_status_input, &update_srvc_status_output, ebufp);
		PIN_FLIST_DESTROY_EX(&update_srvc_status_input, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating product status", ebufp);
			errorCode = ebufp->pin_err;
            field_no = ebufp->field;
            PIN_ERRBUF_RESET(ebufp);

			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51078", ebufp);
            if (errorCode == PIN_ERR_BAD_VALUE && field_no == PIN_FLD_CURRENT_BAL)
            {
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "There is no sufficient balance to activate service", ebufp);
            }
            else
            {
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating product status", ebufp);
            }
			goto CLEANUP;
		}
		PIN_FLIST_DESTROY_EX(&update_srvc_status_output, NULL);
		/******************************************************************
		* Change Service/Purchased_product status from inactive to active
		* End
		*******************************************************************/
		/****************************************************************
		*  Call Deposit API
		*  Start
		****************************************************************/
		//created_billinfo = PIN_FLIST_ELEM_TAKE(mc_ret_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp );
		tmp_flist = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp );
		created_billinfo = PIN_FLIST_COPY(tmp_flist, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "created_billinfo",created_billinfo);

		if (deposit_plan && deposit_plan != NULL)
		{
			vp = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(services_array, MSO_FLD_CATV_INFO, 1, ebufp);
			if (vp && vp != NULL )
			{
				PIN_FLIST_FLD_COPY(vp, MSO_FLD_AGREEMENT_NO, deposit_plan, MSO_FLD_AGREEMENT_NO, ebufp );
			}
			PIN_FLIST_FLD_SET(deposit_plan, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, deposit_plan, PIN_FLD_USERID, ebufp );
			//PIN_FLIST_FLD_COPY(created_billinfo, PIN_FLD_BILLINFO_OBJ, deposit_plan, PIN_FLD_BILLINFO_OBJ, ebufp );
			PIN_FLIST_FLD_COPY(created_billinfo, PIN_FLD_POID, deposit_plan, PIN_FLD_BILLINFO_OBJ, ebufp );
			PIN_FLIST_FLD_SET(deposit_plan, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, deposit_plan, PIN_FLD_PROGRAM_NAME, ebufp );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Input deposit_plan", deposit_plan );
			PCM_OP(ctxp, MSO_OP_PYMT_CREATE_DEPOSIT, 0, deposit_plan, &create_deposit_output, ebufp);
			//PCM_OP(ctxp, PCM_OP_TEST_LOOPBACK, 0, deposit_plan, &create_deposit_output, ebufp);
			PIN_FLIST_DESTROY_EX(&deposit_plan, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Creating Deposit", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51079", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Creating Deposit", ebufp);
				goto CLEANUP;
			}
			if (create_deposit_output && create_deposit_output != NULL)
			{
				deposit_status = PIN_FLIST_FLD_GET(create_deposit_output, PIN_FLD_STATUS , 1, ebufp );
				if (deposit_status && *deposit_status == 1 )
				{
					ret_flistp = create_deposit_output;
					goto CLEANUP;
				}
			}
		}
 		/****************************************************************
		*  Call Deposit API
		*  End
		****************************************************************/

		/******************************************************************
		* apply any deferred purchase fee
		* Start
		*******************************************************************/
		update_srvc_status_input = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_POID, (void *)acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_SERVICE_OBJ, (void *)srvc_pdp, ebufp);
		PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
		if (!prog_name && prog_name != NULL) 
		{
			PIN_FLIST_FLD_SET(update_srvc_status_input, PIN_FLD_PROGRAM_NAME, "oap|csradmin", ebufp);
		}
		


		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Apply deferred purchase fee input flist", update_srvc_status_input);
		PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_FEES, 0, update_srvc_status_input, &update_srvc_status_output, ebufp);

		PIN_FLIST_DESTROY_EX(&update_srvc_status_input, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in applying deferred purchase feee", ebufp);
			PIN_ERRBUF_RESET(ebufp);

			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51080", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in applying deferred purchase feee", ebufp);
			goto CLEANUP;
		}
		PIN_FLIST_DESTROY_EX(&update_srvc_status_output, NULL);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "99999");

		/******************************************************************
		* apply any deferred purchase fee
		* Start
		*******************************************************************/
		/*Associating to proper business profile */
	    bi_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 0, ebufp);
		if(bi_flistp && bi_flistp != NULL )
        billinfo_pdp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_POID, 1, ebufp );
        fm_mso_set_assoc_bus_profile(ctxp, billinfo_pdp, &ret_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp) )
        {
                PIN_ERRBUF_RESET(ebufp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in updating associated business profile", ebufp);
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51719", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating associated business profile.", ebufp);
                goto CLEANUP;
        }

	}
	/******************************************************************
	* Call Provisioning APIs
	* Start
	*******************************************************************/
	device_state = *(int *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE_ID, 0, ebufp);
	catv_info = PIN_FLIST_SUBSTR_GET(services_array, MSO_FLD_CATV_INFO, 0, ebufp);
	
	if (device_state == PREACTIVATED)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PREACTIVATED STB and VC");
		devices_stb = PIN_FLIST_ELEM_GET(services_array, PIN_FLD_DEVICES, 0, 0, ebufp);
		stb_pdp = PIN_FLIST_FLD_GET(devices_stb, PIN_FLD_DEVICE_OBJ, 0, ebufp );
		
		/*fm_mso_get_preactivated_svc_info(ctxp, stb_pdp, &preactivated_svc_info, ebufp );
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in retrieving preactivated_svc_info ", ebufp);
			goto CLEANUP;
		}
		if (!preactivated_svc_info)
		{
		 	ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "preactivated_svc_info reyurning NULL", ebufp);
			goto CLEANUP;
		} */
		preactivated_svc_info = PIN_FLIST_SUBSTR_GET(services_array, PIN_FLD_PROV_DATA, 0, ebufp);

		create_prov_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(create_prov_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_COPY(catv_info, MSO_FLD_NETWORK_NODE, create_prov_iflist, MSO_FLD_NETWORK_NODE, ebufp);
		PIN_FLIST_FLD_SET(create_prov_iflist, PIN_FLD_SERVICE_OBJ, (void*)PIN_FLIST_FLD_GET(services_array, PIN_FLD_SERVICE_OBJ, 1, ebufp), ebufp ) ;
		PIN_FLIST_FLD_SET(create_prov_iflist, PIN_FLD_FLAGS, &flg_preactivated, ebufp);
		/*PIN_FLIST_ELEM_COPY(services_inp, PIN_FLD_DEVICES, 0, create_prov_iflist, PIN_FLD_DEVICES, 0, ebufp);
		PIN_FLIST_ELEM_COPY(services_inp, PIN_FLD_DEVICES, 1, create_prov_iflist, PIN_FLD_DEVICES, 1, ebufp);
		PIN_FLIST_ELEM_COPY(services_inp, PIN_FLD_DEVICES, 2, create_prov_iflist, PIN_FLD_DEVICES, 2, ebufp);*/
		PIN_FLIST_ELEM_COPY(services_array, PIN_FLD_DEVICES, 0, create_prov_iflist, PIN_FLD_DEVICES, 0, ebufp);
		PIN_FLIST_ELEM_COPY(services_array, PIN_FLD_DEVICES, 1, create_prov_iflist, PIN_FLD_DEVICES, 1, ebufp);
		PIN_FLIST_ELEM_COPY(services_array, PIN_FLD_DEVICES, 2, create_prov_iflist, PIN_FLD_DEVICES, 2, ebufp);
		
		products = PIN_FLIST_ELEM_ADD(create_prov_iflist, PIN_FLD_PRODUCTS, 0, ebufp);
		PIN_FLIST_FLD_SET(products, PIN_FLD_STATUS_FLAGS, &prov_action_inactive, ebufp);
		if(preactivated_svc_info && preactivated_svc_info != NULL)
		PIN_FLIST_CONCAT(products, preactivated_svc_info, ebufp);
		//PIN_FLIST_DESTROY_EX(&preactivated_svc_info, NULL);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Debug_001 services_array", services_array);
		while((deals = PIN_FLIST_ELEM_GET_NEXT(services_array, PIN_FLD_DEALS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Debug_002 deals", deals);
			deal_info = PIN_FLIST_SUBSTR_GET(deals, PIN_FLD_DEAL_INFO, 0, ebufp);
			elem_id_1 =0;
			cookie_1 = NULL;
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Debug_003 deals", deal_info);
			while((products_array = PIN_FLIST_ELEM_GET_NEXT(deal_info, PIN_FLD_PRODUCTS,
				&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Debug_004 products_array", products_array);
				products = PIN_FLIST_ELEM_ADD(create_prov_iflist, PIN_FLD_PRODUCTS, count , ebufp);
				PIN_FLIST_FLD_SET(products, PIN_FLD_POID, (void*)PIN_FLIST_FLD_GET(products_array, PIN_FLD_OFFERING_OBJ, 0, ebufp ), ebufp);
				PIN_FLIST_FLD_SET(products, PIN_FLD_STATUS_FLAGS, &prov_action_activate, ebufp);
				count++;
			}
		}

		/* Write USERID, PROGRAM_NAME in buffer  - start */
                PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_USERID,create_prov_iflist,PIN_FLD_USERID,ebufp);
                PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,create_prov_iflist,PIN_FLD_PROGRAM_NAME,ebufp);
                /* Write USERID, PROGRAM_NAME in buffer - end */
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "create_prov_action input list", create_prov_iflist);
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, create_prov_iflist, &create_prov_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&create_prov_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51081", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "create_prov_action input list output flist", create_prov_oflist);
		PIN_FLIST_DESTROY_EX(&create_prov_oflist, NULL);
		/*******************************************************
		*  call fm_mso_update_preactivated_svc() to update 
		* /mso_cfg_preactivated_svc
		*******************************************************/
		fm_mso_update_preactivated_svc(ctxp, acnt_pdp, preactivated_svc_info, &ret_flistp, ebufp);
		if (ret_flistp && ret_flistp != (pin_flist_t *)NULL) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			//PIN_ERRBUF_RESET(ebufp);
			//ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
			//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
			goto CLEANUP;
		}
		if (ret_flistp && ret_flistp != (pin_flist_t *)NULL)
		{
			goto CLEANUP;
		}

		
	}
	else if ( device_state == READY_TO_USE )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NON-PREACTIVATED STB and VC");
		create_prov_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(create_prov_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_COPY(catv_info, MSO_FLD_NETWORK_NODE, create_prov_iflist, MSO_FLD_NETWORK_NODE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_OBJ, create_prov_iflist, MSO_FLD_LCO_OBJ, ebufp);
		PIN_FLIST_FLD_SET(create_prov_iflist, PIN_FLD_SERVICE_OBJ, (void*)PIN_FLIST_FLD_GET(services_array, PIN_FLD_SERVICE_OBJ, 1, ebufp), ebufp ) ;
		PIN_FLIST_FLD_SET(create_prov_iflist, PIN_FLD_FLAGS, &flg_catv_activation, ebufp);

		elem_id =0;
		cookie = NULL;
		count =0;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Debug_001 services_array", services_array);
		while((deals = PIN_FLIST_ELEM_GET_NEXT(services_array, PIN_FLD_DEALS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Debug_002 deals", deals);

			PIN_FLIST_FLD_COPY(deals, PIN_FLD_PLAN_OBJ, create_prov_iflist, PIN_FLD_PLAN_OBJ, ebufp);
			deal_info = PIN_FLIST_SUBSTR_GET(deals, PIN_FLD_DEAL_INFO, 0, ebufp);
			elem_id_1 =0;
			cookie_1 = NULL; 
			while((products_array = PIN_FLIST_ELEM_GET_NEXT(deal_info, PIN_FLD_PRODUCTS,
				&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
			{
				products = PIN_FLIST_ELEM_ADD(create_prov_iflist, PIN_FLD_PRODUCTS, count , ebufp);
				PIN_FLIST_FLD_SET(products, PIN_FLD_POID, (void*)PIN_FLIST_FLD_GET(products_array, PIN_FLD_OFFERING_OBJ, 0, ebufp ), ebufp);
				PIN_FLIST_FLD_SET(products, PIN_FLD_STATUS_FLAGS, &prov_action_activate, ebufp);
				count++;
			}
		}

		/* Write USERID, PROGRAM_NAME in buffer  - start */
                PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_USERID,create_prov_iflist,PIN_FLD_USERID,ebufp);
                PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,create_prov_iflist,PIN_FLD_PROGRAM_NAME,ebufp);
                /* Write USERID, PROGRAM_NAME in buffer - end */
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "create_prov_action input list", create_prov_iflist);
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, create_prov_iflist, &create_prov_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&create_prov_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51083", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_pricing_role SEARCH output flist", create_prov_oflist);
		PIN_FLIST_DESTROY_EX(&create_prov_oflist, NULL); 
	}
	/******************************************************************
	* Call Provisioning APIs
	* End
	*******************************************************************/
	/******************************************************************
	* Call Notification APIs
	* Start
	*******************************************************************/
	create_notif_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(create_notif_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(create_notif_iflist, PIN_FLD_SERVICE_OBJ, (void*)PIN_FLIST_FLD_GET(services_array, PIN_FLD_SERVICE_OBJ, 1, ebufp), ebufp ) ;
	PIN_FLIST_FLD_SET(create_notif_iflist, PIN_FLD_FLAGS, &flg_ntf_catv_activation, ebufp);
        
	plan_list_code = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PLAN_LIST_CODE, 1, ebufp);
        elem_id_p =0;
        cookie_p = NULL;
        count = 0;
        while ((inplan_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_list_code, PIN_FLD_PLAN, &elem_id_p, 1, &cookie_p, ebufp)) != (pin_flist_t *)NULL){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inplan_flistp");
        	PIN_FLIST_FLD_SET(create_notif_iflist, PIN_FLD_CODE, (void*)PIN_FLIST_FLD_GET(inplan_flistp, PIN_FLD_CODE, 1, ebufp), ebufp ) ;
		}


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "create_notif_iflist input list", create_notif_iflist);
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, create_notif_iflist, &create_notif_oflist, ebufp);
	//PCM_OP(ctxp, PCM_OP_TEST_LOOPBACK, 0, create_notif_iflist, &create_notif_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&create_notif_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51084", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "create_notif_oflist", create_notif_oflist);
	PIN_FLIST_DESTROY_EX(&create_notif_oflist, NULL); 
	/******************************************************************
	* Call Notification APIs
	* End
	*******************************************************************/
	
	/********************************************************* 
	 * Errors..?
	 *********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		fm_mso_get_err_desc(ctxp, acnt_pdp, &ret_flistp, ebufp);
		goto CLEANUP;
	}


	//Populate Return Flist for success Case
	/*******************************************************************
	* Create Output flist - End   
	*******************************************************************/
//	ret_flistp = PIN_FLIST_CREATE(ebufp);
//	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
//	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &activate_customer_success, ebufp);
//	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service activated Successfully", ebufp);

	create_out_flist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(create_out_flist, PIN_FLD_POID, acnt_pdp, ebufp );
	PIN_FLIST_FLD_SET(create_out_flist, PIN_FLD_STATUS, &activate_customer_success, ebufp );
	
	srvc_info = PIN_FLIST_SUBSTR_ADD(create_out_flist, PIN_FLD_SERVICE_INFO, ebufp);
	created_services = PIN_FLIST_ELEM_ADD(srvc_info, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_PUT(created_services, PIN_FLD_POID, (void*)PIN_FLIST_FLD_TAKE(services_array, PIN_FLD_SERVICE_OBJ, 1, ebufp), ebufp ) ;
	PIN_FLIST_FLD_PUT(created_services, PIN_FLD_LOGIN, (void*)PIN_FLIST_FLD_TAKE(services_array, PIN_FLD_LOGIN, 1, ebufp), ebufp ) ;
	PIN_FLIST_FLD_SET(created_services, PIN_FLD_STATUS, &srvc_status_active, ebufp ) ;

	PIN_FLIST_SUBSTR_PUT(created_services, (pin_flist_t *)PIN_FLIST_SUBSTR_TAKE(services_array, MSO_FLD_CATV_INFO, 1, ebufp), 
		MSO_FLD_CATV_INFO, ebufp );
	PIN_FLIST_ELEM_PUT(created_services, (pin_flist_t *)PIN_FLIST_ELEM_TAKE(services_array, PIN_FLD_DEVICES, 0, 1, ebufp), 
		PIN_FLD_DEVICES, 0, ebufp );
	PIN_FLIST_ELEM_PUT(created_services, (pin_flist_t *)PIN_FLIST_ELEM_TAKE(services_array, PIN_FLD_DEVICES, 1, 1, ebufp), 
		PIN_FLD_DEVICES, 1, ebufp );


	fm_mso_read_billinfo(ctxp, 
			     (poid_t*)PIN_FLIST_FLD_GET(created_billinfo, PIN_FLD_POID, 0, ebufp),
			     &billinfo_out_flist,
			     ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "billinfo_out_flist",billinfo_out_flist)
//	PIN_FLIST_FLD_DROP(created_billinfo, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_DROP(created_billinfo, PIN_FLD_PAY_TYPE, ebufp );
	PIN_FLIST_FLD_DROP(created_billinfo, PIN_FLD_BILLINFO_ID, ebufp );
//	PIN_FLIST_FLD_DROP(created_billinfo, PIN_FLD_PAYINFO_OBJ, ebufp );
	PIN_FLIST_FLD_DROP(created_billinfo, PIN_FLD_ACTG_TYPE, ebufp );
//	PIN_FLIST_FLD_DROP(created_billinfo, PIN_FLD_CURRENCY, ebufp );
//	PIN_FLIST_FLD_DROP(created_billinfo, PIN_FLD_EFFECTIVE_T, ebufp );
//	PIN_FLIST_FLD_DROP(created_billinfo, PIN_FLD_FLAGS, ebufp );

	PIN_FLIST_FLD_COPY(billinfo_out_flist, PIN_FLD_ACTG_LAST_T, created_billinfo, PIN_FLD_ACTG_LAST_T, ebufp );
	PIN_FLIST_FLD_COPY(billinfo_out_flist, PIN_FLD_ACTG_NEXT_T, created_billinfo, PIN_FLD_ACTG_NEXT_T, ebufp );
	PIN_FLIST_FLD_COPY(billinfo_out_flist, PIN_FLD_ACTG_FUTURE_T, created_billinfo, PIN_FLD_ACTG_FUTURE_T, ebufp );
	PIN_FLIST_DESTROY_EX(&billinfo_out_flist, NULL);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modified BILLINFO", created_billinfo);
	PIN_FLIST_ELEM_PUT(create_out_flist, created_billinfo, PIN_FLD_BILLINFO, 0, ebufp);

	plan_list = PIN_FLIST_ELEM_ADD(create_out_flist, PIN_FLD_PLAN_LISTS, 0, ebufp );
	/*commented below lines for transaction mapping changes*/
	/*elem_id =0;
	cookie = NULL;
	while((deals = PIN_FLIST_ELEM_GET_NEXT(services_array, PIN_FLD_DEALS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		deal_info = PIN_FLIST_SUBSTR_GET(deals, PIN_FLD_DEAL_INFO, 0, ebufp);
		elem_id_1 =0;
		cookie_1 = NULL;
		while((products_array = PIN_FLIST_ELEM_GET_NEXT(deal_info, PIN_FLD_PRODUCTS,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			plan_array = PIN_FLIST_ELEM_ADD(plan_list, PIN_FLD_PLAN, count, ebufp );
			PIN_FLIST_FLD_COPY(deals, PIN_FLD_PLAN_OBJ, plan_array, PIN_FLD_PLAN_OBJ, ebufp );
			PIN_FLIST_FLD_COPY(products_array, PIN_FLD_OFFERING_OBJ, plan_array, PIN_FLD_OFFERING_OBJ, ebufp );
			count++;
		}
	}*/
        //New Tariff - Transaction mapping Start */
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PIN_FLD_PLAN_LISTS");

        plan_list_code = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PLAN_LIST_CODE, 1, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PIN_FLD_PLAN_LIST_CODE");

        elem_id_p =0;
        cookie_p = NULL;
        count = 0;
        while ((inplan_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_list_code, PIN_FLD_PLAN, &elem_id_p, 1, &cookie_p, ebufp)) != (pin_flist_t *)NULL){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "inplan_flistp");
                plan_pdp_1 = PIN_FLIST_FLD_GET(inplan_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
                plan_array = PIN_FLIST_ELEM_ADD(plan_list, PIN_FLD_PLAN, count, ebufp );
                PIN_FLIST_FLD_COPY(inplan_flistp, PIN_FLD_PLAN_OBJ, plan_array, PIN_FLD_PLAN_OBJ, ebufp );

                elem_id = 0;
                cookie = NULL;
                offercheck = 0;
                while((deals = PIN_FLIST_ELEM_GET_NEXT(services_array, PIN_FLD_DEALS,
                        &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
                {
                        deal_info = PIN_FLIST_SUBSTR_GET(deals, PIN_FLD_DEAL_INFO, 0, ebufp);
                        plan_pdp_2 = PIN_FLIST_FLD_GET(deals, PIN_FLD_PLAN_OBJ, 1, ebufp);
                        if(PIN_POID_COMPARE(plan_pdp_1, plan_pdp_2, 0, ebufp) == 0) {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan poid compare");
                                offercheck = 1;
                        }
                        elem_id_1 =0;
                        cookie_1 = NULL;
                        while((products_array = PIN_FLIST_ELEM_GET_NEXT(deal_info, PIN_FLD_PRODUCTS,
                                &elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
                        {
                                //plan_array = PIN_FLIST_ELEM_ADD(plan_list, PIN_FLD_PLAN, count, ebufp );
                                //PIN_FLIST_FLD_COPY(deals, PIN_FLD_PLAN_OBJ, plan_array, PIN_FLD_PLAN_OBJ, ebufp );

                                //New Tariff - Transaction mapping Start */
                                if (offercheck == 1){
                                        eflistp = PIN_FLIST_ELEM_ADD( plan_array,PIN_FLD_OFFERINGS, offer_count++, ebufp);
                                        PIN_FLIST_FLD_COPY(products_array, PIN_FLD_OFFERING_OBJ,eflistp, PIN_FLD_OFFERING_OBJ, ebufp);
                                        PIN_FLIST_FLD_COPY(products_array, PIN_FLD_PACKAGE_ID,eflistp, PIN_FLD_PACKAGE_ID, ebufp);
                                }
                                //New Tariff - Transaction mapping End */
                                count++;
                        }
                }
        }
        //New Tariff - Transaction mapping End */

	if (pymt_status && *pymt_status == 0 )
	{
		PIN_FLIST_ELEM_PUT(create_out_flist, payment_details_out, PIN_FLD_PAYMENT, 0, ebufp);
	}

	if (deposit_status && *deposit_status ==0)
	{
		PIN_FLIST_SUBSTR_PUT(create_out_flist, create_deposit_output, PIN_FLD_DEPOSITS, ebufp)
	}

	group_info = PIN_FLIST_SUBSTR_TAKE(mc_ret_flistp, PIN_FLD_GROUP_INFO, 1 , ebufp);
	if (group_info && group_info != NULL) 
	{
		PIN_FLIST_SUBSTR_PUT(create_out_flist, group_info, PIN_FLD_GROUP_INFO, ebufp );
	}

	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME ,create_out_flist, PIN_FLD_PROGRAM_NAME, ebufp);
	//fm_mso_cust_actv_lc_event(ctxp, i_flistp, create_out_flist, ebufp );
	fm_mso_create_lifecycle_evt(ctxp, i_flistp, create_out_flist, ID_ACTIVATE_CATV_SERVICE ,ebufp );
    if (subscriber_type == 48 && srvc_catv_count != 0 && acnt_no != NULL)
    {
        tmp_flist = PIN_FLIST_ELEM_GET(services_inp, PIN_FLD_DEVICES, 0, 1, ebufp);
        disc_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_POID,
                    PIN_POID_CREATE(PIN_POID_GET_DB(acnt_pdp), "/service", (int64)-1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_ACCOUNT_NO, acnt_no, ebufp);
        PIN_FLIST_FLD_COPY(tmp_flist, MSO_FLD_STB_ID, disc_flistp, PIN_FLD_DEVICE_ID, ebufp);
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
            PIN_FLIST_FLD_PUT(disc_flistp, PIN_FLD_DISCOUNT, pbo_decimal_from_str("0.6", ebufp), ebufp);
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Discount config create input", disc_flistp);
        PCM_OP(ctxp, MSO_OP_CUST_CATV_DISC_CONFIG, flags, disc_flistp, &disc_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&disc_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                    "Discount config create output", disc_oflistp);
        disc_status = (int32 *)PIN_FLIST_FLD_GET(disc_oflistp, PIN_FLD_STATUS, 1, ebufp);
        if ((disc_status && *disc_status == activate_customer_failure) || PIN_ERRBUF_IS_ERR(ebufp))
        {
            PIN_FLIST_DESTROY_EX(&disc_oflistp, ebufp);
            ret_status = 3;
            pin_set_err(ebufp, PIN_ERRLOC_CM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_DUPLICATE,
                PIN_FLD_DISCOUNT, 0, 0);
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Discount already configured", ebufp);
            goto CLEANUP;
        }
    }
	//Catch Unknown error
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_activate_customer error",ebufp);
		fm_mso_get_err_desc(ctxp, acnt_pdp, &ret_flistp, ebufp);
		goto CLEANUP;
	}
	else
	{
		ret_flistp = create_out_flist;
	}
	/*******************************************************************
	* Create Output flist - End
	*******************************************************************/

    /******************************************************************
     * Call ET function 
    *****************************************************************/
    bi_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp);
    if (bi_flistp)
    {
        billinfo_pdp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_POID, 0, ebufp);
        actg_next_t = *(time_t *)PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_ACTG_NEXT_T, 0, ebufp);
        mso_bill_pol_catv_main(ctxp, flags, billinfo_pdp, srvc_pdp, actg_next_t, 11002, &et_ret_flistp, ebufp);
    }	
	
	CLEANUP:
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Final ret_flistp",ret_flistp);
//	PIN_FLIST_DESTROY_EX(&payment_details, NULL);
	PIN_FLIST_DESTROY_EX(&mc_ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&service_details, NULL);
	PIN_FLIST_DESTROY_EX(&read_o_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&read_i_flistp,NULL);
//	PIN_FLIST_DESTROY_EX(&created_billinfo, NULL); 
//	PIN_FLIST_DESTROY_EX(&create_deposit_output, NULL);
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL); 
	return;
}

/**************************************************
* Function: fm_mso_populate_billinfo_and_balinfo()
* 
* 
***************************************************/
static void
fm_mso_populate_billinfo_and_balinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*existing_srvc_flist,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*billinfo = NULL;
	pin_flist_t		*services_array = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*service_catv_info = NULL;
	pin_flist_t		*bal_grp_out_flist = NULL;  
	pin_flist_t		*billinfo_out_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*limit = NULL;

	poid_t			*service_pdp = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*service_type = NULL;
	poid_t			*bal_grp_obj = NULL;
	poid_t			*billinfo_pdp = NULL;
	poid_t			*bal_info = NULL;
	poid_t			*pay_info_obj = NULL;

	int			elem_id = 0;
	pin_cookie_t		cookie = NULL;

	int64			db = -1;
	int32			conn_type = CATV_MAIN;
	int32			service_exist = 0;
	int32			flg_order_additional_catv=0;
	int32			flg_order_catv_main_bb_exist=0;
	int32			flg_order_bb_catv_exist=0;
	int32			actg_type = PIN_ACTG_TYPE_BALANCE_FORWARD;
	int32			pay_type = PIN_PAY_TYPE_INVOICE;
	int32			activate_customer_failure = 1;
	char			*order_service_type = NULL;
	pin_flist_t		*acc_read_inp = NULL;
	pin_flist_t		*acc_read_out = NULL;
	poid_t			*bal_pdp = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_populate_billinfo_and_balinfo", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_billinfo_and_balinfo input flist", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"acnt_pdp", acnt_pdp);

	services_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	order_service_type = (char*)PIN_POID_GET_TYPE((poid_t*)PIN_FLIST_FLD_GET(services_array, PIN_FLD_SERVICE_OBJ, 0, ebufp));

	db = PIN_POID_GET_DB(acnt_pdp);

	/*changes made to get the account level balance group*/
	acc_read_inp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(acc_read_inp,PIN_FLD_POID,acnt_pdp,ebufp);
	PIN_FLIST_FLD_SET(acc_read_inp,PIN_FLD_BAL_GRP_OBJ,NULL,ebufp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, acc_read_inp, &acc_read_out, ebufp);
	bal_pdp = PIN_FLIST_FLD_GET(acc_read_out,PIN_FLD_BAL_GRP_OBJ,0, ebufp);

	service_exist = PIN_FLIST_ELEM_COUNT(existing_srvc_flist, PIN_FLD_RESULTS, ebufp);
	if (service_exist == 0 )// No service exist for the account
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No service exist for the account");
		//Assign Account level billinfo and new service level balance_group
		fm_mso_read_bal_grp(ctxp, acnt_pdp, NULL, &bal_grp_out_flist, ebufp);
		bal_grp_obj = PIN_FLIST_FLD_GET(bal_grp_out_flist, PIN_FLD_POID, 0, ebufp );
		fm_mso_get_billinfo(ctxp, bal_grp_obj, &billinfo_pdp, ebufp );
		fm_mso_read_billinfo(ctxp, billinfo_pdp, &billinfo_out_flist, ebufp );
		PIN_FLIST_DESTROY_EX(&bal_grp_out_flist, NULL);

		billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp);

		PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, billinfo_pdp, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "CATV", ebufp);
//		PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);

		PIN_FLIST_ELEM_SET(services_array, NULL,PIN_FLD_BAL_INFO, 0, ebufp );
		//PIN_FLIST_FLD_SET(services_array, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);

		bal_info = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BAL_INFO, 0,0, ebufp);
		PIN_FLIST_FLD_SET(bal_info, PIN_FLD_POID,bal_grp_obj, ebufp);
		//PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
		//PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "Service Balance Group", ebufp);
		
	//	limit = PIN_FLIST_ELEM_ADD(bal_info, PIN_FLD_LIMIT, CURRENCY_INR, ebufp);
	//	PIN_FLIST_FLD_SET(limit, PIN_FLD_CREDIT_LIMIT, NULL, ebufp);
	//	PIN_FLIST_FLD_SET(limit, PIN_FLD_CREDIT_FLOOR, NULL, ebufp);

		PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);


		/************************************************************* 
		* calling fm_mso_populate_payinfo() will create a new payinfo
		* 
		fm_mso_populate_payinfo(ctxp, i_flistp, ret_flistp, ebufp );
		**************************************************************/
		//Add Payinfo
		pay_info_obj = PIN_FLIST_FLD_TAKE(billinfo_out_flist, PIN_FLD_PAYINFO_OBJ, 0, ebufp);
		payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
		if (payinfo)
		{
			PIN_FLIST_FLD_PUT(payinfo, PIN_FLD_POID, pay_info_obj, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_PAYINFO_OBJ, pay_info_obj, ebufp);
		}
		
		PIN_FLIST_DESTROY_EX(&billinfo_out_flist, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp) )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error Populating Payinfo", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51067", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error Populating Payinfo", ebufp);
			goto CLEANUP;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Service exist for the account");
		while((result_flist = PIN_FLIST_ELEM_GET_NEXT(existing_srvc_flist, PIN_FLD_RESULTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			if (flg_order_additional_catv)
			{
				break;
			}
			service_type = (char*)PIN_POID_GET_TYPE( (poid_t*)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp));

			// order for CATV activation and only BB exist for the account
			if ( (strcmp(order_service_type, SRVC_CATV) == 0) &&
			     (strcmp(service_type, SRVC_BB) ==0 ) &&
			      flg_order_additional_catv ==0
			   ) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"order for CATV activation and only BB exist for the account");
				flg_order_catv_main_bb_exist = 1;
			}
			// order for CATV activation and CATV exist for the account
			if ( (strcmp(order_service_type, SRVC_CATV) == 0) &&
			     (strcmp(service_type, SRVC_CATV) ==0)
			   ) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"order for CATV activation and CATV exist for the account");
				flg_order_additional_catv = 1; //Means already have 1 or more CATV existing
				bal_grp_obj =  PIN_FLIST_FLD_GET(result_flist, PIN_FLD_BAL_GRP_OBJ, 0, ebufp );
			}
			// order for BB activation and CATV exist for the account
			if ( (strcmp(order_service_type, SRVC_BB) == 0) &&
			     (strcmp(service_type, SRVC_CATV) ==0)
			   ) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"order for BB activation and CATV exist for the account");
				flg_order_bb_catv_exist = 1;
			}
		}

		if (flg_order_catv_main_bb_exist || flg_order_bb_catv_exist )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Assign new service level balance_group and new service level billinfo");
			//Assign new service level balance_group and new service level billinfo
			fm_mso_read_bal_grp(ctxp, acnt_pdp, NULL, &bal_grp_out_flist, ebufp);
			bal_grp_obj = PIN_FLIST_FLD_GET(bal_grp_out_flist, PIN_FLD_POID, 0, ebufp );
			fm_mso_get_billinfo(ctxp, bal_grp_obj, &billinfo_pdp, ebufp );
			fm_mso_read_billinfo(ctxp, billinfo_pdp, &billinfo_out_flist, ebufp );
			PIN_FLIST_DESTROY_EX(&bal_grp_out_flist, NULL);

			billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp);
			if (!billinfo)
			{
				billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 0, ebufp);
				PIN_FLIST_FLD_COPY(billinfo_out_flist, PIN_FLD_BILL_WHEN, billinfo, PIN_FLD_BILL_WHEN, ebufp );
				PIN_FLIST_FLD_COPY(billinfo_out_flist, PIN_FLD_ACTG_FUTURE_DOM, billinfo, PIN_FLD_ACTG_FUTURE_DOM, ebufp );
			}
			PIN_FLIST_FLD_SET(billinfo, PIN_FLD_POID, PIN_POID_CREATE(db, "/billinfo", -1, ebufp ), ebufp);
			PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
			PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "CATV", ebufp);
//			PIN_FLIST_ELEM_SET(billinfo, NULL, PIN_FLD_PAYINFO, 0, ebufp);

			PIN_FLIST_ELEM_SET(services_array, NULL, PIN_FLD_BAL_INFO, 0, ebufp );

			bal_info = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BAL_INFO, 0,0, ebufp);
			PIN_FLIST_FLD_SET(bal_info, PIN_FLD_POID,bal_pdp, ebufp);
			//PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
			PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);
			
			/************************************************************* 
			* calling fm_mso_populate_payinfo() will create a new payinfo
			* 
			fm_mso_populate_payinfo(ctxp, i_flistp, ret_flistp, ebufp );
			**************************************************************/	
			//Add Payinfo
			pay_info_obj = PIN_FLIST_FLD_TAKE(billinfo_out_flist, PIN_FLD_PAYINFO_OBJ, 0, ebufp);
			payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
			if (payinfo)
			{
				PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_PAYINFO_OBJ, pay_info_obj, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_PUT(payinfo, PIN_FLD_POID, pay_info_obj, ebufp);
			}
			
			PIN_FLIST_DESTROY_EX(&billinfo_out_flist, NULL);

			if (PIN_ERRBUF_IS_ERR(ebufp) )
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error Populating Payinfo, service exist", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51068", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error Populating Payinfo, service exist", ebufp);
				goto CLEANUP;
			}
			
		}
		if (flg_order_additional_catv)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Assign balance_group and service level billinfo from an existing CATV");
			//Assign balance_group and service level billinfo from an existing CATV
			fm_mso_get_billinfo(ctxp, bal_grp_obj, &billinfo_pdp, ebufp );
			fm_mso_read_billinfo(ctxp, billinfo_pdp, &billinfo_out_flist, ebufp );
			billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp);
			if (!billinfo)
			{
				billinfo = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BILLINFO, 0, ebufp);
				PIN_FLIST_FLD_COPY(billinfo_out_flist, PIN_FLD_BILL_WHEN, billinfo, PIN_FLD_BILL_WHEN, ebufp );	
				PIN_FLIST_FLD_COPY(billinfo_out_flist, PIN_FLD_ACTG_FUTURE_DOM, billinfo, PIN_FLD_ACTG_FUTURE_DOM, ebufp );
			}
			PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_POID, billinfo_pdp, ebufp);
			PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
			PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, "CATV", ebufp);

			PIN_FLIST_ELEM_SET(services_array, NULL, PIN_FLD_BAL_INFO, 0, ebufp );

			bal_info = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BAL_INFO, 0, 0, ebufp);
			//PIN_FLIST_FLD_SET(bal_info, PIN_FLD_POID, bal_grp_obj, ebufp);
			PIN_FLIST_FLD_SET(bal_info, PIN_FLD_POID, bal_pdp, ebufp);	
		
			PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);

			//Add Payinfo
			pay_info_obj = PIN_FLIST_FLD_TAKE(billinfo_out_flist, PIN_FLD_PAYINFO_OBJ, 0, ebufp);
			payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
			if (payinfo)
			{
				PIN_FLIST_FLD_PUT(payinfo, PIN_FLD_POID, pay_info_obj, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_PUT(billinfo, PIN_FLD_PAYINFO_OBJ, pay_info_obj, ebufp);
			}
			PIN_FLIST_DESTROY_EX(&billinfo_out_flist, NULL);

			//Update services.catv_info.PIN_FLD_CONN_TYPE
			service_catv_info = PIN_FLIST_SUBSTR_GET(services_array, MSO_FLD_CATV_INFO, 1, ebufp);
			if (!service_catv_info)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing services.MSO_FLD_CATV_INFO", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51069", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Missing services.MSO_FLD_CATV_INFO", ebufp);
				goto CLEANUP;
			}
			//PIN_FLIST_FLD_SET(service_catv_info, PIN_FLD_CONN_TYPE, &conn_type, ebufp);
		}
	}
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
	PIN_FLIST_DESTROY_EX(&acc_read_inp, NULL);
	PIN_FLIST_DESTROY_EX(&acc_read_out, NULL);
	CLEANUP:
	return;
}

static void
fm_mso_validate_plan_and_device(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*services_array = NULL;
	pin_flist_t		*plan_array = NULL;
	pin_flist_t		*deal_array = NULL;
	pin_flist_t		*cr_lmt_array = NULL;
	pin_flist_t		*device_iflist = NULL;
	pin_flist_t		*device_oflist = NULL;
	pin_flist_t		*srch_profile_iflist = NULL;
	pin_flist_t		*srch_profile_oflist = NULL;
	pin_flist_t		*devices_array = NULL;
	pin_flist_t		*wholesale_info = NULL;
	pin_flist_t		*billinfo = NULL;
	pin_flist_t		*service_catv_info = NULL;
	pin_flist_t		*bal_info = NULL;
	pin_flist_t		*deals_deposit = NULL;
	pin_flist_t		*deals_activation = NULL;
	pin_flist_t		*preactivated_svc_info = NULL;
	pin_flist_t		*prov_data = NULL;
	pin_flist_t		*ret_flistp_1 = NULL;

	poid_t			*acnt_pdp = NULL;
	poid_t			*device_lco_pdp = NULL;	
	poid_t			*tmp_device_lco_pdp = NULL;	
	poid_t			*profile_lco_pdp = NULL;
	poid_t			*device_pdp = NULL;
	poid_t			*alias_list = NULL;

	char			*service_type = NULL;
	char			*device_id = NULL;
	char			*cas = NULL;
	char			network_node[60];
	char			srvc_login[60];
	char			*device_type = NULL;
	char			*device_pdp_type = NULL;

	int			device_state = -1;
	int32			device_search_type = MSO_FLAG_SRCH_BY_ID;
	int32			profile_srch_type = MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER;
	int32			activate_customer_failure = 1;
	int32			billing_dom = 0;
	int32			mso_device_type = MSO_DEVICE_TYPE_SD;
	int			elem_id_1 = 0;
	int32			*ret_status = NULL;

	pin_cookie_t		cookie_1 = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_validate_plan_and_device", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_plan_and_device ", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	/************************************************************************
	PLAN VALIDATION:
	Read plans to get deal and credit limit
	START
	************************************************************************/

	plan_array = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PLAN_LIST_CODE, 0, ebufp);
	if (!plan_array)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing PIN_FLD_PLAN_LIST_CODE", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51085", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Missing PIN_FLD_PLAN_LIST_CODE", ebufp);
		goto CLEANUP;
	}
	
	fm_mso_get_deals_from_plan(ctxp, plan_array, &deal_array, &cr_lmt_array, ebufp);  //For getting deals from plans passed
  	if ((!deal_array) || (!cr_lmt_array) )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in getting Plan Details", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51086", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in getting Plan Details", ebufp);
		goto CLEANUP;
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_deals_from_plan() error",ebufp);
		fm_mso_get_err_desc(ctxp, acnt_pdp, ret_flistp, ebufp);
		goto CLEANUP;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deal_array", deal_array);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cr_lmt_array", cr_lmt_array);	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");

	fm_mso_get_deal_info(ctxp, deal_array, &deals_deposit, &deals_activation,ebufp);
	PIN_FLIST_DESTROY_EX(&deal_array, NULL);
  	if ( (PIN_ERRBUF_IS_ERR(ebufp)))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fetching deal info", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51087", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in fetching deal info", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_deposit", deals_deposit);
	if (deals_deposit)
	{
		PIN_FLIST_ELEM_PUT(i_flistp, deals_deposit, PIN_FLD_DEPOSIT_PLANS, 0, ebufp );
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "i_flistp", i_flistp);
	/************************************************************************
	PLAN VALIDATION:
	Read plans to get deal and credit limit
	END
	************************************************************************/

	/************************************************************************
	DEVICE VALIDATION:
	Read device poid from ID, validate status
	START
	************************************************************************/
	services_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);

	if (!services_array)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Missing PIN_FLD_SERVICES array", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51088", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Missing PIN_FLD_SERVICES array", ebufp);
		goto CLEANUP;
	}
	PIN_FLIST_CONCAT(services_array, deals_activation, ebufp);
	PIN_FLIST_DESTROY_EX(&deals_activation, NULL);


	//Add  PIN_FLD_BAL_INFO in input Flist under SERVICES Array
	bal_info = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
	if (cr_lmt_array)
	{
		PIN_FLIST_CONCAT(bal_info, cr_lmt_array, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&cr_lmt_array, NULL);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "services_array", services_array);

	service_type = (char *)PIN_POID_GET_TYPE( (poid_t *)PIN_FLIST_FLD_GET(services_array, PIN_FLD_SERVICE_OBJ, 1, ebufp ));
	if ( strcmp(service_type, SRVC_CATV) == 0 )
	{
		/*******************************************************************
		*	CATV Validation - Start
		*******************************************************************/
		elem_id_1 = 0;
		cookie_1 = NULL;
		while((devices_array = PIN_FLIST_ELEM_GET_NEXT(services_array, PIN_FLD_DEVICES,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "devices_array: ", devices_array);
			
			/*device_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(device_iflist, PIN_FLD_POID, acnt_pdp, ebufp );*/
			
			if (elem_id_1 == 0) //device STB at elem_id=0
			{
				device_id = PIN_FLIST_FLD_GET(devices_array, MSO_FLD_STB_ID, 1, ebufp);
			}
			if (elem_id_1 == 1) //device VC at elem_id=1
			{
				device_id = PIN_FLIST_FLD_GET(devices_array, MSO_FLD_VC_ID, 1, ebufp);
			}
			
			if (!device_id)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing MSO_FLD_STB_ID or MSO_FLD_VC_ID", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51089", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Missing MSO_FLD_STB_ID or MSO_FLD_VC_ID", ebufp);
				goto CLEANUP;	
			}
			
			/******** VERIMATRIX CHANGES - Begin *****************************
			 * Added check to omit VC_ID field from being set as login
			 * applies to cases where device ID values are NA or N/A
			 * to avoid "device not found error" 
			 *****************************************************************/
			
			if ((device_id) && (strcmp(device_id, "N/A") != 0) && (strcmp(device_id, "NA") != 0))
			{
				device_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, device_iflist, PIN_FLD_USERID, ebufp );
				PIN_FLIST_FLD_SET(device_iflist, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(device_iflist, PIN_FLD_DEVICE_ID, device_id, ebufp );
				PIN_FLIST_FLD_SET(device_iflist, PIN_FLD_SEARCH_TYPE, &device_search_type, ebufp );
			}
			else 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Device ID is NA. Hence exiting the process");
				goto SETLOGIN; // Avoids device search having IDs as NA
			}
			
			/******** VERIMATRIX CHANGES - End *****************************
			 * Added check to omit VC_ID field from being set as login
			 * applies to cases where device ID values are NA or N/A
			 * to avoid "device not found error" 
			 *****************************************************************/
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "device_iflist:", device_iflist);
			
			fm_mso_get_device_info(ctxp, device_iflist, &device_oflist, ebufp );
			PIN_FLIST_DESTROY_EX(&device_iflist, NULL);
			if (!device_oflist)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Device details not present in BRM", ebufp);
				PIN_ERRBUF_RESET(ebufp);
 				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51090", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device details not present in BRM", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_plan_and_device: device_oflist", device_oflist);
			
			device_pdp = PIN_FLIST_FLD_GET(device_oflist, PIN_FLD_POID, 0, ebufp);
			device_pdp_type = (char *)PIN_POID_GET_TYPE(device_pdp);
			device_state = *(int *)PIN_FLIST_FLD_GET(device_oflist, PIN_FLD_STATE_ID, 0, ebufp);
			cas = (char*)PIN_FLIST_FLD_GET(device_oflist, PIN_FLD_MANUFACTURER, 0, ebufp);
			device_type =  (char*)PIN_FLIST_FLD_GET(device_oflist, PIN_FLD_DEVICE_TYPE, 0, ebufp);
			PIN_FLIST_FLD_SET(devices_array, PIN_FLD_DEVICE_TYPE, device_type, ebufp );
			if (device_type && (strstr(device_type, "HD")) )
			{
				mso_device_type = MSO_DEVICE_TYPE_HD;
			}

			if ( !(device_state == READY_TO_USE || device_state == PREACTIVATED || device_state == REPAIRED_TO_USE ))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Device state not READY_TO_USE/PREACTIVATED/REPAIRED_TO_USE", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51091", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device state not READY_TO_USE/PREACTIVATED/REPAIRED_TO_USE", ebufp);
				goto CLEANUP;
			}
			if (device_state == PREACTIVATED && elem_id_1 == 0 )
			{
				fm_mso_get_preactivated_svc_info(ctxp, device_pdp, &preactivated_svc_info, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in retrieving preactivated_svc_info", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51092", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in retrieving preactivated_svc_info", ebufp);
					goto CLEANUP;
				}
				if (!preactivated_svc_info)
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"preactivated_svc_info returned NULL", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51093", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "preactivated_svc_info returned NULL", ebufp);
					goto CLEANUP;
				}
				//PIN_FLIST_SUBSTR_PUT(services_array, preactivated_svc_info, PIN_FLD_PROV_DATA, ebufp);
				prov_data = PIN_FLIST_SUBSTR_ADD(services_array, PIN_FLD_PROV_DATA, ebufp);
				PIN_FLIST_CONCAT(prov_data, preactivated_svc_info, ebufp );
				PIN_FLIST_DESTROY_EX(&preactivated_svc_info, NULL);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "services_array", services_array );
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "i_flistp", i_flistp );

			}
			if (!device_lco_pdp)
			{
				device_lco_pdp = PIN_FLIST_FLD_TAKE(device_oflist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			}
			else
			{
				tmp_device_lco_pdp = PIN_FLIST_FLD_GET(device_oflist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
//				if (PIN_POID_COMPARE(device_lco_pdp, tmp_device_lco_pdp, 0, ebufp) != 0 )
//				{
//					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"All devices not belong to same LCO", ebufp);
//					PIN_ERRBUF_RESET(ebufp);
//					*ret_flistp = PIN_FLIST_CREATE(ebufp);
//					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
//					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
//					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51094", ebufp);
//					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "All devices not belong to same LCO", ebufp);
//					goto CLEANUP;
//				}
			}
			PIN_FLIST_FLD_SET(devices_array, PIN_FLD_DEVICE_OBJ, device_pdp, ebufp );
			if (elem_id_1 == 0 )
			{
				strcpy(network_node,cas);
			}
			/****************************************************************
			* Populate Alias List & LOGIN
			****************************************************************/
			if (strstr(cas ,"NDS") )
			{
				if ( (PIN_FLIST_ELEM_COUNT(services_array, PIN_FLD_DEVICES, ebufp ) != 2 ) ||
				     (elem_id_1 == 0 && (!device_id || strcmp(device_pdp_type, "/device/stb") !=0))
				    )
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"STB is mandatory for manufacturer CAS", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51095", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "STB is mandatory for manufacturer CAS", ebufp);
					goto CLEANUP;
				}
				//strcpy(srvc_login, "xxx:yyy:");	
			}
			else if(strstr(cas ,"CISCO") )
			{
				//strcpy(srvc_login, "xxx:");
			}
			strcpy(network_node,cas);

			// To be added Device and Plan compatibility Checks
			PIN_FLIST_DESTROY_EX(&device_oflist, NULL);
		} //end while loop

		//Update Service Login/Password

		/******** VERIMATRIX CHANGES - Begin *****************************
		 * Added check to omit VC_ID field from being set as login
		 * applies to cases where device ID values are NA or N/A
		 * to avoid "device not found error" 
		 *****************************************************************/
		SETLOGIN:
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting logins");
		
		if ((strcmp(device_id, " ") != 0) && (strcmp(device_id, "N/A") != 0) && (strcmp(device_id, "NA") != 0)) {
			strcpy(srvc_login, "L-");
			strcat(srvc_login, device_id);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,srvc_login );
			PIN_FLIST_FLD_SET(services_array, PIN_FLD_LOGIN, srvc_login, ebufp );
			PIN_FLIST_FLD_SET(services_array, PIN_FLD_PASSWD_CLEAR, srvc_login, ebufp );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,srvc_login );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "services_array", services_array );			
		} else {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "device is NA");
		}
		
		/* Verimatrix - Removing device IDs with "NA" */
		elem_id_1 = 0;
		cookie_1 = NULL;
		while((devices_array = PIN_FLIST_ELEM_GET_NEXT(services_array, PIN_FLD_DEVICES,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "devices array:", devices_array);
			if (elem_id_1 == 1) //device VC at elem_id=1
			{
				device_id = PIN_FLIST_FLD_GET(devices_array, MSO_FLD_VC_ID, 1, ebufp);
				if ((strcmp(device_id, "N/A") == 0) || (strcmp(device_id, "NA") == 0)) {
					PIN_FLIST_ELEM_DROP(services_array, PIN_FLD_DEVICES, 1, ebufp);
				}
			}
		}
				
		/******** VERIMATRIX CHANGES - End *****************************
		 * Added check to omit VC_ID field from being set as login
		 * applies to cases where device ID values are NA or N/A
		 * to avoid "device not found error" 
		 *****************************************************************/
		PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_NETWORK_NODE, network_node, ebufp); 

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "7777");
		//Update services.catv_info.MSO_FLD_NETWORK_NODE
		service_catv_info = PIN_FLIST_SUBSTR_GET(services_array, MSO_FLD_CATV_INFO, 1, ebufp);
		if (!service_catv_info)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Missing services.MSO_FLD_CATV_INFO", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51096", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Missing services.MSO_FLD_CATV_INFO", ebufp);
			goto CLEANUP;
		}
		PIN_FLIST_FLD_SET(service_catv_info, MSO_FLD_NETWORK_NODE, network_node, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "service_catv_info", service_catv_info );

		/******************************************************************* 
		* validate Registering LCO and Device LCO - Start
		*******************************************************************/ 
		srch_profile_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(srch_profile_iflist, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(srch_profile_iflist, PIN_FLD_PROFILE_DESCR, WHOLESALE, ebufp );
		PIN_FLIST_FLD_SET(srch_profile_iflist, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp );
		PIN_FLIST_FLD_SET(srch_profile_iflist, PIN_FLD_OBJECT, acnt_pdp, ebufp );

		fm_mso_get_profile_info(ctxp, srch_profile_iflist, &srch_profile_oflist, ebufp );
		PIN_FLIST_DESTROY_EX(&srch_profile_iflist, NULL);
		if (!srch_profile_oflist)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Profile details not present in BRM", ebufp);
			PIN_ERRBUF_RESET(ebufp);
 			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51097", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Profile details not present in BRM", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_plan_and_device: srch_profile_oflist", srch_profile_oflist);
		profile_lco_pdp = PIN_FLIST_FLD_GET(srch_profile_oflist, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
		if (PIN_POID_COMPARE(device_lco_pdp, profile_lco_pdp, 0, ebufp) != 0 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Registering LCO not matching Device LCO");
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51098", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Registering LCO not matching Device LCO", ebufp);
			goto CLEANUP;
		}

		//Update services.catv_info.MSO_FLD_BOUQUET_ID
		PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_LCO_OBJ, profile_lco_pdp, ebufp);
		fm_mso_populate_bouquet_id(ctxp, i_flistp, mso_device_type, 0, &ret_flistp_1, ebufp);
		if (ret_flistp_1)
		{
			ret_status = PIN_FLIST_FLD_GET(ret_flistp_1, PIN_FLD_STATUS, 1, ebufp);
			if (ret_status && *ret_status == 1)
			{	
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "88888");
				*ret_flistp = PIN_FLIST_COPY(ret_flistp_1, ebufp);
				PIN_FLIST_DESTROY_EX(&ret_flistp_1, NULL);
				goto CLEANUP;
			}
			else
			{
				PIN_FLIST_DESTROY_EX(&ret_flistp_1, NULL);
			}
		}

		/******************************************************************* 
		* Update billinfo-Start
		*******************************************************************/
		wholesale_info = PIN_FLIST_SUBSTR_GET(srch_profile_oflist, MSO_FLD_WHOLESALE_INFO, 0, ebufp);
		if(wholesale_info && wholesale_info != NULL)
		billing_dom = *(int32*)PIN_FLIST_FLD_GET(wholesale_info, MSO_FLD_PREF_DOM, 0, ebufp);
		billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0,1, ebufp);
		if (billinfo && billinfo != NULL)
		{
			PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_FUTURE_DOM, &billing_dom, ebufp);
		}
		PIN_FLIST_DESTROY_EX(&srch_profile_oflist, NULL);
		//PIN_POID_DESTROY(device_lco_pdp, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "device_lco_pdp",device_lco_pdp );
		/******************************************************************* 
		* validate Registering LCO and Device LCO - End
		*******************************************************************/
		
		/*******************************************************************
		*	CATV Validation - End
		*******************************************************************/
	}
	/*******************************************************************
	*	Device Validation - End
	*******************************************************************/

	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_STATE_ID, &device_state, ebufp);
	
	//PIN_FLIST_SUBSTR_DROP(i_flistp, PIN_FLD_PLAN_LIST_CODE, ebufp);

	CLEANUP:
	//PIN_FLIST_DESTROY_EX(&device_iflist, NULL);
	//PIN_FLIST_DESTROY_EX(&device_oflist, NULL); 
	//PIN_FLIST_DESTROY_EX(&srch_profile_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&deals_activation, NULL);
	if (!(PIN_POID_IS_NULL(device_lco_pdp)))
	{
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "device_lco_pdp",device_lco_pdp );
		PIN_POID_DESTROY(device_lco_pdp, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "device_lco_pdp",device_lco_pdp );
	}
	return;
}


/**************************************************
* Function: fm_mso_update_catv_bb_info()
* 
* 
***************************************************/
static void
fm_mso_update_catv_bb_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*gen_seq_iflist = NULL;
	pin_flist_t		*gen_seq_oflist = NULL;
	pin_flist_t		*services_array = NULL;
	pin_flist_t		*srvc_catv_info = NULL;
	pin_flist_t		*read_flds_input = NULL;
	pin_flist_t		*read_flds_output = NULL;
	pin_flist_t		*nameinfo = NULL;
	pin_flist_t		*statuses_array = NULL;
	pin_flist_t             *readacct_inflistp = NULL;
	pin_flist_t             *racct_outflistp = NULL;

	poid_t			*acnt_pdp = NULL;

	int32			seq = 0;
	int32			activate_customer_failure = 1;
	int32			service_status_inactive = PIN_STATUS_INACTIVE;
	int32			service_status_flags = 0;
	char			*template = "select X from /service where F1.id = V1 and F2 = V2 ";
	char			*service_type = NULL;
	char			agr_no[20];
	char			*zip_code = NULL;
	char			region_key[20];
	char			*err_code = NULL;
	int32			*business_type = NULL;
	int32			arr_business_type[8];
	int32			den_hw_flag = -1;
	char			bus_type_str[8];

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_catv_bb_info", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_catv_bb_info input flist", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	/**********************************************************************************
	*  Populate Agreement Number -Start
	***********************************************************************************/
	gen_seq_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(gen_seq_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(gen_seq_iflist, PIN_FLD_NAME, "MSO_SEQUENCE_TYPE_AGREEMENT", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_catv_bb_info gen_seq_iflist", gen_seq_iflist);

	fm_mso_utils_sequence_no(ctxp, gen_seq_iflist, &gen_seq_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&gen_seq_iflist, NULL);
/*	if ((!gen_seq_oflist))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error populating agreement number", ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51070", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error populating agreement number", ebufp);
		goto CLEANUP;
	} 
	if (PIN_ERRBUF_IS_ERR(ebufp) )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_utils_sequence_no()", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51050", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error populating agreement number", ebufp);
		goto CLEANUP;
	}
*/
	if(gen_seq_oflist && gen_seq_oflist != NULL)
	err_code = (char*)PIN_FLIST_FLD_GET(gen_seq_oflist, PIN_FLD_ERROR_CODE, 1, ebufp);
	if ( err_code )
	{
		*ret_flistp = PIN_FLIST_COPY(gen_seq_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&gen_seq_oflist, NULL);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_catv_bb_info gen_seq_oflist", gen_seq_oflist);
	
	seq = *(int32*)PIN_FLIST_FLD_GET(gen_seq_oflist, PIN_FLD_HEADER_NUM, 1, ebufp);
	PIN_FLIST_DESTROY_EX(&gen_seq_oflist, NULL);

	services_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp); 
	if(services_array && services_array != NULL)
	statuses_array = PIN_FLIST_ELEM_ADD(services_array, PIN_FLD_STATUSES, 0, ebufp);
	if(statuses_array && statuses_array != NULL)
	PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS, &service_status_inactive, ebufp);
	PIN_FLIST_FLD_SET(statuses_array, PIN_FLD_STATUS_FLAGS,  &service_status_flags, ebufp);

	service_type = (char *)PIN_POID_GET_TYPE( (poid_t *)PIN_FLIST_FLD_GET(services_array, PIN_FLD_SERVICE_OBJ, 1, ebufp ));
	if (strcmp(service_type, SRVC_CATV) ==0)
	{
		sprintf(agr_no, "%s%s%010d", "CA", "-",seq);
	}
	if (strcmp(service_type, SRVC_BB) ==0)
	{
		sprintf(agr_no, "%s%s%010d", "BB", "-",seq);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "AGR_NO:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, agr_no);

	srvc_catv_info = PIN_FLIST_SUBSTR_GET(services_array, MSO_FLD_CATV_INFO, 1, ebufp);
	PIN_FLIST_FLD_SET(srvc_catv_info, MSO_FLD_AGREEMENT_NO, agr_no, ebufp);
  	/**********************************************************************************
	*  Populate Agreement Number -End
	***********************************************************************************/
  	/**********************************************************************************
	*  Populate Region Key -Start
	***********************************************************************************/
	read_flds_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_POID, acnt_pdp, ebufp );
	nameinfo = PIN_FLIST_ELEM_ADD(read_flds_input, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, ebufp) ;
	PIN_FLIST_FLD_SET(nameinfo, PIN_FLD_ZIP, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_catv_bb_info read input list", read_flds_input);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_input, &read_flds_output, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_input, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Populating REGION_KEY", ebufp);
		PIN_ERRBUF_RESET(ebufp);

		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51073", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Populating REGION_KEY", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_catv_bb_info READ output list", read_flds_output);
	if(read_flds_output && read_flds_output != NULL)
	nameinfo = PIN_FLIST_ELEM_GET(read_flds_output, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 1, ebufp );
	zip_code = (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_ZIP, 0, ebufp);
	
	/**************************** Read ACCT object **********************************/
	readacct_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(readacct_inflistp, PIN_FLD_POID, acnt_pdp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account read account input list", readacct_inflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, readacct_inflistp, &racct_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&readacct_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in accessing account or the account does not exist", ebufp);
		PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51073", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, 
					"Error in Populating REGION_KEY", ebufp);
		PIN_FLIST_DESTROY_EX(&racct_outflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account read account output flist", racct_outflistp);
	  
	if(racct_outflistp && racct_outflistp != NULL)
	{
		business_type = (int32 *)PIN_FLIST_FLD_GET(racct_outflistp, 
						PIN_FLD_BUSINESS_TYPE, 1, ebufp);
	}

	if(business_type && business_type != NULL)     
        sprintf(bus_type_str, "%ld", *business_type);

	num_to_array(*business_type, arr_business_type);
	den_hw_flag = arr_business_type[7];

        if ( bus_type_str
             && strncmp(bus_type_str, "9930", strlen("9930")) == 0)
	{
		strcpy(region_key, "HB");
	}
	else if (den_hw_flag > 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This is the DEN service region key");
		strcpy(region_key, "00");
	}
	else
	{
		strcpy(region_key, "IN");
	}
	strcat(region_key, zip_code);
	PIN_FLIST_FLD_SET(srvc_catv_info, MSO_FLD_REGION_KEY, region_key, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_output, NULL);
 
	/**********************************************************************************
	*  Populate Region Key -End
	***********************************************************************************/	
	CLEANUP:
	return;
}

/**************************************************
* Function: fm_mso_populate_payinfo()
* 
* 
***************************************************/
static void
fm_mso_populate_payinfo(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_flds_iflistp = NULL;
	pin_flist_t		*read_flds_oflistp = NULL;
	pin_flist_t		*nameinfo = NULL;
	pin_flist_t		*payinfo_array = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*inv_info = NULL;

	poid_t			*acnt_pdp = NULL;

	int32			inv_type = PIN_INV_TYPE_SUMMARY;
	int32			pay_type = PIN_PAY_TYPE_INVOICE;
	int32			delivery_pref = PIN_INV_USP_DELIVERY;
	int32			payment_term = 101;
	int32			activate_customer_failure =1;

	int64			db = -1;

	char			name[200];

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_populate_payinfo", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_payinfo input flist", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acnt_pdp);

	read_flds_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflistp, PIN_FLD_POID, acnt_pdp, ebufp );
	nameinfo = PIN_FLIST_ELEM_ADD(read_flds_iflistp, PIN_FLD_NAMEINFO, NAMEINFO_BILLING, ebufp) ;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist", read_flds_iflistp);
	
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflistp, &read_flds_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS: fm_mso_populate_payinfo()", ebufp);
		PIN_ERRBUF_RESET(ebufp);

		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51099", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_READ_FLDS: fm_mso_populate_payinfo()", ebufp);

		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist", read_flds_oflistp);
	nameinfo = PIN_FLIST_ELEM_GET(read_flds_oflistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 1, ebufp );

	//Add  PIN_FLD_PAYINFO
	payinfo_array =  PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_PAYINFO, 0, ebufp );
	PIN_FLIST_FLD_PUT(payinfo_array, PIN_FLD_POID, PIN_POID_CREATE(db, "/payinfo/invoice", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(payinfo_array, PIN_FLD_PAYMENT_TERM, &payment_term, ebufp);

	PIN_FLIST_FLD_SET(payinfo_array, PIN_FLD_INV_TYPE, &inv_type , ebufp );
	PIN_FLIST_FLD_SET(payinfo_array, PIN_FLD_PAY_TYPE, &pay_type , ebufp );

	inherited_info = PIN_FLIST_SUBSTR_ADD(payinfo_array, PIN_FLD_INHERITED_INFO, ebufp );
	inv_info = PIN_FLIST_ELEM_ADD(inherited_info, PIN_FLD_INV_INFO, 0, ebufp );

	PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_PREFER, &delivery_pref  , ebufp );
	PIN_FLIST_FLD_PUT(inv_info, PIN_FLD_ADDRESS, (PIN_FLIST_FLD_TAKE(nameinfo, PIN_FLD_ADDRESS, 0, ebufp))  , ebufp );
	PIN_FLIST_FLD_PUT(inv_info, PIN_FLD_CITY,    (PIN_FLIST_FLD_TAKE(nameinfo, PIN_FLD_CITY, 0, ebufp))  , ebufp );
	PIN_FLIST_FLD_PUT(inv_info, PIN_FLD_COUNTRY, (PIN_FLIST_FLD_TAKE(nameinfo, PIN_FLD_COUNTRY, 0, ebufp))  , ebufp );
	PIN_FLIST_FLD_PUT(inv_info, PIN_FLD_STATE, (PIN_FLIST_FLD_TAKE(nameinfo, PIN_FLD_STATE, 0, ebufp))  , ebufp );
	PIN_FLIST_FLD_PUT(inv_info, PIN_FLD_ZIP, (PIN_FLIST_FLD_TAKE(nameinfo, PIN_FLD_ZIP, 0, ebufp))  , ebufp );
	PIN_FLIST_FLD_PUT(inv_info, PIN_FLD_EMAIL_ADDR, (PIN_FLIST_FLD_TAKE(nameinfo, PIN_FLD_EMAIL_ADDR, 0, ebufp))  , ebufp );

	PIN_FLIST_FLD_SET(inv_info, PIN_FLD_DELIVERY_DESCR, "Delivery By Mail", ebufp);
	strcpy(name, (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_FIRST_NAME, 0, ebufp));
	strcat(name, " ");
	strcat(name, (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_MIDDLE_NAME, 0, ebufp));
	strcat(name, " ");
	strcat(name, (char*)PIN_FLIST_FLD_GET(nameinfo, PIN_FLD_LAST_NAME, 0, ebufp));

	PIN_FLIST_FLD_SET(inv_info, PIN_FLD_NAME, name, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "payinfo_array", payinfo_array );


	CLEANUP:
	PIN_FLIST_DESTROY_EX(&read_flds_oflistp, NULL);
	return;
}

/**************************************************
* Function: fm_mso_update_service()
* 
* 
***************************************************/
static void
fm_mso_update_service(
	pcm_context_t		*ctxp,
	char			*prog_name,
	poid_t			*acnt_pdp,
	pin_flist_t		*service_array,
	int32			conn_type,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*update_srvc_iflistp = NULL;
	pin_flist_t		*srvc_catv_info = NULL;
	pin_flist_t		*update_srvc_array = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*update_srvc_catv_info = NULL;
	pin_flist_t		*personal_bit_info = NULL;
	pin_flist_t		*update_srvc_oflistp = NULL;
	pin_flist_t		*tmp_flist = NULL;
	pin_flist_t		*preactivated_srvc_info = NULL;

	poid_t			*service_pdp = NULL;

	int64			db = -1;
	int32			activate_customer_failure = 1;

	int			elem_id = 0;
	pin_cookie_t		cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_service", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service input flist", service_array);

	db = PIN_POID_GET_DB(acnt_pdp);

	update_srvc_iflistp = PIN_FLIST_CREATE(ebufp);

	srvc_catv_info = PIN_FLIST_SUBSTR_GET(service_array, MSO_FLD_CATV_INFO, 0, ebufp );
	PIN_FLIST_FLD_SET(srvc_catv_info, PIN_FLD_CONN_TYPE, &conn_type, ebufp);

	preactivated_srvc_info = PIN_FLIST_SUBSTR_GET(service_array, PIN_FLD_PROV_DATA, 1, ebufp );
	if (preactivated_srvc_info)
	{
		PIN_FLIST_FLD_COPY(preactivated_srvc_info, MSO_FLD_CAS_SUBSCRIBER_ID, srvc_catv_info, MSO_FLD_CAS_SUBSCRIBER_ID, ebufp);
	}
	service_pdp = PIN_FLIST_FLD_GET(service_array, PIN_FLD_SERVICE_OBJ, 0, ebufp);

	PIN_FLIST_FLD_SET(update_srvc_iflistp, PIN_FLD_POID, acnt_pdp, ebufp );
	PIN_FLIST_FLD_SET(update_srvc_iflistp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp );
	if (!prog_name)
	{
		PIN_FLIST_FLD_SET(update_srvc_iflistp, PIN_FLD_PROGRAM_NAME, "oap|csradmin", ebufp );
	}
	update_srvc_array = PIN_FLIST_ELEM_ADD(update_srvc_iflistp, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_SET(update_srvc_array, PIN_FLD_POID, service_pdp, ebufp );
	PIN_FLIST_FLD_COPY(srvc_catv_info, MSO_FLD_AGREEMENT_NO, update_srvc_array, PIN_FLD_LOGIN, ebufp );
	PIN_FLIST_FLD_COPY(srvc_catv_info, MSO_FLD_AGREEMENT_NO, service_array, PIN_FLD_LOGIN, ebufp );

	inherited_info = PIN_FLIST_SUBSTR_ADD(update_srvc_array, PIN_FLD_INHERITED_INFO, ebufp );
	update_srvc_catv_info = PIN_FLIST_SUBSTR_ADD(inherited_info, MSO_FLD_CATV_INFO, ebufp );
  
	PIN_FLIST_CONCAT(update_srvc_catv_info, srvc_catv_info, ebufp);
 
	while((personal_bit_info = PIN_FLIST_ELEM_GET_NEXT(service_array, MSO_FLD_PERSONAL_BIT_INFO,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		tmp_flist = PIN_FLIST_ELEM_ADD(inherited_info, MSO_FLD_PERSONAL_BIT_INFO, elem_id, ebufp );
		PIN_FLIST_CONCAT(tmp_flist, personal_bit_info, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_service input flist", update_srvc_iflistp);

	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, update_srvc_iflistp, &update_srvc_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&update_srvc_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Updating Service", ebufp);
		PIN_ERRBUF_RESET(ebufp);

		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51075", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Updating Service", ebufp);
		goto CLEANUP;
	}

	CLEANUP:
	//PIN_FLIST_DESTROY_EX(&update_srvc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&update_srvc_oflistp, NULL);
	return;
}

/**************************************************
* Function: fm_mso_update_purchased_prod()
* 
* 
***************************************************/
static void
fm_mso_update_purchased_prod(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
    time_t          purchase_end_t,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*deals_array = NULL;
	pin_flist_t		*deal_info = NULL;
	pin_flist_t		*products_array = NULL;
	pin_flist_t		*update_inflistp = NULL;
	pin_flist_t		*update_outflistp = NULL;
	
	poid_t			*plan_pdp = NULL;
	poid_t			*purchased_prod_pdp = NULL;

	int32			activate_customer_failure = 1;
	int32			pp_status_flags = 8;

	int			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	int			elem_id_1;
	pin_cookie_t		cookie_1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_purchased_prod", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_purchased_prod input flist", in_flistp);	

	while((deals_array = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_DEALS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		plan_pdp = PIN_FLIST_FLD_GET(deals_array, PIN_FLD_PLAN_OBJ, 0, ebufp);
		deal_info = PIN_FLIST_SUBSTR_GET(deals_array, PIN_FLD_DEAL_INFO, 0, ebufp);
		
		elem_id_1 = 0 ;
		cookie_1 = NULL;
		while((products_array = PIN_FLIST_ELEM_GET_NEXT(deal_info, PIN_FLD_PRODUCTS,
		&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			purchased_prod_pdp = PIN_FLIST_FLD_GET(products_array, PIN_FLD_OFFERING_OBJ, 0, ebufp);
			
			update_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(update_inflistp, PIN_FLD_POID, purchased_prod_pdp, ebufp );
			PIN_FLIST_FLD_SET(update_inflistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp );
			PIN_FLIST_FLD_SET(update_inflistp, PIN_FLD_STATUS_FLAGS, &pp_status_flags, ebufp );
            if (purchase_end_t > 0)
            {
                PIN_FLIST_FLD_SET(update_inflistp, PIN_FLD_CYCLE_END_T, &purchase_end_t, ebufp);
                PIN_FLIST_FLD_SET(update_inflistp, PIN_FLD_USAGE_END_T, &purchase_end_t, ebufp);
                PIN_FLIST_FLD_SET(update_inflistp, PIN_FLD_PURCHASE_END_T, &purchase_end_t, ebufp);
            }
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_purchased_prod WRITE_FLDS iflist", update_inflistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_inflistp, &update_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&update_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in adding plan poid in purchased product", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51076", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in adding plan poid in purchased product", ebufp);
				goto CLEANUP;
			}
			PIN_FLIST_DESTROY_EX(&update_outflistp, NULL);
		}
	}
	
	CLEANUP:
	return;
}

/**************************************************
* Function: fm_mso_get_service_info()
* 
* 
***************************************************/

static int32
fm_mso_get_service_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**service_array,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*services_inp = NULL;

	int64			db = -1;
	int32			srch_flag = 256;
	int32			count_catv = 0;
	int32			service_exist = 0;
	int			service_status_active = PIN_STATUS_ACTIVE;
	int			service_status_inactive = PIN_STATUS_INACTIVE;
	int			elem_id = 0;
	pin_cookie_t		cookie = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srvc_catv_pdp = NULL;
	
	//Pawan:10-11-2014 Updated below search template
	//char			*template = "select X from /service where F1.id = V1 and ( F2 = V2 or F3 = V3 ) ";
	char			*template = "select X from /service where F1.id = V1 and ( F2 = V2 or F3 = V3 ) and F4.type like V4 ";
	char			*service_type = NULL; 
	char			*order_service_type = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_service_info", ebufp);
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info acnt_pdp ", acnt_pdp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	services_inp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	order_service_type = (char*)PIN_POID_GET_TYPE((poid_t*)PIN_FLIST_FLD_GET(services_inp, PIN_FLD_SERVICE_OBJ, 0, ebufp));

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
	//Pawan:10-11-2014 - Added Argument 4 to avoid broadband service in search result
	srvc_catv_pdp = PIN_POID_CREATE(db, "/service/catv", -1, ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, srvc_catv_pdp, ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info SEARCH input flist", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, service_array, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info SEARCH output flist", *service_array);
	
	if(*service_array != NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info SEARCH output flist", *service_array);
		
		services_inp = PIN_FLIST_ELEM_GET(*service_array, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
		
		if(services_inp != NULL)
		{			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_info SEARCH output flist", services_inp);

			service_exist = PIN_FLIST_ELEM_COUNT(*service_array, PIN_FLD_RESULTS, ebufp);
			if ( service_exist == 0)
			{
				return 0;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "0001");
				while((result_flist = PIN_FLIST_ELEM_GET_NEXT(*service_array, PIN_FLD_RESULTS,
					&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "0001");
					service_type =  (char *)PIN_POID_GET_TYPE( (poid_t *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 1, ebufp ));
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, service_type);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, SRVC_CATV);
					if ( (strcmp(service_type, SRVC_CATV)) == 0 )
					{
					 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "0001");
					 count_catv++;
					}
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "0001");
				}
			}
		}
	}

	CLEANUP:
	return count_catv;
}

/**************************************************
* Function: fm_mso_get_preactivated_svc_info()
* 
* 
***************************************************/

static void
fm_mso_get_preactivated_svc_info(
	pcm_context_t		*ctxp,
	poid_t			*stb_pdp,
	pin_flist_t		**preactivated_svc_info,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;

	int64			db = -1;
	int32			srch_flag = 512;
	int32			count_catv = 0;

	poid_t			*srch_pdp = NULL;

	char			*template = "select X from /mso_catv_preactivated_svc where F1.id = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_preactivated_svc_info", ebufp);
		return;
	}

	db = PIN_POID_GET_DB(stb_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_STB_OBJ, stb_pdp, ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_BOUQUET_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CAS_SUBSCRIBER_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CA_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_NETWORK_NODE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_REGION_KEY, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_VC_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_STB_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DEVICE_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_preactivated_svc_info SEARCH input flist", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_preactivated_svc_info SEARCH output flist", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
	*preactivated_svc_info = PIN_FLIST_COPY(result_flist, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

/**************************************************
* Function: fm_mso_update_preactivated_svc()
* 
* 
***************************************************/
static void
fm_mso_update_preactivated_svc(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*update_inflistp = NULL;
	pin_flist_t		*update_outflistp = NULL;
	
	poid_t			*plan_pdp = NULL;
	poid_t			*purchased_prod_pdp = NULL;

	int32			assigned = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_preactivated_svc", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_preactivated_svc input flist", in_flistp);	

	update_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, update_inflistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(update_inflistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp );
	PIN_FLIST_FLD_SET(update_inflistp, PIN_FLD_STATUS, &assigned, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_preactivated_svc WRITE_FLDS iflist", update_inflistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_inflistp, &update_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&update_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS: fm_mso_update_preactivated_svc()", ebufp);
		PIN_ERRBUF_RESET(ebufp);

		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &assigned, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51082", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_WRITE_FLDS: fm_mso_update_preactivated_svc()", ebufp);
		goto CLEANUP;
	}
	PIN_FLIST_DESTROY_EX(&update_outflistp, NULL);
	
	CLEANUP:
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


/**************************************************
* Function: fm_mso_validate_acnt_corp_child()
* 
* 
***************************************************/
static void
fm_mso_validate_acnt_corp_child(
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

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_acnt_corp_child", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_acnt_corp_child input flist", in_flistp);

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


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_acnt_corp_child search input", srch_inflist);
	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_inflist, &srch_outflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_inflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: fm_mso_validate_acnt_corp_child()", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, corp_child_acnt, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51102", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: fm_mso_validate_acnt_corp_child()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_acnt_corp_child search output", srch_outflist);

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
		}

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
		else if (acnt_status && (*acnt_status != PIN_STATUS_ACTIVE) )
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
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp", *ret_flistp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_outflist, NULL);
	return;
}

/**************************************************
* Function: fm_mso_populate_billinfo_and_balinfo_corp()
* 
* 
***************************************************/
static void
fm_mso_populate_billinfo_and_balinfo_corp(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	char			*order_service_type,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*billinfo = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*bal_grp_out_flist = NULL;  
	pin_flist_t		*group_info = NULL;
	pin_flist_t		*parent_billinfo_details = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*services_array = NULL;
	pin_flist_t		*billinfo_out_flist = NULL;

	poid_t			*corp_parent_acnt = NULL;
	poid_t			*corp_child_acnt = NULL;
	poid_t			*corp_parent_billinfo_pdp = NULL;
	poid_t			*bal_grp_obj = NULL;
	poid_t			*billinfo_pdp = NULL;
	poid_t			*bal_info = NULL;

	int			elem_id = 0;

	char			billinfo_id[20];
	char			msg[100];
	char			*parent_billinfo_id = NULL;
	pin_cookie_t		cookie = NULL;

	int64			db = -1;
	int32			conn_type = CATV_MAIN;
	int32			service_exist = 0;
	int32			flg_order_additional_catv=0;
	int32			flg_order_catv_main_bb_exist=0;
	int32			flg_order_bb_catv_exist=0;
	int32			actg_type = PIN_ACTG_TYPE_BALANCE_FORWARD;
	//int32			pay_type = PIN_PAY_TYPE_SUBORD;
	int32			*pay_type = NULL;
	int32			activate_customer_failure = 1;
	int32			billinfo_type = 0;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_populate_billinfo_and_balinfo_corp", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_billinfo_and_balinfo_corp input flist", i_flistp);

	corp_child_acnt = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	group_info = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 0, ebufp);
	corp_parent_acnt = PIN_FLIST_FLD_GET(group_info, PIN_FLD_PARENT, 0, ebufp);

	if ( strcmp(order_service_type, "/service/catv") ==0)
	{
		billinfo_type = CATV;
		strcpy(billinfo_id, "CATV");
	}
	if ( strcmp(order_service_type, "/service/telco/broadband") ==0)
	{
		billinfo_type = BB;
		strcpy(billinfo_id, "BB");
	}

	//External function fm_mso_get_all_billinfo()
	fm_mso_get_all_billinfo(ctxp, corp_parent_acnt, billinfo_type, &parent_billinfo_details, ebufp); 

	result_flist = PIN_FLIST_ELEM_GET(parent_billinfo_details, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (result_flist)
	{
		parent_billinfo_id = (char *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_BILLINFO_ID, 0, ebufp );
		corp_parent_billinfo_pdp = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp);
	}
	else
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, corp_child_acnt, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51065", ebufp);
		strcpy(msg, "Parent not allowed to activate child for '" );
		strcat(msg,billinfo_id);
		strcat(msg,"' service. Proceed after modifying parent");
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,msg, ebufp);
		PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;
	}

 	db = PIN_POID_GET_DB(corp_child_acnt);

	fm_mso_read_bal_grp(ctxp, corp_child_acnt, NULL, &bal_grp_out_flist, ebufp);
	bal_grp_obj  = PIN_POID_COPY( PIN_FLIST_FLD_GET(bal_grp_out_flist, PIN_FLD_POID, 0, ebufp ), ebufp);
	billinfo_pdp = PIN_POID_COPY( PIN_FLIST_FLD_GET(bal_grp_out_flist, PIN_FLD_BILLINFO_OBJ, 0, ebufp ), ebufp);
	PIN_FLIST_DESTROY_EX(&bal_grp_out_flist, NULL);

	billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp);
	pay_type = PIN_FLIST_FLD_GET(billinfo, PIN_FLD_PAY_TYPE, 1, ebufp );
	if(pay_type && *pay_type == PIN_PAY_TYPE_SUBORD )  //PIN_PAY_TYPE_SUBORD = 10007
	{
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PARENT_BILLINFO_OBJ, corp_parent_billinfo_pdp, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_AR_BILLINFO_OBJ, corp_parent_billinfo_pdp, ebufp);		
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAYINFO_OBJ, corp_parent_acnt, ebufp);
		PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, pay_type, ebufp);

		PIN_FLIST_FLD_DROP(billinfo, PIN_FLD_ACTG_FUTURE_DOM,  ebufp);
		PIN_FLIST_FLD_DROP(billinfo, PIN_FLD_BILL_WHEN,  ebufp);

        /****** Why to drop need to find out
		payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp );
		if (payinfo)
		{
			PIN_FLIST_ELEM_DROP(i_flistp, PIN_FLD_PAYINFO, 0, ebufp);
		}*/
	}
	else
	{
		fm_mso_read_billinfo(ctxp, billinfo_pdp, &billinfo_out_flist, ebufp );
		PIN_FLIST_FLD_COPY(billinfo_out_flist, PIN_FLD_PAY_TYPE, billinfo, PIN_FLD_PAY_TYPE, ebufp );
		payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp );
		if (payinfo)
		{
			PIN_FLIST_FLD_COPY(billinfo_out_flist, PIN_FLD_PAYINFO_OBJ, payinfo, PIN_FLD_POID, ebufp );
		}
		PIN_FLIST_DESTROY_EX(&billinfo_out_flist, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "billinfo flist", billinfo);

	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_POID, billinfo_pdp, ebufp);
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_BILLINFO_ID, billinfo_id, ebufp);
	/*PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PARENT_BILLINFO_OBJ, corp_parent_billinfo_pdp, ebufp);
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_AR_BILLINFO_OBJ, corp_parent_billinfo_pdp, ebufp);
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAYINFO_OBJ, corp_parent_acnt, ebufp);
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_PAY_TYPE, &pay_type, ebufp);*/
	PIN_FLIST_FLD_SET(billinfo, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);

	services_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	PIN_FLIST_ELEM_SET(services_array, NULL,PIN_FLD_BAL_INFO, 0, ebufp );

	bal_info = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BAL_INFO, 0,0, ebufp);
	PIN_FLIST_FLD_SET(bal_info, PIN_FLD_POID,bal_grp_obj, ebufp);
	//PIN_FLIST_FLD_PUT(bal_info, PIN_FLD_POID, PIN_POID_CREATE(db, "/balance_group", -1, ebufp), ebufp);
	//PIN_FLIST_FLD_SET(bal_info, PIN_FLD_NAME, "Service Balance Group", ebufp);
	PIN_FLIST_ELEM_SET(bal_info, NULL, PIN_FLD_BILLINFO, 0, ebufp);


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating billinfo_and_balinfo", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, corp_child_acnt, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51066", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in updating billinfo_and_balinfo", ebufp);
		goto CLEANUP;
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&bal_grp_out_flist, NULL);
	return;
}

/**************************************************
* Function: fm_mso_populate_bouquet_id()
* 
* 
***************************************************/
void
fm_mso_populate_bouquet_id(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			mso_device_type,
	int32			city_only,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_inflist = NULL;
	pin_flist_t		*srch_outflist = NULL;
	pin_flist_t		*srch_outflist_1 = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*catv_info = NULL;
	pin_flist_t		*code_info = NULL;

	pin_cookie_t		cookie = NULL;

	poid_t			*acnt_pdp = NULL;
	poid_t			*lco_pdp = NULL;

	int64			db = -1;
	int32			srch_flags = 256;
	int32			activate_customer_failure = 1;
	int32			search_by_city = 0;
	int32			carrier_id = 0;
	int32			elem_id = 0;
	char			*template_1 = "select X from /mso_cfg_bouquet_lco_map where F1 = V1 ";
	char			*template_2 = "select X from /mso_cfg_bouquet_city_map  where F1 = V1 and F2 = V2 ";
	char			*city = NULL;
	char			*area = NULL;
	char			*bouquet_id = NULL;
	char			*area_codep = NULL;
	char			*state_city_area_code = NULL;
	char			*network_nodep = NULL;
	char			*tmp_network_nodep = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_populate_bouquet_id", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_bouquet_id input flist", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if (!acnt_pdp)
	{
		acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	}
	lco_pdp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LCO_OBJ, 1, ebufp);

	network_nodep = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_NETWORK_NODE, 1, ebufp);

	db = PIN_POID_GET_DB(acnt_pdp);

	srch_inflist =  PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_inflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_FLAGS, &srch_flags, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_RESULTS, 0, ebufp);
	if (!city_only && lco_pdp)
	{
 		//arg_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 1, ebufp);
		//PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_DEVICE_TYPE, &mso_device_type, ebufp);

		/**********************************************************************
		 * Search for MSO_FLD_BOUQUET_ID based on area_code
		**********************************************************************/
		PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_TEMPLATE, template_1, ebufp);
	
		arg_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, lco_pdp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_bouquet_id search by area input", srch_inflist);
	
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_inflist, &srch_outflist, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_bouquet_id search by area output", srch_outflist);

		result_flist = PIN_FLIST_ELEM_GET(srch_outflist, PIN_FLD_RESULTS, 0, 1, ebufp);

		if (!result_flist)
		{
			search_by_city = 1;
		}

		area_codep = PIN_FLIST_FLD_TAKE(result_flist, PIN_FLD_SERVICE_AREA_CODE, 1, ebufp);
		while(result_flist && (arg_flist = PIN_FLIST_ELEM_GET_NEXT(result_flist, PIN_FLD_SERVICE_CODES,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			tmp_network_nodep = PIN_FLIST_FLD_GET(arg_flist, MSO_FLD_NETWORK_NODE, 0, ebufp);
			if (strcmp(network_nodep, tmp_network_nodep) == 0)
			{
				if (mso_device_type == MSO_DEVICE_TYPE_HD )
				{
					bouquet_id = PIN_FLIST_FLD_GET(arg_flist, MSO_FLD_BOUQUET_ID_HD, 0, ebufp);
				}
				else
				{
					bouquet_id = PIN_FLIST_FLD_GET(arg_flist, MSO_FLD_BOUQUET_ID, 0, ebufp);
				}
				*ret_flistp = PIN_FLIST_COPY(arg_flist, ebufp);;
		
				PIN_FLIST_FLD_COPY(arg_flist, MSO_FLD_POPULATION_ID, i_flistp, MSO_FLD_POPULATION_ID, ebufp);
				break;
			}
		}
        
        PIN_FLIST_FLD_PUT(*ret_flistp, PIN_FLD_SERVICE_AREA_CODE, area_codep, ebufp);
		if (bouquet_id == NULL)
		{
			search_by_city = 1;
		}

		PIN_FLIST_FLD_DROP(srch_inflist, PIN_FLD_TEMPLATE, ebufp);
		PIN_FLIST_ELEM_DROP(srch_inflist, PIN_FLD_ARGS, 1, ebufp);
	}	
	else
	{
		search_by_city = 1;
	}

	if (search_by_city || city_only)
	{
		state_city_area_code = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AREA, 1, ebufp);
	
		if (network_nodep && strstr(network_nodep, "JVM"))
		{
			carrier_id = 1;
		}
		else if (network_nodep && strcmp(network_nodep, "JVM1") == 0)
		{
			carrier_id = 2;
		}
        if (network_nodep && strstr(network_nodep, "NAGRA1"))
        {
            carrier_id = 1;
        }
        if (network_nodep && strstr(network_nodep, "NAGRA"))
        {
            carrier_id = 2;
        }

		fm_mso_get_state_city_area_code(ctxp, state_city_area_code, NULL, &code_info, ebufp );
		if (code_info && (PIN_FLIST_FLD_GET(code_info, PIN_FLD_ERROR_CODE, 1, ebufp)))
		{
			*ret_flistp = PIN_FLIST_COPY(code_info, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			goto CLEANUP;
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state_city_area_code", code_info);
		area = (char*)PIN_FLIST_FLD_GET(code_info, MSO_FLD_AREA_CODE, 1, ebufp);  //State code
		city = (char*)PIN_FLIST_FLD_GET(code_info, MSO_FLD_CITY_CODE, 1, ebufp);

		/**********************************************************************
		 * Search for MSO_FLD_BOUQUET_ID based on city_code
		**********************************************************************/
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Bouquet ID not configured for this area...going to search bouquet id for city");

		PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_TEMPLATE, template_2, ebufp );
		arg_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_CITY_CODE, city, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CARRIER_ID, &carrier_id, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_bouquet_id search by city input", srch_inflist);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_inflist, &srch_outflist_1, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_inflist, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_bouquet_id search by city output", srch_outflist_1);

		result_flist = PIN_FLIST_ELEM_GET(srch_outflist_1, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (!result_flist )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Bouquet ID not configured for this city ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Bouquet ID not configured for this city", ebufp);
			goto CLEANUP;
		}
	
		PIN_FLIST_FLD_COPY(result_flist, MSO_FLD_POPULATION_ID, i_flistp, MSO_FLD_POPULATION_ID, ebufp);
	}

	services  = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
    if (services)
    {
        catv_info = PIN_FLIST_SUBSTR_GET(services, MSO_FLD_CATV_INFO, 0, ebufp);

		if (mso_device_type == MSO_DEVICE_TYPE_HD )
		{
			if (search_by_city)
			{
				PIN_FLIST_FLD_COPY(result_flist, MSO_FLD_BOUQUET_ID_HD, catv_info, MSO_FLD_BOUQUET_ID, ebufp);
			}	
			else
			{
				PIN_FLIST_FLD_SET(catv_info, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
			}
		}
		else
		{
			if (search_by_city)
			{
				PIN_FLIST_FLD_COPY(result_flist, MSO_FLD_BOUQUET_ID, catv_info, MSO_FLD_BOUQUET_ID, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(catv_info, MSO_FLD_BOUQUET_ID, bouquet_id, ebufp);
			}
		}
	}
	else if (city_only)
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_VALIDITY_IN_DAYS, *ret_flistp, PIN_FLD_VALIDITY_IN_DAYS, ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_VALID_DOM, *ret_flistp, PIN_FLD_VALID_DOM, ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_TIMEZONE, *ret_flistp, PIN_FLD_TIMEZONE, ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_SPEED, *ret_flistp, PIN_FLD_SPEED, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_populate_bouquet_id return flist", *ret_flistp);
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_outflist, NULL);
	PIN_FLIST_DESTROY_EX(&srch_outflist_1, NULL);
	PIN_FLIST_DESTROY_EX(&code_info, NULL); 
	return;
}


/**************************************************
* Function: fm_mso_validate_plan_purchase_rule()
*     1.  At any point of time  a service can not have more 
*         than one basic plans in active/suspended state. 
* 
***************************************************/
static void
fm_mso_validate_plan_purchase_rule(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*service_details,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*deal_flistp = NULL;
	pin_flist_t		*dealinfo_flistp = NULL;
	pin_flist_t		*srch_inflist = NULL;
	pin_flist_t		*acnt_pdp = NULL;
	pin_flist_t		*srch_outflist = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
//	pin_flist_t		*usage_map = NULL;

 	poid_t			*deal_pdp = NULL;
	poid_t			*product_pdp = NULL;

	pin_decimal_t		*priority_onethirty = pbo_decimal_from_str("130.0",ebufp);
	pin_decimal_t		*priority_hundred = pbo_decimal_from_str("100.0",ebufp);
	pin_decimal_t		*priority = NULL;

	char			*template = "select X from /product 1,/deal 2 where 2.F1 = V1 and 2.F2 = 1.F3 ";
	char			*area_code = NULL;
	char			*state_code = NULL;
//	char			*evt_type

	int64			db =1; 

	int32			elem_id =0;
	int32			activate_customer_failure = 1;
 	int32			baic_plan_found=0;
 	int32			srch_flags = 256;
	int32			flag_hdw_plan_found = 0;
//	int32			fdp_basic = 0;

	pin_cookie_t		cookie = NULL;
	double			priority_double = 0.0;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_plan_purchase_rule", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_plan_purchase_rule input flist", i_flistp);
	//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_plan_purchase_rule service flist", service_details);
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_ACCOUNT_OBJ,0,ebufp);
	services_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY,1, ebufp);
	area_code = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AREA, 1, ebufp);
	if (area_code)
	{
		state_code =  strtok(area_code,"|");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "state_code");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, state_code);
	}

	while((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(services_flistp, PIN_FLD_DEALS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_plan_purchase_rule plan_flistp", deal_flistp);
		dealinfo_flistp = PIN_FLIST_SUBSTR_GET(deal_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);
		deal_pdp = PIN_FLIST_FLD_GET(dealinfo_flistp,PIN_FLD_POID,0,ebufp);
		
		srch_inflist =  PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_inflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
		PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_TEMPLATE, template, ebufp);		
		
		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, deal_pdp, ebufp);
		
		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 2, ebufp);
		result_flist = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
		
		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);

		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, NULL, ebufp)
		
//		usage_map = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_USAGE_MAP, 0, ebufp); 
//		PIN_FLIST_FLD_SET(usage_map, PIN_FLD_EVENT_TYPE, NULL, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_plan_purchase_rule product search input", srch_inflist);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_inflist, &srch_outflist, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_inflist, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_plan_purchase_rule product search output", srch_outflist);
		result_flist = PIN_FLIST_ELEM_GET(srch_outflist,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
		if (!result_flist )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan Not configured correctly", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plan Not configured correctly", ebufp);
			goto CLEANUP;
		}
		else
		{
			if(PIN_FLIST_ELEM_COUNT(srch_outflist,PIN_FLD_RESULTS,ebufp) >1)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "More than one product: Hardware Plan");
			}
			else
			{	/****************************************************
				*Validation for more than one base plan
				****************************************************/
				priority = PIN_FLIST_FLD_GET(result_flist,PIN_FLD_PRIORITY,0,ebufp);
				priority_double = pbo_decimal_to_double(priority, ebufp);

/*				usage_map =  PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_USAGE_MAP, 0, 1,ebufp);
				if (usage_map)
				{
					evt_type = PIN_FLIST_FLD_GET(usage_map,PIN_FLD_EVENT_TYPE,1,ebufp);
				}

				if (evt_type && (strstr(evt_type, "mso_sb_pkg_fdp") || strstr(evt_type, "mso_sb_alc_fdp") )
				{
					fdp_basic = 1;
				}
*/
//				if (!fdp_basic)
//				{
					if(pbo_decimal_compare(priority,priority_hundred,ebufp)>=0 &&
						pbo_decimal_compare(priority,priority_onethirty,ebufp)<=0)
					{
						if(baic_plan_found == 1)
						{
							PIN_ERRBUF_RESET(ebufp);
							*ret_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
							PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
							PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
							PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "More than one basic package offer not allowed", ebufp);
							goto CLEANUP;
						}
						else
						{
							baic_plan_found = 1;
						}
					}
//				}
				/****************************************************
				*Validation for ET plan for non-delhi customers
				****************************************************/
                                // Changes to impliment Hardware plan not mandatory rule
                                flag_hdw_plan_found = 1;

                                // Changes to impliment Hardware plan not mandatory rule
				if ( priority_double >= HDW_PROD_PRI_START &&
				     priority_double <= HDW_PROD_PRI_END
				   )
				{
					flag_hdw_plan_found = 1;
				}
			}
		}
		
	}
	/*if ((state_code && strcmp(state_code, "DL")!=0)  &&    //state is not DELHI
	     flag_hdw_plan_found ==0
	   )
	{
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Hardware plan is mandatory for non-Delhi customers", ebufp);
		goto CLEANUP;	
	}*/

	if ( baic_plan_found == 0)
	{
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51108", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Base plan is mandatory for activation", ebufp);
		goto CLEANUP;
	}

/*	PIN_ERRBUF_RESET(ebufp);
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Bouquet ID not configured for this city or area", ebufp); 
	goto CLEANUP;*/

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_outflist, NULL); 
	return;
}

/**************************************************
* Function: fm_mso_validate_additional_service_rule()
*     1.  Addl connection should not purchase a base pack
*         of higher priority than the main connection 
*         than one basic plans in active/suspended state. 
*     2. The same above rule to be incorporated in change plan API
* 
***************************************************/
static void
fm_mso_validate_additional_service_rule(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*service_details,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
	{

	pin_flist_t		*ser_in_flistp = NULL;
	pin_flist_t		*ser_out_flistp = NULL;
	pin_flist_t		*catv_info_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*pp_in_flistp = NULL;
	pin_flist_t		*pp_out_flistp = NULL;
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*deal_flistp = NULL;
	pin_flist_t		*dealinfo_flistp = NULL;
	pin_flist_t		*srch_outflist = NULL;
	pin_flist_t		*result_flist = NULL;
	poid_t			*deal_pdp = NULL;
	poid_t			*product_pdp = NULL;
	pin_flist_t		*srch_inflist = NULL;
	int32			service_count=0;
	int32			service_found=0;
	int32			elem_id =0;
	int32			srch_flags = 256;
	int32			prod_active = 1;
	int32			activate_customer_failure = 1;
	int32			con_type = 0;
	int32			primary_service = 0;
	int64			db =1;
	char			*template = "select X from /product 1,/deal 2 where 2.F1 = V1 and 2.F2 = 1.F3 ";
	char			*template_ser = "select X from /service where F1 = V1 ";
	char			*template_pur_prod = "select X from /product 1, /purchased_product 2 where 2.F1 = V1 and 2.F2 = V2 and 2.F3 = 1.F4 and 1.F5 >= V5 and 1.F6 <= V6 ";
	pin_cookie_t		cookie = NULL;
	poid_t			*serv_pdp = NULL;
	poid_t			*pri_serv_pdp = NULL;
	poid_t			*acnt_pdp = NULL;
	pin_decimal_t		*priority_onethirty = pbo_decimal_from_str("130.0",ebufp);
	pin_decimal_t		*priority_hundred = pbo_decimal_from_str("100.0",ebufp);
	pin_decimal_t		*basic_plan_priority = NULL;
	pin_decimal_t		*priority = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_additional_service_rule", ebufp);
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_additional_service_rule input flist", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_additional_service_rule service flist", service_details);
	
	service_count = PIN_FLIST_ELEM_COUNT(service_details,PIN_FLD_RESULTS,ebufp);
	if(service_count == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No existing services. Additional service check not required. Returning");
		return;
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "More than 1 service. Find out the primary service");
		while((result_flist = PIN_FLIST_ELEM_GET_NEXT(service_details, PIN_FLD_RESULTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			if(!service_found)
			{
				ser_in_flistp =  PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(ser_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(ser_in_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
				PIN_FLIST_FLD_SET(ser_in_flistp, PIN_FLD_TEMPLATE, template_ser, ebufp);		
				tmp_flistp = PIN_FLIST_ELEM_ADD(ser_in_flistp, PIN_FLD_ARGS, 1, ebufp);
				PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, tmp_flistp, PIN_FLD_POID, ebufp );
				tmp_flistp = PIN_FLIST_ELEM_ADD(ser_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search input flist", ser_in_flistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, ser_in_flistp, &ser_out_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&ser_in_flistp, NULL);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search output flist", ser_out_flistp);
				result_flist = PIN_FLIST_ELEM_GET(ser_out_flistp, PIN_FLD_RESULTS, 0,1, ebufp);
				catv_info_flistp = PIN_FLIST_SUBSTR_GET(result_flist, MSO_FLD_CATV_INFO, 0, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CATV Info flist", catv_info_flistp);
				if(catv_info_flistp)
				{
					con_type = *((int32 *)PIN_FLIST_FLD_GET(catv_info_flistp,PIN_FLD_CONN_TYPE,0,ebufp));
					if(con_type == primary_service)
					{
						service_found=1;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Primary Service found.");
						pri_serv_pdp = PIN_FLIST_FLD_TAKE(result_flist,PIN_FLD_POID,0,ebufp);
					}
				}
			}
		}
	}
	pp_in_flistp =  PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(pp_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(pp_in_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(pp_in_flistp, PIN_FLD_TEMPLATE, template_pur_prod, ebufp);		
	tmp_flistp = PIN_FLIST_ELEM_ADD(pp_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_SERVICE_OBJ, pri_serv_pdp, ebufp );
	tmp_flistp = PIN_FLIST_ELEM_ADD(pp_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &prod_active, ebufp );
	tmp_flistp = PIN_FLIST_ELEM_ADD(pp_in_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp );
	tmp_flistp = PIN_FLIST_ELEM_ADD(pp_in_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp );
	tmp_flistp = PIN_FLIST_ELEM_ADD(pp_in_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRIORITY, priority_hundred, ebufp );
	tmp_flistp = PIN_FLIST_ELEM_ADD(pp_in_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRIORITY, priority_onethirty, ebufp );
	tmp_flistp = PIN_FLIST_ELEM_ADD(pp_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp );
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRIORITY, NULL, ebufp )
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased product search input flist", pp_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, pp_in_flistp, &pp_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&pp_in_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased product search output flist", pp_out_flistp);
	
	
	if(PIN_FLIST_ELEM_COUNT(pp_out_flistp,PIN_FLD_RESULTS,ebufp) >1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "More than one product with priority between 100 and 130. Taking 1st element.");
	}
	result_flist = PIN_FLIST_ELEM_GET(pp_out_flistp, PIN_FLD_RESULTS, 0,1, ebufp);
	if (!result_flist)
	{
		goto CLEANUP;
	}
	basic_plan_priority = PIN_FLIST_FLD_GET(result_flist,PIN_FLD_PRIORITY,0,ebufp);
	
	elem_id=0;
	cookie=NULL;
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_ACCOUNT_OBJ,0,ebufp);
	services_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY,1, ebufp);
	while((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(services_flistp, PIN_FLD_DEALS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_additional_service_rule plan_flistp", deal_flistp);
		dealinfo_flistp = PIN_FLIST_SUBSTR_GET(deal_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);
		deal_pdp = PIN_FLIST_FLD_GET(dealinfo_flistp,PIN_FLD_POID,0,ebufp);
		srch_inflist =  PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_inflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
		PIN_FLIST_FLD_SET(srch_inflist, PIN_FLD_TEMPLATE, template, ebufp);		
		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, deal_pdp, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 2, ebufp);
		result_flist = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_inflist, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_additional_service_rule product search input", srch_inflist);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_inflist, &srch_outflist, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_inflist, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_additional_service_rule product search output", srch_outflist);
		result_flist = PIN_FLIST_ELEM_GET(srch_outflist,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
		if (!result_flist )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan Not configured correctly", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plan Not configured correctly", ebufp);
			goto CLEANUP;
		}
		else
		{
			if(PIN_FLIST_ELEM_COUNT(srch_outflist,PIN_FLD_RESULTS,ebufp) >1)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "More than one product: Hardware Plan");
			}
			else
			{
				priority = PIN_FLIST_FLD_GET(result_flist,PIN_FLD_PRIORITY,0,ebufp);
				if(pbo_decimal_compare(priority,priority_hundred,ebufp)>=0 &&
					pbo_decimal_compare(priority,priority_onethirty,ebufp)<=0)
				{
					 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Basic plan. Compare with primary service.");
					if(pbo_decimal_compare(priority,basic_plan_priority,ebufp) > 0)
					{
						PIN_ERRBUF_RESET(ebufp);
						*ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Secondary plan priority must be less than basic plan.", ebufp);
						goto CLEANUP;	
					}
				}
			}
		}
	}
		
	PIN_ERRBUF_RESET(ebufp);
/*	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Bouquet ID not configured for this city or area", ebufp); */
	goto CLEANUP;

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_outflist, NULL); 
		PIN_FLIST_DESTROY_EX(&pp_out_flistp, NULL); 
		PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
	return;
}


/**************************************************
* Function: fm_mso_validate_same_family_products()
*     1.  Addl connection should not purchase a base pack
*         of higher priority than the main connection 
*         than one basic plans in active/suspended state. 
*     2. The same above rule to be incorporated in change plan API
* 
***************************************************/
static void
fm_mso_validate_same_family_products(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*services = NULL;
	pin_flist_t		*read_flds_iflist = NULL;
	pin_flist_t		*read_flds_oflist = NULL;
	pin_flist_t		*deal_info = NULL;
	pin_flist_t		*deals = NULL;
	pin_flist_t		*products = NULL;

	poid_t			*prod_pdp = NULL;
	
	int32			elem_id =0;
	int32			elem_id_1 =0;
	int32			elem_id_2 =0;
	int32			activate_customer_failure = 1;
	int32			brk_flag =0;
	int64			db =1;
	int			i=0;
	int			j;
	int			k;
	char			*prov_tag = NULL;
	char			*vp;
	char			prov_tag_list[MAX_PLAN_SELECT_LIMIT][100];
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie_1 = NULL;
	pin_cookie_t		cookie_2 = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_same_family_products", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_same_family_products", i_flistp);

	while((services = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_SERVICES,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_same_family_products:services", services);
		elem_id_1=0;
		cookie_1=NULL;
		while((deals = PIN_FLIST_ELEM_GET_NEXT(services, PIN_FLD_DEALS,
		&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_same_family_products:deals", deals);
			deal_info = PIN_FLIST_SUBSTR_GET(deals, PIN_FLD_DEAL_INFO, 0, ebufp);

			elem_id_2=0;
			cookie_2=NULL;
			while((products = PIN_FLIST_ELEM_GET_NEXT(deal_info, PIN_FLD_PRODUCTS,
			&elem_id_2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_same_family_products:products", products);
				prod_pdp = PIN_FLIST_FLD_GET(products, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
				
				read_flds_iflist =  PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_POID, prod_pdp, ebufp);
				PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_flds_iflist:", read_flds_iflist);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflist, &read_flds_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&read_flds_iflist, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, prod_pdp, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
					goto CLEANUP;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_flds_oflist:", read_flds_oflist);
				vp = PIN_FLIST_FLD_GET(read_flds_oflist, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
				if (vp && strcmp (vp,"")!=0)
				{
					memset(prov_tag_list[i],'\0',sizeof(prov_tag_list[i]));
					strcpy( prov_tag_list[i], vp);
					i++;
				}
				PIN_FLIST_DESTROY_EX(&read_flds_oflist, NULL);
			}
		}
	}

	for (j=0; j<i ; j++)
	{
		for (k=0; k<i ; k++ )
		{
		   if (j!=k)
		   {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "prov_tag_list[j]");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, prov_tag_list[j]);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "prov_tag_list[k]");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, prov_tag_list[k]);
			if (strcmp(prov_tag_list[j], prov_tag_list[k]) ==0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matched");
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Only one plan from same family is allowed", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, prod_pdp, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Only one plan from same family is allowed", ebufp);
				brk_flag=1;
				break;
			}
		   }
		}
		if (brk_flag==1)
		   break;
	}


	CLEANUP:
	return;
}

/**************************************************
* Function: fm_mso_validate_addl_plan_purchase()
*     1. Parent or non child should not be able to  purchase ADDITIONAL PACKAGE 
*     2. The same above rule to be incorporated in change plan API
* 
***************************************************/
static void
fm_mso_validate_addl_plan_purchase(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			srvc_catv_count,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*plan_list_code = NULL;
	pin_flist_t		*plan = NULL;

	poid_t			*prod_pdp = NULL;

	char			*plan_code = NULL;
	
	int32			elem_id =0;
	int32			activate_customer_failure = 1;
	int64			db =1;
	pin_cookie_t		cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_addl_plan_purchase", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_addl_plan_purchase", i_flistp);

	if (srvc_catv_count == 0 )  //0: main connection
	{
		plan_list_code = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PLAN_LIST_CODE, 0, ebufp);
		while((plan = PIN_FLIST_ELEM_GET_NEXT(plan_list_code, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_addl_plan_purchase:plan", plan);
			plan_code = PIN_FLIST_FLD_GET(plan, PIN_FLD_CODE, 0, ebufp); 
			if (strstr(plan_code, "ADDITIONAL"))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matched");
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Only Additional connection is allowed to purchase Additional Plans", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Only Additional connection is allowed to purchase Additional Plans", ebufp);
			}
		}
	}
	CLEANUP:
	//PIN_FLIST_SUBSTR_DROP(i_flistp, PIN_FLD_PLAN_LIST_CODE, ebufp);
	return;
}

/**************************************************
* Function: fm_mso_validate_south_plans()
*     1. South  packages ( Regional Kannada, Regional Tamil etc) 
         should not go with starter package 
*     2. "SUN SOUTH ALL" plan can not be purchased along with 
*         Regional packages (REGIONAL MALAYALAM, REGIONAL TAMIL, 
*         REGIONAL KANNADA, REGIONAL TELUGU)
*     3. Maximum 2 Regional packs can be purchased, while trying
*        to purchase the third one, promted to purchase "SUN SOUTH ALL"
* 
***************************************************/
static void
fm_mso_validate_south_plans(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*services = NULL;
	pin_flist_t		*read_flds_iflist = NULL;
	pin_flist_t		*read_flds_oflist = NULL;
	pin_flist_t		*deal_info = NULL;
	pin_flist_t		*deals = NULL;
	pin_flist_t		*products = NULL;

	poid_t			*prod_pdp = NULL;
	
	int32			elem_id =0;
	int32			elem_id_1 =0;
	int32			elem_id_2 =0;
	int32			activate_customer_failure = 1;
	int32			flg_purchased_7_days_demo_pkg = 0;
	int32			flg_south_pack = 0;
	int32			flg_starter_pack = 0;
	int32			flg_starter_pack_110 = 0;
	int32			flg_south_all = 0;
	int32			south_pack_count = 0;
	int64			db =1;
	int			i=0;
	int			j;
	int			k;
	char			*prov_tag = NULL;
	char			*vp;
	char			*plan_name = NULL;
	char			*product_name = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie_1 = NULL;
	pin_cookie_t		cookie_2 = NULL;

	pin_decimal_t		*priority = NULL;
	double			priority_double = 0.0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_south_plans", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_south_plans", i_flistp);

	while((services = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_SERVICES,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_south_plans:services", services);
		elem_id_1=0;
		cookie_1=NULL;
		while((deals = PIN_FLIST_ELEM_GET_NEXT(services, PIN_FLD_DEALS,
		&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_south_plans:deals", deals);
			plan_name = PIN_FLIST_FLD_GET(deals, PIN_FLD_NAME, 0, ebufp);
			if (plan_name && strstr(plan_name, "SUN SOUTH ALL") )
			{
				flg_south_all = 1;
			}

			deal_info = PIN_FLIST_SUBSTR_GET(deals, PIN_FLD_DEAL_INFO, 0, ebufp);

			elem_id_2=0;
			cookie_2=NULL;
			while((products = PIN_FLIST_ELEM_GET_NEXT(deal_info, PIN_FLD_PRODUCTS,
			&elem_id_2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_south_plans:products", products);
				prod_pdp = PIN_FLIST_FLD_GET(products, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
				
				read_flds_iflist =  PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_POID, prod_pdp, ebufp);
				PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
				PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_PRIORITY, NULL, ebufp);
				PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_NAME, NULL, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_flds_iflist:", read_flds_iflist);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflist, &read_flds_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&read_flds_iflist, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
					goto CLEANUP;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_flds_oflist:", read_flds_oflist);
				/*****************************************************************************
				* Validation for "7 DAYS DEMO PACKAGE" purchase allowed only once
				* Start
				*****************************************************************************
				product_name = (char*)PIN_FLIST_FLD_GET(read_flds_oflist, PIN_FLD_NAME, 1, ebufp);
				if (product_name && (strcmp(product_name, "HD STARTER DEMO 7 DAYS PACKAGE")==0))
				{
					if (flg_purchased_7_days_demo_pkg)
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Package 'HD STARTER DEMO 7 DAYS PACKAGE' can be purchased once in a life time", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						*ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Package 'HD STARTER DEMO 7 DAYS PACKAGE' can be purchased once in a life time", ebufp);
						goto CLEANUP;	
					}
					flg_purchased_7_days_demo_pkg = 1;
				}
				*****************************************************************************
				* Validation for "7 DAYS DEMO PACKAGE" purchase allowed only once
				* End
				*****************************************************************************/				
				prov_tag = (char*)PIN_FLIST_FLD_GET(read_flds_oflist, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
				if (prov_tag && (
				    (strcmp(prov_tag, "ADDON_REG_KAN")==0 && (!strstr(plan_name, "SUN SOUTH ALL"))) ||
				    (strcmp(prov_tag, "ADDON_REG_MAL")==0 && (!strstr(plan_name, "SUN SOUTH ALL"))) ||
				    (strcmp(prov_tag, "ADDON_REG_TAM")==0 && (!strstr(plan_name, "SUN SOUTH ALL"))) ||
				    (strcmp(prov_tag, "ADDON_REG_TEL")==0 && (!strstr(plan_name, "SUN SOUTH ALL"))) )
				    )
				{
					flg_south_pack = 1;
					south_pack_count++;
				}
				priority = PIN_FLIST_FLD_GET(read_flds_oflist, PIN_FLD_PRIORITY, 0, ebufp);
				priority_double = pbo_decimal_to_double(priority, ebufp);
				if (priority_double == 100 )
				{
					flg_starter_pack = 1;
				}
				// Sachid: adding another flag for starter plans
				else if (priority_double == 110 )
				{
					flg_starter_pack_110 = 1;
				}
				PIN_FLIST_DESTROY_EX(&read_flds_oflist, NULL);
			}
		}
	}
	/* Sachid:
	 * priority_double == 100 : Error if flg_south_pack || flg_south_all
	 * priority_double == 110 : Error if (flg_south_pack & south_pack_count>1) || flg_south_all
         */
	if ((flg_south_pack && flg_starter_pack) ||
	    (flg_south_all && flg_starter_pack) )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "South pack not allowed for BST ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "South pack not allowed for BST ", ebufp);
		goto CLEANUP;
	}
	if (((south_pack_count>6) && flg_starter_pack_110) ||
	    (flg_south_all && flg_starter_pack_110) )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Only 6 South packs are allowed for Starter Pack", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Only 6 South packs are allowed for Starter Pack", ebufp);
		goto CLEANUP;
	}
	if (flg_south_all && flg_south_pack )
	{
		//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "All regional plans to be cancelled before purchasing 'SUN SOUTH ALL'", ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Regional plans can not be purchased alongwith 'SUN SOUTH ALL'", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Regional plans can not be purchased alongwith 'SUN SOUTH ALL'", ebufp);
		goto CLEANUP;
	}
	if (south_pack_count > 6)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Go for South combo plan instead of more than 6 regional plans", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Go for South combo plan instead of more than 6 regional plans", ebufp);
		goto CLEANUP;
	}
	CLEANUP:
	return;
}



/**************************************************
* Function: fm_mso_modify_billing_dates()
*     1.  Addl connection should not purchase a base pack
*         of higher priority than the main connection 
*         than one basic plans in active/suspended state. 
*     2. The same above rule to be incorporated in change plan API
* 
***************************************************/
static void
fm_mso_modify_billing_dates(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*read_flds_iflist = NULL;
	pin_flist_t		*read_flds_oflist = NULL;
	pin_flist_t		*update_billinfo_oflist = NULL;
	pin_flist_t		*update_billinfo_iflist = NULL;
	pin_flist_t		*billinfo = NULL;
	pin_flist_t		*billinfo_arr = NULL;
	pin_flist_t		*set_billinfo_iflist = NULL;
	pin_flist_t		*set_billinfo_oflist = NULL;
	pin_flist_t		*bal_info = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*group_info = NULL;

	poid_t			*billinfo_obj = NULL;
	poid_t			*acnt_pdp = NULL;
	
	int32			activate_customer_failure = 1;
	int32			acnt_type_is_prepaid = 0;
	int32			actgcycles_left = 1;
	int32			bill_when = 1;
	int32			acnt_is_corp_child = 0;
	int32			pay_type = 0;
	int32			flg_call_set_billinfo =1;


	time_t			*next_bill_t = NULL;
	time_t			*future_bill_t = NULL;

	void			*vp = NULL;

	struct tm		*timeeff;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_modify_billing_dates", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_billing_dates", i_flistp);

	group_info = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 1, ebufp);
	if (group_info)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_is_corp_child");
		acnt_is_corp_child = 1;

	}

	payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
	if (payinfo)
	{
		inherited_info = PIN_FLIST_SUBSTR_GET(payinfo, PIN_FLD_INHERITED_INFO, 1, ebufp);
		if (inherited_info)
		{
			inv_info = PIN_FLIST_ELEM_GET(inherited_info, PIN_FLD_INV_INFO, 0, 1, ebufp);
			if (inv_info)
			{
				vp = PIN_FLIST_FLD_GET(inv_info, PIN_FLD_INDICATOR, 1, ebufp);
				if (vp && *(int32*)vp == 1 )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_type_is_prepaid");
					acnt_type_is_prepaid=1;
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_type_is_postpaid");
				}
			}
		}
	}

	if (!acnt_is_corp_child && !acnt_type_is_prepaid )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_type_is_postpaid so skipping billing dates modification...");
		return;
	}


	billinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 0, 1, ebufp) ;
	bal_info = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BAL_INFO, 0, 1, ebufp) ;

	pay_type = *(int32*)PIN_FLIST_FLD_GET(billinfo, PIN_FLD_PAY_TYPE, 0, ebufp);


	if (billinfo)
	{
		// Comment out Rama if (acnt_is_corp_child && pay_type == PIN_PAY_TYPE_SUBORD )
		if (acnt_is_corp_child)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_is_corp_child and charges paid by parent");
			billinfo_obj = PIN_FLIST_FLD_GET(billinfo, PIN_FLD_POID, 0 , ebufp);
			acnt_pdp = PIN_FLIST_FLD_GET(group_info, PIN_FLD_PARENT, 0 , ebufp);
			flg_call_set_billinfo = 0;
		}
		else
		{
			billinfo_obj = PIN_FLIST_FLD_GET(billinfo, PIN_FLD_POID, 0 , ebufp);
			acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0 , ebufp);
		}

		vp = PIN_FLIST_FLD_GET(billinfo, PIN_FLD_BILL_WHEN, 1 , ebufp);
		if (vp)
		{
			bill_when = *(int32*)vp;
		}

		if (bal_info)
		{
			PIN_FLIST_ELEM_DROP(bal_info, PIN_FLD_BILLINFO, 0, ebufp);
			PIN_FLIST_FLD_SET(bal_info, PIN_FLD_BILLINFO_OBJ, billinfo_obj, ebufp );
		}
		
		if (flg_call_set_billinfo)
		{
			//Call PCM_OP_CUST_SET_BILLINFO start 
			//For non-paying child do not call PCM_OP_CUST_SET_BILLINFO by BILL_WHEN, instead used the BILL_WHEN of parent
			set_billinfo_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(set_billinfo_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
			PIN_FLIST_FLD_SET(set_billinfo_iflist, PIN_FLD_PROGRAM_NAME, "advance_charging", ebufp);
			
			billinfo_arr = PIN_FLIST_ELEM_ADD(set_billinfo_iflist, PIN_FLD_BILLINFO, 0, ebufp);
			PIN_FLIST_FLD_SET(billinfo_arr, PIN_FLD_POID, billinfo_obj, ebufp);
			PIN_FLIST_FLD_SET(billinfo_arr, PIN_FLD_BILL_WHEN, &bill_when, ebufp);
			PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_PAY_TYPE, billinfo_arr, PIN_FLD_PAY_TYPE, ebufp);
			PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_BILLINFO_ID, billinfo_arr, PIN_FLD_BILLINFO_ID, ebufp);
			PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_PAYINFO_OBJ, billinfo_arr, PIN_FLD_PAYINFO_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_ACTG_TYPE, billinfo_arr, PIN_FLD_ACTG_TYPE, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "set_billinfo_iflist:", set_billinfo_iflist);
			PCM_OP(ctxp, PCM_OP_CUST_SET_BILLINFO, 0, set_billinfo_iflist, &set_billinfo_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&set_billinfo_iflist, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling set_billinfo", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, billinfo_obj, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling set_billinfo: ()", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "set_billinfo_oflist:", set_billinfo_oflist);
			PIN_FLIST_DESTROY_EX(&set_billinfo_oflist, NULL);
			//Call PCM_OP_CUST_SET_BILLINFO end
		}
		//Call PCM_OP_READ_FLDS billinfo start 
		read_flds_iflist =  PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_POID, billinfo_obj, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_ACTG_LAST_T,   NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_LAST_BILL_T,   NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_ACTG_NEXT_T,   NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_NEXT_BILL_T,   NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_ACTG_FUTURE_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_FUTURE_BILL_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_BILL_WHEN,     NULL, ebufp);


		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_flds_iflist:", read_flds_iflist);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflist, &read_flds_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&read_flds_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, billinfo_obj, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_flds_oflist:", read_flds_oflist);
		
		//for non-paying child : update the billinfo of the account 
		billinfo_obj = PIN_FLIST_FLD_GET(billinfo, PIN_FLD_POID, 0 , ebufp);
		update_billinfo_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(update_billinfo_iflist, PIN_FLD_POID, billinfo_obj, ebufp);
		PIN_FLIST_FLD_SET(update_billinfo_iflist, PIN_FLD_BILL_ACTGCYCLES_LEFT, &actgcycles_left, ebufp);

		if (acnt_is_corp_child && pay_type == PIN_PAY_TYPE_SUBORD )
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "billinfo:", billinfo);
			PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_PAY_TYPE,            update_billinfo_iflist, PIN_FLD_PAY_TYPE, ebufp );
			PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_PARENT_BILLINFO_OBJ, update_billinfo_iflist, PIN_FLD_PARENT_BILLINFO_OBJ, ebufp );
			PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_AR_BILLINFO_OBJ,     update_billinfo_iflist, PIN_FLD_AR_BILLINFO_OBJ, ebufp );
			PIN_FLIST_FLD_COPY(billinfo, PIN_FLD_PAYINFO_OBJ,         update_billinfo_iflist, PIN_FLD_PAYINFO_OBJ, ebufp );
			PIN_FLIST_FLD_COPY(read_flds_oflist, PIN_FLD_BILL_WHEN,   update_billinfo_iflist, PIN_FLD_BILL_WHEN, ebufp );
			
			PIN_FLIST_FLD_COPY(read_flds_oflist, PIN_FLD_NEXT_BILL_T, update_billinfo_iflist, PIN_FLD_NEXT_BILL_T, ebufp );
			PIN_FLIST_FLD_COPY(read_flds_oflist, PIN_FLD_FUTURE_BILL_T, update_billinfo_iflist, PIN_FLD_FUTURE_BILL_T, ebufp );
			
			next_bill_t   = PIN_FLIST_FLD_GET(update_billinfo_iflist, PIN_FLD_NEXT_BILL_T, 0, ebufp);
			future_bill_t = PIN_FLIST_FLD_GET(update_billinfo_iflist, PIN_FLD_FUTURE_BILL_T, 0, ebufp);
		}
		else
		{

			PIN_FLIST_FLD_COPY(read_flds_oflist, PIN_FLD_ACTG_NEXT_T, update_billinfo_iflist, PIN_FLD_NEXT_BILL_T, ebufp );
			PIN_FLIST_FLD_COPY(read_flds_oflist, PIN_FLD_NEXT_BILL_T, update_billinfo_iflist, PIN_FLD_FUTURE_BILL_T, ebufp );

			//Get and modify  PIN_FLD_NEXT_BILL_T & PIN_FLD_FUTURE_BILL_T to move by 1 month
			next_bill_t   = PIN_FLIST_FLD_GET(update_billinfo_iflist, PIN_FLD_NEXT_BILL_T, 0, ebufp);
			future_bill_t = PIN_FLIST_FLD_GET(update_billinfo_iflist, PIN_FLD_FUTURE_BILL_T, 0, ebufp);
			timeeff = localtime(future_bill_t);
			timeeff->tm_mon = timeeff->tm_mon + 1;
			*future_bill_t = mktime (timeeff); 
		}
 		 
		 //Call PCM_OP_WRITE_FLDS for update billinfo start 
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_billinfo_iflist:", update_billinfo_iflist);
		PIN_FLIST_FLD_SET(update_billinfo_iflist, PIN_FLD_FUTURE_BILL_T,future_bill_t, ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_billinfo_iflist:", update_billinfo_iflist);
		
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_billinfo_iflist, &update_billinfo_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&update_billinfo_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS: ()", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, billinfo_obj, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_WRITE_FLDS: ()", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update_billinfo_oflist:", update_billinfo_oflist);
		//Call PCM_OP_WRITE_FLDS for update billinfo end

	}
	//Call PCM_OP_READ_FLDS billinfo end

	PIN_FLIST_ELEM_DROP(i_flistp, PIN_FLD_BILLINFO, 0, ebufp) ;
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&read_flds_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&update_billinfo_oflist, NULL);
	return;
}

/**************************************************
* Function: fm_mso_validate_7_days_demo_plan()
*     1. If the validation to be applied at account Level 
* 
***************************************************/
static void
fm_mso_validate_7_days_demo_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*srch_iflist = NULL;
	pin_flist_t		*srch_oflist = NULL;
	pin_flist_t		*args_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*return_flist = NULL;
	poid_t			*account_obj = NULL;
	
	int32			srch_flags = 512;
	int32			failure = 1;
	int32			status = -1;

	int64			db = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_7_days_demo_plan", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_7_days_demo_plan:", i_flistp);

	account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);

	db = PIN_POID_GET_DB(account_obj);
	srch_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
	//PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /product 1, /purchased_product 2 where  2.F1 = V1 and 1.F2 = 2.F3 and 1.F4 = V4 ", ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /product 1, /purchased_product 2 where  2.F1 = V1 and 1.F2 = 2.F3 ", ebufp);
	
	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp );

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp );

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp );
	
	//args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 4, ebufp);
	//PIN_FLIST_FLD_SET(args_flist, PIN_FLD_NAME, "HD STARTER DEMO 7 DAYS PACKAGE", ebufp );

	result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_iflist:", srch_iflist);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_oflist:", srch_oflist);

      /*if(PIN_FLIST_ELEM_COUNT(srch_oflist,PIN_FLD_RESULTS,ebufp) >1)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Package 'HD STARTER DEMO 7 DAYS PACKAGE' can be purchased once in a life time", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp,   PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Package 'HD STARTER DEMO 7 DAYS PACKAGE' can be purchased once in a life time", ebufp);
	}
	*/
	status = validate_demo_packs(ctxp, srch_oflist, &return_flist, ebufp);
	if(status ==1)
	{
		*ret_flistp = return_flist;
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
	return;
}

/*********************************************************
* Function: fm_mso_set_assoc_bus_profile()
* Description: This function changes the associated 
*                                             business profile of billinfo. The new profile
*                                             is evaluated as per rules in iscript.
**********************************************************/
static void fm_mso_set_assoc_bus_profile
(             pcm_context_t                 *ctxp,
                poid_t                                   *billinfo_pdp,
                pin_flist_t                           **ret_flistpp,
                pin_errbuf_t                      *ebufp)
{
                pin_flist_t                           *srch_in_flistp = NULL;
                pin_flist_t                           *srch_out_flistp = NULL;
                pin_flist_t                           *arg_flist = NULL;
                pin_flist_t                           *result_flist = NULL;
                poid_t                                   *srch_pdp = NULL;
                poid_t                                   *bus_prof_pdp = NULL;
                
                int64                                      db = -1;
                int32                                      srch_flag = 256;
                int32                                      one = 1;
                char                                       *template = "select X from /config/business_profile where F1.type like V1 ";

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
                PIN_POID_DESTROY(bus_prof_pdp, NULL);
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
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                return;
}

/* This function loops through the plans being purchased
and sets the discount values , if the configurations is available */

static void
fm_mso_activation_disc(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t	*disc_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*deal_info = NULL;
	pin_flist_t	*deals_flistp = NULL;
	pin_flist_t	*service_flistp = NULL;
	pin_flist_t	*catv_flistp = NULL;
	pin_flist_t	*discount_flistp = NULL;
	pin_flist_t	*product_flistp = NULL;
	poid_t		*acct_pdp = NULL;	
	poid_t		*plan_obj = NULL;
	int32		rec_id1 =0;
	int32		rec_id2 = 0;
	pin_cookie_t	deal_cookie = NULL;
	pin_cookie_t	prod_cookie = NULL;
	char            *das_type = NULL;
	int32		pp_type = 2;
	int32		ret_status =0;
	int32		failure = 1;
	int32		discount_flag = -1;
	pin_decimal_t 	*disc_amt = NULL;
	pin_decimal_t	*zero_disc  = NULL;

	zero_disc = pbo_decimal_from_str("0.0", ebufp );

	PIN_ERR_LOG_MSG(3, "Entering activation discount block");
	PIN_ERR_LOG_FLIST(3, "Activation discount block inflistp", in_flistp);
	
	disc_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CITY, disc_flistp, PIN_FLD_CITY, ebufp);
	service_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_SERVICES, 0, 0, ebufp);
	catv_flistp = PIN_FLIST_SUBSTR_GET(service_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(catv_flistp, PIN_FLD_CONN_TYPE, disc_flistp, PIN_FLD_CONN_TYPE, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        //Get DAS type 
	fm_mso_get_das_type(ctxp, acct_pdp, &das_type, ebufp);	
        if (das_type)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                          "DAS TYPE FOUND... ");
                PIN_FLIST_FLD_SET(disc_flistp,
                          MSO_FLD_DAS_TYPE, das_type, ebufp);
       	}
	//Get account pp_type
	pp_type = fm_mso_get_cust_type(ctxp, acct_pdp, ebufp);
        if (pp_type != 2)
       	{ 
        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                          "Valid PP Type Found... ");
                PIN_FLIST_FLD_SET(disc_flistp,
                           MSO_FLD_PP_TYPE, &pp_type, ebufp);
        }
	while((deals_flistp = PIN_FLIST_ELEM_GET_NEXT(service_flistp, PIN_FLD_DEALS, &rec_id1, 1, &deal_cookie, ebufp)) != (pin_flist_t *)NULL)	
	{
		PIN_ERR_LOG_FLIST(3, "Deals flist", deals_flistp);
		plan_obj = PIN_FLIST_FLD_GET(deals_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_POID, plan_obj, ebufp);
		PIN_FLIST_FLD_SET(disc_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_ERR_LOG_FLIST(3, "fm_mso_activation_discount disc_flistp", disc_flistp);
		discount_flag = -1;
		fm_mso_get_plan_level_discount(ctxp, disc_flistp, &discount_flistp, ebufp);
		PIN_ERR_LOG_FLIST(3, "fm_mso_get_plan_level_discount return flist", discount_flistp);
                if (discount_flistp)
                {	
			PIN_ERR_LOG_MSG(3, "Entering here 1");
			ret_status = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_STATUS, 1, ebufp);
                        if(ret_status != failure )
                        {
				PIN_ERR_LOG_MSG(3, "Entering here 2");
				discount_flag = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT_FLAGS, 1, ebufp);
                                if (discount_flag != -1)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Product Level Discount Config Found");
					disc_amt = PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 1, ebufp);
					deal_info = PIN_FLIST_SUBSTR_GET(deals_flistp, PIN_FLD_DEAL_INFO, 0, ebufp);
					rec_id2 =0;
					prod_cookie = NULL;
					while((product_flistp = PIN_FLIST_ELEM_GET_NEXT(deal_info, PIN_FLD_PRODUCTS,
		                               &rec_id2, 1, &prod_cookie, ebufp)) != (pin_flist_t *)NULL)
					{	
						PIN_ERR_LOG_FLIST(3, "Product flistp", product_flistp);
						//Setting percentage wise discont
					        if (discount_flag == 0)
                        	                {
							PIN_ERR_LOG_MSG(3, "Percentage based discount");
							PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_CYCLE_DISCOUNT, disc_amt, ebufp);
                                              		PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PURCHASE_DISCOUNT, disc_amt, ebufp);
                                	              	PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_USAGE_DISCOUNT, disc_amt, ebufp);
                           	                }
						//Setting flat discount
						else
						{	
							PIN_ERR_LOG_MSG(3, "Flat discount during activation");
							PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_CYCLE_DISC_AMT, disc_amt, ebufp);                                                      					PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PURCHASE_DISC_AMT, disc_amt, ebufp);
							PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_CYCLE_DISCOUNT, zero_disc, ebufp);
                                                        PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_PURCHASE_DISCOUNT, zero_disc, ebufp);
                                                        PIN_FLIST_FLD_SET(product_flistp, PIN_FLD_USAGE_DISCOUNT, zero_disc, ebufp);


						}
					}
				}
			}
		}

		PIN_FLIST_DESTROY_EX(&discount_flistp, NULL);
	}

	PIN_FLIST_DESTROY_EX(&discount_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&disc_flistp, NULL);
//	pbo_decimal_destroy(&disc_amt);
	return;
}	
