/*******************************************************************
 *
 * @(#)%Portal Version: fm_rate_pol_post_rating.c:IDCmod7.3PatchInt:1:2007-Mar-26 14:15:36 %
 *	
 *      Copyright (c) 1999 - 2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_rate_pol_post_rating.c:IDCmod7.3PatchInt:1:2007-Mar-26 14:15:36 %";
#endif

/*******************************************************************
* Contains the PCM_OP_RATE_POL_POST_RATING operation.
*******************************************************************/

#include <stdio.h> 
#include <string.h> 

#include "pcm.h"
#include "ops/rate.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_act.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "pin_rate.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "mso_ops_flds.h"
#include "mso_prov.h"
#include "mso_glid.h"
#include "pin_currency.h"
#include "ops/bal.h"
#include "mso_rate.h"
#include "mso_ntf.h"
#include "mso_cust.h"
#include "mso_errs.h"
#include "mso_ops_flds.h"
#include "mso_commission.h"
#include "mso_lifecycle_id.h"
#define FILE_SOURCE_ID  "fm_rate_pol_post_rating.c(5)"

#define MSO_ET_MAIN "MSO_ET_MAIN"
#define MSO_ET_ADDI "MSO_ET_ADDI"
#define MSO_VAT "MSO_VAT"
#define MSO_VAT_CATV "MSO_VAT_CATV"
#define MSO_VAT_BB "MSO_VAT_BB"
#define MSO_ST "MSO_Service_Tax"
#define MSO_VAT_ST "MSO_VAT_ST"
#define SERVICE_TAX "Service_Tax"
#define EDUCATION_CESS "Education_Cess"
#define SHE_CESS "Secondary_Higher_Education_Cess"
#define SWACHH_CESS "Swachh_Bharat_Cess"
#define KRISH_CESS "Krish_Kalyan_Cess"
#define ADDITIONAL_CESS "Additional_Cess"
#define SGST "SGST"
#define CGST "CGST"
#define IGST "IGST"
#define bb_type "/service/telco/broadband"
#define MSO_ADJUSTMENT_TAX_CODE "9984"
#define MIN_COMMITMENT_TAX_CODE "9984"
#define PENALTY_TAX_CODE "9984"
#define LATE_FEE_TAX_CODE "9984"
#define CN_DN_TAX_CODE "9984"
#define ET_GST_TAX_CODE "9984"

#define WALLET_RES_ID 2300000
#define REF_RES_ID 2300001

// TOD: TEMP DATA TO BE REMOVED AND USED FROM COMMON HEADERS DURING INTEGRATION
int64	FUP_LIMIT_RESOURCE = 1000104;
int32   FREE_MB_RESOURCE = 1100010;
int64	CREDIT_THRESHOLD_BREACH_FLAG = NTF_BB_DATA_LIMIT;
int64	CREDIT_LIMIT_BREACH_FLAG =	NTF_BB_DATA_LIMIT_EXPIRED;
int64	FUP_LIMIT_THRESHOLD_BREACH_FLAG = NTF_BEFORE_FUP;
int64	FUP_LIMIT_BREACH_FLAG =		NTF_AFTER_FUP;


extern credit_threshold_config_t credit_thresholds[10];
extern int32 num_of_resources_configured;
extern cm_cache_t fm_rate_pol_tax_codes_ptr;

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_rate_pol_post_rating(
	cm_nap_connection_t	*connp,
	u_int				opcode,
	u_int				flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

extern void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);
/*NT: Modified return type from void to int32*/
static int32
mso_remove_custom_balImpact_taxation(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	pin_errbuf_t		*ebufp);
/*NT: Added for updating wallet balance*/
static void
mso_update_wallet_balance(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);

static void
mso_adj_add_custom_balImpact_taxation(
	pcm_context_t		*ctxp,
	cm_nap_connection_t	*connp,
	pin_flist_t		**i_flistpp,
	char			*evt_type,
	pin_errbuf_t		*ebufp);

extern pin_decimal_t *st_tax_rate;
extern pin_decimal_t *ecess_tax_rate;
extern pin_decimal_t *shecess_tax_rate;
extern pin_decimal_t *swachh_tax_rate;
extern pin_decimal_t *krish_tax_rate;

static void
mso_call_prov_cancel_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT void
mso_cancel_alc_addon_pdts(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_errbuf_t            *ebufp);

static void
mso_call_bb_cancel_plan(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_errbuf_t		*ebufp);	

/*static void
mso_check_threshold(
        pcm_context_t                   *ctxp,
        pin_flist_t                             **i_flistpp,
        pin_errbuf_t                    *ebufp);
*/
void mso_fm_check_threshold(
	pin_flist_t		*i_flistp,
	pcm_context_t   *ctxp,
	pin_errbuf_t  	*ebufp);

void
mso_rate_pol_record_cur_bal(
        cm_nap_connection_t     *connp,
        pin_flist_t             *i_flistp,
	char                    *service_typep,
        pin_errbuf_t            *ebufp);

void update_credit_limits(pin_flist_t	*res_flistp,
			pin_flist_t	**credit_profile_flistp,
			pin_errbuf_t  	*ebufp);

extern int32 fm_mso_utils_get_tax_excemption_details(
	pcm_context_t           *ctxp,
	pin_flist_t             *exempt_out_flistp,
	pin_errbuf_t            *ebufp);

static void
fm_rate_pol_post_rating_get_aso_object(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp );

extern int
fm_mso_purch_prod_status (
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

static void
mso_cancel_alc_addon_addconn_pdts(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	poid_t            	 *new_serv_pdp,
	pin_errbuf_t            *ebufp);
/*NT : Added to get customer type*/
PIN_EXPORT int32
fm_mso_get_cust_type(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        pin_errbuf_t            *ebufp);
	
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
		
PIN_IMPORT void  
fm_mso_utils_get_catv_children(
        pcm_context_t           *ctxp,
        poid_t                  *act_pdp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void 
mso_retrieve_gst_rate(
        pcm_context_t   *ctxp,
	char		*tax_code,
        poid_t          *acct_pdp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT void
fm_mso_utils_get_profile(
    pcm_context_t   *ctxp,
    poid_t      *account_pdp,
    pin_flist_t **ret_flistpp,
    pin_errbuf_t    *ebufp);

PIN_EXPORT pin_decimal_t *
get_ar_curr_total(
        pcm_context_t   *ctxp,
        poid_t          *ar_billinfo_pdp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT int32
fm_mso_get_wallet_balance(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	int32			wallet_res_id,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);
			
PIN_EXPORT int32
is_charge_tax_code(
        pcm_context_t   *ctxp,
		char			*tax_code,
        pin_errbuf_t    *ebufp);

static void
fm_check_charge_item_applied_commission(
	pcm_context_t   *ctxp,
	poid_t  *acct_pdp,
	poid_t  *charge_item_obj,
	pin_flist_t     **r_flistpp,
	pin_errbuf_t    *ebufp);

extern int32
fm_mso_get_product_priority(
        pcm_context_t           *ctxp,
        poid_t                  *prod_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_get_poid_details(
        pcm_context_t           *ctxp,
        poid_t                  *poid_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void
fm_mso_find_decimal_entry(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

extern
void fm_mso_search_plan_details(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *i_flistp,
        pin_flist_t                     **o_flistpp,
        pin_errbuf_t                    *ebufp);

static void
fm_rate_pol_transition_remove_taxes(
        pcm_context_t   *ctxp,
        pin_flist_t     **i_flistpp,
	char		*svc_type,
        pin_errbuf_t    *ebufp);

static void
mso_adv_payment_chgs(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
		pin_flist_t     **mso_nrc_paymt_trnsfr_rflistpp,
        char            *svc_type,
        int             *flags,
        pin_errbuf_t    *ebufp);

extern int32
fm_validate_prov_ca_id(
        pcm_context_t   *ctxp,
        poid_t          *srvc_pdp,
        char            *prov_tag,
        pin_errbuf_t   *ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_RATE_POL_POST_RATING operation.
 *******************************************************************/
void
op_rate_pol_post_rating(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*commi_flistp = NULL;
	//pin_flist_t		*action_info_flistp=NULL;
	pin_flist_t		*comm_r_flistp=NULL;
	pin_flist_t		*total_arrayp=NULL;
	pin_flist_t             *total_flistp = NULL;
	pin_flist_t             *initial_flistp = NULL;
	pin_flist_t		*bal_impacts = NULL;
	pin_flist_t		*sub_bal_flistp = NULL;
	pin_flist_t             *bal_flistp = NULL;
	pin_flist_t		*cycle_info_flistp = NULL;
	pin_flist_t             *lcc_iflistp = NULL;
        pin_flist_t             *lcc_temp_flistp = NULL;
        pin_flist_t             *prd_flistp = NULL;
        pin_flist_t             *offr_flistp= NULL;
        pin_flist_t             *read_out_flistp= NULL;
        pin_flist_t             *lcc_deal_flistp = NULL;
        pin_flist_t             *rplan_plan_flistp = NULL;
        pin_flist_t             *lcc_r_flistp = NULL;
	pin_flist_t             *srvc_info_flistp=NULL;
	pin_flist_t             *results_flistp = NULL;
	pin_flist_t             *search_flistp = NULL;
	pin_flist_t             *search_oflistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *prod_flistp = NULL;
	pin_flist_t             *charge_head_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *temp_flistp = NULL;
	pin_flist_t             *ch_res_flistp = NULL;
	pin_flist_t             *ch_sub_flistp = NULL;
	pin_flist_t             *read_item_oflistp = NULL;
	pin_flist_t             *items_r_flistp= NULL;
	pin_flist_t             *ch_head_flistp= NULL;
	pin_flist_t             *offer_flistp= NULL;
	pin_flist_t             *srch_iflistp= NULL;
	pin_flist_t             *srch_oflistp= NULL;
	pin_flist_t             *ntotal_flistp = NULL;
	pin_flist_t		*purch_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t             *event_flistp= NULL;
	pin_flist_t		*orig_evt_flistp = NULL;
	pin_flist_t		*product_flistp = NULL;
	pin_flist_t		*service_flistp = NULL;
	pin_flist_t		*ccprofile_flistp = NULL;
	pin_flist_t		*bbinfo_flistp = NULL;
	pin_flist_t		*cfg_cr_flistp = NULL;

	int32			initial_opcode=0;

	cm_op_info_t    	*opstackp = connp->coip;
	cm_op_info_t		*new_opstackp = connp->coip;

	pin_decimal_t 		*quantity = NULL;
	pin_decimal_t           *amt = NULL;
	pin_decimal_t           *impact_amount = NULL;
	pin_decimal_t		*negative_impact_amount = NULL;
	pin_decimal_t           *disc_amount = NULL;
	pin_decimal_t           *coll_amt = NULL;
	pin_decimal_t           *chg_head_totalp = NULL;
	pin_decimal_t           *total_amt = pbo_decimal_from_str("0", ebufp);
	pin_decimal_t           *final_amt = NULL;
	pin_decimal_t           *qty_applied = NULL;
	pin_decimal_t           *current_data = NULL;
	pin_decimal_t           *addl_data = NULL;
	pin_decimal_t           *total_data = NULL;
	pin_decimal_t		*quota_amt= NULL;
	pin_decimal_t		*quota_perc= NULL;
	pin_cookie_t		sub_bal_cookie = NULL;
	pin_cookie_t            bal_cookie = NULL;

	poid_t			*e_pdp = NULL;
	poid_t			*evt_pdp = NULL;
        poid_t                  *svc_pdp = NULL;
	poid_t			*session_obj = NULL;
	poid_t                  *off_obj = NULL;
	poid_t                  *acct_pdp = NULL;
	poid_t                  *bal_grp_pdp = NULL;
	poid_t                  *offering_pdp = NULL;
	poid_t                  *search_pdp = NULL;
	poid_t                  *charge_item_pdp = NULL;
	poid_t			*offering_obj = NULL;
	poid_t			*plan_obj = NULL;
	poid_t			*prod_obj = NULL;

        char                    *svc_type = NULL;
	char			tmp[256];
	char			tmp_buff[128];
	char			*evt_type = NULL;
	char			*prog_name = NULL;
	char			*sys_descr = NULL;
	char                    *evt_template = "select X from /event where F1 = V1 and F2 = V2 and F3 = V3 and F4 > V4 and F5 = V5 and "
	                                         "event_t.poid_type like '/event/billing/product/fee/cycle/%'";

	char			*city = NULL;
	char			*plan_name = NULL;

	int32			*bill_when = NULL;
	int32			*tax_flags = NULL;
	int32                   *status = NULL;
	int32                   bal_impact_flag = 0;
	int32                   commission_flag = 0;
	int32                   elem_id = 0;
	int32                   *resource_id = NULL;
	int32                   *data_res_id = NULL;
	int32           	comm_succ_flag = 0;
	int32			no_tax_flag = 1;
	int32			stack_opcode = 0;
	int32			act_usage_flag = -1;
	int32			flag_process_commission = 1;
	int32			commission_err_code =0;
	int32			sub_bal_elemid = 0;
	int			resourceid;
	int32                   bal_elemid = 0;
	int32                   res_id = PIN_CURRENCY_INR;
	int32                   commission_rev_flag = 0;
	int32                   err = 0;
	int32                   database = 1;
	int32                   c_elemid = 0;
	int                     flag = 512;
	int                     offer_type = -1;
	int32                   *currency_valp = NULL;
	int32                   *comm_ref_flagp = NULL;
	int                     mod_prty = 0;
	int                     prty = 0;
	int			billterm = 0;
	char                    *tmp_str = NULL ;

	void			*vp =NULL;
	void			*vp1 =NULL;
	time_t			current_t = pin_virtual_time(NULL);
	//time_t			start_t = 0;

	pin_cookie_t            cookie = NULL;
	pin_flist_t		*pr_flistp = NULL;
	int32			count = 0;
	time_t                  earned_end_t = 0;

	pin_cookie_t            c_cookie = NULL;
	pin_decimal_t           *init_amt = NULL;
	pin_decimal_t           *priority = NULL;
    	poid_t			*prod_pdp = NULL;
	pin_flist_t		*prod_array = NULL;
	int32			fup_top = 0;
	pin_flist_t		*ret_flistp = NULL;
	int			*indicator = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	char            	msg[1024];	
	pin_decimal_t		*orig_amount = NULL;
	int32			*billwhen = NULL;
	pin_decimal_t		*payterm = NULL;
	pin_flist_t		*out_flistp = NULL;
	pin_decimal_t		*scale_perc = NULL;
	pin_flist_t		*bal_imp_flistp = NULL;
	char			*tax_code = NULL;
	pin_decimal_t		*round_amount = NULL;
	pin_decimal_t		*upd_amt = NULL;
	pin_decimal_t		*scale = NULL;
	pin_flist_t		*city_flistp = NULL;
	int32           precision = 2;
	pin_decimal_t		*tolrnc_amt = NULL;
	pin_decimal_t		*scale_amt = NULL;
	poid_t			*rate_pdp = NULL;	
	char                    buf[250];
	pin_decimal_t           *disc = NULL;
        pin_decimal_t           *zero_disc = NULL;	
	int			cc_elemid = 0;
	pin_cookie_t		cc_cookie = NULL;
	pin_decimal_t		*tax_amt = NULL;
	pin_decimal_t		*cycle_disc = NULL;
	pin_decimal_t		*cycle_disc_amt = NULL;
	pin_decimal_t		*disc_amnt = NULL;
	int32			discount = 0;
	poid_t			*purc_pdp = NULL;
	int32			disc_amt = 0;
	time_t			gst_trans_date = 1498847400;
	time_t			*orig_evt_created = NULL;
	pin_flist_t     *mso_nrc_paymt_trnsfr_retflistp = NULL;
    pin_flist_t     *mso_nrc_paymt_trnsfr_outflistp = NULL;

        int32                   pp_type = -1;
        int32                   service_type = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_RATE_POL_POST_RATING) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_post_rating opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_rate_pol_post_rating input flist", i_flistp);
	evt_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	acct_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	bal_grp_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
	evt_type = (char *)PIN_POID_GET_TYPE(evt_pdp);
	prog_name = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	sys_descr = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SYS_DESCR, 1, ebufp);

	pin_conf("fm_cust_pol", "currency", PIN_FLDT_INT, (caddr_t *)&currency_valp, &err);

	if (currency_valp != (int32 *) NULL)
	{
		res_id = *currency_valp;
		free(currency_valp);
	}

	// Additional Tax should not be applied on the Balance Transfer from Commission module
	if (	prog_name != NULL && 
		strstr(prog_name,"LCO Commission-Bal Transfer") &&
		fm_utils_op_is_ancestor(connp->coip,MSO_OP_COMMISSION_PROCESS_COMMISSION)
	   )
	{
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_FLAGS, &no_tax_flag, ebufp);
	}


	tax_flags = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);

	vp=PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);	
	if(vp) {
		   svc_pdp=(poid_t *)vp;
		   svc_type=(char *)PIN_POID_GET_TYPE(svc_pdp);
	}

	//for commission
	if (evt_type && strcmp(evt_type, "/event/billing/settlement/ws/outcollect/charge_head_based") ==0 )
	{
		session_obj =  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SESSION_OBJ, 1, ebufp);
		ch_head_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_CHARGE_HEAD_INFO, 1, ebufp);
		comm_ref_flagp = PIN_FLIST_FLD_GET(ch_head_flistp, PIN_FLD_FLAGS, 1, ebufp);
		if (session_obj)
		{
			if (strcmp(PIN_POID_GET_TYPE(session_obj),"/event/billing/batch/reversal") ==0 ||                                                                                                                               (comm_ref_flagp != NULL && *comm_ref_flagp == 1))
			{
				while ((bal_impacts = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
					&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
				{
					resource_id = PIN_FLIST_FLD_GET(bal_impacts, PIN_FLD_RESOURCE_ID, 1, ebufp);
					if (resource_id && *resource_id ==356)
					{
						impact_amount = PIN_FLIST_FLD_GET(bal_impacts, PIN_FLD_AMOUNT, 1, ebufp);
						if (impact_amount)
						{
							//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "333");
							negative_impact_amount = pbo_decimal_negate(impact_amount, ebufp);
							PIN_FLIST_FLD_SET(bal_impacts, PIN_FLD_AMOUNT, negative_impact_amount, ebufp);
						}
					}
				}
				if (!pbo_decimal_is_null(negative_impact_amount, ebufp))
				{
					pbo_decimal_destroy(&negative_impact_amount);
				}
				
				total_arrayp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOTAL, 356, 1, ebufp);
				if (total_arrayp)
				{
					impact_amount = PIN_FLIST_FLD_GET(total_arrayp, PIN_FLD_AMOUNT, 1, ebufp);
					if (impact_amount)
					{
						//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "333");
						negative_impact_amount = pbo_decimal_negate(impact_amount, ebufp);
						PIN_FLIST_FLD_SET(total_arrayp, PIN_FLD_AMOUNT, negative_impact_amount, ebufp);
					}
				}

				if (!pbo_decimal_is_null(negative_impact_amount, ebufp))
				{
					pbo_decimal_destroy(&negative_impact_amount);
				}
				if (ch_head_flistp){
					impact_amount = PIN_FLIST_FLD_GET(ch_head_flistp, MSO_FLD_COLLECTION_AMT, 1, ebufp);
					if (impact_amount)
					{
						negative_impact_amount = pbo_decimal_negate(impact_amount, ebufp);
						PIN_FLIST_FLD_SET(ch_head_flistp, MSO_FLD_COLLECTION_AMT, negative_impact_amount, ebufp);
					}
				}
			}
		}
		// Drop the FLAGS  (technically ref flag) which is not required for further processing
		if(comm_ref_flagp != NULL) PIN_FLIST_FLD_DROP(ch_head_flistp, PIN_FLD_FLAGS, ebufp);
	}

	if (evt_type && strcmp(evt_type, "/event/billing/product/action/cancel") ==0 &&
		prog_name && strcmp(prog_name, "pin_cycle") ==0) 
	{
		lcc_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, lcc_iflistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, lcc_iflistp, PIN_FLD_USERID, ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, lcc_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
                lcc_temp_flistp = PIN_FLIST_ELEM_ADD(lcc_iflistp, PIN_FLD_PLAN_LISTS, 0, ebufp);

                prd_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PRODUCT,PIN_ELEMID_ANY, 1, ebufp );
		if (prd_flistp)
		{
                	off_obj=PIN_FLIST_FLD_GET(prd_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp );
		}

                offr_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_POID, (void *)off_obj, ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_PACKAGE_ID, NULL, ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);

                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, offr_flistp, &read_out_flistp, ebufp);

                PIN_FLIST_DESTROY_EX(&offr_flistp, NULL);
	
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Read flds failed of offerin object. ", ebufp);
		}
		else
		{
                	PIN_FLIST_FLD_COPY(read_out_flistp, PIN_FLD_PLAN_OBJ, lcc_temp_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		}
                lcc_deal_flistp = PIN_FLIST_ELEM_ADD(lcc_temp_flistp, PIN_FLD_DEALS, 0, ebufp);
                PIN_FLIST_FLD_COPY(read_out_flistp, PIN_FLD_PACKAGE_ID, lcc_deal_flistp, PIN_FLD_PACKAGE_ID, ebufp);
                PIN_FLIST_FLD_COPY(read_out_flistp, PIN_FLD_DEAL_OBJ, lcc_deal_flistp, PIN_FLD_DEAL_OBJ, ebufp);

                rplan_plan_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(read_out_flistp, PIN_FLD_PLAN_OBJ, rplan_plan_flistp,PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(rplan_plan_flistp,  PIN_FLD_CODE, NULL, ebufp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0,rplan_plan_flistp, &read_out_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&rplan_plan_flistp, NULL);

                PIN_FLIST_FLD_COPY(read_out_flistp, PIN_FLD_CODE, lcc_temp_flistp, PIN_FLD_CODE, ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, lcc_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_create_lifecycle_evt input preparation", lcc_iflistp);

                PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);


		if(strcmp(svc_type,"/service/telco/broadband")==0) {
			mso_call_bb_cancel_plan(ctxp, i_flistp, ebufp);
		} else {
			mso_call_prov_cancel_plan(ctxp, i_flistp, ebufp);
		}
		fm_mso_create_lifecycle_evt(ctxp, lcc_iflistp, lcc_r_flistp, ID_REMOVE_PLAN, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_lifecycle_evt  return flist", lcc_r_flistp);
		PIN_FLIST_DESTROY_EX(&lcc_r_flistp, NULL);	
	}
	
	bal_impact_flag = PIN_FLIST_ELEM_COUNT(i_flistp,PIN_FLD_BAL_IMPACTS,ebufp);
	if (bal_impact_flag > 0) 
	{
		service_type = mso_remove_custom_balImpact_taxation(ctxp, &i_flistp, ebufp);
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before Process Purchase Fee");

	if(tax_flags && ((*tax_flags & PIN_RATE_FLG_INVERTED)))
	{
		new_opstackp = connp->coip;
		initial_flistp = NULL;
		initial_opcode = 0;
		while(new_opstackp != NULL)
		{
			stack_opcode = new_opstackp->opcode;
			if(stack_opcode == 9068)
			{
	                        memset(tmp_buff, '\0', sizeof(tmp_buff));
       	                	sprintf(tmp_buff, "OPCODE=%d", stack_opcode);
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"9068 flistp", new_opstackp->in_flistp);
				initial_flistp = new_opstackp->in_flistp;
				initial_opcode = new_opstackp->opcode;
				break;
			}
			new_opstackp = new_opstackp->next;
		}
		if(initial_flistp)
		{
			product_flistp = PIN_FLIST_ELEM_GET(initial_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);
			if(product_flistp)
			{
				orig_evt_flistp = PIN_FLIST_SUBSTR_GET(product_flistp, PIN_FLD_EVENT, 1, ebufp);
				if(orig_evt_flistp)
				{
					orig_evt_created = PIN_FLIST_FLD_GET(orig_evt_flistp, PIN_FLD_CREATED_T, 0, ebufp);
					
					if(orig_evt_created && (*orig_evt_created < gst_trans_date))
					{
						fm_rate_pol_transition_remove_taxes(ctxp, &i_flistp, svc_type, ebufp);
					}
				}
			}

		}
	}
	

	if (evt_pdp != NULL) 
	{
		if ( strstr(PIN_POID_GET_TYPE(evt_pdp),"/event/billing/product/fee/") && strcmp(prog_name, "STPK_REFUND") != 0) 
		{
			commission_flag = 1; 
			bal_impact_flag = 0; 
			/*Get the balance START*/
			total_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOTAL, 356, 1, ebufp);
			if (total_flistp != NULL) 
			{
				amt = PIN_FLIST_FLD_GET(total_flistp, PIN_FLD_AMOUNT, 1, ebufp);
				if (pbo_decimal_is_zero(amt, ebufp) == 0) 
				{
					bal_impact_flag = 1;
					if (pbo_decimal_sign(amt, ebufp) == -1 )
						commission_rev_flag = 1;
				}
			}
			/*Get the balance END*/
		}
		if ( strstr(PIN_POID_GET_TYPE(evt_pdp),"/event/billing/item/transfer"))
		{
			commission_flag = 2;
		}
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before Commission Process");
	if (commission_flag == 2 || (commission_flag == 1 && bal_impact_flag != 0 ))
	{
		if (svc_type == NULL )
                {
                        if (bal_grp_pdp ){
                                fm_mso_utils_get_service_from_balgrp(ctxp, bal_grp_pdp, &srvc_info_flistp, ebufp);
                                //results_flistp = PIN_FLIST_ELEM_GET(srvc_info_flistp,PIN_FLD_RESULTS, PIN_ELEMID_ANY,0, ebufp);
				//Changed to soft get as its affecting payments without service 
				results_flistp = PIN_FLIST_ELEM_GET(srvc_info_flistp,PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
                                if (results_flistp){
                                        vp = (poid_t *)PIN_FLIST_FLD_GET(results_flistp,PIN_FLD_POID, 0, ebufp);
                                        svc_type = (char *)PIN_POID_GET_TYPE(vp);
                                }
                        }
                }
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, svc_type);
        	if(svc_type && strcmp(svc_type,"/service/catv")!=0)      //Condition Added to not to process this for CATV	
		{
			opstackp = connp->coip;
			act_usage_flag=-1;
			while(opstackp != NULL)
			{
				stack_opcode = opstackp->opcode;
				memset(tmp_buff, '\0', sizeof(tmp_buff));
				sprintf(tmp_buff, "OPCODE=%d", stack_opcode);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"BEFORE PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
				if(stack_opcode == PCM_OP_ACT_USAGE)
				{
					act_usage_flag = opstackp->opflags;
					sprintf(tmp_buff, "PCM_OP_ACT_USAGE FLAG=%d", act_usage_flag);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
					memset(tmp_buff, '\0', sizeof(tmp_buff));
				}
				initial_flistp = opstackp->in_flistp;
				initial_opcode = opstackp->opcode;
				
				opstackp = opstackp->next;
			}
			if( act_usage_flag == 896 || act_usage_flag == 384 )
			{
				flag_process_commission = 0;
			}


			if (flag_process_commission)
			{

				/*Identify whether the event to be considered for commission process or not-START*/
				/******************************************************************************************************
				* This is only when cycle refund is created and we need to reverse the commission which is already applied
				* but only the same quantity that is being refudned.If 50% is refunded, then the commission would be
				* calculated on the same amount and reverse the commission amount.
				*******************************************************************************************************/
				if (commission_rev_flag == 1)
				{
				
					prod_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PRODUCT, 1, ebufp);
					if (prod_flistp){
						offering_pdp = (poid_t *)PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);
					}
					vp = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EARNED_END_T, 1, ebufp);
					earned_end_t = *(time_t *)vp;
					
					search_flistp = PIN_FLIST_CREATE( ebufp );
					search_pdp = PIN_POID_CREATE( database, "/search", -1, ebufp );
					PIN_FLIST_FLD_PUT( search_flistp, PIN_FLD_POID, search_pdp, ebufp );
					
					PIN_FLIST_FLD_SET( search_flistp, PIN_FLD_FLAGS, &flag, ebufp );
					PIN_FLIST_FLD_SET( search_flistp, PIN_FLD_TEMPLATE, evt_template, ebufp );
					
					args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 1, ebufp );
					PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp );
					args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 2, ebufp );
					temp_flistp = PIN_FLIST_ELEM_ADD( args_flistp, PIN_FLD_BAL_IMPACTS, 0, ebufp );
					PIN_FLIST_FLD_SET( temp_flistp, PIN_FLD_OFFERING_OBJ, offering_pdp, ebufp );
					args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 3, ebufp );
					PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_EARNED_END_T, &earned_end_t, ebufp);
					init_amt = pbo_decimal_from_str("0.0", ebufp);
					args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 4, ebufp );
					temp_flistp = PIN_FLIST_ELEM_ADD( args_flistp, PIN_FLD_BAL_IMPACTS, 356, ebufp );
					PIN_FLIST_FLD_PUT( temp_flistp, PIN_FLD_AMOUNT, (void *)init_amt, ebufp );
					args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 5, ebufp );
					temp_flistp = PIN_FLIST_ELEM_ADD( args_flistp, PIN_FLD_BAL_IMPACTS, 0, ebufp );
					PIN_FLIST_FLD_SET( temp_flistp, PIN_FLD_RESOURCE_ID, &res_id, ebufp );
					
					res_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
					PIN_FLIST_FLD_SET( res_flistp, PIN_FLD_POID, NULL, ebufp );
					PIN_FLIST_FLD_SET( res_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );
					PIN_FLIST_FLD_SET( res_flistp, PIN_FLD_ITEM_OBJ, NULL, ebufp );
					
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"post_rating_commission rev search event flist", search_flistp);
					PCM_OP( ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_oflistp, ebufp );
					PIN_FLIST_DESTROY_EX( &search_flistp, NULL );
					if (PIN_ERRBUF_IS_ERR(ebufp)){
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error during commission event charge search", ebufp);
						PIN_FLIST_DESTROY_EX(&srvc_info_flistp, NULL);
						return;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"post_rating_commission rev search event return flist", search_oflistp);
					
					while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT( search_oflistp,
													PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL )
					{
						charge_item_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ITEM_OBJ, 1, ebufp);
						fm_check_charge_item_applied_commission(ctxp, acct_pdp, charge_item_pdp, &charge_head_flistp, ebufp);
						if (PIN_ERRBUF_IS_ERR(ebufp)){
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting charge_head results", ebufp);
							PIN_FLIST_DESTROY_EX(&srvc_info_flistp, NULL);
							return;
						}
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"post_rating_commission reverse final charge flist TEST", charge_head_flistp);
					
						if (PIN_FLIST_ELEM_COUNT(charge_head_flistp, PIN_FLD_RESULTS, ebufp) > 0)
						{
							c_elemid = 0;
							c_cookie = NULL;
							while ((ch_res_flistp = PIN_FLIST_ELEM_GET_NEXT( charge_head_flistp,
										PIN_FLD_RESULTS, &c_elemid, 1, &c_cookie, ebufp )) != NULL )
							{
								ch_sub_flistp = PIN_FLIST_SUBSTR_GET(ch_res_flistp,MSO_FLD_CHARGE_HEAD_INFO,1,ebufp);
								coll_amt = PIN_FLIST_FLD_GET(ch_sub_flistp, MSO_FLD_COLLECTION_AMT, 1, ebufp);
								chg_head_totalp = PIN_FLIST_FLD_GET(ch_sub_flistp, MSO_FLD_CHARGE_HEAD_TOTAL, 1,ebufp);
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "coll amt");
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(coll_amt, ebufp));
								pbo_decimal_add_assign(total_amt, coll_amt, ebufp);
							}
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "total amt");
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(total_amt,ebufp));
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "charge head total amt");
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(chg_head_totalp,ebufp));
					
							qty_applied = pbo_decimal_divide(total_amt, chg_head_totalp, ebufp);
							final_amt = pbo_decimal_from_str("0", ebufp);
							final_amt = pbo_decimal_multiply(amt, qty_applied, ebufp);
							pbo_decimal_round_assign(final_amt, 2, ROUND_HALF_UP, ebufp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "final total commission reverse amt");
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(final_amt,ebufp));
							fm_mso_utils_read_any_object(ctxp, charge_item_pdp, &read_item_oflistp, ebufp);
							if (read_item_oflistp != NULL ){
								items_r_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_ITEMS, elem_id, ebufp);
								PIN_FLIST_FLD_COPY(read_item_oflistp, PIN_FLD_POID,                                                                                                                                             items_r_flistp, PIN_FLD_POID, ebufp);
								PIN_FLIST_FLD_COPY(read_item_oflistp, PIN_FLD_SERVICE_OBJ,                                                                                                                                      items_r_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
								PIN_FLIST_FLD_COPY(read_item_oflistp, PIN_FLD_ACCOUNT_OBJ,                                                                                                                                      items_r_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
								PIN_FLIST_FLD_COPY(read_item_oflistp, PIN_FLD_ITEM_TOTAL,																	items_r_flistp, PIN_FLD_ITEM_TOTAL, ebufp);
								PIN_FLIST_FLD_COPY(read_item_oflistp, PIN_FLD_DUE, items_r_flistp, PIN_FLD_DUE, ebufp);
								PIN_FLIST_FLD_COPY(read_item_oflistp, PIN_FLD_EFFECTIVE_T,                                                                                                                                      items_r_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
								PIN_FLIST_FLD_SET(items_r_flistp, PIN_FLD_ITEM_TYPE,                                                                                                                                            (char *)PIN_POID_GET_TYPE(charge_item_pdp), ebufp);
								PIN_FLIST_FLD_PUT(items_r_flistp, PIN_FLD_AMOUNT, pbo_decimal_negate(final_amt, ebufp), ebufp);
								PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"post_rating_commission reverse commission items",                                                                                                                                items_r_flistp);
							}
							PIN_FLIST_DESTROY_EX(&charge_head_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&read_item_oflistp, NULL);
						}
					}
					PIN_FLIST_DESTROY_EX( &search_oflistp, NULL );
					
				}
				/********* COMMISSION REVERSE incase of CYCLE REFUND ENDS HERE ************************/

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"post_rating_commission start ");
				commi_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"post_rating_commission start input flist", commi_flistp);
			
				PCM_OP(ctxp, MSO_OP_COMMISSION_PROCESS_COMMISSION, LCO_COMMISSION, commi_flistp, &comm_r_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{	
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error set");
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"post_rating_commission output flist", comm_r_flistp);
				PIN_FLIST_DESTROY_EX(&commi_flistp, NULL);
				if (comm_r_flistp != NULL) 
				{
					status = PIN_FLIST_FLD_GET(comm_r_flistp, PIN_FLD_STATUS, 1, ebufp);
					if (status && *status == 0) 
					{
						sprintf(tmp,"Status is: %d", *status);
                                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
							"fm_rate_pol_post_rating process commission succeeded", comm_r_flistp);
						comm_succ_flag = 1;
						PIN_FLIST_DESTROY_EX(&comm_r_flistp, NULL);
					}
					else 
					{
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_rate_pol_post_rating process commission failed", comm_r_flistp);
						vp = PIN_FLIST_FLD_GET(comm_r_flistp, PIN_FLD_ERROR_CODE, 1, ebufp);
						if (vp)
						{
							commission_err_code=atoi((char*)vp);
						}
						PIN_FLIST_DESTROY_EX(&comm_r_flistp, NULL);
						if (commission_err_code)
						{
							pin_set_err(ebufp, PIN_ERRLOC_FM_COMM, PIN_ERRCLASS_APPLICATION, commission_err_code, 0, 0, opcode);
						}
						else
						{
							pin_set_err(ebufp, PIN_ERRLOC_FM_COMM, PIN_ERRCLASS_APPLICATION, PIN_ERR_UNKNOWN_EXCEPTION, 0, 0, opcode);
						}
					//	return;
					}
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"post_rating_commission end");
				/*Identify whether the event to be considered for commission process or not-END*/
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After Commission Process");
		}
		PIN_FLIST_DESTROY_EX(&srvc_info_flistp, NULL);
	}
	// Drop the ITEMS array from input flist after commission process
	if (PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_ITEMS, ebufp) > 0 ){
		elem_id = 0;
		cookie = NULL;
		while ( PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ITEMS, &elem_id, 1, &cookie, ebufp) != NULL){
			PIN_FLIST_ELEM_DROP(i_flistp, PIN_FLD_ITEMS, elem_id, ebufp);
		}
	}
	/***changes for setting the fuptopup balance validity valid_to*/
	prod_array = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PRODUCT, 1, ebufp);
	if(prod_array!=NULL && tax_flags && !(*tax_flags & PIN_RATE_FLG_INVERTED))
	{
        	prod_pdp = PIN_FLIST_FLD_GET(prod_array, PIN_FLD_PRODUCT_OBJ, 1, ebufp);	
		fup_top = fm_mso_get_product_priority(ctxp, prod_pdp, &ret_flistp, ebufp);	
		if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_get_product_priority", ebufp);
                 	return;
        	}
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	}
	if (strstr(evt_type, "mso_grant") && 
		tax_flags && !(*tax_flags & PIN_RATE_FLG_INVERTED) &&
		((prog_name && ((strstr(prog_name, "pin_bill_accts") == NULL) || strstr(prog_name, "pin_cycle") == NULL)) || (fup_top == 1 || fup_top == 2)))
	{
		while( (bal_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
                                &bal_elemid, 1, &bal_cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			sub_bal_flistp = PIN_FLIST_ELEM_GET(bal_flistp, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, 1, ebufp);
			if (sub_bal_flistp)
			{
				PIN_FLIST_FLD_SET(sub_bal_flistp, PIN_FLD_VALID_FROM, &current_t, ebufp);
				if(fup_top == 1 || fup_top == 2)
				{
					fm_mso_get_poid_details(ctxp, svc_pdp, &ret_flistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returning from function fm_mso_get_poid_details1", ebufp);
						return;
					}
					bb_info_flistp = PIN_FLIST_SUBSTR_GET(ret_flistp, MSO_FLD_BB_INFO, 0, ebufp );
					indicator = PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_INDICATOR, 0, ebufp);
					if(indicator && *indicator == 2)
					{
						vp = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EARNED_END_T, 1, ebufp);
						if(vp)
						{
							earned_end_t = *(time_t *)vp;
							PIN_FLIST_FLD_SET(sub_bal_flistp, PIN_FLD_VALID_TO, &earned_end_t, ebufp);
						}
						else
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while setting valid_to where earned_end_t is null", ebufp);
							pin_set_err(ebufp, PIN_ERRLOC_FM,
                        				PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        				PIN_ERR_BAD_VALUE, PIN_FLD_VALID_TO, 0, 0);
							return;
								
						}
						
					}
					PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
				}
			}
		}
	}	

	/********************************************************************************************
	* Below section is added for implementing adding addition DATA to an existing balance
	* by checking if DATA offers avaialble for the customer - ADD DATA OFFERS
	********************************************************************************************/
	if (strstr(evt_type, "mso_grant") && 
		tax_flags && !(*tax_flags & PIN_RATE_FLG_INVERTED))
	{
		bal_elemid = 0;
		bal_cookie = NULL;
		fm_mso_utils_read_any_object(ctxp, svc_pdp, &service_flistp, ebufp);
		if(service_flistp){
			bbinfo_flistp = PIN_FLIST_SUBSTR_GET(service_flistp, MSO_FLD_BB_INFO, 1, ebufp);
			if(bbinfo_flistp){
				city = PIN_FLIST_FLD_GET(bbinfo_flistp, PIN_FLD_CITY, 1, ebufp);
				billwhen = PIN_FLIST_FLD_GET(bbinfo_flistp, PIN_FLD_BILL_WHEN,1, ebufp);
			}
		}
		while( (bal_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
                                &bal_elemid, 1, &bal_cookie, ebufp)) != NULL)
		{
			data_res_id = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_RESOURCE_ID, 1, ebufp);
			if (data_res_id && (*data_res_id == FUP_LIMIT_RESOURCE  || *data_res_id == FREE_MB_RESOURCE)){
				current_data = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_AMOUNT, 1, ebufp);
				//First we will get the QUOATA from creidt_limit which would have been mapped to city
				offering_obj = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
				fm_mso_utils_read_any_object(ctxp, offering_obj, &purch_flistp, ebufp);
				if(purch_flistp){
					plan_obj = PIN_FLIST_FLD_GET(purch_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
					fm_mso_utils_read_any_object(ctxp, plan_obj, &plan_flistp, ebufp);
					if(plan_flistp){
						plan_name = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_NAME, 1, ebufp);
					}
					prod_obj = PIN_FLIST_FLD_GET(purch_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
					fm_mso_utils_read_any_object(ctxp, prod_obj, &product_flistp, ebufp);
					if(product_flistp){
						priority = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_PRIORITY, 1, ebufp);
						if(priority)
							tmp_str = pbo_decimal_to_str(priority, ebufp);
						if(tmp_str)
							prty = atoi(tmp_str);
						if(prty)
							mod_prty = prty%1000;
						
						//For FUP TOPUP and ADD MB plans, payterm would be zero.	
						if(prty >= 1900 && ((mod_prty >= BB_FUP_START 
							&& mod_prty <= BB_FUP_END ) || (mod_prty >= BB_ADD_MB_START
								&& mod_prty <= BB_ADD_MB_END))){
							billwhen = &billterm;
						}
					}
				}
				fm_mso_get_service_quota_codes(ctxp, plan_name, billwhen, city, &cfg_cr_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting service_quota code results", ebufp);
					PIN_FLIST_DESTROY_EX(&purch_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&plan_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&product_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&cfg_cr_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&service_flistp, NULL);
					return;
				}
				if(cfg_cr_flistp){
					ccprofile_flistp = PIN_FLIST_ELEM_GET(cfg_cr_flistp, MSO_FLD_CREDIT_PROFILE, *data_res_id, 1, ebufp);
					if(ccprofile_flistp){
						quota_amt = PIN_FLIST_FLD_TAKE(ccprofile_flistp, PIN_FLD_CREDIT_FLOOR, 1, ebufp);
						quota_perc = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_PERCENT, 1, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(quota_amt, ebufp));
					
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "RATE_POL CREDIT PROFILE", ccprofile_flistp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(quota_amt, ebufp));
						if(!pbo_decimal_is_null(quota_amt, ebufp) && !pbo_decimal_is_zero(quota_amt, ebufp))
							pbo_decimal_multiply_assign(quota_amt, quota_perc, ebufp);	
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(quota_amt, ebufp));
						if(!pbo_decimal_is_null(quota_amt, ebufp) && !pbo_decimal_is_zero(quota_amt, ebufp))
							current_data = pbo_decimal_copy(quota_amt, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(current_data, ebufp));
						if(!pbo_decimal_is_null(quota_amt, ebufp))
							pbo_decimal_destroy(&quota_amt);
					}
						
				}
				if(*data_res_id == FUP_LIMIT_RESOURCE){
					ntotal_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOTAL, FUP_LIMIT_RESOURCE, 1, ebufp);
				}
				else if(*data_res_id == FREE_MB_RESOURCE){
					ntotal_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOTAL, FREE_MB_RESOURCE, 1, ebufp);
				}
				// Procced to check offer data
				// Dont add OFFER data for ADD MB and FUP TOPUP PLANS
                                if(!(prty >= 1900 && ((mod_prty >= BB_FUP_START && mod_prty <= BB_FUP_END )
					|| (mod_prty >= BB_ADD_MB_START && mod_prty <= BB_ADD_MB_END))))
				{
					srch_iflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, srch_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
					offer_type = 2;
					PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TYPE, &offer_type, ebufp);
					fm_mso_search_offer_entries(ctxp, srch_iflistp, &srch_oflistp, ebufp);
					PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp)){
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting charge_head results", ebufp);
						PIN_FLIST_DESTROY_EX(&purch_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&plan_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&product_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&cfg_cr_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&service_flistp, NULL);
						return;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "RATE_POL OFFER RESULTS", srch_oflistp);
					if (srch_oflistp && PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp) > 0){
						offer_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1,ebufp);
						if(offer_flistp){
							addl_data = PIN_FLIST_FLD_GET(offer_flistp, PIN_FLD_AMOUNT, 1, ebufp);	
							if(!pbo_decimal_is_null(addl_data,  ebufp) || !pbo_decimal_is_zero(addl_data,  ebufp)){
								//Add additional offer data to existing balance
								total_data = pbo_decimal_add(current_data, pbo_decimal_negate(addl_data, ebufp), ebufp);
								PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_AMOUNT, total_data, ebufp);
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NEW DATA BALANCE");
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(total_data, ebufp));
								if(*data_res_id == FUP_LIMIT_RESOURCE){
									ntotal_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOTAL, FUP_LIMIT_RESOURCE, 1, ebufp);
								}
								else if(*data_res_id == FREE_MB_RESOURCE){
									ntotal_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOTAL, FREE_MB_RESOURCE, 1, ebufp);
								}
								PIN_FLIST_FLD_SET(ntotal_flistp, PIN_FLD_AMOUNT, total_data, ebufp);	
								if(!pbo_decimal_is_null(total_data,  ebufp))
									pbo_decimal_destroy(&total_data);
							}
						}
					}//Else set back with current data 
					else{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NEW CURRENT BALANCE");
						PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_AMOUNT, current_data, ebufp);
						PIN_FLIST_FLD_SET(ntotal_flistp, PIN_FLD_AMOUNT, current_data, ebufp);	
					}
					PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
				}
				else{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NEW CURRENT BALANCE FUP TOPUP ADDMB plans");
					PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_AMOUNT, current_data, ebufp);
					PIN_FLIST_FLD_SET(ntotal_flistp, PIN_FLD_AMOUNT, current_data, ebufp);	
				}
				PIN_FLIST_DESTROY_EX(&purch_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&plan_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&product_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&cfg_cr_flistp, NULL);
			}
		}
		PIN_FLIST_DESTROY_EX(&service_flistp, NULL);
	}

	/*
	if(evt_type &&
		strcmp(evt_type, "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_grant") == 0 &&
		!fm_utils_op_is_ancestor(connp->coip,
                        PCM_OP_SUBSCRIPTION_CANCEL_PRODUCT)) {
	    send_sms_notification_for_renewal();
	}
	*/

/*
	if (PIN_FLIST_ELEM_COUNT(i_flistp,PIN_FLD_BAL_IMPACTS,ebufp) > 0 )
	{
		mso_remove_custom_balImpact_taxation(ctxp, &i_flistp, ebufp);
	}
*/

	/* This is needed as the Service Taxes is not available in multiple balance impacts
	 * and not passing through our customization done in fm_rate_pol_map_tax_supplier opcode
	 * as well to add any custom tax balance impacts. Hence adding it here.
	 */
	 if (PIN_FLIST_ELEM_COUNT(i_flistp,PIN_FLD_BAL_IMPACTS,ebufp) <= 2 )
	{
		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,evt_type);
	
		if ((evt_type &&
			((strcmp(evt_type, "/event/billing/adjustment/item") ==0) || (strcmp(evt_type, "/event/billing/adjustment/account") ==0) || (strcmp(evt_type, "/event/billing/dispute/item") ==0) || (strcmp(evt_type, "/event/billing/settlement/item") ==0)))&&
			(sys_descr &&
			((strcmp(sys_descr, "Item Adjustment") ==0) || (strcmp(sys_descr, "Event for Account Adjustment") ==0) || (strcmp(sys_descr, "Item Dispute")==0) || (strcmp(sys_descr, "Item Settlement")==0))) &&
			(tax_flags && (*tax_flags == 2))
			)
		{
			mso_adj_add_custom_balImpact_taxation(ctxp, connp, &i_flistp, evt_type, ebufp);
		}

		//Calling the add balance impacts for Minimum Commitment Pre-rated Event
		if (evt_type && (
				(strcmp(evt_type, "/event/billing/mso_catv_commitment") == 0)
				 || (strcmp(evt_type, "/event/billing/mso_cr_dr_note") == 0)
				 || (strcmp(evt_type, "/event/billing/mso_penalty") == 0)
				 || (strcmp(evt_type, "/event/billing/late_fee") == 0)
			 	 || (strcmp(evt_type, "/event/mso_et") == 0)
				)
		   )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Adding tax bal impacts");
			mso_adj_add_custom_balImpact_taxation(ctxp, connp, &i_flistp, evt_type, ebufp);
		}
	}




/**************** calling mso_rate_pol_record_cur_bal *****************/
	total_arrayp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOTAL, 356, 1, ebufp);
	if(evt_type && !strcmp(evt_type, "/event/billing/adjustment/event" )){
		while( (sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT( i_flistp, PIN_FLD_SUB_BAL_IMPACTS,
				&sub_bal_elemid, 1, &sub_bal_cookie, ebufp ))!= (pin_flist_t *)NULL){
		 	vp = PIN_FLIST_FLD_GET( sub_bal_flistp, PIN_FLD_RESOURCE_ID, 1, ebufp );
			if(vp){
				resourceid = *(int *)vp;
			}
			if( resourceid == 356 ){
				total_arrayp = PIN_FLIST_ELEM_GET( sub_bal_flistp, PIN_FLD_SUB_BALANCES, 
								PIN_ELEMID_ANY, 1, ebufp );
			}
		}
	}
	if(total_arrayp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
             		"op_rate_pol_post_rating calling mso_rate_pol_record_cur_bal");

	    impact_amount = PIN_FLIST_FLD_GET(total_arrayp, PIN_FLD_AMOUNT, 0, ebufp);
 	   // disc_amount = PIN_FLIST_FLD_GET(total_arrayp, PIN_FLD_DISCOUNT, 0, ebufp);
	    if (pbo_decimal_is_zero(impact_amount, ebufp) != 0)
	    {
		bal_elemid = 0;
		bal_cookie = NULL;
		while( (bal_flistp = PIN_FLIST_ELEM_GET_NEXT( i_flistp, PIN_FLD_BAL_IMPACTS,
                                &bal_elemid, 1, &bal_cookie, ebufp ))!= (pin_flist_t *)NULL)
		{
                        disc_amount = PIN_FLIST_FLD_GET( bal_flistp, PIN_FLD_DISCOUNT, 1, ebufp );
			if (pbo_decimal_is_zero(disc_amount, ebufp) == 0)	
			{
				break;
                        }
                }
	
	    }	
            //if (pbo_decimal_is_zero(impact_amount, ebufp) == 0)
            if ((pbo_decimal_is_zero(impact_amount, ebufp) == 0) ||
                        (pbo_decimal_is_zero(disc_amount, ebufp) == 0))
	    {
        	
//		opstackp = connp->coip;
//		while(opstackp != NULL)
//		{
//			stack_opcode = opstackp->opcode;
//			memset(tmp_buff, '\0', sizeof(tmp_buff));
//			sprintf(tmp_buff, "OPCODE=%d", stack_opcode);
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
//			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"BEFORE PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
//			if(stack_opcode == PCM_OP_ACT_USAGE)
//			{
//				act_usage_flag = opstackp->opflags;
//				sprintf(tmp_buff, "PCM_OP_ACT_USAGE FLAG=%d", act_usage_flag);
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
//				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
//				memset(tmp_buff, '\0', sizeof(tmp_buff));
//			}
//			initial_flistp = opstackp->in_flistp;
//			initial_opcode = opstackp->opcode;
//			
//			opstackp = opstackp->next;
//		}		
//		if(act_usage_flag != 896)
//		{
			//mso_rate_pol_record_cur_bal(connp,i_flistp,ebufp);
//		}

	/***moved to below ***/
	// Added to prevent the duplicate entried for mso_Account_balance_t
	/*	 if (!(flags & PCM_OPFLG_CALC_ONLY) && zero_chr_catv ==0  )
		{
 
			mso_rate_pol_record_cur_bal(connp,i_flistp,ebufp);
		}
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
               		"op_rate_pol_post_rating modified input flist", i_flistp);
	    }
	}*/
	/**************** calling mso_rate_pol_record_cur_bal *****************/

	/***********************************************************
	 * Just a straight copy of the input flist. 
	 ***********************************************************/
	//r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

	/****created function to handle the decimal changes given in system***/
	if ((svc_type && strcmp(svc_type,"/service/catv")) && !(flags & PCM_OPFLG_CALC_ONLY) && (strstr(PIN_POID_GET_TYPE(evt_pdp),"/event/billing/product/fee/cycle")))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Purchase and refund flow");
		fm_mso_find_decimal_entry(ctxp, i_flistp, &out_flistp, ebufp);		
		if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error after returning functon fm_mso_find_decimal_entry", ebufp);
                	return;
        	}
		if(out_flistp != NULL)
		{
			city_flistp = PIN_FLIST_ELEM_GET(out_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, 1, ebufp);
			if(city_flistp == NULL)
			{
				orig_amount = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_AMOUNT_ORIG, 1, ebufp);
                        	bill_when = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_BILL_WHEN, 1, ebufp);
			}
			else
			{
				orig_amount = PIN_FLIST_FLD_GET(city_flistp, PIN_FLD_AMOUNT_ORIG, 1, ebufp);
				bill_when = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_BILL_WHEN, 1, ebufp);
			}

			if (pbo_decimal_is_null(orig_amount, ebufp))
				goto SKIP;

			cycle_info_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_CYCLE_INFO, 0, ebufp);
                        scale = PIN_FLIST_FLD_GET(cycle_info_flistp, PIN_FLD_SCALE, 0, ebufp);
			round_amount = pbo_decimal_from_str("0.0",ebufp);	
			if(pbo_decimal_compare(scale, round_amount, ebufp) == 0)
				goto SKIP;
			/*if(atoi(pbo_decimal_to_str(scale, ebufp)) <= 0)
				goto SKIP;*/

			sprintf(buf, "%d", *bill_when);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, buf);
			payterm = pbo_decimal_from_str(buf, ebufp);
			round_amount = pbo_decimal_divide(orig_amount, payterm, ebufp);
			sprintf(msg, "Actual amount : %s", pbo_decimal_to_str(round_amount, ebufp));
			PIN_ERR_LOG_MSG(3, msg);
			cookie = NULL;
			while ((bal_imp_flistp =
					PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
				       &elem_id, 1, &cookie, ebufp)) != NULL)
			{

				/**skipping the calculation if any discount present in flist**/
				//disc = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_DISCOUNT, 1, ebufp);
				//zero_disc = pbo_decimal_from_str("0.0",ebufp);
				/*if(!pbo_decimal_compare(zero_disc, disc, ebufp) == 0)
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "having discount", ebufp);
                                	goto SKIP;
				}*/
				purc_pdp = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);	
				//tax_code = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
				//if(tax_code && strcmp(tax_code,"MSO_Service_Tax") ==0)
				//if(purc_pdp != NULL)
				if(purc_pdp && !PIN_POID_IS_NULL(purc_pdp))
				{
					rate_pdp = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_RATE_OBJ, 0, ebufp);
					fm_mso_get_poid_details(ctxp, rate_pdp, &ret_flistp, ebufp);
                        		if (PIN_ERRBUF_IS_ERR(ebufp))
                        		{
                                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_get_poid_details", ebufp);
                                		return;
                        		}
					results_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_QUANTITY_TIERS, 0, 0, ebufp );
					bal_impacts = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_BAL_IMPACTS, 0, 0, ebufp );
					scale_amt = PIN_FLIST_FLD_GET(bal_impacts, PIN_FLD_SCALED_AMOUNT, 0, ebufp);
					if(tax_flags && !(*tax_flags & PIN_RATE_FLG_INVERTED))
					{
						if(pbo_decimal_compare(round_amount, scale_amt, ebufp) < 0)
						{
							pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE,
							PIN_ERR_BAD_VALUE, 0, 0, 0);
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Rate_plan amount is greater than credit limit amount", ebufp);
							return;	
						}
					}
					tolrnc_amt = pbo_decimal_from_str("0.0",ebufp);
					tolrnc_amt = (pin_decimal_t *)pbo_decimal_subtract(round_amount, scale_amt, ebufp);
					sprintf(msg, "Tolerance between Rate_plan amount and credit limit %s", pbo_decimal_to_str(tolrnc_amt, ebufp));
                                                                PIN_ERR_LOG_MSG(3, msg);
					if(tax_flags && !(*tax_flags & PIN_RATE_FLG_INVERTED))
					{
						if(pbo_decimal_to_double(tolrnc_amt,ebufp) >= 1)
						{
							pin_set_err(ebufp, PIN_ERRLOC_FM,
							PIN_ERRCLASS_SYSTEM_DETERMINATE,
							PIN_ERR_BAD_VALUE, 0, 0, 0);
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Tolerance between Rate_plan amount and credit limit amount >=1 ", ebufp);
							return;
						}
					}
					//PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
					/*calculating discount price*/
					upd_amt = pbo_decimal_from_str("0.0",ebufp);
					disc_amnt = pbo_decimal_from_str("0.0",ebufp);

                                        scale_perc = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_PERCENT, 0, ebufp);
	
					fm_mso_get_poid_details(ctxp, purc_pdp, &ret_flistp, ebufp);
                                        cycle_disc_amt = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_CYCLE_DISC_AMT, 0, ebufp);
                                        cycle_disc = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_CYCLE_DISCOUNT, 0, ebufp);
                                        if(!pbo_decimal_sign(cycle_disc, ebufp) == 0)
                                        {
						discount = 1;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Having discount percentage");
						disc_amnt = pbo_decimal_multiply(round_amount, cycle_disc, ebufp);
                                        }
					else if(!pbo_decimal_sign(cycle_disc_amt, ebufp) == 0)
					{
						disc_amt = 1;
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Having discount amount");
						//disc_amnt = pbo_decimal_multiply(round_amount, cycle_disc_amt, ebufp);
						disc_amnt = pbo_decimal_add(disc_amnt, cycle_disc_amt, ebufp);
					}
					if(discount == 1)
					{
						discount = 0;
						if(pbo_decimal_sign(disc_amnt, ebufp) < 0)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "During premium amount");
							upd_amt = pbo_decimal_subtract(disc_amnt, round_amount, ebufp);
                                                        pbo_decimal_abs_assign(upd_amt, ebufp);
							discount = 1;
						}
						else
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "During discount amount");
							if(pbo_decimal_compare(round_amount, disc_amnt, ebufp) < 0)
                                                        {
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount calculation morethan 100% is not handled here");
								pin_set_err(ebufp, PIN_ERRLOC_FM,
                                                		PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                                		PIN_ERR_BAD_VALUE, 0, 0, 0);
                                                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Discount percentage is more than 100 not allowed", ebufp);
								return;	
                                                        }
							upd_amt = pbo_decimal_subtract(round_amount, disc_amnt, ebufp);
							pbo_decimal_abs_assign(upd_amt, ebufp);
							discount = 1;
							
						}	
					}
					if(discount == 1)
					{
						pbo_decimal_multiply_assign(upd_amt,scale_perc, ebufp);
                                                pbo_decimal_multiply_assign(disc_amnt,scale_perc, ebufp);
					}
					else if(disc_amt == 1)
					{
						upd_amt = pbo_decimal_multiply(round_amount, scale_perc, ebufp);
						if(pbo_decimal_sign(disc_amnt, ebufp) > 0)
						{
							if(pbo_decimal_compare(upd_amt, pbo_decimal_abs(disc_amnt, ebufp), ebufp) < 0)
                                                	{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount amount is more than subscription amount");
                                                                disc_amnt = pbo_decimal_from_str("0.0",ebufp);
                                                                disc_amnt = pbo_decimal_add(upd_amt, disc_amnt, ebufp);
                                                                upd_amt = pbo_decimal_from_str("0.0",ebufp);
                                                                goto DISC;
                                                	}
							upd_amt = pbo_decimal_subtract(upd_amt, disc_amnt, ebufp);
						
						}
						else
						{
							upd_amt = pbo_decimal_subtract(disc_amnt, upd_amt, ebufp);
							pbo_decimal_abs_assign(upd_amt, ebufp);
						}
							
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No discount");
						upd_amt = pbo_decimal_multiply(round_amount, scale_perc, ebufp);
					}
					DISC:
					tolrnc_amt = pbo_decimal_from_str("0.0",ebufp);
					if(tax_flags && (*tax_flags & PIN_RATE_FLG_INVERTED))
                                        {
                                                upd_amt = (pin_decimal_t *)pbo_decimal_negate(upd_amt, ebufp);
                                                pbo_decimal_negate_assign(disc_amnt,ebufp);
                                        }
                                        PIN_FLIST_FLD_PUT(bal_imp_flistp, PIN_FLD_DISCOUNT, (void*)pbo_decimal_round( disc_amnt ,
                                                                                        precision, ROUND_HALF_UP, ebufp),ebufp);

					PIN_FLIST_FLD_SET(bal_imp_flistp, PIN_FLD_AMOUNT, (void*)pbo_decimal_round( upd_amt ,
                        								precision, ROUND_HALF_UP, ebufp),ebufp);
				}
				else
				{
					tax_amt = pbo_decimal_from_str("0.0",ebufp);
					scale_perc = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_PERCENT, 0, ebufp);
					tax_amt = pbo_decimal_multiply(upd_amt, scale_perc, ebufp);
					if(tax_flags && (*tax_flags & PIN_RATE_FLG_INVERTED))
                                        {
						if(pbo_decimal_sign(upd_amt, ebufp) > 0)
							upd_amt = (pin_decimal_t *)pbo_decimal_negate(upd_amt, ebufp);
                                        }
					PIN_FLIST_FLD_PUT(bal_imp_flistp, PIN_FLD_AMOUNT, (void*)pbo_decimal_round( tax_amt ,
                                                                                        precision, ROUND_HALF_UP, ebufp),ebufp);
					PIN_FLIST_FLD_PUT(bal_imp_flistp, PIN_FLD_QUANTITY, (void*)pbo_decimal_round( upd_amt ,
                                                                                        precision, ROUND_HALF_UP, ebufp),ebufp);
				}
			}
			PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
		}
			
	}
	if (!pbo_decimal_is_null(round_amount, ebufp))
        {
                pbo_decimal_destroy(&round_amount);
        }
	if (!pbo_decimal_is_null(payterm, ebufp))
        {
                pbo_decimal_destroy(&payterm);
        }
	if (!pbo_decimal_is_null(tolrnc_amt, ebufp))
        {
                pbo_decimal_destroy(&tolrnc_amt);
        }
	/*if (!pbo_decimal_is_null(zero_disc, ebufp))
        {
                pbo_decimal_destroy(&zero_disc);
        }*/
	if (!pbo_decimal_is_null(tax_amt, ebufp))
        {
                pbo_decimal_destroy(&tax_amt);
        }
	if (!pbo_decimal_is_null(disc_amnt, ebufp))
        {
                pbo_decimal_destroy(&disc_amnt);
        }
	SKIP:
	/**Function to Recalculate the charges for subscription product product with NRC table***/
	if (!(flags & PCM_OPFLG_CALC_ONLY) )
        {
		mso_adv_payment_chgs(ctxp, i_flistp, &mso_nrc_paymt_trnsfr_retflistp, svc_type, tax_flags, ebufp);
	}
	 /**************** calling mso_rate_pol_record_cur_bal *****************/
	 // Added to prevent the duplicate entried for mso_Account_balance_t
                 if (!(flags & PCM_OPFLG_CALC_ONLY) )
                {
                       
                        /******************************************************
                         * NT: Update Wallet balance
                         *****************************************************
                        pp_type = fm_mso_get_cust_type(ctxp, acct_pdp, ebufp);
                        if (pp_type == 0 && service_type == 0)
                        {
                                mso_update_wallet_balance(ctxp, i_flistp, ebufp);
                        }*/
                        mso_rate_pol_record_cur_bal(connp,i_flistp, svc_type, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
                        {
                            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "There is no balance to allow transaction", ebufp);
                            return;
                        }


                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_rate_pol_post_rating modified input flist", i_flistp);
						
				/*BEGIN: Change for NRC Object*/				
				if(mso_nrc_paymt_trnsfr_retflistp != NULL)
				{
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EVENT_NO, mso_nrc_paymt_trnsfr_retflistp, PIN_FLD_EVENT_NO, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nrc_paymt_trnsfr CREATE_OBJ input flist", mso_nrc_paymt_trnsfr_retflistp);					
					PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, mso_nrc_paymt_trnsfr_retflistp, &mso_nrc_paymt_trnsfr_outflistp, ebufp);	
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_CREATE_OBJ error in /mso_adv_payment_transfer", ebufp);
					}					
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ output flist", mso_nrc_paymt_trnsfr_outflistp);
					
					PIN_FLIST_DESTROY_EX(&mso_nrc_paymt_trnsfr_retflistp, NULL);	
					PIN_FLIST_DESTROY_EX(&mso_nrc_paymt_trnsfr_outflistp, NULL);
				}
				/*END: Change for NRC Object*/
            }
        }

	/***********************************************************
         * Just a straight copy of the input flist.
         ***********************************************************/
        r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

	/*******************************************************************
	 * Update CYCLE_INFO flags with event flag to prevent missing refund. 
	 ******************************************************************/
	cycle_info_flistp = PIN_FLIST_SUBSTR_GET(r_flistp, PIN_FLD_CYCLE_INFO, 1, ebufp);
	if (cycle_info_flistp && tax_flags && *tax_flags == 2056)
	{
		PIN_FLIST_FLD_SET(cycle_info_flistp, PIN_FLD_FLAGS, tax_flags, ebufp);
	}
	
	/***********************************************************
	 * Drop the Dropped call specific fields if present.
	 *
	 * - PIN_FLD_DROPPED_CALL_QUANTITY
	 * - PIN_FLD_CALL_TYPE
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_DROPPED_CALL_QUANTITY,
				1, ebufp);
	if (vp != NULL) {

		PIN_FLIST_FLD_DROP(r_flistp,
			PIN_FLD_DROPPED_CALL_QUANTITY, ebufp);
	}

	vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_CALL_TYPE, 1, ebufp);
	if (vp != NULL) {
		PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_CALL_TYPE, ebufp);
	}

	/***********************************************************
	 * Drop the EXTENDED_INFO field if present.
	 ***********************************************************/

	vp = PIN_FLIST_SUBSTR_GET(r_flistp, PIN_FLD_EXTENDED_INFO, 1, ebufp);
	if(vp) {
		PIN_FLIST_SUBSTR_DROP(r_flistp, PIN_FLD_EXTENDED_INFO, ebufp);
	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp)){

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_post_rating error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"return flist 1", r_flistp);
	
		/**************************************************************************
                  * Check if the event poid is of the type /event/session or its sub type, then remove the 
                  * PIN_FLD_QUANTITY field from the flist 
                  **************************************************************************/

		e_pdp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 0, ebufp);

	
		if (fm_utils_is_subtype(e_pdp, PIN_OBJ_TYPE_EVENT_SESSION) == 1) {
		
			quantity = (pin_decimal_t *)PIN_FLIST_FLD_TAKE(r_flistp, 
				PIN_FLD_QUANTITY, 1, ebufp);
			pbo_decimal_destroy(&quantity);
		}
		
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
                            
			/***************************************************
			* Log something and return nothing.
			***************************************************/
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"op_rate_pol_post_rating error while removing PIN_FLD_QUANTITY field", ebufp);

			PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
			*o_flistpp = NULL;                    
		 }
		 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"return flist 2", r_flistp);

		if(svc_pdp) {
			svc_type=(char *)PIN_POID_GET_TYPE(svc_pdp);
		    /*if(evt_type && (strcmp(evt_type,"/event/session/telco/broadband")==0) && (strcmp(svc_type,"/service/telco/broadband")==0)) {
		 	//vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp); 	
			//if (vp)
			//{	
			//	start_t = *(time_t *)vp;
			//	current_t = pin_virtual_time(NULL);
				count = fm_mso_purch_prod_status(ctxp, i_flistp, &pr_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        			"Error after calling fm_mso_purch_prod_status function");
				}
				 
	
				//if (current_t-start_t > 86400)
				if(count == 0)
				{
				  mso_fm_check_threshold( i_flistp, ctxp, ebufp);
				}
				else
				PIN_ERR_LOG_MSG(3,"Skipping threshold check");
			//}
		    }*/
		}
	
//		*o_flistpp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"return flist 3", r_flistp);
		
		//Setting the FIRST_USAGE flag to avoid the cache of balance while rating value Start 
		if (comm_succ_flag != 0) {
		    PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_FIRST_USAGE, &comm_succ_flag, ebufp);
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting First Usage Flag");
		}
		//Setting the FIRST_USAGE flag to avoid the cache of balance while rating value End 
		*o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_rate_pol_post_rating return flist", *o_flistpp);
	
	}
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	return;
}


static int32
mso_remove_custom_balImpact_taxation(
	pcm_context_t		*ctxp,
	pin_flist_t			**i_flistpp,
	pin_errbuf_t		*ebufp)
{

	//void			*vp = NULL;
	pin_cookie_t	cookie = NULL;
        poid_t          *service_pdp = NULL;	
	int32			rec_id = 0;
	//pin_cookie_t	cookie1 = NULL;
	//int32			rec_id1 = 0;
	//int32			bal_rec_id = 0;
	int				glid = 0;
	pin_flist_t		*balImpact_flistp = NULL;
	pin_flist_t		*newbalImpact_flistp = NULL;
	pin_flist_t		*t_flistp = NULL;
	pin_flist_t		*read_binfo_oflistp = NULL;

	poid_t			*billinfo_pdp = NULL;
	char			*binfo_id  = NULL;
	char			*rate_tag = NULL;
	char			*tax_code = NULL;
	char                    *service_type = NULL;

	int32			serv_type = -1;
	pin_decimal_t	*et_override_amt = NULL;

	et_override_amt = pbo_decimal_from_str("0.0",ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		  "mso_remove_custom_balImpact_taxation input flist", *i_flistpp);

	t_flistp = PIN_FLIST_CREATE(ebufp);
	t_flistp = PIN_FLIST_COPY(*i_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_remove_custom_balImpact_taxation processing flist", t_flistp);
    	service_pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if(service_pdp)
	{
		service_type = (char *)PIN_POID_GET_TYPE(service_pdp);
		if(service_type && strcmp(service_type, "/service/catv") == 0)
		{
			serv_type = 0;
		}
		else if (service_type && strcmp(service_type, "/service/telco/broadband") == 0)
		{
			serv_type = 1;
		}
	}
	else
	{
		billinfo_pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_BILLINFO_OBJ, 1, ebufp);
		if(billinfo_pdp)
		{
			fm_mso_utils_read_any_object(ctxp, billinfo_pdp, &read_binfo_oflistp, ebufp);
			if(read_binfo_oflistp)
			{
				binfo_id = PIN_FLIST_FLD_GET(read_binfo_oflistp, PIN_FLD_BILLINFO_ID, 0, ebufp);
				if(binfo_id && strcmp(binfo_id, "CATV") == 0)
				{
					serv_type = 0;
				}
				else if (binfo_id && strcmp(binfo_id, "BB") == 0)
				{
					serv_type = 1;
				}
			}
		}
	}	

	while ((balImpact_flistp = PIN_FLIST_ELEM_GET_NEXT(t_flistp, PIN_FLD_BAL_IMPACTS,
						&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		rate_tag = PIN_FLIST_FLD_GET(balImpact_flistp, PIN_FLD_RATE_TAG, 1, ebufp);
		tax_code = PIN_FLIST_FLD_GET(balImpact_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
		if (0==strcmp(rate_tag, "DUMMY_MSO_TAX"))
		{
			PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
		}
		else if ((strcmp(rate_tag, "Tax")==0) && 
			(((strcmp(tax_code, MSO_ST)==0) || (strcmp(tax_code, MSO_VAT_ST)==0) || (strcmp(tax_code, MSO_VAT_BB)==0) || (strcmp(tax_code, MSO_VAT_CATV)==0))
			|| is_charge_tax_code(ctxp, tax_code, ebufp)))
		{
			//Dropping the Dummy Element with the Amount as 0 and Rate Tag as Tax
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"MSO_Service_Tax Taxcode Matched");
			PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
		}
		else if ((strcmp(rate_tag, "MSO_ET")==0) && ((strcmp(tax_code, MSO_ET_MAIN)==0) || (strcmp(tax_code, MSO_ET_ADDI)==0)))
		{
			glid = GLID_ET;
			newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
			PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_AMOUNT, et_override_amt, ebufp);
			PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_AMOUNT_DEFERRED, et_override_amt, ebufp);
			PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Overwritten the ET FLIST", newbalImpact_flistp);
			PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
			/***** Commenting out as this bal impact is not necessary *****
			PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Overwritten the ET Amount");
			***************************************************************/
		}
		else if ((strcmp(rate_tag, "Tax")==0) && ((strcmp(tax_code, MSO_ET_MAIN)==0) || (strcmp(tax_code, MSO_ET_ADDI)==0)))
		{
			glid = GLID_ET;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ET Taxcode Matched, assigning ET GLID");
			newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
			PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
			PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
			PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id - 1) , ebufp);
		}
                else if ((strcmp(rate_tag, "Tax")==0) && strcmp(tax_code, IGST)==0)
                {
                        newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"IGST Taxcode Matched, assigning IGST GLID");
                        if (serv_type == 1) {
				glid = GLID_BB_IGST;
			}
			else{
                        	glid = GLID_IGST;
			}
                        PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
                        PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
                        PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id) , ebufp);
                }
                else if ((strcmp(rate_tag, "Tax")==0) && strcmp(tax_code, CGST)==0)
                {
                        newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"CGST Taxcode Matched, assigning CGST GLID");
			if (serv_type == 1) {
                                glid = GLID_BB_CGST;
                        }
                        else{
                                glid = GLID_CGST;
                        }

                        PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
                        PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
                        PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id) , ebufp);
                }
                else if ((strcmp(rate_tag, "Tax")==0) && strcmp(tax_code, SGST)==0)
                {
                        newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SGST Taxcode Matched, assigning SGST GLID");
			if (serv_type == 1) {
                                glid = GLID_BB_SGST;
                        }
                        else{
                                glid = GLID_SGST;
                        }
                        PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
                        PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
                        PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id) , ebufp);
                }
		else if (strcmp(tax_code, MSO_VAT)==0)
		{
			newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"VAT Taxcode Matched, assigning VAT GLID");
			glid = GLID_VAT;
			PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
			PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
		
			service_pdp = (poid_t *)PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
                  	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "service_pdp", service_pdp);
                  	service_type = (char *)PIN_POID_GET_TYPE(service_pdp);
                  	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "service_type:");
                  	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, service_type);
	                if (strcmp(service_type,"/service/telco/broadband")== 0) {
				
				PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
			
			}
			else {
				PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id - 1)  , ebufp);
			}
		}
		else if (strcmp(tax_code, SERVICE_TAX)==0)
		{
			newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ST Taxcode Matched, assigning ST GLID");
			glid = GLID_ST;
			PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
			PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
			PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id - 1) , ebufp);
		}
		else if (strcmp(tax_code, EDUCATION_CESS)==0)
		{
			newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ECess Taxcode Matched, assigning ECess GLID");
			glid = GLID_ECess;
			PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
			PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
			PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id - 1) , ebufp);
		}
		else if (strcmp(tax_code, SHE_CESS)==0)
		{
			newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"SHECess Taxcode Matched, assigning SHECess GLID");
			glid = GLID_SHECess;
			PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
			PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
			PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id - 1) , ebufp);
		}
		else if (strcmp(tax_code, SWACHH_CESS)==0)
                {
                        newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Swachh Cess Taxcode Matched, assigning Swachh Cess GLID");
                        glid = GLID_SwachhCess;
                        PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
                        PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
                        PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id - 1) , ebufp);
                }
		else if (strcmp(tax_code, KRISH_CESS)==0)
                {
                        newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Krish Cess Taxcode Matched, assigning Krish Cess GLID");
                        glid = GLID_KrishCess;
                        PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
                        PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
                        PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id - 1) , ebufp);
                }
		else if (strcmp(tax_code, ADDITIONAL_CESS)==0)
                {
                        newbalImpact_flistp = PIN_FLIST_COPY(balImpact_flistp, ebufp);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Addtional Cess Taxcode Matched, assigning Additional Cess GLID");
                        glid = GLID_AdditionalCess;
                        PIN_FLIST_FLD_SET(newbalImpact_flistp, PIN_FLD_GL_ID, &glid, ebufp);
                        PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
                        PIN_FLIST_ELEM_PUT(*i_flistpp, newbalImpact_flistp, PIN_FLD_BAL_IMPACTS, (rec_id - 1) , ebufp);
                }


	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		  "mso_remove_custom_balImpact_taxation original output flist", *i_flistpp);
	
	if (et_override_amt)
	{
		pbo_decimal_destroy(&et_override_amt);
	}
	PIN_FLIST_DESTROY_EX(&t_flistp, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Before Return of mso_remove_custom_balImpact_taxation");
	return serv_type;
}


static void
mso_adj_add_custom_balImpact_taxation(
	pcm_context_t		*ctxp,
	cm_nap_connection_t	*connp,	
	pin_flist_t		**i_flistpp,
	char			*evt_type,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*newbal_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*gst_rates_flistp = NULL;
	pin_flist_t		*initial_flistp = NULL;
	pin_flist_t		*tax_code_flistp = NULL;
	pin_flist_t		*event_flistp = NULL;
	pin_flist_t		*pp_flistp = NULL;

	
	pin_decimal_t		*amount = NULL;
	//pin_decimal_t		*st_tax_rate = NULL;
	//pin_decimal_t		*ecess_tax_rate = NULL;
	//pin_decimal_t		*shecess_tax_rate = NULL;
	pin_decimal_t		*st_amount = NULL;
	//pin_decimal_t		*ecess_amount = NULL;
	//pin_decimal_t		*shecess_amount = NULL;
	pin_decimal_t           *swachhcess_amount = NULL;
	pin_decimal_t 		*krish_amount = NULL;
	pin_decimal_t		*temp = NULL;
	pin_decimal_t		*hundred = NULL;
	pin_decimal_t		*exempt_percent = NULL;
	pin_decimal_t		*apply_percent = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*offering_obj = NULL;

	int			glid = 0;
	int			impact_type = 4;
	//int32			errp = 0;
	char			*name = NULL;
	char			*descr = NULL;
	int32			tax_exem_flag = -2;
        int32           	precision = 2;
	pin_flist_t		*exempt_out_flistp = NULL;
	
	int32			tax_type = 0;//service tax hanlded only now
	char			*rate_tag = 0;
	char			*rate_tag_1 = "Rate 1";
	char			*rate_tag_tax = "Tax";
	char			*actual_tax_code = NULL;
	char			tmp_buff[128];
	pin_decimal_t   	*igst_rate = NULL;
	pin_decimal_t   	*cgst_rate = NULL;
	pin_decimal_t   	*sgst_rate = NULL;
	pin_decimal_t		*igst_amount = NULL;
	pin_decimal_t		*sgst_amount = NULL;
	pin_decimal_t		*cgst_amount = NULL;
	
	cm_op_info_t		*opstackp = connp->coip;
	int32				stack_opcode = 0;
	int32				initial_opcode=0;

	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_adj_add_custom_balImpact_taxation input flist", *i_flistpp);

	acct_pdp = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	if ((evt_type &&
			((strcmp(evt_type, "/event/billing/adjustment/item") ==0) || 
			(strcmp(evt_type, "/event/billing/adjustment/account") ==0) || 
			(strcmp(evt_type, "/event/billing/dispute/item") ==0) || 
			(strcmp(evt_type, "/event/billing/settlement/item") ==0))))
	{
		/*while(opstackp != NULL)
		{
			stack_opcode = opstackp->opcode;
			memset(tmp_buff, '\0', sizeof(tmp_buff));
			sprintf(tmp_buff, "OPCODE=%d", stack_opcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"BEFORE PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);

			if(stack_opcode == MSO_OP_AR_DEBIT_NOTE)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_AR_DEBIT_NOTE", initial_flistp);
				actual_tax_code = PIN_FLIST_FLD_GET(opstackp->in_flistp, PIN_FLD_CHARGING_ID, 1, ebufp);
				//strcpy(actual_tax_code, "MSO_CATV_Adjustment");
			}
			else if(stack_opcode == MSO_OP_AR_ADJUSTMENT)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_AR_ADJUSTMENT", initial_flistp);
				//actual_tax_code = PIN_FLIST_FLD_GET(opstackp->in_flistp, PIN_FLD_CHARGING_ID, 1, ebufp);
				if(!actual_tax_code)
				{
					actual_tax_code = malloc(strlen("MSO_Adjustment")+1);
					strcpy(actual_tax_code, "MSO_Adjustment");
				}
			}
			initial_flistp = opstackp->in_flistp;
			initial_opcode = opstackp->opcode;
			opstackp = opstackp->next;
		}*/
		//
                actual_tax_code = malloc(strlen(MSO_ADJUSTMENT_TAX_CODE)+1);
                strcpy(actual_tax_code, MSO_ADJUSTMENT_TAX_CODE);
	}
	else if(evt_type && strcmp(evt_type, "/event/billing/mso_catv_commitment") == 0)
        {
		actual_tax_code = malloc(strlen(MIN_COMMITMENT_TAX_CODE)+1);
                strcpy(actual_tax_code, MIN_COMMITMENT_TAX_CODE);
        }
        else if(evt_type && strcmp(evt_type, "/event/billing/mso_cr_dr_note") == 0)
        {
		actual_tax_code = malloc(strlen(CN_DN_TAX_CODE)+1);
                strcpy(actual_tax_code, CN_DN_TAX_CODE);
        }
        else if(evt_type && strcmp(evt_type, "/event/billing/mso_penalty") == 0)
        {
		actual_tax_code = malloc(strlen(PENALTY_TAX_CODE)+1);
                strcpy(actual_tax_code, PENALTY_TAX_CODE);
        }
        else if(evt_type && strcmp(evt_type, "/event/billing/late_fee") == 0)
        {
		actual_tax_code = malloc(strlen(LATE_FEE_TAX_CODE)+1);
                strcpy(actual_tax_code, LATE_FEE_TAX_CODE);
        }
	else if(evt_type && strcmp(evt_type, "/event/mso_et") == 0)
	{
		/*event_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, 1, ebufp);
		if(event_flistp)
		{
			offering_obj = PIN_FLIST_FLD_GET(event_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);	
			if(offering_obj)
			{
				fm_mso_utils_read_any_object(ctxp, offering_obj, &pp_flistp, ebufp);
				plan_pdp = PIN_FLIST_FLD_GET(pp_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
				fm_mso_get_plan_tax_code(ctxp, plan_pdp, actual_tax_code, &tax_code_flistp, ebufp);
				if(tax_code_flistp)
				{
					actual_tax_code = PIN_FLIST_FLD_GET(pp_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
				}
			}
		}
	
		if(actual_tax_code == NULL)
		{*/
			actual_tax_code = malloc(strlen(ET_GST_TAX_CODE)+1);
               	 	strcpy(actual_tax_code, ET_GST_TAX_CODE);
		//}
	}

	PIN_ERR_LOG_MSG(3, "check 1");
	mso_retrieve_gst_rate(ctxp, actual_tax_code, acct_pdp, &gst_rates_flistp, ebufp);
	PIN_ERR_LOG_FLIST(3, "Tax Flistp", gst_rates_flistp);
	igst_rate = PIN_FLIST_FLD_GET(gst_rates_flistp, MSO_FLD_IGST_RATE, 1, ebufp);     //IGST
	sgst_rate = PIN_FLIST_FLD_GET(gst_rates_flistp, MSO_FLD_SGST_RATE, 1, ebufp);     //SGST
	cgst_rate = PIN_FLIST_FLD_GET(gst_rates_flistp, MSO_FLD_CGST_RATE, 1, ebufp);                     //CGST
	sgst_amount = pbo_decimal_from_str("0.0", ebufp);
	cgst_amount = pbo_decimal_from_str("0.0", ebufp);
	igst_amount = pbo_decimal_from_str("0.0", ebufp);

	bal_flistp = PIN_FLIST_ELEM_GET(*i_flistpp, PIN_FLD_BAL_IMPACTS, 0, 1, ebufp );
	amount = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_AMOUNT, 1,ebufp);
	rate_tag = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_RATE_TAG, 1,ebufp);
	if(rate_tag == NULL || strcmp(rate_tag, "") == 0)
		PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_RATE_TAG, rate_tag_1, ebufp);

	/* Customization for BB*/
	name = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_NAME, 1, ebufp);
	descr = PIN_FLIST_FLD_GET(*i_flistpp, PIN_FLD_DESCR, 1, ebufp);

	if (!pbo_decimal_is_zero(amount, ebufp))
	{
		// Calculating and Preparing the IGST Tax Balance Impact Element
		if(igst_rate && !pbo_decimal_is_zero(igst_rate, ebufp))
		{			
			newbal_flistp = PIN_FLIST_COPY(bal_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_adj_add_custom_balImpact_taxation tax flist", newbal_flistp);
			
			igst_amount = pbo_decimal_multiply(amount, igst_rate, ebufp);
			PIN_FLIST_FLD_PUT(newbal_flistp, PIN_FLD_AMOUNT,
				(void*)pbo_decimal_round(igst_amount, precision, ROUND_HALF_UP, ebufp), ebufp );

			if((name &&	((strcmp(name, "BB_CQ_BOUNCE_PENALTY")==0) || (strcmp(name, "BB_Custom_Late_Fee")==0) || (strcmp(name, "BB_CR_DR_NOTE")==0))) ||
				(descr && (strstr(descr, "Broadband")) || (strstr(descr, "Debit Note charges for :")))
			)
			{
				glid = GLID_BB_IGST;
			}
			else
			{
				glid = GLID_IGST;
			}
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_RATE_TAG, rate_tag_tax, ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_GL_ID, &glid, ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_IMPACT_TYPE, &impact_type, ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_TAX_CODE, "IGST", ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_PERCENT, igst_rate, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Updated IGST flist", newbal_flistp);
			PIN_FLIST_ELEM_SET(*i_flistpp, newbal_flistp, PIN_FLD_BAL_IMPACTS, 1, ebufp);
		}

		// Calculating and Preparing the SGST Tax Balance Impact Element
		if(sgst_rate && !pbo_decimal_is_zero(sgst_rate, ebufp))
		{
			newbal_flistp = PIN_FLIST_COPY(bal_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_adj_add_custom_balImpact_taxation igst tax flist", newbal_flistp);
			
			sgst_amount = pbo_decimal_multiply(amount, sgst_rate, ebufp);
			PIN_FLIST_FLD_PUT(newbal_flistp, PIN_FLD_AMOUNT,
					(void*)pbo_decimal_round(sgst_amount, precision, ROUND_HALF_UP, ebufp), ebufp );

			if((name && ((strcmp(name, "BB_CQ_BOUNCE_PENALTY")==0) || (strcmp(name, "BB_Custom_Late_Fee")==0) || (strcmp(name, "BB_CR_DR_NOTE")==0))) ||
				(descr && (strstr(descr, "Broadband")) || (strstr(descr, "Debit Note charges for :")))
			)
			{
				glid = GLID_BB_SGST;
			}
			else
			{
				glid = GLID_SGST;
			}
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_RATE_TAG, rate_tag_tax, ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_GL_ID, &glid, ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_IMPACT_TYPE, &impact_type, ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_TAX_CODE, "SGST", ebufp)
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_PERCENT, sgst_rate, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Updated SGST flist", newbal_flistp);
			PIN_FLIST_ELEM_SET(*i_flistpp, newbal_flistp, PIN_FLD_BAL_IMPACTS, 2, ebufp);
		}

		// Calculating and Preparing the CGST Tax Balance Impact Element
		if(cgst_rate && !pbo_decimal_is_zero(cgst_rate, ebufp))
		{
			newbal_flistp = PIN_FLIST_COPY(bal_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_adj_add_custom_balImpact_taxation sgst tax flist", newbal_flistp);
			cgst_amount = pbo_decimal_multiply(amount, cgst_rate, ebufp);
			PIN_FLIST_FLD_PUT(newbal_flistp, PIN_FLD_AMOUNT,
							(void*)pbo_decimal_round(cgst_amount, precision, ROUND_HALF_UP, ebufp), ebufp );

			if((name &&	((strcmp(name, "BB_CQ_BOUNCE_PENALTY")==0) || (strcmp(name, "BB_Custom_Late_Fee")==0) || (strcmp(name, "BB_CR_DR_NOTE")==0))) ||
				(descr && (strstr(descr, "Broadband")) || (strstr(descr, "Debit Note charges for :")))
			)
			{
				glid = GLID_BB_CGST;
			}
			else
			{
				glid = GLID_CGST;
			}
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_RATE_TAG, rate_tag_tax, ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_GL_ID, &glid, ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_IMPACT_TYPE, &impact_type, ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_TAX_CODE, "CGST", ebufp);
			PIN_FLIST_FLD_SET(newbal_flistp, PIN_FLD_PERCENT, cgst_rate, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Updated CGST flist", newbal_flistp);
			PIN_FLIST_ELEM_SET(*i_flistpp, newbal_flistp, PIN_FLD_BAL_IMPACTS, 3, ebufp);
		}
	}
		


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_adj_add_custom_balImpact_taxation updated tax flist", *i_flistpp);
	
	CLEANUP:
	if (!pbo_decimal_is_null(igst_amount, ebufp))
	{
		pbo_decimal_destroy(&igst_amount);
	}
	if (!pbo_decimal_is_null(cgst_amount, ebufp))
	{
		pbo_decimal_destroy(&cgst_amount);
	}
	if (!pbo_decimal_is_null(sgst_amount, ebufp))
	{
		pbo_decimal_destroy(&sgst_amount);
	}

	PIN_FLIST_DESTROY_EX(&gst_rates_flistp, NULL);

	return;
}

static void
mso_call_prov_cancel_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{

        int                     flg_cancel_plan = CATV_CANCEL_PLAN;
        pin_flist_t             *prov_iflist    = NULL;
        pin_flist_t             *products = NULL;
        pin_flist_t             *inp_products = NULL;
        pin_flist_t             *prov_oflist = NULL;
        pin_flist_t             *srch_in_flist = NULL;
        pin_flist_t             *srch_out_flist = NULL;
        pin_flist_t             *bb_info_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *action_flistp = NULL;
	pin_flist_t             *write_flds_inflistp = NULL;
	pin_flist_t		*write_flds_outflistp = NULL;	
	
	char			*svc_type = NULL;
	char			*prog_name = NULL;
	char			*template = "select X from /service/telco/broadband where F1 = V1 ";
	char			*template_ppo = "select X from /purchased_product where F1 = V1 ";
	char			*program_namep = "pin_cycle_fees";
	//char			*template_mso = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3";
	int32			srch_flag = 0;
	poid_t			*svc_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*prd_pdp = NULL;
	poid_t			*mso_pur_plan_pdp = NULL;
	int64			db = 1;
	int32			technology = 0;
	int32			*status_flags = NULL;
	int32			is_base_pack = -1;
	int32			add_prov = -1;
	char			*prov_tag = NULL;
	pin_flist_t		*ret_flistpp = NULL;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_call_prov_cancel_plan input flist", i_flistp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_PROGRAM_NAME,1,ebufp);
	action_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACTION_INFO, 0, 1, ebufp);
	if (action_flistp)
	{
		status_flags = PIN_FLIST_FLD_GET(action_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp);
		if (status_flags && (*status_flags == 0xFFF))
		{
			return;
		}
	}
        prov_iflist = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, prov_iflist, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, prov_iflist, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, prov_iflist, PIN_FLD_PROGRAM_NAME, ebufp);

        PIN_FLIST_FLD_SET(prov_iflist, PIN_FLD_FLAGS, &flg_cancel_plan, ebufp );
	products = PIN_FLIST_ELEM_ADD(prov_iflist, PIN_FLD_PRODUCTS, 0, ebufp );

        inp_products = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PRODUCT, 0, 0, ebufp);
        PIN_FLIST_FLD_COPY(inp_products, PIN_FLD_OFFERING_OBJ, products, PIN_FLD_POID, ebufp);
	prd_pdp = PIN_FLIST_FLD_GET(inp_products, PIN_FLD_PRODUCT_OBJ, 0, ebufp);

	svc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);

	if(prog_name && strstr(prog_name, "pin_cycle"))
	{	
		fm_mso_get_poid_details(ctxp, prd_pdp, &ret_flistpp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while returning from function : fm_mso_get_poid_details", ebufp);
				return;
		}
		prov_tag = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		if(prov_tag && strcmp(prov_tag, "") != 0)
		{
			add_prov = fm_validate_prov_ca_id(ctxp, svc_pdp, prov_tag, ebufp);
			if(!add_prov || add_prov == 2)
			{
				add_prov = 0;
        			PIN_FLIST_FLD_SET(prov_iflist, MSO_FLD_PKG_TYPE, &add_prov, ebufp );
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No need to go for cancel provisioning request");
				PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
				return;
			}

		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No prov_tag,No need to go for cancel provisioning request");
			PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
			return;
		}
		PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
	}
//		svc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		svc_type = (char*)PIN_POID_GET_TYPE(svc_pdp);

		if (strcmp(svc_type, bb_type) == 0)
		{
			srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
			srch_in_flist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_TEMPLATE, template, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, svc_pdp, ebufp);
			result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_RESULTS, 0, ebufp);
			bb_info_flistp = PIN_FLIST_ELEM_ADD(result_flistp, MSO_FLD_BB_INFO, 0, ebufp);
			PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_TECHNOLOGY, NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION search input flist for broad band service", srch_in_flist);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flist, &srch_out_flist, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION search output flist for broad band service", srch_out_flist);
			result_flistp = PIN_FLIST_ELEM_GET(srch_out_flist, PIN_FLD_RESULTS, 0, 1, ebufp);
			bb_info_flistp = PIN_FLIST_ELEM_GET(result_flistp, MSO_FLD_BB_INFO, 0, 1, ebufp);
			technology = *(int32 *)PIN_FLIST_FLD_GET(bb_info_flistp, MSO_FLD_TECHNOLOGY, 0, ebufp);

			if(technology == DOCSIS2 || technology == DOCSIS3) {
				flg_cancel_plan = DOC_BB_DEACTIVATE_PKG_EXP;
			} else {
				flg_cancel_plan = ETHERNET_BB_DEACTIVATE_PKG_EXP;
			}
			PIN_FLIST_DESTROY_EX(&srch_in_flist, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_out_flist, ebufp);

			// Get the /mso_purchased_plan object to pass in input flist.
			srch_in_flist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_TEMPLATE, template_ppo, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_COPY(inp_products, PIN_FLD_OFFERING_OBJ, args_flistp, PIN_FLD_POID, ebufp);
			result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION search input flist for plan obj ", srch_in_flist);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flist, &srch_out_flist, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION search output flist for plan obj", srch_out_flist);
			result_flistp = PIN_FLIST_ELEM_GET(srch_out_flist, PIN_FLD_RESULTS, 0, 1, ebufp);		
			plan_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);

			PIN_FLIST_DESTROY_EX(&srch_in_flist, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_out_flist, ebufp);


			srch_in_flist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_POID, srch_pdp, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);
			PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_TEMPLATE, template_ppo, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 1, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, args_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 2, ebufp);
			PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
			result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_RESULTS, 0, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION search input flist for mso purchased plan ", srch_in_flist);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flist, &srch_out_flist, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION search output flist for mso purchased plan ", srch_out_flist);
			result_flistp = PIN_FLIST_ELEM_GET(srch_out_flist, PIN_FLD_RESULTS, 0, 1, ebufp);		
			mso_pur_plan_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);
			
			PIN_FLIST_FLD_SET(prov_iflist, PIN_FLD_FLAGS, &flg_cancel_plan, ebufp );
			// TODO: Change PIN_FLD_PLAN_OBJ to the field name from Prov API
			PIN_FLIST_FLD_SET(prov_iflist, PIN_FLD_PLAN_OBJ, mso_pur_plan_pdp, ebufp );

		} else {

			PIN_FLIST_FLD_SET(prov_iflist, PIN_FLD_FLAGS, &flg_cancel_plan, ebufp );
		}
        
        
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION input flist", prov_iflist);
        PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION, 0, prov_iflist, &prov_oflist, ebufp);
        PIN_FLIST_DESTROY_EX(&prov_iflist, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION output flist", prov_oflist);
        PIN_FLIST_DESTROY_EX(&prov_oflist, ebufp);


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "MSO_OP_PROV_CREATE_ACTION error. Hence bailing out", ebufp);
		goto CLEANUP;
	}	
	 //Setting description as pin_cycle_fees if cancel by pin_cycle_fee
	if (prog_name && strstr(prog_name, "pin_cycle")) 
	{
		write_flds_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(inp_products, PIN_FLD_OFFERING_OBJ, write_flds_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_DESCR, program_namep, ebufp);	
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);
		 
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error in mso_call_prov_cancel_plan: wflds", ebufp);
		}

		PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
        }
	/****************************************************
	 * Check whether product is base pack or not
	 ****************************************************/
	is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp);
	if (is_base_pack == 0)
	{
		mso_cancel_alc_addon_pdts(ctxp, i_flistp, ebufp);
	}

CLEANUP:
        return;
}


void
mso_rate_pol_record_cur_bal(
        cm_nap_connection_t     *connp,
        pin_flist_t             *i_flistp,
	char                    *service_typep,
        pin_errbuf_t            *ebufp)
{

	cm_op_info_t    	*opstackp = connp->coip;
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t     *bal_flistp = NULL;
	pin_flist_t     *profile_flistp = NULL;
        pin_flist_t     *bal_oflistp = NULL;
        pin_flist_t     *wrt_iflp = NULL;
        pin_flist_t     *wrt_oflp = NULL;
        pin_flist_t     *rd_iflp = NULL;
        pin_flist_t     *rd_oflp = NULL;
	//pin_flist_t     *tmp_flp = NULL;
        pin_flist_t     *tmp1_flp = NULL;
        pin_flist_t     *flistp = NULL;
	pin_flist_t     *initial_flistp = NULL;
	int32		initial_opcode=0;


        poid_t          *cur_balpdp = NULL;
        poid_t          *event_pdp = NULL;
	poid_t          *acc_pdp = NULL;
        poid_t          *service_pdp = NULL;
        poid_t          *bi_pdp = NULL;


        int32           flags = 0;
        int32           lock = 0;
        int32           rec_id = 0;
	int32           pp_type = -1;
        pin_cookie_t    cookie = NULL;
        //void            *vp = NULL;

        pin_decimal_t   *previous_totalp = NULL;
        pin_decimal_t   *curr_balp = NULL;
        pin_decimal_t   *tmp_balp = NULL;
	pin_decimal_t   *tmp_disp = NULL;
        pin_decimal_t   *amountp = NULL;
	pin_decimal_t   *disp = NULL;
	pin_decimal_t   *parent_curr_balp = NULL;
	pin_decimal_t   *thresholdp = NULL;
        double          amount = 0;

        char            *action_name = NULL;
        char            *impact_type = NULL;
        char            *event_type = NULL;
        char            *service_type = NULL;
        char            msg[1024];
	char		*pymt_remarks = NULL;
	char		adj_descr[1024]="";

        //int64           event_no = 0;
        poid_t          *class_poid = NULL;

        char            *template = "select X from /balance_group where F1 = V1 ";
	char		*search_count_template = "select X from /mso_customer_credential where F1 = V1 ";
        poid_t          *search_pdp = NULL;
        poid_t          *pdp = NULL;
	poid_t		*billinfo_obj = NULL;
        int32           search_flags = SRCH_DISTINCT;
        int64           db_no = 0;
        pin_flist_t     *search_iflistp = NULL;
        pin_flist_t     *search_oflistp = NULL;
        pin_flist_t     *arg_flistp = NULL;
        pin_flist_t     *result_flistp = NULL;
	pin_flist_t	*read_flds_iflistp = NULL;
	pin_flist_t	*read_flds_oflistp = NULL;
	pin_flist_t	*mso_cred_iflistp = NULL;
	pin_flist_t	*mso_cred_oflistp = NULL;
	pin_flist_t	*inc_fields_iflistp = NULL;
	pin_flist_t	*inc_fields_oflistp = NULL;
	pin_flist_t	*mso_cred_results_oflistp = NULL;

	char            poid_buf[PCM_MAX_POID_TYPE + 1];
        char            *p = NULL;
        int32           len = 0;
        char            *token = NULL;
	char		*program_name = NULL;
	char		tmp_buff[128];
	int32		act_usage_flag = -1;
	int64		inc_counter = 1;
	int64		*rank = NULL;
	int32		stack_opcode=0;
	char		*action = NULL;
	int32		*resource_id = NULL;
	int32       	*business_typep = NULL;

	void		*vp = NULL;
	void		*vp1 = NULL;

		
	poid_t          *reservation_pdp = NULL;
		

	// Added for payment remarks 
	char 		*chq_no = NULL;
	char 		*bank_name = NULL;
	time_t 		eff_t = 0;
	char  		pymt_descr[255] = "";
	struct 		tm  lc;
	char 		chq_dt[30] = "";
	char		*descr = NULL;
	pin_flist_t	*check_flistp = NULL;
	time_t          current_t = 0;
	int32		hardware_flag = 0;

        pin_flist_t     *total_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"mso_rate_pol_record_cur_bal error",ebufp);
                return;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_rate_pol_record_cur_bal input Flist", i_flistp);


        /**********************************************************
         * Record the current balance of the account in
         * /mso_account_balance object
         **********************************************************/
         /* Check if the event if having any balance impact then
            only proceed                                         */
	descr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	action_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp );
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
        if(!PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_BAL_IMPACTS, ebufp))
                return;

        // Find all the balance groups of this account
        search_iflistp = PIN_FLIST_CREATE(ebufp);
        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db_no = PIN_POID_GET_DB(pdp);
        search_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
        PIN_FLIST_FLD_SET(search_iflistp,PIN_FLD_POID,(void *)search_pdp,ebufp);
        PIN_FLIST_FLD_SET(search_iflistp,PIN_FLD_FLAGS,&search_flags,ebufp);
        PIN_FLIST_FLD_SET(search_iflistp,PIN_FLD_TEMPLATE,template,ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(search_iflistp,PIN_FLD_ARGS,1,ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, arg_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(search_iflistp,PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Balance groups input flist",search_iflistp);
        PCM_OP(ctxp,PCM_OP_SEARCH, 0, search_iflistp, &search_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Balance groups output flist",search_oflistp);
	PIN_FLIST_DESTROY_EX(&search_iflistp, NULL);
	program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

/* Comment previous logic of finding balance groups
        bal_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, bal_flistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BAL_GET_ACCT_BAL_GRP_AND_SVC input flist", bal_flistp);
        PCM_OP(ctxp, PCM_OP_BAL_GET_ACCT_BAL_GRP_AND_SVC, 0, bal_flistp, &bal_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&bal_flistp, NULL);
        if(PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting balance groups", ebufp);
                PIN_ERRBUF_CLEAR(ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BAL_GET_ACCT_BAL_GRP_AND_SVC return flist", bal_oflistp);
*/


        while ((flistp = PIN_FLIST_ELEM_GET_NEXT(search_oflistp,
                PIN_FLD_RESULTS, &rec_id, 1, &cookie,
                ebufp)) != (pin_flist_t *)NULL) {
                // Find balances of all the balance groups
                bal_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, bal_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_COPY(flistp, PIN_FLD_POID, bal_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(flistp, PIN_FLD_BILLINFO_OBJ, i_flistp, PIN_FLD_BILLINFO_OBJ, ebufp);
                PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_FLAGS, &flags, ebufp);
                PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_READ_BALGRP_MODE, &lock, ebufp);
                PIN_FLIST_ELEM_SET(bal_flistp,NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BAL_GET_BALANCES input flist", bal_flistp);
                PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES, 0, bal_flistp, &tmp1_flp, ebufp);
                PIN_FLIST_DESTROY_EX(&bal_flistp, NULL);
                if(PIN_ERRBUF_IS_ERR(ebufp)){
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting current balance", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BAL_GET_BALANCES return flist", tmp1_flp);
                bal_flistp = PIN_FLIST_ELEM_GET(tmp1_flp, PIN_FLD_BALANCES, PIN_CURRENCY_INR, 1, ebufp);
                if(bal_flistp){
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "bal_flistp is not null");
                    if( pbo_decimal_is_null(previous_totalp, ebufp)){
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "previous_totalp is null");
                        previous_totalp = pbo_decimal_copy(PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CURRENT_BAL, 1, ebufp), ebufp);
                    }
                    else{
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "previous_totalp is not null");
                        pbo_decimal_add_assign(previous_totalp, PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CURRENT_BAL, 1, ebufp), ebufp);
                    }
                }
        }
	PIN_FLIST_DESTROY_EX(&bal_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_oflistp, NULL);


        rec_id = 0;
        cookie = NULL;

        if(!pbo_decimal_is_null(previous_totalp, ebufp)){
                sprintf(msg, "Copying previous_totalp(%f) to curr_balp",  (double)pbo_decimal_to_double(previous_totalp, ebufp));
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
            curr_balp = pbo_decimal_copy(previous_totalp, ebufp);
        }


    /************************************************************
     * Add the current balance as it will not be commited to DB
     * till now
     ***********************************************************/
    while ((flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,
            PIN_FLD_BAL_IMPACTS, &rec_id, 1, &cookie,
            ebufp)) != (pin_flist_t *)NULL) {
                tmp_balp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_AMOUNT, 1, ebufp);
		// considering discount also to capture in account statement
		tmp_disp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DISCOUNT, 1, ebufp);
		// get resource id and consider only for INR to fix bug for usage charing
		resource_id = PIN_FLIST_FLD_GET(flistp, PIN_FLD_RESOURCE_ID, 1, ebufp);
		if(resource_id && *resource_id  == 356)
		{
			if(tmp_balp){
				if( pbo_decimal_is_null(amountp, ebufp)){
					amountp = pbo_decimal_copy(tmp_balp, ebufp);
					sprintf(msg, "Copying tmp_balp(%f) to accountp",  (double)pbo_decimal_to_double(tmp_balp, ebufp));
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				}
				else{
					pbo_decimal_add_assign(amountp, tmp_balp, ebufp);
					sprintf(msg, "Added tmp_balp(%f) to amountp(%f)",  (double)pbo_decimal_to_double(tmp_balp, ebufp), (double)pbo_decimal_to_double(amountp, ebufp));
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

				}
			}
			
			// considering discount also to capture in account statement
			if(tmp_disp){
				if( pbo_decimal_is_null(disp, ebufp)){
					disp = pbo_decimal_copy(tmp_disp, ebufp);
					sprintf(msg, "Copying tmp_disp(%f) to accountp",  (double)pbo_decimal_to_double(tmp_disp, ebufp));
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
				}
				else{
					pbo_decimal_add_assign(disp, tmp_disp, ebufp);
					sprintf(msg, "Added tmp_balp(%f) to amountp(%f)",  (double)pbo_decimal_to_double(tmp_disp, ebufp), (double)pbo_decimal_to_double(disp, ebufp));
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

				}
			}

		}

        }

        if(PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while getting balances", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                return;
        }
        if( pbo_decimal_is_null(curr_balp, ebufp)){
                curr_balp = pbo_decimal_copy(amountp, ebufp);
                sprintf(msg, "Copying amounpt(%f) to curr_balp",  (double)pbo_decimal_to_double(amountp, ebufp));
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
        }
        else{
                pbo_decimal_add_assign(curr_balp, amountp, ebufp);
                sprintf(msg, "Added amountp(%f) to curr_balp(%ld)",  (double)pbo_decimal_to_double(amountp, ebufp), (double)pbo_decimal_to_double(curr_balp, ebufp));
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
        }

        if(PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while getting balances1", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                return;
        }

	if ((pbo_decimal_sign(amountp, ebufp) == 1) && service_typep && strstr(service_typep, "/service/catv") &&
                program_name && !strcasestr(program_name, "MYJIO") && !strcasestr(program_name, "HybridScheme"))
        {
            fm_mso_utils_get_profile(ctxp, acc_pdp, &profile_flistp, ebufp);
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Customer profile return flist", profile_flistp);
            if (profile_flistp)
            {
                business_typep = (int32 *)PIN_FLIST_FLD_GET(profile_flistp, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
                pp_type = *(int32 *)PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_PP_TYPE, 0, ebufp);

                if (pp_type == 0 && business_typep && *business_typep == 13000000)
                {
                    PIN_FLIST_DESTROY_EX(&profile_flistp, NULL);
                    bi_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILLINFO_OBJ, 1, ebufp);
                    fm_mso_read_billinfo(ctxp, bi_pdp, &profile_flistp, ebufp);
                    bi_pdp = PIN_FLIST_FLD_GET(profile_flistp, PIN_FLD_AR_BILLINFO_OBJ, 1, ebufp);
                    parent_curr_balp = get_ar_curr_total(ctxp, bi_pdp, ebufp);
                    PIN_FLIST_DESTROY_EX(&profile_flistp, NULL);
                    if (!pbo_decimal_is_null(parent_curr_balp, ebufp))
                    {
                        thresholdp = pbo_decimal_from_str("-0.10", ebufp);
                        pbo_decimal_add_assign(parent_curr_balp, thresholdp, ebufp);
                    }
                    if (pbo_decimal_sign(parent_curr_balp, ebufp) == 1 || pbo_decimal_is_null(parent_curr_balp, ebufp))
                    {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error: There is no balance to allow transaction", ebufp);
                        pbo_decimal_destroy(&parent_curr_balp);
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_CURRENT_BAL, 0, 0);
                        return;
                    }
                    else if (pbo_decimal_compare(amountp, pbo_decimal_abs(parent_curr_balp, ebufp), ebufp) > 0)
                    {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error: There is no balance to allow transaction", ebufp);
			pbo_decimal_destroy(&parent_curr_balp);
                        pbo_decimal_destroy(&thresholdp);
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_VALUE, PIN_FLD_CURRENT_BAL, 0, 0);
                        return;
                    }
                }
                else
                {
                    PIN_FLIST_DESTROY_EX(&profile_flistp, NULL);
                }
            }
        }
	
        // Now get Account no from /account
        rd_iflp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, rd_iflp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(rd_iflp, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist", rd_iflp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rd_iflp, &rd_oflp, ebufp);
        PIN_FLIST_DESTROY_EX(&rd_iflp, NULL);
        if(PIN_ERRBUF_IS_ERR(ebufp)){
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while creating /mso_account_balance", ebufp);
            PIN_ERRBUF_RESET(ebufp);
            return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS return flist", rd_oflp);

        action_name = pin_malloc(60);
        memset(action_name, '\0', sizeof(action_name));

        amount = (double)pbo_decimal_to_double(amountp, ebufp);
        impact_type = pin_malloc(20);
        memset(impact_type, '\0', sizeof(impact_type));
        (amount < 0)? strcpy(impact_type,"CREDIT"):strcpy(impact_type,"DEBIT");

        event_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        event_type = pin_malloc(100);
        memset(event_type, '\0', sizeof(event_type));
        strcpy(event_type, PIN_POID_GET_TYPE(event_pdp));
        service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if(service_pdp)
	{
		service_type = pin_malloc(200);
		memset(service_type, '\0', sizeof(service_type));
		strcpy(service_type, PIN_POID_GET_TYPE(service_pdp));
	}

// Check opcode & get action name
//	pin_flist_t             *initial_flistp,
//	int32			initial_opcode,
//	if(initial_opcode == )


	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "event type is :");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, event_type);
	if(event_type && ((strstr(event_type,"/event/billing/product/fee/cycle/cycle_arrear") != NULL) ||
		(strstr(event_type,"/event/billing/product/fee/cycle/cycle_forward") != NULL) ||
		(strstr(event_type,"/event/billing/product/fee/purchase") != NULL)))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "cycle or purchase fee event");
		
		if(event_type && strstr(event_type, AMC_EVENT))
		{

			opstackp = connp->coip;
                        while(opstackp != NULL)
                        {
                                stack_opcode = opstackp->opcode;
                                memset(tmp_buff, '\0', sizeof(tmp_buff));
                                sprintf(tmp_buff, "OPCODE=%d", stack_opcode);
                                if(stack_opcode == PCM_OP_ACT_USAGE)
                                {
                                        act_usage_flag = opstackp->opflags;
                                        sprintf(tmp_buff, "PCM_OP_ACT_USAGE FLAG=%d", act_usage_flag);
                                        memset(tmp_buff, '\0', sizeof(tmp_buff));
                                }
                                initial_flistp = opstackp->in_flistp;
			PIN_ERR_LOG_FLIST(3,"stack flist",initial_flistp);
                                initial_opcode = opstackp->opcode;

                                opstackp = opstackp->next;
                        }
                        if(act_usage_flag == 896 || act_usage_flag == 384)
                        {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"AMC EVENT FOUND BUT NOT NOW");
                                return;
                        }
			else
			{
				 strcpy(action_name, "CPE CHARGES");
				 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"AMC EVENT FOUND");
			}

		}
		if(event_type && strstr(event_type, "mso_hw"))
		{
			hardware_flag = 1;
		}

		if (program_name && strstr(program_name,"pin_cycle_forward"))
		{
			strcpy(action_name, "RENEW PLAN");
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PROGRAM_NAME, "Reactivate Plan", ebufp);
		}
		else if ((program_name) && ((strstr(program_name,"pin_cycle")) || (strstr(program_name,"bill_accts")))
				   && (event_type && !strstr(event_type, AMC_EVENT)))
		{
			strcpy(action_name, "BILLING");
		}
		else if(event_type && !strstr(event_type, AMC_EVENT))
		{
			opstackp = connp->coip;
			while(opstackp != NULL)
			{
				stack_opcode = opstackp->opcode;
				memset(tmp_buff, '\0', sizeof(tmp_buff));
				sprintf(tmp_buff, "OPCODE=%d", stack_opcode);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"BEFORE PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
				if(stack_opcode == PCM_OP_ACT_USAGE)
				{
					act_usage_flag = opstackp->opflags;
					sprintf(tmp_buff, "PCM_OP_ACT_USAGE FLAG=%d", act_usage_flag);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
					memset(tmp_buff, '\0', sizeof(tmp_buff));
				}
				initial_flistp = opstackp->in_flistp;
				initial_opcode = opstackp->opcode;
				
				opstackp = opstackp->next;
			}
			if(act_usage_flag == 896 || act_usage_flag == 384)
			{
				return;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Intial opcode input FLIST", initial_flistp);
			if ((initial_opcode == MSO_OP_PROV_BB_PROCESS_RESPONSE))
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"MSO_OP_PROV_BB_PROCESS_RESPONSE input FLIST",initial_flistp);
				program_name = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
				action = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_ACTION, 0, ebufp);
				if ((action) && strstr(action, "ACTIVATE_SUBSCRIBER"))
				{
					strcpy(action_name, "ACTIVATION");				
				}
				else if((action) && strstr(action, "CHANGE_SERVICE"))
				{
					strcpy(action_name, "CHANGE PLAN");				
				}
				else if((action) && strstr(action, "RENEW_SUBSCRIBER"))
				{
					strcpy(action_name, "RENEW PLAN");				
				}
				else if((action) && strstr(action, "TOPUP_WO_PLAN"))
				{
					strcpy(action_name, "TOPUP PLAN");				
				}
				else if((action) && strstr(action, "HOLD_SUBSCRIBER"))
				{
					strcpy(action_name, "HOLD SERVICE");				
				}
				else if((action) && strstr(action, "UNHOLD_SERVICE"))
				{
					strcpy(action_name, "UNHOLD SERVICE");				
				}
				else if((action) && strstr(action, "SUSPEND_SUBSCRIBER"))
				{
					strcpy(action_name, "SUSPEND SERVICE");				
				}
				else if((action) && strstr(action, "TERMINATE_SUSBCRIBER"))
				{
					strcpy(action_name, "TERMINATE SERVICE");				
				}
				else if((action) && strstr(action, "DEACTIVATE_SUBSCRIBER"))
				{
					strcpy(action_name, "DEACTIVATE SERVICE");				
				}
				else if((action) && strstr(action, "DEACTIVATE_SUBSCRIBER"))
				{
					strcpy(action_name, "DEACTIVATE SERVICE");				
				}
				else
				{
				
					strcpy(action_name, "MISCELLANEOUS");
				}
					
				
			}		
			else if((initial_opcode == MSO_OP_CUST_ACTIVATE_CUSTOMER)  ||(initial_opcode ==  MSO_OP_CUST_ACTIVATE_BB_SERVICE))
			{
				 strcpy(action_name, "ACTIVATION");
				 if((service_type) && (strstr(service_type,"broadband")))
				 {
					strcpy(action_name, "ACTIVATION");
				 }
			}
			else if ((initial_opcode == MSO_OP_CUST_ADD_PLAN) || (initial_opcode == MSO_OP_INTEG_ADD_PLAN))
			{
				program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
				if ( program_name && strcmp(program_name, "STPK_REFUND")  == 0){
					strcpy(action_name, "CANCEL PLAN");
				}
				else{
					strcpy(action_name, "ADD PLAN");
				}
				 if((service_type) && (strstr(service_type,"broadband")))
				 {
					//return;
				 }
			}
			else if ((initial_opcode == MSO_OP_CUST_CHANGE_PLAN) || (initial_opcode == MSO_OP_INTEG_CHANGE_PLAN) )
			{
				 if(hardware_flag == 1)
                                 	strcpy(action_name, "CYCLE CHARGE");
				 else
				 	strcpy(action_name, "CHANGE PLAN");
			}
			else if ((initial_opcode == MSO_OP_CUST_CALC_PRICE))
			{
				 strcpy(action_name, "PRICE CALC");
				return;
			}
			else if ((initial_opcode == MSO_OP_CUST_CANCEL_PLAN) || (initial_opcode == MSO_OP_INTEG_CANCEL_PLAN))
			{
				 strcpy(action_name, "CANCEL PLAN");
			}
			else if ((initial_opcode == MSO_OP_CUST_SUSPEND_SERVICE) || (initial_opcode == MSO_OP_INTEG_SUSPEND_SERVICE))
			{
				 strcpy(action_name, "SUSPEND SERVICE");
			}
			else if ((initial_opcode == MSO_OP_CUST_REACTIVATE_SERVICE) || (initial_opcode == MSO_OP_INTEG_REACTIVATE_SERVICE))
			{
				 strcpy(action_name, "REACTIVATE SERVICE");
			}
			else if (initial_opcode == MSO_OP_CUST_TERMINATE_SERVICE)
			{
				 strcpy(action_name, "TERMINATE SERVICE");
			}
			else if (initial_opcode == MSO_OP_CUST_ALLOCATE_HARDWARE)
			{
				 strcpy(action_name, "HARDWARE ALLOCATION");			
			}
			else if (initial_opcode == MSO_OP_CUST_HOLD_SERVICE)
			{
				 strcpy(action_name, "HOLD SERVICE");			
			}
			else if (initial_opcode == MSO_OP_CUST_UNHOLD_SERVICE)
			{
				 strcpy(action_name, "UNHOLD SERVICE");
				 if((service_type) && (strstr(service_type,"broadband")))
				 {
					return;
				 }
			}
			else if (initial_opcode == MSO_OP_CUST_EXTEND_VALIDITY)
			{
				 strcpy(action_name, "VALIDITY EXTENSION");				 
			}
			else if (initial_opcode == MSO_OP_CUST_TOP_UP_BB_PLAN)
			{
				 if((service_type) && (strstr(service_type,"broadband")))
				 {
					if(hardware_flag == 1)
                                                strcpy(action_name, "CYCLE CHARGE");
                                        else
                                         	return;
				 }
				 else
				  strcpy(action_name, "TOPUP PLAN");
			}
			else if (initial_opcode == MSO_OP_CUST_RENEW_BB_PLAN)
			{
				 if((service_type) && (strstr(service_type,"broadband")))
				 {
					if(hardware_flag == 1)
						strcpy(action_name, "CYCLE CHARGE");
					else
						return;
				 }
				 else
			       	  strcpy(action_name, "RENEW PLAN");
			}
			else if (initial_opcode == MSO_OP_CUST_DEACTIVATE_BB_SERVICE)
			{
				 strcpy(action_name, "DEACTIVATE SERVICE");			
			}
			else if (initial_opcode == MSO_OP_CUST_SWAP_BB_DEVICE)
			{
				 strcpy(action_name, "DEVICE SWAP");			
			}
			else if (initial_opcode == MSO_OP_GRV_UPLOAD)
			{
				 strcpy(action_name, "GRV UPLOAD");			
			}
			else if (initial_opcode == MSO_OP_GRV_UPLOAD)
			{
				 strcpy(action_name, "GRV UPLOAD");			
			}
			else
			{
				strcpy(action_name, "CYCLE CHARGE");			
			}
		}

	}
	else if(event_type && strstr(event_type,"/event/mso_et") != NULL)
	{
		strcpy(action_name, "ENTERTAINMENT TAX");
	}
	else if(event_type && strstr(event_type,"/event/billing/payment") != NULL)
	{

		opstackp = connp->coip;
		while(opstackp != NULL)
		{
			stack_opcode = opstackp->opcode;
			memset(tmp_buff, '\0', sizeof(tmp_buff));
			sprintf(tmp_buff, "OPCODE=%d", stack_opcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"BEFORE PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
			if(stack_opcode == PCM_OP_ACT_USAGE)
			{
				act_usage_flag = opstackp->opflags;
				sprintf(tmp_buff, "PCM_OP_ACT_USAGE FLAG=%d", act_usage_flag);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
				memset(tmp_buff, '\0', sizeof(tmp_buff));
			}
			initial_flistp = opstackp->in_flistp;
			initial_opcode = opstackp->opcode;
			if (initial_opcode == MSO_OP_PYMT_COLLECT)
			{
				check_flistp = opstackp->in_flistp; 	
			}
			opstackp = opstackp->next;
		}		
		if(act_usage_flag == 896)
		{
			return;	
		}
		if(initial_opcode == MSO_OP_PYMT_COLLECT || initial_opcode == MSO_OP_PYMT_LCO_COLLECT )
		{
		// Need to get the payment remarks
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "initial opcode flist", initial_flistp);
			chq_no = PIN_FLIST_FLD_GET(check_flistp, PIN_FLD_CHECK_NO, 1, ebufp);
			if ( chq_no )
			{	
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entering chque block");
				pymt_remarks = PIN_FLIST_FLD_GET(check_flistp, PIN_FLD_DESCR, 1, ebufp);
				bank_name = PIN_FLIST_FLD_GET(check_flistp, PIN_FLD_BANK_NAME, 1, ebufp);
				eff_t = *(time_t *) PIN_FLIST_FLD_GET(check_flistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);
				lc = *localtime(&eff_t);
				strftime(chq_dt, 80, "%x", &lc);
				sprintf(pymt_descr,"BN:%s CN: %s DT: %s RM :%s", bank_name, chq_no, chq_dt, pymt_remarks);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pymt_descr);
			}
			else 
			{
				pymt_remarks = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_DESCR, 1, ebufp);
			}
		}
		strcpy(action_name, "PAYMENT");
	}
	else if(event_type && !strcmp(event_type, "/event/billing/adjustment/event")){
		if(act_usage_flag == 896){
			return;
		}

		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SYS_DESCR, 0, ebufp );
		if(vp){
			sprintf(adj_descr, "%s", (char *)vp );
		}
		/*vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp );
		if( vp ){
			strcpy(action_name, (char *)vp );
		}*/
		strcpy(action_name, "RERATING");
	}
	else if(event_type && strstr(event_type,"/event/billing/adjustment") != NULL)
	{

		opstackp = connp->coip;
		while(opstackp != NULL)
		{
			stack_opcode = opstackp->opcode;
			memset(tmp_buff, '\0', sizeof(tmp_buff));
			sprintf(tmp_buff, "OPCODE=%d", stack_opcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"BEFORE PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
			if(stack_opcode == PCM_OP_ACT_USAGE)
			{
				act_usage_flag = opstackp->opflags;
				sprintf(tmp_buff, "PCM_OP_ACT_USAGE FLAG=%d", act_usage_flag);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
				memset(tmp_buff, '\0', sizeof(tmp_buff));
			}
			initial_flistp = opstackp->in_flistp;
			initial_opcode = opstackp->opcode;
	 	        program_name = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_PROGRAM_NAME,1, ebufp);
			
			opstackp = opstackp->next;
		}		
		if(act_usage_flag == 896)
		{
			return;	
		}
	
                if (descr && strstr(descr, "Debit Note charges for :"))
		{
			PIN_ERR_LOG_MSG(3, "Action for Debit Note");
                        strcpy(action_name, "DEBIT NOTE");
			sprintf(adj_descr, "%s", descr);
		}
                else if (descr && strstr(descr, "PROCESS REFUND REVERSE"))
                {
			PIN_ERR_LOG_MSG(3, "Action for REFUND REVERSE");
                        strcpy(action_name, "REFUND DEPOSIT/REVERSAL");
                        sprintf(adj_descr, "%s", descr);
                }
                else if (descr && strstr(descr, "PROCESS REFUND:"))
                {
			PIN_ERR_LOG_MSG(3, "Action for REFUND");
                        strcpy(action_name, "REFUND LOADING");
                        sprintf(adj_descr, "%s", descr);
                }
		else 
		{
			if(initial_opcode == MSO_OP_AR_ADJUSTMENT || 
				(program_name && !strcmp(program_name,"pin_apply_bulk_adjustment") ))
			{
				// Need to get the payment remarks
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Initial flistp to check adj descr", initial_flistp);
				vp1 = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_DESCR, 0, ebufp);
				vp = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_REASON_CODE, 1, ebufp);
				if (vp)
				{
					sprintf(adj_descr, "Adjustment applied for:%s: %s", (char*)vp, (char*)vp1);
				}
				else
				{
					vp = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_REASON_ID, 1, ebufp);
					/* Adjustment Reasons
					  1. Customer not satisfied with service
					  2. Customer unaware of charges
					  3. Debited account by mistake
					  4. Technical and support charges
					  5. Service charges
					  6. Others	
					*/
					if (vp && *(int32*)(vp) == 1)
					{
						 sprintf(adj_descr, "Adjustment applied for:Settlement against Retention: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 2)
					{
						 sprintf(adj_descr, "Adjustment applied for:Double Top Up: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 3)
					{
						 sprintf(adj_descr, "Adjustment applied for:Plan Change Done: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 4)
					{
						 sprintf(adj_descr, "Adjustment applied for:Wrongly Activated&Terminated: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 5)
					{
						 sprintf(adj_descr, "Adjustment applied for:Non Usage: %s", (char*)vp1);
					}			
					else if (vp && *(int32*)(vp) == 6)
					{
						 sprintf(adj_descr, "Adjustment applied for:Auto Activation Done: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 7)
					{
						sprintf(adj_descr, "Adjustment applied for:Delay In Plan Change: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 8)
					{
						sprintf(adj_descr, "Adjustment applied for:Wrong Plan Given: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 9)
					{
						 sprintf(adj_descr, "Adjustment applied for:New Wi-Fi router deposit - NWIFID: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 10)
					{
						sprintf(adj_descr, "Adjustment applied for:TDS  - TDS: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 11)
					{
						sprintf(adj_descr, "Adjustment applied for:Penalty - Penalty: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 12)
					{
						sprintf(adj_descr, "Adjustment applied for:CPE charges - CPEADJ: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 13)
					{
						 sprintf(adj_descr, "Adjustment applied for:LPF Charges - LPEADJ: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 14)
					{
						sprintf(adj_descr, "Adjustment applied for:Static IP - AMC: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 15)
					{
						sprintf(adj_descr, "Adjustment applied for:Modem Deposit - DEP: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 16)
					{
						sprintf(adj_descr, "Adjustment applied for:Additional uses waiver - AUCW: %s", (char*)vp1);
					}
					else if (vp && *(int32*)(vp) == 17)
                                        {
                                                 sprintf(adj_descr, "Adjustment applied for:CrNoteagainstDrNote(HO): %s", (char*)vp1);
                                        }
                                        else if (vp && *(int32*)(vp) == 18)
                                        {
                                                sprintf(adj_descr, "Adjustment applied for:Discount related: %s", (char*)vp1);
                                        }
                                        else if (vp && *(int32*)(vp) == 19)
                                        {
                                                sprintf(adj_descr, "Adjustment applied for:Initial payment refund(Subscribition): %s", (char*)vp1);
                                        }
                                        else if (vp && *(int32*)(vp) == 20)
                                        {
                                                sprintf(adj_descr, "Adjustment applied for:Cr Note for GPON migration: %s", (char*)vp1);
                                        }
                                        else if (vp && *(int32*)(vp) == 21)
                                        {
                                                sprintf(adj_descr, "Adjustment applied for:Service Issue: %s", (char*)vp1);
                                        }
                                        else if (vp && *(int32*)(vp) == 22)
                                        {
                                                sprintf(adj_descr, "Adjustment applied for:GST: %s", (char*)vp1);
                                        }
                                        else if (vp && *(int32*)(vp) == 23)
                                        {
                                                sprintf(adj_descr, "Adjustment applied for:Cheque bounce billing reversal: %s", (char*)vp1);
                                        }
                                        else if (vp && *(int32*)(vp) == 24)
                                        {
                                                sprintf(adj_descr, "Adjustment applied for:Outstanding reversal for refund: %s", (char*)vp1);
                                        }
                                        else if (vp && *(int32*)(vp) == 33)
                                        {
                                                sprintf(adj_descr, "Adjustment applied for:convenience fee waiver: %s", (char*)vp1);
                                        }
					else
					{
						sprintf(adj_descr, "Adjustment applied for: %s", (char*)vp1);
					}
				}	
			}
			 strcpy(action_name, "ADJUSTMENT");
		}
	}

/*	else if(event_type && strstr(event_type,"/event/billing/adjustment") != NULL)
	{
		strcpy(action_name, "ADJUSTMENT");
	}
*/
	else if(event_type && strstr(event_type,"/event/billing/reversal") != NULL)
	{
		opstackp = connp->coip;
		while(opstackp != NULL)
		{
			stack_opcode = opstackp->opcode;
			memset(tmp_buff, '\0', sizeof(tmp_buff));
			sprintf(tmp_buff, "OPCODE=%d", stack_opcode);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"BEFORE PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
			if(stack_opcode == PCM_OP_ACT_USAGE)
			{
				act_usage_flag = opstackp->opflags;
				sprintf(tmp_buff, "PCM_OP_ACT_USAGE FLAG=%d", act_usage_flag);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
				memset(tmp_buff, '\0', sizeof(tmp_buff));
			}
			initial_flistp = opstackp->in_flistp;
			initial_opcode = opstackp->opcode;
			
			opstackp = opstackp->next;
		}		
		if(act_usage_flag == 896)
		{
			return;	
		}
		if(initial_opcode == MSO_OP_PYMT_REVERSE_PAYMENT )
		{
		// Need to get the payment remarks
			pymt_remarks = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_DESCR, 1, ebufp);
		}
		
		strcpy(action_name, "PAYMENT REVERSAL");
	}
	else if(event_type && strstr(event_type,"/event/billing/mso_catv_commitment") != NULL)
	{
		strcpy(action_name, "MINIMUM COMMITMENT");
	}
	else if(event_type && strstr(event_type,"/event/billing/mso_cr_dr_note") != NULL)
	{
		strcpy(action_name, "DEBIT NOTE");
	}
	else if(event_type && strstr(event_type,"/event/session/telco/broadband") != NULL)
	{
           // pavan gadi added code for account statement for stop status

		// start of modify to handle start/update accounting event type
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Its BB usage session");
			reservation_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_RESERVATION_OBJ, 1, ebufp);
			opstackp = connp->coip;
			while(opstackp != NULL)
			{
				stack_opcode = opstackp->opcode;
				memset(tmp_buff, '\0', sizeof(tmp_buff));
				sprintf(tmp_buff, "OPCODE=%d", stack_opcode);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"BEFORE PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
				if(stack_opcode == PCM_OP_ACT_USAGE)
				{
					act_usage_flag = opstackp->opflags;
					sprintf(tmp_buff, "PCM_OP_ACT_USAGE FLAG=%d", act_usage_flag);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_buff);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"PCM_OP_ACT_USAGE FLIST", opstackp->in_flistp);
					memset(tmp_buff, '\0', sizeof(tmp_buff));
					break;
				}
				initial_flistp = opstackp->in_flistp;
				initial_opcode = opstackp->opcode;
				
				opstackp = opstackp->next;
			}		
			if(act_usage_flag == 128)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning as flag == 128");
				return;	
			}
			if(reservation_pdp)
			{

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Returning as reservation object is there");
				return;
			}
			if(initial_opcode == MSO_OP_PYMT_REVERSE_PAYMENT )
			{
			// Need to get the payment remarks
				pymt_remarks = PIN_FLIST_FLD_GET(initial_flistp, PIN_FLD_DESCR, 1, ebufp);
			}
			// end of pavan gadi code
		strcpy(action_name, "BROADBAND DATA USAGE");

		// end of modify to handle start/update accounting event type



	}

	else if(event_type && strstr(event_type,"/event/billing/mso_penalty") != NULL)
	{
		strcpy(action_name, "PAYMENT REVERSAL PENALTY");
	}
	else if(event_type && strstr(event_type,"/event/billing/mso_refund_credit") != NULL)
	{
		strcpy(action_name, "REFUND DEPOSIT/REVERSAL");
	}
	else if(event_type && strstr(event_type,"/event/billing/mso_refund_debit") != NULL)
	{
		strcpy(action_name, "REFUND LOADING");
	}
	else if (event_type && strstr(event_type,"/event/billing/late_fee") != NULL)
	{
		strcpy(action_name, "LATE FEE");
	}

//	else if(poid_type && strstr(poid_type,"/event/billing/product/fee/cycle/cycle_forward") != NULL)
//	{
////		*descr = strdup("CYCLE FORWARD CHARGES");
//Source to be identified based on opcode.
//	}
//	else if(poid_type && strstr(poid_type,"/event/billing/product/fee/purchase") != NULL)
//	{
//		*descr = strdup("PURCHASE CHARGES");
//Source to be identified based on opcode.
//	}
	else if(event_type && strcmp(event_type,"/event/billing/product/fee/purchase/mso_deposit") == 0)
	{
		strcpy(action_name, "HARDWARE DEPOSIT");
	}
	else if(event_type && strstr(event_type,"/event/billing/settlement/ws/incollect") != NULL)
	{
		strcpy(action_name, "LCO BALANCE IMPACT");
	}
	else if(event_type && strstr(event_type,"/event/billing/settlement/ws/outcollect") != NULL)
	{
		strcpy(action_name, "LCO COMMISSION BALANCE IMPACT");
	}
	else if(event_type && strstr(event_type,"/event/billing/dispute") != NULL)
	{
		strcpy(action_name, "DISPUTE");
	}
	else if(event_type && strstr(event_type,"/event/billing/settlement") != NULL)
	{
		strcpy(action_name, "SETTLEMENT");
	}
	else
	{
		strcpy(action_name, "MISCELLANEOUS");
	}
 
	/* Get the action using the opcode which initiated this */
/*        if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_ACTIVATE_CUSTOMER)){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as ACTIVATION");
                strcpy(action_name, "ACTIVATION");
        }
        else if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_ADD_PLAN)){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as ADD_PLAN");
                strcpy(action_name, "ADD_PLAN");
        }
        else if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_CHANGE_PLAN)){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as CHANGE_PLAN");
                strcpy(action_name, "CHANGE_PLAN");
        }
        else if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_CANCEL_PLAN)){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as CANCEL_PLAN");
                strcpy(action_name, "CANCEL_PLAN");
        }
        else if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_SUSPEND_SERVICE)){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as SUSPEND_SERVICE");
                strcpy(action_name, "SUSPEND_SERVICE");
        }
        else if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_REACTIVATE_SERVICE)){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as REACTIVATE_SERVICE");
                strcpy(action_name, "REACTIVATE_SERVICE");
        }
        else if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_CUST_TERMINATE_SERVICE)){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as TERMINATE_SERVICE");
                strcpy(action_name, "TERMINATE_SERVICE");
        }
        else if(strstr(PIN_POID_GET_TYPE(event_pdp),"/billing/payment")) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as PAYMENT");
                strcpy(action_name, "PAYMENT");
        }
        else if(strstr(PIN_POID_GET_TYPE(event_pdp),"/billing/reversal")){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as REVERSAL");
                strcpy(action_name, "REVERSAL");
        }
        else if(strstr(PIN_POID_GET_TYPE(event_pdp),"/billing/adjustment")){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as ADJUSTMENT");
                strcpy(action_name, "ADJUSTMENT");
        }
        else if(strstr(PIN_POID_GET_TYPE(event_pdp),"/billing/dispute")){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as DISPUTE");
                strcpy(action_name, "DISPUTE");
        }
        else if((PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp) != NULL) &&
                (strstr(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp),"pin_cycle") ||
                 strstr(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp),"pin_bill_accts"))){
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Setting action as BILLING");
                strcpy(action_name, "BILLING");
        }
*/

	/****************************************************************
	 Figure out the Bill_obj to which this event will point and populate.
	 Will be used as DOC_NO to display at front end.
	 *****************************************************************/
//	if( (strstr(event_type,"adjustment") == NULL) &&
//		(strstr(event_type,"payment") == NULL) )
//	{
		/* Get Bill_obj from Billinfo's current_bill_obj */
		billinfo_obj = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_BILLINFO_OBJ,1,ebufp);
		if(!PIN_POID_IS_NULL(billinfo_obj))
		{
			read_flds_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_BILLINFO_OBJ,read_flds_iflistp,PIN_FLD_POID,ebufp);
			PIN_FLIST_FLD_SET(read_flds_iflistp,PIN_FLD_BILL_OBJ,NULL,ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read flds input flist: ",read_flds_iflistp);
			PCM_OP(ctxp,PCM_OP_READ_FLDS,0,read_flds_iflistp,&read_flds_oflistp,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read flds output flist: ",read_flds_oflistp);
			PIN_FLIST_DESTROY_EX(&read_flds_iflistp, NULL);
		}	
//	}
//	else
//	{
		/* Find Bill poid as per effective_t for payments and adjustments */
//		search_bill_iflistp = PIN_FLIST_CREATE(ebufp);
//	}

	/* Determine Rank of event as
	 * Rank = mso_customer_credential.count + 1
	 * Will be later used to sort and display as per rank. */
	mso_cred_iflistp = PIN_FLIST_CREATE(ebufp);
	if(!search_pdp)
	{
        pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db_no = PIN_POID_GET_DB(pdp);
        search_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
	}
        PIN_FLIST_FLD_PUT(mso_cred_iflistp,PIN_FLD_POID,(void *)search_pdp,ebufp);
        PIN_FLIST_FLD_SET(mso_cred_iflistp,PIN_FLD_FLAGS,&search_flags,ebufp);
        PIN_FLIST_FLD_SET(mso_cred_iflistp,PIN_FLD_TEMPLATE,search_count_template,ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(mso_cred_iflistp,PIN_FLD_ARGS,1,ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, arg_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(mso_cred_iflistp,PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(mso_cred_iflistp,PIN_FLD_TEMPLATE,search_count_template,ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_customer_credential input flist",mso_cred_iflistp);
        PCM_OP(ctxp,PCM_OP_SEARCH, 0, mso_cred_iflistp, &mso_cred_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_customer_credential output flist",mso_cred_oflistp);
	PIN_FLIST_DESTROY_EX(&mso_cred_iflistp, NULL);
	mso_cred_results_oflistp = PIN_FLIST_ELEM_GET(mso_cred_oflistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	if(mso_cred_results_oflistp)
	{
		inc_fields_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(mso_cred_results_oflistp,PIN_FLD_POID,inc_fields_iflistp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_SET(inc_fields_iflistp,PIN_FLD_COUNT,&inc_counter,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Increment count input flist",inc_fields_iflistp);
		PCM_OP(ctxp,PCM_OP_INC_FLDS, 0, inc_fields_iflistp, &inc_fields_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Increment count output flist",inc_fields_oflistp);
		PIN_FLIST_DESTROY_EX(&inc_fields_iflistp, NULL);	
		
		if(inc_fields_oflistp)
		{
			rank = PIN_FLIST_FLD_GET(inc_fields_oflistp,PIN_FLD_COUNT,1,ebufp);
		}		
	}
	/*NT: Added amount to 356 balance array*/
        total_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOTAL, 356, 0, ebufp);
        PIN_FLIST_FLD_SET(total_flistp, PIN_FLD_AMOUNT, amountp,  ebufp);

	/* Create the mso_account_balance object */
	if(event_pdp)
	{
        	cur_balpdp = PIN_POID_CREATE(PIN_POID_GET_DB(event_pdp), "/mso_account_balance", -1, ebufp);
	}
        wrt_iflp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(wrt_iflp, PIN_FLD_POID, cur_balpdp, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, wrt_iflp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, wrt_iflp, PIN_FLD_SERVICE_OBJ, ebufp);
        if(pymt_remarks)
	{
		if(pymt_descr && pymt_descr[0] != '\0')
		{
			PIN_FLIST_FLD_SET(wrt_iflp, PIN_FLD_DESCR,pymt_descr, ebufp);
		}
		else 
		{
			PIN_FLIST_FLD_SET(wrt_iflp, PIN_FLD_DESCR,pymt_remarks, ebufp);
		}
	}
	else if (strlen(adj_descr)>0 )
	{
		PIN_FLIST_FLD_SET(wrt_iflp, PIN_FLD_DESCR,adj_descr, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SYS_DESCR , wrt_iflp, PIN_FLD_DESCR, ebufp);
	}
//        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID , wrt_iflp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_SET(wrt_iflp, PIN_FLD_PROGRAM_NAME,program_name, ebufp);
        //PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ITEM_OBJ , wrt_iflp, PIN_FLD_ITEM_OBJ, ebufp);
	if(read_flds_oflistp)
	{
		PIN_FLIST_FLD_COPY(read_flds_oflistp, PIN_FLD_BILL_OBJ , wrt_iflp, PIN_FLD_BILL_OBJ, ebufp);
		PIN_FLIST_DESTROY_EX(&read_flds_oflistp, NULL);
	}
	current_t = pin_virtual_time((time_t *)NULL);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EARNED_START_T , wrt_iflp, PIN_FLD_EARNED_START_T, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EARNED_END_T , wrt_iflp, PIN_FLD_EARNED_END_T, ebufp);
        //PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T , wrt_iflp, PIN_FLD_POSTED_T, ebufp);
	PIN_FLIST_FLD_SET(wrt_iflp,PIN_FLD_POSTED_T, &current_t, ebufp);
        PIN_FLIST_FLD_PUT(wrt_iflp,PIN_FLD_EVENT_TYPE, event_type, ebufp);
        PIN_FLIST_FLD_PUT(wrt_iflp, PIN_FLD_PREVIOUS_TOTAL,previous_totalp,  ebufp);
        PIN_FLIST_FLD_PUT(wrt_iflp, PIN_FLD_AMOUNT,amountp,  ebufp);
	PIN_FLIST_FLD_PUT(wrt_iflp, PIN_FLD_DISCOUNT,disp,  ebufp);
        PIN_FLIST_FLD_PUT(wrt_iflp, PIN_FLD_CURRENT_BAL,curr_balp,  ebufp);
        PIN_FLIST_FLD_PUT(wrt_iflp, PIN_FLD_IMPACT_CATEGORY, impact_type, ebufp);
	PIN_FLIST_FLD_SET(wrt_iflp, PIN_FLD_ACTION_NAME,action_name,  ebufp);
	if(inc_fields_oflistp)
	{
		PIN_FLIST_FLD_COPY(inc_fields_oflistp,PIN_FLD_COUNT,wrt_iflp,PIN_FLD_RANK,ebufp);
	}
	PIN_FLIST_DESTROY_EX(&inc_fields_oflistp,NULL);
//	PIN_FLIST_FLD_PUT(wrt_iflp, PIN_FLD_RANK, rank,ebufp);

        //PIN_FLIST_FLD_COPY(bal_flistp, PIN_FLD_CURRENT_BAL, wrt_iflp, PIN_FLD_CURRENT_BAL, ebufp);
	if(rd_oflp)
	{
        	PIN_FLIST_FLD_COPY(rd_oflp, PIN_FLD_ACCOUNT_NO, wrt_iflp, PIN_FLD_ACCOUNT_NO, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&rd_oflp, NULL);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ input flist", wrt_iflp);
        PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, wrt_iflp, &wrt_oflp, ebufp);
        PIN_FLIST_DESTROY_EX(&wrt_iflp, NULL);
        if(PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while creating /mso_account_balance", ebufp);
                //PIN_ERRBUF_RESET(ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ return flist", wrt_oflp);

        class_poid = PIN_FLIST_FLD_GET(wrt_oflp,PIN_FLD_POID,1,ebufp);
        if(!PIN_POID_IS_NULL(class_poid))
        {
//                event_no = PIN_POID_GET_ID(class_poid);
//		sprintf(msg,"%d",event_no);
//               PIN_FLIST_FLD_SET(i_flistp,PIN_FLD_EVENT_NO,msg,ebufp);

		len = sizeof( poid_buf );
                p = poid_buf;
                PIN_POID_TO_STR( class_poid, &p, &len, ebufp);

                token = strtok(p, " ");
                token = strtok(NULL, " ");
                token = strtok(NULL, " ");

		PIN_FLIST_FLD_SET(i_flistp,PIN_FLD_EVENT_NO,token,ebufp);	
        }


	/* Cleanup memory */
		
	PIN_FLIST_DESTROY_EX(&wrt_oflp, NULL);	
	PIN_FLIST_DESTROY_EX(&flistp, NULL);	
	PIN_FLIST_DESTROY_EX(&mso_cred_oflistp, NULL);
	return;
}

// Changes for Broadband implementation starts

void getCreditProfile(pcm_context_t   *ctxp,
			poid_t		*acc_pdp, 
			pin_flist_t	*svc_flistp,
			pin_flist_t	**credit_profile_flistp,
			pin_errbuf_t  	*ebufp) 
{

//    char template[500] = "select distinct X from /plan 1, /purchased_product 2 where 2.F1 = V1 and 2.F2 = 1.F3 and 2.F4 = V4";
	char template[500] = "select X from /plan 1, /mso_purchased_plan 2 where 2.F1 = V1 and 2.F2 = 1.F3 and 2.F4 = V4";

    char            plan_template[64] = {"select X from /mso_cfg_credit_limit where F1 = V1 "};
    int64 db = 1;
    poid_t	*s_pdp;
    pin_flist_t	*srch_in_flistp = NULL;
	pin_flist_t	*srch_in_flistp2 = NULL;
    pin_flist_t	*tmp_flistp = NULL;
    //pin_flist_t	*ncr_flistp = NULL;
    pin_flist_t	*srch_out_flistp = NULL;
    pin_flist_t	*srch_out_flistp2 = NULL;
    pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*res_flistp2 = NULL;
	pin_flist_t	*all_cities_flistp = NULL;
	pin_flist_t	*bb_info_flistp = NULL;
	pin_flist_t	*cities_flistp = NULL;
    int32		s_flags = 0;
    //void	*vp = NULL;
    int32	rec_id = 0;
    int32	rec_id2 = 0;
    pin_cookie_t    cookie = NULL;
	pin_cookie_t    cookie2 = NULL;
    char		*plan_name = NULL;
	//pin_decimal_t *credit_limit = NULL;
	//pin_decimal_t *credit_floor = NULL;
	//pin_decimal_t *temp_cl = NULL;
	//pin_decimal_t *temp_cf = NULL;
	int32 status = 2;
	char debug_msg[250];
	char	*all_cities = "*";
	char	*city = NULL;
	char	*plan_city = NULL;
	int32	bill_when = 0;
	int32	plan_bill_when = 0;
	int32	city_found = 0;

	/*  Algorithm:
	* Get all the plans for the purchased products which are in active state and are associated with this account.
	* For each of the plans, check if the credit limit config object exists.
	* Ideally, there should not be a config object existing for non-usage plans (like h/w rental, amc, etc..)
	* Add up all the credit limit and floors and derive the cumulative credit range.	
	*/

	*credit_profile_flistp = PIN_FLIST_CREATE(ebufp);
	bb_info_flistp = PIN_FLIST_SUBSTR_GET(svc_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	city = PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_CITY, 0, ebufp);
	bill_when = *(int32 *)PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);

        db = PIN_POID_GET_DB (acc_pdp);		
        s_pdp = PIN_POID_CREATE(db, "/search/pin", -1, ebufp);
        srch_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

        PIN_FLIST_FLD_SET (srch_in_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
        PIN_FLIST_FLD_SET (srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
		    
        tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
    
    tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_ARGS, 4, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, &status, ebufp);
    
    tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);    
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_NAME, "", ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH input flist", srch_in_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH output flist", srch_out_flistp);
    PIN_FLIST_DESTROY_EX(&srch_in_flistp, ebufp);

	
	while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
                           &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		plan_name = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_NAME, 0, ebufp);
	
		srch_in_flistp2 = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_in_flistp2, PIN_FLD_POID,
                                          (void *)s_pdp, ebufp);

        PIN_FLIST_FLD_SET (srch_in_flistp2, PIN_FLD_FLAGS, &s_flags, ebufp);
        PIN_FLIST_FLD_SET (srch_in_flistp2, PIN_FLD_TEMPLATE, plan_template, ebufp);

        tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp2, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET (tmp_flistp, MSO_FLD_PLAN_NAME, plan_name, ebufp)

		tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp2, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PLAN LIMIT SEARCH input flist", srch_in_flistp2);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp2, &srch_out_flistp2, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PLAN LIMIT SEARCH output flist", srch_out_flistp2);
		//PIN_FLIST_DESTROY_EX(&srch_in_flistp2, ebufp);

		city_found = 0;
		res_flistp2 = PIN_FLIST_ELEM_GET(srch_out_flistp2, PIN_FLD_RESULTS, 0, 1, ebufp );

		if(res_flistp2 == NULL) {
			continue;
		}
		rec_id2 = 0;
		cookie2 = NULL;

		while((cities_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp2, MSO_FLD_CITIES,
                           &rec_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL) {
			// Pawan: Added below line to set all_cities_flistp to NULL since it takes reference 
			// from previous iteration flist
			all_cities_flistp = NULL;
			plan_city = PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CITY, 0, ebufp);
			plan_bill_when = *(int32 *)PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
			sprintf(debug_msg, "Service city is %s and bill_when is %d", city, bill_when);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			sprintf(debug_msg, "Plan city is %s and bill_when is %d", plan_city, plan_bill_when);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			// logic to check if the city and payterm are applicable as per service settings.
			if(bill_when == plan_bill_when || plan_bill_when == 0) {
				if(strcmp(city, plan_city) == 0) {
					city_found = 1;
					update_credit_limits(cities_flistp,credit_profile_flistp, ebufp);
				} else if(strcmp(plan_city, all_cities) == 0) {
					all_cities_flistp = cities_flistp;
				}
			}		
			if(city_found == 0 && all_cities_flistp != NULL) {
				update_credit_limits(all_cities_flistp,credit_profile_flistp, ebufp);			
			}
		}		
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Final Credit Profile flist",*credit_profile_flistp);
    PIN_FLIST_DESTROY_EX(&srch_out_flistp, ebufp);    

}


void update_credit_limits(pin_flist_t	*res_flistp,
							pin_flist_t	**credit_profile_flistp,
							pin_errbuf_t  	*ebufp) {

	int32			rec_id = 0;
	pin_cookie_t    cookie = NULL;
	pin_flist_t		*cl_flistp = NULL;
	pin_flist_t		*ncr_flistp = NULL;
	pin_decimal_t	*credit_limit = NULL;
	pin_decimal_t	*credit_floor = NULL;

	while((cl_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, MSO_FLD_CREDIT_PROFILE,
							   &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "result flist is ", cl_flistp);
		ncr_flistp = PIN_FLIST_ELEM_GET(*credit_profile_flistp, MSO_FLD_CREDIT_PROFILE,rec_id, 1, ebufp);
		if (ncr_flistp != NULL)
		{
			credit_limit = PIN_FLIST_FLD_GET(ncr_flistp, PIN_FLD_CREDIT_LIMIT,0,ebufp);
			credit_floor = PIN_FLIST_FLD_GET(ncr_flistp, PIN_FLD_CREDIT_FLOOR,0,ebufp);				
			credit_limit = pbo_decimal_add(credit_limit,PIN_FLIST_FLD_GET(cl_flistp, PIN_FLD_CREDIT_LIMIT,0,ebufp), ebufp );
			credit_floor = pbo_decimal_add(credit_floor,PIN_FLIST_FLD_GET(cl_flistp, PIN_FLD_CREDIT_FLOOR,0,ebufp), ebufp );
			PIN_FLIST_FLD_SET(ncr_flistp,PIN_FLD_CREDIT_LIMIT, credit_limit, ebufp);
			PIN_FLIST_FLD_SET(ncr_flistp,PIN_FLD_CREDIT_FLOOR, credit_floor, ebufp);
		} else {
			ncr_flistp = PIN_FLIST_ELEM_ADD(*credit_profile_flistp,MSO_FLD_CREDIT_PROFILE,rec_id, ebufp);
			PIN_FLIST_FLD_COPY(cl_flistp, PIN_FLD_CREDIT_LIMIT,ncr_flistp, PIN_FLD_CREDIT_LIMIT, ebufp);
			PIN_FLIST_FLD_COPY(cl_flistp, PIN_FLD_CREDIT_FLOOR,ncr_flistp, PIN_FLD_CREDIT_FLOOR, ebufp);
		}		
	}
}


int is_rec_id_configured_for_threshold (int32 rec_id) {
	int i = 0;
	char debug_msg[250];
	sprintf(debug_msg,"Checking the threshold configuration for %d.. ", rec_id);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
	for (i = 0; i < num_of_resources_configured ; i++){
		sprintf(debug_msg,"Resource in iteration is  %d",credit_thresholds[i].resource_id);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);		
		if(credit_thresholds[i].resource_id == rec_id) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "matching res id found. Returning 1.. ");
			return 1;
		}
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No threshold configured for this resource");		
	return 0;
}

void raise_sms_notification_for_perc_threshold(pin_flist_t *i_flistp,
						pcm_context_t   *ctxp,
						int32 rec_id,
						pin_decimal_t	*percentage,
						pin_errbuf_t *ebufp) {

	pin_flist_t	*notif_iflistp = PIN_FLIST_CREATE(ebufp);
	pin_flist_t	*notif_oflistp = NULL;


	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ,notif_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ,notif_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	if(rec_id == FUP_LIMIT_RESOURCE) {
		PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_FLAGS, &FUP_LIMIT_THRESHOLD_BREACH_FLAG, ebufp);
	} else {
		PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_FLAGS, &CREDIT_THRESHOLD_BREACH_FLAG, ebufp);	
	}	
	PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_PERCENT, percentage, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "raise_sms_notification_for_perc_threshold input list", notif_iflistp);
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, notif_iflistp, &notif_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
                PIN_FLIST_DESTROY_EX(&notif_oflistp, NULL);
                return;
        }
}


void check_for_credit_threshold_breach(pin_flist_t *i_flistp,
					pcm_context_t   *ctxp,
					pin_decimal_t *curr_bal, 
					pin_decimal_t *impact_amount, 
					int32 rec_id, 
					pin_decimal_t *credit_range,
					pin_errbuf_t *ebufp) {
	
	pin_decimal_t	*old_per = NULL;
	pin_decimal_t	*new_amt = NULL;
	pin_decimal_t	*new_per = NULL;
	pin_decimal_t	*temp = NULL;
	char	*percent = NULL;
	char	debug_msg[250];
	int	old_per_less_than_thres = 0, new_per_greater_than_thres = 0;
	int i=0,j=0;	
	pin_decimal_t   *hundred = pbo_decimal_from_double(100.0,ebufp);
	temp = pbo_decimal_divide(curr_bal,credit_range,ebufp);
	old_per =  pbo_decimal_multiply(temp,hundred,ebufp);
	new_amt = pbo_decimal_add(curr_bal, impact_amount, ebufp);
	new_per = pbo_decimal_multiply(pbo_decimal_divide(new_amt,credit_range,ebufp),hundred,ebufp);
			
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " old percentage is ");
	percent = pbo_decimal_to_str(old_per, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,percent);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " new percentage is ");
	percent = pbo_decimal_to_str(new_per, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,percent);
	sprintf(debug_msg, "resource id to check for is %d", rec_id);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
	for (i = 0; i < num_of_resources_configured ; i++){
	sprintf(debug_msg, "resource id in iteration is %d", credit_thresholds[i].resource_id);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
		if(credit_thresholds[i].resource_id == rec_id) {			
			for(j = 0; j < credit_thresholds[i].num_of_config; j++) {
				sprintf(debug_msg, "threshold config[%d] is %s", j, 
							pbo_decimal_to_str(credit_thresholds[i].threshold_config[j], ebufp));
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
				old_per_less_than_thres = (pbo_decimal_compare(old_per, credit_thresholds[i].threshold_config[j],ebufp) < 0);
				sprintf(debug_msg, "old_per_less_than_thres is %d", old_per_less_than_thres);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
				new_per_greater_than_thres = (pbo_decimal_compare(new_per,credit_thresholds[i].threshold_config[j],ebufp) >= 0);
				sprintf(debug_msg, "new_per_greater_than_thres is %d", new_per_greater_than_thres);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
				if(old_per_less_than_thres && new_per_greater_than_thres) {
				    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Percentage credit threshold breached.. Raising notificaiton ");
				    raise_sms_notification_for_perc_threshold(i_flistp,ctxp,rec_id,credit_thresholds[i].threshold_config[j], ebufp);
				}
			}			
		}
	}
	pbo_decimal_destroy(&hundred);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Done with checking threshold breach.");
}


int32 check_for_service_suspension(
	pin_flist_t	*i_flistp,
	poid_t		*prod_poid,
	poid_t		*pur_poid,
	pcm_context_t   *ctxp,
	pin_errbuf_t  	*ebufp) 
{

	pin_flist_t		*inp_flistp = PIN_FLIST_CREATE(ebufp);
	pin_flist_t		*out_flistp = NULL;
	pin_flist_t		*read_flds_in_flistp = NULL;
	pin_flist_t		*read_flds_out_flistp = NULL;
	pin_flist_t		*cancel_plan_flistp = NULL;
	pin_flist_t		*cancel_plan_oflistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*deals_flistp = NULL;
	//pin_flist_t		*deactivate_in_flistp = NULL;
	//pin_flist_t		*deactivate_out_flistp = NULL;
	//pin_flist_t		*deal_info_flistp = NULL;
	double			priority;
	int32			lim_pre_start = (BB_PREPAID_START + BB_LIMITED_RANGE_START);
	int32			lim_pre_end = (BB_PREPAID_START + BB_LIMITED_RANGE_END);

	//int32			SME_PRIO_START = (BB_SUB_SME_PRI_START + lim_pre_start);
	//int32			SME_PRIO_END = (BB_SUB_SME_PRI_START + lim_pre_end);
	//int32			RETAIL_PRIO_START = (BB_SUB_RET_PRI_START + lim_pre_start);
	//int32			RETAIL_PRIO_END = (BB_SUB_RET_PRI_START + lim_pre_end);
	//int32			CYB_PRIO_START = (BB_SUB_CYB_PRI_START + lim_pre_start);
	//int32			CYB_PRIO_END = (BB_SUB_CYB_PRI_START + lim_pre_end);
	//int32			CORP_PRIO_START = (BB_SUB_COR_PRI_START + lim_pre_start);
	//int32			CORP_PRIO_END = (BB_SUB_COR_PRI_START + lim_pre_end);
	char			debug_msg[250];
	//char			*descr = "Service Suspension for quota exhaustion of Limited prepaid plan";
	int32			mode = QUOTA_EXHAUSTION;
	char			*program_name = "Usage Rating";
	int32			prov_status = MSO_PROV_DEACTIVE;
	//int32			status_success = 0;


	PIN_FLIST_FLD_SET(inp_flistp, PIN_FLD_POID, prod_poid, ebufp);
	PIN_FLIST_FLD_SET(inp_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read flds input list", inp_flistp);
	PCM_OP(ctxp,PCM_OP_READ_FLDS , 0, inp_flistp, &out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS ", ebufp);
        PIN_FLIST_DESTROY_EX(&inp_flistp, NULL);
        return;
    }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read flds output list", out_flistp);
	priority = pbo_decimal_to_double(PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_PRIORITY, 0, ebufp), ebufp);
	


	sprintf(debug_msg, "priority to check is %f", priority);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);

	priority = ((int)priority%1000);

	sprintf(debug_msg, "priority after modulo is %f", priority);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);

	if ( priority >= lim_pre_start && priority <= lim_pre_end) {

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Limited Prepaid plan.. Service suspension to be done.. ");
		read_flds_in_flistp =PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_flds_in_flistp, PIN_FLD_POID, pur_poid, ebufp);
		PIN_FLIST_FLD_SET(read_flds_in_flistp, PIN_FLD_PACKAGE_ID, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_in_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_in_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_in_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_in_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist", read_flds_in_flistp);
        	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_in_flistp, &read_flds_out_flistp, ebufp);
        	PIN_FLIST_DESTROY_EX(&read_flds_in_flistp, NULL);
		if(PIN_ERRBUF_IS_ERR(ebufp)){
            		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while reading fields for purchased product", ebufp);
            		return;
        	}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist", read_flds_out_flistp);

		cancel_plan_flistp = PIN_FLIST_CREATE(ebufp);

		/*deactivate_in_flistp = PIN_FLIST_CREATE(ebufp);*/
		PIN_FLIST_FLD_COPY(read_flds_out_flistp,PIN_FLD_ACCOUNT_OBJ,cancel_plan_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp,PIN_FLD_ACCOUNT_OBJ,cancel_plan_flistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp,PIN_FLD_ACCOUNT_OBJ,cancel_plan_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp,PIN_FLD_SERVICE_OBJ,cancel_plan_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
		
		plan_flistp = PIN_FLIST_ELEM_ADD(cancel_plan_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp,PIN_FLD_PLAN_OBJ,plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		deals_flistp = PIN_FLIST_ELEM_ADD(plan_flistp, PIN_FLD_DEALS, 0, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp,PIN_FLD_DEAL_OBJ, deals_flistp, PIN_FLD_DEAL_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp, PIN_FLD_PACKAGE_ID, deals_flistp, PIN_FLD_PACKAGE_ID, ebufp);
	        PIN_FLIST_FLD_SET(cancel_plan_flistp,PIN_FLD_MODE, &mode, ebufp);
	        PIN_FLIST_FLD_SET(cancel_plan_flistp,PIN_FLD_STATUS, &prov_status, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_DEACTIVATE_BB_SERVICE input flist", cancel_plan_flistp);
		PCM_OP(ctxp, MSO_OP_CUST_DEACTIVATE_BB_SERVICE, 0, cancel_plan_flistp, &cancel_plan_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_DEACTIVATE_BB_SERVICE output flist", cancel_plan_oflistp);
		if(PIN_ERRBUF_IS_ERR(ebufp)){
            		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while deactviating the plan for limited prepaid account", ebufp);
            		return;
		}
	}
}

void check_for_credit_limit_breach(pin_flist_t	*i_flistp,
					pcm_context_t   *ctxp,
					pin_decimal_t *curr_bal, 
					pin_decimal_t *impact_amount, 
					pin_decimal_t *credit_range,
					int32 rec_id,
					poid_t		*prod_poid,
					poid_t		*pur_poid,
					pin_errbuf_t  	*ebufp					
					) {

	//pin_flist_t	*notif_iflistp = NULL;
	//pin_flist_t	*notif_oflistp = NULL;
	//int32		rating_status = 0;
	pin_decimal_t   *zero = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t	*amt_after_impact = pbo_decimal_add(curr_bal, impact_amount, ebufp);

	if(pbo_decimal_compare(amt_after_impact, zero, ebufp) >= 0) {
	// Credit Limit Breach. Generate a notification.
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Credit Limit Breached. Raising notification.. ");
/*  Commenting now as there is no template for this sms notification. will need to be uncommented when it is in place.
	notif_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ,notif_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ,notif_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_FLAGS, &CREDIT_LIMIT_BREACH_FLAG, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "check_for_credit_limit_breach notification input list", notif_iflistp);
	PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_RESOURCE_ID, &rec_id, ebufp);
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, notif_iflistp, &notif_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
            PIN_FLIST_DESTROY_EX(&notif_oflistp, NULL);
            return;
        }
*/
	// Check if it is prepaid plan and send service suspend request.
	check_for_service_suspension(i_flistp, prod_poid, pur_poid, ctxp, ebufp);
	}		
}


void check_for_fup_limit_breach(
	pin_flist_t		*i_flistp,
	pcm_context_t		*ctxp,
	pin_decimal_t		*curr_bal, 
	pin_decimal_t		*impact_amount, 
	pin_decimal_t		*credit_range,
	int32			rec_id,
	pin_errbuf_t  		*ebufp) 
{

	pin_flist_t	*notif_iflistp = NULL;
	pin_flist_t	*notif_oflistp = NULL;
	pin_flist_t	*prov_in_flistp = NULL;
	pin_flist_t	*prov_out_flistp = NULL;
	pin_flist_t	*wrt_flds_in_flistp = NULL;
	pin_flist_t	*wrt_flds_out_flistp = NULL;
	pin_flist_t	*bb_info_flistp = NULL;
	int32		prov_flag = DOC_BB_UPDATE_CAP;
	int32		fup_status = AFT_FUP; // After FUP.
	pin_decimal_t	*old_per = NULL;
	pin_decimal_t	*per = pbo_decimal_from_str( "100", ebufp );
	poid_t		*acct_pdp = NULL;

	pin_decimal_t	*amt_after_impact = pbo_decimal_add(curr_bal, impact_amount, ebufp);

	old_per = pbo_decimal_divide( curr_bal, credit_range, ebufp );
	pbo_decimal_multiply_assign( old_per, per, ebufp );
	if( old_per && pbo_decimal_compare( old_per, per, ebufp ) >= 0 ){
		pbo_decimal_destroy( &old_per );
		pbo_decimal_destroy( &per );
		return;
	}

	if(credit_range != NULL && (pbo_decimal_compare(amt_after_impact, credit_range, ebufp) >= 0) ) 
	{
		// Credit Limit Breach. Generate a notification.
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " FUP Limit Breached. Raising notification.. ");
		notif_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ,notif_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ,notif_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_FLAGS, &FUP_LIMIT_BREACH_FLAG, ebufp);
		PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_RESOURCE_ID, &rec_id, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "check_for_fup_limit_breach notification input list", notif_iflistp);

		//PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, notif_iflistp, &notif_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
			PIN_FLIST_DESTROY_EX(&notif_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&notif_oflistp, NULL);
			return;
		}
		
		// No need to trigger provisioning action as it is done by network element only.
		acct_pdp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_ACCOUNT_OBJ,1,ebufp);
		prov_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ,prov_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ,prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &prov_flag, ebufp);
		PIN_FLIST_FLD_PUT(prov_in_flistp,MSO_FLD_PURCHASED_PLAN_OBJ,
				PIN_POID_CREATE(PIN_POID_GET_DB(acct_pdp),"/mso_purchased_plan",-1,ebufp),ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist for provisioning action for FUP capping", prov_in_flistp);
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, prov_in_flistp, &prov_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist from provisioning action for FUP capping", prov_out_flistp);
		


		// Update service object to set the FUP_STATUS to AFT_FUP.
		wrt_flds_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ,wrt_flds_in_flistp, PIN_FLD_POID, ebufp);
		bb_info_flistp = PIN_FLIST_ELEM_ADD(wrt_flds_in_flistp, MSO_FLD_BB_INFO, 0, ebufp);
		PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_FUP_STATUS, &fup_status, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist for updating FUP status", wrt_flds_in_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS , 0, wrt_flds_in_flistp, &wrt_flds_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist from updating FUP status", wrt_flds_out_flistp);
		if (!PIN_ERRBUF_IS_ERR(ebufp))
		{
			fm_mso_create_lifecycle_evt(ctxp, notif_iflistp, NULL, ID_FUP_DOWNGRADE ,ebufp );
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
		}
	}
	PIN_FLIST_DESTROY_EX(&notif_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&notif_oflistp, NULL);

	if( old_per ){
		pbo_decimal_destroy( &old_per );
	}
	if( per ){
		pbo_decimal_destroy( &per );
	}
}

void mso_fm_check_threshold(
	pin_flist_t		*i_flistp,
	pcm_context_t   	*ctxp,
	pin_errbuf_t  		*ebufp)
{
	
	//int32                   errp1 = 0;
	//char                    *str = NULL;	
	pin_flist_t		*bal_in_flistp = NULL;
	//int32			credit_threshold_config[20];
	pin_flist_t		*bal_out_flistp = NULL;
	pin_flist_t		*bi_flistp = NULL;
	pin_flist_t		*ncr_flistp = NULL;
	pin_flist_t		*credit_profile_flistp = NULL;
	pin_flist_t		*svc_ro_flistp = NULL;
	pin_flist_t		*svc_ri_flistp = NULL;
	pin_flist_t		*credit_limit_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	pin_flist_t		*session_flistp = NULL;
	int32			res_id;
	void			*vp = NULL;
	pin_cookie_t	cookie = NULL;
	int32			rec_id = 0;
	int 			flags = 0x10;
	pin_decimal_t * impact_amount = NULL;
	pin_decimal_t * current_bal = NULL;
	pin_decimal_t * res_amt = NULL;
	pin_decimal_t * new_bal = NULL;
	pin_decimal_t * credit_floor = NULL;
	pin_decimal_t * credit_limit = NULL;
	pin_decimal_t * credit_range = NULL;
	//pin_decimal_t * temp_bal = NULL;
	char		debug_msg[250];
	poid_t		*acc_poid = NULL;
	poid_t      *svc_pdp = NULL;
	poid_t		*prod_poid = NULL;	
	poid_t		*pur_prod = NULL;
	pin_flist_t	*reservation_flistp = NULL;
	poid_t		*reservation_poid = NULL;
	pin_flist_t	*read_reservation_iflistp = NULL;
	pin_flist_t	*read_reservation_oflistp = NULL;
	pin_flist_t	*reserve_bal_flistp = NULL;
	pin_flist_t	*active_session_flistp = NULL;
	pin_decimal_t   *current_res_amount = NULL;
	pin_decimal_t	*downlink = NULL;
	pin_decimal_t	*uplink = NULL;
	pin_decimal_t	*sum = pin_decimal( "0", ebufp );
	pin_decimal_t	*zero = pin_decimal( "0", ebufp );
	pin_decimal_t	*divide = pin_decimal( "1048576", ebufp );

	int		i = 0, j = 0;
	time_t          pvt = pin_virtual_time((time_t *)NULL);	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_fm_check_threshold input flist", i_flistp);
	acc_poid = 	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
	svc_pdp = 	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );

	bal_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, bal_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, bal_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, bal_in_flistp, PIN_FLD_START_T, ebufp);
	//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, bal_in_flistp, PIN_FLD_END_T, ebufp);
	PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_START_T, (void *)&pvt, ebufp);
	PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_END_T, (void *)&pvt, ebufp);
	PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_FLAGS, (void *)&flags, ebufp);
	PIN_FLIST_ELEM_SET(bal_in_flistp,NULL, PIN_FLD_BALANCES, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "GET_BALANCES input flist", bal_in_flistp);
	PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES, 0, bal_in_flistp, &bal_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&bal_in_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "GET_BALANCES output flist", bal_out_flistp);
	
	
	if(PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_BAL_IMPACTS, ebufp) > 0) {	
		svc_ri_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(svc_ri_flistp, PIN_FLD_POID, svc_pdp, ebufp);
		bb_info_flistp = PIN_FLIST_ELEM_ADD(svc_ri_flistp, MSO_FLD_BB_INFO, 0, ebufp);
		PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_CITY, NULL, ebufp);
		PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read flds input flist", svc_ri_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, svc_ri_flistp, &svc_ro_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read flds output flist", svc_ro_flistp);
		PIN_FLIST_DESTROY_EX(&svc_ri_flistp, ebufp);
		getCreditProfile(ctxp, acc_poid, svc_ro_flistp, &credit_profile_flistp, ebufp);

	}		
			
	while((bi_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
                                          &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Balance impact flist ",bi_flistp);
			vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_RESOURCE_ID, 1, ebufp );
			if(vp) {
				res_id = *((int *)vp);
		}

		sprintf(debug_msg,"Impacted Resource is  %d",res_id);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		ncr_flistp = PIN_FLIST_ELEM_GET(bal_out_flistp, PIN_FLD_BALANCES, res_id, 1, ebufp);
						
		if (ncr_flistp == (pin_flist_t *)NULL) {     
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NCR Flist NULL.. WHY!!!??");
                        continue;
                        /*******/
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "NCR Flist is..", ncr_flistp);
		
		credit_limit_flistp = PIN_FLIST_ELEM_GET(credit_profile_flistp, MSO_FLD_CREDIT_PROFILE, res_id, 1, ebufp);
		if(credit_limit_flistp != NULL) {
			credit_floor = (pin_decimal_t *) PIN_FLIST_FLD_GET(credit_limit_flistp, PIN_FLD_CREDIT_FLOOR, 0, ebufp);
			sprintf(debug_msg, "CREDIT FLOOR IS %s", pbo_decimal_to_str(credit_floor, ebufp));
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
			credit_limit = (pin_decimal_t *) PIN_FLIST_FLD_GET(credit_limit_flistp, PIN_FLD_CREDIT_LIMIT, 0, ebufp);
			sprintf(debug_msg, "CREDIT LIMIT IS %s", pbo_decimal_to_str(credit_limit, ebufp));
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
			
			if((!pbo_decimal_is_null(credit_floor,ebufp)) && (!pbo_decimal_is_null(credit_limit,ebufp))) {
			credit_range = pbo_decimal_subtract(credit_limit, credit_floor, ebufp);	
			sprintf(debug_msg, "Credit Range is %s", pbo_decimal_to_str(credit_range, ebufp));
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
			}
	}

		impact_amount = (pin_decimal_t *) PIN_FLIST_FLD_GET(bi_flistp,PIN_FLD_AMOUNT, 0 ,ebufp);
															
		current_bal = (pin_decimal_t *) PIN_FLIST_FLD_GET(ncr_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
		if(!pbo_decimal_is_null(credit_floor,ebufp)) {
			current_bal = pbo_decimal_subtract(current_bal, credit_floor, ebufp);
		}			
		res_amt = (pin_decimal_t *) PIN_FLIST_FLD_GET(ncr_flistp, PIN_FLD_RESERVED_AMOUNT, 0, ebufp);
		new_bal = pbo_decimal_add(current_bal, res_amt, ebufp);	

		sprintf(debug_msg, "Balance (curr bal + reservation) is %s", pbo_decimal_to_str(new_bal, ebufp));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);											

		// TODO: Check for FUP breach and generate notification for service downgrade.
		/*if(res_id == FUP_LIMIT_RESOURCE && (!pbo_decimal_is_null(credit_range,ebufp))) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Checking FUP Limit Breach.. ");
			check_for_fup_limit_breach(i_flistp,ctxp, new_bal, impact_amount,credit_range,res_id, ebufp);
		}*/

		sprintf(debug_msg,"number of resources configured is %d.. ", num_of_resources_configured);
        for (i = 0; i < num_of_resources_configured ; i++){
                sprintf(debug_msg,"Credit Threshold for %d is:",credit_thresholds[i].resource_id);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
                        for(j = 0; j < credit_thresholds[i].num_of_config; j++) {
                                sprintf(debug_msg, "threshold config[%d] is %s", j, pbo_decimal_to_str(credit_thresholds[i].threshold_config[j], ebufp));
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
                        }
        }

	// get reservation for current session 
		reservation_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_RESERVATION_LIST, PIN_ELEMID_ANY, 1, ebufp);
		if(reservation_flistp)
		{
			reservation_poid = PIN_FLIST_FLD_GET(reservation_flistp, PIN_FLD_RESERVATION_OBJ, 0, ebufp);
			if(reservation_poid && read_reservation_oflistp == NULL )
			{
				read_reservation_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_reservation_iflistp, PIN_FLD_POID, reservation_poid, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Reservation read input flist", read_reservation_iflistp);
				PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_reservation_iflistp, &read_reservation_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "RReservation read output flist", read_reservation_oflistp);
				PIN_FLIST_DESTROY_EX(&read_reservation_iflistp, ebufp);	
				if(PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error reading reservation object");
					//clear the error here
					PIN_ERRBUF_CLEAR(ebufp);
					PIN_FLIST_DESTROY_EX(&read_reservation_oflistp, ebufp);					
				}			
			}

			if(read_reservation_oflistp)
			{
				reserve_bal_flistp = PIN_FLIST_ELEM_GET(read_reservation_oflistp, PIN_FLD_BALANCES, res_id, 1, ebufp);		
				if (reserve_bal_flistp){
					current_res_amount =
					 (pin_decimal_t *) PIN_FLIST_FLD_GET(reserve_bal_flistp,
					 	PIN_FLD_AMOUNT, 0, ebufp);				 
				//deduct already reseved amount from current impact
				//pbo_decimal_subtract_assign(impact_amount,current_res_amount , ebufp);
					impact_amount = pbo_decimal_subtract(impact_amount,
									current_res_amount,ebufp);				
				}
			}
		}
		else{//in case of stop cdr reservation object is deleted before post_rating call. So, need to check the active session
			if( active_session_flistp == NULL ){
				fm_rate_pol_post_rating_get_aso_object( ctxp, i_flistp, &active_session_flistp, ebufp );
			}
			if( PIN_ERRBUF_IS_ERR(ebufp)){
				PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG, "Error searching active session object");
				PIN_ERRBUF_CLEAR(ebufp);
				//PIN_FLIST_DESTROY_EX(&active_session_flistp, NULL);
			}
			//read_reservation_oflistp = PIN_FLIST_ELEM_GET( active_session_flistp, PIN_FLD_SESSION_INFO, 0, 1, ebufp );
			if( active_session_flistp ){
				session_flistp = PIN_FLIST_ELEM_GET( active_session_flistp, PIN_FLD_SESSION_INFO, PIN_ELEMID_ANY, 1, ebufp );
				PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "Active session flist", session_flistp );
				if( session_flistp ){
					downlink = PIN_FLIST_FLD_GET( session_flistp, PIN_FLD_BYTES_DOWNLINK, 0, ebufp );
					uplink = PIN_FLIST_FLD_GET( session_flistp, PIN_FLD_BYTES_UPLINK, 0, ebufp);

					pbo_decimal_multiply_assign( sum, zero, ebufp );
					pbo_decimal_add_assign( sum, downlink, ebufp );
					pbo_decimal_add_assign( sum, uplink, ebufp );
					pbo_decimal_divide_assign( sum, divide, ebufp );
					impact_amount = pbo_decimal_subtract( impact_amount, sum, ebufp );//where sum is the current reserved amount
					if( new_bal ){
						pbo_decimal_add_assign( new_bal, sum, ebufp );
					}
				}
			}
		}


		// TODO: Check for FUP breach and generate notification for service downgrade.
		if(res_id == FUP_LIMIT_RESOURCE && (!pbo_decimal_is_null(credit_range,ebufp))) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Checking FUP Limit Breach.. ");
			check_for_fup_limit_breach(i_flistp,ctxp, new_bal, impact_amount,credit_range,res_id, ebufp);
		}

		if(is_rec_id_configured_for_threshold(res_id) == 1 && (!pbo_decimal_is_null(credit_range,ebufp))) {

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Threshold configured for resource");	
			/* If either floor or ceil is infinite, no check can be done based on percentage. So skip those resources.*/
			//if(pbo_decimal_is_null(credit_floor,ebufp) || pbo_decimal_is_null(credit_limit,ebufp)) {
			//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Either floor or ceil is infinite, no check can be done based on percentage. So skip those resources");
			//	continue;
			//}
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
			check_for_credit_threshold_breach(i_flistp,ctxp,new_bal, impact_amount, res_id, credit_range, ebufp);						
		}		
			
		sprintf(debug_msg,"Impacted Resource is  [%d] and free mb resource is [%d]",res_id,FREE_MB_RESOURCE);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		sprintf(debug_msg,"Equal is  %d",(res_id == FREE_MB_RESOURCE));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		if(res_id == FREE_MB_RESOURCE) {
			//  TODO: Check for credit breach for prepaid limited plan to generate prov action for suspend service.
			vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp );
			if(vp) {
				prod_poid = (poid_t *)vp;
			}
			pur_prod = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp );
			current_bal = (pin_decimal_t *) PIN_FLIST_FLD_GET(ncr_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
                       if(!pbo_decimal_is_null(credit_floor,ebufp)) {
                           new_bal = pbo_decimal_add(current_bal, res_amt, ebufp);
                       } else {
                           new_bal = pbo_decimal_copy(res_amt, ebufp);
                       }
		       if( new_bal && sum && !pbo_decimal_is_zero( sum, ebufp )){
				pbo_decimal_add_assign( new_bal, sum, ebufp );
			}



			
		    check_for_credit_limit_breach(i_flistp,ctxp, new_bal, impact_amount,credit_range,res_id, prod_poid, pur_prod, ebufp);
		}		
	}	

	PIN_FLIST_DESTROY_EX(&svc_ro_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&active_session_flistp, NULL);
	
	//if(!pbo_decimal_is_null(impact_amount,ebufp)) pbo_decimal_destroy(&impact_amount);
	if( sum ){
		pbo_decimal_destroy( &sum );
	}
	if( zero ){
		pbo_decimal_destroy( &zero );
	}
	if( divide ){
		pbo_decimal_destroy( &divide );
	}

}

static void
mso_call_bb_cancel_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
	
	int32		prov_status = MSO_PROV_DEACTIVE;
	poid_t		*srch_pdp = NULL;
	poid_t		*payinfo_pdp = NULL;
	//poid_t		*svc_pdp = NULL;
	char		*payinfo_srch_template = "select X from /payinfo where F1 = V1 and F2.type = V2 ";
	char		*mso_pp_srch_template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
	int32		srch_flag = 0;
	poid_t		*purchased_prod_pdp = NULL;
	int32		pp_indicator = 0;
	int32		PREPAID = 2;
	int32		status_success = 0;
	int32		status = 1;
	int32		prty=0;
	int32		mod_prty=0;
	int64		db = 1;
	char		*descr = NULL;
	char		*error_code = NULL;
	char		*tmp_str = NULL;
	int32		validity_expiry = VALIDITY_EXPIRY;
	pin_decimal_t          *plan_priority = NULL;

	pin_flist_t	*payinfo_srch_in_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	//pin_flist_t	*inh_flistp = NULL;
	pin_flist_t	*inv_info_flistp = NULL;
	pin_flist_t	*payinfo_srch_out_flistp = NULL;
	pin_flist_t	*prod_flistp = NULL;
	pin_flist_t	*mso_pp_in_flistp = NULL;
	pin_flist_t	*mso_pp_out_flistp = NULL;
	pin_flist_t	*deactivate_in_flistp = NULL;
	pin_flist_t	*deactivate_out_flistp = NULL;
	pin_flist_t	*wrt_in_flistp = NULL;
	pin_flist_t	*wrt_out_flistp = NULL;
	pin_flist_t	*product_flistp = NULL;
	pin_flist_t     *write_flds_inflistp = NULL;
	pin_flist_t     *write_flds_outflistp = NULL;
	int32		base_plan = 0;
	poid_t		*base_plan_pdp = NULL;
	char		*prov_tag = NULL;
	int32           elem_id = 0;
	pin_cookie_t    cookie = NULL;
	char            *prog_name = NULL;
	pin_flist_t	*create_iflist = NULL;
	int		set_status = 0;
	time_t		*cycle_end_t = NULL;
	time_t		expiry = 0;
	int             *grace_days = NULL;
	int             perr = 0;
	struct tm       *timeeff;
	poid_t		*s_pdp = NULL;
	pin_flist_t	*create_oflist = NULL;
	pin_flist_t	*offr_flistp = NULL;
	pin_flist_t	*offr_oflistp = NULL;	
	// Steps:
	// 1. check if the account is having prepaid indicator in payinfo
	// 2. Check if the product is from a base plan which is getting cancelled.
	// 3. Call deactivate_bb_service opcode to set the prov status to DEACTIVATE.


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_call_bb_cancel_plan input flist", i_flistp);

	prog_name = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_PROGRAM_NAME,1,ebufp); 
	// 1. check if the account is having prepaid indicator in payinfo
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	payinfo_pdp = PIN_POID_CREATE(db, "/payinfo/invoice", -1, ebufp);
	
	payinfo_srch_in_flistp = PIN_FLIST_CREATE(ebufp);	
	payinfo_srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(payinfo_srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(payinfo_srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(payinfo_srch_in_flistp, PIN_FLD_TEMPLATE, payinfo_srch_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(payinfo_srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(payinfo_srch_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, payinfo_pdp, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(payinfo_srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	inv_info_flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_INV_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(inv_info_flistp, PIN_FLD_INDICATOR, NULL, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " check if the account is having prepaid indicator in payinfo.");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_call_bb_cancel_plan search input flist for prepaid indicator", payinfo_srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, payinfo_srch_in_flistp, &payinfo_srch_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PROV_CREATE_ACTION search output flist for prepaid indicator", payinfo_srch_out_flistp);

	result_flistp = PIN_FLIST_ELEM_GET(payinfo_srch_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	inv_info_flistp = PIN_FLIST_ELEM_GET(result_flistp, PIN_FLD_INV_INFO, 0, 0, ebufp);
	pp_indicator = *(int32 *)PIN_FLIST_FLD_GET(inv_info_flistp, PIN_FLD_INDICATOR, 0, ebufp);

	// Pawan: Moved below code to just before service deactivation in order to handle the 
	// update of mso_pp status in case of FUP topup and Add MB cancellation.
	/*if(pp_indicator != PREPAID) {
		// Not a prepaid account. Nothing to do at cancel here.
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Not a prepaid Account. Nothing to be done for cancel here. returning..");
		return;
	}*/
	PIN_FLIST_DESTROY_EX(&payinfo_srch_in_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&payinfo_srch_out_flistp, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Prepaid account. Going to check for base plan");

	// 2. Check if the purchased product is from a base plan which is getting cancelled.

	prod_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PRODUCT, 1, ebufp);
	if (prog_name && strcmp(prog_name,"pin_cycle")==0) {

                 write_flds_inflistp = PIN_FLIST_CREATE(ebufp);
                 PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_OFFERING_OBJ, write_flds_inflistp, PIN_FLD_POID, ebufp);
                 PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_DESCR, "pin_cycle_fees", ebufp);
                 PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

                       if (PIN_ERRBUF_IS_ERR(ebufp))
                       {
                              PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                "Error in mso_call_prov_cancel_plan: wflds", ebufp);
                       }

                  PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
                  PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);

        }
	purchased_prod_pdp = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);

	mso_pp_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(mso_pp_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(mso_pp_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(mso_pp_in_flistp, PIN_FLD_TEMPLATE, mso_pp_srch_template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	prod_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, purchased_prod_pdp, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	//PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, NULL, ebufp);
	//PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRIORITY, NULL, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Check if the purchased product is from a base plan which is getting cancelled.");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_call_bb_cancel_plan search input flist for base plan search", mso_pp_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, mso_pp_in_flistp, &mso_pp_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_call_bb_cancel_plan search output flist for base plan search", mso_pp_out_flistp);
	PIN_FLIST_DESTROY_EX(&mso_pp_in_flistp, ebufp);
	result_flistp = PIN_FLIST_ELEM_GET(mso_pp_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH or purchased_poid not found ", ebufp);
                PIN_FLIST_DESTROY_EX(&mso_pp_out_flistp, ebufp);
                return;
        }
	descr = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_DESCR, 1, ebufp);

	if(!descr || (strcmp(descr, MSO_BB_BP) != 0)) {
		// Added below if condition to update the mso_pp status to terminate
		// Statement in else part was already there without any condition.
		plan_priority = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRIORITY, 1, ebufp);
		tmp_str = pbo_decimal_to_str(plan_priority, ebufp);
		prty = atoi(tmp_str);
		mod_prty = prty%1000;
		if (prty > 1000 &&
			((mod_prty >= BB_FUP_START && mod_prty <= BB_FUP_END) || (mod_prty >= BB_ADD_MB_START && mod_prty <= BB_ADD_MB_END))
		    )
		{
			wrt_in_flistp = PIN_FLIST_CREATE(ebufp);
			status = MSO_PROV_TERMINATE;
			PIN_FLIST_FLD_COPY(result_flistp,PIN_FLD_POID,wrt_in_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(wrt_in_flistp,PIN_FLD_STATUS, &status, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS input flist", wrt_in_flistp);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wrt_in_flistp, &wrt_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS output flist", wrt_out_flistp);
			PIN_FLIST_DESTROY_EX(&wrt_in_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&wrt_out_flistp, ebufp);
		        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This is not a base plan to deactivate service");
                        PIN_FLIST_DESTROY_EX(&mso_pp_out_flistp, ebufp);
			return;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "The purchased product is not for the base plan. So nothing to do here. returning..");
        		PIN_FLIST_DESTROY_EX(&mso_pp_out_flistp, ebufp);
			return;
		}
	}
	/***checking for the subscription plan to deactivate the service****/
	while((product_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, PIN_FLD_PRODUCTS,
                &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)	
	{
		prov_tag = PIN_FLIST_FLD_GET(product_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
		if(prov_tag && strlen(prov_tag) > 0 )
		{
			base_plan_pdp = PIN_FLIST_FLD_GET(product_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 1, ebufp);
			if (PIN_POID_COMPARE(base_plan_pdp, purchased_prod_pdp, 0, ebufp) == 0)
			{
				base_plan = 1;
			}	
		}
	}
	if(base_plan != 1)
	{
		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This is not a base plan to deactivate service");
        	 PIN_FLIST_DESTROY_EX(&mso_pp_out_flistp, ebufp); 
		 return;
	}
	PIN_FLIST_DESTROY_EX(&mso_pp_out_flistp, ebufp);

	if(pp_indicator != PREPAID) {
		// Not a prepaid account. Nothing to do at cancel here.
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Not a prepaid Account. Nothing to be done for cancel here. returning..");
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " The purchased product is for the base plan. Calling deactivate opcode to set the provisioning status");
	//Added changes for grace job object creation 
	if(base_plan == 1)
	{
		prod_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PRODUCT, 1, ebufp);
		offr_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_OFFERING_OBJ, offr_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_DESCR, NULL, ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_STATUS, NULL, ebufp);
		PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_CYCLE_END_T, NULL, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist rflds", offr_flistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, offr_flistp, &offr_oflistp, ebufp);

                PIN_FLIST_DESTROY_EX(&offr_flistp, NULL);
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                         "error in read_flds", ebufp);
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist rflds", offr_oflistp);
                cycle_end_t = PIN_FLIST_FLD_GET(offr_oflistp, PIN_FLD_CYCLE_END_T, 0, ebufp);

		create_iflist = PIN_FLIST_CREATE(ebufp);
                s_pdp = PIN_POID_CREATE(db, "/mso_bb_grace_job", -1, ebufp);
                PIN_FLIST_FLD_PUT(create_iflist, PIN_FLD_POID, (void *)s_pdp, ebufp);
		PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_PRODUCT_OBJ, create_iflist, PIN_FLD_PRODUCT_OBJ, ebufp);	
		PIN_FLIST_FLD_COPY(prod_flistp, PIN_FLD_OFFERING_OBJ, create_iflist, PIN_FLD_OFFERING_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, create_iflist, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, create_iflist, PIN_FLD_SERVICE_OBJ, ebufp);
		args_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ACTION_INFO, PIN_ELEMID_ANY, 1,  ebufp);
		if(args_flistp)
		{
			PIN_FLIST_FLD_COPY(args_flistp, PIN_FLD_PLAN_OBJ, create_iflist, PIN_FLD_PLAN_OBJ, ebufp);
			PIN_FLIST_FLD_SET(create_iflist, PIN_FLD_PURCHASE_END_T, cycle_end_t, ebufp);
			PIN_FLIST_FLD_SET(create_iflist, PIN_FLD_CHARGED_TO_T, cycle_end_t, ebufp);
		}
		pin_conf("fm_mso_prov", "bb_grace_period", PIN_FLDT_INT, (caddr_t*)&grace_days, &perr);
		timeeff = localtime(cycle_end_t);
		timeeff->tm_mday = timeeff->tm_mday + *grace_days;
		expiry = mktime (timeeff);
		if (perr != PIN_ERR_NONE) {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                                "Error in adding grace days");
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                        PIN_ERR_BAD_VALUE, 0, 0, 0);
			PIN_FLIST_DESTROY_EX(&create_iflist, NULL);
			PIN_FLIST_DESTROY_EX(&offr_oflistp, NULL);
			return;
                }
		PIN_FLIST_FLD_SET(create_iflist, PIN_FLD_EXIT_T, &expiry, ebufp);
                PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, " Calling create_obj input flist", create_iflist);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_iflist, &create_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&create_iflist, NULL);
		PIN_FLIST_DESTROY_EX(&offr_oflistp, NULL);
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                         "error in calling create obj", ebufp);
			return;
                }
		PIN_FLIST_DESTROY_EX(&create_oflist, NULL);
	}
	// 3. Call deactivate_bb_service opcode to set the prov status to DEACTIVATE.
	deactivate_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_ACCOUNT_OBJ,deactivate_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_ACCOUNT_OBJ,deactivate_in_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_SERVICE_OBJ,deactivate_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_SET(deactivate_in_flistp,PIN_FLD_STATUS, &prov_status, ebufp);
	PIN_FLIST_FLD_SET(deactivate_in_flistp,PIN_FLD_MODE, &validity_expiry, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, deactivate_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_DEACTIVATE_BB_SERVICE input flist", deactivate_in_flistp);
	PCM_OP(ctxp, MSO_OP_CUST_DEACTIVATE_BB_SERVICE, 0, deactivate_in_flistp, &deactivate_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_DEACTIVATE_BB_SERVICE output flist", deactivate_out_flistp);

	status = *(int32 *)PIN_FLIST_FLD_GET(deactivate_out_flistp,PIN_FLD_STATUS,0, ebufp);

	if(status != status_success){
		error_code = PIN_FLIST_FLD_GET(deactivate_out_flistp,PIN_FLD_ERROR_CODE, 0, ebufp);
		if(strcmp(error_code, CUST_BB_DEACTIVATE_HAVE_SUBSCRIPTION) == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Having cubscription product active");	
		}
		else if(strcmp(error_code, CUST_BB_DEACTIVATE_ALREADY_IN_PROGRESS) != 0) {
        	    pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        MSO_ERR_DEACTIVATE_BB_SERVICE_FAILED, 0, 0, MSO_OP_CUST_DEACTIVATE_BB_SERVICE); 
		} else {
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Deactivation failed as the provisioning status is already in progress/deactive. Possibly another product for the same plan has called the MSO_OP_CUST_DEACTIVATE_BB_SERVICE before. ");
		}
		PIN_FLIST_DESTROY_EX(&deactivate_in_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&deactivate_out_flistp, ebufp);
		return;
	}

	PIN_FLIST_DESTROY_EX(&deactivate_in_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&deactivate_out_flistp, ebufp);

	return;
}



static void
fm_rate_pol_post_rating_get_aso_object(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp )
{
	pin_flist_t             *srch_iflistp           = NULL;
	pin_flist_t             *srch_oflistp           = NULL;
	pin_flist_t             *args_flistp            = NULL;
	pin_flist_t             *telco_info_flistp      = NULL;

	poid_t                  *srch_pdp               = NULL;
	poid_t                  *pdp                    = NULL;

	char                    *srch_tmplt             = NULL;
	char                    *reservation_no         = NULL;

	int64                   db                      = 0;
	int                     srch_flag               = SRCH_DISTINCT;

	if( PIN_ERRBUF_IS_ERR(ebufp)){
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	pdp = PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_POID,
		0, ebufp );
	db = PIN_POID_GET_DB( pdp );

	telco_info_flistp = PIN_FLIST_ELEM_GET( i_flistp,
		PIN_FLD_TELCO_INFO, PIN_ELEMID_ANY, 0, ebufp );
	reservation_no = PIN_FLIST_FLD_GET( telco_info_flistp,
		PIN_FLD_NETWORK_SESSION_ID, 0, ebufp );

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE( db, "/search", -1,
		ebufp );
	PIN_FLIST_FLD_PUT( srch_iflistp, PIN_FLD_POID,
		(void *)srch_pdp, ebufp );
	PIN_FLIST_FLD_SET( srch_iflistp, PIN_FLD_FLAGS,
		(void *)&srch_flag, ebufp );
	srch_tmplt = "select X from /active_session/telco/broadband where "
		"F1 = V1 ";
	PIN_FLIST_FLD_SET( srch_iflistp, PIN_FLD_TEMPLATE,
		(void *)srch_tmplt, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD( srch_iflistp,
		PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_ACTIVE_SESSION_ID,
		(void *)reservation_no, ebufp );

	PIN_FLIST_ELEM_SET( srch_iflistp, NULL,
		PIN_FLD_RESULTS, 0, ebufp );

	/************************************
	 * Log input flist
	 ***********************************/
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
		"fm_rate_pol_post_rating_get_aso_object: "
		"ASO object search input flist", srch_iflistp );
	/***********************************
	 * Call PCM_OP_SEARCH
	 ***********************************/
	PCM_OP( ctxp, PCM_OP_SEARCH, 0, srch_iflistp,
		&srch_oflistp, ebufp );
	/**********************************
	 * Check error
	 **********************************/
	if( PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"fm_rate_pol_post_rating_get_aso_object: "
			"ASO object search error", ebufp );
		PIN_FLIST_DESTROY_EX( &srch_oflistp, NULL );
	}
	else{
		PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
			"fm_rate_pol_post_rating_get_aso_object: "
			"ASO object search output flist", srch_oflistp );

		*ret_flistpp = PIN_FLIST_ELEM_TAKE( srch_oflistp,
			PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	}
	PIN_FLIST_DESTROY_EX( &srch_iflistp, NULL );
	PIN_FLIST_DESTROY_EX( &srch_oflistp, NULL );
	return;
}



// Changes for Broadband implementation ends
// Function introduced for Grace Period for Base FDP Packs
void
mso_cancel_alc_addon_pdts(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*srch_in_flist = NULL;
	pin_flist_t		*srch_out_flist = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*prov_out_flistp = NULL;
	pin_flist_t		*write_flds_inflistp = NULL;
	pin_flist_t		*write_flds_outflistp = NULL;
	pin_flist_t		*pdt_flistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*child_serv_flist = NULL;
	poid_t			*evt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*prd_pdp = NULL;
	poid_t			*srv_pdp = NULL;
	poid_t			*act_pdp = NULL;
	poid_t			*new_serv_pdp = NULL;
	char			templatep[2001];
	int32			srch_flag = 768;
	int32			new_status_flags = 0xFFF;
	int32			is_base_pack = -1;
	int			active_status = 1;
	int			elem_id = 0;
	int			cancel_plan_flag = 4;
	int32			main_conn_type = -1;
	int32			tmp_conn_type = -1;
	int32			cancel_flag = -1;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	evt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	srv_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);

	main_conn_type = fm_mso_catv_conn_type(ctxp, srv_pdp, ebufp);

	srch_in_flist = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(evt_pdp), "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flist, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);

    	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &active_status, ebufp);

	/***************************************************************
	 * Check if main connection get account level purchased products
	 * otherwise service level purchased products 
	 **************************************************************/
    	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, args_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

	sprintf(templatep, "select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 ");
	PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_TEMPLATE, templatep, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_pdts search input flist", srch_in_flist);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flist, &srch_out_flist, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_pdts search output flist", srch_out_flist);

	PIN_FLIST_DESTROY_EX(&srch_in_flist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in mso_cancel_alc_addon_pdts search", ebufp);
		goto CLEANUP;
	}

	prov_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, prov_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, prov_in_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, prov_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &cancel_plan_flag, ebufp);
	
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flist, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		prd_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
		is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp);
	
		srv_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		tmp_conn_type = fm_mso_catv_conn_type(ctxp, srv_pdp, ebufp);

		/***************************************************************
		 * Check if main connection is having valid othe base pack
		 * If service is having other valid base pack, no need to 
		 * cancel add_on and alacarte packs.
		 **************************************************************/
		if (is_base_pack == 0 && main_conn_type == tmp_conn_type)
		{
			cancel_flag = 0;
			break;
		}
		else
		{
			cancel_flag = 1;
		}
		//Creating the Provisioning Action to Enable the Ala-Carte and Add-on Plans
		pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);
	}

	if (cancel_flag == 1)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Input flist", prov_in_flistp);
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Output flist", prov_out_flistp);

		PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
			goto CLEANUP;
		}

		/***************************************************************
		 * Update purchased_product status_flags to 4095 
		 **************************************************************/
		elem_id = 0;
		cookie = NULL;
		while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flist, 
			PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			write_flds_inflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);
		
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);
	
			PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
			PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
					"Error in mso_cancel_alc_addon_pdts: wflds", ebufp);
				break;
			}
		}
	}

	/*******************************************************************
	 * Check for child connection 
	 *******************************************************************/
	if (main_conn_type == 0 && cancel_flag != 0)
	{
		act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		
		fm_mso_utils_get_catv_children(ctxp, act_pdp, &child_serv_flist, ebufp);
		elem_id = 0;
		cookie = NULL;
		while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(child_serv_flist, 
		PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			new_serv_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 0, ebufp);
			mso_cancel_alc_addon_addconn_pdts(ctxp, i_flistp, new_serv_pdp, ebufp);
		}
	}

	PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flist, NULL);
	return;
}

static void
mso_cancel_alc_addon_addconn_pdts(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	poid_t            	 *new_serv_pdp,
	pin_errbuf_t            *ebufp)
{
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*srch_in_flist = NULL;
	pin_flist_t		*srch_out_flist = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*prov_out_flistp = NULL;
	pin_flist_t		*write_flds_inflistp = NULL;
	pin_flist_t		*write_flds_outflistp = NULL;
	pin_flist_t		*pdt_flistp = NULL;
	poid_t			*evt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	char			templatep[2001];
	int32			srch_flag = 768;
	int32			new_status_flags = 0xFFF;
	int			active_status = 1;
	int			elem_id = 0;
	int			cancel_plan_flag = 4;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	evt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	srch_in_flist = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(evt_pdp), "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flist, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &active_status, ebufp);

	/***************************************************************
	 * Check if main connection get account level purchased products
	 * otherwise service level purchased products 
	 **************************************************************/
	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, new_serv_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);
	sprintf(templatep, "select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 and F4 <> V4 ");
	
	PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_TEMPLATE, templatep, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_addconn_pdts search input flist", srch_in_flist);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flist, &srch_out_flist, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_addconn_pdts search output flist", srch_out_flist);

	PIN_FLIST_DESTROY_EX(&srch_in_flist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in mso_cancel_alc_addon_addconn_pdts search", ebufp);
		goto CLEANUP;
	}

	if (PIN_FLIST_ELEM_COUNT(srch_out_flist, PIN_FLD_RESULTS, ebufp) == 0) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_addconn_pdts search no results.", ebufp);
		goto CLEANUP;
	}

	prov_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, prov_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_SERVICE_OBJ, new_serv_pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, prov_in_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, prov_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &cancel_plan_flag, ebufp);
	
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flist, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		//Creating the Provisioning Action to Enable the Ala-Carte and Add-on Plans
		pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_addconn_pdts Provisioning Input flist", prov_in_flistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_addconn_pdts Provisioning Output flist", prov_out_flistp);

	PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
	PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_cancel_alc_addon_addconn_pdts : Error in MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
		goto CLEANUP;
	}

	/***************************************************************
	 * Update purchased_product status_flags to 4095 
	 **************************************************************/
	elem_id = 0;
	cookie = NULL;
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flist, 
		PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		write_flds_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);
	
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

		PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in mso_cancel_alc_addon_addconn_pdts: wflds", ebufp);
			break;
		}
	}
	PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flist, NULL);
	return;
}

static void
fm_check_charge_item_applied_commission(
	pcm_context_t   *ctxp,
	poid_t  *acct_pdp,
	poid_t  *charge_item_obj,
	pin_flist_t     **r_flistpp,
	pin_errbuf_t    *ebufp
)
{
	pin_flist_t     *search_flistp = NULL;
	pin_flist_t     *search_oflistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *temp_flistp = NULL;
	pin_flist_t     *drop_flistp = NULL;
	pin_flist_t     *chg_flistp = NULL;
	pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *tmp_ch_flistp = NULL;
	
	poid_t          *search_pdp = NULL;
	poid_t          *session_obj = NULL;
	poid_t          *pymt_session_obj = NULL;
	poid_t          *pymt_item_pdp = NULL;
	poid_t          *reverse_item_pdp = NULL;
	
	int32           flag = 512;
	int32           elem_id = 0;
	int32           p_elemid = 0;
	int32           database = 1;
	
	pin_cookie_t    cookie = NULL;
	pin_cookie_t    p_cookie = NULL;
	
	char            *template = "select X from /mso_commission_rpt/chrg_head_based where F1 = V1 and F2 = V2 ";
	
	if( PIN_ERRBUF_IS_ERR( ebufp ) )
		return;
	PIN_ERRBUF_CLEAR( ebufp );
	
	search_flistp = PIN_FLIST_CREATE( ebufp );
	search_pdp = PIN_POID_CREATE( database, "/search", -1, ebufp );
	PIN_FLIST_FLD_PUT( search_flistp, PIN_FLD_POID, search_pdp, ebufp );
	
	PIN_FLIST_FLD_SET( search_flistp, PIN_FLD_FLAGS, &flag, ebufp );
	PIN_FLIST_FLD_SET( search_flistp, PIN_FLD_TEMPLATE, template, ebufp );
	
	args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp );
	args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 2, ebufp );
	temp_flistp = PIN_FLIST_SUBSTR_ADD( args_flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp );
	PIN_FLIST_FLD_SET( temp_flistp, PIN_FLD_POID, charge_item_obj, ebufp );
	
	temp_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET( temp_flistp, PIN_FLD_POID, NULL, ebufp );
	PIN_FLIST_FLD_SET( temp_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );
	PIN_FLIST_FLD_SET( temp_flistp, PIN_FLD_SESSION_OBJ, NULL, ebufp );
	tmp_ch_flistp = PIN_FLIST_SUBSTR_ADD( temp_flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp );
	PIN_FLIST_FLD_SET ( tmp_ch_flistp, MSO_FLD_CHARGE_HEAD_TOTAL,NULL, ebufp );
	PIN_FLIST_FLD_SET ( tmp_ch_flistp, MSO_FLD_COLLECTION_AMT, NULL, ebufp );
	PIN_FLIST_FLD_SET ( tmp_ch_flistp, PIN_FLD_ITEM_OBJ, NULL, ebufp );
	PIN_FLIST_FLD_SET ( tmp_ch_flistp, PIN_FLD_POID, NULL, ebufp );
	PIN_FLIST_FLD_SET ( tmp_ch_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"post_rating_commission rev search charge head flist", search_flistp);
	PCM_OP( ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_oflistp, ebufp );
	PIN_FLIST_DESTROY_EX( &search_flistp, NULL );
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error during commission event charge search", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"post_rating_commission rev search charge head flist", search_oflistp);
	
	while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT( search_oflistp,																	PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL )
	{
		session_obj = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_SESSION_OBJ, 1, ebufp);
		temp_flistp = PIN_FLIST_SUBSTR_GET(res_flistp, MSO_FLD_CHARGE_HEAD_INFO, 1, ebufp);
		if ( strcmp(PIN_POID_GET_TYPE(session_obj), "/event/billing/batch/reversal") == 0){
			reverse_item_pdp = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_ITEM_OBJ, 1, ebufp);
			p_elemid = 0;
			p_cookie = NULL;
			while ((drop_flistp = PIN_FLIST_ELEM_GET_NEXT( search_oflistp,
						PIN_FLD_RESULTS, &p_elemid, 1, &p_cookie, ebufp )) != NULL )
			{
				pymt_session_obj = PIN_FLIST_FLD_GET(drop_flistp, PIN_FLD_SESSION_OBJ, 1, ebufp);
				if ( strcmp(PIN_POID_GET_TYPE(pymt_session_obj), "/event/billing/batch/reversal") != 0){
					chg_flistp = PIN_FLIST_SUBSTR_GET(drop_flistp, MSO_FLD_CHARGE_HEAD_INFO, 1, ebufp);
					pymt_item_pdp = PIN_FLIST_FLD_GET(chg_flistp, PIN_FLD_ITEM_OBJ, 1, ebufp);
					if ( PIN_POID_COMPARE(reverse_item_pdp, pymt_item_pdp,0,ebufp) == 0 )
					{
						PIN_FLIST_ELEM_DROP(search_oflistp, PIN_FLD_RESULTS, elem_id, ebufp);//removal of reverse results
						PIN_FLIST_ELEM_DROP(search_oflistp, PIN_FLD_RESULTS, p_elemid, ebufp);//removal of payment of reverse
					}
				}
			}
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"post_rating_commission final charge_head flist", search_oflistp);
	if (search_oflistp != NULL)
	{
		*r_flistpp = PIN_FLIST_COPY(search_oflistp,ebufp);
	}
	PIN_FLIST_DESTROY_EX(&search_oflistp, NULL);
	return;
}


static void 
fm_mso_find_decimal_entry(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t		**r_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*product_flistp = NULL;
	poid_t			*serv_pdp = NULL;	
	poid_t			*prod_pdp = NULL;
	int64			db = -1;
	pin_flist_t		*s_i_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*s_o_flistp = NULL;
	poid_t			*s_pdp = NULL;
	pin_flist_t		*res_flistp = NULL;
	int32			flags = 768;
	pin_flist_t		*bb_info_flistp = NULL;
	pin_decimal_t		*amount = NULL;
	char			*city = NULL;
	int32			*bill_when = NULL;
	int32			all_payterm = 0;
	pin_decimal_t		*total_arrayp = NULL;
	poid_t			*offer_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	char			*plan_name = NULL;
	int32			*c_bill_when = 0;
	char			*c_city = NULL;
	int			elem_id = 0;
	pin_cookie_t            cookie = NULL;
	time_t			*end_date = NULL;
	pin_decimal_t		*amnt_org = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_decimal_t		*zero_amount = NULL;
	int			*tax_flags = NULL;
	int32			matching_found = 0;
	time_t                  max_end_t = 0;
	time_t			*created_t = NULL;
	int                     elem_id_c = 0;
        pin_cookie_t            cookie_c = NULL;
	int                     elem_id_r = 0;
        pin_cookie_t            cookie_r = NULL;
	pin_flist_t		*cycle_fees_flistp = NULL;
	time_t			val=0;
	char                            tmp[1000];	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error before calling functon fm_mso_find_decimal_entry", ebufp);
                return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_find_decimal_entry input flist", i_flistp);
	serv_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	product_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PRODUCT, 1, ebufp);
	if(product_flistp)
	{
		offer_pdp = PIN_FLIST_SUBSTR_GET(product_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
	}
	total_arrayp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOTAL, 356, 1, ebufp);
	if(!total_arrayp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "doesn't contain the item total array");
                return;	
	}
	fm_mso_get_poid_details(ctxp, serv_pdp, &res_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error after calling functon fm_mso_get_poid_details", ebufp);
                return;
        }
	bb_info_flistp = PIN_FLIST_SUBSTR_GET(res_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	if(bb_info_flistp)
	{
		bill_when = PIN_FLIST_FLD_TAKE(bb_info_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
		city = PIN_FLIST_FLD_TAKE(bb_info_flistp, PIN_FLD_CITY, 0, ebufp);
		PIN_FLIST_DESTROY_EX(&res_flistp,NULL);	
	}
	
		fm_mso_get_poid_details(ctxp, offer_pdp, &res_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error after calling functon fm_mso_get_poid_details1", ebufp);
			return;
		}
		cookie_c = NULL;
		max_end_t = 0;
		if(PIN_FLIST_ELEM_COUNT(res_flistp, PIN_FLD_CYCLE_FEES, ebufp))
		{
			while ((cycle_fees_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, PIN_FLD_CYCLE_FEES,
							&elem_id_c, 1, &cookie_c, ebufp)) != NULL)
			{
				end_date = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
				if(end_date && *end_date > max_end_t)
				{
					max_end_t = *end_date;
				}
			}
		}
		else
		{
			max_end_t = *((time_t *)PIN_FLIST_FLD_TAKE(res_flistp, PIN_FLD_PURCHASE_START_T, 0, ebufp));
		}
		plan_pdp = PIN_FLIST_FLD_TAKE(res_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);	
		PIN_FLIST_DESTROY_EX(&res_flistp,NULL);
	//}
	/**get the plan name ******/
        fm_mso_get_poid_details(ctxp, plan_pdp, &res_flistp, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check000");
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error after calling functon fm_mso_get_poid_details1", ebufp);
                return;
        }
	zero_amount = pbo_decimal_from_str("0.0",ebufp);
	plan_name = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_NAME, 0, ebufp);
	tax_flags = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	s_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, s_i_flistp, PIN_FLD_POID, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_i_flistp, PIN_FLD_PLAN, 0, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PLAN_NAME, plan_name, ebufp);	
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);	
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, city, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PURCHASE_START_T, &max_end_t, ebufp);
	if(tax_flags && ((*tax_flags & PIN_RATE_FLG_INVERTED)))
	{
		PIN_FLIST_FLD_SET(s_i_flistp, PIN_FLD_ACTION, "Cancel_Rounding_Call", ebufp);
	}
	else
	{
        	PIN_FLIST_FLD_SET(s_i_flistp, PIN_FLD_ACTION, "Activation_Rounding_Call", ebufp);	
	}
	fm_mso_search_plan_details(ctxp, s_i_flistp, &s_o_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error after calling functon fm_mso_search_plan_details", ebufp);
                return;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_plan_details output flist", s_o_flistp);
	if(PIN_FLIST_ELEM_COUNT(s_o_flistp, PIN_FLD_RESULTS, ebufp))
	{
		res_flistp = PIN_FLIST_ELEM_GET(s_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		created_t = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_CREATED_T, 0, ebufp);
		sprintf(tmp, "created_t, max_end_t = %d, %d", *created_t, max_end_t);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
		cookie = NULL;
		while((args_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, MSO_FLD_CITIES,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			tmp_flistp = NULL;
			c_bill_when = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_BILL_WHEN, 1, ebufp);
			c_city = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_CITY, 1, ebufp);
			if(strcmp(city, c_city) == 0)
			{
				if(*c_bill_when == *bill_when)
				{
					amnt_org = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_AMOUNT_ORIG, 0, ebufp);	
					if(pbo_decimal_compare(amnt_org, zero_amount, ebufp)==0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Amount is not configured1");
						goto CLEANUP;
					}
					matching_found = 1;
					PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);
					PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_CITY, city, ebufp);	
					*r_flistpp = PIN_FLIST_COPY(args_flistp, ebufp);
					goto CLEANUP;
				}
				
			}
			else if(strcmp(c_city, "*") == 0)
			{
				if(*c_bill_when == *bill_when)
				{
					amnt_org = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_AMOUNT_ORIG, 0, ebufp);
					if(pbo_decimal_compare(amnt_org, zero_amount, ebufp)==0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Amount is not configured2");
						continue;
					}
					matching_found = 1;
					tmp_flistp = PIN_FLIST_ELEM_GET(res_flistp, MSO_FLD_CITIES, elem_id, 1, ebufp);
				}
				/*else if(*c_bill_when == 0) 
				{
					amnt_org = PIN_FLIST_FLD_GET(args_flistp, PIN_FLD_AMOUNT_ORIG, 0, ebufp);
					if(pbo_decimal_compare(amnt_org, zero_amount, ebufp)==0)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Amount is not configured3");
						continue;
					}
					if(tmp_flistp)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "already temp exists");
						continue;
					}
					matching_found == 1;
					tmp_flistp = PIN_FLIST_ELEM_GET(res_flistp, MSO_FLD_CITIES, elem_id, 1, ebufp);			
				}*/
			}
		}
		if(matching_found == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No matching records found in loop");
			goto CLEANUP;
		}
		else
		{
			if(tmp_flistp)
			{
				PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);
                       		PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_CITY, city, ebufp);
				*r_flistpp = PIN_FLIST_COPY(tmp_flistp, ebufp);
			}
                        goto CLEANUP;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No matching records found");
	}
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&s_i_flistp,NULL);
	PIN_FLIST_DESTROY_EX(&s_o_flistp,NULL);
	return;
}

PIN_EXPORT int32
is_charge_tax_code(
        pcm_context_t   *ctxp,
	char		*tax_code,
        pin_errbuf_t    *ebufp)
{
//	pin_flist_t		*srch_in_flistp = NULL;
//	pin_flist_t		*arg_flistp = NULL;
//	pin_flist_t		*result_flistp = NULL;
//	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*flistp = NULL;

//	int32			srch_flag = 512;
//	poid_t			*srch_pdp = NULL;

//	char			*template = "select X from /mso_cfg_gst_rates where F1 = V1 ";
//	int64			db = 1;
	int32			elem_count = 0;
	cm_cache_key_iddbstr_t  kids;
	int32                   err = PIN_ERR_NONE;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling is_charge_tax_code", ebufp);
	}

        /*
        * If the cache is not enabled, short circuit right away
        */
        if (fm_rate_pol_tax_codes_ptr == (cm_cache_t *)NULL) {
                return;
        }

        kids.id = 0;    /* Not relevant for us */
        kids.db = 0;    /* Not relevant for us */
        kids.str = tax_code;

        flistp = cm_cache_find_entry(fm_rate_pol_tax_codes_ptr, &kids, &err);
        switch(err) {
                case PIN_ERR_NONE:
                break;
                case PIN_ERR_NOT_FOUND:
                        PIN_ERRBUF_CLEAR(ebufp);       /* No real error... */
                break;
                default:
                        pin_set_err(ebufp, PIN_ERRLOC_CM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                        err, 0, 0, 0);
                return;
                        /*****/
        }


	/*srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_TAX_CODE, tax_code, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "is_charge_tax_code search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching tax code", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "is_charge_tax_code search output list", srch_out_flistp);
*/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cached flistp", flistp);
	
	if (flistp)
	{
		elem_count = 1;//PIN_FLIST_ELEM_COUNT(flistp, PIN_FLD_RESULTS, ebufp);
	}

	CLEANUP:
	//PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return elem_count;
}

static void
fm_rate_pol_transition_remove_taxes(
	pcm_context_t	*ctxp,
	pin_flist_t	**i_flistpp,
	char		*svc_type,
	pin_errbuf_t	*ebufp)
{
	pin_cookie_t	cookie = NULL;
	int32		rec_id = 0;

	pin_flist_t	*balImpact_flistp = NULL;
	pin_flist_t 	*tax_jur_flistp = NULL;
	pin_flist_t	*t_flistp = NULL;

	char		*rate_tag = NULL;
	char		*tax_code = NULL;
	char		*name = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rate_pol_transition_remove_taxes input flist", *i_flistpp);

	t_flistp = PIN_FLIST_COPY(*i_flistpp, ebufp);

	while ((balImpact_flistp = PIN_FLIST_ELEM_GET_NEXT(t_flistp, PIN_FLD_BAL_IMPACTS, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		rate_tag = PIN_FLIST_FLD_GET(balImpact_flistp, PIN_FLD_RATE_TAG, 1, ebufp);
		tax_code = PIN_FLIST_FLD_GET(balImpact_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
	
		if(svc_type && strcmp(svc_type,"/service/telco/broadband") == 0)
		{	
			if (rate_tag && tax_code && (strcmp(rate_tag, "Tax")==0 || strcmp(rate_tag, "MSO_ET")==0 ) && 
				(strcmp(tax_code, IGST)==0 || strcmp(tax_code, CGST)==0 || strcmp(tax_code, SGST)==0)) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Taxcode Matched");
				PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
			}

		}
		else if (svc_type && strcmp(svc_type,"/service/catv") == 0)
		{
                        if (rate_tag && tax_code && (strcmp(rate_tag, "Tax")==0 || strcmp(rate_tag, "MSO_ET")==0 ) &&
                        	(strcmp(tax_code, MSO_ET_MAIN)==0 || strcmp(tax_code, MSO_ET_ADDI)==0))
                        {
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Taxcode Matched");
                                PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_BAL_IMPACTS, rec_id , ebufp);
                        }
		}
	}

	if(svc_type && strcmp(svc_type,"/service/telco/broadband") == 0)
	{
		cookie = NULL;
		rec_id = 0;
        	while ((tax_jur_flistp = PIN_FLIST_ELEM_GET_NEXT(t_flistp, PIN_FLD_TAX_JURISDICTIONS, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        	{
                	name = PIN_FLIST_FLD_GET(tax_jur_flistp, PIN_FLD_NAME, 1, ebufp);

                	if (name && strstr(name, "GST Tax Component") != NULL)
                	{
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Tax Jurisdictions Matched");
                        	PIN_FLIST_ELEM_DROP(*i_flistpp, PIN_FLD_TAX_JURISDICTIONS, rec_id , ebufp);
                	}
        	}
	}


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_rate_pol_transition_remove_taxes return flistp", *i_flistpp);
	PIN_FLIST_DESTROY_EX(&t_flistp, ebufp);
	return;

}

/*Function to recalculate charges of 
subscription with the data in 
/mso_adv_payment_chgs for NRC */

static void
mso_adv_payment_chgs(
        pcm_context_t   *ctxp,
        pin_flist_t  	*i_flistp,
		pin_flist_t     **mso_nrc_paymt_trnsfr_rflistpp,
        char  		*svc_type,
	int		*flags,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t     *search_flistp = NULL;
        pin_flist_t     *search_oflistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *temp_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;

        poid_t          *search_pdp = NULL;
        int32           flag = 512;
        int32           elem_id = 0;
        int32           database = 0;

        pin_cookie_t    cookie = NULL;
	poid_t		*evt_pdp = NULL;
        char            *template = "select X from /mso_adv_payment_chgs where F1 = V1 and F2 > V2 ";
	pin_flist_t	*total_arrayp = NULL;
	poid_t		*acc_pdp = NULL;
	pin_decimal_t	*zerop = NULL;
	pin_decimal_t	*totl_amt = NULL;
	pin_decimal_t	*advance_bal = NULL;
	pin_decimal_t	*upd_amt = NULL;
	pin_decimal_t   *adv_bal = NULL;
	pin_decimal_t	*tax_amt = NULL;
	pin_decimal_t	*scale_perc = NULL;
	pin_flist_t	*write_flds_inflistp = NULL;
	pin_flist_t	*write_flds_outflistp = NULL;
	int32           precision = 2;
	pin_decimal_t	*adv_consume = NULL;
	pin_decimal_t	*bal_imp_amt = NULL;
	poid_t		*purc_pdp = NULL;
	pin_flist_t	*bal_imp_flistp = NULL;
	int32		update = 0;
	char                    msg[1024];	
	pin_flist_t	*mso_nrc_paymt_trnsfr_flistp = NULL;
	pin_flist_t	*inner_paymt_trnsfr_flistp = NULL;
	poid_t		*trf_pdp = NULL;
	int32		trf_recid = 0;

        if( PIN_ERRBUF_IS_ERR( ebufp ) )
                return;
        PIN_ERRBUF_CLEAR( ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG," mso_adv_payment_chgs input flist", i_flistp);	
	
	evt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	database = PIN_POID_GET_DB(evt_pdp);
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	if((svc_type && strcmp(svc_type,"/service/catv")) && (strstr(PIN_POID_GET_TYPE(evt_pdp),"/event/billing/product/fee/cycle")) && flags && !(*flags & PIN_RATE_FLG_INVERTED))
	{
		total_arrayp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_TOTAL, 356, 1, ebufp);
		if(total_arrayp != NULL)
		{
			zerop = pbo_decimal_from_str("0.0",ebufp);
			totl_amt = PIN_FLIST_FLD_GET(total_arrayp, PIN_FLD_AMOUNT, 0, ebufp);
			if(pbo_decimal_compare(totl_amt, zerop, ebufp) <= 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Amount is less then or equal to zero");
				return ;
			}
		}
        	else  
        	{
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Doesn't have item total array");
                	return;
        	}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not a cycle event and broadband service and subscription flow");
                return;
	}

        search_flistp = PIN_FLIST_CREATE( ebufp );
        search_pdp = PIN_POID_CREATE( database, "/search", -1, ebufp );
        PIN_FLIST_FLD_PUT( search_flistp, PIN_FLD_POID, search_pdp, ebufp );

        PIN_FLIST_FLD_SET( search_flistp, PIN_FLD_FLAGS, &flag, ebufp );
        PIN_FLIST_FLD_SET( search_flistp, PIN_FLD_TEMPLATE, template, ebufp );

        args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp );
	
	args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 2, ebufp );	
	PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_CURRENT_BAL, zerop, ebufp );	

        temp_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_adv_payment_chgs search in_flist", search_flistp);
        PCM_OP( ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_oflistp, ebufp );
        PIN_FLIST_DESTROY_EX( &search_flistp, NULL );
        if (PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error during mso_adv_payment_chgs  search", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_adv_payment_chgs search out_flist", search_oflistp);
	
	if(PIN_FLIST_ELEM_COUNT(search_oflistp, PIN_FLD_RESULTS, ebufp) == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account details not present in this class mso_adv_payment_chgs");
		goto CLEANUP;
	}
	res_flistp = PIN_FLIST_ELEM_GET(search_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	advance_bal = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
	if(pbo_decimal_compare(advance_bal, zerop, ebufp) > 0)
	{
		/*BEGIN: Change for NRC Object*/
		mso_nrc_paymt_trnsfr_flistp = PIN_FLIST_CREATE( ebufp );
        trf_pdp = PIN_POID_CREATE( database, "/mso_adv_payment_transfer", -1, ebufp );
		PIN_FLIST_FLD_PUT( mso_nrc_paymt_trnsfr_flistp, PIN_FLD_POID, trf_pdp, ebufp );
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, mso_nrc_paymt_trnsfr_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, mso_nrc_paymt_trnsfr_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		/*END: Change for NRC Object*/
		
		cookie = NULL;
		while ((bal_imp_flistp =
				PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
			       &elem_id, 1, &cookie, ebufp)) != NULL)
		{
			purc_pdp = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);	
			if(purc_pdp && !PIN_POID_IS_NULL(purc_pdp))
			{
				adv_consume = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_REQ_AMOUNT, 0, ebufp);
				bal_imp_amt = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_AMOUNT, 0, ebufp);
				if((pbo_decimal_compare(bal_imp_amt, advance_bal, ebufp) > 0))
				{
                                        adv_consume = pbo_decimal_add(advance_bal, adv_consume, ebufp);
					upd_amt = pbo_decimal_subtract(bal_imp_amt, advance_bal, ebufp);
					adv_bal = pbo_decimal_from_str("0.0",ebufp);
						
				}
				else if((pbo_decimal_compare(bal_imp_amt, advance_bal, ebufp) <= 0))	
				{
					adv_bal = pbo_decimal_subtract(advance_bal, bal_imp_amt, ebufp);
					upd_amt = pbo_decimal_from_str("0.0",ebufp);
                                        adv_consume = pbo_decimal_add(bal_imp_amt, adv_consume, ebufp);
				}
				
				/*BEGIN: Change for NRC Object*/
				inner_paymt_trnsfr_flistp = PIN_FLIST_ELEM_ADD(mso_nrc_paymt_trnsfr_flistp, PIN_FLD_BAL_IMPACTS, trf_recid, ebufp);
                PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_CURRENT_BAL, inner_paymt_trnsfr_flistp, PIN_FLD_CURRENT_BAL, ebufp);
				PIN_FLIST_FLD_SET(inner_paymt_trnsfr_flistp, PIN_FLD_CURRENT_TOTAL, (void*)adv_bal, ebufp);
				PIN_FLIST_FLD_SET(inner_paymt_trnsfr_flistp, PIN_FLD_AMOUNT, (void*)pbo_decimal_round(upd_amt, precision, ROUND_HALF_UP, ebufp),ebufp);
				PIN_FLIST_FLD_COPY(bal_imp_flistp, PIN_FLD_AMOUNT, inner_paymt_trnsfr_flistp, PIN_FLD_AMOUNT_GROSS, ebufp);
				PIN_FLIST_FLD_COPY(bal_imp_flistp, PIN_FLD_IMPACT_TYPE, inner_paymt_trnsfr_flistp, PIN_FLD_IMPACT_TYPE, ebufp);
				PIN_FLIST_FLD_COPY(bal_imp_flistp, PIN_FLD_TAX_CODE, inner_paymt_trnsfr_flistp, PIN_FLD_TAX_CODE, ebufp);
				trf_recid++;
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_adv_payment_trnsfr input flist", mso_nrc_paymt_trnsfr_flistp);
				/*END: Change for NRC Object*/
				
				/************Updating /mso_adv_payment_chgs with updated amount**************/
				write_flds_inflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_PUT(write_flds_inflistp, PIN_FLD_CURRENT_BAL, adv_bal, ebufp);
				PIN_FLIST_FLD_PUT(write_flds_inflistp, PIN_FLD_REQ_AMOUNT, adv_consume, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_adv_payment_chgs write flds input flist", write_flds_inflistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"Error in mso_adv_payment_chgs: wflds", ebufp);
					goto CLEANUP;
				}
				PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
                		PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
				PIN_FLIST_FLD_PUT(bal_imp_flistp, PIN_FLD_AMOUNT, (void*)pbo_decimal_round( upd_amt ,
                                                                                        precision, ROUND_HALF_UP, ebufp),ebufp);
				update = 1;	
			}
			else
			{
				if(update == 0)
					return;	
				tax_amt = pbo_decimal_from_str("0.0",ebufp);
				scale_perc = PIN_FLIST_FLD_GET(bal_imp_flistp, PIN_FLD_PERCENT, 0, ebufp);
                                tax_amt = pbo_decimal_multiply(upd_amt, scale_perc, ebufp);
				PIN_FLIST_FLD_PUT(bal_imp_flistp, PIN_FLD_AMOUNT, (void*)pbo_decimal_round( tax_amt ,
                                                                                        precision, ROUND_HALF_UP, ebufp),ebufp);
                                PIN_FLIST_FLD_PUT(bal_imp_flistp, PIN_FLD_QUANTITY, (void*)pbo_decimal_round( upd_amt,
                                                                                        precision, ROUND_HALF_UP, ebufp),ebufp);	
			}
		}
		
	}	
	CLEANUP:
	PIN_FLIST_DESTROY_EX( &search_oflistp, NULL );	
	if (!pbo_decimal_is_null(tax_amt, ebufp))
        {
                pbo_decimal_destroy(&tax_amt);
        }
	if (!pbo_decimal_is_null(upd_amt, ebufp))
        {
                pbo_decimal_destroy(&upd_amt);
        }
	if (!pbo_decimal_is_null(zerop, ebufp))
        {
                pbo_decimal_destroy(&zerop);
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG," mso_adv_payment_chgs output flist", i_flistp);
	
	/*BEGIN: Change for NRC Object*/
	if (mso_nrc_paymt_trnsfr_flistp != NULL)
	{
		*mso_nrc_paymt_trnsfr_rflistpp = PIN_FLIST_COPY(mso_nrc_paymt_trnsfr_flistp, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&mso_nrc_paymt_trnsfr_flistp, NULL);
	/*END: Change for NRC Object*/
	
	return;	
}

/*******************************************************************
 * This function adds NCR balance to update wallet balance
 ******************************************************************/
static void
mso_update_wallet_balance(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *bi_flistp = NULL;
        pin_flist_t             *tmp_bi_flistp = NULL;
        pin_flist_t             *total_flistp = NULL;
        pin_flist_t             *bal_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
	pin_flist_t		*wallet_flistp = NULL;
	pin_flist_t             *bal_impacts = NULL;
        poid_t                  *evt_pdp = NULL;
        poid_t                  *acct_pdp = NULL;
	pin_cookie_t		cookie = NULL;
        pin_decimal_t           *amtp = NULL;
        pin_decimal_t           *orig_amtp = NULL;
        pin_decimal_t           *ref_balp = NULL;
        pin_decimal_t           *abs_ref_balp = NULL;
	pin_decimal_t		*zerop = NULL;
	pin_decimal_t		*new_amtp = NULL;
	pin_decimal_t		*disc_amtp = NULL;
	pin_decimal_t		*total_amtp = NULL;
	pin_decimal_t		*pctp = NULL;
        char                    *rate_tagp = "Wallet Bal";
        char                    *evt_typep = NULL;
	char			*tax_codep = NULL;
        int32                   elem_count = -1;
        int32                   wal_res_id = WALLET_RES_ID;
        int32                   ref_res_id = REF_RES_ID;
        int32                   res_id = 0;
        int32                   channel_id = 0;
	int32			res_val = 0;
	int32			ref_full_flag = -1;
	int32			elemid = 0;
	int32                   elem_id_cnt = 0;
	pin_cookie_t            cookie_cnt = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_update_wallet_balance input flist", i_flistp);

        evt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	zerop = pbo_decimal_from_str("0.0",ebufp);
	total_amtp = pbo_decimal_from_str("0.0",ebufp);

        evt_typep = (char *)PIN_POID_GET_TYPE(evt_pdp);

        if (strstr(evt_typep, "/event/billing/payment") != NULL)
        {
                tmp_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PAYMENT, 0, ebufp);
                channel_id = *(int32 *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CHANNEL_ID, 0, ebufp);
                if (channel_id)
                {
                        goto CLEANUP;
                }
        }
	else if (strstr(evt_typep, "/event/billing/adjustment/account") != NULL)
	{
                tmp_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_EVENT_MISC_DETAILS, 0, 0, ebufp);
                channel_id = *(int32 *)PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_REASON_ID, 0, ebufp);
		if (channel_id == 28)
		{
			goto CLEANUP;
		}
	}
	else if (strstr(evt_typep, "/event/billing/reversal"))
	{
		goto CLEANUP;
	}
	else
	{
	}

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	/****************************************************************************
	 * Check multi pack refund balance
	 ***************************************************************************/
	res_val = fm_mso_get_wallet_balance(ctxp, acct_pdp, ref_res_id, &wallet_flistp, ebufp);

	if (res_val)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error calling fm_mso_get_wallet_balance", ebufp);
		goto CLEANUP;
	}
	else if (wallet_flistp)
	{
        	bal_flistp = PIN_FLIST_ELEM_GET(wallet_flistp, PIN_FLD_BALANCES, ref_res_id, 1, ebufp);
		if (bal_flistp)
		{
        		ref_balp = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
		}
	}
	else
	{
	}

        bi_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BAL_IMPACTS, 0, 0, ebufp);
        tmp_bi_flistp = PIN_FLIST_COPY(bi_flistp, ebufp);

        orig_amtp = pbo_decimal_copy(PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_AMOUNT, 0, ebufp), ebufp);

	if (pbo_decimal_is_zero(orig_amtp, ebufp))	
	{
		goto CLEANUP;
	}

	/************************************************************************
	 * This is the debit transaction and hence need to check refund balance,
	 * to offset currency balance impacts
	 ***********************************************************************/
	if (pbo_decimal_compare(orig_amtp, zerop, ebufp) > 0 &&
		!pbo_decimal_is_null(ref_balp, ebufp) &&
		pbo_decimal_compare(ref_balp, zerop, ebufp) < 0)
	{
		abs_ref_balp = pbo_decimal_abs(ref_balp, ebufp);
		if (pbo_decimal_compare(orig_amtp, abs_ref_balp, ebufp) > 0)
		{
			new_amtp = pbo_decimal_subtract(orig_amtp, abs_ref_balp, ebufp);
			ref_full_flag = 0;
		}
		else
		{
			ref_full_flag = 1;
		}
	}	

	while ((bi_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
			&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		res_id = *(int32 *)PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_RESOURCE_ID, 0, ebufp);
        	amtp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_AMOUNT, 0, ebufp);
        	disc_amtp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
		tax_codep = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
		if (res_id == 356)
		{ 
			if (ref_full_flag == 1)
			{
       				PIN_FLIST_FLD_PUT(bi_flistp, PIN_FLD_DISCOUNT, 
					pbo_decimal_add(disc_amtp, amtp, ebufp),  ebufp);
       				PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_AMOUNT, zerop,  ebufp);
			}
			else if (ref_full_flag == 0)
			{
				if (strstr(tax_codep, "GST"))
				{
					pctp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_PERCENT, 0, ebufp);
       					PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_QUANTITY, new_amtp,  ebufp);
       					PIN_FLIST_FLD_PUT(bi_flistp, PIN_FLD_DISCOUNT,
					pbo_decimal_subtract(amtp, pbo_decimal_round(pbo_decimal_multiply(new_amtp, pctp, ebufp),
						2, ROUND_HALF_UP, ebufp), ebufp), ebufp);
       					PIN_FLIST_FLD_PUT(bi_flistp, PIN_FLD_AMOUNT, 
					pbo_decimal_round(pbo_decimal_multiply(new_amtp, pctp, ebufp), 2, ROUND_HALF_UP, ebufp), ebufp);
				}
				else
				{
       					PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_AMOUNT, new_amtp,  ebufp);
       					PIN_FLIST_FLD_PUT(bi_flistp, PIN_FLD_DISCOUNT,
					pbo_decimal_add(disc_amtp, abs_ref_balp, ebufp),  ebufp);
				}
        			amtp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_AMOUNT, 0, ebufp);
				pbo_decimal_add_assign(total_amtp, amtp, ebufp);
			}
			else
			{
				pbo_decimal_add_assign(total_amtp, amtp, ebufp);
			}
		}
	}

        PIN_FLIST_FLD_SET(tmp_bi_flistp, PIN_FLD_RATE_TAG, rate_tagp, ebufp);

	if (ref_full_flag == 1)
	{
       		PIN_FLIST_FLD_SET(tmp_bi_flistp, PIN_FLD_AMOUNT, orig_amtp, ebufp);
	}
	else if (ref_full_flag == 0)
	{
       		PIN_FLIST_FLD_SET(tmp_bi_flistp, PIN_FLD_AMOUNT, abs_ref_balp, ebufp);
	}
	else
	{
		/***********************************************************
		 * This case is no refund balance
		 **********************************************************/
       		PIN_FLIST_FLD_SET(tmp_bi_flistp, PIN_FLD_AMOUNT, total_amtp, ebufp);
	}

	if (ref_full_flag != -1)
	{
		PIN_FLIST_FLD_SET(tmp_bi_flistp, PIN_FLD_RESOURCE_ID, &ref_res_id, ebufp);
		while ((bal_impacts = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
                                         &elem_id_cnt, 1, &cookie_cnt, ebufp)) != (pin_flist_t *)NULL)
                                {
				if (elem_count < elem_id_cnt) {
					elem_count = elem_id_cnt;
					}
				}
        
		//elem_count = PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_BAL_IMPACTS, ebufp);
		elem_count=elem_count+1;
        	PIN_FLIST_ELEM_SET(i_flistp, tmp_bi_flistp, PIN_FLD_BAL_IMPACTS, elem_count, ebufp);
	
	}

	if (ref_full_flag == 0)
	{
		pbo_decimal_add_assign(total_amtp, abs_ref_balp, ebufp);
       		PIN_FLIST_FLD_SET(tmp_bi_flistp, PIN_FLD_AMOUNT, total_amtp, ebufp);
	}

	cookie = NULL;
	elem_id_cnt = 0;

        PIN_FLIST_FLD_SET(tmp_bi_flistp, PIN_FLD_RESOURCE_ID, &wal_res_id, ebufp);

	while ((bal_impacts = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS,
		&elem_id_cnt, 1, &cookie_cnt, ebufp)) != (pin_flist_t *)NULL)
	{
		if (elem_count < elem_id_cnt) {
			elem_count = elem_id_cnt;
		}
	}

	elem_count=elem_count+1;

        PIN_FLIST_ELEM_SET(i_flistp, tmp_bi_flistp, PIN_FLD_BAL_IMPACTS, elem_count, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_update_wallet_balance return with wallet flist", i_flistp);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&tmp_bi_flistp, NULL);
	if (orig_amtp) pbo_decimal_destroy(&orig_amtp);
	if (abs_ref_balp) pbo_decimal_destroy(&abs_ref_balp);
	pbo_decimal_destroy(&zerop);
	pbo_decimal_destroy(&total_amtp);
        return;
}
