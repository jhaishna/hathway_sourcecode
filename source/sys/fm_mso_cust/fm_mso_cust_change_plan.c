/*******************************************************************
 * File:	fm_mso_cust_change_plan.c
 * Opcode:	MSO_OP_CUST_CHANGE_PLAN 
 * Owner:	Rohini Mogili
 * Created:	18-NOV-2013
 * Description: This opcode is for changing the plan 

 r << xx 1
 0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
 0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 44029 10
 0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 16
 0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
 0 PIN_FLD_PLAN_LISTS	ARRAY [0]
 1     PIN_FLD_FLAGS           INT [0] 0
 1     PIN_FLD_PLAN_OBJ    POID [0] 0.0.0.1 /plan 41089 8
 1     PIN_FLD_DEALS          ARRAY [0] allocated 5, used 5
 2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 42530 0
 2         PIN_FLD_PACKAGE_ID      INT [0] 22
 1     PIN_FLD_DEALS          ARRAY [1] allocated 5, used 5
 2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 43554 0
 2         PIN_FLD_PACKAGE_ID      INT [0] 23
 0 PIN_FLD_PLAN_LISTS	ARRAY [1]
 1     PIN_FLD_FLAGS           INT [0] 1
 1     PIN_FLD_PLAN_OBJ    POID [0] 0.0.0.1 /plan 41089 8
 1     PIN_FLD_DEALS          ARRAY [0] allocated 5, used 5
 2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 42530 0
 1     PIN_FLD_DEALS          ARRAY [1] allocated 5, used 5
 2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 43554 0
 xx
 xop 11005 0 1

xop: opcode 11005, flags 0
# number of field entries allocated 20, used 4
0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
0 PIN_FLD_STATUS         ENUM [0] 0
0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 16
0 PIN_FLD_DESCR           STR [0] "ACCOUNT - Service change plan completed successfully"

 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_change_plan.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "pcm_ops.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "ops/price.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_lifecycle_id.h"
#include "pin_pymt.h"
#include "mso_utils.h"


#define READONLY 0
#define READWRITE 1
#define LOCK_OBJ 2

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1

#define NAMEINFO_BILLING 1
#define NAMEINFO_INSTALLATION 2

#define ACCOUNT_MODEL_UNKNOWN 0
#define ACCOUNT_MODEL_RES 1
#define ACCOUNT_MODEL_CORP 2
#define ACCOUNT_MODEL_MDU 3

#define CORPORATE_PARENT 1
#define CORPORATE_CHILD 2

#define FLAG_ADD_PLAN 1
#define FLAG_DEL_PLAN 0
#define	NCF_PKG_TYPE 3

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

extern int 
fm_mso_validate_csr_role(
		pcm_context_t	*ctxp,
		int64		db,
		poid_t		*user_id,
		pin_errbuf_t	*ebufp);

extern void
fm_mso_validation_rule_1(
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		int32			action,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

extern void
fm_mso_validation_rule_2(
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		int32			action,
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
fm_mso_get_account_info(
		pcm_context_t		*ctxp,
		poid_t			*acnt_pdp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

extern void
fm_mso_get_business_type_values(
		pcm_context_t		*ctxp,
		int32			business_type,
		int32			*account_type,
		int32			*account_model,
		int32			*subscriber_type,
		pin_errbuf_t		*ebufp);

extern int32 
fm_mso_get_parent_info(
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flist,
		poid_t			*acnt_pdp,
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
fm_mso_validate_pay_type_corp(
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		int32			opcode,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

extern void 
fm_mso_update_ncr_validty(
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**o_flistpp,
		pin_errbuf_t		*ebufp);

//extern void
//get_evt_lifecycle_template(
//	pcm_context_t			*ctxp,
//	int64				db,
//	int32				string_id, 
//	int32				string_version,
//	char				**lc_template, 
//	pin_errbuf_t			*ebufp);

extern 
void fm_mso_create_lifecycle_evt(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flistp,
		pin_flist_t		*out_flistp,
		int32			action,
		pin_errbuf_t		*ebufp);

extern void
fm_mso_set_fup_status(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flistp,
		poid_t			*subs_plan_pdp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

extern int
fm_mso_cust_release_service_ip(
		pcm_context_t		*ctxp,
		poid_t			*account_obj,
		poid_t			*service_obj,
		char			*prog_name,
		int			*count,
		char			**login,
		pin_errbuf_t		*ebufp);

/**************************************
 * External Functions end
 ***************************************/

/**************************************
 * Local Functions start
 ***************************************/
PIN_IMPORT int32
fm_mso_catv_conn_type(
		pcm_context_t           *ctxp,
		poid_t                  *srv_pdp,
		pin_errbuf_t            *ebufp);

EXPORT_OP int 
fm_mso_change_plan(
		pcm_context_t		*ctxp,
		int32                   flag,
		pin_flist_t		*in_flist,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

static int 
fm_mso_validate_base_change_rule(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flist,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

static void 
fm_mso_bb_change_plan(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flist,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);	

static void 
fm_mso_validate_bb_change_plan(
		pcm_context_t	*ctxp,
		pin_flist_t	*in_flist,
		pin_flist_t	**ret_flistp,
		pin_errbuf_t	*ebufp);

void
fm_mso_get_mso_purplan(
		pcm_context_t           *ctxp,
		poid_t			*acc_pdp,	
		poid_t                  *plan_obj,
		int			status,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);

extern int
fm_mso_update_mso_purplan_status(
		pcm_context_t           *ctxp,
		poid_t                  *mso_purplan_obj,
		int			status,
		pin_errbuf_t            *ebufp);

static void 
fm_mso_validate_bb_plan_custtype(
		pcm_context_t	*ctxp,
		pin_flist_t	*in_flist,
		pin_flist_t	**ret_flistp,
		int		*ether_prov_flag,
		int		*old_type,
		int		*new_type,
		pin_errbuf_t	*ebufp);

static void
fm_mso_cust_get_prod_details(
		pcm_context_t           *ctxp,
		poid_t                  *product_obj,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t            *ebufp);

void
fm_mso_bb_get_payinfo_inv(
		pcm_context_t		*ctxp,
		poid_t			*acnt_pdp,
		pin_flist_t		**ret_flistpp,
		pin_errbuf_t		*ebufp);

static int
fm_mso_update_bb_payinfo(
		pcm_context_t		*ctxp,
		int			pay_type,
		pin_flist_t		*in_flistp,
		pin_errbuf_t		*ebufp);

int
fm_mso_update_mso_purplan(
		pcm_context_t		*ctxp,
		poid_t			*mso_pp_obj,
		int			new_status,
		char 			*descr,
		pin_errbuf_t            *ebufp);

static int
fm_mso_update_service_bb_info(
		pcm_context_t		*ctxp,
		poid_t			*ser_obj,
		int			bill_when,
		int 			indicator,
		pin_errbuf_t            *ebufp);

int
fm_mso_update_ser_prov_status(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flistp,
		int 			prov_status,
		pin_errbuf_t            *ebufp);

static int
fm_mso_bb_get_change_type(
		pcm_context_t		*ctxp,
		pin_flist_t		*old_mso_pp_flistp,
		poid_t			*new_mso_pp_obj,
		pin_errbuf_t		*ebufp);

static void
fm_mso_cust_change_plan_lc_event(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flistp,
		pin_flist_t		*out_flistp,
		pin_errbuf_t		*ebufp);

void
fm_mso_add_prod_info(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flistp,
		pin_errbuf_t		*ebufp);

static	int
fm_mso_compare_ca_ids(
		pcm_context_t           *ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t            *ebufp);

static	void
fm_mso_get_ca_id_from_plan(
		pcm_context_t           *ctxp,
		pin_flist_t		*i_flistp,
		poid_t                  *mso_plan_obj,
		char                  	*mso_node,
		int32			chage_flag,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t            *ebufp);

static int
fm_mso_cust_change_static_ip(
		pcm_context_t		*ctxp,
		pin_flist_t		*old_pp_flistp,
		poid_t			*new_mso_obj,
		char			*cmts_id,
		char			*prog_name,
		pin_errbuf_t		*ebufp);

void
fm_cust_chng_plan_validate_hierarchy(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flistp,
		//	int			*in_indicatorp,
		pin_flist_t		**o_flistpp,
		pin_errbuf_t            *ebufp);

void
fm_mso_utils_filter_non_paying_children(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flistp,
		pin_flist_t             **o_flistpp,
		pin_errbuf_t            *ebufp);

void
fm_utils_fetch_group_details(
		pcm_context_t           *ctxp,
		poid_t                  *in_pdp,
		int                     *flags,
		pin_flist_t             **o_flistpp,
		pin_errbuf_t            *ebufp);

void
fm_utils_fetch_acct_svc_details(
		pcm_context_t           *ctxp,
		poid_t                  *in_pdp,
		pin_flist_t             **o_flistpp,
		pin_errbuf_t            *ebufp);

static int 
fm_mso_change_amc_plan(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flist,
		int32			old_plan_type,
		int32			new_plan_type,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp );

void
mso_get_additional_products(
		pcm_context_t           *ctxp,
		pin_flist_t             *i_flistp,
		pin_errbuf_t            *ebufp);

void
mso_cancel_static_ip_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *refund_calc_flistp,
        int32                   *indicator,
        int32                   plan_change_after_expiry_scn,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void
mso_get_tal_products(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_create_tal_change_lifecycle(
        pcm_context_t   *ctxp,
        pin_flist_t     *in_flist,
        int             *old_bill_when,
        pin_errbuf_t    *ebufp);

PIN_EXPORT void
fm_mso_validate_fdp_et_align(
		pcm_context_t           *ctxp,
		u_int			flags,
		pin_flist_t             *i_flistp,
		pin_flist_t		**ret_flistpp,
		pin_errbuf_t            *ebufp);

PIN_IMPORT int32
fm_mso_catv_pt_pkg_type(
		pcm_context_t           *ctxp,
		poid_t                  *prd_pdp,
		pin_errbuf_t            *ebufp);

PIN_IMPORT void
mso_bill_pol_catv_main(
        pcm_context_t           *ctxp,
        u_int                   flags,
        poid_t                  *binfo_pdp,
        poid_t                  *serv_pdp,
        time_t                  event_t,
        pin_opcode_t            op_num,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void
mso_bill_pol_trigger_et_event(
		pcm_context_t		*ctxp,
		u_int			flags,
		pin_flist_t		*mso_et_flistp,
		pin_flist_t		*et_in_flistp,
		pin_flist_t             **et_out_flistp,
		pin_errbuf_t		*ebufp);

void 
fm_mso_retrieve_et(
		pcm_context_t           *ctxp,
		poid_t                  *srvc_pdp,
		pin_flist_t             **r_flistpp,
		pin_errbuf_t            *ebufp);

void 
fm_mso_retrieve_billinfo(
		pcm_context_t           *ctxp,
		poid_t                  *acct_pdp,
		pin_flist_t             **r_flistpp,
		pin_errbuf_t            *ebufp);

extern int32
fm_mso_utils_discount_validation(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flistp,
		pin_errbuf_t            *ebufp);
extern void
fm_mso_purchase_bb_plans(
		cm_nap_connection_t     *connp,
		pin_flist_t             *mso_act_flistp,
		char                    *prov_tag,
		int                     flags,
		int                     grace_flag,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);

/* Function to expire account level offers in mso_cfg_account_offers*/
static void
fm_mso_cust_expire_offers(
		pcm_context_t           *ctxp,
		pin_flist_t             *i_flistp,
		pin_flist_t             **r_flistpp,
		pin_errbuf_t            *ebufp);


static void
fm_mso_get_active_purch_prod_excp_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *inp_flistp,
        poid_t                  *plan_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

/**************************************
 * Local Functions end
 ***************************************/

/*******************************************************************
 * MSO_OP_CUST_CHANGE_PLAN 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_change_plan(
		cm_nap_connection_t	*connp,
		int32			opcode,
		int32			flags,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t		*ebufp);

static void
fm_mso_cust_change_plan(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
		int32			flags,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t		*ebufp);

extern int
fm_quota_check(
		pcm_context_t                   *ctxp,
		char                            *plan_name,
		char                            *prov_tag,
		int                             *tech,
		int                             *plan_typ,
		pin_decimal_t                   **initial_amount,
		int                             *error_codep,
		pin_errbuf_t                    *ebufp);

void
fm_mso_cust_upgrade_plan_validity(
		pcm_context_t           *ctxp,
		pin_flist_t             *i_flistp,
		pin_flist_t             **r_flistpp,
		pin_errbuf_t            *ebufp);

extern void 
fm_mso_generate_credit_charge(
		pcm_context_t           *ctxp,
		pin_flist_t		*i_flistp,
		poid_t			*cancel_offer_obj,
		poid_t			*cancel_prod_obj,
		pin_flist_t             **r_flistpp,
		pin_errbuf_t            *ebufp);

static int
fm_mso_change_plan_val(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);


/*******************************************************************
 * MSO_OP_CUST_CHANGE_PLAN  
 *******************************************************************/
void 
op_mso_cust_change_plan(
		cm_nap_connection_t	*connp,
		int32			opcode,
		int32			flags,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	int			*status = NULL;
	int32			price_calc_flag = 0;
	int32			status_uncommitted =2;
	poid_t			*inp_pdp = NULL;
	char            *prog_name = NULL;
	char            *descr = NULL;
	void			*vp = NULL;
	int32			failure = 1;

	//data declared to use for calling price calculator opcode
	poid_t                  *poid_p = NULL;
	poid_t                  *plan_obj = NULL;
	poid_t                  *get_deal_obj = NULL;
	poid_t                  *srvc_pdp = NULL;
	pin_flist_t             *price_calc_iflistp = NULL;
	pin_flist_t             *data_array_flistp = NULL;
	pin_flist_t             *planlists_flistp = NULL;
	pin_flist_t             *pricecalc_res_flistp = NULL;
	pin_flist_t             *current_plan_flistp = NULL;
	pin_flist_t             *dealinfo_flistp = NULL;
	pin_flist_t             *deals_flistp = NULL;
	pin_flist_t             *new_plan_flistp = NULL;
	pin_flist_t             *ret_flistp = NULL;
	int32                   *action_flags = NULL;
	int			excl_flg = 0;
	pin_decimal_t           *total_due = NULL;
	int32                   *bill_when = NULL;
	int                     elem_id = 0;
	int                     elem_base = 0;
	pin_cookie_t            cookie_base = NULL;
	pin_cookie_t            cookie = NULL;
	int                     change_flag = 5;
	int                     *price_calc = NULL;
	pin_decimal_t           *zero = NULL;
	int			mode = 0;
	int                     bulk_change_plan_flg = 0;
	char			text_msg[255];
	char			bb_service_type[]="/service/telco/broadband";
	char			*service_typep = NULL; 

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_CHANGE_PLAN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_mso_cust_change_plan error",
				ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"op_mso_cust_change_plan input flist", i_flistp);

	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	/*local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	  if(PIN_ERRBUF_IS_ERR(ebufp))
	  {
	  PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_change_plan()", ebufp);
	  return;
	  }*/

	/* Fetch the value of CALC ONLY flag*/
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 1, ebufp );
	if(vp && *(int32 *)vp == 1) {
		price_calc_flag = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
	}
	/* Changes to call price calculator for validating upgrade */
	if ( PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BOOLEAN, 1, ebufp) != NULL )
	{
		mode = *(int *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BOOLEAN, 1, ebufp);
	}

	/***************** VERIMATRIX CHANGES - BEGIN ********************
	 * changed the position of fetching prog_name and descr
	 * so that descr is available when change plan is called by
	 * MSO_OP_CUST_CALC_PRICE
	 *****************************************************************/
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	sprintf(text_msg, "prog_name = %s", prog_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, text_msg);

	if (fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_CALC_PRICE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "Change plan called again by MSO_OP_CUST_CALC_PRICE");
		if (prog_name && strstr(prog_name,"BULK"))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Ancestor is MSO_OP_CUST_CALC_PRICE and program is BULK");
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DESCR, "Bulk_change_plan", ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_cust_change_plan input flist", i_flistp);
		}
	}		

	descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);	
	sprintf(text_msg, "descr = %s", descr);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, text_msg);

	poid_p = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if(poid_p)
	{
		service_typep = (char *) PIN_POID_GET_TYPE(poid_p);
	}

	/***************** VERIMATRIX CHANGES - END ********************
	 * changed the position of fetching prog_name and descr
	 * so that descr is available when change plan is called by
	 * MSO_OP_CUST_CALC_PRICE
	 *****************************************************************/

        if (descr && (strcmp(descr, "Bulk_change_plan") == 0))
        {
                bulk_change_plan_flg = 1;
        }

	if(!fm_utils_op_is_ancestor(connp->coip, MSO_OP_CUST_CALC_PRICE) && price_calc_flag == 0 && mode == 0 &&
			(service_typep && strncmp(service_typep, bb_service_type, strlen(bb_service_type)) == 0) && bulk_change_plan_flg == 0)
	{
		prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
		price_calc_iflistp = PIN_FLIST_CREATE(ebufp);
		poid_p = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

		PIN_FLIST_FLD_SET(price_calc_iflistp, PIN_FLD_POID,poid_p, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, price_calc_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_FLAGS, price_calc_iflistp, PIN_FLD_FLAGS, ebufp);

		data_array_flistp = PIN_FLIST_ELEM_ADD(price_calc_iflistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, data_array_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, data_array_flistp, PIN_FLD_USERID, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "data_array_flistp: 1", data_array_flistp);

		if (!(prog_name && strstr(prog_name,"pin_deferred_act")))
		{
			while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS, &elem_base, 1, &cookie_base, ebufp )) != NULL)
			{
				action_flags = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_FLAGS, 0, ebufp );
				if(*(int*)action_flags == 1)
				{
					new_plan_flistp = PIN_FLIST_ELEM_ADD(data_array_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
					plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
					PIN_FLIST_FLD_SET(new_plan_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
					PIN_FLIST_FLD_COPY(planlists_flistp, PIN_FLD_FLAGS, new_plan_flistp, PIN_FLD_FLAGS, ebufp);
				}
				else if(*(int*)action_flags == 0)
				{
					excl_flg = fm_mso_change_plan_val(ctxp, i_flistp, ebufp);
					if ( excl_flg == 1)
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Change Plan not Allowed for Current/Old plan",ebufp);
						PIN_ERRBUF_RESET(ebufp);
						*r_flistpp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, poid_p, ebufp );
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &failure, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Change Plan not Allowed for Given plan", ebufp);
						return;
					}
					plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
					current_plan_flistp = PIN_FLIST_ELEM_ADD(data_array_flistp, PIN_FLD_PLAN_LISTS, 1, ebufp);
					while ((deals_flistp = PIN_FLIST_ELEM_GET_NEXT(planlists_flistp, PIN_FLD_DEALS, &elem_id, 1, &cookie, ebufp)) != NULL)
					{
						dealinfo_flistp = PIN_FLIST_ELEM_ADD(current_plan_flistp, PIN_FLD_DEALS, 0, ebufp);
						PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_PACKAGE_ID, dealinfo_flistp, PIN_FLD_PACKAGE_ID, ebufp);
						get_deal_obj = PIN_FLIST_FLD_GET(deals_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
						PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_DEAL_OBJ, get_deal_obj, ebufp);
					}
					PIN_FLIST_FLD_SET(current_plan_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
					PIN_FLIST_FLD_COPY(planlists_flistp, PIN_FLD_FLAGS, current_plan_flistp, PIN_FLD_FLAGS, ebufp);
				}
			}

			PIN_ERRBUF_CLEAR(ebufp);
			zero = pbo_decimal_from_str("0", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, data_array_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_SET(data_array_flistp, PIN_FLD_POID, poid_p , ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BILL_WHEN,data_array_flistp,PIN_FLD_BILL_WHEN, ebufp );
			PIN_FLIST_FLD_SET(price_calc_iflistp, PIN_FLD_FLAGS,&change_flag, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_cust_calc_price input flist", price_calc_iflistp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "Going to Invoke Price Calculator");

			PCM_OP(ctxp, MSO_OP_CUST_CALC_PRICE , 0, price_calc_iflistp, &ret_flistp, ebufp);			

			/***************** VERIMATRIX CHANGES - BEGIN ********************
			 * added error check post opcode - MSO_OP_CUST_CALC_PRICE call
			 *****************************************************************/
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error returning from MSO_OP_CUST_CALC_PRICE", ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_CALC_PRICE input", price_calc_iflistp);
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "Done with Price Calculator");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_CALC_PRICE output", ret_flistp);
			}

			/***************** VERIMATRIX CHANGES - BEGIN ********************
			 * added error check post opcode - MSO_OP_CUST_CALC_PRICE call
			 *****************************************************************/	

			cookie_base = NULL;
			while ((pricecalc_res_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistp, PIN_FLD_PLAN, &elem_base, 1, &cookie_base, ebufp )) != NULL)
			{
				total_due = PIN_FLIST_FLD_GET(pricecalc_res_flistp, PIN_FLD_TOTAL_DUE, 0, ebufp );
			}

			if(!(pbo_decimal_compare(total_due,zero,ebufp)>=0))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Change Plan is not Allowed as the Total Due is less than 0",ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*r_flistpp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, poid_p, ebufp );
				PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &failure, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Change Plan is not Allowed as the Total Due is less than 0", ebufp);
				return;
			}

			pbo_decimal_destroy(&zero);
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_cust_calc_price output flist", ret_flistp);
			PIN_FLIST_DESTROY_EX(&price_calc_iflistp, NULL);
		}
	}


	if (descr && (strcmp(descr, "Bulk_change_plan") == 0))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'Bulk_change_plan' so transaction will not be handled at APIlevel");
	}
	else
	{
		//	if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
		if (!(prog_name && strstr(prog_name,"pin_deferred_act")))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "prog_name is not pin_deferred_act 1");

			inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
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
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'pin_deferred_act' so transaction will not be handled at API level");
		}
	}

	fm_mso_cust_change_plan(connp, ctxp, flags, i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if ( descr && (strcmp(descr, "Bulk_change_plan") == 0))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'Bulk_change_plan' so transaction will not be handled at APIlevel");
	}
	else if((prog_name && strstr(prog_name,"pin_deferred_act")))
	{
		if (PIN_ERRBUF_IS_ERR(ebufp) || (status && *status == 1))
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, PIN_FLD_STATUS, 0, 0);
			return;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "prog_name is not pin_deferred_act and descr is not Bulk_change_plan");
		if (PIN_ERRBUF_IS_ERR(ebufp) || (status && (*(int*)status == 1)) || price_calc_flag ==1) 
		{		
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_change_plan error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Status is fail or Price Calc flag is 1.");

			if(local == LOCAL_TRANS_OPEN_SUCCESS &&
					(!(prog_name && strstr(prog_name,"pin_deferred_act")))
			  )
			{
				fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT aborted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
			}
		}
		else
		{
			if(local == LOCAL_TRANS_OPEN_SUCCESS && (!(prog_name && strstr(prog_name,"pin_deferred_act"))))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "committing change plan");
				fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
			}		
		}
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "Done with change plan");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_change_plan output flist", *r_flistpp);

CLEANUP:
	if(ret_flistp != NULL)
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

	return;
}





/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_change_plan(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
		int32			flags,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*readplan_inflistp = NULL;
	pin_flist_t		*readplan_outflistp = NULL;
	pin_flist_t		*deals_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
    pin_flist_t     *et_rflistp = NULL;
	pin_flist_t		*pdt_flistp = NULL;

	pin_flist_t		*planlist_flistp = NULL;
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*lco_city_flist = NULL;

	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*user_id = NULL;
	poid_t			*service_obj = NULL;
	poid_t			*old_plan_pdp = NULL;
	poid_t                  *plan_pdp = NULL;
	poid_t			*prd_pdp = NULL;
    poid_t          *bi_pdp = NULL;

	//char			*account_no_str = NULL;
	//char			account_no_str[10];
    char            *et_zonep = NULL;
	char			*ser_type = NULL;
	char			bb_service_type[]="/service/telco/broadband";
	int32			change_plan_success = 0;
	int32			change_plan_failure = 1;
	int32			account_type      = -1;
	int32			account_model     = -1;
	int32			subscriber_type   = -1;
	int32			corporate_type    =0;
	int32			tmp_business_type = 0;
	int32			*act_flag = NULL;
	int32			is_base_pack = -1;
	int32                  	elem_base = 0;
	int32			company_type = -1;
	pin_cookie_t            cookie_base = NULL;

	pin_decimal_t		*ref_amt = NULL;

	int64			db = -1;
    time_t      actg_next_t = 0;
	int			csr_access = 0;
	int			acct_update = 0;
	int			ret_status = 0;
	int			arr_business_type[8];

	void			*vp = NULL;
	int			*status = NULL;
        poid_t                  *serv_obj = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan input flist", i_flistp);	

	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	if (routing_poid && routing_poid != NULL)
	{	
		db = PIN_POID_GET_DB(routing_poid);
		account_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		//account_no_str = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp); 
	}
	else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51422", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	if (!account_obj)
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51423", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}

        serv_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
        if (!serv_obj)
        {
                ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51424", ebufp);
                PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service POID value not passed in input as it is mandatory field", ebufp);
                goto CLEANUP;
        }

	/******************************************************************
	 * INITIAL CHECK TO AVOID TRIAL PLAN REACTIVATION WITHIN 30 DAYS
	 ******************************************************************/
	if (strncmp(PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp)),bb_service_type,strlen(bb_service_type))==0)
	{
		time_t 	    *exact_created_t =NULL;
		char  		    *event_typ=NULL;
		poid_t		    *act_pdp=NULL;
		poid_t  	    *plan_pdp=NULL;
		poid_t		    *service_pdp=NULL;
		poid_t		    *prod_pdp =NULL;
		pin_flist_t	    *subscr_prods_flistp = NULL;
		pin_flist_t	    *event_type_flistp = NULL;
		pin_flist_t	    *usage_flistp = NULL;
		pin_flist_t	    *last_plan_flistp = NULL;
		time_t		    current_t = pin_virtual_time(NULL);
		int32		    *days_threshold = NULL;
		int32		    err = 0; 
		int		cpt_telem_id = 0;
		pin_cookie_t	cpt_tcookie = NULL;

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ENTERING TRIAL PLAN VALIDATION");
		pin_conf("fm_mso_cust", "days_threshold", PIN_FLDT_INT, (caddr_t*)&days_threshold, &err ); 
		act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		/*commented as old logic fails in selfcare and UPASS */
		//last_plan_flistp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_LISTS, 1, ebufp);

		while ((last_plan_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,&cpt_telem_id, 1, &cpt_tcookie, ebufp)) != NULL) 
		{
			if (last_plan_flistp && last_plan_flistp != NULL && *(int*) PIN_FLIST_FLD_GET(last_plan_flistp, PIN_FLD_FLAGS, 1, ebufp) == 1)
				break;
		}
		if (last_plan_flistp && last_plan_flistp != NULL)
		{
			plan_pdp = PIN_FLIST_FLD_GET(last_plan_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);		
			fm_mso_get_event_from_plan(ctxp,plan_pdp,&event_type_flistp,ebufp);
			//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "NEW PLAN EVENT TYPE FOR VALIDATION OF TRIAL", event_type_flistp);
			if (event_type_flistp && event_type_flistp != NULL && PIN_FLIST_ELEM_COUNT(event_type_flistp, PIN_FLD_RESULTS, ebufp) > 0)
			{
				last_plan_flistp = PIN_FLIST_ELEM_GET(event_type_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
				usage_flistp = PIN_FLIST_ELEM_GET(last_plan_flistp, PIN_FLD_USAGE_MAP, 0, 1, ebufp);
				event_typ = PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
			}
		}	
		fm_mso_get_subscr_purchased_products(ctxp, act_pdp, service_pdp, &subscr_prods_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SUBSCRIPTION PLAN FOR VALIDATION OF TRIAL", subscr_prods_flistp);

		if (subscr_prods_flistp && subscr_prods_flistp != NULL && PIN_FLIST_ELEM_COUNT(subscr_prods_flistp, PIN_FLD_RESULTS, ebufp) > 0 && event_typ && strstr(event_typ, "trial")!=NULL && (strlen(strstr(event_typ, "trial"))>0))
		{	
			event_type_flistp = (pin_flist_t*)NULL;
			usage_flistp = (pin_flist_t *)NULL;
			last_plan_flistp = (pin_flist_t *)NULL;
			event_typ=(char *)NULL;
			//REINITIALIZING VARIABLES TO AVOID FAILOVERS
			last_plan_flistp = PIN_FLIST_ELEM_GET(subscr_prods_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			exact_created_t = PIN_FLIST_FLD_GET(last_plan_flistp, PIN_FLD_CYCLE_START_T, 1, ebufp);
			prod_pdp = PIN_FLIST_FLD_GET(last_plan_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
			fm_mso_get_event_type_base(ctxp,prod_pdp,&event_type_flistp,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OLD PLAN EVENT TYPE FOR VALIDATION OF TRIAL", event_type_flistp);
			if (event_type_flistp && event_type_flistp != NULL && PIN_FLIST_ELEM_COUNT(event_type_flistp, PIN_FLD_RESULTS, ebufp) > 0)
			{
				last_plan_flistp = PIN_FLIST_ELEM_GET(event_type_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
				event_typ = PIN_FLIST_FLD_GET(last_plan_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
			}

			if (event_typ && event_typ != NULL && strstr(event_typ, "trial")!=NULL && (strlen(strstr(event_typ, "trial"))>0) && (((fm_utils_time_round_to_midnight(current_t) - *exact_created_t)/60/60/24) < *(int32*)days_threshold))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validation Successful.");
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51490", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Sorry Cannot Activate Trial Plan More than Once.", ebufp);
				goto CLEANUP;
			}
			else
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old Plan is Not a Trial Plan or Date Difference is greater than threshold hence Avoiding Validation."); 
		}
		else 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "New Plan is Not a Trial Plan or Last Purchased product not found hence Avoiding Validation.");

		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Some Unhandled Data Exception has Occured in Initial Validation for BB Customer");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Initial Validations", ebufp);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51491", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Unhandled Data Exception in Initial Validation.", ebufp);
			goto CLEANUP;
		}
	}

	/***********************************************************************
	 * END OF INITIAL CHECK TO AVOID TRIAL PLAN REACTIVATION WITHIN 30 DAYS
	 ************************************************************************/


	/******************************************************************
	 * Check the service type. Call BB function for broadband service
	 ******************************************************************/
	service_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if(service_obj && !PIN_POID_IS_NULL(service_obj)) {
		ser_type = (char *)PIN_POID_GET_TYPE(service_obj);
		if(ser_type && strncmp(ser_type,bb_service_type,strlen(bb_service_type))==0) {
			//Service type is broadband
			fm_mso_bb_change_plan(connp, ctxp, i_flistp, &ret_flistp, ebufp);
			//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "OLD PLAN EVENT TYPE FOR VALIDATION OF TRIAL", ret_flistp);
			if((!ret_flistp || PIN_ERRBUF_IS_ERR(ebufp))) {
				PIN_ERRBUF_CLEAR(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51751", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Change plan failed for broadband service.", ebufp);
			}
			goto CLEANUP;
		}
	}


	/*******************************************************************
	 * Validation for Product rule  - 
	 *******************************************************************/
	fm_mso_get_account_info(ctxp, account_obj, &acnt_info, ebufp);
    et_zonep = PIN_FLIST_FLD_GET(acnt_info, MSO_FLD_ET_ZONE, 0, ebufp);
	//	fm_mso_get_business_type_values(ctxp, *(int32*)PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp), 
	//	                        &account_type, &account_model, &subscriber_type, ebufp );

	//subscriber_type = fm_mso_get_parent_info(ctxp, i_flistp, account_obj, &acnt_info, ebufp);
	vp = (int32*)PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
	if (vp && vp != NULL)
	{
		tmp_business_type = *(int32*)vp;
	}
	num_to_array(tmp_business_type, arr_business_type);
	account_type    = 10*(arr_business_type[0])+arr_business_type[1];// First 2 digits
	subscriber_type = 10*(arr_business_type[2])+arr_business_type[3];// next 2 digits
	account_model   = arr_business_type[4];				 // 5th field
	corporate_type  = arr_business_type[6];				 // 7th field
	company_type  = arr_business_type[7];				 // 8th field
    if (subscriber_type == 48)
    {
        company_type = 9;
    }
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_FAMILY_ID, &company_type, ebufp);
	/**** CATV changes ***/
	subscriber_type = fm_mso_get_parent_info(ctxp, i_flistp, account_obj, &acnt_info, ebufp);
        if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
        {

		if ((account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD ) ||(subscriber_type == CORPORATE_CATV_CHILD))
		{
			//		PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_SUBSCRIBER_TYPE, &subscriber_type, ebufp );
			PIN_FLIST_CONCAT(i_flistp,acnt_info, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan input flist before pay_type_corp", i_flistp);
			fm_mso_validate_pay_type_corp( ctxp, i_flistp, MSO_OP_CUST_CHANGE_PLAN, &ret_flistp, ebufp);
			if (ret_flistp && ret_flistp != NULL)
			{
				ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
				if (ret_status == change_plan_failure)
				{
					goto CLEANUP;
				}
			}
		}
		else
		{
			fm_mso_validate_pay_type( ctxp, i_flistp, MSO_OP_CUST_CHANGE_PLAN, &ret_flistp, ebufp);
			if (ret_flistp && ret_flistp != NULL)
			{
				ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
				if (ret_status == change_plan_failure)
				{
					goto CLEANUP;
				}
			}
		}
	}

	fm_mso_validation_rule_1(ctxp, i_flistp, MSO_OP_CUST_CHANGE_PLAN, &ret_flistp, ebufp);
	if (ret_flistp && ret_flistp != NULL)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == change_plan_failure)
		{
			goto CLEANUP;
		}
	}

	/*fm_mso_validation_rule_2(ctxp, i_flistp, MSO_OP_CUST_CHANGE_PLAN, &ret_flistp, ebufp);
	if (ret_flistp && ret_flistp != NULL)
	{
		ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (ret_status == change_plan_failure)
		{
			goto CLEANUP;
		}
	}*/
	/*******************************************************************
	 * Validation for PRICING_ADMIN roles - Start
	 *******************************************************************/

	if (user_id && user_id != NULL)
	{
		csr_access = fm_mso_validate_csr_role(ctxp, db, user_id, ebufp);

		if ( csr_access == 0)
		{		
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51424", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ROLE passed in input does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR haas permission to change account information");	
		}
	}else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51425", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}

	/*******************************************************************
	 * Validation for PRICING_ADMIN roles  - End
	 *******************************************************************/

	/*******************************************************************
	 * Change plan - Start
	 *******************************************************************/

	//CATV_REFUND_API_CHANGE - ADDED flags	
	acct_update = fm_mso_change_plan(ctxp, flags, i_flistp, &ret_flistp, ebufp);

	if ( acct_update == 0)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan no need of Change plan");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan Change plan successful");	
		// Calling Validate for FDP and ET Alignment
        if (service_obj && strstr(PIN_POID_GET_TYPE(service_obj), "catv") && et_zonep && strstr(et_zonep, "SK_"))
		{
            fm_mso_get_all_billinfo(ctxp, account_obj, 1, &r_flistp, ebufp);
            result_flist = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
            bi_pdp = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp);
            actg_next_t = *(time_t *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_ACTG_NEXT_T, 0, ebufp);

            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "I am going to call mso_bill_pol_catv_main");

            mso_bill_pol_catv_main(ctxp, flags, bi_pdp, serv_obj, actg_next_t, 11005, &et_rflistp, ebufp);

            PIN_FLIST_DESTROY_EX(&et_rflistp, NULL);

			/*CATV_REFUND_API_CHANGE -START
			fm_mso_validate_fdp_et_align(ctxp, flags, i_flistp, &r_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ET REFUND OUT FLIST", r_flistp);
			if ( flags == PCM_OPFLG_CALC_ONLY ){
				if (!PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan ET Refund successful");
					if(r_flistp && r_flistp != NULL){
						ref_amt = PIN_FLIST_FLD_GET(r_flistp, MSO_FLD_REFUND_AMOUNT, 1, ebufp);
						if(!pbo_decimal_is_null(ref_amt, ebufp)){
							PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
						}
						else{
							ref_amt = pbo_decimal_from_str("0", ebufp);
							PIN_FLIST_FLD_PUT(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
						}
					}
					else{
						ref_amt = pbo_decimal_from_str("0", ebufp);
						PIN_FLIST_FLD_PUT(ret_flistp, MSO_FLD_REFUND_AMOUNT, ref_amt, ebufp);
					}
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &change_plan_success, ebufp);
					*r_flistpp = ret_flistp;
				}
				else{
					*r_flistpp =  PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ, *r_flistpp, PIN_FLD_SERVICE_OBJ, ebufp );
					PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_ERROR_CODE, "34003", ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp,PIN_FLD_ERROR_DESCR,"ET Refund failed",ebufp);
					PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
				}
				PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
				return;
			}
			//CATV_REFUND_API_CHANGE - END */
		}
		//fm_mso_cust_change_plan_lc_event(ctxp, i_flistp, ret_flistp, ebufp);
                //New Tariff - Transaction mapping commented the below line
		//fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp,ID_CHANGE_PLAN, ebufp );
	}
	else if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan insufficient data provided");	
	}
	else if ( acct_update == 4)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan Validation Rule failed");	
	}

	//
	else if (acct_update == 5)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan Old plan is in deactive state ");
	}
	/*******************************************************************
	 * Change plan  - End
	 *******************************************************************/

CLEANUP:	
	*r_flistpp = ret_flistp;
	return;
}


/**************************************************
 * Function: fm_mso_change_plan()
 * 
 * 
 ***************************************************/
EXPORT_OP int 
fm_mso_change_plan(
		pcm_context_t		*ctxp,
		int32			flag, //CATV_REFUND_API_CHANGE
		pin_flist_t		*in_flist,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*planlists_flistp = NULL;
	pin_flist_t		*canceldeal_inflistp = NULL;
	pin_flist_t		*canceldeal_outflistp = NULL;
	pin_flist_t		*dealinfo_flistp = NULL;
	pin_flist_t		*deals_flistp = NULL;
	pin_flist_t		*readplan_inflistp = NULL;
	pin_flist_t		*readplan_outflistp = NULL;
	pin_flist_t		*search_inflistp = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*purchasedeal_inflistp = NULL;
	pin_flist_t		*purchasedeal_outflistp = NULL;
	pin_flist_t		*products_flistp = NULL;
	pin_flist_t		*statuschange_inflistp = NULL;
	pin_flist_t		*statuschange_outflistp = NULL;
	pin_flist_t		*status_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*read_iflist = NULL;
	pin_flist_t		*read_oflist = NULL;
	pin_flist_t		*planlist = NULL;
	pin_flist_t		*srch_res_flistp = NULL;
	pin_flist_t		*discount_flistp = NULL;
	pin_flist_t		*disc_flistp = NULL;
	pin_flist_t		*subs_setprod_flistp = NULL;
	pin_flist_t		*subs_setprod_oflistp = NULL;
	pin_flist_t		*subs_pdts_flistp = NULL;
	pin_flist_t		*pdt_disc_search_flistp = NULL;
	pin_flist_t		*base_plan_disc_flistp = NULL;
	pin_flist_t		*get_base_plan_flistp = NULL;
	pin_flist_t		*base_res_flistp = NULL;
	pin_flist_t		*copy_in_flistp = NULL;
	//CATV_REFUND_API_CHANGE - START
	pin_flist_t             *total_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *bal_flistp = NULL;
    pin_flist_t     *sort_fld_flistp = NULL;
	//CATV_REFUND_API_CHANGE - END

	poid_t			*service_obj = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*offering_obj = NULL;
	poid_t			*read_deal_obj = NULL;
	poid_t			*get_deal_obj = NULL;
	poid_t			*product_obj = NULL;
	poid_t			*plan_pdp_1 = NULL;
	poid_t			*plan_pdp_2 = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*prd_pdp = NULL;
	poid_t			*deal_pdp = NULL;
	poid_t			*pdt_pdp = NULL;
	poid_t			*base_plan_obj = NULL;
	//CATV_REFUND_API_CHANGE - START
	poid_t			*product_poid = NULL;
	poid_t			*event_pdp = NULL;
	//CATV_REFUND_API_CHANGE -END
    time_t          *purchase_endtp = NULL;
    pin_fld_num_t   ield_no = 0;
	int			elem_id = 0;
	int			elem_base = 0;
	int			*package_id = 0;
	int			elem_pp = 0;
	int			elem_id1 = 0;
	int                     c_elem_id = 0; //CATV_REFUND_API_CHANGE
	int                     b_elem_id = 0;//CATV_REFUND_API_CHANGE
	int			status = PIN_PRODUCT_STATUS_ACTIVE;
	int			status_flags = PIN_STATUS_FLAG_ACTIVATE;
	int			change_plan_success = 0;
	int			change_plan_failure = 1;

	pin_cookie_t		cookie_base = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie_pp = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t		cookie_countdeals = NULL;
	pin_cookie_t            c_cookie = NULL; //CATV_REFUND_API_CHANGE
	pin_cookie_t            b_cookie = NULL; //CATV_REFUND_API_CHANGE

	int			plan_found = 0;
	int			elem_countdeals = 0;
	int			flags = 768;
	int			*action_flags = NULL;
	int			cnt = 0;
	int			cnt2 = 0;
	int32                     srch_flags = 256;
	int64			db = 0;
	int32			count = 0;
	int32			validate_change = 0;
	int32			ret_status = -1;
	int32			failure	= 1;
	int32			errorCode= 0;
	int32                    grace_flags = 0xFFF;
	int32			*status_flgs = NULL;
	pin_decimal_t		*hundred = pbo_decimal_from_str("90.0",ebufp);
	pin_decimal_t		*priority      = NULL;
	pin_decimal_t           *disc_per = NULL;
	pin_decimal_t           *disc_amt = NULL;
	pin_decimal_t           *one = pbo_decimal_from_str("1.0", ebufp );
	pin_decimal_t		*zero_disc  = NULL;
	//CATV_REFUND_API_CHANGE - START
	pin_decimal_t		*ref_amt      = NULL;
	pin_decimal_t		*tax_amtp = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*total_ref = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*taxp = NULL;
	//CATV_REFUND_API_CHANGE - END

	char			search_template[100] = " select X from /purchased_product where F1 = V1 and F2 = V2 ";
	char			*plan_name = NULL;
	char			errorMsg[80]    = "";
	//char			srch_tmpl[100] = " select X from /purchased_product where F1 = V1 and F2 = V2  and F3 = V3  and F4 = V4 ";
	char                  srch_tmpl[100] = " select X from /purchased_product where F1 = V1 and F2 = V2  and F3 = V3 ";
	char			*cust_city = NULL;
	char			*das_type = NULL;
	char			*tax_code = NULL; //CATV_REFUND_API_CHANGE
	char			msg[512] ;

	pin_fld_num_t		field_no = 0;
	int32 			comp_flag = 0;
	int			product_status = 1;
	int32			discount_flag = -1;
	int			pp_type = 2;
	int32			count1 = 0;
	int32			conn_type = -1;
	int32			pkg_type = -1;
	int			add_plan_failure = 1;
	int32			is_base = -1;

        int                     plan_list_count = 0;
        int                     deal_list_count = 0;
	pin_flist_t 		*active_purch_prod_flistp= NULL;
        pin_flist_t             *add_pl_flistp= NULL;
        pin_flist_t             *add_deal_flistp= NULL;
        pin_flist_t             *purchprod_flistp= NULL;
        int                     elem_id_r = 0;
        pin_cookie_t            cookie_r = NULL;
        pin_flist_t             *in_purchdeal_flistp= NULL;
        int                     elem_in = 0;
        pin_cookie_t            cookie_in = NULL;

        //New Tariff - Transaction mapping Start */
        pin_flist_t             *lifecycle_flistp = NULL;
        pin_flist_t             *eflistp = NULL;
        int32                   offer_count = 0;
        //New Tariff - Transaction mapping End */

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;

	zero_disc = pbo_decimal_from_str("0.0", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan : ", in_flist);	

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );

	planlists_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_PLAN_LISTS, 0, 1, ebufp );

	if (!planlists_flistp || !service_obj)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - Service object or plan object is missing in input", ebufp);

		*ret_flistp = r_flistp;
		return 3;
	}

	/***************************************************************************
	 * Apply validation rules for change plan
	 ***************************************************************************/
/*	validate_change = fm_mso_validate_base_change_rule(ctxp,in_flist, ret_flistp, ebufp);

	if(validate_change == 0)
	{
		return 4;
	}

	flags = 3;

	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &flags, ebufp);
*/
	plan_flistp = PIN_FLIST_CREATE(ebufp);

	planlist = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_PLAN_LISTS, 0, 1, ebufp);
	plan_pdp_1 = PIN_FLIST_FLD_GET(planlist, PIN_FLD_PLAN_OBJ, 0, ebufp );

	planlist = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_PLAN_LISTS, 1, 1, ebufp);
	plan_pdp_2 = PIN_FLIST_FLD_GET(planlist, PIN_FLD_PLAN_OBJ, 0, ebufp );

	// validation to prevent change plan on same plan
	if(!PIN_POID_COMPARE(plan_pdp_1, plan_pdp_2, 0,ebufp))	
	{
		PIN_ERRBUF_CLEAR(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51427", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "change plan to same plan not allowed ", ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, ebufp);
		*ret_flistp = r_flistp;
		return 3;
	}


	// Validating for grace period change plan 
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(plan_pdp_1), "/search",-1, ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, srch_pdp, ebufp);	
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, srch_tmpl, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp_1, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);
	//args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 4, ebufp);
	//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS_FLAGS, &grace_flags, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_STATUS_FLAGS , &status_flags, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PACKAGE_ID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(3, "Search flist for grace", search_inflistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_inflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching Deactive plans", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, ebufp);
		return 0;
	}

	PIN_ERR_LOG_FLIST(3, "Search out flist for grace", search_outflistp);

	srch_res_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
        plan_pdp_1 = PIN_FLIST_FLD_TAKE(srch_res_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp );

	if(!srch_res_flistp)
	{
		PIN_ERRBUF_CLEAR(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51427", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Old plan is in inactive/cancelled state", ebufp);
		PIN_FLIST_DESTROY_EX(&search_outflistp, ebufp);
		*ret_flistp = r_flistp;
		return 5 ;
	}

        if(srch_res_flistp)
        {

		status_flgs = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_STATUS_FLAGS, 0, ebufp);
		if(*status_flgs == grace_flags) 
		{
			PIN_ERRBUF_CLEAR(ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51427", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Old plan is in deactive state - do renewal/cancel plan", ebufp);
			PIN_FLIST_DESTROY_EX(&search_outflistp, ebufp);
			*ret_flistp = r_flistp;
			return 5 ;
		}
	}

	/*
	   if ((PIN_POID_COMPARE(plan_pdp_1, plan_pdp_2, 0, ebufp)) == 0)
	   {
	   r_flistp = PIN_FLIST_CREATE(ebufp);
	   PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	   PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
	   PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
	   PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Please choose different plans for ADDTION and CANCELLATION", ebufp);

	 *ret_flistp = r_flistp;
	 return 3;
	 }
	 */
	/*NTO : Additional validations start  */

        //New Tariff - Transaction mapping Start*/
        if(lifecycle_flistp == NULL) {
                lifecycle_flistp = PIN_FLIST_CREATE(ebufp);
        }
        //New Tariff - Transaction mapping End*/
	
	PIN_FLIST_ELEM_DROP(in_flist, PIN_FLD_PLAN_LISTS, 0, ebufp);
	in_purchdeal_flistp = PIN_FLIST_ELEM_ADD(in_flist, PIN_FLD_PLAN_LISTS, 0, ebufp);

        while ((purchprod_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_in, 1, &cookie_in, ebufp )) != NULL)
	{
		add_deal_flistp = PIN_FLIST_ELEM_ADD(in_purchdeal_flistp, PIN_FLD_DEALS, elem_in, ebufp);
		deal_pdp = PIN_FLIST_FLD_TAKE(purchprod_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp); 
		PIN_FLIST_FLD_PUT(add_deal_flistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);

		deal_pdp = PIN_FLIST_FLD_TAKE(purchprod_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp); 
		PIN_FLIST_FLD_PUT(add_deal_flistp, PIN_FLD_PACKAGE_ID, deal_pdp, ebufp);
	}
        PIN_FLIST_FLD_SET(in_purchdeal_flistp, PIN_FLD_FLAGS, 0, ebufp);
        PIN_FLIST_FLD_PUT(in_purchdeal_flistp, PIN_FLD_PLAN_OBJ, plan_pdp_1, ebufp);

        PIN_FLIST_DESTROY_EX(&search_outflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modified input flist to deals of plan from purchased_product ", in_flist);

        /***************************************************************************
         * Apply validation rules for change plan
         ***************************************************************************/
        validate_change = fm_mso_validate_base_change_rule(ctxp,in_flist, ret_flistp, ebufp);

        if(validate_change == 0)
        {
                return 4;
        }

        flags = 3;

        provaction_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
        PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &flags, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "checking for addon active plans");
        fm_mso_get_active_purch_prod_excp_plan(ctxp, in_flist, plan_pdp_1, &active_purch_prod_flistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in active_purch_prod_flistp", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&active_purch_prod_flistp, ebufp);
                return 0;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "active_purch_prod_flistp", active_purch_prod_flistp);
	if ( PIN_FLIST_ELEM_COUNT( active_purch_prod_flistp, PIN_FLD_RESULTS, ebufp) > 0 )
	{
		plan_list_count = PIN_FLIST_ELEM_COUNT(in_flist, PIN_FLD_PLAN_LISTS, ebufp)	;	
		elem_id_r = 0;
		cookie_r = NULL;
		while ((purchprod_flistp = PIN_FLIST_ELEM_GET_NEXT(active_purch_prod_flistp, PIN_FLD_RESULTS, &elem_id_r, 1, &cookie_r, ebufp )) != NULL)
		{
            plan_obj = PIN_FLIST_FLD_GET(purchprod_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
                
		    elem_id1 = 0;
		    cookie1 = NULL;
            plan_found = 0;
            while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, 
                        PIN_FLD_PLAN_LISTS, &elem_id1, 1, &cookie1, ebufp )) != NULL)
            {
                plan_pdp_2 = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
                if (PIN_POID_COMPARE(plan_obj, plan_pdp_2, 0, ebufp) == 0)
                {
                    deal_list_count = PIN_FLIST_ELEM_COUNT(planlists_flistp, PIN_FLD_DEALS, ebufp);

                    add_deal_flistp = PIN_FLIST_ELEM_ADD(planlists_flistp, PIN_FLD_DEALS, deal_list_count++, ebufp);
                    deal_pdp = PIN_FLIST_FLD_TAKE(purchprod_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp);
                    PIN_FLIST_FLD_PUT(add_deal_flistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
			
                    deal_pdp = PIN_FLIST_FLD_TAKE(purchprod_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp); 
			        PIN_FLIST_FLD_PUT(add_deal_flistp, PIN_FLD_PACKAGE_ID, deal_pdp, ebufp);
                    plan_found = 1;
                    break;
                }
            }

            if (!plan_found)
            {
			    add_pl_flistp = PIN_FLIST_ELEM_ADD(in_flist, PIN_FLD_PLAN_LISTS, plan_list_count++, ebufp);
			    PIN_FLIST_FLD_COPY(purchprod_flistp, PIN_FLD_PLAN_OBJ, add_pl_flistp, PIN_FLD_PLAN_OBJ, ebufp );

			    add_deal_flistp = PIN_FLIST_ELEM_ADD(add_pl_flistp, PIN_FLD_DEALS, 0, ebufp);
		
	    		deal_pdp = PIN_FLIST_FLD_TAKE(purchprod_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp); 
		    	PIN_FLIST_FLD_PUT(add_deal_flistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
    
	    		deal_pdp = PIN_FLIST_FLD_TAKE(purchprod_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp); 
		    	PIN_FLIST_FLD_PUT(add_deal_flistp, PIN_FLD_PACKAGE_ID, deal_pdp, ebufp);
			    PIN_FLIST_FLD_SET(add_pl_flistp, PIN_FLD_FLAGS, &plan_found, ebufp);
            }
		}
	}

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modified input flist ", in_flist);
        sort_fld_flistp = PIN_FLIST_CREATE(ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(sort_fld_flistp,PIN_FLD_PLAN_LISTS, 0, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_FLAGS, NULL, ebufp);
        PIN_FLIST_SORT(in_flist,sort_fld_flistp,1,ebufp);

        PIN_FLIST_DESTROY_EX(&sort_fld_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&active_purch_prod_flistp, ebufp);
        /*NTO : Additional validations end  */

	copy_in_flistp = PIN_FLIST_COPY(in_flist, ebufp);
	elem_base = 0;
	cookie_base = NULL;
	while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(copy_in_flistp, PIN_FLD_PLAN_LISTS, &elem_base, 1, &cookie_base, ebufp )) != NULL)
	{
		action_flags = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_FLAGS, 0, ebufp );

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in setting change deal function", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			return 0;
		}

		if (*(int*)action_flags == 0)
		{
			plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );

			if (!service_obj || !plan_obj)
			{
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - plan object is missing in input", ebufp);

				*ret_flistp = r_flistp;
				return 3;
			}

			if(PIN_FLIST_ELEM_COUNT(planlists_flistp, PIN_FLD_DEALS , ebufp) == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Deal array missing in input.");
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51432", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Change plan - Deal array missing in input.", ebufp);
				*ret_flistp = r_flistp;
				return 0;
			}
			db = PIN_POID_GET_DB(plan_obj);
			readplan_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_POID, plan_obj, ebufp );
			PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_NAME, NULL, ebufp );
                        PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_CODE, NULL, ebufp );
			deals_flistp = PIN_FLIST_ELEM_ADD(readplan_inflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_SET(deals_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan read plan input list", readplan_inflistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readplan_inflistp, &readplan_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&readplan_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);

				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - PCM_OP_READ_FLDS of plan poid error", ebufp);

				*ret_flistp = r_flistp;
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan read plan output flist", readplan_outflistp);
			/***************************************************************************
			 * Validation for not allowing to cancel 
			 ****************************************************************************/
			plan_name = (char*)PIN_FLIST_FLD_GET(readplan_outflistp, PIN_FLD_NAME, 1, ebufp);
			if (plan_name && strstr(plan_name, "STB Zero Value" ))
			{
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "STB Zero Value/STB Zero Value ADV Plans can not be cancelled", ebufp);

				*ret_flistp = r_flistp;
                                PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);
				return 0;

			}
			args_flistp = PIN_FLIST_ELEM_ADD(plan_flistp, PIN_FLD_PLAN, cnt2++, ebufp);
			PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_NAME, args_flistp, PIN_FLD_NAME, ebufp);

                        PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_CODE, planlists_flistp, PIN_FLD_CODE, ebufp);

			elem_id = 0;
			cookie = NULL;
			while ((deals_flistp = PIN_FLIST_ELEM_GET_NEXT(readplan_outflistp, PIN_FLD_SERVICES, &elem_id, 1, &cookie, ebufp)) != NULL)
			{
				canceldeal_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, canceldeal_inflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, canceldeal_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, canceldeal_inflistp, PIN_FLD_DESCR, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, canceldeal_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
				dealinfo_flistp = PIN_FLIST_ELEM_ADD(canceldeal_inflistp, PIN_FLD_DEAL_INFO, 0, ebufp );
				PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, dealinfo_flistp, PIN_FLD_DEAL_OBJ, ebufp);
				//PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_PACKAGE_ID, dealinfo_flistp, PIN_FLD_PACKAGE_ID, ebufp);

				read_deal_obj = PIN_FLIST_FLD_GET(deals_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
				elem_countdeals = 0;
				cookie_countdeals = NULL;
				while ((args_flistp = PIN_FLIST_ELEM_GET_NEXT( planlists_flistp, PIN_FLD_DEALS, &elem_countdeals, 1, &cookie_countdeals, ebufp)) != NULL)
				{
					get_deal_obj = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
					if ((PIN_POID_COMPARE(read_deal_obj, get_deal_obj, 0, ebufp)) == 0)
					{
						package_id = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_PACKAGE_ID, 1, ebufp);
						break;
					}
				}
				if(!package_id || (package_id && *package_id == 0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Package Id not provided in input");
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Change plan - Package Id not provided in input", ebufp);
					*ret_flistp = r_flistp;
					return 0;
				}

				PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_PACKAGE_ID, package_id, ebufp);
				
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan input list", canceldeal_inflistp);
				//CATV_REFUND_API_CHANGE - Added flag
				PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_CANCEL_DEAL, flag, canceldeal_inflistp, &canceldeal_outflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&canceldeal_inflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_CANCEL_DEAL", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);

					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - PCM_OP_SUBSCRIPTION_CANCEL_DEAL error", ebufp);

					*ret_flistp = r_flistp;
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan output flist", canceldeal_outflistp);

				// To return the refund amount when this opcode is called in CALC_ONLY mode - //CATV_REFUND_API_CHANGE
				if ((service_obj && strcmp(PIN_POID_GET_TYPE(service_obj),"/service/catv") == 0) && flag == PCM_OPFLG_CALC_ONLY )
				{
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					c_elem_id = 0;
					c_cookie = NULL;
					while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT( canceldeal_outflistp, 																PIN_FLD_RESULTS, &c_elem_id, 1, &c_cookie, ebufp )) != NULL ){
						event_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 1, ebufp);
						if (event_pdp && strstr((char *)PIN_POID_GET_TYPE(event_pdp),"/event/billing/product/fee/cycle")){
							total_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_TOTAL, 356, 1, ebufp);
							if ( total_flistp != NULL ){
								ref_amt = PIN_FLIST_FLD_GET(total_flistp, PIN_FLD_AMOUNT, 1, ebufp);
								//ADDED to capture the two refund scenario
								pbo_decimal_add_assign(total_ref, ref_amt, ebufp);
								//PIN_FLIST_FLD_COPY(total_flistp, PIN_FLD_AMOUNT, r_flistp, PIN_FLD_AMOUNT, ebufp );
							}
						}
						b_elem_id = 0;
						b_cookie = NULL;
						while((bal_flistp = PIN_FLIST_ELEM_GET_NEXT( res_flistp,                                                                                                                             PIN_FLD_BAL_IMPACTS, &b_elem_id, 1, &b_cookie, ebufp )) != NULL){
							tax_code = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_TAX_CODE, 1, ebufp);	
							if(tax_code && (strcmp(tax_code, "CGST") == 0 || strcmp(tax_code, "SGST") == 0 ||
										strcmp(tax_code, "IGST") == 0))
							{
								taxp = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_AMOUNT, 1, ebufp);
								pbo_decimal_add_assign(tax_amtp, taxp, ebufp);
							}
						}
					}
					/*if (ref_amt ){
					  PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_AMOUNT, ref_amt, ebufp);
					  }
					  else{ // if no ref is given, then set to zero
					  PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_AMOUNT, pbo_decimal_from_str("0", ebufp), ebufp);
					  }*/
					PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_AMOUNT, total_ref, ebufp);
					PIN_FLIST_FLD_PUT(r_flistp, PIN_FLD_AMOUNT_TAXED, tax_amtp, ebufp);
					PIN_FLIST_FLD_PUT(r_flistp, MSO_FLD_REFUND_AMOUNT, pbo_decimal_from_str("0", ebufp), ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_success, ebufp);

					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan cancel_deal CALCONLY ouput", r_flistp);
					*ret_flistp = r_flistp;
					PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
					continue;
					//return 2;

				}
				//CATV_REFUND_API_CHANGE - END
				PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);

				/*************************** Searching purchased product *************************************/
				flags = 768;

				search_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &flags, ebufp);
				PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp );
				PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, args_flistp, PIN_FLD_DEAL_OBJ, ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp );
				//PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_PACKAGE_ID, args_flistp, PIN_FLD_PACKAGE_ID, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PACKAGE_ID, package_id, ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp );
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, args_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

				results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp );

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan search purchased product input list", search_inflistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);

					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - Error in fetching PIN_FLD_OFFERING_OBJ from return flist of PCM_OP_SEARCH", ebufp);

					*ret_flistp = r_flistp;
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan search purchased product output flist", search_outflistp);

				elem_pp = 0;
				cookie_pp = NULL;
				while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, &elem_pp, 1, &cookie_pp, ebufp )) != NULL)
				{
					product_poid = NULL;
					is_base = -1;
					offering_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID , 1, ebufp);
					//Fetching offering_obj for ET refund 
					product_poid = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);

					is_base = fm_mso_catv_pt_pkg_type(ctxp, product_poid, ebufp);
					if(is_base ==0)
					{
						PIN_FLIST_FLD_SET(in_flist, PIN_FLD_OFFERING_OBJ, offering_obj, ebufp);
					}

	                                //New Tariff - Transaction mapping Start */
                                         eflistp = PIN_FLIST_ELEM_ADD(lifecycle_flistp, PIN_FLD_OFFERINGS, offer_count++, ebufp);
                                         PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, eflistp, PIN_FLD_OFFERING_OBJ, ebufp);
                                         PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_PACKAGE_ID, eflistp, PIN_FLD_PACKAGE_ID, ebufp);
	                                //New Tariff - Transaction mapping End */

					/****** Read the purchased product and product to get the priority ***************/

					read_iflist = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_POID , offering_obj , ebufp);
					PIN_FLIST_FLD_SET(read_iflist,PIN_FLD_PRODUCT_OBJ , (void *) NULL , ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan read purchased product input flist", read_iflist);

					PCM_OP(ctxp,PCM_OP_READ_FLDS, 0 , read_iflist , &read_oflist , ebufp);

		                        PIN_FLIST_DESTROY_EX(&read_iflist, NULL);

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan read purchased product output flist", read_oflist);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS for offering_obj ", ebufp);
	                                        PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
                                                r_flistp = PIN_FLIST_CREATE(ebufp);
                                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in reading offering_obj", ebufp);
                                                *ret_flistp = r_flistp;
	                                        PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
                                                return 3;
					}

                                        product_obj = PIN_FLIST_FLD_TAKE(read_oflist, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
					read_iflist = PIN_FLIST_CREATE(ebufp);
                                        PIN_FLIST_FLD_PUT(read_iflist, PIN_FLD_POID , product_obj , ebufp);
		                        PIN_FLIST_DESTROY_EX(&read_oflist, NULL);

					PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_PRIORITY, NULL, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan read product input flist", read_iflist);

					PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_iflist, &read_oflist, ebufp);

		                        PIN_FLIST_DESTROY_EX(&read_iflist, NULL);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan read product output flist", read_oflist);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS for reading product object ", ebufp);
	                                        PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&read_oflist, NULL);

					}
					priority = (pin_decimal_t *)PIN_FLIST_FLD_GET(read_oflist, PIN_FLD_PRIORITY, 0, ebufp);
					if (pbo_decimal_compare(priority, hundred, ebufp) == -1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " this is a cancel of Hardware product");
					}
					else
					{

                                        /*NTO:Start Fix to error if provisioning tag not available in /mso_cfg_catv_pt*/
                                        if(is_base == -1)
                                        {
                                                PIN_ERRBUF_RESET(ebufp);
                                                r_flistp = PIN_FLIST_CREATE(ebufp);
                                                PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Provisioning tag notavailable in mso_cfg_catv_pt", ebufp);
                                                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"product_poid not having provisiontag entry in mso_cfg_catv_pt", product_poid);

                                                *ret_flistp = r_flistp;
                                                return 3;
                                        }
                                        /*NTO:End Fix to error out if provisioning tag not available in /mso_cfg_catv_pt*/


						args_flistp = PIN_FLIST_ELEM_ADD(provaction_inflistp, PIN_FLD_PRODUCTS, cnt++, ebufp );
						PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);
						flags = 1;
						PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS_FLAGS, &flags, ebufp);
					}
					PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
				}			
				PIN_FLIST_DESTROY_EX(&search_outflistp, NULL);
			}
			PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);

		}
		else if (*(int*)action_flags == 1 && flag != PCM_OPFLG_CALC_ONLY )
		{
			plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );

			if (!service_obj || !plan_obj)
			{
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - plan object is missing in input", ebufp);

				*ret_flistp = r_flistp;
				return 3;
			}

			readplan_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_POID, plan_obj, ebufp );
			PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_NAME, NULL, ebufp );
                        PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_CODE, NULL, ebufp );
			deals_flistp = PIN_FLIST_ELEM_ADD(readplan_inflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_SET(deals_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan read plan input list", readplan_inflistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readplan_inflistp, &readplan_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&readplan_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);

				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - PCM_OP_READ_FLDS of plan poid error", ebufp);
                                PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);
				*ret_flistp = r_flistp;
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan read plan output flist", readplan_outflistp);
                        PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_CODE, planlists_flistp, PIN_FLD_CODE, ebufp);

			/******************************************************************************
			 * Validation for adding ET rule Delhi
			 * Start
			 ******************************************************************************/
			//		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, readplan_outflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, readplan_outflistp, PIN_FLD_SERVICE_OBJ, ebufp);
			acnt_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp); 
			fm_mso_et_product_rule_delhi(ctxp, readplan_outflistp, acnt_pdp, MSO_OP_CUST_CHANGE_PLAN, &r_flistp, ebufp);
			if (r_flistp)
			{
				ret_status = *(int32*)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STATUS, 0, ebufp);
				if (ret_status == failure)
				{
					*ret_flistp = r_flistp;
					PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);
					return 0;
				}
			}
			/******************************************************************************
			 * Validation for adding ET rule Delhi
			 * End
			 ******************************************************************************/
			args_flistp = PIN_FLIST_ELEM_ADD(plan_flistp, PIN_FLD_PLAN, cnt2++, ebufp);
			PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_NAME, args_flistp, PIN_FLD_NAME, ebufp);

			elem_id = 0;
			cookie = NULL;
			while ((deals_flistp = PIN_FLIST_ELEM_GET_NEXT(readplan_outflistp, PIN_FLD_SERVICES, &elem_id, 1, &cookie, ebufp)) != NULL)
			{
				purchasedeal_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, purchasedeal_inflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, purchasedeal_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
				PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_DESCR,purchasedeal_inflistp,PIN_FLD_DESCR, ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, purchasedeal_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);

				dealinfo_flistp = PIN_FLIST_ELEM_ADD(purchasedeal_inflistp, PIN_FLD_DEAL_INFO, 0, ebufp );
				PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, dealinfo_flistp, PIN_FLD_DEAL_OBJ, ebufp);
				PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan input list", purchasedeal_inflistp);
                                //CATV_REFUND_API_CHANGE - Added flag
				PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, flag, purchasedeal_inflistp, &purchasedeal_outflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&purchasedeal_inflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);

					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - PCM_OP_SUBSCRIPTION_PURCHASE_DEAL error", ebufp);

					*ret_flistp = r_flistp;
					return 0;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan output flist", purchasedeal_outflistp);

				elem_id1 = 0;
				cookie1 = NULL;
				while ((products_flistp = PIN_FLIST_ELEM_GET_NEXT(purchasedeal_outflistp, PIN_FLD_PRODUCTS, &elem_id1, 1, &cookie1, ebufp)) != NULL)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan products_flistp ", products_flistp);

					offering_obj = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp );
					prd_pdp = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);	

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan products_flistp ", products_flistp);

					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fetching PIN_FLD_OFFERING_OBJ from return flist of PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);

						r_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - Error in fetching PIN_FLD_OFFERING_OBJ from return flist of PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);

						*ret_flistp = r_flistp;
						return 0;
					}

					if (offering_obj)
					{
	                                        //New Tariff - Transaction mapping Start */
        	                                 eflistp = PIN_FLIST_ELEM_ADD(lifecycle_flistp, 
										PIN_FLD_OFFERINGS, offer_count++, ebufp);
                	                         PIN_FLIST_FLD_COPY(products_flistp, PIN_FLD_OFFERING_OBJ, 
										eflistp, PIN_FLD_OFFERING_OBJ, ebufp);
                        	                 PIN_FLIST_FLD_COPY(products_flistp, PIN_FLD_PACKAGE_ID, 
										eflistp, PIN_FLD_PACKAGE_ID, ebufp);
                                	        //New Tariff - Transaction mapping End */

						if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
						{
                            purchase_endtp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PURCHASE_END_T, 1, ebufp );
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV Service");
							pkg_type = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp);
							if (pkg_type == 0)
							{
								pkg_type = NCF_PKG_TYPE;
							}
							pdt_disc_search_flistp = PIN_FLIST_CREATE(ebufp); 
							PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, service_obj, ebufp);
							PIN_FLIST_FLD_SET(pdt_disc_search_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
							PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
							PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Looking for Account Level Discount Config");
							ret_status = -1;
							discount_flag = -1;
							fm_mso_get_service_level_discount(ctxp, pdt_disc_search_flistp, 0, &discount_flistp, ebufp);
							PIN_ERR_LOG_FLIST(3, "discount flistp", discount_flistp);
							if (discount_flistp)
							{
								ret_status = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_STATUS, 1, ebufp);
								if (ret_status != failure)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
											"Account Level Discount Config Found");
									discount_flag = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, 
											PIN_FLD_DISCOUNT_FLAGS, 1, ebufp);
									if (discount_flag == 0)
									{
										disc_per = PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 1, ebufp);
									}
									else if (discount_flag == 1)
									{
										disc_amt = PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 1, ebufp);
									}
								}
								else
								{
									PIN_ERR_LOG_MSG(3, "no account level discount");
									if (PIN_ERRBUF_IS_ERR(ebufp))
										PIN_ERRBUF_CLEAR(ebufp);
									PIN_FLIST_DESTROY_EX(&pdt_disc_search_flistp, NULL);
									pdt_disc_search_flistp = PIN_FLIST_CREATE(ebufp);
									//PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, prd_pdp, ebufp);
									PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, plan_obj, ebufp);
									// Retrieving the PP Type 
									PIN_ERR_LOG_MSG(3, "no account level discount1");
									pp_type = fm_mso_get_cust_type(ctxp, acnt_pdp, ebufp);
									if (pp_type != 2)
									{
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
												"Valid PP Type Found... ");
										PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
												MSO_FLD_PP_TYPE, &pp_type, ebufp);
									}

									//  Retrieve the Customer City from Account Object
									PIN_ERR_LOG_MSG(3, "no account level discount2");
									fm_mso_get_custcity(ctxp, acnt_pdp, &cust_city, ebufp);
									if (cust_city)
									{
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CITY FOUND... ");
										PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
												PIN_FLD_CITY, cust_city, ebufp);

									}

									// Retrieve the Customer Service Connection Type
									PIN_ERR_LOG_MSG(3, "no account level discount222");
									conn_type = fm_mso_catv_conn_type(ctxp, service_obj, ebufp);
									if (conn_type != -1)
									{
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
												"Valid Connection Type FOUND... ");
										PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
												PIN_FLD_CONN_TYPE, &conn_type, ebufp);
									}

									// Retrieve the Customer DAS Type 
									fm_mso_get_das_type(ctxp, acnt_pdp, &das_type, ebufp);
									if (das_type)
									{
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
												"DAS TYPE FOUND... ");
										PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
												MSO_FLD_DAS_TYPE, das_type, ebufp);
									}

									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
											"Looking for Product Level Discount Config");
									PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
									fm_mso_get_plan_level_discount(ctxp, pdt_disc_search_flistp, &discount_flistp, ebufp);
									if (discount_flistp)
									{
										ret_status = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, 
												PIN_FLD_STATUS, 1, ebufp);
										if (ret_status != failure)
										{
											PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
													"Product Level Discount Config Found");
											discount_flag = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, 
													PIN_FLD_DISCOUNT_FLAGS, 1, ebufp);
											if (discount_flag == 0)
											{
												disc_per = PIN_FLIST_FLD_GET(discount_flistp, 
														PIN_FLD_DISCOUNT, 1, ebufp);
											}
											else if (discount_flag == 1)
											{
												disc_amt = PIN_FLIST_FLD_GET(discount_flistp, 
														PIN_FLD_DISCOUNT, 1, ebufp);
											}
										}

									}
								}
							}

							if (discount_flag != -1 || purchase_endtp)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount Configs Found");
								if (PIN_ERRBUF_IS_ERR(ebufp))
									PIN_ERRBUF_CLEAR(ebufp);

								subs_setprod_flistp = PIN_FLIST_CREATE(ebufp);
								PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, subs_setprod_flistp, PIN_FLD_POID, ebufp);
								PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, subs_setprod_flistp, 
										PIN_FLD_PROGRAM_NAME, ebufp);
								PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_DESCR,subs_setprod_flistp,PIN_FLD_DESCR,ebufp);
								subs_pdts_flistp = PIN_FLIST_ELEM_ADD(subs_setprod_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PRODUCT_OBJ, prd_pdp, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_OFFERING_OBJ, offering_obj, ebufp);

								if (discount_flag == 0 && disc_per)
								{
									PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_DISCOUNT, disc_per, ebufp);
									PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_DISCOUNT, disc_per, ebufp);
									PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_DISCOUNT, disc_per, ebufp);
								}
								else if (disc_amt)
								{
									PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_DISC_AMT, disc_amt, ebufp);
									PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_DISC_AMT, disc_amt, ebufp);
									//Resetting percentage disc to zero for combo packs
									PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_DISCOUNT, zero_disc, ebufp);
									PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_DISCOUNT, zero_disc, ebufp);
									PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_DISCOUNT, zero_disc, ebufp);

								}

                                if (purchase_endtp)
                                {
                                    PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_END_T, purchase_endtp, ebufp);
                                    PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_END_T, purchase_endtp, ebufp);
                                    PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_END_T, purchase_endtp, ebufp);
                                }
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan set product input flist", 
										subs_setprod_flistp);
								PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODINFO , 0, subs_setprod_flistp, 
										&subs_setprod_oflistp, ebufp);
								PIN_FLIST_DESTROY_EX(&subs_setprod_flistp, NULL);

								if (PIN_ERRBUF_IS_ERR(ebufp))
								{
									PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
											"Error in calling PCM_OP_SUBSCRIPTION_SET_PRODINFO ", ebufp);
									PIN_ERRBUF_RESET(ebufp);
									PIN_FLIST_DESTROY_EX(&subs_setprod_oflistp, NULL);

									r_flistp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service add plan - PCM_OP_SUBSCRIPTION_SET_PRODINFO error", ebufp);

									*ret_flistp = r_flistp;
									return 0;
								}
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan set product output flist", subs_setprod_oflistp);
								PIN_FLIST_DESTROY_EX(&subs_setprod_oflistp, NULL);

							}
							else
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Discount Configs Found");
								if (PIN_ERRBUF_IS_ERR(ebufp))
									PIN_ERRBUF_CLEAR(ebufp);
							}
						}
						status_flags = PIN_STATUS_FLAG_ACTIVATE;
						statuschange_inflistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, statuschange_inflistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, statuschange_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
						PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_DESCR,statuschange_inflistp,PIN_FLD_DESCR, ebufp);

						status_flistp = PIN_FLIST_ELEM_ADD(statuschange_inflistp, PIN_FLD_STATUSES, 0, ebufp );
						PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS, &status, ebufp);
						PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);
						PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_OFFERING_OBJ, offering_obj, ebufp);

						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan status change input list", statuschange_inflistp);
						PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODUCT_STATUS , 0, statuschange_inflistp, &statuschange_outflistp, ebufp);
						PIN_FLIST_DESTROY_EX(&statuschange_inflistp, NULL);
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_SET_PRODUCT_STATUS ", ebufp);
                            field_no = ebufp->field;
                            errorCode = ebufp->pin_err;
							PIN_ERRBUF_RESET(ebufp);
							PIN_FLIST_DESTROY_EX(&statuschange_outflistp, NULL);

							r_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
                            if (errorCode == PIN_ERR_BAD_VALUE && field_no == PIN_FLD_CURRENT_BAL)
                            {
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - There is no sufficient balance", ebufp);
                            }
                            else
                            {
							    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - PCM_OP_SUBSCRIPTION_SET_PRODUCT_STATUS error", ebufp);
                            }

							*ret_flistp = r_flistp;
							return 0;
						}
						/****************** read the purchase product object to get the product object and priority *****/
						read_iflist = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_POID , offering_obj , ebufp);
						PIN_FLIST_FLD_SET(read_iflist,PIN_FLD_PRODUCT_OBJ , (void *) NULL , ebufp);

						PCM_OP(ctxp,PCM_OP_READ_FLDS ,0, read_iflist , &read_oflist , ebufp);
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS for reading purchased product ", ebufp);
							PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
						}

						product_obj = PIN_FLIST_FLD_GET(read_oflist,PIN_FLD_PRODUCT_OBJ,1, ebufp);
						PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_POID , product_obj , ebufp);
						PIN_FLIST_FLD_SET(read_iflist,PIN_FLD_PRIORITY ,(void *) NULL , ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Read product input flist after purchase deal is " , read_iflist);
						PCM_OP(ctxp,PCM_OP_READ_FLDS ,0, read_iflist , &read_oflist , ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Read product output flist after purchase deal is " , read_oflist);
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS for reading product object ", ebufp);
							PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
						}

						priority = (pin_decimal_t *)PIN_FLIST_FLD_GET(read_oflist,PIN_FLD_PRIORITY , 0, ebufp);

						if(pbo_decimal_compare(priority,hundred,ebufp)== -1)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " this is a cancel of Hardware product");

						}
						else
						{
							status_flistp = PIN_FLIST_ELEM_ADD(provaction_inflistp, PIN_FLD_PRODUCTS, cnt++, ebufp );
							PIN_FLIST_FLD_COPY(products_flistp, PIN_FLD_OFFERING_OBJ,status_flistp, PIN_FLD_POID, ebufp);
							flags = 2;
							PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS_FLAGS, &flags, ebufp);
						}
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in comparision ", ebufp);
							PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
						}
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan status change output flist", statuschange_outflistp);
						PIN_FLIST_DESTROY_EX(&statuschange_outflistp, NULL);	
					}
				}
				PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_oflist, NULL);
				PIN_FLIST_DESTROY_EX(&read_iflist, NULL);

			}
		}

	}
	//PIN_FLIST_DESTROY_EX(&copy_in_flistp, NULL);
	/*Validation rule to be applied after puchase deal
	fm_mso_validation_rule_1(ctxp, in_flist, MSO_OP_CUST_CHANGE_PLAN, &r_flistp, ebufp);
	if (r_flistp)
	{
		*ret_flistp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp",*ret_flistp );
		return 0;
	}*/
	if (flag == PCM_OPFLG_CALC_ONLY )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price calc mode so no provisioning");
	        PIN_FLIST_DESTROY_EX(&copy_in_flistp, NULL);
		return 2;	
	}

	if(!pbo_decimal_is_null(hundred,ebufp))
	{
		pbo_decimal_destroy(&hundred);
	}
	/* Write USERID, PROGRAM_NAME in buffer - start */
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
	/* Write USERID, PROGRAM_NAME in buffer - end */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan provisioning input list", provaction_inflistp);
	count = PIN_FLIST_ELEM_COUNT(provaction_inflistp, PIN_FLD_PRODUCTS , ebufp);
	if (count == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No Products hence no provisioning action ");
	}
	else
	{
		comp_flag = fm_mso_compare_ca_ids(ctxp, in_flist, &provaction_inflistp, ebufp);
		if (!comp_flag){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Call Provisionnning Opcoode ");
			PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
		}
	}
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
		errorCode = ebufp->pin_err;
		field_no = ebufp->field;
		memset(errorMsg,'\0',strlen(errorMsg));
		if ( errorCode == PIN_ERR_NOT_FOUND && field_no == MSO_FLD_CA_ID )
		{
			strcpy(errorMsg,"MSO_FLD_CA_ID is Invalid or NULL");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errorMsg, ebufp);
			PIN_ERRBUF_RESET(ebufp);

			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - MSO_OP_PROV_CREATE_ACTION error CAS_SUBSCRIBER_ID is Invalid or NULL", ebufp);
		}
		else if ( errorCode == PIN_ERR_BAD_VALUE && field_no == MSO_FLD_STB_ID )
		{
			strcpy(errorMsg,"MSO_FLD_STB_ID is Invalid or NULL");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errorMsg, ebufp);
			PIN_ERRBUF_RESET(ebufp);

			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - MSO_OP_PROV_CREATE_ACTION error MSO_FLD_STB_ID is Invalid or NULL", ebufp);
		}
		else if ( errorCode == PIN_ERR_BAD_VALUE && field_no == MSO_FLD_VC_ID )
		{
			strcpy(errorMsg,"MSO_FLD_VC_ID is Invalid or NULL");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errorMsg, ebufp);
			PIN_ERRBUF_RESET(ebufp);

			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - MSO_OP_PROV_CREATE_ACTION error MSO_FLD_VC_ID is Invalid or NULL", ebufp);
		}
		else if ( errorCode == PIN_ERR_BAD_VALUE && field_no == MSO_FLD_BOUQUET_ID )
		{
			strcpy(errorMsg,"MSO_FLD_BOUQUET_ID is Invalid or NULL");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errorMsg, ebufp);
			PIN_ERRBUF_RESET(ebufp);

			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - MSO_OP_PROV_CREATE_ACTION error MSO_FLD_BOUQUET_ID is Invalid or NULL", ebufp);
		}
		else if ( errorCode == PIN_ERR_BAD_VALUE && field_no == MSO_FLD_NETWORK_NODE )
		{
			strcpy(errorMsg,"MSO_FLD_NETWORK_NODE is Invalid or NULL");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errorMsg, ebufp);
			PIN_ERRBUF_CLEAR(ebufp);

			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - MSO_OP_PROV_CREATE_ACTION error MSO_FLD_NETWORK_NODE is Invalid or NULL", ebufp);
		}
		else if ( errorCode == PIN_ERR_BAD_VALUE && field_no == MSO_FLD_CAS_SUBSCRIBER_ID )
		{
			strcpy(errorMsg,"MSO_FLD_CAS_SUBSCRIBER_ID is Invalid or NULL");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, errorMsg, ebufp);
			PIN_ERRBUF_RESET(ebufp);

			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service cancel plan - MSO_OP_PROV_CREATE_ACTION error MSO_FLD_CAS_SUBSCRIBER_ID is Invalid or NULL", ebufp);
		}		
		else
		{
			PIN_ERRBUF_CLEAR(ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);
		}

		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
		*ret_flistp = r_flistp;
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

	/******************************************* notification flist ***********************************************/
	/*	flags = 11;
		provaction_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &flags, ebufp);

		PIN_FLIST_CONCAT(provaction_inflistp, plan_flistp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan provisioning input list", provaction_inflistp);
		PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - MSO_OP_NTF_CREATE_NOTIFICATION error", ebufp);

	 *ret_flistp = r_flistp;
	 return 0;
	 }
	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan provisioning output flist", provaction_outflistp);
	 PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
	 */
	PIN_FLIST_DESTROY_EX(&plan_flistp, NULL);

	/*******************************************************************
	 * Create Output flist - Start
	 *******************************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_success, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "ACCOUNT - Service change plan completed successfully", ebufp);

	*ret_flistp = r_flistp;

	/*******************************************************************
	 * Create Output flist - Start
	 ********************************************************************/
        //New Tariff - Transaction mapping Start*/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Copied input flist ", copy_in_flistp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "lifecycle_flistp event",lifecycle_flistp);
        fm_mso_create_lifecycle_evt(ctxp, copy_in_flistp, lifecycle_flistp, ID_CHANGE_PLAN, ebufp );
        //New Tariff - Transaction mapping End*/
        PIN_FLIST_DESTROY_EX(&copy_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&lifecycle_flistp, NULL);
	return 2;
}
static int
fm_mso_validate_base_change_rule(
		pcm_context_t		*ctxp,
		pin_flist_t			*in_flist,
		pin_flist_t			**ret_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*planlists_flistp = NULL;
	pin_flist_t		*dealinfo_flistp = NULL;
	pin_flist_t		*deal_flistp = NULL;
	pin_flist_t		*srch_in_flist = NULL;
	pin_flist_t		*srch_out_flist = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*prd_flistp = NULL;
	pin_flist_t 	*ser_in_flistp = NULL;
	pin_flist_t 	*ser_out_flistp = NULL;
	pin_flist_t 	*services_flistp = NULL;
	pin_flist_t 	*catv_info_flistp = NULL;
	pin_flist_t	*pp_in_flistp  = NULL;
	pin_flist_t     *pp_out_flistp  = NULL;

	pin_cookie_t    cookie_plan = NULL;
	pin_cookie_t	cookie_deal = NULL;
	pin_cookie_t	cookie_res  = NULL;

	int32			*action_flags = NULL;
	int32			elem_plan = 0;
	int32			elem_deal = 0;
	int32			srch_flag = 256;
	int32			change_plan_success = 0;
	int32			change_plan_failure = 1;
	int32			basic_plan_found = 0;
	int32			new_basic_plan_count = 0;
	int32 		con_type = 0;
	int32		service_active = 10100;
	int32		primary_service = 0;
	int32		secondary_service = 1;
	int32		service_type = 0;
	int64			db = 0;
	int32		ctr = 0;
	int32			prod_active = 1;
	int32		res_plan = 0;
	pin_decimal_t   *priority_hundred = pbo_decimal_from_str("100.0",ebufp);
	pin_decimal_t   *priority_onethirty = pbo_decimal_from_str("130.0",ebufp);
	pin_decimal_t	*priority = NULL;
	pin_decimal_t	*basic_plan_priority = NULL;
	pin_decimal_t	*new_basic_plan_priority = NULL;
	pin_decimal_t	*ser_priority = NULL;
	poid_t			*service_obj = NULL;
	poid_t			*deal_pdp = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*plan_obj = NULL;	
	poid_t			*ser_pdp = NULL;
	char			*template = "select X from /product 1,/deal 2 where 2.F1 = V1 and 2.F2 = 1.F3 ";
	char		*template_plan = "select X from /product 1, /deal 2, /plan 3 where 3.F1 = V1 and 3.F2 = 2.F3 and 2.F4 = 1.F5 ";
	char 		*template_ser = "select X from /service where F1 = V1 and F2 = V2 ";
	char		*template_pur_prod = "select X from /product 1, /purchased_product 2 where 2.F1 = V1 and 2.F2 = V2 and 2.F3 = 1.F4 and 1.F5 >= V5 and 1.F6 <= V6 ";

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_base_change_rule input flist", in_flist);	
	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	acnt_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );
	db = PIN_POID_GET_DB(service_obj);
	while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, 
					&elem_plan, 1, &cookie_plan, ebufp )) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_base_change_rule plan flist", planlists_flistp);
		action_flags = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_FLAGS, 1, ebufp );
		if (*(int*)action_flags == 0)
		{
			plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
			if (!service_obj || !plan_obj)
			{
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - plan object is missing in input", ebufp);
				*ret_flistp = r_flistp;
				return 0;
			}

			/**********************************************************************
			 * Get priority of all the products of deals that need to be cancelled 
			 ***********************************************************************/
			elem_deal = 0;
			cookie_deal = NULL;
			while((deal_flistp = PIN_FLIST_ELEM_GET_NEXT(planlists_flistp, PIN_FLD_DEALS,
							&elem_deal, 1, &cookie_deal, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_base_change_rule deal flist", deal_flistp);
				if(!basic_plan_found)
				{
					deal_pdp = PIN_FLIST_FLD_GET(deal_flistp,PIN_FLD_DEAL_OBJ,0,ebufp);
					srch_in_flist =  PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_PUT(srch_in_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);
					PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_TEMPLATE, template, ebufp);		
					tmp_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 1, ebufp);
					PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, deal_pdp, ebufp);
					tmp_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 2, ebufp);
					prd_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
					PIN_FLIST_FLD_SET(prd_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
					tmp_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 3, ebufp);
					PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);
					tmp_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_RESULTS, 0, ebufp);
					PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_base_change_rule Product search input flist", srch_in_flist);
					PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flist, &srch_out_flist, ebufp);
					PIN_FLIST_DESTROY_EX(&srch_in_flist, NULL);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_base_change_rule Product search output flist", srch_out_flist);
					result_flistp = PIN_FLIST_ELEM_GET(srch_out_flist,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
					if (!result_flistp )
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan Not configured correctly", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						r_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Plan Not configured correctly", ebufp);
						*ret_flistp = r_flistp;
			                        PIN_FLIST_DESTROY_EX(&srch_out_flist, NULL);
						return 0;
					}
					cookie_res = NULL;
					res_plan = 0;	
					while ((result_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flist, PIN_FLD_RESULTS,
									&res_plan, 1, &cookie_res, ebufp )) != (pin_flist_t *)NULL)
					{
						priority = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_PRIORITY,0,ebufp);
						if(pbo_decimal_compare(priority,priority_hundred,ebufp)>=0 &&
								pbo_decimal_compare(priority,priority_onethirty,ebufp)<=0)
						{	
							/* Store the basic plan priority */
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Basic plan change scenario");
							basic_plan_priority = pbo_decimal_copy(priority,ebufp);
							basic_plan_found = 1;
							break;
						}
					}
				}
			}
			PIN_FLIST_DESTROY_EX(&srch_out_flist, NULL);
		}
	}

	/**********************************************************************
	 * If Basic Plan is changed then apply the validation rules 
	 ***********************************************************************/
	if(basic_plan_found)
	{
		planlists_flistp = NULL;
		elem_plan = 0;
		cookie_plan = NULL;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Apply validation rule on new plan.");
		while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, 
						&elem_plan, 1, &cookie_plan, ebufp )) != (pin_flist_t *)NULL)
		{
			action_flags = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_FLAGS, 1, ebufp );
			if (*(int*)action_flags == 1)
			{
				plan_obj = PIN_FLIST_FLD_GET(planlists_flistp,PIN_FLD_PLAN_OBJ,0,ebufp);
				srch_in_flist =  PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(srch_in_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);
				PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_TEMPLATE, template_plan, ebufp);		
				tmp_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 1, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, plan_obj, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 2, ebufp);
				prd_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_SERVICES, 0, ebufp);
				PIN_FLIST_FLD_SET(prd_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 3, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 4, ebufp);
				prd_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
				PIN_FLIST_FLD_SET(prd_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 5, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);					
				tmp_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_RESULTS, 0, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_base_change_rule New Product search input flist", srch_in_flist);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flist, &srch_out_flist, ebufp);
				PIN_FLIST_DESTROY_EX(&srch_in_flist, NULL);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_base_change_rule New Product search output flist", srch_out_flist);
				result_flistp = PIN_FLIST_ELEM_GET(srch_out_flist,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
				if (!result_flistp )
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan Not configured correctly", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Plan Not configured correctly", ebufp);
					*ret_flistp = r_flistp;
                                        PIN_FLIST_DESTROY_EX(&srch_out_flist,NULL);
					return 0;
				}
				cookie_res = NULL;
				res_plan = 0;
				while ((result_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flist, PIN_FLD_RESULTS,
								&res_plan, 1, &cookie_res, ebufp )) != (pin_flist_t *)NULL)
				{	
					priority = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_PRIORITY,0,ebufp);
					if(pbo_decimal_compare(priority,priority_hundred,ebufp)>=0 &&
							pbo_decimal_compare(priority,priority_onethirty,ebufp)<=0)
					{		
						/* This is basic plan to be purchased. Increase New basic plan count */
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "New Basic plan found");
						new_basic_plan_priority = pbo_decimal_copy(priority,ebufp);
						new_basic_plan_count = new_basic_plan_count + 1;
						break;
					}
				}
				//if(srch_out_flist)
				PIN_FLIST_DESTROY_EX(&srch_out_flist,NULL);	
			} 
		} // while planlist
		if(new_basic_plan_count == 0)
		{
			/* No New basic plan found. Validation fail */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Change basic  package with non-basic package.");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Changing Basic package with Non basic package is Not Allowed", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Changing Basic package with Non basic package is Not Allowed", ebufp);
			*ret_flistp = r_flistp;
			return 0;
		}
		else if(new_basic_plan_count == 2)
		{
			/* More than one basic plan not allowed. Validation fail */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Change basic  package with more than 1 basic package.");
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "More than one basic package offer not allowed", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "More than one basic package offer not allowed", ebufp);
			*ret_flistp = r_flistp;
			return 0;
		}
		else if(new_basic_plan_count == 1)
		{
			/* Only one basic plan. Check priority of new plan */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Change basic package with 1 basic package");

			/* Fetch all services for this account */
			ser_in_flistp =  PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_PUT(ser_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(ser_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(ser_in_flistp, PIN_FLD_TEMPLATE, template_ser, ebufp);		
			tmp_flistp = PIN_FLIST_ELEM_ADD(ser_in_flistp, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp );
			tmp_flistp = PIN_FLIST_ELEM_ADD(ser_in_flistp, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &service_active, ebufp );			
			tmp_flistp = PIN_FLIST_ELEM_ADD(ser_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search input flist", ser_in_flistp);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, ser_in_flistp, &ser_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&ser_in_flistp, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search output flist", ser_out_flistp);

			/* Loop through each service */
			elem_deal = 0;
			cookie_deal = NULL;
			services_flistp =  PIN_FLIST_CREATE(ebufp);
			while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(ser_out_flistp, PIN_FLD_RESULTS,
							&elem_deal, 1, &cookie_deal, ebufp)) != (pin_flist_t *)NULL)
			{
				catv_info_flistp = PIN_FLIST_SUBSTR_GET(result_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CATV Info flist", catv_info_flistp);
				if(catv_info_flistp)
				{
					con_type = *((int32 *)PIN_FLIST_FLD_GET(catv_info_flistp,PIN_FLD_CONN_TYPE,0,ebufp));
				}

				pp_in_flistp =  PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(pp_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(pp_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
				PIN_FLIST_FLD_SET(pp_in_flistp, PIN_FLD_TEMPLATE, template_pur_prod, ebufp);		
				tmp_flistp = PIN_FLIST_ELEM_ADD(pp_in_flistp, PIN_FLD_ARGS, 1, ebufp);
				PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, tmp_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
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

				/* Store the Base product poid and its priority in Services Flist */
				if(PIN_FLIST_ELEM_COUNT(pp_out_flistp,PIN_FLD_RESULTS,ebufp) > 0 )
				{ 
					ser_pdp = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_POID,0,ebufp);
					if(PIN_POID_COMPARE(service_obj,ser_pdp,0,ebufp) == 0)
					{       /* The service in change plan match this service */
						if(con_type == primary_service)
						{
							service_type = primary_service;
						}
						else if(con_type == secondary_service)
						{
							service_type = secondary_service;

						}
					}
					PIN_FLIST_ELEM_COPY(pp_out_flistp, PIN_FLD_RESULTS, 0, services_flistp, PIN_FLD_RESULTS, ctr, ebufp);
					tmp_flistp = PIN_FLIST_ELEM_GET(services_flistp, PIN_FLD_RESULTS, ctr,0, ebufp);
					PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, tmp_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
					PIN_FLIST_FLD_COPY(catv_info_flistp, PIN_FLD_CONN_TYPE, tmp_flistp, PIN_FLD_CONN_TYPE, ebufp );
					ctr++;
				}
				//if(pp_out_flistp != NULL )
				PIN_FLIST_DESTROY_EX(&pp_out_flistp, NULL);
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Services and its base product priority flist", services_flistp);
			/*Fix for picking the Priority from Main connection */

			elem_deal = 0;
			cookie_deal = NULL;
			while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(services_flistp, PIN_FLD_RESULTS,
							&elem_deal, 1, &cookie_deal, ebufp)) != (pin_flist_t *)NULL)
			{
				con_type = *((int32 *)PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_CONN_TYPE,0,ebufp));
				if(con_type == primary_service)
				{
					priority = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_PRIORITY,0,ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Services and its base product priority flist", services_flistp);
				}
			}
			/*Fix for picking the priority from main connection Ends */
			elem_deal = 0;
			cookie_deal = NULL;
			while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(services_flistp, PIN_FLD_RESULTS,
							&elem_deal, 1, &cookie_deal, ebufp)) != (pin_flist_t *)NULL)
			{
				ser_pdp = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_SERVICE_OBJ,0,ebufp);
				priority = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_PRIORITY,0,ebufp);
				if(PIN_POID_COMPARE(service_obj,ser_pdp,0,ebufp) != 0)
				{
					/*validation code has been commented as confirmed by praful
					  if(service_type == primary_service)
					  {
					 * Primary service is cancelled therefore new base plan priority
					 * must be greater than existing secondary service plan *
					 if(pbo_decimal_compare(new_basic_plan_priority,priority,ebufp)<0)
					 {
					 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Primary base plan priority is less than secondary base plan.");
					 r_flistp = PIN_FLIST_CREATE(ebufp);
					 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
					 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					 PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Primary base plan priority is less than secondary base plan", ebufp);
					 *ret_flistp = r_flistp; 
					 return 0;
					 }
					 }
					 else*/
					if(service_type == secondary_service)
					{
						/* Secondary service is cancelled therefore new base plan priority
						 * must be less than Primary service plan */
						if(pbo_decimal_compare(new_basic_plan_priority,priority,ebufp)>0)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Secondary base plan priority is more than Primary base plan.");
							r_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Secondary base plan priority is more than Primary base plan", ebufp);
							*ret_flistp = r_flistp; 
						        PIN_FLIST_DESTROY_EX(&ser_out_flistp,NULL);
							return 0;
						}					
					}
				}


			}
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Change plan not allowed for Alacarte plans");
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Change plan not allowed for Alacarte plans", ebufp);
                *ret_flistp = r_flistp;
                PIN_FLIST_DESTROY_EX(&ser_out_flistp,NULL);
                return 0;
	}
	/*	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "End of fm_mso_validate_base_change_rule.");
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "END of fm_mso_validate_base_change_rule", ebufp);
	 *ret_flistp = r_flistp; 
	 PIN_FLIST_DESTROY_EX(&pp_out_flistp, NULL);
	 PIN_FLIST_DESTROY_EX(&srch_out_flist,NULL);	
	 */

	//if(ser_out_flistp)
	PIN_FLIST_DESTROY_EX(&ser_out_flistp,NULL);	
	return 1;
}


/**************************************************
 * Function: fm_mso_bb_change_plan()
 * 
 * 
 ***************************************************/
static void 
fm_mso_bb_change_plan(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flist,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*planlists_flistp = NULL;
	pin_flist_t		*canceldeal_inflistp = NULL;
	pin_flist_t		*canceldeal_outflistp = NULL;
	pin_flist_t		*dealinfo_flistp = NULL;
	pin_flist_t		*deals_flistp = NULL;
	pin_flist_t		*readplan_inflistp = NULL;
	pin_flist_t		*readplan_outflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*tmp_res_flistp = NULL;
	pin_flist_t		*pp_flistp = NULL;
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*prov_out_flistp = NULL;
	pin_flist_t		*old_mso_pp_flistp = NULL;
	pin_flist_t		*bb_flistp = NULL;
	pin_flist_t		*read_in_flistp = NULL;
	pin_flist_t		*read_out_flistp = NULL;
	pin_flist_t		*ncr_validity_iflist = NULL;
	pin_flist_t		*ncr_validity_oflist = NULL;
	pin_flist_t		*sort_fld_flistp =  NULL;
	pin_flist_t		*fup_ret_flistp =  NULL;
	pin_flist_t		*pl_flistp = NULL;
	pin_flist_t		*write_in_flistp = NULL;
	pin_flist_t		*write_out_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	pin_flist_t		*old_mso_plan_flistp = NULL;
	pin_flist_t		*extn_flistp = NULL;
	pin_flist_t		*sub_bals_flistp = NULL;
	pin_flist_t		*mso_plan_flistp = NULL;
	pin_flist_t		*mso_res_flistp = NULL;
	pin_flist_t		*mso_prod_flistp = NULL;
	pin_flist_t		*charge_oflistp = NULL;
	pin_flist_t		*rate_oflistp = NULL;
	pin_flist_t		*result_flistp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*read_deal_obj = NULL;
	poid_t			*get_deal_obj = NULL;
	poid_t			*acc_pdp = NULL;
	poid_t			*mso_pp_obj = NULL;
	poid_t			*new_mso_pp_obj = NULL;
	poid_t			*mso_srvc_obj = NULL;
	poid_t			*cancel_offer_obj = NULL;
	poid_t			*cancel_prod_obj = NULL;

	int			elem_id = 0;
	int			elem_base = 0;
	int			melemid = 0;
	int			pelemid = 0;
	int			*package_id = 0;
	int			*prov_call = 0;
	int			status_flags = 0;
	int			status_flags_after_expiry = DOC_BB_RENEW_AEXPIRY_PRE;
	int			change_plan_success = 0;
	int			change_plan_failure = 1;
	int			ether_prov_flag = 0;
	int			*ser_tech = NULL;
	int			subs_type = 1;
	pin_cookie_t		cookie_base = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie_countdeals = NULL;
	pin_cookie_t		mcookie = NULL;
	pin_cookie_t		pcookie = NULL;
	int			*result = NULL;
	int			elem_countdeals = 0;
	int			*action_flags = NULL;
	int			ret_val = 0;
	int			price_calc_flag = 0;
	int32			*ret_status = NULL;
	pin_decimal_t		*hundred = pbo_decimal_from_str("90.0",ebufp);
	pin_decimal_t		*priority      = NULL;
	char			*plan_name = NULL;
	char 			*plan_type = NULL;
	char			*descr = NULL;
	void			*vp = NULL;
	char			*cmts_id = NULL;
	char			*prog_name = NULL;
	char		        *str_rolep = NULL;
	int			*istal = NULL;
	int32			*indicator = NULL;
	int32			*bill_when = NULL;
	char			*city = NULL;

	/*********** Pavan Bellala : 15-07-2015 Changes made to change plan after expiry ********/
	int			*svc_statusp  = NULL;
	int			*prov_statusp = NULL;
	pin_flist_t		*t_flistp = NULL;
	pin_flist_t		*sv_flistp = NULL;
	pin_flist_t		*o_flistp = NULL;
	pin_flist_t		*a_flistp = NULL;
	int			s_status = 0;
	int			status_flag = 0;
	int			plan_change_after_expiry_scn = 0;
	/*********** Pavan Bellala : 15-07-2015 Changes made to change plan after expiry ********/

	// Added below for AMC
	int			old_plan_type=-1;
	int			new_plan_type=-1;

	//Added below for Static IP change 
	pin_flist_t		*network_flistp = NULL;

	poid_t			*base_plan_obj = NULL;
	poid_t			*cancel_plan_obj = NULL;

	int			update_mso_purch_plan = 0;

	int                     elem_id_r = 0;
	pin_cookie_t            cookie_r = NULL;
	pin_flist_t     	*bal_imp = NULL;
	int32           	*bal_res = NULL;
	int             	*tech = NULL;
	int             	plan_typee = 0;
	int                     opcode_num = MSO_OP_CUST_CHANGE_PLAN;
	pin_decimal_t   	*initial_amount = NULL;
	int32               	error_codep = 0;
	pin_flist_t     	*read_mso_plan_oflistp = NULL;
	pin_flist_t     	*read_mso_plan_flistp = NULL;
	pin_flist_t     	*prod_flistp = NULL;
	int             	rec_id_p = 0;
	pin_cookie_t    	cookie_p = NULL;
	pin_flist_t     	*rfld_iflistp = NULL;
	pin_flist_t     	*rfld_oflistp = NULL;
	pin_flist_t     	*temp_flistp = NULL;
	char                    *prov_tag = NULL ;
	char                    *m_prov_tag = NULL ;
	char                    *base = NULL ;
	int32			*extn_status = NULL;
	time_t			purch_end_t = 0;
	time_t			old_purch_end_t = 0;
	time_t			*valid_to = NULL;
	//Added for Refund price calc changes
	pin_flist_t		*refund_calc_flistp = NULL;
	pin_flist_t		*ref_bal_flistp = NULL;
	pin_flist_t		*ref_flistp = NULL;
	pin_flist_t		*sub_bal_flist = NULL;
	pin_flist_t		*search_outflistp = NULL;
	pin_decimal_t		*temp_amount = NULL;
	pin_decimal_t		*total_charge = NULL;
	pin_decimal_t		*charge = NULL;
	pin_decimal_t		*tax = NULL;
	pin_decimal_t		*zero = pbo_decimal_from_str("0.0", ebufp);
	int32   		INR = 356;
	int32   		resource_id = 0;
	char            	*rate_tag = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error before calling fm_mso_bb_change_plan :");
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan :", in_flist);

	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	acc_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );
	planlists_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_PLAN_LISTS, 0, 1, ebufp );

	if (!planlists_flistp || !service_obj)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51752", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - Service object or plan object is missing in input", ebufp);

		*ret_flistp = r_flistp;
		return;
	}

	mso_get_additional_products(ctxp,in_flist, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan : modified ", in_flist);

	/* validate input fields in flist */
	fm_mso_validate_bb_change_plan(ctxp, in_flist, ret_flistp, ebufp );
	if(*ret_flistp)
	{
		result = PIN_FLIST_FLD_GET(*ret_flistp, PIN_FLD_STATUS, 1,ebufp);
		if(result && (*result == change_plan_failure))
		{
			return;
		}
	}

	/* validate plan compatibility with customer type *
	   fm_mso_validate_bb_plan_custtype(ctxp, in_flist, ret_flistp, &ether_prov_flag, &old_plan_type, &new_plan_type, ebufp );
	   if(*ret_flistp)
	   {
	   result = PIN_FLIST_FLD_GET(*ret_flistp, PIN_FLD_STATUS, 1,ebufp);
	   if(result && (*result == change_plan_failure))
	   {
	   return;
	   }
	   }	*/

	// Expire existing account level offers during change plan. Fixing defect resulting in offers getting renewed during top-up even though change plan is done in the original plan.
	fm_mso_cust_expire_offers(ctxp,in_flist,&search_outflistp,ebufp);

	/* Fetch the value of CALC ONLY flag*/
	vp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_MODE, 1, ebufp );
	if(vp && *(int32 *)vp == 1) {
		price_calc_flag = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
	}
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, service_obj, ebufp );

	/*********** Pavan Bellala : 15-07-2015 Changes made to change plan after expiry ********/

	PIN_FLIST_FLD_SET(read_in_flistp,PIN_FLD_STATUS,NULL,ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp,PIN_FLD_ACCOUNT_OBJ,NULL,ebufp);
	t_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp,PIN_FLD_TELCO_FEATURES,PIN_ELEMID_ANY,ebufp);
	PIN_FLIST_FLD_SET(t_flistp,PIN_FLD_STATUS,NULL,ebufp);

	/*********** Pavan Bellala : 15-07-2015 Changes made to change plan after expiry ********/

	bb_flistp = PIN_FLIST_SUBSTR_ADD(read_in_flistp, MSO_FLD_BB_INFO, ebufp );
	PIN_FLIST_FLD_SET(bb_flistp, MSO_FLD_TECHNOLOGY, (int *)NULL, ebufp );
	PIN_FLIST_FLD_SET(bb_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp );
	PIN_FLIST_FLD_SET(bb_flistp, MSO_FLD_ISTAL_FLAG, NULL, ebufp );
	PIN_FLIST_FLD_SET(bb_flistp, MSO_FLD_NETWORK_ELEMENT, NULL, ebufp );
	PIN_FLIST_FLD_SET(bb_flistp, PIN_FLD_INDICATOR, NULL, ebufp );
	PIN_FLIST_FLD_SET(bb_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp );
	PIN_FLIST_FLD_SET(bb_flistp, PIN_FLD_CITY, NULL, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan read service input", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51754", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service change plan - service not found ", ebufp);
		*ret_flistp = r_flistp;
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan read service output", read_out_flistp);
	bb_flistp = PIN_FLIST_SUBSTR_GET(read_out_flistp, MSO_FLD_BB_INFO,1, ebufp );
	ser_tech = PIN_FLIST_FLD_GET(bb_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp );
	istal = PIN_FLIST_FLD_TAKE(bb_flistp, MSO_FLD_ISTAL_FLAG, 1, ebufp );
    str_rolep = PIN_FLIST_FLD_TAKE(bb_flistp, MSO_FLD_ROLES, 1, ebufp);
	cmts_id = PIN_FLIST_FLD_TAKE(bb_flistp, MSO_FLD_NETWORK_ELEMENT, 1, ebufp );
	indicator = PIN_FLIST_FLD_GET(bb_flistp, PIN_FLD_INDICATOR, 1, ebufp );
	bill_when = PIN_FLIST_FLD_GET(bb_flistp, PIN_FLD_BILL_WHEN, 1, ebufp );
	city = PIN_FLIST_FLD_GET(bb_flistp, PIN_FLD_CITY, 1, ebufp );
	PIN_FLIST_FLD_COPY(bb_flistp, MSO_FLD_AGREEMENT_NO, in_flist, MSO_FLD_AGREEMENT_NO, ebufp );
	PIN_FLIST_FLD_COPY(bb_flistp, PIN_FLD_CITY, in_flist, PIN_FLD_CITY, ebufp );
	PIN_FLIST_FLD_COPY(bb_flistp, PIN_FLD_BILL_WHEN, in_flist, MSO_FLD_OLD_VALUE, ebufp ); //This is old plan term

	if(!ser_tech)
	{
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51759", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service change plan - Technology is NULL for service.", ebufp);
		*ret_flistp = r_flistp;
		return;
	}	
	t_flistp = PIN_FLIST_ELEM_GET(read_out_flistp,PIN_FLD_TELCO_FEATURES,PIN_ELEMID_ANY,1,ebufp);
	if (t_flistp && t_flistp != NULL ){
		prov_statusp = PIN_FLIST_FLD_GET(t_flistp,PIN_FLD_STATUS,1,ebufp);
	}
	// Moving this section to get correct indicator	
	/* validate plan compatibility with customer type */
	fm_mso_validate_bb_plan_custtype(ctxp, in_flist, ret_flistp, &ether_prov_flag, &old_plan_type, &new_plan_type, ebufp );
	if(*ret_flistp)
	{
		result = PIN_FLIST_FLD_GET(*ret_flistp, PIN_FLD_STATUS, 1,ebufp);
		if(result && (*result == change_plan_failure))
		{
			return;
		}
	}

	/*********** Pavan Bellala : 15-07-2015 Changes made to change plan after expiry ********/
	svc_statusp = PIN_FLIST_FLD_GET(read_out_flistp,PIN_FLD_STATUS,1,ebufp);
	if(svc_statusp  && (*svc_statusp == PIN_STATUS_INACTIVE))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_change_plan:Service is inactive");
		//t_flistp = PIN_FLIST_ELEM_GET(read_out_flistp,PIN_FLD_TELCO_FEATURES,PIN_ELEMID_ANY,1,ebufp);
		if(t_flistp && t_flistp != NULL)
		{
			prov_statusp = PIN_FLIST_FLD_GET(t_flistp,PIN_FLD_STATUS,1,ebufp);
			if(prov_statusp && (*prov_statusp == MSO_PROV_DEACTIVE))	
			{ 
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_change_plan:Prov status is deactive");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_change_plan:Activating service");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"setting plan_change_after_expiry_scn");

				plan_change_after_expiry_scn = 1;

				a_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(read_out_flistp,PIN_FLD_ACCOUNT_OBJ,a_flistp,PIN_FLD_POID,ebufp);
				PIN_FLIST_FLD_SET(a_flistp,PIN_FLD_PROGRAM_NAME,"fm_mso_bb_change_plan:Activating service",ebufp);
				sv_flistp = PIN_FLIST_ELEM_ADD(a_flistp,PIN_FLD_SERVICES,0,ebufp);
				PIN_FLIST_FLD_COPY(read_out_flistp,PIN_FLD_POID,sv_flistp,PIN_FLD_POID,ebufp);
				s_status =  PIN_STATUS_ACTIVE;
				PIN_FLIST_FLD_SET(sv_flistp,PIN_FLD_STATUS,&s_status,ebufp);
				status_flag = 1;
				PIN_FLIST_FLD_SET(sv_flistp,PIN_FLD_STATUS_FLAGS,&status_flag,ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_change_plan:Call for PCM_OP_CUST_UPDATE_SERVICES in",a_flistp);
				PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, a_flistp, &o_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_change_plan:Call for PCM_OP_CUST_UPDATE_SERVICES out",o_flistp);

				PIN_FLIST_DESTROY_EX(&a_flistp,NULL);
				PIN_FLIST_DESTROY_EX(&o_flistp,NULL);
			}
		}
	}

	/*********** Pavan Bellala : 15-07-2015 Changes made to change plan after expiry ********/


	/* Pawan:03-03-2015 Added below to sort input flist on flag value */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist Before Sort", in_flist);
	sort_fld_flistp = PIN_FLIST_CREATE(ebufp);
	pl_flistp = PIN_FLIST_ELEM_ADD(sort_fld_flistp,PIN_FLD_PLAN_LISTS, 0, ebufp );
	PIN_FLIST_FLD_SET(pl_flistp, PIN_FLD_FLAGS, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Sort Fld flist", sort_fld_flistp);
	PIN_FLIST_SORT(in_flist,sort_fld_flistp,1,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist After Sort", in_flist);

	while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, &elem_base, 1, &cookie_base, ebufp )) != NULL)
	{
		action_flags = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_FLAGS, 1, ebufp );
		if (*(int*)action_flags == 0 )
		{

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Action flag is 0");
			// Below is to give the disconnection credit (refund) for PREPAID plans during change plan only
			//	Get the actual base offer_obj getting cancelled and product 
			if ((indicator && *indicator == 2) && (prov_statusp && *prov_statusp == MSO_PROV_ACTIVE)){
				cancel_plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );

				fm_mso_get_mso_purplan(ctxp,acc_pdp, cancel_plan_obj,MSO_PROV_ACTIVE,&mso_plan_flistp,ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_FLIST_DESTROY_EX(&mso_plan_flistp, NULL);
					return;
				}
				//	if(mso_plan_flistp && PIN_FLIST_ELEM_COUNT(mso_plan_flistp, PIN_FLD_RESULTS, ebufp) > 0){
				//		while((mso_res_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_plan_flistp, 
				//				PIN_FLD_RESULTS, &melemid, 1, &mcookie, ebufp )) != NULL)
				//		{
                                pelemid = 0;
                                pcookie = NULL;
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_plan_flistp");
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_plan_flistp", mso_plan_flistp);
				if (mso_plan_flistp == NULL)
				{
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51769", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service change plan - Invalid Active plan passed", ebufp);
					*ret_flistp = r_flistp;
					return;
				}
				mso_srvc_obj = PIN_FLIST_FLD_GET(mso_plan_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
				base = PIN_FLIST_FLD_GET(mso_plan_flistp, PIN_FLD_DESCR, 1, ebufp);
				if ((mso_srvc_obj && service_obj && PIN_POID_COMPARE(mso_srvc_obj, service_obj, 0 ,ebufp) == 0 )
						&& (base && strcmp(base, MSO_BB_BP) == 0 )){
					while((mso_prod_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_plan_flistp, 
									PIN_FLD_PRODUCTS, &pelemid, 1, &pcookie, ebufp )) != NULL){
						m_prov_tag = PIN_FLIST_FLD_GET(mso_prod_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
						if (m_prov_tag  && strcmp(m_prov_tag, "") != 0){
							cancel_offer_obj = PIN_FLIST_FLD_GET(mso_prod_flistp, 
									MSO_FLD_PURCHASED_PRODUCT_OBJ, 1, ebufp);
							cancel_prod_obj = PIN_FLIST_FLD_GET(mso_prod_flistp, 
									PIN_FLD_PRODUCT_OBJ, 1, ebufp);
						}
					}
				}
				//	}
				//}
			}
			/**
			  plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
			  if(!PIN_POID_IS_NULL(plan_obj))
			  {
			  base_plan_obj = PIN_POID_COPY(plan_obj,ebufp);
			  }
			 **/
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Planlist flist", planlists_flistp);
			elem_countdeals = 0;
			cookie_countdeals = NULL;
			update_mso_purch_plan = 0;
			//This is to generate disconnect charge for prepaid change plan and just call without calc flag
			//if ((indicator && *indicator == 2) && cancel_offer_obj && cancel_prod_obj && price_calc_flag == 0)
			if((indicator && *indicator == 2) && cancel_offer_obj && cancel_prod_obj)
			{
				fm_mso_generate_credit_charge(ctxp, in_flist, cancel_offer_obj, cancel_prod_obj, &charge_oflistp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"fm_mso_generate_credit_charge return flist");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan prepaid refund flist", charge_oflistp);
				//PIN_ERR_LOG_ERROR(PIN_ERR_LEVEL_ERROR, "Error in while proividing for disconnection credit1", ebufp);
				if  (PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in while proividing for disconnection credit", ebufp);
					PIN_FLIST_DESTROY_EX(&mso_plan_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&charge_oflistp, NULL);
					PIN_ERRBUF_RESET(ebufp);
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51770", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service change plan - Error while disconnection credit for prepaid", ebufp);
					*ret_flistp = r_flistp;
					return;
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Before scanning charge flist");
				if(charge_oflistp && charge_oflistp != NULL){
					result_flistp = PIN_FLIST_ELEM_GET(charge_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
					if(result_flistp && result_flistp != NULL){
						//This is added to add the lifecycle in case prepaid dc
						PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_CYCLE_INFO, in_flist, PIN_FLD_CYCLE_INFO, ebufp);		
						//Added block to capture the Refund amount for price calculator
						if(price_calc_flag == 1){
							refund_calc_flistp = PIN_FLIST_CREATE(ebufp);
							melemid = 0;
							mcookie = NULL;
							PIN_FLIST_FLD_COPY(charge_oflistp, PIN_FLD_POID, refund_calc_flistp, PIN_FLD_POID, ebufp);
							while((ref_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp,
											PIN_FLD_BAL_IMPACTS, &melemid, 1, &mcookie, ebufp ))!= NULL)
							{
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bal impact flist", ref_bal_flistp);
								resource_id = *(int32 *)PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_RESOURCE_ID, 0, ebufp);
								rate_tag = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_RATE_TAG, 1, ebufp);
								if(resource_id == INR) {	
									temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
									if(pbo_decimal_is_null(total_charge, ebufp)) {
										PIN_ERR_LOG_MSG(3,"Step1");
										total_charge = temp_amount;
									} else {

										total_charge = pbo_decimal_add(temp_amount,total_charge, ebufp);
										PIN_ERR_LOG_MSG(3,"Step4");
									}
									if(!(rate_tag && strcmp(rate_tag,"Tax") == 0))
									{
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering charge block");
										temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
										if(pbo_decimal_is_null(charge, ebufp)) 
										{
											charge = temp_amount;
											PIN_ERR_LOG_MSG(3,"Step2");
										} 
										else {
											charge = pbo_decimal_add(temp_amount,charge, ebufp);
											PIN_ERR_LOG_MSG(3,"Step5");
										}
									}
									else
									{
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering tax block");
										temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
										if(pbo_decimal_is_null(tax, ebufp))
										{
											tax = temp_amount;
											PIN_ERR_LOG_MSG(3,"Step3");
										}
										else {
											tax = pbo_decimal_add(temp_amount,tax, ebufp);
											PIN_ERR_LOG_MSG(3,"Step6");
										}
									}
								}
							}
							//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling price calc", ebufp);
							//Populating Refund details
							if(!pbo_decimal_is_null(total_charge, ebufp)) {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT, total_charge, ebufp);
							}
							else {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT, zero, ebufp);
							}
							if(!pbo_decimal_is_null(charge, ebufp)) {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_ORIG, charge, ebufp);
							}
							else {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_ORIG, zero, ebufp);
							}
							if(!pbo_decimal_is_null(tax, ebufp)) {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_TAXED, tax, ebufp);
							}
							else {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_TAXED, zero, ebufp);
							}
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Refund calculated flist", refund_calc_flistp);	

						}
					}
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"check2");
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"check3");
				//Now revert with actual flgas..This is being done to avaoid proration for cancel scenariod.
				PIN_FLIST_DESTROY_EX(&charge_oflistp, NULL);
				PIN_FLIST_DESTROY_EX(&mso_plan_flistp, NULL);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"check4");
			}
			while ((args_flistp = PIN_FLIST_ELEM_GET_NEXT(planlists_flistp, PIN_FLD_DEALS, 
							&elem_countdeals, 1, &cookie_countdeals, ebufp)) != NULL)
			{

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "args flist", args_flistp);
				get_deal_obj = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
				plan_obj = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
				if( PIN_POID_IS_NULL(plan_obj))
				{
					base_plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
					PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Log base poid",base_plan_obj);
					plan_obj = PIN_POID_COPY(base_plan_obj,ebufp);
					update_mso_purch_plan = 0;
				} else {
					/****** Pavan Bellala 23-11-2015 ****************************
					  This flag is required to update mso_purchased_plan status to 
					  MSO_PROV_TERMINATE as the plan is cancelled.These plan are handled
					  separately and not along with Base Plan. Hence this flag is required
					  to identify if update is required.
					 *************************************************************/
					update_mso_purch_plan = 1;
				}	
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "get_deal_obj poid:",get_deal_obj);
				if (!service_obj || !plan_obj)
				{
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51760", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, 
							"ACCOUNT - Service change plan - plan object is missing in input", ebufp);

					*ret_flistp = r_flistp;
					return;
				}
				/**
				  if(PIN_FLIST_ELEM_COUNT(planlists_flistp, PIN_FLD_DEALS , ebufp) == 0)
				  {
				  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Deal array missing in input.");
				  r_flistp = PIN_FLIST_CREATE(ebufp);
				  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				  PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				  PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51432", ebufp);
				  PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
				  "ACCOUNT - Change plan - Deal array missing in input.", ebufp);
				 *ret_flistp = r_flistp;
				 return;
				 }
				 ***/
				readplan_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_POID, plan_obj, ebufp );
				PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_NAME, NULL, ebufp );
				PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_CODE, NULL, ebufp );
				deals_flistp = PIN_FLIST_ELEM_ADD(readplan_inflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
				PIN_FLIST_FLD_SET(deals_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp );

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read plan input list", readplan_inflistp);
				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readplan_inflistp, &readplan_outflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&readplan_inflistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);

					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51761", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, 
							"ACCOUNT - Service change plan - plan poid not found", ebufp);

					*ret_flistp = r_flistp;
					return;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " read plan output flist", readplan_outflistp);
				PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_CODE, planlists_flistp, PIN_FLD_CODE, ebufp);

				/***************************************************************************
				 * Change plan Validations. 
				 ****************************************************************************/
				//args_flistp = PIN_FLIST_ELEM_ADD(plan_flistp, PIN_FLD_PLAN, cnt2++, ebufp);
				//PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_NAME, args_flistp, PIN_FLD_NAME, ebufp);

				elem_id = 0;
				cookie = NULL;
				while ((deals_flistp = PIN_FLIST_ELEM_GET_NEXT(readplan_outflistp, PIN_FLD_SERVICES, 
								&elem_id, 1, &cookie, ebufp)) != NULL)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals flist", deals_flistp);
					canceldeal_inflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, canceldeal_inflistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, canceldeal_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, canceldeal_inflistp, PIN_FLD_DESCR, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, canceldeal_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
					dealinfo_flistp = PIN_FLIST_ELEM_ADD(canceldeal_inflistp, PIN_FLD_DEAL_INFO, 0, ebufp );
					PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, dealinfo_flistp, PIN_FLD_DEAL_OBJ, ebufp);
					//PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_PACKAGE_ID, dealinfo_flistp, PIN_FLD_PACKAGE_ID, ebufp);

					read_deal_obj = PIN_FLIST_FLD_GET(deals_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
					//elem_countdeals = 0;
					//cookie_countdeals = NULL;
					//while ((args_flistp = PIN_FLIST_ELEM_GET_NEXT(planlists_flistp, PIN_FLD_DEALS, &elem_countdeals, 1, &cookie_countdeals, ebufp)) != NULL)
					//{
					//get_deal_obj = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
					PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Poid 1 :",read_deal_obj);
					PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Poid 2 :",get_deal_obj);
					if ((PIN_POID_COMPARE(read_deal_obj, get_deal_obj, 0, ebufp)) == 0)
					{
						package_id = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_PACKAGE_ID, 1, ebufp);
						if(!package_id || (package_id && *package_id == 0))
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Package Id not provided in input");
							r_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
									"ACCOUNT - Change plan - Package Id not provided in input", ebufp);
							*ret_flistp = r_flistp;
							return;
						}

						PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_PACKAGE_ID, package_id, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
								"fm_mso_bb_change_plan cancel sub input list", canceldeal_inflistp);
						PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_CANCEL_DEAL, 0, canceldeal_inflistp, &canceldeal_outflistp, ebufp);
						PIN_FLIST_DESTROY_EX(&canceldeal_inflistp, NULL);
						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
									"Error in calling PCM_OP_SUBSCRIPTION_CANCEL_DEAL", ebufp);
							PIN_ERRBUF_RESET(ebufp);
							PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);

							r_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, 
									"ACCOUNT - Service change plan -cancel deal errror", ebufp);
							*ret_flistp = r_flistp;
							return;
						}
						/****************************************************
						  Update the mso_purchased_plan
						 ****************************************************/
						if(update_mso_purch_plan == 1)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Updating mso_pur_plan for addon cancels");
							acc_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1,ebufp);
							fm_mso_get_mso_purplan(ctxp,acc_pdp, plan_obj,MSO_PROV_ACTIVE,&old_mso_plan_flistp,ebufp);
							if(old_mso_plan_flistp && old_mso_plan_flistp != NULL)
							{
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
										"fm_mso_get_mso_purplan return flist for cancelling add plans", old_mso_plan_flistp);
								mso_pp_obj = PIN_FLIST_FLD_GET(old_mso_plan_flistp, PIN_FLD_POID, 1,ebufp);
								if(fm_mso_update_mso_purplan_status(ctxp,mso_pp_obj,MSO_PROV_TERMINATE,ebufp) == 0)
								{
									r_flistp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51762", ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, 
											"Error while updating mso_purhcased_plan status.", ebufp);
									*ret_flistp = r_flistp;
									return;
								}

							}

						}
						PIN_FLIST_DESTROY_EX(&old_mso_plan_flistp,NULL);
						//	break;	
					}
					//}

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan cancel output flist", canceldeal_outflistp);
					//PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
					//Added block to fetch the refund value in case of postpaid
					if(price_calc_flag == 1 && indicator && *indicator == 1)
					{
						melemid = 0;
						mcookie  = NULL;
						resource_id = 0;
						while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(canceldeal_outflistp,
										PIN_FLD_RESULTS, &melemid, 1, &mcookie, ebufp ))!= NULL)
						{
							sub_bal_flist = PIN_FLIST_ELEM_GET(result_flistp, PIN_FLD_SUB_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp);
							if(sub_bal_flist && sub_bal_flist != NULL)
							{
								resource_id = *(int32 *) PIN_FLIST_FLD_GET(sub_bal_flist, PIN_FLD_RESOURCE_ID, 0, ebufp);
								if(resource_id == INR)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Currency impact found");
									break;
								}
							}
						}
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Balimpact flist",result_flistp);
						if(result_flistp && result_flistp != NULL)
						{
							refund_calc_flistp = PIN_FLIST_CREATE(ebufp);
							melemid = 0;
							mcookie  = NULL;
							PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, refund_calc_flistp, PIN_FLD_POID, ebufp);
							while((ref_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp,
											PIN_FLD_BAL_IMPACTS, &melemid, 1, &mcookie, ebufp ))!= NULL)
							{
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bal impact flist", ref_bal_flistp);
								resource_id = *(int32 *)PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_RESOURCE_ID, 0, ebufp);
								rate_tag = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_RATE_TAG, 1, ebufp);
								if(resource_id == INR) {
									temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
									if(pbo_decimal_is_null(total_charge, ebufp)) {
										PIN_ERR_LOG_MSG(3,"Step1");
										total_charge = temp_amount;
									} else {

										total_charge = pbo_decimal_add(temp_amount,total_charge, ebufp);
										PIN_ERR_LOG_MSG(3,"Step4");
									}
									if(!(rate_tag && strcmp(rate_tag,"Tax") == 0))
									{
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering charge block");
										temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
										if(pbo_decimal_is_null(charge, ebufp))
										{
											charge = temp_amount;
											PIN_ERR_LOG_MSG(3,"Step2");
										}
										else {
											charge = pbo_decimal_add(temp_amount,charge, ebufp);
											PIN_ERR_LOG_MSG(3,"Step5");
										}
									}
									else
									{
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering tax block");
										temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
										if(pbo_decimal_is_null(tax, ebufp))
										{
											tax = temp_amount;
											PIN_ERR_LOG_MSG(3,"Step3");
										}
										else {
											tax = pbo_decimal_add(temp_amount,tax, ebufp);
											PIN_ERR_LOG_MSG(3,"Step6");
										}
									}
								}
							}
							//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling price calc", ebufp);
							//Populating Refund details
							if(!pbo_decimal_is_null(total_charge, ebufp)) {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT, total_charge, ebufp);
							}
							else {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT, zero, ebufp);
							}
							if(!pbo_decimal_is_null(charge, ebufp)) {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_ORIG, charge, ebufp);
							}
							else {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_ORIG, zero, ebufp);
							}
							if(!pbo_decimal_is_null(tax, ebufp)) {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_TAXED, tax, ebufp);
							}
							else {
								PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_TAXED, zero, ebufp);
							}
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Refund calculated flist", refund_calc_flistp);

						}			

					}
					PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
				}
			}
			/*******************************************************
			  Update NON currecy resource validity -start
			 *******************************************************/
			ncr_validity_iflist = PIN_FLIST_CREATE(ebufp);

			//PIN_FLIST_FLD_SET(ncr_validity_iflist, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ncr_validity_iflist,PIN_FLD_ACCOUNT_OBJ, ebufp);
			PIN_FLIST_FLD_SET(ncr_validity_iflist, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
			PIN_FLIST_FLD_SET(ncr_validity_iflist, PIN_FLD_PROGRAM_NAME, "set_validity_by_change_plan", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"update ncr validity input",ncr_validity_iflist);
			fm_mso_update_ncr_validty(ctxp, ncr_validity_iflist, &ncr_validity_oflist, ebufp);
			if (ncr_validity_oflist && ncr_validity_oflist != NULL)
			{
				descr = PIN_FLIST_FLD_GET(ncr_validity_oflist, PIN_FLD_DESCR, 1, ebufp);
				if (descr && strcmp(descr ,"success") != 0)
				{
					PIN_FLIST_DESTROY_EX(&ncr_validity_iflist, NULL);
					PIN_FLIST_DESTROY_EX(&ncr_validity_oflist, NULL);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51762", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Updating non-currency validity", ebufp);
					return;
				}
			}
			PIN_FLIST_DESTROY_EX(&ncr_validity_iflist, NULL);
			PIN_FLIST_DESTROY_EX(&ncr_validity_oflist, NULL);
			/*******************************************************
			  Update NON currecy resource validity -start
			 *******************************************************/

			acc_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1,ebufp);
			/**********************************************************
			  Purchase plan will call  PCM_OP_SUBSCRIPTION_PURCHASE_DEAL
			 **********************************************************/
			/** Pavan Bellala - 15/07/2015  Added if condition ***/
			if( plan_change_after_expiry_scn == 1)
			{
				//fm_mso_get_mso_purplan(ctxp,acc_pdp, plan_obj,MSO_PROV_DEACTIVE,&old_mso_pp_flistp, ebufp);
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"base poid used",base_plan_obj);
				fm_mso_get_mso_purplan(ctxp,acc_pdp, base_plan_obj,MSO_PROV_DEACTIVE,&old_mso_pp_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"old_mso_flist returned-1", old_mso_pp_flistp);
			} else 
			{
				//fm_mso_get_mso_purplan(ctxp,acc_pdp, plan_obj,MSO_PROV_ACTIVE,&old_mso_pp_flistp, ebufp);
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"base poid used",base_plan_obj);
				fm_mso_get_mso_purplan(ctxp,acc_pdp, base_plan_obj,MSO_PROV_ACTIVE,&old_mso_pp_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"old_mso_flist returned-2", old_mso_pp_flistp);

			}
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_get_mso_purplan", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51753", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error while getting mso_purhcased_plan.", ebufp);
				*ret_flistp = r_flistp;
				return;
			}
			if(old_mso_pp_flistp && old_mso_pp_flistp != NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside main function. Return Flist");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_purplan return flist", old_mso_pp_flistp);
				mso_pp_obj = PIN_FLIST_FLD_GET(old_mso_pp_flistp, PIN_FLD_POID, 1,ebufp);
				plan_type = PIN_FLIST_FLD_TAKE(old_mso_pp_flistp, PIN_FLD_DESCR, 1,ebufp);
				if(fm_mso_update_mso_purplan_status(ctxp, mso_pp_obj, MSO_PROV_TERMINATE, ebufp ) == 0)
				{
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51762", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error while updating mso_purhcased_plan status.", ebufp);
					*ret_flistp = r_flistp;
					if(plan_type && plan_type != NULL){
						pin_free(plan_type);
						plan_type = NULL;	
					}
					return;					
				}

				if (plan_type && strcmp(plan_type, MSO_BB_BP) == 0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling provisioning action.");
				}
			}

                        //Static IP cancellation
                        if((istal && *istal == 1) || (str_rolep && strstr(str_rolep, "-Static")))
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan before adding TAL plans");
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan Before TAL added :", in_flist);
                                mso_cancel_static_ip_plan(ctxp,in_flist, refund_calc_flistp,indicator, plan_change_after_expiry_scn,ret_flistp,ebufp);
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan after adding TAL plans");
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan After TAL added :", refund_calc_flistp);
                        }

			//}
	}// end if(action_flag ==0)
	else if (*(int*)action_flags == 1)
	{

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Action flag is 1");
		//Pawan: Added below to update the bill when before purchasing new plan
		write_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(write_in_flistp, PIN_FLD_POID, service_obj, ebufp);
		bb_info_flistp = PIN_FLIST_SUBSTR_ADD(write_in_flistp, MSO_FLD_BB_INFO, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_BILL_WHEN, bb_info_flistp, PIN_FLD_BILL_WHEN, ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist for updating Bill When", write_in_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, write_in_flistp, &write_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist for updating Bill When", write_out_flistp);
		PIN_FLIST_DESTROY_EX(&write_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_out_flistp, NULL);

		/* Create flist to call purchase plan function */
		pp_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, pp_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp );
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, pp_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, pp_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_MODE, pp_flistp, PIN_FLD_MODE, ebufp );
		PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_PLAN_LISTS, elem_base, pp_flistp, PIN_FLD_PLAN, 0, ebufp);
		args_flistp = PIN_FLIST_ELEM_GET(pp_flistp, PIN_FLD_PLAN, 0, 0, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, &subs_type, ebufp);
		fm_mso_add_prod_info(connp, ctxp, pp_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_add_prod_info", ebufp);
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_DESTROY_EX(&pp_flistp, NULL);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51763", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error while getting products for plan.", ebufp);
			*ret_flistp = r_flistp;
			return;
		}
		//OPCODE field added for OFFERS change
		PIN_FLIST_FLD_SET(pp_flistp, PIN_FLD_OPCODE, &opcode_num, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_bb_plans input flist", pp_flistp);
		fm_mso_purchase_bb_plans(connp, pp_flistp, (char *)NULL,1, 0, &tmp_res_flistp, ebufp );
		PIN_FLIST_DESTROY_EX(&pp_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			*ret_flistp = tmp_res_flistp;
			return;
		}
		if (tmp_res_flistp && tmp_res_flistp != NULL)
		{
			ret_status = PIN_FLIST_FLD_GET(tmp_res_flistp, PIN_FLD_STATUS, 1, ebufp);
			if (ret_status && (*ret_status == change_plan_failure))
			{
				*ret_flistp = tmp_res_flistp;
				return;
			}
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_bb_plans return flist", tmp_res_flistp);
		new_mso_pp_obj =  PIN_FLIST_FLD_TAKE(tmp_res_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp );

		/* Sisir:30-04-2015 Set FUP_STATUS to 1 */
		//base_plan_pdp=PIN_FLIST_FLD_GET(bb_bp_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID,in_flist,PIN_FLD_ACCOUNT_OBJ,ebufp );
		fm_mso_set_fup_status(ctxp, in_flist, new_mso_pp_obj, &fup_ret_flistp, ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set FUP_STATUS return flist", fup_ret_flistp );

		if (fup_ret_flistp && fup_ret_flistp != NULL)
		{

			ret_status = (int32*)PIN_FLIST_FLD_GET(fup_ret_flistp, PIN_FLD_STATUS, 0, ebufp);
			if (ret_status && (*ret_status == change_plan_failure))
			{
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51790", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in calling Write flds for updating FUP status.", ebufp);
				*ret_flistp = r_flistp;
				PIN_FLIST_DESTROY_EX(&tmp_res_flistp, NULL);
				return;
			}
		}
		PIN_FLIST_DESTROY_EX(&fup_ret_flistp, NULL);


		//For lifecycle
		plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
		if (!service_obj || !plan_obj)
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51764", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - plan object is missing in input", ebufp);

			*ret_flistp = r_flistp;
			return;
		}

		readplan_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_POID, plan_obj, ebufp );
		PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_NAME, NULL, ebufp );
		PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_CODE, NULL, ebufp );
		deals_flistp = PIN_FLIST_ELEM_ADD(readplan_inflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(deals_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan read plan input list", readplan_inflistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readplan_inflistp, &readplan_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&readplan_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);

			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51765", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - service not found", ebufp);

			*ret_flistp = r_flistp;
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan read plan output flist", readplan_outflistp);
		PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_CODE, planlists_flistp, PIN_FLD_CODE, ebufp);
		PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);
	}
	/*else if(*(int*)action_flags == 1)

	// Create flist to call purchase plan function 
	pp_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, pp_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp );
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, pp_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, pp_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
	PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_PLAN_LISTS, elem_base, pp_flistp, PIN_FLD_PLAN, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_bb_plans input flist", pp_flistp);
	fm_mso_purchase_bb_plans(ctxp, pp_flistp, (char *)NULL,1,&tmp_res_flistp, ebufp );
	PIN_FLIST_DESTROY_EX(&pp_flistp, NULL);
	if (tmp_res_flistp)

	ret_status = PIN_FLIST_FLD_GET(tmp_res_flistp, PIN_FLD_STATUS, 1, ebufp);
	if (ret_status && (*ret_status == change_plan_failure))

	 *ret_flistp = tmp_res_flistp;
	 return;


	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_bb_plans return flist", tmp_res_flistp);
	 new_mso_pp_obj =  PIN_FLIST_FLD_TAKE(tmp_res_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 0, ebufp );
	 */
}
//BUG CAUSING DESTROY
//PIN_FLIST_DESTROY_EX(&read_out_flistp,NULL);

prog_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//New Changes for extending the validity date
if( price_calc_flag == 0 && (prog_name && strstr(prog_name, "banner"))){

	fm_mso_cust_upgrade_plan_validity(ctxp, in_flist, &extn_flistp, ebufp);	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "AFTER EXXTN VALIDY", extn_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51765", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Upgrade Extension is failed", ebufp);
		*ret_flistp = r_flistp;
		return;
	}
	if(extn_flistp && extn_flistp != NULL){
		extn_status = PIN_FLIST_FLD_GET(extn_flistp, PIN_FLD_STATUS, 1, ebufp);
		if(extn_status && *extn_status == 0){
			vp = NULL;
			vp = (time_t *)PIN_FLIST_FLD_GET(extn_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
			if(vp && vp != NULL)
				purch_end_t = *(time_t *)vp;
			vp = NULL;
			vp = PIN_FLIST_FLD_GET(extn_flistp, MSO_FLD_OLD_VALIDITY_END_T, 1, ebufp);
			if(vp)
				old_purch_end_t = *(time_t *)vp;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validity is done successfully UPGARDE");
			//Create validityextn lifecycle for maintiang the history
			fm_mso_create_lifecycle_evt(ctxp, extn_flistp, r_flistp, ID_VALIDITY_EXTENTION_FOR_A_SERVICE_PLAN, ebufp);

			PIN_FLIST_DESTROY_EX(&extn_flistp, NULL);
		}
		else{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51768", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in setting the validity.", ebufp);
			*ret_flistp = r_flistp;
			return;					
		}
	}
	else{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51768", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in setting the validity.", ebufp);
		*ret_flistp = r_flistp;
		return;					
	}
}


/* Update the /mso_purchased_plan object for new plan. */
ret_val = fm_mso_update_mso_purplan(ctxp, new_mso_pp_obj, MSO_PROV_IN_PROGRESS, plan_type, ebufp );
if(plan_type && plan_type != NULL){
	pin_free(plan_type);
	plan_type = NULL;	
}
if(ret_val)
{
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51766", ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error while updating mso_purhcased_plan status.", ebufp);
	*ret_flistp = r_flistp;
	return;					
}	

/* Update service provisioning status to In-Progress */
ret_val = fm_mso_update_ser_prov_status(ctxp, in_flist, MSO_PROV_IN_PROGRESS, ebufp );
if(ret_val)
{
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51767", ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error while updating service provisioning status.", ebufp);
	*ret_flistp = r_flistp;
	return;					
}

// Call the function to change the Static IP if IP pool is different
if(cmts_id && ((istal && *istal==1) || (str_rolep && strstr(str_rolep, "-Static"))))
{
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service is IsTAL. Calling change static IP.)");
	//prog_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	ret_val = fm_mso_cust_change_static_ip(ctxp, old_mso_pp_flistp, new_mso_pp_obj, cmts_id, prog_name, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "Error in calling fm_mso_cust_change_static_ip :", ebufp);
		return;
	}
	if(ret_val == 1)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51771", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error while change Static IP to new IP pool.", ebufp);
		*ret_flistp = r_flistp;
		pin_free(istal);
		if (str_rolep) pin_free(str_rolep);
		istal = NULL;
		pin_free(cmts_id);
		cmts_id = NULL;
		return;					
	}

	/****** Pavan Bellala 07-09-2015 ************************
	  Fetch the network flist from the return 
	 ***********************************************************/

	if(old_mso_pp_flistp != NULL && ret_val == 0)
	{
		network_flistp = PIN_FLIST_ELEM_TAKE(old_mso_pp_flistp,PIN_FLD_IP_ADDRESSES,0,1,ebufp);
		PIN_FLIST_FLD_SET(network_flistp,MSO_FLD_ISTAL_FLAG,istal,ebufp);
        PIN_FLIST_FLD_SET(network_flistp, MSO_FLD_ROLES, str_rolep, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_bb_change_plan: Change Static IP task flist",network_flistp);
	}
}
if(cmts_id && cmts_id != NULL){
	pin_free(cmts_id);
	cmts_id = NULL;
}


//PIN_FLIST_DESTROY_EX(&tmp_res_flistp, NULL);

/*Assign ether_prov_flag to status. It will be non-zero only if 
  it is prepaid to postpaid plan change or vice-versa*/
status_flags = ether_prov_flag;
if(!status_flags || *ser_tech==1 || *ser_tech ==2 || *ser_tech == 5 || *ser_tech == 3)
{
	/*Get the appropriate prov action flag.*/
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting the change type(Quota/Non-Quota)");
	status_flags = fm_mso_bb_get_change_type(ctxp, old_mso_pp_flistp, new_mso_pp_obj, ebufp );
	if(!status_flags)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51768", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in getting the plan type.", ebufp);
		*ret_flistp = r_flistp;
		return;					
	}
}


if (price_calc_flag ==0 )
{
	/* Call BB change plan provisioning action*/
	//status_flags = DOC_BB_PC_QUOTA_QUOTA;
	prov_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, prov_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,prov_in_flistp,PIN_FLD_USERID,ebufp);
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,prov_in_flistp,PIN_FLD_PROGRAM_NAME,ebufp);

	/* Changing to handle plan change after expiry with different flag status_flags_after_expiry = DOC_BB_RENEW_AEXPIRY_PRE */
	if( plan_change_after_expiry_scn == 1)
	{
		PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &status_flags_after_expiry, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &status_flags, ebufp);
	}

	PIN_FLIST_FLD_SET(prov_in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, new_mso_pp_obj, ebufp);
	/*Handling to get correct resource_id */
	//PIN_FLIST_ELEM_COPY(tmp_res_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, prov_in_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, ebufp);
	read_mso_plan_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_mso_plan_flistp, PIN_FLD_POID, new_mso_pp_obj, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_change mso_plan: Read input flist", read_mso_plan_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_mso_plan_flistp, &read_mso_plan_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_mso_plan_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_change mso_plan: Read output flist", read_mso_plan_oflistp);		

	while ( (prod_flistp = PIN_FLIST_ELEM_GET_NEXT (read_mso_plan_oflistp,
					PIN_FLD_PRODUCTS, &rec_id_p, 1,&cookie_p, ebufp)) != (pin_flist_t *)NULL )
	{
		prov_tag = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		if ( prov_tag && strlen(prov_tag) > 0)
		{
			break;
		}
	}		

	rfld_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ,rfld_iflistp, PIN_FLD_POID, ebufp);
	//Add the BB Info SUBSTR
	temp_flistp = PIN_FLIST_SUBSTR_ADD(rfld_iflistp, MSO_FLD_BB_INFO, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_TECHNOLOGY, (void *)0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_change: Read Service In flist", rfld_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp, &rfld_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_change: Read Service Out flist", rfld_oflistp);
	temp_flistp = PIN_FLIST_SUBSTR_GET(rfld_oflistp, MSO_FLD_BB_INFO, 0, ebufp);
	tech = (int *)PIN_FLIST_FLD_GET(temp_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);		

	fm_quota_check(ctxp, plan_name, prov_tag, tech, &plan_typee, &initial_amount, &error_codep, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_quota_chek details", ebufp);
		PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_mso_plan_flistp, NULL);
		return;
	}

	while ((bal_imp = PIN_FLIST_ELEM_GET_NEXT(tmp_res_flistp, PIN_FLD_SUB_BAL_IMPACTS, &elem_id_r, 1, &cookie_r, ebufp)) != NULL)
	{
		bal_res = PIN_FLIST_FLD_GET(bal_imp, PIN_FLD_RESOURCE_ID, 1, ebufp);
		if(plan_typee == 1 && (bal_res && (*bal_res == MSO_FREE_MB)))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " This is limited plan ");
			//This is added for banner upgrade of plans AD and for LIMITED plans, set VALID_TO to extended date.
			if(prog_name && strstr(prog_name, "banner")){
				sub_bals_flistp = PIN_FLIST_ELEM_GET(bal_imp, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, 1, ebufp);
				if(sub_bals_flistp && sub_bals_flistp != NULL){
					PIN_FLIST_FLD_SET(sub_bals_flistp, PIN_FLD_VALID_TO, &purch_end_t, ebufp);
				}
			}
			break;
		}
		else if(plan_typee == 2 && (bal_res && (*bal_res == MSO_FUP_TRACK)))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " This is fup plan ");
			break;
		}
		else if(plan_typee == 0 && (bal_res && (*bal_res == MSO_TRCK_USG)))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " This is unlimited plan ");
			break;
		}



	}


	PIN_FLIST_ELEM_SET(prov_in_flistp,bal_imp,PIN_FLD_SUB_BAL_IMPACTS, elem_id_r,ebufp);		

	if(network_flistp != NULL)
	{
		PIN_FLIST_ELEM_PUT(prov_in_flistp,network_flistp,PIN_FLD_IP_ADDRESSES,0,ebufp);
	}
	if (ser_tech && *ser_tech == ETHERNET)
	{
		PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_PLAN_LISTS, 0, prov_in_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
		PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_PLAN_LISTS, 1, prov_in_flistp, PIN_FLD_PLAN_LISTS, 1, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan provisioning input flist", prov_in_flistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, prov_in_flistp, &prov_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);

	if(prov_out_flistp && prov_out_flistp != NULL)
	{
		prov_call = PIN_FLIST_FLD_GET(prov_out_flistp, PIN_FLD_STATUS, 1, ebufp);
		if(prov_call && (*prov_call == 1))
		{
			*ret_flistp = PIN_FLIST_COPY(prov_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_mso_plan_flistp, NULL);
			return;
		}
	}
	else
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51769", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning action.", ebufp);
		PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_mso_plan_flistp, NULL);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan provisioning output flist", prov_out_flistp);
	PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);	
}
else
{
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PRICE_CALCULATOR:Skipped calling MSO_OP_PROV_CREATE_ACTION");
}

//Call AMC function; Moved this function call to after provisioing
//    call to avoid the cancelled AMC plan being fetched in prov opcode.
ret_val==0;
ret_val = fm_mso_change_amc_plan(ctxp, in_flist, old_plan_type, new_plan_type, &tmp_res_flistp, ebufp );
if(ret_val==1)
{
	PIN_ERRBUF_CLEAR(ebufp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51781", ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error in associating AMC plan", ebufp);
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_mso_plan_flistp, NULL);
	*ret_flistp = r_flistp;
	return;
}

/* Success flist*/
//fm_mso_cust_change_plan_lc_event(ctxp, in_flist, NULL, ebufp);
	//Add lifecycle for TAL plan change
	if ( price_calc_flag == 0 )
        {

                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Create Lifecycle event for base plan");
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "input for lifecycle event", in_flist);
                fm_mso_create_lifecycle_evt(ctxp, in_flist, NULL,ID_CHANGE_PLAN, ebufp );
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Create Lifecycle event for base plan over");
                if (istal == NULL)
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test istal flag");
                if ((istal && *istal == 1) || (str_rolep && strstr(str_rolep, "-Static")))
                {
                        PIN_FLIST_FLD_SET(in_flist, PIN_FLD_INDICATOR, indicator, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Create Lifecycle event for Static IP plan");
                        fm_mso_create_tal_change_lifecycle(ctxp, in_flist, bill_when, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Create Lifecycle event for Static IP plan");
                }
        }


	PIN_FLIST_DESTROY_EX(&old_mso_pp_flistp, NULL);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_success, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "Plan changed successfully.", ebufp);
	args_flistp = PIN_FLIST_ELEM_TAKE(tmp_res_flistp,PIN_FLD_DATA_ARRAY,0,1,ebufp);
	if(args_flistp && args_flistp != NULL)
		PIN_FLIST_CONCAT(r_flistp, args_flistp, ebufp);
	//Added to populate the refund details output flist
	if(refund_calc_flistp && refund_calc_flistp != NULL)
	{
		ref_flistp = PIN_FLIST_ELEM_ADD(r_flistp, MSO_FLD_REFUND, 0,ebufp);
		PIN_FLIST_FLD_COPY(refund_calc_flistp, PIN_FLD_AMOUNT, ref_flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_COPY(refund_calc_flistp, PIN_FLD_AMOUNT_ORIG, ref_flistp, PIN_FLD_AMOUNT_ORIG, ebufp);
		PIN_FLIST_FLD_COPY(refund_calc_flistp, PIN_FLD_AMOUNT_TAXED, ref_flistp, PIN_FLD_AMOUNT_TAXED, ebufp);
		PIN_FLIST_FLD_COPY(refund_calc_flistp, MSO_FLD_MISC_AMT, ref_flistp, MSO_FLD_MISC_AMT, ebufp);
	}
        if (istal) {
                pin_free(istal);
                istal = NULL;
        }

        if (str_rolep) pin_free(str_rolep);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Final output flistp", refund_calc_flistp);	
	*ret_flistp = r_flistp;
	PIN_FLIST_DESTROY_EX(&tmp_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_mso_plan_flistp, NULL);
	if(read_mso_plan_oflistp != NULL )PIN_FLIST_DESTROY_EX(&read_mso_plan_oflistp, NULL); 
	if(new_mso_pp_obj != NULL)PIN_POID_DESTROY(new_mso_pp_obj, NULL);
	if(read_out_flistp != NULL ) PIN_FLIST_DESTROY_EX(&read_out_flistp,NULL);
}


static void 
fm_mso_validate_bb_change_plan(
		pcm_context_t	*ctxp,
		pin_flist_t		*in_flist,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t		*planlist_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*plan_obj = NULL;
	pin_cookie_t	cookie = NULL;
	poid_t			*cancel_plan_pdp = NULL;
	poid_t			*add_plan_pdp = NULL;

	int				elem_id = 0;
	int				*flag =  NULL;
	int				remove_found = 0;
	int				add_found = 0;
	int				plan_count = 0;
	int			change_plan_success = 0;
	int			change_plan_failure = 1;	
	int32			*old_plan_statusp = NULL;

	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*so_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;

	int			*old_bill_whenp = NULL;
	int			*new_bill_whenp = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error before calling fm_mso_validate_bb_change_plan :");
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_bb_change_plan input flist", in_flist);
	plan_count = PIN_FLIST_ELEM_COUNT(in_flist, PIN_FLD_PLAN_LISTS, ebufp );
	if(plan_count !=2)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51758", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Number of plans must be Two.", ebufp);
		*ret_flistp = r_flistp;
		return;
	}

	/***** Pavan Bellala - 07/07/2015 *******************************************
	  Fix for Change Plan scenario. Same plan with different pay terms are valid
	 ***** Pavan Bellala - 07/07/2015 *******************************************/
	new_bill_whenp = PIN_FLIST_FLD_GET(in_flist,PIN_FLD_BILL_WHEN,1,ebufp);
	elem_id = 0;
	cookie = NULL;
	while ((planlist_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, &elem_id, 1, &cookie, ebufp )) != NULL)
	{
		plan_obj = PIN_FLIST_FLD_GET(planlist_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
		if(!plan_obj || PIN_POID_IS_NULL(plan_obj))
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51770", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - plan object is missing in input", ebufp);
			*ret_flistp = r_flistp;
			return;
		}
		flag = PIN_FLIST_FLD_GET(planlist_flistp, PIN_FLD_FLAGS, 1, ebufp );
		if (*(int*)flag == 0)
		{
			remove_found = 1;
			cancel_plan_pdp = PIN_FLIST_FLD_GET(planlist_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
			old_plan_statusp = (int32 *) PIN_FLIST_FLD_GET(planlist_flistp, PIN_FLD_STATUS, 1, ebufp);
			PIN_ERR_LOG_POID(3, "cancel_plan_pdp", cancel_plan_pdp);
		}
		else if(*(int*)flag == 1)
		{
			add_found = 1;
			add_plan_pdp = PIN_FLIST_FLD_GET(planlist_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
			PIN_ERR_LOG_POID(3, "add_plan_pdp", add_plan_pdp);
		}
	}

	if(!remove_found)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51755", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Change plan - Plan to be cancelled not found.", ebufp);
		*ret_flistp = r_flistp;
		return;
	}

	if(!add_found)
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51756", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Change plan - Plan to be added, not found.", ebufp);
		*ret_flistp = r_flistp;
		return;
	}	
	if((PIN_POID_COMPARE(cancel_plan_pdp, add_plan_pdp, 0, ebufp)) == 0)
	{
		/***  Pavan Bellala  -  07/07/2015 *****************
		  Check if the payterms are changed or remain the same 
		 ***  Pavan Bellala  -  07/07/2015 *******************/
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test2");
		s_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_SERVICE_OBJ,s_flistp,PIN_FLD_POID,ebufp);
		//r_flistp = PIN_FLIST_SUBSTR_ADD(s_flistp,MSO_FLD_BB_INFO,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read flds input",s_flistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ ,PCM_OPFLG_CACHEABLE , s_flistp, &so_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read flds output",so_flistp);
		PIN_FLIST_DESTROY_EX(&s_flistp,NULL);

		if(so_flistp)
		{
			bb_info_flistp = PIN_FLIST_SUBSTR_GET(so_flistp,MSO_FLD_BB_INFO,1,ebufp);
			if(bb_info_flistp)
			{
				old_bill_whenp = PIN_FLIST_FLD_GET(bb_info_flistp,PIN_FLD_BILL_WHEN,1,ebufp);
				if((old_plan_statusp && *old_plan_statusp == 1)  
					&& old_bill_whenp && new_bill_whenp 
					&& (*old_bill_whenp == *new_bill_whenp))
				{
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51757", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Old plan and new plan are same. No need for change plan", ebufp);
					*ret_flistp = r_flistp;
					PIN_FLIST_DESTROY_EX(&so_flistp,NULL);
					return;
				}
			}	
		}
		PIN_FLIST_DESTROY_EX(&so_flistp,NULL);
	}
	return;
}

/*******************************************************
 * Function: fm_mso_validate_bb_plan_custtype()
 *	Validates the plan to be purchased against the
 *	value of customer type.
 *	New plan must be compatible with cust type.
 *******************************************************/
static void 
fm_mso_validate_bb_plan_custtype(
		pcm_context_t	*ctxp,
		pin_flist_t	*in_flist,
		pin_flist_t	**ret_flistp,
		int		*ether_prov_flag,
		int		*old_type,
		int		*new_type,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t		*planlist_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*plan_obj = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*add_plan_flistp = NULL;
	pin_flist_t		*deal_array = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*deals_activation = NULL;
	pin_flist_t		*pay_flistp = NULL;
	pin_flist_t		*pay_inv_flistp = NULL;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	d_cookie = NULL;
	poid_t			*acc_pdp = NULL;
	poid_t			*prod_pdp = NULL;
	poid_t			*ser_pdp = NULL;
	poid_t			*add_plan_pdp = NULL;
	int				count = 0;
	int				elem_id = 0;
	int				d_elem_id = 0;	
	int				*flag =  NULL;
	int				add_found = 0;
	int				plan_count = 0;
	int				change_plan_success = 0;
	int				change_plan_failure = 1;
	int				set_err = 0;
	int				new_plan_type = -1;
	int				res_val = 0;
	int				*bill_when = NULL;
	int32			*acc_pay_type = NULL;
	int64			db=0;
	int32			srch_flag = 512;
	int32			*customer_type = NULL;
	int				priority_int = -10;
	int				temp_pri = 0;
	char			*search_template = "select X from /mso_customer_credential where F1 = V1 ";
	char			*prov_tag = NULL;
	char			msg[100]="";
	char			*tmp_str = NULL;
	pin_decimal_t	*priority = NULL;
	int		is_change_valid = 0;
	pin_flist_t	*validate_flistp = NULL;
	pin_flist_t	*w_flistp = NULL;
	pin_flist_t	*wo_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	void		*vp =  NULL;
	int		*parent_indp = NULL;
	poid_t		*parent_payinfop = NULL;
	pin_flist_t	*inhinfo_flistp = NULL;
	pin_flist_t	*invinfo_flistp = NULL;
	int		parent_paytype = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error before calling fm_mso_validate_bb_plan_custtype :");
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_bb_plan_custtype input flist", in_flist);
	add_plan_flistp = PIN_FLIST_CREATE(ebufp);

	/* Fetch customer type for this account from mso_customer_credential */
	acc_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	ser_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_CUSTOMER_TYPE, (int32 *)NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer type search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp) || !srch_out_flistp )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51811", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching  customer type for this account", ebufp);
		return ;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer type search output flist", srch_out_flistp);	
	result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	customer_type = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_CUSTOMER_TYPE, 1, ebufp);	
	if ( !customer_type || customer_type == (int32 *)NULL )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Customer type is not updated for this account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51812", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Customer type is not updated for this account", ebufp);
		return ;
	}	

	/*Find the plan obj for new plan*/
	while ((planlist_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, &elem_id, 1, &cookie, ebufp )) != NULL)
	{
		flag = PIN_FLIST_FLD_GET(planlist_flistp, PIN_FLD_FLAGS, 1, ebufp );
		if(flag && *(int*)flag == 1)
		{
			add_found = 1;
			add_plan_pdp = PIN_FLIST_FLD_GET(planlist_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
			PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_PLAN_LISTS, elem_id, add_plan_flistp, PIN_FLD_PLAN, 0, ebufp );
		}
	}
	PIN_FLIST_FLD_SET(add_plan_flistp, PIN_FLD_POID, add_plan_pdp, ebufp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Add Plan poid :",add_plan_pdp);

	/* For getting deals and products for new plan */
	fm_mso_get_deals_from_plan(ctxp, add_plan_flistp, &deal_array, &temp_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deal Array:", deal_array);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Credit Limit:", temp_flistp);
	PIN_FLIST_DESTROY_EX(&temp_flistp, NULL);
	count = PIN_FLIST_ELEM_COUNT(deal_array, PIN_FLD_DEALS, ebufp);
	if( count == 0)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Deals not found for selected Plans.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51813", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Deals not found for selected Plans.", ebufp);
		return;
	}

	fm_mso_get_deal_info(ctxp, deal_array, &temp_flistp, &deals_activation,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Deals Activation Flist", deals_activation);
	PIN_FLIST_DESTROY_EX(&deal_array, NULL);
	PIN_FLIST_DESTROY_EX(&temp_flistp, NULL);

	/*Get any one product poid under the deal*/
	cookie=NULL;
	while ((deal_array = PIN_FLIST_ELEM_GET_NEXT(deals_activation, PIN_FLD_DEALS, 
					&elem_id, 1, &cookie, ebufp )) != NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"In plan -->");
		temp_flistp = PIN_FLIST_SUBSTR_GET(deal_array, PIN_FLD_DEAL_INFO, 1, ebufp );
		if(temp_flistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"In Deal -->");
			planlist_flistp = NULL;
			planlist_flistp = PIN_FLIST_ELEM_GET(temp_flistp, PIN_FLD_PRODUCTS, 0, 1, ebufp );
			if(planlist_flistp)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"In Product -->");
				prod_pdp = PIN_FLIST_FLD_TAKE(planlist_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp );
			}
		}
	}	

	if(!prod_pdp)
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51814", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Change plan - Plan to be added, not found.", ebufp);
		return;
	}

	PIN_FLIST_DESTROY_EX(&deals_activation, NULL);
	temp_flistp = NULL;
	/* Fetch product priority */
	fm_mso_cust_get_prod_details(ctxp, prod_pdp, &temp_flistp, ebufp);
	if(!temp_flistp)
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51815", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in fetching products details for new plan", ebufp);
		return;
	}
	prov_tag = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
	priority = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_PRIORITY, 1, ebufp );
	if ( !priority || priority == (pin_decimal_t *) NULL )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS for product", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51816", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in fetching priority for product.", ebufp);
		return;
	}
	tmp_str = pbo_decimal_to_str(priority, ebufp);
	priority_int = atoi(tmp_str);
	if (tmp_str) {
		pin_free(tmp_str);
		tmp_str = NULL;
	}
	sprintf(msg , " Priority is: %d  and cust type is %d " , priority_int,*(int32 *)customer_type);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	/* Check if the product priority falls with range specified for each cust type. */
	if ( *(int32 *)customer_type == ACCT_TYPE_CORP_SUBS_BB )
	{	
		if ( priority_int < BB_SUB_COR_PRI_START || priority_int > BB_SUB_COR_PRI_END )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_CORP");
			set_err = 1;
			goto NOMATCH;
		}
	}
	/**** Pavan Bellala 28-09-2015 ********
	  Commented as cyber cafe is removed
	  else if ( *(int32 *)customer_type == ACCT_TYPE_CYBER_CAFE_BB )
	  {
	  if ( priority_int < BB_SUB_CYB_PRI_START || priority_int > BB_SUB_RET_PRI_END )
	  {
	  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_CYB");
	  set_err = 1;
	  goto NOMATCH;
	  }
	  }
	 ******************************************/
	  else if ( *(int32 *)customer_type == ACCT_TYPE_RETAIL_BB )
	  {
		  if ( priority_int < BB_SUB_RET_PRI_START || priority_int > BB_SUB_RET_PRI_END )
		  {
			  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_RET");
			  set_err = 1;
			  goto NOMATCH;
		  }
	  }
	  else if ( *(int32 *)customer_type == ACCT_TYPE_SME_SUBS_BB )
	  {
		  if ( priority_int < BB_SUB_SME_PRI_START || priority_int > BB_SUB_SME_PRI_END )
		  {
			  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_SME");
			  set_err = 1;
			  goto NOMATCH;
		  }
	  }
	  //Added condition to handle invalid data
	  else if ( *(int32 *)customer_type == ACCT_TYPE_RESERVED_BB )
	  {
		  set_err = 1;
		  goto NOMATCH;
	  }
	  /* Find the New plan type: Prepaid/Postpaid */
	  temp_pri=priority_int%1000;
	  if(temp_pri < 500) // New plan type if prepaid.
	  {
		  new_plan_type = PAYINFO_BB_PREPAID;
	  } 
	  else // temp_pri >=500; New plan type is postpaid.
	  {
		  new_plan_type = PAYINFO_BB_POSTPAID;
	  }

	  /* Fetch the Payinfo-Indicator for this account. */
	  fm_mso_bb_get_payinfo_inv(ctxp, acc_pdp, &pay_flistp, ebufp);
	  if(!pay_flistp || PIN_ERRBUF_IS_ERR(ebufp))
	  {
		  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in fetching Payinfo->Invoice");
		  PIN_ERRBUF_CLEAR(ebufp);
		  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Change plan input flist",in_flist);
		  *ret_flistp = PIN_FLIST_CREATE(ebufp);
		  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51818", ebufp);
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in fetching Payinfo->Invoice", ebufp);
		  return ;
	  }

	  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, pay_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
	  pay_inv_flistp = PIN_FLIST_ELEM_GET(pay_flistp, PIN_FLD_INV_INFO, 0, 0, ebufp);
	  acc_pay_type = PIN_FLIST_FLD_GET(pay_inv_flistp, PIN_FLD_INDICATOR, 0, ebufp);
	  sprintf(msg , "New plan type is %d  and Cust pay type is %d " , new_plan_type,*(int32 *)acc_pay_type);
	  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

	  //Assign values to old_type and new_type to be returned from function.
	  *old_type = *acc_pay_type;
	  *new_type = new_plan_type;

	  /* Compare the Plan type and Customer Pay type */
	  if(*acc_pay_type != new_plan_type)
	  {

		  /*************Pavan Bellala 10-08-2015 ***************************
		    Bug id : 1062
		    Added functionality to allow change for hierarchy accounts on 
		    conditions
		   ******************************************************************/
		  //fm_cust_chng_plan_validate_hierarchy(ctxp,pay_flistp,&new_plan_type,&validate_flistp,ebufp);
		  fm_cust_chng_plan_validate_hierarchy(ctxp,pay_flistp,&validate_flistp,ebufp);
		  if(validate_flistp)
		  {
			  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_cust_chng_plan_validate_hierarchy-return",validate_flistp); 
			  vp = PIN_FLIST_FLD_GET(validate_flistp,PIN_FLD_RESULT_FLAGS,1,ebufp);
			  is_change_valid = *(int*)vp;	
			  if(is_change_valid == PIN_BOOLEAN_TRUE)
			  {
				  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Cust type NOT equal to Plan Type");		
				  if(new_plan_type == PAYINFO_BB_PREPAID)
				  {
					  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "New Plan is Prepaid. Account will be changed to Prepaid");
					  *ether_prov_flag = DOC_BB_PC_POST_PRE;
				  }
				  else
				  {
					  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "New Plan is Postpaid. Account will be changed to Postpaid");
					  *ether_prov_flag = DOC_BB_PC_PRE_POST;
				  }

				  parent_payinfop = PIN_FLIST_FLD_GET(validate_flistp,PIN_FLD_POID,1,ebufp);
				  if(parent_payinfop && (strstr((char*)PIN_POID_GET_TYPE(parent_payinfop),"payinfo")!= (char *)NULL))
				  {
					  /******** Update the payinfo of the parent ********/		
					  parent_indp = PIN_FLIST_FLD_GET(validate_flistp,PIN_FLD_INDICATOR,1,ebufp);
					  //if(parent_indp && (*parent_indp) == MSO_ACCT_ONLY){
					  /**** INDICATOR field present in flist means, the new child is 10007 and 
					    needed to update parent payinfo too *****/				
					  w_flistp = PIN_FLIST_CREATE(ebufp);
					  PIN_FLIST_FLD_COPY(validate_flistp,PIN_FLD_PARENT,w_flistp,PIN_FLD_POID,ebufp);
					  PIN_FLIST_FLD_COPY(validate_flistp,PIN_FLD_PARENT,w_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
					  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME,w_flistp,PIN_FLD_PROGRAM_NAME,ebufp);
					  tmp_flistp = PIN_FLIST_ELEM_ADD(w_flistp,PIN_FLD_PAYINFO,0,ebufp);
					  PIN_FLIST_FLD_COPY(validate_flistp, PIN_FLD_POID,tmp_flistp,PIN_FLD_POID,ebufp);
					  parent_paytype = PIN_BILL_TYPE_INVOICE;
					  PIN_FLIST_FLD_SET(tmp_flistp,PIN_FLD_PAY_TYPE,&parent_paytype,ebufp);
					  PIN_FLIST_FLD_COPY(validate_flistp, PIN_FLD_PAYMENT_TERM,tmp_flistp,PIN_FLD_PAYMENT_TERM,ebufp);
					  inhinfo_flistp = PIN_FLIST_SUBSTR_ADD(tmp_flistp,PIN_FLD_INHERITED_INFO,ebufp);
					  invinfo_flistp = PIN_FLIST_ELEM_ADD(inhinfo_flistp,PIN_FLD_INV_INFO,0,ebufp);
					  //	PIN_FLIST_FLD_COPY(validate_flistp,PIN_FLD_CHILDREN,tmp_flistp,PIN_FLD_INDICATOR,ebufp);
					  PIN_FLIST_FLD_SET(invinfo_flistp,PIN_FLD_INDICATOR,&new_plan_type,ebufp);
					  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"update parent indicator for paying parent in",w_flistp);
					  PCM_OP(ctxp, PCM_OP_CUST_UPDATE_CUSTOMER, 0, w_flistp, &wo_flistp, ebufp);	
					  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"update parent indicator for paying parent out",wo_flistp);

					  if(PIN_ERRBUF_IS_ERR(ebufp))
					  {
						  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in Updating Parent Payinfo->Indicator.");
						  PIN_ERRBUF_CLEAR(ebufp);
						  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Change plan input flist",in_flist);
						  *ret_flistp = PIN_FLIST_CREATE(ebufp);
						  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
						  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
						  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51823", ebufp);
						  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Parent Payinfo->Indicator.", ebufp);
						  PIN_FLIST_DESTROY_EX(&validate_flistp,NULL);
						  PIN_FLIST_DESTROY_EX(&w_flistp,NULL);
						  PIN_FLIST_DESTROY_EX(&wo_flistp,NULL);
						  return ;
					  }
					  //}
				  }

			  } else {
				  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Plan change not valid for hierarchy.");
				  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Change plan input flist",in_flist);
				  *ret_flistp = PIN_FLIST_CREATE(ebufp);
				  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
				  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
				  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51822", ebufp);
				  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plan change not valid for hierarchy.", ebufp);
				  return ;
			  }

		  }  else {
			  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error in validating account's hierarchy");
			  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Change plan input flist",in_flist);
			  *ret_flistp = PIN_FLIST_CREATE(ebufp);
			  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
			  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51823", ebufp);
			  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in validating account's hierarchy", ebufp);
			  return ;
		  }	
		  /* Update the INDICATOR and PAY_TERM field in PAYINFO */
		  res_val = fm_mso_update_bb_payinfo(ctxp, new_plan_type, pay_flistp, ebufp);
		  if(res_val)
		  {
			  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in Updating Payinfo->Indicator.");
			  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Change plan input flist",in_flist);
			  PIN_ERRBUF_CLEAR(ebufp);
			  *ret_flistp = PIN_FLIST_CREATE(ebufp);
			  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
			  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
			  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51819", ebufp);
			  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Payinfo->Indicator.", ebufp);
			  return ;		
		  }		
	  }

	  /* Fetch the bill when from input flist*/
	  bill_when = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_BILL_WHEN, 0, ebufp);
	  if(!bill_when || PIN_ERRBUF_IS_ERR(ebufp))
	  {
		  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Bill When not found.");
		  PIN_ERRBUF_CLEAR(ebufp);
		  *ret_flistp = PIN_FLIST_CREATE(ebufp);
		  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51821", ebufp);
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Bill When is mandatory with plan change.", ebufp);
		  return ;
	  }
	  /* Update the BILL_WHEN and INDICATOR field in MSO_FLD_BB_INFO */
	  fm_mso_update_service_bb_info(ctxp, ser_pdp, *bill_when, new_plan_type, ebufp);
	  if(res_val)
	  {
		  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in Updating Service->Bill When.");
		  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Change plan input flist",in_flist);
		  PIN_ERRBUF_CLEAR(ebufp);
		  *ret_flistp = PIN_FLIST_CREATE(ebufp);
		  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51820", ebufp);
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Updating Service->Bill When.", ebufp);
		  if(w_flistp)
			  PIN_FLIST_DESTROY_EX(&w_flistp, NULL);
		  if(wo_flistp)
			  PIN_FLIST_DESTROY_EX(&wo_flistp, NULL);
		  return ;		
	  }


	  /*
	     if((PIN_POID_COMPARE(cancel_plan_pdp, add_plan_pdp, 0, ebufp)) == 0)
	     {
	     r_flistp = PIN_FLIST_CREATE(ebufp);
	     PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	     PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
	     PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51757", ebufp);
	     PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Old plan and new plan are same. No need for change plan", ebufp);
	   *ret_flistp = r_flistp;
	   return;
	   }	
	   */
NOMATCH:
	  PIN_FLIST_DESTROY_EX(&temp_flistp, NULL);
	  PIN_POID_DESTROY(prod_pdp, NULL);
	  if ( set_err == 1 )
	  { 
		  PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan type does not match with customer type", ebufp);

		  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Change plan input flist",in_flist);
		  PIN_ERRBUF_RESET(ebufp);
		  *ret_flistp = PIN_FLIST_CREATE(ebufp);
		  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51818", ebufp);
		  PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plan type is not matching with Customer type", ebufp);
		  return ;
	  }
	  if(srch_out_flistp != NULL && srch_out_flistp)
		  PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	  return;
}


/*******************************************************
 * Function: fm_mso_get_mso_purplan()
 *	Returns the MSO_PURCHASED_PLAN poid and status in
 *	the output flist for account and plan obj passed
 *	input.
 *	Status to be passed -1 to ignore the status while
 *	searching purchased plan.
 *******************************************************/
void
fm_mso_get_mso_purplan(
		pcm_context_t           *ctxp,
		poid_t			*acc_pdp,
		poid_t                  *plan_obj,
		int			status,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp)
{

	pin_flist_t *search_flistp = NULL;
	pin_flist_t *search_res_flistp = NULL;
	pin_flist_t *args_flistp = NULL;
	pin_flist_t *result_flistp = NULL;
	pin_flist_t *updsvc_inflistp = NULL;
	pin_flist_t *updsvc_outflistp = NULL;

	poid_t		*search_pdp = NULL;
	poid_t		*mso_pp_obj = NULL;

	int		new_status = 0;
	int             search_flags = 512;
	int32           db = -1;
	char            template_1[100] = " select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
	char            template_2[100] = " select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 ";


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	db = PIN_POID_GET_DB(plan_obj);


	search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	search_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);		
	if(status == -1)
	{
		PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template_1, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template_2, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);
	}

	result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, (char *)NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, (char *)NULL, ebufp);
	PIN_FLIST_ELEM_SET(result_flistp, NULL, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_purplan search input list", search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching MSO Purchased plan", ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_purplan output flist", search_res_flistp);

	result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_purplan result flist", result_flistp);
	if ( !result_flistp )
	{
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NO Result");
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Assigning result flist to return flist");
	*ret_flistp = PIN_FLIST_COPY(result_flistp, ebufp); 
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After Assigning result flist to return flist");
	/*	
	 *ret_flistp = PIN_FLIST_CREATE(ebufp);
	 PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
	 PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_STATUS, *ret_flistp, PIN_FLD_STATUS, ebufp );
	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_purplan return flist", *ret_flistp);
	 */
	PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);

	return;
}	

/*******************************************************
 * Function: fm_mso_update_mso_purplan_status()
 *	Updates the MSO_PURCHASED_PLAN status with the 
 *	value of status passed in input.
 *	Return 1 for success and 0 for Failure
 *
 *******************************************************/
extern int
fm_mso_update_mso_purplan_status(
		pcm_context_t           *ctxp,
		poid_t                  *mso_purplan_obj,
		int			status,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t *wrtie_inflistp = NULL;
	pin_flist_t *wrtie_outflistp = NULL;
	poid_t		*mso_pp_obj = NULL;
	int32           db = -1;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;

	db = PIN_POID_GET_DB(mso_purplan_obj);

	wrtie_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(wrtie_inflistp, PIN_FLD_POID, mso_purplan_obj, ebufp);
	PIN_FLIST_FLD_SET(wrtie_inflistp, PIN_FLD_STATUS, &status, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_mso_purplan_status input list", wrtie_inflistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wrtie_inflistp, &wrtie_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&wrtie_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
		PIN_FLIST_DESTROY_EX(&wrtie_outflistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_mso_purplan_status output list", wrtie_outflistp);
	PIN_FLIST_DESTROY_EX(&wrtie_outflistp, NULL);

	return 1;
}	

/*******************************************************
 * Function: fm_mso_cust_get_prod_details()
 *	Fetch the priority and provisioning tag
 *	 for product poid passed in input.
 *
 *******************************************************/
static void
fm_mso_cust_get_prod_details(
		pcm_context_t           *ctxp,
		poid_t                  *product_obj,
		pin_flist_t				**ret_flistp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t				*read_flistp;
	pin_flist_t				*read_res_flistp;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error before calling fm_mso_cust_get_prod_details.");
		return ;
	}
	if ( !product_obj )
		return ;
	read_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, product_obj, ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_PRIORITY, (pin_decimal_t *)NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_PROVISIONING_TAG, (pin_decimal_t *)NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_prod_details read input list", read_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return ;
	}
	*ret_flistp = PIN_FLIST_COPY(read_res_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_prod_details read output flist", read_res_flistp);
}

/*******************************************************
 * Function: fm_mso_bb_get_payinfo_inv()
 *	Fetch the /payinfo/invoice for BB billinfo
 *	  and return Poid, Payterm and Indicator in 
 *		return flist.
 *******************************************************/
void
fm_mso_bb_get_payinfo_inv(
		pcm_context_t		*ctxp,
		poid_t			*acnt_pdp,
		pin_flist_t		**ret_flistpp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 768;
	int64			db = -1;
	char			*bi_bb = "BB";
	char			*template = "select X from /payinfo 1, /billinfo 2 where 2.F1 = V1 and 2.F2 = V2 and 2.F3 = 1.F4 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_bb_get_payinfo_inv", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_get_payinfo_inv acnt_pdp", acnt_pdp);

	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp , ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_ID, bi_bb , ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PAYINFO_OBJ, NULL , ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 4, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL , ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PAYMENT_TERM, NULL , ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PAY_TYPE, NULL , ebufp);
	arg_flist = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_INV_INFO, 0, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_INDICATOR, NULL , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_get_payinfo_inv search input", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_get_payinfo_inv search output", srch_out_flistp);
	arg_flist = NULL;
	arg_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	if(arg_flist) {
		*ret_flistpp = PIN_FLIST_COPY(arg_flist, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
}		

/*******************************************************
 * Function: fm_mso_update_bb_payinfo()
 *	Updates payinfo with new values of PAY_TERM
 *	  and INDICATOR. 
 *
 *******************************************************/
static int
fm_mso_update_bb_payinfo(
		pcm_context_t		*ctxp,
		int			indicator,
		pin_flist_t		*in_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*pay_in_listp = NULL;
	pin_flist_t		*pay_out_listp = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*inher_info = NULL;
	pin_flist_t		*inv_info = NULL;
	poid_t			*acct_obj = NULL;
	int				pay_term;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error before calling fm_mso_update_bb_payinfo", ebufp);
		return 1;
	}
	if(indicator == PAYINFO_BB_PREPAID) {
		pay_term = PAY_TERM_BB_PREPAID;
	} else {
		pay_term = PAY_TERM_BB_POSTPAID;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bb_payinfo ", in_flistp);
	acct_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);

	pay_in_listp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY( in_flistp, PIN_FLD_ACCOUNT_OBJ,  pay_in_listp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, pay_in_listp, PIN_FLD_PROGRAM_NAME, ebufp );

	payinfo = PIN_FLIST_ELEM_ADD(pay_in_listp, PIN_FLD_PAYINFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID,  payinfo, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PAY_TYPE,  payinfo, PIN_FLD_PAY_TYPE, ebufp );
	PIN_FLIST_FLD_SET(payinfo, PIN_FLD_PAYMENT_TERM,&pay_term, ebufp );

	inher_info = PIN_FLIST_SUBSTR_ADD(payinfo, PIN_FLD_INHERITED_INFO, ebufp);
	inv_info = PIN_FLIST_ELEM_ADD(inher_info, PIN_FLD_INV_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INDICATOR, &indicator, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set Payinfo Input flist ", pay_in_listp);
	PCM_OP(ctxp, PCM_OP_CUST_SET_PAYINFO, 0, pay_in_listp, &pay_out_listp, ebufp);
	PIN_FLIST_DESTROY_EX(&pay_in_listp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating payinfo invoice", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&pay_out_listp, NULL);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Set Payinfo output flist ", pay_out_listp);
	PIN_FLIST_DESTROY_EX(&pay_out_listp, NULL);
	return 0;
}

/*******************************************************
 * Function: fm_mso_update_mso_purplan()
 *	Updates /mso_purchased_plan with new status
 *	  and descr (plan_type) passed in input. 
 *
 *******************************************************/
int
fm_mso_update_mso_purplan(
		pcm_context_t		*ctxp,
		poid_t			*mso_pp_obj,
		int			new_status,
		char 			*descr,
		pin_errbuf_t            *ebufp)
{

	pin_flist_t *write_in_flistp = NULL;
	pin_flist_t *write_out_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 1;

	write_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(write_in_flistp, PIN_FLD_POID, mso_pp_obj, ebufp);
	PIN_FLIST_FLD_SET(write_in_flistp, PIN_FLD_STATUS, &new_status, ebufp);
	PIN_FLIST_FLD_SET(write_in_flistp, PIN_FLD_DESCR, descr, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_mso_purplan input flist", write_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_in_flistp, &write_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&write_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
		PIN_FLIST_DESTROY_EX(&write_out_flistp, NULL);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_mso_purplan output flist", write_out_flistp);
	PIN_FLIST_DESTROY_EX(&write_out_flistp, NULL);

	return 0;
}


/*******************************************************
 * Function: fm_mso_update_service_bb_info()
 *	Updates /service with new value of BILL_WHEN and 
 *	  INDICATOR passed in input. 
 *	If any of these values is passed as -1 then 
 *	the same is not updated
 *******************************************************/
static int
fm_mso_update_service_bb_info(
		pcm_context_t		*ctxp,
		poid_t				*ser_obj,
		int					bill_when,
		int 				indicator,
		pin_errbuf_t            *ebufp)
{

	pin_flist_t *ser_in_flistp = NULL;
	pin_flist_t *ser_out_flistp = NULL;
	pin_flist_t *tmp_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 1;

	ser_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(ser_in_flistp, PIN_FLD_POID, ser_obj, ebufp);
	tmp_flistp = PIN_FLIST_SUBSTR_ADD(ser_in_flistp, MSO_FLD_BB_INFO, ebufp);
	if(bill_when <=0 && indicator<0) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_update_service_bb_info: Nothing to Update. Return.");
		return 1;
	}
	//Pawan: Commented below since BillWhen is updated after cancelling the current plan
	//	to avoid rate plan error
	//if(bill_when>0) {
	//	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_BILL_WHEN, &bill_when, ebufp);
	//}
	if(indicator>=0) {
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_INDICATOR, &indicator, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_bb_info: write input", ser_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, ser_in_flistp, &ser_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ser_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
		PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_bb_info: write output", ser_out_flistp);
	PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
	return 0;
}


/*******************************************************
 * Function: fm_mso_update_ser_prov_status()
 *	Updates service provisioning status
 *	
 *	
 *******************************************************/
int
fm_mso_update_ser_prov_status(
		pcm_context_t	*ctxp,
		pin_flist_t	*in_flistp,
		int 		prov_status,
		pin_errbuf_t    *ebufp)
{

	pin_flist_t	*ser_in_flistp=NULL;
	pin_flist_t	*services_flistp=NULL;
	pin_flist_t	*ser_out_flistp=NULL;
	pin_flist_t	*inher_flistp=NULL;
	pin_flist_t	*tf_flistp=NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 1;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_ser_prov_status input flist", in_flistp);
	ser_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, ser_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, ser_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	services_flistp = PIN_FLIST_ELEM_ADD(ser_in_flistp, PIN_FLD_SERVICES, 0, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, services_flistp, PIN_FLD_POID, ebufp);

	inher_flistp = PIN_FLIST_SUBSTR_ADD(services_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	tf_flistp = PIN_FLIST_ELEM_ADD(inher_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
	PIN_FLIST_FLD_SET(tf_flistp, PIN_FLD_STATUS, &prov_status, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_ser_prov_status: Update service input", ser_in_flistp);
	PCM_OP(ctxp, PCM_OP_CUST_UPDATE_SERVICES, 0, ser_in_flistp, &ser_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ser_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_CUST_UPDATE_SERVICES", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_ser_prov_status: Update service output", ser_out_flistp);
	PIN_FLIST_DESTROY_EX(&ser_out_flistp, NULL);
	return 0;
}

/*******************************************************
 * Function: fm_mso_bb_get_change_type()
 *	Identifies the plan change type and returns
 *	0 : In case of Error
 *	Positive value which will be provisioning flag:
 *		
 *
 *
 *	
 *******************************************************/
static int
fm_mso_bb_get_change_type(
		pcm_context_t		*ctxp,
		pin_flist_t		*old_mso_pp_flistp,
		poid_t			*new_mso_pp_obj,
		pin_errbuf_t		*ebufp)

{
	pin_flist_t		*read_in_flistp=NULL;
	pin_flist_t		*read_out_flistp=NULL;
	pin_flist_t		*prod_flistp=NULL;
	pin_flist_t		*arg_flistp=NULL;
	pin_flist_t		*res_flistp=NULL;

	pin_cookie_t		cookie = NULL;
	poid_t			*srch_pdp = NULL;
	void			*vp = NULL;
	char			old_prov_tag[256];
	char			new_prov_tag[256];
	char			template[50] = "select X from /mso_config_bb_pt where F1 = V1 ";
	char			msg[100];
	int			elem_id = 0;
	int			srch_flag = 256;
	int			*old_plan_quota_flag =  NULL;
	int			*new_plan_quota_flag =  NULL;
	int64			db = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_get_change_type input flist", old_mso_pp_flistp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "New MSO PP Poid :",new_mso_pp_obj);

	/* Fetch the provisioning tag from old Purchased Plan flist */
	cookie=NULL;
	while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(old_mso_pp_flistp, PIN_FLD_PRODUCTS, 
					&elem_id, 1, &cookie, ebufp )) != NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"In product");
		vp = NULL;
		vp = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp );
		if(vp && strlen((char *)vp)>0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Old Prov Tag found");
			strcpy(old_prov_tag,(char *)vp);
			break;
		}
	}

	/* Read new purchased plan using the new_mso_pp_obj */
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, new_mso_pp_obj, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_get_change_type: Read input", read_in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_OBJ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_get_change_type: Read output", read_out_flistp);

	/* Fetch the provisioning tag for NEW Purchased Plan from output flist */
	cookie = NULL;
	while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(read_out_flistp, PIN_FLD_PRODUCTS, 
					&elem_id, 1, &cookie, ebufp )) != NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"In New product");
		vp = NULL;
		vp = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp );
		if(vp && strlen((char *)vp)>0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"New Prov Tag found");
			strcpy(new_prov_tag,(char *)vp);
			break;
		}
	}
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PROV Tags (old and new) are:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,old_prov_tag);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,new_prov_tag);

	/* Search /mso_config_bb_pt for OLD provisioning tag and fetch MSO_FLD_ISQUOTA_FLAG */
	db = PIN_POID_GET_DB(new_mso_pp_obj) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(read_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, old_prov_tag , ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_ISQUOTA_FLAG, NULL , ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_get_change_type: Search input", read_in_flistp);	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, read_in_flistp, &read_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_get_change_type: Search output", read_out_flistp);
	arg_flistp = PIN_FLIST_ELEM_GET(read_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	old_plan_quota_flag = PIN_FLIST_FLD_TAKE(arg_flistp, MSO_FLD_ISQUOTA_FLAG, 1, ebufp );

	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	/* Search /mso_config_bb_pt for NEW provisioning tag and fetch MSO_FLD_ISQUOTA_FLAG*/
	/* Just update the provisioning tag in same flist used earlier */
	arg_flistp = PIN_FLIST_ELEM_GET(read_in_flistp, PIN_FLD_ARGS, 1, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, new_prov_tag , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_get_change_type: Search input", read_in_flistp);	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, read_in_flistp, &read_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
		return 0;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_get_change_type: Search output", read_out_flistp);
	arg_flistp = PIN_FLIST_ELEM_GET(read_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	new_plan_quota_flag = PIN_FLIST_FLD_TAKE(arg_flistp, MSO_FLD_ISQUOTA_FLAG, 1, ebufp );
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);

	/*If any of 2 flags is not found then return error*/
	if(!old_plan_quota_flag || !new_plan_quota_flag)
	{
		pin_free(old_plan_quota_flag);
		old_plan_quota_flag = NULL;
		pin_free(new_plan_quota_flag);
		new_plan_quota_flag = NULL;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "ISQUOTA_FLAG not found for old or new plan");
		return 0;	
	}
	sprintf(msg , "Old Quota flag is: %d  and New Quota flag is: %d " , *old_plan_quota_flag,*new_plan_quota_flag);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	/* Compare Old plan Quota flag and New Plan Quota flag 
	   and return appropriate provisioning action value */

	if(*old_plan_quota_flag==0 && *new_plan_quota_flag==0)
	{
		pin_free(old_plan_quota_flag);
		old_plan_quota_flag = NULL;
		pin_free(new_plan_quota_flag);
		new_plan_quota_flag = NULL;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Provisioning action is DOC_BB_PC_NQUOTA_NQUOTA");
		return DOC_BB_PC_NQUOTA_NQUOTA;
	}
	else if(*old_plan_quota_flag==0 && *new_plan_quota_flag==1)
	{
		pin_free(old_plan_quota_flag);
		old_plan_quota_flag = NULL;
		pin_free(new_plan_quota_flag);
		new_plan_quota_flag = NULL;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Provisioning action is DOC_BB_PC_NQUOTA_QUOTA");
		return DOC_BB_PC_NQUOTA_QUOTA;
	}
	else if(*old_plan_quota_flag==1 && *new_plan_quota_flag==0)
	{
		pin_free(old_plan_quota_flag);
		old_plan_quota_flag = NULL;
		pin_free(new_plan_quota_flag);
		new_plan_quota_flag = NULL;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Provisioning action is DOC_BB_PC_QUOTA_NQUOTA");
		return DOC_BB_PC_QUOTA_NQUOTA;
	}
	else if(*old_plan_quota_flag==1 && *new_plan_quota_flag==1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Provisioning action is DOC_BB_PC_QUOTA_QUOTA");
		pin_free(old_plan_quota_flag);
		old_plan_quota_flag = NULL;
		pin_free(new_plan_quota_flag);
		new_plan_quota_flag = NULL;
		return DOC_BB_PC_QUOTA_QUOTA;
	}
	else
	{
		return 0;	
	}
	if(old_plan_quota_flag) {
		pin_free(old_plan_quota_flag);
		old_plan_quota_flag = NULL;
	}
	if(new_plan_quota_flag) {
		pin_free(new_plan_quota_flag);
		new_plan_quota_flag = NULL;
	}

}	


/*******************************************************
 * Function: fm_mso_add_prod_info()
 *		This function adds products details for each
 *		plan array in input. PIN_FLD_PRODUCTS is added
 *		for each product.
 *	
 *******************************************************/
void
fm_mso_add_prod_info(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
		pin_flist_t			*in_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*usage_map = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*acc_pdp = NULL;	
	int64			db = -1;
	int32			srch_flag = 256;
	char			*template = "select X from /product 1, /deal 2, /plan 3 where 3.F1 = V1 and 2.F2 = 3.F3 and 2.F4 = 1.F5 ";
	char			*prov_tag = NULL;
	char			*evt_type = NULL;
	int				elem_id = 0;
	int				p_elem_id = 0;
	void			*vp = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		p_cookie = NULL;
	int32			ret_val = 0;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_add_prod_info", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_prod_info input flist", in_flistp);
	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	db = PIN_POID_GET_DB(acc_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

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

	result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
	usage_map = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_USAGE_MAP, 0, ebufp );
	PIN_FLIST_FLD_SET(usage_map, PIN_FLD_EVENT_TYPE, NULL, ebufp);
	/* Loop through each plan array */
	cookie = NULL;
	while ((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN, 
					&elem_id, 1, &cookie, ebufp )) != NULL)
	{
		arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_COPY(plan_flistp, PIN_FLD_PLAN_OBJ, arg_flistp, PIN_FLD_POID, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Product Search input", srch_in_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan Product Search input", srch_out_flistp);
		result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp );
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No products found for this plan", ebufp);
			goto CLEANUP;
		}
		p_cookie = NULL;
		while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
						&p_elem_id, 1, &p_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_END_T, 1, ebufp);
			if(vp) {
				PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_PURCHASE_END_T, result_flistp, PIN_FLD_PURCHASE_END_T, ebufp);

			} else {
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
			}
			PIN_FLIST_FLD_COPY(plan_flistp,PIN_FLD_PURCHASE_START_T, result_flistp, PIN_FLD_PURCHASE_START_T, ebufp);
			prov_tag = NULL;
			prov_tag = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
			if(prov_tag && strlen(prov_tag)>0) {
				/*If product has prov tag then it means it is a subs product. Discount should be 
				  applied only on subscription product and not on grant product	*/

				PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_REF_NO, "PLANLIST", ebufp);
				ret_val = fm_mso_utils_discount_validation(ctxp, in_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_utils_discount_validation", ebufp);
				}
				else if(ret_val == 1)
				{
					pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE,
							MSO_FLD_DISC_AMOUNT, 0, 0, 0);
					goto CLEANUP;
				}

				PIN_FLIST_FLD_MOVE(plan_flistp,MSO_FLD_DISC_AMOUNT, result_flistp, MSO_FLD_DISC_AMOUNT, ebufp);
				PIN_FLIST_FLD_MOVE(plan_flistp,MSO_FLD_DISC_PERCENT, result_flistp, MSO_FLD_DISC_PERCENT, ebufp);
			}
			//Prov tag will not exist in case of Add plan -> FUP topup, ADD MB GB 
			else if(connp->coip->opcode ==MSO_OP_CUST_ADD_PLAN)
				//(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_ADD_PLAN))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called by ADD PLAN");
				usage_map = PIN_FLIST_ELEM_GET(result_flistp, PIN_FLD_USAGE_MAP, 0, 0, ebufp);
				evt_type = PIN_FLIST_FLD_GET(usage_map,PIN_FLD_EVENT_TYPE, 1, ebufp);
				if(evt_type && !strstr(evt_type,"mso_grant"))
				{
					//In case of add plan, if event type is not like "mso_grant" then
					// add the discount details in product.
					PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_REF_NO, "PLANLIST", ebufp);
					ret_val = fm_mso_utils_discount_validation(ctxp, in_flistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_utils_discount_validation", ebufp);
					}
					else if(ret_val == 1)
					{
						pin_set_err(ebufp, PIN_ERRLOC_FM,
								PIN_ERRCLASS_SYSTEM_DETERMINATE,
								MSO_FLD_DISC_AMOUNT, 0, 0, 0);
						goto CLEANUP;
					}
					PIN_FLIST_FLD_MOVE(plan_flistp,MSO_FLD_DISC_AMOUNT, result_flistp, MSO_FLD_DISC_AMOUNT, ebufp);
					PIN_FLIST_FLD_MOVE(plan_flistp,MSO_FLD_DISC_PERCENT, result_flistp, MSO_FLD_DISC_PERCENT, ebufp);
				}
			}
			PIN_FLIST_ELEM_DROP(result_flistp,PIN_FLD_USAGE_MAP, 0, ebufp);
			PIN_FLIST_FLD_DROP(result_flistp,PIN_FLD_PROVISIONING_TAG, ebufp);
			PIN_FLIST_FLD_RENAME(result_flistp,PIN_FLD_POID,PIN_FLD_PRODUCT_OBJ, ebufp);
			PIN_FLIST_ELEM_SET(plan_flistp, result_flistp, PIN_FLD_PRODUCTS, p_elem_id, ebufp);
		}
		vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_END_T, 1, ebufp);
		if(vp)
			PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_PURCHASE_END_T, ebufp);

		vp = PIN_FLIST_FLD_GET(plan_flistp,PIN_FLD_PURCHASE_START_T, 1, ebufp);
		if(vp)
			PIN_FLIST_FLD_DROP(plan_flistp,PIN_FLD_PURCHASE_START_T, ebufp);	
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After adding products", in_flistp);
CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
}

static	int
fm_mso_compare_ca_ids(
	pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t				*tmp_flistp = NULL;
	pin_flist_t				*from_plan_flistp = NULL;
	pin_flist_t				*to_plan_flistp = NULL;
	pin_flist_t				*flistp = NULL;
	pin_flist_t				*flistp_ca_id = NULL;
	pin_flist_t				*old_flistp = NULL;
	pin_flist_t				*old_flistp_ca_id = NULL;
	pin_flist_t 			*read_o_flistp = NULL;
	pin_flist_t 			*read_flistp = NULL;
	pin_flist_t 			*rflistp = NULL;


	void					*vp = NULL;
	char					msg[1024];
	char					func_name[] = "fm_mso_compare_ca_ids";
	char					*network_node = NULL;
	char					*new_network_node = NULL;
	char					*ca_id = NULL;
	char					*old_ca_id = NULL;
	char					*node = NULL;

	int32					compare_flag = 0;
	int32					rec_id = 0;	
	int32					rec_id1 = 0;
	pin_cookie_t			cookie = NULL;
	pin_cookie_t			cookie1 = NULL;
	int32					old_prd_count = 0;
	int32					new_prd_count = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing");	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		goto CLEANUP;
	}
	sprintf(msg,"%s: Compare CA_IDs", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_compare_ca_ids input list", i_flistp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1 ,ebufp);
	if (vp){
		read_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, (poid_t *)vp, ebufp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_o_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			sprintf(msg,"%s: Error before processing");	
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
			goto CLEANUP;
		}
		if (read_o_flistp){
			rflistp = PIN_FLIST_SUBSTR_GET(read_o_flistp, MSO_FLD_CATV_INFO, 0, ebufp);
			node = PIN_FLIST_FLD_GET(rflistp, MSO_FLD_NETWORK_NODE, 1 ,ebufp);
		}

	}

	//getting details of from_plan
	tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS, 0, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PLAN_OBJ, 1 ,ebufp);

	if (vp){
		fm_mso_get_ca_id_from_plan(ctxp, *ret_flistp, (poid_t *)vp, node, 0, &from_plan_flistp, ebufp);
	}

	//getting details of to_plan
	tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS, 1, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PLAN_OBJ, 1 ,ebufp);
	if (vp){
		fm_mso_get_ca_id_from_plan(ctxp, *ret_flistp, (poid_t *)vp, node, 1, &to_plan_flistp, ebufp);
	}
	sprintf(msg,"%s: From Plan flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, from_plan_flistp);
	sprintf(msg,"%s: To Plan flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, to_plan_flistp);

	old_prd_count = PIN_FLIST_ELEM_COUNT(from_plan_flistp, PIN_FLD_RESULTS,ebufp);
	new_prd_count = PIN_FLIST_ELEM_COUNT(to_plan_flistp, PIN_FLD_RESULTS,ebufp);

	if (old_prd_count > 0 && new_prd_count > 0 && old_prd_count == new_prd_count)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 1");
		if ( node )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 1.1");
			while ((flistp = PIN_FLIST_ELEM_GET_NEXT(to_plan_flistp,
							PIN_FLD_RESULTS, &rec_id, 1, &cookie,
							ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 2");
				compare_flag = 0;
				flistp_ca_id = PIN_FLIST_ELEM_GET(flistp, MSO_FLD_CATV_PT_DATA, PIN_ELEMID_ANY, 0, ebufp);
				if (flistp_ca_id != NULL)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 3");
					ca_id = PIN_FLIST_FLD_GET(flistp_ca_id, MSO_FLD_CA_ID, 1 ,ebufp);
					sprintf(msg,"%s: ca_id", ca_id);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					while ((old_flistp = PIN_FLIST_ELEM_GET_NEXT(from_plan_flistp,
									PIN_FLD_RESULTS, &rec_id1, 1, &cookie1,
									ebufp)) != (pin_flist_t *)NULL)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 4");
						old_flistp_ca_id = PIN_FLIST_ELEM_GET(old_flistp, MSO_FLD_CATV_PT_DATA, PIN_ELEMID_ANY, 0, ebufp);
						if (old_flistp_ca_id != NULL)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 5");
							old_ca_id = PIN_FLIST_FLD_GET(old_flistp_ca_id, MSO_FLD_CA_ID, 1 ,ebufp);
							sprintf(msg,"%s: old_ca_id", old_ca_id);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
							sprintf(msg,"%s: new_ca_id", ca_id);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
							/******************************************************
							 * NTO: Handle NA CA_ID
							 *****************************************************/
							if (strcmp(old_ca_id, "NA") != 0 && old_ca_id && ca_id && strcmp(ca_id,old_ca_id) == 0)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 6");
								compare_flag = 1;
								break;
							}
						}
					}
				}
				if (compare_flag == 0){
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 7");
					break;
				}
			}
		}
	}

CLEANUP:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"I am in Cleanup");
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_o_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&from_plan_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&to_plan_flistp, NULL);
	return compare_flag;
}

static	void
fm_mso_get_ca_id_from_plan(
	pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
	poid_t                  *mso_plan_obj,
	char                  	*mso_node,
	int32			change_flag,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t            *ebufp)
{

	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*tmp_flp = NULL;
	pin_flist_t		*out_flistp = NULL;
	poid_t			*pdp = NULL;

	char			msg[1024];
	int32			srch_flag = 768;
	char            template1 [] = "select X from /mso_cfg_catv_pt 1 , /product 2, /deal 3, /plan 4 where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = 4.F6 and 4.F7 = V7 and 1.F8 = V8 ";
	char            template2 [] = "select X from /mso_cfg_catv_pt 1, /mso_cfg_catv_pt_channel_map 2 where 1.F1 = 2.F2 and 2.F3 = V3 and 1.F4 = V4 ";
	char			func_name[] = "fm_mso_get_ca_id_from_plan";
	char			*descrp = NULL;
	int32			args_cnt = 1;
	int32			dpo_flags = 0;
	int64           db = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing");	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;
	}

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, mso_plan_obj, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_DESCR, NULL, ebufp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, srch_flistp, &srch_out_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	descrp = PIN_FLIST_FLD_GET(srch_out_flistp, PIN_FLD_DESCR, 0, ebufp);
	if (descrp && strstr(descrp, "DPO-"))
	{
		dpo_flags = 1;
		tmp_flp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS, 0, 1, ebufp);
		if (tmp_flp)
		{
			tmp_flp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_PLAN_LISTS, 1, ebufp);
			PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_PLAN_OBJ, mso_plan_obj, ebufp); 
			PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_FLAGS, &change_flag, ebufp);
		}
		else
		{
			tmp_flp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
			PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_PLAN_OBJ, mso_plan_obj, ebufp); 
			PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_FLAGS, &change_flag, ebufp);
		}
	}

	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	sprintf(msg,"%s: Prepare search flist to search CA_ID using PLAN", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1 ,ebufp);
	db = PIN_POID_GET_DB(pdp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	if (dpo_flags)
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2, ebufp);
	} 
	else
	{
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1, ebufp);
	}

	// Add ARGS array
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PROVISIONING_TAG,"",ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	if (dpo_flags)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(arg_flist, MSO_FLD_CATV_CHANNELS, PIN_ELEMID_ANY, ebufp);
	}
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PROVISIONING_TAG, "",ebufp);

	if (!dpo_flags)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
		PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID,NULL,ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
		tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,PIN_FLD_PRODUCTS,PIN_ELEMID_ANY,ebufp);
		PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
		PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID,NULL,ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
		tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,PIN_FLD_SERVICES,PIN_ELEMID_ANY,ebufp);
		PIN_FLIST_FLD_SET(tmp_flp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
	}

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	if (dpo_flags)
	{
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_PLAN_OBJ, mso_plan_obj, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, mso_plan_obj, ebufp);
	}

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	tmp_flp = PIN_FLIST_ELEM_ADD(arg_flist,MSO_FLD_CATV_PT_DATA,PIN_ELEMID_ANY,ebufp);
	PIN_FLIST_FLD_SET(tmp_flp, MSO_FLD_NETWORK_NODE, mso_node, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

	sprintf(msg, "%s: PCM_OP_SEARCH input flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,msg, srch_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		goto CLEANUP;
	}
	sprintf(msg,"%s: PCM_OP_SEARCH return flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, srch_out_flistp);
	*ret_flistp = PIN_FLIST_COPY(srch_out_flistp, ebufp);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
}

static int
fm_mso_cust_change_static_ip(
		pcm_context_t		*ctxp,
		pin_flist_t		*old_pp_flistp,
		poid_t			*new_mso_obj,
		char			*cmts_id,
		char			*prog_name,
		pin_errbuf_t		*ebufp)
{


	pin_flist_t		*read_in_flistp = NULL;
	pin_flist_t		*read_out_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*cc_flistp = NULL;
	pin_flist_t		*prod_flistp = NULL;
	pin_flist_t		*static_in_flistp = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*service_obj = NULL;
	char			*p_tag = NULL;
	char			*old_pool_name = NULL;
	char			*new_pool_name = NULL;
	char			prov_tag[255];
	char			old_prov_tag[255];
	char			*ip_add_list = NULL;
	int32			elem_id=0;
	int32			flags=512;
	int32			ip_cc_id = IP_POOL;
	int32			ip_change_req = 0;
	int32			ctr = 0;
	int64			db = 0;
	int32			ret_val = -1;
	int32			count = 0;
	pin_cookie_t		cookie = NULL;
	char			template1[255] = "select X from /mso_cfg_cmts_cc_mapping where F1 = V1 and F2 = V2 and F3 = V3 ";

	int			ip_flag = 0;
	char			*new_ip_add = NULL;
	char			*new_ip_add_list = NULL;
	pin_flist_t		*network_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_cust_change_static_ip", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_static_ip input flist", old_pp_flistp);
	if (!old_pp_flistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Old Purchased plan not found");
		return 1;
	}
	// Loop through the product array in input flist to find the Old prov tag
	memset(old_prov_tag, '\0', sizeof(prov_tag));
	while ( (prod_flistp = PIN_FLIST_ELEM_GET_NEXT (old_pp_flistp, 
					PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
	{
		p_tag = (char *)PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp );
		if(p_tag && strlen(p_tag)>0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, p_tag);
			strcpy(old_prov_tag,p_tag);
			break;
		}
	}

	memset(prov_tag, '\0', sizeof(prov_tag));
	account_obj = PIN_FLIST_FLD_GET(old_pp_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	service_obj = PIN_FLIST_FLD_GET(old_pp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(account_obj);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Acc Poid :", account_obj);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "MSO PP obj :", new_mso_obj);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, cmts_id);

	// Read MSO PP object
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_in_flistp,PIN_FLD_POID, new_mso_obj, ebufp );  
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0,read_in_flistp, &read_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read MSO PP object output flist", read_out_flistp);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);

	plan_obj = PIN_FLIST_FLD_TAKE(read_out_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp );
	// Loop through the product array in input flist to find the prov tag
	cookie = NULL;
	while ( (prod_flistp = PIN_FLIST_ELEM_GET_NEXT (read_out_flistp, 
					PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL )
	{
		p_tag = (char *)PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp );
		if(p_tag && strlen(p_tag)>0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, p_tag);
			strcpy(prov_tag,p_tag);
			break;
		}
	}
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);

	//Fetch the IP pool name for Old prov tag
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template1, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &flags, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_CMTS_ID, cmts_id, ebufp );
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	cc_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CC_MAPPING, 0, ebufp);
	PIN_FLIST_FLD_SET(cc_flistp, MSO_FLD_CLIENT_CLASS_ID, &ip_cc_id, ebufp );
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp);
	cc_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CC_MAPPING, 0, ebufp);
	PIN_FLIST_FLD_SET(cc_flistp, PIN_FLD_PROVISIONING_TAG, old_prov_tag, ebufp );
	PIN_FLIST_ELEM_SET(srch_in_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search CC map (old prov tag) input flist :", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search CC map (old prov tag) output flist :", srch_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_change_static_ip search CC map error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) > 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CC search return more than 1 result. Invalid Configuration");
		return 1;
	}
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CC search return 0 result. Invalid Configuration");
		return 1;
	}
	result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	cc_flistp = PIN_FLIST_ELEM_GET(result_flistp, MSO_FLD_CC_MAPPING, PIN_ELEMID_ANY, 0, ebufp);
	old_pool_name = PIN_FLIST_FLD_TAKE(cc_flistp, MSO_FLD_CLIENT_CLASS_VALUE, 0, ebufp );
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	// Fetch the IP pool name for New prov tag
	// Use the same input flist used in above search.
	arg_flistp = PIN_FLIST_ELEM_GET(srch_in_flistp, PIN_FLD_ARGS, 3, 0, ebufp);
	cc_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CC_MAPPING, 0, ebufp);
	PIN_FLIST_FLD_SET(cc_flistp, PIN_FLD_PROVISIONING_TAG, prov_tag, ebufp )

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search New CC map input flist :", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search New CC map output flist :", srch_out_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_change_static_ip search CC map error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) > 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CC map search return more than 1 result. Invalid Configuration");
		return 1;
	}
	if(PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CC search return 0 result. Invalid Configuration");
		return 1;
	}
	result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	cc_flistp = PIN_FLIST_ELEM_GET(result_flistp, MSO_FLD_CC_MAPPING, PIN_ELEMID_ANY, 0, ebufp);
	new_pool_name = PIN_FLIST_FLD_TAKE(cc_flistp, MSO_FLD_CLIENT_CLASS_VALUE, 0, ebufp );
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

	// Compare the old and new pool name
	if(old_pool_name && new_pool_name)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, old_pool_name);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, new_pool_name);
		if(strcmp(new_pool_name,old_pool_name)==0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Old and new pool name are same. No need to change static IP.");
			pin_free(old_pool_name);
			old_pool_name = NULL;
			return 2;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Old and new pool name different. Change IP");
			ip_change_req = 1;
			pin_free(old_pool_name);
			old_pool_name = NULL;
		}
	}
	else
	{
		if(old_pool_name){
			pin_free(old_pool_name);
			old_pool_name = NULL;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "IP Pool name is NULL");
		return 1;
	}

	//IP pool is different so Static IP must be changed.
	if(ip_change_req)
	{
		ret_val = fm_mso_cust_release_service_ip(ctxp, account_obj, service_obj, prog_name, &count, &ip_add_list, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "IP Add List");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, ip_add_list);
		if(ret_val == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error in releasing existing IP.");
			return 1;
		}
		// Purchase the same number of Static IPs which service has.
		for(ctr=0;ctr<count;ctr++)
		{
			srch_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchase Static IP input flist",srch_in_flistp);
			ret_val = fm_mso_purchase_static_ip(ctxp, srch_in_flistp, MSO_ADD_DEVICE, ebufp );

			if ( ret_val == 1 ||  ret_val == 2)
			{
				return 1;
			}
			else if ( ret_val == 0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Static IP device associated successfully");
				/********* Pavan Bellala 07-09-2015 ********************
				  Keep the new IP address to pass into provisioning flist.
				 *******************************************************/
				new_ip_add = PIN_FLIST_FLD_GET(srch_in_flistp, MSO_FLD_NETWORKID, 1, ebufp);
				if(new_ip_add != NULL)
				{
					if(ip_flag == 1)
					{
						new_ip_add_list = (char *)realloc(new_ip_add_list,(strlen(new_ip_add_list)+strlen(new_ip_add))+2);
						strcat(new_ip_add_list, ",");
						strcat(new_ip_add_list, new_ip_add);
					}
					else{
						new_ip_add_list = (char*)malloc(strlen(new_ip_add)+1);
						strcpy(new_ip_add_list, new_ip_add);
						ip_flag = 1;
					}			
				}

			}

			PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
		}

		network_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(network_flistp,MSO_FLD_NETWORKID,new_ip_add_list,ebufp);
		PIN_FLIST_FLD_PUT(network_flistp,MSO_FLD_OLD_NETWORKID,ip_add_list,ebufp);
		PIN_FLIST_ELEM_PUT(old_pp_flistp,network_flistp,PIN_FLD_IP_ADDRESSES,0,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_cust_change_static_ip: modified input flistp",old_pp_flistp);
	}
	//PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_POID_DESTROY(plan_obj, NULL);
	return 0;
}

void
fm_mso_utils_filter_non_paying_children(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flistp,
		pin_flist_t             **o_flistpp,
		pin_errbuf_t            *ebufp)
{

	pin_flist_t	*result_flistp = NULL;
	pin_flist_t	*members_flistp = NULL;
	pin_flist_t	*m_flistp = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*po_flistp = NULL;
	pin_flist_t	*so_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;

	int64		db = 1;
	int32		elem_id = 0;
	int32		elem_id1= 0;
	int		i = 0;
	int		*pay_typep = NULL;
	int		srch_flag = SRCH_DISTINCT;

	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie1 = NULL;

	poid_t		*member_pdp = NULL;
	poid_t		*parent_pdp = NULL;
	char		*billinfo_template1 = "select X from /billinfo where F1 = V1 ";


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error before calling fm_mso_utils_filter_non_paying_children", ebufp);
		return;
	}
	*o_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_filter_non_paying_children :input ", in_flistp);

	while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_RESULTS,
					&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		parent_pdp = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_PARENT,1,ebufp);
		db = PIN_POID_GET_DB(parent_pdp);
		elem_id1 = 0;
		cookie1 = NULL;
		while((members_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, PIN_FLD_MEMBERS,
						&elem_id1,1,&cookie1,ebufp))!=(pin_flist_t *)NULL)
		{
			member_pdp = PIN_FLIST_FLD_GET(members_flistp,PIN_FLD_OBJECT,1,ebufp);
			if(!PIN_POID_IS_NULL(member_pdp))
			{
				s_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
				PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
				PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, billinfo_template1 , ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS,1,ebufp);
				PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,member_pdp,ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search billinfo member input", s_flistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &po_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search billinfo member output", po_flistp);
				PIN_FLIST_DESTROY_EX(&s_flistp,NULL);

				if(po_flistp && PIN_FLIST_ELEM_COUNT(po_flistp,PIN_FLD_RESULTS,ebufp)>0)
				{
					res_flistp = PIN_FLIST_ELEM_GET(po_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
					pay_typep = PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_PAY_TYPE,1,ebufp);
					if(pay_typep && (*pay_typep == PIN_PAY_TYPE_SUBORD))
					{
						m_flistp = PIN_FLIST_ELEM_ADD(*o_flistpp,PIN_FLD_MEMBERS,i,ebufp);
						PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_ACCOUNT_OBJ,m_flistp,PIN_FLD_OBJECT,ebufp);
						i++;
					}
				}
				PIN_FLIST_DESTROY_EX(&po_flistp,NULL);
			}									
		}
		PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_PARENT,parent_pdp,ebufp);			
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_filter_non_paying_children output", *o_flistpp);
}

void
fm_cust_chng_plan_validate_hierarchy(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flistp,
		//int                     *in_indicatorp,
		pin_flist_t             **o_flistpp,
		pin_errbuf_t            *ebufp)

{
	pin_flist_t     *s_flistp = NULL;
	pin_flist_t     *args_flistp  = NULL;
	pin_flist_t     *tmp_flistp = NULL;
	pin_flist_t     *so_flistp = NULL;
	pin_flist_t     *parent_flistp = NULL;
	pin_flist_t     *siblings_flistp = NULL;
	pin_flist_t     *members_flistp = NULL;

	void            *vp = NULL;
	int             pay_type = 0;
	int64           db = 0;
	poid_t          *p_pdp = NULL;
	poid_t          *svc_pdp = NULL;
	poid_t          *acct_pdp = NULL;
	int             valid = PIN_BOOLEAN_TRUE;
	int             nonpaying_member_count = 0;
	int		flags = -1;
	int		srch_flag = SRCH_DISTINCT;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error before calling fm_cust_chng_plan_validate_hierarchy", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_chng_plan_validate_hierarchy :input ", in_flistp);
	/*
	   if(in_indicatorp != NULL)
	   {
	   child_indicator = *(int*)in_indicatorp;
	   }
	 */
	/***********************************************
	  Fetch all parents on which this child is dependent
	 *************************************************/

	*o_flistpp = PIN_FLIST_CREATE(ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_ACCOUNT_OBJ,1,ebufp);
	if(!PIN_POID_IS_NULL(acct_pdp)){
		db = PIN_POID_GET_DB(acct_pdp);
	}else {
		db = 1;
	}

	/********************************************
	  Fetch the account pay type from /billinfo
	 ********************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /billinfo where F1 = V1 " , ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS,1,ebufp);
	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,acct_pdp,ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search billinfo input", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search billinfo output", so_flistp);
	PIN_FLIST_DESTROY_EX(&s_flistp,NULL);
	if(so_flistp && PIN_FLIST_ELEM_COUNT(so_flistp,PIN_FLD_RESULTS,ebufp)>0)
	{
		args_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,0,ebufp);
		vp = PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_PAY_TYPE,1,ebufp);
		if(vp) pay_type = *(int*)vp;

	} else {
		valid =  PIN_BOOLEAN_FALSE;
		PIN_FLIST_DESTROY_EX(&so_flistp,NULL);
		PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_RESULT_FLAGS,&valid,ebufp);
		return;
	}
	PIN_FLIST_DESTROY_EX(&so_flistp,NULL);
	if(pay_type == PIN_PAY_TYPE_SUBORD)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account under process is non-paying child");
		flags = 0;
		fm_utils_fetch_group_details(ctxp,acct_pdp,&flags,&siblings_flistp,ebufp);
		if(siblings_flistp != NULL)
		{
			/**** Filter siblings which are non-paying *****/
			fm_mso_utils_filter_non_paying_children(ctxp,siblings_flistp,&members_flistp,ebufp);
			if(members_flistp!= NULL)
			{
				nonpaying_member_count = PIN_FLIST_ELEM_COUNT(members_flistp,PIN_FLD_MEMBERS,ebufp);
			}
			if(nonpaying_member_count > 1 ) /*****One member is account under process****/
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Has dependent siblings. Cannot change plan");
				valid = PIN_BOOLEAN_FALSE;
				PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_RESULT_FLAGS,&valid,ebufp);
				PIN_FLIST_DESTROY_EX(&siblings_flistp,NULL);
				PIN_FLIST_DESTROY_EX(&members_flistp,NULL);
				return;
			} else if(nonpaying_member_count == 1){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account under process is only dependent child");
				p_pdp = PIN_FLIST_FLD_GET(members_flistp,PIN_FLD_PARENT,1,ebufp);
				fm_utils_fetch_acct_svc_details(ctxp,p_pdp,&parent_flistp,ebufp);
				if(parent_flistp != NULL)
				{
					svc_pdp = PIN_FLIST_FLD_GET(parent_flistp,PIN_FLD_SERVICE_OBJ,1,ebufp);
					if(!PIN_POID_IS_NULL(svc_pdp))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Parent has service,Cannot change plan for child");
						valid = PIN_BOOLEAN_FALSE;
						PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_RESULT_FLAGS,&valid,ebufp);
						PIN_FLIST_DESTROY_EX(&parent_flistp,NULL);
						return;
					} else {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Parent has not service,Hence change plan valid for only child");
						valid = PIN_BOOLEAN_TRUE;
						PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_RESULT_FLAGS,&valid,ebufp);
						PIN_FLIST_CONCAT(*o_flistpp,parent_flistp,ebufp);
						return;
					}

				}

			}	

		} else {
			valid = PIN_BOOLEAN_FALSE;
			PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_RESULT_FLAGS,&valid,ebufp);
			return;
		}

	} else if(pay_type == PIN_PAY_TYPE_INVOICE){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account under process is parent or self paying child");
		flags = 0;
		fm_utils_fetch_group_details(ctxp,acct_pdp,&flags,&siblings_flistp,ebufp);
		if((siblings_flistp != NULL)&&(PIN_FLIST_ELEM_COUNT(siblings_flistp,PIN_FLD_RESULTS,ebufp)>0))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account under process is self paying child");
			args_flistp = PIN_FLIST_ELEM_GET(siblings_flistp,PIN_FLD_RESULTS,0,1,ebufp);
			p_pdp = PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_PARENT,1,ebufp);
			if(p_pdp && acct_pdp)
			{
				if(PIN_POID_COMPARE(p_pdp,acct_pdp,0,ebufp)!=0)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account under process is self paying child");
					valid = PIN_BOOLEAN_TRUE;
					PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_RESULT_FLAGS,&valid,ebufp);
					PIN_FLIST_DESTROY_EX(&siblings_flistp,NULL);
					return;
				}
			}

		}else if((siblings_flistp != NULL)&&(PIN_FLIST_ELEM_COUNT(siblings_flistp,PIN_FLD_RESULTS,ebufp)==0)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account under process is not child");
			flags = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Search /group as parent");
			fm_utils_fetch_group_details(ctxp,acct_pdp,&flags,&siblings_flistp,ebufp);
			if((siblings_flistp!= NULL)&& (PIN_FLIST_ELEM_COUNT(siblings_flistp,PIN_FLD_RESULTS,ebufp)>0)){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account is parent");
				fm_mso_utils_filter_non_paying_children(ctxp,siblings_flistp,&members_flistp,ebufp);
				if(members_flistp!= NULL)
				{
					nonpaying_member_count = PIN_FLIST_ELEM_COUNT(members_flistp,PIN_FLD_MEMBERS,ebufp);
				}					
				if(nonpaying_member_count ==0){
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Parent has all independent children");
					valid = PIN_BOOLEAN_TRUE;
					PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_RESULT_FLAGS,&valid,ebufp);
					PIN_FLIST_DESTROY_EX(&members_flistp,NULL);
					PIN_FLIST_DESTROY_EX(&siblings_flistp,NULL);
					return;
				} else if(nonpaying_member_count > 0){
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account is parent with dependent children");
					valid = PIN_BOOLEAN_FALSE;
					PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_RESULT_FLAGS,&valid,ebufp);
					PIN_FLIST_DESTROY_EX(&members_flistp,NULL);
					PIN_FLIST_DESTROY_EX(&siblings_flistp,NULL);
					return;
				}
			} else {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Account not under hierarchy");
				valid = PIN_BOOLEAN_TRUE;
				PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_RESULT_FLAGS,&valid,ebufp);
				PIN_FLIST_DESTROY_EX(&siblings_flistp,NULL);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_chng_plan_validate_hierarchy output", *o_flistpp);
				return;
			}
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_chng_plan_validate_hierarchy output", *o_flistpp);
}





void
fm_utils_fetch_group_details(
		pcm_context_t           *ctxp,
		poid_t	                *in_pdp,
		int			*flags,
		pin_flist_t             **o_flistpp,	
		pin_errbuf_t            *ebufp)

{

	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*so_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;

	char		*s_template = "select X from /group/billing where F1 = V1 ";
	int		srch_flag = SRCH_DISTINCT;
	int64		db = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error before calling fm_utils_fetch_group_details", ebufp);
		return;
	}

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"fm_utils_fetch_group_details : input",in_pdp);
	if(!PIN_POID_IS_NULL(in_pdp)){
		db = PIN_POID_GET_DB(in_pdp);
	}else {
		db = 1;
	}
	s_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template , ebufp);

	if(*flags == 0)
	{
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS,1,ebufp);
		tmp_flistp  = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_MEMBERS,PIN_ELEMID_ANY,ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp,PIN_FLD_OBJECT,in_pdp,ebufp);
	} else if (*flags == 1){
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS,1,ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_PARENT,in_pdp,ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search group billing input", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search group billing output", so_flistp);
	PIN_FLIST_DESTROY_EX(&s_flistp,NULL);
	//if(so_flistp && PIN_FLIST_ELEM_COUNT(so_flistp,PIN_FLD_RESULTS,ebufp)>0)
	//{
	//*o_flistpp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_RESULTS,0,1,ebufp);
	*o_flistpp = so_flistp;
	//}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_utils_fetch_group_details: return",*o_flistpp);
}





void
fm_utils_fetch_acct_svc_details(
		pcm_context_t           *ctxp,
		poid_t	                *in_pdp,
		pin_flist_t             **o_flistpp,	
		pin_errbuf_t            *ebufp)

{

	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*so_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;

	char		*s_template = "select X from /billinfo where F1 = V1 ";
	int		srch_flag = SRCH_DISTINCT;
	int64		db = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error before calling fm_utils_fetch_acct_svc_details", ebufp);
		return;
	}

	if(!PIN_POID_IS_NULL(in_pdp)){
		db = PIN_POID_GET_DB(in_pdp);
	}else {
		db = 1;
	}

	*o_flistpp = PIN_FLIST_CREATE(ebufp);

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"fm_utils_fetch_acct_svc_details : input",in_pdp);

	/********************************************
	  Search from /billinfo
	 ********************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template , ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS,1,ebufp);
	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,in_pdp,ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_utils_fetch_acct_svc_details:search billinfo input", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_utils_fetch_acct_svc_details:search billinfo output", so_flistp);
	PIN_FLIST_DESTROY_EX(&s_flistp,NULL);
	if(so_flistp && PIN_FLIST_ELEM_COUNT(so_flistp,PIN_FLD_RESULTS,ebufp)>0)
	{
		args_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_RESULTS,0,1,ebufp);
		if(args_flistp!=NULL)
		{
			PIN_FLIST_FLD_MOVE(args_flistp,PIN_FLD_PAYINFO_OBJ,*o_flistpp,PIN_FLD_POID,ebufp);
		}
	}
	PIN_FLIST_DESTROY_EX(&so_flistp,NULL);

	/********************************************
	  Search from /service
	 ********************************************/

	s_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,
			"select X from /service where F1 = V1 ", ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS,1,ebufp);
	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,in_pdp,ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_utils_fetch_acct_svc_details:search service input", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &so_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_utils_fetch_acct_svc_details:search service output", so_flistp);
	PIN_FLIST_DESTROY_EX(&s_flistp,NULL);
	if(so_flistp && PIN_FLIST_ELEM_COUNT(so_flistp,PIN_FLD_RESULTS,ebufp)>0)
	{
		args_flistp = PIN_FLIST_ELEM_GET(so_flistp,PIN_FLD_RESULTS,0,1,ebufp);
		if(args_flistp!=NULL)
		{
			PIN_FLIST_FLD_MOVE(args_flistp,PIN_FLD_POID,*o_flistpp,PIN_FLD_SERVICE_OBJ,ebufp);
		}
	}
	PIN_FLIST_DESTROY_EX(&so_flistp,NULL);
	PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_ACCOUNT_OBJ,in_pdp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_utils_fetch_acct_svc_details: return flist",*o_flistpp);

}


static int 
fm_mso_change_amc_plan(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flist,
		int32			old_plan_type,
		int32			new_plan_type,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp )
{
	pin_flist_t	*amc_flistp = NULL;
	pin_flist_t	*amc_out_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
        pin_flist_t     *ppf_flistp  = NULL;
        pin_flist_t     *pr_ret_flistp  = NULL;

	poid_t		*acc_obj = NULL;
	poid_t		*svc_obj = NULL;
	poid_t		*user_obj = NULL;
	poid_t          *pr_poid  = NULL;
	int32		amc_flag = -1;
	int32		mode = 0;
	char		msg[256] = {"\0"};
	void		*vp = NULL;
	int32   count =0;
	int32		elem_id = 0;
	pin_cookie_t	cookie = NULL;
	char		*event_type = NULL;

        pin_decimal_t   *pd_priority = NULL;
        char            *strp = NULL;
        int32           prty =0;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 1;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_amc_plan input flist", in_flist);

	acc_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp );
	svc_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 0, ebufp );
	user_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_USERID, 0, ebufp );
	vp =  PIN_FLIST_FLD_GET(in_flist, PIN_FLD_MODE, 1, ebufp );
	if(vp) {
		mode = *((int32 *)vp);
	}

	sprintf(msg, "old_plan_type=%d and new_plan_type=%d",old_plan_type,new_plan_type);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

	// Based on old_plan_type and new_plan_type value find the amc_flag value
	if(old_plan_type==PAYINFO_BB_PREPAID && new_plan_type==PAYINFO_BB_PREPAID) {
		amc_flag = AMC_ON_CHANGE_PRE_PRE;
	}
	else if(old_plan_type==PAYINFO_BB_PREPAID && new_plan_type==PAYINFO_BB_POSTPAID) {
		amc_flag = AMC_ON_CHANGE_PRE_POST;
	}
	else if(old_plan_type==PAYINFO_BB_POSTPAID && new_plan_type==PAYINFO_BB_PREPAID) {
		amc_flag = AMC_ON_CHANGE_POST_PRE;
	}
	else if(old_plan_type==PAYINFO_BB_POSTPAID && new_plan_type==PAYINFO_BB_POSTPAID) {
		amc_flag = AMC_ON_CHANGE_POST_POST;
	}

	sprintf(msg, "AMC flag = %d",amc_flag);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

	//If amc_flag valus is -1 then AMC call is not required.
	if(amc_flag == -1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Old/new Payinfo Indicator value not correct.");
		return 1;
	}

	//Call AMC opcode
	amc_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_POID, acc_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_SERVICE_OBJ, svc_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_USERID, user_obj, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_FLAGS, &amc_flag, ebufp);
	PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_MODE, &mode, ebufp);

	while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(*ret_flistp, PIN_FLD_RESULTS, 
					&elem_id, 1, &cookie, ebufp)) != NULL)
	{	
                 //Dont copy Tal IP  plan details for AMC
                ppf_flistp = PIN_FLIST_SUBSTR_GET(tmp_flistp, PIN_FLD_PRODUCT, 1,ebufp);
                if(ppf_flistp) {
                        pr_poid = PIN_FLIST_FLD_GET(ppf_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp );
                        fm_mso_get_poid_details(ctxp,pr_poid,&pr_ret_flistp,ebufp);
                        pd_priority = PIN_FLIST_FLD_GET(pr_ret_flistp, PIN_FLD_PRIORITY, 1, ebufp );
                        strp = pbo_decimal_to_str(pd_priority, ebufp);
                        prty = atoi(strp);
                }
                if (!( prty > 5000 && prty < 6000 )) {

                PIN_FLIST_ELEM_SET(amc_flistp, tmp_flistp, PIN_FLD_RESULTS, elem_id, ebufp);
                }
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC input", amc_flistp);
	PCM_OP(ctxp, MSO_OP_CUST_BB_HW_AMC, 0, amc_flistp, &amc_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_BB_HW_AMC", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		return 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC output", amc_out_flistp);

	// If Change plan is called by price calculator then copy the results array 
	// from AMC return flist to return flist of function.
	if(mode==1)
	{
		args_flistp = PIN_FLIST_ELEM_GET(*ret_flistp,PIN_FLD_DATA_ARRAY,0,1,ebufp);
		if(args_flistp) 
		{
			count = PIN_FLIST_ELEM_COUNT(args_flistp, PIN_FLD_RESULTS, ebufp);
			cookie = NULL;
            elem_id = 0;
			while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(amc_out_flistp, PIN_FLD_RESULTS, 
							&elem_id, 1, &cookie, ebufp)) != NULL)
			{
				event_type = (char *) PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 1, ebufp ));
				if(event_type && strstr(event_type, "mso_hw_amc"))
				{	
					PIN_FLIST_ELEM_SET(args_flistp, tmp_flistp, PIN_FLD_RESULTS, count, ebufp);
					count++;
				}
			}
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_amc_plan return flist", *ret_flistp);
	PIN_FLIST_DESTROY_EX(&amc_flistp, NULL);
	//PIN_FLIST_DESTROY_EX(&amc_out_flistp, NULL);
	return 0;
}




void
mso_get_additional_products(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flistp,
		pin_errbuf_t            *ebufp)
{

	poid_t	*account_pdp = NULL;
	poid_t	*product_pdp = NULL;

	pin_flist_t	*plan_list_flistp = NULL;
	pin_flist_t	*search_inflistp = NULL;
	pin_flist_t	*search_outflistp = NULL;
	pin_flist_t	*args_flistp  = NULL;
	pin_flist_t	*read_iflist = NULL;
	pin_flist_t	*read_oflist = NULL;
	pin_flist_t	*deal_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*d_flistp = NULL;

	int		act_status = 1;
	int64		db = 1;
	int		deal_cnt = 0;
	int		prty = 0;
	int		mod_prty = 0;
	int		flags = 256;
	int		elem_pp = 0;
	pin_decimal_t	*priority = NULL;

	char		*tmp_str = NULL;

	char		*search_template = "select X from /purchased_product where F1 = V1 and F2 = V2 ";
	pin_cookie_t	cookie_pp = NULL;
	pin_cookie_t	d_cookie = NULL;	

	int		d_elemid = 0;
	int		match_found = 0;

	int		*d_packageid = NULL;
	int		*r_packageid = NULL;
	char                    msg[100];

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_get_additional_products:input flist",in_flistp);

	account_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	db = PIN_POID_GET_DB(account_pdp);

	plan_list_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN_LISTS,1, 0, ebufp);

	deal_cnt = PIN_FLIST_ELEM_COUNT(plan_list_flistp,PIN_FLD_DEALS, ebufp);

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Error in input flist add plan error", in_flistp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in input flist add plan error", ebufp);
		return;

	}
	search_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ,account_pdp , ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search purchased product in", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search purchased product out", search_outflistp);

	PIN_FLIST_DESTROY_EX(&search_inflistp,NULL);

	if(search_outflistp)
	{
		while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS, 
						&elem_pp, 1, &cookie_pp, ebufp )) != NULL)
		{
			product_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
			r_packageid = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PACKAGE_ID, 1, ebufp);
			read_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_POID , product_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_PRIORITY , NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read product input flist", read_iflist);
			PCM_OP(ctxp,PCM_OP_READ_FLDS, 0 , read_iflist , &read_oflist , ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read product output flist", read_oflist);

			if(read_oflist)
			{
				priority = PIN_FLIST_FLD_GET(read_oflist, PIN_FLD_PRIORITY, 1, ebufp );
				if(priority)
					tmp_str = pbo_decimal_to_str(priority, ebufp);
				if(tmp_str)
					prty = atoi(tmp_str);
				if(prty)
					mod_prty = prty%1000;
				if (tmp_str) {
					pin_free(tmp_str);
					tmp_str = NULL;
				}
				if((BB_ADD_MB_START <= mod_prty && BB_ADD_MB_END >= mod_prty)||
						(BB_FUP_START <= mod_prty && BB_FUP_START >= mod_prty))
				{
					d_elemid = 0;
					d_cookie = NULL;
					match_found = 0;
					/**** Check if already added ****/
					while (( d_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_list_flistp, PIN_FLD_DEALS, 		
									&d_elemid, 1, &d_cookie, ebufp))!= NULL)
					{
						d_packageid = PIN_FLIST_FLD_GET(d_flistp,PIN_FLD_PACKAGE_ID,1,ebufp);
						if(r_packageid && d_packageid && (*r_packageid == *d_packageid))
						{
							match_found = 1;
							PIN_FLIST_FLD_SET(plan_list_flistp, PIN_FLD_STATUS, &match_found, ebufp);
							break;
						}
					}	
					if(match_found == 0)
					{	
						deal_flistp = PIN_FLIST_ELEM_ADD( plan_list_flistp, PIN_FLD_DEALS, deal_cnt++, ebufp);
						PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_PLAN_OBJ, deal_flistp,PIN_FLD_PLAN_OBJ,ebufp);
						PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_DEAL_OBJ, deal_flistp,PIN_FLD_DEAL_OBJ,ebufp);
						PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_PACKAGE_ID, deal_flistp,PIN_FLD_PACKAGE_ID,ebufp);
					}
				}

			}

			PIN_FLIST_DESTROY_EX(&read_iflist,NULL);
			PIN_FLIST_DESTROY_EX(&read_oflist,NULL);

		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"plan flist", plan_list_flistp);

	}

	PIN_FLIST_DESTROY_EX(&search_outflistp,NULL);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Modified input flist", in_flistp);

}

// Added for Validating the FDP Packs - Change Plan refunding ET for old plan
void
fm_mso_validate_fdp_et_align(
		pcm_context_t           *ctxp,
		u_int			flags,
		pin_flist_t             *i_flistp,
		pin_flist_t             **ret_flistpp, //CATV_REFUND_API_CHANGE
		pin_errbuf_t            *ebufp)
{
	pin_flist_t		*plan_list_flistp = NULL;
	pin_flist_t		*deal_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*et_in_flistp = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*rd_inv_info = NULL;
	pin_flist_t		*read_flds_iflistp = NULL;
	pin_flist_t		*read_flds_oflistp = NULL;
	pin_flist_t		*write_flds_inflistp = NULL;
	pin_flist_t		*write_flds_outflistp = NULL;
	pin_flist_t		*pdt_flistp = NULL;
	//CATV_REFUND_API_CHANGE
	pin_flist_t             *et_out_flistp = NULL;
	pin_flist_t             *tmp_flistp = NULL;
	pin_flist_t             *bal_imp_flistp = NULL;

	poid_t			*payinfo_obj = NULL;
	poid_t			*deal_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*pdt_pdp = NULL;
	int32			is_base_fdp_pack = -1;
	int32			*pay_indicator = NULL;

	int             *status = NULL;
	int32             *res_idp = NULL; //CATV_REFUND_API_CHANGE
	int             scale = 0;
	int             charge_to_t_eligibility = 1;
	int64			no_of_days = 0;
	char			msg[100];
	char			*act = NULL;
	char                    *tax_codep = NULL; //CATV_REFUND_API_CHANGE
	time_t			*next_bill_t = NULL;
	time_t			*charge_to_t = NULL;
	time_t			zero = 0;
	time_t			*event_charge_from_t = &zero;
	time_t			*event_charge_to_t = &zero;
	time_t			*tmp = NULL;
	time_t          calc_evt_from_t = 0;
	time_t          calc_evt_to_t = 0;
	int32			event_count = 0;
	time_t                  current_t = 0;
	int32                   t_elemid = 0;    //CATV_REFUND_API_CHANGE
	pin_cookie_t            t_cookie = NULL; //CATV_REFUND_API_CHANGE

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Validate FDP for ET Alignment input flist", i_flistp);

	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);

	plan_list_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS, 0, 1, ebufp);
	deal_flistp = PIN_FLIST_ELEM_GET(plan_list_flistp, PIN_FLD_DEALS, 0, 1, ebufp);
	deal_pdp = PIN_FLIST_ELEM_GET(deal_flistp, PIN_FLD_DEAL_OBJ, 0, 1, ebufp);
	current_t = pin_virtual_time(NULL);
	current_t = fm_utils_time_round_to_midnight(current_t);	

	if (deal_pdp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling Validation of Base FDP Pack");
		is_base_fdp_pack = fm_mso_search_base_fdp(ctxp, deal_pdp, &pdt_flistp, ebufp);
		if (pdt_flistp)
		{
			pdt_pdp = PIN_FLIST_FLD_GET(pdt_flistp, PIN_FLD_POID, 1, ebufp);
		}
		if (is_base_fdp_pack == 1 && pdt_pdp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan Changed is a FDP Plan (OLD)...");
			//			r_flistp = PIN_FLIST_CREATE(ebufp);
			//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling Validation only Base FDP Pack from Purchased Product");
			//			is_only_base = fm_mso_pp_validate_fdp(ctxp, srvc_pdp, ebufp);
			fm_mso_retrieve_et(ctxp, srvc_pdp, &r_flistp, ebufp);

			if (r_flistp)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET Charge Details Retrieved");
				charge_to_t = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
				calc_evt_to_t = *charge_to_t;
				fm_mso_retrieve_billinfo(ctxp, acct_pdp, &res_flistp, ebufp);
				if (res_flistp)
				{
					payinfo_obj = PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_PAYINFO_OBJ,1,ebufp);
					if(!PIN_POID_IS_NULL(payinfo_obj))
					{
						read_flds_iflistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(read_flds_iflistp , PIN_FLD_POID , payinfo_obj , ebufp );
						rd_inv_info = PIN_FLIST_ELEM_ADD(read_flds_iflistp, PIN_FLD_INV_INFO, PIN_ELEMID_ANY, ebufp );
						PIN_FLIST_FLD_SET(rd_inv_info , PIN_FLD_INDICATOR , 0 , ebufp );

						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Payinfo input flist", read_flds_iflistp);
						PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflistp, &read_flds_oflistp, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read Payinfo output flist", read_flds_oflistp);

						if(read_flds_oflistp)
						{
							inv_info = PIN_FLIST_ELEM_GET(read_flds_oflistp, PIN_FLD_INV_INFO, PIN_ELEMID_ANY, 1, ebufp);
							if(inv_info)
							{
								pay_indicator = PIN_FLIST_FLD_GET(inv_info, PIN_FLD_INDICATOR, 1, ebufp);
							}
						}
					}

					next_bill_t = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_NEXT_BILL_T, 1, ebufp);
					//calc_evt_from_t = *next_bill_t;
					calc_evt_from_t = current_t;
					if(charge_to_t && current_t && (*charge_to_t > current_t))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Charge_to_t > Next_bill_t");
						/*	if( mso_get_days_from_timestamp(*charge_to_t) - mso_get_days_from_timestamp(*next_bill_t) > 0 )
							{	
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test1");
							scale = (mso_get_months_from_timestamp(*charge_to_t) 
							- mso_get_months_from_timestamp(*next_bill_t)) 
							+ ((mso_get_years_from_timestamp(*charge_to_t) 
							- mso_get_years_from_timestamp(*next_bill_t)) * 12 );
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test2");
						/*									+ ((mso_get_days_from_timestamp(*charge_to_t)
						- mso_get_days_from_timestamp(*next_bill_t))
						/ (mso_get_days_from_timestamp(*charge_to_t)
						- mso_get_days_from_timestamp(*next_bill_t)));*
						}
						else
						{*/
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test1a");
						scale = (mso_get_months_from_timestamp(*charge_to_t)
								- mso_get_months_from_timestamp(current_t))
							+ ((mso_get_years_from_timestamp(*charge_to_t)
										- mso_get_years_from_timestamp(current_t)) * 12 );
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "test2a");
						//	}

						no_of_days = (*charge_to_t - current_t)/60/60/24;
						scale = scale * (-1);
						/* Set values to be used for invoicing */
						*event_charge_from_t = current_t;
						*event_charge_to_t = *charge_to_t;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test3");
						sprintf(msg,"event_charge_from_t: %ld",*event_charge_from_t);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);


						*charge_to_t = current_t;
						if (pay_indicator && *pay_indicator == 0)
						{
							act = (char *) malloc(strlen("POST:CHANGE PLAN - ET REFUND")+1);
							strcpy(act,"POST:CHANGE PLAN - ET REFUND");
						}
						else
						{

							act = (char *) malloc(strlen("PRE:CHANGE PLAN - ET REFUND")+1);
							strcpy(act,"PRE:CHANGE PLAN - ET REFUND");
						}



						/*********** Trigger ET ***********/
						if(no_of_days >30)
						{

							sprintf(msg,"event_charge_to_t: %d",*event_charge_to_t);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
							sprintf(msg,"event_charge_to_t: %ld",calc_evt_to_t);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
							*event_charge_from_t = current_t;
							sprintf(msg,"event_charge_from_t: %ld",calc_evt_from_t);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

							et_in_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_END_T, charge_to_t, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_NUMBER_OF_UNITS, &scale, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
							PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_BAL_GRP_OBJ, et_in_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
							//CATV_REFUND_API_CHANGE
							PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OFFERING_OBJ, et_in_flistp, PIN_FLD_OFFERING_OBJ, ebufp);
							//PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_FLAGS, &karnataka_eligibility_flag, ebufp);
							//PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_ACTG_NEXT_T, et_in_flistp, PIN_FLD_EVENT_COUNT, ebufp);

							tmp = (time_t *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACTG_NEXT_T, 1, ebufp);
							event_count = *tmp;
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_EVENT_COUNT, &event_count, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_STATUS_FLAGS, &charge_to_t_eligibility, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_CHARGED_FROM_T, &calc_evt_from_t, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_CHARGED_TO_T, &calc_evt_to_t, ebufp);
							PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_BILL_OBJ, et_in_flistp, PIN_FLD_BILL_OBJ, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_INDICATOR, pay_indicator, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_NEXT_BILL_T, next_bill_t, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_ACTION, act, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_PRODUCT_OBJ, pdt_pdp, ebufp);

							//Adding actual program name in flist for refund cases
							PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, et_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
							PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_CYCLE_FEE_FLAGS,  &charge_to_t_eligibility, ebufp);
							//CATV_REFUND_API_CHANGE - START
							mso_bill_pol_trigger_et_event(ctxp, flags, r_flistp, et_in_flistp, &et_out_flistp, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ET REFUND TRIGGER", et_out_flistp);
							if (!PIN_ERRBUF_IS_ERR(ebufp) && flags == PCM_OPFLG_CALC_ONLY && et_out_flistp != NULL){
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan ET Refund successful");
								*ret_flistpp = PIN_FLIST_CREATE(ebufp);
								while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(et_out_flistp, PIN_FLD_RESULTS, &t_elemid, 1,                                                                                                                 &t_cookie, ebufp)) != NULL)
								{
									bal_imp_flistp = PIN_FLIST_ELEM_GET(tmp_flistp, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY,1, ebufp);
									tax_codep = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
									res_idp = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_RESOURCE_ID, 1, ebufp);
									if ((res_idp && *res_idp == CURRENCY ) && (tax_codep && (strcmp(tax_codep, "MSO_ET_MAIN")
													|| strcmp(tax_codep, "MSO_ET_ADDI")) == 0)){
										PIN_FLIST_FLD_COPY(bal_imp_flistp, PIN_FLD_AMOUNT, *ret_flistpp,                                                                                                                                        MSO_FLD_REFUND_AMOUNT, ebufp);
									}
								}

							}

							PIN_FLIST_DESTROY_EX(&et_out_flistp, NULL);	
							//CATV_REFUND_API_CHANGE - END
						}
						else
						{
							write_flds_inflistp = PIN_FLIST_CREATE(ebufp);

							PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
							PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_CHARGED_TO_T, &current_t, ebufp);

							PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
										"Error in fm_mso_reactivate_plan: wflds", ebufp);
							}

							PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
							PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
						}
					}

					/*						write_flds_inflistp = PIN_FLIST_CREATE(ebufp);

											PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
											PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_CHARGED_TO_T, event_charge_to_t, ebufp);

											PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

											if (PIN_ERRBUF_IS_ERR(ebufp))
											{
											PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
											"Error in fm_mso_reactivate_plan: wflds", ebufp);
											}

											PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
											PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
					 */
											else if(charge_to_t && current_t && (*charge_to_t < current_t))
											{
												PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Charge_to_t < Next_bill_t. No need of Refund, but align the ET details object.");

												write_flds_inflistp = PIN_FLIST_CREATE(ebufp);

												PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
												PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_CHARGED_TO_T, &current_t, ebufp);

												PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

												if (PIN_ERRBUF_IS_ERR(ebufp))
												{
													PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
															"Error in fm_mso_reactivate_plan: wflds", ebufp);
												}

												PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
												PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
											}

				}
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET Charge Details Not Available. No Need to Align ET...");
			}
		}
	}

	PIN_FLIST_DESTROY_EX(&et_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_flds_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_flds_oflistp, NULL);
}

int32 
fm_mso_search_base_fdp(
		pcm_context_t           *ctxp,
		poid_t                  *deal_pdp,
		pin_flist_t				**ret_flistpp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_flistp = NULL;
	pin_flist_t             *srch_out_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *r_flistp = NULL;
	pin_flist_t             *pdts_flistp = NULL;
	pin_flist_t             *usg_flistp = NULL;
	pin_flist_t             *out_flistp = NULL;
	char                    template2[200] = "select X from /product 1, /deal 2, /mso_cfg_catv_pt 3 where 1.F1 = 2.F2 and 2.F3 = V3 and 1.F4 = 3.F5 and 1.F6 like V6 and 3.F7 = V7";
	char                    *ptp = "";
	int32                   srch_flag = 768;
	int32                   pkg_type = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(PIN_POID_GET_DB(deal_pdp), "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp,PIN_FLD_ARGS,1,ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, deal_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp,PIN_FLD_ARGS,2,ebufp);
	pdts_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PRODUCTS, 0,ebufp);
	PIN_FLIST_FLD_SET(pdts_flistp, PIN_FLD_PRODUCT_OBJ, deal_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp,PIN_FLD_ARGS,3,ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, deal_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp,PIN_FLD_ARGS,4,ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, ptp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp,PIN_FLD_ARGS,5,ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, ptp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp,PIN_FLD_ARGS,6,ebufp);
	usg_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_USAGE_MAP, 0,ebufp);
	PIN_FLIST_FLD_SET(usg_flistp, PIN_FLD_EVENT_TYPE, "%fdp%", ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp,PIN_FLD_ARGS,7,ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PKG_TYPE, 0, ebufp);

	r_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_search_base_fdp input flist", srch_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_base_fdp : error getting Package_type", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_mso_search_base_fdp output flist", srch_out_flistp);

	r_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (r_flistp)
	{

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base FDP Package Available and being changed...");
		out_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_POID, out_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(out_flistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
		*ret_flistpp = PIN_FLIST_COPY(out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
		return 1;

	}
	else
	{
		pkg_type = -1;
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return 0;

}


void 
fm_mso_retrieve_et(
		pcm_context_t           *ctxp,
		poid_t                  *srvc_pdp,
		pin_flist_t             **r_flistpp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_flistp = NULL;
	pin_flist_t             *srch_out_flistp = NULL;
	pin_flist_t             *arg_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	char                    template2[100] = "select X from /mso_et_charge_details where F1 = V1";
	char                    *ptp = "";
	int32                   srch_flag = 768;
	int32                   pkg_type = -1;
	int                     success_status = 0;
	int                     failure_status = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID,
			PIN_POID_CREATE(PIN_POID_GET_DB(srvc_pdp), "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_et_charge_details input flist", srch_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"mso_et_charge_details : error getting Package_type", ebufp);

		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_et_charge_details output flist", srch_out_flistp);

	res_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (res_flistp)
	{
		*r_flistpp = PIN_FLIST_COPY(res_flistp, ebufp);
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

}

void 
fm_mso_retrieve_billinfo(
		pcm_context_t           *ctxp,
		poid_t                  *acct_pdp,
		pin_flist_t             **r_flistpp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_flistp = NULL;
	pin_flist_t             *srch_out_flistp = NULL;
	pin_flist_t             *arg_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	char                    template2[100] = "select X from /billinfo where F1 = V1 and F2 = V2";
	char                    *ptp = "";
	int32                   srch_flag = 768;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID,
			PIN_POID_CREATE(PIN_POID_GET_DB(acct_pdp), "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_BILLINFO_ID, "CATV", ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_NEXT_BILL_T, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACTG_NEXT_T, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_BILL_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PAYINFO_OBJ, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"billinfo input flist", srch_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "billinfo : error getting Package_type", ebufp);

		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "billinfo output flist", srch_out_flistp);

	res_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (res_flistp)
	{
		*r_flistpp = PIN_FLIST_COPY(res_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "billinfo search return flist", *r_flistpp);
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);

}

void
fm_mso_cust_upgrade_plan_validity(
		pcm_context_t           *ctxp,
		pin_flist_t             *i_flistp,
		pin_flist_t             **r_flistpp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_offer_iflistp = NULL;
	pin_flist_t		*srch_offer_oflistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*srvc_flistp = NULL;
	pin_flist_t		*bb_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*validty_iflistp = NULL;
	pin_flist_t		*validty_oflistp = NULL;
	pin_flist_t		*offering_flistp = NULL;
	pin_flist_t		*purch_prod_flistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*mso_flistp = NULL;
	pin_flist_t		*prod_arry_flistp = NULL;
	pin_flist_t		*product_oflistp = NULL;
	pin_flist_t		*cfee_flistp = NULL;
	pin_flist_t		*usage_flistp = NULL;

	poid_t			*act_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*offer_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*prod_pdp = NULL;
	poid_t			*mso_offer_pdp = NULL;
	poid_t			*mso_prod_pdp = NULL;

	int32			*type = NULL;
	int32			*bill_when = NULL;
	int32			num_months = 0;
	int32			validity_failure = 1;
	int32			flags = 0;

	char			*city = NULL;
	char			*strp = NULL;
	char			*evt_type = NULL;
	time_t			purch_end_t = 0;
	time_t			old_purch_end_t = 0;

	struct tm               *ts = NULL ;

	void			*vp = NULL;

	int			elem_id=0;
	int			relem_id=0;
	int			prty = 0;
	int32			mod_prty=0;
	int32			fup_plan = 0;
	int32			success = 0;
	int32			*package_id = NULL;
	pin_decimal_t           *priority = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		rcookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_upgrade_plan_validity input flistp", i_flistp);

	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	fm_mso_utils_read_any_object(ctxp, srvc_pdp, &srvc_flistp, ebufp);
	if(srvc_flistp){
		bb_flistp = PIN_FLIST_SUBSTR_GET(srvc_flistp, MSO_FLD_BB_INFO, 1, ebufp);
		if(bb_flistp){
			city = PIN_FLIST_FLD_GET(bb_flistp, PIN_FLD_CITY, 1, ebufp);
		}
	}
	bill_when = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_WHEN, 1, ebufp);

	//Search for Upgrade offers in the system based on the city
	//If GLOBAL, then use *
	/***********************************************************
	 ***********************************************************/
	srch_offer_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_offer_iflistp, PIN_FLD_CITY, city, ebufp);
	PIN_FLIST_FLD_SET(srch_offer_iflistp, PIN_FLD_BILL_WHEN, bill_when, ebufp); 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Upgrade Offer details input flistp", srch_offer_iflistp);
	fm_mso_search_upgrade_offer_details(ctxp, srch_offer_iflistp, &srch_offer_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_offer_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling upgrade banner offer details", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, act_pdp, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validity_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51158", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error while getting upgrade offer details", ebufp);
		PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
		*r_flistpp = ret_flistp;
		return;

	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Upgrade Offer details results flistp", srch_offer_oflistp);
	if(srch_offer_oflistp && PIN_FLIST_ELEM_COUNT(srch_offer_oflistp, PIN_FLD_RESULTS, ebufp) > 0 ){
		while((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_offer_oflistp, PIN_FLD_RESULTS,
						&elem_id, 1, &cookie, ebufp)) != NULL)
		{
			type = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_UNIT, 1, ebufp);
			if(type && *type == 1){  //Unit is MONTHS, so add months to end date
				vp = (int32 *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_NUM_UNITS, 1, ebufp);	
				if(vp)
					num_months = *(int32*)vp;
			}
			fm_mso_get_subscr_purchased_products(ctxp, act_pdp, srvc_pdp, &purch_prod_flistp, ebufp);
			//fm_mso_utils_get_active_subscr_product(ctxp, act_pdp, srvc_pdp, 1, &purch_prod_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp) || purch_prod_flistp == NULL) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while getting active subscr product details", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, act_pdp, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validity_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51158", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in getting active product details", ebufp);
				PIN_FLIST_DESTROY_EX(&srch_offer_oflistp, NULL);
				PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
				*r_flistpp = ret_flistp;
				return;
			}
			if(purch_prod_flistp){
				offering_flistp = PIN_FLIST_ELEM_GET(purch_prod_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);//Get first product which is newly added
				if(offering_flistp){
					offer_pdp = PIN_FLIST_FLD_GET(offering_flistp, PIN_FLD_POID, 0, ebufp);
					plan_pdp = PIN_FLIST_FLD_GET(offering_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
					prod_pdp = PIN_FLIST_FLD_GET(offering_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
					package_id = PIN_FLIST_FLD_GET(offering_flistp, PIN_FLD_PACKAGE_ID, 0, ebufp);
					purch_end_t = *(time_t *)PIN_FLIST_FLD_GET(offering_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
					old_purch_end_t = purch_end_t; //Store to trigger validity extn lifecycle
					ts = localtime(&purch_end_t);
					ts->tm_mon = ts->tm_mon+num_months;
					purch_end_t = mktime(ts);
					//Get Priority for product
					fm_mso_utils_read_any_object(ctxp, prod_pdp, &product_oflistp, ebufp);		
					if(product_oflistp){
						priority = PIN_FLIST_FLD_GET(product_oflistp, PIN_FLD_PRIORITY, 1, ebufp);
						strp = pbo_decimal_to_str(priority, ebufp);
						prty = atoi(strp);
						if (strp) {
							pin_free(strp);
							strp = NULL;
						}
						mod_prty = prty%1000;
						if(mod_prty >= BB_UNLIMITED_FUP_RANGE_START && mod_prty <= BB_UNLIMITED_FUP_RANGE_END)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FUP Plan");
							fup_plan = 1;
						}
					}
					PIN_FLIST_DESTROY_EX(&product_oflistp, NULL);
					fm_mso_utils_get_mso_purchase_details(ctxp, act_pdp, srvc_pdp, offer_pdp, package_id, &mso_flistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp) || mso_flistp == NULL) {
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while getting active subscr product details", ebufp);
						if(PIN_ERRBUF_IS_ERR(ebufp))
							PIN_ERRBUF_CLEAR(ebufp);
						ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, act_pdp, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validity_failure, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51158", ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in getting active product details", ebufp);
						PIN_FLIST_DESTROY_EX(&srch_offer_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
						*r_flistpp = ret_flistp;
						return;
					}
					if(mso_flistp){
						while((prod_arry_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_flistp, PIN_FLD_RESULTS,
										&relem_id, 1, &rcookie, ebufp)) != NULL){
							mso_offer_pdp =  PIN_FLIST_FLD_GET(prod_arry_flistp, PIN_FLD_POID, 0, ebufp);
							/*mso_prod_pdp =  PIN_FLIST_FLD_GET(prod_arry_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
							  product_oflistp = NULL;
							  fm_mso_utils_read_any_object(ctxp, mso_prod_pdp, &product_oflistp, ebufp);
							  if(product_oflistp){
							  usage_flistp = PIN_FLIST_ELEM_GET(product_oflistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 1, ebufp);
							  if(usage_flistp){
							  evt_type = PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
							  }
							  }*/
							validty_iflistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(validty_iflistp, PIN_FLD_POID, mso_offer_pdp, ebufp);
							PIN_FLIST_FLD_SET(validty_iflistp, PIN_FLD_PURCHASE_END_T, &purch_end_t, ebufp);
							PIN_FLIST_FLD_SET(validty_iflistp, PIN_FLD_USAGE_END_T, &purch_end_t, ebufp);
							PIN_FLIST_FLD_SET(validty_iflistp, PIN_FLD_CYCLE_END_T, &purch_end_t, ebufp);
							/*if(!(evt_type && strstr(evt_type, "mso_grant") && fup_plan == 1 )){//Dont set for fup plans
							  cfee_flistp = PIN_FLIST_ELEM_GET(validty_iflistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);
							  if(cfee_flistp){
							  PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_CHARGED_TO_T, &purch_end_t, ebufp);
							  PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_CYCLE_FEE_END_T, &purch_end_t, ebufp);
							  }
							  }*/
							//PIN_FLIST_DESTROY_EX(&product_oflistp, NULL);

							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Upgrade Offer validity flistp", validty_iflistp);
							//PCM_OP(ctxp, MSO_OP_CUST_EXTEND_VALIDITY, 0, validty_iflistp, &validty_oflistp, ebufp);
							PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, validty_iflistp, &validty_oflistp, ebufp);
							PIN_FLIST_DESTROY_EX(&validty_iflistp, NULL);
							if (PIN_ERRBUF_IS_ERR(ebufp)) {
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while getting active subscr product details", ebufp);
								PIN_ERRBUF_RESET(ebufp);
								ret_flistp = PIN_FLIST_CREATE(ebufp);
								PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, act_pdp, ebufp);
								PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validity_failure, ebufp);
								PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51158", ebufp);
								PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in getting active product details", ebufp);
								PIN_FLIST_DESTROY_EX(&srch_offer_oflistp, NULL);
								PIN_FLIST_DESTROY_EX(&mso_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
								*r_flistpp = ret_flistp;
								return;
							}
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Upgrade Offer validity Oflistp", validty_oflistp);
						}
					}
					if(validty_oflistp){
						//*r_flistpp = PIN_FLIST_COPY(validty_oflistp, ebufp);
						*r_flistpp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &success, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_PURCHASE_END_T, &purch_end_t, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_OLD_VALIDITY_END_T, &old_purch_end_t, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_END_T, &purch_end_t, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, act_pdp, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, *r_flistpp, PIN_FLD_USERID, ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, *r_flistpp, PIN_FLD_PROGRAM_NAME, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

						//Create validityextn lifecycle for maintiang the history
						//fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_VALIDITY_EXTENTION_FOR_A_SERVICE_PLAN, ebufp);
						/******************************************* notification flist ***********************************************/
						/*flags = 29;
						  provaction_inflistp = PIN_FLIST_CREATE(ebufp);
						  PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
						  PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
						  PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, provaction_inflistp, PIN_FLD_AMOUNT, ebufp);
						  PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &flags, ebufp);
						  PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_NUM_UNITS, &num_months, ebufp);
						  PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_CITY, city, ebufp);

						  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "upgrdation plan notification input list", provaction_inflistp);
						  PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
						  PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
						  if (PIN_ERRBUF_IS_ERR(ebufp))
						  {
						  PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
						  PIN_ERRBUF_RESET(ebufp);
						  PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						  ret_flistp = PIN_FLIST_CREATE(ebufp);
						  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, act_pdp, ebufp);
						  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &validity_failure, ebufp);
						  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51158", ebufp);
						  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in getting active product details", ebufp);
						  PIN_FLIST_DESTROY_EX(&srch_offer_oflistp, NULL);
						  PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);
						  PIN_FLIST_DESTROY_EX(&validty_oflistp, NULL);
						  PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
						  PIN_FLIST_DESTROY_EX(&mso_flistp, NULL);
						 *r_flistpp = ret_flistp;

						 return ;
						 }
						 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "upgrdation plan notification output flist", provaction_outflistp);
						 PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						 */
					}
					PIN_FLIST_DESTROY_EX(&validty_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&mso_flistp, NULL);

				}
			}
			PIN_FLIST_DESTROY_EX(&purch_prod_flistp, NULL);

		}
	}
	PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_offer_oflistp, NULL);
	return;

}

extern void 
fm_mso_generate_credit_charge(
		pcm_context_t           *ctxp,
		pin_flist_t		*i_flistp,
		poid_t			*cancel_offer_obj,
		poid_t			*cancel_prod_obj,
		pin_flist_t             **r_flistpp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t	*event_det_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*plan_flistp = NULL;
	pin_flist_t	*topup_flistp = NULL;
	pin_flist_t	*ri_flistp = NULL;
	pin_flist_t	*ro_flistp = NULL;
	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*purch_prod_oflistp = NULL;
	pin_flist_t	*prod_oflistp = NULL;
	pin_flist_t	*cfee_flistp = NULL;
	pin_flist_t	*sub_cfee_flistp = NULL;
	pin_flist_t	*usage_flistp = NULL;
	pin_flist_t	*prods_flistp = NULL;
	pin_flist_t	*rate_oflistp = NULL;
	pin_flist_t	*rate2_oflistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*subscr_prods_flistp = NULL;
	pin_flist_t	*balimp_flistp = NULL;
	pin_flist_t	*plan_ret_flistp = NULL;
	pin_flist_t	*srch_plan_flistp = NULL;
	pin_flist_t	*srch_plan_retflistp = NULL;
	pin_flist_t	*plan_arry_flp = NULL;
	pin_flist_t	*pack_flistp = NULL;
	pin_flist_t	*city_flistp = NULL;
	pin_flist_t	*taxes_flistp = NULL;
	pin_flist_t	*tax_flistp = NULL;
	pin_flist_t	*total_flistp = NULL;
	pin_flist_t 	*topup_prod_flistp = NULL;
	pin_flist_t 	*sort_flistp = NULL;
	pin_flist_t 	*validity_flistp = NULL;

	poid_t		*act_pdp = NULL;
	poid_t		*t_srvc_obj = NULL;
	poid_t		*service_pdp = NULL;
	poid_t		*old_plan_obj = NULL;
	poid_t          *base_plan_obj = NULL;
	poid_t		*new_plan_obj = NULL;
	poid_t		*plan_obj = NULL;
	poid_t		*event_pdp = NULL;
	poid_t		*new_purch_obj = NULL;
	poid_t		*prev_new_purch_obj = NULL;
	poid_t		*offer_obj = NULL;
	poid_t 		*bal_prod_obj = NULL;
	poid_t 		*bal_grp_obj = NULL;
	poid_t 		*bal_rate_obj = NULL;
	poid_t		*cancel_plan_obj = NULL;
	poid_t		*orig_cancel_offer_obj = NULL;
	poid_t		*orig_cancel_prod_obj = NULL;

	int32		*action = NULL;
	int32		*flags = NULL;
	int32		*opcode = NULL;
	//int32		*resource_id = NULL;
	int32		*bill_when = NULL;
	int32		*status = NULL;
	int             *istal = NULL;
    char        *str_rolep = NULL;
	char		event_type[60] = "/event/activity/mso_lifecycle/topup_plan";
	char		*prod_event_type = NULL;
	char		*tax_code = NULL;
	char		*plan_name = NULL;
	char		*city = NULL;
	time_t		current_t = pin_virtual_time(NULL);
	time_t		*charged_to_t = NULL;
	time_t		*orig_charged_to_t = NULL;
	time_t		*charged_from_t = NULL;
	time_t		*created_t = NULL;
	time_t		*sub_charged_to_t = NULL;
	time_t		*sub_purch_end_t = NULL;
	time_t		*p_cfee_start_t = NULL;
	time_t		*p_cfee_end_t = NULL;
	time_t		*valid_end_t = NULL;
	time_t		*e_created_t = NULL;
	time_t		*p_created_t = NULL;
	time_t		*pr_created_t = NULL;
	time_t		*old_charged_to_t = NULL;
	// LESS REFUND CHG
	time_t		*exact_charge_from_t = NULL;
	time_t 	    *exact_created_t =NULL;	
	time_t		*valida_ext_end = NULL;
	time_t		*comp_from_t = NULL;
	pin_flist_t 	*exact_charge_find = NULL;
	int            is_exclud=0;
	int            count_fin_exac=0;
	pin_decimal_t	*zer_disp = pbo_decimal_from_str("0", ebufp);
	//LESS REFUND CHG END
	void		*vp = NULL;

	pin_decimal_t	*percent = NULL;
	pin_decimal_t	*discount = NULL;
	pin_decimal_t	*top_disc_percent = NULL;
	pin_decimal_t	*top_discount = NULL;
	pin_decimal_t	*scalep = pbo_decimal_from_str("1", ebufp);
	pin_decimal_t	*total_disc_perc = pbo_decimal_from_str("0", ebufp);
	pin_decimal_t	*total_disc = pbo_decimal_from_str("0", ebufp);
	pin_decimal_t	*one_dp = pbo_decimal_from_str("1", ebufp);
	pin_decimal_t	*cf_amt = NULL;
	pin_decimal_t	*pf_amt = NULL;
	pin_decimal_t	*ch_amt = NULL;
	pin_decimal_t	*tot_amt = NULL;
	pin_decimal_t	*bal_disc_amt = NULL;
	pin_decimal_t	*zero_decimalp = pbo_decimal_from_str("0", ebufp);

	int		elem_id = 0;
	int		telem_id = 0;
	int		sort_flag = 1;
	int		ref_failure = 1;
	int		count = 0;
	int 		relem_id = 0;
	int 		belem_id = 0 ;
	int 		pelem_id = 0 ;
	int 		velem_id = 0 ;
	int		action_flag  =1;
	int		tax_index = 6;
	int		tax_imp_type = 1;
	int		is_extn = 0;
	int		is_charged = 0;
	int		is_cancel = 0;
	int		diff_days = 0;
	int		curr_diff_days = 0;
	int		is_curr_plan = 0;
	int		is_in_offer = 0;
	int		deduct_days = 0;
	int		extn_days = 0;
	int32		resource_id = 0;
	int32		bal_gl_id = 0;

	double		orig_quantity=0;
	double		quantity=0;
	double		scale=1;
	double		final_disc_perc=0;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	tcookie = NULL;
	pin_cookie_t 	rcookie = NULL;
	pin_cookie_t	bcookie  = NULL;
	pin_cookie_t	pcookie  = NULL;
	pin_cookie_t	vcookie  = NULL;
	int32		precision = 2;
	int32           round_mode = ROUND_HALF_UP;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	// Check for cancellation offer obj which should be populated for refund
	if(!cancel_offer_obj){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Offering obj is NULL and no further action is needed.");
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_generate_credit_charge input flist:", i_flistp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"cancel_prod_obj : input",cancel_prod_obj);
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"cancel_offer_obj : input",cancel_offer_obj);

	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp); // Check if REFUND flags passed from any other scenarios
	opcode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OPCODE, 1, ebufp); // Check if REFUND flags passed from any other scenarios

	//Take Original CANCEL offer and prodcut detail as we would be resetting a few
	orig_cancel_offer_obj = PIN_POID_COPY(cancel_offer_obj, ebufp);
	orig_cancel_prod_obj = PIN_POID_COPY(cancel_prod_obj, ebufp);

	while ((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,
					&elem_id, 1, &cookie, ebufp)) != NULL) 
	{
		action = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_FLAGS, 1, ebufp);
		if (action && action != NULL && *action == 0){	
			old_plan_obj = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		}
		else if (action && action != NULL && *action == 1){	
			new_plan_obj = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		}
	}

        //Added for tal plan
        istal = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ISTAL_FLAG, 1, ebufp);
        str_rolep = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ROLES, 1, ebufp);
        if ((istal && *istal == 1) || (str_rolep && strstr(str_rolep, "-Static")))
        {
                base_plan_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
        }
        else
        {
                if(old_plan_obj && old_plan_obj != NULL)
                        base_plan_obj = PIN_POID_COPY(old_plan_obj,ebufp);
        }

	// Check if Change plan is happening for same plan (Possible case is TOPUP for same plan )
	if ((old_plan_obj && old_plan_obj != NULL && new_plan_obj &&  new_plan_obj != NULL && PIN_POID_COMPARE(old_plan_obj, new_plan_obj, 0, ebufp) != 0) ||
			(flags && *flags == 1)){
		// Check if any topup is done for an account on same plan
		fm_mso_utils_read_any_object(ctxp, cancel_offer_obj, &purch_prod_oflistp, ebufp);
		if (purch_prod_oflistp && purch_prod_oflistp != (pin_flist_t *)NULL ){
			cancel_plan_obj = PIN_FLIST_FLD_GET(purch_prod_oflistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
			created_t = PIN_FLIST_FLD_GET(purch_prod_oflistp, PIN_FLD_CREATED_T, 1, ebufp);
			cfee_flistp = PIN_FLIST_ELEM_GET(purch_prod_oflistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);	
			if (cfee_flistp && cfee_flistp != (pin_flist_t *)NULL){
				charged_to_t = PIN_FLIST_FLD_GET(cfee_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
				charged_from_t = PIN_FLIST_FLD_GET(cfee_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
			}
			percent = PIN_FLIST_FLD_GET(purch_prod_oflistp, PIN_FLD_PURCHASE_DISCOUNT, 1, ebufp );
			discount = PIN_FLIST_FLD_GET(purch_prod_oflistp, PIN_FLD_PURCHASE_DISC_AMT, 1, ebufp );
			if(!pbo_decimal_is_null(percent, ebufp))
				pbo_decimal_add_assign(total_disc_perc, pbo_decimal_subtract(one_dp, percent,ebufp), ebufp);
			if(!pbo_decimal_is_null(discount, ebufp))
				pbo_decimal_add_assign(total_disc, discount, ebufp);
		}
		sort_flag = 1; // 0 - ASC, 1- DESC
		// Check if any topup is done for an account on same plan and would get realted events created > purchased_created
		//fm_mso_utils_get_event_details(ctxp, act_pdp, service_pdp, event_type, 0, sort_flag, &event_det_flistp, ebufp);
		topup_prod_flistp = PIN_FLIST_CREATE(ebufp);

                //For Static IP product no need to check for products having provisioning ID
                if ((istal && *istal == 1) || (str_rolep && strstr(str_rolep, "-Static")))
                {
                        subscr_prods_flistp = PIN_FLIST_CREATE(ebufp);
                        //purch_prod_flistp = PIN_FLIST_ELEM_ADD(subscr_prods_flistp,PIN_FLD_RESULTS,0,ebufp);
                        PIN_FLIST_ELEM_SET(subscr_prods_flistp, purch_prod_oflistp, PIN_FLD_RESULTS, 0, ebufp );
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Kali  subscr_prods_flistp");
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Kali: subscr_prods_flistp :", subscr_prods_flistp);
                }
                else
                {
                        fm_mso_get_subscr_purchased_products(ctxp, act_pdp, service_pdp, &subscr_prods_flistp, ebufp);
                }


		//CHANGES MADE FOR LESS REEFUND TO GET ACTUAL CHARGE FROM_T
		if (subscr_prods_flistp && subscr_prods_flistp != NULL && PIN_FLIST_ELEM_COUNT(subscr_prods_flistp, PIN_FLD_RESULTS, ebufp) > 0)
		{
			while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(subscr_prods_flistp, PIN_FLD_RESULTS,
							&telem_id, 1, &tcookie, ebufp)) != NULL) 
			{
				exact_charge_find=PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp); 
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					exact_charge_find=(pin_flist_t *)NULL;
					PIN_ERRBUF_CLEAR(ebufp);
				}
				if(exact_charge_find && exact_charge_find!=(pin_flist_t *)NULL)
				{
					comp_from_t = PIN_FLIST_FLD_GET(exact_charge_find, PIN_FLD_CHARGED_FROM_T, 1, ebufp);                                             

					if (count_fin_exac == 0)
					{
						exact_charge_from_t = PIN_FLIST_FLD_GET(exact_charge_find, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
						exact_created_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CYCLE_START_T, 1, ebufp);
					}
					if (count_fin_exac > 0 && *comp_from_t >=*exact_charge_from_t)
					{
						exact_charge_from_t = PIN_FLIST_FLD_GET(exact_charge_find, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
						exact_created_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CYCLE_START_T, 1, ebufp);
					}
					count_fin_exac=count_fin_exac+1;
				}
			}
			telem_id = 0;
			tcookie = NULL;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TRAVERSE TO FIND EXACT CHARGE FROM DATE");
		}   

		if (subscr_prods_flistp && subscr_prods_flistp != NULL && PIN_FLIST_ELEM_COUNT(subscr_prods_flistp, PIN_FLD_RESULTS, ebufp) > 0){
			while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(subscr_prods_flistp, PIN_FLD_RESULTS,
							&telem_id, 1, &tcookie, ebufp)) != NULL) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ENTERING SUBSCRIPTION LOOP1...");
				status = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_STATUS, 1, ebufp);
				sub_cfee_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);	
				if(opcode && *opcode == MSO_OP_CUST_BB_HW_AMC && telem_id == 0){
					if (status && *status == 3){
						is_cancel = 1; //This is to avoid some racecondition which is returning cancelled product in AMC case as first
						//continue;//Skip first element in case of AMC refund as results would return new active
					}else{
						continue;
					}
				}
				//below condition is to check for AMC to set the actual cancel offer and product obj and check topup scenarios 
				if(opcode && *opcode == MSO_OP_CUST_BB_HW_AMC && 
						((telem_id == 1 && is_cancel == 0)|| (telem_id == 0 && is_cancel == 1))){
					old_plan_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
                        		base_plan_obj = PIN_POID_COPY(old_plan_obj,ebufp);
					cancel_offer_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
					cancel_prod_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
					created_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CREATED_T, 1, ebufp);
					//sub_cfee_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);	
					//Get charged_to_t from current active offering
					if(sub_cfee_flistp && sub_cfee_flistp != (pin_flist_t *)NULL)	
						charged_to_t = PIN_FLIST_FLD_GET(sub_cfee_flistp, PIN_FLD_CYCLE_FEE_END_T, 1, ebufp);
					//Copy PRODUCTS to check validity extn which were TOPUP done
					PIN_FLIST_ELEM_COPY(subscr_prods_flistp, PIN_FLD_RESULTS, telem_id, topup_prod_flistp,
							PIN_FLD_PRODUCTS, telem_id, ebufp);
					charged_to_t = PIN_FLIST_FLD_GET(sub_cfee_flistp, PIN_FLD_CYCLE_FEE_END_T, 1, ebufp);
					continue;//Skip first element in case of AMC refund as results would return new active
				}
				//In case of subscription refund scenario, we get first element as current active plan
				if((!opcode || (opcode && *opcode != MSO_OP_CUST_BB_HW_AMC)) && telem_id == 0){
					PIN_FLIST_ELEM_COPY(subscr_prods_flistp, PIN_FLD_RESULTS, telem_id, topup_prod_flistp,
							PIN_FLD_PRODUCTS, telem_id, ebufp);
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ENTERING SUBSCRIPTION LOOP2...");
				offer_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
				new_purch_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
				t_srvc_obj = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
				if (offer_obj && offer_obj != NULL && cancel_offer_obj && cancel_offer_obj != NULL && PIN_POID_COMPARE(offer_obj, cancel_offer_obj, 0, ebufp) != 0 &&

						(service_pdp && t_srvc_obj && PIN_POID_COMPARE(service_pdp, t_srvc_obj, 0, ebufp) == 0)){
					if ((new_purch_obj && new_purch_obj != NULL && cancel_prod_obj && cancel_prod_obj != NULL && PIN_POID_COMPARE(new_purch_obj, cancel_prod_obj, 0, ebufp) == 0 ))
						//(opcode && *opcode == MSO_OP_CUST_BB_HW_AMC ))
					{
						sub_purch_end_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
						//sub_cfee_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);	
						/*****************************************************************************************
						 * For Topups, we would iterate through same plans purchased continuous and then count but
						 * if renewal is done,then also, we would get the same plan, hence we would ensure if 
						 * product got expired with full cycle and renewed again 
						 *****************************************************************************************/
						if (sub_cfee_flistp && sub_cfee_flistp != (pin_flist_t *)NULL){
							sub_charged_to_t = PIN_FLIST_FLD_GET(sub_cfee_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
							if (sub_purch_end_t && sub_purch_end_t != NULL && sub_charged_to_t &&sub_charged_to_t != NULL && *sub_purch_end_t == *sub_charged_to_t)
								break; //This is to avoid the renewal plans (in case of renewal, it would be same plan)
						}
						else{//if sub_cfee_flistp is null means accounts dectivated long ago and doesn't have cycle info details
							continue; //There is an issue with accounts renewed recently of deactivated long ago
						}
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TOP UP is done for this plan");
						// Get charged_from_t from results and final value would be first product actual charge_from_t

						created_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CREATED_T, 1, ebufp);

						if ((!opcode || (opcode && *opcode != MSO_OP_CUST_BB_HW_AMC  ))&& (created_t && exact_charge_from_t) && (*created_t < *exact_charge_from_t)){
							PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PURCHASE_DISCOUNT, zer_disp, ebufp );
							PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PURCHASE_DISC_AMT, zer_disp, ebufp );
						}

						if (!opcode || (opcode && *opcode != MSO_OP_CUST_BB_HW_AMC  )){
							top_disc_percent = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PURCHASE_DISCOUNT, 1, ebufp );
							if(!pbo_decimal_is_null(top_disc_percent, ebufp)) 
								pbo_decimal_add_assign(total_disc_perc, 
										pbo_decimal_subtract(one_dp,top_disc_percent, ebufp), ebufp);
							top_discount = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PURCHASE_DISC_AMT, 1, ebufp );
							if(!pbo_decimal_is_null(top_discount, ebufp)) 
								pbo_decimal_add_assign(total_disc, top_discount, ebufp);
						}

						tmp_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);	
						if (tmp_flistp && tmp_flistp != (pin_flist_t *)NULL){
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_generate_credit_charge cycle fess flist:", tmp_flistp);
							charged_from_t = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
						}
						PIN_FLIST_ELEM_COPY(subscr_prods_flistp, PIN_FLD_RESULTS, telem_id, 
								topup_prod_flistp, PIN_FLD_PRODUCTS, telem_id, ebufp);
						count = count+1;
					}
					else{ // If we get non-matching product, then it was not a topup hence leave now. 
						break;
					}
				}

			}
			//PIN_FLIST_DESTROY_EX(&event_det_flistp, NULL);
		}
		//Now get VALIDITY EXTENSION details to get actual charge_to_t
		memset(event_type, 60, '\0');
		strcpy(event_type, "/event/activity/mso_lifecycle/validity_extension");	
		sort_flag = 0; // We need first event to find OLD_VALIDITY date
		fm_mso_utils_get_event_details(ctxp, act_pdp, service_pdp, event_type, created_t, sort_flag, &event_det_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in creating refund ");
			PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&purch_prod_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&event_det_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&topup_prod_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&subscr_prods_flistp, NULL);
			return ;
		}
		if(event_det_flistp && event_det_flistp !=(pin_flist_t *)NULL && PIN_FLIST_ELEM_COUNT(event_det_flistp, PIN_FLD_RESULTS, ebufp) > 0){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validity Extension is done. Get the old vlaidity end t");

			if(PIN_FLIST_ELEM_COUNT(event_det_flistp, PIN_FLD_RESULTS, ebufp) > 0 && (!opcode || (opcode && *opcode != MSO_OP_CUST_BB_HW_AMC)))
			{//In case of validity extension event and ie old event
				while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(event_det_flistp, PIN_FLD_RESULTS,
								&velem_id, 1, &vcookie, ebufp)) != NULL){
					topup_flistp = PIN_FLIST_ELEM_GET(results_flistp, MSO_FLD_EXTEND_PLAN_VALIDITY, 0, 1, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TRAVERSE TO OLD EVENT DATE");
					if (topup_flistp && topup_flistp != (pin_flist_t *)NULL)
					{
						plan_obj = PIN_FLIST_FLD_GET(topup_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
					}
					//if((plan_obj && plan_obj != NULL && PIN_POID_COMPARE(old_plan_obj, plan_obj, 0, ebufp) == 0))
					if((plan_obj && plan_obj != NULL && PIN_POID_COMPARE(base_plan_obj, plan_obj, 0, ebufp) == 0))
						valida_ext_end = PIN_FLIST_FLD_GET(topup_flistp, PIN_FLD_END_T, 1, ebufp);
					if(exact_charge_from_t && exact_charge_from_t != NULL && valida_ext_end && valida_ext_end != NULL && *exact_charge_from_t <= *valida_ext_end)
						is_exclud=1;
				}
				if (is_exclud == 0){
					is_extn = 0;
					is_exclud =2;
					*charged_from_t = *exact_charge_from_t;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "OLD EXTENSION EVENT ENCOUNTERED HENCE FOLLOWING NEW FLOW NOT CALCULATING DIFF DAYS");
				}
				velem_id=0;
				vcookie=NULL;
			}


			if(count == 0){//In case no TOPUP done, continue with regular flow
				results_flistp = PIN_FLIST_ELEM_GET(event_det_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);

				if(results_flistp && results_flistp != (pin_flist_t *)NULL){
					topup_flistp = PIN_FLIST_ELEM_GET(results_flistp, MSO_FLD_EXTEND_PLAN_VALIDITY, 0, 1, ebufp);
					if (topup_flistp && topup_flistp != (pin_flist_t *)NULL)
					{
						plan_obj = PIN_FLIST_FLD_GET(topup_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
					}
					if((plan_obj && plan_obj != NULL && PIN_POID_COMPARE(base_plan_obj, plan_obj, 0, ebufp) == 0))
						orig_charged_to_t = PIN_FLIST_FLD_GET(topup_flistp, MSO_FLD_OLD_VALIDITY_END_T, 1, ebufp);
					if(charged_to_t && charged_to_t != NULL && orig_charged_to_t && orig_charged_to_t != NULL && *charged_to_t >= *orig_charged_to_t)
						charged_to_t = orig_charged_to_t;
					is_extn = 1;
				}

				if((!opcode || (opcode && *opcode != MSO_OP_CUST_BB_HW_AMC))&& is_extn==1)
				{
					if (exact_created_t && exact_charge_from_t && (*exact_created_t < *exact_charge_from_t))
					{
						*charged_from_t = *exact_created_t;
					}
					else 
					{ 
						*charged_from_t = *exact_charge_from_t;
					}

					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PATCH WORK FOR DOUBLE EXTN BUG");
				}

			}else if(count > 0 && topup_prod_flistp && topup_prod_flistp != NULL){
				//SORT is done to place the first product before topup as VALIDITY EXTN results in ASC order
				sort_flistp = PIN_FLIST_CREATE(ebufp);
				tmp_flistp = PIN_FLIST_ELEM_ADD(sort_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
				PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
				PIN_FLIST_SORT(topup_prod_flistp, sort_flistp, 1, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Topup Products Sorted", topup_prod_flistp);
				PIN_FLIST_DESTROY_EX(&sort_flistp, NULL);
				while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(topup_prod_flistp, PIN_FLD_PRODUCTS,
								&pelem_id, 1, &pcookie, ebufp)) != NULL){
					p_created_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CREATED_T, 1, ebufp);
					tmp_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);
					if(tmp_flistp && tmp_flistp != (pin_flist_t *)NULL){
						p_cfee_end_t = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CYCLE_FEE_END_T, 1, ebufp);
						p_cfee_start_t = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CYCLE_FEE_START_T, 1, ebufp);
					}
					is_charged = 0;
					velem_id = 0;
					vcookie = NULL;
					while ((validity_flistp = PIN_FLIST_ELEM_GET_NEXT(event_det_flistp, PIN_FLD_RESULTS,
									&velem_id, 1, &vcookie, ebufp)) != NULL){	
						e_created_t = PIN_FLIST_FLD_GET(validity_flistp, PIN_FLD_CREATED_T, 1, ebufp);
						//Check if validty extn lifecycle is created after product creation date
						//if(p_created_t && e_created_t && *p_created_t < *e_created_t)
						//	continue;
						topup_flistp = PIN_FLIST_ELEM_GET(validity_flistp, MSO_FLD_EXTEND_PLAN_VALIDITY, 0, 1, ebufp);
						if(topup_flistp && topup_flistp != (pin_flist_t *)NULL){
							plan_obj = PIN_FLIST_FLD_GET(topup_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
							valid_end_t = PIN_FLIST_FLD_GET(topup_flistp, PIN_FLD_END_T, 1, ebufp);
						}				
						if((plan_obj && plan_obj != NULL && PIN_POID_COMPARE(base_plan_obj, plan_obj, 0, ebufp) == 0)&& (*valid_end_t >= *exact_charge_from_t)){
							valid_end_t = PIN_FLIST_FLD_GET(topup_flistp, PIN_FLD_END_T, 1, ebufp);
							//In case topup and validity is done, cycle_end_t should be equal to end_t of EXTN event
							if(p_cfee_end_t && p_cfee_end_t != NULL && valid_end_t && valid_end_t!= NULL && fm_utils_time_round_to_midnight(*p_cfee_end_t) == *valid_end_t){ 
								old_charged_to_t = PIN_FLIST_FLD_GET(topup_flistp, MSO_FLD_OLD_VALIDITY_END_T, 1, ebufp);
								//Find out total diff days to be exempted from refund
								if(old_charged_to_t && old_charged_to_t != NULL && *valid_end_t >= *old_charged_to_t ){
									diff_days = diff_days+((*valid_end_t - *old_charged_to_t)/60/60/24);
									if(is_exclud==1)
									{deduct_days = deduct_days+((*valid_end_t-*old_charged_to_t)/60/60/24);}




									//Check if valid_end_t is equal to current plan charged_to_t and set the same for calc
									if(*valid_end_t == *charged_to_t){
										charged_to_t = old_charged_to_t; 
										is_curr_plan = 1;
										//curr_diff_days = (*valid_end_t - *old_charged_to_t)/60/60/24;
									}else{
										if(fm_utils_time_round_to_midnight(current_t) < *valid_end_t && 
												is_in_offer != 1 &&
												fm_utils_time_round_to_midnight(current_t) >= *old_charged_to_t)
										{
											//current_t = *valid_end_t;
											is_in_offer = 1;
										}//get total extn days excluding offer period
										else if(fm_utils_time_round_to_midnight(current_t) < *old_charged_to_t)
										{
											extn_days = extn_days+((*valid_end_t - *old_charged_to_t)/60/60/24);
										}
									}
								}
								else if( *valid_end_t < *old_charged_to_t){//This is for adding total deduct days
									deduct_days = deduct_days+((*old_charged_to_t - *valid_end_t)/60/60/24);
									if(is_exclud==1)
									{diff_days = diff_days+((*old_charged_to_t - *valid_end_t)/60/60/24);}
								}
								is_extn = 2;
								if((!opcode || (opcode && *opcode != MSO_OP_CUST_BB_HW_AMC))&& is_exclud==1)
								{
									if (exact_created_t && exact_charge_from_t && (*exact_created_t < *exact_charge_from_t))
									{
										*charged_from_t = *exact_created_t;
									}
									else 
									{ 
										*charged_from_t = *exact_charge_from_t;
									}
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "MULTIPLE EVENT BYPASS SET");
								}

								break;
							}	
						}
					}
				}

			}
		}
		PIN_FLIST_DESTROY_EX(&topup_prod_flistp, NULL);
		// Get EVENT TYPE for refund to be created
		fm_mso_utils_read_any_object(ctxp, orig_cancel_prod_obj, &prod_oflistp, ebufp);
		if ( prod_oflistp && prod_oflistp !=(pin_flist_t *)NULL){ 
			usage_flistp = PIN_FLIST_ELEM_GET(prod_oflistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 1, ebufp);	
			if (usage_flistp && usage_flistp !=(pin_flist_t *)NULL){
				prod_event_type = PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
			}

		}             
		city =	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 1, ebufp);
		bill_when = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_OLD_VALUE, 1, ebufp);	
		fm_mso_utils_read_any_object(ctxp, cancel_plan_obj, &plan_ret_flistp, ebufp);
		if(plan_ret_flistp && plan_ret_flistp !=(pin_flist_t *)NULL)
			plan_name = PIN_FLIST_FLD_GET(plan_ret_flistp, PIN_FLD_NAME, 1, ebufp);

		//fm_mso_search_plan_details(ctxp, srch_plan_flistp, &srch_plan_retflistp, ebufp);*/

		fm_mso_utils_get_event_balimpacts(ctxp, act_pdp, service_pdp, orig_cancel_offer_obj, created_t, prod_event_type, &srch_plan_retflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH CHARGE DETAILS RETURN", srch_plan_retflistp);
		if  (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in while searching charge details", ebufp);
			PIN_FLIST_DESTROY_EX(&plan_ret_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_plan_retflistp, NULL);
			PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&purch_prod_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&event_det_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&subscr_prods_flistp, NULL);
			return;
		}
		if(srch_plan_retflistp && srch_plan_retflistp !=(pin_flist_t *)NULL && PIN_FLIST_ELEM_COUNT(srch_plan_retflistp, PIN_FLD_RESULTS, ebufp) > 0){
			relem_id = 0;
			rcookie = NULL;
			while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_plan_retflistp, PIN_FLD_RESULTS,
							&relem_id, 1, &rcookie, ebufp)) != NULL) 
			{
                                belem_id = 0;
                                bcookie = NULL;
				while ((balimp_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_BAL_IMPACTS,
								&belem_id, 1, &bcookie, ebufp)) != NULL) 
				{
					bal_prod_obj =  PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
					vp = (int32 *)PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_RESOURCE_ID, 1, ebufp);
					if(vp) resource_id  = *(int32 *)vp;
					tax_code =  PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
					bal_rate_obj =  PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_RATE_OBJ, 1, ebufp);
					bal_grp_obj =  PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
					vp =  (int32 *)PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_GL_ID, 1, ebufp);
					if(vp && vp != NULL) bal_gl_id = *(int32 *)vp;
					if((orig_cancel_prod_obj && orig_cancel_prod_obj != NULL && bal_prod_obj && bal_prod_obj != NULL && PIN_POID_COMPARE(orig_cancel_prod_obj, bal_prod_obj, 0,ebufp) == 0)
							&& resource_id == 356){
						ch_amt = PIN_FLIST_FLD_TAKE(balimp_flistp, PIN_FLD_AMOUNT, 1, ebufp);
						bal_disc_amt = PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_DISCOUNT, 1, ebufp);
						//If any discount is given during the plan purchase, consider to get full charge amount.
						if(bal_disc_amt && bal_disc_amt != NULL && !pbo_decimal_is_null(ch_amt, ebufp))
							pbo_decimal_add_assign(ch_amt, bal_disc_amt, ebufp);
					}
				}
			}
		}
		if (pbo_decimal_is_null(ch_amt, ebufp) || pbo_decimal_is_zero(ch_amt, ebufp)){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No need of refund as proudct hasn't been charged");
			PIN_FLIST_DESTROY_EX(&plan_ret_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_plan_retflistp, NULL);
			PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&purch_prod_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&event_det_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&subscr_prods_flistp, NULL);
			if(!pbo_decimal_is_null(ch_amt, ebufp))
				pbo_decimal_destroy(&ch_amt);
			return;

		}
		/************************* END NEW FUNCTIONALITY CHECKING *******************************************************/

		//Now trigger DISCONNECT CREDIT CHARGE (Refund) for prepaid subscription
		ri_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_POID, act_pdp, ebufp);
		PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
		PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_OFFERING_OBJ, orig_cancel_offer_obj, ebufp);
		PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_PRODUCT_OBJ, orig_cancel_prod_obj, ebufp);
		PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_PROGRAM_NAME, "PREPAY_DISCONNECTION_CREDIT", ebufp);
		PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_START_T, &current_t, ebufp);
		PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_END_T, charged_to_t, ebufp);
		PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_EVENT_TYPE, prod_event_type, ebufp);
		PIN_FLIST_FLD_PUT(ri_flistp, PIN_FLD_PERCENT, pbo_decimal_from_str("0", ebufp), ebufp); // If any discount, Set PERCENT to zero as we handle it in scale
		// Calculate SCALE to be refunded
		if(is_extn == 0){
			orig_quantity = (*charged_to_t - *charged_from_t)/60/60/24;
			quantity = (*charged_to_t - fm_utils_time_round_to_midnight(current_t))/60/60/24;
		}
		else if(is_extn == 1) //Check if validity extn is done only on current plan, then take ORIG VALIDITY END DATE
		{
			orig_quantity = (*orig_charged_to_t - *charged_from_t)/60/60/24;
			quantity = (*charged_to_t - fm_utils_time_round_to_midnight(current_t))/60/60/24;
		}else if(is_extn == 2){  //Valdiity extnesion before  or after or both topups
			if(is_curr_plan == 1){ //If current plan is having validity extn (not deduct)
				orig_quantity = ((*charged_to_t - *charged_from_t)/60/60/24)+deduct_days-(diff_days-curr_diff_days);
				quantity = ((*charged_to_t - fm_utils_time_round_to_midnight(current_t))/60/60/24)-extn_days;
			}
			else{
				//In this case, if extn is not on current plan, exclude diff days 
				orig_quantity = ((*charged_to_t - *charged_from_t)/60/60/24)+deduct_days-diff_days; 
				quantity = ((*charged_to_t - fm_utils_time_round_to_midnight(current_t))/60/60/24)-extn_days;
			}
		}
		scale = quantity/orig_quantity;


		if((is_extn !=2 && is_exclud !=2) || (opcode && *opcode == MSO_OP_CUST_BB_HW_AMC))
		{
			final_disc_perc = pbo_decimal_to_double(total_disc_perc, ebufp);
			final_disc_perc = final_disc_perc/(count+1); //count+1 = topups+actual_charge 2.5/3 = 0.833333333
		}

		//If both PERCENT and DISCOUNT are not zero and this might happen due to TOPUPS so then add the DISCOUNT amount as prorated amount.
		if(!pbo_decimal_is_null(total_disc, ebufp) && !pbo_decimal_is_zero(total_disc, ebufp) 
				&& !pbo_decimal_is_null(total_disc_perc, ebufp) && !pbo_decimal_is_zero(total_disc_perc, ebufp)){
			pbo_decimal_multiply_assign(total_disc, pbo_decimal_from_double(scale, ebufp), ebufp);
		}
		if((is_extn !=2 && is_exclud !=2) || (opcode && *opcode == MSO_OP_CUST_BB_HW_AMC))
		{
			scale = scale + (scale*count);
		}
		if ((!opcode || (opcode && *opcode != MSO_OP_CUST_BB_HW_AMC)) && final_disc_perc!=0){
			scale = scale*final_disc_perc; //final scale = 2.94545445(based on refund dates+topups)*0.83333333333
		}

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "REFUND SCALE calculation:", ebufp);
		scalep = pbo_decimal_from_double(scale,ebufp);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "REFUND SCALE calculation22:", ebufp);
		//We arrive with some cases where scale could become -ve like charged_to_t < current or some toup and extension cases
		if(pbo_decimal_sign(scalep, ebufp) == -1 || pbo_decimal_is_zero(scalep, ebufp)){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No refund is required in case of charged_to_t is < current_date or some topup cases");
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_FLIST_DESTROY_EX(&plan_ret_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_plan_retflistp, NULL);
			PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&purch_prod_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&event_det_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&subscr_prods_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&ri_flistp, NULL);
			if(!pbo_decimal_is_null(ch_amt, ebufp))
				pbo_decimal_destroy(&ch_amt);
			if(!pbo_decimal_is_null(total_disc_perc, ebufp))
				pbo_decimal_destroy(&total_disc_perc);
			return;

		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "REFUND SCALE calculation:");
		if (ch_amt && (!pbo_decimal_is_null(ch_amt, ebufp) || !pbo_decimal_is_zero(ch_amt, ebufp))){
			pf_amt = pbo_decimal_from_str("0", ebufp);
			cf_amt = pbo_decimal_from_str("0", ebufp);

			pbo_decimal_multiply_assign(ch_amt, scalep, ebufp);
			pbo_decimal_negate_assign(ch_amt, ebufp);
			if((is_extn !=2 && is_exclud !=2) || (opcode && *opcode == MSO_OP_CUST_BB_HW_AMC))
			{	 
				if(!pbo_decimal_is_null(total_disc, ebufp) && !pbo_decimal_is_zero(total_disc, ebufp))
					pbo_decimal_add_assign(ch_amt, total_disc, ebufp);
			}
			pbo_decimal_round_assign(ch_amt, precision, round_mode, ebufp);
			cf_amt = pbo_decimal_copy(ch_amt, ebufp);
			pack_flistp = PIN_FLIST_CREATE(ebufp);
			city_flistp = PIN_FLIST_ELEM_ADD(pack_flistp, MSO_FLD_CITIES, 0,  ebufp);
			PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CYCLE_FEE_AMT, cf_amt, ebufp);
			PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_PURCHASE_FEE_AMT, pf_amt, ebufp);
			PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CHARGE_AMT, ch_amt, ebufp);
			PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CITY, city, ebufp);
			fm_mso_search_tax_code(ctxp, pack_flistp, act_pdp, bill_when, ch_amt, cf_amt, pf_amt, plan_name, &tot_amt, &taxes_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&pack_flistp, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH TAX DETAILS RETURN", taxes_flistp);
			prods_flistp = PIN_FLIST_ELEM_ADD(ri_flistp, PIN_FLD_BAL_IMPACTS, 0, ebufp);
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_TAX_CODE, tax_code, ebufp);	
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);	
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_OFFERING_OBJ, orig_cancel_offer_obj, ebufp);	
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_RATE_OBJ, bal_rate_obj, ebufp);	
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_RATE_TAG, "New Rate", ebufp);	
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_GL_ID, &bal_gl_id, ebufp);	
			PIN_FLIST_FLD_PUT(prods_flistp, PIN_FLD_QUANTITY, one_dp, ebufp);	
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_IMPACT_TYPE, &tax_imp_type, ebufp);	
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_PERCENT, scalep, ebufp);	
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_AMOUNT, ch_amt, ebufp);	
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_RESOURCE_ID, &resource_id, ebufp);
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_AMOUNT_DEFERRED, zero_decimalp, ebufp);	
			PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);	
			if(taxes_flistp && taxes_flistp !=(pin_flist_t *)NULL){
				elem_id = 0;
				cookie = NULL;
				tax_index = 6; //This is due taxation in hathway 
				tax_imp_type = 4;
				while ((tax_flistp = PIN_FLIST_ELEM_GET_NEXT(taxes_flistp, PIN_FLD_TAXES,
								&elem_id, 1, &cookie, ebufp)) != NULL) {
					prods_flistp = PIN_FLIST_ELEM_ADD(ri_flistp, PIN_FLD_BAL_IMPACTS, tax_index, ebufp);
					PIN_FLIST_FLD_COPY(tax_flistp, PIN_FLD_TAX_CODE, prods_flistp, PIN_FLD_TAX_CODE, ebufp);
					PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);	
					PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_RATE_TAG, "Tax", ebufp);	
					PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_QUANTITY, ch_amt, ebufp);	
					PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_IMPACT_TYPE, &tax_imp_type, ebufp);	
					PIN_FLIST_FLD_COPY(tax_flistp, PIN_FLD_TAX_CODE, prods_flistp, PIN_FLD_TAX_CODE, ebufp);
					PIN_FLIST_FLD_COPY(tax_flistp, PIN_FLD_PERCENT, prods_flistp, PIN_FLD_PERCENT, ebufp);
					PIN_FLIST_FLD_COPY(tax_flistp, PIN_FLD_TAX, prods_flistp, PIN_FLD_AMOUNT, ebufp);
					PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_RESOURCE_ID, &resource_id, ebufp);
					PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_AMOUNT_DEFERRED, zero_decimalp, ebufp);	
					PIN_FLIST_FLD_SET(prods_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);
					tax_index = tax_index+1;
				}
			}
			if (!pbo_decimal_is_null(tot_amt, ebufp) && ri_flistp != (pin_flist_t *)NULL){
				total_flistp = PIN_FLIST_ELEM_ADD(ri_flistp, PIN_FLD_TOTAL, 356, ebufp);
				PIN_FLIST_FLD_PUT(total_flistp, PIN_FLD_AMOUNT, tot_amt, ebufp);
			}
		}
		PIN_FLIST_FLD_PUT(ri_flistp, PIN_FLD_SCALE, scalep, ebufp); // If any discount given while purchase
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_generate_credit_charge refund iflist:", ri_flistp);
		fm_mso_create_refund_for_cancel_plan(ctxp, ri_flistp, &ro_flistp, ebufp );
		PIN_FLIST_DESTROY_EX(&ri_flistp, NULL);
		if(!pbo_decimal_is_null(total_disc_perc, ebufp))
			pbo_decimal_destroy(&total_disc_perc);
		if(!pbo_decimal_is_null(ch_amt, ebufp))
			pbo_decimal_destroy(&ch_amt);
		if(!pbo_decimal_is_null(cf_amt, ebufp))
			pbo_decimal_destroy(&cf_amt);
		if(!pbo_decimal_is_null(pf_amt, ebufp))
			pbo_decimal_destroy(&pf_amt);
		if(!pbo_decimal_is_null(zero_decimalp, ebufp))
			pbo_decimal_destroy(&zero_decimalp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_generate_credit_charge refund oflist:", ro_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in creating refund ");
			PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&purch_prod_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&event_det_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&subscr_prods_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&plan_ret_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&taxes_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_plan_retflistp, NULL);
			return ;
		}
		if (ro_flistp && ro_flistp != (pin_flist_t *)NULL){
			*r_flistpp = ro_flistp;
		}
		PIN_FLIST_DESTROY_EX(&prod_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&purch_prod_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&event_det_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&subscr_prods_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&plan_ret_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&taxes_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_plan_retflistp, NULL);
	}
	else{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Could be either TOPUP change plan or both plans are same so no disconnection credit");
		return;
	}
	return;
}


/*******************************************************************
 * Function to expire account level offers.
 *******************************************************************/
static void 
fm_mso_cust_expire_offers(
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t		*ebufp)
{

	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*mso_purch_flistp = NULL;
	pin_flist_t		*mso_prod_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*srvc_flistp = NULL;
	pin_flist_t		*debit_iflistp = NULL;
	pin_flist_t		*debit_oflistp = NULL;
	pin_flist_t		*debit_flistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*svc_up_flistp = NULL;
	pin_flist_t 		*mso_offer_flistp = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*ret_pdp = NULL;
	poid_t			*mso_bb_bp_obj = NULL;
	poid_t			*mso_bb_obj = NULL;

	int32			*flag = NULL;
	int32			type = 1;
	int32			*tech = NULL;
	int32			error_codep = 0;
	int32			opcode = MSO_OP_CUST_CREATE_OFFERS;
	int32			create_offer_failure = 1;
	int32			success = 0;
	int32			oflags = 1;
	int32			fup_status = 1;
	int32			prov_flags = DOC_BB_ADD_MB;
	int32			plan_type = 0;
	int32			res_id = 0;

	int			elem_id =0;
	int			is_apply_now = 0;
	int			ret_val =0;
	int			status = 1;

	char			*prov_tag = NULL; 
	char			*plan_name = NULL;

	char 			error_descr[200]="";
	char 			valid_to_str[50]="";

	time_t			valid_to_t = 0;
	time_t			valid_from_t = 0;
	time_t			*old_valid_to_t = NULL;

	pin_decimal_t		*initial_amount = NULL;
	pin_decimal_t		*amount = NULL;

	void			*vp = NULL;
	pin_cookie_t		cookie = NULL;
	int			*prov_call = NULL;
	int			fup_stat = 0;	

	int			fup_flag = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in calling fm_mso_cust_expire_offers", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_expire_offers input flist", i_flistp);



	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	//	valid_to_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_VALID_TO, 1, ebufp);
	//      Set valid_to_t as current date
	valid_to_t = pin_virtual_time(NULL);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TYPE,&type,ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS,&oflags,ebufp);


	if (srvc_pdp && srvc_pdp != NULL && strcmp(PIN_POID_GET_TYPE(srvc_pdp), "/service/telco/broadband") == 0){
		//	offer_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE, 1, ebufp);
		//		if (!offer_type || ( offer_type && (*offer_type != 1 && *offer_type != 2 )))

		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_FLAGS, &oflags, ebufp);
		fm_mso_search_offer_entries(ctxp, srch_flistp, &srch_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_expire_offers search output flist", srch_oflistp);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, 
					"Error in calling fm_mso_cust_expire_offers", ebufp);
			PIN_ERRBUF_CLEAR(ebufp);	
			return;

		}
		if (srch_oflistp && srch_oflistp !=(pin_flist_t *)NULL && PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp)  > 0){
			results_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			old_valid_to_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_VALID_TO, 1, ebufp);
			if (valid_to_t && old_valid_to_t && *old_valid_to_t > 0 && valid_to_t > *old_valid_to_t ){
				valid_from_t = *old_valid_to_t+1; // VALID_FROM should be equal to next of current expirey date 
				is_apply_now = 1; //If set to 1, no need to update balance and provision since offer was already applied.
			}
			else {
				/*
				   ret_flistp = PIN_FLIST_CREATE(ebufp);
				   PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
				   PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, ret_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				   PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_offer_failure, ebufp);
				   PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "71000", ebufp);
				   if(old_valid_to_t)
				   strftime( valid_to_str, sizeof(valid_to_str), "%d-%b-%Y",  localtime( (time_t *) old_valid_to_t) );
				   if(!old_valid_to_t || (old_valid_to_t && *old_valid_to_t == 0)){
				   strcpy(error_descr, "Account has already lifetime valid offer.");
				   }
				   else if (offer_type && *offer_type == 1)
				   {
				   sprintf(error_descr, "Please select VALIDITY END DATE greater than the existing discount offer validity end date %s", 	
				   valid_to_str);
				   }
				   else if (offer_type && *offer_type == 2)
				   {
				   sprintf(error_descr, "Please select VALIDITY END DATE  greater than the existing DATA offer validity end date %s", 
				   valid_to_str);
				   }
				   PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
				   error_descr, ebufp);
				 *r_flistpp = ret_flistp;
				 PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
				 return;
				 */
			}
		}
	}
	else{
		// Return
		//   Check if all offers have to be expired or only some.
		return;
	}

	//Proceed to expire /mso_cfg_account_offers
	//PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_VALID_FROM, &valid_from_t, ebufp);
	//PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_FLAGS, ebufp);
	//update_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	//PIN_FLIST_FLD_DROP(update_flistp, PIN_FLD_PROGRAM_NAME, ebufp);

	if(srch_oflistp && srch_oflistp != NULL )
	{
		while( (mso_offer_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS,
						&elem_id, 1, &cookie, ebufp)) != NULL )
		{
                        PIN_FLIST_FLD_SET(mso_offer_flistp, PIN_FLD_VALID_TO, &valid_to_t, ebufp);
                        PIN_FLIST_FLD_SET(mso_offer_flistp, PIN_FLD_VALID_FROM, &valid_to_t, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_offers update obj inflist", mso_offer_flistp);
                        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, mso_offer_flistp, &ret_flistp, ebufp);
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_expire_offers update obj outflist", ret_flistp);
		}

                        if (PIN_ERRBUF_IS_ERR(ebufp)){
                                PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
                                PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                "Error in calling fm_mso_cust_expire_offers", ebufp);
                                return;

                        }

		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_expire_offers update obj outflist", ret_flistp);
	}
	*r_flistpp = ret_flistp;
	return;

}

void
mso_get_tal_products(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t     *plan_list_flistp = NULL;
        pin_flist_t     *deal_flistp = NULL;
        pin_flist_t     *search_inflistp = NULL;
        pin_flist_t     *search_outflistp = NULL;
        pin_flist_t     *args_flistp  = NULL;
        pin_flist_t     *plan_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
        pin_flist_t     *cycle_fee_flistp = NULL;

        poid_t  *product_pdp = NULL;
        poid_t  *account_pdp = NULL;

        int             flags = 256;
        int             elem_pp = 0;
        int             plan_list_cnt = 0;
        int             deal_cnt = 0;
        int             plan_flag = 0;
        int             pur_prod_status = PIN_PRODUCT_STATUS_ACTIVE;
//      int             *packageid = NULL;

        int64           db = 1;

        pin_cookie_t    cookie_pp = NULL;

        time_t          purchase_end = 0;
        time_t          now = 0;

        char            *search_template = "select X from /purchased_product 1, /config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3 and 1.F4 = V4 ";

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_get_tal_products:input flist",in_flistp);

        account_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
        db = PIN_POID_GET_DB(account_pdp);

        plan_list_cnt = PIN_FLIST_ELEM_COUNT(in_flistp,PIN_FLD_PLAN_LISTS, ebufp);

        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Error in input flist add plan error", in_flistp);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in input flist add plan error", ebufp);
                return;

        }
        search_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, search_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp );
        plan_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ,account_pdp , ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 4, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS,&pur_prod_status , ebufp);

        results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search purchased product in", search_inflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search purchased product out", search_outflistp);

        PIN_FLIST_DESTROY_EX(&search_inflistp,NULL);

        if(search_outflistp)
        {
                while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(search_outflistp, PIN_FLD_RESULTS,
                                                                &elem_pp, 1, &cookie_pp, ebufp )) != NULL)
                {
                        product_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
                        //packageid = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PACKAGE_ID, 1, ebufp);
                        purchase_end = *(time_t*)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);

                        now = pin_virtual_time(NULL);
                        if((now < purchase_end) || (purchase_end == 0))
                        {
                                //product is active add plan info on flist
                                plan_list_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_SET(plan_list_flistp, PIN_FLD_FLAGS , &plan_flag, ebufp);
                                PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_PLAN_OBJ, plan_list_flistp,PIN_FLD_PLAN_OBJ,ebufp);
                                deal_flistp = PIN_FLIST_ELEM_ADD( plan_list_flistp, PIN_FLD_DEALS, deal_cnt++, ebufp);
                                PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_DEAL_OBJ, deal_flistp,PIN_FLD_DEAL_OBJ,ebufp);
                                PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_PACKAGE_ID, deal_flistp,PIN_FLD_PACKAGE_ID,ebufp);

                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Tal Plan details", plan_list_flistp);
                                *ret_flistp = PIN_FLIST_COPY(plan_list_flistp, ebufp);
                        }
                }
        }

        PIN_FLIST_DESTROY_EX(&search_outflistp,NULL);
        PIN_FLIST_DESTROY_EX(&plan_list_flistp,NULL);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Tal plan flist", *ret_flistp);

}

void
mso_cancel_static_ip_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *refund_calc_flistp,
        int32                   *indicator,
        int32                   plan_change_after_expiry_scn,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{

        pin_flist_t             *mso_plan_flistp = NULL;
        pin_flist_t             *planlists_flistp = NULL;
        pin_flist_t             *mso_prod_flistp = NULL;
        pin_flist_t             *charge_oflistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        pin_flist_t             *canceldeal_inflistp = NULL;
        pin_flist_t             *canceldeal_outflistp = NULL;
        pin_flist_t             *dealinfo_flistp = NULL;
        pin_flist_t             *sub_bal_flist = NULL;
//      pin_flist_t             *refund_calc_flistp = NULL;
        pin_flist_t             *ref_bal_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *readplan_inflistp = NULL;
        pin_flist_t             *readplan_outflistp = NULL;
        pin_flist_t             *deals_flistp = NULL;
        pin_flist_t             *old_mso_plan_flistp = NULL;
        pin_flist_t             *old_mso_pp_flistp = NULL;
        pin_flist_t             *tal_planlists_flistp = NULL;
        pin_flist_t             *temp_plan_flist = NULL;
        pin_flist_t             *plan_flistp = NULL;
        pin_flist_t             *plan_list_flistp = NULL;

        poid_t                  *acc_pdp = NULL;
        poid_t                  *cancel_plan_obj = NULL;
        poid_t                  *cancel_offer_obj = NULL;
        poid_t                  *cancel_prod_obj = NULL;
        poid_t                  *get_deal_obj = NULL;
        poid_t                  *read_deal_obj = NULL;
        poid_t                  *plan_obj = NULL;
        poid_t                  *base_plan_obj = NULL;
        poid_t                  *mso_pp_obj = NULL;

        void                    *vp = NULL;
        pin_cookie_t            cookie = NULL;
        pin_cookie_t            pcookie = NULL;
        pin_cookie_t            mcookie = NULL;
        pin_cookie_t            cookie_countdeals = NULL;

        pin_decimal_t           *total_charge = NULL;
        pin_decimal_t           *temp_amount = NULL;
        pin_decimal_t           *charge = NULL;
        pin_decimal_t           *tax = NULL;
        pin_decimal_t           *zero = pbo_decimal_from_str("0.0", ebufp);
        pin_decimal_t           *total_tal_charge = NULL;

        int                     *package_id = 0;
        int                     change_plan_failure = 1;

        int32                   elem_id = 0;
        int32                   pelemid = 0;
        int32                   melemid = 0;
        int32                   elem_countdeals = 0;
        int32                   update_mso_purch_plan = 0;
        int32                   price_calc_flag = 0;
        int32                   resource_id = 0;
        int32                   *action = NULL;

        char                    *rate_tag = NULL;
        char                    *plan_type = NULL;

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Static IP product cancellation started");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali : in_flistp");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Kali : in_flistp", in_flistp);
        mso_get_tal_products(ctxp,in_flistp, &planlists_flistp,ebufp);

        //Get old base plan object
        while ((plan_list_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN_LISTS,
                        &elem_id, 1, &cookie, ebufp)) != NULL)
        {
                action = PIN_FLIST_FLD_GET(plan_list_flistp, PIN_FLD_FLAGS, 1, ebufp);
                if (action && action != NULL && *action == 0){
                        base_plan_obj = PIN_FLIST_FLD_GET(plan_list_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
                }
        }
        elem_id = 0;
        cookie = NULL;
        if(planlists_flistp)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali : tal_plan_flist");
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Kali : tal_plan_flist", planlists_flistp);
        }
        else
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali : There is no active plan for cancellation");
                return;
        }

        /* Fetch the value of CALC ONLY flag*/
        vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_MODE, 1, ebufp );
        if(vp && *(int32 *)vp == 1) {
                price_calc_flag = 1;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
        }
        acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp );
        // Below is to give the disconnection credit (refund) for PREPAID plans during change plan only
        //      Get the actual base offer_obj getting cancelled and product
        if(refund_calc_flistp)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Refund ccal flist exists");
                total_charge = PIN_FLIST_FLD_GET(refund_calc_flistp, PIN_FLD_AMOUNT, 1, ebufp);
                charge = PIN_FLIST_FLD_GET(refund_calc_flistp, PIN_FLD_AMOUNT_ORIG, 1, ebufp);
                tax = PIN_FLIST_FLD_GET(refund_calc_flistp, PIN_FLD_AMOUNT_TAXED, 1, ebufp);
                total_tal_charge = PIN_FLIST_FLD_GET(refund_calc_flistp, MSO_FLD_MISC_AMT, 1, ebufp);
        }
        else
        {
                refund_calc_flistp = PIN_FLIST_CREATE(ebufp);
        }
        if (*indicator == 2) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Processing for prepaid");
                cancel_plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );

                fm_mso_get_mso_purplan(ctxp,acc_pdp, cancel_plan_obj,MSO_PROV_ACTIVE,&mso_plan_flistp,ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp)){
                        PIN_FLIST_DESTROY_EX(&mso_plan_flistp, NULL);
                        return;
                }
                pelemid = 0;
                pcookie = NULL;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali : mso_plan_flistp");
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Kali : mso_plan_flistp", mso_plan_flistp);
                while((mso_prod_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_plan_flistp,
                                        PIN_FLD_PRODUCTS, &pelemid, 1, &pcookie, ebufp )) != NULL){
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali : Getting cancel poid for TAL");
                                cancel_offer_obj = PIN_FLIST_FLD_GET(mso_prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 1, ebufp);
                                cancel_prod_obj = PIN_FLIST_FLD_GET(mso_prod_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
                                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Log cancel_offer_obj :",cancel_offer_obj);
                                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Log cancel_prod_obj :",cancel_prod_obj);

                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Planlist flist", planlists_flistp);

                //This is to generate disconnect charge for prepaid change plan and just call without calc flag
                if( cancel_offer_obj && cancel_prod_obj)
                {
                        charge_oflistp = PIN_FLIST_CREATE(ebufp);
                        temp_plan_flist = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, temp_plan_flist, PIN_FLD_USERID, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, temp_plan_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, temp_plan_flist, PIN_FLD_SERVICE_OBJ, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, temp_plan_flist, PIN_FLD_POID, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BILL_WHEN, temp_plan_flist, PIN_FLD_BILL_WHEN, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, temp_plan_flist, PIN_FLD_PROGRAM_NAME, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_MODE, temp_plan_flist, PIN_FLD_MODE, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_FLAGS, temp_plan_flist, PIN_FLD_FLAGS, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_AGREEMENT_NO, temp_plan_flist, MSO_FLD_AGREEMENT_NO, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CITY, temp_plan_flist, PIN_FLD_CITY, ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_OLD_VALUE, temp_plan_flist, MSO_FLD_OLD_VALUE, ebufp);
                        PIN_FLIST_FLD_SET(temp_plan_flist, MSO_FLD_ISTAL_FLAG, &change_plan_failure, ebufp);
                        PIN_FLIST_FLD_SET(temp_plan_flist, PIN_FLD_PLAN_OBJ, base_plan_obj, ebufp);

                        plan_flistp = PIN_FLIST_ELEM_ADD( temp_plan_flist, PIN_FLD_PLAN_LISTS, 0, ebufp);
                        PIN_FLIST_FLD_COPY(planlists_flistp, PIN_FLD_PLAN_OBJ, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
                        PIN_FLIST_FLD_COPY(planlists_flistp, PIN_FLD_FLAGS, plan_flistp, PIN_FLD_FLAGS, ebufp);
                        PIN_FLIST_FLD_COPY(planlists_flistp, PIN_FLD_DEALS, plan_flistp, PIN_FLD_DEALS, ebufp);

                        //PIN_FLIST_CONCAT(temp_plan_flist,planlists_flistp,ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali : Charhe calculation Input flist");
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "TAL prepaid refund flist", temp_plan_flist);
                        fm_mso_generate_credit_charge(ctxp, temp_plan_flist, cancel_offer_obj, cancel_prod_obj, &charge_oflistp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali : charge_oflistp");
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "TAL prepaid refund flist", charge_oflistp);
                        if  (PIN_ERRBUF_IS_ERR(ebufp)){
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in while proividing for disconnection credit", ebufp);
                                PIN_FLIST_DESTROY_EX(&mso_plan_flistp, NULL);
                                PIN_FLIST_DESTROY_EX(&charge_oflistp, NULL);
                                PIN_ERRBUF_RESET(ebufp);
                                r_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51770", ebufp);
                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service change plan - Error while disconnection credit for prepaid", ebufp);
                                *ret_flistp = r_flistp;
                                return;
                        }
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali : charge_oflistp checking");
                        if(charge_oflistp){
                                result_flistp = PIN_FLIST_ELEM_GET(charge_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                                if(result_flistp){
                                        //This is added to add the lifecycle in case prepaid dc
                                        PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_CYCLE_INFO, planlists_flistp, PIN_FLD_CYCLE_INFO, ebufp);
                                        //Added block to capture the Refund amount for price calculator
                                        melemid = 0;
                                        mcookie = NULL;
                                        //PIN_FLIST_FLD_COPY(charge_oflistp, PIN_FLD_POID, refund_calc_flistp, PIN_FLD_POID, ebufp);
                                        while((ref_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp,
                                                      PIN_FLD_BAL_IMPACTS, &melemid, 1, &mcookie, ebufp ))!= NULL)
                                        {
                                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bal impact flist", ref_bal_flistp);
                                                rate_tag = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_RATE_TAG, 1, ebufp);
                                                temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                                                if(pbo_decimal_is_null(total_charge, ebufp)) {
                                                        PIN_ERR_LOG_MSG(3,"Step1");
                                                        total_charge = temp_amount;
                                                 } else {

                                                        total_charge = pbo_decimal_add(temp_amount,total_charge, ebufp);
                                                        PIN_ERR_LOG_MSG(3,"Step4");
                                                  }
                                                if(!(rate_tag && strcmp(rate_tag,"Tax") == 0))
                                                {
                                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering charge block");
                                                        temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                                                        if(pbo_decimal_is_null(charge, ebufp))
                                                        {
                                                                charge = temp_amount;
                                                                PIN_ERR_LOG_MSG(3,"Kali Tax setting");
                                                        }
                                                        else {
                                                                charge = pbo_decimal_add(temp_amount,charge, ebufp);
                                                                PIN_ERR_LOG_MSG(3,"Kali Tax adding");
                                                        }
                                                }
                                                else
                                                {
                                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering tax block");
                                                        temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                                                        if(pbo_decimal_is_null(tax, ebufp))
                                                        {
                                                                tax = temp_amount;
                                                                PIN_ERR_LOG_MSG(3,"Step3");
                                                        }
                                                        else {
                                                                tax = pbo_decimal_add(temp_amount,tax, ebufp);
                                                                PIN_ERR_LOG_MSG(3,"Step6");
                                                        }
                                                }
                                                if(pbo_decimal_is_null(total_tal_charge, ebufp)) {
                                                        PIN_ERR_LOG_MSG(3,"Tal Charge assign");
                                                        total_tal_charge = temp_amount;
                                                 } else {

                                                        total_tal_charge = pbo_decimal_add(temp_amount,total_tal_charge, ebufp);
                                                        PIN_ERR_LOG_MSG(3,"Tal Charge add");
                                                 }
                                        }
                                        //Populating Refund details
                                        if(!pbo_decimal_is_null(total_charge, ebufp)) {
                                                 PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT, total_charge, ebufp);
                                        }
                                        else {
                                                PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT, zero, ebufp);
                                        }
                                        if(!pbo_decimal_is_null(charge, ebufp)) {
                                                 PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_ORIG, charge, ebufp);
                                        }
                                        else {
                                                PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_ORIG, zero, ebufp);
                                        }
                                        if(!pbo_decimal_is_null(tax, ebufp)) {
                                                 PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_TAXED, tax, ebufp);
                                        }
                                        else {
                                               PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_TAXED, zero, ebufp);
                                        }
                                        if(!pbo_decimal_is_null(total_tal_charge, ebufp)) {
                                                 PIN_FLIST_FLD_SET(refund_calc_flistp, MSO_FLD_MISC_AMT, total_tal_charge, ebufp);
                                        }
                                        else {
                                               PIN_FLIST_FLD_SET(refund_calc_flistp, MSO_FLD_MISC_AMT, zero, ebufp);
                                        }
                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Refund calculated flist", refund_calc_flistp);

                                }
                        }
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"check2");
                }
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"check3");
        //Now revert with actual flgas..This is being done to avaoid proration for cancel scenariod.
        PIN_FLIST_DESTROY_EX(&charge_oflistp, NULL);
        PIN_FLIST_DESTROY_EX(&mso_plan_flistp, NULL);

        elem_countdeals = 0;
        cookie_countdeals = NULL;
        while ((args_flistp = PIN_FLIST_ELEM_GET_NEXT(planlists_flistp, PIN_FLD_DEALS,
                                        &elem_countdeals, 1, &cookie_countdeals, ebufp)) != NULL)
        {

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "args flist", args_flistp);
                get_deal_obj = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
                plan_obj = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
                if( PIN_POID_IS_NULL(plan_obj))
                {
                        base_plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
                        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Log base poid",base_plan_obj);
                        plan_obj = PIN_POID_COPY(base_plan_obj,ebufp);
                        update_mso_purch_plan = 0;
                } else {
                /************************************************************
                This flag is required to update mso_purchased_plan status to
                MSO_PROV_TERMINATE as the plan is cancelled.These plan are handled
                separately and not along with Base Plan. Hence this flag is required
                to identify if update is required.
                *************************************************************/
                        if(price_calc_flag == 0)
                                update_mso_purch_plan = 1;
                }
                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "get_deal_obj poid:",get_deal_obj);
                if (!plan_obj)
                {
                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51760", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service change plan - plan object is missing in input", ebufp);
                        *ret_flistp = r_flistp;
                        return;
                }
                readplan_inflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_POID, plan_obj, ebufp );
                PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_NAME, NULL, ebufp );
                PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_CODE, NULL, ebufp );
                deals_flistp = PIN_FLIST_ELEM_ADD(readplan_inflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
                PIN_FLIST_FLD_SET(deals_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp );
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read plan input list", readplan_inflistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readplan_inflistp, &readplan_outflistp, ebufp);
                PIN_FLIST_DESTROY_EX(&readplan_inflistp, NULL);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);

                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51761", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                                        "ACCOUNT - Service change plan - PCM_OP_READ_FLDS of plan poid error", ebufp);

                        *ret_flistp = r_flistp;
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " read plan output flist", readplan_outflistp);
                PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_CODE, planlists_flistp, PIN_FLD_CODE, ebufp);

                /***************************************************************************
                * Change plan Validations.
                ****************************************************************************/

                if(price_calc_flag == 0)
                {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali price_calc_flag 0 go for cancel");
                        elem_id = 0;
                        cookie = NULL;
                        while ((deals_flistp = PIN_FLIST_ELEM_GET_NEXT(readplan_outflistp, PIN_FLD_SERVICES,
                                                                        &elem_id, 1, &cookie, ebufp)) != NULL)
                        {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali deals flist");
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals flist", deals_flistp);
                                canceldeal_inflistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, canceldeal_inflistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, canceldeal_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DESCR, canceldeal_inflistp, PIN_FLD_DESCR, ebufp);
                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, canceldeal_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
                                dealinfo_flistp = PIN_FLIST_ELEM_ADD(canceldeal_inflistp, PIN_FLD_DEAL_INFO, 0, ebufp );
                                PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, dealinfo_flistp, PIN_FLD_DEAL_OBJ, ebufp);

                                read_deal_obj = PIN_FLIST_FLD_GET(deals_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
                                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Poid 1 :",read_deal_obj);
                                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Poid 2 :",get_deal_obj);
                                if ((PIN_POID_COMPARE(read_deal_obj, get_deal_obj, 0, ebufp)) == 0)
                                {
                                        package_id = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_PACKAGE_ID, 1, ebufp);
                                        if(!package_id || (package_id && *package_id == 0))
                                        {
                                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Package Id not provided in input");
                                                r_flistp = PIN_FLIST_CREATE(ebufp);
                                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                                                         "ACCOUNT - Change plan - Package Id not provided in input", ebufp);
                                                *ret_flistp = r_flistp;
                                                return;
                                        }

                                        PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_PACKAGE_ID, package_id, ebufp);
                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                                        "fm_mso_bb_change_plan cancel sub input list", canceldeal_inflistp);
                                        PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_CANCEL_DEAL, 0, canceldeal_inflistp, &canceldeal_outflistp, ebufp);
                                        PIN_FLIST_DESTROY_EX(&canceldeal_inflistp, NULL);
                                        if (PIN_ERRBUF_IS_ERR(ebufp))
                                        {
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                        "Error in calling PCM_OP_SUBSCRIPTION_CANCEL_DEAL", ebufp);
                                                PIN_ERRBUF_RESET(ebufp);
                                                PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);

                                                r_flistp = PIN_FLIST_CREATE(ebufp);
                                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                                                        "ACCOUNT - Service change plan - PCM_OP_SUBSCRIPTION_CANCEL_DEAL error", ebufp);
                                                *ret_flistp = r_flistp;
                                                return;
                                        }
                                        /****************************************************
                                        Update the mso_purchased_plan
                                        ****************************************************/
					if(update_mso_purch_plan == 1)
                                        {
                                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Updating mso_pur_plan for addon cancels");
                                                acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1,ebufp);
                                                fm_mso_get_mso_purplan(ctxp,acc_pdp, plan_obj,MSO_PROV_ACTIVE,&old_mso_plan_flistp,ebufp);
                                                if(old_mso_plan_flistp)
                                                {
                                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                                        "fm_mso_get_mso_purplan return flist for cancelling add plans", old_mso_plan_flistp);
                                                        mso_pp_obj = PIN_FLIST_FLD_GET(old_mso_plan_flistp, PIN_FLD_POID, 1,ebufp);
                                                        if(fm_mso_update_mso_purplan_status(ctxp,mso_pp_obj,MSO_PROV_TERMINATE,ebufp) == 0)
                                                        {
                                                                r_flistp = PIN_FLIST_CREATE(ebufp);
                                                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51762", ebufp);
                                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                                                                        "Error while updating mso_purhcased_plan status.", ebufp);
                                                                *ret_flistp = r_flistp;
                                                                return;
                                                        }
                                                }
                                        }
                                        PIN_FLIST_DESTROY_EX(&old_mso_plan_flistp,NULL);
                                }
                        }

                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_change_plan cancel output flist", canceldeal_outflistp);
                }
                //PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
                //Added block to fetch the refund value in case of postpaid
                if(price_calc_flag == 1 && *indicator == 1)
                {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali Postpaid price calculator . No cancel value required");
                        elem_id = 0;
                        cookie = NULL;
                        while ((deals_flistp = PIN_FLIST_ELEM_GET_NEXT(readplan_outflistp, PIN_FLD_SERVICES,
                                                                        &elem_id, 1, &cookie, ebufp)) != NULL)
                        {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Kali deals flist");
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals flist", deals_flistp);
                                canceldeal_inflistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, canceldeal_inflistp, PIN_FLD_POID, ebufp);
                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, canceldeal_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DESCR, canceldeal_inflistp, PIN_FLD_DESCR, ebufp);
                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, canceldeal_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
                                dealinfo_flistp = PIN_FLIST_ELEM_ADD(canceldeal_inflistp, PIN_FLD_DEAL_INFO, 0, ebufp );
                                PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, dealinfo_flistp, PIN_FLD_DEAL_OBJ, ebufp);
                        //PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_PACKAGE_ID, dealinfo_flistp, PIN_FLD_PACKAGE_ID, ebufp);

                                read_deal_obj = PIN_FLIST_FLD_GET(deals_flistp, PIN_FLD_DEAL_OBJ, 1, ebufp);
                                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Poid 1 :",read_deal_obj);
                                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Poid 2 :",get_deal_obj);
                                if ((PIN_POID_COMPARE(read_deal_obj, get_deal_obj, 0, ebufp)) == 0)
                                {
                                        package_id = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_PACKAGE_ID, 1, ebufp);
                                        if(!package_id || (package_id && *package_id == 0))
                                        {
                                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Package Id not provided in input");
                                                r_flistp = PIN_FLIST_CREATE(ebufp);
                                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                                                         "ACCOUNT - Change plan - Package Id not provided in input", ebufp);
                                                *ret_flistp = r_flistp;
                                                return;
                                        }

                                        PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_PACKAGE_ID, package_id, ebufp);
                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                                        "fm_mso_bb_change_plan cancel sub input list", canceldeal_inflistp);
                                        PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_CANCEL_DEAL, PCM_OPFLG_CALC_ONLY, canceldeal_inflistp, &canceldeal_outflistp, ebufp);
                                        PIN_FLIST_DESTROY_EX(&canceldeal_inflistp, NULL);
                                        if (PIN_ERRBUF_IS_ERR(ebufp))
                                        {
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                        "Error in calling PCM_OP_SUBSCRIPTION_CANCEL_DEAL", ebufp);
                                                PIN_ERRBUF_RESET(ebufp);
                                                PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);

                                                r_flistp = PIN_FLIST_CREATE(ebufp);
                                                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51426", ebufp);
                                                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
                                                        "ACCOUNT - Service change plan - PCM_OP_SUBSCRIPTION_CANCEL_DEAL error", ebufp);
                                                *ret_flistp = r_flistp;
                                                return;
                                        }
                                        PIN_FLIST_DESTROY_EX(&old_mso_plan_flistp,NULL);
                                }

                                melemid = 0;
                                mcookie  = NULL;
                                resource_id = 0;
                                while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(canceldeal_outflistp,
                                                         PIN_FLD_RESULTS, &melemid, 1, &mcookie, ebufp ))!= NULL)
                                {
                                        sub_bal_flist = PIN_FLIST_ELEM_GET(result_flistp, PIN_FLD_SUB_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp);
                                        if(sub_bal_flist)
                                        {
                                                resource_id = *(int32 *) PIN_FLIST_FLD_GET(sub_bal_flist, PIN_FLD_RESOURCE_ID, 0, ebufp);
                                                if(resource_id == 356)
                                                {
                                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Currency impact found");
                                                        break;
                                                }
                                        }
                                }
                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Balimpact flist",result_flistp);
                                if(result_flistp)
                                {
                                        //refund_calc_flistp = PIN_FLIST_CREATE(ebufp);
                                        melemid = 0;
                                        mcookie  = NULL;
                                        //PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, refund_calc_flistp, PIN_FLD_POID, ebufp);
                                        while((ref_bal_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp,
                                                                PIN_FLD_BAL_IMPACTS, &melemid, 1, &mcookie, ebufp ))!= NULL)
                                        {
                                                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bal impact flist", ref_bal_flistp);
                                                resource_id = *(int32 *)PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_RESOURCE_ID, 0, ebufp);
                                                rate_tag = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_RATE_TAG, 1, ebufp);
                                                if(resource_id == 356) {
                                                        temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                                                        if(pbo_decimal_is_null(total_charge, ebufp)) {
                                                                PIN_ERR_LOG_MSG(3,"Step1");
                                                                total_charge = temp_amount;
                                                        } else {
                                                                total_charge = pbo_decimal_add(temp_amount,total_charge, ebufp);
                                                                PIN_ERR_LOG_MSG(3,"Step4");
                                                        }
                                                        if(!(rate_tag && strcmp(rate_tag,"Tax") == 0))
                                                        {
                                                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering charge block");
                                                                temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                                                                if(pbo_decimal_is_null(charge, ebufp))
                                                                {
                                                                        charge = temp_amount;
                                                                        PIN_ERR_LOG_MSG(3,"Step2");
                                                                }
                                                                else {
                                                                        charge = pbo_decimal_add(temp_amount,charge, ebufp);
                                                                        PIN_ERR_LOG_MSG(3,"Step5");
                                                                }
                                                        }
                                                        else
                                                        {
                                                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering tax block");
                                                                temp_amount = PIN_FLIST_FLD_GET(ref_bal_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                                                                if(pbo_decimal_is_null(tax, ebufp))
                                                                {
                                                                        tax = temp_amount;
                                                                        PIN_ERR_LOG_MSG(3,"Step3");
                                                                }
                                                                else {
                                                                        tax = pbo_decimal_add(temp_amount,tax, ebufp);
                                                                        PIN_ERR_LOG_MSG(3,"Step6");
                                                                }
                                                        }
                                                        if(pbo_decimal_is_null(total_tal_charge, ebufp)) {
                                                                PIN_ERR_LOG_MSG(3,"Tal Charge assign");
                                                                total_tal_charge = temp_amount;
                                                        } else {

                                                                total_tal_charge = pbo_decimal_add(temp_amount,total_tal_charge, ebufp);
                                                                PIN_ERR_LOG_MSG(3,"Tal Charge add");
                                                        }
                                                }
                                         }

                                        //Populating Refund details
                                        if(!pbo_decimal_is_null(total_charge, ebufp)) {
                                                PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT, total_charge, ebufp);
                                        }
                                        else {
                                                PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT, zero, ebufp);
                                        }
                                        if(!pbo_decimal_is_null(charge, ebufp)) {
                                                PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_ORIG, charge, ebufp);
                                        }
                                        else {
                                                PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_ORIG, zero, ebufp);
                                        }
                                        if(!pbo_decimal_is_null(tax, ebufp)) {
                                                PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_TAXED, tax, ebufp);
                                        }
                                        else {
                                                PIN_FLIST_FLD_SET(refund_calc_flistp, PIN_FLD_AMOUNT_TAXED, zero, ebufp);
                                        }
                                        if(!pbo_decimal_is_null(total_tal_charge, ebufp)) {
                                                PIN_FLIST_FLD_SET(refund_calc_flistp, MSO_FLD_MISC_AMT, total_tal_charge, ebufp);
                                        }
                                        else {
                                                PIN_FLIST_FLD_SET(refund_calc_flistp, MSO_FLD_MISC_AMT, zero, ebufp);
                                        }
                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Refund calculated flist", refund_calc_flistp);

                                }
                        }
                }
                PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
        }
        acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1,ebufp);
        /**********************************************************
        Purchase plan will call  PCM_OP_SUBSCRIPTION_PURCHASE_DEAL
        **********************************************************/

        if( plan_change_after_expiry_scn == 1)
        {
                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"base poid used",base_plan_obj);
                fm_mso_get_mso_purplan(ctxp,acc_pdp, base_plan_obj,MSO_PROV_DEACTIVE,&old_mso_pp_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"old_mso_flist returned-1", old_mso_pp_flistp);
        } else
        {
                PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"base poid used",base_plan_obj);
                fm_mso_get_mso_purplan(ctxp,acc_pdp, base_plan_obj,MSO_PROV_ACTIVE,&old_mso_pp_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"old_mso_flist returned-2", old_mso_pp_flistp);

        }
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_get_mso_purplan", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                PIN_FLIST_DESTROY_EX(&canceldeal_outflistp, NULL);
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51753", ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error while getting mso_purhcased_plan.", ebufp);
                *ret_flistp = r_flistp;
                return;
        }
        if(old_mso_pp_flistp)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside main function. Return Flist");
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_purplan return flist", old_mso_pp_flistp);
                mso_pp_obj = PIN_FLIST_FLD_GET(old_mso_pp_flistp, PIN_FLD_POID, 1,ebufp);
                plan_type = PIN_FLIST_FLD_TAKE(old_mso_pp_flistp, PIN_FLD_DESCR, 1,ebufp);
                if(fm_mso_update_mso_purplan_status(ctxp, mso_pp_obj, MSO_PROV_TERMINATE, ebufp ) == 0)
                {
                        r_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &change_plan_failure, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51762", ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Error while updating mso_purhcased_plan status.", ebufp);
                        *ret_flistp = r_flistp;
                        if(plan_type){
                                pin_free(plan_type);
                                plan_type = NULL;
                        }
                        return;
                }

        }
}

void
fm_mso_create_tal_change_lifecycle(
        pcm_context_t   *ctxp,
        pin_flist_t     *in_flist,
        int             *old_bill_when,
        pin_errbuf_t    *ebufp)
{

        pin_flist_t     *planlists_flistp = NULL;
        pin_flist_t     *robj_iflp = NULL;
        pin_flist_t     *robj_oflp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *city_flistp = NULL;
        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *s_oflistp = NULL;
        pin_flist_t     *s_n_oflistp = NULL;
        pin_flist_t     *pln_type_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;

        poid_t          *old_plan_obj = NULL;
        poid_t          *new_plan_obj = NULL;
        poid_t          *s_pdp = NULL;
        poid_t          *account_pdp = NULL;
        poid_t          *old_tal_plan_pdp = NULL;
        poid_t          *new_tal_plan_pdp = NULL;

        int32           elem_base = 0;

        int64           db = 1;

        pin_cookie_t    cookie_base = NULL;
        int             *action_flags = NULL;
        int             *bill_when = NULL;
        int             *waiver_flag = NULL;
        int             flags = 256;

        char            *indicator = NULL;
        char            *old_indicator = NULL;
        char            *old_tal_plan_code = NULL;
        char            *new_tal_plan_code = NULL;
        char            s_template[300] = " select X from /mso_cfg_credit_limit 1 ,/config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3  and 2.F4 = V4  and 1.F5 != V5 and not exists (select 1 from config_tal_plan_exempt c where c.plan_obj_id0 = ";
        char            s_template1[300] = " select X from /mso_cfg_credit_limit 1 ,/config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3  and 2.F4 = V4  and 1.F5 != V5 and not exists (select 1 from config_tal_plan_exempt c where c.plan_obj_id0 = ";
        char            s_template_zero[300] = " select X from /mso_cfg_credit_limit 1 ,/config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3  and 2.F4 = V4  and 1.F5 = V5 and not exists (select 1 from config_tal_plan_exempt c where c.plan_obj_id0 = ";
        char            s_template1_zero[300] = " select X from /mso_cfg_credit_limit 1 ,/config/product/tal 2 where 1.F1 = 2.F2 and 1.F3 = V3  and 2.F4 = V4  and 1.F5 = V5 and not exists (select 1 from config_tal_plan_exempt c where c.plan_obj_id0 = ";
        char            plan_poid_id[50];
        char            new_plan_poid_id[50];

        pin_decimal_t   *zero_amt = pbo_decimal_from_str("0", ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"input flist:",in_flist);

        account_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
        db = PIN_POID_GET_DB(account_pdp);
        old_indicator =(char *) PIN_FLIST_FLD_GET(in_flist, PIN_FLD_INDICATOR, 1, ebufp );

        while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, &elem_base, 1, &cookie_base, ebufp )) != NULL)
        {
                action_flags = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_FLAGS, 1, ebufp );
                if (*(int*)action_flags == 0 )
                {
                        old_plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
                        sprintf(plan_poid_id,"%lld ) ",PIN_POID_GET_ID(old_plan_obj));
                }
                else
                {
                        new_plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
                        sprintf(new_plan_poid_id,"%lld ) ",PIN_POID_GET_ID(new_plan_obj));
                }
        }

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,plan_poid_id);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,new_plan_poid_id);

        //Kali read service to get new bill_when

        robj_iflp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_SERVICE_OBJ,robj_iflp,PIN_FLD_POID, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(robj_iflp, MSO_FLD_BB_INFO, 0, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_ISTAL_FLAG, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_REAUTH_FLAG, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Is_tal input flist:",robj_iflp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflp, &robj_oflp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Is_tal output flist:",robj_oflp);

        PIN_FLIST_DESTROY_EX(&robj_iflp, NULL);


        args_flistp = PIN_FLIST_ELEM_GET(robj_oflp, MSO_FLD_BB_INFO, 0, 1, ebufp );
        bill_when = (int *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_BILL_WHEN, 1, ebufp);
        indicator      = (char *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_INDICATOR,1, ebufp);
        waiver_flag = (int *)PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_REAUTH_FLAG, 1, ebufp);

        //Kali get old plan details

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Searching Old plan details");
                // Retrieve all TAL products.
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        if(*waiver_flag == 0)
        {
                strcat(s_template,plan_poid_id);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
        }
        else
        {
                strcat(s_template_zero,plan_poid_id);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template_zero, ebufp);
        }
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, old_plan_obj, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        city_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_PLAN_OBJ, old_plan_obj, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, old_bill_when, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
        pln_type_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(pln_type_flistp, MSO_FLD_PLAN_TYPE, old_indicator, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
        city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CHARGE_AMT, zero_amt, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Input flist:",s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 512, s_flistp, &s_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Output flist:",s_oflistp);

        if(s_oflistp)
        {
		if(PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp) > 0)
                {
                results_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                old_tal_plan_pdp = PIN_FLIST_FLD_GET(results_flistp,PIN_FLD_PLAN_OBJ, 1, ebufp);
                old_tal_plan_code = PIN_FLIST_FLD_GET(results_flistp,MSO_FLD_PLAN_NAME, 1, ebufp);
		}
        }

        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

        //Kali get new plan details
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Searching New plan details");
                // Retrieve all TAL products.
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        if(*waiver_flag == 0)
        {
                strcat(s_template1,new_plan_poid_id);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template1, ebufp);
        }
        else
        {
                strcat(s_template1_zero,new_plan_poid_id);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template1_zero, ebufp);
        }
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, new_plan_obj, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        city_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_PLAN_OBJ, new_plan_obj, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
        pln_type_flistp = PIN_FLIST_ELEM_ADD(args_flistp,PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(pln_type_flistp, MSO_FLD_PLAN_TYPE, indicator, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
        city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CHARGE_AMT, zero_amt, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Input flist:",s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 512, s_flistp, &s_n_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Output flist:",s_n_oflistp);

        if(s_n_oflistp)
        {
		if(PIN_FLIST_ELEM_COUNT(s_n_oflistp, PIN_FLD_RESULTS, ebufp) > 0)
                {
                results_flistp = PIN_FLIST_ELEM_GET(s_n_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                new_tal_plan_pdp = PIN_FLIST_FLD_GET(results_flistp,PIN_FLD_PLAN_OBJ, 1, ebufp);
                new_tal_plan_code = PIN_FLIST_FLD_GET(results_flistp,MSO_FLD_PLAN_NAME, 1, ebufp);
		}
        }

        elem_base = 0;
        cookie_base = NULL;
        while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, &elem_base, 1, &cookie_base, ebufp )) != NULL)
        {
                action_flags = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_FLAGS, 1, ebufp );
                if (*(int*)action_flags == 0 )
                {
                        PIN_FLIST_FLD_SET(planlists_flistp, PIN_FLD_PLAN_OBJ, old_plan_obj, ebufp);
                        PIN_FLIST_FLD_SET(planlists_flistp, PIN_FLD_CODE, old_tal_plan_code, ebufp);
                }
                else
                {
                        PIN_FLIST_FLD_SET(planlists_flistp, PIN_FLD_PLAN_OBJ, new_plan_obj, ebufp);
                        PIN_FLIST_FLD_SET(planlists_flistp, PIN_FLD_CODE, new_tal_plan_code, ebufp);
                }
        }

        fm_mso_create_lifecycle_evt(ctxp, in_flist, NULL,ID_CHANGE_PLAN, ebufp );
        PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
        PIN_FLIST_DESTROY_EX(&s_n_oflistp, NULL);
}


static void
fm_mso_get_active_purch_prod_excp_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *inp_flistp,
        poid_t                  *plan_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_iflistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *s_oflistp = NULL;
        poid_t                  *s_pdp = NULL;
        int32                   flags = 256;
        int64                   db = 0;
        int64                   id = -1;
        char                    *s_template = " select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 and F4 != V4 ";
        int32                   act_status = 1;
        poid_t                  *serv_pdp = NULL;
        poid_t                  *act_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_active_purch_prod_excp_plan input flist", inp_flistp);

        act_pdp = (poid_t *)PIN_FLIST_FLD_GET(inp_flistp, PIN_FLD_POID, 0, ebufp);
        serv_pdp = (poid_t *)PIN_FLIST_FLD_GET(inp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);

        db = PIN_POID_GET_DB(act_pdp);
        s_iflistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_PUT(s_iflistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, s_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
		
	args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

       	args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PACKAGE_ID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_active_purch_prod_excp_plan search input list", s_iflistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&s_iflistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_active_purch_prod_excp_plan - Error in calling SEARCH", ebufp);
                PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_active_purch_prod_excp_plan search output flist", s_oflistp);
        *ret_flistpp = PIN_FLIST_COPY(s_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
	return;
}

static int
fm_mso_change_plan_val(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_in_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *planlists_flistp = NULL;
        int32                   srch_flag = 512;
        time_t                  current_t = 0;
        int                     db = 1;
        poid_t          *a_pdp = NULL;
        poid_t          *old_pl_pdp = NULL;
        poid_t          *new_pl_pdp = NULL;
		int                     elem_base = 0;
        int             *action_flags = NULL;
		pin_cookie_t            cookie_base = NULL;

		

        char                    *template = "select X from /mso_cfg_change_plan_excl where F1 = V1 ";


		while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS, &elem_base, 1, &cookie_base, ebufp )) != NULL)
		{
			action_flags = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_FLAGS, 0, ebufp );
			if(*(int*)action_flags == 1)
			{				
				new_pl_pdp = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
			}
			else if(*(int*)action_flags == 0)
			{
				old_pl_pdp = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
			}
		}

		if(!PIN_POID_COMPARE(new_pl_pdp, old_pl_pdp, 0,ebufp))	
		{
			return 0;
		}



		
        srch_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PLAN_OBJ, old_pl_pdp, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan_val search input flist", srch_in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_plan_val output flist", srch_out_flistp);

        if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
        {
                PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                return 0;
        }
        else
        {
	        PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			return 1;

        }
                                                        
}
