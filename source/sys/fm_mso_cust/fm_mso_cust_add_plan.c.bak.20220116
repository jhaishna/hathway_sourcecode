/*********************************************************************************************************
 * File:	fm_mso_cust_add_plan.c
 * Opcode:	MSO_OP_CUST_ADD_PLAN 
 * Owner:	Leela Pratyush
 * Created:	18-AUG-2014
 * Description: This opcode is for purchasing new plan

 r << xx 1
 0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
 0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 44029 10
 0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 16
 0 PIN_FLD_PROGRAM_NAME    STR [0] "Customer Center"
 0 PIN_FLD_PLAN_LISTS	ARRAY [0]
 1     PIN_FLD_PLAN_OBJ    POID [0] 0.0.0.1 /plan 41089 8
 1     PIN_FLD_DEALS          ARRAY [0] allocated 5, used 5
 2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 42530 0
 1     PIN_FLD_DEALS          ARRAY [1] allocated 5, used 5
 2         PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 43554 0
 xx
 xop 11007 0 1

xop: opcode 11006, flags 0
# number of field entries allocated 20, used 4
0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 41345 11
0 PIN_FLD_STATUS         ENUM [0] 0
0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/catv 37393 16
0 PIN_FLD_DESCR           STR [0] "ACCOUNT - Service add plan completed successfully"

 *
 * Modification History:
 * Modified By: Jyothirmayi Kachiraju
 * Date: 19th Nov 2019
 * Modification details : Support for new events created for 360, 330, 300, 270, 240, 210, 150 days.
 *
 *
 * Modified By: Jyothirmayi Kachiraju
 * Date: 10th Apr 2020
 * Modification details : Added functionality to send the SMS Notifications when ever an 
 *			  additional GB FUP Plan is purchased from Quick Pay / OAP.
 *
 *********************************************************************************************************/
 
#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_add_plan.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "pin_cust.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_prov.h"
#include "mso_lifecycle_id.h"
#include "mso_device.h"
#include "mso_utils.h"
#include "mso_errs.h"
#include "mso_ntf.h"

#define NCF_PKG_TYPE	3
#define ALL_PKG_TYPE	10	
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

extern  void
fm_mso_validation_rule_1(
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		int32			action,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

extern void
fm_mso_commission_error_processing (
		pcm_context_t           *ctxp,
		int                     error_no,
		char                    *error_in,
		pin_flist_t             *i_flistp,
		pin_flist_t             **r_flistpp,
		pin_errbuf_t            *ebufp);

PIN_IMPORT void
fm_mso_utils_get_catv_children(
		pcm_context_t           *ctxp,
		poid_t                  *act_pdp,
		pin_flist_t             **o_flistpp,
		pin_errbuf_t            *ebufp);

PIN_IMPORT void
fm_mso_utils_get_catv_main(
		pcm_context_t           *ctxp,
		poid_t                  *act_pdp,
		pin_flist_t             **o_flistpp,
		pin_errbuf_t            *ebufp);

PIN_IMPORT void
fm_mso_hier_group_get_parent(
		pcm_context_t		*ctxp,
		poid_t			*serv_pdp,
		pin_flist_t		**ret_flistpp,
		pin_errbuf_t		*ebufp);

PIN_EXPORT void
mso_bill_pol_catv_main(
        pcm_context_t           *ctxp,
        u_int                   flags,
        poid_t                  *binfo_pdp,
        poid_t                  *service_pdp,
        time_t                  event_t,
        pin_opcode_t            op_num,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
/* NTO void
   fm_mso_validation_rule_2(
   pcm_context_t		*ctxp,
   pin_flist_t		*i_flistp,
   int32			action,
   pin_flist_t		**ret_flistp,
   pin_errbuf_t		*ebufp); */

void
fm_mso_retrieve_pdt_info(
		pcm_context_t		*ctxp,
		pin_flist_t			*i_flistp,
		pin_flist_t			**ret_flistp,
		pin_errbuf_t		*ebufp);

void
fm_mso_pdt_channels_validation(
		pcm_context_t		*ctxp,
		pin_flist_t		*i_flistp,
		poid_t			*srvc_pdp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

void
fm_mso_get_channels(
		pcm_context_t		*ctxp,
		poid_t				*user_pdp,
		poid_t				*pdt_pdp,
		int					*elem_id,
		pin_flist_t			**ret_flistp,
		pin_errbuf_t		*ebufp);

int
fm_mso_find_channel_existence(
		pcm_context_t		*ctxp,
		pin_flist_t			*new_flistp,
		pin_flist_t			*existing_flistp,
		pin_errbuf_t		*ebufp);

extern int 
fm_mso_validate_csr_role(
		pcm_context_t		*ctxp,
		int64			db,
		poid_t			*user_id,
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

/* NTO
   extern void 
   fm_mso_et_product_rule_delhi(
   pcm_context_t		*ctxp,
   pin_flist_t		*i_flist,
   poid_t			*acnt_pdp,
   int32			action,
   pin_flist_t		**ret_flistp,
   pin_errbuf_t		*ebufp); */

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

extern void
fm_mso_add_prod_info(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
		pin_flist_t			*in_flistp,
		pin_errbuf_t		*ebufp);

extern int
fm_mso_update_mso_purplan_status(
		pcm_context_t           *ctxp,
		poid_t                  *mso_purplan_obj,
		int						status,
		pin_errbuf_t            *ebufp);

extern int
fm_mso_update_ser_prov_status(
		pcm_context_t		*ctxp,
		pin_flist_t			*in_flistp,
		int 				prov_status,
		pin_errbuf_t            *ebufp);

//extern void
//get_evt_lifecycle_template(
//	pcm_context_t			*ctxp,
//	int64				db,
//	int32				string_id, 
//	int32				string_version,
//	char				**lc_template, 
//	pin_errbuf_t			*ebufp);

static void fm_mso_cust_call_prov(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flist,
		pin_flist_t             **r_flistp,
		pin_errbuf_t            *ebufp);

static int fm_mso_cust_call_notif(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flist,
		pin_errbuf_t            *ebufp);

static void
fm_mso_get_curr_mso_purplan(
		pcm_context_t           *ctxp,
		poid_t					*acc_pdp,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);

static int
fm_mso_set_prod_end_date(
		pcm_context_t           *ctxp,
		poid_t				*offering_obj,
		int32				*indicator,
		pin_flist_t			*pp_flistp,
		pin_errbuf_t            *ebufp);

static void 
fm_mso_activate_child_alc_addon_pdts(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flistp,
		poid_t          *product_pdp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

PIN_IMPORT int32
fm_mso_catv_pt_pkg_type(
		pcm_context_t           *ctxp,
		poid_t                  *prd_pdp,
		pin_errbuf_t            *ebufp);

PIN_IMPORT int32
fm_mso_catv_conn_type(
		pcm_context_t           *ctxp,
		poid_t                  *srv_pdp,
		pin_errbuf_t            *ebufp);

extern time_t
fm_prov_get_expiry(
		pcm_context_t                   *ctxp,
		poid_t                          *mso_plan,
		poid_t                          *ac_pdp,
		poid_t                          *srv_pdp,
		int                             srv_typ,
		time_t                          *start,
		pin_errbuf_t                    *ebufp);

extern void
fm_mso_cust_get_bp_obj(
		pcm_context_t           *ctxp,
		poid_t                  *account_obj,
		poid_t                  *service_obj,
		int                     mso_status,
		poid_t                  **bp_obj,
		poid_t                  **mso_obj,
		pin_errbuf_t            *ebufp);


/**************************************
 * External Functions end
 ***************************************/

/**************************************
 * Local Functions start
 ***************************************/
EXPORT_OP int 
fm_mso_add_deal(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
        int32           flags,
		pin_flist_t		*in_flist,
		pin_flist_t		**ret_flistp,
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
		pin_flist_t		**ret_flistp,
		pin_errbuf_t            *ebufp);

static	void
fm_mso_get_cust_active_plans(
		pcm_context_t           *ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t            *ebufp);

void
fm_mso_get_alc_addon_pdts(
		pcm_context_t		*ctxp,
		poid_t			*acct_pdp,
		poid_t			*srvc_pdp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

PIN_EXPORT void
fm_mso_get_base_pdts(
		pcm_context_t		*ctxp,
		poid_t			    *acct_pdp,
		poid_t			    *srvc_pdp,
		pin_flist_t		    **ret_flistp,
		pin_errbuf_t		*ebufp);

static void 
fm_mso_activate_alc_addon_pdts(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flistp,
		poid_t          *product_pdp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

PIN_EXPORT int 
fm_mso_get_cust_type(
		pcm_context_t		*ctxp,
		poid_t  			*acct_pdp,
		pin_errbuf_t		*ebufp);

void
fm_mso_get_custcity(
		pcm_context_t   *ctxp,
		poid_t          *acct_obj,
		char          **cust_city,
		pin_errbuf_t    *ebufp);

void
fm_mso_get_grace_config(
		pcm_context_t   *ctxp,
		int             *pp_type,
		char            *cust_city,
		pin_flist_t     **out_flistp,
		pin_errbuf_t    *ebufp);

int 
fm_mso_valid_reactivate(
		pcm_context_t		*ctxp,
		poid_t              *acct_pdp,
		pin_flist_t         *results_flistp,
		pin_errbuf_t		*ebufp);

PIN_EXPORT time_t 
fm_mso_base_pack_validation(
		pcm_context_t   *ctxp,
		pin_flist_t     *alc_addon_pdts_flistp,
		int32		is_base_pack,
		pin_errbuf_t    *ebufp);


PIN_EXPORT void
fm_mso_get_service_level_discount(
		pcm_context_t           *ctxp,
		pin_flist_t		*in_flistp,
		int32			dpo_flag,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t            *ebufp);

void
fm_mso_get_plan_level_discount(
		pcm_context_t           *ctxp,
		pin_flist_t		        *input_flistp,
		pin_flist_t		        **ret_flistp,
		pin_errbuf_t            *ebufp);

void
fm_mso_get_das_type(
		pcm_context_t   *ctxp,
		poid_t          *acct_obj,
		char            **das_type,
		pin_errbuf_t    *ebufp);

static int fm_mso_validate_tal_plan_addition(
		pcm_context_t   *ctxp,
		pin_flist_t     *in_flistp,
		pin_flist_t     **rs_flistp,
		pin_errbuf_t    *ebufp);

/*******************************************************************
 * fm_mso_cust_add_gb_notification 
 * 
 * This function checks if the FUP Plan being added is related to 
 * any of the additonal GB plans and triggers the SMS / Email 
 * notifications
 *******************************************************************/	
static void fm_mso_cust_add_gb_notification(
		pcm_context_t           *ctxp,
		time_t					*grant_valid_from_date,
		time_t					*grant_valid_to_date,
		pin_decimal_t			*grant_amount,
		pin_flist_t             *in_flist,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);

/*************************************************************************
 * fm_mso_cust_get_grant_expiry_date 
 * 
 * This function gets the active base plan based on the account object,
 * service object and plan object and calculates the grant expiry date
 *************************************************************************/	
void fm_mso_cust_get_grant_expiry_date(
		pcm_context_t           *ctxp,
		poid_t			*account_obj,
		poid_t                  *service_obj,
		poid_t                  *plan_obj,
		poid_t			*mso_pp_obj,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);

/**************************************
 * Local Functions end
 ***************************************/

/*******************************************************************
 * MSO_OP_CUST_ADD_PLAN 
 *
 * This policy provides a hooks for enhancing OP_ACT_POL_AUTHORIZE
 * before aclling the actual opcode.
 *******************************************************************/

EXPORT_OP void
op_mso_cust_add_plan(
		cm_nap_connection_t	*connp,
		int32			opcode,
		int32			flags,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t		*ebufp);

static void
fm_mso_cust_add_plan(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
		int32			flags,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t		*ebufp);

/*********
 * Functions added for Plan renewal 
 *********/
extern void
fm_mso_reactivate_plan(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flist,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);

void
fm_mso_search_event_map(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flist,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);

static void
fm_mso_bb_add_plan(
		cm_nap_connection_t	*connp,
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flist,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);

extern void
fm_mso_purchase_bb_plans(
		cm_nap_connection_t     *connp,
		pin_flist_t             *mso_act_flistp,
		char			*prov_tag,
		int			flags,
		int			grace_glag,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp);

static void
fm_mso_update_device_plan_obj(
		pcm_context_t		*ctxp,
		pin_flist_t			*in_flist,
		poid_t			*mso_pp_obj,
		int			device_type,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp);

//Future dated renewal updation
void fm_mso_future_dated_renewal(
		pcm_context_t *ctxp,
		pin_flist_t    *in_flistp,
		pin_errbuf_t    *ebufp);

//Get deal discounts
void fm_mso_fetch_deal_disc(
		pcm_context_t   *ctxp,
		pin_flist_t     *in_flistp,
		pin_errbuf_t    *ebufp);

extern void
get_last_mso_purchased_plan_modified(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flist,
		poid_t                  *plan_pdp,
		int32                   flag,
		pin_flist_t             **bb_ret_flistp,
		pin_errbuf_t            *ebufp);

extern void
fm_mso_get_poid_details(
		pcm_context_t           *ctxp,
		poid_t                  *poid_pdp,
		pin_flist_t             **ret_flistpp,
		pin_errbuf_t            *ebufp);

int32
fm_validate_prov_ca_id(
		pcm_context_t   *ctxp,
		poid_t          *srvc_pdp,
		char            *prov_tag,
		pin_errbuf_t   *ebufp);
/*******************************************************************
 * MSO_OP_CUST_ADD_PLAN  
 *******************************************************************/
void 
op_mso_cust_add_plan(
		cm_nap_connection_t	*connp,
		int32			opcode,
		int32			flags,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	pin_flist_t             *rs_flistpp = NULL;
	int			local = LOCAL_TRANS_OPEN_SUCCESS;
	int			*status = NULL;
	int                     tal_check = 0;
	int32			*renew_flag = NULL;
	int32			status_uncommitted =2;
	int32			price_calc_flag = 0;
	poid_t			*inp_pdp = NULL;
	char            *prog_name = NULL;
	char            *descr = NULL;
	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_ADD_PLAN) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_mso_cust_add_plan error",
				ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"op_mso_cust_add_plan input flist", i_flistp);
	/*******************************************************************
	 * Call the default implementation 
	 ******************************************************************
	 inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	 local = fm_mso_trans_open(ctxp, inp_pdp, READWRITE,ebufp);
	 if(PIN_ERRBUF_IS_ERR(ebufp))
	 {
	 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Err before calling fm_mso_cust_add_plan()", ebufp);
	 return;
	 }*/

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	renew_flag	=	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	/* Fetch the value of CALC ONLY flag*/
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 1, ebufp );
	if(vp && *(int32 *)vp == 1) {
		price_calc_flag = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
	}

	if ( descr && ((strcmp(descr,"Bulk_add_plan") ==0)||(strcmp(descr,"Bulk_renew_plan") ==0)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'Bulk_add_plan' so transaction will not be handled at APIlevel");
	}
	else
	{
		//	if (!(prog_name && strcmp(prog_name,"pin_deferred_act") ==0 ))
		if (!(prog_name && strstr(prog_name,"pin_deferred_act")))
		{
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

	/*
	 * If flag then it is renewal call
	 */
	if((renew_flag) && (*renew_flag == 1))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " This is renewal plan ");
		fm_mso_reactivate_plan(ctxp, i_flistp, r_flistpp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Return flist final is ", *r_flistpp);
	}
	else
	{
		tal_check = fm_mso_validate_tal_plan_addition(ctxp,i_flistp,r_flistpp,ebufp);
		if(tal_check == 1) {
			fm_mso_cust_add_plan(connp, ctxp, flags, i_flistp, r_flistpp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Add Plan completed ");
		} else {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Add Plan cannot be done");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "op_mso_cust_add_plan output flist", *r_flistpp);
		}
	}
JUMPTO:
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if ( descr && ((strcmp(descr,"Bulk_add_plan") ==0)||(strcmp(descr,"Bulk_renew_plan") ==0)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Called by 'Bulk_add_plan' so transaction will not be handled at APIlevel");
	}
	else
	{	
		if (PIN_ERRBUF_IS_ERR(ebufp) || (status && (*(int*)status == 1)) || (price_calc_flag && price_calc_flag ==1))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"op_mso_cust_add_plan error", i_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_add_plan error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			else if (status && *(int *)status == 1)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Status is fail.");
			}
			else if (price_calc_flag == 1)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Price Calc flag is 1.");
			}
			else
			{
			}

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
			if(local == LOCAL_TRANS_OPEN_SUCCESS && 
					(!(prog_name && strstr(prog_name,"pin_deferred_act")))
			  )
			{
				fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
			}		
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_add_plan output flist", *r_flistpp);
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_add_plan(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
		int32			flags,
		pin_flist_t		*i_flistp,
		pin_flist_t		**r_flistpp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*read_flistp = NULL;	
	pin_flist_t		*read_res_flistp = NULL;
	poid_t			*routing_poid = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*service_obj = NULL;
	poid_t			*user_id = NULL;

	//char			*account_no_str = NULL;
	//char			account_no_str[10];
	int32			account_type      = -1;
	int32			account_model     = -1;
	int32			subscriber_type   = -1;
	int32			add_plan_success = 0;
	int32			corporate_type    =0;
	int32			add_plan_failure = 1;
	int32			tmp_business_type = 0;
	int32			company_type = -1;
	int32			*status = NULL;
	int32		mode = 0;

	int64			db = -1;
	int			csr_access = 0;
	int			acct_update = 0;
	int			ret_status = 0;
	int			arr_business_type[8];
	

	void			*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_plan input flist", i_flistp);	

	routing_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	service_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
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
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51412", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	if (!account_obj || !service_obj )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51413", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Account POID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}
	/*******************************************************************
	 * Validation product rules - Start
	 *******************************************************************/
	fm_mso_get_account_info(ctxp, account_obj, &acnt_info, ebufp);
	//	fm_mso_get_business_type_values(ctxp, *(int32*)PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp), 
	//	                        &account_type, &account_model, &subscriber_type, ebufp );
    PIN_FLIST_FLD_COPY(acnt_info, MSO_FLD_ET_ZONE, i_flistp, MSO_FLD_ET_ZONE, ebufp);
	vp =  (int32*)PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
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

	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
	{

		read_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, service_obj, ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_STATUS, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_plan read service input list", read_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_flistp, NULL);

		if ( PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BOOLEAN, 1, ebufp) != NULL )
		{
			mode = *(int *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BOOLEAN, 1, ebufp);
		}
		status = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_STATUS, 1, ebufp );


		if ( mode==0 && (*(int32 *)status != MSO_ACTIVE_STATUS ))
		{
			PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51616", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service status for this service is not active - Cannot add plan", ebufp);
			goto CLEANUP;
		}

	}

	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		goto ROLE;		
	}
	//subscriber_type = fm_mso_get_parent_info(ctxp, i_flistp, account_obj, &acnt_info, ebufp);
	/**** CATV changes ****/
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "before calling fm_mso_get_parent_info");
	//	PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_SUBSCRIBER_TYPE, &subscriber_type, ebufp );
	acnt_info = NULL;
	subscriber_type = fm_mso_get_parent_info(ctxp, i_flistp, account_obj, &acnt_info, ebufp);
	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{	
		if ((account_model == ACCOUNT_MODEL_CORP && corporate_type == CORPORATE_CHILD ) ||(subscriber_type == CORPORATE_CATV_CHILD))
		{
			PIN_FLIST_CONCAT(i_flistp,acnt_info, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan input flist before pay_type_corp", i_flistp);

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "before calling fm_mso_validate_pay_type_corp");
			fm_mso_validate_pay_type_corp( ctxp, i_flistp, MSO_OP_CUST_ADD_PLAN, &ret_flistp, ebufp);
			if (ret_flistp && ret_flistp != NULL)
			{
				ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
				if (ret_status == add_plan_failure)
				{
					goto CLEANUP;
				}
			}
		}
		else
		{
			fm_mso_validate_pay_type( ctxp, i_flistp, MSO_OP_CUST_ADD_PLAN, &ret_flistp, ebufp);
			if (ret_flistp && ret_flistp != NULL)
			{
				ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
				if (ret_status == add_plan_failure)
				{
					goto CLEANUP;
				}
			}
		}
	}


	/**********************************************************************************
	 * NTO: Commenting of this rule as not valid after New Tariff

	 fm_mso_validation_rule_2(ctxp, i_flistp, MSO_OP_CUST_ADD_PLAN, &ret_flistp, ebufp);
	 if (ret_flistp && ret_flistp != NULL)
	 {
	 ret_status = *(int32*)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
	 if (ret_status == add_plan_failure)
	 {
	 goto CLEANUP;
	 }
	 }
	 **********************************************************************************/

	/*******************************************************************
	 * Validation for PRICING_ADMIN roles - Start
	 *******************************************************************/
ROLE:
	if (user_id && user_id != NULL)
	{
		csr_access = fm_mso_validate_csr_role(ctxp, db, user_id, ebufp);

		if ( csr_access == 0)
		{		
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51414", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ROLE passed in input does not have permissions to change the service status", ebufp);
			goto CLEANUP;
		}else 
		{		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_service_status CSR haas permission to change account information");	
		}
	}else
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51415", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "User ID value not passed in input as it is mandatory field", ebufp);
		goto CLEANUP;
	}

	/*******************************************************************
	 * Validation for PRICING_ADMIN roles  - End
	 *******************************************************************/


	/*******************************************************************
	 * Add plan - Start
	 *******************************************************************/

	acct_update = fm_mso_add_deal(connp, ctxp, flags, i_flistp, &ret_flistp, ebufp);

	if ( acct_update == 0)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account information update failed");
	}
	else if ( acct_update == 1)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_plan no need of add plan");	
	}
	else if ( acct_update == 2)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_plan add plan successful");
		//fm_mso_cust_add_plan_lc_event(ctxp, i_flistp, ret_flistp, ebufp);
		//Added condition to call lifecycle here only for BB
		if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
		{
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_ADD_PLAN, ebufp);
		}
	}
	else if ( acct_update == 3)
	{		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_plan insufficient data provided");	
	}

	/*******************************************************************
	 * Add plan  - End
	 *******************************************************************/

CLEANUP:	
	*r_flistpp = ret_flistp;
	return;
}


/**************************************************
 * Function: fm_mso_add_deal()
 * 
 * 
 ***************************************************/
EXPORT_OP int 
fm_mso_add_deal(
		cm_nap_connection_t	*connp,
		pcm_context_t		*ctxp,
        int32           flags,
		pin_flist_t		*in_flist,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*planlists_flistp = NULL;
	pin_flist_t		*purchasedeal_inflistp = NULL;
	pin_flist_t		*purchasedeal_outflistp = NULL;
	pin_flist_t		*dealinfo_flistp = NULL;
	pin_flist_t		*deals_flistp = NULL;
	pin_flist_t		*products_flistp = NULL;
	pin_flist_t		*statuschange_inflistp = NULL;
	pin_flist_t		*statuschange_outflistp = NULL;
	pin_flist_t		*readplan_inflistp = NULL;
	pin_flist_t		*readplan_outflistp = NULL;
	pin_flist_t		*status_flistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*pdt_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*bb_ret_flistp = NULL;
	pin_flist_t             *add_plan_rflistp = NULL;/*saabrish - new flist introduced because the old return flist in fm_mso_et_product_rule_delhi function is used in the further loop process so that flist got overidden because of that condition getting failed and code dump occuring and not able add multiple plans for main connection */
	pin_flist_t		*alc_addon_pdts_flistp = NULL;
	pin_flist_t		*alc_addon_pdts_main_flistp = NULL;
	pin_flist_t		*alc_addon_pdts_child_flistp = NULL;
	pin_flist_t		*svc_flistp = NULL;
	pin_flist_t		*svc_res_flistp = NULL;
	pin_flist_t		*read_flistp = NULL;
	pin_flist_t		*usage_flistp = NULL;
	pin_flist_t		*deal_inflistp = NULL;
	pin_flist_t		*pdts_flistp = NULL;
	pin_flist_t		*discount_flistp = NULL;
	pin_flist_t		*disc_flistp = NULL;
	pin_flist_t		*subs_setprod_flistp = NULL;
	pin_flist_t		*subs_setprod_oflistp = NULL;
	pin_flist_t		*subs_pdts_flistp = NULL;
	pin_flist_t		*pdt_disc_search_flistp = NULL;
	pin_flist_t		*base_res_flistp = NULL;
	pin_flist_t		*lco_city_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*parent_serv_flistp = NULL;
	pin_flist_t		*prod_in_flistp = NULL;
	pin_flist_t		*prod_out_flistp = NULL;
    pin_flist_t     *et_rflistp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*new_serv_pdp = NULL;
	poid_t			*prd_pdp = NULL;
	poid_t			*offering_obj = NULL;
	poid_t			*deal_pdp = NULL;
	poid_t			*parent_acc_pdp = NULL;
	poid_t			*base_plan_obj = NULL;
    poid_t          *bi_pdp = NULL;
	char			msg[512] ;
	char			*event_type = NULL;
	char			*base_event_type = NULL;
	char			*cycle_type = NULL;
	char			*plan_name = NULL;
	char			*cust_city = NULL;
	char			*das_type = NULL;
	char			*lco_city = NULL;
	char			*prov_tag = NULL;
    char            *et_zonep = NULL;
	int			ser_err_no = 100004;
	int			elem_id = 0;
	int			elem_id1 = 0;
	int			elem_id2 = 0;
	int			elem_base = 0;
	int			status = PIN_PRODUCT_STATUS_ACTIVE;
	int			status_flags = PIN_STATUS_FLAG_ACTIVATE;
	int			cnt = 0;
	int			add_plan_success = 0;
	int			add_plan_failure = 1;
	int			prov_action = 0;
	int32			*prov_call = NULL;
	time_t			bp_valid_flag = 0;
	time_t			bp_parent_valid_flag = 0;
	time_t			child_bp_valid_flag = 0;
	time_t			main_bp_valid_flag = 0;
    time_t          actg_next_t = 0;
    time_t          *alignment_datep = NULL;
	int			product_status = 1;
	int32			discount_flag = -1;
	int			pp_type = 2;
	int32			count = 0;
    int32           errorCode = 0;
	int32			ret_status = -1;
	int32			failure = 1;
	int32 			comp_flag = 0;
	int32			is_base_pack = -1;
    int32           base_pack_add = 0;
	int32			pkg_type = -1;
	int32			skip_discount = 0;
	int32			enable_multi_pack = 0;
	int32			conn_type = -1;
	int32			add_prov = -1;
	int32			validity_align = 0;
    int32           child_align = 0;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t		cookie2 = NULL;
	pin_cookie_t		cookie_base = NULL;
	pin_flist_t		*lco_err_flistp = NULL;
	int64			lco_err_code = 0;
	pin_decimal_t		*disc_per = NULL;
	pin_decimal_t		*disc_amt = NULL;
	pin_decimal_t		*zero_disc = NULL;
    pin_fld_num_t       field_no = 0;
	//New Tariff - Transaction mapping Start */
	pin_flist_t             *lifecycle_flistp = NULL;
	pin_flist_t             *lc_plan_list_flistp = NULL;
	pin_flist_t             *eflistp = NULL;
	int32                   offer_count = 0;
	//New Tariff - Transaction mapping End */

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return 0;
	PIN_ERRBUF_CLEAR(ebufp);

	zero_disc = pbo_decimal_from_str("0.0", ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal :", in_flist);
	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );

	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		fm_mso_bb_add_plan(connp, ctxp, in_flist, &bb_ret_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in adding Plan for BB service", ebufp);
			*ret_flistp = bb_ret_flistp;
			return 0;
		}
		else if(bb_ret_flistp && bb_ret_flistp != NULL)
		{
			ret_status = *(int32*)PIN_FLIST_FLD_GET(bb_ret_flistp, PIN_FLD_STATUS, 0, ebufp);
			if (ret_status == failure)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in adding Plan for BB service");
				*ret_flistp = bb_ret_flistp;
				return 0;
			}
		}
		*ret_flistp = bb_ret_flistp;	
		return 2;
	} 
	/*Validation rule to be applied before  puchase deal*/
	r_flistp = NULL;	
	fm_mso_validation_rule_1(ctxp, in_flist, MSO_OP_CUST_ADD_PLAN, &r_flistp, ebufp);
	if (r_flistp && r_flistp != NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "00000");
		*ret_flistp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp",*ret_flistp );
		return 0;
	} 
	/*************************** Provisoning calling *************************************/
	//New Tariff - Transaction mapping Start*/
	if(lifecycle_flistp == NULL) {
		lifecycle_flistp = PIN_FLIST_CREATE(ebufp);
	}
	//New Tariff - Transaction mapping End*/

	status_flags = 2;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);	

	plan_flistp = PIN_FLIST_CREATE(ebufp);

	while ((planlists_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, &elem_base, 1, &cookie_base, ebufp )) != NULL)
	{	
		plan_obj = PIN_FLIST_FLD_GET(planlists_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
        base_plan_obj = PIN_FLIST_FLD_GET(provaction_inflistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
        if (!base_plan_obj)
        {            
            PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
        }            

		//New Tariff - Transaction mapping added the below line
		lc_plan_list_flistp = PIN_FLIST_ELEM_ADD(lifecycle_flistp, PIN_FLD_PLAN_LISTS, elem_base, ebufp);
		PIN_FLIST_FLD_COPY(planlists_flistp, PIN_FLD_PLAN_OBJ, lc_plan_list_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		offer_count = 0;

		if (!service_obj || !plan_obj)
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service add plan - Service object or plan object is missing in input", ebufp);

			*ret_flistp = r_flistp;
			return 3;
		}

		if(readplan_inflistp == NULL)
			readplan_inflistp = PIN_FLIST_CREATE(ebufp);

		PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_POID, plan_obj, ebufp );
		PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_NAME, NULL, ebufp );
		PIN_FLIST_FLD_SET(readplan_inflistp, PIN_FLD_DESCR, NULL, ebufp );
		deals_flistp = PIN_FLIST_ELEM_ADD(readplan_inflistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(deals_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal read plan input list", readplan_inflistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, readplan_inflistp, &readplan_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&readplan_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);

			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service add plan - PCM_OP_READ_FLDS of plan poid error", ebufp);

			*ret_flistp = r_flistp;
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal read plan output flist", readplan_outflistp);
		/*if (readplan_outflistp)
		  {
		  plan_name = PIN_FLIST_FLD_GET(readplan_outflistp, PIN_FLD_NAME, 1, ebufp);
		  if (plan_name && strstr(plan_name,"ROYAL HD"))
		  {
		  PIN_ERR_LOG_MSG(3,"Inside Skip Flag");
		  skip_validation = 1;	
		  }
		  }*/

		//		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, readplan_outflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, readplan_outflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		acnt_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp); 
		srvc_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 0, ebufp); 
		prov_call = (int32 *)PIN_FLIST_FLD_GET(in_flist, PIN_FLD_VALIDITY_FLAGS, 1, ebufp);
		if (prov_call)
		{
			validity_align = *prov_call;
		}
        prov_call = (int32 *)PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PARENT_FLAGS, 1, ebufp);
        if (prov_call)
        {
            child_align = *prov_call;
        }
        alignment_datep = (time_t *)PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PURCHASE_END_T, 1, ebufp);
		conn_type = fm_mso_catv_conn_type(ctxp, srvc_pdp, ebufp);
		/******************************************************************************
		 * NTO: Validation for adding ET rule Delhi
		 * Start
		 fm_mso_et_product_rule_delhi(ctxp, readplan_outflistp, acnt_pdp, MSO_OP_CUST_ADD_PLAN, &add_plan_rflistp, ebufp);
		 PIN_ERR_LOG_MSG(3,"After returning");
		 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal read plan output flist", add_plan_rflistp);
		 if (add_plan_rflistp && add_plan_rflistp != NULL)
		 {
		 ret_status = *(int32*)PIN_FLIST_FLD_GET(add_plan_rflistp, PIN_FLD_STATUS, 0, ebufp);
		 if (ret_status == failure)
		 {
		 *ret_flistp = add_plan_rflistp;
		 PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);
		 return 0;
		 }
		 }
		 PIN_FLIST_DESTROY_EX(&add_plan_rflistp, NULL);
		 ******************************************************************************/

		/******************************************************************************
		 * Validation for adding ET rule Delhi
		 * End
		 ******************************************************************************/

		/*************************************************************************************
		 * Validation for Channels under New Products and Products Purchased in CATV Service
		 * Start
		 **************************************************************************************/
		//		PIN_FLIST_FLD_SET(readplan_outflistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp); 
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "after if check111");
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, readplan_outflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, readplan_outflistp, PIN_FLD_USERID, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "before th if cond");
		if (service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj), MSO_CATV)) == 0 )
		{
			fm_mso_retrieve_pdt_info(ctxp, readplan_outflistp, &pdt_flistp, ebufp);

			ret_status = *(int32*)PIN_FLIST_FLD_GET(pdt_flistp, PIN_FLD_STATUS, 1, ebufp);
			if (ret_status == failure)
			{
				*ret_flistp = r_flistp;
				PIN_FLIST_DESTROY_EX(&pdt_flistp, NULL);
				return 0;
			}

			prd_pdp = PIN_FLIST_FLD_GET(pdt_flistp, PIN_FLD_POID, 0, ebufp);
			is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp);

			// Validating for Ala-Carte Paid (140) and Ala-Carte FTA (150)
			if (is_base_pack != 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Product is a Non-Base Pack. Need to check the Base Pack status");

				fm_mso_get_base_pdts(ctxp, acnt_pdp, service_obj, &alc_addon_pdts_main_flistp, ebufp);
				if (alc_addon_pdts_main_flistp && alc_addon_pdts_main_flistp != NULL)
				{
					ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp, PIN_FLD_STATUS, 1, ebufp);
					if (ret_status == failure)
					{
						r_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);      
						*ret_flistp = r_flistp;
						PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
						return 0;
					}

				}

				bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 0, ebufp);

				if (bp_valid_flag == 0)
				{
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);      
					goto CLEANUP;

				}

				base_event_type = (char *)PIN_FLIST_FLD_TAKE(alc_addon_pdts_main_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);

				PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);

				PIN_FLIST_FLD_COPY(pdt_flistp, PIN_FLD_POID, readplan_outflistp, PIN_FLD_PRODUCT_OBJ, ebufp);
				/****************************************************************************
				 * NTO: comment out channel validation
				 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_pdt_channels_validation function");
				 fm_mso_pdt_channels_validation(ctxp, readplan_outflistp, service_obj, &r_flistp, ebufp);
				 if (r_flistp && r_flistp != NULL)
				 {
				 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_pdt_channels_validation final output flist", r_flistp);
				 ret_status = *(int32*)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STATUS, 0, ebufp);
				 if (ret_status == failure)
				 {
				 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_pdt_channels_validation error output flist", r_flistp);
				 *ret_flistp = r_flistp;
				//						PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);
				return 0;
				}
				PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
				}
				 *************************************************************************/
			}

			// Validating the FDP Packs purchase to enable / reactivate the Ala-Carte & Add-On products
			// This section is for renewal of FDP plan after Default Grace Date and / or Add_on Grace Date

			/********************************
			 * Read the product to get the actual event type 
			 ********************/

			read_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, prd_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_PRIORITY, 0, ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
			//ADDED for prevent prov call for convinience fee/ activation packs
			usage_flistp = PIN_FLIST_ELEM_ADD(read_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
			PIN_FLIST_FLD_SET(usage_flistp, PIN_FLD_EVENT_TYPE, NULL, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Readflds products input", read_flistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &r_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while product read flds", ebufp);
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Purchased Poid is NULL");
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal failed : ", ebufp);		
				goto CLEANUP;
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Readflds products output", r_flistp);

			usage_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 0,  ebufp);
			event_type = (char *)PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);
			prov_tag = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
			cycle_type = strrchr(event_type, '/');

			if( (!strcmp("/mso_sb_adn_fdp", cycle_type)) || (!strcmp("/mso_sb_pkg_fdp", cycle_type)) || (!strcmp("/mso_sb_alc_fdp", cycle_type)) )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " This is fixed duration plan ");
				svc_flistp = PIN_FLIST_CREATE(ebufp);
				//Validating the Main Connection or Child Connection
				fm_mso_utils_get_catv_main(ctxp, acnt_pdp, &svc_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "svc_flistp check for main", svc_flistp);
				//if (svc_flistp && svc_flistp != NULL)
				if ( PIN_FLIST_ELEM_COUNT(svc_flistp, PIN_FLD_RESULTS, ebufp) > 0 )
				{
					svc_res_flistp = PIN_FLIST_ELEM_GET(svc_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
					new_serv_pdp = PIN_FLIST_FLD_GET(svc_res_flistp, PIN_FLD_POID, 1, ebufp);
					if (PIN_POID_COMPARE(service_obj, new_serv_pdp, 0, ebufp) == 0 || 
						conn_type == 1 || conn_type == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Service is a Main Connection");
						// Current Service is a Main Service
						// Validating the Base FDP Packs purchase to enable / reactivate the Ala-Carte & Add-On products
						// This section is for renewal of Base FDP plan after Grace Date

						is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp); 
						if (is_base_pack == 0 )
						{
                            base_pack_add = 1;
							enable_multi_pack = 1;
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Pack of Main Connection");

							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_activate_alc_addon_pdts function");
						        fm_mso_activate_alc_addon_pdts(ctxp, in_flist, prd_pdp, &alc_addon_pdts_flistp, ebufp);
							if (alc_addon_pdts_flistp)
							{
								ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_flistp, PIN_FLD_STATUS, 1, ebufp);
								if (ret_status == failure)
								{
									r_flistp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);		
									*ret_flistp = r_flistp;
							 		PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
									return 0;
								}
							}
							PIN_ERRBUF_RESET(ebufp);
						}
						else
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Product is a Non-Base Pack. Need to check the Base Pack status");

							fm_mso_get_base_pdts(ctxp, acnt_pdp, service_obj, &alc_addon_pdts_main_flistp, ebufp);
							if (alc_addon_pdts_main_flistp && alc_addon_pdts_main_flistp != NULL)
							{
								ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp, PIN_FLD_STATUS, 1, ebufp);
								if (ret_status == failure)
								{
									r_flistp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);		
									*ret_flistp = r_flistp;
									PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
									return 0;
								}

								//Checking the Base Pack Validity for Re-activation of other Packs
								bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 0, ebufp);

								if (bp_valid_flag == 0)
								{
									r_flistp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);		
									goto CLEANUP;

								}

								base_event_type = (char *)PIN_FLIST_FLD_TAKE(alc_addon_pdts_main_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);

								PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);

							}
						}
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Service is a Child Connection");
						// Current Service is a Child Service

						// This section is for renewal of Base FDP plan after Grace Date
						is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp);

						if (is_base_pack == 0 )
						{
							enable_multi_pack = 1 ;
                            base_pack_add = 1;
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Pack of Child Connection");
							/************************************************************************************
							 * NTO 2.0: Going back to prior NTO 1.0 requirement.
							fm_mso_activate_alc_addon_pdts(ctxp, in_flist, prd_pdp, &alc_addon_pdts_flistp, ebufp);
							if (alc_addon_pdts_flistp && alc_addon_pdts_flistp != NULL)
							{
								ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_flistp, PIN_FLD_STATUS, 1, ebufp);
								if (ret_status == failure)
								{
									r_flistp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package of Main Connection is Inactive. First Activate the Base Pack", ebufp);	
									*ret_flistp = r_flistp;
									PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
									return 0;
								}
								PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
							}
							***********************************************************************************/

							//Validating the Main Connection's Base Pack
							fm_mso_get_base_pdts(ctxp, acnt_pdp, new_serv_pdp, &alc_addon_pdts_main_flistp, ebufp);
							if (alc_addon_pdts_main_flistp && alc_addon_pdts_main_flistp != NULL)
							{
								ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp, PIN_FLD_STATUS, 1, ebufp);
								if (ret_status == failure)
								{
									r_flistp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package of Main Connection is Inactive. First Activate the Base Pack", ebufp);		
							 		*ret_flistp = r_flistp;
							 		PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
							 		return 0;
							 	}
							 	main_bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 0, ebufp);

							 	if (main_bp_valid_flag > 0)
							 	{

							 		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_activate_alc_addon_pdts function");
							 		fm_mso_activate_alc_addon_pdts(ctxp, in_flist, prd_pdp, &alc_addon_pdts_flistp, ebufp);
							 		if (alc_addon_pdts_flistp && alc_addon_pdts_flistp != NULL)
							 		{
							 			ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_flistp, PIN_FLD_STATUS, 1, ebufp);
							 			if (ret_status == failure)
							 			{
							 				r_flistp = PIN_FLIST_CREATE(ebufp);
							 				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
							 				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
							 				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
							 				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package of Main Connection is Inactive. First Activate the Base Pack", ebufp);	
							 				*ret_flistp = r_flistp;
							 				PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
							 				return 0;
							 			}
							 		}
							 		PIN_ERRBUF_RESET(ebufp);
							 	}
							 	else
							 	{
							 		r_flistp = PIN_FLIST_CREATE(ebufp);
							 		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
							 		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
							 		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
							 		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package of Main Connection is Inactive. First Activate the Base Pack", ebufp);		
							 		*ret_flistp = r_flistp;
							 		goto CLEANUP;
							 	}
								PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
							 }
						}
						else
						{   
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Product is a Non-Base Pack of Child Connection. Need to check the Base Pack status");

							fm_mso_get_base_pdts(ctxp, acnt_pdp, service_obj, &alc_addon_pdts_child_flistp, ebufp);
							fm_mso_get_base_pdts(ctxp, acnt_pdp, new_serv_pdp, &alc_addon_pdts_main_flistp, ebufp);
							if (alc_addon_pdts_child_flistp)
							{
								 ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp, PIN_FLD_STATUS, 1, ebufp);
								 if (ret_status == failure)
								 {
								 	r_flistp = PIN_FLIST_CREATE(ebufp);
								 	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
								 	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
								 	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
								 	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive in Main or Child Connection. First Activate the Base Pack", ebufp);	
								 	*ret_flistp = r_flistp;
								 	PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
								 	return 0;
								 }

								ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_child_flistp, PIN_FLD_STATUS, 1, ebufp);
								if (ret_status == failure )
								{
									r_flistp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive in Main or Child Connection. First Activate the Base Pack", ebufp);	
									*ret_flistp = r_flistp;
									PIN_FLIST_DESTROY_EX(&alc_addon_pdts_child_flistp, NULL);
									return 0;
								}
								//Checking the Base Pack Validity for Re-activation of other Packs
								bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 0, ebufp);

								child_bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_child_flistp, 0, ebufp);
								if (bp_valid_flag == 0 || child_bp_valid_flag == 0)
								{
									r_flistp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
									PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive in Child Connection. First Activate the Base Pack", ebufp);		
									*ret_flistp = r_flistp;
									goto CLEANUP;
								}

								base_event_type = (char *)PIN_FLIST_FLD_TAKE(alc_addon_pdts_child_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);

								PIN_FLIST_DESTROY_EX(&alc_addon_pdts_child_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
							}

						}
					}
					PIN_FLIST_DESTROY_EX(&svc_flistp, NULL);	
				}

			}
			PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
			PIN_ERRBUF_RESET(ebufp);

		}

		if (validity_align)
		{
			// Jyothirmayi Kachiraju - Changes for new FDP Event Creation
			if ((bp_valid_flag != 0 || child_bp_valid_flag != 0) && 
				(
				 (strstr(event_type, "cycle_forward_sixty_days") && !strstr(base_event_type, "cycle_forward_sixty_days")) || 
				 (strstr(event_type, "cycle_forward_ninety_days") && !strstr(base_event_type, "cycle_forward_ninety_days")) ||
				 (strstr(event_type, "cycle_forward_onetwenty_days") && !strstr(base_event_type, "cycle_forward_onetwenty_days")) ||
				 (strstr(event_type, "cycle_forward_oneeighty_days") && !strstr(base_event_type, "cycle_forward_oneeighty_days")) ||		
				 (strstr(event_type, "cycle_forward_onefifty_days") && !strstr(event_type, "cycle_forward_onefifty_days")) || 
				 (strstr(event_type, "cycle_forward_twoten_days") && !strstr(event_type, "cycle_forward_twoten_days")) || 
				 (strstr(event_type, "cycle_forward_twoforty_days") && !strstr(event_type, "cycle_forward_twoforty_days")) || 
				 (strstr(event_type, "cycle_forward_twoseventy_days") && !strstr(event_type, "cycle_forward_twoseventy_days")) || 
				 (strstr(event_type, "cycle_forward_threehundred_days") && !strstr(event_type, "cycle_forward_threehundred_days")) || 
				 (strstr(event_type, "cycle_forward_threethirty_days") && !strstr(event_type, "cycle_forward_threethirty_days")) || 
				 (strstr(event_type, "cycle_forward_threesixty_days") && !strstr(event_type, "cycle_forward_threesixty_days"))))
			{
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51419", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Pack payterm is not matching to align, please disable validity align flag", ebufp);      
				goto CLEANUP;
			}
		}
		/*************************************************************************************
		 * Validation for Channels under New Products and Products Purchased in CATV Service
		 * End
		 **************************************************************************************/

		args_flistp = PIN_FLIST_ELEM_ADD(plan_flistp, PIN_FLD_PLAN, elem_base, ebufp);
		PIN_FLIST_FLD_COPY(readplan_outflistp, PIN_FLD_NAME, args_flistp, PIN_FLD_NAME, ebufp);

		elem_id = 0;
		cookie = NULL;
		while ((deals_flistp = PIN_FLIST_ELEM_GET_NEXT(readplan_outflistp, PIN_FLD_SERVICES, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			purchasedeal_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, purchasedeal_inflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, purchasedeal_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_DESCR, purchasedeal_inflistp, PIN_FLD_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, purchasedeal_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);

			dealinfo_flistp = PIN_FLIST_ELEM_ADD(purchasedeal_inflistp, PIN_FLD_DEAL_INFO, 0, ebufp );
			PIN_FLIST_FLD_COPY(deals_flistp, PIN_FLD_DEAL_OBJ, dealinfo_flistp, PIN_FLD_DEAL_OBJ, ebufp);
			PIN_FLIST_FLD_SET(dealinfo_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal input list", purchasedeal_inflistp);
			PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, 0, purchasedeal_inflistp, &purchasedeal_outflistp, ebufp);

			switch (ebufp->pin_err) {
				case 100002:
				case 100004:
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "lco error set");
					lco_err_code = ebufp->pin_err;
					break;
				default: lco_err_code = 0;
			}

			if (lco_err_code != 0) {
				PIN_ERRBUF_RESET(ebufp);
				lco_err_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_POID,lco_err_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "lco error flistp", lco_err_flistp);
			}

			PIN_FLIST_DESTROY_EX(&purchasedeal_inflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp) || lco_err_code != 0)
			{
				/*LCO Error Handling Start*/
				if (lco_err_code != 0) {
					fm_mso_commission_error_processing(ctxp, lco_err_code,
							"Error in fm_mso_cust_activate_customer", lco_err_flistp, &r_flistp, ebufp);
				}
				/*LCO Error Handling End*/
				else 
					/*if (PIN_ERRBUF_IS_ERR(ebufp))*/
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);

					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service add plan - PCM_OP_SUBSCRIPTION_PURCHASE_DEAL error", ebufp);

					//*ret_flistp = r_flistp;
					//return 0;
				}
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal output flist", purchasedeal_outflistp);

			elem_id1 = 0;
			cookie1 = NULL;
			while ((products_flistp = PIN_FLIST_ELEM_GET_NEXT(purchasedeal_outflistp, PIN_FLD_PRODUCTS, &elem_id1, 1, &cookie1, ebufp)) != NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal products_flistp ", products_flistp);

				//resetting discounting parameters 
				discount_flag = -1;

				offering_obj = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);
				prd_pdp = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fetching PIN_FLD_OFFERING_OBJ from return flist of PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);

					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service add plan - Error in fetching PIN_FLD_OFFERING_OBJ from return flist of PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);

					*ret_flistp = r_flistp;
					return 0;
				}

				/*******************************************************************************
				 * Read provisioning tag from product to support multiple deals in a plan
				 ******************************************************************************/
				prod_in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(products_flistp, PIN_FLD_PRODUCT_OBJ, prod_in_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET (prod_in_flistp, PIN_FLD_PROVISIONING_TAG,(void *)NULL, ebufp);
				usage_flistp = PIN_FLIST_ELEM_ADD(prod_in_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
				PIN_FLIST_FLD_SET(usage_flistp, PIN_FLD_EVENT_TYPE, NULL, ebufp);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, prod_in_flistp, &prod_out_flistp, ebufp);

				PIN_FLIST_DESTROY_EX(&prod_in_flistp, NULL);	

				usage_flistp = PIN_FLIST_ELEM_GET(prod_out_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 0,  ebufp);
				event_type = (char *)PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);
				prov_tag = (char *)PIN_FLIST_FLD_GET(prod_out_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);	

				if (offering_obj)
				{
					//New Tariff - Transaction mapping Start */
					eflistp = PIN_FLIST_ELEM_ADD(lc_plan_list_flistp,PIN_FLD_OFFERINGS, offer_count++, ebufp);
					PIN_FLIST_FLD_COPY(products_flistp, PIN_FLD_OFFERING_OBJ,eflistp, PIN_FLD_OFFERING_OBJ, ebufp);
					PIN_FLIST_FLD_COPY(products_flistp, PIN_FLD_PACKAGE_ID,eflistp, PIN_FLD_PACKAGE_ID, ebufp);
					//New Tariff - Transaction mapping End */ 
					if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV Service");

						if (strstr(prov_tag, "NCF") == NULL)
						{
							pkg_type = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp);
						}
						else
						{
							pkg_type = NCF_PKG_TYPE;
						}

						/************************************************************************
						 * Get registered parent service for NCF products 
						 ***********************************************************************/
						if (pkg_type == NCF_PKG_TYPE)
						{
							fm_mso_hier_group_get_parent(ctxp, service_obj, &parent_serv_flistp, ebufp);
						}
						if (parent_serv_flistp)
						{
							new_serv_pdp = PIN_FLIST_FLD_GET(parent_serv_flistp, PIN_FLD_OBJECT, 0, ebufp);
							parent_acc_pdp = PIN_FLIST_FLD_GET(parent_serv_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
							fm_mso_get_base_pdts(ctxp, parent_acc_pdp, new_serv_pdp, &alc_addon_pdts_main_flistp, ebufp);

							if (alc_addon_pdts_main_flistp && alc_addon_pdts_main_flistp != NULL)
							{
								ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp,
										PIN_FLD_STATUS, 0, ebufp);
								if (ret_status == failure)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Package of main coon is Inactive.");
									skip_discount = 1;
								}	
							}

							bp_parent_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 0, ebufp);
							if (bp_parent_valid_flag == 0)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Package of main coon is Inactive.");
								skip_discount = 1;
							}
							PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
						}

						if (!skip_discount)
						{
							pdt_disc_search_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, service_obj, ebufp);
							PIN_FLIST_FLD_SET(pdt_disc_search_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
							PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
							PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_PLAN_OBJ, plan_obj, ebufp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Looking for Account Level Discount Config");
							fm_mso_get_service_level_discount(ctxp, pdt_disc_search_flistp, 0, &discount_flistp, ebufp);
							PIN_FLIST_DESTROY_EX(&pdt_disc_search_flistp, NULL);
						}
						if (discount_flistp && discount_flistp != NULL)
						{
							ret_status = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_STATUS, 1, ebufp);
							if (ret_status != failure)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
										"Account Level Discount Config Found");
								discount_flag = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, 
										PIN_FLD_DISCOUNT_FLAGS, 0, ebufp);
								if (discount_flag == 0)
								{
									disc_per = PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
								}
								else if (discount_flag == 1)
								{
									disc_amt = PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
								}
							}
							else
							{
								if (PIN_ERRBUF_IS_ERR(ebufp))
									PIN_ERRBUF_RESET(ebufp);
								pdt_disc_search_flistp = PIN_FLIST_CREATE(ebufp);
								//PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, prd_pdp, ebufp);
								PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, plan_obj, ebufp);
								// Retrieving the PP Type 
								pp_type = fm_mso_get_cust_type(ctxp, acnt_pdp, ebufp);
								if (pp_type != 2)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
											"Valid PP Type Found... ");
									PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
											MSO_FLD_PP_TYPE, &pp_type, ebufp);
								}

								//  Retrieve the Customer City from Account Object
								fm_mso_get_custcity(ctxp, acnt_pdp, &cust_city, ebufp);
								if (cust_city && cust_city != NULL)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CITY FOUND... ");
									PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
											PIN_FLD_CITY, cust_city, ebufp);

								}

								// Retrieve the Customer Service Connection Type
								conn_type = fm_mso_catv_conn_type(ctxp, srvc_pdp, ebufp);
								if (conn_type != -1)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
											"Valid Connection Type FOUND... ");
									PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
											PIN_FLD_CONN_TYPE, &conn_type, ebufp);
								}

								// Retrieve the Customer DAS Type 
								fm_mso_get_das_type(ctxp, acnt_pdp, &das_type, ebufp);
								if (das_type && das_type != NULL)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
											"DAS TYPE FOUND... ");
									PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
											MSO_FLD_DAS_TYPE, das_type, ebufp);
								}

								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
										"Looking for Product Level Discount Config");
								PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
								fm_mso_get_plan_level_discount(ctxp, pdt_disc_search_flistp, 
										&discount_flistp, ebufp);
								if (discount_flistp && discount_flistp != NULL)
								{
									ret_status = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, 
											PIN_FLD_STATUS, 1, ebufp);
									if (ret_status != failure)
									{
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
												"Product Level Discount Config Found");
										discount_flag = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, 
												PIN_FLD_DISCOUNT_FLAGS, 0, ebufp);
										if (discount_flag == 0)
										{
											disc_per = PIN_FLIST_FLD_GET(discount_flistp, 
													PIN_FLD_DISCOUNT, 0, ebufp);
										}
										else if (discount_flag == 1)
										{
											disc_amt = PIN_FLIST_FLD_GET(discount_flistp, 
													PIN_FLD_DISCOUNT, 0, ebufp);
										}
									}
								}
							}
						}

						if (discount_flag != -1)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount Configs Found");
							if (PIN_ERRBUF_IS_ERR(ebufp))
								PIN_ERRBUF_RESET(ebufp);

							subs_setprod_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, subs_setprod_flistp, PIN_FLD_POID, ebufp);
							PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, subs_setprod_flistp, 
									PIN_FLD_PROGRAM_NAME, ebufp);
							PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_DESCR,subs_setprod_flistp,PIN_FLD_DESCR,ebufp);
							subs_pdts_flistp = PIN_FLIST_ELEM_ADD(subs_setprod_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
							PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PRODUCT_OBJ, prd_pdp, ebufp);
							PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_OFFERING_OBJ, offering_obj, ebufp);

							if (discount_flag == 0)
							{
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_DISCOUNT, disc_per, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_DISCOUNT, disc_per, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_DISCOUNT, disc_per, ebufp);
							}
							else
							{
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_DISC_AMT, disc_amt, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_DISC_AMT, disc_amt, ebufp);
								//Resetting percentage discount to zero in case of combo packs with deal level disc
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_DISCOUNT, zero_disc, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_DISCOUNT, zero_disc, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_DISCOUNT, zero_disc, ebufp);
							}

							/****************************************************
							 * NTO: Logic for the date alignment
							 ***************************************************/	
                            if (alignment_datep)
                            {
                                PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_END_T, alignment_datep, ebufp);
                                PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_END_T, alignment_datep, ebufp);
                                PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_END_T, alignment_datep, ebufp);
                            }
							else if (child_bp_valid_flag > 1 && validity_align)
							{
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_END_T, &child_bp_valid_flag, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_END_T, &child_bp_valid_flag, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_END_T, &child_bp_valid_flag, ebufp);
							}
							else if (bp_valid_flag > 1 && validity_align)
							{
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_END_T, &bp_valid_flag, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_END_T, &bp_valid_flag, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_END_T, &bp_valid_flag, ebufp);
							}
                            else if (main_bp_valid_flag > 1 && child_align)
                            {
                                PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_END_T, &main_bp_valid_flag, ebufp);
                                PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_END_T, &main_bp_valid_flag, ebufp);
                                PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_END_T, &main_bp_valid_flag, ebufp);
                            }

							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal set product input flist", 
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
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal set product output flist", subs_setprod_oflistp);
							PIN_FLIST_DESTROY_EX(&subs_setprod_oflistp, NULL);

						}
						else if (child_bp_valid_flag > 0 || bp_valid_flag > 0)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Discount Configs Found");
							if (PIN_ERRBUF_IS_ERR(ebufp))
								PIN_ERRBUF_RESET(ebufp);

							/****************************************************
							 * NTO: Logic for the validity alignment
							 ***************************************************/	
							subs_setprod_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, subs_setprod_flistp, PIN_FLD_POID, ebufp);
							PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, subs_setprod_flistp, 
									PIN_FLD_PROGRAM_NAME, ebufp);
							PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_DESCR,subs_setprod_flistp,PIN_FLD_DESCR,ebufp);
							subs_pdts_flistp = PIN_FLIST_ELEM_ADD(subs_setprod_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
							PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PRODUCT_OBJ, prd_pdp, ebufp);
							PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_OFFERING_OBJ, offering_obj, ebufp);

							if (child_bp_valid_flag > 1 && validity_align)
							{
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_END_T, &child_bp_valid_flag, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_END_T, &child_bp_valid_flag, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_END_T, &child_bp_valid_flag, ebufp);
							}
							else if (bp_valid_flag > 1 && validity_align)
							{
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_USAGE_END_T, &bp_valid_flag, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_CYCLE_END_T, &bp_valid_flag, ebufp);
								PIN_FLIST_FLD_SET(subs_pdts_flistp, PIN_FLD_PURCHASE_END_T, &bp_valid_flag, ebufp);
							}
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal set product input flist", 
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
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal set product output flist", subs_setprod_oflistp);
							PIN_FLIST_DESTROY_EX(&subs_setprod_oflistp, NULL);
						}
					}
					prov_action = 1;
					status_flags = PIN_STATUS_FLAG_ACTIVATE;
					statuschange_inflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, statuschange_inflistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, statuschange_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
					PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_DESCR,statuschange_inflistp,PIN_FLD_DESCR,ebufp);

					status_flistp = PIN_FLIST_ELEM_ADD(statuschange_inflistp, PIN_FLD_STATUSES, 0, ebufp );
					PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS, &status, ebufp);
					PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);
					PIN_FLIST_FLD_SET(status_flistp, PIN_FLD_OFFERING_OBJ, offering_obj, ebufp);

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal status change input list", statuschange_inflistp);
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
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
                        if (errorCode == PIN_ERR_BAD_VALUE && field_no == PIN_FLD_CURRENT_BAL)
                        {
                            PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service add plan - There is no sufficient balance", ebufp);
                        }
                        else
                        {
						    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service add plan - PCM_OP_SUBSCRIPTION_SET_PRODUCT_STATUS error", ebufp);
                        }

						*ret_flistp = r_flistp;
						return 0;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal status change output flist", statuschange_outflistp);
					PIN_FLIST_DESTROY_EX(&statuschange_outflistp, NULL);	

					if(strcmp(prov_tag, "")!=0)
					{
						add_prov = fm_validate_prov_ca_id(ctxp, srvc_pdp, prov_tag, ebufp);
						if(add_prov)
						{
							status_flistp = PIN_FLIST_ELEM_ADD(provaction_inflistp, PIN_FLD_PRODUCTS, cnt++, ebufp );
							PIN_FLIST_FLD_COPY(products_flistp, PIN_FLD_OFFERING_OBJ,status_flistp, PIN_FLD_POID, ebufp);
						}

						if (add_prov == 2)
						{
							add_prov = 0;
							PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PKG_TYPE, &add_prov, ebufp);
						}
					}

				}
				PIN_FLIST_DESTROY_EX(&prod_out_flistp, NULL);	
			}
			PIN_FLIST_DESTROY_EX(&purchasedeal_outflistp, NULL);
		}//inner while loop end
		PIN_FLIST_DESTROY_EX(&readplan_outflistp, NULL);
	}//outer while loop end

	if (prov_action == 1)
	{
		/* Write USERID, PROGRAM_NAME in buffer - start */
		PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
		PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
		/* Write USERID, PROGRAM_NAME in buffer - end */
		if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
		{
			comp_flag = fm_mso_compare_ca_ids(ctxp, in_flist,ret_flistp, ebufp);
			if (!comp_flag && (PIN_FLIST_ELEM_COUNT(provaction_inflistp, PIN_FLD_PRODUCTS, ebufp)>0))
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal provisioning input list", provaction_inflistp);
				PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No  Need for Provisioning");
				PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
				goto SKIP;
			}
		}
		else
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal provisioning input list", provaction_inflistp);
			PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
		}
		PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service add plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);

			*ret_flistp = r_flistp;
			return 0;
		}

		if(provaction_outflistp)
		{
			prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
			if(prov_call && (*prov_call == 1))
			{
				*ret_flistp = PIN_FLIST_COPY(provaction_outflistp, ebufp);
				return 0;
			}
		}
		else
		{
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51751", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning action.", ebufp);
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal provisioning output flist", provaction_outflistp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal provisioning no need as there is no purchased product got created");
		PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	}


	/******************************************* notification flist ***********************************************/
	/*	status_flags = 10;
		provaction_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

		PIN_FLIST_CONCAT(provaction_inflistp, plan_flistp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal provisioning input list", provaction_inflistp);
		PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);

		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Service add plan - MSO_OP_NTF_CREATE_NOTIFICATION error", ebufp);

	 *ret_flistp = r_flistp;
	 return 0;
	 }

	 if(provaction_outflistp && provaction_outflistp != NULL)
	 {
	 prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
	 if(prov_call && (*prov_call == 1))
	 {
	 *ret_flistp = PIN_FLIST_COPY(provaction_outflistp, ebufp);
	 return 0;
	 }
	 }
	 else
	 {
	 *ret_flistp = PIN_FLIST_CREATE(ebufp);
	 PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
	 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
	 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51752", ebufp);
	 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning notification.", ebufp);
	 return 0;
	 }
	 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal provisioning output flist", provaction_outflistp);
	 */
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_flistp, NULL);


	if (is_base_pack == 0 || enable_multi_pack == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Pack of Main Connection");

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_activate_alc_addon_pdts function");
		fm_mso_activate_alc_addon_pdts(ctxp, in_flist, prd_pdp, &alc_addon_pdts_flistp, ebufp);
		if (alc_addon_pdts_flistp && alc_addon_pdts_flistp != NULL)
		{
			ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_flistp, PIN_FLD_STATUS, 1, ebufp);
			if (ret_status == failure)
			{
				r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);		
				*ret_flistp = r_flistp;
				PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
				return 0;
			}
			PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
		}
	}


	/*******************************************************************
	 * Create Output flist - Start
	 *******************************************************************/
SKIP:	
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_success, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_VALIDITY_FLAGS, r_flistp, PIN_FLD_VALIDITY_FLAGS, ebufp);
	if (child_bp_valid_flag > 1 && validity_align)
	{
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_CYCLE_END_T, &child_bp_valid_flag, ebufp);
	}
	else if (bp_valid_flag > 1 && validity_align)
	{
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_CYCLE_END_T, &bp_valid_flag, ebufp);
	}
	else
	{
	}
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "ACCOUNT - Service add plan completed successfully", ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_add_deal  output flist", r_flistp);

	*ret_flistp = PIN_FLIST_COPY(r_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

    if (base_pack_add)
    {
        et_zonep = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_ET_ZONE, 0, ebufp);
        if (et_zonep && strstr(et_zonep, "SK_"))
        {
            fm_mso_get_all_billinfo(ctxp, acnt_pdp, 1, &r_flistp, ebufp);
            result_flist = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
            bi_pdp = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp);
            actg_next_t = *(time_t *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_ACTG_NEXT_T, 0, ebufp);
            mso_bill_pol_catv_main(ctxp, flags, bi_pdp, service_obj, actg_next_t, 11007, &et_rflistp, ebufp);
            PIN_FLIST_DESTROY_EX(&et_rflistp, NULL);
        }
    }


	/*******************************************************************
	 * Create Output flist - Start
	 *******************************************************************/
	/*New Tariff - Transaction mapping Start */
	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_CATV)) == 0 )
	{
		fm_mso_create_lifecycle_evt(ctxp, in_flist, lifecycle_flistp, ID_ADD_PLAN, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&lifecycle_flistp, NULL);
	/*New Tariff - Transaction mapping End */

	return 2;

CLEANUP:
	*ret_flistp = PIN_FLIST_COPY(r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	if (base_event_type) free(base_event_type);
	return;	


}


/********
 * Plan Renewal implementation
 ********/

extern void
fm_mso_reactivate_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*read_flistp = NULL;
	pin_flist_t		*usage_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;	
	pin_flist_t		*cycle_flist = NULL;	
	pin_flist_t		*status_flistp = NULL;
	pin_flist_t		*prod_oflist = NULL;
	pin_flist_t		*prod_iflist = NULL;
	pin_flist_t		*products_flist = NULL;
	pin_flist_t		*usage_oflistp = NULL;
	pin_flist_t		*read_oflistp = NULL;
	pin_flist_t		*cycle_fee_flist = NULL;
	pin_flist_t		*purchase_product_roflist = NULL;
	pin_flist_t		*cycle_fwd_oflistp = NULL;
	pin_flist_t		*cycle_fwd_iflistp = NULL;
	pin_flist_t		*lc_inflist = NULL;
	pin_flist_t		*parent_serv_flistp = NULL;
    pin_flist_t     *et_rflistp = NULL;
    pin_fld_num_t   field_no = 0;
	char			*event_type = NULL;
	char			*base_event_type = NULL;
	char			*cycle_type = NULL;
	char			*program_name = "Reactivate Plan";
	char			 msg[512];
	char			*plan_name = NULL;
    char            *et_zonep = NULL;
	int			flags = 512;
	int32			add_plan_success = 0;
	int32			add_plan_failure = 1;
	int32			*status_flags = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*pp_pdp = NULL;
	poid_t			*product_pdp	= NULL;
	poid_t			*srch_pdp	= NULL;
	poid_t			*acc_pdp	= NULL;
	poid_t			*parent_acc_pdp	= NULL;
    poid_t          *bi_pdp = NULL;
	int			elem_id = 0;
	int			elem_id1 = 0;
	int			*count	= NULL;
	int32			month	= 5;
	int32			mode	= 4;
	int32			unit	= 0;
	int32			offset	= 0;
	int32			validity_align = 0;
    int32           child_align = 0;
    int32           errorCode = 0;
	struct tm		*current_time;
	int32			*validity_alignp = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;

	struct tm	*timeeff;
	time_t		*charged_from_t = NULL;
	time_t		*charged_to_t = NULL;
	time_t		*charged_to = NULL;
	time_t		*cycle_end_t = NULL;
	time_t		*cycle_start_t = NULL;
	time_t		*purchase_end_t = NULL;
	time_t		*purchase_start_t = NULL;
	time_t		*purchase_end = NULL;
	time_t		current_t = 0;
	time_t		cet = 0;
	time_t		*end_t = NULL;
    time_t      actg_next_t = 0;
	// Added for renewal catv
	time_t		*in_purchase_end_t = NULL;
	pin_flist_t	*wrt_iflistp = NULL;
	pin_flist_t	*wrt_oflistp = NULL;
	pin_flist_t	*alc_addon_pdts_flistp = NULL;
	pin_flist_t 	*alc_addon_pdts_main_flistp = NULL;
	pin_flist_t 	*alc_addon_pdts_child_flistp = NULL;
	int32		ret_status = -1;
	int32		failure = 1;
	int32		align_flag = 1;
	// Added for combo plan renewl 
	pin_flist_t	*srch_i_flistp	= NULL;
	pin_flist_t	*srch_o_flistp	= NULL;
	pin_flist_t	*args_flistp	= NULL;
	pin_flist_t	*results_flistp	= NULL;
	pin_flist_t	*plans_flistp	= NULL;
	pin_flist_t	*res_flistp	= NULL;
	pin_flist_t     *prov_in_flistp = NULL;
	pin_flist_t     *prov_out_flistp = NULL;
	pin_flist_t     *pdt_flistp = NULL;
	pin_flist_t     *write_flds_inflistp = NULL;
	pin_flist_t     *write_flds_outflistp = NULL;
	pin_flist_t     *svc_flistp = NULL;
	pin_flist_t     *svc_res_flistp = NULL;
	pin_flist_t	*read_plan_flistp = NULL;
	pin_flist_t	*read_plan_oflistp = NULL;
	pin_flist_t	*res_flistp1 = NULL;
	pin_flist_t	*reorder_flistp = NULL;
	pin_flist_t	*discount_flistp = NULL;
	pin_flist_t	*subs_pdts_flistp = NULL;
	pin_flist_t	*pdt_disc_search_flistp = NULL;
	pin_flist_t	*base_res_flistp = NULL;
	pin_flist_t	*disc_flistp = NULL;

	poid_t		*plan_obj = NULL;
	poid_t		*plan_pdp = NULL;
	poid_t		*prod_pdp = NULL;
	poid_t		*new_serv_pdp   = NULL;
	poid_t		*base_plan_obj = NULL;
	int32		srch_flags		= 256;
	int32		prod_status		= 1;
	char		*srch_tmpl		= "select X from /purchased_product where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3";
	int32		status			= 1;
	int32           is_base_pack    = -1;
    int32       base_pack_renew = 0;
	time_t          bp_valid_flag   = 0;
	time_t          bp_parent_valid_flag = 0;
	time_t          child_bp_valid_flag = 0;
	int32           new_status_flags = 8;
	int		add_plan_flag = CATV_ADD_PACKAGE;
    int         adv_renew_flag = 1;
	int32	   	prd_cnt  = 0;
	int32	    	new_elem_id = 0;
	int32		date_align = 2;
	int32		skip_discount = 0;

	char		*cust_city = NULL;
	char		*das_type = NULL;
	int32		discount_flag = -1;
	int32		count1 = 0;
	int32		pkg_type = -1;
	int32		conn_type = -1;
	int		pp_type = 2;
	pin_decimal_t	*disc_per = NULL;
	pin_decimal_t	*disc_amt = NULL;
	pin_decimal_t	*zero_disc = pbo_decimal_from_str("0.0", ebufp);
	//New Tariff - Transaction mapping */
	pin_flist_t             *lc_plan_list_flistp = NULL;
	pin_flist_t             *eflistp = NULL;
	int32                   offer_count = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return ;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_plan input flist", in_flist);
	current_t = pin_virtual_time(NULL);

	acc_pdp = PIN_FLIST_FLD_GET(in_flist,PIN_FLD_POID , 1 , ebufp);
	service_obj = PIN_FLIST_FLD_GET(in_flist,PIN_FLD_SERVICE_OBJ , 1 , ebufp);
	plans_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_PLAN_LISTS, 0, 1,ebufp);
	plan_pdp = PIN_FLIST_FLD_GET(plans_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);

	validity_alignp = (int32 *)PIN_FLIST_FLD_GET(in_flist, PIN_FLD_VALIDITY_FLAGS, 1, ebufp);
	if (validity_alignp)
	{
		validity_align = *validity_alignp;
	}
    validity_alignp = (int32 *)PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PARENT_FLAGS, 1, ebufp);
    if (validity_alignp)
    {
        child_align = *validity_alignp;
    }
	conn_type = fm_mso_catv_conn_type(ctxp, service_obj, ebufp);
	/* checking for demo pack renewal validation*/
	read_plan_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_plan_flistp, PIN_FLD_POID, plan_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_plan_flistp, PIN_FLD_NAME, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_reactivate_plan readplan input flist", read_plan_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_plan_flistp, &read_plan_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_plan_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling Read_plan name for Renewal ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_plan_oflistp, ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal Failed :Read PLan failed ", ebufp);
		*ret_flistp = r_flistp;
		goto CLEANUP;
	}
	plan_name = PIN_FLIST_FLD_GET(read_plan_oflistp, PIN_FLD_NAME, 0, ebufp);
	if (strcmp(plan_name,"HD STARTER DEMO 7 DAYS PACKAGE") ==0 || strcmp(plan_name,"WEST BASIC SERVICE TIER DEMO 7DAY") ==0 || strcmp(plan_name,"KARNATAKA BASIC SERVICE TIER SP DEMO") ==0 || strcmp(plan_name,"HYDERABAD PRIME DEMO 7DAY") ==0 ||strcmp(plan_name,"EAST BASIC SERVICE TIER DEMO 7DAY") ==0 || strcmp(plan_name,"JAIPUR BASIC SERVICE TIER DEMO 7DAY") ==0 ||strcmp(plan_name,"MP BASIC SERVICE TIER DEMO 7DAY") ==0  || strcmp(plan_name,"NORTH BASIC SERVICE TIER DEMO 7DAY") ==0 || strcmp(plan_name,"HD PREMIUM DEMO 10DAY") ==0 ||strcmp(plan_name,"HYBRID STB SCHEME Rs 1500 60d") ==0 ||strcmp(plan_name,"KA DTH TO HYBRID STB SCHEME 1500 90d") ==0) 
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal Failed :Demo plan renewal not allowed ", ebufp);
		*ret_flistp = r_flistp;
		goto CLEANUP;
	}

	/*Searching puchased product to find the products purchased under this plan*/
	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(service_obj), "/search", (int64)-1, ebufp);
	srch_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE,	srch_tmpl, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp , PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp , PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp , PIN_FLD_STATUS, &prod_status, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS,	PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(3, "Search input flist", srch_i_flistp);

	PCM_OP(ctxp , PCM_OP_SEARCH, 0, srch_i_flistp, &srch_o_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_i_flistp,ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH for Renewal ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal Failed : ", ebufp);		
		*ret_flistp = r_flistp;
		goto CLEANUP;
	} 
	prd_cnt = PIN_FLIST_ELEM_COUNT(srch_o_flistp, PIN_FLD_RESULTS, ebufp);

	PIN_ERR_LOG_FLIST(3, "Search out flist", srch_o_flistp);	
	if(prd_cnt >= 1)
	{	
		reorder_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(srch_o_flistp, PIN_FLD_POID, reorder_flistp, PIN_FLD_POID, ebufp);
		while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_o_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			pp_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
			is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, pp_pdp, ebufp);
			if(is_base_pack == 0)	
			{	
                base_pack_renew = 1;
				validity_align = 0;		
				PIN_FLIST_ELEM_COPY(srch_o_flistp, PIN_FLD_RESULTS, elem_id, reorder_flistp, PIN_FLD_RESULTS, new_elem_id, ebufp);
				while ((res_flistp1 = PIN_FLIST_ELEM_GET_NEXT(srch_o_flistp,
								PIN_FLD_RESULTS, &elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
				{
					prod_pdp = PIN_FLIST_FLD_GET(res_flistp1, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
					if(PIN_POID_COMPARE(pp_pdp, prod_pdp, 0, ebufp))
					{
						new_elem_id++;
						PIN_FLIST_ELEM_COPY(srch_o_flistp, PIN_FLD_RESULTS, elem_id1, reorder_flistp, PIN_FLD_RESULTS, new_elem_id,ebufp);
					}
				}
				break;
			}
		} 
		if(PIN_FLIST_ELEM_COUNT(reorder_flistp, PIN_FLD_RESULTS, ebufp))
		{
			PIN_ERR_LOG_FLIST(3, "Reordered flist", reorder_flistp);
			PIN_FLIST_DESTROY_EX(&srch_o_flistp,ebufp);
			srch_o_flistp = PIN_FLIST_COPY(reorder_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&reorder_flistp, ebufp);
		}

	}
	else if (prd_cnt == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This plan is having no purchased product !!!");
        status = 0;
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51418", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Plan trying to renew is not active", ebufp);
		*ret_flistp = r_flistp;
		goto CLEANUP;
	}
	else
	{
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51417", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renew Failed as no active plan ", ebufp);
		*ret_flistp = r_flistp;
		goto CLEANUP;
	}


	PIN_ERR_LOG_FLIST(3, "Search output flist", srch_o_flistp);
	//New Tariff - Transaction mapping added the below line
	lc_inflist = PIN_FLIST_COPY(in_flist, ebufp);
	lc_plan_list_flistp = PIN_FLIST_ELEM_GET(lc_inflist, PIN_FLD_PLAN_LISTS, 0, 1, ebufp);

	is_base_pack = -1;
	elem_id = 0;
	cookie = NULL;	
	while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_o_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{

		//pp_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_OFFERING_OBJ, 0, ebufp);
		pp_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);

		in_purchase_end_t = PIN_FLIST_FLD_GET(in_flist,PIN_FLD_PURCHASE_END_T, 1, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_CUST_ADD_PLAN for Renewal ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal Failed : ", ebufp);		
			*ret_flistp = r_flistp;
			goto CLEANUP;
		} 
		if(PIN_POID_IS_NULL(pp_pdp))
		{
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Purchased Poid is NULL");
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal failed : ", ebufp);		
			*ret_flistp = r_flistp;
			goto CLEANUP ;
		}

		/********
		 * Read the purchase poid to get the charged dates 
		 *********/
		/*commenting the read object becuase already search results are available*
		  read_flistp = PIN_FLIST_CREATE(ebufp);
		  PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, pp_pdp, ebufp);
		  PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_flistp, &read_oflistp, ebufp);

		  purchase_product_roflist = PIN_FLIST_COPY(read_oflistp, ebufp);
		  PIN_FLIST_DESTROY_EX(&read_flistp, NULL);

		  if (PIN_ERRBUF_IS_ERR(ebufp))
		  {
		  PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_OBJ", ebufp);
		  PIN_ERRBUF_RESET(ebufp);
		  PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
		  return;
		  }*/

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG," Purchased product read flds output flist is " , res_flistp);
		purchase_end_t	= PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
		purchase_start_t = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PURCHASE_START_T, 1 , ebufp);
		product_pdp	= PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
		status_flags = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp);
		cet = *(time_t *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
		/*	purchase_end_details = PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_PURCHASE_END_DETAILS, 0, ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Error in getting end details ");
			PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
			return;
			}
		 */
		cycle_flist = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);

		if(cycle_flist != NULL)
		{
			charged_from_t = PIN_FLIST_FLD_GET(cycle_flist, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
			charged_to_t   = PIN_FLIST_FLD_GET(cycle_flist, PIN_FLD_CHARGED_TO_T, 1, ebufp);
			cycle_end_t = PIN_FLIST_FLD_GET(cycle_flist, PIN_FLD_CYCLE_FEE_END_T, 1, ebufp);
			cycle_start_t = PIN_FLIST_FLD_GET(cycle_flist, PIN_FLD_CYCLE_FEE_START_T, 1, ebufp);
		}

		/***** 
		 * decode the end details to get the offset value 
		 ****/
		//	offset = PIN_VALIDITY_GET_OFFSET(*purchase_end_details);

		//sprintf(msg , " offset fetched value %d " , offset);

		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " Offset value is ");
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , msg);


		/********************************
		 * Read the product to get the actual event type 
		 ********************/

		read_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, product_pdp, ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_END_T, end_t, ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_PRIORITY, 0, ebufp);
		usage_flistp = PIN_FLIST_ELEM_ADD(read_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(usage_flistp, PIN_FLD_EVENT_TYPE, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Readflds products input", read_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &r_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while product read flds", ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Purchased Poid is NULL");
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal failed : ", ebufp);		
			goto CLEANUP;
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Readflds products output", r_flistp);

		end_t = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_END_T, 0, ebufp);
		if(end_t && *end_t != 0 && *end_t <= current_t)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Renewal not allowed for Expired Package");
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51417", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal failed : Renewal not allowed for Expired Package", ebufp);
			*ret_flistp = r_flistp;
			goto CLEANUP;
		}
		usage_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 0,  ebufp);
		event_type = (char *)PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);
		cycle_type = strrchr(event_type, '/');

		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_EVENT_TYPE, event_type, ebufp);

		if( (!strcmp("/mso_sb_adn_fdp", cycle_type)) || (!strcmp("/mso_sb_pkg_fdp", cycle_type)) || (!strcmp("/mso_sb_alc_fdp", cycle_type)) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , " This is fixed duration plan ");
			//lc_inflist = PIN_FLIST_COPY(in_flist, ebufp);
			//New Tariff - Transaction mapping Start */
			eflistp = PIN_FLIST_ELEM_ADD(lc_plan_list_flistp,PIN_FLD_OFFERINGS, offer_count++, ebufp);
			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, eflistp, PIN_FLD_OFFERING_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_PACKAGE_ID, eflistp, PIN_FLD_PACKAGE_ID, ebufp);
			//New Tariff - Transaction mapping End */

			/************************
			 * search the event_map to get the count of the event
			 ****************************************/
			//	if(charged_to_t)
			//	{
			//		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_CHARGED_TO_T ,charged_to_t, ebufp);
			//	}
			//	else
			//	{
			//		PIN_FLIST_FLD_SET(r_flistp,PIN_FLD_PURCHASE_END_T,purchase_end_t, ebufp);
			//	}
			if(purchase_end_t)
			{	
				PIN_FLIST_FLD_SET(r_flistp,PIN_FLD_PURCHASE_END_T,purchase_end_t, ebufp);
			}

			// Validation for Renewal of the Grace Period Set products
			if (status_flags && ((*status_flags == 0xFFFF) || (*status_flags == 0xFFF)))
			{
				if (purchase_end_t && cet && (*purchase_end_t > cet))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG , "Fixing the Actual Purchase End Date");
					// Fixing the new purchase from the actual purchase end date before renewal date
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_PURCHASE_END_T, &cet, ebufp);

				}

				if (*status_flags == 0xFFF)
				{
					svc_flistp = PIN_FLIST_CREATE(ebufp);
					//Validating the Main Connection or Child Connection
					fm_mso_utils_get_catv_main(ctxp, acc_pdp, &svc_flistp, ebufp);
					if (svc_flistp)
					{
						svc_res_flistp = PIN_FLIST_ELEM_GET(svc_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
						new_serv_pdp = PIN_FLIST_FLD_GET(svc_res_flistp, PIN_FLD_POID, 1, ebufp);
						if (PIN_POID_COMPARE(service_obj, new_serv_pdp, 0, ebufp) == 0 || conn_type == 0 ||
							conn_type == 1)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Service is a Main Connection");
							// Current Service is a Main Service
							// Validating the Base FDP Packs purchase to enable / reactivate the Ala-Carte & Add-On products
							// This section is for renewal of Base FDP plan after Grace Date
							is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, product_pdp, ebufp);
							if (is_base_pack == 0)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Pack of Main Connection");

								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_activate_alc_addon_pdts function");
								fm_mso_activate_alc_addon_pdts(ctxp, in_flist, product_pdp, &alc_addon_pdts_flistp, ebufp);
								if (alc_addon_pdts_flistp)
								{
									ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_flistp, PIN_FLD_STATUS, 1, ebufp);
									if (ret_status == failure)
									{
										r_flistp = PIN_FLIST_CREATE(ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);
										*ret_flistp = r_flistp;
										PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
										return ;
									}
								}
								PIN_ERRBUF_RESET(ebufp);
							}
							else
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Product is a Non-Base Pack. Need to check the Base Pack status");

								fm_mso_get_base_pdts(ctxp, acc_pdp, service_obj, &alc_addon_pdts_main_flistp, ebufp);
								if (alc_addon_pdts_main_flistp)
								{
									ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp, PIN_FLD_STATUS, 1, ebufp);
									if (ret_status == failure)
									{
										r_flistp = PIN_FLIST_CREATE(ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);
										*ret_flistp = r_flistp;
										PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
										return ;
									}

									//Checking the Base Pack Validity for Re-activation of other Packs
									bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 0, ebufp);

									if (bp_valid_flag > 0)
									{

										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Pack is Active, hence the current pack can be re-activated.");
										prov_in_flistp = PIN_FLIST_CREATE(ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, prov_in_flistp, PIN_FLD_POID, ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, prov_in_flistp, PIN_FLD_USERID, ebufp);
										PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_PROGRAM_NAME, "Plan Re-activation|Grace Period Renewal", ebufp);
										PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &add_plan_flag, ebufp);

										pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
										//PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_OFFERING_OBJ, pdt_flistp, PIN_FLD_POID, ebufp);
										//combo pack renewal issue fix
										PIN_FLIST_FLD_SET(pdt_flistp, PIN_FLD_POID, pp_pdp, ebufp);

										PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Input flist", prov_in_flistp);
										PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
										PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Output flist", prov_out_flistp);

										PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
										if (PIN_ERRBUF_IS_ERR(ebufp))
											PIN_ERRBUF_RESET(ebufp);

										PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);

										write_flds_inflistp = PIN_FLIST_CREATE(ebufp);
										//Combo pack renewal issue fix
										// PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_OFFERING_OBJ, write_flds_inflistp, PIN_FLD_POID, ebufp);
										PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_POID, pp_pdp, ebufp);
										PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);

										PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

										if (PIN_ERRBUF_IS_ERR(ebufp))
										{
											PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
													"Error in fm_mso_reactivate_plan: wflds", ebufp);
										}

										PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
										PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
									}
									else
									{
										r_flistp = PIN_FLIST_CREATE(ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);		
										*ret_flistp = r_flistp;
										goto CLEANUP;
									}
									base_event_type = PIN_FLIST_FLD_TAKE(alc_addon_pdts_main_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);

									PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
								}

							}   
						}
						else
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Service is a Child Connection");
							// Current Service is a Child Service

							// This section is for renewal of Base FDP plan after Grace Date
							is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, product_pdp, ebufp);
							if (is_base_pack == 0)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Pack of Child Connection");
								//Validating the Main Connection's Base Pack
								fm_mso_get_base_pdts(ctxp, acc_pdp, new_serv_pdp, &alc_addon_pdts_main_flistp, ebufp);
								if (alc_addon_pdts_main_flistp)
								{
									ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp, PIN_FLD_STATUS, 1, ebufp);
									if (ret_status == failure)
									{
										r_flistp = PIN_FLIST_CREATE(ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
											"Base Package in Main Connection is Inactive. First Activate the Base Pack", ebufp);
								 		*ret_flistp = r_flistp;
								 		PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
								 		return ;
								 	}
								 	bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 0, ebufp);

								 	if (bp_valid_flag > 0)
								 	{

								 		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_activate_alc_addon_pdts function");
								 		fm_mso_activate_alc_addon_pdts(ctxp, in_flist, product_pdp, &alc_addon_pdts_flistp, ebufp);
								 		if (alc_addon_pdts_flistp)
								 		{
								 			ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_flistp, PIN_FLD_STATUS, 1, ebufp);
								 			if (ret_status == failure)
								 			{
								 				r_flistp = PIN_FLIST_CREATE(ebufp);
								 				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
								 				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
								 				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
								 				PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
								 				"Base Package in Main Connection is Inactive. First Activate the Base Pack", ebufp);
								 				*ret_flistp = r_flistp;
								 				PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
								 				return ;
								 			}
								 		}
								 		PIN_ERRBUF_RESET(ebufp);
								 	}
								 	else
								 	{
								 		r_flistp = PIN_FLIST_CREATE(ebufp);
								 		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
								 		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
								 		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
								 		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
								 		"Base Package in Main Connection is Inactive. First Activate the Base Pack", ebufp);		
								 		*ret_flistp = r_flistp;
								 		goto CLEANUP;
								 	}
								 }
							}
							else
							{   
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
										"Current Product is a Non-Base Pack of Child Connection. Need to check the Both Base Pack status");

								fm_mso_get_base_pdts(ctxp, acc_pdp, service_obj, &alc_addon_pdts_child_flistp, ebufp);
								fm_mso_get_base_pdts(ctxp, acc_pdp, new_serv_pdp, &alc_addon_pdts_main_flistp, ebufp);
								if (alc_addon_pdts_main_flistp && alc_addon_pdts_child_flistp)
								{
									 ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp, PIN_FLD_STATUS, 1, ebufp);
									 if (ret_status == failure)
									 {
									 	r_flistp = PIN_FLIST_CREATE(ebufp);
									 	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
									 	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
									 	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
									 	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
									 		"Base Package is Inactive in Main or Child Connection. First Activate the Base Pack", ebufp);	
									 	*ret_flistp = r_flistp;
									 	PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
									 	return ;
									 }

									ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_child_flistp, PIN_FLD_STATUS, 1, ebufp);
									if (ret_status == failure)
									{
										r_flistp = PIN_FLIST_CREATE(ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
												"Base Package is Inactive in  Child Connection. First Activate the Base Pack", ebufp);	
										*ret_flistp = r_flistp;
										PIN_FLIST_DESTROY_EX(&alc_addon_pdts_child_flistp, NULL);
										return ;
									}

									//Checking the Base Pack Validity for Re-activation of other Packs
									bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 0, ebufp);
									child_bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_child_flistp, 0, ebufp);

									if (bp_valid_flag > 0 && child_bp_valid_flag > 0)
									{
										prov_in_flistp = PIN_FLIST_CREATE(ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, prov_in_flistp, PIN_FLD_POID, ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, prov_in_flistp, PIN_FLD_USERID, ebufp);
										PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_PROGRAM_NAME, "Plan Re-activation|Grace Period Renewal", ebufp);
										PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &add_plan_flag, ebufp);

										pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_OFFERING_OBJ, pdt_flistp, PIN_FLD_POID, ebufp);

										PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Input flist", prov_in_flistp);
										PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
										PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Output flist", prov_out_flistp);

										PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
										if (PIN_ERRBUF_IS_ERR(ebufp))
											PIN_ERRBUF_RESET(ebufp);

										PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);

										write_flds_inflistp = PIN_FLIST_CREATE(ebufp);

										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_OFFERING_OBJ, write_flds_inflistp, PIN_FLD_POID, ebufp);
										PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);

										PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

										if (PIN_ERRBUF_IS_ERR(ebufp))
										{
											PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
													"Error in fm_mso_reactivate_plan: wflds", ebufp);
										}

										PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
										PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
									}
									else
									{
										r_flistp = PIN_FLIST_CREATE(ebufp);
										PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
										PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR,
												"Base Package is Inactive in Main or Child Connection. First Activate the Base Pack", ebufp);		
										*ret_flistp = r_flistp;
										goto CLEANUP;
									}
									base_event_type = PIN_FLIST_FLD_TAKE(alc_addon_pdts_child_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);

									PIN_FLIST_DESTROY_EX(&alc_addon_pdts_child_flistp, NULL);
									PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
								}
							} /* Non base pack*/
						} /* Child connection condition end */
					} /* Service flist check condition end */
				} /* Grace inner if condition end */
			} /* Grace outer if condition end */

			if (validity_align)
			{
				// Jyothirmayi Kachiraju - Changes for new FDP Event Creation
				if ((bp_valid_flag != 0 || child_bp_valid_flag != 0) && 
				((strstr(event_type, "cycle_forward_thirty_days") && !strstr(base_event_type, "cycle_forward_thirty_days")) ||
				 (strstr(event_type, "cycle_forward_sixty_days") && !strstr(base_event_type, "cycle_forward_sixty_days")) || 
				 (strstr(event_type, "cycle_forward_ninety_days") && !strstr(base_event_type, "cycle_forward_ninety_days")) ||
				 (strstr(event_type, "cycle_forward_onetwenty_days") && !strstr(base_event_type, "cycle_forward_onetwenty_days")) ||
				 (strstr(event_type, "cycle_forward_oneeighty_days") && !strstr(base_event_type, "cycle_forward_oneeighty_days")) ||		
				 (strstr(event_type, "cycle_forward_onefifty_days") && !strstr(event_type, "cycle_forward_onefifty_days")) || 
				 (strstr(event_type, "cycle_forward_twoten_days") && !strstr(event_type, "cycle_forward_twoten_days")) || 
				 (strstr(event_type, "cycle_forward_twoforty_days") && !strstr(event_type, "cycle_forward_twoforty_days")) || 
				 (strstr(event_type, "cycle_forward_twoseventy_days") && !strstr(event_type, "cycle_forward_twoseventy_days")) || 
				 (strstr(event_type, "cycle_forward_threehundred_days") && !strstr(event_type, "cycle_forward_threehundred_days")) || 
				 (strstr(event_type, "cycle_forward_threethirty_days") && !strstr(event_type, "cycle_forward_threethirty_days")) || 
				 (strstr(event_type, "cycle_forward_threesixty_days") && !strstr(event_type, "cycle_forward_threesixty_days"))))
				{
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51419", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Pack payterm is not matching to align, please disable validity align flag", ebufp);      
					goto CLEANUP;
				}
			}

			fm_mso_search_event_map(ctxp, r_flistp, &usage_oflistp, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " usage_oflistp is ", usage_oflistp);   //new date
			//PIN_FLIST_FLD_COPY(res_flistp,  PIN_FLD_PURCHASE_END_T, lc_inflist, MSO_FLD_FROM_DATE, ebufp);
			//Setting cycle_end_t to populate correct dates as purchase end date gas grace end date
			PIN_FLIST_FLD_COPY(res_flistp,  PIN_FLD_CYCLE_END_T, lc_inflist, MSO_FLD_FROM_DATE, ebufp);
			//PIN_FLIST_FLD_COPY(read_oflistp,  PIN_FLD_CYCLE_END_T, lc_inflist, MSO_FLD_TO_DATE, ebufp);
			//PIN_FLIST_FLD_COPY(usage_oflistp, PIN_FLD_CHARGED_TO_T, lc_inflist, MSO_FLD_FROM_DATE, ebufp);
			if(usage_oflistp)
			{
				//count = (int *)PIN_FLIST_FLD_GET(usage_oflistp, PIN_FLD_COUNT, 0, ebufp);
				//	charged_to = PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
				purchase_end = PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
				//*count = *count + offset;
			}


			/*************************
			 * Prepare the flist to call PCM_OP_SUBSCRIPTION_SET_PRODINFO opcode 
			 *************************/

			if (in_purchase_end_t && *in_purchase_end_t !=0 && validity_align == 0) 
			{

				//Added to prevent date alignment less that current date
				if(*in_purchase_end_t <= current_t)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Date alignment not allowed less than current date");
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Date alignment not allowed less than current date", ebufp);
					*ret_flistp = r_flistp;
					goto CLEANUP;
				}
				//Date aligment date validation end	
				pkg_type = fm_mso_catv_pt_pkg_type(ctxp, product_pdp, ebufp);
				if (pkg_type == 0)
				{
					prov_in_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, prov_in_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, prov_in_flistp, PIN_FLD_USERID, ebufp);
					plan_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 0, ebufp);
					sprintf(msg, "LBO|%s", plan_name);
					PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_PROGRAM_NAME, msg, ebufp);
					PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &add_plan_flag, ebufp);

					pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
					PIN_FLIST_FLD_SET(pdt_flistp, PIN_FLD_POID, pp_pdp, ebufp);

					pkg_type = NCF_PKG_TYPE;
				}
				PIN_FLIST_DESTROY_EX(&wrt_oflistp, NULL);  
				wrt_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(res_flistp,PIN_FLD_POID,wrt_iflistp,PIN_FLD_POID,ebufp);
				PIN_FLIST_FLD_SET(wrt_iflistp , PIN_FLD_PURCHASE_END_T , in_purchase_end_t , ebufp);
				PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_CYCLE_END_T, in_purchase_end_t, ebufp);		
				PIN_FLIST_FLD_SET(wrt_iflistp , PIN_FLD_USAGE_END_T , in_purchase_end_t , ebufp);

				/********************************************************************************
				 * Check if existing status_flags is grace flag, then only reset
				 *******************************************************************************/	
				if (status_flags && (*status_flags == 0xFFFF))
				{
					PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_STATUS_FLAGS, &align_flag, ebufp);
				}	
				cycle_flist = PIN_FLIST_ELEM_ADD(wrt_iflistp,PIN_FLD_CYCLE_FEES,1,ebufp);
				PIN_FLIST_FLD_SET(cycle_flist,PIN_FLD_CHARGED_TO_T,in_purchase_end_t,ebufp);
				PIN_FLIST_FLD_SET(cycle_flist,PIN_FLD_CYCLE_FEE_END_T,in_purchase_end_t,ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "FDP plan renewal write flds input flist", wrt_iflistp);

				PCM_OP(ctxp,PCM_OP_WRITE_FLDS,0,wrt_iflistp,&wrt_oflistp,ebufp);
				PIN_FLIST_DESTROY_EX(&wrt_iflistp, NULL); 
				if (PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while calling PCM_OP_WRITE_FLDS in plan renewal ", ebufp);
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal failed : ", ebufp);		
					*ret_flistp = r_flistp;
					goto CLEANUP;
				}

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "FDP plan renewal write flds output flist", wrt_oflistp);
				PIN_FLIST_FLD_SET(wrt_oflistp, PIN_FLD_STATUS, &add_plan_success, ebufp);
				PIN_FLIST_FLD_SET(wrt_oflistp, PIN_FLD_ERROR_DESCR, "Renewal Happened Sucessfully", ebufp);

				PIN_FLIST_FLD_SET(lc_inflist,MSO_FLD_TO_DATE ,in_purchase_end_t, ebufp);
				PIN_FLIST_FLD_SET(lc_inflist, PIN_FLD_FLAGS, &date_align, ebufp);
				//*ret_flistp = PIN_FLIST_COPY(wrt_oflistp, ebufp);

			}
			else
			{
				// Renewal will  not be allowed before 27 days of package expiry from current date
				current_t = pin_virtual_time(NULL);
				if ((cet - fm_utils_time_round_to_midnight(current_t)) > 2332800 ) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Renewal not allowed before 27 days of pakcage expire");
					r_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_CYCLE_END_T, &cet, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ACTION_T, &current_t, ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
					PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal failed : Not allowed before 27 days of package expiry", ebufp);        
					*ret_flistp = r_flistp;
					goto CLEANUP;
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting Package Type");
					//added this condition because in case of combo packs with base pack both packs need discount
					/* Comment this line to verify discount at pkg_type whithin a DPO pack*/
					//if(pkg_type !=0)
					//{
					pkg_type = fm_mso_catv_pt_pkg_type(ctxp, product_pdp, ebufp);
					if (pkg_type == 0)
					{
						prov_in_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, prov_in_flistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, prov_in_flistp, PIN_FLD_USERID, ebufp);
						plan_name = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PROGRAM_NAME, 0, ebufp);
						sprintf(msg, "LBO|%s", plan_name);
						PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_PROGRAM_NAME, msg, ebufp);
						PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &add_plan_flag, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before advance renewal check");
                        PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_OVERRIDE_FLAGS, &adv_renew_flag, ebufp);
						pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
						PIN_FLIST_FLD_SET(pdt_flistp, PIN_FLD_POID, pp_pdp, ebufp);

						pkg_type = NCF_PKG_TYPE;
					}
					//}
					if (pkg_type != 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Product is a Non-Base Pack. Need to check the Base Pack status");

						fm_mso_get_base_pdts(ctxp, acc_pdp, service_obj, &alc_addon_pdts_main_flistp, ebufp);
						if (alc_addon_pdts_main_flistp && alc_addon_pdts_main_flistp != NULL)
						{
							ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp, PIN_FLD_STATUS, 1, ebufp);
							if (ret_status == failure)
							{
								r_flistp = PIN_FLIST_CREATE(ebufp);
								PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
								PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
								PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
								PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);      
								*ret_flistp = r_flistp;
								PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
								goto CLEANUP;
							}
						}

						bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 1, ebufp);

						if (bp_valid_flag == 0)
						{
							r_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
							PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Base Package is Inactive. First Activate the Base Pack", ebufp);      
							goto CLEANUP;
						}
					}
					/************************************************************************
					 * Get registered parent service for NCF products 
					 ***********************************************************************/
					if (pkg_type == NCF_PKG_TYPE)
					{
						fm_mso_hier_group_get_parent(ctxp, service_obj, &parent_serv_flistp, ebufp);
					}
					if (parent_serv_flistp)
					{
						new_serv_pdp = PIN_FLIST_FLD_GET(parent_serv_flistp, PIN_FLD_OBJECT, 0, ebufp);
						parent_acc_pdp = PIN_FLIST_FLD_GET(parent_serv_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
						fm_mso_get_base_pdts(ctxp, parent_acc_pdp, new_serv_pdp, &alc_addon_pdts_main_flistp, ebufp);

						if (alc_addon_pdts_main_flistp && alc_addon_pdts_main_flistp != NULL)
						{
							ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp,
									PIN_FLD_STATUS, 0, ebufp);
							if (ret_status == failure)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Package of main coon is Inactive.");
								skip_discount = 1;
							}	
						}

						bp_parent_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 1, ebufp);
						if (bp_parent_valid_flag == 0)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Package of main coon is Inactive.");
							skip_discount = 1;
						}
						PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
					}

					if (!skip_discount)
					{
						pdt_disc_search_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, service_obj, ebufp);
						PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
						PIN_FLIST_FLD_SET(pdt_disc_search_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
						PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Looking for Account Level Discount Config");
						discount_flag = -1;
						fm_mso_get_service_level_discount(ctxp, pdt_disc_search_flistp, 0, &discount_flistp, ebufp);
					}
					if (discount_flistp)
					{
						ret_status = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_STATUS, 1, ebufp);
						if (ret_status != failure)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
									"Account Level Discount Config Found");
							discount_flag = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, 
									PIN_FLD_DISCOUNT_FLAGS, 0, ebufp);
							if (discount_flag == 0)
							{
								disc_per = PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
							}
							else if (discount_flag == 1)
							{
								disc_amt = PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
							}
						}
						else
						{
							if (PIN_ERRBUF_IS_ERR(ebufp))
								PIN_ERRBUF_RESET(ebufp);
							PIN_FLIST_DESTROY_EX(&pdt_disc_search_flistp, NULL);
							pdt_disc_search_flistp = PIN_FLIST_CREATE(ebufp);
							//PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, product_pdp, ebufp);
							PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, plan_pdp,ebufp);
							// Retrieving the PP Type 
							pp_type = fm_mso_get_cust_type(ctxp, acc_pdp, ebufp);
							if (pp_type != 2)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
										"Valid PP Type Found... ");
								PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
										MSO_FLD_PP_TYPE, &pp_type, ebufp);
							}

							//  Retrieve the Customer City from Account Object
							fm_mso_get_custcity(ctxp, acc_pdp, &cust_city, ebufp);
							if (cust_city)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CITY FOUND... ");
								PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
										PIN_FLD_CITY, cust_city, ebufp);

							}

							// Retrieve the Customer Service Connection Type
							conn_type = fm_mso_catv_conn_type(ctxp, service_obj, ebufp);
							if (conn_type != -1)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
										"Valid Connection Type FOUND... ");
								PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
										PIN_FLD_CONN_TYPE, &conn_type, ebufp);
							}

							// Retrieve the Customer DAS Type 
							fm_mso_get_das_type(ctxp, acc_pdp, &das_type, ebufp);
							if (das_type)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
										"DAS TYPE FOUND... ");
								PIN_FLIST_FLD_SET(pdt_disc_search_flistp, 
										MSO_FLD_DAS_TYPE, das_type, ebufp);
							}
							PIN_FLIST_DESTROY_EX(&discount_flistp, NULL);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
									"Looking for Product Level Discount Config");
							PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
							fm_mso_get_plan_level_discount(ctxp, pdt_disc_search_flistp, 
									&discount_flistp, ebufp);
							if (discount_flistp)
							{
								ret_status = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, 
										PIN_FLD_STATUS, 1, ebufp);
								if (ret_status != failure)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
											"Product Level Discount Config Found");
									discount_flag = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, 
											PIN_FLD_DISCOUNT_FLAGS, 0, ebufp);
									if (discount_flag == 0)
									{
										disc_per = PIN_FLIST_FLD_GET(discount_flistp, 
												PIN_FLD_DISCOUNT, 0, ebufp);
									}
									else if (discount_flag == 1)
									{
										disc_amt = PIN_FLIST_FLD_GET(discount_flistp, 
												PIN_FLD_DISCOUNT, 0, ebufp);
									}
								}

							}
						}
					}

					/*************************
					 * Prepare the flist to call PCM_OP_SUBSCRIPTION_SET_PRODINFO opcode 
					 *************************/
					PIN_FLIST_DESTROY_EX(&prod_oflist, NULL);
					prod_iflist = PIN_FLIST_CREATE(ebufp);

					PIN_FLIST_FLD_SET(prod_iflist, PIN_FLD_POID , acc_pdp, ebufp);
					if(service_obj)
						PIN_FLIST_FLD_SET(prod_iflist,PIN_FLD_SERVICE_OBJ , service_obj, ebufp);

					PIN_FLIST_FLD_SET(prod_iflist, PIN_FLD_PROGRAM_NAME , program_name, ebufp);
					products_flist = PIN_FLIST_ELEM_ADD(prod_iflist, PIN_FLD_PRODUCTS, 0, ebufp);
					PIN_FLIST_FLD_SET(products_flist, PIN_FLD_OFFERING_OBJ, pp_pdp, ebufp);
					PIN_FLIST_FLD_SET(products_flist, PIN_FLD_PRODUCT_OBJ , product_pdp, ebufp);
					PIN_FLIST_FLD_COPY(usage_oflistp, PIN_FLD_PURCHASE_END_T, lc_inflist,MSO_FLD_TO_DATE , ebufp);

					PIN_FLIST_FLD_COPY(usage_oflistp, PIN_FLD_PURCHASE_END_T,products_flist , PIN_FLD_PURCHASE_END_T ,ebufp);
					PIN_FLIST_FLD_COPY(usage_oflistp, PIN_FLD_PURCHASE_END_T,products_flist , PIN_FLD_CYCLE_END_T ,ebufp);
					PIN_FLIST_FLD_COPY(usage_oflistp, PIN_FLD_PURCHASE_END_T,products_flist , PIN_FLD_USAGE_END_T ,ebufp);
					//Calling future dated renewal function
					fm_mso_future_dated_renewal(ctxp, res_flistp, ebufp);
					if (discount_flag != -1)
					{
						if (discount_flag == 0)
						{
							PIN_FLIST_FLD_SET(products_flist, PIN_FLD_CYCLE_DISCOUNT, disc_per, ebufp);
							PIN_FLIST_FLD_SET(products_flist, PIN_FLD_PURCHASE_DISCOUNT, disc_per, ebufp);
							PIN_FLIST_FLD_SET(products_flist, PIN_FLD_USAGE_DISCOUNT, disc_per, ebufp);
						}
						else
						{
							PIN_FLIST_FLD_SET(products_flist, PIN_FLD_CYCLE_DISC_AMT, disc_amt, ebufp);
							PIN_FLIST_FLD_SET(products_flist, PIN_FLD_PURCHASE_DISC_AMT, disc_amt, ebufp);
							//REsetting percentage disc to remove % disc configured at deal level
							PIN_FLIST_FLD_SET(products_flist, PIN_FLD_CYCLE_DISCOUNT, zero_disc, ebufp);
							PIN_FLIST_FLD_SET(products_flist, PIN_FLD_PURCHASE_DISCOUNT, zero_disc, ebufp);
							PIN_FLIST_FLD_SET(products_flist, PIN_FLD_USAGE_DISCOUNT, zero_disc, ebufp);
						}
					}
					else
					{
						PIN_ERR_LOG_MSG(3, "NO discount configs available so restting existing values");
						fm_mso_fetch_deal_disc(ctxp, res_flistp, ebufp);

						//PIN_FLIST_FLD_SET(products_flist, PIN_FLD_CYCLE_DISCOUNT, zero_disc, ebufp);
						//PIN_FLIST_FLD_SET(products_flist, PIN_FLD_PURCHASE_DISCOUNT, zero_disc, ebufp);
						//PIN_FLIST_FLD_SET(products_flist, PIN_FLD_USAGE_DISCOUNT, zero_disc, ebufp);
						PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_CYCLE_DISCOUNT, products_flist, PIN_FLD_CYCLE_DISCOUNT, ebufp);
						PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_PURCHASE_DISCOUNT, products_flist, PIN_FLD_PURCHASE_DISCOUNT, ebufp);
						PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_USAGE_DISCOUNT, products_flist, PIN_FLD_USAGE_DISCOUNT, ebufp);
						PIN_FLIST_FLD_SET(products_flist, PIN_FLD_CYCLE_DISC_AMT, zero_disc, ebufp);
						PIN_FLIST_FLD_SET(products_flist, PIN_FLD_PURCHASE_DISC_AMT, zero_disc, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Discount Configs Found");
						if (PIN_ERRBUF_IS_ERR(ebufp))
							PIN_ERRBUF_RESET(ebufp);
					}

					/********************************************************************************
					 * Check if existing status_flags is grace flag, then only reset
					 *******************************************************************************/	
					if (status_flags && (*status_flags == 0xFFFF))
					{
						PIN_FLIST_FLD_SET(products_flist, PIN_FLD_STATUS_FLAGS, &align_flag, ebufp);
					}
                    if (in_purchase_end_t && validity_align)
                    {
                        PIN_FLIST_FLD_SET(products_flist, PIN_FLD_USAGE_END_T, in_purchase_end_t, ebufp);
                        PIN_FLIST_FLD_SET(products_flist, PIN_FLD_CYCLE_END_T, in_purchase_end_t, ebufp);
                        PIN_FLIST_FLD_SET(products_flist, PIN_FLD_PURCHASE_END_T, in_purchase_end_t, ebufp);
                    }
					if (child_bp_valid_flag > cet && validity_align)
					{
						PIN_FLIST_FLD_SET(products_flist, PIN_FLD_USAGE_END_T, &child_bp_valid_flag, ebufp);
						PIN_FLIST_FLD_SET(products_flist, PIN_FLD_CYCLE_END_T, &child_bp_valid_flag, ebufp);
						PIN_FLIST_FLD_SET(products_flist, PIN_FLD_PURCHASE_END_T, &child_bp_valid_flag, ebufp);
					}
					else if (bp_valid_flag > cet && validity_align)
					{
						PIN_FLIST_FLD_SET(products_flist, PIN_FLD_USAGE_END_T, &bp_valid_flag, ebufp);
						PIN_FLIST_FLD_SET(products_flist, PIN_FLD_CYCLE_END_T, &bp_valid_flag, ebufp);
						PIN_FLIST_FLD_SET(products_flist, PIN_FLD_PURCHASE_END_T, &bp_valid_flag, ebufp);
					}
                    else if (bp_parent_valid_flag> 1 && child_align)
                    {
                        PIN_FLIST_FLD_SET(products_flist, PIN_FLD_USAGE_END_T, &bp_parent_valid_flag, ebufp);
                        PIN_FLIST_FLD_SET(products_flist, PIN_FLD_CYCLE_END_T, &bp_parent_valid_flag, ebufp);
                        PIN_FLIST_FLD_SET(products_flist, PIN_FLD_PURCHASE_END_T, &bp_parent_valid_flag, ebufp);
                    }

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_SUBSCRIPTION_SET_PRODINFO Input flist", prod_iflist);
					PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_SET_PRODINFO, 0, prod_iflist , &prod_oflist , ebufp);
					PIN_FLIST_DESTROY_EX(&prod_iflist, NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp)) 
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while calling PCM_OP_SUBSCRIPTION_SET_PRODINFO ", ebufp);
                        field_no = ebufp->field;
                        errorCode = ebufp->pin_err;

                        PIN_ERRBUF_RESET(ebufp);
						r_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
                        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
                        if (errorCode == PIN_ERR_BAD_VALUE && field_no == PIN_FLD_CURRENT_BAL)
                        {
                            PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service renew plan - There is no sufficient balance", ebufp);
                        }
                        else
                        {
						    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal failed : ", ebufp);		
                        }
						*ret_flistp = r_flistp;
						goto CLEANUP;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_SUBSCRIPTION_SET_PRODINFO output flist", prod_oflist);
					PIN_FLIST_FLD_SET(prod_oflist, PIN_FLD_STATUS, &add_plan_success, ebufp);
					PIN_FLIST_FLD_SET(prod_oflist, PIN_FLD_ERROR_DESCR, "Renewal Happened Sucessfully", ebufp);

					PIN_FLIST_DESTROY_EX(&discount_flistp, NULL);
				}

			}
			if (prov_in_flistp)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning NCF product Input flist", prov_in_flistp);
				PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning NCF product output flist", prov_out_flistp);

				PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in NCF MSO_OP_PROV_CREATE_CATV_ACTION ", ebufp);
					PIN_ERRBUF_RESET(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
			}

		}
		else
		{
			PIN_FLIST_DESTROY_EX(&lc_inflist, NULL);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Nothing to be done as this is not a fixed duration product ");
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "This is not a Fixed Duration package hence no renewal", ebufp);
			*ret_flistp = r_flistp;
			status = 0;
			break;

		}
	}

    if (base_pack_renew)
    {
        fm_mso_get_account_info(ctxp, acc_pdp, &r_flistp, ebufp);
        et_zonep = PIN_FLIST_FLD_GET(r_flistp, MSO_FLD_ET_ZONE, 0, ebufp);
        if (et_zonep && strstr(et_zonep, "SK_"))
        {
            PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
            fm_mso_get_all_billinfo(ctxp, acc_pdp, 1, &r_flistp, ebufp);
            results_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
            bi_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 0, ebufp);
            actg_next_t = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_ACTG_NEXT_T, 0, ebufp);
            mso_bill_pol_catv_main(ctxp, flags, bi_pdp, service_obj, actg_next_t, 0, &et_rflistp, ebufp);
            PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
            PIN_FLIST_DESTROY_EX(&et_rflistp, NULL);
        }
    }

	if(status ==1)
	{
		fm_mso_create_lifecycle_evt(ctxp, lc_inflist, NULL, ID_FDP_RENEW, ebufp );
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating Lifecycle object ", ebufp);
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "51416", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Renewal failed : ", ebufp);
			*ret_flistp = r_flistp;
			goto CLEANUP;
		}
		if(prod_oflist)
		{
			*ret_flistp = PIN_FLIST_COPY(prod_oflist, ebufp);
		}
		else
		{
			*ret_flistp = PIN_FLIST_COPY(wrt_oflistp, ebufp);
		}
	}
CLEANUP :
	//	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	pbo_decimal_destroy(&zero_disc);
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&prod_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&lc_inflist, NULL);
	PIN_FLIST_DESTROY_EX(&wrt_oflistp, NULL); 
	PIN_FLIST_DESTROY_EX(&svc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_o_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&read_plan_oflistp, NULL);
	if (base_event_type) free(base_event_type);

	return;
}

void
fm_mso_search_event_map(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flist,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp)
{

	int32			db = 0;
	int			flags = 768;
	int32                   *unit = NULL;
	pin_flist_t		*srch_flistp   = NULL;
	pin_flist_t		*srch_oflistp  = NULL;
	pin_flist_t		*args_flistp   = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*event_map_flistp = NULL;
	pin_flist_t		*events_flistp = NULL;
	pin_flist_t		*usage_flistp = NULL;
	pin_flist_t		*out_flistp = NULL;

	poid_t			*poid_p	= NULL;
	poid_t			*srch_pdp	= NULL;
	poid_t			*config_pdp	= NULL;

	int32			elemid = 0;
	int32			elemid1 = 0;
	int			*count	= NULL;

	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie1 = NULL;

	char	*template = "select X from /config where F1.type = V1 ";
	char	*event_type = NULL;
	char	*permitted = NULL;
	char	*config_event_type = NULL;
	time_t		*charged_to_t = NULL;
	time_t		*purchase_end_t = NULL;
	struct tm	*timeeff;
	int32           *flag = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in renew plan list ",ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " fm_mso_search_event_map input flist is " , in_flist);

	out_flistp	= PIN_FLIST_CREATE(ebufp);

	flag = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_FLAGS, 1, ebufp);
	usage_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 1,  ebufp);
	event_type = (char *)PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);

	charged_to_t = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_CHARGED_TO_T, 1, ebufp);
	if(charged_to_t)
		timeeff = localtime(charged_to_t);

	purchase_end_t = PIN_FLIST_FLD_GET(in_flist,PIN_FLD_PURCHASE_END_T, 1, ebufp);
	if(purchase_end_t)
		timeeff = localtime(purchase_end_t);

	poid_p = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	db = PIN_POID_GET_DB(poid_p);

	srch_pdp = PIN_POID_CREATE(db, "/search", (int64)-1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);

	config_pdp = PIN_POID_CREATE(db, "/config/event_map", (int64)-1, ebufp);

	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, config_pdp, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_event_map search input flist is ", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while readflds", ebufp);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_event_map search output flist is ", srch_oflistp);

	result_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, 1, ebufp);

	if(result_flistp != NULL)
	{
		while ((event_map_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, PIN_FLD_EVENT_MAP,&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			permitted = PIN_FLIST_FLD_GET(event_map_flistp, PIN_FLD_PERMITTED, 1, ebufp);	
			if(flag && *flag == 1)
			{
				if(!strcmp(permitted, "/service/telco/broadband"))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," for BB service");
					break;
				}
			}
			else if(!strcmp(permitted, "/account"))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," This event_map we need to check ");
				break;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Nothing to check ");
			}
		}
		while ((events_flistp = PIN_FLIST_ELEM_GET_NEXT(event_map_flistp, PIN_FLD_EVENTS,&elemid1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			config_event_type = PIN_FLIST_FLD_GET(events_flistp,PIN_FLD_EVENT_TYPE,1,ebufp);

			if(!strcmp(config_event_type,event_type))
			{

				count = PIN_FLIST_FLD_GET(events_flistp, PIN_FLD_COUNT, 0, ebufp);
				unit  = PIN_FLIST_FLD_GET(events_flistp, PIN_FLD_UNIT, 0, ebufp);
				if(flag != NULL && *flag == 1)
				{
					PIN_FLIST_FLD_SET(out_flistp , PIN_FLD_COUNT , count , ebufp);
					PIN_FLIST_FLD_SET(out_flistp , PIN_FLD_UNIT , unit , ebufp);
					goto MATCH;

				}
				if((count) && (*count > 0) && (*unit == 1))
				{
					timeeff->tm_mday = timeeff->tm_mday + *count;

					if(charged_to_t)
					{
						*charged_to_t = mktime (timeeff);
					}
					else
					{
						*purchase_end_t = mktime (timeeff);
					}

				}
				else {
					timeeff->tm_mon = timeeff->tm_mon + *count;
					if(charged_to_t)
					{
						*charged_to_t = mktime (timeeff);
					}
					else
					{
						*purchase_end_t = mktime (timeeff);
					}


				}
				PIN_FLIST_FLD_SET(out_flistp , PIN_FLD_COUNT , count , ebufp);
				if(charged_to_t)
					PIN_FLIST_FLD_SET(out_flistp , PIN_FLD_CHARGED_TO_T , charged_to_t , ebufp);
				if(purchase_end_t)
					PIN_FLIST_FLD_SET(out_flistp , PIN_FLD_PURCHASE_END_T ,purchase_end_t , ebufp);
				break;
			}
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " No usage info ");
	}
MATCH:
	*ret_flistp = PIN_FLIST_COPY(out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " return_flistp from search is ",*ret_flistp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);        
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&out_flistp, NULL); 
	return;
}

static void
fm_mso_bb_add_plan(
		cm_nap_connection_t	*connp,
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flist,
		pin_flist_t             **bb_ret_flistp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t		*dealinfo_flistp = NULL;
	pin_flist_t		*deals_flistp = NULL;
	pin_flist_t		*products_flistp = NULL;
	pin_flist_t		*statuschange_inflistp = NULL;
	pin_flist_t		*statuschange_outflistp = NULL;
	pin_flist_t		*read_flistp = NULL;
	pin_flist_t		*read_res_flistp = NULL;
	pin_flist_t		*status_flistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*return_flistp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*prov_res_flistp = NULL;
	pin_flist_t		*plan_in_flistp = NULL;
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*pur_return_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*mso_pp_flistp = NULL;
	pin_flist_t		*wrt_flds_in_flistp = NULL;
	pin_flist_t		*wrt_flds_out_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	pin_flist_t		*stpk_flistp = NULL;
	pin_flist_t		*prd_flistp = NULL;
	pin_flist_t		*ri_flistp = NULL;
	pin_flist_t		*ro_flistp = NULL;

	poid_t			*service_obj = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*mso_pp_obj = NULL;
	poid_t			*mso_plan_obj = NULL;
	poid_t			*account_obj = NULL;
	poid_t			*product_obj = NULL;
	poid_t			*offering_obj = NULL;
	poid_t			*subs_plan_obj = NULL;
	poid_t			*mso_bp_obj = NULL;
	poid_t			*r_prod_obj = NULL;
	poid_t			*r_offer_obj = NULL;

	char			msg[512] ;
	char			tmp_buf[512]="";
	char			*tmp_str = NULL ;
	char			*prov_tag = NULL ;
	char            *prov_tag1 = NULL; 
	char			*agreement_no = NULL;
	char			*search_template = "select X from /mso_customer_credential where F1 = V1 ";

	int			ret_val = 0;
	int			res_id = 0;
	int			elem_id = 0;
	int			elem_id2 = 0;
	int			elem_id3 = 0;
	int			selem_id = 0;
	int			prty = 0;
	int			prov_result = 0;
	int			notif_result = 0;
	int			*inp_flag = NULL;
	int			*ret_status = NULL;
	int32			*tech = NULL ;
	int32			technology = 0;
	int32			*status = NULL;
	int32			*prov_status = NULL;
	int			plan_count = 0;
	int			prov_counter = 0;
	int32                  *indicator = NULL;
	int32                  *is_tal;
	int32			db = 0;
	int32                  *customer_type = NULL;
	int32			srch_flag = 512;
	int			set_err = 0;
	int32			add_plan_success = 0;
	int32			add_plan_failure = 1;
	int			mod_prty = 0;
	int32			*p_status = NULL;
	//int			BB_ADD_MB_START = 900;
	//int			BB_ADD_MB_END = 950;
	int			ADD_MB_DATA_OR_FUP_FLAG = 0;
	int			ADD_MB_DATA = 0;
	int			FUP_FLAG =0;
	int			HW_PLAN_FLAG = 0;
	int			SUBS_PLAN_FLAG = 1; // Set it to 1. If HW or MISC plan then set it to 0.
	int			subs_type = 1;
	int			price_calc_flag = 0;
	int32			fup_status = BEF_FUP;
	int32			prod_count = -1;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie2 = NULL;
	pin_cookie_t		cookie3 = NULL;
	pin_cookie_t		scookie = NULL;
	pin_cookie_t		res_cookie = NULL;
	pin_decimal_t           *priority = NULL;
	void			*vp = NULL;

	int			is_device_validated = PIN_BOOLEAN_FALSE;
	int			*device_plan_type_indicatorp = NULL;
	int			dvc_srch_type = MSO_FLAG_SRCH_BY_ID;	
	int			conn_type = 0;
	int			device_type=0;
	char			*device_idp = NULL;
	char			*d_strp = NULL;
	pin_flist_t		*d_flistp = NULL;
	pin_flist_t		*dr_flistp = NULL;

	//Added for AMC
	pin_flist_t	*amc_flistp = NULL;
	pin_flist_t	*amc_out_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*dev_ret_flistp = NULL;
	int32		amc_flag = AMC_ON_POSTPAID_HW_PURCHASE;
	int32		count = 0;
	int32		mode = 0;
	char		*event_type = NULL;
	int32           *prov_ret_status = NULL;
	int32		*fup_stat = NULL;
	int32		wgn_plan = 0;
	time_t		*start_t = NULL;
	time_t		*end_t = NULL;

	pin_decimal_t		*discount = NULL;
	pin_decimal_t		*percent = NULL;
	
	// Added for Additional Topup
	time_t 			*grant_valid_from_date = NULL;
	time_t 			*grant_valid_to_date = NULL;
	pin_decimal_t		*grant_amount = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return ;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan :", in_flist);
	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	account_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp );	
	//indicator = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_INDICATOR, 1, ebufp );	
	inp_flag = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_FLAGS, 1, ebufp );	
	//tech = PIN_FLIST_FLD_GET(in_flist, MSO_FLD_TECHNOLOGY, 1, ebufp);

	/* Fetch the value of CALC ONLY flag*/
	vp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_MODE, 1, ebufp );
	if(vp && *(int32 *)vp == 1) {
		price_calc_flag = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Price Calc Flag (PIN_FLD_MODE) is 1");
	}

	if ( !service_obj || !account_obj || !inp_flag || (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) != 0 )
	{
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ), ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51413", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Incorrect POID value passed in input", ebufp);
		*bb_ret_flistp = ret_flistp;
		return ;
	}

	read_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, service_obj, ebufp);
	PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_STATUS, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(read_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(read_flistp, MSO_FLD_BB_INFO, 0, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_TECHNOLOGY, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_FUP_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_ISTAL_FLAG, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan read service input list", read_flistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS for service", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - PCM_OP_READ_FLDS of service poid error", ebufp);

		*bb_ret_flistp = ret_flistp;
		return ;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan read service output flist", read_res_flistp);
	status = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_STATUS, 1, ebufp );
	args_flistp = PIN_FLIST_ELEM_GET(read_res_flistp, PIN_FLD_TELCO_FEATURES, 0, 1, ebufp );
	prov_status = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_STATUS, 1, ebufp );
	args_flistp = PIN_FLIST_SUBSTR_GET(read_res_flistp, MSO_FLD_BB_INFO, 1, ebufp );
	PIN_FLIST_FLD_COPY(args_flistp, MSO_FLD_AGREEMENT_NO, in_flist, MSO_FLD_AGREEMENT_NO, ebufp);
	indicator = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_INDICATOR, 1, ebufp );
	fup_stat = PIN_FLIST_FLD_TAKE(args_flistp, MSO_FLD_FUP_STATUS, 1, ebufp );
	if (indicator) conn_type = *(int *)indicator;
	tech = PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp);
	technology = *(int32 *)tech;
	is_tal =PIN_FLIST_FLD_GET(args_flistp, MSO_FLD_ISTAL_FLAG, 1, ebufp);

        if ( PIN_FLIST_FLD_GET(in_flist, PIN_FLD_BOOLEAN, 1, ebufp) != NULL )
        {
                mode = *(int *) PIN_FLIST_FLD_GET(in_flist, PIN_FLD_BOOLEAN, 1, ebufp);
        }


	if ( mode==0 && (*(int32 *)status != MSO_ACTIVE_STATUS || *(int32 *)prov_status != MSO_PROV_ACTIVE ))
	{
		PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51616", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Service or Provisioning status for this service is not active - Cannot add plan", ebufp);
		*bb_ret_flistp = ret_flistp;
		return ;
	}


	plan_count = PIN_FLIST_ELEM_COUNT(in_flist, PIN_FLD_PLAN_LISTS, ebufp);			
	if ( plan_count == 0 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No plans to purchase in input flist");
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51628", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "No plans to purchase in input flist", ebufp);
		*bb_ret_flistp = ret_flistp;
		return ;
	}
	//PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);

	db = PIN_POID_GET_DB(service_obj);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
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
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51629", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching  customer type for this account", ebufp);
		*bb_ret_flistp = ret_flistp;
		return ;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "customer type search output flist", srch_out_flistp);	
	results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	customer_type = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CUSTOMER_TYPE, 1, ebufp);	
	if ( !customer_type || customer_type == (int32 *)NULL )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Customer type is not updated for this account", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51630", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Customer type is not updated for this account", ebufp);
		*bb_ret_flistp = ret_flistp;
		return ;
	}
	// Convert PLAN_LIST array to PLAN array.
	cookie = NULL;
	while ( (plan_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN_LISTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_PLAN_LISTS, elem_id, in_flist, PIN_FLD_PLAN, elem_id, ebufp);
		PIN_FLIST_ELEM_DROP(in_flist, PIN_FLD_PLAN_LISTS, elem_id, ebufp);
		cookie = NULL;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist after copying PLAN array", in_flist);

	// add_prod_info will add product details in the plan array.
	fm_mso_add_prod_info(connp, ctxp, in_flist, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Errro after returing from fm_mso_add_prod_info", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51630", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Either wrong discount given or Error in fm_mso_add_prod_info", ebufp);
		*bb_ret_flistp = ret_flistp;
		return ;
	}	
	elem_id = 0;
	cookie = NULL;
	while ( (plan_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		plan_obj = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		if ( !plan_obj )
		{
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ), ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51413", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Incorrect POID value passed in input", ebufp);
			*bb_ret_flistp = ret_flistp;
			return ;
		}
		prov_tag = NULL;
		prov_counter = 0;
		elem_id2 = 0;
		cookie2 = NULL;
		while ((products_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_PRODUCTS, &elem_id2, 1, &cookie2, ebufp )) != NULL)
		{
			product_obj = PIN_FLIST_FLD_GET(products_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
			if ( !product_obj )
			{
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ), ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51413", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Incorrect POID value passed in input", ebufp);
				*bb_ret_flistp = ret_flistp;
				return ;
			}
			read_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, product_obj, ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_PRIORITY, (pin_decimal_t *)NULL, ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_PROVISIONING_TAG, (pin_decimal_t *)NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan read product input list", read_flistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_res_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS for product", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51432", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - PCM_OP_READ_FLDS of product poid error", ebufp);

				*bb_ret_flistp = ret_flistp;
				return ;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan read product output flist", read_res_flistp);

			prov_tag = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
			priority = PIN_FLIST_FLD_GET(read_res_flistp, PIN_FLD_PRIORITY, 1, ebufp );
			if ( !priority || priority == (pin_decimal_t *) NULL )
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS for product", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51433", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - PCM_OP_READ_FLDS of product poid error", ebufp);

				*bb_ret_flistp = ret_flistp;
				return ;
			}
			tmp_str = pbo_decimal_to_str(priority, ebufp);
			prty = atoi(tmp_str);
			/**added condition to change the plan_type for installation plan during add plan***/
			if(prty >= BB_INST_PRI_START && prty <= BB_INST_PRI_END)
			{
				subs_type = 2;
				SUBS_PLAN_FLAG = 0;
				/************************************************************************************
				 * Perform all STPK validations.
				 * 1. Check if there is any cancelled STPK and provide Refund.
				 * 2. Dont allow if there is active STPK already exists and should have been cancelled
				 * to add new plan.
				 *************************************************************************************/


				fm_mso_get_purchased_install_products(ctxp, account_obj, service_obj, &stpk_flistp, ebufp);
				prod_count = PIN_FLIST_ELEM_COUNT(stpk_flistp, PIN_FLD_RESULTS, ebufp);
				while((prd_flistp = PIN_FLIST_ELEM_GET_NEXT(stpk_flistp, PIN_FLD_RESULTS, &selem_id, 1, &scookie, ebufp)) != NULL)
				{
					p_status = PIN_FLIST_FLD_GET(prd_flistp, PIN_FLD_STATUS, 1, ebufp);
					if (p_status && *p_status == 1 && mode==0){
						//is_active_stpk = 1;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active STPK/Installation Plan already exists");
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&stpk_flistp, NULL);
						ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51439", ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
								"ADD PLAN - Only one Active STPK/Installation allowed, to add new, Cancel existing and add new one", ebufp);

						*bb_ret_flistp = ret_flistp;
						return ;
					}
					else if (p_status && *p_status == 3){ 
						//Since results sorted by status and purchase_end_t, last element will have latest cancellation STPK
						if ( selem_id == prod_count-1){
							r_offer_obj = PIN_FLIST_FLD_GET(prd_flistp, PIN_FLD_POID, 1, ebufp );	
							r_prod_obj = PIN_FLIST_FLD_GET(prd_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp );
							start_t = PIN_FLIST_FLD_GET(prd_flistp, PIN_FLD_PURCHASE_START_T, 1, ebufp );
							end_t = PIN_FLIST_FLD_GET(prd_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp );
							percent = PIN_FLIST_FLD_GET(prd_flistp, PIN_FLD_PURCHASE_DISCOUNT, 1, ebufp );
							discount = PIN_FLIST_FLD_GET(prd_flistp, PIN_FLD_PURCHASE_DISC_AMT, 1, ebufp );

						}
						else{
							continue; //Nothing to do
						}
						//fm_mso_create_refund_for_cancel_plan(ctxp, in_flistp, );	
					}
				}
				mode=0;
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "OFFER OBJ FINAL", r_offer_obj);
				if (r_offer_obj && r_prod_obj){
					ri_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_POID, account_obj, ebufp);
					PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
					PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_OFFERING_OBJ, r_offer_obj, ebufp);
					PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_PRODUCT_OBJ, r_prod_obj, ebufp);
					PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_PROGRAM_NAME, "STPK_REFUND", ebufp);
					PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_START_T, start_t, ebufp);
					PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_END_T, end_t, ebufp);
					PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_EVENT_TYPE, "/event/billing/product/fee/purchase", ebufp);
					PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_PERCENT, percent, ebufp); // If any discount given while purchase
					PIN_FLIST_FLD_SET(ri_flistp, PIN_FLD_DISCOUNT, discount, ebufp); // If any discount given while purchase
					fm_mso_create_refund_for_cancel_plan(ctxp, ri_flistp, &ro_flistp, ebufp );
					PIN_FLIST_DESTROY_EX(&ri_flistp, NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in creating refund ");
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
						ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51440", ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
								"ACCOUNT - Add Plan - Error in creating Refund for Cancelled STPK", ebufp);

						*bb_ret_flistp = ret_flistp;
						return ;
					}
					PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
				}

			}
			/* Check if the plan is hardware plan */
			if((BB_HW_DEPOSIT_RANGE_START <= prty && BB_HW_DEPOSIT_RANGE_END >= prty)
					|| (BB_HW_OUTRIGHT_RANGE_START <= prty && BB_HW_OUTRIGHT_RANGE_END >= prty)
					|| (BB_HW_RENTAL_RANGE_START <= prty && BB_HW_RENTAL_RANGE_END >= prty)
					//|| prty == BB_MISC_ADD_IP)
				|| prty == BB_HW_ADDITIONAL_IP_RENT_PRIO
					|| prty == BB_HW_ADDITIONAL_IP_OR_PRIO)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"This is Hardware plan.");
						HW_PLAN_FLAG = 1;
						SUBS_PLAN_FLAG = 0;
						subs_type = 2;
						//Pawan: Added below condition to find the device_type flag
						if((BB_HW_DEPOSIT_RANGE_START <= prty && BB_HW_DEPOSIT_RANGE_END >= prty)
								|| (BB_HW_RENTAL_RANGE_START <= prty && BB_HW_RENTAL_RANGE_END >= prty))
						{
							device_type=RENTAL_DEPOSIT_PLAN;
						}
						else if(BB_HW_OUTRIGHT_RANGE_START <= prty && BB_HW_OUTRIGHT_RANGE_END >= prty)
						{
							device_type=OUTRIGHT_PLAN;
						}
						/* Device is enough  to be validated with only one of the products in plans array */	
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"in_flist here:",in_flist);
						device_plan_type_indicatorp = PIN_FLIST_FLD_GET(in_flist,PIN_FLD_INDICATOR,1,ebufp);

						// Added AND in below condition to avoid validation for static IP product.
						if(is_device_validated == PIN_BOOLEAN_FALSE && 
								prty != BB_HW_ADDITIONAL_IP_OR_PRIO && prty != BB_HW_ADDITIONAL_IP_RENT_PRIO )	
						{
							/******** Pavan Bellala - 31072015 ***********************************
							  Defect ID : 1294 : Validation of device type is required for add 
							  hardware plans
Solution : New input field PIN_FLD_INDICATOR added to main input flist 
which gets value from OAP
Allowed values : 1=MODEM; 2=CABLE ROUTER; 3=WIFI ROUTER
4=NAT;   5=STATIC IP;    6=FRAME IP;
The device poid_type is checked with this value to validate
							 ************ Pavan Bellala - 31072015 ********************************/
							if(device_plan_type_indicatorp == NULL || (!device_plan_type_indicatorp))
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"The plan type indicator is mandatory for HW plans");
								set_err = 2;
								is_device_validated = PIN_BOOLEAN_TRUE;
								goto NOMATCH;	

							} else {

								device_idp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_DEVICE_ID,1,ebufp);
								if(!device_idp || strcmp(device_idp,"")==0)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Device id is NULL");
									set_err = 3;
									is_device_validated = PIN_BOOLEAN_TRUE;
									goto NOMATCH;
								} else {
									d_flistp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_POID,d_flistp,PIN_FLD_POID,ebufp);
									PIN_FLIST_FLD_SET(d_flistp,PIN_FLD_DEVICE_ID,device_idp,ebufp);
									PIN_FLIST_FLD_SET(d_flistp,PIN_FLD_SEARCH_TYPE,&dvc_srch_type,ebufp);
									fm_mso_get_device_info(ctxp,d_flistp,&dr_flistp,ebufp);
									PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"device_info return",dr_flistp);
									PIN_FLIST_DESTROY_EX(&d_flistp,NULL);
									d_strp = (char *)PIN_POID_GET_TYPE((poid_t*)PIN_FLIST_FLD_GET(dr_flistp,PIN_FLD_POID,0,ebufp));
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,d_strp);
									if(d_strp != NULL)
									{
										if((strstr(d_strp,MSO_OBJ_TYPE_DEVICE_MODEM) && 
													MSO_DVC_TYPE_MODEM!= *(int*)device_plan_type_indicatorp)||
												(strstr(d_strp,MSO_OBJ_TYPE_DEVICE_ROUTER_CABLE) && 
												 MSO_DVC_TYPE_ROUTER_CABLE!=*(int*)device_plan_type_indicatorp )||
												(strstr(d_strp,MSO_OBJ_TYPE_DEVICE_ROUTER_WIFI) && 
												 MSO_DVC_TYPE_ROUTER_WIFI!= *(int*)device_plan_type_indicatorp )||
												(strstr(d_strp,MSO_OBJ_TYPE_DEVICE_NAT) && 
												 MSO_DVC_TYPE_NAT!= *(int*)device_plan_type_indicatorp)||
												(strstr(d_strp,MSO_OBJ_TYPE_DEVICE_IP_STATIC) && 
												 MSO_DVC_TYPE_IP_STATIC!= *(int*)device_plan_type_indicatorp )||
												(strstr(d_strp,MSO_OBJ_TYPE_DEVICE_IP_FRAMED) && 
												 MSO_DVC_TYPE_IP_FRAMED!= *(int*)device_plan_type_indicatorp))
										{
											PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Device type mismatch");
											set_err = 4;
											is_device_validated = PIN_BOOLEAN_TRUE;
											goto NOMATCH;
										}
									}
								}		

							}

						}

					}

			/* Check if the plan is Misc plan */
			if(prty == BB_MISC_CABLE_SHIFT)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"This is Hardware plan.");
				SUBS_PLAN_FLAG = 0;
				subs_type = 3;
			}

			/*Pawan:12-12-2014: If add plan is for Additional MB/GB 
			  then dont apply any validation since these plans are common
			  for all customers. */
			mod_prty = prty%1000;
			if((BB_ADD_MB_START <= mod_prty && BB_ADD_MB_END >= mod_prty)||
					(BB_FUP_START <= mod_prty && BB_FUP_START >= mod_prty))
			{
				if(BB_FUP_START <= mod_prty && BB_FUP_START >= mod_prty)
				{
					if(fup_stat && *fup_stat == 0)
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
								"Fup topup is being added to non-fup customer", ebufp);
						wgn_plan=1;
					}
				}
				else if(BB_ADD_MB_START <= mod_prty && BB_ADD_MB_END >= mod_prty)
				{
					if(fup_stat && *fup_stat != 0)
					{
						wgn_plan=1;
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
								"ADD MB plan is being added to fup customer", ebufp);
					}
				}
				if(wgn_plan == 1)
				{
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ), ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Wrong plan is added to customer", ebufp);
					*bb_ret_flistp = ret_flistp;
					return ;
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"This is Additional MB plan or FUP topup.");
				ADD_MB_DATA_OR_FUP_FLAG = 1;
				fm_mso_get_curr_mso_purplan(ctxp, account_obj, &mso_pp_flistp, ebufp);
				if ( !mso_pp_flistp || PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_RESET(ebufp);
					ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp ), ebufp );
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51421", ebufp);
					PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "MSO purchased plan not found", ebufp);
					*bb_ret_flistp = ret_flistp;
					return ;
				}
				else
				{
					prov_tag1 = PIN_FLIST_FLD_TAKE(mso_pp_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);

					subs_plan_obj = PIN_FLIST_FLD_TAKE(mso_pp_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
					mso_bp_obj = PIN_FLIST_FLD_TAKE(mso_pp_flistp, PIN_FLD_POID, 0, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Base plan prov TAG");
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, prov_tag1);
					offering_obj = PIN_FLIST_FLD_TAKE(mso_pp_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);
					PIN_FLIST_DESTROY_EX(&mso_pp_flistp, NULL);
				}
				// For additional MB/GB plans and FUP topup plans other 
				// validation are not required. GOTO NOMATCH.
				/********************************************************
				  For lifecycle -Start
				 ********************************************************/
				sprintf(tmp_buf, "mod_prty = %d", mod_prty);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp_buf);

				sprintf(tmp_buf, "ADD_MB_DATA_OR_FUP_FLAG = %d", ADD_MB_DATA_OR_FUP_FLAG);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp_buf);

				sprintf(tmp_buf, "BB_ADD_MB_START = %d", BB_ADD_MB_START);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp_buf);

				sprintf(tmp_buf, "BB_ADD_MB_END = %d", BB_ADD_MB_END);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp_buf);

				sprintf(tmp_buf, "BB_FUP_START = %d", BB_FUP_START);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp_buf);

				sprintf(tmp_buf, "BB_FUP_END = %d", BB_FUP_END);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp_buf);


				if (ADD_MB_DATA_OR_FUP_FLAG == 1 &&
						(BB_ADD_MB_START <= mod_prty && BB_ADD_MB_END >= mod_prty)
				   )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ADD_MB_DATA = 1");
					ADD_MB_DATA = 1;
				}
				if ( ADD_MB_DATA_OR_FUP_FLAG == 1 &&
						(BB_FUP_START <= mod_prty && BB_FUP_END >= mod_prty)
				   )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"FUP_FLAG = 1");
					FUP_FLAG = 1;
				}
				/********************************************************
				  For lifecycle -Start
				 ********************************************************/

				goto NOMATCH;
			}


			/*if ( *(int32 *)inp_flag == 0 && ADD_MB_DATA_OR_FUP_FLAG && SUBS_PLAN_FLAG)
			  {
			  if ( *(int32 *)customer_type == ACCT_TYPE_CORP_SUBS_BB )
			  {	
			  if ( prty < BB_SUB_COR_PRI_START || prty > BB_SUB_COR_PRI_END )
			  {
			  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_CORP");
			  set_err = 1;
			  }
			  }
			  else if ( *(int32 *)customer_type == ACCT_TYPE_CYBER_CAFE_BB )
			  {
			  if ( prty < BB_SUB_CYB_PRI_START || prty > BB_SUB_RET_PRI_END )
			  {
			  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_CYB");
			  set_err = 1;
			  }
			  }
			  else if ( *(int32 *)customer_type == ACCT_TYPE_RETAIL_BB )
			  {
			  if ( prty < BB_SUB_RET_PRI_START || prty > BB_SUB_RET_PRI_END )
			  {
			  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_RET");
			  set_err = 1;
			  }
			  }
			  else if ( *(int32 *)customer_type == ACCT_TYPE_SME_SUBS_BB )
			  {
			  if ( prty < BB_SUB_SME_PRI_START || prty > BB_SUB_SME_PRI_END )
			  {
			  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Cust = BB_SME");
			  set_err = 1;
			  }
			  }
			// For additional MB/GB plans other validation are not required.
			// GOTO NOMATCH.
			goto NOMATCH;
			}	*/
			/* till here */

			if ( *(int32 *)inp_flag == 0 && SUBS_PLAN_FLAG)
			{
				if ( *(int32 *)customer_type == ACCT_TYPE_CORP_SUBS_BB )
				{	
					if ( prty < BB_COR_FUP_START || prty > BB_COR_FUP_END )
					{
						set_err = 1;
						goto NOMATCH;
					}
				}
				/*** Pavan Bellala *******************************
				  Commented as cyber cafe is removed
				  else if ( *(int32 *)customer_type == ACCT_TYPE_CYBER_CAFE_BB )
				  {
				  if ( prty < BB_CYB_FUP_START || prty > BB_CYB_FUP_END )
				  {
				  set_err = 1;
				  goto NOMATCH;
				  }
				  }
				 **************************************************/
				  else if ( *(int32 *)customer_type == ACCT_TYPE_RETAIL_BB )
				  {
					  if ( prty < BB_RET_FUP_START || prty > BB_RET_FUP_END )
					  {
						  set_err = 1;
						  goto NOMATCH;
					  }
				  }
				  else if ( *(int32 *)customer_type == ACCT_TYPE_SME_SUBS_BB )
				  {
					  if ( prty < BB_SME_FUP_START || prty > BB_SME_FUP_END )
					  {
						  set_err = 1;
						  goto NOMATCH;
					  }
				  }
				  /*** Added error data condition ****/
				  else if ( *(int32 *)customer_type == ACCT_TYPE_RESERVED_BB )
				  {
					  set_err = 1;
					  goto NOMATCH;
				  }
			}	

			if ( *(int32 *)inp_flag == 50 )
			{
				if ( *(int32 *)customer_type == ACCT_TYPE_CORP_SUBS_BB )
				{
					if ( *(int32 *)indicator == MSO_PREPAID )
					{
						if ( technology == MSO_DOCSIS_2 )
						{	
							if ( prty < BB_COR_PREPAID_DOCSIS2_START || prty > BB_COR_PREPAID_DOCSIS2_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						else if ( technology == MSO_DOCSIS_3 )
						{	
							if ( prty < BB_COR_PREPAID_DOCSIS3_START || prty > BB_COR_PREPAID_DOCSIS3_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						else if ( technology != MSO_DOCSIS_2 && technology != MSO_DOCSIS_3 )
						{	
							if ( prty < BB_COR_PREPAID_NORMAL_START || prty > BB_COR_PREPAID_NORMAL_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
					}
					//else if ( *(int32 *)indicator == MSO_POSTPAID )
					else if ( conn_type == MSO_POSTPAID )
					{
						if ( technology == MSO_DOCSIS_2 )
						{	
							if ( prty < BB_COR_POSTPAID_DOCSIS2_START || prty > BB_COR_POSTPAID_DOCSIS2_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						if ( technology == MSO_DOCSIS_3 )
						{	
							if ( prty < BB_COR_POSTPAID_DOCSIS3_START || prty > BB_COR_POSTPAID_DOCSIS3_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						if ( technology != MSO_DOCSIS_2 && technology != MSO_DOCSIS_3 )
						{	
							if ( prty < BB_COR_POSTPAID_NORMAL_START || prty > BB_COR_POSTPAID_NORMAL_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
					} 
				}
				/******* 
				  else if ( *(int32 *)customer_type == ACCT_TYPE_CYBER_CAFE_BB )
				  {
				//if ( *(int32 *)indicator == MSO_PREPAID )
				if ( conn_type == MSO_PREPAID )
				{
				if ( technology == MSO_DOCSIS_2 )
				{	
				if ( prty < BB_CYB_PREPAID_DOCSIS2_START || prty > BB_CYB_PREPAID_DOCSIS2_END )
				{
				set_err = 1;
				goto NOMATCH;
				}
				}
				else if ( technology == MSO_DOCSIS_3 )
				{	
				if ( prty < BB_CYB_PREPAID_DOCSIS3_START || prty > BB_CYB_PREPAID_DOCSIS3_END )
				{
				set_err = 1;
				goto NOMATCH;
				}
				}
				else if ( technology != MSO_DOCSIS_2 && technology != MSO_DOCSIS_3 )
				{	
				if ( prty < BB_CYB_PREPAID_NORMAL_START || prty > BB_CYB_PREPAID_NORMAL_END )
				{
				set_err = 1;
				goto NOMATCH;
				}
				}
				}
				//else if ( *(int32 *)indicator == MSO_POSTPAID )
				else if ( conn_type == MSO_POSTPAID )
				{
				if ( technology == MSO_DOCSIS_2 )
				{	
				if ( prty < BB_CYB_POSTPAID_DOCSIS2_START || prty > BB_CYB_POSTPAID_DOCSIS2_END )
				{
				set_err = 1;
				goto NOMATCH;
				}
				}
				else if ( technology == MSO_DOCSIS_3 )
				{	
				if ( prty < BB_CYB_POSTPAID_DOCSIS3_START || prty > BB_CYB_POSTPAID_DOCSIS3_END )
				{
				set_err = 1;
				goto NOMATCH;
				}
				}
				else if ( technology != MSO_DOCSIS_2 && technology != MSO_DOCSIS_3 )
				{	
				if ( prty < BB_CYB_POSTPAID_NORMAL_START || prty > BB_CYB_POSTPAID_NORMAL_END )
				{
				set_err = 1;
				goto NOMATCH;
				}
				}

				} 

				}
				 ******/
				else if ( *(int32 *)customer_type == ACCT_TYPE_RETAIL_BB )
				{
					//if ( *(int32 *)indicator == MSO_PREPAID )
					if ( conn_type  == MSO_PREPAID )
					{
						if ( technology == MSO_DOCSIS_2 )
						{	
							if ( prty < BB_RET_PREPAID_DOCSIS2_START || prty > BB_RET_PREPAID_DOCSIS2_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						else if ( technology == MSO_DOCSIS_3 )
						{	
							if ( prty < BB_RET_PREPAID_DOCSIS3_START || prty > BB_RET_PREPAID_DOCSIS3_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						else if ( technology != MSO_DOCSIS_2 && technology != MSO_DOCSIS_3 )
						{	
							if ( prty < BB_RET_PREPAID_NORMAL_START || prty > BB_RET_PREPAID_NORMAL_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
					}
					//else if ( *(int32 *)indicator == MSO_POSTPAID )
					else if ( conn_type == MSO_POSTPAID )
					{
						if ( technology == MSO_DOCSIS_2 )
						{	
							if ( prty < BB_RET_POSTPAID_DOCSIS2_START || prty > BB_RET_POSTPAID_DOCSIS2_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						if ( technology == MSO_DOCSIS_3 )
						{	
							if ( prty < BB_RET_POSTPAID_DOCSIS3_START || prty > BB_RET_POSTPAID_DOCSIS3_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						if ( technology != MSO_DOCSIS_2 && technology != MSO_DOCSIS_3 )
						{	
							if ( prty < BB_RET_POSTPAID_NORMAL_START || prty > BB_RET_POSTPAID_NORMAL_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}

					} 
				}
				else if ( *(int32 *)customer_type == ACCT_TYPE_SME_SUBS_BB )
				{
					//if ( *(int32 *)indicator == MSO_PREPAID )
					if ( conn_type == MSO_PREPAID )
					{
						if ( technology == MSO_DOCSIS_2 )
						{	
							if ( prty < BB_SME_PREPAID_DOCSIS2_START || prty > BB_SME_PREPAID_DOCSIS2_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						if ( technology == MSO_DOCSIS_3 )
						{	
							if ( prty < BB_SME_PREPAID_DOCSIS3_START || prty > BB_SME_PREPAID_DOCSIS3_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						if ( technology != MSO_DOCSIS_2 && technology != MSO_DOCSIS_3 )
						{	
							if ( prty < BB_SME_PREPAID_NORMAL_START || prty > BB_SME_PREPAID_NORMAL_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
					}
					//if ( *(int32 *)indicator == MSO_POSTPAID )
					if ( conn_type == MSO_POSTPAID )
					{
						if ( technology == MSO_DOCSIS_2 )
						{	
							if ( prty < BB_SME_POSTPAID_DOCSIS2_START || prty > BB_SME_POSTPAID_DOCSIS2_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						if ( technology == MSO_DOCSIS_3 )
						{	
							if ( prty < BB_SME_POSTPAID_DOCSIS3_START || prty > BB_SME_POSTPAID_DOCSIS3_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
						if ( technology != MSO_DOCSIS_2 && technology != MSO_DOCSIS_3 )
						{	
							if ( prty < BB_SME_POSTPAID_NORMAL_START || prty > BB_SME_POSTPAID_NORMAL_END )
							{
								set_err = 1;
								goto NOMATCH;
							}
						}
					} 
				}
			}
NOMATCH:
			if ( set_err == 1 )
			{ 
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"Error input flist",in_flist);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan type does not match with customer type", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&stpk_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51631", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Plan type is not matching with Customer type", ebufp);
				*bb_ret_flistp = ret_flistp;
				return ;
			} else if( set_err == 2 ) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside erro type 2");
				if(PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "plan type indicator is mandatory for HW plans",ebufp);
					PIN_ERRBUF_RESET(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&stpk_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51631", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Plan type indicator is mandatory for HW plans", ebufp);
				*bb_ret_flistp = ret_flistp;
				return ;
			} else if( set_err == 3 ){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside erro type 3");
				if(PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Device id is NULL",ebufp);
					PIN_ERRBUF_RESET(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&stpk_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51631", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid : Device id is NULL", ebufp);
				*bb_ret_flistp = ret_flistp;
				return ;
			} else if( set_err == 4 ){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside erro type 4");

				if(PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Device type mismatch",ebufp);
					PIN_ERRBUF_RESET(ebufp);
				}
				PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&stpk_flistp, NULL);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51631", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid : Device type mismatch", ebufp);
				*bb_ret_flistp = ret_flistp;
				return ;
			}

			if (  prov_tag && strlen(prov_tag)>0 )
			{
				prov_counter = 1;
			}
			//PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
		}
		if (PIN_ERRBUF_IS_ERR(ebufp) )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in validating plans", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&stpk_flistp, NULL);
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51631", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in validating plans", ebufp);
			*bb_ret_flistp = ret_flistp;
			return ;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Preparing Plan flist for calling purchase bb plans function");
		plan_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(plan_in_flistp, PIN_FLD_ACCOUNT_OBJ, account_obj, ebufp);
		PIN_FLIST_FLD_SET(plan_in_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, plan_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_MODE, plan_in_flistp, PIN_FLD_MODE, ebufp);
		PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_PLAN, elem_id, plan_in_flistp, PIN_FLD_PLAN, elem_id, ebufp);
		PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_PLAN_TYPE, plan_in_flistp, MSO_FLD_PLAN_TYPE, ebufp);
		args_flistp = PIN_FLIST_ELEM_GET(plan_in_flistp, PIN_FLD_PLAN, elem_id, 0, ebufp);
		//Added plan is subscription plan.
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TYPE, &subs_type, ebufp);
		if(ADD_MB_DATA_OR_FUP_FLAG == 1)
		{
			//For Add MB and FUP topup, set the end date of product
			//ret_val = fm_mso_set_prod_end_date( ctxp, offering_obj, indicator, plan_in_flistp, ebufp);
			sprintf(tmp_buf, "Indicator in input flist = %d",conn_type);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buf);
			ret_val = fm_mso_set_prod_end_date( ctxp, offering_obj, &conn_type, plan_in_flistp, ebufp);
			if(ret_val == 0) {
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51641", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Setting End date", ebufp);
				*bb_ret_flistp = ret_flistp;
				return ;			
			}
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan calling purchase plans.", plan_in_flistp);
		fm_mso_purchase_bb_plans ( connp, plan_in_flistp, prov_tag, 1, 0, &return_flistp, ebufp);	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_bb_plans return flist.", return_flistp);
		//fm_mso_purchase_bb_plans ( ctxp, plan_in_flistp, &return_flistp, ebufp);	

		if ( return_flistp && return_flistp != (pin_flist_t *)NULL )
		{
			ret_status = PIN_FLIST_FLD_GET(return_flistp, PIN_FLD_STATUS, 1, ebufp);
			//mso_pp_flistp = PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_PRODUCTS, 0, 1, ebufp);
			mso_pp_obj = PIN_FLIST_FLD_GET(return_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 1, ebufp);
		}
		PIN_FLIST_ELEM_GET(return_flistp, PIN_FLD_PRODUCTS, 0, 1, ebufp);
		if ( PIN_ERRBUF_IS_ERR(ebufp) || ((ret_status) && *(int32 *)ret_status == 1) )
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in purchasing plan ", ebufp);
			PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&stpk_flistp, NULL);
			*bb_ret_flistp = return_flistp;
			return ;
		}
		//if ( HW_PLAN_FLAG == 1 && mod_prty!=BB_MISC_ADD_IP)
		if ( HW_PLAN_FLAG == 1 && mod_prty!=BB_HW_ADDITIONAL_IP_RENT_PRIO && mod_prty!= BB_HW_ADDITIONAL_IP_OR_PRIO)
		{
			fm_mso_update_device_plan_obj(ctxp, in_flist, mso_pp_obj, device_type, &dev_ret_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_device_plan_obj return flist", dev_ret_flistp);
			ret_status = PIN_FLIST_FLD_GET(dev_ret_flistp, PIN_FLD_STATUS, 1, ebufp);
			if ( PIN_ERRBUF_IS_ERR(ebufp) || ((ret_status) && *(int32 *)ret_status == 1) )
			{
				*bb_ret_flistp = dev_ret_flistp;
				return;
			}
		}

		//Call AMC opcode if Add HW plan is Rental plan.
		if(HW_PLAN_FLAG == 1 && device_type==RENTAL_DEPOSIT_PLAN && mod_prty!=BB_HW_ADDITIONAL_IP_RENT_PRIO && conn_type == MSO_POSTPAID )
		{
			if(price_calc_flag == 1)
				mode = 1;
			amc_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_POID, account_obj, ebufp);
			PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_USERID, amc_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_FLAGS, &amc_flag, ebufp);
			PIN_FLIST_FLD_SET(amc_flistp, PIN_FLD_MODE, &mode, ebufp);

			// Below function will set the results array in input flist
			// with subscription prod details which is required in AMC opcode
			fm_mso_cust_bb_hw_amc_get_cycle_details(ctxp, amc_flistp, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC input", amc_flistp);
			PCM_OP(ctxp, MSO_OP_CUST_BB_HW_AMC, 0, amc_flistp, &amc_out_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51781", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in associating AMC plan", ebufp);
				*bb_ret_flistp = ret_flistp;
				return ;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_BB_HW_AMC output", amc_out_flistp);
			// If Add plan is called by price calculator then copy the results array 
			// from AMC return flist to return flist of function.
			if(mode==1)
			{
				args_flistp = PIN_FLIST_ELEM_GET(return_flistp,PIN_FLD_DATA_ARRAY,0,1,ebufp);
				if(args_flistp && args_flistp != NULL)
				{
					count = PIN_FLIST_ELEM_COUNT(args_flistp, PIN_FLD_RESULTS, ebufp);
					cookie3 = NULL;
					while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(amc_out_flistp, PIN_FLD_RESULTS, 
									&elem_id3, 1, &cookie3, ebufp)) != NULL)
					{
						event_type = (char *) PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_POID, 1, ebufp ));
						if(event_type && strstr(event_type, "mso_hw_amc"))
						{
							count++;
							PIN_FLIST_ELEM_SET(args_flistp, tmp_flistp, PIN_FLD_RESULTS, count, ebufp);
						}
					}
				}
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_change_amc_plan return flist", return_flistp);
		}

		// Update prov tag in mso_purchased_plan of AddMB/FUP topup.
		if(ADD_MB_DATA_OR_FUP_FLAG == 1)
		{
			//If Add MB GB or FUP Topup then update mso_purchased_plan
			// with prov_tag of Base plan
			srch_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(return_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, srch_flistp, PIN_FLD_POID, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, prov_tag1, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Write fields input list", srch_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, srch_flistp, &srch_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Write fields output list", srch_out_flistp);
			prov_counter = 1;
		}

		if (FUP_FLAG == 1)
		{
			// Update service object to set the FUP_STATUS to AFT_FUP.
			wrt_flds_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(wrt_flds_in_flistp, PIN_FLD_POID, service_obj, ebufp);
			bb_info_flistp = PIN_FLIST_ELEM_ADD(wrt_flds_in_flistp, MSO_FLD_BB_INFO, 0, ebufp);
			PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_FUP_STATUS, &fup_status, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist for updating FUP status", wrt_flds_in_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, wrt_flds_in_flistp, &wrt_flds_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist from updating FUP status", wrt_flds_out_flistp);
		}


		if (  prov_counter == 1 )
		{
			res_id = 0;
			res_cookie = NULL;
			prov_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_POID, account_obj, ebufp);	
			PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);	
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_PROGRAM_NAME, prov_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);	
			PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,prov_in_flistp,PIN_FLD_USERID,ebufp);
			PIN_FLIST_ELEM_COPY(return_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, prov_in_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, ebufp);
			/*while ((pur_return_flistp = PIN_FLIST_ELEM_GET_NEXT(return_flistp, PIN_FLD_RESULTS, &res_id, 1, &res_cookie, ebufp )) != NULL)
			  {
			  mso_pp_obj = PIN_FLIST_FLD_GET(pur_return_flistp, PIN_FLD_POID, 1, ebufp);
			  mso_plan_obj = PIN_FLIST_FLD_GET(pur_return_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
			  prov_res_flistp = PIN_FLIST_ELEM_ADD( prov_in_flistp, PIN_FLD_RESULTS, res_id, ebufp);
			  PIN_FLIST_FLD_SET(prov_res_flistp, PIN_FLD_POID, mso_pp_obj, ebufp);	
			  PIN_FLIST_FLD_SET(prov_res_flistp, PIN_FLD_PLAN_OBJ, mso_plan_obj, ebufp);	
			  }*/
			if(mso_pp_obj && mso_pp_obj != NULL) {
				PIN_FLIST_FLD_SET(prov_in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_pp_obj, ebufp);
			}

			/*RAVI  setting  the priority of the product into provision input flist*/
			PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_PRIORITY, priority, ebufp);

			fm_mso_cust_call_prov( ctxp, prov_in_flistp, &ret_flistp, ebufp); 	
			PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL); 
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51434", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);

				*bb_ret_flistp = PIN_FLIST_COPY(ret_flistp,ebufp);
				goto CLEANUP;
			}
			if(ret_flistp != NULL)
			{
				prov_ret_status = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 1, ebufp );
				if(prov_ret_status && *prov_ret_status == 1)
				{
					*bb_ret_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
					goto CLEANUP;
				}
			}	
			/*  if (PIN_ERRBUF_IS_ERR(ebufp) || prov_result == 10 )
			    {
			    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
			    PIN_ERRBUF_RESET(ebufp);
			    PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
			    PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
			    PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			    PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
			    ret_flistp = PIN_FLIST_CREATE(ebufp);
			    PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			    PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			    PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51431", ebufp);
			    PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - MSO_OP_PROV_CREATE_ACTION error", ebufp);

			 *bb_ret_flistp = ret_flistp;
			 return ;
			 }*/
			/*notif_result = fm_mso_cust_call_notif( ctxp, prov_in_flistp, ebufp);

			  if (PIN_ERRBUF_IS_ERR(ebufp) || prov_result == 10 )
			  {
			  PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
			  PIN_ERRBUF_RESET(ebufp);
			  PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
			  PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
			  PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			  PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
			  PIN_FLIST_DESTROY_EX(&stpk_flistp, NULL);
			  ret_flistp = PIN_FLIST_CREATE(ebufp);
			  PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
			  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51434", ebufp);
			  PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "ACCOUNT - Add Plan - MSO_OP_NTF_CREATE_NOTIFICATION error", ebufp);

			 *bb_ret_flistp = ret_flistp;
			 return ;
			 }*/
			/* Pawan: Added below to update the status of mso_pur_plan to in progress */
			if(fm_mso_update_mso_purplan_status(ctxp,mso_bp_obj, MSO_PROV_IN_PROGRESS, ebufp ) == 0)
			{
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51754", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error while updating mso_purhcased_plan status.", ebufp);
				*bb_ret_flistp = ret_flistp;
				return;
			}
			/* Pawan: Added below to update the prov status of service */
			ret_val = fm_mso_update_ser_prov_status(ctxp, in_flist, MSO_PROV_IN_PROGRESS, ebufp );
			if(ret_val)
			{
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51767", ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error while updating service provisioning status.", ebufp);
				*bb_ret_flistp = ret_flistp;
				return;
			}
		}
	}
	r_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &add_plan_success, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_DESCR, "ACCOUNT - Service add plan completed successfully", ebufp);
	args_flistp = PIN_FLIST_ELEM_TAKE(return_flistp,PIN_FLD_DATA_ARRAY,0,1,ebufp);
	if(args_flistp && args_flistp != NULL)
		PIN_FLIST_CONCAT(r_flistp, args_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan  output flist", r_flistp);

	//fm_mso_cust_add_plan_lc_event(ctxp, in_flist, r_flistp, ebufp);
	//fm_mso_create_lifecycle_evt(ctxp, in_flist, r_flistp, ID_ADD_PLAN, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}



	if (ADD_MB_DATA == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling LC for ADD_MB_DATA");
		PIN_FLIST_FLD_PUT(in_flist, MSO_FLD_SUBSCRIPTION_PLAN_OBJ, subs_plan_obj, ebufp);
		fm_mso_create_lifecycle_evt(ctxp, in_flist, NULL, ID_ADDITONAL_MB, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling LC for ADD_MB_DATA");
	}

	if (FUP_FLAG == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling LC for FUP_TOPUP");
		PIN_FLIST_FLD_PUT(in_flist, MSO_FLD_SUBSCRIPTION_PLAN_OBJ, subs_plan_obj, ebufp);
		fm_mso_create_lifecycle_evt(ctxp, in_flist, NULL, ID_FUP_TOPUP, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Printing the Input Flist for fm_mso_bb_add_plan... ");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan:  input list", in_flist);
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Printing the return_flistp for fm_mso_purchase_bb_plans... ");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bb_add_plan:  return_flistp ", return_flistp);

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check if a new FUP plan being added is for Additional GB and send SMS... ");
		fm_mso_cust_add_gb_notification(ctxp, grant_valid_from_date, grant_valid_to_date, grant_amount, in_flist, &r_flistp, ebufp);		
	}


	*bb_ret_flistp = PIN_FLIST_COPY(r_flistp,ebufp);
CLEANUP:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_res_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&stpk_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&return_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	return;
}

static void fm_mso_cust_call_prov(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flist,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp)
{	
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*service_obj = NULL;
	int		status_flags = 7;	
	int		elem_id = 0;
	int		add_plan_failure = 1;
	int		*prov_call = NULL;
	int                     mod_prty = 0;
	pin_decimal_t           *priority = NULL;
	int                     prty = 0;
	char                    *tmp_str = NULL ;
	int32			statuss = 1;	

	pin_cookie_t	cookie = NULL;	
	poid_t		*acct_pdp = NULL;
	poid_t		*mso_bb_obj = NULL;
	int32		ind = 2;
	time_t		start = 0;
	time_t		end_date = 0;
	pin_flist_t	*sub_bal_imp = NULL;
	pin_flist_t	*sub_bal = NULL;	
	poid_t		*mso_bb_bp_obj = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return ;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_call_prov :");
	status_flags = 7;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_create_action:  input list", in_flist);
	acct_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	service_obj = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if( service_obj && (strcmp(PIN_POID_GET_TYPE(service_obj),MSO_BB)) == 0 )
	{
		status_flags = DOC_BB_FUP_REVERSAL;		
		fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, -1, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
		if(PIN_POID_IS_NULL(mso_bb_obj)){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_pur_plan is not active");
			fm_mso_cust_get_bp_obj( ctxp, acct_pdp, service_obj, MSO_PROV_HOLD, &mso_bb_bp_obj, &mso_bb_obj, ebufp);
			if(PIN_POID_IS_NULL(mso_bb_obj)){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"mso_pur_plan is not in progress");
				pin_set_err(ebufp, PIN_ERRLOC_FM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE,
						MSO_ERR_PLAN_NOT_FOUND, 0, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"mso_purchased_plan not found", ebufp);
				return;
			}
		}
		if(mso_bb_obj != NULL)
		{
			//get the base plan grant product's charged_to date
			end_date = fm_prov_get_expiry(ctxp, mso_bb_obj, acct_pdp, service_obj, ind, &start, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)){
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"fm_mso_cust_call_prov : Error after returning function fm_prov_get_expiry", ebufp);
			}
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_create_action:  input list1", in_flist);
	/*RAVI comparing the priority and setting the flag*/
	priority = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_PRIORITY, 1, ebufp );
	if(priority && priority != NULL)
		tmp_str = pbo_decimal_to_str(priority, ebufp);
	if(tmp_str && tmp_str != NULL)
		prty = atoi(tmp_str);
	if(prty)
		mod_prty = prty%1000;

	if(BB_ADD_MB_START <= mod_prty && BB_ADD_MB_END >= mod_prty)
	{
		status_flags = DOC_BB_ADD_MB;
	}
	/* changes ended*/
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, MSO_FLD_PURCHASED_PLAN_OBJ, provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);
	if(status_flags && (status_flags == DOC_BB_FUP_REVERSAL || status_flags == DOC_BB_ADD_MB ))
	{
		PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_FUP_STATUS, &statuss, ebufp);
		// Adding below to support SERVICE_CODE for FUPTOUPUP
		PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_OLD_PURCHASED_PLAN_OBJ, mso_bb_obj, ebufp);
		PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_OLD_PLAN_OBJ, mso_bb_bp_obj, ebufp);
	}

	/* Write USERID, PROGRAM_NAME in buffer - start */
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
	PIN_FLIST_FLD_COPY(in_flist,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
	/**setting the subscription product's charged_to date to te topup product********/
	sub_bal_imp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_SUB_BAL_IMPACTS, PIN_ELEMID_ANY,1, ebufp);
	if(sub_bal_imp && sub_bal_imp != NULL)
	{
		sub_bal = PIN_FLIST_ELEM_GET(sub_bal_imp, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY,1, ebufp);
		if(sub_bal && sub_bal != NULL)
		{
			PIN_FLIST_FLD_SET(sub_bal, PIN_FLD_VALID_TO, &end_date, ebufp);
		}
	}
	PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_SUB_BAL_IMPACTS, 0, provaction_inflistp, PIN_FLD_SUB_BAL_IMPACTS, 0, ebufp);
	/* Write USERID, PROGRAM_NAME in buffer - end */
	while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp )) != NULL)
	{
		PIN_FLIST_ELEM_COPY(in_flist, PIN_FLD_RESULTS, elem_id, provaction_inflistp, PIN_FLD_RESULTS, elem_id, ebufp); 
	} 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_create_action: provisioning input list", provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	if(provaction_outflistp && provaction_outflistp != NULL)
	{
		prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
		if(prov_call && (*prov_call == 1))
		{
			*ret_flistp = PIN_FLIST_COPY(provaction_outflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
			return;
		}
	}
	else
	{
		if (PIN_ERRBUF_IS_ERR(ebufp))
			PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51769", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in Provisioning action.", ebufp);
		return;
	}
	/*  if (PIN_ERRBUF_IS_ERR(ebufp))
	    {
	    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
	    PIN_ERRBUF_RESET(ebufp);
	    PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
	    return 10;
	    } */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_create_action: provisioning output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
	return;
}

static int fm_mso_cust_call_notif(
		pcm_context_t           *ctxp,
		pin_flist_t             *in_flist,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t	*provaction_inflistp = NULL;
	pin_flist_t	*provaction_outflistp = NULL;
	int		status_flags = 7;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return ;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_call_prov :");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_create_action:  input list", in_flist);
	status_flags = 7;
	provaction_inflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, provaction_inflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &status_flags, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_call_notif notification input list", provaction_inflistp);
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
		return 10;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_call_notif notification output flist", provaction_outflistp);
	PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
	return 1;
}

/**************************************************
 * Function: 	fm_mso_update_device_plan_obj()
 * Decription:   Returns the device id of all the 
 *            devices associated with service
 *			  passed in input.
 * 
 ***************************************************/
static void
fm_mso_update_device_plan_obj(
		pcm_context_t		*ctxp,
		pin_flist_t			*in_flist,
		poid_t			*mso_pp_obj,
		int			device_type,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*wrt_out_flistp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*pp_obj = NULL;
	int32			add_plan_success = 0;
	int32			add_plan_failure = 1;
	int32			srch_flag = 512;
	int64			db = 1;
	char			*dev_id = NULL;
	char			*template = "select X from /device  where F1 = V1 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_device_plan_obj", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_device_plan_obj input flist",in_flist);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	arg_flistp = PIN_FLIST_ELEM_GET(in_flist, PIN_FLD_PLAN, PIN_ELEMID_ANY,1, ebufp );
	dev_id = PIN_FLIST_FLD_GET(arg_flistp, PIN_FLD_DEVICE_ID, 1, ebufp);

	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_DEVICE_ID, dev_id, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_ELEM_SET(result_flist,NULL, PIN_FLD_PLAN, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_device_plan_obj search input flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51435", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in searching Device", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_device_plan_obj search output flist", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	if (!result_flist)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Device Not found with specified Device ID");
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51436", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device Not found with specified Device ID", ebufp);
		return;
	}
	//Elem Id 0 is for storing Hardware plan details;
	plan_flistp = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_PLAN, 0, 1, ebufp );
	if(plan_flistp)
	{
		pp_obj = PIN_FLIST_FLD_GET(plan_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 1, ebufp);
		if(pp_obj) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Device is already associated to MSO Pur plan.");
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51437", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Device already associated to MSO Purchased plan.", ebufp);
			return;
		}
	}

	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, srch_in_flistp, PIN_FLD_POID, ebufp);
	//Elem Id 0 is for storing Hardware plan details;
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_PLAN, 0, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_pp_obj, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_TYPE, &device_type, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_device_plan_obj Write flds input", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, PCM_OPFLG_ADD_ENTRY, srch_in_flistp, &wrt_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling WRITE_FLDS", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &add_plan_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51438", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Add Plan - Error in updating Device with MSO_PP", ebufp);
		return;
	}
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&wrt_out_flistp, NULL);
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flist, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &add_plan_success, ebufp);
	return;
}

/*******************************************************
 * Function: fm_mso_get_curr_mso_purplan()
 *	Returns the MSO_PURCHASED_PLAN poid and status in
 *	the output flist for account and plan obj passed
 *	input.
 *	Status to be passed -1 to ignore the status while
 *	searching purchased plan.
 *******************************************************/
static void
fm_mso_get_curr_mso_purplan(
		pcm_context_t           *ctxp,
		poid_t			*acc_pdp,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp)
{

	pin_flist_t *search_flistp = NULL;
	pin_flist_t *search_res_flistp = NULL;
	pin_flist_t *args_flistp = NULL;
	pin_flist_t *result_flistp = NULL;
	pin_flist_t *prod_flistp = NULL;

	poid_t		*search_pdp = NULL;
	poid_t		*mso_pp_obj = NULL;
	poid_t		*offering_obj = NULL;
	int			prov_tag_found = 0;
	int             search_flags = 512;
	int		elem_id = 0;
	int32           db = -1;
	char	template_2[100] = " select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and status in (2) ";
	char	*prov_tag = NULL;
	void	*vp = NULL;
	pin_cookie_t	cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	db = PIN_POID_GET_DB(acc_pdp);


	search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	search_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template_2, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);		

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, "base plan", ebufp);

	//args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
	//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);


	result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, (char *)NULL, ebufp);
	PIN_FLIST_ELEM_SET(result_flistp, NULL, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_curr_mso_purplan search input list", search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching MSO Purchased plan", ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_curr_mso_purplan output flist", search_res_flistp);

	result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_curr_mso_purplan result flist", result_flistp);
	if ( !result_flistp )
	{
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NO Result");
		return;
	}
	while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, PIN_FLD_PRODUCTS, 
					&elem_id, 1, &cookie, ebufp )) != NULL)
	{
		vp = (char *)PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
		if(vp && strlen(vp)>0) {
			prov_tag = strdup(vp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Prov tag is");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, prov_tag);
			offering_obj = PIN_FLIST_FLD_TAKE(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 1, ebufp);
			prov_tag_found = 1;
			break;
		}
	}

	if(prov_tag_found)
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_PRIORITY, *ret_flistp, PIN_FLD_PRIORITY, ebufp );
		PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_PLAN_OBJ, *ret_flistp, PIN_FLD_PLAN_OBJ, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_PROVISIONING_TAG, prov_tag , ebufp );
		PIN_FLIST_FLD_PUT(*ret_flistp, PIN_FLD_OFFERING_OBJ, offering_obj , ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_curr_mso_purplan return flist", *ret_flistp);
	}
	PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	return;
}	

/********************************************************
 * Function: fm_mso_set_prod_end_date()
 *	This function calculates the end date and sets 
 *	the end date in the input flist of purchase_bb_plans.
 *	For postpaid, the date is set to next bill_t 
 *	and for prepaid, the date is set to base plan end date.
 *********************************************************/
static int
fm_mso_set_prod_end_date(
		pcm_context_t           *ctxp,
		poid_t			*offering_obj,
		int32			*indicator,
		pin_flist_t		*pp_flistp,
		pin_errbuf_t            *ebufp)
{

	pin_flist_t		*read_in_flistp = NULL;
	pin_flist_t		*read_out_flistp = NULL;
	pin_flist_t		*prod_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_cookie_t		cookie = NULL;
	time_t			*end_date = NULL;
	poid_t			*search_pdp = NULL;
	int32			search_flags = 512;
	int32			db = -1;
	int32			elem_id = 0;
	char			template[100] = "select X from /billinfo where F1 = V1 and F2 = V2 ";

	char			tmp_buf[100];
	int			conn_type = 0;
	int32			flag = 1;	
	time_t			max_end_t = 0;
	pin_flist_t		*cycle_fees_flistp = NULL;
	int			*unit = NULL;
	int			*count = NULL;	
	poid_t			*serv_pdp = NULL;
	pin_flist_t		*ret_flistpp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	int			*fup_status = NULL;
	time_t			current_t = pin_virtual_time(NULL);
	int32			fup_plan = 0;	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_set_prod_end_date", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_prod_end_date input flist",pp_flistp);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Offering OBJ", offering_obj);
	serv_pdp = PIN_FLIST_FLD_GET(pp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(offering_obj);

	if (indicator) conn_type = *(int *)indicator;
	else {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_APPLICATION,
				PIN_ERR_IS_NULL, PIN_FLD_INDICATOR, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_set_prod_end_date: Indicator is NULL", ebufp);
		return;

	}
	sprintf(tmp_buf, "fm_mso_set_prod_end_date : Indicator in input flist = %d",conn_type);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buf);

	//if(*indicator == MSO_PREPAID)
	if( conn_type == MSO_PREPAID)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account is Prepaid");
		fm_mso_get_poid_details(ctxp, serv_pdp, &ret_flistpp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while returning from function : fm_mso_get_poid_details", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			return 0;
		}
		bb_info_flistp = PIN_FLIST_SUBSTR_GET(ret_flistpp, MSO_FLD_BB_INFO, 0, ebufp );
		fup_status = PIN_FLIST_FLD_TAKE(bb_info_flistp, MSO_FLD_FUP_STATUS, 0, ebufp);
		if(fup_status && *fup_status != 0)
		{
			fup_plan = 1;
			PIN_FLIST_DESTROY_EX(&ret_flistpp,NULL);
			get_last_mso_purchased_plan_modified(ctxp, pp_flistp, offering_obj, flag ,&ret_flistpp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while returning from function : get_last_mso_purchased_plan_modified", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				return 0;
			}
			cookie = NULL;
			while ((cycle_fees_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistpp, PIN_FLD_CYCLE_FEES,
							&elem_id, 1, &cookie, ebufp)) != NULL)
			{
				end_date = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
				if(end_date && *end_date > max_end_t)
				{
					max_end_t = *end_date;
				}
			}	
		}
		else
		{
			read_in_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_POID, offering_obj, ebufp );
			PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_CYCLE_END_T, NULL, ebufp );
			PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_prod_end_date read input", read_in_flistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_in_flistp, &read_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&ret_flistpp,NULL);
				return 0;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_prod_end_date read output", read_out_flistp);
			end_date  = PIN_FLIST_FLD_TAKE(read_out_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp );
		}
		PIN_FLIST_DESTROY_EX(&ret_flistpp,NULL);
	}
	//else if(*indicator == MSO_POSTPAID)
	else if(conn_type == MSO_POSTPAID)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account is Postpaid");
		search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		read_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(read_in_flistp, PIN_FLD_POID, search_pdp, ebufp);
		PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
		PIN_FLIST_FLD_SET(read_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(pp_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);		

		args_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILLINFO_ID, "BB", ebufp);

		result_flistp = PIN_FLIST_ELEM_ADD(read_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACTG_NEXT_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_NEXT_BILL_T, NULL, ebufp);


		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_prod_end_date search input ", read_in_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, read_in_flistp, &read_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching billinfo", ebufp);
			PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_prod_end_date output flist", read_out_flistp);
		result_flistp = PIN_FLIST_ELEM_GET(read_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		end_date  = PIN_FLIST_FLD_TAKE(result_flistp, PIN_FLD_ACTG_NEXT_T, 1, ebufp );
	}
	plan_flistp = PIN_FLIST_ELEM_GET(pp_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 0, ebufp );
	cookie = NULL;

	while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp, PIN_FLD_PRODUCTS, 
					&elem_id, 1, &cookie, ebufp )) != NULL)
	{
		if(fup_plan == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check5");
			if(max_end_t == 0)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No end date was set", ebufp);
				return 0;
			}
			else if(current_t > max_end_t)
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Fup reset not done", ebufp);
				return 0;	
			}
			PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_END_T, &max_end_t, ebufp );
		}
		else if(end_date)
		{
			PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PURCHASE_END_T, end_date, ebufp );
		}
	}
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in setting end date.", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		return 0;
	}
	return 1;
}

static	int
fm_mso_compare_ca_ids(
		pcm_context_t           *ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*from_plan_flistp = NULL;
	pin_flist_t		*to_plan_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*flistp_ca_id = NULL;
	pin_flist_t		*old_flistp = NULL;
	pin_flist_t		*old_flistp_ca_id = NULL;
	pin_flist_t 		*read_o_flistp = NULL;
	pin_flist_t 		*read_flistp = NULL;
	pin_flist_t 		*r_flistp = NULL;
	pin_flist_t 		*rflistp = NULL;
	pin_flist_t 		*pp_out_flistp = NULL;

	void			*vp = NULL;
	char			msg[1024];
	char			func_name[] = "fm_mso_compare_ca_ids";
	char			*ca_id = NULL;
	char			*old_ca_id = NULL;
	char			*node = NULL;

	int32			compare_flag = 0;
	int32			rec_id = 0;	
	int32			rec_id1 = 0;
	int32                   rec_id2 = 0;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t            cookie2 = NULL;
	int32			old_prd_count = 0;
	int32			new_prd_count = 0;

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

	//getting details of added plan
	tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS, PIN_ELEMID_ANY, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PLAN_OBJ, 1 ,ebufp);


	if (vp){
		fm_mso_get_ca_id_from_plan(ctxp, i_flistp, (poid_t *)vp, node, &from_plan_flistp, ebufp);
	}

	sprintf(msg,"%s: Added Plan flist", func_name);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, from_plan_flistp);

	old_prd_count = PIN_FLIST_ELEM_COUNT(from_plan_flistp, PIN_FLD_RESULTS,ebufp);
	fm_mso_get_cust_active_plans (ctxp, i_flistp, &pp_out_flistp, ebufp);	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pp_out_flistp: ", pp_out_flistp);
	while ((r_flistp = PIN_FLIST_ELEM_GET_NEXT(pp_out_flistp,
					PIN_FLD_RESULTS, &rec_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PP IN");
		vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);

		if  (vp)
			fm_mso_get_ca_id_from_plan(ctxp, i_flistp, (poid_t *)vp, node, &to_plan_flistp, ebufp);

		sprintf(msg,"%s: Active Plan flist", func_name);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, to_plan_flistp);
		new_prd_count = PIN_FLIST_ELEM_COUNT(to_plan_flistp, PIN_FLD_RESULTS,ebufp);

		if (old_prd_count > 0 && new_prd_count >0 && old_prd_count >= new_prd_count)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 1");

			/********* VERIMATRIX CHANGES - Enhanced condition to check for Verimatrix network node ***/
			if ( node )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 1.1");
				rec_id = 0;
				cookie = NULL;
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
						rec_id1 = 0;
						cookie1 = NULL;
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
								if (strcmp(old_ca_id, "NA") != 0 && old_ca_id && ca_id && strcmp(ca_id,old_ca_id) == 0)
								{
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,"Duplicate CA ID");
									compare_flag = 1;
									break;
								}
							}
						}
					}
					/*if (compare_flag == 1){
					  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 7");
					  break;
					  }*/
				}
			}
			PIN_FLIST_DESTROY_EX(&to_plan_flistp, NULL);
		}
		if (compare_flag == 1){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check 8");
			break;
		}
	}		

CLEANUP:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"I am in Cleanup");
	PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&from_plan_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pp_out_flistp, NULL);
	return compare_flag;
}

static	void
fm_mso_get_ca_id_from_plan(
		pcm_context_t           *ctxp,
		pin_flist_t				*i_flistp,
		poid_t                  *mso_plan_obj,
		char                  	*mso_node,
		pin_flist_t				**ret_flistp,
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
	char			func_name[] = "fm_mso_get_ca_id_from_plan";
	int32			args_cnt = 1;
	int32			search_fail = 1;
	int64           db = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing");	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;
	}

	sprintf(msg,"%s: Prepare search flist to search CA_ID using PLAN", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1 ,ebufp);
	db = PIN_POID_GET_DB(pdp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);

	// Add ARGS array
	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PROVISIONING_TAG,"",ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_PROVISIONING_TAG,"",ebufp);

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

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, args_cnt++, ebufp);
	PIN_FLIST_FLD_SET(arg_flist,PIN_FLD_POID, mso_plan_obj,ebufp);

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

static	void
fm_mso_get_cust_active_plans(
		pcm_context_t           *ctxp,
		pin_flist_t		*i_flistp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;

	poid_t			*serv_pdp = NULL;
	poid_t 			*added_plan_pdp = NULL;

	char			msg[1024];
	int32			srch_flags = 768;
	char			template [] = "select X from /purchased_product where  F1 = V1 and F2 != V2 and F3 = V3 and F4 = V4 ";
	char			func_name[] = "fm_mso_get_cust_active_plans";
	int32			prod_status = 1;
	int32			status_failure =-1;
	int64           	db = 1;

	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing", func_name);	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;
	}

	tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS, PIN_ELEMID_ANY, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_PLAN_OBJ, 1 ,ebufp);

	if (vp){
		added_plan_pdp = (poid_t *)vp;
	}

	serv_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1 ,ebufp);
	if (!serv_pdp){
		*ret_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
		goto CLEANUP;
	}
	sprintf(msg,"%s: Prepare search flist to search active purchased plan", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, added_plan_pdp, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prod_status, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp );

	result_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch purchased product poids iflist:", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()");
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch purchased product poids oflist:", srch_oflistp);
	*ret_flistp = PIN_FLIST_COPY(srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_cust_active_plans return flist", *ret_flistp);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
}

void
fm_mso_pdt_channels_validation(
		pcm_context_t		*ctxp,
		pin_flist_t			*i_flistp,
		poid_t				*srvc_pdp,
		pin_flist_t			**ret_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*res_channels_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*pdts_flistp = NULL;
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*active_plans_iflistp = NULL;
	pin_flist_t		*active_plans_oflistp = NULL;
	pin_flist_t		*new_channels_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;

	pin_flist_t		*existing_channels_flistp = NULL;

	poid_t			*pdt_pdp = NULL;
	poid_t			*new_pdt_pdp = NULL;
	poid_t			*user_pdp = NULL;

	int				elem_id = 0;
	int				elem_id1 = 0;
	int				elem_id2 = 0;
	static int		elem_id3 = 0;
	int				status = 0;
	int				failure = 1;
	int				success = 0;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie_1 = NULL;
	pin_cookie_t	cookie_2 = NULL;


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_pdt_channels_validation Input flist", i_flistp);

	user_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);

	new_pdt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
	if (new_pdt_pdp)
	{
		// Retrieving Channels for New Plans
		fm_mso_get_channels(ctxp, user_pdp, new_pdt_pdp, &elem_id2, &new_channels_flistp, ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Concantenated New Channels Flist", new_channels_flistp);

	// Retrieving Existing Active Plans
	active_plans_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, active_plans_iflistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, active_plans_iflistp, PIN_FLD_SERVICE_OBJ, ebufp );
	plan_flistp = PIN_FLIST_ELEM_ADD(active_plans_iflistp, PIN_FLD_PLAN_LISTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp );
	fm_mso_get_cust_active_plans (ctxp, active_plans_iflistp, &active_plans_oflistp, ebufp);

	existing_channels_flistp = PIN_FLIST_CREATE(ebufp);
	if (!active_plans_oflistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active Plan Search ERROR");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_get_cust_active_plans", ebufp);
		PIN_ERRBUF_RESET(ebufp);

		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Active Plans Search Error", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
		*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&new_channels_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&active_plans_oflistp, NULL);
		return;

	}
	// Retrieving & Capturing Channels for Existing Active Plans
	while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(active_plans_oflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Results Array Flist", result_flistp);
		pdt_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
		if (pdt_pdp)
		{
			fm_mso_get_channels(ctxp, user_pdp, pdt_pdp, &elem_id3, &existing_channels_flistp, ebufp);
		}
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Concantenated Existing Channels Flist", existing_channels_flistp);


	if (new_channels_flistp && existing_channels_flistp)
	{
		status = fm_mso_find_channel_existence(ctxp, new_channels_flistp, existing_channels_flistp, ebufp);
	}

	if (status == 1)
	{
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Channel Already Available");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Channel(s) about to Purchase is Already Available !!!", ebufp);
		PIN_ERRBUF_RESET(ebufp);

		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Channel(s) about to Purchase is Already Available !!!", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
		*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&new_channels_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&existing_channels_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&active_plans_oflistp, NULL);
		return;
	}

	*ret_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &success, ebufp);

	PIN_FLIST_DESTROY_EX(&active_plans_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&active_plans_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&new_channels_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&existing_channels_flistp, NULL);
}

void
fm_mso_get_channels(
		pcm_context_t		*ctxp,
		poid_t				*user_pdp,
		poid_t				*pdt_pdp,
		int					*elem_id3,
		pin_flist_t			**ret_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*channels_search_iflistp = NULL;
	pin_flist_t		*channels_search_oflistp = NULL;
	pin_flist_t		*res_channels_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	int				search_flag = 9;
	int				elem_id1 = 0;
	int				elem_id2 = 0;
	pin_cookie_t	cookie_1 = NULL;
	pin_cookie_t	cookie_2 = NULL;


	channels_search_iflistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(channels_search_iflistp, PIN_FLD_POID, pdt_pdp, ebufp);
	PIN_FLIST_FLD_SET(channels_search_iflistp, PIN_FLD_USERID, user_pdp, ebufp);
	PIN_FLIST_FLD_SET(channels_search_iflistp, PIN_FLD_PROGRAM_NAME, "fm_mso_pdt_channels_validation", ebufp);
	PIN_FLIST_FLD_SET(channels_search_iflistp, PIN_FLD_FLAGS, &search_flag, ebufp);
	// Retrieving the Channels for the New Plan Purchase

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Channels Search Input Flist", channels_search_iflistp);
	PCM_OP(ctxp, MSO_OP_OPERATIONS_SEARCH, 0, channels_search_iflistp, &channels_search_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Channels Search Output Flist", channels_search_oflistp);

	while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(channels_search_oflistp, PIN_FLD_RESULTS, &elem_id1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Results Channels Flist", result_flistp);
		while((res_channels_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, MSO_FLD_CATV_CHANNELS, &elem_id2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_FLIST_ELEM_COPY(result_flistp, MSO_FLD_CATV_CHANNELS, elem_id2, *ret_flistp, MSO_FLD_CATV_CHANNELS, *elem_id3, ebufp);
			(*elem_id3)++;
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Concantenated Channels Flist", *ret_flistp);
		}
		elem_id2=0;
		cookie_2 = NULL;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Concantenated Channels Flist", *ret_flistp);

	//*ret_flistp = PIN_FLIST_COPY(existing_channels_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&channels_search_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&channels_search_oflistp, NULL);

}

int
fm_mso_find_channel_existence(
		pcm_context_t		*ctxp,
		pin_flist_t			*new_flistp,
		pin_flist_t			*existing_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*channel_flistp = NULL;
	pin_flist_t		*existing_channel_flistp = NULL;
	char			*channel_id = NULL;
	char			*exist_channel_id = NULL;
	int				elem_id1 = 0;
	int				elem_id2 = 0;
	pin_cookie_t	cookie_1 = NULL;
	pin_cookie_t	cookie_2 = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Channels Existence Validation New Plans Flist", new_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Channels Existence Validation Existing Plans Flist", existing_flistp);

	while((channel_flistp = PIN_FLIST_ELEM_GET_NEXT(new_flistp, MSO_FLD_CATV_CHANNELS, &elem_id1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
	{
		channel_id = PIN_FLIST_FLD_GET(channel_flistp, MSO_FLD_CHANNEL_ID, 1, ebufp);
		if (channel_id && (strcmp(channel_id,"")!=0))
		{
			while((existing_channel_flistp = PIN_FLIST_ELEM_GET_NEXT(existing_flistp, MSO_FLD_CATV_CHANNELS, &elem_id2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
			{
				exist_channel_id = PIN_FLIST_FLD_GET(existing_channel_flistp, MSO_FLD_CHANNEL_ID, 1, ebufp);
				if (exist_channel_id && (strcmp(exist_channel_id,"")!=0))
				{
					if (strcmp(channel_id, exist_channel_id) == 0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Channels About to Purchase is Already Available");
						return 1;

					}
				}
			}
			elem_id2 = 0;
			cookie_2 = NULL;

		}
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Channels About to Purchase is Not Available");
	return 0;
}

void
fm_mso_retrieve_pdt_info(
		pcm_context_t		*ctxp,
		pin_flist_t			*i_flistp,
		pin_flist_t			**ret_flistp,
		pin_errbuf_t		*ebufp)
{

	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*pdts_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*srch_cfg_iflistp = NULL;
	pin_flist_t		*srch_cfg_oflistp = NULL;
	pin_flist_t		*results_flistp	 = NULL;
	poid_t			*deal_pdp = NULL;
	char			template1[] = "select X from /product 1, /deal 2 where  2.F1 = V1 and 2.F2 = 1.F3";
	char			template2[] = "select X from /mso_cfg_catv_pt 1 where  1.F1 = V1 and 1.F2 = V2";
	int32			srch_flags = 768;
	int64			db = 1;
	int				failure = 1;
	int				success = 0;
	int				base_pack = 0;
	int			elem_id = 0;
	pin_cookie_t	cookie = NULL;

	services_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY , 1, ebufp);
	deal_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_DEAL_OBJ, 1 ,ebufp);

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template1, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, deal_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	pdts_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(pdts_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	res_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_NAME, "", ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PRIORITY, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Products Input FLIST", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);

		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Products output flist", srch_oflistp);

	while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		srch_cfg_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_cfg_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_cfg_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
		PIN_FLIST_FLD_SET(srch_cfg_iflistp, PIN_FLD_TEMPLATE, template2, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_cfg_iflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_PROVISIONING_TAG,args_flistp, PIN_FLD_PROVISIONING_TAG, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_cfg_iflistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PKG_TYPE, &base_pack, ebufp);
		results_flistp = PIN_FLIST_ELEM_ADD(srch_cfg_iflistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search BAse pack input flist", srch_cfg_iflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_cfg_iflistp, &srch_cfg_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_cfg_iflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);

			PIN_FLIST_DESTROY_EX(&srch_cfg_oflistp, NULL);
			return;
		}	
		results_flistp = PIN_FLIST_ELEM_GET(srch_cfg_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if (results_flistp)
		{
			PIN_ERR_LOG_FLIST(3, "Base pack found", res_flistp);
			break;
		}
		PIN_FLIST_DESTROY_EX(&srch_cfg_oflistp, NULL);
	}

	if(!res_flistp)
	{
		res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	}

	if (!res_flistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan Not Available");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Plan Not Available", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plan Not Available", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);

		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return;
	}

	*ret_flistp = PIN_FLIST_COPY(res_flistp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &success, ebufp);


	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

}

/***********************************************************************
 * fm_mso_activate_alc_addon_pdts() 
 *
 * This function retrieves suspended active plans and reactivates them. 
 ***********************************************************************/
void 
fm_mso_activate_alc_addon_pdts(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flistp,
		poid_t          *product_pdp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t		*alc_addon_pdts_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*write_flds_inflistp = NULL;
	pin_flist_t		*write_flds_outflistp = NULL;
	pin_flist_t		*s_outflistp = NULL;
	pin_flist_t		*tmp_in_flistp = NULL;
	pin_flist_t		*prov_out_flistp = NULL;
	pin_flist_t		*pdt_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*tmp_srvc_pdp = NULL;
	poid_t          *res_pdt_pdp = NULL;
	int32			ret_status = 0;
	int32			main_conn_type = -1;
	int32           defatul_gp_flags = 0xFFFF;
	int			elem_id = 0;
	int			add_plan_flag = CATV_ADD_PACKAGE;
	int			failure = 1;
	int			new_status_flags = 8;
	int         reactivate_flag = 0;
	pin_cookie_t	cookie = NULL;
	time_t		cet = 0;
	time_t		pet = 0;
	time_t		bp_valid_flag = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);	

	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);

	main_conn_type = fm_mso_catv_conn_type(ctxp, srvc_pdp, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_get_alc_addon_pdts function");
	fm_mso_get_alc_addon_pdts(ctxp, acct_pdp, srvc_pdp, &alc_addon_pdts_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_alc_addon_pdts return flist", alc_addon_pdts_flistp);
	if (alc_addon_pdts_flistp)
	{
		ret_status = *(int32 *)PIN_FLIST_FLD_GET(alc_addon_pdts_flistp, PIN_FLD_STATUS, 1, ebufp);
		if (ret_status == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "There are no add_on and ALC products to provision");
			goto CHILD;		
		}
		else
		{
			//Checking the Base Pack Validity for Re-activation of other Packs
			//bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_flistp, 0, ebufp);

			//if (bp_valid_flag > 0)
			//{
				prov_in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, prov_in_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, prov_in_flistp, PIN_FLD_USERID, ebufp);
				PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_PROGRAM_NAME, "Plan Re-activation|Grace Period Renewal", ebufp);
				PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &add_plan_flag, ebufp);
				while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(alc_addon_pdts_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased Products Individual results flist", results_flistp);
					tmp_srvc_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
					res_pdt_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
					write_flds_inflistp = PIN_FLIST_CREATE(ebufp);

					//Compare the Renewal Product and Other Purchased Products
					if (PIN_POID_COMPARE(product_pdp, res_pdt_pdp, 0, ebufp) != 0 )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Non-Renewal Product");
						pet = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
						cet = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
						if (pet > cet)
						{
							// PET & CET mismatch, hence no need to re-activate.
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " PET > CET");
							//Checking whether the Plan is under Default Grace Mode or not
							reactivate_flag = fm_mso_valid_reactivate(ctxp, acct_pdp, results_flistp, ebufp);
							if (reactivate_flag == 1)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Difference Lesser than the Default Grace Period");

								//Creating the Provisioning Action to Enable the Ala-Carte and Add-on Plans
								pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
								PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);

								PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
								PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &defatul_gp_flags, ebufp);

								PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

								if (PIN_ERRBUF_IS_ERR(ebufp))
								{
									PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
										"Error in fm_mso_activate_alc_addon_pdts: wflds", ebufp);
								}

							}
							else
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Difference More than the Default Grace Period. No need for Re-activation");
							}
						}
						else
						{
							//Creating the Provisioning Action to Enable the Ala-Carte and Add-on Plans
							pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
							PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);

							PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
							PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);

							PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
									"Error in fm_mso_activate_alc_addon_pdts: wflds", ebufp);
							}
						}
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Renewal Product");
						//Creating the Provisioning Action to Enable the Ala-Carte and Add-on Plans
						pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
						PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);

						PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);

						PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
								"Error in fm_mso_activate_alc_addon_pdts: wflds", ebufp);
						}
					}
					PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
					PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);

				}

				if (PIN_FLIST_ELEM_COUNT(prov_in_flistp, PIN_FLD_PRODUCTS, ebufp) > 0)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Input flist", prov_in_flistp);
					PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Output flist", prov_out_flistp);
					PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
					if (prov_out_flistp)
					{
						*ret_flistp = PIN_FLIST_COPY(prov_out_flistp, ebufp);
					}
					else
					{
						err_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
						PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
						PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in calling MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
						PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
						*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
						PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
						return;
					}
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Skipping Provisioning as no Product to Provision");
				}
				PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					err_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in calling MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
					*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);

					PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
					return;
				}
		    	//}
			/*else
		        {
				PIN_ERRBUF_RESET(ebufp);
				err_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Base Pack Not Active. Please Activate the Base Pack first", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
			 	*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);
			 	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
			 	return;
			 }*/
		}
		PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
	}

CHILD :
	 if (main_conn_type == 0)
	 {
	 	PIN_ERR_LOG_MSG(3,"Entering child activation block");
	 	fm_mso_utils_get_catv_children(ctxp, acct_pdp, &s_outflistp, ebufp);

	 	elem_id = 0;
	 	cookie = NULL;
	 	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(s_outflistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	 	{
	 		tmp_in_flistp = PIN_FLIST_COPY(in_flistp, ebufp);
	 		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, tmp_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

	 		fm_mso_activate_child_alc_addon_pdts(ctxp, tmp_in_flistp, product_pdp, &alc_addon_pdts_flistp, ebufp);

	 		PIN_FLIST_DESTROY_EX(&tmp_in_flistp, NULL);
	 		PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
	 	}
	 	PIN_FLIST_DESTROY_EX(&s_outflistp, NULL);
	 }
	 else
	{
	 	PIN_ERR_LOG_MSG(3,"This is a child service");
	}
	return;
}


/***********************************************************************
 * fm_mso_activate_child_alc_addon_pdts() 
 *
 * This function retrieves suspended active child plans and reactivates them. 
 ***********************************************************************/
static void 
fm_mso_activate_child_alc_addon_pdts(
		pcm_context_t		*ctxp,
		pin_flist_t		*in_flistp,
		poid_t          *product_pdp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp)
{
	pin_cookie_t    cookie = NULL;
	pin_flist_t		*alc_addon_pdts_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*write_flds_inflistp = NULL;
	pin_flist_t		*write_flds_outflistp = NULL;
	pin_flist_t		*prov_out_flistp = NULL;
	pin_flist_t		*pdt_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*tmp_srvc_pdp = NULL;
	poid_t          *res_pdt_pdp = NULL;
	time_t			pet = 0;
	time_t			cet = 0;
	int32			ret_status = 0;
	int32			new_status_flags = 8;
	int32           default_gp_flags = 0xFFFF;
	int			    elem_id = 0;
	int			    add_plan_flag = CATV_ADD_PACKAGE;
	int			    failure = 1;
	int             reactivate_flag = 0;
	time_t             bp_valid_flag = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);	

	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Child: Calling fm_mso_get_alc_addon_pdts function");
	fm_mso_get_alc_addon_pdts(ctxp, acct_pdp, srvc_pdp, &alc_addon_pdts_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Child: fm_mso_get_alc_addon_pdts return flist", alc_addon_pdts_flistp);
	if (alc_addon_pdts_flistp)
	{
		ret_status = *(int32 *)PIN_FLIST_FLD_GET(alc_addon_pdts_flistp, PIN_FLD_STATUS, 1, ebufp);
		if (ret_status == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "There are no add_on and ALC products to provision");
			return;
		}
		else
		{
			//Checking the Base Pack Validity for Re-activation of other Packs
			bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_flistp, 0, ebufp);

			if (bp_valid_flag > 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering Re-activation zone, as Base Pack is eligible for Re-activation");
				prov_in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, prov_in_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, prov_in_flistp, PIN_FLD_USERID, ebufp);
				PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_PROGRAM_NAME, "Plan Re-activation|Grace Period Renewal", ebufp);
				PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &add_plan_flag, ebufp);
				while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(alc_addon_pdts_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased Products Individual results flist", results_flistp);

					tmp_srvc_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
					res_pdt_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
					write_flds_inflistp = PIN_FLIST_CREATE(ebufp);

					//Compare the Renewal Product and Other Purchased Products
					if (PIN_POID_COMPARE(product_pdp, res_pdt_pdp, 0, ebufp) != 0 )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Non-Renewal Product");
						pet = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
						cet = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
						if (pet > cet)
						{
							// PET & CET mismatch, hence no need to re-activate.
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " PET > CET");
							//Checking whether the Plan is under Default Grace Mode or not
							reactivate_flag = fm_mso_valid_reactivate(ctxp, acct_pdp, results_flistp, ebufp);
							if (reactivate_flag == 1)
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Difference Lesser than the Default Grace Period");

								//Creating the Provisioning Action to Enable the Ala-Carte and Add-on Plans
								pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
								PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);

								PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
								PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &default_gp_flags, ebufp);

								PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

								if (PIN_ERRBUF_IS_ERR(ebufp))
								{
									PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
											"Error in fm_mso_activate_alc_addon_pdts: wflds", ebufp);
								}

							}
							else
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Difference More than the Default Grace Period. No need for Re-activation");
							}

						}
						else
						{
							//Creating the Provisioning Action to Enable the Ala-Carte and Add-on Plans
							pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
							PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);

							PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
							PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);

							PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
										"Error in fm_mso_activate_alc_addon_pdts: wflds", ebufp);
							}

						}

					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Current Renewal Product");
						//Creating the Provisioning Action to Enable the Ala-Carte and Add-on Plans
						pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
						PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);

						PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);

						PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

						if (PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
									"Error in fm_mso_activate_alc_addon_pdts: wflds", ebufp);
						}
					}
					PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
					PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);

				}

				if (PIN_FLIST_ELEM_COUNT(prov_in_flistp, PIN_FLD_PRODUCTS, ebufp) > 0)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Input flist", prov_in_flistp);
					PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Output flist", prov_out_flistp);
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No Need of Provisioning");
				}

				PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					err_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in calling MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
					*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);

					PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
					return;
				}

				if (prov_out_flistp)
				{
					*ret_flistp = PIN_FLIST_COPY(prov_out_flistp, ebufp);
				}
				else
				{
					err_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in calling MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
					*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);

					PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
					return;
				}
			}
			else
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling ADD_PLAN", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				err_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Base Pack Not Active. Please Activate the Base Pack first", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
				*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);

				PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
				return;
			}
		}
		PIN_FLIST_DESTROY_EX(&alc_addon_pdts_flistp, NULL);
	}

}

/************************************************************************
 * fm_mso_get_alc_addon_pdts() 
 *
 * This function retrieves suspended purchased products of given service 
 *************************************************************************/
void
fm_mso_get_alc_addon_pdts(
		pcm_context_t		*ctxp,
		poid_t			*acct_pdp,
		poid_t			*srvc_pdp,
		pin_flist_t		**ret_flistp,
		pin_errbuf_t		*ebufp)
{

	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*pdts_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	poid_t			*deal_pdp = NULL;
	char			templatep[2001];
	int32			srch_flags = 768;
	int32			status_flags = 0xFFF;
	int64			db = 1;
	int			failure = 1;
	int			success = 0;
	int32			active_status = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;	
	PIN_ERRBUF_CLEAR(ebufp);

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &active_status, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);

	sprintf(templatep, "select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 and F4 = V4 ");
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, templatep, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CYCLE_END_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS_FLAGS, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Purchase Products Input FLIST", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);

		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
		*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Purchase Products output flist", srch_oflistp);

	res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

	if (!res_flistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan Not Available");
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Plan Not Available", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
		*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);

		return;
	}

	*ret_flistp = PIN_FLIST_COPY(srch_oflistp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &success, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
}


/************************************************************************
 * fm_mso_get_base_pdts() 
 *
 * This function retrieves suspended purchased products of given service 
 *************************************************************************/
void
fm_mso_get_base_pdts(
		pcm_context_t		*ctxp,
		poid_t			    *acct_pdp,
		poid_t			    *srvc_pdp,
		pin_flist_t		    **ret_flistp,
		pin_errbuf_t		*ebufp)
{

	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*pdts_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	poid_t			*deal_pdp = NULL;
	char			templatep[2001];
	int32			srch_flags = 768;
	int32			status_flags = 0xFFF;
	int64			db = 1;
	int			    failure = 1;
	int			    success = 0;
	int             base_pack = 0;
	int32			active_status = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;	
	PIN_ERRBUF_CLEAR(ebufp);

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &active_status, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, srvc_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, srvc_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, srvc_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, srvc_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 8, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PKG_TYPE, &base_pack, ebufp);

	sprintf(templatep, "select X from /purchased_product 1, /product 2, /mso_cfg_catv_pt 3 where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3 and 1.F4 = 2.F5 and 2.F6 = 3.F7 and 3.F8 = V8 ");
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, templatep, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CYCLE_END_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS_FLAGS, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Purchase Products Input FLIST", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);

		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
		*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Purchase Products output flist", srch_oflistp);

	res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

	if (!res_flistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Plan Not Available");
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Base Plan Not Available", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &failure, ebufp);
		*ret_flistp = PIN_FLIST_COPY(err_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);

		return;
	}

	*ret_flistp = PIN_FLIST_COPY(srch_oflistp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &success, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
}

int 
fm_mso_valid_reactivate(
		pcm_context_t	*ctxp,
		poid_t          *acct_pdp,
		pin_flist_t     *results_flistp,
		pin_errbuf_t	*ebufp)

{
	time_t              pet = 0;
	time_t              cet = 0;
	int                 ret_status = -1;
	int32		add_gp_flags = 0xFFF;
	int32		status_flags = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;	
	PIN_ERRBUF_CLEAR(ebufp);

	pet = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
	cet = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CYCLE_END_T, 0, ebufp);
	status_flags = *(int32 *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_STATUS_FLAGS, 0, ebufp);

	if (pet > cet && status_flags == add_gp_flags)
	{
		ret_status = 0;	
	}
	else
	{
		ret_status = 1;
	}

	return ret_status;

	/*  Retrieve the Customer Type from Profile Object
	    pp_type = fm_mso_get_cust_type(ctxp, acct_pdp, ebufp);

	    if (pp_type == 2)
	    {
	    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Invalid PP_TYPE Values...");
	    return;
	    }

	//  Retrieve the Customer City from Account Object
	fm_mso_get_custcity(ctxp, acct_pdp, &cust_city, ebufp);
	if (!cust_city)
	{
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CITY Not FOUND... ");
	return;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)cust_city);

	fm_mso_get_grace_config(ctxp, &pp_type, cust_city, &out_flistp, ebufp);

	if (out_flistp)
	{
	grace_days = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_VALID_TO_DETAILS, 1, ebufp);
	if (grace_days)
	{
	calc_days = (int)(pet - cet)/86400;
	if (calc_days > *grace_days)
	{
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Difference Beyond grace days");
	return 0;
	}
	else
	{
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Difference within grace days");
	return 1;
	}

	}
	} */
}


/**************************************************
 * Function: fm_mso_get_cust_type()
 * Retrieves the end customer type from Profile Obj.
 * 
 ***************************************************/
int fm_mso_get_cust_type(
		pcm_context_t		*ctxp,
		poid_t  			*acct_pdp,
		pin_errbuf_t		*ebufp)
{
	pin_flist_t     *search_inflistp = NULL;
	pin_flist_t     *search_outflistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *tmp_flistp = NULL;
	pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *flistp = NULL;
	poid_t          *s_pdp = NULL;
	void            *vp = NULL;

	int64           db = 1;
	char            template[500] = {"select X from /profile/customer_care where F1 = V1 and F2.type = V2 "};
	int32           s_flags = 768;
	int             *pp_type = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);


	db = PIN_POID_GET_DB(acct_pdp);

	// Extracting the Customer's PP_TYPE from profile object
	search_inflistp = PIN_FLIST_CREATE(ebufp);

	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/customer_care", -1, ebufp), ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
	vp = 0;
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, vp, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
	tmp_flistp = PIN_FLIST_SUBSTR_ADD(results_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_PP_TYPE, vp, ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "Profile Search Input Flist", search_inflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Profile Search Output Flist", search_outflistp);

	PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
	if (search_outflistp)
	{
		res_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if (res_flistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Profile Object Found");
			flistp = PIN_FLIST_SUBSTR_GET(res_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);

			pp_type = PIN_FLIST_FLD_GET(flistp, MSO_FLD_PP_TYPE, 1, ebufp);

			if (pp_type)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Customer Type Found...");
				return *pp_type;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Customer Type...");
				//Returning ERROR value
				return 2;
			}
		}
		else
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Profile Object Not Found");
	}

}


/**************************************************
 * Function: fm_mso_get_custcity()
 * Retrieves the end customer city from Account Obj.
 * 
 ***************************************************/
void
fm_mso_get_custcity(
		pcm_context_t   *ctxp,
		poid_t          *acct_obj,
		char            **cust_city,
		pin_errbuf_t    *ebufp)
{
	pin_flist_t     *read_flds_input = NULL;
	pin_flist_t     *read_flds_output = NULL;
	pin_flist_t     *nm_info_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_custcity", ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
	}

	read_flds_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_POID, acct_obj, ebufp);
	nm_info_flistp = PIN_FLIST_ELEM_ADD(read_flds_input, PIN_FLD_NAMEINFO, 1, ebufp);
	PIN_FLIST_FLD_SET(nm_info_flistp, PIN_FLD_CITY, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_custcity READ input list", read_flds_input);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_input, &read_flds_output, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_input, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_custcity READ output list", read_flds_output);

	if (read_flds_output)
	{
		nm_info_flistp = PIN_FLIST_ELEM_GET(read_flds_output, PIN_FLD_NAMEINFO, 1, 0, ebufp);
		*cust_city = PIN_FLIST_FLD_TAKE(nm_info_flistp, PIN_FLD_CITY, 0, ebufp);
	}

	PIN_FLIST_DESTROY_EX(&read_flds_output, NULL);
	return;
}


/**************************************************
 * Function: fm_mso_get_grace_config()
 * Retrieves the config Obj.
 * 
 ***************************************************/
void
fm_mso_get_grace_config(
		pcm_context_t   *ctxp,
		int             *pp_type,
		char            *cust_city,
		pin_flist_t     **out_flistp,
		pin_errbuf_t    *ebufp)
{
	pin_flist_t     *srch_input_flistp = NULL;
	pin_flist_t     *srch_output_flistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *flistp = NULL;
	poid_t          *s_pdp = NULL;

	int64           db = 1;
	char            template[500] = {"select X from /mso_cfg_grace_period_ntf where F1 = V1 and F2 = V2"};
	int32           s_flags = 768;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	srch_input_flistp = PIN_FLIST_CREATE(ebufp);

	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_input_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, cust_city, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PP_TYPE, pp_type, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_VALID_TO_DETAILS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_VALIDITY_IN_DAYS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_FLAGS, 0, ebufp);


	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_cfg_grace_period_ntf Search Input Flist", srch_input_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_input_flistp, &srch_output_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_cfg_grace_period_ntf Search Output Flist", srch_output_flistp);

	PIN_FLIST_DESTROY_EX(&srch_input_flistp, NULL);
	if (srch_output_flistp)
	{
		res_flistp = PIN_FLIST_ELEM_GET(srch_output_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if (res_flistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Config Object Found");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_cfg_grace_period_ntf RESULTS Flist", res_flistp);
			*out_flistp = PIN_FLIST_COPY(res_flistp, ebufp);
		}
		else 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "Config Object Not Found...");

	}

	return;
}


time_t
fm_mso_base_pack_validation(
		pcm_context_t   *ctxp,
		pin_flist_t     *alc_addon_pdts_flistp,
		int32		is_base_pack,
		pin_errbuf_t    *ebufp)
{
	pin_flist_t     *results_flistp = NULL;
	pin_flist_t	*read_iflistp = NULL;
	pin_flist_t	*read_oflistp = NULL;
	pin_flist_t	*usage_flistp = NULL;
	poid_t          *res_pdt_pdp = NULL;
	poid_t          *acct_pdp = NULL;
	time_t          pet = 0;
	time_t          cet = 0;
	char		*event_typep = NULL;
	int             reactivate_flag = 0;
	int		elem_id = 0;
	int32		base_pack_flag = -1;
	pin_cookie_t	cookie = NULL;



	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(alc_addon_pdts_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased Products Individual results flist", results_flistp);
		res_pdt_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
		acct_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

		if (is_base_pack == 0)
		{
			base_pack_flag = fm_mso_catv_pt_pkg_type(ctxp, res_pdt_pdp, ebufp);
			if (base_pack_flag != 0)
			{
				continue;
			}
		}

		read_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, res_pdt_pdp, ebufp);
		usage_flistp = PIN_FLIST_ELEM_ADD(read_iflistp, PIN_FLD_USAGE_MAP, 0, ebufp);
		PIN_FLIST_FLD_SET(usage_flistp, PIN_FLD_EVENT_TYPE, NULL, ebufp);

		PIN_ERR_LOG_FLIST(3, "Product read flds flist", read_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_iflistp, &read_oflistp,  ebufp);

		PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(1, "Error during product readflds");
			PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
			break;
		}

		/*****************************************************************
		 * Check if event type is 30 days event or not 
		 *****************************************************************/
		usage_flistp = PIN_FLIST_ELEM_GET(read_oflistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 1,  ebufp);
		if (usage_flistp)
		{
			event_typep = PIN_FLIST_FLD_TAKE(usage_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);
		}

		pet = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
		cet = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
		if (pet > cet)
		{
			// PET & CET mismatch, hence no need to re-activate.
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " PET > CET");
			//Checking whether the Plan is under Default Grace Mode or not
			reactivate_flag = fm_mso_valid_reactivate(ctxp, acct_pdp, results_flistp, ebufp);
			if (reactivate_flag == 1)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Difference Lesser than the Default Grace Period");
			}
			else
			{
				PIN_ERR_LOG_MSG(3, "Difference More than the Default Grace Period. No need for Re-activation");
				cet = 0;
			}
			break;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " PET <= CET");
			break;
		}
		PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased Products Individual results flist", alc_addon_pdts_flistp);

	if (event_typep)
	{
		PIN_FLIST_FLD_PUT(alc_addon_pdts_flistp, PIN_FLD_EVENT_TYPE, event_typep, ebufp);
	}
	return cet;
}


void
fm_mso_get_service_level_discount(
	pcm_context_t          *ctxp,
	pin_flist_t		*in_flistp,
	int32			dpo_flag,
	pin_flist_t	       **ret_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t             *tax_code_flistp = NULL;
	pin_flist_t             *gst_rate_flistp = NULL;
	pin_flist_t             *plan_flistp = NULL;

	poid_t			*service_pdp = NULL;
	poid_t                  *plan_pdp = NULL;
	poid_t                  *acct_pdp = NULL;
	pin_decimal_t           *total_gst = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t           *base_disc = NULL;
	pin_decimal_t           *discount = NULL;
	pin_decimal_t		*gst_rate = NULL;
	char                    *tax_code = NULL;
	char		msg[1024];
	int32		srch_flags = 768;
	int32		disc_flag = -1;
	int32		all_pkg_type = ALL_PKG_TYPE;
	//	char		template [] = "select X from /mso_cfg_disc_account 1, /account 2 where  1.F1 = 2.F2 and 2.F3 = V3 and 1.F4 = V4 and (1.F5 >= V5 or 1.F6 = V6)";
	char		template [] = "select X from /mso_cfg_disc_service where F1 = V1 and (F2 = V2 or F5 = V5) and (F3 >= V3 or F4 = V4)";
	char		func_name[] = "fm_mso_get_service_level_discount";
	int	        status_failure = 1;
	int             status_success = 0;
	int64           db = 1;
	time_t		current_t = 0;
	time_t		current_date = 0;
	int		infinite_validity = 0;
	void		*vp = NULL;
	pin_decimal_t           *plan_mrp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing", func_name);	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;
	}
	current_t = pin_virtual_time(NULL);
	//current_date = fm_utils_time_round_to_midnight(current_t);
	service_pdp = PIN_FLIST_FLD_GET(in_flistp , PIN_FLD_POID, 0, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	sprintf(msg,"%s: Prepare search flist to search account level discounting config", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	plan_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
	//Check if plan price is zero
	fm_mso_get_plan_price(ctxp, plan_pdp, NULL, &plan_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "54003", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Failed in plan price calculation", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(3, "Plan price flist", plan_flistp);
	plan_mrp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	if(pbo_decimal_is_zero(plan_mrp, ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Zero price plan no need to apply discount");
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, service_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Discount Config Not Available", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);

		goto CLEANUP;
	}

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_PKG_TYPE, args_flistp, MSO_FLD_PKG_TYPE, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, &current_t, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, &infinite_validity, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PKG_TYPE, &all_pkg_type, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DISCOUNT_FLAGS, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DISCOUNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, MSO_FLD_PKG_TYPE, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "account level discounting config iflist:", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()");
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, service_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "account level discounting config oflist:", srch_oflistp);
	
	res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (res_flistp && dpo_flag)
	{	
		res_flistp = srch_oflistp;	
	}

	if (!res_flistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount Config Not Available");
		//	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Discount Config Not Available", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, service_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Discount Config Not Available", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);

		goto CLEANUP;
	}
	else
	{
		/*disc_flag = *(int32 *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_DISCOUNT_FLAGS, 0, ebufp);
		  discount = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
		  if(disc_flag == 1)
		  {
		//block to remove GST tax rate from the MRP beug configured
		plan_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		fm_mso_get_plan_tax_code (ctxp, plan_pdp, tax_code, &tax_code_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERRBUF_RESET(ebufp);
		 *ret_flistp = PIN_FLIST_CREATE(ebufp);
		 PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "54003", ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Failed in Getting Tax Code", ebufp);
		 goto CLEANUP;
		 }
		 if(tax_code_flistp)
		 tax_code = PIN_FLIST_FLD_GET(tax_code_flistp, PIN_FLD_TAX_CODE, 0, ebufp);

		 if (tax_code == NULL) {
		 *ret_flistp = PIN_FLIST_CREATE(ebufp);
		 PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "54003", ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Invastplid Tax Code", ebufp);
		 goto CLEANUP;
		 }
		 mso_retrieve_gst_rate(ctxp, tax_code, acct_pdp, &gst_rate_flistp, ebufp);
		 if (PIN_ERRBUF_IS_ERR(ebufp) || gst_rate_flistp == NULL)
		 {
		 *ret_flistp = PIN_FLIST_CREATE(ebufp);
		 PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "54005", ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during retrieving GST rates", ebufp);
		 goto CLEANUP;
		 }
		 PIN_ERR_LOG_FLIST(3, "GST rates output flist", gst_rate_flistp);
		 gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_IGST_RATE, 1, ebufp);
		 if(!pbo_decimal_is_null(gst_rate, ebufp))
		 {
		 pbo_decimal_add_assign(total_gst, gst_rate, ebufp);
		 }
		 gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_SGST_RATE, 1, ebufp);
		 if(!pbo_decimal_is_null(gst_rate, ebufp))
		 {
		 pbo_decimal_add_assign(total_gst, gst_rate, ebufp);
		 }
		 gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_CGST_RATE, 1,  ebufp);
		 if(!pbo_decimal_is_null(gst_rate, ebufp))
		 {
		 pbo_decimal_add_assign(total_gst, gst_rate, ebufp);
		 }
		 sprintf(msg,"Total GST Rate is : %s ", pbo_decimal_to_str(total_gst, ebufp));
		 PIN_ERR_LOG_MSG(3, msg);
		//Calling function to calculate the Base discount price.
		calc_base_disc(ctxp, discount, total_gst, &base_disc, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
		 *ret_flistp = PIN_FLIST_CREATE(ebufp);
		 PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "54005", ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during Base discount calculation", ebufp);
		 goto CLEANUP;
		 }
		 PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_DISCOUNT, base_disc, ebufp);

		 }*/
		PIN_ERR_LOG_MSG(3, "Discount Available");

	}

	*ret_flistp = PIN_FLIST_COPY(res_flistp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_success, ebufp);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&plan_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_level_discount return flist", *ret_flistp);
	return;
}

void
fm_mso_get_plan_level_discount(
		pcm_context_t           *ctxp,
		pin_flist_t		        *input_flistp,
		pin_flist_t		        **ret_flistp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*tax_code_flistp = NULL;
	pin_flist_t		*gst_rate_flistp = NULL;

	poid_t          	*prod_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	pin_decimal_t           *gst_rate = NULL;
	pin_decimal_t		*total_gst = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*base_disc = NULL;
	pin_decimal_t		*discount = NULL;
	char			*tax_code = NULL;

	char			msg[1024];
	int32			srch_flags = 768;
	char			template [] = "select X from /mso_cfg_disc_plan where  F1 = V1 and upper(F2) = upper(V2) and F3 = V3 and F4 = V4 and upper(F5) = upper(V5) and (F6 >= V6 or F7 = V7)";
	char			func_name[] = "fm_mso_get_plan_level_discount";
	int32			status_failure = 1;
	int32			status_success = 0;
	int64			db = 1;
	int			infinite_validity = 0;
	int32			disc_flag = 0;

	void			*vp = NULL;
	time_t			current_t = 0;
	time_t			current_date = 0;


	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing", func_name);	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Product level discounting config input flist:", input_flistp);

	current_t = pin_virtual_time(NULL);
	current_date = fm_utils_time_round_to_midnight(current_t);

	sprintf(msg,"%s: Prepare search flist to search product level discounting config", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	acct_pdp = PIN_FLIST_FLD_GET(input_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_PLAN_OBJ, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_CITY, args_flistp, MSO_FLD_CITY, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_PP_TYPE, args_flistp, MSO_FLD_PP_TYPE, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_CONN_TYPE, args_flistp, PIN_FLD_CONN_TYPE, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_COPY(input_flistp, MSO_FLD_DAS_TYPE, args_flistp, MSO_FLD_DAS_TYPE, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, &current_date, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, &infinite_validity, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DISCOUNT_FLAGS, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DISCOUNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_NAME, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Product level discounting config iflist:", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()");
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Product level discounting config oflist:", srch_oflistp);
	res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

	if (!res_flistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount Config Not Available");
		//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Discount Config Not Available", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "0000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Discount Config Not Available", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);

		goto CLEANUP;
	}
	else
	{	
		/*	
			disc_flag = *(int32 *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_DISCOUNT_FLAGS, 0, ebufp);
			discount = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
			if(disc_flag == 1)
			{
		//block to remove GST tax rate from the MRP beug configured 
		plan_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		fm_mso_get_plan_tax_code (ctxp, plan_pdp, tax_code, &tax_code_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERRBUF_RESET(ebufp);
		 *ret_flistp = PIN_FLIST_CREATE(ebufp);
		 PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "54003", ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Failed in Getting Tax Code", ebufp);
		 goto CLEANUP;
		 }
		 if(tax_code_flistp)
		 tax_code = PIN_FLIST_FLD_GET(tax_code_flistp, PIN_FLD_TAX_CODE, 0, ebufp);

		 if (tax_code == NULL) {
		 *ret_flistp = PIN_FLIST_CREATE(ebufp);
		 PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "54003", ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Invastplid Tax Code", ebufp);
		 goto CLEANUP;
		 }

		 mso_retrieve_gst_rate(ctxp, tax_code, acct_pdp, &gst_rate_flistp, ebufp);	
		 if (PIN_ERRBUF_IS_ERR(ebufp) || gst_rate_flistp == NULL)
		 {
		 *ret_flistp = PIN_FLIST_CREATE(ebufp);
		 PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "54005", ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during retrieving GST rates", ebufp);
		 goto CLEANUP;
		 }
		 PIN_ERR_LOG_FLIST(3, "GST rates output flist", gst_rate_flistp);
		 gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_IGST_RATE, 1, ebufp);
		 if(!pbo_decimal_is_null(gst_rate, ebufp))
		 {
		 pbo_decimal_add_assign(total_gst, gst_rate, ebufp);
		 }
		 gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_SGST_RATE, 1, ebufp);
		 if(!pbo_decimal_is_null(gst_rate, ebufp))
		 {
		 pbo_decimal_add_assign(total_gst, gst_rate, ebufp);
		 }
		 gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_CGST_RATE, 1,  ebufp);
		 if(!pbo_decimal_is_null(gst_rate, ebufp))
		 {
		 pbo_decimal_add_assign(total_gst, gst_rate, ebufp);
		 }

		 sprintf(msg,"Total GST Rate is : %s ", pbo_decimal_to_str(total_gst, ebufp));	
		 PIN_ERR_LOG_MSG(3, msg);
		//Calling function to calculate the Base discount price.
		calc_base_disc(ctxp, discount, total_gst, &base_disc, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
		 *ret_flistp = PIN_FLIST_CREATE(ebufp);
		 PIN_FLIST_FLD_COPY(input_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_failure, ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "54005", ebufp);
		 PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error during Base discount calculation", ebufp);
		 goto CLEANUP;
		 }
		 PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_DISCOUNT, base_disc, ebufp);

	}*/
	PIN_ERR_LOG_MSG(3, "Discount Available");			
	}

	*ret_flistp = PIN_FLIST_COPY(res_flistp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_success, ebufp);

CLEANUP:
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_plan_level_discount return flist", *ret_flistp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	if(!pbo_decimal_is_null(base_disc, ebufp))
	{
		pbo_decimal_destroy(&base_disc);
	}

}

/**************************************************
 * Function: fm_mso_get_das_type()
 * Retrieves the end customer DAS TYPE from Account Obj.
 * 
 ***************************************************/
void
fm_mso_get_das_type(
		pcm_context_t   *ctxp,
		poid_t          *acct_pdp,
		char            **das_type,
		pin_errbuf_t    *ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t     *cc_flistp = NULL;

	char			msg[1024];
	int32			srch_flags = 768;
	char			template[] = "select X from /profile/customer_care where  F1 = V1 and F2.type = V2 ";
	char			func_name[] = "fm_mso_get_das_type";
	int32		    status_failure = 1;
	int32           status_success = 0;
	int64           	db = 1;

	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing", func_name);	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;
	}

	sprintf(msg,"%s: Prepare search flist to search das type", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/customer_care", -1, ebufp), ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	cc_flistp = PIN_FLIST_SUBSTR_ADD(result_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
	PIN_FLIST_FLD_SET(cc_flistp, MSO_FLD_DAS_TYPE, "", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DAS TYPE Search iflist:", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DAS TYPE Search oflist:", srch_oflistp);

	res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (res_flistp)
	{

		cc_flistp = PIN_FLIST_SUBSTR_GET(res_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
		*das_type = PIN_FLIST_FLD_TAKE(cc_flistp, MSO_FLD_DAS_TYPE, 1, ebufp);

	}

	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

}

int32
fm_mso_get_current_pkd_count(
		pcm_context_t		*ctxp,
		poid_t			*srvc_pdp,
		int32			pkg_type,
		pin_errbuf_t		*ebufp)
{

	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*cc_flistp = NULL;

	char			msg[1024];
	int32			srch_flags = 768;
	char			template[] = "select X from /purchased_product 1, /product 2, /mso_cfg_catv_pt 3 where 1.F1 = V1 and 1.F2 = 2.F3 and 2.F4 = 3.F5 and 3.F6 = V6 and 1.F7 != V7";
	char			func_name[] = "fm_mso_get_current_pkd_count";
	int32			count = 0;
	int			active = 1;
	int			inactive = 2;
	int			cancelled = 3;
	int64           	db = 1;

	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)){
		sprintf(msg,"%s: Error before processing", func_name);	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, msg, ebufp);
		return;
	}

	sprintf(msg,"%s: Prepare search flist to search das type", func_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, srvc_pdp, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, srvc_pdp, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &cancelled, ebufp );

	result_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, srvc_pdp, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased Product Search iflist:", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased Product Search oflist:", srch_oflistp);

	count = PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp);

	sprintf(msg,"count: %d", count);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

	return count;

}

void fm_mso_future_dated_renewal(
		pcm_context_t *ctxp,
		pin_flist_t    *in_flistp,
		pin_errbuf_t    *ebufp)
{
	pin_flist_t     *wrt_iflistp = NULL;
	pin_flist_t     *wrt_oflistp = NULL;
	pin_flist_t     *srch_i_flistp = NULL;
	pin_flist_t     *srch_o_flistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *cycle_flistp = NULL;
	pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *results_flistp = NULL;
	poid_t          *srch_pdp = NULL;
	poid_t          *pp_pdp = NULL;
	int32           srch_flags = 256;
	time_t          charged_to_t = 0;
	time_t          actg_next_t = 0;
	time_t          actg_last_t = 0;
	time_t          current_t = 0;
	char            *srch_template = "select X from /billinfo where F1 = V1";
	char		*event_typep = NULL;

	current_t = pin_virtual_time(NULL);

	PIN_ERR_LOG_FLIST(3, "Enterting future dated renewal check", in_flistp);

	/*srch_i_flistp  = PIN_FLIST_CREATE(ebufp);
	  pp_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	  srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(pp_pdp), "/search", -1, ebufp);
	  PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	  PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, srch_template, ebufp);
	  PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	  args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	  PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	  res_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
	  PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	  PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACTG_NEXT_T, NULL, ebufp);
	  PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACTG_LAST_T, NULL, ebufp);

	  PIN_ERR_LOG_FLIST(3, "Search input flistp", srch_i_flistp);

	  PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp, &srch_o_flistp, ebufp);
	  if (PIN_ERRBUF_IS_ERR(ebufp))
	  {
	  PIN_ERR_LOG_FLIST(1, "Error during billinfo search", srch_o_flistp);

	  }
	  PIN_ERR_LOG_FLIST(3, "Search output flistp", srch_o_flistp);
	  results_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	  if(results_flistp)
	  {
	  actg_next_t = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_ACTG_NEXT_T, 0, ebufp);
	  actg_last_t = *(time_t *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_ACTG_LAST_T, 0, ebufp);
	  }
	  else
	  {
	  PIN_ERR_LOG_MSG(1, "Billinfo doesnt exist");
	  goto CLEANUP;
	  }
	 */
	cycle_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_CYCLE_FEES, 1, 1, ebufp);

	/***************************************************************************
	 * Check if bill cycle is 31 days, then reduce one day from actg_next_t
	 * to trigger charges for future 
	 ***************************************************************************
	 event_typep = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);
	 if (strstr(event_typep, "cycle_forward_thirty_days") &&
	 ((actg_next_t - actg_last_t)/86400 > 30))
	 {
	 actg_next_t = actg_next_t - 86400;
	 }*/
	if(cycle_flistp)
	{
		charged_to_t = *(time_t *)PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);
		if(charged_to_t >= current_t)
		{
			PIN_ERR_LOG_MSG(3, "Future dated renewal");
			wrt_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(in_flistp , PIN_FLD_POID, wrt_iflistp, PIN_FLD_POID, ebufp);
			res_flistp = PIN_FLIST_ELEM_ADD(wrt_iflistp, PIN_FLD_CYCLE_FEES, 1, ebufp);
			PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CHARGED_TO_T, &current_t, ebufp);
			//PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CYCLE_FEE_END_T, &current_t, ebufp);

			PIN_ERR_LOG_FLIST(3, "Write fields to current date", wrt_iflistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wrt_iflistp, &wrt_oflistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_FLIST(1, "Error during in write fields", wrt_oflistp);

			}
			PIN_FLIST_DESTROY_EX(&wrt_iflistp, NULL)
				PIN_FLIST_DESTROY_EX(&wrt_oflistp, NULL);
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(1, "Cycle info flistp not available");
	}
CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);


	return;

}

void fm_mso_fetch_deal_disc(
		pcm_context_t	*ctxp,
		pin_flist_t	*in_flistp,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t	*srch_i_flistp = NULL;
	pin_flist_t	*srch_o_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*deal_prod_flp = NULL;
	pin_flist_t	*srch_res_flistp = NULL;
	pin_flist_t	*prods_flistp = NULL;
	poid_t		*srch_pdp = NULL;
	int32		db_no = 1;
	int32		srch_flag = 256;
	char		*template = "select X from /deal where 1.F1= V1 and 1.F2=V2";


	srch_pdp = PIN_POID_CREATE(db_no, "/search", -1, ebufp);
	srch_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp , PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp , PIN_FLD_TEMPLATE, template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DEAL_OBJ, args_flistp, PIN_FLD_POID, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp);
	deal_prod_flp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PRODUCT_OBJ, deal_prod_flp, PIN_FLD_PRODUCT_OBJ, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(3, "search input flist for deal ", srch_i_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp, &srch_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_FLIST(1, "Error during seacrh_deal ", srch_o_flistp);
	}

	PIN_ERR_LOG_FLIST(3, "search output flist for deal ", srch_i_flistp);
	srch_res_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp, PIN_FLD_RESULTS, 0, 0,  ebufp);
	prods_flistp = PIN_FLIST_ELEM_GET(srch_res_flistp, PIN_FLD_PRODUCTS, 0, 0, ebufp);

	PIN_FLIST_FLD_COPY(prods_flistp, PIN_FLD_CYCLE_DISCOUNT, in_flistp, PIN_FLD_CYCLE_DISCOUNT, ebufp);
	PIN_FLIST_FLD_COPY(prods_flistp, PIN_FLD_PURCHASE_DISCOUNT, in_flistp, PIN_FLD_PURCHASE_DISCOUNT, ebufp);
	PIN_FLIST_FLD_COPY(prods_flistp, PIN_FLD_USAGE_DISCOUNT, in_flistp, PIN_FLD_USAGE_DISCOUNT, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);	
	return;
}

int32
fm_validate_prov_ca_id(
		pcm_context_t   *ctxp,
		poid_t          *srvc_pdp,
		char            *prov_tag,
		pin_errbuf_t    *ebufp)
{
	pin_flist_t     *read_iflistp = NULL;
	pin_flist_t     *read_oflistp = NULL;
	pin_flist_t     *srvc_flistp = NULL;
	pin_flist_t     *ca_flistp = NULL;
	char            *ne = NULL;
	char            *ca_id = NULL;
	int32		ret_status = 1;
	int32		pkg_type = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return ret_status;	
	PIN_ERRBUF_CLEAR(ebufp);

	read_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, srvc_pdp, ebufp);
	srvc_flistp = PIN_FLIST_SUBSTR_ADD(read_iflistp, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_FLD_SET(srvc_flistp, MSO_FLD_NETWORK_NODE, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_validate_prov_ca_id rflds input flist", read_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_iflistp, &read_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_validate_prov_ca_id rflds output flist", read_oflistp);

	srvc_flistp = PIN_FLIST_SUBSTR_GET(read_oflistp, MSO_FLD_CATV_INFO, 0, ebufp);
	ne = PIN_FLIST_FLD_GET(srvc_flistp, MSO_FLD_NETWORK_NODE, 0, ebufp);
    if( prov_tag && ne)
    {    
	mso_prov_get_ca_id(ctxp, prov_tag, ne, &ca_flistp, ebufp);
	PIN_ERR_LOG_FLIST(3, "CA_ID outputflistp", ca_flistp);
	if(ca_flistp)
	{
		ca_id = PIN_FLIST_FLD_GET(ca_flistp, MSO_FLD_CA_ID, 0, ebufp);
		if (strcmp(ca_id, "NA")==0 || strcmp(ca_id, "NA1")==0 || strcmp(prov_tag, "PROV_NCF_PRO") == 0)
		{
			pkg_type = *(int32 *) PIN_FLIST_FLD_GET(ca_flistp, MSO_FLD_PKG_TYPE, 0, ebufp);
			if (pkg_type != 0 )
			{
				ret_status = 0;
			}
			else
			{
				ret_status = 2;
			}
			goto CLEANUP;
		}
	}
    }
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fm_validate_prov_ca_id", ebufp);
	}

CLEANUP :
	PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&ca_flistp, NULL);
	return ret_status;
}
/**
 *Validation for Tal_IP plan addition
 */
static int  fm_mso_validate_tal_plan_addition(
		pcm_context_t   *ctxp,
		pin_flist_t     *in_flistp,
		pin_flist_t     **r_flistpp,
		pin_errbuf_t    *ebufp)
{
	pin_flist_t     *result_flistp = NULL;
	pin_flist_t     *svc_details = NULL;
	pin_flist_t     *tal_details  = NULL;
	pin_flist_t     *purch_prod_flistp = NULL;
	pin_flist_t     *offering_flistp = NULL;
	pin_flist_t     *plans_flistp = NULL;
	time_t          purch_end_t = 0;
	time_t          current_t = 0;
	poid_t          *inp_pdp = NULL;
	poid_t          *plan_pdp = NULL;
	poid_t          *act_pdp = NULL;
	poid_t          *srvc_pdp = NULL;
	int32           *indicator;
	int32           *plan_type;
	int32           *is_tal;
	int32           status_uncommitted =1;
	int32           *bill_when_svc = NULL;
	int32           *bill_when_tal = NULL;;
	int32           ret_plan_type=PLAN_TYPE_TAL;
    char            *str_rolep = NULL;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside fm_mso_validate_tal_plan_addition ");

	inp_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	act_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	//First check the plan is Tal plan
	fm_mso_validate_tal_plan(ctxp,in_flistp,&tal_details,ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Response from fm_mso_validate_tal_plan");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Plan to be added is Tal plan",tal_details);
	if (tal_details) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Response from fm_mso_validate_tal_plan 1.1 ");
		plan_pdp = PIN_FLIST_FLD_GET(tal_details, PIN_FLD_PLAN_OBJ, 1, ebufp);
		plan_type = PIN_FLIST_FLD_GET(tal_details, MSO_FLD_PLAN_TYPE, 1, ebufp);
		bill_when_tal = PIN_FLIST_FLD_GET(tal_details, PIN_FLD_BILL_WHEN, 1, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Response from fm_mso_validate_tal_plan 1.2 ");
		if (plan_pdp) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Plan to be added is Tal plan");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Validate customer for IS_TAL flag");
			fm_mso_get_srvc_info(ctxp,in_flistp,&svc_details,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Service details",svc_details);
			is_tal = (int *)PIN_FLIST_FLD_GET(svc_details, MSO_FLD_ISTAL_FLAG, 1, ebufp );
            str_rolep = PIN_FLIST_FLD_GET(svc_details, MSO_FLD_ROLES, 0, ebufp);
			indicator = (int *)PIN_FLIST_FLD_GET(svc_details,PIN_FLD_INDICATOR, 1, ebufp);
			bill_when_svc = (int *)PIN_FLIST_FLD_GET(svc_details,PIN_FLD_BILL_WHEN, 1, ebufp);
			if (!strstr(str_rolep,"-Static") && *is_tal == 0) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"You cannot add Tal plan on a NON_TAL customer");
				*r_flistpp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
				PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "84100", ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "You cannot add Tal plan on a NON_TAL customer", ebufp);

				return 2;
			} else {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check for postpaid and prepaid plan validation");
				if (*indicator == *plan_type) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"postpaid and prepaid plan validation success");
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Check for payment term validation");
					if (*bill_when_svc == *bill_when_tal) {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"payment term validation success");
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Proceed to add requested Tal plan");
						PIN_FLIST_FLD_SET(in_flistp,MSO_FLD_PLAN_TYPE,&ret_plan_type, ebufp);
						//Find out existing base plan period.Valid for prepaid accounts only
						current_t = pin_virtual_time(NULL);
						if (*indicator == PAYINFO_BB_PREPAID) {
							fm_mso_get_subscr_purchased_products(ctxp, act_pdp, srvc_pdp, &purch_prod_flistp, ebufp);
							if(purch_prod_flistp){
								//Get first product which is newly added
								offering_flistp = PIN_FLIST_ELEM_GET(purch_prod_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
								if(offering_flistp){
									purch_end_t = *(time_t *)PIN_FLIST_FLD_GET(offering_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
									if (purch_end_t >= current_t) {
										plans_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN_LISTS, 0, 1,ebufp);
										PIN_FLIST_FLD_SET(plans_flistp,PIN_FLD_PURCHASE_END_T,&purch_end_t, ebufp);                                                                                             return 1;
									} else {
										*r_flistpp = PIN_FLIST_CREATE(ebufp);
										PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
										PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
										PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "84107", ebufp);
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,                                                                                                                                                          " base plan validity is expired.Plan cannot be added");
										PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR,                                                                                                                                           "base plan validity is expired.Plan cannot be added", ebufp);
										return 2;

									}

								} else {
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error while getting active subscr product details");
									*r_flistpp = PIN_FLIST_CREATE(ebufp);
									PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
									PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
									PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51158", ebufp);
									PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR,                                                                                                                                             "Error in getting active product details", ebufp);
									return 2;

								}
							} else {
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error while getting active subscr product details");
								*r_flistpp = PIN_FLIST_CREATE(ebufp);
								PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
								PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
								PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51158", ebufp);
								PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Error in getting active product details", ebufp);

								return 2;

							}
						}
						return 1;
					}else {
						*r_flistpp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "84103", ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Payment term do not match with base plan");
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Payment term do not match with base plan", ebufp);
						return 2;
					}

				} else  {
					*r_flistpp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
					PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "84100", ebufp);
					if(*indicator == PAYINFO_BB_POSTPAID) {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"You cannot add Prepaid Tal Plan on Postpaid Tal Customer");
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "You cannot add Prepaid Tal Plan on Postpaid Tal Customer", ebufp);
					}
					if(*indicator == PAYINFO_BB_PREPAID) {
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"You cannot add Postpaid Tal Plan on Prepaid Tal Customer");
						PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "You cannot add Postpaid Tal Plan on Prepaid Tal Customer", ebufp);
					}
					return 2;
				}

			}

		} else {

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Plan poid is null");
		}
	} else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Plan to be added is Not tal.Proceeding to add the plan");
		return 1;
	}


	return 1;
}

/*******************************************************************
 * fm_mso_cust_add_gb_notification 
 * 
 * This function checks if the Plan being added is related to any
 * of the additonal GB plans and triggers the SMS / Email 
 * notifications
 *******************************************************************/	
static void fm_mso_cust_add_gb_notification(
		pcm_context_t           *ctxp,
		time_t					*grant_valid_from_date,
		time_t					*grant_valid_to_date,
		pin_decimal_t			*grant_amount,
		pin_flist_t             *in_flist,
		pin_flist_t             **ret_flistp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*service_info = NULL;
	pin_flist_t		*sms_in_flist = NULL;
	pin_flist_t		*sms_out_flist = NULL;
	pin_flist_t     	*tmp_flistp = NULL;
	pin_flist_t		*event_flistp = NULL;
	
	poid_t			*acct_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	
	char			*plan_name = NULL;
	char			*srch_template = "select X from /mso_cfg_fup_gb_plan where F1 = V1 " ;
	char			*additional_gb = NULL;
	char			*expiry_date = NULL;
	char            	*contact_no = NULL;
    char            	*email_addr = NULL;
	char			*gb_grant_str = NULL;
	
	int			elem_base = 0;
	int32			srch_flag = 512;
	int64			db = 1;
	int32			action_flag = NTF_EXTRA_GB_TOPUP;
	
	pin_decimal_t	*gb = pbo_decimal_from_str("1024", ebufp);
	pin_decimal_t	*gb_grant = NULL;
	
	pin_cookie_t	cookie_base = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_gb_notification :");
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_gb_notification:  input list", in_flist);
	
	acct_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 0, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Checking if the service is Broadband...");
	
	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp), MSO_BB)) == 0 )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service is Broadband");
		
		while ((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flist, PIN_FLD_PLAN, &elem_base, 1, &cookie_base, ebufp )) != NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_gb_notification:  Plan list", plan_flistp);
			
			plan_pdp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );

			if (!PIN_POID_IS_NULL(plan_pdp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan Poid is not NULL...");
				
				srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

				srch_in_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
				PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
				PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, srch_template , ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

				result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
								
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_gb_notification search input flist", srch_in_flistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
					PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
					PIN_ERRBUF_RESET(ebufp);						
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_gb_notification search output flist", srch_out_flistp);

				if (srch_out_flistp != NULL)
				{
					result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
					if (result_flist != NULL)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan name found in the Additional GB Config table");
						
						plan_name  = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_NAME, 1, ebufp );
						additional_gb  = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_VALUE, 1, ebufp);
						
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Fetching the Account Information...");
						fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
						
						if(acnt_info != NULL)
						{
							contact_no = PIN_FLIST_FLD_TAKE(acnt_info, MSO_FLD_RMN, 0, ebufp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "contact_no >> ");
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, contact_no);
							tmp_flistp = PIN_FLIST_ELEM_TAKE(acnt_info, PIN_FLD_NAMEINFO, 1, 0, ebufp);
							email_addr = PIN_FLIST_FLD_TAKE(tmp_flistp, PIN_FLD_EMAIL_ADDR, 0, ebufp);	
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "email_addr >> ");
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, email_addr);
						}
						
						PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
						
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before Preparing SMS Notification Input Flist...");
						
						if ( plan_name && strlen(plan_name) != 0 && 
							additional_gb && strlen(additional_gb) != 0 &&
							contact_no && strlen(contact_no) != 0 &&
							grant_valid_to_date && *grant_valid_to_date > 0 )
						{
							if(!pbo_decimal_is_null(grant_amount, ebufp))
							{
								grant_amount = pbo_decimal_negate(grant_amount, ebufp);
								gb_grant = pbo_decimal_round(pbo_decimal_divide(grant_amount, gb, ebufp), 0, ROUND_DOWN, ebufp);
               
								if (!pbo_decimal_is_null(gb_grant, ebufp))
								{
									gb_grant_str = pbo_decimal_to_str(gb_grant, ebufp);
									
									if (gb_grant_str && strlen(gb_grant_str) != 0 && (strcmp(gb_grant_str, additional_gb) != 0))
									{
										PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Grant configured in the Plan and given to the account not matching...");
										strcpy(additional_gb, gb_grant_str);
									}
								}								
							}
							
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Preparing SMS Notification Input Flist...");						
							sms_in_flist = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(sms_in_flist, PIN_FLD_POID, acct_pdp, ebufp);
							PIN_FLIST_FLD_SET(sms_in_flist, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "contact_no >> ");
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, contact_no);
							PIN_FLIST_FLD_SET(sms_in_flist, PIN_FLD_PHONE, contact_no, ebufp);
							PIN_FLIST_FLD_SET(sms_in_flist, PIN_FLD_FLAGS, &action_flag, ebufp);
							PIN_FLIST_FLD_SET(sms_in_flist, PIN_FLD_NAME, plan_name, ebufp);
							PIN_FLIST_FLD_SET(sms_in_flist, PIN_FLD_VALUE, additional_gb, ebufp);
							// Reducing one sec from the cycle expiry date
							*grant_valid_to_date = *grant_valid_to_date - 1;
							PIN_FLIST_FLD_SET(sms_in_flist, PIN_FLD_END_T, grant_valid_to_date, ebufp);
														
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_gb_notification SMS input flist", sms_in_flist);
							PCM_OP(ctxp, MSO_OP_NTF_CREATE_SMS_NOTIFICATION, 0, sms_in_flist, &sms_out_flist, ebufp);
							PIN_FLIST_DESTROY_EX(&sms_in_flist, NULL);
							
							if (PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in sending SMS Notification", ebufp);
								PIN_ERRBUF_RESET(ebufp);				
								PIN_FLIST_DESTROY_EX(&sms_out_flist, NULL);	
								return;								
							}
							
							if(sms_out_flist != NULL)
							{
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
										"fm_mso_cust_add_gb_notification output flist", sms_out_flist);
								PIN_FLIST_DESTROY_EX(&sms_out_flist, NULL);	
							}								
						}													
					}
				}
			}
		}	
	}
	
	return;
}


/*************************************************************************
 * fm_mso_cust_get_grant_expiry_date 
 * 
 * This function gets the active base plan based on the account object,
 * service object and plan object and calculates the grant expiry date
 *************************************************************************/		
void fm_mso_cust_get_grant_expiry_date(
	pcm_context_t           *ctxp,
	poid_t			*account_obj,
	poid_t                  *service_obj,
	poid_t                  *plan_obj,
	poid_t			*mso_pp_obj,
	pin_flist_t             **ret_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t     *readobj_oflistp = NULL;
	pin_flist_t	*prod_flistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *result_flistp = NULL;
	pin_flist_t     *search_flistp = NULL;
	pin_flist_t     *search_res_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;

	char            *search_template = " select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 and F4 = V4 and F5 = V5 ";
	char		*vp = NULL;
	
	int32           db = -1;
	int             search_flags = 512;
	int32		prod_status = 1;
	int		elem_id = 0;
	
	pin_cookie_t	cookie = NULL;
	
	poid_t          *search_pdp = NULL;
	poid_t		*offering_obj = NULL;
		
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "@ fm_mso_cust_get_grant_expiry_date...");
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Reading mso_pp_obj... ");
	
	if (!PIN_POID_IS_NULL(mso_pp_obj))
	{
		fm_mso_utils_read_any_object(ctxp, mso_pp_obj, &readobj_oflistp, ebufp); 
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_get_grant_expiry_date : Error in reading purchased plan object", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&readobj_oflistp, NULL);
			return;
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Offering Object Read Output flist", readobj_oflistp);
				
		if (readobj_oflistp != NULL)
		{
			while ((prod_flistp = PIN_FLIST_ELEM_GET_NEXT(readobj_oflistp, PIN_FLD_PRODUCTS, 
						&elem_id, 1, &cookie, ebufp )) != NULL)
			{
				vp = (char *)PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
				if(vp && strlen(vp) > 0) 
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Prov tag is");
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, strdup(vp));
					offering_obj = PIN_FLIST_FLD_GET(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 1, ebufp);				
					break;
				}
			}
		}
		PIN_FLIST_DESTROY_EX(&readobj_oflistp, NULL);
	}
	
	if (!PIN_POID_IS_NULL(offering_obj))
	{
		fm_mso_utils_read_any_object(ctxp, offering_obj, &readobj_oflistp, ebufp); 
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_cust_get_grant_expiry_date : Error in reading Offering object", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&readobj_oflistp, NULL);
			return;
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Offering Object Read Output flist", readobj_oflistp);
		
		if(readobj_oflistp != NULL)
		{
			*ret_flistp = PIN_FLIST_COPY(readobj_oflistp, ebufp);			
			PIN_FLIST_DESTROY_EX(&readobj_oflistp, NULL);
		}
	}
	
	return;
}

