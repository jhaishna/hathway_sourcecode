/**************************************************************************************
 * File:	fm_mso_utils_lifecycle.c
 * Opcode:	None
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description:  Common functions for "fm_mso_cust" module. The
 *               functions should be moved to global common 
 *               place if other modules need to use the same function
 *
 * Modification History: 
 * DATE     |  CHANGE                                           |  Author
 ***********|***************************************************|**********************
 |26/12/2019|Added functionality for creating Life Cycle Events | Jyothirmayi Kachiraju
            |for Tagging and Detagging of Hybrid Accounts       | 
 |13/02/2020|Changes to include the Termination reason type,    | Jyothirmayi Kachiraju
            |subtype in the Life Cycle Events for CATV Service  |
            |Termination cases                                  |
 |**********|***************************************************|**********************
 **************************************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_utils_common_functions.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "pin_pymt.h"
#include "mso_cust.h"
#include "mso_device.h"
#include "mso_operations.h"
#include "mso_ops_flds.h"
#include "mso_lifecycle_id.h"
#include "mso_utils.h"

#define PAY_TYPE_CASH       10011
#define PAY_TYPE_CHECK      10012
#define PAY_TYPE_ONLINE     10013
#define PAY_TYPE_TDS        10018
#define PAY_TYPE_SP_AUTOPAY 10019

#define PAYMENT_COLLECT		"PAYMENT_COLLECT"
#define BAL_TRANS_LCO_SOURCE	"BAL_TRANS_LCO_SOURCE"
#define BAL_TRANS_LCO_DEST	"BAL_TRANS_LCO_DEST"

#define PAYMENT_REVERSAL	"PAYMENT_REVERSAL"
#define CQ_BOUNCE_PENALTY	"CQ_BOUNCE_PENALTY"
#define BAL_TRANS_LCO_SOURCE	"BAL_TRANS_LCO_SOURCE"
#define BAL_TRANS_LCO_DEST	"BAL_TRANS_LCO_DEST"

#define EVENT_ADJUSTMENT "EVENT_ADJUSTMENT"
#define BILL_ADJUSTMENT "BILL_ADJUSTMENT"
#define ITEM_ADJUSTMENT "ITEM_ADJUSTMENT"
#define ACCOUNT_ADJUSTMENT "ACCOUNT_ADJUSTMENT"

#define CATV_MAIN        0
#define CATV_ADDITIONAL  1
#define FLAG_ADD_PLAN    1
#define FLAG_DEL_PLAN    0

#define	DELIVERY_PREF_EMAIL    0
#define	DELIVERY_PREF_POST     1

#define	INDICATOR_POSTPAID     0
#define	INDICATOR_ADVANCE      1
#define	INDICATOR_PREPAID      2

#define	DEFAULT		0
#define	NOT_RECEIVED	1

#define EVENT_DISPUTE "EVENT_DISPUTE"
#define BILL_DISPUTE "BILL_DISPUTE"
#define ITEM_DISPUTE "ITEM_DISPUTE"
#define EVENT_SETTLEMENT "EVENT_SETTLEMENT"
#define BILL_SETTLEMENT "BILL_SETTLEMENT"
#define ITEM_SETTLEMENT "ITEM_SETTLEMENT"

/*
MSO_FREE_MB    1100010
MSO_TRCK_USG   1000103
MSO_FUP_TRACK  1000104
*/

PIN_IMPORT void
fm_mso_get_acnt_from_acntno(
	pcm_context_t	*ctxp,
	char			*acnt_no,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_EXPORT void
get_strings_template(
    pcm_context_t           *ctxp,
    int64               db,
    char                *domainp,
    int32               string_id,
    int32               string_version,
    char                **lc_template,
    pin_errbuf_t            *ebufp);	
/***********************************************
* Function Declaration -Start
***********************************************/

void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

void 
fm_mso_convert_epoch_to_date(
	time_t		epoch_time,
	char		*buf);
void
fm_mso_get_device_type_from_poid(
	pcm_context_t		*ctxp,
	poid_t			*device_pdp,
	char			*device_type,
	pin_errbuf_t		*ebufp);

void get_receipt_cheque_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*r_flistp,
	char			*cheque_num,
	pin_errbuf_t		*ebufp);

void get_base_plan_from_mso_purchased_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*iflist,
	pin_flist_t		**o_flistp,
	pin_errbuf_t		*ebufp);

void
mso_op_payment_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*s_oflistp,
	pin_errbuf_t		*ebufp);

void
mso_op_pymt_reverse_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *s_oflistp,
	pin_errbuf_t            *ebufp);

void
mso_op_deposit_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *s_oflistp,
	pin_errbuf_t            *ebufp);

void
mso_op_refund_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *s_oflistp,
	pin_errbuf_t            *ebufp);

void
fm_mso_cust_actv_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*actvn_out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_react_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_susp_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_term_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_change_plan_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_modify_actv_plan_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_add_plan_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_cancel_plan_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_mod_srvc_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*acnt_info,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_mod_cust_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*acnt_info,
	pin_errbuf_t		*ebufp);

void
fm_mso_refund_reverse_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*acnt_info,
	pin_errbuf_t		*ebufp);

void
fm_mso_cr_dr_note_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
        pin_flist_t             *au_flistp,
	pin_errbuf_t            *ebufp);

void 
mso_op_adjustment_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *s_oflistp,
	pin_errbuf_t            *ebufp);

void 
fm_mso_topup_prepaid_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *s_oflistp,
	pin_errbuf_t            *ebufp);

void
fm_mso_renew_prepaid_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*bal_flistp,
	pin_errbuf_t            *ebufp);

void
fm_mso_cust_renew_fdp_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_mod_payinfo_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_deact_validity_quota_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_swap_device_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_allocate_bb_hw_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_hold_srvc_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);


void
fm_mso_unhold_srvc_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);


void
mso_op_grv_upload_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp);

void
mso_cust_bu_reg_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_create_bb_srvc_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);


void
fm_mso_activate_bb_srvc_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_modify_tax_exemp_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_update_hierarchy_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_update_business_type_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_modify_bb_service_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_modify_csr_roles_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);


void
fm_mso_modify_billinfo_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_modify_self_care_cred_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_activate_deactivate_bu_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_modify_csr_access_control_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
mso_op_billing_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_device_diassociate_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_modify_csr_ar_limit_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
mso_op_prov_order_resubmit_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
mso_op_cust_create_schedule_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
mso_op_dispute_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
mso_op_dispute_settlement_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
mso_op_auto_renew_plan_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
mso_op_cust_add_remove_ip_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_fup_topup_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_add_additional_mb_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_extend_validity_mb_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);


void
fm_mso_resub_susp_evt_mb_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);


void
fm_mso_update_bu_profile_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_apply_late_fee_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_fup_downgrade_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_ops_add_mod_del_action_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_service_tech_change_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *o_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_cust_swap_tagging_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             *out_flistp,
	pin_errbuf_t            *ebufp);

void
fm_mso_cust_transfer_subscr_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *in_flistp,
	pin_flist_t             *out_flistp,
	pin_errbuf_t            *ebufp);
void
fm_mso_cust_create_offers_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_create_gst_info_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_update_gst_info_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t   		*ebufp); 

void
fm_mso_update_poi_poa_lc_event(
        pcm_context_t   *ctxp,
        pin_flist_t     *in_flistp,
        pin_flist_t     *out_flistp,
        pin_errbuf_t    *ebufp);

void
fm_mso_cust_create_netflix_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp);
void
fm_mso_netfr_gen_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);

// Function for generating Life Cycle Event for Tagging Hybrid Accounts
void
fm_mso_tag_hybrid_acct_gen_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);

// Function for generating Life Cycle Event for De-Tagging Hybrid Accounts
void
fm_mso_detag_hybrid_acct_gen_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);

/***********************************************
* Function Declaration -End
***********************************************/

/***********************************************
* Function Definition -Start
***********************************************/
EXPORT_OP void
fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_create_lifecycle_evt ", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_lifecycle_evt", in_flistp );
	/* Get Lifecycle event template */

	switch (action)
	{
		case ID_REGISTER_BUSINESS_UNIT : /*1*/
		case ID_REGISTER_BUSINESS_USER : /*2*/
		case ID_REGISTER_CUSTOMER : /*3*/
			mso_cust_bu_reg_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_CREATE_BB_SERVICE : /*4*/
			fm_mso_create_bb_srvc_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_ALLOCATE_HARDWARE : /*5*/
			fm_mso_allocate_bb_hw_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_MODIFY_ACTIVATION_PLAN_LIST : /*6*/
			fm_mso_modify_actv_plan_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_ACTIVATE_BB_SERVICE : /*7*/
			fm_mso_activate_bb_srvc_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_ACTIVATE_CATV_SERVICE	: /*8*/
			  fm_mso_cust_actv_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_SUSPEND_SERVICE : /*9*/
			  fm_mso_cust_susp_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_REACTIVATE_SERVICE : /*10*/
			  fm_mso_cust_react_lc_event(ctxp, in_flistp, out_flistp, ebufp	);
			break;
		case ID_TERMINATE_SERVICE : /*11*/
			  fm_mso_cust_term_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_HOLD_SERVICE : /*12*/
			fm_mso_hold_srvc_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_UNHOLD_SERVICE : /*13*/
			fm_mso_unhold_srvc_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_ADD_PLAN : /*14*/
			  fm_mso_cust_add_plan_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_CHANGE_PLAN :	/*15*/
			  fm_mso_cust_change_plan_lc_event(ctxp, in_flistp, out_flistp,	ebufp);
			break;
		case ID_REMOVE_PLAN :	/*16*/
			   fm_mso_cust_cancel_plan_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_TOPUP_PREPAID	: /*17*/
			fm_mso_topup_prepaid_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_RENEW_PREPAID	: /*18*/
			fm_mso_renew_prepaid_lc_event(ctxp, in_flistp, out_flistp, ebufp);	
			break;
		case ID_RENEW_POSTPAID_AUTOMATIC : /*19*/
			mso_op_auto_renew_plan_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_FUP_TOPUP : /*20*/
			fm_mso_cust_fup_topup_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_FDP_RENEW : /*21*/
			fm_mso_cust_renew_fdp_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_MODIFY_CUSTOMER_ADDRESS : /*22*/
		case ID_MODIFY_CUSTOMER_CONTACT : /*23*/
			  fm_mso_cust_mod_cust_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_MODIFY_CUSTOMER_TAX_EXEMPTION	: /*24*/
			fm_mso_modify_tax_exemp_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_MODIFY_CUSTOMER_BILL_SUPPRESSION : /*25*/
			fm_mso_modify_billinfo_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_VALIDITY_EXTENTION_FOR_A_SERVICE_PLAN	: /*26*/
			fm_mso_cust_extend_validity_mb_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_MODIFY_CREDIT_LIMIT :	/*27*/
			fm_mso_cust_mod_srvc_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_MODIFY_BB_SEVICE_CMTS	: /*28*/
		case ID_MODIFY_BB_SERVICE_AUTHENTICATION : /*29*/
			fm_mso_modify_bb_service_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_MODIFY_CATV_SERVICE_REGION_KEY : /*30*/
		case ID_MODIFY_CATV_SERVICE_BOUQUET_ID : /*31*/
		case ID_MODIFY_CATV_SERVICE_CAF_RECEIVED_STATUS : /*32*/
		case ID_MODIFY_CATV_SERVICE_CAS_SUBSCRIBER_ID	: /*33*/
		case ID_MODIFY_CATV_SERVICE_PERSONAL_BIT : /*34*/
			  fm_mso_cust_mod_srvc_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_HIERARCHY_MODIFICATION : /*35*/
			fm_mso_update_hierarchy_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_BUSINESS_TYPE_MODIFICATION : /*36*/
			fm_mso_update_business_type_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_BILL_INFORMATION_MODIFICATION	: /*37*/
			fm_mso_modify_billinfo_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_PAYMENT_INFORMATION_MODIFICATION : /*38*/
			fm_mso_cust_mod_payinfo_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_DEACTIVATE_VALIDITY_EXPIRY : /*39*/
		case ID_DEACTIVATE_QUOTA_EXPIRY : /*40*/
			fm_mso_deact_validity_quota_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_SWAP_DEVICE : /*41*/
			fm_mso_swap_device_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_SWAP_IP : /*42*/  /*Handled in ID_SWAP_DEVICE*/
			break;
		case ID_MODIFY_CUSTOMER_CHANGE_USERNAME_PASSWORD : /*43*/
			fm_mso_modify_self_care_cred_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_MODIFY_CUSTOMER_AR_LIMIT_BUS_USER : /*44*/
			fm_mso_modify_csr_ar_limit_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_MODIFY_ROLES_OF_CSR :	/*45*/
			fm_mso_modify_csr_roles_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_MODIFY_ACCESSCONTROL_FOR_CSR : /*46*/
			fm_mso_modify_csr_access_control_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_MODIFY_PROFILE_INFO_FOR_BUSINESS_UNIT	: /*47*/
			fm_mso_update_bu_profile_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		case ID_MODIFY_BB_SERVICE_LOGIN : /*48*/
		case ID_RESET_BB_SERVICE_LOGIN_PASSWORD : /*49*/
			fm_mso_modify_bb_service_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_MODIFY_CUSTOMER_DEACTIVATE_ACTIVATE_BUS_USER : /*50*/
			fm_mso_activate_deactivate_bu_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_DEVICE_DISASSOCIATE :	/*51*/
			fm_mso_device_diassociate_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_ADDITONAL_MB : /*52*/
			fm_mso_cust_add_additional_mb_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_PAYMENT : /*53*/
			 mso_op_payment_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_PAYMENT_REVERSAL : /*54*/
			mso_op_pymt_reverse_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_GRV_UPLOAD : /*55*/
			mso_op_grv_upload_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_REFUND : /*56*/
			mso_op_refund_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_REFUND_REVERSAL : /*57*/
			fm_mso_refund_reverse_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_DEPOSIT : /*58*/   
			mso_op_deposit_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_CREDIT_DEBIT_NOTE : /*59*/
			fm_mso_cr_dr_note_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_ADJUSTMENT : /*60*/
			mso_op_adjustment_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_DISPUTE : /*61*/
			mso_op_dispute_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_SETTLEMENT_OF_DISPUTE	: /*62*/
			mso_op_dispute_settlement_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_BILLING : /*63*/
			mso_op_billing_gen_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_PROV_ORDER_RESUBMIT :	/*64*/
			mso_op_prov_order_resubmit_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_CREATE_SCHEDULE_ACTION : /*65*/
		case ID_MODIFY_SCHEDULE_ACTION : /*66*/
			mso_op_cust_create_schedule_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_RESUBMIT_SUSPENDED_EVENT : /*67*/
			fm_mso_resub_susp_evt_mb_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_OPERATION_ADD_ACTION : /*68*/
		case ID_OPERATION_MODIFY_ACTION : /*69*/
		case ID_OPERATION_DELETE_ACTION : /*70*/
			fm_mso_ops_add_mod_del_action_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_ADD_IP : /*71*/
		case ID_REMOVE_IP : /*72*/
			mso_op_cust_add_remove_ip_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_LATE_FEE : /*73*/
			fm_mso_apply_late_fee_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			break;
		case ID_FUP_DOWNGRADE : /*74*/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_fup_downgrade_lc_event");
			fm_mso_fup_downgrade_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called fm_mso_fup_downgrade_lc_event");
			break;
		case ID_TECHNOLOGY_CHANGE : /*75*/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_service_tech_change_lc_event");
			fm_mso_service_tech_change_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning from fm_mso_service_tech_change_lc_event");
			break;
		case ID_SWAP_TAGGING : /*76*/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_cust_swap_tagging_lc_event");
			fm_mso_cust_swap_tagging_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning from fm_mso_cust_swap_tagging_lc_event");
			break;
		case ID_SUBSCRIPTION_TRANSFER : /*77*/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_cust_transfer_subscr_lc_event");
			fm_mso_cust_transfer_subscr_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning from fm_mso_cust_transfer_subscr_lc_event");
			break;
		case ID_ADD_OFFERS: /*81*/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_cust_transfer_subscr_lc_event");
			fm_mso_cust_create_offers_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning from fm_mso_cust_transfer_subscr_lc_event");
			break;
                case ID_ADD_NETFLIX: /*85*/
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_cust_create_netflix_lc_event");
                        fm_mso_cust_create_netflix_lc_event(ctxp, in_flistp, out_flistp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning from fm_mso_cust_create_netflix_lc_event");
                        break;
		case ID_CREATE_GST_INFO :
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_create_gst_info_lc_event");
			fm_mso_create_gst_info_lc_event(ctxp, in_flistp, out_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning fm_mso_create_gst_info_lc_event");
		 case ID_UPDATE_GST_INFO :
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_update_gst_info_lc_event");
                        fm_mso_update_gst_info_lc_event(ctxp, in_flistp, out_flistp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning fm_mso_update_gst_info_lc_event");
		case ID_UPDATE_POI_POA :
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_update_poi_poa_lc_event");
                        fm_mso_update_poi_poa_lc_event(ctxp, in_flistp, out_flistp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning fm_mso_update_poi_poa_lc_event");
		case ID_NETFLIX_REVERSAL :
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_netfr_gen_lc_event");
                        fm_mso_netfr_gen_lc_event(ctxp,in_flistp,ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning fm_mso_netfr_gen_lc_event");
		case ID_SWAP_OTT :
			fm_mso_swap_device_lc_event(ctxp, in_flistp, out_flistp, ebufp );
			break;
		// Creating the Life cycle event for Tagging Hybrid Accounts
		case ID_TAG_HYBRID_ACCT :
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_tag_hybrid_acct_gen_lc_event");
			fm_mso_tag_hybrid_acct_gen_lc_event(ctxp,in_flistp,ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning from fm_mso_tag_hybrid_acct_gen_lc_event");
			break;
		// Creating the Life cycle event for De-Tagging Hybrid Accounts
		case ID_DETAG_HYBRID_ACCT :
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling fm_mso_detag_hybrid_acct_gen_lc_event");
			fm_mso_detag_hybrid_acct_gen_lc_event(ctxp,in_flistp,ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning from fm_mso_detag_hybrid_acct_gen_lc_event");
			break;
	}
	return;
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returned");
}

void
fm_mso_convert_epoch_to_date(
	time_t		epoch_time,
	char		*buf)
{
	struct tm		ts;

	ts = *localtime(&epoch_time);
	strftime(buf, 80, "%d-%b-%Y %H:%M:%S", &ts);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_convert_epoch_to_date" );
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, buf );

	return;
}


void
fm_mso_get_device_type_from_poid(
	pcm_context_t		*ctxp,
	poid_t			*device_pdp,
	char			*device_type,
	pin_errbuf_t		*ebufp)
{
	char			*poid_type = NULL; 

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;


	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "device_pdp", device_pdp);
	poid_type = (char*)PIN_POID_GET_TYPE(device_pdp);

	if (poid_type && strcmp(poid_type, "/device/ip/framed") ==0 )
	{
		strcpy(device_type, "IP Framed");
	}
	else if (poid_type && strcmp(poid_type, "/device/ip/static")==0 )
	{
		strcpy(device_type, "IP Static");
	}
	else if (poid_type && strcmp(poid_type, "/device/modem") ==0 )
	{
		strcpy(device_type, "Modem");
	}
	else if (poid_type && strcmp(poid_type, "/device/nat") ==0 )
	{
		strcpy(device_type, "NAT");
	}
	else if (poid_type && strcmp(poid_type, "/device/router/cable") ==0 )
	{
		strcpy(device_type, "Router Cable");
	}
	else if (poid_type && strcmp(poid_type, "/device/router/wifi")==0 )
	{
		strcpy(device_type, "Router WiFi");
	}
	else if (poid_type && strcmp(poid_type, "/device/stb")==0 )
	{
		strcpy(device_type, "STB");
	}
	else if (poid_type && strcmp(poid_type, "/device/vc")==0 )
	{
		strcpy(device_type, "VC");
	}
	else if (poid_type && strcmp(poid_type, "/device/voucher")==0 )
	{
		strcpy(device_type, "Voucher");
	}

	return;
}


void get_mso_cfg_credit_limit(
	pcm_context_t		*ctxp,
	pin_flist_t		*plans_array,
	pin_flist_t		**o_flistp,
	pin_errbuf_t		*ebufp)
{
	int32			srch_flags =256;
	poid_t			*srch_pdp = NULL;
	int64			db = 1;
	char			*template = "select X from /mso_cfg_credit_limit where F1 = V1 ";
	char			*plan_name = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*plan_info_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "populate_additional_info plan array ", plans_array);

	plan_name = PIN_FLIST_FLD_GET(plans_array, PIN_FLD_NAME, 0, ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PLAN_NAME, plan_name, ebufp);
	
	results_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "populate_additional_info srch in flist", srch_in_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "populate_additional_info srch out flist", srch_out_flistp);

	results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(results_flistp != NULL) {
		*o_flistp = PIN_FLIST_COPY(results_flistp, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "populate_additional_info plan array updated", plans_array);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}


void get_base_plan_from_mso_purchased_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*iflist,
	pin_flist_t		**o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*plan_info_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	
	poid_t			*srch_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	
	int64			db = 1;

	char			*template = "select X from /plan 1, /mso_purchased_plan 2 where 1.F1 = 2.F2 and 2.F3 = V3 and 2.F4 = V4 and 2.F5 = V5 ";
	char			*plan_name = NULL;

	int32			srch_flags =256;
	int32			elem_id = 0;
	int32			prov_active = MSO_PROV_ACTIVE;

	pin_cookie_t		cookie = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get mso_purchased_plan array ", iflist);

	srvc_pdp = PIN_FLIST_FLD_GET(iflist, PIN_FLD_SERVICE_OBJ, 0, ebufp);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prov_active, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, MSO_BB_BP, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "populate_additional_info srch in flist", srch_in_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_purchased_plan srch out flist", srch_out_flistp);
	
	results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(results_flistp != NULL) {
		*o_flistp = PIN_FLIST_COPY(results_flistp, ebufp);
	}

	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

void get_receipt_cheque_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		*r_flistp,
	char			*cheque_num,
	pin_errbuf_t		*ebufp)
{
	int32			srch_flags =256;
	poid_t			*srch_pdp = NULL;
	int64			db = 1;
	char			*template = "select X from /mso_receipt where F1 = V1 ";
	char			*receipt_no = NULL;
	
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;

	void			*vp = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "get_receipt_cheque_no ", in_flist);

	receipt_no = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_RECEIPT_NO, 1, ebufp);
	if (!receipt_no)
	{
		return;
	}


	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RECEIPT_NO, receipt_no, ebufp);
	
	results_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search cheque num in flist", srch_in_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search cheque num out flist", srch_out_flistp);

	results_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(!results_flistp) {

		goto CLEANUP;
	}

	vp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_CHECK_NO, 1, ebufp);
	if (vp)
	{
		strcpy(cheque_num, (char*)vp);
	}
	 
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "cheque_num");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, cheque_num);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}


/*******************************************************
* This function creates the lifecycle event for payment 
*******************************************************/
void
mso_op_payment_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*s_oflistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*charge_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	
	poid_t			*acc_pdp = NULL;
	
	char			*event = "/event/activity/mso_lifecycle/payment";
	char			*action_name = "PAYMENT";
	char			*disp_name = NULL;
	char			*lc_template = NULL;
	char			service_type[32]="";
	char			msg[255]="";
	char			payment_type[32]="";
	char			*amount_str = NULL;
	char			*account_no = NULL;
	char			*lco_account_no = NULL;
	char			*prog_name = NULL;
	char			*descr = NULL;


	pin_decimal_t		*trans_amt = NULL;
	pin_decimal_t		*dec_amount = NULL;

	int64			db = 1;
	int			string_id = ID_PAYMENT;

	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_payment_gen_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Event Flist:", s_oflistp);

	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	descr =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
 	disp_name = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_NAME, 0, ebufp );
	dec_amount = (pin_decimal_t*)PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_AMOUNT, 1, ebufp );

	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	vp = (int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 1, ebufp); 
	if (vp && *(int32*)vp == SERVICE_TYPE_CATV )
	{
		strcpy(service_type, "CATV");
	}
	if (vp && *(int32*)vp == SERVICE_TYPE_BB )
	{
		strcpy(service_type, "BROADBAND");
	}

 	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_PAYMENT, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_RECEIPT_NO, lc_temp_flistp, PIN_FLD_RECEIPT_NO, ebufp);

	charge_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_CHARGES, PIN_ELEMID_ANY, 1, ebufp);
	if(charge_flistp)
	{
		vp = (int32*)PIN_FLIST_FLD_GET(charge_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
		if (vp && *(int32*)vp == PAY_TYPE_CASH )
		{
			strcpy(payment_type, "CASH");
		}
		else if ( vp && *(int32*)vp == PAY_TYPE_CHECK)
		{
			strcpy(payment_type, "CHEQUE");
		}
		else if ( vp && *(int32*)vp == PAY_TYPE_ONLINE)
		{
			strcpy(payment_type, "ONLINE");
		}
		else if ( vp && *(int32*)vp == PAY_TYPE_TDS)
		{
			strcpy(payment_type, "TDS");
		}
		else if ( vp && *(int32*)vp == PAY_TYPE_SP_AUTOPAY)
		{
			strcpy(payment_type, "SP AUTO");
		}
	}

	if(disp_name && strcmp(disp_name, PAYMENT_COLLECT) == 0)
	{
		if (charge_flistp)
		{
			dec_amount = PIN_FLIST_FLD_GET(charge_flistp, PIN_FLD_AMOUNT,0,ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, dec_amount, ebufp);
			if (dec_amount)
			{
				amount_str = pbo_decimal_to_str(dec_amount, ebufp);
			}
		}
		get_evt_lifecycle_template(ctxp, db, string_id, VER_PAYMENT_COLLECT, &lc_template, ebufp);
		sprintf(msg,lc_template, payment_type, amount_str, service_type, account_no );
		if (lc_template)
		{
			free(lc_template);
		}
	}
	else if(disp_name && strcmp(disp_name, BAL_TRANS_LCO_SOURCE) ==0)
	{
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, dec_amount, ebufp);
		if (dec_amount)
		{
			amount_str = pbo_decimal_to_str(dec_amount, ebufp);
		}

		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_LCOINFO, 0, 1, ebufp );
		if (vp)
		{
			lco_account_no = PIN_FLIST_FLD_GET(vp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
		}

		get_evt_lifecycle_template(ctxp, db, string_id, VER_PAYMENT_LCO_SOURCE, &lc_template, ebufp);
		sprintf(msg, lc_template, payment_type, amount_str, account_no, lco_account_no, service_type );
		if (lc_template)
		{
			free(lc_template);
		}
	}
	else if(disp_name && strcmp(disp_name, BAL_TRANS_LCO_DEST) ==0)
	{
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, dec_amount, ebufp);
		if (dec_amount)
		{
			amount_str = pbo_decimal_to_str(dec_amount, ebufp);
		}
		trans_amt = pbo_decimal_negate(dec_amount ,ebufp);
		PIN_FLIST_FLD_PUT(lc_temp_flistp, PIN_FLD_AMOUNT, trans_amt, ebufp);

		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_LCOINFO, 0, 1, ebufp );
		if (vp)
		{
			lco_account_no = PIN_FLIST_FLD_GET(vp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
		}

		get_evt_lifecycle_template(ctxp, db, string_id, VER_PAYMENT_LCO_DEST, &lc_template, ebufp);
		sprintf(msg, lc_template, payment_type, amount_str, lco_account_no, account_no, service_type );
		if (lc_template)
		{
			free(lc_template);
		}
	}

	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, disp_name, ebufp);  
	//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, lc_temp_flistp, PIN_FLD_USERID, ebufp);

	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event Event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	if (amount_str)
	{
		free(amount_str);
	}
	return;
}


/****************************************************************
* This function creates the lifecycle event for payment reversal 
****************************************************************/
void
mso_op_pymt_reverse_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *s_oflistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t		*balimp_flistp = NULL;
	pin_flist_t		*sub_balimp_flistp = NULL;
	pin_flist_t		*sbalnces_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;

	poid_t                  *acct_pdp = NULL;
	char                    *event = "/event/activity/mso_lifecycle/payment_reversal";
	char			*disp_name = NULL;
	char			*action_name = "PAYMENT_REVERSAL";
	char			msg[255]="";
	char			cheque_num[64]="";
	char			reason[255]="";
	char			*descr = NULL;
	char			*receipt_no  = NULL;
	char			*account_no  = NULL;
	char			*lco_account_no  = NULL;
	char			*lc_template = NULL;
	char			*amount_str = NULL;
	char			*prog_name = NULL;
	char			payment_type[32]="";
	char			lco_payment_type[32]="";

	pin_decimal_t		*ip_amount = NULL;
	pin_decimal_t		*fine_amount = NULL;

	int64			db = 1;
	int			string_id = ID_PAYMENT_REVERSAL;

	void			*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_pymt_reverse_gen_lc_event Input Flist:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_pymt_reverse_gen_lc_event s_oflistp Flist:", s_oflistp);

	acct_pdp = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
	descr =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
 	disp_name = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_NAME, 1, ebufp );
	ip_amount = (pin_decimal_t*)PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_AMOUNT, 0, ebufp );
	receipt_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_RECEIPT_NO, 1, ebufp );

	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	vp = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
	if (vp && *(int32*)vp == PAY_TYPE_CASH )
	{
		strcpy(payment_type, "CASH");
	}
	else if ( vp && *(int32*)vp == PAY_TYPE_CHECK)
	{
		strcpy(payment_type, "CHEQUE");
	}
	else if ( vp && *(int32*)vp == PAY_TYPE_ONLINE)
	{
		strcpy(payment_type, "ONLINE");
	}
	else if ( vp && *(int32*)vp == PAY_TYPE_TDS)
	{
		strcpy(payment_type, "TDS");
	}
	else if ( vp && *(int32*)vp == PAY_TYPE_SP_AUTOPAY)
	{
		strcpy(payment_type, "SP AUTO");
	}


	get_receipt_cheque_no(ctxp, i_flistp, NULL, cheque_num, ebufp);
	if (strlen(cheque_num)!=0 && descr )
	{
		strcpy(reason, ".Due to: ");
		strcat(reason, descr);
		strcat(reason, ". For cheque number :");
		strcat(reason, cheque_num);
	}
	else if (descr)
	{
		strcpy(reason, ".Due to: ");
		strcat(reason, descr);
	}
	else if (strlen(cheque_num)!=0)
	{
		strcpy(reason, ". For cheque number :");
		strcat(reason, cheque_num);
	}


	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_PAYMENT_REVERSALS, 0, ebufp);
 	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_RECEIPT_NO, lc_temp_flistp, PIN_FLD_RECEIPT_NO, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, lc_temp_flistp, PIN_FLD_AMOUNT_SRC, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, disp_name, ebufp);  
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STR_VERSION, lc_temp_flistp, PIN_FLD_STR_VERSION, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STRING_ID, lc_temp_flistp, PIN_FLD_STRING_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, lc_temp_flistp, PIN_FLD_REASON_ID, ebufp);


	if(disp_name && strcmp(disp_name, PAYMENT_REVERSAL) == 0)
	{
		balimp_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_BAL_IMPACTS, 0, 1, ebufp);
		if(balimp_flistp)
		{
			amount_str = pbo_decimal_to_str(PIN_FLIST_FLD_GET(balimp_flistp, PIN_FLD_AMOUNT,0,ebufp), ebufp);
			PIN_FLIST_FLD_COPY(balimp_flistp, PIN_FLD_AMOUNT, lc_temp_flistp, PIN_FLD_AMOUNT, ebufp);
		}
		get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, payment_type, amount_str, receipt_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}
	}
	else if(disp_name && strcmp(disp_name, CQ_BOUNCE_PENALTY) == 0)
	{
		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if(res_flistp)
		{
			if (!acct_pdp)
			{
				acct_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
				if (acct_pdp)
				{
					fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
					account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
					PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
					PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
				}
			}
			sub_balimp_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_SUB_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp);
			if(sub_balimp_flistp)
			{
				sbalnces_flistp = PIN_FLIST_ELEM_GET(sub_balimp_flistp, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, 1, ebufp);
				if(sbalnces_flistp)
				{
					amount_str = pbo_decimal_to_str(PIN_FLIST_FLD_GET(sbalnces_flistp, PIN_FLD_AMOUNT,0,ebufp), ebufp);
					PIN_FLIST_FLD_COPY(sbalnces_flistp, PIN_FLD_AMOUNT, lc_temp_flistp, PIN_FLD_AMOUNT, ebufp);
				}
			}
		}
		
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_LCOINFO, 0, 1, ebufp );
		if (vp)
		{
			lco_account_no = PIN_FLIST_FLD_GET(vp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
		}
		get_evt_lifecycle_template(ctxp, db, string_id, VER_PYMT_REV_CHQ_BOUNCE, &lc_template, ebufp);
		sprintf(msg, lc_template, payment_type, amount_str, receipt_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}
	}
	else if(disp_name && strcmp(disp_name, BAL_TRANS_LCO_SOURCE) ==0)
	{
		amount_str = pbo_decimal_to_str(ip_amount, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, ip_amount, ebufp);

		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_LCOINFO, 0, 1, ebufp );
		if (vp)
		{
			lco_account_no = PIN_FLIST_FLD_GET(vp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
		}

		get_evt_lifecycle_template(ctxp, db, string_id, VER_PYMT_REV_LCO_SOURCE, &lc_template, ebufp);
		sprintf(lco_payment_type, "LCO(S) %s", payment_type);
		sprintf(msg, lc_template, lco_payment_type, amount_str, receipt_no, lco_account_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}
	}
	else if(disp_name && strcmp(disp_name, BAL_TRANS_LCO_DEST) ==0)
	{
		amount_str = pbo_decimal_to_str(ip_amount, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, ip_amount, ebufp);
		
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_LCOINFO, 0, 1, ebufp );
		if (vp)
		{
			lco_account_no = PIN_FLIST_FLD_GET(vp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
		}		
		get_evt_lifecycle_template(ctxp, db, string_id, VER_PYMT_REV_LCO_DEST, &lc_template, ebufp);
		sprintf(lco_payment_type, "LCO(D) %s", payment_type);
		sprintf(msg, lc_template, lco_payment_type, amount_str, receipt_no, account_no, lco_account_no );
		if (lc_template)
		{
			free(lc_template);
		}
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event Event Flist:", lc_iflistp);



	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event Event Flist:", lc_iflistp);

	if (strlen(reason)!=0)
	{
		strcat(msg, reason);
	}

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);

	if (amount_str)
	{
		free(amount_str);
	}

	return;
}

void
mso_op_deposit_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *dep_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;

	pin_decimal_t		*dep_total = NULL ;

	poid_t                  *user_id = NULL;
	poid_t                  *acct_pdp = NULL;
	poid_t                  *serv_pdp = NULL;

	char                    *event = "/event/activity/mso_lifecycle/deposit";
	char			*action_name = "DEPOSIT";
	char			msg[255]="";
	char			*dep_no = NULL;
	char                    *disp_name = NULL;
	char			*lc_template = NULL;
	char			*amount_str = NULL;
	char			*account_no = NULL;
	char			*prog_name = NULL;
	char			*plan_name = NULL;
	char			*agreement_no = NULL;


	int64			db = 1;
	int32			string_id = ID_DEPOSIT;

	void			*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_deposit_gen_lc_event Input FList:", i_flistp);

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
	serv_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	//user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp );
	dep_total = PIN_FLIST_FLD_GET(dep_flistp, MSO_FLD_DEPOSIT_AMOUNT, 1, ebufp);
	dep_no =  PIN_FLIST_FLD_GET(dep_flistp, MSO_FLD_DEPOSIT_NO, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	if (serv_pdp && (strcmp( PIN_POID_GET_TYPE(serv_pdp),  "/service/telco/broadband")==0) )
	{
		  plan_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CODE, 1, ebufp);
	}
	else if (serv_pdp && (strcmp( PIN_POID_GET_TYPE(serv_pdp),  "/service/catv")==0) )
	{
		vp = (pin_flist_t*) PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 1, ebufp);
		if (vp)
		{
			plan_name = PIN_FLIST_FLD_GET(vp, PIN_FLD_CODE, 1, ebufp);
		}
	}

	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_DEPOSIT, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_COPY(dep_flistp, MSO_FLD_DEPOSIT_NO, lc_temp_flistp ,MSO_FLD_DEPOSIT_NO, ebufp);
	PIN_FLIST_FLD_COPY(dep_flistp, MSO_FLD_DEPOSIT_AMOUNT, lc_temp_flistp ,MSO_FLD_DEPOSIT_AMOUNT, ebufp);  
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_name, ebufp);

	if (dep_total)
	{
		amount_str = pbo_decimal_to_str(dep_total, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, amount_str, dep_no, agreement_no, account_no );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_gen_event Event FList:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, serv_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


 	if (amount_str)
	{
		free(amount_str);
	}
	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	return;
}


void
mso_op_refund_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *r_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	
	poid_t			*acct_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*event_pdp = NULL;
	
	pin_decimal_t		*dep_total = NULL;
	pin_decimal_t		*abs_dep_total = NULL;

	char			*dep_number = NULL;
	char			*event = "/event/activity/mso_lifecycle/refund";
	char			*action_name = "DEPOSIT_REFUND";
	char			msg[255]="";
	char			*prog_name = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*amount_str = NULL;
	char			*lc_template = NULL;

	int32			string_id = ID_REFUND;
	int64			db = 1;

	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_fld_refund_gen_lc_event Input FList:", i_flistp);

	acct_pdp   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
	account_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);

	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	
	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DEPOSITS, PIN_ELEMID_ANY, 1, ebufp);
	if (vp)
	{
		srvc_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
		fm_mso_get_srvc_info(ctxp, vp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
		dep_number = PIN_FLIST_FLD_GET(vp, MSO_FLD_DEPOSIT_NO, 1, ebufp );
	}

	dep_total =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp );
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);


	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_REFUND, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_REFUND_NO, dep_number, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_REFUND_AMOUNT, dep_total, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event Event Flist:", lc_iflistp);
 	
	if (dep_total)
	{
		abs_dep_total = pbo_decimal_negate(dep_total, ebufp);
		amount_str = pbo_decimal_to_str(abs_dep_total, ebufp);
	}
	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, amount_str, dep_number, agreement_no, account_no );

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

 	if (amount_str)
	{
		free(amount_str);
	}
	if (lc_template)
	{
		free(lc_template);
	}
	if ( !pbo_decimal_is_null(abs_dep_total, ebufp) )
	{
		pbo_decimal_destroy(&abs_dep_total);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);

	return;
}


void
fm_mso_cust_actv_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*actvn_out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;
	pin_flist_t		*d_flistp = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*device_pdp = NULL;
		
	int64			db = 0;
	int32			elem_id = 0;
	int32			*conn_type = NULL;
	int			string_id = ID_ACTIVATE_CATV_SERVICE;

	char			*event = "/event/activity/mso_lifecycle/activate_catv_service";
	char			*action_name = "ACTIVATE_CATV_CUSTOMER";
	char			*action_name1 = "ACTIVATE_CATV_CUSTOMER_PLANS";

	char			*lc_template = NULL;
	char			msg[255]="";
	char			conn_type_str[20]="";
	char			*prog_name = NULL;
	char			*stb_id = NULL;
	char			*vc_id = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*plan_code = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;
	void			*vp2 = NULL;

	pin_cookie_t		cookie = NULL;
        //New Tariff - Transaction mapping */
        pin_flist_t             *offering_flistp = NULL;
        pin_flist_t             *eflistp = NULL;
        int32                   elem_id_off = 0;
        pin_cookie_t            cookie_off = NULL;
        int32                   elem_id_1 = 0;
        pin_cookie_t            cookie_1 = NULL;
        pin_flist_t             *plan_lists_flistp = NULL;
        pin_flist_t             *plan_flistp = NULL;
        poid_t                  *plan_pdp_1 = NULL;
        poid_t                  *plan_pdp_2 = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_actv_lc_event", in_flistp );
	/* Get Lifecycle event template */
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "actvn_out_flistp", actvn_out_flistp );

	acc_pdp = PIN_FLIST_FLD_GET(actvn_out_flistp, PIN_FLD_POID, 1, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);

	vp = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(actvn_out_flistp, PIN_FLD_SERVICE_INFO, 1, ebufp);
	if (vp)
	{
		vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_SERVICES, 0, 1, ebufp);
		if (vp1)
		{
			service_pdp = PIN_FLIST_FLD_GET(vp1, PIN_FLD_POID, 1, ebufp); 
			vp2 = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(vp1, MSO_FLD_CATV_INFO, 1, ebufp);
			if (vp2)
			{
				agreement_no = PIN_FLIST_FLD_GET(vp2, MSO_FLD_AGREEMENT_NO, 1, ebufp);
				conn_type = (int32*)PIN_FLIST_FLD_GET(vp2, PIN_FLD_CONN_TYPE,1, ebufp);
				if (conn_type && *conn_type == CATV_MAIN)
				{
					strcpy(conn_type_str,"MAIN");
				}
				if (conn_type && *conn_type == CATV_ADDITIONAL)
				{
					strcpy(conn_type_str,"ADDITIONAL");
				}
			}
		}
	}

	/*************************************************************************
         * Get DEVICE_ID from PIN_FLD_SERVICES.PIN_FLD_DEVICES array
	 ************************************************************************/
	while((d_flistp = PIN_FLIST_ELEM_GET_NEXT(vp1, PIN_FLD_DEVICES,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		device_pdp = PIN_FLIST_FLD_GET(d_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
		if (strstr(PIN_POID_GET_TYPE(device_pdp), "/device/stb"))
		{
			stb_id =  PIN_FLIST_FLD_GET(d_flistp, MSO_FLD_STB_ID, 0, ebufp);
		}
		else if (strstr(PIN_POID_GET_TYPE(device_pdp), "/device/vc"))
		{
			vc_id =  PIN_FLIST_FLD_GET(d_flistp, MSO_FLD_VC_ID, 0, ebufp);
		}
		else
		{
		}
	}
	
	cookie = NULL;
	elem_id = 0;
	
	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	vp = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_PLAN_LIST_CODE, 1, ebufp);
	plan_list = PIN_FLIST_COPY(vp, ebufp);

  	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg,lc_template,conn_type_str, agreement_no, account_no, stb_id, vc_id);
	if (lc_template)
	{
		free(lc_template);
	}

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_ACTIVATE_CATV_SERVICE, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ,  acc_pdp, ebufp);   
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp); 
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	if (vc_id) PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_VC_ID, vc_id, ebufp);
	if (stb_id) PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_STB_ID, stb_id, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_actv_lc_event gen_lifecycle_evt Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);
        //New Tariff - Transaction mapping Start */
        plan_lists_flistp = PIN_FLIST_ELEM_GET(actvn_out_flistp, PIN_FLD_PLAN_LISTS, 0, 0, ebufp);
        //New Tariff - Transaction mapping End */


	get_evt_lifecycle_template(ctxp, db, string_id, 2, &lc_template, ebufp);

	if (plan_list)
	{
		while((plans = PIN_FLIST_ELEM_GET_NEXT(plan_list, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			plan_code = PIN_FLIST_FLD_GET(plans, PIN_FLD_CODE, 1, ebufp);
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, lc_temp_flistp, PIN_FLD_PLAN_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_CODE, lc_temp_flistp, PIN_FLD_CODE, ebufp);
                        //New Tariff - Transaction mapping start*/
                        plan_pdp_1 = PIN_FLIST_FLD_GET(plans, PIN_FLD_PLAN_OBJ, 0, ebufp);
                        elem_id_1 = 0;
                        cookie_1 = NULL;
                        while((plan_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_lists_flistp, PIN_FLD_PLAN,
                                &elem_id_1, 1, &cookie_1, ebufp)) != NULL) {
                                plan_pdp_2 = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);

                                if(PIN_POID_COMPARE(plan_pdp_1, plan_pdp_2, 0, ebufp) == 0) {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Compare plan poid");
		                        elem_id_off = 0;
                		        cookie_off = NULL;
		                        while ((offering_flistp = PIN_FLIST_ELEM_GET_NEXT(plan_flistp,
                                                                PIN_FLD_OFFERINGS, &elem_id_off, 1, &cookie_off, ebufp))!= NULL) {
						PIN_FLIST_ELEM_SET(lc_iflistp, offering_flistp, PIN_FLD_OFFERINGS, elem_id_off, ebufp);
					}
                                }
                        }
                        //New Tariff - Transaction mapping end*/

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_actv_lc_event gen_lifecycle_evt Flist:", lc_iflistp);

			sprintf(msg,lc_template, plan_code, conn_type_str, agreement_no, account_no, stb_id, vc_id);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
				msg, action_name1, event, lc_iflistp, ebufp);

			PIN_FLIST_FLD_DROP(lc_temp_flistp, PIN_FLD_PLAN_OBJ, ebufp);
			PIN_FLIST_FLD_DROP(lc_temp_flistp, PIN_FLD_CODE, ebufp);
		}
	}
	if (lc_template)
	{
		free(lc_template);
	}

	PIN_FLIST_DESTROY_EX(&plan_list, NULL);
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	return;
}

void
fm_mso_cust_react_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_REACTIVATE_SERVICE;
    int32           reason_id = 0;
    int32           reason_domain_id = 0;
	char			*event = "/event/activity/mso_lifecycle/reactivate_service";
	char			*action_name = "REACTIVATE_SERVICE";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name = NULL;
	char			*reason = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
    char            *domainp = "Reason Codes-Active Status Reasons";
    void            *vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_react_lc_event", in_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	db = PIN_POID_GET_DB(acc_pdp);
	
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV)) == 0 )
	{
		strcpy(service_type, "CATV") ;
	}
	else if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0)
	{
		strcpy(service_type, "BROADBAND");
	}

	agreement_no = PIN_FLIST_FLD_GET(out_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	reason = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DESCR, 1, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	sprintf(msg,lc_template, service_type, agreement_no, reason, account_no );

    vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_REASON_ID, 1, ebufp);
    if (vp)
    {
        reason_id = *(int32 *)vp;

        vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_REASON_DOMAIN_ID, 1, ebufp);
        if (vp)
        {
            reason_domain_id = *(int32 *)vp;
        }
        free(lc_template);
        lc_template = NULL;
        get_strings_template(ctxp, db, domainp, reason_id, reason_domain_id, &lc_template, ebufp);
    }

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_SERVICE_REACTIVATE, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR,        reason, ebufp);
    if (reason_id != 0 && lc_template)
    {
        PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_REASON_CODE, lc_template, ebufp);
    }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_react_lc_event gen_lifecycle_evt flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	return;
}

void
fm_mso_cust_susp_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_SUSPEND_SERVICE;
    int32           reason_id = 0;
    int32           reason_domain_id = 0;

	char			*event = "/event/activity/mso_lifecycle/suspend_service";
	char			*action_name = "SUSPEND_SERVICE";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name = NULL;
	char			*reason = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
    char            *domainp = "Reason Codes-Inactive Status Reasons";
    void            *vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_susp_lc_event", in_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	db = PIN_POID_GET_DB(acc_pdp);
	
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV)) == 0 )
	{
		strcpy(service_type, "CATV") ;
	}
	else if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0)
	{
		strcpy(service_type, "BROADBAND");
	}

	agreement_no = PIN_FLIST_FLD_GET(out_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	reason = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DESCR, 1, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	sprintf(msg,lc_template, service_type, agreement_no, reason, account_no );

    vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_REASON_ID, 1, ebufp);
    if (vp)
    {
        reason_id = *(int32 *)vp;

        vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_REASON_DOMAIN_ID, 1, ebufp);
        if (vp)
        {
            reason_domain_id = *(int32 *)vp;
        }
        free(lc_template);
        lc_template = NULL;
        get_strings_template(ctxp, db, domainp, reason_id, reason_domain_id, &lc_template, ebufp);
    }

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_SUSPEND_SERVICE, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR,        reason, ebufp);
    if (reason_id != 0 && lc_template)
    {
        PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_REASON_CODE, lc_template, ebufp);
    }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_susp_lc_event gen_lifecycle_evt flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	return;
}

/*************************************************************
 * Changes made to this function to include the Termination
 * Reason Type, Sub Type in the Life Cycle Event for 
 * Service Termination 
 *************************************************************/
void
fm_mso_cust_term_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
		
	int64			db = 0;
	int32			elem_id = 0;
	int32			srvc_type = 0;
	int32			srch_flag = 256;
	int			string_id = ID_TERMINATE_SERVICE;

	char			*event = "/event/activity/mso_lifecycle/terminate_service";
	char			*action_name = "SERVICE_TERMINATION";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			srvc_type_str[20]="";
	char			*prog_name = NULL;
	char			*reason = NULL;
	//CATV GRV AD == Declaration for Reason Type and Sub Type
	char			*reason_type = NULL;
	char			*reason_subtype = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;

	void			*vp = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		previous_cookie = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_term_lc_event in_flistp", in_flistp  );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_term_lc_event out_flistp", out_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(out_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	reason =  PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DESCR, 1, ebufp);
	// CATV GRV AD == Getting the Reason Type and Sub Type from the Input flist for Service Termination.
	reason_type =  PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TYPE_STR, 1, ebufp);
	reason_subtype =  PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_VALUE, 1, ebufp);

	if (service_pdp && strcmp((char*)PIN_POID_GET_TYPE(service_pdp),MSO_CATV)==0 )
	{
		srvc_type = CATV;
		strcpy(srvc_type_str,"CATV");
	}
	else
	{
		srvc_type = BB;
		strcpy(srvc_type_str,"BROADBAND");
	}

	db = PIN_POID_GET_DB(acc_pdp);
	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	if (srvc_type == CATV )
	{
	}

	sprintf(msg,lc_template,srvc_type_str, agreement_no, reason, account_no);
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_SERVICE_TERMINATE, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, reason, ebufp);
	// CATV GRV AD == Adding the Reason Type and Sub Type to the Life Cycle Event for Service Termination.
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_TYPE_STR, reason_type, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_VALUE, reason_subtype, ebufp);

	if (PIN_FLIST_ELEM_COUNT(out_flistp, PIN_FLD_PLAN, ebufp) >0 )
	{
		while((plans = PIN_FLIST_ELEM_GET_NEXT(out_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_PLAN_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_CODE, lc_temp_flistp, PIN_FLD_CODE, ebufp);
	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_term_lc_event gen_lifecycle_evt Flist:", lc_iflistp);
	
			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name, msg, action_name, event, lc_iflistp, ebufp);
	
			PIN_FLIST_ELEM_DROP(out_flistp, PIN_FLD_PLAN, elem_id, ebufp );
			cookie = previous_cookie;
	
			previous_cookie = cookie;
		}
	}
	else
	{
            	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_term_lc_event gen_lifecycle_evt Flist:", lc_iflistp);
           	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name, msg, action_name, event, lc_iflistp, ebufp);

	}

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	return;
}

void
fm_mso_cust_change_plan_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;
	pin_flist_t		*cycle_flistp = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_CHANGE_PLAN;
	int32			elem_id = 0;
	int32			*flag = NULL;

	char			*event = "/event/activity/mso_lifecycle/change_plan";
	char			*action_name = "CHANGE_PLAN";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name = NULL;
	char			*reason = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*old_plan_code = NULL;
	char			*new_plan_code = NULL;
	char			time_buf[100] = {'\0'};
	char			eff_time[80] = {'\0'};
        time_t                  *purc_end = NULL;
        time_t                  *cyc_start_t = NULL;
        time_t                  *cyc_end_t = NULL;
        struct tm                       *gm_time;

	pin_cookie_t		cookie = NULL;

        //New Tariff - Transaction mapping */
        pin_flist_t             *event_flistp = NULL;
        pin_flist_t             *eflistp = NULL;
        int32                   offer_count = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan_lc_event", in_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	db = PIN_POID_GET_DB(acc_pdp);
	
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV)) == 0 )
	{
		strcpy(service_type, "CATV") ;
	}
	else if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0)
	{
		strcpy(service_type, "BROADBAND");
	}

	agreement_no = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);

	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV)) == 0 )
	{
	 	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	}
	else if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0)
	{
		string_id = ID_CHANGE_PLAN_BB;
		get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	}

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CHANGE_PLAN, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);

	while((plans = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN_LISTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		purc_end = (time_t*)PIN_FLIST_FLD_GET(plans, PIN_FLD_PURCHASE_END_T, 1, ebufp );
		flag = (int32*)PIN_FLIST_FLD_GET(plans, PIN_FLD_FLAGS, 1, ebufp );
		if (flag && *flag == FLAG_ADD_PLAN )
		{
			new_plan_code = PIN_FLIST_FLD_GET(plans, PIN_FLD_CODE, 1, ebufp );
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, lc_temp_flistp, MSO_FLD_NEW_PLAN_OBJ, ebufp );
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_CODE, lc_temp_flistp, PIN_FLD_TO_CODE, ebufp );
		}
		else if (flag && *flag == FLAG_DEL_PLAN )
		{
			old_plan_code = PIN_FLIST_FLD_GET(plans, PIN_FLD_CODE, 1, ebufp );
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, lc_temp_flistp, MSO_FLD_OLD_PLAN_OBJ, ebufp );
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_CODE, lc_temp_flistp, PIN_FLD_FROM_CODE, ebufp );
		}
	}
	//This would be valid only in case of PREPAID DC otherwise no CYCLE_INFO details
	cycle_flistp = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_CYCLE_INFO, 1, ebufp);
	if(cycle_flistp){
		cyc_start_t = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_CYCLE_START_T, 1, ebufp);
		cyc_end_t = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_CYCLE_END_T, 1, ebufp);
		strftime( time_buf, sizeof(time_buf), " Prepaid Charges refunded from %d-%b-%Y",  localtime(cyc_start_t));
		//strcat(time_buf, eff_time);
		memset(eff_time, '\0', sizeof(eff_time));
		*cyc_end_t = *cyc_end_t-1;//End of the previous day for OAP display
		strftime( eff_time, sizeof(eff_time), " to %d-%b-%Y",  localtime(cyc_end_t));
		strcat(time_buf, eff_time);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Prepaid Time:");
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, time_buf);
	}

	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV)) == 0 )
	{
	 	sprintf(msg,lc_template, old_plan_code, new_plan_code, service_type, agreement_no, account_no, time_buf );
	}
	else if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0)
	{
		sprintf(msg,lc_template, old_plan_code, new_plan_code, agreement_no);		
	}
        //New Tariff - Transaction mapping Start */
        if ( out_flistp != NULL )
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "checking event count");
                offer_count = PIN_FLIST_ELEM_COUNT(out_flistp, PIN_FLD_OFFERINGS, ebufp);
                if (offer_count > 0) {
                        elem_id = 0;
                        cookie = NULL;
                        while ((event_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
                                PIN_FLD_OFFERINGS, &elem_id, 1, &cookie, ebufp))!= NULL) {
                                eflistp = PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_OFFERINGS, elem_id, ebufp);
                                PIN_FLIST_FLD_COPY(event_flistp, PIN_FLD_OFFERING_OBJ, eflistp, PIN_FLD_OFFERING_OBJ, ebufp);
                                PIN_FLIST_FLD_COPY(event_flistp, PIN_FLD_PACKAGE_ID, eflistp, PIN_FLD_PACKAGE_ID, ebufp);
                        }
                }
        }
        //New Tariff - Transaction mapping End */

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_change_plan_lc_event gen_lifecycle_evt flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	return;
}


void
fm_mso_cust_add_plan_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;
	pin_flist_t		*plan_info = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_ADD_PLAN;
	int32			elem_id = 0;

	char			*event = "/event/activity/mso_lifecycle/add_plan";
	char			*action_name = "ADD_PLAN";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name = NULL;
	char			*reason = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*plan_code = NULL;

	pin_cookie_t		cookie = NULL;
        //New Tariff - Transaction mapping */
        pin_flist_t             *offering_flistp = NULL;
        pin_flist_t             *pl_flistp = NULL;
        pin_flist_t             *eflistp = NULL;
        int32                   offer_count = 0;
        int32                   offercheck = 0;
        poid_t                  *plan_pdp_1 = NULL;
        poid_t                  *plan_pdp_2 = NULL;
        int32                   elem_id_pl = 0;
        pin_cookie_t            cookie_pl = NULL;
        int32                   elem_id_off = 0;
        pin_cookie_t            cookie_off = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_plan_lc_event", in_flistp );
	/* Get Lifecycle event template */
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "out_flistp", out_flistp );
	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
	}
	if(acnt_info != NULL)
	{
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	db = PIN_POID_GET_DB(acc_pdp);
	
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_ADD_PLAN, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
        //New Tariff - Transaction mapping Start */
        if ( out_flistp != NULL )
        {
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "checking event count");
                offer_count = PIN_FLIST_ELEM_COUNT(out_flistp, PIN_FLD_PLAN_LISTS, ebufp);
                if (offer_count > 0) {
			offercheck = 1;
		}
	}
        //New Tariff - Transaction mapping End */

	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV)) == 0 )
	{
		strcpy(service_type, "CATV") ;

		while((plans = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN_LISTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			plan_code = PIN_FLIST_FLD_GET(plans, PIN_FLD_CODE, 1, ebufp );
			sprintf(msg,lc_template, plan_code, service_type, agreement_no, account_no );

			PIN_FLIST_FLD_COPY(plans, PIN_FLD_CODE, lc_temp_flistp, PIN_FLD_CODE, ebufp);
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, lc_temp_flistp, PIN_FLD_PLAN_OBJ, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_plan_lc_event gen_lifecycle_evt flist:", lc_iflistp);
		        //New Tariff - Transaction mapping Start */
                        plan_pdp_1 = PIN_FLIST_FLD_GET(plans, PIN_FLD_PLAN_OBJ, 0, ebufp);
		        if (offercheck == 1) {
                		elem_id_pl = 0;
		                cookie_pl = NULL;
                                while ((pl_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
                                                        PIN_FLD_PLAN_LISTS, &elem_id_pl, 1, &cookie_pl, ebufp))!= NULL) {
					
	                                plan_pdp_2 = PIN_FLIST_FLD_GET(pl_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);

	                                if(PIN_POID_COMPARE(plan_pdp_1, plan_pdp_2, 0, ebufp) == 0) {
						elem_id_off = 0;
						cookie_off = NULL;
	        	        		while ((offering_flistp = PIN_FLIST_ELEM_GET_NEXT(pl_flistp,
        	        	                		PIN_FLD_OFFERINGS, &elem_id_off, 1, &cookie_off, ebufp))!= NULL) {
			        	        	eflistp = PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_OFFERINGS, elem_id_off, ebufp);
                				        PIN_FLIST_FLD_COPY(offering_flistp, PIN_FLD_OFFERING_OBJ, 
											eflistp, PIN_FLD_OFFERING_OBJ, ebufp);
                                			PIN_FLIST_FLD_COPY(offering_flistp, PIN_FLD_PACKAGE_ID, 
										eflistp, PIN_FLD_PACKAGE_ID, ebufp);
						}
					}
                        	}	
                	}
		        //New Tariff - Transaction mapping End */
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "lc_iflistp:", lc_iflistp);
			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
				msg, action_name, event, lc_iflistp, ebufp);
		}
	}
	else if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0)
	{
		strcpy(service_type, "BROADBAND");

		while((plans = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			fm_mso_cust_get_plan_code(ctxp, plans, &plan_info, ebufp);
			if(plan_info)
			{
				plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp );
			}
			sprintf(msg,lc_template, plan_code, service_type, agreement_no, account_no );

			PIN_FLIST_FLD_COPY(plans, PIN_FLD_CODE, lc_temp_flistp, PIN_FLD_CODE, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_plan_lc_event gen_lifecycle_evt flist:", lc_iflistp);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
				msg, action_name, event, lc_iflistp, ebufp);
		}
	}


	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&plan_info, NULL);
	return;
}

void
fm_mso_cust_cancel_plan_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_REMOVE_PLAN;
	int32			elem_id = 0;

	char			*event = "/event/activity/mso_lifecycle/remove_plan";
	char			*action_name = "REMOVE_PLAN";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name = NULL;
	char			*reason = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*plan_code = NULL;

	pin_cookie_t		cookie = NULL;
        //New Tariff - Transaction mapping */
        pin_flist_t             *offering_flistp = NULL;
        pin_flist_t             *pl_flistp = NULL;
        pin_flist_t             *eflistp = NULL;
        int32                   offer_count = 0;
        int32                   offercheck = 0;
        poid_t                  *plan_pdp_1 = NULL;
        poid_t                  *plan_pdp_2 = NULL;
        int32                   elem_id_pl = 0;
        pin_cookie_t            cookie_pl = NULL;
        int32                   elem_id_off = 0;
        pin_cookie_t            cookie_off = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan_lc_event", in_flistp );
	/* Get Lifecycle event template */
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "out_flistp", out_flistp );

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	db = PIN_POID_GET_DB(acc_pdp);
	
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV)) == 0 )
	{
		strcpy(service_type, "CATV") ;
		//New Tariff - Transaction mapping Start */
	        if ( out_flistp != NULL )
        	{
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "checking event count");
	                offer_count = PIN_FLIST_ELEM_COUNT(out_flistp, PIN_FLD_PLAN_LISTS, ebufp);
        	        if (offer_count > 0) {
                	        offercheck = 1;
                	}
        	}
	        //New Tariff - Transaction mapping End */

	}
	else if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0)
	{
		strcpy(service_type, "BROADBAND");
	}

	fm_mso_get_srvc_info(ctxp, in_flistp, &srvc_info, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_REMOVE_PLAN, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);

	while((plans = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN_LISTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		plan_code = PIN_FLIST_FLD_GET(plans, PIN_FLD_CODE, 1, ebufp );
		sprintf(msg,lc_template, plan_code, service_type, agreement_no, account_no );

		PIN_FLIST_FLD_COPY(plans, PIN_FLD_CODE, lc_temp_flistp, PIN_FLD_CODE, ebufp);
		PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, lc_temp_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_cancel_plan_lc_event gen_lifecycle_evt flist:", lc_iflistp);
                //New Tariff - Transaction mapping Start */
                plan_pdp_1 = PIN_FLIST_FLD_GET(plans, PIN_FLD_PLAN_OBJ, 0, ebufp);
                if (offercheck == 1) {
        	        elem_id_pl = 0;
                        cookie_pl = NULL;
                        while ((pl_flistp = PIN_FLIST_ELEM_GET_NEXT(out_flistp,
                                                        PIN_FLD_PLAN_LISTS, &elem_id_pl, 1, &cookie_pl, ebufp))!= NULL) {
	                        plan_pdp_2 = PIN_FLIST_FLD_GET(pl_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
                                if(PIN_POID_COMPARE(plan_pdp_1, plan_pdp_2, 0, ebufp) == 0) {
	                                elem_id_off = 0;
                                        cookie_off = NULL;
                                        while ((offering_flistp = PIN_FLIST_ELEM_GET_NEXT(pl_flistp,
                                                                PIN_FLD_OFFERINGS, &elem_id_off, 1, &cookie_off, ebufp))!= NULL) {
         	                               eflistp = PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_OFFERINGS, elem_id_off, ebufp);
                                               PIN_FLIST_FLD_COPY(offering_flistp, PIN_FLD_OFFERING_OBJ,
                                                                                eflistp, PIN_FLD_OFFERING_OBJ, ebufp);
                                               PIN_FLIST_FLD_COPY(offering_flistp, PIN_FLD_PACKAGE_ID,
                                                                                eflistp, PIN_FLD_PACKAGE_ID, ebufp);
                                         }
                                 }
                         }
                }
                //New Tariff - Transaction mapping End */


		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
			msg, action_name, event, lc_iflistp, ebufp);
	}

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info,  NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	return;
}


void
fm_mso_cust_mod_srvc_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*acnt_info,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*srvc_info_new = NULL;
	pin_flist_t		*srvc_info_old = NULL;
	pin_flist_t		*srvc_catv_info_new = NULL;
	pin_flist_t		*srvc_catv_info_old = NULL;
	pin_flist_t		*region_key_info = NULL;
	pin_flist_t		*bouquet_id_info = NULL;
	pin_flist_t		*caf_info = NULL;
	pin_flist_t		*cas_subs_id_info = NULL;
	pin_flist_t		*pers_bit_info_old = NULL;
	pin_flist_t		*pers_bit_info_new = NULL;
	pin_flist_t		*credit_profile_old = NULL;
	pin_flist_t		*credit_profile_new = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
		
	int64			db = 0;
	int			string_id_region_key  = ID_MODIFY_CATV_SERVICE_REGION_KEY;
	int			string_id_bouquet_id  = ID_MODIFY_CATV_SERVICE_BOUQUET_ID;
	int			string_id_caf         = ID_MODIFY_CATV_SERVICE_CAF_RECEIVED_STATUS;
	int			string_id_cas_subs_id = ID_MODIFY_CATV_SERVICE_CAS_SUBSCRIBER_ID;
	int			string_id_pers_bit    = ID_MODIFY_CATV_SERVICE_PERSONAL_BIT;
	int			string_id_cr_limit    = ID_MODIFY_CREDIT_LIMIT;
	int32			elem_id = 0;


	char			*event_region_key  = "/event/activity/mso_lifecycle/modify_regionkey";
	char			*event_bouquet_id  = "/event/activity/mso_lifecycle/modify_bouquet";
	char			*event_caf         = "/event/activity/mso_lifecycle/modify_caf_status";
	char			*event_cas_subs_id = "/event/activity/mso_lifecycle/modify_casid";
	char			*event_pers_bit    = "/event/activity/mso_lifecycle/set_personal_bit";
	char			*event_cr_limit    = "/event/activity/mso_lifecycle/set_credit_limit";

	char			*action_modify_region_key   = "MODIFY_CATV_SERVICE_REGION_KEY";
	char			*action_modify_bouquet_id   = "MODIFY_CATV_SERVICE_BOUQUET_ID";
	char			*action_modify_caf          = "MODIFY_CATV_SERVICE_CAF_RECEIVED_STATUS";
	char			*action_modify_cas_subs_id  = "MODIFY_CATV_SERVICE_CAS_SUBSCRIBER_ID";
	char			*action_modify_pers_bit     = "MODIFY_CATV_SERVICE_PERSONAL_BIT"; 
	char			*action_modify_cr_limit     = "MODIFY_CREDIT_LIMIT";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			caf_status_str[32]="";
	char			*prog_name = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*region_key_old = NULL;
	char			*region_key_new = NULL;
	char			*bouquet_id_old = NULL;
	char			*bouquet_id_new = NULL;
	char			*cas_subs_id_old = NULL;
	char			*cas_subs_id_new = NULL;
	char			*pers_bit_value_old = NULL;
	char			*pers_bit_name_old = NULL;
	char			*pers_bit_value_new = NULL;
	char			*pers_bit_name_new = NULL;

	int32			*caf_new = NULL;
	int32			*caf_old = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;
	void			*vp2 = NULL;

	pin_cookie_t		cookie = NULL;

	pin_decimal_t		*credit_limit_old = NULL;
	pin_decimal_t		*credit_floor_old = NULL;
	pin_decimal_t		*credit_limit_new = NULL;
	pin_decimal_t		*credit_floor_new = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_srvc_lc_event", in_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	account_no = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);

	db = PIN_POID_GET_DB(acc_pdp);

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	if (vp)
	{
		srvc_info_old = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_SERVICES, 0, 1, ebufp );
		if (srvc_info_old)
		{
			srvc_pdp = PIN_FLIST_FLD_GET(srvc_info_old, PIN_FLD_SERVICE_OBJ, 1, ebufp);
			srvc_catv_info_old = PIN_FLIST_SUBSTR_GET(srvc_info_old, MSO_FLD_CATV_INFO, 1, ebufp );
		}
		credit_limit_old = (pin_flist_t*)PIN_FLIST_FLD_GET(srvc_info_old, PIN_FLD_CREDIT_LIMIT, 1, ebufp );
		credit_floor_old = (pin_flist_t*)PIN_FLIST_FLD_GET(srvc_info_old, PIN_FLD_CREDIT_FLOOR, 1, ebufp );
	}

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	if (vp)
	{
		srvc_info_new = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_SERVICES, 0, 1, ebufp );
		if (srvc_info_new)
		{
			srvc_catv_info_new = PIN_FLIST_SUBSTR_GET(srvc_info_new, MSO_FLD_CATV_INFO, 1, ebufp );
		}
		credit_limit_new = (pin_flist_t*)PIN_FLIST_FLD_GET(srvc_info_new, PIN_FLD_CREDIT_LIMIT, 1, ebufp );
		credit_floor_new = (pin_flist_t*)PIN_FLIST_FLD_GET(srvc_info_new, PIN_FLD_CREDIT_FLOOR, 1, ebufp );
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srvc_info_new", srvc_info_new );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srvc_info_old", srvc_info_old );


	if (srvc_catv_info_new)
	{
		region_key_new = PIN_FLIST_FLD_GET(srvc_catv_info_new, MSO_FLD_REGION_KEY, 1, ebufp );
		bouquet_id_new = PIN_FLIST_FLD_GET(srvc_catv_info_new, MSO_FLD_BOUQUET_ID, 1, ebufp );
		caf_new = (int32*)PIN_FLIST_FLD_GET(srvc_catv_info_new, MSO_FLD_CARF_RECEIVED, 1, ebufp );
		cas_subs_id_new = PIN_FLIST_FLD_GET(srvc_catv_info_new, MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp );
	}

	/*******************************************************************************
	*  Modify REGION KEY 
	*******************************************************************************/
	if (region_key_new)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "region_key modification");
		region_key_old = PIN_FLIST_FLD_GET(srvc_catv_info_old, MSO_FLD_REGION_KEY, 1, ebufp );

		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		region_key_info = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_REGIONKEY, 0, ebufp);
		PIN_FLIST_FLD_SET(region_key_info, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(region_key_info, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp );
		PIN_FLIST_FLD_SET(region_key_info, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
		PIN_FLIST_FLD_SET(region_key_info, PIN_FLD_NEW_VALUE, region_key_new, ebufp );
		PIN_FLIST_FLD_SET(region_key_info, PIN_FLD_OLD_VALUE, region_key_old, ebufp );

		get_evt_lifecycle_template(ctxp, db, string_id_region_key,1, &lc_template, ebufp);
		sprintf(msg, lc_template, region_key_old, region_key_new, agreement_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_srvc_lc_event region_key flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, srvc_pdp, prog_name,
			msg, action_modify_region_key, event_region_key, lc_iflistp, ebufp);

		PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	}

	/*******************************************************************************
	*  Modify BOUQUET_ID
	*******************************************************************************/
	if (bouquet_id_new)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "bouquet_id modification");
		bouquet_id_old = PIN_FLIST_FLD_GET(srvc_catv_info_old, MSO_FLD_BOUQUET_ID, 1, ebufp );

		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		bouquet_id_info = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_BOUQUET, 0, ebufp);
		PIN_FLIST_FLD_SET(bouquet_id_info, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(bouquet_id_info, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp );
		PIN_FLIST_FLD_SET(bouquet_id_info, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
		PIN_FLIST_FLD_SET(bouquet_id_info, PIN_FLD_NEW_VALUE, bouquet_id_new, ebufp );
		PIN_FLIST_FLD_SET(bouquet_id_info, PIN_FLD_OLD_VALUE, bouquet_id_old, ebufp );

		get_evt_lifecycle_template(ctxp, db, string_id_bouquet_id, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, bouquet_id_old, bouquet_id_new, agreement_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_srvc_lc_event bouquet_id flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, srvc_pdp, prog_name,
			msg, action_modify_bouquet_id, event_bouquet_id, lc_iflistp, ebufp);

		PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	}

	
	/*******************************************************************************
	*  Modify CAF/CRF
	*******************************************************************************/
	if (caf_new)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CAF modification");
		caf_old = (int32*)PIN_FLIST_FLD_GET(srvc_catv_info_old, MSO_FLD_CARF_RECEIVED, 1, ebufp );

		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		caf_info = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_CAF_STATUS, 0, ebufp);
		PIN_FLIST_FLD_SET(caf_info, PIN_FLD_ACCOUNT_NO,          account_no, ebufp );
		PIN_FLIST_FLD_SET(caf_info, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp );
		PIN_FLIST_FLD_SET(caf_info, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
		PIN_FLIST_FLD_SET(caf_info, MSO_FLD_NEW_VALUE, caf_new, ebufp );
		PIN_FLIST_FLD_SET(caf_info, MSO_FLD_OLD_VALUE, caf_old, ebufp );

		if (*caf_new == 1)
		{
			strcpy(caf_status_str, "RECEIVED");
		}
		if (*caf_new == 0)
		{
			strcpy(caf_status_str, "NOT-RECEIVED");
		}
		get_evt_lifecycle_template(ctxp, db, string_id_caf, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, caf_status_str, agreement_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_srvc_lc_event caf flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, srvc_pdp, prog_name,
			msg, action_modify_caf, event_caf, lc_iflistp, ebufp);

		PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	}

	/*******************************************************************************
	*  Modify CAS SUBSCRIBER ID 
	*******************************************************************************/
	if (cas_subs_id_new)
	{

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "cas_subscriber_id modification");
		cas_subs_id_old = PIN_FLIST_FLD_GET(srvc_catv_info_old, MSO_FLD_CAS_SUBSCRIBER_ID, 1, ebufp );

		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		cas_subs_id_info = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_CAS_ID, 0, ebufp);
		PIN_FLIST_FLD_SET(cas_subs_id_info, PIN_FLD_ACCOUNT_NO,          account_no, ebufp );
		PIN_FLIST_FLD_SET(cas_subs_id_info, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp );
		PIN_FLIST_FLD_SET(cas_subs_id_info, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
		PIN_FLIST_FLD_SET(cas_subs_id_info, PIN_FLD_NEW_VALUE, cas_subs_id_new, ebufp );
		PIN_FLIST_FLD_SET(cas_subs_id_info, PIN_FLD_OLD_VALUE, cas_subs_id_old, ebufp );

		get_evt_lifecycle_template(ctxp, db, string_id_cas_subs_id, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, cas_subs_id_old, cas_subs_id_new, agreement_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_srvc_lc_event cas_subs_id flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, srvc_pdp, prog_name,
		msg, action_modify_cas_subs_id, event_cas_subs_id, lc_iflistp, ebufp);

		PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	}

	/*******************************************************************************
	*  Modify Personal BIT 
	*******************************************************************************/
	while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(srvc_info_new, MSO_FLD_PERSONAL_BIT_INFO,
	&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "personal_bit modification");
		pers_bit_name_new = PIN_FLIST_FLD_GET(vp, PIN_FLD_PARAM_NAME, 1, ebufp );
		pers_bit_value_new = PIN_FLIST_FLD_GET(vp, PIN_FLD_PARAM_VALUE, 1, ebufp );
		if (pers_bit_name_new)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "personal_bit modification");
			vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(srvc_info_old, MSO_FLD_PERSONAL_BIT_INFO, elem_id, 1, ebufp);
			if (vp1 )
			{
				pers_bit_name_old = PIN_FLIST_FLD_GET(vp1, PIN_FLD_PARAM_NAME, 1, ebufp );
				pers_bit_value_old = PIN_FLIST_FLD_GET(vp1, PIN_FLD_PARAM_VALUE, 1, ebufp );
			}
			

			if ( !vp1 || (pers_bit_name_new && pers_bit_value_new && pers_bit_name_old && pers_bit_value_new &&
			     (strcmp(pers_bit_name_new, pers_bit_name_old)   ==0 )  &&
			     (strcmp(pers_bit_value_new, pers_bit_value_old) !=0 ))
			   )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "personal_bit modification");
				lc_iflistp = PIN_FLIST_CREATE(ebufp);
				pers_bit_info_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_PERSONAL_BIT, 0, ebufp);
				pers_bit_info_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_PERSONAL_BIT, 1, ebufp);

				PIN_FLIST_FLD_SET(pers_bit_info_old, PIN_FLD_ACCOUNT_NO,          account_no, ebufp );
				//PIN_FLIST_FLD_SET(pers_bit_info_old, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp );
				PIN_FLIST_FLD_SET(pers_bit_info_old, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
				vp2 = (pin_flist_t*)PIN_FLIST_ELEM_ADD(pers_bit_info_old, MSO_FLD_PERSONAL_BIT_INFO, elem_id, ebufp );
				PIN_FLIST_FLD_SET(vp2, PIN_FLD_PARAM_NAME, pers_bit_name_old, ebufp );
				PIN_FLIST_FLD_SET(vp2, PIN_FLD_PARAM_VALUE, pers_bit_value_old, ebufp );

				PIN_FLIST_FLD_SET(pers_bit_info_new, PIN_FLD_ACCOUNT_NO,          account_no, ebufp );
				//PIN_FLIST_FLD_SET(pers_bit_info_new, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp );
				PIN_FLIST_FLD_SET(pers_bit_info_new, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp );
				vp2 = (pin_flist_t*)PIN_FLIST_ELEM_ADD(pers_bit_info_new, MSO_FLD_PERSONAL_BIT_INFO, elem_id, ebufp );
				PIN_FLIST_FLD_SET(vp2, PIN_FLD_PARAM_NAME, pers_bit_name_new, ebufp );
				PIN_FLIST_FLD_SET(vp2, PIN_FLD_PARAM_VALUE, pers_bit_value_new, ebufp );

				get_evt_lifecycle_template(ctxp, db, string_id_pers_bit, 1, &lc_template, ebufp);
				sprintf(msg, lc_template, pers_bit_value_new, pers_bit_name_new, agreement_no, account_no );
				if (lc_template)
				{
					free(lc_template);
				}

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_srvc_lc_event pers_bit_name flist:", lc_iflistp);

				fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, srvc_pdp, prog_name,
					msg, action_modify_pers_bit, event_pers_bit, lc_iflistp, ebufp);

				PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
			}
			
 		}
	}
	/*******************************************************************************
	*  Modify CREDIT LIMIT
	*******************************************************************************/
	if (credit_limit_new || credit_floor_new)
	{

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "credit_limit modification");

		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		credit_profile_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_CREDIT_LIMIT, 0, ebufp);
		credit_profile_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_CREDIT_LIMIT, 1, ebufp);

		PIN_FLIST_FLD_SET(credit_profile_old, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(credit_profile_old, MSO_FLD_LIMIT, credit_limit_old, ebufp );
		PIN_FLIST_FLD_SET(credit_profile_old, MSO_FLD_FLOOR, credit_floor_old, ebufp );

		PIN_FLIST_FLD_SET(credit_profile_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(credit_profile_new, MSO_FLD_LIMIT, credit_limit_new, ebufp );
		PIN_FLIST_FLD_SET(credit_profile_new, MSO_FLD_FLOOR, credit_floor_new, ebufp );
		

		get_evt_lifecycle_template(ctxp, db, string_id_cr_limit, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, "CATV", account_no );
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_srvc_lc_event cas_subs_id flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, srvc_pdp, prog_name,
		msg, action_modify_cr_limit, event_cr_limit, lc_iflistp, ebufp);

		PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	}


	return;
}


void
fm_mso_cust_mod_cust_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*acnt_info,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*nameinfo_existing = NULL;
	pin_flist_t		*contact_info_old = NULL;
	pin_flist_t		*contact_info_new = NULL;
	pin_flist_t		*nameinfo_input = NULL;
	pin_flist_t		*nameinfo_new = NULL;
	pin_flist_t		*nameinfo_old = NULL;
	pin_flist_t		*phones = NULL;

	pin_flist_t		*addr_info_bill = NULL;
	pin_flist_t		*addr_info_inst = NULL;
	pin_flist_t		*addr_info_comp = NULL;
	pin_flist_t		*nameinfo_bill_input = NULL;
	pin_flist_t		*nameinfo_inst_input = NULL;
	pin_flist_t		*nameinfo_comp_input = NULL;
	pin_flist_t		*nameinfo_bill_existing = NULL;
	pin_flist_t		*nameinfo_inst_existing = NULL;
	pin_flist_t		*nameinfo_comp_existing = NULL;
	pin_flist_t		*mso_cust_credentials_info = NULL;
	pin_flist_t		*nameinfo_cc_bill_existing = NULL;
	pin_flist_t		*nameinfo_cc_inst_existing = NULL;
	pin_flist_t		*nameinfo_cc_comp_existing = NULL;

	poid_t			*acc_pdp = NULL;
		
	int64			db = 0;
	int			string_id_contact = ID_MODIFY_CUSTOMER_CONTACT;
	int			string_id_address = ID_MODIFY_CUSTOMER_ADDRESS;
	int32			elem_id = 0;
	int32			addr_type_bill = 1;
	int32			addr_type_inst = 2;
	int32			addr_type_comp = 3;
	int32			flg_del_acnt_info=1;

	char			*event_contact = "/event/activity/mso_lifecycle/update_contact";
	char			*event_address = "/event/activity/mso_lifecycle/update_address";

	char			*action_name_contact = "MODIFY_CUSTOMER_CONTACT";
	char			*action_name_address = "MODIFY_CUSTOMER_ADDRESS";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			address_type[64]="";
	char			*prog_name = NULL;
	char			*account_no = NULL;

	pin_cookie_t		cookie = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;
	void			*vp2 = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_cust_lc_event", in_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if (acnt_info)
	{
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		nameinfo_existing = PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, 1, 1, ebufp);
		flg_del_acnt_info = 0;
	}
	else if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		nameinfo_existing = PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, 1, 1, ebufp);
	}
	db = PIN_POID_GET_DB(acc_pdp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "acnt_info", acnt_info );

	fm_mso_get_mso_cust_credentials(ctxp, acc_pdp, &mso_cust_credentials_info, ebufp);

	/*******************************************************************************/
	/*Update CONTACT inforrmation -start*/
	/*******************************************************************************/
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	contact_info_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_UPDATE_CONTACT, 0, ebufp);
	contact_info_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_UPDATE_CONTACT, 1, ebufp);

	PIN_FLIST_FLD_SET(contact_info_old, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
	PIN_FLIST_FLD_SET(contact_info_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp );

	//nameinfo_input = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_NAMEINFO, 1, 1, ebufp);
	nameinfo_input = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 1, ebufp);
	nameinfo_old = PIN_FLIST_ELEM_ADD(contact_info_old, PIN_FLD_NAMEINFO, 0, ebufp);
	nameinfo_new = PIN_FLIST_ELEM_ADD(contact_info_new, PIN_FLD_NAMEINFO, 1, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_input", nameinfo_input );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_existing", nameinfo_existing );

	vp = (char*)PIN_FLIST_FLD_GET(nameinfo_input, PIN_FLD_SALUTATION, 1, ebufp);
	if (vp)
	{
		vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_existing, PIN_FLD_SALUTATION, 1, ebufp);
		if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
		{
			PIN_FLIST_FLD_COPY(nameinfo_input,    PIN_FLD_SALUTATION, nameinfo_new, PIN_FLD_SALUTATION, ebufp);
			PIN_FLIST_FLD_COPY(nameinfo_existing, PIN_FLD_SALUTATION, nameinfo_old, PIN_FLD_SALUTATION, ebufp);
		}
 	}

	vp = (char*)PIN_FLIST_FLD_GET(nameinfo_input, PIN_FLD_COMPANY, 1, ebufp);
	if (vp)
	{
		vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_existing, PIN_FLD_COMPANY, 1, ebufp);
		if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
		{
			PIN_FLIST_FLD_COPY(nameinfo_input,    PIN_FLD_COMPANY, nameinfo_new, PIN_FLD_COMPANY, ebufp);
			PIN_FLIST_FLD_COPY(nameinfo_existing, PIN_FLD_COMPANY, nameinfo_old, PIN_FLD_COMPANY, ebufp);
 		}
	}

	vp = (char*)PIN_FLIST_FLD_GET(nameinfo_input, PIN_FLD_FIRST_NAME, 1, ebufp);
	if (vp)
	{
		vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_existing, PIN_FLD_FIRST_NAME, 1, ebufp);
		if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
		{
			PIN_FLIST_FLD_COPY(nameinfo_input,    PIN_FLD_FIRST_NAME, nameinfo_new, PIN_FLD_FIRST_NAME, ebufp);
			PIN_FLIST_FLD_COPY(nameinfo_existing, PIN_FLD_FIRST_NAME, nameinfo_old, PIN_FLD_FIRST_NAME, ebufp);
		}
	}
	vp = (char*)PIN_FLIST_FLD_GET(nameinfo_input, PIN_FLD_MIDDLE_NAME, 1, ebufp);
	if (vp)
	{
		vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_existing, PIN_FLD_MIDDLE_NAME, 1, ebufp);
		if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
		{
			PIN_FLIST_FLD_COPY(nameinfo_input,    PIN_FLD_MIDDLE_NAME, nameinfo_new, PIN_FLD_MIDDLE_NAME, ebufp);
			PIN_FLIST_FLD_COPY(nameinfo_existing, PIN_FLD_MIDDLE_NAME, nameinfo_old, PIN_FLD_MIDDLE_NAME, ebufp);
		}
 	}

	vp = (char*)PIN_FLIST_FLD_GET(nameinfo_input, PIN_FLD_LAST_NAME, 1, ebufp);
	if (vp)
	{
		vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_existing, PIN_FLD_LAST_NAME, 1, ebufp);
		if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
		{
			PIN_FLIST_FLD_COPY(nameinfo_input,    PIN_FLD_LAST_NAME, nameinfo_new, PIN_FLD_LAST_NAME, ebufp);
			PIN_FLIST_FLD_COPY(nameinfo_existing, PIN_FLD_LAST_NAME, nameinfo_old, PIN_FLD_LAST_NAME, ebufp);
		}
 	}

	vp = (char*)PIN_FLIST_FLD_GET(nameinfo_input, PIN_FLD_EMAIL_ADDR, 1, ebufp);
	if (vp)
	{
		vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_existing, PIN_FLD_EMAIL_ADDR, 1, ebufp);
		if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
		{
			PIN_FLIST_FLD_COPY(nameinfo_input,    PIN_FLD_EMAIL_ADDR, nameinfo_new, PIN_FLD_EMAIL_ADDR, ebufp);
			PIN_FLIST_FLD_COPY(nameinfo_existing, PIN_FLD_EMAIL_ADDR, nameinfo_old, PIN_FLD_EMAIL_ADDR, ebufp);
		}
	}

	if (nameinfo_input)
	{
		while((phones = PIN_FLIST_ELEM_GET_NEXT(nameinfo_input, PIN_FLD_PHONES,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			vp =  (char*)PIN_FLIST_FLD_GET(phones, PIN_FLD_PHONE, 1, ebufp);
			vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(nameinfo_existing, PIN_FLD_PHONES, elem_id, 1, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Phones old", vp1);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Phones new", phones);

			if (vp)
			{
				if (vp1)
				{
					vp2 =  (char*)PIN_FLIST_FLD_GET(vp1, PIN_FLD_PHONE, 1, ebufp);
				}

				if (!vp1 || (vp2 && strcmp(vp2, vp)!=0 ))
				{
					PIN_FLIST_ELEM_SET(contact_info_new, phones, PIN_FLD_PHONES, elem_id, ebufp );
					PIN_FLIST_ELEM_COPY(nameinfo_existing, PIN_FLD_PHONES, elem_id, contact_info_old, PIN_FLD_PHONES, elem_id, ebufp );
				}
			}
		}
	}

	if (nameinfo_new  && PIN_FLIST_COUNT(nameinfo_new, ebufp)>0 )
	{
		get_evt_lifecycle_template(ctxp, db, string_id_contact,1, &lc_template, ebufp);
		sprintf(msg, lc_template, account_no );
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_cust_lc_event gen_lifecycle_evt flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,
		msg, action_name_contact, event_contact, lc_iflistp, ebufp);
		
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	/*******************************************************************************/
	/*Update CONTACT inforrmation -end*/
	/*******************************************************************************/

	/*******************************************************************************/
	/*Update ADDRESS inforrmation -start*/
	/*******************************************************************************/
 	lc_iflistp = PIN_FLIST_CREATE(ebufp);

	nameinfo_bill_input = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_NAMEINFO, 1, 1, ebufp);
	nameinfo_inst_input = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_NAMEINFO, 2, 1, ebufp);
	nameinfo_comp_input = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_NAMEINFO, 3, 1, ebufp);


	if (nameinfo_bill_input)
	{
		addr_info_bill = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_UPDATE_ADDRESS, 1, ebufp);
		PIN_FLIST_FLD_SET(addr_info_bill, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(addr_info_bill, PIN_FLD_TYPE, &addr_type_bill, ebufp );

		nameinfo_bill_existing =  PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, 1, 1, ebufp);
		nameinfo_cc_bill_existing =  PIN_FLIST_ELEM_GET(mso_cust_credentials_info, PIN_FLD_NAMEINFO, 1, 1, ebufp);
		
		nameinfo_old = PIN_FLIST_ELEM_ADD(addr_info_bill, PIN_FLD_NAMEINFO, 0, ebufp);
		nameinfo_new = PIN_FLIST_ELEM_ADD(addr_info_bill, PIN_FLD_NAMEINFO, 1, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_bill_input", nameinfo_bill_input );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_bill_existing", nameinfo_bill_existing );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_cc_bill_existing", nameinfo_cc_bill_existing );

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, PIN_FLD_ADDRESS, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_existing, PIN_FLD_ADDRESS, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,    PIN_FLD_ADDRESS, nameinfo_new, PIN_FLD_ADDRESS, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_bill_existing, PIN_FLD_ADDRESS, nameinfo_old, PIN_FLD_ADDRESS, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, PIN_FLD_CITY, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_existing, PIN_FLD_CITY, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,    PIN_FLD_CITY, nameinfo_new, PIN_FLD_CITY, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_bill_existing, PIN_FLD_CITY, nameinfo_old, PIN_FLD_CITY, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, PIN_FLD_STATE, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_existing, PIN_FLD_STATE, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,    PIN_FLD_STATE, nameinfo_new, PIN_FLD_STATE, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_bill_existing, PIN_FLD_STATE, nameinfo_old, PIN_FLD_STATE, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, PIN_FLD_COUNTRY, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_existing, PIN_FLD_COUNTRY, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,    PIN_FLD_COUNTRY, nameinfo_new, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_bill_existing, PIN_FLD_COUNTRY, nameinfo_old, PIN_FLD_COUNTRY, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, PIN_FLD_ZIP, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_existing, PIN_FLD_ZIP, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,    PIN_FLD_ZIP, nameinfo_new, PIN_FLD_ZIP, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_bill_existing, PIN_FLD_ZIP, nameinfo_old, PIN_FLD_ZIP, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, MSO_FLD_AREA_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_bill_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_bill_existing, MSO_FLD_AREA_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,        MSO_FLD_AREA_NAME, nameinfo_new, MSO_FLD_AREA_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_bill_existing,  MSO_FLD_AREA_NAME, nameinfo_old, MSO_FLD_AREA_NAME, ebufp);
			}
		    }
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, MSO_FLD_LOCATION_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_bill_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_bill_existing, MSO_FLD_LOCATION_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,       MSO_FLD_LOCATION_NAME, nameinfo_new, MSO_FLD_LOCATION_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_bill_existing, MSO_FLD_LOCATION_NAME, nameinfo_old, MSO_FLD_LOCATION_NAME, ebufp);
			}
		    }
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, MSO_FLD_STREET_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_bill_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_bill_existing, MSO_FLD_STREET_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,       MSO_FLD_STREET_NAME, nameinfo_new, MSO_FLD_STREET_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_bill_existing, MSO_FLD_STREET_NAME, nameinfo_old, MSO_FLD_STREET_NAME, ebufp);
			}
		    }
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, MSO_FLD_BUILDING_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_bill_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_bill_existing, MSO_FLD_BUILDING_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,       MSO_FLD_BUILDING_NAME, nameinfo_new, MSO_FLD_BUILDING_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_bill_existing, MSO_FLD_BUILDING_NAME, nameinfo_old, MSO_FLD_BUILDING_NAME, ebufp);
			}
		    }
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, MSO_FLD_LANDMARK, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_bill_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_bill_existing, MSO_FLD_LANDMARK, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,       MSO_FLD_LANDMARK, nameinfo_new, MSO_FLD_LANDMARK, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_bill_existing, MSO_FLD_LANDMARK, nameinfo_old, MSO_FLD_LANDMARK, ebufp);
			}
		    }
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_bill_input, MSO_FLD_ADDRESS_NODE, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_bill_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_bill_existing, MSO_FLD_ADDRESS_NODE, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_bill_input,       MSO_FLD_ADDRESS_NODE, nameinfo_new, MSO_FLD_ADDRESS_NODE, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_bill_existing, MSO_FLD_ADDRESS_NODE, nameinfo_old, MSO_FLD_ADDRESS_NODE, ebufp);
			}
		    }
	        }
	}

	if (nameinfo_inst_input)
	{
		addr_info_inst = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_UPDATE_ADDRESS, 2, ebufp);
		PIN_FLIST_FLD_SET(addr_info_inst, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(addr_info_inst, PIN_FLD_TYPE, &addr_type_inst, ebufp );

		nameinfo_inst_existing =  PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, 2, 1, ebufp);
		nameinfo_cc_inst_existing =  PIN_FLIST_ELEM_GET(mso_cust_credentials_info, PIN_FLD_NAMEINFO, 2, 1, ebufp);

		nameinfo_old = PIN_FLIST_ELEM_ADD(addr_info_inst, PIN_FLD_NAMEINFO, 0, ebufp);
		nameinfo_new = PIN_FLIST_ELEM_ADD(addr_info_inst, PIN_FLD_NAMEINFO, 1, ebufp);

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, PIN_FLD_ADDRESS, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_existing, PIN_FLD_ADDRESS, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,    PIN_FLD_ADDRESS, nameinfo_new, PIN_FLD_ADDRESS, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_inst_existing, PIN_FLD_ADDRESS, nameinfo_old, PIN_FLD_ADDRESS, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, PIN_FLD_CITY, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_existing, PIN_FLD_CITY, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,    PIN_FLD_CITY, nameinfo_new, PIN_FLD_CITY, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_inst_existing, PIN_FLD_CITY, nameinfo_old, PIN_FLD_CITY, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, PIN_FLD_STATE, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_existing, PIN_FLD_STATE, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,    PIN_FLD_STATE, nameinfo_new, PIN_FLD_STATE, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_inst_existing, PIN_FLD_STATE, nameinfo_old, PIN_FLD_STATE, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, PIN_FLD_COUNTRY, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_existing, PIN_FLD_COUNTRY, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,    PIN_FLD_COUNTRY, nameinfo_new, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_inst_existing, PIN_FLD_COUNTRY, nameinfo_old, PIN_FLD_COUNTRY, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, PIN_FLD_ZIP, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_existing, PIN_FLD_ZIP, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,    PIN_FLD_ZIP, nameinfo_new, PIN_FLD_ZIP, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_inst_existing, PIN_FLD_ZIP, nameinfo_old, PIN_FLD_ZIP, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AREA, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(acnt_info, MSO_FLD_AREA, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(in_flistp,    MSO_FLD_AREA,      nameinfo_new, MSO_FLD_AREA_CODE, ebufp);
				PIN_FLIST_FLD_COPY(acnt_info,    MSO_FLD_AREA,      nameinfo_old, MSO_FLD_AREA_CODE, ebufp);
			}
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, MSO_FLD_AREA_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_inst_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_inst_existing, MSO_FLD_AREA_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,        MSO_FLD_AREA_NAME, nameinfo_new, MSO_FLD_AREA_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_inst_existing,  MSO_FLD_AREA_NAME, nameinfo_old, MSO_FLD_AREA_NAME, ebufp);
			}
		    }
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, MSO_FLD_LOCATION_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_inst_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_inst_existing, MSO_FLD_LOCATION_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,       MSO_FLD_LOCATION_NAME, nameinfo_new, MSO_FLD_LOCATION_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_inst_existing, MSO_FLD_LOCATION_NAME, nameinfo_old, MSO_FLD_LOCATION_NAME, ebufp);
			}
		    }
		}

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, MSO_FLD_STREET_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_inst_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_inst_existing, MSO_FLD_STREET_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,       MSO_FLD_STREET_NAME, nameinfo_new, MSO_FLD_STREET_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_inst_existing, MSO_FLD_STREET_NAME, nameinfo_old, MSO_FLD_STREET_NAME, ebufp);
			}
		    }
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, MSO_FLD_BUILDING_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_inst_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_inst_existing, MSO_FLD_BUILDING_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,       MSO_FLD_BUILDING_NAME, nameinfo_new, MSO_FLD_BUILDING_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_inst_existing, MSO_FLD_BUILDING_NAME, nameinfo_old, MSO_FLD_BUILDING_NAME, ebufp);
			}
		    }
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, MSO_FLD_LANDMARK, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_inst_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_inst_existing, MSO_FLD_LANDMARK, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,       MSO_FLD_LANDMARK, nameinfo_new, MSO_FLD_LANDMARK, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_inst_existing, MSO_FLD_LANDMARK, nameinfo_old, MSO_FLD_LANDMARK, ebufp);
			}
		    }
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_inst_input, MSO_FLD_ADDRESS_NODE, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_inst_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_inst_existing, MSO_FLD_ADDRESS_NODE, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_inst_input,       MSO_FLD_ADDRESS_NODE, nameinfo_new, MSO_FLD_ADDRESS_NODE, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_inst_existing, MSO_FLD_ADDRESS_NODE, nameinfo_old, MSO_FLD_ADDRESS_NODE, ebufp);
			}
		     }
	        }
	}

	if (nameinfo_comp_input)
     	{
		addr_info_comp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_UPDATE_ADDRESS, 3, ebufp);
		PIN_FLIST_FLD_SET(addr_info_comp, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(addr_info_comp, PIN_FLD_TYPE, &addr_type_comp, ebufp );

		nameinfo_comp_existing =  PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, 3, 1, ebufp);
		nameinfo_cc_comp_existing =  PIN_FLIST_ELEM_GET(mso_cust_credentials_info, PIN_FLD_NAMEINFO, 3, 1, ebufp);

		nameinfo_old = PIN_FLIST_ELEM_ADD(addr_info_comp, PIN_FLD_NAMEINFO, 0, ebufp);
		nameinfo_new = PIN_FLIST_ELEM_ADD(addr_info_comp, PIN_FLD_NAMEINFO, 1, ebufp);

		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, PIN_FLD_ADDRESS, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_existing, PIN_FLD_ADDRESS, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
				{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,    PIN_FLD_ADDRESS, nameinfo_new, PIN_FLD_ADDRESS, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_comp_existing, PIN_FLD_ADDRESS, nameinfo_old, PIN_FLD_ADDRESS, ebufp);
			}
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, PIN_FLD_CITY, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_existing, PIN_FLD_CITY, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,    PIN_FLD_CITY, nameinfo_new, PIN_FLD_CITY, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_comp_existing, PIN_FLD_CITY, nameinfo_old, PIN_FLD_CITY, ebufp);
			}
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, PIN_FLD_STATE, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_existing, PIN_FLD_STATE, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,    PIN_FLD_STATE, nameinfo_new, PIN_FLD_STATE, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_comp_existing, PIN_FLD_STATE, nameinfo_old, PIN_FLD_STATE, ebufp);
			}
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, PIN_FLD_COUNTRY, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_existing, PIN_FLD_COUNTRY, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,    PIN_FLD_COUNTRY, nameinfo_new, PIN_FLD_COUNTRY, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_comp_existing, PIN_FLD_COUNTRY, nameinfo_old, PIN_FLD_COUNTRY, ebufp);
			}
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, PIN_FLD_ZIP, 1, ebufp);
		if (vp)
		{
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_existing, PIN_FLD_ZIP, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,    PIN_FLD_ZIP, nameinfo_new, PIN_FLD_ZIP, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_comp_existing, PIN_FLD_ZIP, nameinfo_old, PIN_FLD_ZIP, ebufp);
			}
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, MSO_FLD_AREA_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_comp_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_comp_existing, MSO_FLD_AREA_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,        MSO_FLD_AREA_NAME, nameinfo_new, MSO_FLD_AREA_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_comp_existing,  MSO_FLD_AREA_NAME, nameinfo_old, MSO_FLD_AREA_NAME, ebufp);
			}
		    }
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, MSO_FLD_LOCATION_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_comp_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_comp_existing, MSO_FLD_LOCATION_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,       MSO_FLD_LOCATION_NAME, nameinfo_new, MSO_FLD_LOCATION_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_comp_existing, MSO_FLD_LOCATION_NAME, nameinfo_old, MSO_FLD_LOCATION_NAME, ebufp);
			}
		    }
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, MSO_FLD_STREET_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_comp_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_comp_existing, MSO_FLD_STREET_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,       MSO_FLD_STREET_NAME, nameinfo_new, MSO_FLD_STREET_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_comp_existing, MSO_FLD_STREET_NAME, nameinfo_old, MSO_FLD_STREET_NAME, ebufp);
			}
		    }
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, MSO_FLD_BUILDING_NAME, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_comp_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_comp_existing, MSO_FLD_BUILDING_NAME, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,       MSO_FLD_BUILDING_NAME, nameinfo_new, MSO_FLD_BUILDING_NAME, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_comp_existing, MSO_FLD_BUILDING_NAME, nameinfo_old, MSO_FLD_BUILDING_NAME, ebufp);
			}
		    }
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, MSO_FLD_LANDMARK, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_comp_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_comp_existing, MSO_FLD_LANDMARK, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,       MSO_FLD_LANDMARK, nameinfo_new, MSO_FLD_LANDMARK, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_comp_existing, MSO_FLD_LANDMARK, nameinfo_old, MSO_FLD_LANDMARK, ebufp);
			}
		    }
		}
		vp = (char*)PIN_FLIST_FLD_GET(nameinfo_comp_input, MSO_FLD_ADDRESS_NODE, 1, ebufp);
		if (vp)
		{
		    if(nameinfo_cc_comp_existing)
		    {
			vp1 = (char*)PIN_FLIST_FLD_GET(nameinfo_cc_comp_existing, MSO_FLD_ADDRESS_NODE, 1, ebufp);
			if (!vp1 || (vp1 && strcmp (vp, vp1)!=0) )
			{
				PIN_FLIST_FLD_COPY(nameinfo_comp_input,       MSO_FLD_ADDRESS_NODE, nameinfo_new, MSO_FLD_ADDRESS_NODE, ebufp);
				PIN_FLIST_FLD_COPY(nameinfo_cc_comp_existing, MSO_FLD_ADDRESS_NODE, nameinfo_old, MSO_FLD_ADDRESS_NODE, ebufp);
			}
		    }
	        }
	}

	if (  (addr_info_bill  && PIN_FLIST_COUNT(addr_info_bill, ebufp)>2) && 
	      (addr_info_inst  && PIN_FLIST_COUNT(addr_info_inst, ebufp)>2) &&
	      (addr_info_comp  && PIN_FLIST_COUNT(addr_info_comp, ebufp)>2)
	   )
	{
		strcpy(address_type, "Billing, Installation and Company");
	}
	else if (  (addr_info_bill  && PIN_FLIST_COUNT(addr_info_bill, ebufp)>2) && 
	           (addr_info_inst  && PIN_FLIST_COUNT(addr_info_inst, ebufp)>2)
	        )
	{
		strcpy(address_type, "Billing and Installation");
	}
	else if (  (addr_info_bill  && PIN_FLIST_COUNT(addr_info_bill, ebufp)>2) && 
	           (addr_info_comp  && PIN_FLIST_COUNT(addr_info_comp, ebufp)>2)
	        )
	{
		strcpy(address_type, "Billing and Company");
	}
	else if (  (addr_info_comp  && PIN_FLIST_COUNT(addr_info_comp, ebufp)>2) && 
	           (addr_info_inst  && PIN_FLIST_COUNT(addr_info_inst, ebufp)>2)
	        )
	{
		strcpy(address_type, "Installation and Company");
	}
	else if (  (addr_info_bill  && PIN_FLIST_COUNT(addr_info_bill, ebufp)>2)
	        )
	{
		strcpy(address_type, "Billing");
	}
	else if (  (addr_info_inst  && PIN_FLIST_COUNT(addr_info_inst, ebufp)>2)
	        )
	{
		strcpy(address_type, "Installation");
	}
	else if (  (addr_info_comp  && PIN_FLIST_COUNT(addr_info_comp, ebufp)>2)
	        )
	{
		strcpy(address_type, "Company");
	}


	if (  (addr_info_bill  && PIN_FLIST_COUNT(addr_info_bill, ebufp)>2) || 
	      (addr_info_inst  && PIN_FLIST_COUNT(addr_info_inst, ebufp)>2) ||
	      (addr_info_comp  && PIN_FLIST_COUNT(addr_info_comp, ebufp)>2)
	   )
	{
		get_evt_lifecycle_template(ctxp, db, string_id_address,1, &lc_template, ebufp);
		sprintf(msg, lc_template, address_type, account_no );
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_cust_lc_event gen_lifecycle_evt flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,
		msg, action_name_address, event_address, lc_iflistp, ebufp);
		
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);

	/*******************************************************************************/
	/*Update ADDRESS inforrmation -end*/
	/*******************************************************************************/


	if (flg_del_acnt_info)
	{
			PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	}
	PIN_FLIST_DESTROY_EX(&mso_cust_credentials_info, NULL);	 
	return;
}


/**************************************************************************
* Function:     mso_fld_refund_reverse_gen_lc_event()
* Owner:        Hrish Kulkarni
* Decription:   This module creates the lifecycle event for refund reversal
*
**************************************************************************/
void
fm_mso_refund_reverse_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*au_oflistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t             *res_flistp = NULL;

	
	poid_t                  *user_id = NULL;
	poid_t                  *acct_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	
	char			*event = "/event/activity/mso_lifecycle/refund_reversal";
	char			msg[255]="";
	char			*action_name = "REFUND_REVERSAL";
	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*prog_name = NULL;
	char			*descr = NULL;
	char			*refund_no = NULL;
	char			*amount_str = NULL;

	pin_decimal_t		*tot_amount;

	int64			db = 1;

	int32			string_id = ID_REFUND_REVERSAL;
	int32			*reason_id = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_fld_refund_reverse_gen_lc_event Input FList:", i_flistp);
	
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	reason_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 1, ebufp);
	refund_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_REFUND_NO, 1, ebufp);
	tot_amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp);
	if (tot_amount)
	{
			amount_str = pbo_decimal_to_str(tot_amount, ebufp);
	}

	res_flistp = PIN_FLIST_ELEM_GET(au_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if(res_flistp)
	{
		acct_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
		srvc_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
		if (acct_pdp)
		{
			fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
			account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		}
		if (srvc_pdp)
		{
			fm_mso_get_srvc_info(ctxp, res_flistp, &srvc_info, ebufp);
			agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
		}
 	}

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_REFREV, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_REFUND_NO, lc_temp_flistp, MSO_FLD_REFUND_NO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, lc_temp_flistp, PIN_FLD_REASON_ID, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_REFUND_AMOUNT, tot_amount, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event Event Flist:", lc_iflistp);

	get_evt_lifecycle_template(ctxp, db, string_id,1, &lc_template, ebufp);
	sprintf(msg, lc_template, amount_str, refund_no, agreement_no, account_no );

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	if (amount_str)
	{
		free(amount_str);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	return;
}

void
fm_mso_cr_dr_note_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *out_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*res_flistp = NULL;
	
	poid_t			*acct_pdp = NULL;
	
	char			*event = "/event/activity/mso_lifecycle/cr_dr_note";
	char			msg[255]="";
	char			*action_name = "CREDIT_DEBIT_NOTE";
	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*prog_name = NULL;
	char			*descr = NULL;
	char			*recpt_no = NULL;
	char			*amount_str = NULL;

	pin_decimal_t		*amount = NULL;
	pin_decimal_t		*abs_amount= NULL;

	int64			db = 1;

	int32			string_id = ID_CREDIT_DEBIT_NOTE;
	int32			string_ver = VER_CR_NOTE;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_crdr_note_gen_lc_event Input FList:", i_flistp);

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	recpt_no = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_RECEIPT_NO, 1, ebufp);
	amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp);
	if (amount && pbo_decimal_sign(amount, ebufp) == -1 )
	{
		abs_amount = pbo_decimal_negate(amount, ebufp);
		amount_str = pbo_decimal_to_str(abs_amount, ebufp);
		string_ver = VER_CR_NOTE;
	}
	else if (amount)
	{
		amount_str = pbo_decimal_to_str(amount, ebufp);
		string_ver = VER_DR_NOTE;
	}

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CRDR_NOTE, 0, ebufp);

	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,  account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_RECEIPT_NO,  recpt_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT,  amount , ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_gen_event Event FList:", lc_iflistp);

	get_evt_lifecycle_template(ctxp, db, string_id, string_ver, &lc_template, ebufp);
	sprintf(msg, lc_template, amount_str, recpt_no, account_no );

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
 	if (amount_str)
	{
		free(amount_str);
	}
	if ( !pbo_decimal_is_null(abs_amount, ebufp) )
	{
		pbo_decimal_destroy(&abs_amount);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	return;
}


void
mso_op_adjustment_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *s_oflistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *adj_flistp = NULL;
	pin_flist_t             *evt_flistp = NULL;  
	pin_flist_t		*ro_flistp = NULL;
	pin_flist_t		*read_bill_oflist = NULL;

	poid_t                  *ip_pdp = NULL;
	poid_t                  *acct_pdp = NULL;
	poid_t                  *event_pdp = NULL;

	pin_decimal_t		*amount = NULL;
	pin_decimal_t		*abs_amount = NULL;

	char                    *event = "/event/activity/mso_lifecycle/adjustment";
	char			msg[255]="";
	char			adj_type[16]="";
	char			last_str[64]="";
	char			*action_name = "ADJUSTMENT";
	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*prog_name = NULL;
	char			*descr = NULL;
	char			*recpt_no = NULL;
	char			*amount_str = NULL;
	char			*disp_name = NULL;
	char			*bill_no = NULL;

	int64			db = 1;

	int32			string_id = ID_ADJUSTMENT;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_adjustment_gen_lc_event Input FList:", i_flistp);

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	disp_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ORG_DISPLAY_NAME, 1, ebufp);

	if (acct_pdp && strcmp(PIN_POID_GET_TYPE(acct_pdp), "/account") == 0 )
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		strcpy(last_str, "A/C No: ") ;
		strcat(last_str, account_no) ;
	}
	else if (acct_pdp && strcmp(PIN_POID_GET_TYPE(acct_pdp), "/bill") == 0)
	{
		bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO, 1, ebufp);
		strcpy(last_str, "Bill No: ") ;
		if (!bill_no)
		{
			
			fm_mso_read_bill(ctxp, i_flistp, &read_bill_oflist, ebufp);
			if (read_bill_oflist)
			{
				bill_no = PIN_FLIST_FLD_GET(read_bill_oflist, PIN_FLD_BILL_NO, 1, ebufp);
			}
		}

		if (bill_no)
		{
			strcat(last_str, bill_no) ;
		}
		fm_mso_get_acntinfo_from_bill(ctxp, i_flistp, &acnt_info, ebufp); 
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		acct_pdp   = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_POID, 1, ebufp);
	}

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_ADJUSTMENT, 0, ebufp);

	ip_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
	if (strcmp(disp_name,EVENT_ADJUSTMENT) !=0 && strcmp(disp_name,ACCOUNT_ADJUSTMENT) !=0)
	{
		amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp );
	}
	else
	{
		acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
		if(strcmp(disp_name,EVENT_ADJUSTMENT) ==0)
		{
			adj_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ADJUSTMENT_INFO, 0, 1, ebufp);
			amount = PIN_FLIST_FLD_GET(adj_flistp, PIN_FLD_AMOUNT, 0, ebufp );
			res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
			evt_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, 1, ebufp);
		}
		else
		{
			amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp );
			res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		}
	}

	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, amount, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, disp_name, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Event Flist:", s_oflistp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_gen_event Event Flist:", lc_iflistp);

	if (amount && pbo_decimal_sign(amount, ebufp) == -1 )
	{
		abs_amount = pbo_decimal_negate(amount, ebufp);
		amount_str = pbo_decimal_to_str(abs_amount, ebufp);
		strcpy(adj_type, "CREDIT");

	}
	else if (amount)
	{
		amount_str = pbo_decimal_to_str(amount, ebufp);
		strcpy(adj_type, "DEBIT");
	}

	
	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, adj_type ,disp_name , amount_str, last_str );

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
 	if (amount_str)
	{
		free(amount_str);
	}
	if ( !pbo_decimal_is_null(abs_amount, ebufp) )
	{
		pbo_decimal_destroy(&abs_amount);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);	
	PIN_FLIST_DESTROY_EX(&read_bill_oflist, NULL);
	return;
}



void
fm_mso_topup_prepaid_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*bal_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t		*plan_info  = NULL;

	
	poid_t                  *plan_pdp = NULL;
	poid_t                  *acct_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	
	char			*event = "/event/activity/mso_lifecycle/topup_plan";
	char			msg[255]="";
	char			service_type[16]="";
	char			*action_name = "TOPUP_PREPAID";
	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*prog_name = NULL;
	char			*descr = NULL;
	char			*refund_no = NULL;
	char			*amount_str = NULL;
	char			*plan_code = NULL;
	char			rollover_amount_str[255] = "";

	pin_decimal_t		*disc_amount = NULL;
	pin_decimal_t		*disc_percent = NULL;
	pin_decimal_t		*rollover_bal = NULL;
	pin_decimal_t		*curr_bal = NULL;
	pin_decimal_t		*zero_decimal = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*abs_rollover_bal = NULL;

	int64			db = 1;

	int32			string_id = ID_TOPUP_PREPAID;
	int32			elem_id = 0;
	int32			elem_id1 = 0;

	void			*vp = NULL;
	void			*vp1 = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_topup_prepaid_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_topup_prepaid_lc_event Balance FList:", bal_flistp);
	
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp)
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);

		if( srvc_pdp && (strcmp(PIN_POID_GET_TYPE(srvc_pdp),"/service/catv")) == 0 )
		{
			strcpy(service_type, "CATV") ;
		}
		else if ( srvc_pdp && (strcmp(PIN_POID_GET_TYPE(srvc_pdp),"/service/telco/broadband")) == 0)
		{
			strcpy(service_type, "BROADBAND");
		}

	}
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	disc_amount  = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DISC_AMOUNT, 1, ebufp);
	disc_percent = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DISC_PERCENT, 1, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_TOPUP_PLAN, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

	fm_mso_cust_get_plan_code(ctxp, i_flistp, &plan_info, ebufp );
	if (plan_info)
	{
		plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp);
	}
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp);

 	if (bal_flistp)
	{	
		while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_BALANCES,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			
			if (elem_id == MSO_FREE_MB  || 	 // Free MB for limited plan
			    elem_id == MSO_TRCK_USG || 	 
			    elem_id == MSO_FUP_TRACK     // FUP quota MB remaining
			   )
			{
				rollover_bal = pbo_decimal_from_str("0.0", ebufp);
				elem_id1 = 0;
				cookie1 = NULL;
				while((vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(vp, PIN_FLD_SUB_BALANCES,
					&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
				{
					curr_bal = PIN_FLIST_FLD_GET(vp1, PIN_FLD_CURRENT_BAL, 1, ebufp);
					if (curr_bal)
					{
						pbo_decimal_add_assign(rollover_bal, curr_bal, ebufp );
					}
				}
				
				if (pbo_decimal_compare(rollover_bal, zero_decimal, ebufp)!=0)
				{
					if (elem_id ==MSO_FREE_MB )
					{
						abs_rollover_bal = pbo_decimal_negate(rollover_bal, ebufp);
						vp1 = (char*)pbo_decimal_to_str(abs_rollover_bal, ebufp);
						sprintf(rollover_amount_str, "Free MB %s carry forwarded.", vp1);

					}

					if (elem_id ==MSO_FUP_TRACK )
					{
						abs_rollover_bal = pbo_decimal_negate(rollover_bal, ebufp);
						vp1 = (char*)pbo_decimal_to_str(abs_rollover_bal, ebufp);
						sprintf(rollover_amount_str, "FUP quota MB %s carry forwarded.", vp1);
					}
					vp1 = PIN_FLIST_ELEM_ADD(lc_temp_flistp, PIN_FLD_BALANCES, elem_id, ebufp);
					PIN_FLIST_FLD_SET(vp1, PIN_FLD_RESOURCE_ID, &elem_id, ebufp);
					PIN_FLIST_FLD_PUT(vp1, PIN_FLD_ROLLOVER_UNITS, rollover_bal, ebufp);
				}
			}
		}
	}

	if (disc_amount)
	{
		amount_str = pbo_decimal_to_str(disc_amount, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_DISC_AMOUNT,disc_amount, ebufp );
	
		get_evt_lifecycle_template(ctxp, db, string_id,1, &lc_template, ebufp);
		sprintf(msg, lc_template, amount_str, plan_code, service_type, agreement_no, account_no, rollover_amount_str );
		if (lc_template)
		{
			free(lc_template);
		}

	}
	else if (disc_percent)
	{
		amount_str = pbo_decimal_to_str(disc_percent, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_DISC_PERCENT,disc_percent, ebufp );
	
		get_evt_lifecycle_template(ctxp, db, string_id,2, &lc_template, ebufp);
		sprintf(msg, lc_template, amount_str, plan_code, service_type,agreement_no, account_no, rollover_amount_str );
		if (lc_template)
		{
			free(lc_template);
		}
	}
	else
	{
		get_evt_lifecycle_template(ctxp, db, string_id, 3, &lc_template, ebufp);
		sprintf(msg, lc_template, plan_code, service_type,agreement_no, account_no, rollover_amount_str );
		if (lc_template)
		{
			free(lc_template);
		}
	}


 	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event Event Flist:", lc_iflistp);


	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


	if (amount_str)
	{
		free(amount_str);
	}
	if ( !pbo_decimal_is_null(zero_decimal, ebufp) )
	{
		pbo_decimal_destroy(&zero_decimal);
	}
	if ( !pbo_decimal_is_null(abs_rollover_bal, ebufp) )
	{
		pbo_decimal_destroy(&abs_rollover_bal);
	}


	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	PIN_FLIST_DESTROY_EX(&plan_info, NULL);
	return;
}

void
fm_mso_renew_prepaid_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*bal_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t		*plan_info  = NULL;

	
	poid_t                  *plan_pdp = NULL;
	poid_t                  *acct_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	poid_t                  *srvc_netflix_pdp = NULL;
	
	char			*event = "/event/activity/mso_lifecycle/renew_plan";
	char			msg[255]="";
	char			service_type[16]="";
	char			*action_name = "RENEW_PREPAID";
	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*prog_name = NULL;
	char			*descr = NULL;
	char			*refund_no = NULL;
	char			*amount_str = NULL;
	char			*plan_code = NULL;
	char			rollover_amount_str[255] = "";

	pin_decimal_t		*disc_amount = NULL;
	pin_decimal_t		*disc_percent = NULL;
	pin_decimal_t		*rollover_bal = NULL;
	pin_decimal_t		*curr_bal = NULL;
	pin_decimal_t		*zero_decimal = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*abs_rollover_bal = NULL;

	int64			db = 1;

	int32			string_id = ID_RENEW_PREPAID;
	int32			elem_id = 0;
	int32			elem_id1 = 0;

	void			*vp = NULL;
	void			*vp1 = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_renew_prepaid_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_renew_prepaid_lc_event Balance FList:", bal_flistp);
	
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
	srvc_netflix_pdp = PIN_POID_COPY(srvc_pdp,ebufp);
	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp)
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

		if( srvc_pdp && (strcmp(PIN_POID_GET_TYPE(srvc_pdp),"/service/catv")) == 0 )
		{
			strcpy(service_type, "CATV") ;
		}
		else if ( srvc_pdp && (strcmp(PIN_POID_GET_TYPE(srvc_pdp),"/service/telco/broadband")) == 0)
		{
			strcpy(service_type, "BROADBAND");
		}
                else if ( srvc_pdp && (strcmp(PIN_POID_GET_TYPE(srvc_pdp),"/service/netflix")) == 0)
                {
                        strcpy(service_type, "NETFLIX");
                }

	}
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//	disc_amount  = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DISC_AMOUNT, 1, ebufp);
//	disc_percent = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DISC_PERCENT, 1, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_RENEW_PLAN, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

	fm_mso_cust_get_plan_code(ctxp, i_flistp, &plan_info, ebufp );
	if (plan_info)
	{
		plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp);
	}
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp);

	if (bal_flistp)
	{	
		while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_BALANCES,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			
			if (elem_id == MSO_FREE_MB  || 
			    elem_id == MSO_TRCK_USG || 
			    elem_id == MSO_FUP_TRACK 
			   )
			{
				rollover_bal = pbo_decimal_from_str("0.0", ebufp);
				elem_id1 = 0;
				cookie1 = NULL;
				while((vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(vp, PIN_FLD_SUB_BALANCES,
					&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
				{
					curr_bal = PIN_FLIST_FLD_GET(vp1, PIN_FLD_CURRENT_BAL, 1, ebufp);
					if (curr_bal)
					{
						pbo_decimal_add_assign(rollover_bal, curr_bal, ebufp );
					}
				}
				if (pbo_decimal_compare(rollover_bal, zero_decimal, ebufp)!=0)
				{
					if (elem_id ==MSO_FREE_MB)
					{
						abs_rollover_bal = pbo_decimal_negate(rollover_bal, ebufp);
						vp1 = (char*)pbo_decimal_to_str(abs_rollover_bal, ebufp);
						sprintf(rollover_amount_str, "Free MB %s carry forwarded.", vp1);
					}
					if (elem_id ==MSO_FUP_TRACK )
					{
						abs_rollover_bal = pbo_decimal_negate(rollover_bal, ebufp);
						vp1 = (char*)pbo_decimal_to_str(abs_rollover_bal, ebufp);
						sprintf(rollover_amount_str, "FUP quota MB %s carry forwarded.", vp1);
					}

					vp1 = PIN_FLIST_ELEM_ADD(lc_temp_flistp, PIN_FLD_BALANCES, elem_id, ebufp);
					PIN_FLIST_FLD_SET(vp1, PIN_FLD_RESOURCE_ID, &elem_id, ebufp);
					PIN_FLIST_FLD_PUT(vp1, PIN_FLD_ROLLOVER_UNITS, rollover_bal, ebufp);
				}
			}
		}
	}
        //if ( srvc_pdp && (strcmp(PIN_POID_GET_TYPE(srvc_pdp),"/service/netflix")) == 0)
        if ( srvc_netflix_pdp && (strcmp(PIN_POID_GET_TYPE(srvc_netflix_pdp),"/service/netflix")) == 0)
        {
                lc_template = "Netflix Service";
        }
	else
	{
		get_evt_lifecycle_template(ctxp, db, string_id,1, &lc_template, ebufp);
	}
	sprintf(msg, lc_template, plan_code, service_type, agreement_no, account_no, rollover_amount_str );


 	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event Event Flist:", lc_iflistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Lifecycle event");


	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	//if (lc_template)
	//{
	//	free(lc_template);
	//}
	if (amount_str)
	{
		free(amount_str);
	}
	if ( !pbo_decimal_is_null(zero_decimal, ebufp) )
	{
		pbo_decimal_destroy(&zero_decimal);
	}
         if ( !pbo_decimal_is_null(abs_rollover_bal, ebufp) )
        {
                pbo_decimal_destroy(&abs_rollover_bal);
        }

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	PIN_FLIST_DESTROY_EX(&plan_info, NULL);
	return;
}



void
fm_mso_cust_renew_fdp_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;
	pin_flist_t		*plan_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t			*acc_pdp  = NULL;
	poid_t			*service_pdp  = NULL;
	poid_t			*plan_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_FDP_RENEW;
	int32			elem_id = 0;

	char			*event = "/event/activity/mso_lifecycle/fdp_renew";
	char			*action_name = "FDP_RENEW";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name = NULL;
	char			*reason = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*plan_code = NULL;
	char			old_time_str[80]="";
	char			new_time_str[80]="";

	time_t			*old_end_t = NULL;
	time_t			*new_end_t = NULL;

	pin_cookie_t		cookie = NULL;
	int32			*align_flags = NULL;
        //New Tariff - Transaction mapping */
        pin_flist_t             *offering_flistp = NULL;
        pin_flist_t             *eflistp = NULL;
        int32                   elem_id_off = 0;
        pin_cookie_t            cookie_off = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_renew_fdp_lc_event", in_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	old_end_t = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_FROM_DATE, 1, ebufp);
	new_end_t = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_TO_DATE,1, ebufp);
	
	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	db = PIN_POID_GET_DB(acc_pdp);
	
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	if (!agreement_no && service_pdp )
	{
		fm_mso_get_srvc_info(ctxp, in_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}
	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	if (old_end_t)
	{
		fm_mso_convert_epoch_to_date(*old_end_t, old_time_str);
	}

	if (new_end_t)
	{
		fm_mso_convert_epoch_to_date(*new_end_t, new_time_str);
	}

	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_FDP_RENEW, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_FROM_DATE, lc_temp_flistp, MSO_FLD_FROM_DATE, ebufp );
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_TO_DATE, lc_temp_flistp, MSO_FLD_TO_DATE, ebufp );

	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV)) == 0 )
	{
		strcpy(service_type, "CATV") ;
		// Added to identify date alignment cases 
		align_flags = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS, 1, ebufp);
		if(align_flags && (*align_flags ==2 ))
		{
			action_name = "DATE_ALIGN";
		}

		while((plans = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN_LISTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			plan_pdp = PIN_FLIST_FLD_GET(plans, PIN_FLD_PLAN_OBJ, 1, ebufp);
			if (plan_pdp)
			{
				fm_mso_cust_get_plan_code(ctxp, plans, &plan_info, ebufp);
			}
			if(plan_info)
			{
				plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp );
			}
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp );
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp );
		        //New Tariff - Transaction mapping Start*/
                        elem_id_off = 0;
                        cookie_off = NULL;
                        while ((offering_flistp = PIN_FLIST_ELEM_GET_NEXT(plans,
                                                                PIN_FLD_OFFERINGS, &elem_id_off, 1, &cookie_off, ebufp))!= NULL) {
 	                         eflistp = PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_OFFERINGS, elem_id_off, ebufp);
                                 PIN_FLIST_FLD_COPY(offering_flistp, PIN_FLD_OFFERING_OBJ, eflistp, PIN_FLD_OFFERING_OBJ, ebufp);
                                 PIN_FLIST_FLD_COPY(offering_flistp, PIN_FLD_PACKAGE_ID, eflistp, PIN_FLD_PACKAGE_ID, ebufp);
                        }
		        //New Tariff - Transaction mapping End*/


			sprintf(msg, lc_template, plan_code, old_time_str, new_time_str, service_type, agreement_no, account_no );

			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
				msg, action_name, event, lc_iflistp, ebufp);
		}
	}
	else if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0)
	{
		strcpy(service_type, "BROADBAND");

		while((plans = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			fm_mso_cust_get_plan_code(ctxp, plans, &plan_info, ebufp);
			if(plan_info)
			{
				plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp );
			}
			sprintf(msg, lc_template, plan_code, old_time_str, new_time_str, service_type, agreement_no, account_no );

//			PIN_FLIST_FLD_COPY(plans, PIN_FLD_CODE, lc_temp_flistp, PIN_FLD_CODE, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_renew_fdp_lc_event gen_lifecycle_evt flist:", lc_iflistp);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
				msg, action_name, event, lc_iflistp, ebufp);
		}
	}


	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&plan_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);

	return;
}



//void
//fm_mso_cust_fup_topup_lc_event(
//	pcm_context_t		*ctxp,
//	pin_flist_t		*in_flistp,
//	pin_flist_t		*out_flistp,
//	pin_errbuf_t		*ebufp)
//{
//	pin_flist_t		*lc_iflistp = NULL;
//	pin_flist_t		*lc_temp_flistp = NULL;
//	pin_flist_t		*acnt_info = NULL;
//	pin_flist_t		*plan_list = NULL;
//	pin_flist_t		*plans = NULL;
//	pin_flist_t		*plan_info = NULL;
//	pin_flist_t		*srvc_info = NULL;
//
//	poid_t			*acc_pdp  = NULL;
//	poid_t			*service_pdp  = NULL;
//	poid_t			*plan_pdp = NULL;
//		
//	int64			db = 0;
//	int			string_id = ID_FUP_TOPUP;
//	int32			elem_id = 0;
//
//	char			*event = "/event/activity/mso_lifecycle/fup_topup";
//	char			*action_name = "FUP_TOPUP";
//	char			*lc_template = NULL;
//	char			msg[255]="";
//	char			service_type[50]="";
//	char			*prog_name = NULL;
//	char			*reason = NULL;
//	char			*account_no = NULL;
//	char			*agreement_no = NULL;
//	char			*plan_code = NULL;
//
//	pin_cookie_t		cookie = NULL;
//
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//		return;
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_fup_topup_lc_event", in_flistp );
//	/* Get Lifecycle event template */
//
//	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
//	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
//	old_end_t = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_FROM_DATE, 1, ebufp);
//	new_end_t = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_TO_DATE,1, ebufp);
//	
//	if (acc_pdp)
//	{
//		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
//		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
//	}
//	db = PIN_POID_GET_DB(acc_pdp);
//	
//	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
//	agreement_no = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
//	if (!agreement_no && service_pdp )
//	{
//		fm_mso_get_srvc_info(ctxp, in_flistp, &srvc_info, ebufp);
//		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
//	}
//	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
//
//	
//	lc_iflistp = PIN_FLIST_CREATE(ebufp);
//	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_FUP_TOPUP, 0, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
//	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_FROM_DATE, lc_temp_flistp, MSO_FLD_FROM_DATE, ebufp );
//	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_TO_DATE, lc_temp_flistp, MSO_FLD_TO_DATE, ebufp );
//
//	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV)) == 0 )
//	{
//		strcpy(service_type, "CATV") ;
//
//		while((plans = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN_LISTS,
//			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
//		{
//			plan_pdp = PIN_FLIST_FLD_GET(plans, PIN_FLD_PLAN_OBJ, 1, ebufp);
//			if (plan_pdp)
//			{
//				fm_mso_cust_get_plan_code(ctxp, plans, &plan_info, ebufp);
//				plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp );
//			}
//			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp );
//			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp );
//
//			sprintf(msg, lc_template, plan_code, old_time_str, new_time_str, service_type, agreement_no, account_no );
//
//			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
//				msg, action_name, event, lc_iflistp, ebufp);
//		}
//	}
//	else if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0)
//	{
//		strcpy(service_type, "BROADBAND");
//
//		while((plans = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN,
//			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
//		{
//			fm_mso_cust_get_plan_code(ctxp, plans, &plan_info, ebufp);
//			plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp );
//			sprintf(msg, lc_template, plan_code, old_time_str, new_time_str, service_type, agreement_no, account_no );
//
////			PIN_FLIST_FLD_COPY(plans, PIN_FLD_CODE, lc_temp_flistp, PIN_FLD_CODE, ebufp);
//			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp );
//			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_fup_topup_lc_event gen_lifecycle_evt flist:", lc_iflistp);
//
//			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
//				msg, action_name, event, lc_iflistp, ebufp);
//		}
//	}
//
//
//
//	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
//	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
//	PIN_FLIST_DESTROY_EX(&plan_info, NULL);
//	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
//
//	return;
//}


void
fm_mso_cust_mod_payinfo_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*acnt_info,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*payinfo_inv_new = NULL;
	pin_flist_t		*payinfo_inv_old = NULL;
	pin_flist_t		*lc_tmp_flistp = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
		
	int64			db = 0;
	int			string_id  = ID_PAYMENT_INFORMATION_MODIFICATION;
	int32			elem_id = 0;


	char			*event  = "/event/activity/mso_lifecycle/modify_payinfo";

	char			*action   = "PAYMENT_INFORMATION_MODIFICATION";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			modified_field[128]="";
	char			*prog_name = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;

	int32			*delivery_pref_new = NULL;
	int32			*delivery_pref_old = NULL;
	int32			*indicator_new = NULL;
	int32			*indicator_old = NULL;
	int32			*tds_recv_new = NULL;
	int32			*tds_recv_old = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_payinfo_lc_event", in_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	account_no = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);

	db = PIN_POID_GET_DB(acc_pdp);

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	if (vp)
	{
		vp1 = PIN_FLIST_ELEM_GET(vp, PIN_FLD_PAYINFO, PIN_ELEMID_ANY, 1, ebufp );
		payinfo_inv_old = PIN_FLIST_ELEM_GET(vp1, PIN_FLD_INV_INFO, PIN_ELEMID_ANY, 1, ebufp );

	}

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	if (vp)
	{
		vp1 = PIN_FLIST_ELEM_GET(vp, PIN_FLD_PAYINFO, PIN_ELEMID_ANY, 1, ebufp );
		payinfo_inv_new = PIN_FLIST_ELEM_GET(vp1, PIN_FLD_INV_INFO, PIN_ELEMID_ANY, 1, ebufp );
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "payinfo_inv_new", payinfo_inv_new );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "payinfo_inv_old", payinfo_inv_old );


	if (payinfo_inv_new)
	{
		delivery_pref_new = PIN_FLIST_FLD_GET(payinfo_inv_new, PIN_FLD_DELIVERY_PREFER, 1, ebufp );
		indicator_new     = PIN_FLIST_FLD_GET(payinfo_inv_new, PIN_FLD_INDICATOR      , 1, ebufp );
		tds_recv_new      = PIN_FLIST_FLD_GET(payinfo_inv_new, MSO_FLD_TDS_RECV       , 1, ebufp );

	}

	/*******************************************************************************
	*  Modify delivery  pref
	*******************************************************************************/
	if (delivery_pref_new)
	{
		memset(modified_field, '\0', strlen(modified_field)+1);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "delivery_pref modification");
		delivery_pref_old = PIN_FLIST_FLD_GET(payinfo_inv_old, PIN_FLD_DELIVERY_PREFER, 1, ebufp );

		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_tmp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_PAYINFO, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_tmp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(lc_tmp_flistp, PIN_FLD_DELIVERY_PREFER, delivery_pref_old, ebufp );

		lc_tmp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_PAYINFO, 1, ebufp);
		PIN_FLIST_FLD_SET(lc_tmp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(lc_tmp_flistp, PIN_FLD_DELIVERY_PREFER, delivery_pref_new, ebufp );

		if (delivery_pref_new && delivery_pref_old )
		{
			if (*delivery_pref_new == DELIVERY_PREF_EMAIL && *delivery_pref_old == DELIVERY_PREF_POST )
			{
				strcpy(modified_field, "Delivery Preference modified from POST to EMAIL");
			}
			else if (*delivery_pref_new == DELIVERY_PREF_POST && *delivery_pref_old == DELIVERY_PREF_EMAIL )
			{
				strcpy(modified_field, "Delivery Preference modified from EMAIL to POST");
			}
			else
			{
				strcpy(modified_field, "Delivery Preference modified ");
			}

			get_evt_lifecycle_template(ctxp, db, string_id,1, &lc_template, ebufp);
			sprintf(msg, lc_template, modified_field, account_no );
			if (lc_template)
			{
				free(lc_template);
			}
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_payinfo_lc_event delivery pref flist:", lc_iflistp);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,
				msg, action, event, lc_iflistp, ebufp);

			PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
		}
	}

 
	/*******************************************************************************
	*  Modify delivery  pref
	*******************************************************************************/
	if (indicator_new)
	{
		memset(modified_field, '\0', strlen(modified_field)+1);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "indicator modification");
		indicator_old = PIN_FLIST_FLD_GET(payinfo_inv_old, PIN_FLD_INDICATOR      , 1, ebufp );

		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_tmp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_PAYINFO, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_tmp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(lc_tmp_flistp, PIN_FLD_INDICATOR, indicator_old, ebufp );
		
		lc_tmp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_PAYINFO, 1, ebufp);
		PIN_FLIST_FLD_SET(lc_tmp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(lc_tmp_flistp, PIN_FLD_INDICATOR, indicator_new, ebufp );

		if (indicator_new && indicator_old )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "indicator_new && indicator_old exist");
			if (*indicator_new == INDICATOR_POSTPAID && *indicator_old == INDICATOR_ADVANCE )
			{
				strcpy(modified_field, "PayType modified from ADVANCE to POSTPAID");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, modified_field);
			}
			else if (*indicator_new == INDICATOR_POSTPAID && *indicator_old == INDICATOR_PREPAID )
			{
				strcpy(modified_field, "PayType modified from PREPAID to POSTPAID");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, modified_field);
			}
			else if (*indicator_new == INDICATOR_ADVANCE && *indicator_old == INDICATOR_POSTPAID )
			{
				strcpy(modified_field, "PayType modified from POSTPAID to ADVANCE");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, modified_field);
			}
			else if (*indicator_new == INDICATOR_ADVANCE && *indicator_old == INDICATOR_PREPAID )
			{
				strcpy(modified_field, "PayType modified from PREPAID to ADVANCE");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, modified_field);
			}
			else if (*indicator_new == INDICATOR_PREPAID && *indicator_old == INDICATOR_POSTPAID )
			{
				strcpy(modified_field, "PayType modified from POSTPAID to PREPAID");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, modified_field);
			}
			else if (*indicator_new == INDICATOR_PREPAID && *indicator_old == INDICATOR_ADVANCE )
			{
				strcpy(modified_field, "PayType modified from ADVANCE to PREPAID");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, modified_field);
			}
			else
			{
				strcpy(modified_field, "PayType modified");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, modified_field);
			}

			get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
			sprintf(msg, lc_template, modified_field, account_no );
			if (lc_template)
			{
				free(lc_template);
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_payinfo_lc_event indicator flist:", lc_iflistp);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,
				msg, action, event, lc_iflistp, ebufp);

			PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
		}
	}

	/*******************************************************************************
	*  Modify tds recv
	*******************************************************************************/
	if (tds_recv_new)
	{
		memset(modified_field, '\0', strlen(modified_field)+1);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "tds recv modification");
		tds_recv_old      = PIN_FLIST_FLD_GET(payinfo_inv_old, MSO_FLD_TDS_RECV       , 1, ebufp );

		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_tmp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_PAYINFO, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_tmp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(lc_tmp_flistp, MSO_FLD_TDS_RECV, tds_recv_old, ebufp );

		lc_tmp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_PAYINFO, 1, ebufp);
		PIN_FLIST_FLD_SET(lc_tmp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp );
		PIN_FLIST_FLD_SET(lc_tmp_flistp, MSO_FLD_TDS_RECV, tds_recv_new, ebufp );

		if (tds_recv_new && tds_recv_old )
		{
			if (*tds_recv_new == DEFAULT && *tds_recv_old == NOT_RECEIVED )
			{
				strcpy(modified_field, "TDS received status modified from NOT_RECEIVED to DEFAULT");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, modified_field);
			}
			else if (*tds_recv_new == NOT_RECEIVED && *tds_recv_old == DEFAULT )
			{
				strcpy(modified_field, "TDS received status modified from DEFAULT to NOT_RECEIVED");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, modified_field);
			}
			else
			{
				strcpy(modified_field, "TDS received status modified");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, modified_field);
			}

			get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
			sprintf(msg, lc_template, modified_field, account_no );
			if (lc_template)
			{
				free(lc_template);
			}
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_mod_payinfo_lc_event tds recv flist:", lc_iflistp);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,
				msg, action, event, lc_iflistp, ebufp);

			PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
		}
	}
	return;
}



void
fm_mso_deact_validity_quota_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*o_flist,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t		*plan_info  = NULL;

	
	poid_t                  *plan_pdp = NULL;
	poid_t                  *acct_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	
	char			*event = "/event/activity/mso_lifecycle/expired_validity";
	char			*event1 = "/event/activity/mso_lifecycle/expired_quota";

	char			msg[255]="";
	char			*action_name = "DEACTIVATE_VALIDITY_EXPIRY";
	char			*action_name1 = "DEACTIVATE_QUOTA_EXPIRY";
	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*prog_name = NULL;
	char			*refund_no = NULL;
	char			*plan_code = NULL;

	int64			db = 1;

	int32			string_id = ID_DEACTIVATE_VALIDITY_EXPIRY;
	int32			string_id1 = ID_DEACTIVATE_QUOTA_EXPIRY;
	int32			*mode = NULL;




	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_deact_validity_quota_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_deact_validity_quota_lc_event Balance FList:", o_flist);
	
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp );
	mode     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MODE, 1, ebufp ); 
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp)
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}
	if (plan_pdp)
	{
		fm_mso_cust_get_plan_code(ctxp, i_flistp, &plan_info, ebufp );
		plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp);
	}
	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);

	if (mode && *mode == VALIDITY_EXPIRY )
	{
		lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_EXPIRED_VALIDITY, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE,    plan_code, ebufp);

		get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, plan_code, agreement_no, account_no );
 		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event validity expiry  Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action_name, event, lc_iflistp, ebufp);
	}
	if (mode && *mode == QUOTA_EXHAUSTION )
	{
		lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_EXPIRED_QUOTA, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE,    plan_code, ebufp);
			
		get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, plan_code, agreement_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event quota exhausted Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action_name1, event1, lc_iflistp, ebufp);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	PIN_FLIST_DESTROY_EX(&plan_info, NULL);
	return;
}


void
fm_mso_swap_device_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*o_flist,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t		*devices = NULL;

	poid_t                  *acct_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	poid_t			*device_pdp = NULL;
	
	char			*event = "/event/activity/mso_lifecycle/swap_device";
	char			*event1 = "/event/activity/mso_lifecycle/change_ip";

	char			msg[255]="";
	char			reason_str[128]="";
	char			device_type[32] = ""; //Modem
	char			*action_name = "SWAP_DEVICE";
	char			*action_name1 = "SWAP_IP";

	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*prog_name = NULL;
	char			*old_device_id = NULL;
	char			*new_device_id = NULL;
	char			*poid_type = NULL;

	int64			db = 1;

	int32			string_id = ID_SWAP_DEVICE;
	int32			string_id1 = ID_SWAP_IP;
	int32			*flags = NULL;
	int32			*reason_id = NULL;
	int32			elem_id = 0;
	int32			device_is_ip = 0;
       time_t               swp_time = pin_virtual_time(NULL);

	pin_cookie_t		cookie = NULL;




	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_swap_device_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_swap_device_lc_event Balance FList:", o_flist);
	
	acct_pdp   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
	srvc_pdp   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	prog_name  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	reason_id  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 1, ebufp);
	device_pdp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_OLD_DEVICE_OBJ, 1, ebufp);
	if (device_pdp)
	{
		memset(device_type, '\0', strlen(device_type)+1);
		poid_type = (char*)PIN_POID_GET_TYPE(device_pdp);
		if (poid_type && strcmp(poid_type, "/device/ip/framed") ==0 )
		{
			strcpy(device_type, "IP Framed");
			device_is_ip = 1;
		}
		else if (poid_type && strcmp(poid_type, "/device/ip/static")==0 )
		{
			strcpy(device_type, "IP Static");
			device_is_ip = 1;
		}
		else if (poid_type && strcmp(poid_type, "/device/modem") ==0 )
		{
			strcpy(device_type, "Modem");
		}
		else if (poid_type && strcmp(poid_type, "/device/nat") ==0 )
		{
			strcpy(device_type, "NAT");
		}
		else if (poid_type && strcmp(poid_type, "/device/router/cable") ==0 )
		{
			strcpy(device_type, "Router Cable");
		}
		else if (poid_type && strcmp(poid_type, "/device/router/wifi")==0 )
		{
			strcpy(device_type, "Router WiFi");
		}
		else if (poid_type && strcmp(poid_type, "/device/stb")==0 )
		{
			strcpy(device_type, "STB");
		}
		else if (poid_type && strcmp(poid_type, "/device/vc")==0 )
		{
			strcpy(device_type, "VC");
		}
		else if (poid_type && strcmp(poid_type, "/device/voucher")==0 )
		{
			strcpy(device_type, "Voucher");
		}
		else
		{
			return;
		}
	}

	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp)
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}
	
	while((devices = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_DEVICES,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		flags = PIN_FLIST_FLD_GET(devices, PIN_FLD_FLAGS, 1, ebufp );
		if (flags && *flags == 0 ) //0: old device
		{
			old_device_id = PIN_FLIST_FLD_GET(devices, PIN_FLD_DEVICE_ID, 1, ebufp );
		}
		if (flags && *flags == 1) //1: new device
		{
			new_device_id = PIN_FLIST_FLD_GET(devices, PIN_FLD_DEVICE_ID, 1, ebufp );
		}
	}

	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEFAULT_FLAG, 1, ebufp) != NULL)
	{
		old_device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OLD_VALUE, 1, ebufp);
		new_device_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NEW_VALUE, 1, ebufp);
		strcpy(device_type, "OTT Box");
		strcpy(reason_str, "OTT SWAP");
	}

	if (reason_id && *reason_id == MSO_DEVICE_CHANGE_FAULTY_DEVICE )
	{
		strcpy(reason_str, "Fault in Device");
	}
	else if ( reason_id && *reason_id == MSO_DEVICE_CHANGE_TEMPORARY_DEVICE )
	{
		strcpy(reason_str, "Changing Temporary Device");
	}
	else if ( reason_id && *reason_id == MSO_DEVICE_CHANGE_TECH_DEVICE)
	{
		strcpy(reason_str, "Device Upgradation");
	}
	/*More Reasons may need to be added*/

	 
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_DEVICE_SWAP, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_OLD_DEVICE_OBJ, lc_temp_flistp, MSO_FLD_OLD_DEVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_NEW_DEVICE_OBJ, lc_temp_flistp, MSO_FLD_NEW_DEVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, lc_temp_flistp, PIN_FLD_REASON_ID, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_REASON_CODE, reason_str, ebufp);
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEFAULT_FLAG, 1, ebufp) != NULL)
	{
		elem_id = OTT_MODEM_VAL;
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OLD_VALUE, lc_temp_flistp, PIN_FLD_OLD_VALUE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NEW_VALUE, lc_temp_flistp, PIN_FLD_NEW_VALUE, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_OFFER_VALUE, &elem_id, ebufp);
        	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_VALID_TO, (void *)&swp_time, ebufp);
	}

	if (device_is_ip)
	{
		PIN_FLIST_FLD_RENAME(lc_iflistp, MSO_FLD_DEVICE_SWAP, MSO_FLD_CHANGE_IP, ebufp);
		get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, old_device_id, new_device_id, agreement_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event validity expiry  Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action_name1, event1, lc_iflistp, ebufp);

	}
	else if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEFAULT_FLAG, 1, ebufp) != NULL)
	{
		get_evt_lifecycle_template(ctxp, db, ID_SWAP_OTT, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, device_type, old_device_id, new_device_id, reason_str, agreement_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event validity expiry  Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action_name, event, lc_iflistp, ebufp);
	}
	else
	{
 		get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, device_type, old_device_id, new_device_id, reason_str, agreement_no, account_no );
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event validity expiry  Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action_name, event, lc_iflistp, ebufp);

	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	return;
}


void
fm_mso_allocate_bb_hw_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*o_flist,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*plan_info = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t		*devices = NULL;
	pin_flist_t		*hw_plans = NULL;

	poid_t                  *acct_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*device_pdp = NULL;
	
	char			*event = "/event/activity/mso_lifecycle/allocate_bb_hardware";

	char			msg[255]="";
	char			device_type[32] = ""; //Modem
	char			*action_name = "ALLOCATE_HARDWARE";
	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*prog_name = NULL;
	char			*plan_code = NULL;
	char			*stock_point_id = NULL;
	char			*serial_no = NULL;
	char			*device_id = NULL;
	char			*poid_type = NULL;

	int64			db = 1;

	int32			string_id = ID_ALLOCATE_HARDWARE;
	int32			*flags = NULL;
	int32			elem_id = 0;
	int32			elem_id1 = 0;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;




	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_allocate_bb_hw_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_allocate_bb_hw_lc_event Balance FList:", o_flist);
	
	acct_pdp   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
	srvc_pdp   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	prog_name  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp)
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}

	hw_plans = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_HARDWARE_PLANS, 1, ebufp);
	
	while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(hw_plans, PIN_FLD_PLAN,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "vp", vp);
		plan_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_PLAN_OBJ, 1, ebufp );
		if (plan_pdp)
		{
			fm_mso_cust_get_plan_code(ctxp, vp, &plan_info, ebufp );
			plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp);
		}
		
		elem_id1 =0;
		cookie1 = NULL;
		while((vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(vp, PIN_FLD_DEVICES,
			&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "vp1", vp1);
			device_id = PIN_FLIST_FLD_GET(vp1, PIN_FLD_DEVICE_ID, 1, ebufp);
			
			stock_point_id =  PIN_FLIST_FLD_GET(vp1, PIN_FLD_SOURCE, 1, ebufp);
			serial_no =       PIN_FLIST_FLD_GET(vp1, MSO_FLD_SERIAL_NO, 1, ebufp);
			device_pdp =      PIN_FLIST_FLD_GET(vp1, PIN_FLD_POID, 1, ebufp);
			if (device_pdp)
			{
				memset(device_type, '\0', strlen(device_type)+1);
				//poid_type = (char*)PIN_POID_GET_TYPE(device_pdp);
				fm_mso_get_device_type_from_poid(ctxp, device_pdp, device_type, ebufp);
			}
			lc_iflistp = PIN_FLIST_CREATE(ebufp);
			lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_ALLOCATE_BB_HARDWARE, 0, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp);

			get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
			sprintf(msg, lc_template, device_type, serial_no, device_id, stock_point_id, agreement_no);
			if (lc_template)
			{
				free(lc_template);
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event allocate_bb_hw_lc_event Flist:", lc_iflistp);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
				msg, action_name, event, lc_iflistp, ebufp);
			

			PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
		}
		PIN_FLIST_DESTROY_EX(&plan_info, NULL) ;
	}

	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ; 
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	 
	return;
}



void
fm_mso_hold_srvc_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*o_flist,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t                  *acct_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*device_pdp = NULL;
	
	char			*event = "/event/activity/mso_lifecycle/hold_service";

	char			msg[255]="";
	char			srvc_type[32] = ""; //Modem
	char			*action_name = "HOLD_SERVICE";
	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*prog_name = NULL;
	char			*descr = NULL;
	char			*poid_type = NULL;

	int64			db = 1;

	int32			string_id = ID_HOLD_SERVICE;

	void			*vp = NULL;
	void			*vp1 = NULL;




	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_srvc_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_srvc_lc_event Balance FList:", o_flist);
	
	acct_pdp   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
	srvc_pdp   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	prog_name  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	descr      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);

	poid_type = (char*)PIN_POID_GET_TYPE(srvc_pdp);
	if (poid_type && strcmp(poid_type, "/service/catv") == 0 )
	{
		memset(srvc_type, '\0', strlen(srvc_type)+1);
		strcpy(srvc_type, "CATV");
	}
	else if (poid_type && strcmp(poid_type, "/service/telco/broadband") == 0 )
	{
		memset(srvc_type, '\0', strlen(srvc_type)+1);
		strcpy(srvc_type, "BROADBAND");
	}

	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp)
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}
	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_SERVICE_HOLD, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, srvc_type, agreement_no, descr, account_no);
	if (lc_template)
	{
		free(lc_template);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hold_srvc_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);
	

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	 
	return;
}

void
fm_mso_unhold_srvc_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*o_flist,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t                  *acct_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*device_pdp = NULL;
	
	char			*event = "/event/activity/mso_lifecycle/unhold_service";

	char			msg[255]="";
	char			srvc_type[32] = ""; //Modem
	char			*action_name = "UNHOLD_SERVICE";
	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*prog_name = NULL;
	char			*descr = NULL;
	char			*poid_type = NULL;

	int64			db = 1;

	int32			string_id = ID_UNHOLD_SERVICE;

	void			*vp = NULL;
	void			*vp1 = NULL;




	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_unhold_srvc_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_unhold_srvc_lc_event Balance FList:", o_flist);
	
	acct_pdp   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp );
	srvc_pdp   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	prog_name  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	descr      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);

	poid_type = (char*)PIN_POID_GET_TYPE(srvc_pdp);
	if (poid_type && strcmp(poid_type, "/service/catv") == 0 )
	{
		memset(srvc_type, '\0', strlen(srvc_type)+1);
		strcpy(srvc_type, "CATV");
	}
	else if (poid_type && strcmp(poid_type, "/service/telco/broadband") == 0 )
	{
		memset(srvc_type, '\0', strlen(srvc_type)+1);
		strcpy(srvc_type, "BROADBAND");
	}

	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp)
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}

	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_SERVICE_UNHOLD, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, srvc_type, agreement_no, descr, account_no);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_unhold_srvc_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);
	
	if (lc_template)
	{
		free(lc_template);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	 
	return;
}




void
mso_op_grv_upload_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*o_flist,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t                  *acct_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*device_pdp = NULL;
	poid_t			*grv_object = NULL;

	char			*event = "/event/activity/mso_lifecycle/grv_upload";

	char			msg[255]="";
	char			srvc_type[32] = ""; //Modem
	char			device_type[32] = "";
	char			*device_id = NULL;
	char			*action_name = "GRV_UPLOAD";
	char			*lc_template = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*prog_name = NULL;
	char			*descr = NULL;
	char			*poid_type = NULL;
	char			*deposit_no = NULL;
	char			*grv_no = NULL;
	char			*lifecycle_action = NULL;

	pin_decimal_t		*tot_amount = NULL;

	int64			db = 1;

	int32			string_id = ID_GRV_UPLOAD;

	void			*vp = NULL;
	void			*vp1 = NULL;




	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_grv_upload_gen_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_grv_upload_gen_lc_event Output FList:", o_flist);
	
	acct_pdp   	 = 	PIN_FLIST_FLD_GET(o_flist,  PIN_FLD_ACCOUNT_OBJ,  1, ebufp);
	srvc_pdp   	 = 	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ,  1, ebufp);
	prog_name  	 = 	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	descr      	 = 	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 	  1, ebufp);
	device_id    	 = 	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_ID,    1, ebufp);
	device_pdp   	 = 	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEVICE_OBJ,   1, ebufp);
	grv_object   	 = 	PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_GRV_OBJ,      1, ebufp);
	tot_amount   	 = 	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT,       1, ebufp);
	deposit_no   	 = 	PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DEPOSIT_NO,   1, ebufp);
	account_no   	 = 	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO,   1, ebufp);
	agreement_no     = 	PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	grv_no       	 = 	PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_REF_NO, 	  1, ebufp);
	action_name	 	 =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 	  1, ebufp);

	poid_type = (char*)PIN_POID_GET_TYPE(srvc_pdp);
	if (poid_type && strcmp(poid_type, "/service/catv") == 0 )
	{
		//memset(srvc_type, '\0', strlen(srvc_type)+1);
		strcpy(srvc_type, "CATV");
	}
	else if (poid_type && strcmp(poid_type, "/service/telco/broadband") == 0 )
	{
		//memset(srvc_type, '\0', strlen(srvc_type)+1);
		strcpy(srvc_type, "BROADBAND");
	}

	if (acct_pdp && !account_no )
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp && !agreement_no )
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}

	if (device_pdp)
	{
		//memset(device_type, '\0', strlen(device_type)+1);
		poid_type = (char*)PIN_POID_GET_TYPE(device_pdp);

		if (poid_type && strcmp(poid_type, "/device/ip/framed") ==0 )
		{
			strcpy(device_type, "IP Framed");
		}
		else if (poid_type && strcmp(poid_type, "/device/ip/static")==0 )
		{
			strcpy(device_type, "IP Static");
		}
		else if (poid_type && strcmp(poid_type, "/device/modem") ==0 )
		{
			strcpy(device_type, "Modem");
		}
		else if (poid_type && strcmp(poid_type, "/device/nat") ==0 )
		{
			strcpy(device_type, "NAT");
		}
		else if (poid_type && strcmp(poid_type, "/device/router/cable") ==0 )
		{
			strcpy(device_type, "Router Cable");
		}
		else if (poid_type && strcmp(poid_type, "/device/router/wifi")==0 )
		{
			strcpy(device_type, "Router WiFi");
		}
		else if (poid_type && strcmp(poid_type, "/device/stb")==0 )
		{
			strcpy(device_type, "STB");
		}
		else if (poid_type && strcmp(poid_type, "/device/vc")==0 )
		{
			strcpy(device_type, "VC");
		}
		else if (poid_type && strcmp(poid_type, "/device/voucher")==0 )
		{
			strcpy(device_type, "Voucher");
		}
	}

	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_GRV_UPLOAD, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_GRV_OBJ, grv_object, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DEVICE_OBJ, device_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_REFUND_AMOUNT, tot_amount, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_DEPOSIT_NO, deposit_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, action_name, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, grv_no, device_type, device_id, account_no);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_grv_upload_gen_lc_event Flist:", lc_iflistp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "prog_name >> ");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, prog_name);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "msg >> ");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "action_name >> ");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, action_name);
	
	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name, 
		msg, action_name, event, lc_iflistp, ebufp);
	
	if (lc_template)
	{
		free(lc_template);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	 
	return;
}



void
mso_cust_bu_reg_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
		
	pin_flist_t		*ro_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*user_id = NULL;
	poid_t			*event_pdp = NULL;
	poid_t			*acct_pdp = NULL;
		
	int64			db = 1;
	char			*event1 = "/event/activity/mso_lifecycle/register_bunit";
	char			*event2 = "/event/activity/mso_lifecycle/register_buser";
	char			*event3 = "/event/activity/mso_lifecycle/register_customer";

	char			*action_name1 = "BUSINESS_UNIT_REGISTRATION";
	char			*action_name2 = "BUSINESS_USER_REGISTRATION";
	char			*action_name3 = "REGISTER_END_CUSTOMER";
	char			*lc_template = NULL;
	char			*prog_name = NULL;
	char			*account_no = NULL;
	char			*last_stmt = NULL;

	int			string_id1 = ID_REGISTER_BUSINESS_UNIT;
	int			string_id2 = ID_REGISTER_BUSINESS_UNIT;
	int			string_id3 = ID_REGISTER_CUSTOMER;
	int			acnt_category = 0;
	int			tmp_business_type = -1;
	int			*business_type = NULL;
	int			srvc_type = -1;
	int			account_model = -1;
	int			subscriber_type = -1;

	int32			*account_type = NULL;

	void			*vp = NULL;

	
	char			msg[255]="";
	char			account_type_str[64]="";
	char			final_account_type_str[256]="";

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cust_bu_reg_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cust_bu_reg_lc_event Balance FList:", o_flistp);
	
	acct_pdp     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_POID, 1, ebufp );
	prog_name    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	account_no   = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_NO,   1, ebufp );
	last_stmt    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LASTSTAT_CMNT,   1, ebufp );
	account_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_TYPE,   1, ebufp );

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACCTINFO,0, 1, ebufp );
	if (vp)
	{
		business_type = PIN_FLIST_FLD_GET(vp, PIN_FLD_BUSINESS_TYPE,1, ebufp );
		if (business_type)
		{
			//account_type = *business_type/1000000;
			tmp_business_type = *(int*)business_type/100;  //tmp_business_type {99 27 1 2}
			srvc_type = tmp_business_type%10;             //tmp_business_type {99 27 1 2}, HYBRID, BB, CATV

			tmp_business_type = tmp_business_type/10;     //tmp_business_type {99 27 1}
			account_model = tmp_business_type%10;         //tmp_business_type {99 27 1}

			tmp_business_type = tmp_business_type/10;     //tmp_business_type {99 27}
			subscriber_type =  tmp_business_type%100;    
		}
	}

  //99 27 1 2 00
	/*******************
	* Business unit
	*******************/
	if (account_type && *account_type == ACCT_TYPE_HTW )
	{
		strcpy(account_type_str, "OPERATOR");
		acnt_category = ID_REGISTER_BUSINESS_UNIT;
	}
	else if (account_type && *account_type == ACCT_TYPE_JV )
	{
		strcpy(account_type_str, "JV");
		acnt_category = ID_REGISTER_BUSINESS_UNIT;
	}
	else if (account_type && *account_type == ACCT_TYPE_DTR )
	{
		strcpy(account_type_str, "DT");
		acnt_category = ID_REGISTER_BUSINESS_UNIT;
	}
	else if (account_type && *account_type == ACCT_TYPE_SUB_DTR )
	{
		strcpy(account_type_str, "SDT");
		acnt_category = ID_REGISTER_BUSINESS_UNIT;
	}
	else if (account_type && *account_type == ACCT_TYPE_LCO )
	{
		strcpy(account_type_str, "LCO");
		acnt_category = ID_REGISTER_BUSINESS_UNIT;
	}
	else if (account_type && *account_type == ACCT_TYPE_LOGISTICS )
	{
		strcpy(account_type_str, "LOGISTICS");
		acnt_category = ID_REGISTER_BUSINESS_UNIT;
	}
	/*******************
	* Business user
	*******************/
	else if (account_type && *account_type == ACCT_TYPE_CSR_ADMIN )
	{
		strcpy(account_type_str, "CSR ADMIN");
		acnt_category = ID_REGISTER_BUSINESS_USER;
	}
	else if (account_type && *account_type == ACCT_TYPE_CSR )
	{
		strcpy(account_type_str, "CSR");
		acnt_category = ID_REGISTER_BUSINESS_USER;
	}
	else if (account_type && *account_type == ACCT_TYPE_OPERATION )
	{
		strcpy(account_type_str, "OPERATION");
		acnt_category = ID_REGISTER_BUSINESS_USER;
	}
	else if (account_type && *account_type == ACCT_TYPE_FIELD_SERVICE )
	{
		strcpy(account_type_str, "FIELD_SERVICE");
		acnt_category = ID_REGISTER_BUSINESS_USER;
	}
	else if (account_type && *account_type == ACCT_TYPE_COLLECTION_AGENT )
	{
		strcpy(account_type_str, "COLLECTION_AGENT");
		acnt_category = ID_REGISTER_BUSINESS_USER;
	}
	else if (account_type && *account_type == ACCT_TYPE_SALES_PERSON )
	{
		strcpy(account_type_str, "SALES_PERSON");
		acnt_category = ID_REGISTER_BUSINESS_USER;
	}
	else if (account_type && *account_type == ACCT_TYPE_PRE_SALES )
	{
		strcpy(account_type_str, "PRE_SALES");
		acnt_category = ID_REGISTER_BUSINESS_USER;
	}
	/*******************
	* Customer
	*******************/
	else if (account_type && *account_type == ACCT_TYPE_SUBSCRIBER )
	{
		strcpy(account_type_str, "CATV SUBSCRIBER");
		acnt_category = ID_REGISTER_CUSTOMER;
	}
	else if (account_type && *account_type == ACCT_TYPE_SME_SUBS_BB )
	{
		strcpy(account_type_str, "SME SUBSCRIBER");
		acnt_category = ID_REGISTER_CUSTOMER;
	}
	else if (account_type && *account_type == ACCT_TYPE_RETAIL_BB )
	{
		strcpy(account_type_str, "RETAIL SUBSCRIBER");
		acnt_category = ID_REGISTER_CUSTOMER;
	}
	/*********************
	else if (account_type && *account_type == ACCT_TYPE_CYBER_CAFE_BB )
	{
		strcpy(account_type_str, "CYBER CAFE SUBSCRIBER");
		acnt_category = ID_REGISTER_CUSTOMER;
	}
	**********************/
	else if (account_type && *account_type == ACCT_TYPE_CORP_SUBS_BB )
	{
		strcpy(account_type_str, "CORPORATE SUBSCRIBER");
		acnt_category = ID_REGISTER_CUSTOMER;
	}

	if (acnt_category == ID_REGISTER_BUSINESS_UNIT)
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_REGISTER_BUNIT, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_LASTSTAT_CMNT, last_stmt, ebufp);

		get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, account_type_str, account_no);
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cust_bu_reg_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
			msg, action_name1, event1, lc_iflistp, ebufp);
	}
	else if (acnt_category == ID_REGISTER_BUSINESS_USER)
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_REGISTER_BUSER, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_LASTSTAT_CMNT, last_stmt, ebufp);

		get_evt_lifecycle_template(ctxp, db, string_id2, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, account_type_str, account_no);
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cust_bu_reg_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
			msg, action_name2, event2, lc_iflistp, ebufp);
	}
	else if (acnt_category == ID_REGISTER_CUSTOMER)
	{
		if (srvc_type == SRVC_PERMIT_HYBRID )
		{
			strcpy(final_account_type_str, "HYBRID ");
			strcat(final_account_type_str, account_type_str);
		}
		else if (srvc_type == SRVC_PERMIT_CATV )
		{
			strcpy(final_account_type_str, "CATV ");
			strcat(final_account_type_str, account_type_str);
		}
		else if (srvc_type == SRVC_PERMIT_BB )
		{
			strcpy(final_account_type_str, "BB ");
			strcat(final_account_type_str, account_type_str);
		}

		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_REGISTER_CUSTOMER, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_LASTSTAT_CMNT, last_stmt, ebufp);

		get_evt_lifecycle_template(ctxp, db, string_id3, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, final_account_type_str, account_no);
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cust_bu_reg_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
			msg, action_name3, event3, lc_iflistp, ebufp);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	 
	return;

}

void
fm_mso_create_bb_srvc_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*ro_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*inst_plan_info = NULL;
	pin_flist_t		*subs_plan_info = NULL;
	pin_flist_t		*srvc_bb_info = NULL;

	poid_t			*srvc_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*inst_plan_pdp = NULL;
	poid_t			*subs_plan_pdp = NULL;
		
	int64			db = 1;
	char			*event = "/event/activity/mso_lifecycle/create_bb_service";

	char			*action_name  = "CREATE_BB_SERVICE";
	char			*action_name1 = "CREATE_BB_SERVICE_PLANS";
	char			*lc_template = NULL;
	char			*prog_name = NULL;
	char			*account_no = NULL;
	char			*inst_plan_code = NULL;
	char			*subs_plan_code = NULL;
	char			*agreement_no = NULL;
	char			msg[255]="";

	int			string_id = ID_CREATE_BB_SERVICE;
	int32			*plan_type = NULL;
	int32			elem_id = 0;
	int32			one = 1;
	int32			two = 2;
	int32			three = 3;

	pin_cookie_t		cookie = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_bb_srvc_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_bb_srvc_lc_event Balance FList:", o_flistp);
	
	acct_pdp     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
	prog_name    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
	if (vp)
	{
		srvc_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
		agreement_no = PIN_FLIST_FLD_GET(vp, MSO_FLD_AGREEMENT_NO, 1, ebufp );
	}
	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, account_no, agreement_no);
	if (lc_template)
	{
		free(lc_template);
	}

//	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp );
//	if (vp)
//	{
//		vp1 = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(vp, PIN_FLD_INHERITED_INFO, 1, ebufp );
//		if (vp1)
//		{
//			srvc_bb_info = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(vp1, MSO_FLD_BB_INFO, 1, ebufp );
//		}
//	}

 	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CREATE_BB_SERVICE, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_bb_srvc_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id, 2, &lc_template, ebufp);

	vp = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_INSTALLATION_PLANS, 1, ebufp );
	if (vp)
	{
		elem_id = 0;
		cookie = NULL;
		while((vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(vp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			inst_plan_pdp = PIN_FLIST_FLD_GET(vp1, PIN_FLD_PLAN_OBJ, 1, ebufp );
			fm_mso_cust_get_plan_code(ctxp, vp1, &inst_plan_info, ebufp );
			inst_plan_code = PIN_FLIST_FLD_GET(inst_plan_info, PIN_FLD_CODE, 1, ebufp);

			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, inst_plan_pdp, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, inst_plan_code, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_PLAN_TYPE, &three, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_TYPE_STR, "INSTALLATION_PLAN", ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_bb_srvc_lc_event Flist:", lc_iflistp);

			sprintf(msg, lc_template, "Installation", inst_plan_code, account_no, agreement_no);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
				msg, action_name1, event, lc_iflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&inst_plan_info, NULL) ;
		}
	}

	vp = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_SUBSCRIPTION_PLANS, 1, ebufp );
	if (vp)
	{
		elem_id = 0;
		cookie = NULL;
		while((vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(vp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			subs_plan_pdp = PIN_FLIST_FLD_GET(vp1, PIN_FLD_PLAN_OBJ, 1, ebufp );
			fm_mso_cust_get_plan_code(ctxp, vp1, &subs_plan_info, ebufp );
			subs_plan_code = PIN_FLIST_FLD_GET(subs_plan_info, PIN_FLD_CODE, 1, ebufp);

			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, subs_plan_pdp, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, subs_plan_code, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_PLAN_TYPE, &one, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_TYPE_STR, "SUBSCRIPTION_PLAN", ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_bb_srvc_lc_event Flist:", lc_iflistp);

			sprintf(msg, lc_template, "Subscription", subs_plan_code, account_no, agreement_no);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
				msg, action_name1, event, lc_iflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&subs_plan_info, NULL) ;
		}
	}

	if (lc_template)
	{
		free(lc_template);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&subs_plan_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&inst_plan_info, NULL) ;
	 
	return;

}


void
fm_mso_activate_bb_srvc_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*ro_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*plan_info = NULL;
	pin_flist_t		*device_info = NULL;

	poid_t			*srvc_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*device_pdp = NULL;
		
	int64			db = 1;

	int32			one = 1;
	int32			two = 2;
	int32			three = 3;

	char			*event = "/event/activity/mso_lifecycle/activate_bb_service";

	char			*action_name = "ACTIVATE_BB_SERVICE";
	char			*action_name1 = "ACTIVATE_BB_SERVICE_PLANS";

	char			*lc_template = NULL;
	char			*lc_template_hw = NULL;
	char			*prog_name = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*plan_code = NULL;
	char			*device_id = NULL;

	int			string_id = ID_ACTIVATE_BB_SERVICE;

	int32			elem_id = 0;
	int32			elem_id1=0;
	int32			*plan_type = NULL;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;

	
	char			msg[255]="";
	char			device_type[64]="";

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_activate_bb_srvc_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_activate_bb_srvc_lc_event Balance FList:", o_flistp);
	
	acct_pdp     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
	srvc_pdp     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );
	prog_name    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp)
	{
		fm_mso_get_srvc_info(ctxp, o_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}

	//Search template for activation
	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, account_no, agreement_no );
	if (lc_template)
	{
		free(lc_template);
	}

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_ACTIVATE_BB_SERVICE, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_activate_bb_srvc_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	//Search template for add plan
	get_evt_lifecycle_template(ctxp, db, string_id, 2, &lc_template, ebufp);
	get_evt_lifecycle_template(ctxp, db, string_id, 3, &lc_template_hw, ebufp);

	while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(o_flistp, PIN_FLD_PLAN,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		plan_type =  (int32*)PIN_FLIST_FLD_GET(vp, PIN_FLD_TYPE, 1, ebufp);
		if (plan_type && *plan_type == 1 ) //SUBSCRIPTION_PLANS
		{
			plan_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_PLAN_OBJ, 1, ebufp);

			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
			fm_mso_cust_get_plan_code(ctxp, vp, &plan_info, ebufp );
			plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_PLAN_TYPE, &one, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_TYPE_STR, "SUBSCRIPTION_PLAN", ebufp);
			//PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DEVICE_ID,   device_id, ebufp);

			sprintf(msg, lc_template, "Subscription",plan_code, account_no, agreement_no );

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_activate_bb_srvc_lc_event Flist:", lc_iflistp);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
				msg, action_name1, event, lc_iflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&plan_info, NULL);
		}
		else if (plan_type && *plan_type == 2 )	//HARDWARE_PLANS
		{
			plan_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_PLAN_OBJ, 1, ebufp);

			elem_id1 = 0;
			cookie1 = NULL;
			while ((vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(vp, PIN_FLD_DEVICES,
				&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
			{
				device_id = PIN_FLIST_FLD_GET(vp1, PIN_FLD_DEVICE_ID, 1, ebufp);
				PIN_FLIST_FLD_COPY(vp, PIN_FLD_PLAN_OBJ, vp1, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(vp1, PIN_FLD_SEARCH_TYPE, &one, ebufp);
				fm_mso_get_device_info(ctxp, vp1, &device_info, ebufp);

				if (device_info)
				{
					device_pdp = PIN_FLIST_FLD_GET(device_info, PIN_FLD_POID, 1, ebufp );
				}
				fm_mso_get_device_type_from_poid(ctxp, device_pdp, device_type, ebufp);
				//strcpy(device_type_str, device_type);
				//strcat(device_type_str, " ");
				//strcat(device_type_str, device_id);
				
				PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
				fm_mso_cust_get_plan_code(ctxp, vp, &plan_info, ebufp );
				plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp);
				PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp);
				PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_PLAN_TYPE, &two, ebufp);
				PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_TYPE_STR, "HARDWARE_PLAN", ebufp);
				PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_activate_bb_srvc_lc_event Flist:", lc_iflistp);

				sprintf(msg, lc_template_hw, plan_code, device_type, device_id, account_no, agreement_no );

				fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
					msg, action_name1, event, lc_iflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&device_info, NULL);
				PIN_FLIST_DESTROY_EX(&plan_info, NULL);
			}
		}
		else if (plan_type && *plan_type == 3 )	//INSTALLATION_PLANS
		{
			plan_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_PLAN_OBJ, 1, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
			fm_mso_cust_get_plan_code(ctxp, vp, &plan_info, ebufp );
			plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_PLAN_TYPE, &one, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_TYPE_STR, "INSTALLATION_PLAN", ebufp);
			//PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DEVICE_ID,   device_id, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_activate_bb_srvc_lc_event Flist:", lc_iflistp);

			sprintf(msg, lc_template,"Installation", plan_code, account_no, agreement_no );

			fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
				msg, action_name1, event, lc_iflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&plan_info, NULL);
		}
	}
	
	if (lc_template)
	{
		free(lc_template);
	}
	if (lc_template_hw)
	{
		free(lc_template_hw);
	}


	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&device_info, NULL) ;
	 
	return;
}


void
fm_mso_modify_actv_plan_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*hw_plans = NULL;
	pin_flist_t		*inst_plans = NULL;
	pin_flist_t		*subs_plans = NULL;
	pin_flist_t		*old_plan_flistp = NULL;
	pin_flist_t		*new_plan_flistp = NULL;

	poid_t			*srvc_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*old_plan_pdp = NULL;
	poid_t			*new_plan_pdp = NULL;
		
	int64			db = 1;

	char			*event = "/event/activity/mso_lifecycle/modify_activation_plan";

	char			*action_name = "MODIFY_ACTIVATION_PLAN";

	char			*lc_template = NULL;
	char			*prog_name = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*old_plan = NULL;
	char			*new_plan = NULL;
	char			*action = NULL;
	char			*old_plan_code = NULL;
	char			*new_plan_code = NULL;

	int			string_id = ID_MODIFY_ACTIVATION_PLAN_LIST;

	int32			elem_id = 0;

	pin_cookie_t		cookie = NULL;

	void			*vp = NULL;
	
	char			msg[255]="";

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_actv_plan_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_actv_plan_lc_event Output FList:", o_flistp);
	
	acct_pdp     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	srvc_pdp     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ,  1, ebufp );
	prog_name    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp)
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_ACTIVATION_PLAN, 0, ebufp);
	//PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	//PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);

	hw_plans = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_HARDWARE_PLANS, 1, ebufp);
	if (hw_plans)
	{
		elem_id = 0;
		cookie = NULL;
		while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(hw_plans, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			action = PIN_FLIST_FLD_GET(vp, PIN_FLD_ACTION, 1, ebufp);
			if (action && strcmp(action, "DELETE") == 0)
			{
				old_plan_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_PLAN_OBJ, 1, ebufp );
				fm_mso_cust_get_plan_code(ctxp, vp, &old_plan_flistp, ebufp );
				old_plan_code = PIN_FLIST_FLD_GET(old_plan_flistp, PIN_FLD_CODE, 1, ebufp);
				//old_plan = PIN_FLIST_FLD_GET(vp, MSO_FLD_PLAN_NAME, 1, ebufp);
				PIN_FLIST_FLD_COPY(vp, PIN_FLD_PLAN_OBJ, lc_temp_flistp, MSO_FLD_OLD_PLAN_OBJ, ebufp);
				PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_FROM_CODE, old_plan_code, ebufp);
				//PIN_FLIST_FLD_COPY(vp, MSO_FLD_PLAN_NAME, lc_temp_flistp, PIN_FLD_FROM_CODE, ebufp);
			}
			else if (action && strcmp(action, "ADD") == 0)
			{
				new_plan_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_PLAN_OBJ, 1, ebufp );
				fm_mso_cust_get_plan_code(ctxp, vp, &new_plan_flistp, ebufp );
				new_plan_code = PIN_FLIST_FLD_GET(new_plan_flistp, PIN_FLD_CODE, 1, ebufp);
				//new_plan = PIN_FLIST_FLD_GET(vp, MSO_FLD_PLAN_NAME, 1, ebufp);
				PIN_FLIST_FLD_COPY(vp, PIN_FLD_PLAN_OBJ, lc_temp_flistp, MSO_FLD_NEW_PLAN_OBJ, ebufp);
				PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_TO_CODE, new_plan_code, ebufp);
				//PIN_FLIST_FLD_COPY(vp, MSO_FLD_PLAN_NAME, lc_temp_flistp, PIN_FLD_TO_CODE, ebufp);
			}
		}
		sprintf(msg, lc_template, "Hardware", old_plan_code, new_plan_code, account_no, agreement_no );
		//sprintf(msg, lc_template, old_plan, new_plan, account_no, agreement_no );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_actv_plan_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action_name, event, lc_iflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&old_plan_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&new_plan_flistp, NULL);
	}
	
	inst_plans = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_INSTALLATION_PLANS, 1, ebufp);
	if (inst_plans)
	{
		elem_id = 0;
		cookie = NULL;
		while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(inst_plans, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			action = PIN_FLIST_FLD_GET(vp, PIN_FLD_ACTION, 1, ebufp);
			if (action && strcmp(action, "DELETE") == 0)
			{
				old_plan_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_PLAN_OBJ, 1, ebufp );
				fm_mso_cust_get_plan_code(ctxp, vp, &old_plan_flistp, ebufp );
				old_plan_code = PIN_FLIST_FLD_GET(old_plan_flistp, PIN_FLD_CODE, 1, ebufp);
				//old_plan = PIN_FLIST_FLD_GET(vp, MSO_FLD_PLAN_NAME, 1, ebufp);
				PIN_FLIST_FLD_COPY(vp, PIN_FLD_PLAN_OBJ, lc_temp_flistp, MSO_FLD_OLD_PLAN_OBJ, ebufp);
				PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_FROM_CODE, old_plan_code, ebufp);
				//PIN_FLIST_FLD_COPY(vp, MSO_FLD_PLAN_NAME, lc_temp_flistp, PIN_FLD_FROM_CODE, ebufp);
			}
			else if (action && strcmp(action, "ADD") == 0)
			{
				new_plan_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_PLAN_OBJ, 1, ebufp );
				fm_mso_cust_get_plan_code(ctxp, vp, &new_plan_flistp, ebufp );
				new_plan_code = PIN_FLIST_FLD_GET(new_plan_flistp, PIN_FLD_CODE, 1, ebufp);
				//new_plan = PIN_FLIST_FLD_GET(vp, MSO_FLD_PLAN_NAME, 1, ebufp);
				PIN_FLIST_FLD_COPY(vp, PIN_FLD_PLAN_OBJ, lc_temp_flistp, MSO_FLD_NEW_PLAN_OBJ, ebufp);
				PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_TO_CODE, new_plan_code, ebufp);
				//PIN_FLIST_FLD_COPY(vp, MSO_FLD_PLAN_NAME, lc_temp_flistp, PIN_FLD_TO_CODE, ebufp);
			}
		}
		sprintf(msg, lc_template, "Installation", old_plan_code, new_plan_code, account_no, agreement_no );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_actv_plan_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action_name, event, lc_iflistp, ebufp);
		
		PIN_FLIST_DESTROY_EX(&old_plan_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&new_plan_flistp, NULL);
	}
	subs_plans = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_SUBSCRIPTION_PLANS, 1, ebufp);
	if (subs_plans)
	{
		elem_id = 0;
		cookie = NULL;
		while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(subs_plans, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			action = PIN_FLIST_FLD_GET(vp, PIN_FLD_ACTION, 1, ebufp);
			if (action && strcmp(action, "DELETE") == 0)
			{
				old_plan_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_PLAN_OBJ, 1, ebufp );
				fm_mso_cust_get_plan_code(ctxp, vp, &old_plan_flistp, ebufp );
				old_plan_code = PIN_FLIST_FLD_GET(old_plan_flistp, PIN_FLD_CODE, 1, ebufp);
				//old_plan = PIN_FLIST_FLD_GET(vp, MSO_FLD_PLAN_NAME, 1, ebufp);
				PIN_FLIST_FLD_COPY(vp, PIN_FLD_PLAN_OBJ, lc_temp_flistp, MSO_FLD_OLD_PLAN_OBJ, ebufp);
				//PIN_FLIST_FLD_COPY(vp, MSO_FLD_PLAN_NAME, lc_temp_flistp, PIN_FLD_FROM_CODE, ebufp);
				PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_FROM_CODE, old_plan_code, ebufp);
			}
			else if (action && strcmp(action, "ADD") == 0)
			{
				new_plan_pdp = PIN_FLIST_FLD_GET(vp, PIN_FLD_PLAN_OBJ, 1, ebufp );
				fm_mso_cust_get_plan_code(ctxp, vp, &new_plan_flistp, ebufp );
				new_plan_code = PIN_FLIST_FLD_GET(new_plan_flistp, PIN_FLD_CODE, 1, ebufp);
				//new_plan = PIN_FLIST_FLD_GET(vp, MSO_FLD_PLAN_NAME, 1, ebufp);
				PIN_FLIST_FLD_COPY(vp, PIN_FLD_PLAN_OBJ, lc_temp_flistp, MSO_FLD_NEW_PLAN_OBJ, ebufp);
				//PIN_FLIST_FLD_COPY(vp, MSO_FLD_PLAN_NAME, lc_temp_flistp, PIN_FLD_TO_CODE, ebufp);
				PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_TO_CODE, new_plan_code, ebufp);
			}
		}
		sprintf(msg, lc_template, "Subscription", old_plan_code, new_plan_code, account_no, agreement_no );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_actv_plan_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action_name, event, lc_iflistp, ebufp);

		PIN_FLIST_DESTROY_EX(&old_plan_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&new_plan_flistp, NULL) ;
	}

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	 
	return;
}


void
fm_mso_modify_tax_exemp_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*hw_plans = NULL;
	pin_flist_t		*inst_plans = NULL;
	pin_flist_t		*subs_plans = NULL;
	pin_flist_t		*browse_exmp_arr = NULL;
	pin_flist_t		*sub_exmp_arr = NULL;
	pin_flist_t		*new_tax_exemp = NULL;
	pin_flist_t		*old_tax_exemp = NULL;
	pin_flist_t		*old_tax_exemp_info = NULL;
	pin_flist_t		*new_tax_exemp_info = NULL;

	poid_t			*srvc_pdp = NULL;
	poid_t			*acct_pdp = NULL;
		
	int64			db = 1;

	char			*event = "/event/activity/mso_lifecycle/update_tax_exemp";

	char			*action_name_add = "ADD_CUSTOMER_TAX_EXEMPTION";
	char			*action_name_mod = "MODIFY_CUSTOMER_TAX_EXEMPTION";
	char			*action_name_del = "DELETE_CUSTOMER_TAX_EXEMPTION";

	char			*lc_template = NULL;
	char			*prog_name = NULL;
	char			*account_no = NULL;
	char			*certificate_num_old = NULL;
	char			*certificate_num_new = NULL;
	
	pin_decimal_t		*percent_old = NULL;
	pin_decimal_t		*percent_new = NULL;

	int			string_id = ID_MODIFY_CUSTOMER_TAX_EXEMPTION;

	time_t			*start_t_old = NULL;
	time_t			*start_t_new = NULL;
	time_t			*end_t_old = NULL;
	time_t			*end_t_new = NULL;

	int32			elem_id = 0;
	int32			elem_id1 = 0;
	int32			*type = NULL;
	int32			*type1 = NULL;
	int32			add_exmp = 0;
	int32			modify_exmp = 0;
	int32			gen_evt = 0;
	
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;

	void			*vp  = NULL;
	void			*vp1 = NULL;
	
	char			msg[255]="";

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_tax_exemp_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_tax_exemp_lc_event Output FList:", o_flistp);
	
	acct_pdp     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	srvc_pdp     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_SERVICE_OBJ,  1, ebufp );
	prog_name    = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );

	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
//	if (srvc_pdp)
//	{
//		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
//		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
//	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_UPDATE_TAX_EXEMPTION, 0, ebufp);
	lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_UPDATE_TAX_EXEMPTION, 1, ebufp);

	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);

	old_tax_exemp_info = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	new_tax_exemp_info = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "old_tax_exemp_info:", old_tax_exemp_info);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "new_tax_exemp_info:", new_tax_exemp_info);

	
	if (PIN_FLIST_ELEM_COUNT(old_tax_exemp_info, PIN_FLD_EXEMPTIONS, ebufp) >= PIN_FLIST_ELEM_COUNT(new_tax_exemp_info, PIN_FLD_EXEMPTIONS, ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");
		browse_exmp_arr = old_tax_exemp_info;
		sub_exmp_arr = new_tax_exemp_info;
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");
		browse_exmp_arr = new_tax_exemp_info;
		sub_exmp_arr = old_tax_exemp_info;
		add_exmp = 1;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");

	while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(browse_exmp_arr, PIN_FLD_EXEMPTIONS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "vp:", vp);
		type = PIN_FLIST_FLD_GET(vp, PIN_FLD_TYPE, 1, ebufp );
		elem_id1=0;
		cookie1=NULL;
		while((vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(sub_exmp_arr, PIN_FLD_EXEMPTIONS,
			&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "vp1:", vp1);
			type1 =  PIN_FLIST_FLD_GET(vp1, PIN_FLD_TYPE, 1, ebufp );
			if (type && type1 && *type == *type1 )
			{
				modify_exmp=1;
				break;
			}
		}

		/*************************
		 case: Add EXEMPTIONS
		 ************************/
		if (add_exmp == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");
			PIN_FLIST_ELEM_COPY(browse_exmp_arr, PIN_FLD_EXEMPTIONS, elem_id, lc_temp_flistp_new, PIN_FLD_EXEMPTIONS, elem_id, ebufp);
			sprintf(msg, lc_template, "Added", account_no);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_tax_exemp_lc_event Flist:", lc_iflistp);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
				msg, action_name_add, event, lc_iflistp, ebufp);
		}


		/*************************
		 case: Modify EXEMPTIONS
		 ************************/
		if (modify_exmp == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");
			new_tax_exemp = PIN_FLIST_ELEM_ADD(lc_temp_flistp_new, PIN_FLD_EXEMPTIONS, elem_id1, ebufp );
			old_tax_exemp = PIN_FLIST_ELEM_ADD(lc_temp_flistp_old, PIN_FLD_EXEMPTIONS, elem_id, ebufp );

			certificate_num_old = PIN_FLIST_FLD_GET(vp,  PIN_FLD_CERTIFICATE_NUM, 1, ebufp );
			certificate_num_new = PIN_FLIST_FLD_GET(vp1, PIN_FLD_CERTIFICATE_NUM, 1, ebufp );
			if (certificate_num_old && certificate_num_new && strcmp(certificate_num_old, certificate_num_new) !=0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");
				PIN_FLIST_FLD_SET(new_tax_exemp, PIN_FLD_CERTIFICATE_NUM, certificate_num_new, ebufp);
				PIN_FLIST_FLD_SET(old_tax_exemp, PIN_FLD_CERTIFICATE_NUM, certificate_num_old, ebufp);
				gen_evt =1;
			}

			start_t_old = PIN_FLIST_FLD_GET(vp,  PIN_FLD_START_T, 1, ebufp );
			start_t_new = PIN_FLIST_FLD_GET(vp1, PIN_FLD_START_T, 1, ebufp );
			if (start_t_old && start_t_new && *start_t_old != *start_t_new )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");
				PIN_FLIST_FLD_SET(new_tax_exemp, PIN_FLD_START_T, start_t_new, ebufp);
				PIN_FLIST_FLD_SET(old_tax_exemp, PIN_FLD_START_T, start_t_old, ebufp);
				gen_evt =1;
			}

			end_t_old = PIN_FLIST_FLD_GET(vp,  PIN_FLD_END_T, 1, ebufp );
			end_t_new = PIN_FLIST_FLD_GET(vp1, PIN_FLD_END_T, 1, ebufp );
			if (end_t_old && end_t_new && *end_t_old != *end_t_new )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");
				PIN_FLIST_FLD_SET(new_tax_exemp, PIN_FLD_END_T, end_t_new, ebufp);
				PIN_FLIST_FLD_SET(old_tax_exemp, PIN_FLD_END_T, end_t_old, ebufp);
				gen_evt =1;
			}

			percent_old = PIN_FLIST_FLD_GET(vp,  PIN_FLD_PERCENT, 1, ebufp );
			percent_new = PIN_FLIST_FLD_GET(vp1, PIN_FLD_PERCENT, 1, ebufp );
			if (percent_old && percent_new && pbo_decimal_compare(percent_old, percent_new, ebufp) != 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");
				PIN_FLIST_FLD_SET(new_tax_exemp, PIN_FLD_PERCENT, percent_new, ebufp);
				PIN_FLIST_FLD_SET(old_tax_exemp, PIN_FLD_PERCENT, percent_old, ebufp);
				gen_evt =1;
			}

			PIN_FLIST_FLD_SET(old_tax_exemp, PIN_FLD_TYPE, type, ebufp);
			PIN_FLIST_FLD_SET(new_tax_exemp, PIN_FLD_TYPE, type1, ebufp);
				
			if (gen_evt == 1)
			{
				sprintf(msg, lc_template, "Modified", account_no);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_tax_exemp_lc_event Flist:", lc_iflistp);

				fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
					msg, action_name_mod, event, lc_iflistp, ebufp);
			}
		}
		/*************************
		 case: Delete EXEMPTIONS
		 ************************/
		if (add_exmp ==0 && modify_exmp ==0 )
		{
			PIN_FLIST_ELEM_COPY(browse_exmp_arr, PIN_FLD_EXEMPTIONS, elem_id, lc_temp_flistp_old, PIN_FLD_EXEMPTIONS, elem_id, ebufp);
			sprintf(msg, lc_template, "Deleted", account_no);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_tax_exemp_lc_event Flist:", lc_iflistp);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
				msg, action_name_del, event, lc_iflistp, ebufp);
		}
	}

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	 
	return;
}

void
fm_mso_update_hierarchy_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*old_data_array = NULL;
	pin_flist_t		*new_data_array = NULL;
	pin_flist_t		*old_cc_info = NULL;
	pin_flist_t		*new_cc_info = NULL;
	pin_flist_t		*old_ws_info = NULL;
	pin_flist_t		*new_ws_info = NULL;
	pin_flist_t		*results = NULL;
	pin_flist_t		*old_parent_acnt_info = NULL;
	pin_flist_t		*new_parent_acnt_info = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*old_lco_obj         = NULL;
	poid_t			*new_lco_obj         = NULL;
	poid_t			*old_sdt_obj         = NULL;
	poid_t			*new_sdt_obj         = NULL;
	poid_t			*old_dt_obj          = NULL;
	poid_t			*new_dt_obj          = NULL;
	poid_t			*old_jv_obj          = NULL;
	poid_t			*new_jv_obj          = NULL;
	poid_t			*old_parent_acnt_obj = NULL;
	poid_t			*new_parent_acnt_obj = NULL;
	poid_t			*old_parent_obj = NULL;
	poid_t			*new_parent_obj = NULL;
		
	int64			db = 1;

	char			*event = "/event/activity/mso_lifecycle/update_org";

	char			*action_name_add = "MODIFY_HIERARCHY";
	char			*lc_template = NULL;
	char			*prog_name             = NULL;
	char			*child_account_no      = NULL;
	char			*old_parent_account_no = NULL;
	char			*new_parent_account_no = NULL;
	char			change_parent_type[64]="";
	char			msg[255]="";
	
	int			string_id = ID_HIERARCHY_MODIFICATION;


	int32			elem_id = 0;
	

	void			*vp  = NULL;
	void			*vp1 = NULL;
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_hierarchy_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_hierarchy_lc_event Output FList:", o_flistp);
	
	acct_pdp      = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	child_account_no = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );

	if (acct_pdp && !child_account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		child_account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_ORG, 0, ebufp);
	lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_ORG, 1, ebufp);

	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, child_account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_NO, child_account_no, ebufp);

	old_data_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	old_cc_info = PIN_FLIST_SUBSTR_GET(old_data_array, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
	old_ws_info = PIN_FLIST_SUBSTR_GET(old_data_array, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
	
	new_data_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	new_cc_info = PIN_FLIST_SUBSTR_GET(new_data_array, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
	new_ws_info = PIN_FLIST_SUBSTR_GET(new_data_array, MSO_FLD_WHOLESALE_INFO, 1, ebufp);


	/*****************************
	 LCO change for cust
	*****************************/
	if (old_cc_info && new_cc_info)
	{

		old_lco_obj  = (poid_t*)PIN_FLIST_FLD_GET(old_cc_info, MSO_FLD_LCO_OBJ, 1, ebufp);
		new_lco_obj = (poid_t*)PIN_FLIST_FLD_GET(new_cc_info, MSO_FLD_LCO_OBJ, 1, ebufp);

		old_sdt_obj  = (poid_t*)PIN_FLIST_FLD_GET(old_cc_info, MSO_FLD_SDT_OBJ, 1, ebufp);
		new_sdt_obj = (poid_t*)PIN_FLIST_FLD_GET(new_cc_info, MSO_FLD_SDT_OBJ, 1, ebufp);

		old_dt_obj  = (poid_t*)PIN_FLIST_FLD_GET(old_cc_info, MSO_FLD_DT_OBJ, 1, ebufp);
		new_dt_obj = (poid_t*)PIN_FLIST_FLD_GET(new_cc_info, MSO_FLD_DT_OBJ, 1, ebufp);

		old_jv_obj  = (poid_t*)PIN_FLIST_FLD_GET(old_cc_info, MSO_FLD_JV_OBJ, 1, ebufp);
		new_jv_obj = (poid_t*)PIN_FLIST_FLD_GET(new_cc_info, MSO_FLD_JV_OBJ, 1, ebufp);

		old_parent_obj = (poid_t*)PIN_FLIST_FLD_GET(old_cc_info, PIN_FLD_PARENT, 1, ebufp);
		new_parent_obj = (poid_t*)PIN_FLIST_FLD_GET(new_cc_info, PIN_FLD_PARENT, 1, ebufp);


		if (old_lco_obj && old_lco_obj && PIN_POID_COMPARE(old_lco_obj, new_lco_obj, 0, ebufp)!=0 )
		{
			strcpy(change_parent_type, "LCO");
		}
 		else if (old_sdt_obj && new_sdt_obj && PIN_POID_COMPARE(old_sdt_obj, new_sdt_obj, 0, ebufp)!=0 )
		{
			strcpy(change_parent_type, "SDT");
		}
		else if (old_dt_obj && new_dt_obj && PIN_POID_COMPARE(old_dt_obj, new_dt_obj, 0, ebufp)!=0 )
		{
			strcpy(change_parent_type, "DT");
		}
		else if (old_jv_obj && new_jv_obj && PIN_POID_COMPARE(old_jv_obj, new_jv_obj, 0, ebufp)!=0 )
		{
			strcpy(change_parent_type, "JV");
		}

		old_parent_acnt_obj =  PIN_FLIST_FLD_GET(old_cc_info, MSO_FLD_LCO_OBJ, 1, ebufp);
		if (old_parent_acnt_obj)
		{
			fm_mso_get_account_info(ctxp, old_parent_acnt_obj, &old_parent_acnt_info, ebufp);
			old_parent_account_no = PIN_FLIST_FLD_GET(old_parent_acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		}

		results = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (results)
		{
			new_parent_account_no = PIN_FLIST_FLD_GET(results, PIN_FLD_ACCOUNT_NO, 1, ebufp);
			child_account_no  = PIN_FLIST_FLD_GET(results, PIN_FLD_DESCR, 1, ebufp); 
		}
		else
		{
			new_parent_acnt_obj =  PIN_FLIST_FLD_GET(new_cc_info, MSO_FLD_LCO_OBJ, 1, ebufp);
			if (new_parent_acnt_obj)
			{
				fm_mso_get_account_info(ctxp, new_parent_acnt_obj, &new_parent_acnt_info, ebufp);
				new_parent_account_no = PIN_FLIST_FLD_GET(new_parent_acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
			}
		}
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_LCO_OBJ, old_lco_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_SDT_OBJ, old_sdt_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_DT_OBJ,  old_dt_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_JV_OBJ,  old_jv_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_PARENT,  old_parent_obj, ebufp);
		PIN_FLIST_FLD_COPY(old_cc_info, PIN_FLD_CUSTOMER_TYPE, lc_temp_flistp_old, PIN_FLD_CUSTOMER_TYPE, ebufp);

		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_LCO_OBJ, new_lco_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_SDT_OBJ, new_sdt_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_DT_OBJ,  new_dt_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_JV_OBJ,  new_jv_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_PARENT,  new_parent_obj, ebufp);
		PIN_FLIST_FLD_COPY(old_cc_info, PIN_FLD_CUSTOMER_TYPE, lc_temp_flistp_new, PIN_FLD_CUSTOMER_TYPE, ebufp);

		sprintf(msg, lc_template, change_parent_type,  old_parent_account_no, new_parent_account_no, child_account_no );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_hierarchy_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
			msg, action_name_add, event, lc_iflistp, ebufp);
	}

	/*****************************
	 BU Change
	*****************************/
	if (old_ws_info && new_ws_info)
	{

		old_lco_obj  = (poid_t*)PIN_FLIST_FLD_GET(old_ws_info, MSO_FLD_LCO_OBJ, 1, ebufp);
		new_lco_obj = (poid_t*)PIN_FLIST_FLD_GET(new_ws_info, MSO_FLD_LCO_OBJ, 1, ebufp);

		old_sdt_obj  = (poid_t*)PIN_FLIST_FLD_GET(old_ws_info, MSO_FLD_SDT_OBJ, 1, ebufp);
		new_sdt_obj = (poid_t*)PIN_FLIST_FLD_GET(new_ws_info, MSO_FLD_SDT_OBJ, 1, ebufp);

		old_dt_obj  = (poid_t*)PIN_FLIST_FLD_GET(old_ws_info, MSO_FLD_DT_OBJ, 1, ebufp);
		new_dt_obj = (poid_t*)PIN_FLIST_FLD_GET(new_ws_info, MSO_FLD_DT_OBJ, 1, ebufp);

		old_jv_obj  = (poid_t*)PIN_FLIST_FLD_GET(old_ws_info, MSO_FLD_JV_OBJ, 1, ebufp);
		new_jv_obj = (poid_t*)PIN_FLIST_FLD_GET(new_ws_info, MSO_FLD_JV_OBJ, 1, ebufp);

		if (old_lco_obj && old_lco_obj && PIN_POID_COMPARE(old_lco_obj, new_lco_obj, 0, ebufp)!=0 )
		{
			strcpy(change_parent_type, "LCO");
		}
 		else if (old_sdt_obj && new_sdt_obj && PIN_POID_COMPARE(old_sdt_obj, new_sdt_obj, 0, ebufp)!=0 )
		{
			strcpy(change_parent_type, "SDT");
		}
		else if (old_dt_obj && new_dt_obj && PIN_POID_COMPARE(old_dt_obj, new_dt_obj, 0, ebufp)!=0 )
		{
			strcpy(change_parent_type, "DT");
		}
		else if (old_jv_obj && new_jv_obj && PIN_POID_COMPARE(old_jv_obj, new_jv_obj, 0, ebufp)!=0 )
		{
			strcpy(change_parent_type, "JV");
		}

		old_parent_acnt_obj =  PIN_FLIST_FLD_GET(old_ws_info, PIN_FLD_PARENT, 1, ebufp);
		if (old_parent_acnt_obj)
		{
			fm_mso_get_account_info(ctxp, old_parent_acnt_obj, &old_parent_acnt_info, ebufp);
			old_parent_account_no = PIN_FLIST_FLD_GET(old_parent_acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		}


		results = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (results)
		{
			new_parent_account_no = PIN_FLIST_FLD_GET(results, PIN_FLD_ACCOUNT_NO, 1, ebufp);
			child_account_no  = PIN_FLIST_FLD_GET(results, PIN_FLD_DESCR, 1, ebufp); 
		}

		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_LCO_OBJ, old_lco_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_SDT_OBJ, old_sdt_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_DT_OBJ,  old_dt_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_JV_OBJ,  old_jv_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_PARENT,  old_parent_obj, ebufp);
		PIN_FLIST_FLD_COPY(old_ws_info, PIN_FLD_CUSTOMER_TYPE, lc_temp_flistp_old, PIN_FLD_CUSTOMER_TYPE, ebufp);

		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_LCO_OBJ, new_lco_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_SDT_OBJ, new_sdt_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_DT_OBJ,  new_dt_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_JV_OBJ,  new_jv_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_PARENT,  new_parent_obj, ebufp);
		PIN_FLIST_FLD_COPY(old_ws_info, PIN_FLD_CUSTOMER_TYPE, lc_temp_flistp_new, PIN_FLD_CUSTOMER_TYPE, ebufp);

		sprintf(msg, lc_template, change_parent_type, old_parent_account_no, new_parent_account_no, child_account_no );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_hierarchy_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
			msg, action_name_add, event, lc_iflistp, ebufp);
	}


	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&old_parent_acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&new_parent_acnt_info, NULL) ;
	 
	return;
}

void
fm_mso_update_business_type_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*old_data_array = NULL;
	pin_flist_t		*new_data_array = NULL;

	poid_t			*acct_pdp = NULL;
		
	int64			db = 1;

	char			*event = "/event/activity/mso_lifecycle/business_type";

	char			*action_name_add = "MODIFY_BUSINESS_TYPE";
	char			*lc_template = NULL;
	char			*prog_name             = NULL;
	char			*account_no      = NULL;
	char			msg[255]="";
	
	int			string_id = ID_BUSINESS_TYPE_MODIFICATION;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_business_type_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_business_type_lc_event Output FList:", o_flistp);
	
	acct_pdp      = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );

	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_BUSYNESS_TYPE, 0, ebufp);
	lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_BUSYNESS_TYPE, 1, ebufp);

	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

	old_data_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	PIN_FLIST_FLD_COPY(old_data_array, PIN_FLD_BUSINESS_TYPE, lc_temp_flistp_old, PIN_FLD_BUSINESS_TYPE, ebufp);
	
	new_data_array = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	PIN_FLIST_FLD_COPY(new_data_array, PIN_FLD_BUSINESS_TYPE, lc_temp_flistp_new, PIN_FLD_BUSINESS_TYPE, ebufp);


	sprintf(msg, lc_template, account_no);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_business_type_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action_name_add, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	return;
}


void
fm_mso_modify_bb_service_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*old_bb_info = NULL;
	pin_flist_t		*new_bb_info = NULL;
	pin_flist_t		*old_bal_info = NULL;
	pin_flist_t		*new_bal_info = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
		
	int64			db = 1;

	char			*event1 = "/event/activity/mso_lifecycle/modify_cmts";
	char			*event2 = "/event/activity/mso_lifecycle/modify_authentication";
	char			*event3 = "/event/activity/mso_lifecycle/change_bb_login";
	char			*event4 = "/event/activity/mso_lifecycle/change_bb_password";
	char			*event5 = "/event/activity/mso_lifecycle/set_credit_limit";

	char			*action1= "MODIFY_BB_SEVICE_CMTS";
	char			*action2= "MODIFY_BB_SERVICE_AUTHENTICATION";
	char			*action3= "MODIFY_BB_SERVICE_LOGIN";
	char			*action4= "RESET_BB_SERVICE_PASSWD";
	char			*action5= "MODIFY_CREDIT_LIMIT";

	char			*lc_template = NULL;
	char			*prog_name   = NULL;
	char			*account_no  = NULL;
	char			*agreement_no = NULL;
	char			msg[255]="";
	char			auth_info[128]="";
	char			*old_ne = NULL;
	char			*new_ne = NULL;
	char			*old_login = NULL;
	char			*new_login = NULL;
	char			*old_passwd = NULL;
	char			*new_passwd = NULL;
	
	int			string_id1 = ID_MODIFY_BB_SEVICE_CMTS;
	int			string_id2 = ID_MODIFY_BB_SERVICE_AUTHENTICATION;
	int			string_id3 = ID_MODIFY_BB_SERVICE_LOGIN;
	int			string_id4 = ID_RESET_BB_SERVICE_LOGIN_PASSWORD;
	int			string_id5 = ID_MODIFY_CREDIT_LIMIT;

	void			*vp = NULL;
	void			*vp1 = NULL;

	int32			*old_is_tal = NULL;
	int32			*new_is_tal = NULL;
	int32			*old_is_click = NULL;
	int32			*new_is_click = NULL;
        int32                   *old_waiver_flag = NULL;
        int32                   *new_waiver_flag = NULL;

	int32			elem_id = 0;

	pin_decimal_t		*credit_limit_old = NULL;
	pin_decimal_t		*credit_limit_new = NULL;
	pin_decimal_t		*credit_floor_new = NULL;
	pin_decimal_t		*credit_floor_old = NULL;

	pin_cookie_t		cookie = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_bb_service_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_bb_service_lc_event Output FList:", o_flistp);
	
	acct_pdp      = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no    = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
	agreement_no  = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp );

	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	if (vp)
	{
		vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_SERVICES, 0, 1, ebufp );
		if (vp1)
		{
			srvc_pdp    = PIN_FLIST_FLD_GET(vp1, PIN_FLD_SERVICE_OBJ,  1, ebufp );
			old_bb_info = PIN_FLIST_SUBSTR_GET(vp1, MSO_FLD_BB_INFO, 1, ebufp);
			old_login   = PIN_FLIST_FLD_GET(vp1, PIN_FLD_LOGIN,  1, ebufp );
			old_passwd  = PIN_FLIST_FLD_GET(vp1, PIN_FLD_PASSWD,  1, ebufp );
		}
		old_bal_info = PIN_FLIST_ELEM_GET(vp, PIN_FLD_BAL_IMPACTS, 0, 1, ebufp );
	}

	vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	if (vp)
	{
		vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_SERVICES, 0, 1, ebufp );
		if (vp1)
		{
			new_bb_info = PIN_FLIST_SUBSTR_GET(vp1, MSO_FLD_BB_INFO, 1, ebufp);
			new_login   = PIN_FLIST_FLD_GET(vp1, PIN_FLD_LOGIN,  1, ebufp );
			new_passwd  = PIN_FLIST_FLD_GET(vp1, PIN_FLD_PASSWD,  1, ebufp );
		}
		new_bal_info = PIN_FLIST_ELEM_GET(vp, PIN_FLD_BAL_IMPACTS, 0, 1, ebufp );
	}

	if (old_bb_info)
	{
		old_ne = PIN_FLIST_FLD_GET(old_bb_info,MSO_FLD_NETWORK_ELEMENT, 1, ebufp);
		old_is_tal = PIN_FLIST_FLD_GET(old_bb_info,MSO_FLD_ISTAL_FLAG, 1, ebufp);
		old_is_click = PIN_FLIST_FLD_GET(old_bb_info,MSO_FLD_IS1CLICK_FLAG, 1, ebufp);
		old_waiver_flag = PIN_FLIST_FLD_GET(old_bb_info,PIN_FLD_REAUTH_FLAG, 1, ebufp);
	}

	if (new_bb_info)
	{
		new_ne = PIN_FLIST_FLD_GET(new_bb_info,MSO_FLD_NETWORK_ELEMENT, 1, ebufp);
		new_is_tal = PIN_FLIST_FLD_GET(new_bb_info,MSO_FLD_ISTAL_FLAG, 1, ebufp);
		new_is_click = PIN_FLIST_FLD_GET(new_bb_info,MSO_FLD_IS1CLICK_FLAG, 1, ebufp);
		new_waiver_flag = PIN_FLIST_FLD_GET(new_bb_info,PIN_FLD_REAUTH_FLAG, 1, ebufp);
	}


	if (old_ne && new_ne && strcmp(old_ne, new_ne) != 0)
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_CMTS, 0, ebufp);
 //		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_OLD_VALUE, old_ne, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_NEW_VALUE, new_ne, ebufp);

		get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, old_ne, new_ne, agreement_no, account_no);
		if (lc_template)
		{
			free(lc_template);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_bb_service_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action1, event1, lc_iflistp, ebufp);

	}

	if ( (old_is_tal && new_is_tal && *old_is_tal != *new_is_tal) ||
	     (old_is_click && new_is_click && *old_is_click != *new_is_click)||
	     (old_waiver_flag && new_waiver_flag && *old_waiver_flag != *new_waiver_flag)
	   )
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_AUTHENTICATION, 0, ebufp);
		lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_AUTHENTICATION, 1, ebufp);

 //		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_NO,    account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_SERVICE_OBJ,   srvc_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_ISTAL_FLAG,    old_is_tal, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_IS1CLICK_FLAG, old_is_click, ebufp);


 //		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO,    account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_SERVICE_OBJ,   srvc_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_ISTAL_FLAG,    new_is_tal, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_IS1CLICK_FLAG, new_is_click, ebufp);

        	if ( (old_is_tal && new_is_tal && *old_is_tal==0 && *new_is_tal ==1) &&
		 	(old_is_click && new_is_click && *old_is_click==1 && *new_is_click ==0)
                   )
		{
			strcpy(auth_info, "from IS CLICK YES to IS TAL");
		}
		else if ( (old_is_tal && new_is_tal && *old_is_tal==0 && *new_is_tal ==1) &&
			 (old_is_click && new_is_click && *old_is_click==2 && *new_is_click ==0)
               		    )
		{
			strcpy(auth_info, "from IS CLICK NOCLICK to IS TAL");
		}
		else if ( (old_is_tal && new_is_tal && *old_is_tal==0 && *new_is_tal ==1) &&
			 (old_is_click && new_is_click && *old_is_click==0 && *new_is_click ==0)
       		            )
		 {
			strcpy(auth_info, "from IS CLICK NO to IS TAL");
		}
		else if ( (old_is_tal && new_is_tal && *old_is_tal==1 && *new_is_tal ==0) &&
			 (old_is_click && new_is_click && *old_is_click==0 && *new_is_click ==0)
       		            )
		{
			strcpy(auth_info, "from IS TAL to IS CLICK NO");
		}
		else if ( (old_is_tal && new_is_tal && *old_is_tal==1 && *new_is_tal ==0) &&
			(old_is_click && new_is_click && *old_is_click==0 && *new_is_click ==1)
       		            )
		{
			strcpy(auth_info, "from IS TAL to IS CLICK YES");
		}
		else if ( (old_is_tal && new_is_tal && *old_is_tal==1 && *new_is_tal ==0) &&
			 (old_is_click && new_is_click && *old_is_click==0 && *new_is_click ==2)
       		            )
		{
			strcpy(auth_info, "from IS TAL to IS CLICK NOCLICK");
		}
		else if ( (old_is_tal && new_is_tal && *old_is_tal==0 && *new_is_tal ==0) &&
			 (old_is_click && new_is_click && *old_is_click==0 && *new_is_click ==1)
       		            )
		{
			strcpy(auth_info, "from IS CLICK NO to IS CLICK YES");
		}
		else if ( (old_is_tal && new_is_tal && *old_is_tal==0 && *new_is_tal ==0) &&
	 		(old_is_click && new_is_click && *old_is_click==0 && *new_is_click ==2)
                   	)
		{
			strcpy(auth_info, "from IS CLICK NO to IS CLICK NOCLICK");
		}
	 	else if ( (old_is_tal && new_is_tal && *old_is_tal==0 && *new_is_tal ==0) &&
			(old_is_click && new_is_click && *old_is_click==1 && *new_is_click ==0)
               		    )
		{
			strcpy(auth_info, "from IS CLICK YES to IS CLICK NO");
		}
		else if ( (old_is_tal && new_is_tal && *old_is_tal==0 && *new_is_tal ==0) &&
			 (old_is_click && new_is_click && *old_is_click==1 && *new_is_click ==2)
       		            )
		{
			strcpy(auth_info, "from IS CLICK YES to IS CLICK NOCLICK");
		}
		else if ( (old_is_tal && new_is_tal && *old_is_tal==0 && *new_is_tal ==0) &&
			 (old_is_click && new_is_click && *old_is_click==2 && *new_is_click ==0)
       		            )
		{
			strcpy(auth_info, "from IS CLICK NOCLICK to IS CLICK NO");
		}
		else if ( (old_is_tal && new_is_tal && *old_is_tal==0 && *new_is_tal ==0) &&
			 (old_is_click && new_is_click && *old_is_click==2 && *new_is_click ==1)
       		            )
		{
			strcpy(auth_info, "from IS CLICK NOCLICK to IS CLICK YES");
		}
                else if ( (old_waiver_flag && new_waiver_flag && *old_waiver_flag==0 && *new_waiver_flag ==1) )
                {
                        strcpy(auth_info, "from NON WAIVER to WAIVER");
                }
                else if ( (old_waiver_flag && new_waiver_flag && *old_waiver_flag==1 && *new_waiver_flag ==0) )
                {
                        strcpy(auth_info, "from WAIVER to NON WAIVER");
                }

	
		get_evt_lifecycle_template(ctxp, db, string_id2, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, auth_info, agreement_no, account_no);
		if (lc_template)
		{
			free(lc_template);
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_bb_service_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action2, event2, lc_iflistp, ebufp);
		
	}
	if (old_login && new_login && strcmp(old_login, new_login) != 0)
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CHANGE_BB_LOGIN, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_OLD_VALUE, old_login, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_NEW_VALUE, new_login, ebufp);

		get_evt_lifecycle_template(ctxp, db, string_id3, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, agreement_no, account_no);
		if (lc_template)
		{
			free(lc_template);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_bb_srvc_login_pwd_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action3, event3, lc_iflistp, ebufp);
	}
	if (old_passwd && new_passwd && strcmp(old_passwd, new_passwd) != 0)
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CHANGE_BB_PASSWORD, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

		get_evt_lifecycle_template(ctxp, db, string_id4, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, agreement_no, account_no);
		if (lc_template)
		{
			free(lc_template);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_bb_srvc_login_pwd_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action4, event4, lc_iflistp, ebufp);
	}
	//Credit limit modify
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "new_bal_info", new_bal_info);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "old_bal_info", old_bal_info);

	while ( new_bal_info != NULL && 
			(vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(new_bal_info, PIN_FLD_BAL_INFO, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "vp", vp);

		credit_limit_new = PIN_FLIST_FLD_GET(vp, PIN_FLD_CREDIT_LIMIT, 1, ebufp );
		credit_floor_new = PIN_FLIST_FLD_GET(vp, PIN_FLD_CREDIT_FLOOR, 1, ebufp );

		vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(old_bal_info, PIN_FLD_BAL_INFO, elem_id, 1, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "vp1", vp1);
		if (vp1)
		{
			credit_limit_old = PIN_FLIST_FLD_GET(vp1, PIN_FLD_CREDIT_LIMIT, 1, ebufp );
			credit_floor_old = PIN_FLIST_FLD_GET(vp1, PIN_FLD_CREDIT_FLOOR, 1, ebufp );
		}

 		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_CREDIT_LIMIT, 1, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_LIMIT, credit_limit_new, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_FLOOR, credit_floor_new, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_FLOOR, credit_floor_new, ebufp);
		//PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_RESOURCE_ID, elem_id, ebufp);

		lc_temp_flistp_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_CREDIT_LIMIT, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_LIMIT, credit_limit_old, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_FLOOR, credit_floor_old, ebufp);
		//PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_RESOURCE_ID, elem_id, ebufp);


		get_evt_lifecycle_template(ctxp, db, string_id5, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, "Broadband", account_no);
		if (lc_template)
		{
			free(lc_template);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_bb_credit_limit Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action5, event5, lc_iflistp, ebufp);
	}

/*	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "old_passwd" );
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, old_passwd);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "new_passwd" );
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, new_passwd );
*/

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	return;
}


void
fm_mso_modify_csr_roles_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
		
	int64			db = 1;

	char			*event1 = "/event/activity/mso_lifecycle/change_roles";

	char			*action1= "MODIFY_ROLES_OF_CSR";
	char			*lc_template = NULL;
	char			*prog_name   = NULL;
	char			*account_no  = NULL;
	char			*agreement_no = NULL;
	char			msg[255]="";
	char			*old_role = NULL;
	char			*new_role = NULL;
	
	int			string_id1 = ID_MODIFY_ROLES_OF_CSR;

	void			*vp = NULL;
	void			*vp1 = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_csr_roles_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_csr_roles_lc_event Output FList:", o_flistp);
	
	acct_pdp      = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no    = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
	agreement_no  = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp );

	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	if (vp)
	{
		vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_ROLE, 0, 1, ebufp );
		if (vp1)
		{
			old_role      = PIN_FLIST_FLD_GET(vp1, MSO_FLD_ROLES,  1, ebufp );
		}
	}

	vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	if (vp)
	{
		vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_ROLE, 0, 1, ebufp );
		if (vp1)
		{
			new_role = PIN_FLIST_SUBSTR_GET(vp1, MSO_FLD_ROLES, 1, ebufp);
		}
	}

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CHANGE_ROLES, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_OLD_VALUE, old_role, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_NEW_VALUE, new_role, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, account_no);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_csr_roles_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action1, event1, lc_iflistp, ebufp);


	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	return;
}


void
fm_mso_modify_billinfo_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
		
	int64			db = 1;

	char			*event1 = "/event/activity/mso_lifecycle/modify_billinfo";
	char			*event2 = "/event/activity/mso_lifecycle/update_bill_suppression";

	char			*action1= "BILL_INFORMATION_MODIFICATION";
	char			*action2= "BILL_SUPPRESSION_MODIFICATION";
	char			*lc_template = NULL;
	char			*prog_name   = NULL;
	char			*account_no  = NULL;
	char			*agreement_no = NULL;
	char			msg[255]="";
	
	int			string_id1 = ID_BILL_INFORMATION_MODIFICATION;
	int			string_id2 = ID_MODIFY_CUSTOMER_BILL_SUPPRESSION;

	int32			*old_bill_when    = NULL;
	int32			*old_bill_segment  = NULL;
	int32			*old_bdom         = NULL;
	int32			*old_reason         = NULL;
	int32			*old_flag         = NULL;

	int32			*new_bill_when    = NULL;
	int32			*new_bill_segment  = NULL;
	int32			*new_bdom         = NULL;
	int32			*new_reason         = NULL;
	int32			*new_flag         = NULL;
	int32			lc_create_flag    = 0;

	void			*vp = NULL;
	void			*vp1 = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_billinfo_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_billinfo_lc_event Output FList:", o_flistp);
	
	acct_pdp      = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	srvc_pdp      = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_SERVICE_OBJ,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no    = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_NO,   1, ebufp );
	agreement_no  = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp );

	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp && !agreement_no )
	{
		fm_mso_get_srvc_info(ctxp, o_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}

	vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	if (vp)
	{
		vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_BILLINFO, 0, 1, ebufp );
		if (vp1)
		{
			old_bill_when    = PIN_FLIST_FLD_GET(vp1, PIN_FLD_BILL_WHEN,  1, ebufp );
			old_bill_segment = PIN_FLIST_FLD_GET(vp1, PIN_FLD_BILLING_SEGMENT,  1, ebufp );
			old_bdom         = PIN_FLIST_FLD_GET(vp1, PIN_FLD_ACTG_CYCLE_DOM,  1, ebufp );
			old_reason       = PIN_FLIST_FLD_GET(vp1, PIN_FLD_REASON,  1, ebufp ); 
			old_flag         = PIN_FLIST_FLD_GET(vp1, PIN_FLD_FLAGS,  1, ebufp );
		}
		
	}

	vp  = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	if (vp)
	{
		vp1 = (pin_flist_t*)PIN_FLIST_ELEM_GET(vp, PIN_FLD_BILLINFO, 0, 1, ebufp );
		if (vp1)
		{
			new_bill_when = PIN_FLIST_SUBSTR_GET(vp1, PIN_FLD_BILL_WHEN, 1, ebufp);
			new_bill_segment  = PIN_FLIST_FLD_GET(vp1, PIN_FLD_BILLING_SEGMENT,  1, ebufp );
			new_bdom         = PIN_FLIST_FLD_GET(vp1, PIN_FLD_ACTG_CYCLE_DOM,  1, ebufp );
			new_reason       = PIN_FLIST_FLD_GET(vp1, PIN_FLD_REASON,  1, ebufp ); 
			new_flag         = PIN_FLIST_FLD_GET(vp1, PIN_FLD_FLAGS,  1, ebufp );
		}
	}

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_BILLINFO, 0, ebufp);
	lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_MODIFY_BILLINFO, 1, ebufp);
	//PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp_new, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp_old, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);

	/*****************************************************************
	BILL_INFORMATION_MODIFICATION
	*****************************************************************/
	if (old_bill_when && new_bill_when && *old_bill_when!=*new_bill_when)
	{
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_BILL_WHEN, old_bill_when, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_BILL_WHEN, new_bill_when, ebufp);
		lc_create_flag = 1;
	}
	if (old_bill_when && new_bill_when && *old_bill_when!=*new_bill_when)
	{
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_BILLING_SEGMENT, old_bill_when, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_BILLING_SEGMENT, new_bill_when, ebufp);
		lc_create_flag = 1;
	}
	if (old_bdom && new_bdom && *old_bdom!=*new_bdom)
	{
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACTG_CYCLE_DOM, old_bdom, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACTG_CYCLE_DOM, new_bdom, ebufp);
		lc_create_flag = 1;
	}

	/*****************************************************************
	BILL_SUPPRESSION_MODIFICATION
	*****************************************************************/
	if (old_reason && new_reason && *old_reason!=*new_reason)
	{
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_REASON, old_reason, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_REASON, new_reason, ebufp);
		lc_create_flag = 2;
	}	
	
	if (old_flag && new_flag && *old_flag!=*new_flag)
	{
		PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_FLAGS, old_flag, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_FLAGS, new_flag, ebufp);
		lc_create_flag = 2;
	}
	if (lc_create_flag == 1)
	{
		get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, account_no);
		if (lc_template)
		{
			free(lc_template);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_billinfo_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
			msg, action1, event1, lc_iflistp, ebufp);
	}

	if (lc_create_flag == 2 )
	{
		PIN_FLIST_FLD_RENAME(lc_iflistp, MSO_FLD_MODIFY_BILLINFO, MSO_FLD_UPDATE_BILL_SUPPRESSION, ebufp);
		get_evt_lifecycle_template(ctxp, db, string_id2, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, account_no);
		if (lc_template)
		{
			free(lc_template);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_billinfo_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
			msg, action2, event2, lc_iflistp, ebufp);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	return;
}


void
fm_mso_modify_self_care_cred_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*user_id = NULL;
		
	int64			db = 1;

	int32			*caller = NULL ;

	char			*event1 = "/event/activity/mso_lifecycle/change_password";

	char			*action1= "SELFCARE_CHANGE_PASSWORD";
	char			*lc_template  = NULL;
	char			*prog_name    = NULL;
	char			*account_no   = NULL;
	char			*agreement_no = NULL;
	char			*login        = NULL;
	char			*old_login    = NULL;
	char			*new_login    = NULL;
	char			*old_passwd   = NULL;
	char			*new_passwd   = NULL;
	char			msg[255]="";
	
	int			string_id1 = ID_MODIFY_CUSTOMER_CHANGE_USERNAME_PASSWORD;

	void			*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_self_care_cred_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_self_care_cred_lc_event Output FList:", o_flistp);
	
	caller = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_MODE,  1, ebufp );
	
	/*****************************************************************
	Called by MSO_OP_CUST_MODIFY_CUSTOMER_CREDENTIALS
	******************************************************************/
	if (caller && *caller == MSO_OP_CUST_MODIFY_CUSTOMER_CREDENTIALS )
	{
		acct_pdp      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,  1, ebufp );
		user_id       = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID,  1, ebufp );
		prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
		account_no    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO,   1, ebufp );
		login         = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 1, ebufp );

		if (acct_pdp && !account_no)
		{
			fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
			account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		}

		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CHANGE_PASSWORD, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_USERID, user_id, ebufp);
//		PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);

		get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, account_no, login);
		if (lc_template)
		{
			free(lc_template);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_self_care_cred_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
			msg, action1, event1, lc_iflistp, ebufp);
	}
 	/*****************************************************************
	Called by MSO_OP_CUST_MODIFY_CUSTOMER
	******************************************************************/
 	else if (caller && *caller == MSO_OP_CUST_MODIFY_CUSTOMER_CREDENTIALS )
	{
		acct_pdp      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,  1, ebufp );
		user_id       = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID,  1, ebufp );
		prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
		account_no    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO,   1, ebufp );
		login         = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 1, ebufp );

		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
		if (vp)
		{
			old_login = PIN_FLIST_FLD_GET(vp, PIN_FLD_LOGIN, 1, ebufp );
			old_passwd = PIN_FLIST_FLD_GET(vp, PIN_FLD_PASSWD, 1, ebufp );
		}
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
		if (vp)
		{
			new_login = PIN_FLIST_FLD_GET(vp, PIN_FLD_LOGIN, 1, ebufp );
			new_passwd = PIN_FLIST_FLD_GET(vp, PIN_FLD_PASSWD, 1, ebufp );
		}

		if ((old_login && new_login && strcmp(old_login,new_login)!=0) || 
		    (old_passwd && new_passwd && strcmp(old_passwd,new_passwd)!=0) 
		   )
		{

			if (acct_pdp && !account_no)
			{
				fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
				account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
			}

			lc_iflistp = PIN_FLIST_CREATE(ebufp);
			lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CHANGE_PASSWORD, 0, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_USERID, user_id, ebufp);
//			PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);

			get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
			sprintf(msg, lc_template, account_no, login);
			if (lc_template)
			{
				free(lc_template);
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_self_care_cred_lc_event Flist:", lc_iflistp);

			fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
				msg, action1, event1, lc_iflistp, ebufp);
		}
		
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	return;
}


void
fm_mso_activate_deactivate_bu_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*user_id = NULL;
		
	int64			db = 1;

	int32			*caller = NULL ;

	char			*event1 = "/event/activity/mso_lifecycle/change_account_status";

	char			*action1= "DEACTIVATE_ACTIVATE_BUS_USER";
	char			*lc_template  = NULL;
	char			*prog_name    = NULL;
	char			*account_no   = NULL;
	char			*status_msg   = NULL;
	char			msg[255]="";
	
	int			string_id1 = ID_MODIFY_CUSTOMER_DEACTIVATE_ACTIVATE_BUS_USER;

	void			*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_activate_deactivate_bu_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_activate_deactivate_bu_lc_event Output FList:", o_flistp);
	

	acct_pdp      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	status_msg    = PIN_FLIST_FLD_TAKE(o_flistp, PIN_FLD_MESSAGE, 1, ebufp );
	vp            = (pin_flist_t*)PIN_FLIST_ELEM_GET(o_flistp, MSO_FLD_DEACTIVATE_ACCOUNT, 0, 1, ebufp);
	if (vp)
	{
		account_no = PIN_FLIST_FLD_GET(vp, PIN_FLD_ACCOUNT_NO,  1, ebufp );
	}

	lc_iflistp = PIN_FLIST_COPY(o_flistp, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, status_msg, account_no);
	if (lc_template)
	{
		free(lc_template);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_activate_deactivate_bu_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action1, event1, lc_iflistp, ebufp);

	if (status_msg)
	{
		free(status_msg);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	return;
}



void
fm_mso_modify_csr_access_control_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*user_id = NULL;
		
	int64			db = 1;

	int32			*caller = NULL ;

	char			*event1 = "/event/activity/mso_lifecycle/change_access";

	char			*action1= "MODIFY_CSR_ACCESSCONTROL";
	char			*lc_template  = NULL;
	char			*prog_name    = NULL;
	char			*account_no   = NULL;
	char			*status_msg   = NULL;
	char			msg[255]="";
	
	int			string_id1 = ID_MODIFY_ACCESSCONTROL_FOR_CSR;

	void			*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_csr_access_control_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_csr_access_control_lc_event Output FList:", o_flistp);
	

	acct_pdp      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO,  1, ebufp );

	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}


	lc_iflistp = PIN_FLIST_COPY(o_flistp, ebufp);
	PIN_FLIST_FLD_RENAME(lc_iflistp, PIN_FLD_DATA_ARRAY, MSO_FLD_CHANGE_ACCESS, ebufp );


	get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, account_no);
	if (lc_template)
	{
		free(lc_template);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_csr_access_control_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action1, event1, lc_iflistp, ebufp);

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	return;
}



void
mso_op_billing_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	pin_decimal_t		*due = NULL;
	pin_decimal_t		*total_due = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*user_id = NULL;
		
	int64			db = 1;

	int32			*caller = NULL ;

	char			*event1 = "/event/activity/mso_lifecycle/billing";

	char			*action1= "BILLING";
	char			*lc_template  = NULL;
	char			*prog_name    = NULL;
	char			*account_no   = NULL;
	char			*due_str      = NULL;
	char			*total_due_str = NULL;
	char			*bill_no      = NULL;
	char			msg[255]      ="";
	
	int			string_id1    = ID_BILLING;

	void			*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_billing_gen_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_billing_gen_lc_event Output FList:", o_flistp);
	

	acct_pdp      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO,  1, ebufp );
	due           = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DUE,  1, ebufp );
	total_due     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TOTAL_DUE,  1, ebufp );
	bill_no       = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO,  1, ebufp );

	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	if (due)
	{
		due_str = (char*)pbo_decimal_to_str(due, ebufp);
	}
	if (total_due)
	{
		total_due_str = (char*)pbo_decimal_to_str(total_due, ebufp);
	}
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp= PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_BILLS, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, due, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_BILL_NO, bill_no, ebufp);


	get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, due_str, total_due_str, bill_no, account_no);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_billing_gen_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action1, event1, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	if (due_str)
	{
		free(due_str);
	}
	if (total_due_str)
	{
		free(total_due_str);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	return;
}



void
fm_mso_device_diassociate_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t			*acct_pdp   = NULL;
	poid_t			*user_id    = NULL;
	poid_t			*device_pdp = NULL;
	poid_t			*srvc_pdp   = NULL;
		
	int64			db = 1;

	int32			*caller = NULL ;

	char			*event1 = "/event/activity/mso_lifecycle/device_diassociate";

	char			*action1= "DEVICE_DISASSOCIATE";
	char			*lc_template  = NULL;
	char			*prog_name    = NULL;
	char			*account_no   = NULL;
	char			*agreement_no = NULL;
	char			*device_id      = NULL;
	char			*device_sl_no = NULL;
	char			msg[255]      ="";
	char			device_type[64] = "";
	
	int			string_id1    = ID_DEVICE_DISASSOCIATE;

	void			*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_device_diassociate_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_device_diassociate_lc_event Output FList:", o_flistp);
	

	acct_pdp      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	srvc_pdp      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO,  1, ebufp );

	device_id     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_DEVICE_ID,  1, ebufp );
	device_sl_no  = PIN_FLIST_FLD_GET(o_flistp, MSO_FLD_SERIAL_NO,  1, ebufp );
	device_pdp    = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_POID, 1, ebufp);
	if (device_pdp)
	{
		memset(device_type, '\0', strlen(device_type)+1);
		//poid_type = (char*)PIN_POID_GET_TYPE(device_pdp);
		fm_mso_get_device_type_from_poid(ctxp, device_pdp, device_type, ebufp);
	}

	if (strstr(device_type, "/device/ip"))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning due to device type is IP");
		return;
	}


	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp && !agreement_no )
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}


	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp= PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_DEALLOCATE_HARDWARE, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
	PIN_FLIST_FLD_COPY(o_flistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	
	get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, device_type, device_sl_no, device_id, agreement_no);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_device_diassociate_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
		msg, action1, event1, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	return;
}




void
fm_mso_modify_csr_ar_limit_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*old_data_array = NULL;
	pin_flist_t		*new_data_array = NULL;

	poid_t			*acct_pdp = NULL;
		
	int64			db = 1;

	char			*event = "/event/activity/mso_lifecycle/change_ar_limit";

	char			*action_name_add = "MODIFY_AR_LIMIT_BUS_USER";
	char			*lc_template = NULL;
	char			*prog_name             = NULL;
	char			*account_no      = NULL;
	char			msg[255]="";
	
	int			string_id = ID_MODIFY_CUSTOMER_AR_LIMIT_BUS_USER;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_csr_ar_limit_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_csr_ar_limit_lc_event Output FList:", o_flistp);
	
	acct_pdp      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );



	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CHANGE_AR_LIMIT, 0, ebufp);
	lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CHANGE_AR_LIMIT, 1, ebufp);

//	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

	old_data_array = PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
	PIN_FLIST_FLD_COPY(old_data_array, MSO_FLD_ADJ_CURRENT_VALUE, lc_temp_flistp_old, MSO_FLD_ADJ_CURRENT_VALUE, ebufp);
	PIN_FLIST_FLD_COPY(old_data_array, MSO_FLD_ADJ_LIMIT,         lc_temp_flistp_old, MSO_FLD_ADJ_LIMIT, ebufp);
	PIN_FLIST_FLD_COPY(old_data_array, MSO_FLD_CURRENT_MONTH,     lc_temp_flistp_old, MSO_FLD_CURRENT_MONTH, ebufp);
	PIN_FLIST_FLD_COPY(old_data_array, MSO_FLD_MONTHLY_ADJ_LIMIT, lc_temp_flistp_old, MSO_FLD_MONTHLY_ADJ_LIMIT, ebufp);
	
	new_data_array = PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
	PIN_FLIST_FLD_COPY(new_data_array, MSO_FLD_ADJ_CURRENT_VALUE, lc_temp_flistp_new, MSO_FLD_ADJ_CURRENT_VALUE, ebufp);
	PIN_FLIST_FLD_COPY(new_data_array, MSO_FLD_ADJ_LIMIT,         lc_temp_flistp_new, MSO_FLD_ADJ_LIMIT, ebufp);
	PIN_FLIST_FLD_COPY(new_data_array, MSO_FLD_CURRENT_MONTH,     lc_temp_flistp_new, MSO_FLD_CURRENT_MONTH, ebufp);
	PIN_FLIST_FLD_COPY(new_data_array, MSO_FLD_MONTHLY_ADJ_LIMIT, lc_temp_flistp_new, MSO_FLD_MONTHLY_ADJ_LIMIT, ebufp);


	sprintf(msg, lc_template, account_no);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_modify_csr_ar_limit_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action_name_add, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	return;
}


void
mso_op_prov_order_resubmit_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*old_data_array = NULL;
	pin_flist_t		*new_data_array = NULL;

	poid_t			*acct_pdp = NULL;
		
	int64			db = 1;

	char			*event = "/event/activity/mso_lifecycle/order_resubmit";

	char			*action_name_add = "PROV_ORDER_RESUBMIT";
	char			*lc_template = NULL;
	char			*prog_name   = NULL;
	char			*account_no  = NULL;
	char			*order_id     = NULL;
	char			msg[255]="";
	
	int			string_id = ID_PROV_ORDER_RESUBMIT;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_prov_order_resubmit_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_prov_order_resubmit_lc_event Output FList:", o_flistp);
	
	acct_pdp      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
	order_id      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ORDER_ID, 1, ebufp );



	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_ORDERS, 0, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ORDER_ID, lc_temp_flistp_new, PIN_FLD_ORDER_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_PROV_ORDER_OBJ, lc_temp_flistp_new, MSO_FLD_PROV_ORDER_OBJ, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

	sprintf(msg, lc_template, order_id, account_no);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_prov_order_resubmit_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action_name_add, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	return;
}

void
mso_op_cust_create_schedule_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*old_data_array = NULL;
	pin_flist_t		*new_data_array = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*schedule_obj = NULL;
		
	int64			db = 1;
	int32			*caller = NULL;

	char			*event = "/event/activity/mso_lifecycle/create_schedule";

	char			*action_name_cre = "CREATE_SCHEDULE_ACTION";
	char			*action_name_mod = "MODIFY_SCHEDULE_ACTION";
	char			*lc_template = NULL;
	char			*prog_name   = NULL;
	char			*account_no  = NULL;
	char			*action_name     = NULL;
	char			msg[255]="";
	char			when_t_str[64]="";

	time_t			*when_t = NULL;
	
	int			string_id = ID_CREATE_SCHEDULE_ACTION;
	int			string_id1 = ID_MODIFY_SCHEDULE_ACTION;



	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_cust_create_schedule_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_cust_create_schedule_lc_event Output FList:", o_flistp);
	
	acct_pdp      = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );
	action_name   = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION_NAME, 1, ebufp );
	schedule_obj  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SCHEDULE_OBJ, 1, ebufp );
	when_t        = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_WHEN_T, 1, ebufp );
	caller        = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OPCODE, 1, ebufp );

	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	if (when_t)
	{
		fm_mso_convert_epoch_to_date(*when_t, when_t_str );
	}

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_SCHEDULE_MODIFY, 0, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACTION_NAME, lc_temp_flistp_new, PIN_FLD_ACTION_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SCHEDULE_OBJ, lc_temp_flistp_new, PIN_FLD_SCHEDULE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	
	if (caller && *caller == MSO_OP_CUST_CREATE_SCHEDULE)
	{
		get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, action_name, when_t_str, account_no);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_cust_create_schedule_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
			msg, action_name_cre, event, lc_iflistp, ebufp);
	}
	else if (caller && *caller == MSO_OP_CUST_MODIFY_SCHEDULE)
	{
		get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, action_name, when_t_str, account_no);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_cust_create_schedule_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
			msg, action_name_mod, event, lc_iflistp, ebufp);	
	}

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	return;
}

void
mso_op_dispute_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*s_oflistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp     = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*ro_flistp      = NULL;
	pin_flist_t		*res_flistp     = NULL;
	pin_flist_t		*evt_flistp     = NULL;
	pin_flist_t		*temp_flistp    = NULL;
	pin_flist_t		*acnt_info      = NULL;

	poid_t			*user_id  = NULL;
	poid_t			*ip_pdp   = NULL;
	poid_t			*acct_pdp = NULL;

	pin_decimal_t		*amount   = NULL;
	
	char			*amount_str = NULL;
	char			msg[255]="";
	char			*event       = "/event/activity/mso_lifecycle/dispute";
	char			*disp_name   = NULL;
	char			*descr       = NULL;
	char			*account_no  = NULL;
	char			*lc_template = NULL;
	char			*action_name = "DISPUTE";
	char			*prog_name = NULL;

	int			string_id    = ID_DISPUTE;

	int64			db = 1;



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_dispute_gen_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_dispute_gen_lc_event Output FList:", s_oflistp);

	disp_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 1, ebufp);
	descr         = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR,1, ebufp);
	prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_DISPUTE, 0, ebufp);

	ip_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
	if (disp_name && strcmp(disp_name, EVENT_DISPUTE) !=0)
	{
		fm_mso_utils_read_any_object(ctxp, ip_pdp, &ro_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error in fm_mso_utils_read_any_object", ebufp);
			PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
			return;
		}
		acct_pdp = PIN_FLIST_FLD_GET(ro_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
		if(disp_name && strcmp(disp_name,BILL_DISPUTE) ==0)
		{
			res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			if(res_flistp)
				PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_ITEM_OBJ, ebufp);
		}
		else if(disp_name && strcmp(disp_name,ITEM_DISPUTE) ==0)
		{
			//res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			//if(res_flistp)
			PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_ITEM_OBJ, ebufp);
		}
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, lc_temp_flistp, PIN_FLD_AMOUNT, ebufp);
		amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp);
	}
	else
	{
		acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
		evt_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, 1, ebufp);
		//if(evt_flistp)
		PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_ITEM_OBJ, lc_temp_flistp, PIN_FLD_ITEM_OBJ, ebufp);

		temp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DISPUTES, PIN_ELEMID_ANY, 0, ebufp );
		PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_AMOUNT, lc_temp_flistp, PIN_FLD_AMOUNT, ebufp);
		amount = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_AMOUNT, 1, ebufp);
	}
	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	if (amount)
	{
		amount_str = pbo_decimal_to_str(amount, ebufp);
	}


	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, amount_str, account_no);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_gen_event Event FList:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	if (amount_str)
	{
		free(amount_str);
	}

	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
	return;
}



void
mso_op_dispute_settlement_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *s_oflistp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *ro_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *evt_flistp = NULL;
	pin_flist_t             *temp_flistp = NULL;
	pin_flist_t		*acnt_info      = NULL;

	pin_decimal_t		*amount   = NULL;

	poid_t                  *user_id = NULL;
	poid_t                  *ip_pdp = NULL;
	poid_t                  *acct_pdp = NULL;

	char                    *event = "/event/activity/mso_lifecycle/settlement";
	char			*amount_str = NULL;
	char			msg[255]="";
	char			*disp_name   = NULL;
	char			*descr       = NULL;
	char			*account_no  = NULL;
	char			*lc_template = NULL;
	char			*action_name = "SETTLEMENT_OF_DISPUTE";
	char			*prog_name   = NULL;

	int			string_id    = ID_SETTLEMENT_OF_DISPUTE;

	int64			db = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_settlement_gen_lc_event Input FList:", i_flistp);
 	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_dispute_settlement_gen_lc_event Output FList:", s_oflistp);

	disp_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 1, ebufp);
	descr         = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR,1, ebufp);
	prog_name     = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );


	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_SETTLEMENT, 0, ebufp);

	ip_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
	if (ip_pdp)
	{
		fm_mso_utils_read_any_object(ctxp, ip_pdp, &ro_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Error in fm_mso_utils_read_any_object", ebufp);
			PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
				return;
		}
		acct_pdp = PIN_FLIST_FLD_GET(ro_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
	}
	
	
	if (strcmp(disp_name,EVENT_SETTLEMENT) !=0)
	{
		if(strcmp(disp_name,BILL_SETTLEMENT) ==0)
		{
			res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			if(res_flistp)
				PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID , lc_temp_flistp, PIN_FLD_ITEM_OBJ, ebufp);
		}
		else if(strcmp(disp_name,ITEM_SETTLEMENT) ==0)
		{
			//res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			//if(res_flistp)
			PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_ITEM_OBJ, ebufp);
		}
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, lc_temp_flistp, PIN_FLD_AMOUNT, ebufp);
		amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp);
	}
	else
	{
		acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
		evt_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, 1, ebufp);
		//if(evt_flistp)
		PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_ITEM_OBJ, lc_temp_flistp, PIN_FLD_ITEM_OBJ, ebufp);

		temp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ADJUSTMENT_INFO, PIN_ELEMID_ANY, 0, ebufp );
		PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_AMOUNT, lc_temp_flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);
		amount = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_AMOUNT, 1, ebufp);
	}

	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	if (amount)
	{
		amount_str = pbo_decimal_to_str(amount, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, amount_str, account_no);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_gen_event Event FList:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	if (amount_str)
	{
		free(amount_str);
	}

	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
	return;
}


void
mso_op_auto_renew_plan_gen_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*plan_info = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*plan_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_RENEW_POSTPAID_AUTOMATIC;
	int32			elem_id = 0;

	char			*event = "/event/activity/mso_lifecycle/auto_renew_plan";
	char			*action_name = "AUTOMATIC_RENEW_POSTPAID";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name    = NULL;
	char			*reason       = NULL;
	char			*account_no   = NULL;
	char			*agreement_no = NULL;
	char			*plan_code    = NULL;

	pin_cookie_t		cookie = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_auto_renew_plan_gen_lc_event", in_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	plan_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (service_pdp)
	{
		fm_mso_get_srvc_info(ctxp, in_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}
	if (plan_pdp)
	{
		fm_mso_cust_get_plan_code(ctxp, in_flistp, &plan_info, ebufp );
		plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp); 
	}
	db = PIN_POID_GET_DB(acc_pdp);
	
	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_AUTO_RENEW_PLAN, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp);

	if ( strcmp(PIN_POID_GET_TYPE(service_pdp), "/service/catv") ==0 )
	{
		strcpy(service_type, "CATV");
	}
	else
	{
		strcpy(service_type, "BB");
	}
	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg,lc_template, plan_code, service_type, agreement_no, account_no );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_auto_renew_plan_gen_lc_event gen_lifecycle_evt flist:", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


	if (lc_template)
	{
		free(lc_template);
	}


	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	PIN_FLIST_DESTROY_EX(&plan_info, NULL);
	return;
}



void
mso_op_cust_add_remove_ip_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;

	poid_t			*acct_pdp   = NULL;
	poid_t			*user_id    = NULL;
	poid_t			*device_pdp = NULL;
	poid_t			*srvc_pdp   = NULL;
		
	int64			db = 1;

	int32			*caller = NULL ;

	char			*event1 = "/event/activity/mso_lifecycle/add_ip";
	char			*event2 = "/event/activity/mso_lifecycle/remove_ip";

	char			*action1= "ADD_IP";
	char			*action2= "REMOVE_IP";
	char			*lc_template  = NULL;
	char			*prog_name    = NULL;
	char			*account_no   = NULL;
	char			*agreement_no = NULL;
	char			*device_id      = NULL;
	char			*device_sl_no = NULL;
	char			msg[255]      ="";
	char			device_type[64] = "";
	char			*action_add_remove = NULL;
	
	int			string_id1    = ID_ADD_IP;
	int			string_id2    = ID_REMOVE_IP;

	void			*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_cust_add_remove_ip_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_cust_add_remove_ip_lc_event Output FList:", o_flistp);
	

	acct_pdp          = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
	srvc_pdp          = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ,  1, ebufp );
	prog_name         = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
	account_no        = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO,  1, ebufp );
	action_add_remove = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION,  1, ebufp );

	device_id     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_DEVICE_ID,  1, ebufp );
	device_sl_no  = PIN_FLIST_FLD_GET(o_flistp, MSO_FLD_SERIAL_NO,  1, ebufp );
	device_pdp    = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_POID, 1, ebufp);
	if (device_pdp)
	{
		memset(device_type, '\0', strlen(device_type)+1);
		//poid_type = (char*)PIN_POID_GET_TYPE(device_pdp);
		fm_mso_get_device_type_from_poid(ctxp, device_pdp, device_type, ebufp);
	}


	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (srvc_pdp && !agreement_no )
	{
		fm_mso_get_srvc_info(ctxp, i_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}

	if (action_add_remove && strcmp(action_add_remove, "ASSOCIATION") == 0 )
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp= PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_ADD_IP, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
 		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
		PIN_FLIST_FLD_COPY(o_flistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
		
		get_evt_lifecycle_template(ctxp, db, string_id1, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, device_type, device_sl_no, device_id, agreement_no);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_cust_add_remove_ip_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action1, event1, lc_iflistp, ebufp);
	}
	else if (action_add_remove && strcmp(action_add_remove, "DISASSOCIATION") == 0)
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp= PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_REMOVE_IP, 0, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
 		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
		PIN_FLIST_FLD_COPY(o_flistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
		
		get_evt_lifecycle_template(ctxp, db, string_id2, 1, &lc_template, ebufp);
		sprintf(msg, lc_template, device_type, device_sl_no, device_id, agreement_no);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_cust_add_remove_ip_lc_event Flist:", lc_iflistp);

		fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, srvc_pdp, prog_name,
			msg, action2, event2, lc_iflistp, ebufp);
	}

	if (lc_template)
	{
		free(lc_template);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL) ;
	return;
}


void
fm_mso_cust_fup_topup_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;
	pin_flist_t		*plan_info = NULL;
	pin_flist_t		*mso_cred_limit_info = NULL;
	pin_flist_t		*credit_profile = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*plan_pdp = NULL;

	pin_decimal_t		*decimal_zero = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*credit_limit = NULL;
	pin_decimal_t		*credit_floor = NULL;
	pin_decimal_t		*amount_mb = NULL;
	
	int64			db = 0;
	int			string_id = ID_FUP_TOPUP;
	int32			elem_id = 0;
	int32			*bill_when = NULL;
	int32			*cfg_bill_when = NULL;

	char			*event = "/event/activity/mso_lifecycle/fup_topup";
	char			*action_name = "FUP_TOPUP";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name = NULL;
	char			*reason = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*plan_code = NULL;
	char			*plan_name = NULL;
	char			*account_city = NULL;
	char			*cfg_city = NULL;
	char			*amount_mb_str = NULL;

	void			*vp = NULL;

	pin_cookie_t		cookie = NULL;




	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_fup_topup_lc_event", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_fup_topup_lc_event", out_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, 1, ebufp );
		account_city = PIN_FLIST_FLD_GET(vp, PIN_FLD_CITY, 1, ebufp);
	}
	db = PIN_POID_GET_DB(acc_pdp);
	
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	if (service_pdp /*&& !agreement_no*/)
	{
		fm_mso_get_srvc_info(ctxp, in_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
		bill_when  =   PIN_FLIST_FLD_GET(srvc_info, PIN_FLD_BILL_WHEN, 1, ebufp);
	}

	plans = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, 0, 1, ebufp);
	if (plans)
	{
		plan_pdp = PIN_FLIST_FLD_GET(plans, PIN_FLD_PLAN_OBJ, 1, ebufp);
		if (plan_pdp)
		{
			fm_mso_cust_get_plan_code(ctxp, plans, &plan_info, ebufp );
			if (plan_info)
			{
				plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp); 
				plan_name = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_NAME, 1, ebufp);
				get_mso_cfg_credit_limit(ctxp, plan_info, &mso_cred_limit_info, ebufp);
			}
		}
	}

	while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(mso_cred_limit_info, MSO_FLD_CITIES,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		cfg_city = PIN_FLIST_FLD_GET(vp, PIN_FLD_CITY, 1, ebufp);
		cfg_bill_when = PIN_FLIST_FLD_GET(vp, PIN_FLD_BILL_WHEN, 1, ebufp);
		if (account_city && cfg_city && strcmp(account_city,cfg_city)==0 &&
		    bill_when && cfg_bill_when && *bill_when == *cfg_bill_when
		   )
		{
			credit_profile = PIN_FLIST_ELEM_GET(vp, MSO_FLD_CREDIT_PROFILE, MSO_FUP_TRACK, 1, ebufp);
			if (credit_profile)
			{
				credit_limit = PIN_FLIST_FLD_GET(credit_profile, PIN_FLD_CREDIT_LIMIT, 1, ebufp);
				credit_floor = PIN_FLIST_FLD_GET(credit_profile, PIN_FLD_CREDIT_FLOOR, 1, ebufp);

				if (credit_limit && credit_floor && pbo_decimal_compare(credit_limit, decimal_zero, ebufp) !=1 )
				{
					amount_mb = pbo_decimal_subtract(credit_limit, credit_floor, ebufp );
					if (amount_mb)
					{
						amount_mb_str = pbo_decimal_to_str(amount_mb, ebufp);
					}
				}
				else
				{
					amount_mb_str = strdup("Unlimited");
				}
			}
		}
	}


	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, plan_code, "BB", agreement_no, account_no, amount_mb_str );
	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_FUP_TOPUP, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_SUBSCRIPTION_PLAN_OBJ, lc_temp_flistp, MSO_FLD_SUBSCRIPTION_PLAN_OBJ, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, amount_mb, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_fup_topup_lc_event gen_lifecycle_evt flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


	if (lc_template)
	{
		free(lc_template);
	}
	if (amount_mb_str)
	{
		free(amount_mb);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	PIN_FLIST_DESTROY_EX(&plan_info, NULL);	
	PIN_FLIST_DESTROY_EX(&mso_cred_limit_info, NULL);
	return;
}


void
fm_mso_cust_add_additional_mb_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*plans = NULL;
	pin_flist_t		*plan_info = NULL;
	pin_flist_t		*mso_cred_limit_info = NULL;
	pin_flist_t		*credit_profile = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*plan_pdp = NULL;

	pin_decimal_t		*decimal_zero = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*credit_limit = NULL;
	pin_decimal_t		*credit_floor = NULL;
	pin_decimal_t		*amount_mb = NULL;
	
	int64			db = 0;
	int			string_id = ID_ADDITONAL_MB;
	int32			elem_id = 0;
	int32			*bill_when = NULL;
	int32			*cfg_bill_when = NULL;

	char			*event = "/event/activity/mso_lifecycle/add_extra_usage";
	char			*action_name = "ADD_ADDITONAL_MB";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name = NULL;
	char			*reason = NULL;
	char			*account_no = NULL;
	char			*agreement_no = NULL;
	char			*plan_code = NULL;
	char			*plan_name = NULL;
	char			*account_city = NULL;
	char			*cfg_city = NULL;
	char			*amount_mb_str = NULL;

	void			*vp = NULL;

	pin_cookie_t		cookie = NULL;




	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error before fm_mso_cust_add_additional_mb_lc_event");
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_additional_mb_lc_event", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_additional_mb_lc_event", out_flistp );
	/* Get Lifecycle event template */

	acc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, NAMEINFO_INSTALLATION, 1, ebufp );
		account_city = PIN_FLIST_FLD_GET(vp, PIN_FLD_CITY, 1, ebufp);
	}
	db = PIN_POID_GET_DB(acc_pdp);
	
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	if (service_pdp /*&& !agreement_no*/)
	{
		fm_mso_get_srvc_info(ctxp, in_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
		bill_when    = PIN_FLIST_FLD_GET(srvc_info, PIN_FLD_BILL_WHEN, 1, ebufp);
	}

	plans = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, 0, 1, ebufp);
	if (plans)
	{
		plan_pdp = PIN_FLIST_FLD_GET(plans, PIN_FLD_PLAN_OBJ, 1, ebufp);
		if (plan_pdp)
		{
			fm_mso_cust_get_plan_code(ctxp, plans, &plan_info, ebufp );
			if (plan_info)
			{
				plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp); 
				plan_name = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_NAME, 1, ebufp);
				get_mso_cfg_credit_limit(ctxp, plan_info, &mso_cred_limit_info, ebufp);
			}
		}
	}

	while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(mso_cred_limit_info, MSO_FLD_CITIES,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		cfg_city = PIN_FLIST_FLD_GET(vp, PIN_FLD_CITY, 1, ebufp);
		cfg_bill_when    = PIN_FLIST_FLD_GET(vp, PIN_FLD_BILL_WHEN, 1, ebufp);
		if (account_city && cfg_city && strcmp(account_city,cfg_city)==0 &&
		    bill_when && cfg_bill_when && *bill_when == *bill_when
		 )
		{
			credit_profile = PIN_FLIST_ELEM_GET(vp, MSO_FLD_CREDIT_PROFILE, MSO_FREE_MB, 1, ebufp);
			if (credit_profile)
			{
				credit_limit = PIN_FLIST_FLD_GET(credit_profile, PIN_FLD_CREDIT_LIMIT, 1, ebufp);
				credit_floor = PIN_FLIST_FLD_GET(credit_profile, PIN_FLD_CREDIT_FLOOR, 1, ebufp);

				if (credit_limit && credit_floor && pbo_decimal_compare(credit_limit, decimal_zero, ebufp) !=1 )
				{
					amount_mb = pbo_decimal_subtract(credit_limit, credit_floor, ebufp );
					if (amount_mb)
					{
						amount_mb_str = pbo_decimal_to_str(amount_mb, ebufp);
					}
				}
				else
				{
					amount_mb_str = strdup("Unlimited");
				}
			}
		}
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg, lc_template, plan_code, agreement_no, account_no, amount_mb_str );
	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_ADD_EXTRA_USAGE, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_SUBSCRIPTION_PLAN_OBJ, lc_temp_flistp, MSO_FLD_SUBSCRIPTION_PLAN_OBJ, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, amount_mb, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_add_additional_mb_lc_event gen_lifecycle_evt flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


	if (lc_template)
	{
		free(lc_template);
	}
	if (amount_mb_str)
	{
		free(amount_mb);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	PIN_FLIST_DESTROY_EX(&plan_info, NULL);	
	PIN_FLIST_DESTROY_EX(&mso_cred_limit_info, NULL);
	return;
}



void
fm_mso_cust_extend_validity_mb_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*plan_info = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*plan_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_VALIDITY_EXTENTION_FOR_A_SERVICE_PLAN;
	int32			elem_id = 0;

	char			*event = "/event/activity/mso_lifecycle/validity_extension";
	char			*action_name = "VALIDITY_EXTENTION_FOR_A_SERVICE_PLAN";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name    = NULL;
	char			*reason       = NULL;
	char			*account_no   = NULL;
	char			*agreement_no = NULL;
	char			*plan_code    = NULL;
	char			old_end_t_str[32]="";
	char			new_end_t_str[32]="";

	pin_cookie_t		cookie = NULL;

	time_t			*new_end_t = NULL;
	time_t			*old_end_t = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity_mb_lc_event", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity_mb_lc_event", out_flistp );
	/* Get Lifecycle event template */

	acc_pdp      = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	service_pdp  = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	reason       = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_REASON_CODE, 1, ebufp);
	prog_name    = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	new_end_t    = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_END_T, 1, ebufp);
	old_end_t    = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_OLD_VALIDITY_END_T, 1, ebufp);

	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (service_pdp && !agreement_no)
	{
		fm_mso_get_srvc_info(ctxp, in_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}
	if(prog_name && strstr(prog_name, "banner")){
		plan_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		fm_mso_utils_read_any_object(ctxp, plan_pdp, &plan_info, ebufp);
		if (plan_info)
		{
			plan_pdp  = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_POID, 1, ebufp); 
			plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp); 
		}
	}
	else{
		get_base_plan_from_mso_purchased_plan(ctxp, in_flistp, &plan_info, ebufp);
		if (plan_info)
		{
			plan_pdp  = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_POID, 1, ebufp); 
			plan_code = PIN_FLIST_FLD_GET(plan_info, PIN_FLD_CODE, 1, ebufp); 
		}
	}

	db = PIN_POID_GET_DB(acc_pdp);
	
	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_EXTEND_PLAN_VALIDITY, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_CODE, plan_code, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_OLD_VALIDITY_END_T, old_end_t, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_END_T, new_end_t, ebufp);

	if ( old_end_t )
	{
		fm_mso_convert_epoch_to_date(*old_end_t, old_end_t_str);
	}
	if ( new_end_t )
	{
		fm_mso_convert_epoch_to_date(*new_end_t, new_end_t_str);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg,lc_template, old_end_t_str, new_end_t_str, plan_code, agreement_no, account_no );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_extend_validity_mb_lc_event gen_lifecycle_evt flist:", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


	if (lc_template)
	{
		free(lc_template);
	}


	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	PIN_FLIST_DESTROY_EX(&plan_info, NULL);
	return;
}


void
fm_mso_resub_susp_evt_mb_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*plan_info = NULL;

	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*event_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_RESUBMIT_SUSPENDED_EVENT;
	int32			elem_id = 0;

	char			*event = "/event/activity/mso_lifecycle/resubmit_suspend_usage";
	char			*action_name = "RESUBMIT_SUSPENDED_EVENT";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			*prog_name    = NULL;
	char			*reason       = NULL;
	char			*account_no   = NULL;
	char			*agreement_no = NULL;
	char			*auth_id      = NULL;
	char			*user_name    = NULL;

	pin_cookie_t		cookie = NULL;

	time_t			*new_end_t = NULL;
	time_t			*old_end_t = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_resub_susp_evt_mb_lc_event", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_resub_susp_evt_mb_lc_event", out_flistp );
	/* Get Lifecycle event template */

	acc_pdp      = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(out_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	service_pdp  = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	prog_name    = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	event_pdp    = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	auth_id      = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_AUTHORIZATION_ID, 1, ebufp);
	user_name    = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_USER_NAME, 1, ebufp);

	if (acc_pdp)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}
	if (service_pdp && !agreement_no)
	{
		fm_mso_get_srvc_info(ctxp, in_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}

	db = PIN_POID_GET_DB(acc_pdp);
	
	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_RESUBMIT_SUSP_USAGE, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ,  acc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_EVENT_OBJ,    event_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_USER_NAME,    user_name, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AUTHORIZATION_ID, auth_id, ebufp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg,lc_template, auth_id, agreement_no, account_no );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_resub_susp_evt_mb_lc_event gen_lifecycle_evt flist:", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


	if (lc_template)
	{
		free(lc_template);
	}


	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	PIN_FLIST_DESTROY_EX(&plan_info, NULL);
	return;
}



void
fm_mso_update_bu_profile_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp_old = NULL;
	pin_flist_t		*lc_temp_flistp_new = NULL;
	pin_flist_t		*acnt_info = NULL;

	poid_t			*acct_pdp = NULL;
		
	int64			db = 1;

	char			*event = "/event/activity/mso_lifecycle/change_profile";

	char			*action_name = "MODIFY_BUSINESS_UNIT_PROFILE_INFO";
	char			*lc_template           = NULL;
	char			*prog_name             = NULL;
	char			*account_no      = NULL;
	char			msg[255]="";
	char			mod_list[255] = "";
	char			*ent_tax_no   = NULL;
	char			*pan_no       = NULL;
	char			*st_reg_no    = NULL;
	char			*vat_tax_no   = NULL;
	
	int			string_id = ID_MODIFY_PROFILE_INFO_FOR_BUSINESS_UNIT;

	int32			elem_id = 0;
	int32			*customer_type        = NULL;
	int32			*pref_dom             = NULL;
	int32			*pp_type              = NULL;
	int32			*erp_control_acct_id  = NULL;
	int32			*commision_model      = NULL;
	int32			*commision_service    = NULL;
	int32			*customer_segment     = NULL;

	void			*vp  = NULL;
	void			*vp1 = NULL;
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bu_profile_lc_event Input FList:", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bu_profile_lc_event Output FList:", o_flistp);
	
	acct_pdp      = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_POID,  1, ebufp );
	prog_name     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );



	if (acct_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp_old = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CHANGE_PROFILE, 0, ebufp);
	vp  = (pin_flist_t*) PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp);
	if (vp)
	{
		vp1 = (pin_flist_t*) PIN_FLIST_SUBSTR_GET(vp, MSO_FLD_WHOLESALE_INFO, 1, ebufp); 

		customer_type         = PIN_FLIST_FLD_GET(vp1, PIN_FLD_CUSTOMER_TYPE       , 1, ebufp );
		pref_dom              = PIN_FLIST_FLD_GET(vp1, MSO_FLD_PREF_DOM            , 1, ebufp );
		pp_type               = PIN_FLIST_FLD_GET(vp1, MSO_FLD_PP_TYPE             , 1, ebufp );
		erp_control_acct_id   = PIN_FLIST_FLD_GET(vp1, MSO_FLD_ERP_CONTROL_ACCT_ID , 1, ebufp );
		ent_tax_no            = PIN_FLIST_FLD_GET(vp1, MSO_FLD_ENT_TAX_NO          , 1, ebufp );
		pan_no                = PIN_FLIST_FLD_GET(vp1, MSO_FLD_PAN_NO              , 1, ebufp );
		st_reg_no             = PIN_FLIST_FLD_GET(vp1, MSO_FLD_ST_REG_NO           , 1, ebufp );
		vat_tax_no            = PIN_FLIST_FLD_GET(vp1, MSO_FLD_VAT_TAX_NO          , 1, ebufp );
		commision_model       = PIN_FLIST_FLD_GET(vp1, MSO_FLD_COMMISION_MODEL     , 1, ebufp );
		commision_service     = PIN_FLIST_FLD_GET(vp1, MSO_FLD_COMMISION_SERVICE   , 1, ebufp );
		customer_segment      = PIN_FLIST_FLD_GET(vp1, PIN_FLD_CUSTOMER_SEGMENT    , 1, ebufp );

		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,mod_list );
		if (customer_type)
		{
			strcpy (mod_list, "CUSTOMER_TYPE");
		}
		if (pref_dom)
		{
		//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,mod_list );
			if( strlen(mod_list) == 0)
			{
				strcpy(mod_list, "PREF_DOM");
			}
			else
			{
				strcat(mod_list, ",PREF_DOM");
			}
		//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,mod_list );
		}
		if (pp_type)
		{
		//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,mod_list );
			if( strlen(mod_list) == 0)
			{
				strcpy(mod_list, "PP_TYPE");
			}
			else
			{
				strcat(mod_list, ",PP_TYPE");
			}
		//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,mod_list );
		}
		if (erp_control_acct_id)
		{
			if( strlen(mod_list) == 0)
			{
				strcpy(mod_list, "ERP_CONTROL_ACCT_ID");
			}
			else
			{
				strcat(mod_list, ",ERP_CONTROL_ACCT_ID");
			}
		}
		if (ent_tax_no)
		{
			if( strlen(mod_list) == 0)
			{
				strcpy(mod_list, "ENT_TAX_NO");
			}
			else
			{
				strcat(mod_list, ",ENT_TAX_NO");
			}
		}
		if (pan_no)
		{
			if( strlen(mod_list) == 0)
			{
				strcpy(mod_list, "PAN_NO");
			}
			else
			{
				strcat(mod_list, ",PAN_NO");
			}
		}
		if (st_reg_no)
		{
			if( strlen(mod_list) == 0)
			{
				strcpy(mod_list, "ST_REG_NO");
			}
			else
			{
				strcat(mod_list, ",ST_REG_NO");
			}
		}
		if (vat_tax_no)
		{
			if( strlen(mod_list) == 0)
			{
				strcpy(mod_list, "VAT_TAX_NO");
			}
			else
			{
				strcat(mod_list, ",VAT_TAX_NO");
			}
		}
		if (commision_model)
		{
			if( strlen(mod_list) == 0)
			{
				strcpy(mod_list, "COMMISION_MODEL");
			}
			else
			{
				strcat(mod_list, ",COMMISION_MODEL");
			}
		}
		if (commision_service)
		{
			if( strlen(mod_list) == 0)
			{
				strcpy(mod_list, "COMMISION_SERVICE");
			}
			else
			{
				strcat(mod_list, ",COMMISION_SERVICE");
			}
		}
		if (customer_segment)
		{
			if( strlen(mod_list) == 0)
			{
				strcpy(mod_list, "SEGMENT_FLAG");
			}
			else
			{
				strcat(mod_list, ",SEGMENT_FLAG");
			}
		}

		if (vp1)
		{
			PIN_FLIST_CONCAT(lc_temp_flistp_old, vp1, ebufp);
		}
	}

	lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CHANGE_PROFILE, 1, ebufp);
	vp  = (pin_flist_t*) PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp);
	if (vp)
	{
		vp1 = (pin_flist_t*)PIN_FLIST_SUBSTR_GET(vp, MSO_FLD_WHOLESALE_INFO, 1, ebufp); 
		if (vp1)
		{
			PIN_FLIST_CONCAT(lc_temp_flistp_new, vp1, ebufp);
		}
	}

	sprintf(msg, lc_template, mod_list, account_no);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_bu_profile_lc_event Flist:", lc_iflistp);

	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL) ;
	 
	return;
}



void
fm_mso_apply_late_fee_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*res_flist = NULL;
	pin_flist_t		*sub_bal_impacts_array = NULL;
	pin_flist_t		*sub_bal_array = NULL;


	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*event_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_LATE_FEE;
	int32			elem_id = 0;

	char			*event = "/event/activity/mso_lifecycle/late_fee";
	char			*action_name = "APPLY_LATE_FEE";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			*prog_name    = NULL;
	char			*reason       = NULL;
	char			*account_no   = NULL;
	char			*agreement_no = NULL;
	char			*auth_id      = NULL;
	char			*user_name    = NULL;
	char			*amount_str   = NULL;

	pin_decimal_t		*amount = NULL;

	pin_cookie_t		cookie = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_late_fee_lc_event input", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_late_fee_lc_event output", out_flistp );
	/* Get Lifecycle event template */

	acc_pdp      = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_POID, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(out_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	account_no   = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	res_flist    = PIN_FLIST_ELEM_GET(out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

	if (res_flist)
	{
		service_pdp  = PIN_FLIST_FLD_GET(res_flist, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		event_pdp    = PIN_FLIST_FLD_GET(res_flist, PIN_FLD_POID, 1, ebufp);
		
		/*while((vp = (pin_flist_t*)PIN_FLIST_ELEM_GET_NEXT(res_flist, PIN_FLD_BAL_IMPACTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			vp1 = PIN_FLIST_FLD_GET((pin_flist_t*)vp, PIN_FLD_AMOUNT, 1, ebufp);
			if (vp1)
			{
				pbo_decimal_add_asign(amount, (pin_decimal_t*)vp1, ebufp);
			}
		}*/
		sub_bal_impacts_array = PIN_FLIST_ELEM_GET(res_flist, PIN_FLD_SUB_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp );
		if (sub_bal_impacts_array)
		{
			sub_bal_array = PIN_FLIST_ELEM_GET(sub_bal_impacts_array, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, 1, ebufp );
			if (sub_bal_array)
			{
				amount = PIN_FLIST_FLD_GET(sub_bal_array, PIN_FLD_AMOUNT, 1, ebufp);
			}
		}

		if (amount)
		{
			amount_str = pbo_decimal_to_str(amount, ebufp);
		}

		if (service_pdp && !agreement_no)
		{
			fm_mso_get_srvc_info(ctxp, res_flist, &srvc_info, ebufp);
			agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
		}
	}

	prog_name    = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	
	if (acc_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}




	db = PIN_POID_GET_DB(acc_pdp);
	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_APPLY_LATE_FEES, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ,  acc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_EVENT_OBJ,    event_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT,       amount, ebufp);


	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg,lc_template, amount_str, agreement_no, account_no );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_apply_late_fee_lc_event gen_lifecycle_evt flist:", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, prog_name,
		msg, action_name, event, lc_iflistp, ebufp);


	if (lc_template)
	{
		free(lc_template);
	}
	if (amount_str)
	{
		free(amount_str);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	return;
}



void
fm_mso_fup_downgrade_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*res_flist = NULL;
	pin_flist_t		*sub_bal_impacts_array = NULL;
	pin_flist_t		*sub_bal_array = NULL;


	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*event_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_FUP_DOWNGRADE;
	int32			elem_id = 0;

	char			*event = "/event/activity/mso_lifecycle/fup_downgrade";
	char			*action_name  = "FUP_DOWNGRADE";
	char			*lc_template  = NULL;
	char			msg[255]      ="";
	char			*prog_name    = NULL;
	char			*reason       = NULL;
	char			*account_no   = NULL;
	char			*agreement_no = NULL;
	char			*auth_id      = NULL;
	char			*user_name    = NULL;
	char			*amount_str   = NULL;

	void			*vp           = NULL;
	void			*vp1          = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_fup_downgrade_lc_event input", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_fup_downgrade_lc_event output", out_flistp );
	/* Get Lifecycle event template */

	acc_pdp      = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp);
	service_pdp  = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	account_no   = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);

	if (service_pdp && !agreement_no)
	{
		fm_mso_get_srvc_info(ctxp, in_flistp, &srvc_info, ebufp);
		agreement_no = PIN_FLIST_FLD_GET(srvc_info, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	}

	if (acc_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}




	db = PIN_POID_GET_DB(acc_pdp);
	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_FUP_DOWNGRADE, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ,  acc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);


	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);
	sprintf(msg,lc_template, agreement_no, account_no );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_fup_downgrade_lc_event gen_lifecycle_evt flist:", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, service_pdp, "Rating",
		msg, action_name, event, lc_iflistp, ebufp);


	if (lc_template)
	{
		free(lc_template);
	}


	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	return;
}

void
fm_mso_ops_add_mod_del_action_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_info = NULL;
	pin_flist_t		*res_flist = NULL;
	pin_flist_t		*sub_bal_impacts_array = NULL;
	pin_flist_t		*sub_bal_array = NULL;
	pin_flist_t		*data_array = NULL;
	pin_flist_t		*lc_info = NULL;


	poid_t			*acc_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*cfg_obj = NULL;
		
	int64			db = 0;
	int			string_id_add = ID_OPERATION_ADD_ACTION;
	int			string_id_mod = ID_OPERATION_MODIFY_ACTION;
	int			string_id_del = ID_OPERATION_DELETE_ACTION;
	int32			elem_id = 0;
	int32			action_mode = 0;
	int32			ops_task = 0;

	char			*event_add = "/event/activity/mso_lifecycle/add_config";
	char			*event_mod = "/event/activity/mso_lifecycle/modify_config";
	char			*event_del = "/event/activity/mso_lifecycle/delete_config";
	char			*action_name_add  = "OPERATION_ADD_ACTION";
	char			*action_name_mod  = "OPERATION_MODIFY_ACTION";
	char			*action_name_del  = "OPERATION_DELETE_ACTION";
	char			*lc_template  = NULL;
	char			msg[255]      ="";
	char			*prog_name    = NULL;
	char			*account_no   = NULL;
	char			*cfg_type   = NULL;
	char			*descr = NULL;

	void			*vp           = NULL;
	void			*vp1          = NULL;



	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ops_add_mod_del_action_lc_event input", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ops_add_mod_del_action_lc_event output", out_flistp );
	/* Get Lifecycle event template */

	acc_pdp      = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_USERID, 1, ebufp);
	account_no   = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	prog_name    = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	cfg_obj      = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_POID, 1, ebufp);
	descr        = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DESCR, 1, ebufp);

	vp           = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACTION_MODE, 1, ebufp);
	if (vp)
	{
		action_mode = *(int32*)vp;
	}

	vp           = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_OPERATION_TASK, 1, ebufp);
	if (vp)
	{
		ops_task = *(int32*)vp;
	}


	if (acc_pdp && !account_no)
	{
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		account_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	
	db = PIN_POID_GET_DB(acc_pdp);

	data_array = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_DATA_ARRAY, PIN_ELEMID_ANY, 1, ebufp);
	lc_info    = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_OPS_CREATE_CONFIG, PIN_ELEMID_ANY, 1, ebufp); 
	if (lc_info)
	{
		cfg_type = PIN_FLIST_FLD_GET(lc_info, PIN_FLD_OBJ_TYPE, 1, ebufp);
	}
	 
	if (action_mode == ADD_CONF_OBJ)
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_OPS_CREATE_CONFIG, 0, ebufp);
//		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ,  acc_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_OBJECT, cfg_obj, ebufp);


		get_evt_lifecycle_template(ctxp, db, string_id_add, 1, &lc_template, ebufp);
		sprintf(msg,lc_template, cfg_type, account_no );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ops_add_action_lc_event gen_lifecycle_evt flist:", lc_iflistp);
		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,
			msg, action_name_add, event_add, lc_iflistp, ebufp);
			
	}

	if (action_mode == UPDATE_CONF_OBJ)
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_OPS_MODIFY_CONFIG, 0, ebufp);
//		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ,  acc_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_OBJECT, cfg_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);


		get_evt_lifecycle_template(ctxp, db, string_id_mod, 1, &lc_template, ebufp);
		sprintf(msg,lc_template, cfg_type, account_no );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ops_update_action_lc_event gen_lifecycle_evt flist:", lc_iflistp);
		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,
			msg, action_name_mod, event_mod, lc_iflistp, ebufp);
			
	}

	if (action_mode == DELETE_CONF_OBJ_FLD)
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_OPS_MODIFY_CONFIG, 0, ebufp);
//		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ,  acc_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_OBJECT, cfg_obj, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);


		get_evt_lifecycle_template(ctxp, db, string_id_mod, 1, &lc_template, ebufp);
		sprintf(msg,lc_template, cfg_type, account_no );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ops_add_action_lc_event gen_lifecycle_evt flist:", lc_iflistp);
		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,
			msg, action_name_mod, event_mod, lc_iflistp, ebufp);
			
	}

	if (action_mode == DELETE_CONF_OBJ)
	{
		lc_iflistp = PIN_FLIST_CREATE(ebufp);
		lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_OPS_DELETE_CONFIG, 0, ebufp);
//		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   account_no, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ,  acc_pdp, ebufp);
		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_OBJECT, cfg_obj, ebufp);
 		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);


		get_evt_lifecycle_template(ctxp, db, string_id_del, 1, &lc_template, ebufp);
		sprintf(msg,lc_template, cfg_type, account_no );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ops_add_action_lc_event gen_lifecycle_evt flist:", lc_iflistp);
		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,
			msg, action_name_del, event_del, lc_iflistp, ebufp);
			
	}


	if (lc_template)
	{
		free(lc_template);
	}


	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_info, NULL);
	return;
}

void
fm_mso_service_tech_change_lc_event(
	pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *o_flistp,
        pin_errbuf_t            *ebufp)
{
	int64                   db = 1;

        char                    *event = "/event/activity/mso_lifecycle/service_technology_change";
	pin_flist_t             *lc_iflistp = NULL;
        pin_flist_t             *lc_temp_flistp_old = NULL;
        pin_flist_t             *lc_temp_flistp_new = NULL;
        pin_flist_t             *old_data_array = NULL;
        pin_flist_t             *new_data_array = NULL;

        poid_t                  *acct_pdp = NULL;

        char                    *action_name_add = "TECHNOLOGY CHANGE";
        char                    *lc_template = NULL;
        char                    *prog_name             = NULL;
        char                    *account_no      = NULL;
        char                    msg[255]="";

        int                     string_id = ID_TECHNOLOGY_CHANGE;
	char            	old_tech[100] = {'\0'};
	char            	new_tech[100] = {'\0'};
	int32			*o_tech = NULL;
	int32                   *n_tech = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                        return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_service_tech_change_lc_event Input FList:", i_flistp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_service_tech_change_lc_event Output FList:", o_flistp);
	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	acct_pdp      = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_OBJ,  1, ebufp );
        prog_name     = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );
        account_no = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp );	
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
        lc_temp_flistp_old = PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_SERVICES, 0, ebufp);
        lc_temp_flistp_new = PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_SERVICES, 1, ebufp);

        PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
        PIN_FLIST_FLD_SET(lc_temp_flistp_new, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
        PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
        PIN_FLIST_FLD_SET(lc_temp_flistp_old, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

        old_data_array = PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_DATA_ARRAY, 0, 1, ebufp );
        PIN_FLIST_FLD_COPY(old_data_array, MSO_FLD_TECHNOLOGY, lc_temp_flistp_old, MSO_FLD_TECHNOLOGY, ebufp);

        new_data_array = PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_DATA_ARRAY, 1, 1, ebufp );
        PIN_FLIST_FLD_COPY(new_data_array, MSO_FLD_TECHNOLOGY, lc_temp_flistp_new, MSO_FLD_TECHNOLOGY, ebufp);
	o_tech = PIN_FLIST_FLD_GET(old_data_array, MSO_FLD_TECHNOLOGY, 0, ebufp);
	n_tech = PIN_FLIST_FLD_GET(lc_temp_flistp_new, MSO_FLD_TECHNOLOGY, 0, ebufp);

	if(o_tech && *o_tech == 1)	
	{
		strcpy(old_tech,"DOCSIS-2");
	}
	if(o_tech && *o_tech == 2)
	{
		strcpy(old_tech,"DOCSIS-3");
	}
	if(o_tech && *o_tech == 3)
	{
		strcpy(old_tech,"ETHERNET");
	}
	if(o_tech && *o_tech == 4)
	{
		strcpy(old_tech,"FIBER");
	}
	if(n_tech && *n_tech == 1)
        {
                strcpy(new_tech,"DOCSIS-2");
        }
        if(n_tech && *n_tech == 2)
        {
                strcpy(new_tech,"DOCSIS-3");
        }
        if(n_tech && *n_tech == 3)
        {
                strcpy(new_tech,"ETHERNET");
        }
        if(n_tech && *n_tech == 4)
        {
                strcpy(new_tech,"FIBER");
        }
        sprintf(msg, lc_template, account_no,old_tech,new_tech);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_service_tech_change_lc_event Flist:", lc_iflistp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	
	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name,
                msg, action_name_add, event, lc_iflistp, ebufp);

	if (lc_template)
        {
                free(lc_template);
        }
        PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
        return;
}

void
fm_mso_cust_transfer_subscr_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_flistp = NULL;
	pin_flist_t		*services_flistp = NULL;
	pin_flist_t		*term_flistp = NULL;
	pin_flist_t		*dvc_flistp = NULL;
	pin_flist_t		*catv_flistp = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*old_acct_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*old_srvc_pdp = NULL;
		
	int64			db = 0;
	int			string_id = ID_SUBSCRIPTION_TRANSFER;

	char			*event = "/event/activity/mso_lifecycle/transfer_subscription";
	char			*action_name = "TRANSFER_SUBSCRIPTION";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name = NULL;
	char			*reason = NULL;
	char			*acct_no = NULL;
	char			*old_acct_no = NULL;
	char			*agreement_no = NULL;
	char			*device_id = NULL;
	char			*vc_id = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_transfer_subscr_lc_event in_flistp", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_transfer_subscr_lc_event out_flistp", out_flistp );
	/* Get Lifecycle event template */

	old_acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp); // OLD ACCT PDP from INPUT flist
	acct_no = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp); // New Account No
	device_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DEVICE_ID, 1, ebufp); // DEVICE ID 
	acct_pdp = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_POID, 1, ebufp); // New ACCT POID from out flistp
	
	prog_name = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if (old_acct_pdp)
	{
		fm_mso_get_account_info(ctxp, old_acct_pdp, &acnt_info, ebufp);
		old_acct_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp); 
	}
	db = PIN_POID_GET_DB(acct_pdp);

	srvc_flistp = PIN_FLIST_SUBSTR_GET(out_flistp, PIN_FLD_SERVICE_INFO, 1, ebufp);
	services_flistp = PIN_FLIST_ELEM_GET(srvc_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, 1, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(services_flistp, PIN_FLD_POID, 1, ebufp);
	term_flistp = PIN_FLIST_ELEM_GET(out_flistp, MSO_FLD_SERVICE_TERMINATE, PIN_ELEMID_ANY, 1, ebufp);
	old_srvc_pdp = PIN_FLIST_FLD_GET(term_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	if( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV)) == 0 )
	{
		strcpy(service_type, "CATV") ;
	}
	catv_flistp = PIN_FLIST_FLD_GET(services_flistp, MSO_FLD_CATV_INFO, 1, ebufp);
	agreement_no = PIN_FLIST_FLD_GET(catv_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);

	dvc_flistp = PIN_FLIST_ELEM_GET(services_flistp, PIN_FLD_DEVICES, 1, 1, ebufp);
	if (dvc_flistp)
	{
		vc_id	= PIN_FLIST_FLD_GET(dvc_flistp, MSO_FLD_VC_ID, 1, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_TRANSFER_SUBSCR, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_POID,  acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_FROM_OBJ,  old_acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ,  service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PREV_SERVICE_OBJ,  old_srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   acct_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_OLD_VALUE,   old_acct_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_AGREEMENT_NO, agreement_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DEVICE_ID, device_id, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_VC_ID, vc_id, ebufp);
	
	sprintf(msg,lc_template, device_id, old_acct_no, acct_no, agreement_no);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_transfer_subscr_lc_event life cycle flistp", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, service_pdp, prog_name,
			msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info,  NULL);
	return;
}

void
fm_mso_cust_swap_tagging_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*srvc_flistp = NULL;
	pin_flist_t		*device_iflistp = NULL;
	pin_flist_t		*device_oflistp = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*old_acct_pdp = NULL;
	poid_t			*service_pdp = NULL;
	poid_t			*child_srvc_pdp = NULL;
		
	int64			db = 0;
	int32			device_search_type = MSO_FLAG_SRCH_BY_SERVICE;
	int			string_id = ID_SWAP_TAGGING;

	char			*event = "/event/activity/mso_lifecycle/swap_tagging";
	char			*action_name = "PARENT_CHILD_STB_TAGGING";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			service_type[50]="";
	char			*prog_name = "STB Swap tagging";
	char			*reason = NULL;
	char			*acct_no = NULL;
	char			*old_acct_no = NULL;
	char			*agreement_no = NULL;
	char			*device_id = NULL;
	char			*old_parent_dvc_id = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_tagging_lc_event in_flistp", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_tagging_lc_event out_flistp", out_flistp );

	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 1, ebufp); // OLD ACCT PDP from INPUT flist
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp); // Parent SERVICE details
	device_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DEVICE_ID, 1, ebufp); // Child STB ID 
	child_srvc_pdp = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_OBJECT, 1, ebufp); // Child SERVICE object 
	
	prog_name = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		acct_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp); 
	}
	db = PIN_POID_GET_DB(acct_pdp);

	device_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(device_iflistp, PIN_FLD_POID, acct_pdp, ebufp );
	PIN_FLIST_FLD_SET(device_iflistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp );
	PIN_FLIST_FLD_SET(device_iflistp, PIN_FLD_SEARCH_TYPE, &device_search_type, ebufp );

	fm_mso_get_device_info(ctxp, device_iflistp, &device_oflistp, ebufp );
	PIN_FLIST_DESTROY_EX(&device_iflistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Error in getting the STB details", ebufp);
		PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
		return;
	}
	if(device_oflistp){
		old_parent_dvc_id = PIN_FLIST_FLD_GET(device_oflistp, PIN_FLD_DEVICE_ID, 1, ebufp);
	}

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_SWAP_TAGGING, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_POID,  acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   acct_no, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_OLD_VALUE,   old_parent_dvc_id, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NEW_VALUE, device_id, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_PREV_SERVICE_OBJ, service_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_SERVICE_OBJ, child_srvc_pdp, ebufp);
	
	sprintf(msg,lc_template, old_parent_dvc_id, device_id, acct_no );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_tagging_lc_event life cycle flistp", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, child_srvc_pdp, "STB Swap Tagging",
			msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info,  NULL);
	PIN_FLIST_DESTROY_EX(&device_oflistp,  NULL);
	return;
}

void
fm_mso_cust_create_netflix_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *out_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *lc_iflistp = NULL;
        pin_flist_t             *acnt_info = NULL;

        poid_t                  *acct_pdp = NULL;
        poid_t                  *service_pdp = NULL;

        int64                   db = 0;

        char                    *event = "/event/activity/mso_lifecycle/create_netflix_service";
        char                    msg[255]="";
        char                    *action_name = "ACTIVATE_NETFLIX_SERVICE";
        char                    *prog_name = NULL;
        char                    *acct_no = NULL;
        char                    *plan_name = NULL;


        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_netflix_lc_event in_flistp", in_flistp );
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_netflix_lc_event out_flistp", out_flistp );

        acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
        service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
        plan_name = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PLAN_NAME, 1, ebufp);
        prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

        if (acct_pdp)
        {
                fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
                acct_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp);
        }
        db = PIN_POID_GET_DB(acct_pdp);

        lc_iflistp = PIN_FLIST_CREATE(ebufp);
   //PIN_FLIST_FLD_SET(lc_iflistp,MSO_FLD_PLAN_NAME,plan_name,ebufp);

        sprintf(msg,action_name, acct_no, plan_name);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_netflix_lc_event life cycle flistp", lc_iflistp);
        fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, service_pdp, prog_name,
                        msg, action_name, event, lc_iflistp, ebufp);

        PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
        PIN_FLIST_DESTROY_EX(&acnt_info,  NULL);
        return;
}


void
fm_mso_cust_create_offers_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lc_iflistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*service_pdp = NULL;
		
	int64			db = 0;
	int32			*offer_type = NULL;
	int32			*disc_flags = NULL;
	int			string_id = ID_ADD_OFFERS;

	char			*event = "/event/activity/mso_lifecycle/add_offers";
	char			*action_name = "ADD_OFFERS";
	char			*lc_template = NULL;
	char			msg[255]="";
	char			type_str[30]="";
	char			disc_str[30]="";
	char			*prog_name = NULL;
	char			*amount_str = NULL;
	char			*acct_no = NULL;
	char			valid_from_str[50] = "";
	char			valid_to_str[50] = "";

	time_t			*valid_from_t = NULL;
	time_t			*valid_to_t = NULL;

	pin_decimal_t		*amount = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_offers_lc_event in_flistp", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_offers_lc_event out_flistp", out_flistp );

	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp); 
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	amount = PIN_FLIST_FLD_TAKE(in_flistp, PIN_FLD_AMOUNT, 1, ebufp);
	valid_from_t = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_VALID_FROM, 1, ebufp);
	valid_to_t = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_VALID_TO, 1, ebufp);
	disc_flags = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DISCOUNT_FLAGS, 1, ebufp);
	offer_type = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TYPE, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if(!prog_name){
		prog_name = "ADD_OFFERS";
	}
	
	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
		acct_no = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_ACCOUNT_NO, 1, ebufp); 
	}
	db = PIN_POID_GET_DB(acct_pdp);

	if( offer_type && *offer_type == 1){
		strcpy(type_str, "DISCOUNT");
	}
	else{
		strcpy(type_str, "DATA");
	}

	if(disc_flags && *disc_flags == 1){
		strcpy(disc_str, "PERCENT");
		pbo_decimal_multiply_assign(amount, pbo_decimal_from_str("100", ebufp), ebufp);
	}
	else if(disc_flags && *disc_flags == 2){
		strcpy(disc_str, "AMOUNT");
	}
	else if(offer_type && *offer_type == 2){
		strcpy(disc_str, "QUOTA");
		pbo_decimal_divide_assign(amount, pbo_decimal_from_str("1024", ebufp), ebufp);
	}
	
	if(amount){
		amount_str = pbo_decimal_to_str(pbo_decimal_round(amount,1,ROUND_DOWN, ebufp), ebufp);
		if(offer_type && *offer_type == 2)
			strcat(amount_str, " GB");
	}
	if (valid_from_t)
		strftime( valid_from_str, sizeof(valid_from_str), "%d-%b-%Y",  localtime( (time_t *) valid_from_t) );
	if (valid_to_t)
		strftime( valid_to_str, sizeof(valid_to_str), "%d-%b-%Y",  localtime( (time_t *) valid_to_t) );
	

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_OFFER, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ,  acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_ACCOUNT_NO,   acct_no, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_VALID_FROM, lc_temp_flistp, PIN_FLD_VALID_FROM, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_VALID_TO, lc_temp_flistp, PIN_FLD_VALID_TO, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, lc_temp_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_TYPE, lc_temp_flistp, PIN_FLD_TYPE, ebufp);
	PIN_FLIST_FLD_PUT(lc_temp_flistp, PIN_FLD_AMOUNT, pbo_decimal_round(amount,1,ROUND_DOWN, ebufp), ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DISCOUNT_FLAGS, lc_temp_flistp, PIN_FLD_DISCOUNT_FLAGS, ebufp);
	
	sprintf(msg,lc_template, type_str, disc_str, amount_str, acct_no, valid_from_str, valid_to_str);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_offers_lc_event life cycle flistp", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, service_pdp, prog_name,
			msg, action_name, event, lc_iflistp, ebufp);

	if (lc_template)
	{
		free(lc_template);
	}
	if (amount_str)
	{
		free(amount_str);
	}
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info,  NULL);
	return;
}

void
fm_mso_create_gst_info_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *gst_info_flistp = NULL;
	poid_t                  *acct_pdp = NULL;

	int64                   db = 1;
	int32                    string_id = ID_CREATE_GST_INFO;

	char                    *event = "/event/activity/mso_lifecycle/create_gst_info";
	char                    *action_name = "GST Information Creation";
	char                    msg[255]="";
	char                    *prog_name = "GST Information Creation";
	char                    *acct_no = NULL;
	char			*lc_template = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_gst_info_lc_event in_flistp", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_gst_info_lc_event out_flistp", out_flistp );

	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	acct_no = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	db = PIN_POID_GET_DB(acct_pdp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	gst_info_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_GST_INFO, PIN_ELEMID_ANY, 1, ebufp);
	PIN_FLIST_ELEM_SET(lc_iflistp, gst_info_flistp, MSO_FLD_GST_INFO, 0, ebufp);
	sprintf(msg, lc_template, acct_no );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_tagging_lc_event life cycle flistp", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name, msg, action_name, event, lc_iflistp, ebufp);

	if(lc_template)
	{
		free(lc_template);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	return;
}

void
fm_mso_update_gst_info_lc_event(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*lc_iflistp = NULL;
	pin_flist_t			*gst_info_flistp = NULL;
	pin_flist_t			*acnt_info_flistp = NULL;
	
	poid_t				*acct_pdp = NULL;

	int64				db = 1;
	int32				string_id = ID_UPDATE_GST_INFO;

	char				*event = "/event/activity/mso_lifecycle/update_gst_info";
	char				*action_name = "GST Information updation";
	char				msg[255]="";
	char				*prog_name = "GST Information updation";
	char				*acct_no = NULL;
	char				*lc_template = NULL;
	char				*state = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
					return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_gst_info_lc_event in_flistp", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_gst_info_lc_event out_flistp", out_flistp );
	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	if (acct_pdp)
	{
		fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info_flistp, ebufp);
		acct_no = PIN_FLIST_FLD_GET(acnt_info_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	}

	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	db = PIN_POID_GET_DB(acct_pdp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	gst_info_flistp = PIN_FLIST_ELEM_GET(in_flistp, MSO_FLD_GST_INFO, PIN_ELEMID_ANY, 1, ebufp);
	state = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_STATE, 0, ebufp);
	PIN_FLIST_ELEM_SET(lc_iflistp, out_flistp, MSO_FLD_GST_INFO, 0, ebufp);
	PIN_FLIST_ELEM_SET(lc_iflistp, gst_info_flistp, MSO_FLD_GST_INFO, 1, ebufp);
	sprintf(msg, lc_template, acct_no, state);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_swap_tagging_lc_event life cycle flistp", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name, msg, action_name, event, lc_iflistp, ebufp);

	if(lc_template)
	{
			free(lc_template);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	return;
}

void
fm_mso_update_poi_poa_lc_event(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	*out_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*lc_iflistp = NULL;
	pin_flist_t	*acnt_info_flistp = NULL;

	poid_t		*acct_pdp = NULL;

	int64		db = 1;
	int32		string_id = ID_UPDATE_POI_POA;

	char		*event = "/event/activity/mso_lifecycle/update_poi_poa";
	char		*action_name = "POI/POA Updation";
	char		msg[255]="";
	char		*prog_name = "POI/POA Updation";
	char		*acct_no = NULL;
	char		*lc_template = NULL;
	char		*old_poi = NULL;
	char		*old_poa = NULL;
	char		*new_poi = NULL;
	char		*new_poa = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_poi_poa_lc_event in_flistp", in_flistp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_poi_poa_lc_event out_flistp", out_flistp );
	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	acct_no = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	
	new_poi =  PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCESS_CODE1, 1, ebufp);
	new_poa =  PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCESS_CODE2, 1, ebufp);
	
	old_poi =  PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_ACCESS_CODE1, 1, ebufp);
	old_poa =  PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_ACCESS_CODE2, 1, ebufp);
	
	
	prog_name = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	db = PIN_POID_GET_DB(acct_pdp);

	get_evt_lifecycle_template(ctxp, db, string_id, 1, &lc_template, ebufp);

	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	acnt_info_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_ACCTINFO, 0, ebufp);
	PIN_FLIST_FLD_SET(acnt_info_flistp, PIN_FLD_ACCESS_CODE1, old_poi, ebufp);
	PIN_FLIST_FLD_SET(acnt_info_flistp, PIN_FLD_ACCESS_CODE2, old_poa, ebufp);
	acnt_info_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, PIN_FLD_ACCTINFO, 1, ebufp);
	PIN_FLIST_FLD_SET(acnt_info_flistp, PIN_FLD_ACCESS_CODE1, new_poi, ebufp);
	PIN_FLIST_FLD_SET(acnt_info_flistp, PIN_FLD_ACCESS_CODE2, new_poa, ebufp);
	sprintf(msg, lc_template, acct_no);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_poi_poa_lc_event life cycle flistp", lc_iflistp);
	fm_mso_utils_gen_lifecycle_evt(ctxp, acct_pdp, NULL, prog_name, msg, action_name, event, lc_iflistp, ebufp);

	if(lc_template)
	{
		free(lc_template);
	}

	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	return;
}

void fm_mso_netfr_gen_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
	poid_t			*acc_pdp = NULL;
	pin_flist_t		*lc_flistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	char			*event = "/event/activity/mso_lifecycle/payment_reversal";
	char			*action_name = NETF_RACTION;
	char			msg[255]="";
	char			*prog_name = NF_RPROG_NAME;
	char			*name = "NETF_PAYMENT_REVERSAL";
	char			*descr = "NETFLIX REVERSAL";
	char			*lc_template = NULL;
	int			string_id = ID_NETFLIX_REVERSAL;
	int64                   db = 1;

	    acc_pdp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_ACCOUNT_OBJ,1,ebufp);
	    lc_flistp = PIN_FLIST_CREATE(ebufp);
	    lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_flistp, MSO_FLD_PAYMENT_REVERSALS, 0, ebufp);
	    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, lc_temp_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, lc_temp_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
	    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_RECEIPT_NO, lc_temp_flistp, PIN_FLD_RECEIPT_NO, ebufp);
	    PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);
	    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, lc_temp_flistp, PIN_FLD_AMOUNT, ebufp);
	    PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, name, ebufp);  
	    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, lc_temp_flistp, PIN_FLD_REASON_ID, ebufp);
		get_evt_lifecycle_template(ctxp, db, string_id, 1,&lc_template, ebufp);
		if (lc_template)
		{
			sprintf(msg,lc_template,(char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_RECEIPT_NO, 1, ebufp), pbo_decimal_to_str(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp),ebufp), (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp));
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Lifecycle Creation input flist", lc_flistp);
		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,msg, action_name, event, lc_flistp, ebufp);
		
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
		 	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error lifecycle event creation", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_FLIST_DESTROY_EX(&lc_flistp, NULL);
		return;
}

/*******************************************************************************
 * fm_mso_tag_hybrid_acct_gen_lc_event()
 *
 * Inputs: flist with DESCR, ACCOUNT_NO and ACCOUNT_TAG are mandatory fields 

 * Outputs: void; ebuf set if errors encountered
 *
 * Description:
 * This function creates the Life Cycle Event for Tagging Hybrid Accounts.
 ******************************************************************************/
void
fm_mso_tag_hybrid_acct_gen_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
	poid_t			*acc_pdp = NULL;
	
	pin_flist_t		*lc_flistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	
	char			*event = TAG_HYBRID_ACCTS_EVENT;
	char			*action_name = HYBRID_ACCTS_TAG_ACTION;
	char			msg[255]="";
	char			*prog_name = NULL;
	char			*descr = "Tagging Hybrid Accounts";
	char			*lc_template = NULL;
	
	int			string_id = ID_TAG_HYBRID_ACCT;
	int64           	db = 1;
	
	if (i_flistp)
	{
		acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
		
		if(acc_pdp)
		{
			lc_flistp = PIN_FLIST_CREATE(ebufp);
			lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_flistp, MSO_FLD_ACCOUNT_DETAIL, 0, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, lc_temp_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_TAG, lc_temp_flistp, PIN_FLD_ACCOUNT_TAG, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);
				
			get_evt_lifecycle_template(ctxp, db, string_id, 1,&lc_template, ebufp);
			if (lc_template)
			{
				sprintf(msg,lc_template,(char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp), 
										(char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_TAG, 1, ebufp), descr, 1, ebufp);
										
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Lifecycle Creation input flist for tagging Hybrid Accounts", lc_flistp);
			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name, msg, action_name, event, lc_flistp, ebufp);
			
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in lifecycle event creation for tagging Hybrid Accounts", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&lc_flistp, NULL);
		}
	}
	return;		
}

/*******************************************************************************
 * fm_mso_detag_hybrid_acct_gen_lc_event()
 *
 * Inputs: flist with DESCR, ACCOUNT_NO and ACCOUNT_TAG are mandatory fields 

 * Outputs: void; ebuf set if errors encountered
 *
 * Description:
 * This function creates the Life Cycle Event for De-Tagging Hybrid Accounts.
 ******************************************************************************/
void
fm_mso_detag_hybrid_acct_gen_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
	poid_t			*acc_pdp = NULL;
	
	pin_flist_t		*lc_flistp = NULL;
	pin_flist_t		*lc_temp_flistp = NULL;
	
	char			*event = DETAG_HYBRID_ACCTS_EVENT;
	char			*action_name = HYBRID_ACCTS_DETAG_ACTION;
	char			msg[255]="";
	char			*prog_name = NULL;
	char			*descr = "De-Tagging Hybrid Accounts";
	char			*account_no = NULL;
	char			*lc_template = NULL;
	
	int			string_id = ID_DETAG_HYBRID_ACCT;
	int64           	db = 1;
	
	if (i_flistp)
	{
		acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
		
		if(acc_pdp)
		{
			lc_flistp = PIN_FLIST_CREATE(ebufp);
			lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_flistp, MSO_FLD_ACCOUNT_DETAIL, 0, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, lc_temp_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_TAG, lc_temp_flistp, PIN_FLD_ACCOUNT_TAG, ebufp);
			PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_DESCR, descr, ebufp);
				
			get_evt_lifecycle_template(ctxp, db, string_id, 1,&lc_template, ebufp);
			if (lc_template)
			{
				sprintf(msg,lc_template,(char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp),
						 (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_TAG, 1, ebufp), 1, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}
				
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Lifecycle Creation input flist for de-tagging Hybrid Accounts", lc_flistp);
			fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name, msg, action_name, event, lc_flistp, ebufp);
			
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in lifecycle event creation for de-tagging Hybrid Accounts", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&lc_flistp, NULL);
		}
	}
	return;		
}



