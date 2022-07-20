/*******************************************************************
 * File:	mso_cust_common_functions.c
 * Opcode:	None
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description:  Common functions for "fm_mso_cust" module. The
 *               functions should be moved to global common 
 *               place if other modules need to use the same function
 *
 * Modification History: 
 * Modified By	     | Date 	   | Modification details
 * --------------------------------------------------------------------------------------
 * Sachidanand Joshi | 22-Jul-2014 | Allowed starter plans to add one regional south plan
 * Sachidanand Joshi | 24-Jul-2014 | Bug fix
 * Leela Pratyush    | 20-Aug-2014 | Added function for Lifecycle events
		                     Fixed memory leak issue in fm_mso_get_account_info
 * Jyothirmayi K     | 30-Dec-2019 | Added function for Lifecycle events for Hybrid Accounts
 * Jyothirmayi K     | 14-Mar-2020 | Bug fix in validate_hybrid_acnt_type function to handle
                                     cases where bill_info_id is null / empty / any value 
                                     other than CATV or BB 
 *******************************************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_utils_common_functions.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/cust.h"
#include "ops/act.h"
#include "ops/bill.h"
#include "ops/pymt.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "pin_pymt.h"
#include "pin_flds.h"

#include "mso_cust.h"
#include "pin_currency.h"
#include "ops/bal.h"
#include "mso_lifecycle_id.h"
#include "mso_errs.h"
#include "pin_cust.h"
#include "mso_utils.h"
#include "mso_prov.h"

#define WHOLESALE "/profile/wholesale"
#define MSO_FLAG_SRCH_BY_SELF_ACCOUNT 9

/***********************************************
* Function Definition 
***********************************************/

void fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

void fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

void
fm_mso_get_deals_from_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_deal_flistp,
	pin_flist_t		**r_limit_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_products_from_deal(
	pcm_context_t		*ctxp,
	int32			fliter_products,
	int32			add_info_flag,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp);
void
fm_mso_get_deal_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**deals_deposit,
	pin_flist_t		**deals_activation,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_device_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_profile_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT char*
fm_mso_utils_get_state_code(
	pcm_context_t	*ctxp,
	poid_t			*account_pdp,
	char			*state_name,
	pin_errbuf_t	*ebufp);

PIN_EXPORT int32
fm_mso_is_den_sp(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT int32 
fm_mso_utils_gst_sequence_no(
	pcm_context_t	*ctxp,
	poid_t		*account_pdp,
	char		*name,
	pin_errbuf_t	*ebufp);
	
PIN_EXPORT void
fm_mso_hier_group_get_parent(
	pcm_context_t		*ctxp,
        poid_t                  *serv_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

PIN_EXPORT void
fm_mso_fetch_offer_purchased_plan(
	pcm_context_t           *ctxp,
        poid_t                  *acc_poidp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t            *ebufp);

PIN_EXPORT pin_decimal_t *
get_ar_curr_total(
        pcm_context_t   *ctxp,
        poid_t          *ar_billinfo_pdp,
        pin_errbuf_t    *ebufp);

void
fm_mso_read_bal_grp(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	poid_t			*srvc_pdp,
	pin_flist_t		**bal_grp_oflist,
	pin_errbuf_t		*ebufp);
void
fm_mso_get_billinfo(
	pcm_context_t		*ctxp,
	poid_t			*bal_grp_obj,
	poid_t			**billinfo,
	pin_errbuf_t		*ebufp);

void
fm_mso_read_billinfo(
	pcm_context_t		*ctxp,
	poid_t			*billinfo_obj,
	pin_flist_t		**billinfo_out_flist,
	pin_errbuf_t		*ebufp);
void 
fm_mso_utils_sequence_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);
void
fm_mso_filter_deposit_prod(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**deals_deposit,
	pin_flist_t		**deals_activation,
	pin_errbuf_t		*ebufp);

char*
fm_mso_encrypt_passwd(
	pcm_context_t		*ctxp,
	poid_t			*service_pdp,
	char			*non_encrypted_pwd,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_all_billinfo(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	int32			billinfo_type,
	pin_flist_t		**billinfo,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_err_desc(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_business_type_values(
	pcm_context_t		*ctxp,
	int32			business_type,
	int32			*account_type,
	int32			*account_model,
	int32			*subscriber_type,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_account_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_state_city_area_code(
	pcm_context_t		*ctxp,
	char			*state_city_area_code,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void 
replace_char (
	char	*sptr, 
	char	find, 
	char	replace);

PIN_EXPORT
void insert_char (
	char	*srcp,
	char	in_char,
	char	*dstp);

PIN_EXPORT void 
num_to_array( 
	int32	num, 
	int32	arr[]);

void
fm_mso_validation_rule_1(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			action,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_validation_rule_2(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			action,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_validate_pay_type(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			opcode,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_validate_pay_type_corp(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			opcode,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

int32 
fm_mso_get_parent_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT void
fm_mso_is_child_service(
	pcm_context_t		*ctxp,
	poid_t			*serv_pdp,
	int32			parent_info_flag,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void 
fm_mso_et_product_rule_delhi(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	poid_t			*acnt_pdp,
	int32			action,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_bill_segment(
	pcm_context_t		*ctxp,
	char			*state_code,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_acnt_from_acntno(
        pcm_context_t           *ctxp,
        char*                   acnt_no,
        pin_flist_t             **acnt_details,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_lco_services_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        char                    *ser_type,
        pin_errbuf_t            *ebufp);

PIN_EXPORT int32
fm_mso_catv_pt_pkg_type(
	pcm_context_t           *ctxp,
	poid_t			*prd_pdp,
	pin_errbuf_t            *ebufp);

PIN_EXPORT int32
fm_mso_bb_pt_pkg_type(
        pcm_context_t           *ctxp,
        poid_t                  *prd_pdp,
	pin_flist_t             *events_flistp,
        pin_errbuf_t            *ebufp);




PIN_EXPORT int32
fm_mso_catv_conn_type(
	pcm_context_t           *ctxp,
	poid_t			*srv_pdp,
	pin_errbuf_t            *ebufp);

PIN_EXPORT void 
fm_mso_utils_get_catv_children(
	pcm_context_t		*ctxp,
	poid_t			*act_pdp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT void 
fm_mso_utils_get_catv_main(
	pcm_context_t		*ctxp,
	poid_t			*act_pdp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT int32 
validate_demo_packs(
	pcm_context_t	*ctxp,
        pin_flist_t     *in_flistp,
	pin_flist_t	**ret_flistp,
        pin_errbuf_t    *ebufp);

/* CM_RP_0001: Gather the product information - beings */
//Input  : Subscriber Account POID 
//Output : LCO Account POID and Commission model
void
fm_mso_util_get_customer_lco_info(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_util_get_lco_settlement_services(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_read_object (
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *objp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

void 
fm_mso_utils_get_bal_grp
(
    pcm_context_t       *ctxp,
    poid_t              *act_pdp,
    char		*flags,
    poid_t             	**ret_bal_grp_poidp,
    poid_t             	**ret_serv_poidp,
    pin_errbuf_t       	*ebufp);

void 
fm_mso_utils_calc_tax_share
(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
	pin_decimal_t		*lco_share,
	pin_decimal_t		**lco_tax_p,
	pin_decimal_t		**hath_share_p,
    pin_errbuf_t        *ebufp);

void
fm_mso_account_adjustment (
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistpp,
	pin_errbuf_t	*ebufp);

void
fm_mso_utils_prepare_status_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        int                     flag,
        char                    *status_descr,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_prepare_error_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        char                    *error_code,
        char                    *error_descr,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_services_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        char                    *ser_type,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_lco_services_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        char                    *ser_type,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_item_type (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_lco_settlement_report(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_plan_types (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        int32                   srv_type,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);


/* CM_RP_0001: Gather the product information - ends */

void
fm_pymt_pol_pymt_priority_search(
        pcm_context_t           *ctxp,
        poid_t                  *a_pdp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_pymt_pol_get_item_by_charge_head(
	cm_nap_connection_t     *connp,
        char            	*header,
	char			**item_tag,
        pin_errbuf_t    	*ebufp);

char *
fm_pymt_pol_get_config_item_tag(
        pcm_context_t   *ctxp,
        poid_t          *a_pdp,
        char            *i_type,
        pin_errbuf_t    *ebufp); 

void fm_mso_utils_close_context(
	pcm_context_t  		*ctxp,
        pin_errbuf_t            *ebufp);

void fm_mso_utils_open_context(
	pin_flist_t    		*i_flistp,
	pcm_context_t           *ctxp,
	pcm_context_t  		**new_ctxp,
        pin_errbuf_t            *ebufp);

void fm_mso_utils_get_billinfo_bal_details(
	pcm_context_t		*ctxp,
	poid_t				*acct_pdp,
	char				*billinfo_id,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT void 
	fm_mso_utils_get_billinfo_bal_details_woserv(
	pcm_context_t		*ctxp,
	poid_t				*acct_pdp,
	char				*billinfo_id,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

void fm_mso_utils_balance_transfer(
	pcm_context_t		*ctxp,
	poid_t			*s_acct_pdp,
	poid_t			*d_acct_pdp,
	poid_t			*s_bal_pdp,
	poid_t			*d_bal_pdp,
	pin_decimal_t		*amount,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

void
fm_mso_utils_read_any_object(
	pcm_context_t		*ctxp,
	poid_t			*objp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);

void fm_mso_utils_get_service_details(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	int32			serv_type,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT void 
fm_mso_utils_get_close_service_details(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	int32			serv_type,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);


void fm_mso_utils_get_service_from_balgrp(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

void fm_mso_utils_create_pre_rated_impact(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	poid_t			*service_pdp,
	char			*program_name,
	char			*sys_description,
	char			*event_type,
	pin_decimal_t		*amount,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

void
fm_mso_utils_convert_flist_to_buffer (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

int32 fm_mso_utils_get_tax_excemption_details(
        pcm_context_t           *ctxp,
	pin_flist_t             *exempt_out_flistp,
        pin_errbuf_t            *ebufp);


void
fm_mso_utils_gen_event(
	pcm_context_t   *ctxp,
	poid_t          *acct_pdp,
	poid_t          *serv_pdp,
	char            *program,
	char            *descr,
	char            *event_type,
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp);

void
fm_mso_utils_prioritize_pymt_items(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

void 
fm_mso_utils_get_open_bill_items(
	pcm_context_t		*ctxp,
	poid_t			*ar_bill_pdp,
    int32               all_items_flag,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

void 
fm_mso_utils_get_pending_bill_items(
	pcm_context_t		*ctxp,
	poid_t			*billinfo_pdp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);


void
fm_mso_get_object_poid (
        pcm_context_t           *ctxp,
        char                    *template,
        pin_flist_t             *arg_flistp,
        void                    **ret_objp,
        pin_errbuf_t            *ebufp) ;

int32
fm_mso_utils_get_account_type(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
        pin_errbuf_t            *ebufp);

void 
fm_mso_update_ncr_validty(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);
	

void fm_mso_utils_get_event_action(
	char		*poid_type,
	char		**descr,
	pin_errbuf_t	*ebufp);

void
get_evt_lifecycle_template(
	pcm_context_t			*ctxp,
	int64				db,
	int32				string_id, 
	int32				string_version,
	char				**lc_template, 
	pin_errbuf_t			*ebufp);
	
PIN_EXPORT void
get_strings_template(
	pcm_context_t       *ctxp,
	int64				db,
    char                *domainp,
	int32				string_id, 
	int32				string_version,
	char				**lc_template, 
	pin_errbuf_t	    *ebufp);
	
void
fm_mso_utils_gen_lifecycle_evt(
        pcm_context_t   *ctxp,
        poid_t          *acct_pdp,
        poid_t          *serv_pdp,
        char            *program,
        char            *descr,
	char		*name,
        char            *event_type,
        pin_flist_t     *in_flistp,
        pin_errbuf_t    *ebufp);	

void
fm_mso_check_csr_access_level(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *acnt_info,
        pin_flist_t             **ret_flist,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_srvc_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_cust_get_plan_code(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_mso_cust_credentials(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_acntinfo_from_bill(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_set_bp_status(
	pcm_context_t           *ctxp,
	poid_t                  *service_obj,
	poid_t                  *bp_obj,
	int32					old_status,
	pin_errbuf_t            *ebufp);
void
fm_mso_create_ar_limit(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        pin_flist_t             **ar_limit,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_credit_profile(
	pcm_context_t		*ctxp,
	int32			elem_id,
	pin_flist_t		**credit_limit_array,
	pin_errbuf_t		*ebufp);

void
fm_mso_get_srvc_of_bal_grp(
	pcm_context_t		*ctxp,
	poid_t			*in_flist,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);

void
fm_mso_read_bill(
	pcm_context_t		*ctxp,
	poid_t			*in_flist,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp);
	
void
fm_mso_get_channels_from_prov_tag(
        pcm_context_t           *ctxp,
        char*                   prov_tag,
        pin_flist_t             **channel_details,
        pin_errbuf_t            *ebufp);

int32
fm_mso_convert_ebuf_to_msg(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistp,
	char			*error_code,
	char			*error_descr,
	pin_errbuf_t		*ebufp);

int
fm_mso_fetch_purchased_plan(
        pcm_context_t           *ctxp,
        int                     *flags,
        pin_flist_t             *in_flistp,
        poid_t                  **o_poidpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_fetch_ar_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        poid_t                  *acct_pdp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);
	

int
fm_mso_purch_prod_status (
	pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

int32
fm_mso_get_product_priority(
        pcm_context_t           *ctxp,
        poid_t                  *prod_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_poid_details(
        pcm_context_t           *ctxp,
        poid_t                  *poid_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_purchased_prod_active(
        pcm_context_t           *ctxp,
        pin_flist_t		*inp_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_purchased_prod_all(
        pcm_context_t           *ctxp,
        poid_t                  *inp_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

PIN_EXPORT  void 
fm_mso_get_pref_bdom (
        pcm_context_t   *ctxp,
        char            *area,
        pin_flist_t     **r_flistp,
        pin_errbuf_t    *ebufp);

void
fm_mso_catv_add_demo_pack(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	poid_t                  *act_pdp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp);

PIN_EXPORT void
fm_mso_get_lco_city(
	pcm_context_t		*ctxp,
	poid_t			*account_pdp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_search_plan_details(
        pcm_context_t                   *ctxp,
        pin_flist_t                     *i_flistp,
        pin_flist_t                     **o_flistpp,
        pin_errbuf_t                    *ebufp);
int32
fm_mso_utils_discount_validation(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_balances(
        pcm_context_t           *ctxp,
        poid_t                  *account_pdp,
        time_t                  bal_srch_t,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_search_offer_entries(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_purchased_install_products(
        pcm_context_t           *ctxp,
        poid_t			*act_pdp,
        poid_t			*srvc_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_create_refund_for_cancel_plan(
        pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_service_quota_codes(
	pcm_context_t                   *ctxp,
	char                            *plan_name,
	int32                           *bill_when,
	char                            *city,
	pin_flist_t                     **r_flistpp,
	pin_errbuf_t                    *ebufp);

PIN_EXPORT void
fm_mso_utils_get_profile(
	pcm_context_t	*ctxp,
	poid_t		*account_pdp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

void
fm_mso_utils_get_mso_baseplan_details(
	pcm_context_t           *ctxp,
	poid_t                  *act_pdp,
	poid_t                  *srvc_pdp,
	int                     status,
	int                     prod_status,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp);
PIN_EXPORT void
fm_mso_get_subscriber_profile(
	pcm_context_t	*ctxp,
	poid_t		*account_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_EXPORT void
fm_mso_get_bills(
        pcm_context_t   *ctxp,
	char		*bill_no,
        poid_t  	*account_pdp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

PIN_EXPORT void
fm_mso_cust_create_bill_adj_sequence(
        pcm_context_t   *ctxp,
        poid_t          *account_pdp,
        int32           flag,
	pin_flist_t	**r_flistpp,
        pin_errbuf_t    *ebufp);

void
fm_mso_search_upgrade_offer_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_subscr_purchased_products(
        pcm_context_t           *ctxp,
        poid_t                  *act_pdp,
        poid_t                  *srvc_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_event_type_base(
        pcm_context_t           *ctxp,
        poid_t                  *prod_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_event_from_plan(
        pcm_context_t           *ctxp,
        poid_t                  *plan_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_helpdesk_details(
        pcm_context_t           *ctxp,
        char                    *city,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_mso_purchase_details(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        poid_t                  *srvc_pdp,
        poid_t                  *offer_pdp,
	int32                   *pckg_id,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_event_details(
        pcm_context_t           *ctxp,
        poid_t			*act_pdp,
        poid_t			*srvc_pdp,
        char			*event_type,
	time_t			*created_t,
	int			sort,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_event_balimpacts(
        pcm_context_t           *ctxp,
        poid_t                  *act_pdp,
        poid_t                  *srvc_pdp,
        poid_t                  *offer_obj,
        time_t                  *created_t,
        char                    *event_type,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_create_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_purchase_deal(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_create_balgrp(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_create_billinfo(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_utils_get_act_bal_grp
(
    pcm_context_t       *ctxp,
    poid_t              *act_pdp,
    poid_t              **ret_bal_grp_poidp,
    pin_errbuf_t        *ebufp);

void
fm_mso_validate_tal_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

/***************************************************************
 * This function gets the billinfo details based on the
 * based on the Account POID.
 ***************************************************************/		
void fm_mso_get_acct_billinfo(
	pcm_context_t	*ctxp,
	poid_t		*acnt_pdp,
	pin_flist_t	**bill_infop,
	pin_errbuf_t	*ebufp);

/************************************************************************
 * This function is validates the given BusinessType and BillInfoID of 
 * and returns the AccountType. 
 ************************************************************************/	
void 
validate_hybrid_acnt_type(
	pcm_context_t	*ctxp,
	int32 		*business_typep,
	char		*billinfo_idp,
	char		**acnt_type,
	pin_errbuf_t	*ebufp);

/*************************************************************
* Function  : fm_mso_cust_get_pp_type()
* Decription: Function to fetch PP_TYPE of a CATV Customer
*************************************************************/
void
fm_mso_cust_get_pp_type(
	pcm_context_t	    	*ctxp,
        poid_t              *acct_pdp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp);

PIN_EXPORT void
fm_mso_cust_get_acc_from_sno(
	pcm_context_t	*ctxp,
	pin_flist_t     *i_flistp,
    pin_flist_t     **ret_flistpp,
    pin_errbuf_t    *ebufp);

/**************************************************
* Function: 	fm_mso_trans_open 
* Owner:    	Gautam Adak 		
* Decription:	For opening local transaction
* Input    
*          pdp  Poid to be locked
*          flag 0-READONLY
*               1-READWRITE
*               2-LOCK_OBJ
*
* Return   0   if transaction opened successfully
*          1   otherwise
***************************************************/
EXPORT_OP int32 
fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	int32		flag,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*tr_flistp = NULL;
	pin_flist_t	*lock_flistp = NULL;
	pin_flist_t	*tar_flistp = NULL;

	int32		t_status = 0;

	t_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	/* Open transaction */
	if (flag ==0 )
	{
		PCM_OP(ctxp, PCM_OP_TRANS_OPEN, PCM_TRANS_OPEN_READONLY, t_flistp, &tr_flistp, ebufp);
	}
	else if (flag ==1)
	{
		PCM_OP(ctxp, PCM_OP_TRANS_OPEN, PCM_TRANS_OPEN_READWRITE, t_flistp, &tr_flistp, ebufp);
	}
	else if (flag ==2)
	{
		PCM_OP(ctxp, PCM_OP_TRANS_OPEN, PCM_TRANS_OPEN_READWRITE, t_flistp, &tr_flistp, ebufp);
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling lock..");
	//	PCM_OPREF(ctxp, PCM_OP_LOCK_OBJ, PCM_TRANS_OPEN_LOCK_OBJ, t_flistp, &lock_flistp, ebufp);
	}
                else if (flag == 3)
        {

                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Invoked new flag inside utils..");
                PCM_OP(ctxp, PCM_OP_TRANS_OPEN,PCM_TRANS_OPEN_READWRITE | PCM_TRANS_OPEN_LOCK_DEFAULT, t_flistp, &tr_flistp, ebufp);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling new lock..");
        }
	else
	{
	PCM_OP(ctxp, PCM_OP_TRANS_OPEN, PCM_TRANS_OPEN_READWRITE, t_flistp, &tr_flistp, ebufp);
	}

	if(ebufp->pin_err == PIN_ERR_TRANS_ALREADY_OPEN)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans open:alrady open");
		PIN_ERRBUF_CLEAR(ebufp);
		t_status = 1;
	}
	else if(ebufp->pin_err != PIN_ERR_NONE)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_trans open:error", ebufp);
		t_status = 1;
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans open:success");
	}

	/* cleanup */
	if(t_flistp)
		PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	if(tr_flistp)
		PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);
	if(tar_flistp)
		PIN_FLIST_DESTROY_EX(&tar_flistp, NULL);
	if(lock_flistp)
		PIN_FLIST_DESTROY_EX(&lock_flistp, NULL);

	return t_status;
}

/**************************************************
* Function:     fm_mso_trans_commit
* Owner:        Gautam Adak
* Decription:   For committing local transaction
***************************************************/
EXPORT_OP void fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*tr_flistp = NULL;

	t_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	/* Commit transaction */
	PCM_OP(ctxp, PCM_OP_TRANS_COMMIT, 0, t_flistp, &tr_flistp, ebufp);

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_trans commit error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans commit");
	}

	/* cleanup */
	if(t_flistp)
		PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	if(tr_flistp)
		PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);

	return;
}

/**************************************************
* Function:     fm_mso_trans_abort
* Owner:        Gautam Adak
* Decription:   For aborting local transaction
* Return   	0   if transaction opened successfully
*          	1   otherwise
***************************************************/
EXPORT_OP void fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*tr_flistp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp) ) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_trans abort error b4", ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
	}

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "POID to abort", pdp);
        t_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	/* Open transaction */
	PCM_OP(ctxp, PCM_OP_TRANS_ABORT, 0, t_flistp, &tr_flistp, ebufp);

	if(ebufp->pin_err != PIN_ERR_NONE)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_trans abort error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_trans abort");
	}

	/* cleanup */
	if(t_flistp)
		PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	if(tr_flistp)
		PIN_FLIST_DESTROY_EX(&tr_flistp, NULL);

	return;
}

/**************************************************
* Function: 	fm_mso_get_deals_from_plan()
* Owner:        Gautam Adak
* Decription:   For getting deals from plans passed
*		in array during activation
* 
* 
***************************************************/
void
fm_mso_get_deals_from_plan(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_deal_flistp,
	pin_flist_t		**r_limit_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*plan_array = NULL;
	pin_flist_t		*read_plan_iflist = NULL;
	pin_flist_t		*read_plan_oflist = NULL;
	pin_flist_t		*deals_array = NULL;
	pin_flist_t		*deal_flist = NULL;
	pin_flist_t		*cr_limit_array = NULL;
	pin_flist_t		*cr_limit_flist = NULL;	
	pin_flist_t		*deals = NULL;
	pin_flist_t		*tmp_flist = NULL;

	poid_t			*plan_pdp = NULL;
	poid_t			*deal_pdp = NULL;

	int			elem_id = 0;
	pin_cookie_t		cookie = NULL;

	int			elem_id_1 = 0;
	pin_cookie_t		cookie_1 = NULL;

	int			deal_count = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_deals_from_plan", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_deals_from_plan input flist", in_flistp);
	
	deal_flist = PIN_FLIST_CREATE(ebufp);
	cr_limit_flist =PIN_FLIST_CREATE(ebufp); 

	while((plan_array = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		plan_pdp = PIN_FLIST_FLD_GET(plan_array, PIN_FLD_PLAN_OBJ, 0, ebufp);
		read_plan_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_POID, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_CODE, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_NAME, NULL, ebufp);
		PIN_FLIST_ELEM_SET(read_plan_iflist, NULL, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_ELEM_SET(read_plan_iflist, NULL, PIN_FLD_LIMIT, PIN_ELEMID_ANY, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist", read_plan_iflist);

		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_plan_iflist, &read_plan_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&read_plan_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist", read_plan_oflist);
		PIN_FLIST_FLD_COPY(read_plan_oflist, PIN_FLD_CODE, plan_array, PIN_FLD_CODE, ebufp );

		/******************************
		* Read Deals
		*******************************/
		elem_id_1=0;
		cookie_1=NULL;
		while((deals = PIN_FLIST_ELEM_GET_NEXT(read_plan_oflist, PIN_FLD_SERVICES,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			deal_pdp = PIN_FLIST_FLD_TAKE(deals, PIN_FLD_DEAL_OBJ, 0, ebufp);
			deals_array = PIN_FLIST_ELEM_ADD(deal_flist, PIN_FLD_DEALS, deal_count, ebufp );
			PIN_FLIST_FLD_PUT(deals_array, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
			PIN_FLIST_FLD_SET(deals_array, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
			PIN_FLIST_FLD_COPY(read_plan_oflist, PIN_FLD_CODE, deals_array, PIN_FLD_CODE, ebufp );
			PIN_FLIST_FLD_COPY(read_plan_oflist, PIN_FLD_NAME, deals_array, PIN_FLD_NAME, ebufp );
			deal_count++;
		}
		/******************************
		* Read Credit Limit
		*******************************/
		elem_id_1=0;
		cookie_1=NULL;
		while((cr_limit_array = PIN_FLIST_ELEM_GET_NEXT(read_plan_oflist, PIN_FLD_LIMIT,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			tmp_flist = PIN_FLIST_COPY(cr_limit_array, ebufp);
			PIN_FLIST_ELEM_PUT(cr_limit_flist, tmp_flist, PIN_FLD_LIMIT, elem_id_1, ebufp );
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cr_limit_flist", cr_limit_flist);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deal_flist", deal_flist);
		PIN_FLIST_DESTROY_EX(&read_plan_oflist, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cr_limit_flist", cr_limit_flist);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deal_flist", deal_flist);

	*r_deal_flistp = deal_flist;
	*r_limit_flistp = cr_limit_flist;
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&read_plan_oflist, NULL);
	return;
}


int32 getDeviceLevelPriorities(double prod_priority_double, int32 device_type, double *prio_start, double *prio_end,
				pin_flist_t	*plans_array, pin_errbuf_t            *ebufp) { 
	
	char debug_msg[250];
	int32	technology = 0;
	switch(device_type) {
		case NORMAL_TECH:
			*prio_end = *prio_start + BB_NORMAL_DEVICE_RANGE_END;
			*prio_start = *prio_start + BB_NORMAL_DEVICE_RANGE_START;
			sprintf(debug_msg, "NORMAL Device Type... Setting priority to [%f - %f]",*prio_start, *prio_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			technology = NORMAL_TECH;
			break;
		case DOCSIS2_TECH:
			*prio_end = *prio_start + BB_DOCSIS2_DEVICE_RANGE_END;
			*prio_start = *prio_start + BB_DOCSIS2_DEVICE_RANGE_START;
			sprintf(debug_msg, "DOCSIS 2.0 Device Type... Setting priority to [%f - %f]",*prio_start, *prio_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			technology = DOCSIS2_TECH;
			break;
		case DOCSIS3_TECH:
			*prio_end = *prio_start + BB_DOCSIS3_DEVICE_RANGE_END;
			*prio_start = *prio_start + BB_DOCSIS3_DEVICE_RANGE_START;
			sprintf(debug_msg, "DOCSIS 3.0 Device Type... Setting priority to [%f - %f]",*prio_start, *prio_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			technology = DOCSIS3_TECH;
			break;
		case GPON_TECH:
			*prio_end = *prio_start + BB_DOCSIS3_DEVICE_RANGE_END;
			*prio_start = *prio_start + BB_DOCSIS3_DEVICE_RANGE_START;
			sprintf(debug_msg, "GPON Device Type... Setting priority to [%f - %f]",*prio_start, *prio_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			technology = GPON_TECH;
			break;
		case FIBER_TECH:
			*prio_end = *prio_start + BB_FIBER_DEVICE_RANGE_END;
			*prio_start = *prio_start + BB_FIBER_DEVICE_RANGE_START;
			sprintf(debug_msg, "FIBER Device Type... Setting priority to [%f - %f]",*prio_start, *prio_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			technology = FIBER_TECH;
			break;
		case ALL_DEVICE_TYPE:
			sprintf(debug_msg, "ALL Device Type... Setting priority to [%f - %f]",*prio_start, *prio_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			technology = ALL_DEVICE_TYPE;
			break;
	}
	if(prod_priority_double >= *prio_start &&    
		prod_priority_double <= *prio_end ) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matched priority. Returning 1");
		PIN_FLIST_FLD_SET(plans_array, MSO_FLD_DEVICE_TYPE, &technology, ebufp);
		return 1;
    }
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Priority did not match. Returning 0");
    return 0;
}

int32 getHwSubTypePriorities (double prod_priority_double, int32 plan_sub_type
)
{
	switch(plan_sub_type) {
	    case BB_HW_MODEM:
		if(prod_priority_double == BB_HW_CABLE_MODEM_DEPO_PRIO ||
			prod_priority_double == BB_HW_DCM_DEPO_PRIO ||
			prod_priority_double == BB_HW_CABLE_MODEM_OR_PRIO ||
			prod_priority_double == BB_HW_DCM_OR_PRIO ||
			prod_priority_double == BB_HW_CABLE_MODEM_RENT_PRIO ||
			prod_priority_double == BB_HW_DCM_RENT_PRIO) 
			{
			    return 1;
			}
		break;
	    case BB_HW_CABLE_ROUTER:
		if(prod_priority_double == BB_HW_CABLE_ROUTER_DEPO_PRIO ||
                        prod_priority_double == BB_HW_CABLE_ROUTER_OR_PRIO ||
			prod_priority_double == BB_HW_CABLE_ROUTER_RENT_PRIO) 
                        {
                            return 1;
                        }
                break;
	    case BB_WIFI_ROUTER:
		if(prod_priority_double ==  BB_HW_WIFI_DEVICE_DEPO_PRIO ||
                        prod_priority_double ==  BB_HW_WIFI_DEVICE_OR_PRIO ||
                        prod_priority_double == BB_HW_WIFI_DEVICE_RENT_PRIO ||
			prod_priority_double == BB_HW_CISCO_SOHO_DEPO_PRIO ||
			prod_priority_double == BB_HW_CISCO_SOHO_OR_PRIO ||
			prod_priority_double == BB_HW_CISCO_SOHO_RENT_PRIO ||
			prod_priority_double == BB_HW_HW_ROUTER_DEPO_PRIO ||
			prod_priority_double == BB_HW_HW_ROUTER_OR_PRIO ||
			prod_priority_double == BB_HW_HW_ROUTER_RENT_PRIO)
                        {
                            return 1;
                        }
                break;
	    case NAT:
		if(prod_priority_double ==  BB_HW_NAT_DEVICE_DEPO_PRIO ||
                        prod_priority_double ==  BB_HW_WI_NAT_DEVICE_DEPO_PRIO ||
                        prod_priority_double == BB_HW_NAT_DEVICE_OR_PRIO ||
			prod_priority_double == BB_HW_WI_NAT_DEVICE_OR_PRIO ||
			prod_priority_double == BB_HW_NAT_DEVICE_RENT_PRIO ||
			prod_priority_double == BB_HW_WI_NAT_DEVICE_RENT_PRIO)
                        {
                            return 1;
                        }
                break;
	    case ADDITIONAL_IP:
		if(prod_priority_double == BB_HW_ADDITIONAL_IP_RENT_PRIO ||
			prod_priority_double == BB_HW_ADDITIONAL_IP_OR_PRIO)
			{
			    return 1;
			}
		break;
	}
	return 0;
}
int32 getSubTypePriorities(double prod_priority_double, int32 plan_sub_type, int32 *device_type, double *prio_start, double *prio_end,
			   pin_flist_t	*plans_array, pin_errbuf_t            *ebufp) {
	
	int32 in_range = 0;
	double p_start, p_end;
	p_start = *prio_start;
	p_end = *prio_end;
	char	debug_msg[250];

	if(plan_sub_type == BB_LIMITED || plan_sub_type == ALL_PLAN_SUB_TYPE) {		
		p_start = *prio_start + BB_LIMITED_RANGE_START;
		p_end = *prio_start + BB_LIMITED_RANGE_END;	
		sprintf(debug_msg, "Limited/All Plan Sub Type... Setting priority to [%f - %f]",p_start, p_end);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		in_range = getDeviceLevelPriorities(prod_priority_double, *device_type,&p_start, &p_end, plans_array, ebufp);
		if(in_range == 1 || plan_sub_type == BB_LIMITED) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan sub-type matched. Returning 1");
			return in_range;
		}
	}
	if(plan_sub_type == BB_UNLIMITED_FUP || plan_sub_type == ALL_PLAN_SUB_TYPE) {
		p_start = *prio_start + BB_UNLIMITED_FUP_RANGE_START;
		p_end = *prio_start + BB_UNLIMITED_FUP_RANGE_END;
		sprintf(debug_msg, "Unlimited_FUP/All Plan Sub Type... Setting priority to [%f - %f]",p_start, p_end);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		in_range = getDeviceLevelPriorities(prod_priority_double, *device_type,&p_start, &p_end, plans_array, ebufp);
		if(in_range == 1 || plan_sub_type == BB_UNLIMITED_FUP) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan sub-type matched. Returning 1");
			return in_range;
		}
	}
	if(plan_sub_type == BB_UNLIMITED_NO_FUP || plan_sub_type == ALL_PLAN_SUB_TYPE) {
		p_start = *prio_start + BB_UNLIMITED_NO_FUP_RANGE_START;
		p_end = *prio_start + BB_UNLIMITED_NO_FUP_RANGE_END;
		sprintf(debug_msg, "Unlimited_No_FUP/All Plan Sub Type... Setting priority to [%f - %f]",p_start, p_end);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		in_range = getDeviceLevelPriorities(prod_priority_double, *device_type,&p_start, &p_end, plans_array, ebufp);
		if(in_range == 1 || plan_sub_type == BB_UNLIMITED_NO_FUP) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan sub-type matched. Returning 1");
			return in_range;
		}
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Plan sub-type is not matching. Returning 0");
	return 0;
}

int32 getPriorities(double prod_priority_double, int32 *payment_type, int32 *device_type, int32 plan_sub_type, double *prio_start, double *prio_end,
		    pin_flist_t	*plans_array, pin_errbuf_t *ebufp) {

	char debug_msg[250];
	double p_start, p_end;
	int32	ret_val = 0;
	p_start = *prio_start;
	p_end = *prio_end;
	int32	pymt_type = 0;
	if(*payment_type == BB_PREPAID || *payment_type == ALL_PYMT_TYPE) {
			p_end = *prio_start + BB_PREPAID_END;
			p_start = *prio_start + BB_PREPAID_START;
			sprintf(debug_msg, "Prepaid/All Payment Type... Setting priority to [%f - %f]",p_start, p_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			ret_val = getSubTypePriorities(prod_priority_double, plan_sub_type,device_type, &p_start, &p_end, plans_array, ebufp);			
			if(ret_val == 1)
			{
			    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Product Prioirty matched. Returning 1");
			    pymt_type = BB_PREPAID;
			    PIN_FLIST_FLD_SET(plans_array, MSO_FLD_PAYMENT_TYPE, &pymt_type, ebufp);
			    return ret_val;
			}
	}
	if(*payment_type == BB_POSTPAID || *payment_type == ALL_PYMT_TYPE) {
			p_end = *prio_start + BB_POSTPAID_END;		
			p_start = *prio_start + BB_POSTPAID_START;
			sprintf(debug_msg, "Postpaid/All Payment Type... Setting priority to [%f - %f]",p_start, p_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			ret_val = getSubTypePriorities(prod_priority_double, plan_sub_type,device_type, &p_start, &p_end, plans_array, ebufp);
			if(ret_val == 1)
                        {
			    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Product Prioirty matched. Returning 1");
			    pymt_type = BB_POSTPAID;
			    PIN_FLIST_FLD_SET(plans_array, MSO_FLD_PAYMENT_TYPE, &pymt_type, ebufp);
                            return ret_val;
                        }
						
	}
	if(*payment_type == BB_FUP || *payment_type == ALL_PYMT_TYPE) {
			p_end = *prio_start + BB_FUP_END;
			p_start = *prio_start + BB_FUP_START;
			sprintf(debug_msg, "FUP/All Payment Type... Setting priority to [%f - %f]",p_start, p_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			if(plan_sub_type == ALL_PLAN_SUB_TYPE && *device_type == ALL_DEVICE_TYPE) {
			    if(prod_priority_double >= p_start && prod_priority_double <= p_end) {
			    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Product Prioirty matched. Returning 1");
			    pymt_type = BB_FUP;
			    PIN_FLIST_FLD_SET(plans_array, MSO_FLD_PAYMENT_TYPE, &pymt_type, ebufp);
				return 1;
			    }
			}
	}
	if(*payment_type == BB_ADD_MB || *payment_type == ALL_PYMT_TYPE) {
			p_end = *prio_start + BB_ADD_MB_END;
			p_start = *prio_start + BB_ADD_MB_START;
			sprintf(debug_msg, "Additional MB/All Payment Type... Setting priority to [%f - %f]",p_start, p_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			sprintf(debug_msg, "plan sub type is %d and device type is %d",plan_sub_type, *device_type);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			if(plan_sub_type == ALL_PLAN_SUB_TYPE && *device_type == ALL_DEVICE_TYPE) {
			    if(prod_priority_double >= p_start && prod_priority_double <= p_end) {
			    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Product Prioirty matched. Returning 1");
			    pymt_type = BB_ADD_MB;
			    PIN_FLIST_FLD_SET(plans_array, MSO_FLD_PAYMENT_TYPE, &pymt_type, ebufp);
				return 1;
			    }
			}
	}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Product Prioirty not matching. Returning 0");
	return 0;
}

int32 isProductInSubsRange(pin_flist_t *in_flistp,
							pin_flist_t	*plans_array,
							double prod_priority_double,
							char			*plan_name,
							int32			add_info_flag,
							pcm_context_t		*ctxp,
							pin_errbuf_t		*ebufp) {

	double	prio_start = 0.0;
	double	prio_end = 0.0;
	int32	*plan_category = NULL, *payment_type = NULL, *device_type = NULL, plan_sub_type;
	pin_decimal_t	*plan_sub_dec = NULL;
	char debug_msg[250];
	char	*city = NULL;
	int32	is_prod_in_range = 0;

	sprintf(debug_msg, "Before reading plan stuff");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);

	plan_category = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PLAN_CATEGORY, 1, ebufp);
	payment_type = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PAYMENT_TYPE, 1, ebufp);

	plan_sub_dec = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PLAN_SUB_TYPE, 1, ebufp);
	plan_sub_type = atoi(pbo_decimal_to_str(plan_sub_dec, ebufp));
	device_type = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_DEVICE_TYPE, 1, ebufp);
	city = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_CITY, 1, ebufp);

	sprintf(debug_msg, "Plan category is %d",*plan_category);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
	sprintf(debug_msg, "Payment Type is %d",*payment_type);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
	sprintf(debug_msg, "Plan sub type is %d",plan_sub_type);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
	sprintf(debug_msg, "Technology type is %d",*device_type);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);

		if(*plan_category == BB_SME || *plan_category == ALL_CATEGORY) {
			prio_start = prio_start + BB_SUB_SME_PRI_START;
			prio_end = prio_end + BB_SUB_SME_PRI_END;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " SME Plan Category");
			sprintf(debug_msg, "Setting priority to [%f - %f]",prio_start, prio_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			is_prod_in_range = getPriorities(prod_priority_double, payment_type, device_type, plan_sub_type, &prio_start, &prio_end, plans_array, ebufp);
			}
		if(!is_prod_in_range && (*plan_category == BB_RETAIL || *plan_category == ALL_CATEGORY)) {
			prio_start = prio_start + BB_SUB_RET_PRI_START;
			prio_end = prio_end + BB_SUB_RET_PRI_END;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " RETAIL Plan Category");
			sprintf(debug_msg, "Setting priority to [%f - %f]",prio_start, prio_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			is_prod_in_range = getPriorities(prod_priority_double, payment_type, device_type, plan_sub_type, &prio_start, &prio_end, plans_array, ebufp);
			}
		if(!is_prod_in_range && (*plan_category == BB_CYBERCAFE || *plan_category == ALL_CATEGORY)) {
			prio_start = prio_start + BB_SUB_CYB_PRI_START;
			prio_end = prio_end + BB_SUB_CYB_PRI_END;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " CyberCafe Plan Category");
			sprintf(debug_msg, "Setting priority to [%f - %f]",prio_start, prio_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			is_prod_in_range = getPriorities(prod_priority_double, payment_type, device_type, plan_sub_type, &prio_start, &prio_end, plans_array, ebufp);
			}
		if(!is_prod_in_range && (*plan_category == BB_CORPORATE || *plan_category == ALL_CATEGORY)) {
			prio_start = prio_start + BB_SUB_COR_PRI_START;
			prio_end = prio_end + BB_SUB_COR_PRI_END;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Corporate Plan Category");
			sprintf(debug_msg, "Setting priority to [%f - %f]",prio_start, prio_end);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			is_prod_in_range = getPriorities(prod_priority_double, payment_type, device_type, plan_sub_type, &prio_start, &prio_end, plans_array, ebufp);
			}

	if(is_prod_in_range && city != NULL && add_info_flag) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matching product found. Checking for city");
		return is_product_applies_to_city(in_flistp, city, plan_name, ctxp, ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Matching product found.");
	return is_prod_in_range;	
}




int32 is_product_applies_to_city(pin_flist_t	*in_flistp,
								char *city, 
								char	*plan_name,
								pcm_context_t	*ctxp, 
								pin_errbuf_t	*ebufp) 
{

	
	pin_flist_t	*srch_plan_details_in_flistp = NULL;
	pin_flist_t	*srch_plan_details_out_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	pin_flist_t	*rates_flistp = NULL;
	pin_flist_t	*cities_flistp = NULL;

	poid_t		*srch_pdp = NULL;
	poid_t		*prod_poidp = NULL;
	int32		srch_flag = 0;
	int32		elem_id = 0;
	char		*city_in_config = NULL;
	char		*all_cities = "*";
	char		*plan_city = NULL;
	pin_cookie_t	cookie = NULL;
	int64		db = 1;
	char		debug_msg[250];

	if(strcmp(city, all_cities) == 0) {
		return 1;
	}

	//char *template = "select X from /mso_cfg_credit_limit where F1 = V1 and ( F2 = V2 or F3 = V3) ";
	char *template = "select X from /mso_cfg_credit_limit where F1 = V1 ";
	sprintf(debug_msg, "Searching if plan is applicable for [%s]",city);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_plan_details_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_plan_details_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_plan_details_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_plan_details_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp,MSO_FLD_PLAN_NAME, plan_name, ebufp);
	
/*	args_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_CITY, city, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_CITY, all_cities, ebufp); */

	result_flistp = PIN_FLIST_ELEM_ADD(srch_plan_details_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_ELEM_SET(result_flistp,NULL,MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan search input flist", srch_plan_details_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_plan_details_in_flistp, &srch_plan_details_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan search output flist", srch_plan_details_out_flistp);
	result_flistp = PIN_FLIST_ELEM_GET(srch_plan_details_out_flistp,PIN_FLD_RESULTS, 0, 1, ebufp );
	if(result_flistp != NULL) {
		elem_id=0;
		cookie=NULL;
		while((cities_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, MSO_FLD_CITIES,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			plan_city = PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CITY, 0, ebufp);
	sprintf(debug_msg, "City in plan is [%s]",plan_city);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
			if((strcmp(plan_city, city) == 0) || (strcmp(plan_city, all_cities) == 0)) {
	sprintf(debug_msg, "Matching city or all cities found.. returning true");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
				return 1;
			}
		}
	}
		
	return 0;

}


int32 verify_misc_plan(double prod_priority_double, int32 plan_sub_type) {

	int32 flag_select_prod = 0;
	char debug_msg[250];

	sprintf(debug_msg, "Incoming priority is  %f",prod_priority_double);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);

	sprintf(debug_msg, "Plan sub type is  %d",plan_sub_type);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);

	switch(plan_sub_type) {									
		case BB_MISC_ADD_IP:
			if(prod_priority_double == BB_MISC_ADD_IP) {
				flag_select_prod = 1;
			}
			break;
		case BB_MISC_HOST_DOM:
			if(prod_priority_double == BB_MISC_HOST_DOM) {
				flag_select_prod = 1;
			}
			break;
		case BB_MISC_CABLE_SHIFT:
			if(prod_priority_double == BB_MISC_CABLE_SHIFT) {
				flag_select_prod = 1;
			}
			break;
		case BB_MISC_ADD_EMAIL:
			if(prod_priority_double == BB_MISC_ADD_EMAIL) {
				flag_select_prod = 1;
			}
			break;
		case BB_MISC_ADD_SPACE:
			if(prod_priority_double == BB_MISC_ADD_SPACE) {
				flag_select_prod = 1;
			}
			break;
		case BB_RELOCATION_CHARGE:
			if(prod_priority_double == BB_RELOCATION_CHARGE) {
				flag_select_prod = 1;
			}
			break;
		case BB_MISC_ALL:
			if (prod_priority_double >= BB_MISC_PRI_START &&
			prod_priority_double <= BB_MISC_PRI_END)
			{
				flag_select_prod = 1;
			}
			break;
	}
	sprintf(debug_msg, "Flag select prod is %d",flag_select_prod);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
	return flag_select_prod;
}


void populate_additional_info(pcm_context_t		*ctxp,
			pin_flist_t		*plans_array, 
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
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
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
	    plan_info_flistp = PIN_FLIST_COPY(results_flistp, ebufp);
	    PIN_FLIST_ELEM_SET(plans_array, plan_info_flistp, MSO_FLD_PLAN_INFO, 0, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "populate_additional_info plan array updated", plans_array);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_get_products_from_deal()
* Owner:        Gautam Adak
* Decription:   For getting products from deal
*		during customer registration
*  if fliter_products=1, then products will be
      filtered
   if fliter_products=0, all products will be
      returned
* 
* 
* Input Structure:
* 0 PIN_FLD_POID                   POID [0] 0.0.0.1 /search -1 0
* 0 MSO_FLD_PLAN_TYPE               INT [0] 0
* 0 MSO_FLD_PLAN_SUB_TYPE       DECIMAL [0] 100
* 0 PIN_FLD_RESULTS               ARRAY [0] allocated 20, used 7
* 1     PIN_FLD_POID               POID [0] 0.0.0.1 /plan 379112 0
* 1     PIN_FLD_SERVICES          ARRAY [0] allocated 20, used 1
* 2         PIN_FLD_DEAL_OBJ   POID [0] 0.0.0.1 /deal 45536 0
* 0 PIN_FLD_RESULTS           ARRAY [1] allocated 20, used 7
* 1     PIN_FLD_SERVICES      ARRAY [0] allocated 20, used 1
* 2         PIN_FLD_DEAL_OBJ    OID [0] 0.0.0.1 /deal 41697 0

***************************************************/
void
fm_mso_get_products_from_deal(
	pcm_context_t		*ctxp,
	int32			fliter_products,
	int32			add_info_flag,
	pin_flist_t		*in_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*plans_array = NULL;
	pin_flist_t		*deals_array = NULL;
	pin_flist_t		*read_flds_iflisp = NULL;
	pin_flist_t		*read_flds_oflisp = NULL;
	pin_flist_t		*prod_array = NULL;
	pin_flist_t		*read_prod_oflisp = NULL;
	pin_flist_t		*copy_in_flistp = NULL;
	pin_flist_t		*prod_output_flistp = NULL;
	pin_flist_t		*inp_plan_array = NULL;
	pin_flist_t		*inp_deals_array = NULL;
	pin_flist_t		*new_plans_array = NULL;

	poid_t			*deal_pdp = NULL;
	poid_t			*prod_pdp = NULL;

	int				elem_id = 0;
	pin_cookie_t	cookie = NULL;
	int				elem_id_1 = 0;
	pin_cookie_t	cookie_1 = NULL;
	int				elem_id_2 = 0;
	pin_cookie_t	cookie_2 = NULL;

	int32			plan_type = 0;
	int32			plan_sub_type = 0;
	int32			flag_select_prod = 0;
	int32			enum_catv = CATV;
	int32			enum_bb = BB;
	int32			drop_plan;
	int32			ret_plan_type;
	int32			is_bb_service = 0;
	int32			add_prod = 1;
	pin_decimal_t		*priority = NULL;
	pin_decimal_t		*prod_priority = NULL;	
	double			prod_priority_double = 0.0;
	char			*service_type = NULL;
	char			*plan_name = NULL;
	char			*city = NULL;
	char debug_msg[250];
	int32 		*technology = NULL;
	int32		*payment_type = NULL;
	int32           *device_type = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_products_from_deal", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_products_from_deal input flist", in_flistp);
 	
	plan_type = *(int32*)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PLAN_TYPE, 1, ebufp);
	priority = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PLAN_SUB_TYPE, 1, ebufp);  //MSO_FLD_PLAN_SUB_TYPE = product priority
	city = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_CITY, 1, ebufp);
	plan_sub_type = atoi(pbo_decimal_to_str(priority, ebufp));
	device_type = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_DEVICE_TYPE, 1, ebufp);

	PIN_FLIST_FLD_RENAME(in_flistp, PIN_FLD_RESULTS, PIN_FLD_PLAN, ebufp);
  	while((plans_array = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_PLAN,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		ret_plan_type = -1;
		elem_id_1     = 0;  
		cookie_1      = NULL;
		service_type  = NULL;

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0001");
		plan_name = PIN_FLIST_FLD_GET(plans_array, PIN_FLD_NAME, 0, ebufp);
		sprintf(debug_msg, "Checking the plan [%s]",plan_name);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		PIN_FLIST_FLD_RENAME(plans_array, PIN_FLD_SERVICES, PIN_FLD_DEALS, ebufp);
		while((deals_array = PIN_FLIST_ELEM_GET_NEXT(plans_array, PIN_FLD_DEALS,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			if (!service_type)
			{
				service_type  = (char*)PIN_POID_GET_TYPE((poid_t*)PIN_FLIST_FLD_GET(deals_array, PIN_FLD_SERVICE_OBJ, 0, ebufp));
			}
			sprintf(debug_msg, "Service type is %s", service_type);
			is_bb_service = (strcmp(service_type, "/service/telco/broadband") == 0);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			PIN_FLIST_FLD_DROP(deals_array, PIN_FLD_SERVICE_OBJ, ebufp );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0002");
			deal_pdp = PIN_FLIST_FLD_GET(deals_array, PIN_FLD_DEAL_OBJ, 0 , ebufp);
			read_flds_iflisp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_POID, (void *)deal_pdp, ebufp);
			PIN_FLIST_ELEM_SET(read_flds_iflisp, NULL, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist", read_flds_iflisp);
			
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflisp, &read_flds_oflisp, ebufp);
			PIN_FLIST_DESTROY_EX(&read_flds_iflisp, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist", read_flds_oflisp);
			elem_id_2 = 0;
			cookie_2 = NULL;
			while((prod_array = PIN_FLIST_ELEM_GET_NEXT(read_flds_oflisp, PIN_FLD_PRODUCTS,
				&elem_id_2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
			{
				flag_select_prod =0;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0003");
				prod_pdp = PIN_FLIST_FLD_GET(prod_array, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
				if (prod_pdp)
				{
					read_flds_iflisp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_POID, (void *)prod_pdp, ebufp);
					PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_DESCR, NULL, ebufp);
					PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
					PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_PRIORITY, NULL, ebufp);
					PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_NAME, NULL, ebufp);

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist", read_flds_iflisp);  

					PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflisp, &read_prod_oflisp, ebufp);
					PIN_FLIST_DESTROY_EX(&read_flds_iflisp, NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
						PIN_FLIST_DESTROY_EX(&read_prod_oflisp, NULL);
						goto CLEANUP;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist", read_prod_oflisp);
					if (fliter_products)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Products to be fileterd based on criteria");
						prod_priority = PIN_FLIST_FLD_GET(read_prod_oflisp, PIN_FLD_PRIORITY, 0, ebufp);
						
						prod_priority_double = pbo_decimal_to_double(prod_priority, ebufp);

						if (priority && (pbo_decimal_compare(priority, prod_priority, ebufp ) ==0) )
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matching priority product found");
							flag_select_prod = 1;
						}
						else if(priority && is_bb_service ) {
							if(plan_type == PLAN_TYPE_HDW || plan_type == PLAN_TYPE_ALL) {
								switch(plan_sub_type) {
									case 15:
										if ((prod_priority_double >= BB_HW_RENTAL_RANGE_START &&
											prod_priority_double <= BB_HW_RENTAL_RANGE_END) || 
											(prod_priority_double >= BB_HW_DEPOSIT_RANGE_START &&
											prod_priority_double <= BB_HW_DEPOSIT_RANGE_END)
											)
										{
											PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matching HW Rental and Deposit products..");
											flag_select_prod = getHwSubTypePriorities(prod_priority_double, *device_type);
										}
										break;
									case 25:
										if (prod_priority_double >= BB_HW_OUTRIGHT_RANGE_START &&
											prod_priority_double <= BB_HW_OUTRIGHT_RANGE_END
											)
										{
											PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matching HW outright products..");
											flag_select_prod = getHwSubTypePriorities(prod_priority_double, *device_type);
										}
										break;
									case 35:
										if (prod_priority_double >= BB_MISC_PRI_END &&
											prod_priority_double <= 770
											)
										{
											PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matching HW AMC products..");
											flag_select_prod = 1;
										}
										break;
									case 0:
										if (prod_priority_double >= BB_HW_PRI_START &&
											prod_priority_double <= BB_HW_PRI_END
											)
										{
											PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matching HW ALL products..");
											flag_select_prod = 1;
										}
										break;
								}

								if(flag_select_prod == 1 && add_info_flag) {									
									PIN_FLIST_FLD_SET(plans_array,MSO_FLD_PLAN_SUB_TYPE, priority, ebufp);
									flag_select_prod = is_product_applies_to_city(in_flistp, city, plan_name, ctxp, ebufp);
									sprintf(debug_msg, "product is applicable to the input city");
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
								}
							} 
							if (!flag_select_prod && (plan_type == PLAN_TYPE_SUBS || plan_type == PLAN_TYPE_ALL)) {					
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Finding the subscription products");
								flag_select_prod = isProductInSubsRange(in_flistp, plans_array, prod_priority_double, plan_name, add_info_flag, ctxp, ebufp);
							} 
							if (!flag_select_prod && (plan_type == PLAN_TYPE_MISC || plan_type == PLAN_TYPE_ALL)) {
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Verifying the Miscellaneous plan types");
								flag_select_prod = verify_misc_plan(prod_priority_double, plan_sub_type);
							} 
							if (!flag_select_prod && (plan_type == PLAN_TYPE_INST || plan_type == PLAN_TYPE_ALL)) {
								if (prod_priority_double >= BB_HW_INST_RANGE_START &&
									prod_priority_double <= BB_HW_INST_RANGE_END
								) {
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matching Installation products..");
									flag_select_prod = 1;
								}
							}
						}
						else if(!priority)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No eaxct priority mentioned by user, so proceeding for priority range");
							switch (plan_type)
							{
							  case PLAN_TYPE_ALL :
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "All hardware/subscription/otc products will be returned");
								flag_select_prod = 1;
								break;
							  case PLAN_TYPE_HDW :
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "All hardware products will be returned");
								if ((prod_priority_double >= HDW_PROD_PRI_START && 
								    prod_priority_double <= HDW_PROD_PRI_END ) ||
									(prod_priority_double >= BB_HW_PRI_START &&
									prod_priority_double <= BB_HW_PRI_END )
									)								{
									flag_select_prod = 1;
								}
								break;
							  case PLAN_TYPE_SUBS :
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "All subscription products will be returned");
								if (( prod_priority_double >=SUBS_PROD_PRI_START && 
									prod_priority_double <= SUBS_PROD_PRI_END
									) || 
									( prod_priority_double >= BB_SUBS_PRI_START && 
									 prod_priority_double <= BB_SUBS_PRI_END
									))
								{
									flag_select_prod = 1;
								}
								 break;
							  case PLAN_TYPE_OTC   :
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "All otc products will be returned");
								if (prod_priority_double >= OTC_PROD_PRI_START  && 
								    prod_priority_double <= OTC_PROD_PRI_END )
								{
									flag_select_prod = 1;
								}
								break;
								case PLAN_TYPE_INST:
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "All installation products will be returned");
								if(prod_priority_double >=BB_INST_PRI_START && 
								  prod_priority_double <= BB_INST_PRI_END) 
								{
									flag_select_prod = 1;		

								}
								break;
								case PLAN_TYPE_MISC:
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "All misc products will be returned");
								if (prod_priority_double >=BB_MISC_PRI_START && 
								  prod_priority_double <= BB_MISC_PRI_END)
								{
									flag_select_prod = 1;
								}
							}

							if (is_bb_service && flag_select_prod)
							{
								flag_select_prod = is_product_applies_to_city(in_flistp, city, plan_name, ctxp, ebufp);
								sprintf(debug_msg,"Plan applicable to city is %d",flag_select_prod);
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
							}
						}
						//compare product priority to populate PLAN_TYPE
						if (ret_plan_type == -1)
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ret_plan_type =-1");
							if ((!is_bb_service && (prod_priority_double >= HDW_PROD_PRI_START && 
							    prod_priority_double <= HDW_PROD_PRI_END 
							    )) ||
								(is_bb_service && (prod_priority_double >= BB_HW_PRI_START && 
							    prod_priority_double <= BB_HW_PRI_END)
								))
							{
								ret_plan_type = PLAN_TYPE_HDW;
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ret_plan_type = PLAN_TYPE_HDW");
							}
							else if ((!is_bb_service && ( prod_priority_double >=SUBS_PROD_PRI_START && 
								  prod_priority_double <= SUBS_PROD_PRI_END
								)) || 
								(is_bb_service && ( prod_priority_double >= BB_SUBS_PRI_START && 
								  prod_priority_double <= BB_SUBS_PRI_END
								)))
							{
								ret_plan_type = PLAN_TYPE_SUBS;
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ret_plan_type = PLAN_TYPE_SUBS");
							}
							else if ((!is_bb_service && prod_priority_double >=OTC_PROD_PRI_START && 
								  prod_priority_double <= OTC_PROD_PRI_END))
							{
								ret_plan_type = PLAN_TYPE_OTC;
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ret_plan_type = PLAN_TYPE_OTC");
							}
							else if(prod_priority_double >=BB_INST_PRI_START && 
								  prod_priority_double <= BB_INST_PRI_END) {
								ret_plan_type = PLAN_TYPE_INST;
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ret_plan_type = PLAN_TYPE_INST");

							} 
							else if (prod_priority_double >=BB_MISC_PRI_START && 
								  prod_priority_double <= BB_MISC_PRI_END){
								ret_plan_type = PLAN_TYPE_MISC;
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ret_plan_type = PLAN_TYPE_MISC");
							}
							else if (prod_priority_double >=BB_MISC_PRI_END &&
                                                                  prod_priority_double <= 770){
                                                                ret_plan_type = PLAN_TYPE_AMC;
                                                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ret_plan_type = PLAN_TYPE_AMC");
                                                        }
						}
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "All products to be returned");
						flag_select_prod = 1;
					}

					if (flag_select_prod)
					{
						sprintf(debug_msg, "Adding product to the deals array output");
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,debug_msg);
						PIN_FLIST_FLD_RENAME(read_prod_oflisp, PIN_FLD_PRIORITY, MSO_FLD_PLAN_SUB_TYPE, ebufp);
						PIN_FLIST_ELEM_PUT(deals_array, read_prod_oflisp, PIN_FLD_PRODUCTS, elem_id_2, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Deals array after adding product is ",deals_array);
					}	
					else
					{
						PIN_FLIST_DESTROY_EX(&read_prod_oflisp, NULL);
					}	
				}
			} //End of Products
		}//End of Deals
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "service_type");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, service_type);
		if (strcmp(service_type, "/service/catv") == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "000");
			PIN_FLIST_FLD_SET(plans_array, PIN_FLD_TYPE_OF_SERVICE, &enum_catv, ebufp );
		}
		else if (strcmp(service_type, "/service/telco/broadband") == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "111");
			PIN_FLIST_FLD_SET(plans_array, PIN_FLD_TYPE_OF_SERVICE, &enum_bb, ebufp );
		}
		PIN_FLIST_FLD_SET(plans_array, MSO_FLD_PLAN_TYPE, &ret_plan_type, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plans_array", plans_array);

	}//End of Plans
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_products_from_deal modified input flist", in_flistp);

	copy_in_flistp = PIN_FLIST_COPY(in_flistp, ebufp);

	elem_id =0;
	cookie = NULL;
	while((plans_array = PIN_FLIST_ELEM_GET_NEXT(copy_in_flistp, PIN_FLD_PLAN,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		plan_type = *(int32*)PIN_FLIST_FLD_GET(plans_array, MSO_FLD_PLAN_TYPE, 1, ebufp);
		elem_id_1 = 0;  
		cookie_1 = NULL;
		drop_plan=1;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_products_from_deal plans array flist", plans_array);
		while((deals_array = PIN_FLIST_ELEM_GET_NEXT(plans_array, PIN_FLD_DEALS,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " deals array is ",deals_array);
			if ( (PIN_FLIST_ELEM_COUNT(deals_array, PIN_FLD_PRODUCTS, ebufp)) != 0 )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Drop plan made 0");
				drop_plan=0;
				break;
			}
		}
		if (drop_plan || plan_type == -1 )  /* plan_type == -1 means Invalid Priority*/
		{
			PIN_FLIST_ELEM_DROP(in_flistp, PIN_FLD_PLAN, elem_id, ebufp)
		}
		else {
		    if(add_info_flag) {
		        new_plans_array = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, elem_id, 0, ebufp);
		        populate_additional_info(ctxp, new_plans_array, ebufp);
		    }
		}
	}
	PIN_FLIST_DESTROY_EX(&copy_in_flistp, NULL);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&read_flds_oflisp, NULL);
	//PIN_FLIST_DESTROY_EX(&read_prod_oflisp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_get_deal_info()
* Owner:        Gautam Adak
* Decription:   For getting products from deal
*		during customer activation
*  if fliter_products=1, then products will be
      filtered
   if fliter_products=0, all products will be
      returned
* 
* 
* Input Structure:
* 0 PIN_FLD_DEALS             ARRAY [0] allocated 20, used 1
* 1     PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 36338 0
* 0 PIN_FLD_DEALS             ARRAY [1] allocated 20, used 1
* 1     PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 35593 0
* 0 PIN_FLD_DEALS             ARRAY [2] allocated 20, used 1
* 1     PIN_FLD_DEAL_OBJ       POID [0] 0.0.0.1 /deal 34569 0


***************************************************/
void
fm_mso_get_deal_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**deals_deposit,
	pin_flist_t		**deals_activation,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*plans_array = NULL;
	pin_flist_t		*deals_array = NULL;
	pin_flist_t		*read_flds_iflisp = NULL;
	pin_flist_t		*read_flds_oflisp = NULL;
	pin_flist_t		*prod_array = NULL;
	pin_flist_t		*read_prod_oflisp = NULL;

	poid_t			*deal_pdp = NULL;
	poid_t			*prod_pdp = NULL;

	int			elem_id_1 = 0;
	int			elem_id = 0;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie_1 = NULL;

	int32			encoded_val;
	int32			unit_var;
	int32			offset_var;

	pin_validity_modes_t	mode_variable;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_deal_info", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_deal_info input flist", in_flistp);
	
	while((deals_array = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_DEALS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_array", deals_array);
		deal_pdp = PIN_FLIST_FLD_GET(deals_array, PIN_FLD_DEAL_OBJ, 0 , ebufp);

		read_flds_iflisp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_POID, (void *)deal_pdp, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_NAME, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_START_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_END_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_FLAGS, NULL, ebufp);
		PIN_FLIST_ELEM_SET(read_flds_iflisp, NULL, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist", read_flds_iflisp);
		
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflisp, &read_flds_oflisp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_flds_iflisp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist", read_flds_oflisp);
		
		elem_id_1 =0;
		cookie_1 =NULL;
		while((prod_array = PIN_FLIST_ELEM_GET_NEXT(read_flds_oflisp, PIN_FLD_PRODUCTS,
			&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
		{
			encoded_val=-1;
			unit_var=-1;
			offset_var=-1;
			encoded_val = *(int32*)PIN_FLIST_FLD_GET(prod_array,PIN_FLD_CYCLE_END_DETAILS, 0, ebufp);
			PIN_VALIDITY_DECODE_FIELD(encoded_val, mode_variable, unit_var, offset_var);
			PIN_FLIST_FLD_SET(prod_array, PIN_FLD_CYCLE_END_UNIT, &unit_var, ebufp );
			PIN_FLIST_FLD_SET(prod_array, PIN_FLD_CYCLE_END_OFFSET, &offset_var, ebufp );

			encoded_val=-1;
			unit_var=-1;
			offset_var=-1;
			encoded_val = *(int32*)PIN_FLIST_FLD_GET(prod_array,PIN_FLD_PURCHASE_END_DETAILS, 0, ebufp);
			PIN_VALIDITY_DECODE_FIELD(encoded_val, mode_variable, unit_var, offset_var);
			PIN_FLIST_FLD_SET(prod_array, PIN_FLD_PURCHASE_END_UNIT, &unit_var, ebufp );
			PIN_FLIST_FLD_SET(prod_array, PIN_FLD_PURCHASE_END_OFFSET, &offset_var, ebufp );

			encoded_val=-1;
			unit_var=-1;
			offset_var=-1;
			encoded_val = *(int32*)PIN_FLIST_FLD_GET(prod_array,PIN_FLD_USAGE_END_DETAILS, 0, ebufp);
			PIN_VALIDITY_DECODE_FIELD(encoded_val, mode_variable, unit_var, offset_var);
			PIN_FLIST_FLD_SET(prod_array, PIN_FLD_USAGE_END_UNIT, &unit_var, ebufp );
			PIN_FLIST_FLD_SET(prod_array, PIN_FLD_USAGE_END_OFFSET, &offset_var, ebufp );
		}

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "modified read_flds_oflisp", read_flds_oflisp);

		PIN_FLIST_FLD_DROP(deals_array, PIN_FLD_DEAL_OBJ, ebufp);
		PIN_FLIST_SUBSTR_PUT(deals_array, read_flds_oflisp, PIN_FLD_DEAL_INFO, ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modified deals_array", deals_array);
	}//End of Deals
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_deal_info modified input flist", in_flistp);
	fm_mso_filter_deposit_prod(ctxp, in_flistp, deals_deposit, deals_activation, ebufp); 

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_filter_deposit_prod", ebufp);
		goto CLEANUP;	
	}

	CLEANUP:
	//PIN_FLIST_DESTROY_EX(&read_flds_oflisp, NULL);
	//PIN_FLIST_DESTROY_EX(&read_prod_oflisp, NULL);
	return;
}


/**************************************************
* Function: 	fm_mso_get_device_info()
* Owner:        Gautam Adak
* Decription:   For getting device information
*                based on 
*		-Device ID (search_type = MSO_FLAG_SRCH_BY_ID)
*		-PIN (search_type = MSO_FLAG_SRCH_BY_PIN)
*               - More options can be added
*		
* 
* 
***************************************************/
void
fm_mso_get_device_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*srvc_flistp = NULL;

	int32			srch_flag = 512;
	int32			srch_type = -1;
	int32			den_sp = 0;

	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	
	char			*den_nwp = "NDS1";
	char			*dsn_nwp = "NDS2";
	char			*hw_nwp = "NDS";
	char			*device_id = NULL;
	char			template[200];
	int64			db =-1;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_device_info", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_info input flist", in_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	srch_type = *(int32*)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SEARCH_TYPE, 0, ebufp);
	device_id = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DEVICE_ID , 1, ebufp);

	db = PIN_POID_GET_DB(acnt_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

	if ( srch_type == MSO_FLAG_SRCH_BY_ID )
	{
		den_sp = fm_mso_is_den_sp(ctxp, in_flistp, ebufp);
		if (device_id && strncmp(device_id, "000", 3) == 0)
		{
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
			if (den_sp == 1)
			{
				PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, den_nwp, ebufp);
			}
			else if(den_sp == 2)
			{
				PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, dsn_nwp, ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_MANUFACTURER, hw_nwp, ebufp);
			}
			strcpy(template, "select X from /device where F1 = V1 and F2 = V2 ");
		}
		else
		{
			strcpy(template, "select X from /device where F1 = V1 or F2 = V2 ");
		}
	}
	
	if ( srch_type == MSO_FLAG_SRCH_BY_PIN )
	{
		strcpy(template, "select X from /device where F1 = V1 or F2 = V2 ");
	}

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	if ( srch_type == MSO_FLAG_SRCH_BY_SERVICE)
	{
		strcpy(template, "select X from /device where F1 = V1 ");
		srvc_flistp = PIN_FLIST_ELEM_ADD(arg_flist, PIN_FLD_SERVICES, 0, ebufp );
		PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_SERVICE_OBJ, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp), ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_DEVICE_ID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DEVICE_ID, 0, ebufp), ebufp);
		if (device_id && strncmp(device_id, "000", 3) != 0)
		{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_SERIAL_NO, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DEVICE_ID, 0, ebufp), ebufp);
        }   
	}
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_info SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_device_info SEARCH output list", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		*ret_flistp = PIN_FLIST_COPY( result_flist, ebufp);
	}


	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_get_profile_info()
* Owner:        Gautam Adak
* Decription:   For getting .profile information
*                based on 
*		-PROFILE_DESCR=wholesale or customer_care 
*		-ACCOUNT (search_type = MSO_FLAG_SRCH_BY_ACCOUNT)
*		-JV (search_type = MSO_FLAG_SRCH_BY_JV)
*		-DT (search_type = MSO_FLAG_SRCH_BY_DT)
*		-SDT (search_type = MSO_FLAG_SRCH_BY_SDT)
*		-LCO (search_type = MSO_FLAG_SRCH_BY_LCO)
*               -ACCOUNT (search_type = MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER)
*               -More options can be added
*		
* 
* 
***************************************************/
void
fm_mso_get_profile_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*profile_substr = NULL;

	int32			srch_flag = 512;
	int32			srch_type = -1;

	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*obj = NULL;

	char			template[200];
	char			*profile_type = NULL;
	int64			db = -1;


	if (PIN_ERRBUF_IS_ERR(ebufp))
        return;
    PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_info input flist", in_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	profile_type = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROFILE_DESCR, 0, ebufp);
	srch_type = *(int32*)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SEARCH_TYPE, 0, ebufp);
	obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_OBJECT, 0, ebufp);

	db = PIN_POID_GET_DB(acnt_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_info SEARCH input list", srch_flistp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );

	if ((strcmp(profile_type,WHOLESALE) ==0) && srch_type == MSO_FLAG_SRCH_BY_SELF_ACCOUNT)
	{
		//Do not add PIN_FLD_CUSTOMER_CARE_INFO or MSO_FLD_WHOLESALE_INFO 
	}
	else if ((strcmp(profile_type,WHOLESALE) ==0) && srch_type == MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER)
	{
		profile_substr = PIN_FLIST_SUBSTR_ADD(arg_flist, PIN_FLD_CUSTOMER_CARE_INFO, ebufp );
	}
	else if ((strcmp(profile_type,WHOLESALE) ==0) && srch_type != MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER)
	{
		profile_substr = PIN_FLIST_SUBSTR_ADD(arg_flist, MSO_FLD_WHOLESALE_INFO, ebufp );
	}
	else if ((strcmp(profile_type,WHOLESALE) ==0) && srch_type != MSO_FLAG_SRCH_BY_ACCOUNT )
	{
		profile_substr = PIN_FLIST_SUBSTR_ADD(arg_flist, PIN_FLD_CUSTOMER_CARE_INFO, ebufp );
	}

	switch (srch_type)
	{
	    case MSO_FLAG_SRCH_BY_SELF_ACCOUNT:
		strcpy(template, "select X from /profile where F1.id = V1 and F2.type = V2 ");
		//PIN_FLIST_SUBSTR_DROP(arg_flist, MSO_FLD_WHOLESALE_INFO, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, obj, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, profile_type, -1, ebufp), ebufp);
		break;

	    case MSO_FLAG_SRCH_BY_ACCOUNT :
		strcpy(template, "select X from /profile where F1.id = V1 and F2.type = V2 ");
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, obj, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, profile_type, -1, ebufp), ebufp);
		break;

	    case MSO_FLAG_SRCH_BY_JV : 
		strcpy(template, "select X from /profile where F1.id = V1 and F2.type = V2 ");
		PIN_FLIST_FLD_SET(profile_substr, MSO_FLD_JV_OBJ, obj, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, profile_type, -1, ebufp), ebufp);
		break;

	    case MSO_FLAG_SRCH_BY_DT : 
		strcpy(template, "select X from /profile where F1.id = V1 and F2.type = V2 ");
		PIN_FLIST_FLD_SET(profile_substr, MSO_FLD_DT_OBJ, obj, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, profile_type, -1, ebufp), ebufp);
		break;

	    case MSO_FLAG_SRCH_BY_SDT : 
		strcpy(template, "select X from /profile where F1.id = V1 and F2.type = V2 ");
		PIN_FLIST_FLD_SET(profile_substr, MSO_FLD_SDT_OBJ, obj, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, profile_type, -1, ebufp), ebufp);
		break;

	    case MSO_FLAG_SRCH_BY_LCO :
		strcpy(template, "select X from /profile where F1.id = V1 and F2.type = V2 ");
		PIN_FLIST_FLD_SET(profile_substr, MSO_FLD_LCO_OBJ, obj, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, profile_type, -1, ebufp), ebufp);
		break;

	    case MSO_FLAG_SRCH_LCO_OF_SUBSCRIBER :
		strcpy(template, "select X from /profile/wholesale 1,  /profile/customer_care 2 where 2.F2.id = V2 and 2.F1.id = 1.F3.id and 1.F4.type = V4 and 2.F5.type = V5 ");
		PIN_FLIST_FLD_SET(profile_substr, MSO_FLD_LCO_OBJ, NULL, ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, obj, ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	
                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
                PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/wholesale", -1, ebufp), ebufp);

                arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
                PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/customer_care", -1, ebufp), ebufp);
		break;

	    default :
		break;
	}

	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_info SEARCH input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL)
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_profile_info SEARCH output list", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		*ret_flistp = PIN_FLIST_COPY( result_flist, ebufp);
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_read_bal_grp()
* Owner:        Gautam Adak
* Decription:   For getting /billinfo from /balance_group
*		
* 
* 
***************************************************/
void
fm_mso_read_bal_grp(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	poid_t			*srvc_pdp,
	pin_flist_t		**bal_grp_oflist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*arg_flist = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*bal_grp_pdp = NULL;

	char			*template_1 = "select X from /balance_group 1, /account 2 where 2.F1.id = V1 and 1.F2 = 2.F3 ";
	char			*template_2 = "select X from /balance_group 1, /service 2 where 2.F1.id = V1 and 2.F2.type = V2 and 1.F3 = 2.F4 ";
	char			*template_3 = "select X from /balance_group  where F1.id = V1  ";
	char			*srvc_type = NULL;
	char			*poid_type = NULL;

	int64			db = -1;

	int32			srch_flags = 768;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_read_bal_grp", ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_read_bal_grp input flist");

	poid_type = (char*)PIN_POID_GET_TYPE(srvc_pdp);
	

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	if (srvc_pdp && strcmp(PIN_POID_GET_TYPE(srvc_pdp), "/balance_group") == 0 )
	{
		db = PIN_POID_GET_DB(srvc_pdp);
		srvc_type = (char*)PIN_POID_GET_TYPE(srvc_pdp);
		PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template_3, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, srvc_pdp, ebufp);
	}
	else if (srvc_pdp)
	{
		db = PIN_POID_GET_DB(srvc_pdp);
		srvc_type = (char*)PIN_POID_GET_TYPE(srvc_pdp);
		PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template_2, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, srvc_pdp, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
		//PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/service/catv", -1, ebufp), ebufp);
		PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, srvc_type, -1, ebufp), ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	}
	else
	{
		db = PIN_POID_GET_DB(acnt_pdp);
		PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template_1, ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, acnt_pdp, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	}
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp ),ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
//	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );
//	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILLINFO_OBJ, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_read_bal_grp read input list", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_read_bal_grp READ output list", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0,1, ebufp);
	if (result_flist)
	{
		*bal_grp_oflist = PIN_FLIST_COPY(result_flist, ebufp);	
	}

	return;
}

/**************************************************
* Function: 	fm_mso_get_billinfo()
* Owner:        Gautam Adak
* Decription:   For getting /billinfo from /balance_group
*		
* 
* 
***************************************************/
void
fm_mso_get_billinfo(
	pcm_context_t		*ctxp,
	poid_t			*bal_grp_obj,
	poid_t			**billinfo,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_flds_input = NULL;
	pin_flist_t		*read_flds_output = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_billinfo", ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_billinfo input flist");

	
	read_flds_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_POID, bal_grp_obj, ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_billinfo read input list", read_flds_input);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_input, &read_flds_output, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_input, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_billinfo READ output list", read_flds_output);

	if (read_flds_output)
	{
		*billinfo = PIN_FLIST_FLD_TAKE(read_flds_output, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	}

	PIN_FLIST_DESTROY_EX(&read_flds_output, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_read_billinfo()
* Owner:        Gautam Adak
* Decription:   For reading /billinfo 
*		
* 
* 
***************************************************/
void
fm_mso_read_billinfo(
	pcm_context_t		*ctxp,
	poid_t			*billinfo_obj,
	pin_flist_t		**billinfo_out_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_flds_input = NULL;
	pin_flist_t		*read_flds_output = NULL;
	poid_t			*bal_grp_pdp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_read_billinfo", ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_read_billinfo input flist");

	
	read_flds_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_POID, billinfo_obj, ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_ACTG_NEXT_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_ACTG_LAST_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_ACTG_FUTURE_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_PAY_TYPE, NULL, ebufp);
    	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_BILL_WHEN, NULL, ebufp);
    	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_BILLINFO_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_AR_BILLINFO_OBJ, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_read_billinfo read input list", read_flds_input);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_input, &read_flds_output, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_input, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_read_billinfo READ output list", read_flds_output);

	if (read_flds_output)
	{
		*billinfo_out_flist = read_flds_output;
		
	}

	return;
}

/**************************************************
* Function: 	fm_mso_utils_sequence_no()
* Owner:        Vilva
* Decription:   
*		
* 
* 
***************************************************/
void 
fm_mso_utils_sequence_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*new_ctxp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*inc_flistp = NULL;
	char			*name = NULL;
	poid_t			*i_pdp = NULL;
	poid_t			*pdp = NULL;
	poid_t			*s_pdp = NULL;
	u_int64			id = (u_int64)-1;
	u_int64			db = 0;
	int32			s_flags = 256;
	int32			*hdr_nump = NULL;
	int32			hdr_num = 0;
	int32			hdr_inc = 1;
	char			*hdr_str = NULL;
	char			*t1 = NULL;
	char			*t2 = NULL;
	char			str[100];
	
	srch_flistp = PIN_FLIST_CREATE(ebufp);

	i_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(i_pdp);
	name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"sequence creation error flist", i_flistp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_POID, i_pdp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53999", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Sequence Creation Error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS,&hdr_inc, ebufp);
		return;
	}

	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, "select X from /data/sequence where F1 = V1 ", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, (char *)name, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sequence Object Find Input Flist", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &rs_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sequence Object Find Output Flist", rs_flistp);
	
	r_flistp = PIN_FLIST_ELEM_GET(rs_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	s_pdp = PIN_FLIST_FLD_TAKE(r_flistp, PIN_FLD_POID, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "sequence creation error flist", rs_flistp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_POID, i_pdp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53999", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Sequence Creation Error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &hdr_inc, ebufp);
		return;
	}
	else if (r_flistp)
	{
		
		hdr_str = PIN_FLIST_FLD_TAKE(r_flistp, PIN_FLD_HEADER_STR, 1, ebufp);
		if (hdr_str)
		{
			t1 = strtok(hdr_str,":");
			t2 = strtok(NULL, ":");
		}
		PIN_FLIST_DESTROY_EX(&rs_flistp, NULL);
		r_flistp = NULL;
		inc_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(inc_flistp, PIN_FLD_POID, s_pdp, ebufp);
		PIN_FLIST_FLD_SET(inc_flistp, PIN_FLD_HEADER_NUM, &hdr_inc, ebufp);
		//Updating the incremented value for the sequencing object
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling INC_FLDS");
		PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
		// Removing locking since it is resulting in deadlock and blocking sessions.There is no need to lock heare since already the sequence is fetched above without locking.
		PCM_OP(new_ctxp, PCM_OP_INC_FLDS, 0, inc_flistp, &r_flistp, ebufp);
//		PCM_OP(new_ctxp, PCM_OP_INC_FLDS, PCM_OPFLG_LOCK_OBJ, inc_flistp, &r_flistp, ebufp);
		PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
		hdr_nump = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_HEADER_NUM, 0, ebufp);
		
		if (hdr_nump)
		{
			hdr_num = *hdr_nump;
		}
		
		//str = pin_malloc(100);
	        //memset(str,'\0',sizeof(str));
		memset(str,'\0',strlen(str));
		if (t1 && t2)
		{
			sprintf(str, "%s%s%d", t2, t1, hdr_num);
		}
//		*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_CONCAT(*r_flistpp, r_flistp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STRING, (void *)str, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SEPARATOR, (void *)t1, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SEQUENCE_ID, (void *)t2, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"SEQUENCE RETURN flist", *r_flistpp);
	}

	PIN_FLIST_DESTROY_EX(&inc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	

	if (hdr_str)
	{
		free(hdr_str);
	}

	PIN_ERRBUF_CLEAR(ebufp);
	return;

}

/**************************************************
* Function: 	fm_mso_filter_deposit_prod()
* Owner:        Gautam Adak
* Decription:   
* 
* 
* Input Structure:
0 PIN_FLD_DEALS         ARRAY [0] allocated 20, used 2
1     PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 33778 0
1     PIN_FLD_DEAL_INFO    SUBSTRUCT [0] allocated 20, used 5
2         PIN_FLD_POID           POID [0] 0.0.0.1 /deal 35593 6
....
2         PIN_FLD_PRODUCTS      ARRAY [0] allocated 20, used 19
...
0 PIN_FLD_DEALS         ARRAY [0] allocated 20, used 2
1     PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 33778 0
1     PIN_FLD_DEAL_INFO    SUBSTRUCT [0] allocated 20, used 5
2         PIN_FLD_POID           POID [0] 0.0.0.1 /deal 35593 6
....
2         PIN_FLD_PRODUCTS      ARRAY [0] allocated 20, used 19
....

***************************************************/

void
fm_mso_filter_deposit_prod(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**deals_deposit,
	pin_flist_t		**deals_activation,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*deals_array = NULL;
	pin_flist_t		*dealinfo = NULL;
	pin_flist_t		*prod_array = NULL;
	pin_flist_t		*read_flds_iflisp = NULL;
	pin_flist_t		*read_prod_oflisp = NULL;
	pin_flist_t		*plan_array = NULL;
	pin_flist_t		*deals_array_activation = NULL;
	pin_flist_t		*dealinfo_activation = NULL;

	poid_t			*prod_pdp = NULL;

	int			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	int			elem_id_1 = 0;
	pin_cookie_t		cookie_1 = NULL;
	int			elem_id_2 = 0;
	pin_cookie_t		cookie_2 = NULL;

	int32			count = 0;
	int32			activation_deal_count =0;
	int32			deposit_plan_count =0;
	int32			last_prod_was_deposit;

	pin_decimal_t		*prod_priority = NULL;	

	double			prod_priority_double = 0.0;
	char			msg[512];


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_filter_deposit_prod", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_filter_deposit_prod input flist", in_flistp);

	//*deals_deposit = PIN_FLIST_CREATE(ebufp);
	*deals_activation = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*deals_activation, PIN_FLD_DESCR, "Dummy Field", ebufp);

	while((deals_array = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_DEALS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		deals_array_activation = PIN_FLIST_ELEM_ADD(*deals_activation, PIN_FLD_DEALS, activation_deal_count, ebufp);
		PIN_FLIST_FLD_COPY(deals_array, PIN_FLD_PLAN_OBJ, deals_array_activation, PIN_FLD_PLAN_OBJ, ebufp );
		PIN_FLIST_FLD_COPY(deals_array, PIN_FLD_CODE, deals_array_activation, PIN_FLD_CODE, ebufp );
		PIN_FLIST_FLD_COPY(deals_array, PIN_FLD_NAME, deals_array_activation, PIN_FLD_NAME, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_activation_1", *deals_activation);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "loop:PIN_FLD_DEALS", deals_array);
		
		dealinfo = PIN_FLIST_SUBSTR_GET(deals_array, PIN_FLD_DEAL_INFO, 0, ebufp);

		dealinfo_activation = PIN_FLIST_SUBSTR_ADD(deals_array_activation, PIN_FLD_DEAL_INFO, ebufp );
		PIN_FLIST_FLD_COPY(dealinfo, PIN_FLD_POID, dealinfo_activation, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_COPY(dealinfo, PIN_FLD_NAME, dealinfo_activation, PIN_FLD_NAME, ebufp );
		PIN_FLIST_FLD_COPY(dealinfo, PIN_FLD_START_T, dealinfo_activation, PIN_FLD_START_T, ebufp );
		PIN_FLIST_FLD_COPY(dealinfo, PIN_FLD_END_T, dealinfo_activation, PIN_FLD_END_T, ebufp );
		PIN_FLIST_FLD_COPY(dealinfo, PIN_FLD_FLAGS, dealinfo_activation, PIN_FLD_FLAGS, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_activation_2", *deals_activation);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "loop:PIN_FLD_DEAL_INFO", dealinfo);
		elem_id_2 = 0;
		cookie_2 = NULL;
		count =0;
		//last_prod_was_deposit=0;
		while((prod_array = PIN_FLIST_ELEM_GET_NEXT(dealinfo, PIN_FLD_PRODUCTS,
			&elem_id_2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
		{
			sprintf(msg,"array index value = %d", count);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "loop:PIN_FLD_PRODUCTS", prod_array);
			prod_pdp = PIN_FLIST_FLD_GET(prod_array, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
			if (prod_pdp)
			{
				PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "PIN_FLD_PRODUCT_OBJ", prod_pdp);
				read_flds_iflisp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_POID, (void *)prod_pdp, ebufp);
				PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_PRIORITY, NULL, ebufp);
				PIN_FLIST_FLD_SET(read_flds_iflisp, PIN_FLD_DESCR, NULL, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS input flist", read_flds_iflisp);  

				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflisp, &read_prod_oflisp, ebufp);
				PIN_FLIST_DESTROY_EX(&read_flds_iflisp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS", ebufp);
					PIN_FLIST_DESTROY_EX(&read_prod_oflisp, NULL);
					goto CLEANUP;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist", read_prod_oflisp);
				PIN_FLIST_FLD_COPY(read_prod_oflisp,  PIN_FLD_DESCR, prod_array, PIN_FLD_DESCR, ebufp );

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Products to be fileterd based on criteria");
				prod_priority = PIN_FLIST_FLD_GET(read_prod_oflisp, PIN_FLD_PRIORITY, 0, ebufp);
				prod_priority_double = pbo_decimal_to_double(prod_priority, ebufp);

				if (prod_priority_double == DEPOSIT_PRODUCT )
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "prod_priority_double == DEPOSIT_PRODUCT" );
					if (!(*deals_deposit))	//create flist once only, and keep adding on the same flist in successive iterations
					{
						*deals_deposit = PIN_FLIST_CREATE(ebufp);
					}
					plan_array = PIN_FLIST_ELEM_ADD(*deals_deposit, PIN_FLD_PLAN, deposit_plan_count, ebufp );
					PIN_FLIST_FLD_COPY(deals_array, PIN_FLD_PLAN_OBJ,    plan_array, PIN_FLD_PLAN_OBJ, ebufp );
					PIN_FLIST_FLD_COPY(deals_array, PIN_FLD_NAME,        plan_array, PIN_FLD_NAME, ebufp );
					PIN_FLIST_FLD_COPY(deals_array, PIN_FLD_CODE,        plan_array, PIN_FLD_CODE, ebufp );
					PIN_FLIST_FLD_COPY(dealinfo,    PIN_FLD_POID,        plan_array, PIN_FLD_DEAL_OBJ, ebufp );
					PIN_FLIST_FLD_COPY(prod_array,  PIN_FLD_PRODUCT_OBJ, plan_array, PIN_FLD_PRODUCT_OBJ, ebufp );

//					if (!last_prod_was_deposit)
//					{	//Do not drop PIN_FLD_DEALS when consecutive deposit products are found
//						//activation_deal_count--;
//						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_activation_3", *deals_activation);
//						//PIN_FLIST_ELEM_DROP(*deals_activation, PIN_FLD_DEALS, elem_id, ebufp );
//						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_activation_4", *deals_activation);
//						last_prod_was_deposit=1;
//					}
					deposit_plan_count++;
				}
				else
				{
//					if (last_prod_was_deposit)
//					{
//						// As the last product processed in the loop was of deposit type and PIN_FLD_DEALS from the flist
//						// *deals_activation was dropped so it is recreated here
//						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_activation_5", *deals_activation);
//						last_prod_was_deposit=0;
//						deals_array_activation = PIN_FLIST_ELEM_ADD(*deals_activation, PIN_FLD_DEALS, activation_deal_count, ebufp);
//						PIN_FLIST_FLD_COPY(deals_array, PIN_FLD_PLAN_OBJ, deals_array_activation, PIN_FLD_PLAN_OBJ, ebufp );
//						PIN_FLIST_FLD_COPY(deals_array, PIN_FLD_CODE, deals_array_activation, PIN_FLD_CODE, ebufp );
//						PIN_FLIST_FLD_COPY(deals_array, PIN_FLD_NAME, deals_array_activation, PIN_FLD_NAME, ebufp );
//
//						dealinfo_activation = PIN_FLIST_SUBSTR_ADD(deals_array_activation, PIN_FLD_DEAL_INFO, ebufp );
//						PIN_FLIST_FLD_COPY(dealinfo, PIN_FLD_POID, dealinfo_activation, PIN_FLD_POID, ebufp );
//						PIN_FLIST_FLD_COPY(dealinfo, PIN_FLD_NAME, dealinfo_activation, PIN_FLD_NAME, ebufp );
//						PIN_FLIST_FLD_COPY(dealinfo, PIN_FLD_START_T, dealinfo_activation, PIN_FLD_START_T, ebufp );
//						PIN_FLIST_FLD_COPY(dealinfo, PIN_FLD_END_T, dealinfo_activation, PIN_FLD_END_T, ebufp );
//						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_activation_6", *deals_activation);
//					}
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "prod_priority_double != DEPOSIT_PRODUCT" );
					//PIN_FLIST_ELEM_COPY(in_flistp, PIN_FLD_DEALS, elem_id_1, *deals_activation, PIN_FLD_DEALS, count, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_activation_7", *deals_activation); 
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_array_activation", deals_array_activation);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "dealinfo_activation", dealinfo_activation);
					PIN_FLIST_ELEM_SET(dealinfo_activation, prod_array, PIN_FLD_PRODUCTS, count, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "TEST_0000");
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_activation_8", *deals_activation);
					count++;
				}
			}
			PIN_FLIST_DESTROY_EX(&read_prod_oflisp, NULL);
		} //end PRODUCTS loop

		deals_array_activation = PIN_FLIST_ELEM_GET(*deals_activation, PIN_FLD_DEALS, elem_id, 0, ebufp);
		dealinfo_activation = PIN_FLIST_SUBSTR_GET(deals_array_activation, PIN_FLD_DEAL_INFO, 0, ebufp );
		if ( PIN_FLIST_ELEM_COUNT(dealinfo_activation, PIN_FLD_PRODUCTS, ebufp)==0)
		{
			PIN_FLIST_ELEM_DROP(*deals_activation, PIN_FLD_DEALS, elem_id, ebufp );
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "deals_activation_9", *deals_activation);
		activation_deal_count++;
	}

	PIN_FLIST_FLD_DROP(*deals_activation, PIN_FLD_DESCR, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_filter_deposit_prod *deals_activation", *deals_activation);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_filter_deposit_prod *deals_deposit", *deals_deposit);

	CLEANUP:
	//PIN_FLIST_DESTROY_EX(&read_prod_oflisp, NULL);
	return;
}

/**************************************************
* Function: fm_mso_encrypt_passwd()
* 
* 
***************************************************/
char*
fm_mso_encrypt_passwd(
	pcm_context_t		*ctxp,
	poid_t			*service_pdp,
	char			*non_encrypted_pwd,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*encrypt_pwd_input = NULL;
	pin_flist_t		*encrypt_pwd_output = NULL;
	char			*encrypted_pwd = NULL;

	encrypt_pwd_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(encrypt_pwd_input, PIN_FLD_POID, service_pdp, ebufp);
	PIN_FLIST_FLD_SET(encrypt_pwd_input, PIN_FLD_PASSWD_CLEAR, non_encrypted_pwd, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_encypt_passwd input list", encrypt_pwd_input);
	PCM_OP(ctxp, PCM_OP_CUST_POL_ENCRYPT_PASSWD, 0, encrypt_pwd_input, &encrypt_pwd_output, ebufp);
	PIN_FLIST_DESTROY_EX(&encrypt_pwd_input, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_encrypt_passwd", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_encrypt_passwd output flist", encrypt_pwd_output);
	encrypted_pwd = (char *)PIN_FLIST_FLD_TAKE(encrypt_pwd_output, PIN_FLD_PASSWD, 0, ebufp );

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&encrypt_pwd_output, NULL);
	return encrypted_pwd;
}

/**************************************************
* Function: 	fm_mso_get_all_billinfo()
* Owner:        Gautam Adak
* Decription:   For getting /billinfo for an account
*		
* 
* 
***************************************************/
void
fm_mso_get_all_billinfo(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	int32			billinfo_type,
	pin_flist_t		**billinfo,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_input = NULL;
	pin_flist_t		*srch_output = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;

	poid_t			*srch_pdp = NULL;

	int32			srch_flags = 768;

	char			*template_1 =  " select X from /billinfo where F1.id = V1 ";
	char			*template_2 =  " select X from /billinfo where F1.id = V1 and F2 = V2 ";

	int64			db = -1;



	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_all_billinfo", ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_all_billinfo input flist");

	db = PIN_POID_GET_DB(acnt_pdp);
	srch_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_input, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp ), ebufp);
	PIN_FLIST_FLD_SET(srch_input, PIN_FLD_FLAGS, &srch_flags, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_input, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	if (billinfo_type == CATV)
	{
		PIN_FLIST_FLD_SET(srch_input, PIN_FLD_TEMPLATE, template_2, ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_input, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_ID, "CATV", ebufp);
	}
	else if (billinfo_type == BB)
	{
		PIN_FLIST_FLD_SET(srch_input, PIN_FLD_TEMPLATE, template_2, ebufp);
		
		arg_flist = PIN_FLIST_ELEM_ADD(srch_input, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_ID, "BB", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(srch_input, PIN_FLD_TEMPLATE, template_1, ebufp);
	}

	result_flist = PIN_FLIST_ELEM_ADD(srch_input, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILLINFO_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PAY_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACTG_NEXT_T, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_all_billinfo search input ", srch_input);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_input, &srch_output, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_input, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_all_billinfo search output", srch_output);

	if (srch_output)
	{
		*billinfo = PIN_FLIST_COPY(srch_output, ebufp);
	}

	PIN_FLIST_DESTROY_EX(&srch_output, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_get_err_desc()
* Owner:        Gautam Adak
* Decription:   For populating error description
*		
* 
* 
***************************************************/
void
fm_mso_get_err_desc(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{

	int32			errorCode	= 0;
	int32			status_fail	= 1;

	pin_fld_num_t		field_no	= 0;

	char			*errorDesc	= NULL;
	char			*errorField	= NULL;
	char			errorMsg[80]	= "";	


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
		strcat(errorMsg, ": Try with another STB/VC") ;
	}

	PIN_ERRBUF_CLEAR(ebufp);
	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	if (!acnt_pdp)
	{
		acnt_pdp = PIN_POID_CREATE(1, "/account", -1, ebufp );
	}
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_fail, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51000", ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, errorMsg, ebufp);
 
	return;
}

/**************************************************
* Function: 	fm_mso_get_business_type_values()
* Owner:        Gautam Adak
* Decription:   For populating business_type
*		
* 
* 
***************************************************/
void
fm_mso_get_business_type_values(
	pcm_context_t		*ctxp,
	int32			business_type,
	int32			*account_type,
	int32			*account_model,
	int32			*subscriber_type,
	pin_errbuf_t		*ebufp)
{
		
	PIN_ERR_LOG_MSG(3,"bus type 1");
	int32		tmp_business_type = -1;
 	
	/*******************************************************************
	* Derive account_type,account_model & subscriber_type from business_type 
	*  - Start
	*******************************************************************/
	tmp_business_type = business_type;
	*account_type = tmp_business_type/1000000;
	PIN_ERR_LOG_MSG(3,"bus type 2");

	//reserved_bits = tmp_business_type%1000;
	tmp_business_type = tmp_business_type/1000;

	*account_model = tmp_business_type%10;
	PIN_ERR_LOG_MSG(3,"bus type 3");

	tmp_business_type = tmp_business_type/10;
	*subscriber_type =  tmp_business_type%100;
	PIN_ERR_LOG_MSG(3,"bus type 4");
	/************************************************************************
	* Derive account_type,account_model & subscriber_type from business_type 
	* END
	************************************************************************/
	return;
}

/**************************************************
* Function: 	fm_mso_get_account_info()
* Owner:        Gautam Adak
* Decription:   For getting device information
*                based on 
*		-Device ID (search_type = MSO_FLAG_SRCH_BY_ID)
*		-PIN (search_type = MSO_FLAG_SRCH_BY_PIN)
*               - More options can be added
*		
* 
* 
***************************************************/
void
fm_mso_get_account_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_flds_iflist = NULL;
	pin_flist_t		*read_flds_oflist = NULL;
	pin_flist_t		*nameinfo = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_account_info", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_account_info acnt_pdp", acnt_pdp);

	read_flds_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_BUSINESS_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_CREATED_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_MOD_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, MSO_FLD_RMN, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, MSO_FLD_AREA, NULL, ebufp);
 	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_LASTSTAT_CMNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, MSO_FLD_LEGACY_ACCOUNT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_ACCESS_CODE1, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_ACCESS_CODE2, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_AAC_PROMO_CODE, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_AAC_PACKAGE, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_AAC_SOURCE, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, MSO_FLD_ET_ZONE, NULL, ebufp);
	PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	PIN_FLIST_ELEM_SET(read_flds_iflist, NULL, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);

	nameinfo= PIN_FLIST_ELEM_ADD(read_flds_iflist, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_ELEM_SET(read_flds_iflist, NULL, PIN_FLD_EXEMPTIONS, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_account_info read input list", read_flds_iflist);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_iflist, &read_flds_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		PIN_FLIST_DESTROY_EX(&read_flds_oflist, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_account_info READ output list", read_flds_oflist);

	//*ret_flistp = PIN_FLIST_COPY(read_flds_oflist, ebufp);
	
	*ret_flistp = read_flds_oflist;

	//PIN_FLIST_DESTROY_EX(&read_flds_oflist, NULL);

	CLEANUP:
	return;
}

/**************************************************
* Function: 	fm_mso_get_state_city_area_code()
* Owner:        Gautam Adak
* Decription:   For getting device information
*                based on 
*		-Device ID (search_type = MSO_FLAG_SRCH_BY_ID)
*		-PIN (search_type = MSO_FLAG_SRCH_BY_PIN)
*               - More options can be added
*		
* 
* 
***************************************************/
void
fm_mso_get_state_city_area_code(
	pcm_context_t		*ctxp,
	char			*state_city_area_code,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*acnt_info = NULL;

	char			*state_code = NULL;
	char			*city_code = NULL;
	char			*area_code = NULL;
	char			tmp_state_city_area_code[200];


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_state_city_area_code", ebufp);
		return;
	}

	if ( acnt_pdp && (!state_city_area_code || strcmp(state_city_area_code, "")==0))
	{
		fm_mso_get_account_info(ctxp, acnt_pdp, &acnt_info, ebufp);
		state_city_area_code = (char*)PIN_FLIST_FLD_GET(acnt_info, MSO_FLD_AREA, 0, ebufp);
	}
	else if (!(state_city_area_code && strcmp(state_city_area_code, "")!=0))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Fetching state_city_area_code with invalid input", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Fetching state_city_area_code with invalid input", ebufp);
		goto CLEANUP;
	}

	memset(tmp_state_city_area_code, '\0',sizeof(tmp_state_city_area_code));
	strcpy(tmp_state_city_area_code, state_city_area_code);
	
	state_code = strtok(tmp_state_city_area_code, "|");
	city_code = strtok(NULL, "|");
	area_code = strtok(NULL, "|");

	if (!state_code || !city_code || !area_code )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Invalid MSO_FLD_AREA", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid MSO_FLD_AREA", ebufp);
		goto CLEANUP;	
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "state_code");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  state_code);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "city_code");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  city_code);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "area_code");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  area_code);

	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATE, state_code, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, MSO_FLD_CITY_CODE, city_code, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, MSO_FLD_AREA_CODE, area_code, ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	return;
}

void replace_char (
	char	*s, 
	char	find, 
	char	replace)
{
	int i;

	for (i=0; i<strlen(s); i++)
	{
		if(s[i]	== find)
		{
			s[i]=replace;
		}
	}
}

void insert_char(
	char	*srcp,
	char	in_char,
	char	*dstp)
{
	int32	i = 0;
	int32	j = 0;

	//dstp = (char *)pin_malloc((sizeof (char) * strlen(srcp)+6));

	for (i = 1; i <= strlen(srcp); i++)
	{
		dstp[j] = srcp[i-1];
		if (i % 2 == 0)
		{
			j++;
			dstp[j] = in_char;
		}
		j++;
	}
	dstp[strlen(dstp)-1] = '\0';
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, dstp);	
}	

void num_to_array(int32 num, int32 arr[])
{
	int	i;
	int	size;
	char	num_str[20]="";

	sprintf(num_str, "%d", num);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, num_str);	
	size = strlen(num_str);
	for (i=0; i<size; i++)
	{
		arr[size-i-1]=num%10;
		num = num/10;
	}
}


/**************************************************
* Function: fm_mso_validation_rule_1()
*     1.  Should not allow more than one plan of same family
*     2. Any BB ,ADD-ON or ALC is part of ACTIVE DPO plan then individual plan should not be allowed for 
*	 activation(Based on provisioning TAG).
*     3. Any channel is part of DPO and trying to activate individual ALCARTE then we should not allow activation.	      
* 
***************************************************/

void
fm_mso_validation_rule_1(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			action,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_iflist = NULL;
	pin_flist_t		*srch_oflist = NULL;
	pin_flist_t		*srch_oflist_1 = NULL;
	pin_flist_t		*args_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*result_flist_1 = NULL;
	pin_flist_t		*plan_list = NULL;
	pin_flist_t		*read_plan_iflist = NULL;
	pin_flist_t		*read_plan_oflist = NULL;
	pin_flist_t		*return_flistp = NULL;
	pin_flist_t		*tmp_plan_prod_flistp = NULL;
	pin_flist_t		*plan_prod_flistp = NULL;

	poid_t			*service_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*new_plan_pdp = NULL;

	int32			srch_flags =512;
	int32			srch_distinct_flags =256;
	int32			activate_customer_failure = 1;
	int32			link_direction = 1;
	int32			new_dpo_flags = 0;
	int32			flag_dup_channel = 0;	
	int32			dpo_flags = 0;
	int32			alc_flags = 0;
	int32			elem_id =0;
	int32			elem_id_1 =0;
	int32			elem_id_2 = 0;
	int32			prod_count = 0;
	int32			family_id = -1;
	int32			pp_status_cancel   = PIN_PRODUCT_STATUS_CANCELLED;
	int32			flg_same_family = 0;
	int32			*flags = NULL;
	int32			count_basic_plan = 0;
	int32			basic_flag = 1;
	int32			prod_rec_id = 0;

	int64			db =1;
	char			*new_prov_tag = NULL;
	char			*prov_tag = NULL;
	char			*new_channel_namep = NULL;
	char			*channel_namep = NULL;
	char			*vp;
	char			*plan_code = NULL;
	char 			srch_template[600];
	char			tmp_str[50];
	char			*plan_name = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie_1 = NULL;
	pin_cookie_t		cookie_2 = NULL;
	
	pin_decimal_t		*priority = NULL;
	double			priority_double = 0.0;
	char    num_str[20]="";
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validation_rule_1", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validation_rule_1", i_flistp);
	
	service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
	flags = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FAMILY_ID, 1, ebufp );
	if (flags)
	{
		family_id = *flags;
	}
	flags = NULL;

	if ( action == MSO_OP_CUST_ADD_PLAN )
	{
		while ((plan_list = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			new_plan_pdp = PIN_FLIST_FLD_GET(plan_list, PIN_FLD_PLAN_OBJ, 0, ebufp);
			read_plan_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_POID, new_plan_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_CODE, NULL, ebufp );
			PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_NAME, NULL, ebufp); 
			PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_DESCR, NULL, ebufp); 
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_plan_iflist:", read_plan_iflist);

			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_plan_iflist, &read_plan_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&read_plan_iflist, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plan doesn't exists in the OBRM system", ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_plan_oflist:", read_plan_oflist);

			plan_name = PIN_FLIST_FLD_GET(read_plan_oflist, PIN_FLD_DESCR, 0, ebufp);
			
			if (!strstr(plan_name, "#HATHWAY") && (family_id == 0 || family_id == 1))
			{
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00001", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is not valid for Hathway", ebufp);
			    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "This plan is not valid for Hathway", read_plan_oflist);
				goto CLEANUP;
			}
            else if (!strstr(plan_name, "#DEN") && (family_id == 2 || family_id == 3))
			{
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00002", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is not valid for DEN", ebufp);
			    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "This plan is not valid for DEN", read_plan_oflist);
				goto CLEANUP;
			}
            else if (!strstr(plan_name, "#DSN") && (family_id == 4 || family_id == 5))
			{
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00002", ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is not valid for DSN", ebufp);
			    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "This plan is not valid for DSN", read_plan_oflist);
				goto CLEANUP;
			}
            else if (!strstr(plan_name, "#IPTV") && (family_id == 9))
            {
                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00002", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is not valid for IPTV", ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "This plan is not valid for IPTV", read_plan_oflist);
                goto CLEANUP;
            }


			if (strstr(plan_name, "DPO-"))
			{
				new_dpo_flags = 1;
			}
			else if (strstr(plan_name, "ALC-"))
			{
				alc_flags = 1;
			}
			PIN_FLIST_FLD_COPY(read_plan_oflist, PIN_FLD_CODE, plan_list, PIN_FLD_CODE, ebufp );
			PIN_FLIST_DESTROY_EX(&read_plan_oflist, NULL);

			fm_mso_get_event_from_plan(ctxp, new_plan_pdp, &tmp_plan_prod_flistp, ebufp);

			if (plan_prod_flistp == NULL)
			{
				plan_prod_flistp  = PIN_FLIST_CREATE(ebufp);
			}
			elem_id_1 = 0;
			cookie_1 = NULL;	
			while ((result_flist = PIN_FLIST_ELEM_TAKE_NEXT(tmp_plan_prod_flistp, PIN_FLD_RESULTS,
				&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_FLIST_ELEM_PUT(plan_prod_flistp, result_flist, PIN_FLD_RESULTS, prod_rec_id, ebufp);
				prod_rec_id++;
			}
			PIN_FLIST_DESTROY_EX(&tmp_plan_prod_flistp, NULL);
		}
	}
	if ( action == MSO_OP_CUST_CHANGE_PLAN )
	{
		while ((plan_list = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			flags = (int32*)PIN_FLIST_FLD_GET(plan_list, PIN_FLD_FLAGS, 0, ebufp );
			if (flags && *flags ==1)
			{
				new_plan_pdp = PIN_FLIST_FLD_GET(plan_list, PIN_FLD_PLAN_OBJ, 0, ebufp);
				read_plan_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_POID, new_plan_pdp, ebufp);
				PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_CODE, NULL, ebufp );
				PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_NAME, NULL, ebufp);
				PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_DESCR, NULL, ebufp); 
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_srvc_oflist:", read_plan_iflist);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_plan_iflist, &read_plan_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&read_plan_iflist, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Plan doesn't exists in the OBRM system", ebufp);
					goto CLEANUP;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_plan_oflist:", read_plan_oflist);

				PIN_FLIST_FLD_COPY(read_plan_oflist, PIN_FLD_CODE, plan_list, PIN_FLD_CODE, ebufp );
				plan_name = PIN_FLIST_FLD_GET(read_plan_oflist, PIN_FLD_DESCR, 0, ebufp);
			
				if (!strstr(plan_name, "#HATHWAY") && (family_id == 0 || family_id == 1))
				{
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00001", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is not valid for Hathway", ebufp);
					goto CLEANUP;
				}
                else if (!strstr(plan_name, "#DEN") && (family_id == 2 || family_id == 3))
			    {
				    *ret_flistp = PIN_FLIST_CREATE(ebufp);
				    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
				    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00002", ebufp);
				    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is not valid for DEN", ebufp);
			        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "This plan is not valid for DEN", read_plan_oflist);
				    goto CLEANUP;
			    }
                else if (!strstr(plan_name, "#DSN") && (family_id == 4 || family_id == 5))
			    {
				    *ret_flistp = PIN_FLIST_CREATE(ebufp);
				    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
				    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
				    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00002", ebufp);
				    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is not valid for DSN", ebufp);
			        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "This plan is not valid for DSN", read_plan_oflist);
				    goto CLEANUP;
			    }
                else if (!strstr(plan_name, "#IPTV") && (family_id == 9))
                {
                    *ret_flistp = PIN_FLIST_CREATE(ebufp);
                    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
                    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
                    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00002", ebufp);
                    PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "This plan is not valid for IPTV", ebufp);
                    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "This plan is not valid for IPTV", read_plan_oflist);
                    goto CLEANUP;
                }


				if (strstr(plan_name, "DPO-"))
				{
					new_dpo_flags = 1;
				}
				else if (strstr(plan_name, "ALC-"))
				{
					alc_flags = 1;
				}
				PIN_FLIST_DESTROY_EX(&read_plan_oflist, NULL);

				fm_mso_get_event_from_plan(ctxp, new_plan_pdp, &tmp_plan_prod_flistp, ebufp);
		
				if (plan_prod_flistp == NULL)
				{	
					plan_prod_flistp  = PIN_FLIST_CREATE(ebufp);
				}
				elem_id_1 = 0;
				cookie_1 = NULL;	
				while ((result_flist = PIN_FLIST_ELEM_TAKE_NEXT(tmp_plan_prod_flistp, PIN_FLD_RESULTS,
					&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
				{
					PIN_FLIST_ELEM_PUT(plan_prod_flistp, result_flist, PIN_FLD_RESULTS, prod_rec_id, ebufp);
					prod_rec_id++;
				}
				PIN_FLIST_DESTROY_EX(&tmp_plan_prod_flistp, NULL);
			}
			else if (flags && *flags ==0)
			{
				vp = PIN_FLIST_FLD_GET(plan_list, PIN_FLD_PLAN_OBJ, 0, ebufp);
				if (vp)
				{
					read_plan_iflist = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_POID, (poid_t*)vp, ebufp );
					PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_CODE, NULL, ebufp );
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_srvc_oflist:", read_plan_iflist);

					PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_plan_iflist, &read_plan_oflist, ebufp);
					PIN_FLIST_DESTROY_EX(&read_plan_iflist, NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						*ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
						goto CLEANUP;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_plan_oflist:", read_plan_oflist);

					plan_code = PIN_FLIST_FLD_GET(read_plan_oflist, PIN_FLD_CODE, 0, ebufp );
					PIN_FLIST_FLD_SET(plan_list, PIN_FLD_CODE, plan_code,  ebufp );
					PIN_FLIST_DESTROY_EX(&read_plan_oflist, NULL);
				}
			}
		}
	}

	if ( action == MSO_OP_CUST_ADD_PLAN )
	{
	/****************************************************************
	* Search product from /purchased_product to get existing products
	****************************************************************/
	srch_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /product 1, /purchased_product 2  where 2.F1 = V1 and 2.F2 = V2 and 2.F3 != V3 and 1.F4 = 2.F5 ", ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, args_flist, PIN_FLD_ACCOUNT_OBJ, ebufp );

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, args_flist, PIN_FLD_SERVICE_OBJ, ebufp );

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_STATUS, &pp_status_cancel, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);
                                                                                                           
	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PRIORITY, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_NAME, NULL, ebufp);
	result_flist = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_LINKED_OBJ, 2, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LINK_DIRECTION, &link_direction, ebufp);
	result_flist = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_EXTRA_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PLAN_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch product poids iflist:", srch_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch product poids oflist:", srch_oflist);

	prod_count = PIN_FLIST_ELEM_COUNT(srch_oflist, PIN_FLD_RESULTS, ebufp);
	if (prod_count == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "There are no existing products.Skipping" );
		goto CLEANUP;
	}

	elem_id_1 = 0;
	cookie_1 = NULL;
 	while ((result_flist_1 = PIN_FLIST_ELEM_GET_NEXT(plan_prod_flistp, PIN_FLD_RESULTS,
		&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
	{
		priority = PIN_FLIST_FLD_GET(result_flist_1, PIN_FLD_PRIORITY, 0, ebufp);
		priority_double = pbo_decimal_to_double(priority, ebufp);
		
		if (priority_double >= 100 && priority_double <= 130 )
		{
			count_basic_plan = count_basic_plan + 1;
		}
		new_prov_tag = PIN_FLIST_FLD_GET(result_flist_1, PIN_FLD_PROVISIONING_TAG, 0, ebufp);

		elem_id = 0;
		cookie = NULL;
 		while ((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflist, PIN_FLD_RESULTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			prov_tag = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
			/*************************************************************
			 * Check whether its a DPO plan or not.
			 ************************************************************/
			if (strcmp(prov_tag, "PROV_NCF_PRO") == 0)
			{
				plan_list = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_EXTRA_RESULTS, elem_id+1, 0, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "This is the base plan flist:", plan_list);
				plan_pdp = PIN_FLIST_FLD_GET(plan_list, PIN_FLD_PLAN_OBJ, 0, ebufp);
			
				read_plan_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_POID, plan_pdp, ebufp);
				PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_DESCR, NULL, ebufp); 
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_plan_iflist:", read_plan_iflist);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_plan_iflist, &read_plan_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&read_plan_iflist, NULL);
				plan_name = PIN_FLIST_FLD_GET(read_plan_oflist, PIN_FLD_DESCR, 0, ebufp);
				if (strstr(plan_name, "DPO-"))
				{
					dpo_flags = 1;
				}
				PIN_FLIST_DESTROY_EX(&read_plan_oflist, NULL);
			}
			priority = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PRIORITY, 0, ebufp);
			priority_double = pbo_decimal_to_double(priority, ebufp);


			if (basic_flag && priority_double >= 100 && priority_double <= 130)
			{
				count_basic_plan = count_basic_plan + 1;						
				basic_flag = 0;
			}

			if (prov_tag && new_prov_tag && strcmp(new_prov_tag, prov_tag) == 0)
			{
				flg_same_family = 1;
			}
		}
	}
	}	
	if (count_basic_plan > 1)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING, "More than one basic package offer not allowed", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "51107", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "More than one basic package offer not allowed", ebufp);
		goto CLEANUP;
	}

	if (flg_same_family > 0 )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING, "Only one plan from same family is allowed", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Only one plan from same family is allowed", ebufp);
		goto CLEANUP;
	}

	if ((alc_flags || new_dpo_flags) && new_plan_pdp)
	{
		read_plan_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_POID, new_plan_pdp, ebufp);
		if (new_dpo_flags)
		{
			PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_FLAGS, &new_dpo_flags, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_FLAGS, &alc_flags, ebufp);
		}

		PCM_OP(ctxp, MSO_OP_CUST_GET_PKG_CHANNELS, 0, read_plan_iflist, &read_plan_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&read_plan_iflist, NULL);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validation_rule_1 pkg_channels",
                        read_plan_oflist);
         
		prod_count = PIN_FLIST_ELEM_COUNT(plan_prod_flistp, PIN_FLD_RESULTS, ebufp);
		elem_id = 0;
		cookie = NULL;
 		while (new_dpo_flags && (result_flist = PIN_FLIST_ELEM_GET_NEXT(read_plan_oflist, PIN_FLD_PRODUCTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			result_flist_1 = PIN_FLIST_ELEM_ADD(plan_prod_flistp, PIN_FLD_RESULTS, prod_count, ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PRODUCT_OBJ, result_flist_1, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PRODUCT_NAME, result_flist_1, PIN_FLD_PRODUCT_NAME, ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PROVISIONING_TAG, result_flist_1, PIN_FLD_PROVISIONING_TAG, ebufp);
			prod_count = prod_count + 1;
		}
	}

	/********************************************
	 * Validate ALC channel
	 ********************************************/
	if (alc_flags && read_plan_oflist)
	{
		plan_list = PIN_FLIST_ELEM_GET(read_plan_oflist, PIN_FLD_PRODUCTS, 0, 0, ebufp);
		plan_list = PIN_FLIST_ELEM_GET(plan_list, MSO_FLD_CATV_CHANNELS, 0, 0, ebufp);
		new_channel_namep = PIN_FLIST_FLD_GET(plan_list, MSO_FLD_CHANNEL_NAME, 0, ebufp);	

		elem_id = 0;
		cookie = NULL;
 		while ((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflist, PIN_FLD_EXTRA_RESULTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validation_rule_1 existing plan",
                        	result_flist);
			read_plan_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PLAN_OBJ, read_plan_iflist, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_FLAGS, &alc_flags, ebufp);

			PCM_OP(ctxp, MSO_OP_CUST_GET_PKG_CHANNELS, 0, read_plan_iflist, &srch_oflist_1, ebufp);
			PIN_FLIST_DESTROY_EX(&read_plan_iflist, NULL);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validation_rule_1 pkg_channels",
                        	srch_oflist_1);

			elem_id_1 = 0;
			cookie_1 = NULL;
 			while ((result_flist_1 = PIN_FLIST_ELEM_GET_NEXT(srch_oflist_1, PIN_FLD_PRODUCTS,
				&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
			{
				elem_id_2 = 0;
				cookie_2 = NULL;
				while ((return_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flist_1, MSO_FLD_CATV_CHANNELS,
                                	&elem_id_2, 1, &cookie_2, ebufp)) != (pin_flist_t *)NULL)
				{
					channel_namep = PIN_FLIST_FLD_GET(return_flistp, MSO_FLD_CHANNEL_NAME, 0, ebufp); 
					if (strcmp(new_channel_namep, channel_namep) == 0)
					{
						flag_dup_channel = 1;
						break;
					}
				}
				if (flag_dup_channel)
				{
					break;
				}
			}

			if (flag_dup_channel)
			{
				break;
			}
			PIN_FLIST_DESTROY_EX(&srch_oflist_1, NULL);
		}
	}

	if (flag_dup_channel)
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING, "ALC channel already in subscribed packs", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00002", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "ALC channel already in subscribed packs", ebufp);
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modifiled plan products:", plan_prod_flistp);
	if (dpo_flags && plan_pdp)
	{
		read_plan_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_POID, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_FLAGS, &dpo_flags, ebufp);

		PCM_OP(ctxp, MSO_OP_CUST_GET_PKG_CHANNELS, 0, read_plan_iflist, &srch_oflist_1, ebufp);
		PIN_FLIST_DESTROY_EX(&read_plan_iflist, NULL);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validation_rule_1 pkg_channels",
                        read_plan_oflist);

        	prod_count = 0; 
		prod_count = PIN_FLIST_ELEM_COUNT(srch_oflist, PIN_FLD_RESULTS, ebufp);
		elem_id = 0;
		cookie = NULL;
 		while ((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflist_1, PIN_FLD_PRODUCTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			result_flist_1 = PIN_FLIST_ELEM_ADD(srch_oflist, PIN_FLD_RESULTS, prod_count, ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PRODUCT_OBJ, result_flist_1, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PRODUCT_NAME, result_flist_1, PIN_FLD_PRODUCT_NAME, ebufp);
			PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_PROVISIONING_TAG, result_flist_1, PIN_FLD_PROVISIONING_TAG, ebufp);
			prod_count = prod_count + 1;
		}
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Modifiled existing products:", srch_oflist);

	elem_id_1 = 0;
	cookie_1 = NULL;
 	while((result_flist_1 = PIN_FLIST_ELEM_GET_NEXT(plan_prod_flistp, PIN_FLD_RESULTS,
		&elem_id_1, 1, &cookie_1, ebufp)) != (pin_flist_t *)NULL)
	{
		new_prov_tag = PIN_FLIST_FLD_GET(result_flist_1, PIN_FLD_PROVISIONING_TAG, 0, ebufp);

		elem_id = 0;
		cookie = NULL;
 		while(srch_oflist && (result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflist, PIN_FLD_RESULTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			prov_tag = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
			if (prov_tag && new_prov_tag && strcmp(new_prov_tag, prov_tag) == 0)
			{
				flg_same_family = 1;
			}
		}
	}

	if (flg_same_family > 0 )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING, "Only one plan from same family is allowed", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00001", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Only one plan from same family is allowed", ebufp);
		goto CLEANUP;
	}


	CLEANUP:
	PIN_FLIST_DESTROY_EX(&read_plan_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflist_1, NULL);
	PIN_FLIST_DESTROY_EX(&plan_prod_flistp, NULL);
	return;
}

/**************************************************
* Function: fm_mso_validation_rule_2()
*     1.  Additional package to be allowed only for 
*          additional connection
*     2. "SUN SOUTH ALL" plan can not be purchased along with          
*         Regional packages (REGIONAL MALAYALAM, REGIONAL TAMIL,       
*         REGIONAL KANNADA, REGIONAL TELUGU)                           
*     3. Maximum 2 Regional packs can be purchased, while trying       
*        to purchase the third one, promted to purchase "SUN SOUTH ALL"
* 
***************************************************/
void
fm_mso_validation_rule_2(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			action,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_srvc_iflist = NULL;
	pin_flist_t		*read_srvc_oflist = NULL;
	pin_flist_t		*catv_info = NULL;
	pin_flist_t		*read_plan_iflist = NULL;
	pin_flist_t		*read_plan_oflist = NULL;
	pin_flist_t		*plan_list = NULL;

	int32			conn_type =-1;
	int32			activate_customer_failure = 1;
	int32			elem_id =0;
	int32			*flags = NULL;
	int64			db =1;
	pin_cookie_t		cookie = NULL;

	void			*vp = NULL;
	char			*plan_code = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validation_rule_2", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validation_rule_2", i_flistp);
	
	read_srvc_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, read_srvc_iflist, PIN_FLD_POID, ebufp);
	catv_info = PIN_FLIST_SUBSTR_ADD(read_srvc_iflist, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_FLD_SET(catv_info, PIN_FLD_CONN_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(catv_info, MSO_FLD_AGREEMENT_NO, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_srvc_iflist:", read_srvc_iflist);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_srvc_iflist, &read_srvc_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&read_srvc_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_srvc_oflist:", read_srvc_oflist);

	catv_info = PIN_FLIST_SUBSTR_GET(read_srvc_oflist, MSO_FLD_CATV_INFO, 0, ebufp);
	conn_type = *(int32*)PIN_FLIST_FLD_GET(catv_info, PIN_FLD_CONN_TYPE, 0, ebufp);
	PIN_FLIST_FLD_COPY(catv_info, MSO_FLD_AGREEMENT_NO, i_flistp, MSO_FLD_AGREEMENT_NO, ebufp );

	PIN_FLIST_DESTROY_EX(&read_srvc_oflist, NULL);

	if (conn_type == 0)
	{
	   if ( action == MSO_OP_CUST_ADD_PLAN )
	   {
		while((plan_list = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			vp = PIN_FLIST_FLD_GET(plan_list, PIN_FLD_PLAN_OBJ, 0, ebufp);
			if (vp)
			{
				read_plan_iflist = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_POID, (poid_t*)vp, ebufp );
				PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_CODE, NULL, ebufp );
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_plan_iflist:", read_plan_iflist);

				PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_plan_iflist, &read_plan_oflist, ebufp);
				PIN_FLIST_DESTROY_EX(&read_plan_iflist, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
					goto CLEANUP;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_plan_oflist:", read_plan_oflist);

				plan_code = PIN_FLIST_FLD_GET(read_plan_oflist, PIN_FLD_CODE, 0, ebufp );
				if (plan_code && strstr(plan_code, "ADDITIONAL"))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matched");
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Only Additional connection is allowed to purchase Additional Plans", ebufp);
					PIN_ERRBUF_RESET(ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Only Additional connection is allowed to purchase Additional Plans",ebufp); 
				}
				PIN_FLIST_DESTROY_EX(&read_plan_oflist, NULL);
			}
		}
	   }
	   if ( action == MSO_OP_CUST_CHANGE_PLAN )
	   {
		while((plan_list = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			flags = (int32*)PIN_FLIST_FLD_GET(plan_list, PIN_FLD_FLAGS, 0, ebufp );
			if (flags && *flags ==1)
			{
				vp = PIN_FLIST_FLD_GET(plan_list, PIN_FLD_PLAN_OBJ, 0, ebufp);
				if (vp)
				{
					read_plan_iflist = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_POID, (poid_t*)vp, ebufp );
					PIN_FLIST_FLD_SET(read_plan_iflist, PIN_FLD_CODE, NULL, ebufp );
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_plan_iflist:", read_plan_iflist);

					PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_plan_iflist, &read_plan_oflist, ebufp);
					PIN_FLIST_DESTROY_EX(&read_plan_iflist, NULL);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						*ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_READ_FLDS: ()", ebufp);
						goto CLEANUP;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_plan_oflist:", read_plan_oflist);

					plan_code = PIN_FLIST_FLD_GET(read_plan_oflist, PIN_FLD_CODE, 0, ebufp );
					if (plan_code && strstr(plan_code, "ADDITIONAL"))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Matched");
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Only Additional connection is allowed to purchase Additional Plans", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						*ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Only Additional connection is allowed to purchase Additional Plans", ebufp); 
					}
					PIN_FLIST_DESTROY_EX(&read_srvc_oflist, NULL);
				}
			}
		}
	   }
	}
	CLEANUP:
	return;
}


/**************************************************
* Function: fm_mso_validate_pay_type()
*     1.  While purchasing a plan if the plan and account 
*         both does not have same pay_type(PREPAID/POSTPAID)
*         then purchase is not allowed
*     2. Called from Activation, Add_plan, and change Plan
* 
***************************************************/
void
fm_mso_validate_pay_type(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			opcode,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_iflist = NULL;
	pin_flist_t		*srch_oflist = NULL;
	pin_flist_t		*args_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*usage_map = NULL;
	pin_flist_t		*plan_list_code = NULL;
	pin_flist_t		*plans = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*catv_info = NULL;

	poid_t			*acnt_pdp = NULL;
	poid_t			*service_pdp = NULL;

	int32			srch_flags  = 512;
	int32			*action = NULL;
	int32			*conn_type = NULL;
	int32			elem_id =0;
	int32			plan_type_is_prepaid = 0;
	int32			acnt_type_is_prepaid = 0;
	int32			plan_type_is_postpaid = 0;
	int32			acnt_type_is_postpaid = 0;
	int32			failure = 1;
	int32			flg_read_payinfo_for_addl_tv =0;
	int32			business_type = -1;
	int32			account_type = -1;
	int32			account_model = -1;
	int32			subscriber_type = -1;

	int64			db =1;
	pin_cookie_t		cookie = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;

	char			srch_template[2000];
	char			tmp_str[50];
	char			*event_type = NULL;
	char			*service_pdp_type = NULL;
	char			billinfo_id[10];

	int32			id = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_pay_type", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_pay_type", i_flistp);
  
	srch_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp);
	vp =  PIN_FLIST_ELEM_ADD(args_flist, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(vp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 4, ebufp);
	vp =  PIN_FLIST_ELEM_ADD(args_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(vp, PIN_FLD_DEAL_OBJ, NULL, ebufp);

	strcpy(srch_template, "select X from /product 1, /deal 2, /plan 3 where 1.F1 = 2.F2 and 2.F3 = 3.F4 and (");
	if (opcode == MSO_OP_CUST_ACTIVATE_CUSTOMER )
	{
		acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called by MSO_OP_CUST_ACTIVATE_CUSTOMER");
		plan_list_code = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PLAN_LIST_CODE, 1, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plan_list_code", plan_list_code);
		if (plan_list_code)
		{
			while((plans = PIN_FLIST_ELEM_GET_NEXT(plan_list_code, PIN_FLD_PLAN,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1111");
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plans", plans);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "srch_template");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, srch_template);
				if (id == 1)
				{
					memset(tmp_str,'\0',sizeof(tmp_str));
					sprintf (tmp_str, " 3.F%d = V%d", id+4, id+4);
					strcat( srch_template, tmp_str);
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "2222");
					memset(tmp_str,'\0',sizeof(tmp_str));
					sprintf (tmp_str, " or 3.F%d = V%d", id+4, id+4);
					strcat( srch_template, tmp_str);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "3333");
				}
				args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, id+4, ebufp);
				PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, args_flist, PIN_FLD_POID,  ebufp);
				id++;
			}
		}

		// Parse PIN_FLD_SERVICES to get conn_type to determine MAIN/ADDITIONAL 
		services = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
		if (services)
		{
			catv_info = PIN_FLIST_SUBSTR_GET(services, MSO_FLD_CATV_INFO, 1, ebufp);
			if (catv_info)
			{
				conn_type = (int32*)PIN_FLIST_FLD_GET(catv_info, PIN_FLD_CONN_TYPE, 1, ebufp);
			}
		}

		// PIN_FLD_PAYINFO will be present in case of MAIN TV activation
		payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
		if (payinfo)
		{
			inherited_info = PIN_FLIST_SUBSTR_GET(payinfo, PIN_FLD_INHERITED_INFO, 1, ebufp);
			if (inherited_info)
			{
				inv_info = PIN_FLIST_ELEM_GET(inherited_info, PIN_FLD_INV_INFO, 0, 1, ebufp);
				if (inv_info)
				{
					vp1 = PIN_FLIST_FLD_GET(inv_info, PIN_FLD_INDICATOR, 1, ebufp);
					if (vp1 && *(int32*)vp1 == 1 )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_type_is_prepaid");
						acnt_type_is_prepaid=1;	
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_type_is_postpaid");
						acnt_type_is_postpaid=1;
					}
				}
			}
		}
		else if (conn_type && *conn_type == 1 ) //1:Additional 0:Main
		{
		// PIN_FLD_PAYINFO will NOT be present in case of ADDITIONAL TV activation so will read the same
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flg_read_payinfo_for_addl_tv=1");
			acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
			service_pdp = PIN_FLIST_FLD_GET(services, PIN_FLD_SERVICE_OBJ, 0, ebufp );
			flg_read_payinfo_for_addl_tv =1;
		}
	}
	else if (opcode == MSO_OP_CUST_ADD_PLAN)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called by MSO_OP_CUST_ADD_PLAN");
		while((plans = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			if (id == 1)
			{
				memset(tmp_str,'\0',sizeof(tmp_str));
				sprintf (tmp_str, " 3.F%d = V%d", id+4, id+4);
				strcat( srch_template, tmp_str);
			}
			else
			{
				memset(tmp_str,'\0',sizeof(tmp_str));
				sprintf (tmp_str, " or 3.F%d = V%d", id+4, id+4);
				strcat( srch_template, tmp_str);
			}
			args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, id+4, ebufp);
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, args_flist, PIN_FLD_POID,  ebufp);
			id++;
		}
	}
	else if (opcode == MSO_OP_CUST_CHANGE_PLAN)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called by MSO_OP_CUST_CHANGE_PLAN");
		while((plans = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			action = (int32*)PIN_FLIST_FLD_GET(plans, PIN_FLD_FLAGS, 1, ebufp);
			if (action && *action == 1 )
			{
			}
			if (id == 1)
			{
				memset(tmp_str,'\0',sizeof(tmp_str));
				sprintf (tmp_str, " 3.F%d = V%d", id+4, id+4);
				strcat( srch_template, tmp_str);
			}
			else
			{
				memset(tmp_str,'\0',sizeof(tmp_str));
				sprintf (tmp_str, " or 3.F%d = V%d", id+4, id+4);
				strcat( srch_template, tmp_str);
			}
			args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, id+4, ebufp);
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, args_flist, PIN_FLD_POID,  ebufp);
			id++;
		}
	}
	strcat( srch_template, " )");

	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, srch_template, ebufp );
	result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	usage_map = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(usage_map, PIN_FLD_EVENT_TYPE, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_iflist:", srch_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
//		PIN_ERRBUF_RESET(ebufp);
//		*ret_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_oflist:", srch_oflist);

	elem_id = 0;
	cookie = NULL;
	while((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflist, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		usage_map = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_USAGE_MAP, 0, 0, ebufp);
		event_type = PIN_FLIST_FLD_GET(usage_map, PIN_FLD_EVENT_TYPE, 0 , ebufp);
		if (event_type && 
		    strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward") &&
		    (!strstr(event_type, "mso_sb_adn_fdp")) &&
		    (!strstr(event_type, "mso_sb_pkg_fdp")) &&
		    (!strstr(event_type, "mso_sb_alc_fdp")) &&
		    (!strstr(event_type, "mso_et"))
		   )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan_type_is_prepaid");
			plan_type_is_prepaid=1;
			break;
		}
		else if (event_type && 
			 strstr(event_type, "/event/billing/product/fee/cycle/cycle_arrear") &&
			 (!strstr(event_type, "mso_sb_adn_fdp")) &&
		         (!strstr(event_type, "mso_sb_pkg_fdp")) &&
		         (!strstr(event_type, "mso_sb_alc_fdp")) &&
			 (!strstr(event_type, "mso_et"))
			)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan_type_is_postpaid");
			plan_type_is_postpaid=1;
		}
	}
	PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);

	if (opcode == MSO_OP_CUST_ADD_PLAN ||
	    opcode == MSO_OP_CUST_CHANGE_PLAN ||
	    flg_read_payinfo_for_addl_tv == 1
	   ) 
	{
		if (flg_read_payinfo_for_addl_tv != 1 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "reading acnt_pdp and service_pdp") ;
			acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
			service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "already read acnt_pdp and service_pdp");
		}

		service_pdp_type = (char*)PIN_POID_GET_TYPE(service_pdp);
		if (service_pdp_type && strcmp (service_pdp_type,"/service/catv")==0)
		{
			strcpy(billinfo_id, "CATV");
		}
		else
		{
			strcpy(billinfo_id, "BB");
		}
		srch_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
		PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /payinfo 1, /billinfo 2 where 2.F1 = V1 and 2.F2 = V2 and 2.F3 = 1.F4 ", ebufp);
		
		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp );
		//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, args_flist,PIN_FLD_ACCOUNT_OBJ,  ebufp );

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_BILLINFO_ID, billinfo_id, ebufp);

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);

		result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		inv_info = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_INV_INFO, 0, ebufp);
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INDICATOR, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_iflist:", srch_iflist);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
	//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
	//		PIN_ERRBUF_RESET(ebufp);
	//		*ret_flistp = PIN_FLIST_CREATE(ebufp);
	//		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
	//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
	//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
	//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_oflist:", srch_oflist);

		result_flist = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_RESULTS, 0, 1, ebufp);
		inv_info = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_INV_INFO, 0, 1, ebufp);
		vp1 = PIN_FLIST_FLD_GET(inv_info, PIN_FLD_INDICATOR, 1, ebufp);
		if (vp1 && *(int32*)vp1 == 1 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_type_is_prepaid");
			acnt_type_is_prepaid=1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_type_is_postpaid");
			acnt_type_is_postpaid=1;
		}
	}

	if (acnt_type_is_prepaid && plan_type_is_postpaid )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Account pay-type:Prepaid, allowed to select Prepaid plans only", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Account pay-type:Prepaid, allowed to select Prepaid plans only", ebufp);
		goto CLEANUP;
	}
	if ( acnt_type_is_postpaid && plan_type_is_prepaid )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Account pay-type:Postpaid, allowed to select Postpaid plans only", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Account pay-type:Postpaid, allowed to select Postpaid plans only", ebufp);
		goto CLEANUP;
	}

	CLEANUP:
	return;
}

/**************************************************
* Function: fm_mso_validate_pay_type_corp()
*     1.  While purchasing a plan if the plan and account 
*         both does not have same pay_type(PREPAID/POSTPAID)
*         then purchase is not allowed
*     2. If pay-term of parent not equal to the pay-term of child
*         then not allowed to proceed
*     2. Called from Activation, Add_plan, and change Plan
* 
***************************************************/
void
fm_mso_validate_pay_type_corp(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			opcode,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*payinfo = NULL;
	pin_flist_t		*inherited_info = NULL;
	pin_flist_t		*inv_info = NULL;
	pin_flist_t		*group_info = NULL;
	pin_flist_t		*srch_iflist = NULL;
	pin_flist_t		*srch_oflist = NULL;
	pin_flist_t		*args_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*plan_list_code = NULL;
	pin_flist_t		*plans = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*catv_info = NULL;
	pin_flist_t		*usage_map = NULL;


	poid_t			*parent_acnt_pdp = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*service_pdp = NULL;

	int32			srch_flags  = 512;
	int32			*action = NULL;
	int32			*conn_type = NULL;
	int32			elem_id =0;
	int32			plan_type_is_prepaid = 0;
	int32			plan_type_is_postpaid = 0;

	int32			corp_child_acnt_type_is_prepaid = 0;
	int32			corp_child_acnt_type_is_postpaid = 0;

	int32			failure = 1;
	int32			child_pay_term = 0;
	int32			flg_read_payinfo_for_addl_tv = 0;

	int64			db =1;
	pin_cookie_t		cookie = NULL;

	void			*vp = NULL;
	void			*vp1 = NULL;

	char			srch_template[200];
	char			tmp_str[50];
	char			*event_type = NULL;
	char			*service_pdp_type = NULL;
	char			billinfo_id[10];

	int32			id = 1;
	int32			indicator = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_pay_type_corp", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_pay_type_corp", i_flistp);

	/*****************************************************************************
	For corp child CATV activation if PIN_FLD_PAYINFO is passed then INDICATOR
	should be same as that of parent
	START
	******************************************************************************/
	payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
	if (payinfo)
	{
		inherited_info = PIN_FLIST_SUBSTR_GET(payinfo, PIN_FLD_INHERITED_INFO, 1, ebufp);
		if (inherited_info)
		{
			inv_info = PIN_FLIST_ELEM_GET(inherited_info, PIN_FLD_INV_INFO, 0, 1, ebufp);
			if (inv_info)
			{
				vp1 = PIN_FLIST_FLD_GET(inv_info, PIN_FLD_INDICATOR, 1, ebufp);
				if (vp1 && *(int32*)vp1 == 1 )
				{
					child_pay_term = *(int32*)vp1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "corp_child_acnt_type_is_prepaid");
					corp_child_acnt_type_is_prepaid=1;	
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "corp_child_acnt_type_is_postpaid");
					corp_child_acnt_type_is_postpaid=1;
				}
			}
		}
	}
	/*****************************************************************************
	For corp child CATV activation if PIN_FLD_PAYINFO is passed then INDICATOR
	should be same as that of parent
	End
	******************************************************************************/
	group_info = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 0, ebufp);
	parent_acnt_pdp = PIN_FLIST_FLD_GET(group_info, PIN_FLD_PARENT, 0, ebufp); 

	srch_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /payinfo where F1 = V1 ", ebufp);
	
	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flist,PIN_FLD_ACCOUNT_OBJ, parent_acnt_pdp, ebufp );

	result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	inv_info = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_INV_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INDICATOR, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_iflist:", srch_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
//		PIN_ERRBUF_RESET(ebufp);
//		*ret_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_oflist:", srch_oflist);

	result_flist = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (result_flist)
	{
		inv_info = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_INV_INFO, 0, 1, ebufp);
		if (inv_info )
		{
			indicator = *(int32*)PIN_FLIST_FLD_GET(inv_info, PIN_FLD_INDICATOR, 1, ebufp);
		}
		if (payinfo && (indicator != child_pay_term) )
		{
			PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Child PAY-term is not matching Parent PAY-term", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Child PAY-term is not matching Parent PAY-term", ebufp);
			goto CLEANUP;
		}
		PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);

		if (indicator == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "corp_child_acnt_type_is_prepaid");
			corp_child_acnt_type_is_prepaid = 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "corp_child_acnt_type_is_postpaid");
			corp_child_acnt_type_is_postpaid =1;
		}
	}
	

	/*****************************************************************************
	Search event type of selected plans to determine plan type(prepaid/postpaid)
	START
	******************************************************************************/
	srch_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp);
	vp =  PIN_FLIST_ELEM_ADD(args_flist, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(vp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 4, ebufp);
	vp =  PIN_FLIST_ELEM_ADD(args_flist, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(vp, PIN_FLD_DEAL_OBJ, NULL, ebufp);

	strcpy(srch_template, "select X from /product 1, /deal 2, /plan 3 where 1.F1 = 2.F2 and 2.F3 = 3.F4 and (");
	if (opcode == MSO_OP_CUST_ACTIVATE_CUSTOMER )
	{
		group_info = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 0, ebufp);
		parent_acnt_pdp = PIN_FLIST_FLD_GET(group_info, PIN_FLD_PARENT, 0, ebufp); 
		
		acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called by MSO_OP_CUST_ACTIVATE_CUSTOMER");
		plan_list_code = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_PLAN_LIST_CODE, 1, ebufp);
		if (plan_list_code)
		{
			while((plans = PIN_FLIST_ELEM_GET_NEXT(plan_list_code, PIN_FLD_PLAN,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				if (id == 1)
				{
					memset(tmp_str,'\0',sizeof(tmp_str));
					sprintf (tmp_str, " 3.F%d = V%d", id+4, id+4);
					strcat( srch_template, tmp_str);
				}
				else
				{
					memset(tmp_str,'\0',sizeof(tmp_str));
					sprintf (tmp_str, " or 3.F%d = V%d", id+4, id+4);
					strcat( srch_template, tmp_str);
				}
				args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, id+4, ebufp);
				PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, args_flist, PIN_FLD_POID,  ebufp);
				id++;
			}
		}

		// Parse PIN_FLD_SERVICES to get conn_type to determine MAIN/ADDITIONAL 
		services = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SERVICES, 0, 1, ebufp);
		if (services)
		{
			catv_info = PIN_FLIST_SUBSTR_GET(services, MSO_FLD_CATV_INFO, 1, ebufp);
			if (catv_info)
			{
				conn_type = (int32*)PIN_FLIST_FLD_GET(catv_info, PIN_FLD_CONN_TYPE, 1, ebufp);
			}
		}

		// PIN_FLD_PAYINFO will be present in case of MAIN TV activation
		payinfo = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 1, ebufp);
		if (payinfo)
		{
			inherited_info = PIN_FLIST_SUBSTR_GET(payinfo, PIN_FLD_INHERITED_INFO, 1, ebufp);
			if (inherited_info)
			{
				inv_info = PIN_FLIST_ELEM_GET(inherited_info, PIN_FLD_INV_INFO, 0, 1, ebufp);
				if (inv_info)
				{
					vp1 = PIN_FLIST_FLD_GET(inv_info, PIN_FLD_INDICATOR, 1, ebufp);
					if (vp1 && *(int32*)vp1 == 1 )
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "corp_child_acnt_type_is_prepaid");
						corp_child_acnt_type_is_prepaid=1;	
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "corp_child_acnt_type_is_postpaid");
						corp_child_acnt_type_is_postpaid=1;
					}
				}
			}
		}
		else if (conn_type && *conn_type == 1 ) //1:Additional 0:Main
		{
		// PIN_FLD_PAYINFO will NOT be present in case of ADDITIONAL TV activation so will read the same
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "flg_read_payinfo_for_addl_tv=1");
			acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
			service_pdp = PIN_FLIST_FLD_GET(services, PIN_FLD_SERVICE_OBJ, 0, ebufp );
			flg_read_payinfo_for_addl_tv =1;
		}
	}
	else if (opcode == MSO_OP_CUST_ADD_PLAN)
	{
		//search in /group/billing to get the parent account 
//		0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
//		0 PIN_FLD_FLAGS           INT [0] 768
//		0 PIN_FLD_TEMPLATE        STR [0] "select X from /group/billing  where F1 = V1 "
//		0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
//		1    PIN_FLD_MEMBERS       ARRAY [1] allocated 20, used 2
//		2       PIN_FLD_OBJECT         POID [0] 0.0.0.1 /account 3183477 1
//		0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 2
//		1    PIN_FLD_ACCOUNT_OBJ    POID [0]NULL
 
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called by MSO_OP_CUST_ADD_PLAN");
		while((plans = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			if (id == 1)
			{
				memset(tmp_str,'\0',sizeof(tmp_str));
				sprintf (tmp_str, " 3.F%d = V%d", id+4, id+4);
				strcat( srch_template, tmp_str);
			}
			else
			{
				memset(tmp_str,'\0',sizeof(tmp_str));
				sprintf (tmp_str, " or 3.F%d = V%d", id+4, id+4);
				strcat( srch_template, tmp_str);
			}
			args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, id+4, ebufp);
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, args_flist, PIN_FLD_POID,  ebufp);
			id++;
		}
	}
	else if (opcode == MSO_OP_CUST_CHANGE_PLAN)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called by MSO_OP_CUST_CHANGE_PLAN");
		while((plans = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			action = (int32*)PIN_FLIST_FLD_GET(plans, PIN_FLD_FLAGS, 1, ebufp);
			if (action && *action == 1 )
			{
			}
			if (id == 1)
			{
				memset(tmp_str,'\0',sizeof(tmp_str));
				sprintf (tmp_str, " 3.F%d = V%d", id+4, id+4);
				strcat( srch_template, tmp_str);
			}
			else
			{
				memset(tmp_str,'\0',sizeof(tmp_str));
				sprintf (tmp_str, " or 3.F%d = V%d", id+4, id+4);
				strcat( srch_template, tmp_str);
			}
			args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, id+4, ebufp);
			PIN_FLIST_FLD_COPY(plans, PIN_FLD_PLAN_OBJ, args_flist, PIN_FLD_POID,  ebufp);
			id++;
		}
	}
	strcat( srch_template, " )");

	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, srch_template, ebufp );
	result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	usage_map = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(usage_map, PIN_FLD_EVENT_TYPE, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_iflist:", srch_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
//		PIN_ERRBUF_RESET(ebufp);
//		*ret_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_oflist:", srch_oflist);
	/*****************************************************************************
	Search event type of selected plans to determine plan type(prepaid/postpaid)
	END
	******************************************************************************/
 
	elem_id = 0;
	cookie = NULL;
	while((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_oflist, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		usage_map = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_USAGE_MAP, 0, 0, ebufp);
		event_type = PIN_FLIST_FLD_GET(usage_map, PIN_FLD_EVENT_TYPE, 0 , ebufp);
		if (event_type && 
		    strstr(event_type, "/event/billing/product/fee/cycle/cycle_forward") &&
		    (!strstr(event_type, "mso_sb_adn_fdp")) &&
		    (!strstr(event_type, "mso_sb_pkg_fdp")) &&
		    (!strstr(event_type, "mso_sb_alc_fdp")) &&
		     (!strstr(event_type, "mso_et"))
		   )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan_type_is_prepaid");
			plan_type_is_prepaid=1;
			break;
		}
		else if (event_type && 
			 strstr(event_type, "/event/billing/product/fee/cycle/cycle_arrear") &&
			 (!strstr(event_type, "mso_sb_adn_fdp")) &&
		         (!strstr(event_type, "mso_sb_pkg_fdp")) &&
		         (!strstr(event_type, "mso_sb_alc_fdp")) &&
			 (!strstr(event_type, "mso_et"))
			)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "plan_type_is_postpaid");
			plan_type_is_postpaid=1;
		}
	}
	PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);

  
	/*****************************************************************************
	Search payinfo of parent for all cases 
	START
	******************************************************************************/
	if (opcode == MSO_OP_CUST_ADD_PLAN ||
	    opcode == MSO_OP_CUST_CHANGE_PLAN ||
	    flg_read_payinfo_for_addl_tv == 1
	   ) 
	{
		if (flg_read_payinfo_for_addl_tv != 1 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "reading acnt_pdp and service_pdp") ;
			acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
			service_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp );
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "already read acnt_pdp and service_pdp");
		}

		service_pdp_type = (char*)PIN_POID_GET_TYPE(service_pdp);
		if (service_pdp_type && strcmp (service_pdp_type,"/service/catv")==0)
		{
			strcpy(billinfo_id, "CATV");
		}
		else
		{
			strcpy(billinfo_id, "BB");
		}
		srch_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
		PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /payinfo where 1.F1 = V1 ", ebufp);
		//PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /payinfo 1, /billinfo 2 where 2.F1 = V1 and 2.F2 = V2 and 2.F3 = 1.F4 ", ebufp);
		
		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, args_flist,PIN_FLD_ACCOUNT_OBJ,  ebufp );
/*
		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_BILLINFO_ID, billinfo_id, ebufp);

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_PAYINFO_OBJ, NULL, ebufp);

		args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flist, PIN_FLD_POID, NULL, ebufp);
*/
		result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		inv_info = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_INV_INFO, 0, ebufp);
		PIN_FLIST_FLD_SET(inv_info, PIN_FLD_INDICATOR, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_iflist:", srch_iflist);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
	//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
	//		PIN_ERRBUF_RESET(ebufp);
	//		*ret_flistp = PIN_FLIST_CREATE(ebufp);
	//		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
	//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &activate_customer_failure, ebufp);
	//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
	//		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_oflist:", srch_oflist);

		result_flist = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_RESULTS, 0, 1, ebufp);
		inv_info = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_INV_INFO, 0, 1, ebufp);
		vp1 = PIN_FLIST_FLD_GET(inv_info, PIN_FLD_INDICATOR, 1, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                      PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error PCM_OP_SEARCH result: ()", ebufp);
                      PIN_ERRBUF_RESET(ebufp);
                      goto CLEANUP;
                }

		if (vp1 && *(int32*)vp1 == 1 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "corp_child_acnt_type_is_prepaid");
			corp_child_acnt_type_is_prepaid=1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "corp_child_acnt_type_is_postpaid");
			corp_child_acnt_type_is_postpaid=1;
		}
	}
	/*****************************************************************************
	Search payinfo of parent for all cases 
	START
	******************************************************************************/
	if (corp_child_acnt_type_is_prepaid && plan_type_is_postpaid )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Account pay-type:Prepaid, allowed to select Prepaid plans only", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Account pay-type:Prepaid, allowed to select Prepaid plans only", ebufp);
		goto CLEANUP;
	}
	if ( corp_child_acnt_type_is_postpaid && plan_type_is_prepaid )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Account pay-type:Postpaid, allowed to select Postpaid plans only", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Account pay-type:Postpaid, allowed to select Postpaid plans only", ebufp);
		goto CLEANUP;
	}

	CLEANUP:
	return;
}

/***************************************************************
* Function:     fm_mso_get_parent_info()
* Owner:        Gautam Adak
* Decription:   For getting device information
*                based on 
*               - Device ID (search_type = MSO_FLAG_SRCH_BY_ID)
*               - PIN (search_type = MSO_FLAG_SRCH_BY_PIN)
*               - More options can be added
*
*
*
****************************************************************/
int32 
fm_mso_get_parent_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_iflist  = NULL;
	pin_flist_t		*srch_oflist  = NULL;
	pin_flist_t		*args_flist   = NULL;
	pin_flist_t		*members      = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*group_info = NULL;
	pin_flist_t		*get_bust_flistp = NULL;
	pin_flist_t		*get_bust_flistp_ret_flistp = NULL;
	int32                   bus_type = 0;
	int32                   sub_type   = -1;
	int                     arr_business_type[8];

	int64			db = -1;

	int32			srch_flags = 768;
	int32			failure = 1; 
	int32			ret_val = -1;



	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_parent_info", ebufp);
		return -1 ;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_parent_info acnt_pdp", acnt_pdp);

	db = PIN_POID_GET_DB(acnt_pdp);
	srch_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /group/billing where F1.id = V1 ", ebufp);
	
	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
	members = PIN_FLIST_ELEM_ADD(args_flist, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(members, PIN_FLD_OBJECT, acnt_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_iflist:", srch_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flist, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
		return -1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_oflist:", srch_oflist);
	result_flist = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		group_info = PIN_FLIST_SUBSTR_ADD(i_flist, PIN_FLD_GROUP_INFO, ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_ACCOUNT_OBJ, group_info, PIN_FLD_PARENT, ebufp );
		/***** CATV Changes *****/
		//ret_val = CORPORATE_CHILD;
                get_bust_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(get_bust_flistp,PIN_FLD_POID,acnt_pdp, ebufp );

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read object input:", get_bust_flistp);
                PCM_OP(ctxp, PCM_OP_READ_OBJ, 0,get_bust_flistp, &get_bust_flistp_ret_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&get_bust_flistp,NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read object output:", get_bust_flistp_ret_flistp);

                bus_type = *(int32*)PIN_FLIST_FLD_GET(get_bust_flistp_ret_flistp,PIN_FLD_BUSINESS_TYPE, 1, ebufp);
        	num_to_array(bus_type, arr_business_type);
        	sub_type = 10*(arr_business_type[2])+arr_business_type[3];// next 2 digits
		if(sub_type == CORPORATE_CATV_CHILD)
			ret_val = CORPORATE_CATV_CHILD;
		else
			ret_val = CORPORATE_CHILD;
	}
	PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
	//Added in CATV merge
	PIN_FLIST_DESTROY_EX(&get_bust_flistp_ret_flistp, NULL);

	CLEANUP:
	return ret_val;
}


/***************************************************************
* Function:     fm_mso_is_child_service()
* Decription:   This function will check whether given service 
*               is a child connection or not  
****************************************************************/
void
fm_mso_is_child_service(
        pcm_context_t           *ctxp,
        poid_t                  *serv_pdp,
	int32			parent_info_flag,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_iflist  = NULL;
        pin_flist_t             *srch_oflist  = NULL;
        pin_flist_t             *args_flist   = NULL;
        pin_flist_t             *mem_flistp   = NULL;
        pin_flist_t             *result_flist = NULL;
        poid_t                  *parent_acnt_pdp = NULL;
        int64                   db = -1;
        int32                   srch_flags = 768;
        int32                   failure = 1;
	int32			active_status = 10100;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_is_child_service service_pdp", serv_pdp);

        db = PIN_POID_GET_DB(serv_pdp);
        srch_iflist = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
        PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, "select X from /group/mso_hierarchy where F1.id = V1 and F2 = V2 ", ebufp);

        args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp);
        mem_flistp = PIN_FLIST_ELEM_ADD(args_flist, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(mem_flistp, PIN_FLD_OBJECT, serv_pdp, ebufp);

	args_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flist, PIN_FLD_STATUS, &active_status, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PARENT, NULL, ebufp);
        mem_flistp = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search hierarachy group", srch_iflist);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);

        PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                *ret_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, serv_pdp, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
                PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calling PCM_OP_SEARCH: ()", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_oflist:", srch_oflist);
        result_flist = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if (result_flist && parent_info_flag)
        {
                parent_acnt_pdp = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_PARENT, 0, ebufp);
                fm_mso_get_account_info(ctxp, parent_acnt_pdp, &args_flist, ebufp);

                mem_flistp = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_MEMBERS, PIN_ELEMID_ANY, 1, ebufp);
                if (mem_flistp)
                {
                        if (args_flist)
                        {
                                PIN_FLIST_FLD_COPY(args_flist, PIN_FLD_ACCOUNT_NO, mem_flistp, MSO_FLD_PARENT_ACCT_NO, ebufp );
                        }
                        *ret_flistp = PIN_FLIST_COPY(mem_flistp, ebufp);

                }
                PIN_FLIST_DESTROY_EX(&args_flist, NULL);
        }

CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
        return;
}

/***************************************************************
* Function:     fm_mso_et_product_rule_delhi()
* Owner:        Gautam Adak
* Decription:   
*   1. ET plan purchase is allowed only for main CATV if STATE is DELHI
*   

****************************************************************/
void 
fm_mso_et_product_rule_delhi(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flist,
	poid_t			*acnt_pdp,
	int32			action,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*plan_list_code         = NULL;
	pin_flist_t		*plans                  = NULL;
	pin_flist_t		*r_flistp               = NULL;
	pin_flist_t		*nameinfo_billing       = NULL;
	pin_flist_t		*nameinfo_installation  = NULL;
	pin_flist_t		*read_srvc_iflist       = NULL;
	pin_flist_t		*read_srvc_oflist       = NULL;
	pin_flist_t		*catv_info              = NULL;

	int32			elem_id = 0;
	int32			failure = 1;
	int32			*conn_type = NULL;

	pin_cookie_t		cookie = NULL;

	char			*plan_code = NULL;
	char			*state = NULL;

  
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_et_product_rule_delhi", ebufp);
		return ;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_et_product_rule_delhi acnt_pdp", acnt_pdp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "i_flist", i_flist);

	//Parse plan list to determine if ET plan is passed

	if (action == MSO_OP_CUST_ACTIVATE_CUSTOMER)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called by ACTIVATE_CUSTOMER");

		plan_list_code = PIN_FLIST_SUBSTR_GET(i_flist, PIN_FLD_PLAN_LIST_CODE, 1, ebufp);
		if (plan_list_code)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plan_list_code", plan_list_code);
			while((plans = PIN_FLIST_ELEM_GET_NEXT(plan_list_code, PIN_FLD_PLAN,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "plans", plans);
				plan_code = PIN_FLIST_FLD_GET(plans, PIN_FLD_CODE, 1, ebufp);
				if (plan_code && strstr(plan_code , "STB Zero Value"))
				{
					fm_mso_get_account_info(ctxp, acnt_pdp, &r_flistp, ebufp);
					nameinfo_billing = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_NAMEINFO, 1, 1, ebufp);
					nameinfo_installation = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_NAMEINFO, 2, 1, ebufp);

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_billing", nameinfo_billing);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_installation", nameinfo_installation);

					state = (char*)PIN_FLIST_FLD_GET(nameinfo_billing, PIN_FLD_STATE, 1, ebufp);
					if (strcmp(state, "DELHI") == 0 )
					{
					
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ET Plan can be added only in main CATV for Delhi", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						*ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flist, PIN_FLD_ACCOUNT_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "ET Plan can be added only in main CATV for Delhi", ebufp);
					}
					PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
				}
			}
		}
	}
	if (action == MSO_OP_CUST_ADD_PLAN)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called by ADD_PLAN");
		
		read_srvc_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flist, PIN_FLD_SERVICE_OBJ, read_srvc_iflist, PIN_FLD_POID, ebufp);
		PIN_FLIST_SUBSTR_SET(read_srvc_iflist, NULL, MSO_FLD_CATV_INFO, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_srvc_iflist", read_srvc_iflist);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_srvc_iflist, &read_srvc_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&read_srvc_iflist, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in read srvc iflist", ebufp);
			PIN_ERRBUF_RESET(ebufp);

			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in read srvc iflist", ebufp);
	                PIN_FLIST_DESTROY_EX(&read_srvc_oflist, NULL);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_srvc_oflist", read_srvc_oflist);

		catv_info = PIN_FLIST_SUBSTR_GET(read_srvc_oflist, MSO_FLD_CATV_INFO, 1, ebufp);
		if (catv_info)
		{
			conn_type = (int32*)PIN_FLIST_FLD_GET(catv_info, PIN_FLD_CONN_TYPE, 1, ebufp);
			if (conn_type && *conn_type == 1)
			{
				plan_code = PIN_FLIST_FLD_GET(i_flist, PIN_FLD_NAME, 1, ebufp);
				if (plan_code && strstr(plan_code , "STB Zero Value"))
				{
					fm_mso_get_account_info(ctxp, acnt_pdp, &r_flistp, ebufp);
					nameinfo_billing = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_NAMEINFO, 1, 1, ebufp);
					nameinfo_installation = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_NAMEINFO, 2, 1, ebufp);

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_billing", nameinfo_billing);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_installation", nameinfo_installation);

					state = (char*)PIN_FLIST_FLD_GET(nameinfo_billing, PIN_FLD_STATE, 1, ebufp);
					if (strcmp(state, "DELHI") == 0 )
					{
					
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ET Plan can be added only in main CATV for Delhi", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						*ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flist, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "ET Plan can be added only in main CATV for Delhi", ebufp);
					}
					PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
				}
			}
		}
	}
	if (action == MSO_OP_CUST_CHANGE_PLAN)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Called by CHANGE_PLAN");
		
		read_srvc_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flist, PIN_FLD_SERVICE_OBJ, read_srvc_iflist, PIN_FLD_POID, ebufp);
		PIN_FLIST_SUBSTR_SET(read_srvc_iflist, NULL, MSO_FLD_CATV_INFO, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_srvc_iflist", read_srvc_iflist);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_srvc_iflist, &read_srvc_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&read_srvc_iflist, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in read srvc iflist", ebufp);
			PIN_ERRBUF_RESET(ebufp);

			*ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
			PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "Error in read srvc iflist", ebufp);
	                PIN_FLIST_DESTROY_EX(&read_srvc_oflist, NULL);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read_srvc_oflist", read_srvc_oflist);

		catv_info = PIN_FLIST_SUBSTR_GET(read_srvc_oflist, MSO_FLD_CATV_INFO, 1, ebufp);
		if (catv_info)
		{
			conn_type = (int32*)PIN_FLIST_FLD_GET(catv_info, PIN_FLD_CONN_TYPE, 1, ebufp);
			if (conn_type && *conn_type == 1)
			{
				plan_code = PIN_FLIST_FLD_GET(i_flist, PIN_FLD_NAME, 1, ebufp);
				if (plan_code && strstr(plan_code , "STB Zero Value"))
				{
					fm_mso_get_account_info(ctxp, acnt_pdp, &r_flistp, ebufp);
					nameinfo_billing = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_NAMEINFO, 1, 1, ebufp);
					nameinfo_installation = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_NAMEINFO, 2, 1, ebufp);

					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_billing", nameinfo_billing);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "nameinfo_installation", nameinfo_installation);

					state = (char*)PIN_FLIST_FLD_GET(nameinfo_billing, PIN_FLD_STATE, 1, ebufp);
					if (strcmp(state, "DELHI") == 0 )
					{
					
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "ET Plan can be added only in main CATV for Delhi", ebufp);
						PIN_ERRBUF_RESET(ebufp);
						*ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flist, PIN_FLD_SERVICE_OBJ, *ret_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &failure, ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "00000", ebufp);
						PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, "ET Plan can be added only in main CATV for Delhi", ebufp);
					}
					PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
				}
			}
		}
	}

	CLEANUP:
	return ;
}


/**************************************************
* Function: fm_mso_get_bill_segment()
* 
*
* 
***************************************************/
void
fm_mso_get_bill_segment(
	pcm_context_t		*ctxp,
	char			*state_code,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*bill_segment_array = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*cfg_pdp = NULL;

	int32			pay_type = PIN_PAY_TYPE_SUBORD;
	int32			srch_flag = 768;

	char			*template = "select X from /config where F1.type = V1 ";
	char			*descr = NULL;
	char			*field_1 = NULL;

	int64			db = 1;
	int32			billing_segment = 0;
	int32			elem_id =0;

	pin_cookie_t		cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_bill_segment", ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_segment: result flist", *r_flistp);

	/**********************************************************************
	* Search for  /config/billing_segment
	**********************************************************************/
	//db = PIN_POID_GET_DB(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp));
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	cfg_pdp = PIN_POID_CREATE(db, "/config/billing_segment", -1, ebufp);
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, cfg_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_segment SEARCH input flist", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH: fm_mso_get_bill_segment()", ebufp);
		goto CLEANUP;;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_segment SEARCH output flist", srch_out_flistp);
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (result_flist)
	{
		while((bill_segment_array = PIN_FLIST_ELEM_GET_NEXT(result_flist, PIN_FLD_BILLING_SEGMENTS,
		    &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bill_segment_array", bill_segment_array);
			descr = (char*)PIN_FLIST_FLD_GET(bill_segment_array, PIN_FLD_DESCR, 1, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"descr");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,descr);
			if (descr )
			{
				/*field_1 =  strtok(descr,"|");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"city_code");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,city_code);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"field_1");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,field_1);

				if (city_code && field_1 && strcmp(field_1, city_code)==0 )
				{
					billing_segment=elem_id;
					PIN_FLIST_FLD_SET( *r_flistp, PIN_FLD_BILLING_SEGMENT,&billing_segment, ebufp);
					break;
				}*/
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"state_code");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,state_code);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"descr");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,descr);

				if (state_code && descr && strcmp(descr, state_code)==0 )
				{
					billing_segment=elem_id;
					//PIN_FLIST_FLD_SET( *r_flistp, PIN_FLD_BILLING_SEGMENT,&billing_segment, ebufp);
					break;
				}
			}
		}
	}
	if (billing_segment == 0)
	{
		billing_segment = 2000;
	}
	PIN_FLIST_FLD_SET( *r_flistp, PIN_FLD_BILLING_SEGMENT,&billing_segment, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "billing segment output ", *r_flistp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return ;
}

/**************************************************
* Function: 	fm_mso_get_acnt_from_acntno()
* Owner:        Gautam Adak
* Decription:   
*               
*		
* 
* 
***************************************************/
void
fm_mso_get_acnt_from_acntno(
	pcm_context_t		*ctxp,
	char*			acnt_no,
	pin_flist_t		**acnt_details,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*nameinfo = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*exmp_arr = NULL;
	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int32			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	int64			db = 1;
	
	char			*template = "select X from /account  where (F1 = V1 or F2 = V2) " ;
	char			*template1 = "select X from /mso_customer_credential  where F1 = V1 " ;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_acnt_from_acntno", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_acntno :");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, acnt_no);

	//db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, acnt_no, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_LEGACY_ACCOUNT_NO, acnt_no, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );

	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BUSINESS_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CREATED_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_MOD_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_RMN, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_CONTACT_PREF, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_LASTSTAT_CMNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, MSO_FLD_LEGACY_ACCOUNT_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCESS_CODE1, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCESS_CODE2, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_AAC_PACKAGE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_AAC_PROMO_CODE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_AAC_VENDOR, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_TAG, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_AAC_SOURCE, NULL, ebufp);

	nameinfo = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);
	exmp_arr = PIN_FLIST_ELEM_ADD(result_flist, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_acntno search billinfo input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_acntno search billinfo output list", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	if (result_flist)
	{
		*acnt_details = PIN_FLIST_COPY(result_flist,ebufp );
		PIN_FLIST_DESTROY_EX(&result_flist, NULL);

	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account number not found");
		goto CLEANUP;
	}
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	/*Added below to add the custom address fields*/
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(*acnt_details, PIN_FLD_POID, arg_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acnt_from_acntno: search input", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	while((arg_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flist, PIN_FLD_NAMEINFO,
	&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		nameinfo_flistp = PIN_FLIST_ELEM_GET(*acnt_details, PIN_FLD_NAMEINFO, elem_id, 1, ebufp );
		PIN_FLIST_CONCAT(nameinfo_flistp, arg_flistp, ebufp);
	}
CLEANUP:
	PIN_FLIST_DESTROY_EX(&result_flist, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	
	return;
}

/*******************************************************************
 * fm_pymt_pol_pymt_priority_search():
 *
 *      Search for the /mso_cfg_pymt_priority objects in the database
 *
 *******************************************************************/

void
fm_pymt_pol_pymt_priority_search(
        pcm_context_t   *ctxp,
        poid_t          *a_pdp,
        pin_flist_t     **o_flistpp,
        pin_errbuf_t    *ebufp) {

        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *flistp = NULL;
        poid_t          *pdp = NULL;
        char            *templ = NULL;
        int64           db = PIN_POID_GET_DB(a_pdp);
        int32           s_flags = SRCH_DISTINCT;

        /*
         * Create the search flist
         */
        s_flistp = PIN_FLIST_CREATE(ebufp);

	pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

        templ = "select X from /mso_cfg_pymt_priority where F1 = V1 ";
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, templ, ebufp);

        pdp = PIN_POID_CREATE(db, "/mso_cfg_pymt_priority", -1, ebufp);
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);

        /*********************************************************
        * Return everything
        *********************************************************/
        PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);

        /*
         * Clean up.
        */
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_pymt_pol_pymt_priority_search error", ebufp);
        }

        return;
}

/*******************************************************************
 *fm_pymt_pol_get_item_by_charge_head():
 *
 *     Get the item type for the given charge head 
 *
 *******************************************************************/

void
fm_pymt_pol_get_item_by_charge_head(
	cm_nap_connection_t     *connp,
        char          		*header,
	char			**item_tag,
        pin_errbuf_t    	*ebufp) {

	poid_t                  *pdp = NULL;
        int32                   cfg_rec_id = 0;
        int32                   i_rec_id = 0;
        char                    *cfg_item_type = NULL;
        char                    *charge_head = NULL;
        pin_cookie_t            i_cookie = NULL;
        pin_cookie_t            cfg_cookie = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *cfg_flistp = NULL;
        pin_flist_t             *c_flistp = NULL;
        pin_flist_t             *item_flistp = NULL;
	pcm_context_t           *ctxp = connp->dm_ctx;

	*item_tag = NULL;
	pdp = PCM_GET_USERID(ctxp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)header);

	fm_pymt_pol_pymt_priority_search(ctxp, pdp, &res_flistp, ebufp);
        cfg_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_pymt_pol_get_item_by_charge_head flist", cfg_flistp);

        if(!cfg_flistp) {
                goto cleanup;
        }

	cfg_cookie = NULL;
        while ((c_flistp = PIN_FLIST_ELEM_GET_NEXT(cfg_flistp, PIN_FLD_PAY_ORDER_INFO,
                &cfg_rec_id, 1, &cfg_cookie, ebufp)) != (pin_flist_t *)NULL) {

                i_cookie = NULL;
                while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp, PIN_FLD_ITEM_TYPES,
                        &i_rec_id, 1, &i_cookie, ebufp)) != (pin_flist_t *)NULL) {
			charge_head = (char *)PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_HEADER_STR,
				0, ebufp);
			if (header && charge_head &&
				( strcmp( header, charge_head ) == 0 ) ) {
                       		cfg_item_type = (char *)PIN_FLIST_FLD_GET(item_flistp, 
					PIN_FLD_ITEM_TYPE, 0, ebufp);
				if(cfg_item_type) {
					*item_tag = fm_pymt_pol_get_config_item_tag(
						ctxp, pdp, cfg_item_type, ebufp); 
					goto cleanup;
				}
			}
		}			
	}
	cleanup:
		if (*item_tag) PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)*item_tag);
        	PIN_FLIST_DESTROY(res_flistp, NULL);
        	return;
}

/*******************************************************************
 * fm_pymt_pol_get_config_item_tag():
 *
 *      Find ITEM TAG for the corresponing item type 
 *       from /config/item_types object
 *
 *******************************************************************/

char *
fm_pymt_pol_get_config_item_tag(
        pcm_context_t   *ctxp,
        poid_t          *a_pdp,
	char		*i_type,
        pin_errbuf_t    *ebufp) {

        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
	pin_flist_t     *item_flistp = NULL;
	pin_flist_t     *cfg_flistp = NULL;
	pin_flist_t     *flistp = NULL;
        poid_t          *pdp = NULL;
        char            *templ = NULL;
        int64           db = PIN_POID_GET_DB(a_pdp);
        int32           s_flags = SRCH_DISTINCT;
	int32           i_rec_id = 0;
        pin_cookie_t   	i_cookie = NULL;
	char            *item_type = NULL;
	char		*item_tag = NULL;


	/*
         * Create the search flist
         */
        s_flistp = PIN_FLIST_CREATE(ebufp);

        pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

        templ = "select X from /config where F1 = V1 ";
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, templ, ebufp);

        pdp = PIN_POID_CREATE(db, "/config/item_types", -1, ebufp);
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);

        /*********************************************************
        * Return everything
        *********************************************************/
        PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_pymt_pol_get_config_item_tag error", ebufp);
		goto cleanup;	
        }

	cfg_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 
			PIN_ELEMID_ANY, 1, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_pymt_pol_get_config_item_tag flist", cfg_flistp);

        if(!cfg_flistp) {
                goto cleanup;
        }

	i_cookie = NULL;
        while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(cfg_flistp, PIN_FLD_ITEM_TYPES,
        	&i_rec_id, 1, &i_cookie, ebufp)) != (pin_flist_t *)NULL) {
                        item_type = (char *)PIN_FLIST_FLD_GET(item_flistp, 
				PIN_FLD_ITEM_TYPE, 0, ebufp);
		if (item_type && i_type &&
                                ( strcmp( item_type, i_type ) == 0 ) ) {
                                 item_tag = strdup ((char *)PIN_FLIST_FLD_GET(item_flistp,
                                        PIN_FLD_ITEM_TAG, 0, ebufp));
		}
	}


	/*
        * Clean up & Return
        */

	cleanup:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

        	return item_tag;
} 

void fm_mso_utils_open_context(
	pin_flist_t    		*i_flistp,
	pcm_context_t           *ctxp,
	pcm_context_t  		**new_ctxp,
        pin_errbuf_t            *ebufp)
{
	int32		in_login_type 	= 1;
	pin_flist_t    	*in_flistp 	= NULL;
	pin_flist_t    	*s_flistp 	= NULL;
	pin_flist_t    	*r_flistp 	= NULL;
	int64           db_id 		= 0;
	poid_t		*s_pdp		= NULL;
	poid_t		*user_pdp	= NULL;
	char		*login		= NULL;
        

	/***********************************************************
        * Find the service from input.
        ***********************************************************/
	user_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	login = (char *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 0, ebufp);
        db_id = PIN_POID_GET_DB(user_pdp);
        s_pdp = PIN_POID_CREATE(db_id, "/service/admin_client", -1, ebufp);

	s_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_LOGIN, login, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_act_find input flist: ", s_flistp);

        PCM_OP(ctxp, PCM_OP_ACT_FIND, 0, s_flistp, &r_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_act_find result flist: ", r_flistp);

	if(PIN_ERRBUF_IS_ERR(ebufp)) {
         
                PIN_FLIST_LOG_ERR("fm_mso_utils_open_context ACT_FIND err",
                                ebufp);
		goto cleanup;                   
        }
	s_pdp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 0, ebufp);

	/***********************************************************
        * open the context.
        ***********************************************************/
        
	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TYPE, (void *)&in_login_type, ebufp);

        PCM_CONTEXT_OPEN(new_ctxp, in_flistp, ebufp);
	
 	if(PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_FLIST_LOG_ERR("fm_mso_utils_open_context err",
                                ebufp);
                   
        }

cleanup:
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	return;
}

/**************************************************************************
* Function: 	fm_mso_utils_get_billinfo_bal_details()
* Owner:        Hrish Kulkarni
* Decription:   For getting /balance_group and /billinfo poids for the 
*		given billinfo_id("CATV" OR " "BB").
**************************************************************************/

void fm_mso_utils_get_billinfo_bal_details(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	char			*billinfo_id,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*lnkobj_flistp = NULL;
	pin_flist_t		*exra_rslt_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32			flags = 256;
	int64			db = 0;
	int64			id = -1;
	int32			lnk_dir = -1;
	poid_t			*serv_pdp = NULL;
	char			*s_template = "select X from /service 1, /balance_group 2 "
					"where 1.F1 = V1 and 1.F2 = 2.F3 and 1.F4 = V4 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_utils_get_billinfo_bal_details", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_utils_get_billinfo_bal_details");
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Account poid is: ", acct_pdp);
	db = PIN_POID_GET_DB(acct_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG1");

	/*******************************************************
	Input Flist:
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS           INT [0] 256
	0 PIN_FLD_TEMPLATE        STR [0] " select X from /service 1, /balance_group 2 	
					where 1.F1 = V1 and 1.F2 = 2.F3 and 1.F4 = V4 "
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	1     PIN_FLD_POID    POID [0] 0.0.0.1 /service/telco/broadband -1 0
	0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
	1     PIN_FLD_BAL_GRP_OBJ    POID [0] NULL
	0 PIN_FLD_ARGS          ARRAY [3] allocated 20, used 1
	1     PIN_FLD_POID    POID [0] NULL
	0 PIN_FLD_ARGS          ARRAY [4] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 2332059
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 0
	1 PIN_FLD_POID   POID [0] NULL
	1 PIN_FLD_BAL_GRP_OBJ POID [0] NULL
	1 PIN_FLD_LINKED_OBJ ARRAY [2] allocated 3, used 3
	2 PIN_FLD_LINK_DIRECTION ENUM [0] -1
	2 PIN_FLD_BAL_GRP_OBJ POID [0] NULL
	2 PIN_FLD_EXTRA_RESULTS ARRAY [*] allocated 1, used 1
	3 PIN_FLD_BILLINFO_OBJ POID [0] NULL

	Output Flist:
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 3
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /service/telco/broadband 2338826 15
	1     PIN_FLD_BAL_GRP_OBJ    POID [0] 0.0.0.1 /balance_group 2342898 0
	1     PIN_FLD_LINKED_OBJS   ARRAY [2] allocated 20, used 1
	2         PIN_FLD_LINKED_OBJ    ARRAY [0] allocated 20, used 1
	3             PIN_FLD_POID           POID [0] 0.0.0.1 /balance_group 2342898 7
	0 PIN_FLD_EXTRA_RESULTS  ARRAY [1] allocated 20, used 2
	1     PIN_FLD_BILLINFO_OBJ   POID [0] 0.0.0.1 /billinfo 2333595 0
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /balance_group 2342898 7
	*******************************************************/
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG2");
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	if(billinfo_id && (strcmp(billinfo_id, BILLINFOID_BB) == 0))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG3");
		serv_pdp= PIN_POID_CREATE(db, "/service/telco/broadband", id, ebufp);
		PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, serv_pdp, ebufp);
	}
	else if(billinfo_id && (strcmp(billinfo_id, BILLINFOID_CATV) == 0))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG4");
		serv_pdp= PIN_POID_CREATE(db, "/service/catv", id, ebufp);
		PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, serv_pdp, ebufp);
	}
	else if(billinfo_id && (strcmp(billinfo_id, BILLINFOID_WS) == 0))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG5");
		serv_pdp= PIN_POID_CREATE(db, "/service/settlement/ws/incollect", id, ebufp);
		PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, serv_pdp, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG6");
		goto CLEANUP;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG7");
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	lnkobj_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_LINKED_OBJ, 2, ebufp);
	PIN_FLIST_FLD_SET(lnkobj_flistp, PIN_FLD_LINK_DIRECTION, &lnk_dir, ebufp);
	PIN_FLIST_FLD_SET(lnkobj_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	exra_rslt_flistp = PIN_FLIST_ELEM_ADD(lnkobj_flistp, PIN_FLD_EXTRA_RESULTS, 
					PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(exra_rslt_flistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Billinfo Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Billinfo Output flist:",*o_flistpp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG8");
	return;
}

void fm_mso_utils_get_billinfo_bal_details_woserv(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	char			*billinfo_id,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	int32			flags = 256;
	int64			db = 0;
	int64			id = -1;
	int32			lnk_dir = -1;
	poid_t			*s_pdp = NULL;
	char			*s_template = "select X from /balance_group where F1 = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_utils_get_billinfo_bal_details", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_utils_get_billinfo_bal_details_woserv");
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Account poid is: ", acct_pdp);
	db = PIN_POID_GET_DB(acct_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG1");

	/*******************************************************
	Input Flist:
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS           INT [0] 256
	0 PIN_FLD_TEMPLATE        STR [0] "select X from /balance_group where F1 = V1 "
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 4167510542 0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 3

	Output Flist:
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 12
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /balance_group 4167510798 0
	1     PIN_FLD_CREATED_T    TSTAMP [0] (1458211246) Thu Mar 17 16:10:46 2016
	1     PIN_FLD_MOD_T        TSTAMP [0] (1458211246) Thu Mar 17 16:10:46 2016
	1     PIN_FLD_READ_ACCESS     STR [0] "L"
	1     PIN_FLD_WRITE_ACCESS    STR [0] "L"
	1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 4167510542 0
	1     PIN_FLD_BATCH_CNTR      INT [0] 0
	1     PIN_FLD_BILLINFO_OBJ   POID [0] 0.0.0.1 /billinfo 4167508238 0
	1     PIN_FLD_EFFECTIVE_T  TSTAMP [0] (1458211246) Thu Mar 17 16:10:46 2016
	1     PIN_FLD_NAME            STR [0] "Default Balance Group"
	1     PIN_FLD_OBJECT_CACHE_TYPE   ENUM [0] 0
	1     PIN_FLD_REALTIME_CNTR    INT [0] 0
	
	*******************************************************/
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Billinfo Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Billinfo Output flist:",*o_flistpp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG8");
	return;
}


/**************************************************************************
* Function: 	fm_mso_utils_balance_transfer()
* Owner:        Hrish Kulkarni
* Decription:   For transfering balance from source to destination account.
**************************************************************************/
void fm_mso_utils_balance_transfer(
	pcm_context_t		*ctxp,
	poid_t			*s_acct_pdp,
	poid_t			*d_acct_pdp,
	poid_t			*s_bal_pdp,
	poid_t			*d_bal_pdp,
	pin_decimal_t		*amount,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*tb_iflistp = NULL;
	pin_flist_t		*tb_oflistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	int32			verify_bal = 0;
	int32			currency = 356;
	char			*program_name = "PYMT_COLLECT_LCO_TRANSFER";
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error before calling fm_mso_utils_balance_transfer", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_utils_balance_transfer");

	tb_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(tb_iflistp, PIN_FLD_POID, s_acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(tb_iflistp, PIN_FLD_TRANSFER_TARGET, d_acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(tb_iflistp, PIN_FLD_AMOUNT, amount, ebufp);
	PIN_FLIST_FLD_SET(tb_iflistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	PIN_FLIST_FLD_SET(tb_iflistp, PIN_FLD_CURRENCY, &currency, ebufp);
	PIN_FLIST_FLD_SET(tb_iflistp, PIN_FLD_VERIFY_BALANCE, &verify_bal, ebufp);
	
	if(s_bal_pdp)
	{
		bal_flistp = PIN_FLIST_ELEM_ADD(tb_iflistp, PIN_FLD_BAL_INFO, 0, ebufp);
		PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_BAL_GRP_OBJ, s_bal_pdp, ebufp);
	}
	if(d_bal_pdp)
	{
		bal_flistp = PIN_FLIST_ELEM_ADD(tb_iflistp, PIN_FLD_BAL_INFO, 1, ebufp);
		PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_BAL_GRP_OBJ, d_bal_pdp, ebufp);
	}

	/*******************************************************
	PCM_OP_BILL_TRANSFER_BALANCE Input Flist:
	0 PIN_FLD_POID POID [0] 0.0.0.1 /account 12345  --> Source Account
	0 PIN_FLD_TRANSFER_TARGET POID [0] 0.0.0.1 /account 54321  --> Destination Account
	0 PIN_FLD_AMOUNT DECIMAL [0] 30.00
	0 PIN_FLD_PROGRAM_NAME STR [0] "PYMT_COLLECT_LCO_TRANSFER"
	0 PIN_FLD_CURRENCY INT [0] 356
	0 PIN_FLD_VERIFY_BALANCE ENUM [0] 0 --> Transfer Balance even if it goes positive
	0 PIN_FLD_BAL_INFO ARRAY [0]  --> Source Bal Group
	1   PIN_FLD_BAL_GRP_OBJ POID [0] 0.0.0.1 /balance_group 68767
	0 PIN_FLD_BAL_INFO ARRAY [1]  --> Desination Bal Group
	1   PIN_FLD_BAL_GRP_OBJ POID [0] 0.0.0.1 /balance_group 34345
	*******************************************************/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BILL_TRANSFER_BALANCE Input flist:",tb_iflistp);
	PCM_OP(ctxp, PCM_OP_BILL_TRANSFER_BALANCE, 0, tb_iflistp, &tb_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BILL_TRANSFER_BALANCE Output flist:",tb_oflistp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "fm_mso_utils_balance_transfer err calling PCM_OP_BILL_TRANSFER_BALANCE",
                ebufp);
                goto CLEANUP;
        }

	*ret_flistpp = PIN_FLIST_COPY(tb_oflistp, ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&tb_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&tb_oflistp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_utils_read_any_object()
* Owner:        Hrish Kulkarni
* Decription:   For reading the class object for poid.
***************************************************/

void
fm_mso_utils_read_any_object(
	pcm_context_t		*ctxp,
	poid_t			*objp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_obj_input = NULL;
	pin_flist_t		*read_obj_output = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
		"Error before calling fm_mso_utils_read_any_object", ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_read_any_object input flist");

	
	read_obj_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_obj_input, PIN_FLD_POID, objp, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_utils_read_any_object read input list", read_obj_input);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_obj_input, &read_obj_output, ebufp);
	PIN_FLIST_DESTROY_EX(&read_obj_input, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in calling PCM_OP_READ_OBJ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_utils_read_any_object READ output list", read_obj_output);

	if (read_obj_output)
	{
		*out_flistpp = read_obj_output;
		
	}

	return;
}

void fm_mso_utils_close_context(
	pcm_context_t  		*ctxp,
        pin_errbuf_t            *ebufp)
{
	if(ctxp) {
		PCM_CONTEXT_CLOSE(ctxp, 0, ebufp);
	}
	return;
}

/**************************************************************************
* Function: 	fm_mso_utils_get_service_details()
* Owner:        Hrish Kulkarni
* Decription:   For getting service details of broadband / CATV accounts.
*
**************************************************************************/
void fm_mso_utils_get_service_details(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	int32			serv_type,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*serv_pdp = NULL;
	int32			flags = 256;
	int32			srvc_status_active  = 10100;
 	int32			srvc_status_inactive = 10102;
	int64			db = 0;
	int64			id = -1;
	char			*s_template = " select X from /service where F1 = V1 and F2 = V2 and ( F3 = V3 or F4 = V4) ";
	char                    *s_act_temp = " select X from /service where F1 = V1 and F2 = V2 and  F3 = V3 and F4 != V4 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_utils_get_service_details", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_utils_get_service_details");
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "acct_pdp", acct_pdp );

	db = PIN_POID_GET_DB(acct_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        if (serv_type == 0)
        {
                serv_pdp = PIN_POID_CREATE(db, "/service/catv", id, ebufp);
        }

        if (serv_type == 1)
        {
                serv_pdp = PIN_POID_CREATE(db, "/service/telco/broadband", id, ebufp);
        }
        if (serv_type == 2)
        {
                     PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_act_temp, ebufp);
                     serv_pdp = PIN_POID_CREATE(db, "/service/telco/broadband", id, ebufp);
        }
        else
        {
                     PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
        }

	/*******************************************************
	PCM_OP_SEARCH Input:
	0 PIN_FLD_POID POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_FLAGS INT [0] 256
	0 PIN_FLD_TEMPLATE STR [0] " select X from /service where F1 = V1 and F2 = V2 "
	0 PIN_FLD_ARGS ARRAY [1]
	1   PIN_FLD_ACCOUNT_OBJ POID [0] 0.0.0.1 /account 2321559
	0 PIN_FLD_ARGS ARRAY [2]
	1   PIN_FLD_POID POID [0] 0.0.0.1 /service/telco/broadband -1
	0 PIN_FLD_RESULTS ARRAY [0] NULL
	*******************************************************/
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, serv_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &srvc_status_active, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &srvc_status_inactive, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Output flist:",*o_flistpp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}

/**************************************************************************
* Function: 	fm_mso_utils_get_close_service_details()
* Owner:        Rama Puppala 
* Decription:   For getting closed service details of broadband / CATV accounts.
*
**************************************************************************/
void fm_mso_utils_get_close_service_details(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	int32			serv_type,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*serv_pdp = NULL;
	int32			flags = 256;
	int32			srvc_status_close = 10103;
	int64			db = 0;
	int64			id = -1;
	char			*s_template = " select X from /service where F1 = V1 and F2 = V2 and F3 = V3 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_utils_get_service_details", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_utils_get_service_details");
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "acct_pdp", acct_pdp );

	db = PIN_POID_GET_DB(acct_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	if (serv_type == 1)
	{
		serv_pdp = PIN_POID_CREATE(db, "/service/telco/broadband", id, ebufp);
	}
	if (serv_type == 0)
	{
		serv_pdp = PIN_POID_CREATE(db, "/service/catv", id, ebufp);
	}

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, serv_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &srvc_status_close, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Output flist:",*o_flistpp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}
/**************************************************************************
* Function: 	fm_mso_utils_get_service_from_balgrp()
* Owner:        Hrish Kulkarni
* Decription:   For getting service details from balance group poid.
*
**************************************************************************/
void fm_mso_utils_get_service_from_balgrp(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32			flags = 256;
	int64			db = 0;
	int64			id = -1;
	char			*s_template = " select X from /service where F1 = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_utils_get_service_from_balgrp", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_utils_get_service_from_balgrp");
	db = PIN_POID_GET_DB(balgrp_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	/*******************************************************
	PCM_OP_SEARCH Input:
	0 PIN_FLD_POID POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_FLAGS INT [0] 256
	0 PIN_FLD_TEMPLATE STR [0] " select X from /service where F1 = V1 "
	0 PIN_FLD_ARGS ARRAY [1]
	1   PIN_FLD_BAL_GRP_OBJ POID [0] 0.0.0.1 /balance_group 2321559
	0 PIN_FLD_RESULTS ARRAY [0] NULL
	*******************************************************/
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BAL_GRP_OBJ, balgrp_pdp, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Output flist:",*o_flistpp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}

/**************************************************************************
* Function:     fm_mso_utils_create_pre_rated_impact()
* Owner:        Hrish Kulkarni
* Decription:   For generating the Pre-Rated event.
*
**************************************************************************/

void fm_mso_utils_create_pre_rated_impact(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	poid_t			*service_pdp,
	char			*program_name,
	char			*sys_description,
	char			*event_type,
	pin_decimal_t		*amount,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*input_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*event_flistp = NULL;
	pin_flist_t		*total_flistp = NULL;
	int64			db = 1;
	time_t			pvt = pin_virtual_time(NULL);
	int			currency = PIN_CURRENCY_INR;
	poid_t			*event_pdp = NULL;
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
        poid_t			*search_pdp = NULL;
	poid_t			*billinfo_obj = NULL;
        poid_t			*pdp = NULL;
        int32			search_flags = 512;
	pin_flist_t		*result_flistp = NULL;
	char			*template = "select X from /balance_group 1,/service 2 where 2.F1 = V1 and 2.F2 = 1.F3 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"Error before calling fm_mso_utils_create_pre_rated_impact", ebufp);
		return;
	}

	// search billinfo
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp,PIN_FLD_POID,PIN_POID_CREATE(db, "/search", -1, ebufp),ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp,PIN_FLD_FLAGS,&search_flags,ebufp);
        PIN_FLIST_FLD_SET(srch_iflistp,PIN_FLD_TEMPLATE,template,ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp,PIN_FLD_ARGS,1,ebufp);
        PIN_FLIST_FLD_SET(arg_flistp,PIN_FLD_POID,service_pdp, ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp,PIN_FLD_ARGS,2,ebufp);
        PIN_FLIST_FLD_SET(arg_flistp,PIN_FLD_BAL_GRP_OBJ,NULL, ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp,PIN_FLD_ARGS,3,ebufp);
        PIN_FLIST_FLD_SET(arg_flistp,PIN_FLD_POID,NULL, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp,PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Billinfo Search input flist",srch_iflistp);
        PCM_OP(ctxp,PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, " Billinfo Search output flist",srch_oflistp);

	result_flistp = PIN_FLIST_ELEM_GET(srch_oflistp,PIN_FLD_RESULTS, PIN_ELEMID_ANY,0, ebufp);
	if(result_flistp)
	{
		billinfo_obj = PIN_FLIST_FLD_GET(result_flistp,PIN_FLD_BILLINFO_OBJ,0, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"Error before calling fm_mso_utils_create_pre_rated_impact", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
		"Inside function fm_mso_utils_create_pre_rated_impact");
	input_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_POID, acct_pdp, ebufp);
	event_flistp = PIN_FLIST_SUBSTR_ADD(input_flistp, PIN_FLD_EVENT, ebufp);




	db = PIN_POID_GET_DB(acct_pdp);
	event_pdp = PIN_POID_CREATE(db, event_type, (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(event_flistp, PIN_FLD_POID, event_pdp, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_obj, ebufp);
	
	if(strstr(event_type,EVENT_MSO_REFUND_CREDIT))
	{
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_NAME, "MSO_REFUND_CREDIT", ebufp);
	}
	else if (strstr(event_type,EVENT_MSO_REFUND_DEBIT))
	{
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_NAME, "MSO_REFUND_DEBIT", ebufp);
	}
	else if (strstr(event_type,CHK_BOUNCE_EVENT))
	{
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_NAME, "CHK_BOUNCE_EVENT", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_NAME, "UNDEFINED", ebufp);	
	}
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_DESCR, sys_description, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_SYS_DESCR, sys_description, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_START_T, &pvt, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_END_T, &pvt, ebufp);
	if (service_pdp!=NULL)
	{
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
	}

	total_flistp=PIN_FLIST_ELEM_ADD(event_flistp, PIN_FLD_TOTAL, currency, ebufp);
	PIN_FLIST_FLD_SET(total_flistp,PIN_FLD_AMOUNT,amount,ebufp)

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_utils_create_pre_rated_impact prepared flist for PCM_OP_ACT_USAGE", 
		input_flistp);

	PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, input_flistp, &res_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_mso_utils_create_pre_rated_impact err calling PCM_OP_ACT_USAGE error",
		ebufp);
		goto CLEANUP;
	}
	
	*r_flistpp = PIN_FLIST_COPY(res_flistp, ebufp);
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&input_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
	return;
}

/*******************************************************************************
 * fm_mso_util_get_customer_lco_info()
 *
 * Description:
 * Search LCO account poid from customer /account object
   also searches the commission model
 ******************************************************************************/
void
fm_mso_util_get_customer_lco_info(
    pcm_context_t           *ctxp,
    poid_t                  *cust_pdp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    poid_t                  *dtr_objp = NULL;
    poid_t                  *sdtr_objp = NULL;
    char                    *template =  NULL;
    int32                   s_flags = 0;
    int64                   database = 0;
    pin_flist_t             *tmp1_flistp= NULL;

    pin_flist_t             *flistp0=NULL;
    pin_flist_t             *flistp1=NULL;
    pin_flist_t             *r_flistp=NULL;
    char                    *tmp_str0="select X from /profile/wholesale 1, /profile/customer_care 2 where 2.F2.id = V2 and 2.F1.id = 1.F3.id and 1.F4.type = V4 and 2.F5.type = V5 ";
    u_int                   tmp_enum1=0;
    int32                   *comm_model=NULL;
    int32                   *comm_serv=NULL;
    void                    *vp=NULL;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    database = PIN_POID_GET_DB(cust_pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);


    flistp0 = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    flistp1 = PIN_FLIST_SUBSTR_ADD(flistp0, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
    PIN_FLIST_FLD_SET(flistp1,MSO_FLD_LCO_OBJ, NULL, ebufp);
    flistp0 = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(flistp0,PIN_FLD_ACCOUNT_OBJ, cust_pdp, ebufp);
    flistp0 = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET(flistp0,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

    flistp0 = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 4, ebufp );
    PIN_FLIST_FLD_PUT(flistp0, PIN_FLD_POID, PIN_POID_CREATE(database, "/profile/wholesale", -1, ebufp), ebufp);

    flistp0 = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 5, ebufp );
    PIN_FLIST_FLD_PUT(flistp0, PIN_FLD_POID, PIN_POID_CREATE(database, "/profile/customer_care", -1, ebufp), ebufp);
    PIN_FLIST_FLD_SET(search_i_flistp,PIN_FLD_TEMPLATE, tmp_str0, ebufp);
    flistp0 = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET(flistp0,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
    flistp1 = PIN_FLIST_SUBSTR_ADD(flistp0, MSO_FLD_WHOLESALE_INFO, ebufp);
    PIN_FLIST_FLD_SET(flistp1,MSO_FLD_COMMISION_MODEL, (void*)&tmp_enum1, ebufp);
    PIN_FLIST_FLD_SET(flistp1,MSO_FLD_COMMISION_SERVICE, (void*)&tmp_enum1, ebufp);
    PIN_FLIST_FLD_SET(flistp1,MSO_FLD_DT_OBJ, NULL, ebufp);
    PIN_FLIST_FLD_SET(flistp1,MSO_FLD_SDT_OBJ, NULL, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_util_get_customer_lco_info search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp, &r_flistp, ebufp);

    flistp0 = PIN_FLIST_SUBSTR_GET(r_flistp, PIN_FLD_RESULTS, 0, ebufp);
    flistp1 = PIN_FLIST_SUBSTR_GET(flistp0, MSO_FLD_WHOLESALE_INFO, 0, ebufp);
    *r_flistpp = PIN_FLIST_CREATE(ebufp);
    vp = PIN_FLIST_FLD_GET(flistp0, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, vp, ebufp);
    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
    comm_model = (int32*)PIN_FLIST_FLD_GET(flistp1, MSO_FLD_COMMISION_MODEL, 0, ebufp);
    PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_COMMISION_MODEL, (int32*)comm_model, ebufp);
    comm_serv = (int32*)PIN_FLIST_FLD_GET(flistp1, MSO_FLD_COMMISION_SERVICE, 0, ebufp);
    PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_COMMISION_SERVICE, (int32*)comm_serv, ebufp);
    dtr_objp = PIN_FLIST_FLD_GET(flistp1, MSO_FLD_DT_OBJ, 0, ebufp);
    sdtr_objp = PIN_FLIST_FLD_GET(flistp1, MSO_FLD_SDT_OBJ, 0, ebufp);
    PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_DT_OBJ, dtr_objp, ebufp);
    PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_SDT_OBJ, sdtr_objp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_util_get_customer_lco_info search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

    return;
}

/*******************************************************************************
 * fm_mso_util_get_lco_settlement_services()
 *
 * Description:
 * Search LCO settlement service poids from lco /account object
 ******************************************************************************/
void
fm_mso_util_get_lco_settlement_services(
    pcm_context_t           *ctxp,
    poid_t                  *cust_pdp,
    pin_flist_t             **r_flistpp,
    pin_errbuf_t            *ebufp)
{
    pin_flist_t             *search_i_flistp = NULL;
    pin_flist_t             *tmp_flistp = NULL;
    poid_t                  *s_pdp = NULL;
    poid_t                  *pdp = NULL;
    char                    *template =  NULL;
    int32                   s_flags = 0;
    int64                   database = 0;
    pin_flist_t             *tmp1_flistp= NULL;

    pin_flist_t             *flistp0=NULL;
    pin_flist_t             *flistp1=NULL;
    pin_flist_t             *r_flistp=NULL;
    char                    *tmp_str0="select X from /service where F1 = V1 and status = 10100 and poid_type in ('/service/commission') ";
    u_int                   tmp_enum1=0;
    int32                   *comm_model=NULL;
    void                    *vp=NULL;


    if (PIN_ERRBUF_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR(ebufp);

    database = PIN_POID_GET_DB(cust_pdp); 

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);


    flistp0 = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    flistp1 = PIN_FLIST_SUBSTR_ADD(flistp0, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
    PIN_FLIST_FLD_SET(flistp1,MSO_FLD_LCO_OBJ, NULL, ebufp);
    flistp0 = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(flistp0,PIN_FLD_ACCOUNT_OBJ, cust_pdp, ebufp);
    flistp0 = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET(flistp0,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
    PIN_FLIST_FLD_SET(search_i_flistp,PIN_FLD_TEMPLATE, tmp_str0, ebufp);
    flistp0 = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET(flistp0,PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
    flistp1 = PIN_FLIST_SUBSTR_ADD(flistp0, MSO_FLD_WHOLESALE_INFO, ebufp);
    PIN_FLIST_FLD_SET(flistp1,MSO_FLD_COMMISSION_MODEL, (void*)&tmp_enum1, ebufp);

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_util_get_customer_lco_info search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_i_flistp, &r_flistp, ebufp);

    flistp0 = PIN_FLIST_SUBSTR_GET(r_flistp, PIN_FLD_RESULTS, 0, ebufp);
    flistp1 = PIN_FLIST_SUBSTR_GET(flistp0, MSO_FLD_WHOLESALE_INFO, 0, ebufp);
    *r_flistpp = PIN_FLIST_CREATE(ebufp);
    vp = PIN_FLIST_FLD_GET(flistp0, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
    comm_model = (int32*)PIN_FLIST_FLD_GET(flistp1, MSO_FLD_COMMISSION_MODEL, 0, ebufp);
    PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_COMMISSION_MODEL, (int32*)comm_model, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_util_get_customer_lco_info search output flist", *r_flistpp);

    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

    return;
}


/*******************************************************************************
 * fm_mso_utils_read_object()
 *
 * Description:
 * To read and return the required fields from any object passing as input
 ******************************************************************************/

void
fm_mso_utils_read_object (
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *objp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

        pin_flist_t             *tmp_flistp=NULL;
        pin_flist_t             *r_flistp=NULL;
        pin_flist_t             *elem_flistp=NULL;
        void                    *vp=NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        tmp_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, objp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_read_object input flist", tmp_flistp);

        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, tmp_flistp, &r_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_read_object search output flist", r_flistp);
        switch (flags) {
           case PURCHASED_PRODUCT_DATA:
                vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PLAN_OBJ, vp, ebufp);
                *r_flistpp = tmp_flistp;
                break;
           case PLAN_OBJECT:
           case DEAL_OBJECT:
           case PRODUCT_OBJECT:
                vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_NAME, 0, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, vp, ebufp);
                *r_flistpp = tmp_flistp;
                break;
           case ACCOUNT_OBJECT:
                vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_NO, vp, ebufp);
                vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_BUSINESS_TYPE, vp, ebufp);
                *r_flistpp = tmp_flistp;
                break;
           case EVENT_OBJECT:
                vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_SESSION_OBJ, 0, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SESSION_OBJ, vp, ebufp);
                *r_flistpp = tmp_flistp;
                break;
           case ITEM_OBJECT:
                vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, vp, ebufp);
                vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_ITEM_TOTAL, 0, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ITEM_TOTAL, vp, ebufp);
                vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_DUE, 0, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DUE, vp, ebufp);
                vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_EFFECTIVE_T, vp, ebufp);
                vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
                *r_flistpp = tmp_flistp;
                break;
	   case BB_SERVICE_OBJECT:
                elem_flistp = PIN_FLIST_ELEM_GET(r_flistp, MSO_FLD_BB_INFO, 0, 0, ebufp);
                PIN_FLIST_FLD_COPY(elem_flistp, MSO_FLD_AGREEMENT_NO, tmp_flistp, MSO_FLD_AGREEMENT_NO, ebufp);
                *r_flistpp = tmp_flistp;
		break;
	   case CATV_SERVICE_OBJECT:
                elem_flistp = PIN_FLIST_ELEM_GET(r_flistp, MSO_FLD_CATV_INFO, 0, 0, ebufp);
                PIN_FLIST_FLD_COPY(elem_flistp, MSO_FLD_AGREEMENT_NO, tmp_flistp, MSO_FLD_AGREEMENT_NO, ebufp);
                *r_flistpp = tmp_flistp;
		break;
           default:
                PIN_FLIST_DESTROY_EX(&tmp_flistp, NULL);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_read_object output flist", *r_flistpp);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;

}
/* utils get LCO bal_grp */

void fm_mso_utils_get_bal_grp
(
    pcm_context_t       *ctxp,
    poid_t              *act_pdp,
    char		*flags,
    poid_t              **ret_bal_grp_poidp,
    poid_t              **ret_serv_poidp,
    pin_errbuf_t        *ebufp)
{
	pin_flist_t	 *srch_i_flistp = NULL;
	pin_flist_t	 *srch_o_flistp = NULL;
	pin_flist_t	 *srch_res_flistp = NULL;
	pin_flist_t	 *results_flistp = NULL;
	pin_flist_t   *args_flistp = NULL;
	char		template[] = "select X from /service where F1 = V1 and F2.type like V2"; 
	int		database = 0;
	poid_t		*srch_pdp = NULL;
	poid_t		*service_pdp = NULL;
	int			srch_flags = 256;
	  
	//if(strcmp(PIN_POID_GET_TYPE(act_pdp),"/account")==0) {
	if(act_pdp && strcmp(PIN_POID_GET_TYPE(act_pdp),"/account")==0) {
		service_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(PCM_GET_USERID(ctxp)),flags,-1,ebufp);
		srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(PCM_GET_USERID(ctxp)),"/search",-1,ebufp);
		  
		srch_i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch_i_flistp,PIN_FLD_POID,(void *)srch_pdp,ebufp);
		PIN_FLIST_FLD_SET(srch_i_flistp,PIN_FLD_FLAGS,(void *)&srch_flags,ebufp);
		PIN_FLIST_FLD_SET(srch_i_flistp,PIN_FLD_TEMPLATE,(void *)template,ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp,PIN_FLD_ARGS,1,ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,(void *)act_pdp,ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp,PIN_FLD_ARGS,2,ebufp);
		PIN_FLIST_FLD_PUT(args_flistp,PIN_FLD_POID,(void *)service_pdp,ebufp);
		srch_res_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp,PIN_FLD_RESULTS,0,ebufp);
		PIN_FLIST_FLD_SET(srch_res_flistp,PIN_FLD_BAL_GRP_OBJ,NULL,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_bal_grp search input flist", srch_i_flistp);
		
		PCM_OP(ctxp,PCM_OP_SEARCH,0,srch_i_flistp,&srch_o_flistp,ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error during search",ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_bal_grp search input flist", srch_o_flistp);
		results_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp,PIN_FLD_RESULTS,0,1,ebufp);
		if(results_flistp != NULL) {
			*ret_bal_grp_poidp = PIN_FLIST_FLD_TAKE(results_flistp,PIN_FLD_BAL_GRP_OBJ,0,ebufp);
			*ret_serv_poidp = PIN_FLIST_FLD_TAKE(results_flistp,PIN_FLD_POID,0,ebufp);
		}
		else {
			*ret_bal_grp_poidp = NULL;
			*ret_serv_poidp = NULL;
		}
	}
	else if (strncmp(PIN_POID_GET_TYPE(act_pdp),"/service",8)==0) {
		*ret_bal_grp_poidp = NULL;
		*ret_serv_poidp = NULL;
	}
	else{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Invalid Poid_type");
	}
cleanup :
	PIN_FLIST_DESTROY_EX(&srch_o_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&srch_i_flistp,ebufp);
	return;
}
	
void fm_mso_utils_calc_tax_share
(
    pcm_context_t       *ctxp,
    pin_flist_t         *i_flistp,
	pin_decimal_t		*lco_share,
	pin_decimal_t		**lco_tax_p,
	pin_decimal_t		**hath_share_p,
    pin_errbuf_t        *ebufp)
	{

		pin_decimal_t		*tax_p = NULL;
		pin_decimal_t		*total_p = NULL;
		pin_decimal_t		*amount_p = NULL;
		pin_decimal_t		*percp = NULL;
		pin_decimal_t		*total_lco_share = NULL;
		void		*vp = NULL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_calc_tax_share input flist", i_flistp);
	vp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_TAX,0,ebufp);
	if(vp) tax_p = pbo_decimal_copy(vp,ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_AMOUNT,0,ebufp);
	if(vp) amount_p = pbo_decimal_copy(vp,ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_TOTALS,0,ebufp);
	if(vp) total_p = pbo_decimal_copy(vp,ebufp);
	percp = pbo_decimal_divide(lco_share,amount_p,ebufp);
	*lco_tax_p = pbo_decimal_multiply(tax_p,percp,ebufp);
	total_lco_share = pbo_decimal_add(*lco_tax_p,lco_share,ebufp);
	*hath_share_p = pbo_decimal_subtract(total_p,total_lco_share,ebufp);

	if (!pbo_decimal_is_null(tax_p, ebufp))
	{
		pbo_decimal_destroy(&tax_p);
	}
	if (!pbo_decimal_is_null(amount_p, ebufp))
	{
		pbo_decimal_destroy(&amount_p);
	}
	if (!pbo_decimal_is_null(total_p, ebufp))
	{
		pbo_decimal_destroy(&total_p);
	}
	if (!pbo_decimal_is_null(percp, ebufp))
	{
		pbo_decimal_destroy(&percp);
	}
	if (!pbo_decimal_is_null(total_lco_share, ebufp))
	{
		pbo_decimal_destroy(&total_lco_share);
	}
	 return;

}





/*******************************************************************
 * Function: fm_mso_utils_prepare_status_flist
 *
 * This function is used to prepare the status flist
 *
 * Arguments:
 *      1. Context Pointer
 *      2. Input Flist
 *      3. Output Flist [Used to return the status flist)
 *      4. flag         [Used to include the field in the status flist]
 *      5. Status Description
 *      6. Error buffer
 *
 * Return value: None
 *******************************************************************/
void
fm_mso_utils_prepare_status_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        int                     flag,
        char                    *status_descr,
        pin_errbuf_t            *ebufp)
{
        int     success=0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_utils_prepare_status_flist input flist",i_flistp);

        *r_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &success, ebufp);
        switch(flag) {
            case PIN_FLD_SERVICE_OBJ:
                 PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *r_flistpp, PIN_FLD_SERVICE_OBJ, ebufp);
                 break;
            default:
                 PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, *r_flistpp, PIN_FLD_ACCOUNT_OBJ, ebufp);
                 break;
        }
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, status_descr, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_utils_prepare_status_flist output flist",*r_flistpp);
}


/*******************************************************************
 * Function: fm_mso_utils_prepare_error_flist
 *
 * This function is used to prepare the error return flist
 *
 * Arguments:
 *      1. Context Pointer
 *      2. Input Flist
 *      3. Output Flist [Used to return the error flist)
 *      4. Error Code
 *      5. Error Description
 *      6. Error buffer
 *
 * Return value: None
 *******************************************************************/

void
fm_mso_utils_prepare_error_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        char                    *error_code,
        char                    *error_descr,
        pin_errbuf_t            *ebufp)
{
        int     failure=1;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_utils_prepare_error_flist input flist",i_flistp);

        *r_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp );
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, error_code, ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_utils_prepare_error_flist output flist",*r_flistpp);
}


/*******************************************************************
 * Function: fm_mso_utils_get_services_info
 *
 * This function is used to retrive the service object of LCO account
 * Where the service object would be returned according to the input.
 *
 * Arguments:
 *      1. Context Pointer
 *      2. Input Flist
 *      3. Output Flist (Used to return service object)
 *      4. Service Type (Incollect or Outcollect Service)
 *      5. Error Buffer
 *
 * Return value: None
 *******************************************************************/

void
fm_mso_utils_get_services_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        char                    *ser_type,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;

        int64                   db = -1;
        int32                   srch_flag = 256;

        poid_t                  *srch_pdp = NULL;
        poid_t                  *acnt_pdp = NULL;
        poid_t                  *ser_pdp = NULL;
        char                    *template = "select X from /service where F1.id = V1 and F2.type = V2 ";

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_utils_get_services_info", ebufp);
                return;
        }

        acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(acnt_pdp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        srch_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        ser_pdp = PIN_POID_CREATE(db, ser_type, 1, ebufp);
        PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, ser_pdp, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_services_info Search input flist", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &result_flist, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_services_info Search output flist", result_flist);
        *r_flistp = result_flist;

CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
}






void
fm_mso_utils_get_item_type (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) 
{

	/* Variable Declaration Start */
	pin_flist_t             *flistp=NULL;
	poid_t                  *tmp_pdp0=NULL;
	poid_t                  *tmp_pdp1=NULL;
	poid_t                  *tmp_pdp2=NULL;
	poid_t                  *tmp_pdp3=NULL;
	pin_flist_t             *flistp0=NULL;
	pin_flist_t             *flistp1=NULL;
	poid_t                  *tmp_pdp4=NULL;
	poid_t                  *inp_pdp=NULL;
	char                    *tmp_str0="";
	int                     local = LOCAL_TRANS_OPEN_SUCCESS;
	/* Variable Declaration End */

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_utils_get_item_type", ebufp);
                return;
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_item_type input flist", i_flistp);
	flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, flistp, PIN_FLD_POID, ebufp);
	flistp0 = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_EVENT, ebufp);
	PIN_FLIST_CONCAT(flistp0, i_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_item_type search input flist", flistp);
	PCM_OP(ctxp, PCM_OP_BILL_ITEM_ASSIGN, 0, flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_item_type search output flist", *r_flistpp);

	if ( PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in test", ebufp);
		goto cleanup;
	}
	
	tmp_str0 = (char*)PIN_POID_GET_TYPE(PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_ITEM_OBJ, 0, ebufp));
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp_str0);
	PIN_FLIST_FLD_SET (*r_flistpp, PIN_FLD_ITEM_TYPE, tmp_str0, ebufp);

	/* Destroy Section Start */
	cleanup:
	PIN_FLIST_DESTROY_EX(&flistp, NULL);
	/* Destroy Section End */
}

void
fm_mso_utils_get_lco_settlement_report(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;

        int64                   db = -1;
        int32                   srch_flag = 256;

        poid_t                  *srch_pdp = NULL;
        poid_t                  *event_objp = NULL;
        poid_t                  *sess_objp = NULL;
        char                    *template = "select X from /mso_lco_settlement_report where F1.type = V1 and F2 = V2 ";

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_utils_get_lco_settlement_report", ebufp);
                return;
        }

        event_objp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        sess_objp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SESSION_OBJ, 0, ebufp);
        db = PIN_POID_GET_DB(event_objp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        srch_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_EVENT_OBJ, event_objp, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_SESSION_OBJ, sess_objp, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        //PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_lco_settlement_report Search input flist", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &result_flist, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_lco_settlement_report Search output flist", result_flist);
        *r_flistp = result_flist;

CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
}
void
fm_mso_utils_get_plan_types (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        int32                   srv_type,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        /* Variable Declaration Start */
        pin_flist_t             *flistp=NULL;
        pin_flist_t             *r_flistp=NULL;
        pin_flist_t             *plan_arry = NULL;
        pin_flist_t             *res_out = NULL;
        pin_flist_t             *deal_arry=NULL;
        pin_flist_t             *prd_arry=NULL;
        poid_t                  *tmp_pdp0=NULL;
        int32                   tmp_enum0=5;
        int32                   tmp_enum1=0;
        poid_t                  *tmp_pdp1=NULL;
        poid_t                  *dbt_pdp=NULL;
        poid_t                  *pln_pdp = NULL;
        int32                  *pln_type = NULL;
        int32                  *pln_sub_type = NULL;
        char                    *tmp_str0="%";
        int32                   tmp_enum2=0;
        int32                   tmp_enum3=0;
	
        /* Variable Declaration End */

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_utils_get_plan_types", ebufp);
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);
	dbt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
//	db = PIN_POID_GET_DB(dbt_pdp);

        flistp = PIN_FLIST_CREATE(ebufp);
//        tmp_pdp0 = PIN_POID_CREATE(db, "/plan", -1, ebufp);
        PIN_FLIST_FLD_PUT(flistp,PIN_FLD_POID, dbt_pdp, ebufp);
       PIN_FLIST_FLD_SET(flistp,PIN_FLD_SEARCH_KEY, (void*)&tmp_enum0, ebufp);
        PIN_FLIST_FLD_SET(flistp,PIN_FLD_SEARCH_TYPE, (void*)&tmp_enum1, ebufp);
        tmp_pdp1 = PIN_POID_FROM_STR("0.0.0.1 /service/admin_client 56432", NULL, ebufp);
        PIN_FLIST_FLD_PUT(flistp,PIN_FLD_USERID, tmp_pdp1, ebufp);
        PIN_FLIST_FLD_SET(flistp,PIN_FLD_VALUE, tmp_str0, ebufp);
        PIN_FLIST_FLD_SET(flistp,PIN_FLD_TYPE_OF_SERVICE, (void*)&srv_type, ebufp);
        PIN_FLIST_FLD_SET(flistp,MSO_FLD_PLAN_TYPE, (void*)&tmp_enum3, ebufp);

        PCM_OP(ctxp, MSO_OP_CUST_GET_PLAN_DETAILS, 0, flistp, &r_flistp, ebufp);
        if ( PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in test", ebufp);
                goto cleanup;
        }
        //PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_plan_types output flist", r_flistp);
        res_out = PIN_FLIST_CREATE(ebufp);
if ( PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in test", ebufp);
                goto cleanup;
        }

        plan_arry=PIN_FLIST_ELEM_GET(r_flistp,PIN_FLD_PLAN,0,0,ebufp);
        pln_pdp = (void *)PIN_FLIST_FLD_GET(plan_arry,PIN_FLD_POID,0,ebufp);
        deal_arry = PIN_FLIST_ELEM_GET(plan_arry,PIN_FLD_DEALS,0,0,ebufp);
        prd_arry = PIN_FLIST_ELEM_GET(deal_arry,PIN_FLD_PRODUCTS,0,0,ebufp);
        pln_type = (int32 *)PIN_FLIST_FLD_GET(plan_arry,MSO_FLD_PLAN_TYPE,0,ebufp);
        pln_sub_type = (int32 *)PIN_FLIST_FLD_GET(prd_arry,MSO_FLD_PLAN_SUB_TYPE,0,ebufp);
        PIN_FLIST_FLD_SET(res_out,PIN_FLD_POID,pln_pdp,ebufp);
        PIN_FLIST_FLD_SET(res_out,MSO_FLD_PLAN_TYPE,pln_type,ebufp);
        PIN_FLIST_FLD_SET(res_out,MSO_FLD_PLAN_SUB_TYPE,pln_sub_type,ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_plan_types output flist", plan_arry);
//      *r_flistpp = r_flistp;
        *r_flistpp = res_out;
        /* Destroy Section Start */
cleanup:
        PIN_FLIST_DESTROY_EX(&flistp, NULL);
}

void
fm_mso_utils_gen_event(
        pcm_context_t   *ctxp,
        poid_t          *acct_pdp,
        poid_t          *serv_pdp,
        char            *program,
        char            *descr,
        char            *event_type,
        pin_flist_t     *in_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *temp_flistp = NULL;

        poid_t              *e_pdp = NULL;
        poid_t              *user_pdp = NULL;

        time_t              event_time;
        char                  msg[64];

        if(PIN_ERRBUF_IS_ERR(ebufp))
                            return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist is:", in_flistp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to create LC event");
        /* event creation flist */
        flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)acct_pdp, ebufp);
        temp_flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_EVENT, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to create LC event2");
        e_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(acct_pdp), event_type, (int64)-1, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_POID, (void *)e_pdp, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acct_pdp, ebufp);
	if(serv_pdp != NULL)
        	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_SERVICE_OBJ, (void *)serv_pdp, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to create LC event3");
        sprintf(msg, "%s %s", descr, "event log");
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_NAME, (void *)msg, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_PROGRAM_NAME, (void *)program, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_SYS_DESCR, (void *)descr, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_DESCR, (void *)descr, ebufp);

        PIN_FLIST_CONCAT( temp_flistp, in_flistp, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to create LC event4");

        /* get csr id */
        user_pdp = CM_FM_USERID(ebufp);
        if(user_pdp)
        {
            PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_USERID, (void *)user_pdp, ebufp);
        }
        event_time = pin_virtual_time(NULL);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_START_T, (void *)&event_time, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_END_T, (void *)&event_time, ebufp);

        sprintf(msg, "%s %s", descr, "input flist");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, flistp);

        /* call opcode */
        PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, flistp, &r_flistp, ebufp);

        sprintf(msg, "%s %s", descr, "output flist");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, msg, r_flistp);

        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
                            //sprintf(msg, "%s event generate error", descr);
                            PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_utils_gen_event() error", ebufp);
        }

        PIN_FLIST_DESTROY_EX(&flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;
}

/**************************************************************************
* Function:		fm_mso_utils_gen_lifecycle_evt()	
* Decription:	For getting the sorted open items for all bills.
**************************************************************************/
void
fm_mso_utils_gen_lifecycle_evt(
        pcm_context_t   *ctxp,
        poid_t          *acct_pdp,
        poid_t          *serv_pdp,
        char            *program,
        char            *descr,
	char		*name,
        char            *event_type,
        pin_flist_t     *in_flistp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *temp_flistp = NULL;
	pin_flist_t     *p_flistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *p_oflistp = NULL;
	pin_flist_t	*change_plan_flistp = NULL;
	pin_flist_t     *e_flistp = NULL;
	pin_flist_t     *pr_flistp = NULL;
	pin_flist_t	*sort_fld_flistp = NULL;
	pin_flist_t     *r1_flistp = NULL;
	pin_flist_t     *mppr_flistp = NULL;	

        poid_t              *e_pdp = NULL;
        poid_t              *user_pdp = NULL;
	poid_t              *p_pdp = NULL;
	poid_t 		    *s_pdp = NULL;
	poid_t		    *act_pdp = NULL;
	poid_t              *old_plan_pdp = NULL;
        poid_t              *new_plan_pdp = NULL;
	poid_t              *mso_oplan_pdp = NULL;	
	poid_t 		    *mso_nplan_pdp = NULL;

	int32               flags = 0;
        int64               db = 0;
	int64               id = -1;
	int32               op_status = 7;
	int32               np_status = 1;
	int32		    mso_pp_count = 0;
	int32		    *mpp_status = 0;		
	int32               elem_id = 0;
        pin_cookie_t        cookie = NULL;

	
        time_t              event_time;
        char                  msg[64];
	char                *p_template = "select X from /mso_purchased_plan where ( F1 = V1 or F2 = V2 ) and F3 = V3 and ( F4 = V4 or F5 = V5 ) ";
	char                *s_type=NULL;	
        void                    *vp = NULL;

        if(PIN_ERRBUF_IS_ERR(ebufp))
                            return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist is:", in_flistp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to create LC event");
        /* event creation flist */
        flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)acct_pdp, ebufp);
        
	temp_flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_EVENT, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to create LC event2");
        
	e_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(acct_pdp), event_type, (int64)-1, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_POID, (void *)e_pdp, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acct_pdp, ebufp);
	if(serv_pdp != NULL && PIN_POID_GET_ID(serv_pdp) > 1)
        	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_SERVICE_OBJ, (void *)serv_pdp, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to create LC event3");
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_NAME, (void *)name, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_PROGRAM_NAME, (void *)program, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_SYS_DESCR, (void *)descr, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_DESCR, (void *)name, ebufp);

        PIN_FLIST_CONCAT( temp_flistp, in_flistp, ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Starting to create LC event4");

        /* get csr id */
        user_pdp = CM_FM_USERID(ebufp);
        if(user_pdp)
        {
            PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_USERID, (void *)user_pdp, ebufp);
        }
        event_time = pin_virtual_time(NULL);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_START_T, (void *)&event_time, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_END_T, (void *)&event_time, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_ACT_USAGE input flist", flistp);
        	
	e_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_EVENT, PIN_ELEMID_ANY, 1, ebufp);
	s_pdp = PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_SERVICE_OBJ,1, ebufp);
	s_type = (char *)PIN_POID_GET_TYPE(s_pdp); 

	if (s_type && strcmp(s_type, "/service/telco/broadband")==0) {
	
		change_plan_flistp = PIN_FLIST_ELEM_GET(e_flistp, MSO_FLD_CHANGE_PLAN, PIN_ELEMID_ANY, 1, ebufp);
		act_pdp = PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	if (change_plan_flistp) {

		/*to find Old mso purchase plan obj*/
		old_plan_pdp = PIN_FLIST_FLD_GET(change_plan_flistp, MSO_FLD_OLD_PLAN_OBJ, 0, ebufp);	
		new_plan_pdp = PIN_FLIST_FLD_GET(change_plan_flistp, MSO_FLD_NEW_PLAN_OBJ, 0, ebufp);
	
		db = PIN_POID_GET_DB(new_plan_pdp);
        	p_flistp = PIN_FLIST_CREATE(ebufp);
        	p_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	
		PIN_FLIST_FLD_PUT(p_flistp, PIN_FLD_POID, p_pdp, ebufp);
        	PIN_FLIST_FLD_SET(p_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        	PIN_FLIST_FLD_SET(p_flistp, PIN_FLD_TEMPLATE, p_template, ebufp);

        	args_flistp = PIN_FLIST_ELEM_ADD(p_flistp, PIN_FLD_ARGS, 1, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, old_plan_pdp, ebufp);
	
		args_flistp = PIN_FLIST_ELEM_ADD(p_flistp, PIN_FLD_ARGS, 2, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, new_plan_pdp, ebufp);

        	args_flistp = PIN_FLIST_ELEM_ADD(p_flistp, PIN_FLD_ARGS, 3, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(p_flistp, PIN_FLD_ARGS, 4, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &op_status, ebufp);

                args_flistp = PIN_FLIST_ELEM_ADD(p_flistp, PIN_FLD_ARGS, 5, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &np_status, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(p_flistp, PIN_FLD_RESULTS, 0, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, 0, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_MOD_T, NULL, ebufp);

        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input Search flist for new and old mso_purchase_plan poid:",p_flistp);
        	PCM_OP(ctxp, PCM_OP_SEARCH, 0, p_flistp, &p_oflistp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output Search flist for new and old mso_purchase_plan poid", p_oflistp);	
	
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "flist Before Sort", p_oflistp);	
		sort_fld_flistp = PIN_FLIST_CREATE(ebufp);
        	r1_flistp = PIN_FLIST_ELEM_ADD(sort_fld_flistp,PIN_FLD_RESULTS, 0, ebufp );
		PIN_FLIST_FLD_SET(r1_flistp, PIN_FLD_MOD_T, 0, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Sort Fld flist", sort_fld_flistp);
		PIN_FLIST_SORT_REVERSE(p_oflistp,sort_fld_flistp,0,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "flist After Sort", p_oflistp);			

		while ((mppr_flistp = PIN_FLIST_ELEM_GET_NEXT(p_oflistp,PIN_FLD_RESULTS,&elem_id, 1, &cookie, ebufp))!= NULL && mso_pp_count!=2) {
			
			mpp_status = PIN_FLIST_FLD_GET(mppr_flistp, PIN_FLD_STATUS, 0, ebufp);
			
			if (mpp_status && *mpp_status == op_status) {
			
                		mso_oplan_pdp = PIN_FLIST_FLD_GET(mppr_flistp, PIN_FLD_POID, 0, ebufp);
				mso_pp_count++;
				op_status = 0;

			}
			else if (mpp_status && *mpp_status == np_status) {

				mso_nplan_pdp = PIN_FLIST_FLD_GET(mppr_flistp, PIN_FLD_POID, 0, ebufp);
                                mso_pp_count++;
				np_status = 0;

			}	
			
		}		

		PIN_FLIST_FLD_SET(change_plan_flistp,MSO_FLD_OLD_PURCHASED_PLAN_OBJ,mso_oplan_pdp,ebufp);
                PIN_FLIST_FLD_SET(change_plan_flistp,MSO_FLD_NEW_PURCHASED_PLAN_OBJ,mso_nplan_pdp,ebufp);

		PIN_FLIST_DESTROY_EX(&p_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&p_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&sort_fld_flistp, NULL);
	
		//PIN_FLIST_FLD_SET(change_plan_flistp,MSO_FLD_OLD_PURCHASED_PLAN_OBJ,mso_oplan_pdp,ebufp);
		//PIN_FLIST_FLD_SET(change_plan_flistp,MSO_FLD_NEW_PURCHASED_PLAN_OBJ,mso_nplan_pdp,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After ADDING NEW and OLD Purchase Plan obj", flistp);
		}
	}		

        /* call opcode *//*commented to chaneg PCM_OP_ACT_USAGE to PCM_OP_CREATE_OBJ as part of NTO*/
        //PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, flistp, &r_flistp, ebufp);
	if (s_type && strcmp(s_type, "/service/telco/broadband")!=0) {
		e_flistp = PIN_FLIST_SUBSTR_GET(flistp, PIN_FLD_EVENT, 1, ebufp);
	}

	vp = PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_START_T, 1, ebufp);
	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_EARNED_START_T, vp, ebufp);
	vp = PIN_FLIST_FLD_GET(e_flistp, PIN_FLD_END_T, 1, ebufp);
	PIN_FLIST_FLD_SET(e_flistp, PIN_FLD_EARNED_END_T, vp, ebufp);


	/*NTO -- call PCM_OP_CREATE_OBJ opcode*/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ input flist", e_flistp);
        PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, e_flistp, &r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ output flist", r_flistp);


        //PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_ACT_USAGE output flist", r_flistp);
	//	PIN_FLIST_DESTROY_EX(&flistp, NULL);

        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "PCM_OP_ACT_USAGE error", ebufp);
			//return;
        }
        PIN_FLIST_DESTROY_EX(&flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return;
}


/**************************************************************************
* Function:		fm_mso_utils_get_pending_bill_items()
* Owner:		Hrish Kulkarni
* Decription:	For getting the sorted open items for all bills.
**************************************************************************/
void fm_mso_utils_get_pending_bill_items(
	pcm_context_t			*ctxp,
	poid_t				*billinfo_pdp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*s_flistp = NULL;
	pin_flist_t			*s_oflistp = NULL;
	pin_flist_t			*args_flistp = NULL;
	pin_flist_t			*res_flistp = NULL;
	pin_flist_t			*item_flistp = NULL;
	pin_flist_t			*item_arrp = NULL;
	pin_flist_t			*sorted_item_flistp = NULL;
	poid_t				*s_pdp = NULL;
	int32				flags = 256;
	int64				db = 0;
	int64				id = -1;
	pin_decimal_t			*zero_due = NULL;
	char				*s_template = " select X from /bill where F1 = V1 and F2 > V2"
					" and bill_no is not null order by end_t asc ";
	int32                  		elem_id1 = 0;
	pin_cookie_t            	cookie1 = NULL;
	int32				elem_id2 = 0;
	pin_cookie_t			cookie2 = NULL;
	poid_t				*ar_bill_pdp = NULL;
	int32				item_count = 0;
	int32				rec_id = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error before calling fm_mso_get_pending_bills", ebufp);
		return;
	}
	zero_due = pbo_decimal_from_str("0.0", ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_get_pending_bills");
	db = PIN_POID_GET_DB(billinfo_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	/*******************************************************
	PCM_OP_SEARCH Input:
	0 PIN_FLD_POID POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_FLAGS INT [0] 256
	0 PIN_FLD_TEMPLATE STR [0] " select X from /bill where F1 = V1 and F2 > V2 
							and bill_no is not null order by end_t asc "
	0 PIN_FLD_ARGS ARRAY [1]
	1   PIN_FLD_BILLINFO_OBJ POID [0] 0.0.0.1 /billinfo 2333595
	0 PIN_FLD_ARGS ARRAY [2]
	1   PIN_FLD_DUE DECIMAL [0] 0
	0 PIN_FLD_RESULTS ARRAY [0]
	1   PIN_FLD_END_T TSTAMP [0] (0)
	1   PIN_FLD_DUE DECIMAL [0] 0

	PCM_OP_SEARCH Input:
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 3
	1     PIN_FLD_END_T        TSTAMP [0] (1409509800) Mon Sep  1 00:00:00 2014
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /bill 2332571 8
	0 PIN_FLD_RESULTS       ARRAY [1] allocated 20, used 3
	1     PIN_FLD_END_T        TSTAMP [0] (1412101800) Wed Oct  1 00:00:00 2014
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /bill 2334619 4
	0 PIN_FLD_RESULTS       ARRAY [2] allocated 20, used 3
	1     PIN_FLD_END_T        TSTAMP [0] (1414780200) Sat Nov  1 00:00:00 2014
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /bill 2344720 4
	*******************************************************/
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_DUE, zero_due, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, (time_t *)NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search pending bill Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_utils_get_pending_bill_items PCM_OP_SEARCH error.", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search pending bill Output flist:",s_oflistp);	
	res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (res_flistp == NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
			"There are no pending bills for alloction!");
		goto CLEANUP;
	}

	sorted_item_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(sorted_item_flistp, PIN_FLD_POID, billinfo_pdp, ebufp);
	while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(s_oflistp, PIN_FLD_RESULTS,
			&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				"Processig the Bill Array:",res_flistp);
		ar_bill_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);
		/* Call the function to get the open bill items */
		fm_mso_utils_get_open_bill_items(ctxp, ar_bill_pdp, 0, &item_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_utils_get_open_bill_items error.", ebufp);
			goto CLEANUP;
		}

		item_count = PIN_FLIST_ELEM_COUNT(item_flistp, PIN_FLD_ITEMS, ebufp);
		if (item_count == 0)
		{
			continue;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				"Sorted Items for the bill:",item_flistp);
		
		elem_id2 = 0;
		cookie2 = NULL;

		while((item_arrp = PIN_FLIST_ELEM_GET_NEXT(item_flistp, PIN_FLD_ITEMS,
				&elem_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_FLIST_ELEM_COPY(item_flistp, PIN_FLD_ITEMS, elem_id2,
				sorted_item_flistp, PIN_FLD_ITEMS, rec_id, ebufp);
			rec_id++;
		}
		PIN_FLIST_DESTROY_EX(&item_flistp, NULL);
		item_flistp = NULL;
	}

	*o_flistpp = PIN_FLIST_COPY(sorted_item_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
				"Final Sorted Open Items:",*o_flistpp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&item_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&sorted_item_flistp, NULL);
	return;
}

/**************************************************************************
* Function:		fm_mso_utils_get_open_bill_items()
* Owner:		Hrish Kulkarni
* Decription:	For getting the sorted open items for a bill.
**************************************************************************/
void fm_mso_utils_get_open_bill_items(
	pcm_context_t		*ctxp,
	poid_t				*ar_bill_pdp,
    int32               all_items_flag,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*s_flistp = NULL;
	pin_flist_t			*s_oflistp = NULL;
	pin_flist_t			*args_flistp = NULL;
	pin_flist_t			*res_flistp = NULL;
	pin_flist_t			*item_flistp = NULL;
	poid_t				*s_pdp = NULL;
	int32				flags = 256;
	int64				db = 0;
	int64				id = -1;
	pin_decimal_t		*zero_due = NULL;
	pin_decimal_t		*amount = NULL;
	pin_decimal_t		*sub_amtp = NULL;
	pin_decimal_t		*rental_amtp = NULL;
	pin_decimal_t		*cpe_amtp = NULL;
	pin_decimal_t		*deposit_amtp = NULL;
	pin_decimal_t		*gst_amtp = NULL;
	pin_decimal_t		*gst_perp = NULL;
	char				*s_template = " select X from /item where F1 = V1 and F2 = V2 and F3 > V3 ";
	char				*s_templatep = " select X from /item where F1 = V1 and F2 > V2 ";
    char                *item_namep = NULL;
    char                *sub_hsnp = "9984";
    char                *cpe_hsnp = "9987";
    char                *rental_hsnp = "9973";
    char                *gst_hsnp = "GST";
    char                *deposit_hsnp = "HW_DEPOSIT";
	int32               elem_id = 0;
	pin_cookie_t        cookie = NULL;
	int32				open_status = 2;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error before calling fm_mso_utils_get_open_bill_items", ebufp);
		return;
	}
	zero_due = pbo_decimal_from_str("0.0", ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_utils_get_open_bill_items");

	db = PIN_POID_GET_DB(ar_bill_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	/*******************************************************
	PCM_OP_SEARCH Input:
	0 PIN_FLD_POID POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_FLAGS INT [0] 256
	0 PIN_FLD_TEMPLATE STR [0] " select X from /item where F1 = V1 and F2 = V2 and F3 > V3 "
	0 PIN_FLD_ARGS ARRAY [1]
	1   PIN_FLD_AR_BILL_OBJ POID [0] 0.0.0.1 /bill 2332571
	0 PIN_FLD_ARGS ARRAY [2]
	1   PIN_FLD_STATUS ENUM [0] 2
	0 PIN_FLD_ARGS ARRAY [3]
	1   PIN_FLD_DUE          DECIMAL [0] 0
	0 PIN_FLD_RESULTS ARRAY [0]
	1   PIN_FLD_DUE   DECIMAL[0] NULL

	PCM_OP_SEARCH Output:
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 3
	1     PIN_FLD_DUE     DECIMAL [0] 12.50
	1     PIN_FLD_POID       POID [0] 0.0.0.1 /item/cycle_forward/mso_hw_rental 2342026 5
	0 PIN_FLD_RESULTS       ARRAY [1] allocated 20, used 3
	1     PIN_FLD_DUE     DECIMAL [0] 155.00
	1     PIN_FLD_POID       POID [0] 0.0.0.1 /item/misc 2341642 3
	*******************************************************/
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_AR_BILL_OBJ, ar_bill_pdp, ebufp);

    if (all_items_flag)
    { 
	    PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_templatep, ebufp);
	    args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	    PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_ITEM_TOTAL, zero_due, ebufp);
    }
    else
    {
	    PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
	    args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &open_status, ebufp);

	    args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	    PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_DUE, zero_due, ebufp);
    }

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DUE, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_AR_BILL_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CURRENCY, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DISPUTED, NULL, ebufp);

	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EFFECTIVE_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ITEM_TOTAL, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RECVD, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ADJUSTED, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TRANSFERED, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ITEM_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_AR_BILLINFO_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EVENT_POID_LIST, NULL, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Open item Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_utils_get_open_bill_items PCM_OP_SEARCH error.", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Open item Output flist:",s_oflistp);
	res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (res_flistp == NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
			"There are no open items for alloction!");
		goto CLEANUP;
	}
	PIN_FLIST_FLD_RENAME(s_oflistp, PIN_FLD_RESULTS, PIN_FLD_ITEMS, ebufp);
	gst_amtp = pbo_decimal_from_str("0.0", ebufp);
	sub_amtp = pbo_decimal_from_str("0.0", ebufp);
	rental_amtp = pbo_decimal_from_str("0.0", ebufp);
	cpe_amtp = pbo_decimal_from_str("0.0", ebufp);
	deposit_amtp = pbo_decimal_from_str("0.0", ebufp);
	gst_perp = pbo_decimal_from_str("1.18", ebufp);
	while((item_flistp = PIN_FLIST_ELEM_GET_NEXT(s_oflistp, PIN_FLD_ITEMS,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		amount = PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_DUE, 0, ebufp);
		PIN_FLIST_FLD_PUT(item_flistp, PIN_FLD_AMOUNT, pbo_decimal_negate(amount, ebufp), ebufp);
        if (all_items_flag)
        {
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Creating HSN bucket amounts:", item_flistp);
		    amount = PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_ITEM_TOTAL, 0, ebufp);
		    item_namep = PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_NAME, 0, ebufp);
            if (item_namep && strstr(item_namep, "Global Service Tax"))
            {
                pbo_decimal_add_assign(gst_amtp, amount, ebufp);
            }
            else if (item_namep && strstr(item_namep, "deposit"))
            {
                pbo_decimal_add_assign(deposit_amtp, amount, ebufp);
            }
            else if (item_namep && strstr(item_namep, "CPE Charge"))
            {
                pbo_decimal_add_assign(cpe_amtp, amount, ebufp);
            }
            else if (item_namep && strstr(item_namep, "Rental Charge"))
            {
                pbo_decimal_add_assign(rental_amtp, amount, ebufp);
            }   
            else
            {
                pbo_decimal_add_assign(sub_amtp, amount, ebufp);
            }
         }
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_utils_get_open_bill_items - Item flist before prioity sort:",s_oflistp);
	/* Sort the items based on priority */
	fm_mso_utils_prioritize_pymt_items(ctxp, s_oflistp, o_flistpp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_utils_get_open_bill_items PCM_OP_SEARCH error.", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_utils_get_open_bill_items - Item flist after prioity sort:",*o_flistpp);
    if (all_items_flag)
    {
    item_flistp = PIN_FLIST_ELEM_ADD(*o_flistpp, PIN_FLD_TAXES, 0, ebufp);
    PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_TAX_CODE, sub_hsnp, ebufp);
    pbo_decimal_multiply_assign(sub_amtp, gst_perp, ebufp);
    PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_AMOUNT, sub_amtp, ebufp);

    item_flistp = PIN_FLIST_ELEM_ADD(*o_flistpp, PIN_FLD_TAXES, 1, ebufp);
    PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_TAX_CODE, cpe_hsnp, ebufp);
    pbo_decimal_multiply_assign(cpe_amtp, gst_perp, ebufp);
    PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_AMOUNT, cpe_amtp, ebufp);

    item_flistp = PIN_FLIST_ELEM_ADD(*o_flistpp, PIN_FLD_TAXES, 2, ebufp);
    PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_TAX_CODE, rental_hsnp, ebufp);
    pbo_decimal_multiply_assign(rental_amtp, gst_perp, ebufp);
    PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_AMOUNT, rental_amtp, ebufp);

    item_flistp = PIN_FLIST_ELEM_ADD(*o_flistpp, PIN_FLD_TAXES, 3, ebufp);
    PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_TAX_CODE, deposit_hsnp, ebufp);
    PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_AMOUNT, deposit_amtp, ebufp);

/*    item_flistp = PIN_FLIST_ELEM_ADD(*o_flistpp, PIN_FLD_TAXES, 3, ebufp);
    PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_TAX_CODE, gst_hsnp, ebufp);
    PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_AMOUNT, gst_amtp, ebufp);*/
    }
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
	    if (!pbo_decimal_is_null(gst_amtp, ebufp)) pbo_decimal_destroy(&gst_amtp);
        if (!pbo_decimal_is_null(sub_amtp, ebufp)) pbo_decimal_destroy(&sub_amtp);
        if (!pbo_decimal_is_null(sub_amtp, ebufp)) pbo_decimal_destroy(&cpe_amtp);
        if (!pbo_decimal_is_null(sub_amtp, ebufp)) pbo_decimal_destroy(&rental_amtp);
	return;
}

/****************************************************************************
* Re-arrage the input items based on payment knock out priority
****************************************************************************/

void
fm_mso_utils_prioritize_pymt_items(
        pcm_context_t   *ctxp,
        pin_flist_t	*i_flistp,
        pin_flist_t     **o_flistpp,
        pin_errbuf_t    *ebufp)
{
	poid_t                  *pdp = NULL;
        int32                   rec_id = 0;
        int32                   cfg_rec_id = 0;
        int32                   i_rec_id = 0;
        int32                   elem_sort = 0;
	char                    *cfg_item_type = NULL;
        char                    *in_item_type = NULL;
	pin_cookie_t            cookie = NULL;
        pin_cookie_t            i_cookie = NULL;
        pin_cookie_t            cfg_cookie = NULL;
	pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *cfg_flistp = NULL;
        pin_flist_t             *sort_flistp = NULL;
        pin_flist_t             *c_flistp = NULL;
        pin_flist_t             *flistp = NULL;
        pin_flist_t             *item_flistp = NULL;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_utils_prioritize_pymt_items input flist", i_flistp);
	/*
         * Retrieve payment allocation order object from DB
         */
        pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID,
                        0, ebufp);

        fm_pymt_pol_pymt_priority_search(ctxp, pdp, &res_flistp, ebufp);
        cfg_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_utils_prioritize_pymt_items priority flist", cfg_flistp);

	if(!cfg_flistp) {
		*o_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
		goto cleanup;
	}

        /***********************************************************
         * Go thru the input items and re-arrange according to the
         * charge head knock out priority
         ***********************************************************/

	in_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
        sort_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

        cookie = NULL;
        while ((flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_ITEMS,
                &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {
                PIN_FLIST_ELEM_DROP( sort_flistp, PIN_FLD_ITEMS, rec_id, ebufp);
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"sort_flistp before process:",sort_flistp);
	rec_id = 0;

        while (cfg_flistp && ((c_flistp = PIN_FLIST_ELEM_GET_NEXT(cfg_flistp, PIN_FLD_PAY_ORDER_INFO,
                &cfg_rec_id, 1, &cfg_cookie, ebufp)) != (pin_flist_t *)NULL)) {

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"c_flistp :",c_flistp);
	
                i_cookie = NULL;
		i_rec_id = 0;
                while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp, PIN_FLD_ITEM_TYPES,
                        &i_rec_id, 1, &i_cookie, ebufp)) != (pin_flist_t *)NULL) {

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"item_flistp :",item_flistp);
                        cfg_item_type = (char *)PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_ITEM_TYPE,
                                        0, ebufp);

                        cookie = NULL;
			rec_id = 0;
                        while ((flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_ITEMS,
                                &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"IN flistp:",flistp);

                                pdp = (poid_t *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_POID,
                                                0, ebufp);
                                if(pdp) in_item_type = (char *)PIN_POID_GET_TYPE(pdp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"cfg_item_type:");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,cfg_item_type);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"in_item_type:");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,in_item_type);

                                if(cfg_item_type && in_item_type &&
                                         ( strcmp( cfg_item_type, in_item_type ) == 0 ) ) {

					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Match");
                                        PIN_FLIST_ELEM_SET(sort_flistp, flistp, PIN_FLD_ITEMS,
                                                elem_sort++, ebufp);
                                        PIN_FLIST_ELEM_DROP(in_flistp, PIN_FLD_ITEMS, rec_id, ebufp);
                                        cookie = NULL;

                                }
                        }
                }
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "sort_flistp before:", sort_flistp);
	/***********************************************************
         * Append the remaining items at the bottom
        ***********************************************************/
        cookie = NULL;
        while ((flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_ITEMS,
                &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {
                PIN_FLIST_ELEM_SET(sort_flistp, flistp, PIN_FLD_ITEMS,
                                                elem_sort++, ebufp);
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "op_pymt_pol_under_payment sort flist", sort_flistp);
	*o_flistpp = sort_flistp;

cleanup:
	PIN_FLIST_DESTROY(in_flistp, NULL);
        PIN_FLIST_DESTROY(res_flistp, NULL);
	return;
}

void
fm_mso_get_object_poid (
        pcm_context_t           *ctxp,
        char                    *template,
        pin_flist_t             *arg_flistp,
        void                    **ret_objp,
        pin_errbuf_t            *ebufp) {

        int64                   db=1;
        int32                   srch_flag=256;
        char                    msg[2048];

        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_r_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        poid_t                  *src_poidp = NULL;

        if(PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_object_poid arg flist", arg_flistp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        src_poidp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, src_poidp, ebufp);

        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        result_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);

        PIN_FLIST_CONCAT(srch_flistp, arg_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_object_poid search input flist", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_object_poid search output flist", srch_r_flistp);

        result_flistp = PIN_FLIST_ELEM_GET(srch_r_flistp,PIN_FLD_RESULTS,0,1,ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                goto cleanup;
        }
        if (result_flistp != NULL) {
                *ret_objp = PIN_FLIST_FLD_TAKE(result_flistp, PIN_FLD_POID, 1, ebufp);
        }

cleanup:
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_r_flistp, NULL);
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_object_poid search output poid", *ret_objp);
}

void
fm_mso_utils_convert_flist_to_buffer (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp) {

	pin_buf_t buffer;
	char	  *buffer_str=NULL;
	int	  buffer_len = 0;

        if(PIN_ERRBUF_IS_ERR(ebufp))
		return;
        PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_convert_flist_to_buffer input flist", i_flistp);

	PIN_FLIST_TO_STR(i_flistp, &buffer_str, &buffer_len, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, buffer_str);
	buffer.flag = 0; 
	buffer.size = buffer_len;
	buffer.offset = 0;
	buffer.data = buffer_str;
	buffer.xbuf_file = NULL;

	*r_flistpp = PIN_FLIST_CREATE (ebufp);
	PIN_FLIST_FLD_PUT (*r_flistpp, PIN_FLD_BUFFER, &buffer, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_convert_flist_to_buffer input flist", *r_flistpp);

}

void
fm_mso_get_bals_credit_limit_bal (
        pcm_context_t           *ctxp,
        poid_t                  *account_poidp,
        poid_t                  *bal_grp_poidp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp) {

	pin_flist_t		*flistp = NULL;
	pin_flist_t		*bals_flistp = NULL;
        int32                   resource = 356;
	pin_decimal_t		*zerop = NULL;

        if(PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

	flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(flistp,PIN_FLD_POID, account_poidp, ebufp);
	PIN_FLIST_FLD_SET(flistp,PIN_FLD_BAL_GRP_OBJ, bal_grp_poidp, ebufp);
	bals_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_BALANCES, resource, ebufp);
	zerop = pbo_decimal_from_str("0.0",ebufp);
	PIN_FLIST_FLD_SET(bals_flistp,PIN_FLD_CURRENT_BAL, (void*)zerop, ebufp);
	PIN_FLIST_FLD_PUT(bals_flistp,PIN_FLD_CREDIT_LIMIT, (void*)zerop, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bals_credit_limit_bal input flist", flistp);
	PCM_OP (ctxp, PCM_OP_BAL_GET_BALANCES, 0, flistp, r_flistp, ebufp);
	if ( PIN_ERRBUF_IS_ERR(ebufp)) {
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in test", ebufp);
        	goto cleanup;
	}


cleanup:
        PIN_FLIST_DESTROY_EX(&flistp, NULL);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bals_credit_limit_bal output flistp", *r_flistp);
}


int32
fm_mso_utils_get_account_type(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
        pin_errbuf_t            *ebufp) {

	pin_flist_t		*r_flistp = NULL;
	int32			*account_type = NULL;
	char			*account_type_str = NULL;

        if(PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
	
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_account_type input poid", acnt_pdp);

	fm_mso_utils_read_object(ctxp, ACCOUNT_OBJECT, acnt_pdp, &r_flistp, ebufp);

	if (r_flistp != NULL) {
		account_type = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
		if (account_type == NULL) return 0;
		switch(*account_type/1000000) {
		    case ACCT_TYPE_CSR_ADMIN: account_type_str = "ACCT_TYPE_CSR_ADMIN"; break;
		    case ACCT_TYPE_CSR: account_type_str = "ACCT_TYPE_CSR"; break;
		    case ACCT_TYPE_LCO: account_type_str = "ACCT_TYPE_LCO"; break;
		    case ACCT_TYPE_JV: account_type_str = "ACCT_TYPE_JV"; break;
		    case ACCT_TYPE_DTR: account_type_str = "ACCT_TYPE_DTR"; break;
		    case ACCT_TYPE_SUB_DTR: account_type_str = "ACCT_TYPE_SUB_DTR"; break;
		    case ACCT_TYPE_OPERATION: account_type_str = "ACCT_TYPE_OPERATION"; break;
		    case ACCT_TYPE_FIELD_SERVICE: account_type_str = "ACCT_TYPE_FIELD_SERVICE"; break;
		    case ACCT_TYPE_LOGISTICS: account_type_str = "ACCT_TYPE_LOGISTICS"; break;
		    case ACCT_TYPE_COLLECTION_AGENT: account_type_str = "ACCT_TYPE_COLLECTION_AGENT"; break;
		    case ACCT_TYPE_SALES_PERSON: account_type_str = "ACCT_TYPE_SALES_PERSON"; break;
		    case ACCT_TYPE_HTW: account_type_str = "ACCT_TYPE_HTW"; break;
		    case ACCT_TYPE_PRE_SALES: account_type_str = "ACCT_TYPE_PRE_SALES"; break;
		    case ACCT_TYPE_SUBSCRIBER: account_type_str = "ACCT_TYPE_SUBSCRIBER"; break;
		    case ACCT_TYPE_SME_SUBS_BB: account_type_str = "ACCT_TYPE_SME_SUBS_BB"; break;
		    case ACCT_TYPE_RETAIL_BB: account_type_str = "ACCT_TYPE_RETAIL_BB"; break;
		    //case ACCT_TYPE_CYBER_CAFE_BB: account_type_str = "ACCT_TYPE_CYBER_CAFE_BB"; break;
		    case ACCT_TYPE_CORP_SUBS_BB: account_type_str = "ACCT_TYPE_CORP_SUBS_BB"; break;
		    default: account_type_str = "ACCT_TYPE_UNKNOWN"; break;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, account_type_str);
	}
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	return *account_type/1000000;

}

/******************************************************
* Function fm_mso_utils_get_tax_excemption_details()
* Author: Harish Kulkarni
* Modificattion done to hanlde validity verification & tax type by Sisir
* Descr: This function returns the Tax exemption details
* for an account. 
* 0- ST
* 1- VAT
* 3 - ET
******************************************************/
int32 fm_mso_utils_get_tax_excemption_details(
        pcm_context_t           *ctxp,
	pin_flist_t             *exempt_out_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *exemtion_flistp = NULL;
        int32                   *exemtion_type = NULL;
        char                    *account_type_str = NULL;
        char                    tmp[256];
	time_t			*exempt_start_t = NULL;
	time_t			*exempt_end_t = NULL;
	time_t			now_t = 0;
	pin_cookie_t		cookie = NULL;
	int			elem_id = 0;
	poid_t			*acct_pdp = NULL;
	int32                   *input_type = NULL;
	pin_decimal_t		*percent = NULL;

        if(PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_tax_excemption_details input flist", exempt_out_flistp);
	acct_pdp = PIN_FLIST_FLD_GET(exempt_out_flistp, PIN_FLD_POID, 0, ebufp);
	input_type = PIN_FLIST_FLD_GET(exempt_out_flistp, PIN_FLD_TYPE, 0, ebufp);


        // Read the account object
        fm_mso_utils_read_any_object(ctxp, acct_pdp, &r_flistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Error in function fm_mso_utils_read_any_object", ebufp);
                PIN_FLIST_DESTROY_EX(&r_flistp,ebufp);
                return (-1);
        }

	while ((exemtion_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp,PIN_FLD_EXEMPTIONS 
, &elem_id, 1, &cookie, ebufp))!= NULL)
	{
		exempt_start_t = PIN_FLIST_FLD_GET(exemtion_flistp, PIN_FLD_START_T, 0, ebufp);
		exempt_end_t = PIN_FLIST_FLD_GET(exemtion_flistp, PIN_FLD_END_T, 0, ebufp);
		exemtion_type = PIN_FLIST_FLD_GET(exemtion_flistp, PIN_FLD_TYPE, 0, ebufp);
		percent = PIN_FLIST_FLD_GET(exemtion_flistp, PIN_FLD_PERCENT, 0, ebufp);
		now_t = pin_virtual_time(NULL);
		if((exempt_start_t && (*exempt_start_t == 0 || now_t >= *exempt_start_t)) &&
			(exempt_end_t && (*exempt_end_t == 0 || now_t < *exempt_end_t)) &&
			(exemtion_type && input_type && (*exemtion_type == *input_type)))
		{
			 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account is Tax Exempted");
			 PIN_FLIST_FLD_SET(exempt_out_flistp, PIN_FLD_TYPE, exemtion_type, ebufp);
			 PIN_FLIST_FLD_SET(exempt_out_flistp, PIN_FLD_PERCENT, percent, ebufp);
			 PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_tax_excemption_details output flist", exempt_out_flistp);
			return 1;
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account is not Tax Exempted");
			return 0;
		}

		

	}

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account is not in Tax Exemtion");

	PIN_FLIST_DESTROY_EX(&r_flistp,ebufp);
        return 0;
}


/*******************************************************************
 * Function: fm_mso_utils_get_lco_services_info
 *
 * This function is used to retrive the service object of LCO account
 * Where the service object would be returned according to the input.
 *
 * Arguments:
 *      1. Context Pointer
 *      2. Input Flist
 *      3. Output Flist (Used to return service object)
 *      4. Service Type (Incollect or Outcollect Service)
 *      5. Error Buffer
 *
 * Return value: None
 *******************************************************************/
void
fm_mso_utils_get_lco_services_info(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        char                    *ser_type,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;

        int64                   db = -1;
        int32                   srch_flag = 256;

        poid_t                  *srch_pdp = NULL;
        poid_t                  *acnt_pdp = NULL;
        poid_t                  *ser_pdp = NULL;
        char                    *template = "select X from /service where F1.id = V1 and F2.type = V2 ";

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_utils_get_lco_services_info", ebufp);
                return;
        }

        acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(acnt_pdp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        srch_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        ser_pdp = PIN_POID_CREATE(db, ser_type, 1, ebufp);
        PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, ser_pdp, ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_lco_services_info Search input flist", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &result_flist, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_lco_services_info Search output flist", result_flist);
        *r_flistp = result_flist;

CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
}

/****************************************************************
fm_mso_update_ncr_validty()
updating the PIN_FLD_VALID_TO of /balance_group during
	cancel plan
	change plan
	service termination
****************************************************************/

void
fm_mso_update_ncr_validty(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*tmp_flist = NULL;
	pin_flist_t		*tmp_flist1 = NULL;
	pin_flist_t		*tmp_flist2 = NULL;
	pin_flist_t		*srch_iflist = NULL;
	pin_flist_t		*srch_oflist =	NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*change_validity_iflist = NULL;
	pin_flist_t		*change_validity_oflist = NULL;
	pin_flist_t		*bal_grp_flist = NULL;
	pin_flist_t             *bal_flistp = NULL;
	pin_flist_t             *sbal_flistp = NULL;

	int64			db = -1;
	int32			srch_flag = 256;

	poid_t			*acnt_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*granter_obj = NULL;
	poid_t			*tmp_pp_obj = NULL;

	int32			elem_id  = 0;
	int32			elem_id1 = 0;
	int32			elem_id2 = 0;
	int32			flags    = 768;
	int32			sub_bal_elem_id_old = 0;
	int32			sub_bal_elem_id_new = 1;

	pin_cookie_t		cookie  = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t		cookie2 = NULL;

	time_t			time_now;

	char			*tmpl_srch_bal_grp = "select X from /balance_group 1, /service 2 where 1.F1.id= V1 and 1.F2 = 2.F3 and 1.F4 > V4 ";
	char			*tmp_prov_tag = NULL;
	char			*prog_name    = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_update_ncr_validty", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_ncr_validty input ", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp );
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp );

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp );

	if (!acnt_pdp || !srvc_pdp )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Missing account/service poid in input");
		return;
	}

	db = PIN_POID_GET_DB(acnt_pdp); 

	time_now = pin_virtual_time(NULL);
	//time_now = fm_utils_time_round_to_midnight(time_now);


	/******************************************
	Fetch the balance group -Start
	*******************************************/
	srch_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp) 

	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, tmpl_srch_bal_grp, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 4, ebufp );
	bal_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp );
	sbal_flistp = PIN_FLIST_ELEM_ADD(bal_flistp, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, ebufp );
        PIN_FLIST_FLD_SET(sbal_flistp, PIN_FLD_VALID_TO, &time_now, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search balance_group input list", srch_iflist);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
		return ;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search balance_group output flist", srch_oflist);
	bal_grp_flist = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_RESULTS, 0, 1, ebufp );
	if (!bal_grp_flist)
		goto CLEANUP;
	/******************************************
	Fetch the balance group -End
	*******************************************/

	change_validity_iflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(change_validity_iflist, PIN_FLD_POID, acnt_pdp, ebufp);
	if (prog_name)
	{
		PIN_FLIST_FLD_SET(change_validity_iflist, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(change_validity_iflist, PIN_FLD_PROGRAM_NAME, "custom_set_validity", ebufp);
	}

//	while ((tmp_flist = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PRODUCTS,&elem_id, 1, &cookie, ebufp))
//		!= (pin_flist_t *)NULL) 
//	{
//		tmp_prov_tag = PIN_FLIST_FLD_GET(tmp_flist, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
//		if (tmp_prov_tag)
//		{
//			tmp_pp_obj = PIN_FLIST_FLD_GET(tmp_flist, MSO_FLD_PURCHASED_PRODUCT_OBJ, 1, ebufp);
			/******************************************
			Loop the balance group -Start
			*******************************************/
			elem_id1 = 0;
			cookie1  = NULL;
			while ((tmp_flist1 = PIN_FLIST_ELEM_GET_NEXT(bal_grp_flist, PIN_FLD_BALANCES, &elem_id1, 1, &cookie1, ebufp))
				!= (pin_flist_t *)NULL) 
			{
				if ( elem_id1 != CURRENCY
				/*  elem_id1 == DATA_CONSUMED_MB || 
				    elem_id1 == FUP_DATA_CONSUMED_MB || 
				    elem_id1 == FREE_MB	    */
				   )
				{
					elem_id2 = 0;
					cookie2 = NULL;
					while ((tmp_flist2 = PIN_FLIST_ELEM_GET_NEXT(tmp_flist1, PIN_FLD_SUB_BALANCES, &elem_id2, 1, &cookie2, ebufp))
						!= (pin_flist_t *)NULL) 
					{
//						granter_obj = PIN_FLIST_FLD_GET(tmp_flist2, PIN_FLD_GRANTOR_OBJ, 1, ebufp);
//						if (  PIN_POID_COMPARE(granter_obj, tmp_pp_obj, 0, ebufp) == 0 )
//						{
							// Old value PIN_FLD_SUB_BALANCES
							arg_flistp = PIN_FLIST_ELEM_ADD(change_validity_iflist, PIN_FLD_SUB_BALANCES, sub_bal_elem_id_old, ebufp );
							PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_RESOURCE_ID, &elem_id1,  ebufp);
							PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ELEMENT_ID, &elem_id2,  ebufp);
							PIN_FLIST_FLD_COPY(tmp_flist2, PIN_FLD_VALID_FROM, arg_flistp, PIN_FLD_VALID_FROM, ebufp);
							PIN_FLIST_FLD_COPY(tmp_flist2, PIN_FLD_VALID_TO, arg_flistp, PIN_FLD_VALID_TO, ebufp);
							PIN_FLIST_FLD_COPY(bal_grp_flist, PIN_FLD_POID, arg_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
							sub_bal_elem_id_old = sub_bal_elem_id_old + 2;

							// New value PIN_FLD_SUB_BALANCES
							arg_flistp = PIN_FLIST_ELEM_ADD(change_validity_iflist, PIN_FLD_SUB_BALANCES, sub_bal_elem_id_new, ebufp );
							PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_RESOURCE_ID, &elem_id1,  ebufp);
							PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ELEMENT_ID, &elem_id2,  ebufp);
							PIN_FLIST_FLD_COPY(tmp_flist2, PIN_FLD_VALID_FROM, arg_flistp, PIN_FLD_VALID_FROM, ebufp);
							PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_VALID_TO, &time_now, ebufp);
							PIN_FLIST_FLD_COPY(bal_grp_flist, PIN_FLD_POID, arg_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
							sub_bal_elem_id_new = sub_bal_elem_id_new + 2;
//						}
					}
				}
			}
			/******************************************
			Loop the balance group -End
			*******************************************/
//		}
//	}
	if (PIN_FLIST_ELEM_COUNT(change_validity_iflist, PIN_FLD_SUB_BALANCES, ebufp ) >0 )
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "change_validity_iflist input list", change_validity_iflist);
		PCM_OP(ctxp, PCM_OP_BAL_CHANGE_VALIDITY, 0, change_validity_iflist, &change_validity_oflist, ebufp);
		PIN_FLIST_DESTROY_EX(&change_validity_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_BAL_CHANGE_VALIDITY", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&change_validity_iflist, NULL);
			return ;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "change_validity_oflist output flist", change_validity_oflist);
	}

	*o_flistpp = PIN_FLIST_COPY(change_validity_oflist, ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&change_validity_iflist, NULL);
	PIN_FLIST_DESTROY_EX(&change_validity_oflist, NULL);
}

/****************************************
* Function fm_mso_utils_get_event_action()
* Harish Kulkarni
* This function returns the poid type for
* the different event types
****************************************/
void fm_mso_utils_get_event_action(
	char		*poid_type,
	char		**descr,
	pin_errbuf_t	*ebufp)
{
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_event_action: Input poid is:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, poid_type);
	if(poid_type && strstr(poid_type,"/event/billing/late_fee") != NULL)
	{
		*descr = strdup("LATE FEE");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/mso_catv_commitment") != NULL)
	{
		*descr = strdup("MINIMUM COMMITMENT");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/mso_cr_dr_note") != NULL)
	{
		*descr = strdup("CREDIT/DEBIT NOTE");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/mso_penalty") != NULL)
	{
		*descr = strdup("PAYMENT REVERSAL PENALTY");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/mso_refund_credit") != NULL)
	{
		*descr = strdup("DEPOSIT REFUND");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/mso_refund_debit") != NULL)
	{
		*descr = strdup("REFUND LOADING");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/product/fee/cycle/cycle_arrear") != NULL)
	{
		*descr = strdup("CYCLE ARREAR CHARGES");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/product/fee/cycle/cycle_forward") != NULL)
	{
		*descr = strdup("CYCLE FORWARD CHARGES");
	}
	else if(poid_type && strcmp(poid_type,"/event/billing/product/fee/purchase/mso_deposit") == 0)
	{
		*descr = strdup("HARDWARE DEPOSIT");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/product/fee/purchase") != NULL)
	{
		*descr = strdup("PURCHASE CHARGES");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/settlement/ws/incollect") != NULL)
	{
		*descr = strdup("LCO BALANCE IMPACT");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/settlement/ws/outcollect") != NULL)
	{
		*descr = strdup("LCO COMMISSION BALANCE IMPACT");
	}
	else if(poid_type && strstr(poid_type,"/event/mso_et") != NULL)
	{
		*descr = strdup("ENTERTAINMENT TAX");
	}
	else if(poid_type && strstr(poid_type,"/event/session/telco/broadband") != NULL)
	{
		*descr = strdup("BROADBAND DATA USAGE");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/payment") != NULL)
	{
		*descr = strdup("PAYMENT");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/reversal") != NULL)
	{
		*descr = strdup("PAYMENT REVERSAL");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/dispute") != NULL)
	{
		*descr = strdup("DISPUTE");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/settlement") != NULL)
	{
		*descr = strdup("SETTLEMENT");
	}
	else if(poid_type && strstr(poid_type,"/event/billing/adjustment") != NULL)
	{
		*descr = strdup("ADJUSTMENT");
	}
	else
	{
		*descr = strdup(poid_type);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_event_action: Return description is:");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, *descr);

	return;
}

void
get_evt_lifecycle_template(
	pcm_context_t			*ctxp,
	int64				db,
	int32				string_id, 
	int32				string_version,
	char				**lc_template, 
	pin_errbuf_t			*ebufp)
{
	int32				*flag = NULL;
	poid_t				*pdp = NULL;
	poid_t				*s_pdp = NULL;
	int64				database;
	pin_flist_t			*search_i_flistp = NULL;
	pin_flist_t			*search_o_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	int32				s_flags ;
	char				*msg_template = NULL;
	int32				str_ver;
	char				*template = "select X from /strings where F1 = V1 and F2 = V2 and F3 = V3 ";


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
        "get_evt_lifecycle_template");

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DOMAIN, "LIFECYCLE_EVENT", ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STRING_ID, &string_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STR_VERSION, &string_version, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STRING, NULL, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_evt_lifecycle_template search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_evt_lifecycle_template search output flist", search_o_flistp);

    tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
    msg_template = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STRING, 0, ebufp);

    if (msg_template && strlen(msg_template) > 0 )
    {
         *lc_template = (char *) malloc(strlen(msg_template)+1);
         memset(*lc_template, 0, strlen(msg_template)+1);
         strcpy(*lc_template,msg_template);
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*lc_template );
    }

    /*if (msg_template)
	free(msg_template);*/
    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}


void
get_strings_template(
	pcm_context_t	    *ctxp,
	int64				db,
    char                *domainp,
	int32				string_id, 
	int32				string_version,
	char				**lc_template, 
	pin_errbuf_t		*ebufp)
{
	int32				*flag = NULL;
	poid_t				*pdp = NULL;
	poid_t				*s_pdp = NULL;
	int64				database;
	pin_flist_t			*search_i_flistp = NULL;
	pin_flist_t			*search_o_flistp = NULL;
	pin_flist_t			*tmp_flistp = NULL;
	int32				s_flags ;
	char				*msg_template = NULL;
	int32				str_ver;
	char				*template = "select X from /strings where F1 = V1 and F2 = V2 and F3 = V3 ";


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return;

    PIN_ERRBUF_CLEAR(ebufp);

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
        "get_strings_template");

    search_i_flistp = PIN_FLIST_CREATE(ebufp);
    s_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

    s_flags = SRCH_DISTINCT ;
    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

    PIN_FLIST_FLD_SET(search_i_flistp, PIN_FLD_TEMPLATE, (void*)template, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DOMAIN, domainp, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STRING_ID, &string_id, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD(search_i_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STR_VERSION, &string_version, ebufp);

    tmp_flistp = PIN_FLIST_ELEM_ADD (search_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STRING, NULL, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_strings_template search input flist ", search_i_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_CACHEABLE, search_i_flistp,
        &search_o_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "get_strings_template search output flist", search_o_flistp);

    tmp_flistp = PIN_FLIST_ELEM_GET(search_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
    if (tmp_flistp)
    {
        msg_template = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STRING, 0, ebufp);
    }

    if (msg_template && strlen(msg_template) > 0 )
    {
         *lc_template = (char *) malloc(strlen(msg_template)+1);
         memset(*lc_template, 0, strlen(msg_template)+1);
         strcpy(*lc_template,msg_template);
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,*lc_template );
    }

    /*if (msg_template)
	free(msg_template);*/
    PIN_FLIST_DESTROY_EX(&search_i_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&search_o_flistp, NULL);

    return;
}


/**************************************************
* Function: 	fm_mso_check_csr_access_level()
* Decription:   Checks if the CSR is authorized to
*		perform this operation as per the access
*		rights provided to CSR (in ACCESS_INFO)
* 
***************************************************/
void
fm_mso_check_csr_access_level(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*acnt_info,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*cc_info = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*access_info = NULL;
	pin_flist_t		*profie_csr = NULL; 
	pin_flist_t		*wholesale_info = NULL;
	pin_flist_t		*profile_info = NULL;
	pin_flist_t		*jv_nameinfo = NULL;
	pin_flist_t		*jv_details = NULL;
	pin_flist_t		*prof_nameinfo = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*acc_details = NULL;

	poid_t			*acnt_pdp = NULL;
	poid_t			*csr_acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*parent_pdp = NULL;
	poid_t			*jv_obj = NULL;
	poid_t			*dt_obj = NULL;
	poid_t			*sdt_obj = NULL;
	poid_t			*lco_obj = NULL;
	poid_t			*profile_pdp = NULL;
	poid_t			*compare_poid = NULL;
	poid_t			*org_obj = NULL;
	
	int32			*org_access_level = NULL;
	int32			*data_access_level = NULL;
	int32			srch_flag = 768;
	int32			access_flag = 0;
	int32			searched_acnt_type = -1;
	int32			*business_type = NULL;
	int32			tmp_business_type = -1;
	int32			account_type = -1;
	int32			org_access_flag = 0;
	int64			db = 1;

	char			*template_srch_acnt = "select X from /profile where F1.id = V1 and profile_t.poid_type in ('/profile/wholesale','/profile/customer_care') " ;
	char			*template_srch_csr = "select X from /profile/wholesale  where F1.id = V1 and F2.type = V2 " ;
	char 			compare_value[100];
	char			*profile_type = NULL;
	char			*acc_address = NULL;
	char			*access_value = NULL;
	
	int				elem_id = 0;
	pin_cookie_t    cookie = NULL;

	/*
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS           INT [0] 512
	0 PIN_FLD_TEMPLATE        STR [0] "select X from /profile/wholesale 1,  /profile/customer_care where ( 2.F2.id = V2 and 2.F1.id = 1.F3.id ) "
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	1     PIN_FLD_CUSTOMER_CARE_INFO SUBSTRUCT [0] allocated 20, used 1
	2         PIN_FLD_PARENT        POID [0] NULL poid pointer
	0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 63542 0
	0 PIN_FLD_ARGS          ARRAY [3] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_OBJ    POID [0] NULL poid pointer
	0 PIN_FLD_RESULTS       ARRAY [*] allocated 20, used 0
	*/

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_check_csr_access_level", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_access_level input flist", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_access_level acnt info flist", acnt_info);

	csr_acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	/* Search profile of searched account */
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_srch_acnt , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp );

	PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_access_level search account profile input", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_access_level search account profile output", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	if (result_flist)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_access_level search account result_flist", result_flist);
		profile_pdp = (poid_t *)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_POID, 0, ebufp);
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Account profile_pdp", profile_pdp);
		profile_type = (char *)PIN_POID_GET_TYPE(profile_pdp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account profile_type:");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, profile_type);
		
		
		if ( strstr(profile_type, "wholesale" ) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account profile is Wholesale");
			profile_info = PIN_FLIST_SUBSTR_GET(result_flist, MSO_FLD_WHOLESALE_INFO, 0, ebufp);
		}
		else if ( strstr(profile_type, "customer_care" ) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account profile is Customer_care");
			profile_info = PIN_FLIST_SUBSTR_GET(result_flist, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
		}
		/* Get the Org information for that account. */
		if(profile_info)
		{
			parent_pdp = PIN_FLIST_FLD_GET(profile_info, PIN_FLD_PARENT, 0, ebufp );
			jv_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_JV_OBJ, 0, ebufp );
			dt_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_DT_OBJ, 0, ebufp );
			sdt_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_SDT_OBJ, 0, ebufp );
			lco_obj = PIN_FLIST_FLD_GET(profile_info, MSO_FLD_LCO_OBJ, 0, ebufp );
			searched_acnt_type = *(int32*)PIN_FLIST_FLD_GET(profile_info, PIN_FLD_CUSTOMER_TYPE, 0, ebufp );
		}
		else
			return;
	}
	
	/**************************************************************************
	*  Search wholesale profile csr
	**************************************************************************/
	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_srch_csr , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, csr_acnt_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/wholesale", -1, ebufp), ebufp);

	PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_access_level search CSR profile input", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_access_level search CSR profile output", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp );
	if (result_flist)
	{
		access_info = PIN_FLIST_ELEM_GET(result_flist, MSO_FLD_ACCESS_INFO, 0, 1, ebufp);
		if(access_info)
		{
			org_access_level = PIN_FLIST_FLD_GET(access_info, MSO_FLD_ORG_ACCESS_LEVEL, 1, ebufp );
			data_access_level = PIN_FLIST_FLD_GET(access_info, MSO_FLD_DATA_ACCESS_LEVEL, 1, ebufp );

			/* If Org access level exist then fetch Org access list */
			if(org_access_level)
			{
				if(*org_access_level == MSO_ORG_ACCESS_ALL)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ORG Access is ALL.");
					org_access_flag = 1;
				}
				else
				{
					/* Based on the Org Access level of CSR, determine the poid
						 that should be compared from customer org structure. */
					switch (*org_access_level)
					{
					  case MSO_ORG_ACCESS_SP_JV :
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ORG Access is SP/JV.");
						compare_poid = jv_obj;
						break;
					  case MSO_ORG_ACCESS_DT :
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ORG Access is DT.");
						compare_poid = dt_obj;
						break;
					  case MSO_ORG_ACCESS_SDT :
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ORG Access is SDT.");
						compare_poid = sdt_obj;
						break;
					  case MSO_ORG_ACCESS_LCO :
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ORG Access is LCO.");
						compare_poid = lco_obj;
						break;
					  default :
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ORG Access Info not Matching.");
					}
				}
				
				if(!org_access_flag && compare_poid)
				{
					while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(access_info, MSO_FLD_ORG_ACCESS_LIST,
							&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
					{
						org_obj = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_ORG_OBJ, 1, ebufp );
						/* If POID matches with one in the list then set access flag */
						if (PIN_POID_COMPARE(compare_poid, org_obj, 0, ebufp ) == 0)
						{
							org_access_flag = 1;
							break;
						}
					}
				}
				
				
			} // END: Org Access Level
			
			if(data_access_level)
			{					
				if(*data_access_level == MSO_DATA_ACCESS_GLOBAL)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Data Access is Global.");
					access_flag = 1;
				}
				else
				{
					/* Fetch the searched account details */
					fm_mso_get_account_info(ctxp, acnt_pdp, &acc_details, ebufp );
					tmp_flistp = PIN_FLIST_ELEM_GET(acc_details, PIN_FLD_NAMEINFO, 1, 0, ebufp );
					
					/* Based on the Data Access level of CSR, determine the city/state/area
						 that should be compared with searched account city/state/area. */
					switch (*data_access_level)
					{
					  case MSO_DATA_ACCESS_STATE :
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DATA Access is STATE.");
						acc_address = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATE, 0, ebufp );
						strcpy(compare_value,acc_address);
						break;
					  case MSO_DATA_ACCESS_CITY :
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DATA Access is CITY.");
						acc_address = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CITY, 0, ebufp );
						strcpy(compare_value,acc_address);
						break;
					  case MSO_DATA_ACCESS_AREA :
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DATA Access is AREA.");
						tmp_flistp = PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, 1, 0, ebufp );
						acc_address = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_AREA_NAME, 0, ebufp );
						strcpy(compare_value,acc_address);
						break;
					  case MSO_DATA_ACCESS_LOCATION :
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DATA Access is LOCATION.");
						tmp_flistp = PIN_FLIST_ELEM_GET(acnt_info, PIN_FLD_NAMEINFO, 1, 0, ebufp );
						acc_address = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_LOCATION_NAME, 0, ebufp );						
						strcpy(compare_value,acc_address);
						break;
					  default :
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Data Access Info not Matching.");
					}
				}
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Compare Value:");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, compare_value);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Access info flist", access_info);
				if(!access_flag && compare_value)
				{
					cookie = NULL;
					while((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(access_info, MSO_FLD_DATA_ACCESS_LIST,
							&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
					{
						access_value = PIN_FLIST_ELEM_GET(tmp_flistp, MSO_FLD_DATA_ACCESS, 0, 1, ebufp );
				
						/* If access_val (granted access value to CSR) matches customer's
							state/city/area the set access flag to 1*/
						if (strcmp(access_value, compare_value) == 0)
						{
							access_flag = 1;
							break;
						}
					}
				}
				
			}// Data Access Level
			
		}
		//business_type = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
		//if(access_flag && *business_type!=21000000 && *business_type!=11000000)
		//if(org_access_flag && access_flag && *business_type!=21000000)
		if(org_access_flag && access_flag)
		{
			if ( jv_obj && !(PIN_POID_IS_NULL(jv_obj)))
				fm_mso_get_account_info(ctxp, jv_obj, &jv_details, ebufp );
			else if ( dt_obj && !(PIN_POID_IS_NULL(dt_obj)))
				fm_mso_get_account_info(ctxp, dt_obj, &jv_details, ebufp );
			else if ( sdt_obj && !(PIN_POID_IS_NULL(sdt_obj)))
				fm_mso_get_account_info(ctxp, sdt_obj, &jv_details, ebufp );
			else if ( lco_obj && !(PIN_POID_IS_NULL(lco_obj)))
				fm_mso_get_account_info(ctxp, lco_obj, &jv_details, ebufp );
			if(jv_details)
			{
				business_type = PIN_FLIST_FLD_GET(jv_details, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
				tmp_business_type = *business_type;
				account_type = tmp_business_type/1000000;

				PIN_FLIST_FLD_COPY(jv_details, PIN_FLD_ACCOUNT_NO, profile_info, PIN_FLD_ACCOUNT_NO, ebufp );
				jv_nameinfo = PIN_FLIST_ELEM_GET(jv_details, PIN_FLD_NAMEINFO, 1, 1, ebufp);

				prof_nameinfo = PIN_FLIST_ELEM_ADD(profile_info, PIN_FLD_NAMEINFO, 1, ebufp);
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_FIRST_NAME, prof_nameinfo, PIN_FLD_FIRST_NAME, ebufp );
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_MIDDLE_NAME, prof_nameinfo, PIN_FLD_MIDDLE_NAME, ebufp );
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_LAST_NAME, prof_nameinfo, PIN_FLD_LAST_NAME, ebufp );
				PIN_FLIST_FLD_COPY(jv_nameinfo, PIN_FLD_COMPANY, prof_nameinfo, PIN_FLD_COMPANY, ebufp );
				PIN_FLIST_FLD_SET(prof_nameinfo, PIN_FLD_ACCOUNT_TYPE, &account_type, ebufp);
			}
			PIN_FLIST_DESTROY_EX(&jv_details, NULL);
			*ret_flist = PIN_FLIST_COPY(profile_info, ebufp);
			//PIN_FLIST_ELEM_COPY(result_flist,MSO_FLD_ACCESS_INFO,0,*ret_flist,MSO_FLD_ACCESS_INFO,0,ebufp );
		}
	}	

	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "test_001");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_check_csr_access_level return flist", *ret_flist);

	CLEANUP:
	return;
}


void
fm_mso_get_srvc_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*service_pdp = NULL;
	int64			db =1;
		

	int32			status_actve = 1;
	int32			status_inactve = 2;
	int32			srch_flag = 256;
	int32			srvc_type = -1;
	char			*template = "select X from /service where  F1.id = V1 ";


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_srvc_info", in_flistp );
	/* Get Lifecycle event template */
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV");
	/*Search Plans from purchased product*/
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, service_pdp, ebufp);
	
	if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_CATV))==0)
	{
		srvc_type = CATV;
	}
	else if ( service_pdp && (strcmp(PIN_POID_GET_TYPE(service_pdp),MSO_BB)) == 0)
	{
		srvc_type = BB;
	}


	if (srvc_type == CATV )
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_CATV_INFO, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_srvc_info SEARCH CATV info input flist", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_srvc_info SEARCH CATV info output flist", srch_out_flistp);
		arg_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
		*out_flistp = PIN_FLIST_SUBSTR_TAKE(arg_flist, MSO_FLD_CATV_INFO, 1, ebufp );

   	}
	else if (srvc_type == BB)
	{
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, MSO_FLD_BB_INFO, NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_srvc_info SEARCH BB info input flist", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_srvc_info SEARCH BB info input flist", srch_out_flistp);
		arg_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
		*out_flistp = PIN_FLIST_SUBSTR_TAKE(arg_flist, MSO_FLD_BB_INFO, 1, ebufp );
	}
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Kali End");
	return;
}


void
fm_mso_cust_get_plan_code(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_flds_iflist = NULL;
	pin_flist_t		*read_flds_oflist = NULL;

	poid_t			*plan_pdp = NULL;

	pin_cookie_t		cookie = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_plan_code", in_flistp );
	/* Get Lifecycle event template */

	plan_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
	if (plan_pdp)
	{
		read_flds_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_POID, plan_pdp, ebufp );
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_CODE, NULL, ebufp );
		PIN_FLIST_FLD_SET(read_flds_iflist, PIN_FLD_NAME, NULL, ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_plan_code input list", read_flds_iflist);
		PCM_OP(ctxp, PCM_OP_READ_FLDS , 0, read_flds_iflist, out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_flds_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling read fm_mso_cust_get_plan_code() ", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			PIN_FLIST_DESTROY_EX(&read_flds_oflist, NULL);
			return ;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_plan_code output list", *out_flistp);
	}
	return;
}

void
fm_mso_get_mso_cust_credentials(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_flistp = NULL;
	pin_flist_t		*srch_outflist = NULL;
	pin_flist_t		*args_flistp = NULL;

	poid_t			*srch_pdp = NULL;

	char			*template = "select X from /mso_customer_credential where F1.id = V1 ";

	int64			db = -1;
	int32			srch_flag = 256;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_mso_cust_credentials", ebufp);
		return;
	}
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_cust_credentials acnt_pdp", acnt_pdp);

	db = PIN_POID_GET_DB(acnt_pdp);
	search_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_cust_credentials search flist", search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &srch_outflist, ebufp);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_get_mso_cust_credentials: Error in search", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_outflist, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_mso_cust_credentials :search result flist", srch_outflist);
	
	*ret_flistp = PIN_FLIST_ELEM_TAKE(srch_outflist, PIN_FLD_RESULTS, 0, 1, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_outflist, NULL);

	return;
}

void
fm_mso_get_acntinfo_from_bill(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*search_flistp = NULL;
	pin_flist_t		*srch_outflist = NULL;
	pin_flist_t		*args_flistp = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*bill_pdp = NULL;

	char			*template = "select X from /account 1, /bill 2 where 1.F1 = 2.F2 and 2.F3 = V3  ";  
	char			*bill_no = NULL;

	int64			db = 1;
	int32			srch_flag = 256;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_acntinfo_from_bill", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acntinfo_from_bill acnt_pdp", in_flistp);

	bill_pdp = PIN_FLIST_FLD_GET(in_flistp,  PIN_FLD_POID, 1, ebufp );
	bill_no  = PIN_FLIST_FLD_GET(in_flistp,  PIN_FLD_BILL_NO, 1, ebufp );

	search_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp );
	if (bill_pdp)
	{
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, bill_pdp, ebufp);
	}
	else if (bill_no)
	{
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_NO, bill_no, ebufp);
	}

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acntinfo_from_bill search flist", search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &srch_outflist, ebufp);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, " fm_mso_get_acntinfo_from_bill: Error in search", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_outflist, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acntinfo_from_bill :search result flist", srch_outflist);
	
	*ret_flistp = PIN_FLIST_ELEM_TAKE(srch_outflist, PIN_FLD_RESULTS, 0, 1, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_outflist, NULL);

	return;
}

/********************************************
* function: fm_mso_set_bp_status()
*	This function searches mso_purchased_plan
*	with status passed in input and updates
*	the status to In Progress (1)
********************************************/
void
fm_mso_set_bp_status(
	pcm_context_t           *ctxp,
	poid_t                  *service_obj,
	poid_t                  *bp_obj,
	int32					old_status,
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
	int32           db = -1;
	int             search_flags = 512;
	// Pawan: 08-Dec-14: Removed the 3rd condition to avoid check on STATUS.
	char            search_template[100] = " select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3";
	//char            search_template[100] = " select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

        db = PIN_POID_GET_DB(service_obj);
	new_status = 1;	

        search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        search_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, service_obj, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, bp_obj, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &old_status, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, (char *)NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_bp_status search mso purchased plan input list", search_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching MSO Purchased plan", ebufp);
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_bp_status search mso purchased plan output list", search_res_flistp);

        result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if ( !result_flistp )
        {
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
                return;
        }
	new_status = MSO_PROV_IN_PROGRESS;
        mso_pp_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp);
        updsvc_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(updsvc_inflistp, PIN_FLD_POID, mso_pp_obj, ebufp);
        PIN_FLIST_FLD_SET(updsvc_inflistp, PIN_FLD_STATUS, &new_status, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_bp_status mso purchase plan status update input list", updsvc_inflistp);
        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, updsvc_inflistp, &updsvc_outflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&updsvc_inflistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_WRITE_FLDS", ebufp);
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
                PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_set_bp_status mso purchase plan status update output list", updsvc_outflistp);
        PIN_FLIST_DESTROY_EX(&updsvc_outflistp, NULL);
        PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	return;
}	

/**************************************************
* Function: fm_mso_create_ar_limit()
*
*
*
***************************************************/
void
fm_mso_create_ar_limit(
        pcm_context_t           *ctxp,
        poid_t                  *account_obj,
        pin_flist_t             **ar_limit,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *i_flistp = NULL;
        pin_flist_t             *r_flistp = NULL;

        poid_t                  *ar_pdp = NULL;
        //pin_decimal_t         *cr_limit = NULL;
        pin_decimal_t           *zero = NULL;
        int64                   db = -1;
        time_t                  now = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_create_ar_limit", ebufp);
                return;
        }
        db = PIN_POID_GET_DB(account_obj);
        now = pin_virtual_time((time_t *)NULL);
        zero  = pbo_decimal_from_str("0.0",ebufp);
        //Create the input flist
        i_flistp = PIN_FLIST_CREATE(ebufp);
        ar_pdp = PIN_POID_CREATE(db, "/mso_ar_limit", -1, ebufp);
        PIN_FLIST_FLD_PUT(i_flistp,
                PIN_FLD_POID, (void *)ar_pdp, ebufp);
        /*PIN_FLIST_FLD_SET(i_flistp,
                MSO_FLD_CR_ADJ_DATE, (void *)&now, ebufp);
        PIN_FLIST_FLD_SET(i_flistp,
                MSO_FLD_DR_ADJ_DATE, (void *)&now, ebufp); */
        PIN_FLIST_FLD_SET(i_flistp,
                PIN_FLD_ACCOUNT_OBJ, (void *)account_obj, ebufp);
        /*PIN_FLIST_FLD_MOVE(*ar_limit, MSO_FLD_CR_ADJ_LIMIT,
                i_flistp, MSO_FLD_CR_ADJ_LIMIT, ebufp);
        PIN_FLIST_FLD_MOVE(*ar_limit, MSO_FLD_DR_ADJ_LIMIT,
                i_flistp, MSO_FLD_DR_ADJ_LIMIT, ebufp);
        PIN_FLIST_FLD_MOVE(*ar_limit, PIN_FLD_TYPE,
                i_flistp, PIN_FLD_TYPE, ebufp); */
        PIN_FLIST_FLD_SET(i_flistp,
                MSO_FLD_CURRENT_MONTH, (void *)&now, ebufp);
        PIN_FLIST_FLD_MOVE(*ar_limit, MSO_FLD_MONTHLY_ADJ_LIMIT,
                i_flistp, MSO_FLD_MONTHLY_ADJ_LIMIT, ebufp);
        PIN_FLIST_FLD_MOVE(*ar_limit, MSO_FLD_ADJ_LIMIT,
                i_flistp, MSO_FLD_ADJ_LIMIT, ebufp);
        PIN_FLIST_FLD_SET(i_flistp,
                MSO_FLD_ADJ_CURRENT_VALUE, (void *)zero, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_create_ar_limit: Error creating input flist", ebufp);
                goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_create_ar_limit: Input Flist", i_flistp);
        PCM_OP(ctxp, PCM_OP_CREATE_OBJ,
                0, i_flistp, &r_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_create_ar_limit: Error calling creating /mso_ar_limit object", ebufp);
                goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_create_ar_limit: Output Flist", r_flistp);

        cleanup:
                PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
                PIN_FLIST_DESTROY_EX(ar_limit, NULL);
                PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
                return;

}


/********************************************
* function: fm_mso_get_credit_profile()
*	
********************************************/
void
fm_mso_get_credit_profile(
	pcm_context_t		*ctxp,
	int32			elem_id,
	pin_flist_t		**credit_limit_array,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*search_flistp = NULL;
	pin_flist_t		*search_res_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;

	poid_t			*search_pdp = NULL;

	int32			search_flags = 256;
	int64			db = 1;

	char			*template = "selece X from /config/credit_profile where F1.type = V1 " ;

	void			*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	search_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID,  PIN_POID_CREATE(db, "/config/credit_profile", -1, ebufp), ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_credit_profile input list", search_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching MSO Purchased plan", ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_credit_profile output list", search_res_flistp);

	result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if ( !result_flistp )
	{
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		return;
	}
	if (elem_id == -1 )
	{
		*credit_limit_array = PIN_FLIST_COPY(result_flistp, ebufp);
	}
	else
	{
		vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(result_flistp, PIN_FLD_PROFILES, elem_id, 1, ebufp);
		if (vp)
		{
			*credit_limit_array = PIN_FLIST_COPY(vp, ebufp);
		}
	}

	PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	return;
}	



/**************************************************
* Function: 	fm_mso_read_bal_grp()
* Owner:        Gautam Adak
* Decription:   For getting Account & srvc poid 
*   of a provided bal grp
*
* 
* 
***************************************************/
void
fm_mso_get_srvc_of_bal_grp(
	pcm_context_t		*ctxp,
	poid_t			*in_flist,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*arg_flist = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*bal_grp_pdp = NULL;

	char			*template_2 = "select X from /account where F1.id = V1 ";
	char			*template_1 = "select X from /service where F1.id = V1 ";
	char			*srvc_type = NULL;

	int64			db = 1;

	int32			srch_flags = 256;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_srvc_of_bal_grp", ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_srvc_of_bal_grp input flist");

	bal_grp_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);

	srch_iflistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/search", -1, ebufp)), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template_1, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BAL_GRP_OBJ, bal_grp_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SERVICE_LEVEL fm_mso_get_srvc_of_bal_grp read input list", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SERVICE_LEVEL fm_mso_get_srvc_of_bal_grp READ output list", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto CLEANUP;
	}
	else if (result_flist)
	{
		*ret_flist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, *ret_flist, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_ACCOUNT_OBJ, *ret_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, *ret_flist, PIN_FLD_SERVICE_OBJ, ebufp);
		goto CLEANUP;
	}
	else
	{
		PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template_2, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ACCOUNT_LEVEL fm_mso_get_srvc_of_bal_grp read input list", srch_iflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_RESET(ebufp);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ACCOUNT_LEVEL fm_mso_get_srvc_of_bal_grp READ output list", srch_oflistp);
	}

	result_flist = PIN_FLIST_ELEM_GET(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
	
	if (result_flist)
	{
		*ret_flist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, *ret_flist, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, *ret_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);
		goto CLEANUP;
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_read_bill()
* Owner:        Gautam Adak
* Decription:   For getting Account & srvc poid 
*   of a provided bal grp
*
* 
* 
***************************************************/
void
fm_mso_read_bill(
	pcm_context_t		*ctxp,
	poid_t			*in_flist,
	pin_flist_t		**ret_flist,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_iflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*arg_flist = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*bill_pdp = NULL;

	char			*template_1 = "select X from /bill where F1.id = V1 ";
	char			*srvc_type = NULL;

	int64			db = 1;

	int32			srch_flags = 256;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_read_bill", ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_read_bill input flist");

	bill_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_POID, 1, ebufp);
	if (!bill_pdp || strcmp(PIN_POID_GET_TYPE(bill_pdp),"/bill")!=0 )
	{
		bill_pdp = PIN_FLIST_FLD_GET(in_flist, PIN_FLD_BILL_OBJ, 1, ebufp);
	}

	srch_iflistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, (PIN_POID_CREATE(db, "/search", -1, ebufp)), ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template_1, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, bill_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_read_bill read input list", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_read_bill READ output list", srch_oflistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto CLEANUP;
	}
	result_flist = PIN_FLIST_ELEM_GET(srch_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
	
	if (result_flist)
	{
		*ret_flist = PIN_FLIST_COPY(result_flist, ebufp);
		goto CLEANUP;
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}
/**************************************************
* Function: 	fm_mso_get_channels_from_prov_tag 
* Owner:    	 		
* Decription:   Get channel details from provisioning Tag	
* Input    
*          provisioning Tag 
* output   channnel detail
***************************************************/
void
fm_mso_get_channels_from_prov_tag(
        pcm_context_t           *ctxp,
        char*                   prov_tag,
        pin_flist_t             **channel_details,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *channels_array_flistp = NULL;
        pin_flist_t             *result_channels_flistp = NULL;
        pin_flist_t             *channels_oflistp = NULL;
        poid_t                  *srch_pdp = NULL;
        int32                   srch_flag = 512;
        int64                   db = 1;
 	char *template = "select X from /mso_cfg_catv_pt_channel_map  where F1 = V1" ;

	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

       	arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp) ;
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, prov_tag, ebufp);

       	result_channels_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);
	channels_array_flistp = PIN_FLIST_ELEM_ADD(result_channels_flistp, MSO_FLD_CATV_CHANNELS, PIN_ELEMID_ANY, ebufp) ;
        PIN_FLIST_FLD_SET(channels_array_flistp, MSO_FLD_CHANNEL_ID, NULL, ebufp );
        PIN_FLIST_FLD_SET(channels_array_flistp, MSO_FLD_CHANNEL_NAME, NULL, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Channels input list", srch_flistp);
       	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &channels_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
               	return;
	}
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Channels output flist", channels_oflistp);
	*channel_details=channels_oflistp;
	return;
}


/**************************************************
* Function: 	fm_mso_convert_ebuf_to_msg 
* Owner:    	 Gautam Adak		
* Decription:   
* Input    
*          
* output   
***************************************************/
int32
fm_mso_convert_ebuf_to_msg(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistp,
	char			*error_code,
	char			*error_descr,
	pin_errbuf_t		*ebufp)
{
	int32			error_location     ;
	int32			error_pin_errclass ;
	int32			error_pin_err      ;
	int32			error_field        ;
	int32			error_rec_id       ;
	int32			error_reserved     ;
	int32			error_line_no      ;

	//char			err_msg[255];
	pin_flist_t		*array1_flistp = NULL;
	pin_flist_t		*array2_flistp = NULL;
	
	char			*namep = NULL;
	char			*descrp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		error_location     = ebufp->location;
		error_pin_errclass = ebufp->pin_errclass;
		error_pin_err      = ebufp->pin_err;
		error_field        = ebufp->field;
		error_rec_id       = ebufp->rec_id;
		error_reserved     = ebufp->reserved;
		error_line_no      = ebufp->line_no;


	}

	if (error_location == PIN_ERRLOC_FM_COMM ) //for commission model
	{
		switch (error_pin_err)
		{
		   case 61000 :
			strcpy(error_descr, "COMMISSION - Commission Model or Commission Service is not valid");
			break;
		   case 61001 :
			strcpy(error_descr, "COMMISSION - Balance Bucket not found");
			break;
		   case 61002 :
			strcpy(error_descr, "COMMISSION - Commission Bucket not found");
			break;
		   case 61003 :
			strcpy(error_descr, "COMMISSION - DTR Commission Bucket not found");
			break;
		   case 61004 :
			strcpy(error_descr, "COMMISSION - No Active Agreement(LCO)");
			break;
		   case 61005 :
			strcpy(error_descr, "COMMISSION - No Active Agreement (DTR)");
			break;
		   case 61006 :
			strcpy(error_descr, "LCO COMMISSION - LCO Balance Bucket not having balance");
			break;
		   case 61007 :
			strcpy(error_descr, "LCO COMMISSION - LCO Balance Bucket having insufficient balance");
			break;
		   case 61008 :
			strcpy(error_descr, "LCO COMMISSION - Credit Limit Exceed for LCO");
			break;
		   case 61009 :
			strcpy(error_descr, "LCO COMMISSION - No Credit Limit for LCO");
			break;
		   case 61010 :
			strcpy(error_descr, "LCO COMMISSION - No Credit Limit for LCO");
			break;
		   case 61011 :
			strcpy(error_descr, "LCO COMMISSION - Invalid Commission Model");
			break;
		   case 61012 :
			strcpy(error_descr, "LCO COMMISSION - Commission Model is not defined");
			break;
		   case 61013 :
			strcpy(error_descr, "LCO COMMISSION - Commission Service is invalid");
			break;
		   case 61014 :
			strcpy(error_descr, "LCO COMMISSION - LCO Account does not exists");
			break;
		   case 61015 :
			strcpy(error_descr, "LCO COMMISSION - Insufficient LCO Balance");
			break;

		   case 61050 :
			strcpy(error_descr, "COMMISSION - Commission Model is not defined");
			break;
		   case 61051 :
			strcpy(error_descr, "COMMISSION - Commission Service is invalid");
			break;
		   case 61052 :
			strcpy(error_descr, "COMMISSION - LCO Account does not exists");
			break;


		   default :
		        strcpy(error_descr, "LCO COMMISSION");
			break;
		}
		sprintf(error_code, "%d", error_pin_err);
		return PIN_ERRLOC_FM_COMM;
	}
	
	else if (error_location == PIN_ERRLOC_FM)
	{
		switch (error_pin_err)
		{
		   case PIN_ERR_BAD_VALUE :
			if (error_field == PIN_FLD_END_T)
			{
				strcpy(error_descr, "Backdating is not allowed before service effective date");
			/*** Pavan Bellala - 27/07/2015 Added new if-else condition *****/
			} else if(error_reserved == PIN_STATUS_INACTIVE ){
				
				strcpy(error_descr, "Can not add a deal into an inactive service/account");
			} else if(error_field == PIN_FLD_LOGIN){
				if(i_flistp != NULL)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"fm_mso_convert_ebuf_to_msg:input",i_flistp);
					array1_flistp = PIN_FLIST_ELEM_GET(i_flistp,PIN_FLD_FIELD,0,1,ebufp);
					if(array1_flistp!= NULL){
						array2_flistp = PIN_FLIST_ELEM_GET(array1_flistp,PIN_FLD_FIELD,0,1,ebufp);
						if(array2_flistp != NULL){
							descrp = PIN_FLIST_FLD_GET(array2_flistp,PIN_FLD_DESCR,1,ebufp);
							namep = PIN_FLIST_FLD_GET(array2_flistp,PIN_FLD_NAME,1,ebufp);
							if(descrp && namep){
								strcpy(error_descr, "Device id-");
								strcat(error_descr,descrp);
								strcat(error_descr," - ");
								strcat(error_descr,namep);
							}					
						}
					}
				} else {
					strcpy(error_descr, "Duplicate value in device");
				}
			}
		    break;
		    default :
			strcpy(error_descr, "Unhandled error");
			break;
		}
		sprintf(error_code, "%5d", error_pin_err);
		return PIN_ERRLOC_FM;
	}

	return 0;
}


int
fm_mso_fetch_purchased_plan(
        pcm_context_t           *ctxp,
	int			*flags,
        pin_flist_t		*in_flistp,
        poid_t                  **o_poidpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t	*search_flistp = NULL;
        pin_flist_t	*search_res_flistp = NULL;
        pin_flist_t	*args_flistp = NULL;
        pin_flist_t	*result_flistp = NULL;

	poid_t          *search_pdp = NULL;
	poid_t		*pdp = NULL;
	int32           db = -1;
        int             search_flags = 512;
        //char            search_template[100] = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 ";
        char            search_template[100] = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
	int		vp = 0;

	int		*plan_typep = NULL;
	int		status = 0;
	int		rv = 0;
	int32		status_act = MSO_PROV_ACTIVE;
	int32		*action_flag = NULL;
	char            search_template1[100] = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3";	

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return PIN_RESULT_FAIL;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_fetch_purchased_plan:input flistp",in_flistp);
	action_flag = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_FLAGS,1,ebufp);
	
        db = PIN_POID_GET_DB((poid_t*)PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_POID,1,ebufp));
        vp = *(int*)flags;

        search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        search_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	if( action_flag && *action_flag == DOC_BB_UPDATE_CAP)
	{
	   PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template1, ebufp);
	   args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
	   PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status_act, ebufp);
	}
	else
	{
           PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);
	}
        args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
	if(vp == MSO_FLAG_SRCH_BY_ACCOUNT)	
	{
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, args_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
	} else if(vp == MSO_FLAG_SRCH_BY_SERVICE)
	{
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, args_flistp,PIN_FLD_SERVICE_OBJ,ebufp);
	}
	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
	plan_typep = PIN_FLIST_FLD_GET(in_flistp,MSO_FLD_PLAN_TYPE,1,ebufp);
	if(plan_typep){
		if(*plan_typep == PLAN_TYPE_SUBS){
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_DESCR,MSO_BB_BP,ebufp);
		}else {
			PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_DESCR,"",ebufp);
		}
	}
	//args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS,3, ebufp);
	//status = MSO_PROV_ACTIVE;
	//PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_STATUS,&status,ebufp);		
        result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_fetch_purchased_plan search mso purchased plan input list", search_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching MSO Purchased plan", ebufp);
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		rv = PIN_RESULT_FAIL;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_fetch_purchased_plan search mso purchased plan output list", search_res_flistp);
        result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if (result_flistp == NULL ){
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
		rv = PIN_RESULT_FAIL;
        } else {
		pdp = PIN_FLIST_FLD_TAKE(result_flistp,PIN_FLD_POID,0,ebufp);
		rv = PIN_RESULT_PASS;
		*o_poidpp = PIN_POID_COPY(pdp,ebufp);
		PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	}
	return rv;

}


void
fm_mso_utils_fetch_ar_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
	poid_t			*acct_pdp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)

{

	pin_flist_t	*ar_flistp = NULL;
	pin_flist_t	*aro_flistp = NULL;
	pin_flist_t	*billinfo_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;

	time_t		*last_bill_tp = NULL;
	int		format = PIN_ITEM_SEARCH_FORMAT_ITEMS;
	int		srch_type = PIN_ITEM_SEARCH_TYPE_HIERARCHY;
	int		rec_id = 0;

	pin_decimal_t	*adj_amtp = NULL;
	pin_decimal_t	*pymt_amtp = NULL;
	pin_decimal_t	*dis_amtp = NULL;
	pin_decimal_t	*total_pymtp = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t	*total_adjp = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t	*total_disp = pbo_decimal_from_str("0.0",ebufp);

	pin_cookie_t	cookie = NULL;
	char		msg[100];

	char		*search_template = "select X from /item where F1 = V1 and F2.type like V2 and F3 >= V3 ";
	int		search_flags = 256;
	poid_t		*p_pdp = NULL;
	poid_t		*search_pdp = NULL;
	int64		db = 0;


        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_utils_fetch_payment_details", ebufp);
                return;
        }

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,"fm_mso_utils_fetch_ar_details:input poid",acct_pdp);
	*o_flistpp = PIN_FLIST_CREATE(ebufp);

	db = PIN_POID_GET_DB(acct_pdp);

	if(in_flistp!= NULL){
		billinfo_flistp = PIN_FLIST_ELEM_GET(in_flistp,PIN_FLD_BILLINFO,0,1,ebufp);
		if(billinfo_flistp != NULL){
			last_bill_tp = PIN_FLIST_FLD_GET(billinfo_flistp,PIN_FLD_LAST_BILL_T,1,ebufp);
			if(last_bill_tp != NULL)
			{
				/*** Fetch the payment details ****/
				ar_flistp = PIN_FLIST_CREATE(ebufp);
				/****
				PIN_FLIST_FLD_SET(ar_flistp,PIN_FLD_POID,acct_pdp,ebufp);
				PIN_FLIST_FLD_SET(ar_flistp,PIN_FLD_RESULT_FORMAT,&format,ebufp);
				PIN_FLIST_FLD_SET(ar_flistp,PIN_FLD_SEARCH_TYPE,&srch_type,ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(ar_flistp,PIN_FLD_ARGS,0,ebufp);
				PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,acct_pdp,ebufp);

				PIN_FLIST_FLD_PUT(args_flistp,PIN_FLD_POID,
			  	(poid_t*)PIN_POID_CREATE(PIN_POID_GET_DB(acct_pdp),PIN_OBJ_TYPE_ITEM_PAYMENT,-1,ebufp),ebufp);
				PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_START_T,last_bill_tp,ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(ar_flistp,PIN_FLD_RESULTS,0,ebufp);
				PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ITEM_TOTAL,total_pymtp,ebufp);
				****/
				search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
				PIN_FLIST_FLD_PUT(ar_flistp, PIN_FLD_POID, search_pdp, ebufp);
				PIN_FLIST_FLD_SET(ar_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
				PIN_FLIST_FLD_SET(ar_flistp, PIN_FLD_INDEX_NAME, "I_ITEM_ACCOUNT_OBJ__ID", ebufp);
				PIN_FLIST_FLD_SET(ar_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(ar_flistp, PIN_FLD_ARGS, 1, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ,acct_pdp , ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(ar_flistp, PIN_FLD_ARGS, 2, ebufp);
				p_pdp = PIN_POID_CREATE(db, "/item/payment%", -1, ebufp);
				PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, p_pdp, ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(ar_flistp, PIN_FLD_ARGS, 3, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EFFECTIVE_T,last_bill_tp , ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(ar_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"fm_mso_utils_fetch_ar_details:input for PCM_OP_PYMT_ITEM_SEARCH (pymt)",ar_flistp);
				//PCM_OP(ctxp, PCM_OP_PYMT_ITEM_SEARCH, 0, ar_flistp, &aro_flistp, ebufp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, ar_flistp, &aro_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"fm_mso_utils_fetch_ar_details:out for PCM_OP_PYMT_ITEM_SEARCH(pymt)",aro_flistp);
				PIN_FLIST_DESTROY_EX(&ar_flistp,NULL);
				if(aro_flistp!= NULL)
				{
					rec_id = 0, cookie = NULL;
					while((args_flistp = PIN_FLIST_ELEM_GET_NEXT (aro_flistp,
						PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL){
						pymt_amtp = PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_ITEM_TOTAL,1,ebufp);
						if(!pbo_decimal_is_null(pymt_amtp,ebufp)){
							pbo_decimal_add_assign(total_pymtp,pymt_amtp,ebufp);
						}	
					}
					sprintf(msg,"total payment : %s",pbo_decimal_to_str(total_pymtp,ebufp));
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
				} else {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"no payment results");
				}

				PIN_FLIST_DESTROY_EX(&aro_flistp,NULL);


				/***** Fetch adjustment amount ********/
				ar_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ar_flistp,PIN_FLD_POID,acct_pdp,ebufp);
				PIN_FLIST_FLD_SET(ar_flistp,PIN_FLD_RESULT_FORMAT,&format,ebufp);
				PIN_FLIST_FLD_SET(ar_flistp,PIN_FLD_SEARCH_TYPE,&srch_type,ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(ar_flistp,PIN_FLD_ARGS,0,ebufp);
				PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,acct_pdp,ebufp);

				PIN_FLIST_FLD_PUT(args_flistp,PIN_FLD_POID,
				(poid_t*)PIN_POID_CREATE(PIN_POID_GET_DB(acct_pdp),PIN_OBJ_TYPE_ITEM_ADJUSTMENT,-1,ebufp),ebufp);
				PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_START_T,last_bill_tp,ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(ar_flistp,PIN_FLD_RESULTS,0,ebufp);
				PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ITEM_TOTAL,total_adjp,ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"fm_mso_utils_fetch_ar_details:input for PCM_OP_PYMT_ITEM_SEARCH(adj)",ar_flistp);
				PCM_OP(ctxp, PCM_OP_PYMT_ITEM_SEARCH, 0, ar_flistp, &aro_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"fm_mso_utils_fetch_ar_details:out for PCM_OP_PYMT_ITEM_SEARCH(adj)",aro_flistp);
				PIN_FLIST_DESTROY_EX(&ar_flistp,NULL);
				if(aro_flistp!= NULL)
				{
					rec_id = 0, cookie = NULL;
					while((args_flistp = PIN_FLIST_ELEM_GET_NEXT (aro_flistp,
						PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL){
						adj_amtp = PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_ITEM_TOTAL,1,ebufp);
						if(!pbo_decimal_is_null(adj_amtp,ebufp)){
							pbo_decimal_add_assign(total_adjp,adj_amtp,ebufp);
						}	
					}
					sprintf(msg,"total adjustment : %s",pbo_decimal_to_str(total_adjp,ebufp));
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
				} else {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"no adjustment results");
				}

				PIN_FLIST_DESTROY_EX(&aro_flistp,NULL);
			
				/*****  Fetch disputeed amount ****/
				ar_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(ar_flistp,PIN_FLD_POID,acct_pdp,ebufp);
				PIN_FLIST_FLD_SET(ar_flistp,PIN_FLD_RESULT_FORMAT,&format,ebufp);
				PIN_FLIST_FLD_SET(ar_flistp,PIN_FLD_SEARCH_TYPE,&srch_type,ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(ar_flistp,PIN_FLD_ARGS,0,ebufp);
				PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,acct_pdp,ebufp);

				PIN_FLIST_FLD_PUT(args_flistp,PIN_FLD_POID,
				(poid_t*)PIN_POID_CREATE(PIN_POID_GET_DB(acct_pdp),PIN_OBJ_TYPE_ITEM_DISPUTE,-1,ebufp),ebufp);
				PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_START_T,last_bill_tp,ebufp);
				args_flistp = PIN_FLIST_ELEM_ADD(ar_flistp,PIN_FLD_RESULTS,0,ebufp);
				PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ITEM_TOTAL,total_disp,ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"fm_mso_utils_fetch_ar_details:input for PCM_OP_PYMT_ITEM_SEARCH(disp)",ar_flistp);
				PCM_OP(ctxp, PCM_OP_PYMT_ITEM_SEARCH, 0, ar_flistp, &aro_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
						"fm_mso_utils_fetch_ar_details:out for PCM_OP_PYMT_ITEM_SEARCH(disp)",aro_flistp);
				PIN_FLIST_DESTROY_EX(&ar_flistp,NULL);
				if(aro_flistp!= NULL)
				{
					rec_id = 0, cookie = NULL;
					while((args_flistp = PIN_FLIST_ELEM_GET_NEXT (aro_flistp,
						PIN_FLD_RESULTS, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL){
						dis_amtp = PIN_FLIST_FLD_GET(args_flistp,PIN_FLD_ITEM_TOTAL,1,ebufp);
						if(!pbo_decimal_is_null(dis_amtp,ebufp)){
							pbo_decimal_add_assign(total_disp,dis_amtp,ebufp);
						}	
					}
					sprintf(msg,"total dispute : %s",pbo_decimal_to_str(total_disp,ebufp));
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
				} else {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"no dispute results");
				}

				PIN_FLIST_DESTROY_EX(&aro_flistp,NULL);
			}
			
		}

	}
	PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_AMOUNT_ORIGINAL_PAYMENT,total_pymtp,ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_AMOUNT_ADJUSTED,total_adjp,ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp,PIN_FLD_DISPUTED,total_disp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_utils_fetch_ar_details:out flistp",*o_flistpp);
	return;	
}

void 
fm_mso_get_pref_bdom (
        pcm_context_t   *ctxp,
        char            *area,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t      *srch_i_flistp = NULL;
        pin_flist_t      *srch_o_flistp = NULL;
	pin_flist_t      *arg_flistp = NULL;
        pin_flist_t      *result_flistp = NULL;
	pin_flist_t      *code_info_flistp = NULL;
        poid_t           *srch_pdp = NULL;
        int32            srch_flag = 768;
	int32		 actg_dom = -1; 
        int64            db = 1;
	int32		 status = 10100;
	char		 *state_code = NULL;
        char             *template = "select X from /mso_cfg_bdom_map where F1 = V1 and F2 = V2 ";
	void		 *vp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_get_pref_bdom");
                return;
        }
	
	fm_mso_get_state_city_area_code(ctxp, area, NULL, &code_info_flistp, ebufp );
	
	if (code_info_flistp && (PIN_FLIST_FLD_GET(code_info_flistp, PIN_FLD_ERROR_CODE, 1, ebufp)))
	{
          PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting state:fm_mso_get_pref_bdom", ebufp);
          goto CLEANUP;;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state_city_area_code", code_info_flistp);
	state_code = (char*)PIN_FLIST_FLD_GET(code_info_flistp, PIN_FLD_STATE, 1, ebufp);
	
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        srch_i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_STATE_NAME, state_code, ebufp);
		
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_STATUS, &status, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACTG_CYCLE_DOM, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_get_pref_bdom SEARCH input flist",srch_i_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp, &srch_o_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
          PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH:fm_mso_get_pref_bdom", ebufp);
          goto CLEANUP;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_get_pref_bdom SEARCH out flist",srch_o_flistp);
	
        result_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if (result_flistp)
        {	
		vp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_ACTG_CYCLE_DOM, 1, ebufp);
	}	

	if(vp)
		actg_dom = *(int32 *)vp;
	else
		actg_dom = 4;
		
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_PREF_DOM, &actg_dom, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_pref_bdom output flist", *r_flistpp);

    	CLEANUP:
	PIN_ERR_LOG_MSG(3, "Check 1");
	PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
	PIN_ERR_LOG_MSG(3, "Check 2");
    	PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);
	PIN_ERR_LOG_MSG(3, "Check 3");
    	return ;
}

int
fm_mso_purch_prod_status (
	pcm_context_t   *ctxp,
        pin_flist_t     *in_flistp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)

{
	pin_flist_t 	*purch_prod_iflistp = NULL;
	pin_flist_t	*bal_flistp = NULL;
	poid_t		*pdp = NULL;
	int64           db_no = 0;
	poid_t		*search_pdp = NULL;
	int32           search_flags = SRCH_DISTINCT;
	pin_flist_t	*tmp_flistp = NULL;
	char purc_template[500] = "select X from /purchased_product  where F1 = V1 and F2 = V2 ";
	poid_t		*purch_poid = NULL;	
	int32		canc_status = 3;	
	pin_flist_t	*purch_prod_oflistp = NULL;
	int32		count = 0;
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_purch_prod_status");
                return;
        }
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Rate pol post rating input flist", in_flistp);
	bal_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_BAL_IMPACTS, 0, 1, ebufp );
	if(bal_flistp)
	purch_poid = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
	purch_prod_iflistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
        db_no = PIN_POID_GET_DB(pdp);
        search_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(purch_prod_iflistp , PIN_FLD_POID, (void *)search_pdp, ebufp);
	PIN_FLIST_FLD_SET(purch_prod_iflistp,PIN_FLD_FLAGS,&search_flags,ebufp);
        PIN_FLIST_FLD_SET(purch_prod_iflistp,PIN_FLD_TEMPLATE,purc_template,ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD (purch_prod_iflistp, PIN_FLD_ARGS, 1, ebufp);	
	if(purch_poid)
	{
		PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, purch_poid, ebufp);
	}
	else{
		count = -1;
		goto CLEANUP;	
	}
	tmp_flistp = PIN_FLIST_ELEM_ADD (purch_prod_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, &canc_status, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD (purch_prod_iflistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH input flist", purch_prod_iflistp);
    	PCM_OP(ctxp, PCM_OP_SEARCH, 0, purch_prod_iflistp, &purch_prod_oflistp, ebufp);
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH output flist", purch_prod_oflistp);
    	PIN_FLIST_DESTROY_EX(&purch_prod_iflistp, ebufp);	
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                        "Error while calling purchased product search");
              	goto CLEANUP; 
        }
	if(PIN_FLIST_ELEM_COUNT(purch_prod_oflistp, PIN_FLD_RESULTS, ebufp) > 0) {
	    count = 1;
	}
	*r_flistpp = PIN_FLIST_COPY(purch_prod_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "r_flistpp is:", *r_flistpp);
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&purch_prod_oflistp, NULL);
	return count;
}

/*******************************************************************
 * fm_mso_catv_pt_pkg_type() 
 *
 * This function retrieves product package type from config PT and 
 * returns the same. 
 *******************************************************************/
int32
fm_mso_catv_pt_pkg_type(
	pcm_context_t		*ctxp,
	poid_t			*prd_pdp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	char                    template2[1024] = "select X from /mso_cfg_catv_pt 1, /product 2 where 1.F1 = 2.F2 and 2.F3 = V3 ";
	char			*ptp = "";
	int32			srch_flag = 768;
	int32			pkg_type = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, 
	PIN_POID_CREATE(PIN_POID_GET_DB(prd_pdp), "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, ptp,ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, ptp,ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, prd_pdp, ebufp);

        r_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_catv_pt_pkg_type input flist", srch_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_catv_pt_pkg_type : error getting Package_type", ebufp);
		goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_catv_pt_pkg_type output flist", srch_out_flistp);

	r_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (r_flistp)
	{	
                pkg_type = *(int32 *)PIN_FLIST_FLD_GET(r_flistp, MSO_FLD_PKG_TYPE, 0, ebufp);
        }

CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        return pkg_type;
}

/*******************************************************************
 * fm_mso_catv_conn_type() 
 *
 * This function retrieves CATV service connection type and 
 * returns the same. 
 *******************************************************************/
int32
fm_mso_catv_conn_type(
	pcm_context_t           *ctxp,
	poid_t			*srv_pdp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *rfld_iflistp = NULL;
	pin_flist_t             *rfld_oflistp = NULL;
	pin_flist_t             *temp_flistp = NULL;
	int32                   conn_type = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	rfld_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID, srv_pdp, ebufp);
	temp_flistp = PIN_FLIST_SUBSTR_ADD(rfld_iflistp, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_CONN_TYPE, &conn_type, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_catv_conn_type: Read Service In flist", rfld_iflistp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp, &rfld_oflistp, ebufp);

	PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_catv_conn_type error: Read Flds Error", ebufp);
		conn_type = -1;
		goto CLEANUP;
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_catv_conn_type: Read Service Out flist", rfld_oflistp);

	temp_flistp = PIN_FLIST_SUBSTR_GET(rfld_oflistp, MSO_FLD_CATV_INFO, 1, ebufp);
	if (temp_flistp)
	{
		conn_type = *(int32 *)PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_CONN_TYPE, 0, ebufp);
	}
	else
	{
		conn_type = -1;
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
	return conn_type;
}

/**************************************************************************
 * fm_mso_utils_get_catv_children()
 *
 * For getting CATV child service objects 
 **************************************************************************/
void fm_mso_utils_get_catv_children(
	pcm_context_t		*ctxp,
	poid_t			*act_pdp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32			flags = 256;
	int64			db = 0;
	int64			id = -1;
	int32			child_con_type = 2;
	char			*s_template = " select X from /service/catv where F1 = V1 and F2 = V2 and F3 = V3 ";
	int32			serv_status = 10100;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	db = PIN_POID_GET_DB(act_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &serv_status, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	args_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CONN_TYPE, &child_con_type, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Output flist:",*o_flistpp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}

/**************************************************************************
 * fm_mso_utils_get_catv_main()
 *
 * For getting CATV main service object 
 **************************************************************************/
void fm_mso_utils_get_catv_main(
	pcm_context_t		*ctxp,
	poid_t              *act_pdp,
	pin_flist_t		    **o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32			flags = 256;
	int64			db = 0;
	int64			id = -1;
	int32			main_conn_type = 0;
	char			*s_template = " select X from /service/catv where F1 = V1 and F2 = V2 and F3 = V3 ";
	int32			serv_status = 10100;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	db = PIN_POID_GET_DB(act_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &serv_status, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	args_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_CATV_INFO, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CONN_TYPE, &main_conn_type, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Output flist:",*o_flistpp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}

/**************************************************
* Function  : fm_mso_get_product_priority() 
* Decription: function to check weather product is
*             FUP TOPUP product or not 
*
***************************************************/
int32
fm_mso_get_product_priority(
	pcm_context_t           *ctxp,
	poid_t			*prod_pdp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	
	pin_flist_t     *r_in_flistp = NULL;
        pin_flist_t     *r_out_flistp = NULL;
	float		prod_priority_double = 0;
	int32		fup_topup = 0;	
 	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_product_priority", ebufp);
                return;
        }	

	r_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(r_in_flistp, PIN_FLD_POID, prod_pdp, ebufp );
        PIN_FLIST_FLD_SET(r_in_flistp, PIN_FLD_PRIORITY, 0, ebufp );
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_product_priority read product input list", r_in_flistp);
        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, r_in_flistp, &r_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&r_in_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_product_priority Error in calling PCM_OP_READ_FLDS", ebufp);
        	PIN_FLIST_DESTROY_EX(&r_out_flistp, NULL);
        	return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_product_priority read product out list", r_out_flistp);
        prod_priority_double = pbo_decimal_to_double( PIN_FLIST_FLD_GET(r_out_flistp, PIN_FLD_PRIORITY, 0, ebufp),ebufp);
        prod_priority_double = ((int)prod_priority_double) % 1000;
        if(prod_priority_double >= BB_FUP_START && prod_priority_double < BB_FUP_END)
        {
        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "THIS IS AN FUP TOPUP PRODUCT");
		fup_topup = 1;
	}
	else if(prod_priority_double >= BB_ADD_MB_START && prod_priority_double <= BB_ADD_MB_END)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "THIS IS AN ADD MB/GB PRODUCT");
                fup_topup = 2;
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "THIS IS NOT AN FUP TOPUP PRODUCT");
		fup_topup = -1;
	}
	*ret_flistpp = PIN_FLIST_COPY(r_out_flistp,ebufp);
	PIN_FLIST_DESTROY_EX(&r_out_flistp, NULL);
	return fup_topup;	
}

void
fm_mso_get_poid_details(
	pcm_context_t           *ctxp,
        poid_t                  *poid_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
	 pin_flist_t     *r_in_flistp = NULL;
        pin_flist_t     *r_out_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_poid_details", ebufp);
                return;
        }
	
	r_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(r_in_flistp, PIN_FLD_POID, poid_pdp, ebufp );
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_product_priority read product input list", r_in_flistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, r_in_flistp, &r_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&r_in_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_product_priority Error in calling PCM_OP_READ_FLDS", ebufp);
                PIN_FLIST_DESTROY_EX(&r_out_flistp, NULL);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_product_priority read product out list", r_out_flistp);
	*ret_flistpp = PIN_FLIST_COPY(r_out_flistp,ebufp);
        PIN_FLIST_DESTROY_EX(&r_out_flistp, NULL);
	return;
}

/**************************************************
* Function  : fm_mso_get_purchased_prod_active()
* Decription: function to fetch active purchased_product's
*             for service and account
*
***************************************************/
void 
fm_mso_get_purchased_prod_active(
        pcm_context_t           *ctxp,
        pin_flist_t		*inp_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        poid_t                  *s_pdp = NULL;
        int32                   flags = 256;
        int64                   db = 0;
        int64                   id = -1;
        char                    *s_template = " select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 ";
        int32                   act_status = 1;
	poid_t			*serv_pdp = NULL;
	poid_t			*act_pdp = NULL;
	pin_flist_t		*ret_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
	
	act_pdp = (poid_t *)PIN_FLIST_FLD_GET(inp_flistp, PIN_FLD_POID, 0, ebufp);
	serv_pdp = (poid_t *)PIN_FLIST_FLD_GET(inp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
        db = PIN_POID_GET_DB(act_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_active search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_purchased_prod_active - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_active search output flist", ret_flistp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

/**************************************************
* Function  : fm_mso_get_purchased_prod_all()
* Decription: function to fetch active purchased_product's
*             for service and account
*
***************************************************/
void
fm_mso_get_purchased_prod_all(
        pcm_context_t           *ctxp,
        poid_t                  *inp_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        poid_t                  *s_pdp = NULL;
        int32                   flags = 256;
        int64                   db = 0;
        int64                   id = -1;
        char                    *s_template = " select X from /purchased_product where F1 = V1 and F2 = V2 ";
        poid_t                  *serv_pdp = NULL;
        poid_t                  *act_pdp = NULL;
        pin_flist_t             *ret_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_all input flist");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_all input list", inp_flistp);
        act_pdp = (poid_t *)PIN_FLIST_FLD_GET(inp_flistp, PIN_FLD_POID, 0, ebufp);
        serv_pdp = (poid_t *)PIN_FLIST_FLD_GET(inp_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_all Got fields");
        db = PIN_POID_GET_DB(act_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);


        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_all search input list");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_all search input list", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_purchased_prod_all - Error in calling SEARCH", ebufp);
                PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_prod_all search output flist", ret_flistp);
        *ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

int32
validate_demo_packs(
	pcm_context_t	*ctxp,
        pin_flist_t     *in_flistp,
	pin_flist_t	**ret_flistp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t	*demo_flistp    = NULL;
	pin_flist_t	*args_flistp    = NULL;
	pin_flist_t	*res_flistp		= NULL;
	pin_flist_t	*res_in_flistp  = NULL;
	
	pin_cookie_t	cookie_out = NULL;
	pin_cookie_t	cookie_in  = NULL;

	int32	rec_id_out	= 0;
	int32	rec_id_in	= 0;
	int32	demo_counter = 0;
	int32	status_fail  = 1;

	char	*product_name = NULL;
	char	*demo_product_name = NULL;
	char	error_str[300] ;

	poid_t	*in_srvc_pdp = NULL;
	poid_t  *srvc_pdp = NULL;	
	
	PIN_ERR_LOG_FLIST(3, "Demo pack validation inflistp", in_flistp);
	in_srvc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	demo_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, demo_flistp, PIN_FLD_POID, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(demo_flistp, PIN_FLD_ARGS, 0, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, "HD STARTER DEMO 7 DAYS PACKAGE", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(demo_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, "WEST BASIC SERVICE TIER DEMO 7DAY", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(demo_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, "KARNATAKA BASIC SERVICE TIER SP DEMO", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(demo_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, "HYDERABAD PRIME DEMO 7DAY", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(demo_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, "EAST BASIC SERVICE TIER DEMO 7DAY", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(demo_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, "JAIPUR BASIC SERVICE TIER DEMO 7DAY", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(demo_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, "NORTH BASIC SERVICE TIER DEMO 7DAY", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(demo_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, "MP BASIC SERVICE TIER DEMO 7DAY", ebufp);

	PIN_ERR_LOG_FLIST(3, "Demo plans flistp", demo_flistp);

	while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(demo_flistp, PIN_FLD_ARGS,
		&rec_id_out, 1, &cookie_out, ebufp)) != (pin_flist_t *)NULL) 
	{
		demo_product_name = (char*)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_NAME, 1, ebufp);
		rec_id_in = 0;
		cookie_in = NULL;
		demo_counter = 0;
		while ((res_in_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_RESULTS,
			&rec_id_in, 1, &cookie_in, ebufp)) != (pin_flist_t *)NULL) 
		{
			product_name = (char*)PIN_FLIST_FLD_GET(res_in_flistp, PIN_FLD_NAME, 1, ebufp);
			srvc_pdp = PIN_FLIST_FLD_GET(res_in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
			if (product_name && (strcmp(product_name, demo_product_name)==0) && (PIN_POID_COMPARE(in_srvc_pdp, srvc_pdp, 0, ebufp) == 0))
			{
				if(demo_counter)
				{	
					sprintf(error_str, "Package %s can be purchased once in a lifetime", product_name);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, error_str, ebufp);
					PIN_ERRBUF_RESET(ebufp);
					*ret_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(demo_flistp, PIN_FLD_POID, *ret_flistp, PIN_FLD_POID, ebufp );
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &status_fail, ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_CODE, "53001", ebufp);
					PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_ERROR_DESCR, error_str, ebufp);
					PIN_FLIST_DESTROY_EX(&demo_flistp,NULL);
					return status_fail;	
				}
				demo_counter =1;
			}
		}
		demo_counter = 0;
	}
	PIN_FLIST_DESTROY_EX(&demo_flistp,NULL);
	return 0;
}

void
fm_mso_catv_add_demo_pack(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	poid_t                  *act_pdp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *read_iflistp = NULL;
	pin_flist_t             *read_oflistp = NULL;
	pin_flist_t             *ret_flistp = NULL;
	pin_flist_t             *tmp_flistp = NULL;
	pin_flist_t             *temp_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *plan_flistp = NULL;
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *s_oflistp = NULL;
	pin_flist_t             *result_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *search_flistp = NULL;
	pin_flist_t             *search_oflistp = NULL;
	
	poid_t                  *plan_pdp = NULL;
	poid_t                  *prod_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	poid_t                  *s_pdp = NULL;
	
	char                    *state_code = NULL;
	char                    *addl_plan_name = NULL;
        char                    s_template[150] = "select X from /mso_cfg_catv_state_demo_pack where F1 = V1 ";
        char                    template[150];
	
	int32                   flags = 512;
	int                     elem_id  = 0;
	int                     db = 1;
	int			status = 0;
	
	pin_cookie_t            cookie = NULL;
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "In CATV DEMO Pack adding");
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, act_pdp, ebufp );
        
	fm_mso_get_state_city_area_code(ctxp, NULL, act_pdp, &read_oflistp, ebufp );
	
	if ((read_oflistp && (PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_ERROR_CODE, 1, ebufp))) || PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting state:fm_mso_catv_add_demo_pack", ebufp);
		PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state_city_area_code flistp", read_oflistp);
	state_code = (char*)PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_STATE, 1, ebufp);

	if (state_code){
		db = PIN_POID_GET_DB(act_pdp);
		s_flistp = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATE, state_code, ebufp);
		
		res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_add_demo_pack search input list", s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_catv_add_demo_pack - Error in calling SEARCH", ebufp);
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_add_demo_pack search output list", s_oflistp);
		if (s_oflistp && PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp) > 0){
			res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			addl_plan_name = PIN_FLIST_FLD_GET(res_flistp,  MSO_FLD_PLAN_NAME, 1, ebufp);
			// Get DEMO PLAN for state mapped to account
			if(addl_plan_name){
				search_flistp = PIN_FLIST_CREATE(ebufp);
				s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
				PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, s_pdp, ebufp);
				PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &flags, ebufp);
				strcpy(template, "select X from /plan where F1 = V1 ");
				PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, template, ebufp);

				args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
				PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, addl_plan_name, ebufp);
				temp_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_add_demo_pack plans search input list", 
									search_flistp);
				PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_oflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_catv_add_demo_pack- Error in SEARCH", 																ebufp);
					PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_catv_add_demo_pack plans search output list", 
									search_oflistp);
				if (search_oflistp && PIN_FLIST_ELEM_COUNT(search_oflistp, PIN_FLD_RESULTS,ebufp) > 0){
					temp_flistp = PIN_FLIST_ELEM_GET(search_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
					plan_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_PLAN, elem_id,ebufp);
					PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_POID, plan_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_NAME, plan_flistp, PIN_FLD_NAME, ebufp);
					PIN_FLIST_FLD_COPY(temp_flistp, PIN_FLD_CODE, plan_flistp, PIN_FLD_CODE, ebufp);
				}
			}
			else{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No DEMO pack found");
				status = 1;
			}
		}
		else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No DEMO Pack found for the city");
			status = 1;
		}
	}
	PIN_FLIST_DESTROY_EX(&read_oflistp, NULL);
	
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status, ebufp );
	
	*r_flistpp = ret_flistp;
	PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_oflistp, NULL);
	return;
}

void
fm_mso_get_lco_city(
	pcm_context_t		*ctxp,
	poid_t			*account_pdp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*profile_substr = NULL;
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*read_iflist = NULL;

	int32			srch_flag = 512;
	poid_t			*srch_pdp = NULL;
	poid_t			*lco_obj = NULL;

	char			*template = "select X from /profile/customer_care where F1 = V1 and profile_t.poid_type='/profile/customer_care' ";
	int64			db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_lco_city", ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_get_lco_city");

	db = PIN_POID_GET_DB(account_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);


	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_lco_city search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_lco_city search output list", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		profile_substr = PIN_FLIST_SUBSTR_GET(result_flist, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
		lco_obj = PIN_FLIST_FLD_GET(profile_substr, MSO_FLD_LCO_OBJ, 0, ebufp);
		read_iflist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_POID, lco_obj, ebufp);
		PIN_FLIST_FLD_SET(read_iflist, MSO_FLD_RMN, NULL, ebufp);
                PIN_FLIST_FLD_SET(read_iflist, MSO_FLD_RMAIL, NULL, ebufp);
                PIN_FLIST_FLD_SET(read_iflist, PIN_FLD_ACCOUNT_NO, NULL, ebufp);
		PIN_FLIST_ELEM_SET(read_iflist, NULL, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_lco_city read account", read_iflist);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_iflist, r_flistpp, ebufp);
		PIN_FLIST_DESTROY_EX(&read_iflist, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_lco_city read account output", *r_flistpp);
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

int32 
fm_mso_utils_discount_validation(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_errbuf_t            *ebufp)
{
	pin_decimal_t           *disc_percent = NULL;
        pin_decimal_t           *hund_disc = NULL;
        pin_decimal_t           *disc_amt = NULL;
        poid_t                  *service_pdp = NULL;
        pin_flist_t             *bb_info_flistp = NULL;
        pin_flist_t             *ret_flistpp = NULL;
        pin_decimal_t           *actual_amnt = NULL;
        poid_t                  *plan_pdp = NULL;
	pin_flist_t		*ser_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	int32			ret_val= 0;
	char			*action = NULL;
	pin_decimal_t		*zero_disc = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_utils_discount_validation", ebufp);
        }
	action = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_REF_NO, 1, ebufp);
	disc_percent = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_DISC_PERCENT, 1, ebufp);
	if(pbo_decimal_is_null(disc_percent, ebufp))
	{
		plan_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 1, ebufp);
		if(plan_flistp != NULL)
		{
			disc_percent = PIN_FLIST_FLD_GET(plan_flistp, MSO_FLD_DISC_PERCENT, 1, ebufp);
		}
	}
	disc_amt = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_DISC_AMOUNT, 1, ebufp);
	if(pbo_decimal_is_null(disc_amt, ebufp))
	{
		plan_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 1, ebufp);
		if(plan_flistp != NULL)
                {
			disc_amt = PIN_FLIST_FLD_GET(plan_flistp, MSO_FLD_DISC_AMOUNT, 1, ebufp);
		}
	}

	zero_disc = pbo_decimal_from_str("0.0",ebufp);
	if(!pbo_decimal_is_null(disc_percent, ebufp))
	{
		hund_disc = pbo_decimal_from_str("100.0",ebufp);
		if(pbo_decimal_compare(disc_percent, hund_disc, ebufp) >0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount percentage is more than 100");
			ret_val =1;
			goto CLEANUP;
		}
		/*else if(pbo_decimal_compare(zero_disc, disc_percent, ebufp) >0)
		{
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount percentage is less then Zero");
                        ret_val =1;
                        goto CLEANUP;
		}*/
	}
	if(!pbo_decimal_is_null(disc_amt, ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check7" );
		/*if(pbo_decimal_compare(zero_disc, disc_amt, ebufp) >0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount amount is less then Zero");
                        ret_val =1;
                        goto CLEANUP;
		}*/	
		service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		fm_mso_get_poid_details(ctxp, service_pdp, &ret_flistpp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_get_poid_details", ebufp);
        	}
		bb_info_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(ret_flistpp, PIN_FLD_ACCOUNT_OBJ, bb_info_flistp, PIN_FLD_POID, ebufp);
		ser_flistp = PIN_FLIST_SUBSTR_GET(ret_flistpp, MSO_FLD_BB_INFO, 0, ebufp);
		arg_flistp = PIN_FLIST_ELEM_ADD(bb_info_flistp, PIN_FLD_PLAN, 0, ebufp);
		PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_BILL_WHEN, arg_flistp, PIN_FLD_BILL_WHEN, ebufp);
		PIN_FLIST_FLD_COPY(ser_flistp, PIN_FLD_CITY, arg_flistp, PIN_FLD_CITY, ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
		if(action && strcmp(action,"PLANLIST") == 0)
		{
			plan_pdp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		}
		else
		{
			plan_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		}
		fm_mso_get_poid_details(ctxp, plan_pdp, &ret_flistpp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after calling fm_mso_get_poid_details1", ebufp);
                }
		PIN_FLIST_FLD_COPY(ret_flistpp, PIN_FLD_NAME, arg_flistp, MSO_FLD_PLAN_NAME, ebufp);
		PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(bb_info_flistp, PIN_FLD_ACTION, "DISC AMT", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "serv_chk", bb_info_flistp);
		PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);

		/*calling function in price_calc*/
		fm_mso_search_plan_details(ctxp, bb_info_flistp, &ret_flistpp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error after returing fm_mso_search_plan_details", ebufp);
                }
		actual_amnt = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_CYCLE_FEE_AMT, 0, ebufp);
		if(pbo_decimal_compare(disc_amt, actual_amnt, ebufp) >0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Discount amount  is more than actual amount");
			ret_val=1;
			goto CLEANUP;
		}

	}
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
        PIN_FLIST_DESTROY_EX(&bb_info_flistp, NULL);
	if (!pbo_decimal_is_null(hund_disc, ebufp))
                pbo_decimal_destroy(&hund_disc);
	if (!pbo_decimal_is_null(zero_disc, ebufp))
                pbo_decimal_destroy(&zero_disc);
	return ret_val;
}

void
fm_mso_get_balances(
        pcm_context_t           *ctxp,
        poid_t                  *account_pdp,
        time_t                  bal_srch_t,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *tmp_flistp = NULL;
        pin_flist_t             *srch_in_flistp = NULL;
        pin_flist_t             *srch_out_flistp =  NULL;
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *bal_flistp = NULL;
        pin_flist_t             *sbal_flistp = NULL;
        pin_flist_t             *result_flistp = NULL;
        pin_flist_t             *bal_grp_flistp = NULL;

        int64                   db = -1;
        int32                   srch_flag = 768;

        char                    *template = "select X from /balance_group where F1 = V1 and F2 > V2 and F3 <= V3 ";

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_balances", ebufp);
                return;
        }

        if (!account_pdp)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_balances : missing account poid in input");
                return;
        }

        db = PIN_POID_GET_DB(account_pdp);

        srch_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
        bal_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp );
        sbal_flistp = PIN_FLIST_ELEM_ADD(bal_flistp, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, ebufp );
        PIN_FLIST_FLD_SET(sbal_flistp, PIN_FLD_VALID_TO, &bal_srch_t, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
        bal_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp );
        sbal_flistp = PIN_FLIST_ELEM_ADD(bal_flistp, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, ebufp );
        PIN_FLIST_FLD_SET(sbal_flistp, PIN_FLD_VALID_FROM, &bal_srch_t, ebufp);

        result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_balances : search balance_group input list", srch_in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_balances : Error in calling PCM_OP_SEARCH", ebufp);
                PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                return ;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_balances : search bal_grp output flist", srch_out_flistp);
        bal_grp_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
        if (!bal_grp_flistp)
	{
		*o_flistpp = PIN_FLIST_COPY(srch_out_flistp, ebufp);
                goto CLEANUP;
	}
        tmp_flistp = PIN_FLIST_ELEM_GET(bal_grp_flistp, PIN_FLD_BALANCES, CURRENCY, 1, ebufp );
        if (tmp_flistp)
                PIN_FLIST_ELEM_DROP(bal_grp_flistp, PIN_FLD_BALANCES, CURRENCY, ebufp );


        *o_flistpp = PIN_FLIST_COPY(bal_grp_flistp, ebufp);

        CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
}

/*******************************************************************************
* Function: fm_mso_search_offer_entries
* Description: This function is used to find the offer entries such DISCOUNT
	and DATA offers if any 
*******************************************************************************/
void
fm_mso_search_offer_entries(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	poid_t                  *act_pdp = NULL;
	poid_t                  *s_pdp = NULL;
	int32                   flags = 512;
	int32                   *oflags = NULL;
	int32                   *type = NULL;
	int64                   db = 0;
	int64                   id = -1;
	char			s_template[250] = "";
	pin_flist_t             *ret_flistp = NULL;
	time_t                  current_t = pin_virtual_time(NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	oflags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	type  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE, 1, ebufp);
	db = PIN_POID_GET_DB(act_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	//PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_TO, &current_t, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TYPE, args_flistp, PIN_FLD_TYPE, ebufp);
	
	if(!oflags) { // This is optional field and would be sent during creation only 
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_VALID_FROM, &current_t, ebufp);
		if(type && *type == 0){ //Currently we have 1 and 2 types but if we send zero, that is to return all types of offers
			strcpy(s_template, "select X from /mso_cfg_account_offers where F1 = V1 and F2 >= V2 and F3 > V3 and F4 <= V4 order by created_t desc ");
		}else{
			strcpy(s_template, "select X from /mso_cfg_account_offers where F1 = V1 and F2 >= V2 and F3 = V3 and F4 <= V4 order by created_t desc ");
		}
	}
	else if(oflags && *oflags == 1){
                strcpy(s_template, "select X from /mso_cfg_account_offers where F1 = V1 and F2 >= V2 order by created_t desc ");
	}
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_TYPE, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_DISCOUNT_FLAGS, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_AMOUNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_VALID_FROM, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_VALID_TO, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_offer_entries search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_offer_entries - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_offer_entries search output flist", ret_flistp);
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

/**************************************************
* Function  : fm_mso_get_purchase_install_products()
* Decription: function to fetch purchased install
*	products based on status passed
***************************************************/
void
fm_mso_get_purchased_install_products(
        pcm_context_t           *ctxp,
        poid_t			*act_pdp,
        poid_t			*srvc_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *sort_flistp = NULL;
        poid_t                  *s_pdp = NULL;
        int32                   flags = 256;
        double 			start_pri = BB_INST_PRI_START;
        double			end_pri = BB_INST_PRI_END;
        int64                   db = 0;
        int64                   id = -1;
        char                    *s_template = " select X from /purchased_product 1, /product 2 where 1.F1 = V1 and 1.F2 = V2"
						" and 2.F3 >= V3 and 2.F4 <= V4 and 1.F6 = 2.F5 order by status, purchase_end_t asc ";
	pin_flist_t		*ret_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
	
        db = PIN_POID_GET_DB(act_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
        
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, pbo_decimal_from_double(start_pri, ebufp), ebufp);
        
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, pbo_decimal_from_double(end_pri, ebufp), ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_STATUS, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_EFFECTIVE_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PURCHASE_START_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CYCLE_END_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PURCHASE_DISCOUNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PURCHASE_DISC_AMT, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CYCLE_DISCOUNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_USAGE_DISCOUNT, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_install_products search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_purchased_install_products - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_purchased_install_products search output flist", ret_flistp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

/******************************************************************
* Function  : fm_mso_create_refund_for_cancel_plan()
* Decription: function to create refund for cancel plan
*	      can be enhanced interms of input flist building
*******************************************************************/
void
fm_mso_create_refund_for_cancel_plan(
        pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *act_flistp = NULL;
	pin_flist_t             *event_flistp = NULL;
	pin_flist_t             *prod_flistp = NULL;
	pin_flist_t             *read_oflistp = NULL;
	pin_flist_t             *cfee_flistp = NULL;
	poid_t                  *act_pdp = NULL;
	poid_t                  *srvc_pdp = NULL;
	poid_t                  *evt_pdp = NULL;
	poid_t                  *prod_pdp = NULL;
	int32                   flags = 33554452;
	char			*prog_name = NULL;
	char			*event_type  = NULL;
	char			*sys_descr = NULL;
	char			*name = NULL;
	char			descr[30];
	int64                   db = 0;
	int64                   id = -1;
	pin_flist_t		*ret_flistp = NULL;
	time_t			current_t = pin_virtual_time(NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_refund_for_cancel_plan refund iflist:", i_flistp);	
	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	prod_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
	event_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);
        db = PIN_POID_GET_DB(act_pdp);

	fm_mso_utils_read_any_object(ctxp, prod_pdp, &read_oflistp, ebufp);	
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_create_refund_for_cancel_plan - Error in reading product", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
        }
	if (read_oflistp){
		name = PIN_FLIST_FLD_GET(read_oflistp, PIN_FLD_NAME, 0, ebufp);
		if (event_type && strcmp(event_type, "/event/billing/product/fee/purchase") == 0){
			strcpy(descr, "Purchase Fees (srvc) (srvc): ");
		}
		else{
			strcpy(descr, "Cycle Forward Fees (srvc): ");
		}
		sys_descr = (char*)malloc((sizeof (char) * strlen(descr)+1));
		strcpy(sys_descr, descr);
		sys_descr = (char *) realloc(sys_descr, (strlen(descr)+strlen(name))+1);
		strcat(sys_descr, name);
	}
	
        act_flistp = PIN_FLIST_CREATE(ebufp);
        evt_pdp = PIN_POID_CREATE(db, event_type, id, ebufp);
        PIN_FLIST_FLD_SET(act_flistp, PIN_FLD_POID, act_pdp, ebufp);
        PIN_FLIST_FLD_SET(act_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	event_flistp = PIN_FLIST_SUBSTR_ADD(act_flistp, PIN_FLD_EVENT, ebufp);
        PIN_FLIST_FLD_PUT(event_flistp, PIN_FLD_POID, evt_pdp, ebufp);
        PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
        PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
        PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_NAME, "Refund plan", ebufp);
        PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_PUT(event_flistp, PIN_FLD_SYS_DESCR, sys_descr, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, event_flistp, PIN_FLD_START_T, ebufp);
	if (event_type && strcmp(event_type, "/event/billing/product/fee/purchase") == 0){
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, event_flistp, PIN_FLD_START_T, ebufp);
        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, event_flistp, PIN_FLD_END_T, ebufp);
	}else{
		PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_START_T, &current_t, ebufp);
        	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_END_T, &current_t, ebufp);
	}
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, event_flistp, PIN_FLD_EARNED_START_T, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, event_flistp, PIN_FLD_EARNED_END_T, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, event_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	prod_flistp = PIN_FLIST_SUBSTR_ADD(event_flistp, PIN_FLD_PRODUCT, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OFFERING_OBJ, prod_flistp, PIN_FLD_OFFERING_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PRODUCT_OBJ, prod_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_OFFERING_OBJ, act_flistp, PIN_FLD_OFFERING_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PRODUCT_OBJ, act_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PERCENT, act_flistp, PIN_FLD_PERCENT, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DISCOUNT, act_flistp, PIN_FLD_DISCOUNT, ebufp);
        PIN_FLIST_FLD_SET(act_flistp, PIN_FLD_NAME, event_type, ebufp);
	if (event_type && strstr(event_type, "/event/billing/product/fee/cycle") ){
        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PRODUCTS, act_flistp, PIN_FLD_PRODUCTS, ebufp);
		cfee_flistp = PIN_FLIST_SUBSTR_ADD(event_flistp, PIN_FLD_CYCLE_INFO, ebufp);
        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, cfee_flistp, PIN_FLD_CYCLE_START_T, ebufp);
        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, cfee_flistp, PIN_FLD_CYCLE_END_T, ebufp);
        	PIN_FLIST_FLD_SET(cfee_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        	PIN_FLIST_FLD_PUT(cfee_flistp, PIN_FLD_ORIGINAL_SCALE, pbo_decimal_from_str("1", ebufp), ebufp);
        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SCALE, cfee_flistp, PIN_FLD_SCALE, ebufp);
        	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SCALE, act_flistp, PIN_FLD_SCALE, ebufp);
        	PIN_FLIST_FLD_PUT(act_flistp, PIN_FLD_ORIGINAL_SCALE, pbo_decimal_from_str("1", ebufp), ebufp);
	}
	if(PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_BAL_IMPACTS, ebufp) > 0){
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BAL_IMPACTS, event_flistp, PIN_FLD_BAL_IMPACTS, ebufp);
	}
	if(PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_TOTAL, ebufp) > 0){
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TOTAL, event_flistp, PIN_FLD_TOTAL, ebufp);
	}
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_refund_for_cancel_plan refund input list", act_flistp);
	PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, act_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&act_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_create_refund_for_cancel_plan - Error in calling ACT_USAGE", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_refund_for_cancel_plan refund outflist", ret_flistp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	//PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

/*********************************************************************************
* This function is to get the mso_cfg_credit_limit details based on the 
* PLAN, CITY, and PAYTERM and returns the results such as SERVICE_CODE, QUOTA_CODE
* DL_SPEED, FUP_DL_SPEED, UL_SPEED, FUP_UL_SPEED etc..
*********************************************************************************/

void
fm_mso_get_service_quota_codes(
	pcm_context_t			*ctxp,
	char				*plan_name,
	int32				*bill_when,
	char				*city,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t			*ebufp)
{
	
	pin_flist_t		*search_plan_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*city_flistp = (pin_flist_t *)NULL;
	pin_flist_t		*cit_flistp = NULL;
	pin_flist_t		*search_plan_o_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;

	poid_t			*plan_pdp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*pdp = NULL;

	int32                   s_flags = 768;
	int64                   database = 1;
	char			*srvc_code = NULL;
	char			*pquota_code = NULL;
	char			*plan_city = NULL;

	pin_decimal_t		*dlspeed = NULL;
	pin_decimal_t		*ulspeed = NULL;
	pin_decimal_t		*fdlspeed = NULL;
	pin_decimal_t		*fulspeed = NULL;

	int			elem_id = 0;
	pin_cookie_t		cookie =NULL;
	
	
	search_plan_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	
	PIN_FLIST_FLD_PUT(search_plan_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_plan_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	
	PIN_FLIST_FLD_SET(search_plan_flistp, PIN_FLD_TEMPLATE,
	"select X from /mso_cfg_credit_limit where F1 = V1 and F2 = V2 and ( F3 = V3 or F4 = V4 ) " , ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, MSO_FLD_PLAN_NAME, plan_name, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 2, ebufp);
	city_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CITIES, 0, ebufp);
	PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 3, ebufp);
	city_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CITIES, 0, ebufp);
	PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CITY, city, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(search_plan_flistp, PIN_FLD_ARGS, 4, ebufp);
	city_flistp = PIN_FLIST_ELEM_ADD(arg_flistp, MSO_FLD_CITIES, 0, ebufp);
	PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CITY, "*", ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD( search_plan_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "mso_cfg_credit_limit service_code search input", search_plan_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_plan_flistp, &search_plan_o_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_plan_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "mso_cfg_credit_limit service_code search output", search_plan_o_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "error getting last plan ", ebufp);
             goto cleanup;
	}	
	
	result_flistp = PIN_FLIST_ELEM_GET(search_plan_o_flistp,PIN_FLD_RESULTS,0,1,ebufp);
	if(result_flistp)
	{
		while( (city_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, MSO_FLD_CITIES,
                        &elem_id, 1, &cookie, ebufp)) != NULL )
		{
			plan_city = PIN_FLIST_FLD_GET(city_flistp, PIN_FLD_CITY, 1, ebufp);
			//Priority is at CITY LEVEL and if no city entry is found, then go for global
			if(city && plan_city && strcmp(city, plan_city) == 0){
				cit_flistp = NULL; //In case if it is set in all cities category
				srvc_code = PIN_FLIST_FLD_GET(city_flistp, PIN_FLD_SERVICE_CODE, 1, ebufp);
				pquota_code = PIN_FLIST_FLD_GET(city_flistp, MSO_FLD_QUOTA_CODE, 1, ebufp);
				dlspeed = PIN_FLIST_FLD_GET(city_flistp, MSO_FLD_DL_SPEED, 1, ebufp);
				ulspeed = PIN_FLIST_FLD_GET(city_flistp, MSO_FLD_UL_SPEED, 1, ebufp);
				fulspeed = PIN_FLIST_FLD_GET(city_flistp, MSO_FLD_FUP_UL_SPEED, 1, ebufp);
				fdlspeed = PIN_FLIST_FLD_GET(city_flistp, MSO_FLD_FUP_DL_SPEED, 1, ebufp);
				cit_flistp = PIN_FLIST_COPY(city_flistp, ebufp);
				break;
			}
			//If CITY level not found, then check for all cities
			else if (plan_city && strcmp(plan_city, "*") == 0){
				srvc_code = PIN_FLIST_FLD_GET(city_flistp, PIN_FLD_SERVICE_CODE, 1, ebufp);
				pquota_code = PIN_FLIST_FLD_GET(city_flistp, MSO_FLD_QUOTA_CODE, 1, ebufp);
				dlspeed = PIN_FLIST_FLD_GET(city_flistp, MSO_FLD_DL_SPEED, 1, ebufp);
				ulspeed = PIN_FLIST_FLD_GET(city_flistp, MSO_FLD_UL_SPEED, 1, ebufp);
				fulspeed = PIN_FLIST_FLD_GET(city_flistp, MSO_FLD_FUP_UL_SPEED, 1, ebufp);
				fdlspeed = PIN_FLIST_FLD_GET(city_flistp, MSO_FLD_FUP_DL_SPEED, 1, ebufp);
				cit_flistp = PIN_FLIST_COPY(city_flistp, ebufp);
			}
			else{
				continue;
			}
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_quota_codes CITY flist", cit_flistp);

		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_PLAN_NAME, plan_name, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_CITY, city, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SERVICE_CODE, srvc_code, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_QUOTA_CODE, pquota_code, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_DL_SPEED, dlspeed, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_UL_SPEED, ulspeed, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_FUP_UL_SPEED, fulspeed, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_FUP_DL_SPEED, fdlspeed, ebufp);
		PIN_FLIST_FLD_COPY(cit_flistp, MSO_FLD_CREDIT_PROFILE, *r_flistpp, MSO_FLD_CREDIT_PROFILE, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_service_quota_codes return flist", *r_flistpp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "SERVICE CODE CITY MAPPING NOT FOUND" );
		goto cleanup;
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&search_plan_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&cit_flistp, NULL);

	return;

}

/**************************************************
* Function  : fm_mso_utils_get_mso_baseplan_details()
* Decription: function to fetch baseplan products
***************************************************/
void
fm_mso_utils_get_mso_baseplan_details(
        pcm_context_t           *ctxp,
        poid_t			*act_pdp,
        poid_t			*srvc_pdp,
	int			status,
	int			prod_status,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        poid_t                  *s_pdp = NULL;
        int32                   flags = 512;
        //int32                   prod_status = 1;
        int64                   db = 0;
        int64                   id = -1;
        char                    *s_template = " select X from /mso_purchased_plan 1, /purchased_product 2 where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3 "
						"and 1.F4 = V4 and 1.F5 = 2.F6 and 2.F7 = V7 and 1.F8 = 2.F9 and 1.F10 is not null order by created_t desc " ;
	pin_flist_t		*ret_flistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);
	
        db = PIN_POID_GET_DB(act_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
        
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, "base plan", ebufp);
        
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
        
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &prod_status, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 10, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_mso_baseplan_details search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_utils_get_mso_baseplan_details - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_mso_baseplan_details search output flist", ret_flistp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

PIN_EXPORT char*
fm_mso_utils_get_state_code(
	pcm_context_t   *ctxp,
	poid_t		*account_pdp,
	char          	*state_name,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t	*srch_in_flistp = NULL;
	pin_flist_t	*arg_flist = NULL;
	pin_flist_t	*result_flist = NULL;
	pin_flist_t	*srch_out_flistp = NULL;

	int32		srch_flag = 512;
	poid_t		*srch_pdp = NULL;

	char		*template = "select X from /mso_cfg_state_census_map where F1 = V1 ";
	char		*state_code = NULL;
	char		*ret_val = NULL;
	int64		db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_utils_get_state_code", ebufp);
	}
	
	db = PIN_POID_GET_DB(account_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_STATE, state_name, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_STATE, state_name, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CODE, state_name, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_state_code search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_state_code search output list", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		state_code = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_CODE, 1, ebufp);
	}

	if (!state_code)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MATCH, PIN_FLD_CODE, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_utils_get_state_code error", ebufp);
		goto CLEANUP;
	}
	else
	{
		ret_val = (char *) malloc(sizeof(state_code));
		strcpy(ret_val, state_code);
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return ret_val;
}

PIN_EXPORT int32 
fm_mso_utils_gst_sequence_no(
	pcm_context_t	*ctxp,
	poid_t		*account_pdp,
	char		*name,
	pin_errbuf_t	*ebufp)
{
	pcm_context_t	*new_ctxp = NULL;
	pin_flist_t	*srch_in_flistp = NULL;
	pin_flist_t	*srch_out_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	pin_flist_t	*inc_in_flistp = NULL;
	pin_flist_t	*inc_out_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;

	poid_t		*srch_pdp = NULL;
	poid_t		*seq_pdp = NULL;
	int64		id = -1;
	int64		db = 0;
	int32		s_flags = 256;
	int32		hdr_num = -1;
	int32		hdr_inc = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error before Sequence", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_MSG(3, "Getting Sequence for:");
	PIN_ERR_LOG_MSG(3, name);
	db = PIN_POID_GET_DB(account_pdp);
	PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
	
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, "select X from /data/sequence where F1 = V1 and F2 = V2 and F3.type = V3 ", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, name, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/data/sequence", -1, ebufp), ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sequence Object Find Input Flist", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting Sequence", ebufp);
                goto CLEANUP;
        }


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sequence Object Find Output Flist", srch_out_flistp);
	
	result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(result_flistp == NULL)
	{
		if(name && strcmp(name, "ADJUSTMENT") == 0)
		{
			fm_mso_cust_create_bill_adj_sequence(new_ctxp, account_pdp, 1, &r_flistp, ebufp);
		}
		else
		{
			fm_mso_cust_create_bill_adj_sequence(new_ctxp, account_pdp, 2, &r_flistp, ebufp);
		}
	        if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating sequence", ebufp);
                	goto CLEANUP;
        	}

		seq_pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_POID, 0, ebufp);
	}
	else
	{
		seq_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);
	}

	inc_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(inc_in_flistp, PIN_FLD_POID, seq_pdp, ebufp);
	PIN_FLIST_FLD_SET(inc_in_flistp, PIN_FLD_HEADER_NUM, &hdr_inc, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Inc Sequence in", inc_in_flistp);
	PCM_OP(new_ctxp, PCM_OP_INC_FLDS, PCM_OPFLG_LOCK_OBJ, inc_in_flistp, &inc_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&inc_in_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting Sequence", ebufp);
                goto CLEANUP;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Inc Sequence out", inc_out_flistp);

	hdr_num = *(int32 *) PIN_FLIST_FLD_GET(inc_out_flistp, PIN_FLD_HEADER_NUM, 0, ebufp);
	
	CLEANUP:
	PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&inc_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	return hdr_num;
}


PIN_EXPORT void
fm_mso_get_subscriber_profile(
	pcm_context_t	*ctxp,
	poid_t		*account_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*srch_in_flistp = NULL;
	pin_flist_t	*arg_flist = NULL;
	pin_flist_t	*result_flist = NULL;
	pin_flist_t	*srch_out_flistp = NULL;
	pin_flist_t	*profile_substr = NULL;
	pin_flist_t	*nameinfo_flistp = NULL;
	pin_flist_t	*read_iflist = NULL;

	int32		srch_flag = 512;
	poid_t		*srch_pdp = NULL;
	poid_t		*lco_obj = NULL;

	char		*template = "select X from /profile/customer_care where F1 = V1 and profile_t.poid_type='/profile/customer_care' ";
	int64		db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_subscriber_profile", ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_get_subscriber_profile");

	db = PIN_POID_GET_DB(account_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscriber_profile search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscriber_profile search output list", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		profile_substr = PIN_FLIST_SUBSTR_GET(result_flist, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(profile_substr, MSO_FLD_LCO_OBJ, *r_flistpp, MSO_FLD_LCO_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(profile_substr, MSO_FLD_PP_TYPE, *r_flistpp, MSO_FLD_PP_TYPE, ebufp);
		PIN_FLIST_FLD_COPY(profile_substr, MSO_FLD_JV_OBJ, *r_flistpp, MSO_FLD_JV_OBJ, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscriber_profile read account output", *r_flistpp);
	}

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

PIN_EXPORT void
fm_mso_get_bills(
	pcm_context_t	*ctxp,
	char		*bill_no,
	poid_t		*account_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*srch_in_flistp = NULL;
	pin_flist_t	*arg_flist = NULL;
	pin_flist_t	*result_flist = NULL;
	pin_flist_t	*srch_out_flistp = NULL;
	pin_flist_t	*profile_substr = NULL;
	pin_flist_t	*nameinfo_flistp = NULL;
	pin_flist_t	*read_iflist = NULL;

	int32		srch_flag = 256;
	poid_t		*srch_pdp = NULL;
	poid_t		*lco_obj = NULL;
	poid_t		*bill_pdp = NULL;

	pin_decimal_t   *zero_due = NULL;
	int32		end_date = 0;

	char		*bill_template = "select X from /bill where F1 = V1 and F2 <> V2 and F3 <> V3 and F4 = V4 ";
	char		*template = "select X from /bill where F1 = V1 and F2 <> V2 and F3 <> V3 order by F2 desc ";
	int64		db = -1;
	
	pin_decimal_t	*current_bal = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_bills", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_get_bills");
	
    zero_due = pbo_decimal_from_str("0.0", ebufp);

	db = PIN_POID_GET_DB(account_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_END_T, &end_date, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_CURRENT_TOTAL, zero_due, ebufp);

	if(bill_no)
	{
		PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, bill_template, ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 4, ebufp );
        	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILL_NO, bill_no, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	}


	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bills search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, r_flistpp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bills search output list", *r_flistpp);
	return;
}

PIN_EXPORT void
fm_mso_utils_get_profile(
	pcm_context_t	*ctxp,
	poid_t		*account_pdp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*srch_in_flistp = NULL;
	pin_flist_t	*arg_flist = NULL;
	pin_flist_t	*result_flist = NULL;
	pin_flist_t	*srch_out_flistp = NULL;
	pin_flist_t	*profile_flistp = NULL;
	pin_flist_t	*nameinfo_flistp = NULL;
	pin_flist_t	*read_iflist = NULL;

	int32		srch_flag = 512;
	poid_t		*srch_pdp = NULL;
	poid_t		*lco_pdp = NULL;

	char		*template = "select X from /profile/customer_care where F1 = V1 and F2.type = V2 ";
	int64		db = -1;
	int32		gst_type = -1;
	void		*vp = NULL;

	//Added for GST changes	
        int32           lco_type = -1; //0-primary 1-secondary
        int32           gst_exempt = 0;
	int32		state_code_val = 0;
        pin_flist_t     *profile_gst_flistp = NULL;
        pin_flist_t     *gst_info_flistp = NULL;
        char            *gst_no = NULL;
	char		state_code[3] = "\0";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_utils_get_profile", ebufp);
	}
	db = PIN_POID_GET_DB(account_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_PUT(arg_flist, PIN_FLD_POID, PIN_POID_CREATE(db, "/profile/customer_care", 1, ebufp), ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_profile search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_profile search output list", srch_out_flistp);

	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		profile_flistp = PIN_FLIST_SUBSTR_GET(result_flist, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
		lco_pdp = PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
		vp = PIN_FLIST_FLD_GET(profile_flistp, PIN_FLD_CONN_TYPE, 1, ebufp);
                /*Changes to check LCO is GSTIN registered or not and to populate gst_exempt flag-- Chandrakala*/
                lco_type = *(int32 *)PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_PP_TYPE, 1, ebufp);

		if(vp)
		{
			gst_type = *(int32 *)vp;
                        /*Start-Changes to check LCO is GSTIN registered or not and to populate gst_exempt flag-- Chandrakala*/
                        if(lco_type){
                                getProfileGSTInfo(ctxp, lco_pdp, &profile_gst_flistp, ebufp);
                                if (profile_gst_flistp != NULL){
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "profile_gst_details");
                                        gst_info_flistp = PIN_FLIST_ELEM_GET( profile_gst_flistp, MSO_FLD_GST_INFO, 0, 1, ebufp );
                                        gst_no = PIN_FLIST_FLD_GET(gst_info_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, gst_no);
                                }
                                if (gst_no == NULL){
                                        gst_exempt = 1;
                                }
                                else{
					/********************************************************
					 * GST state code values from 01 to 37
					 * GSTIN must start bewtween 01 to 37 state code
					 ********************************************************/
					strncpy(state_code, gst_no, 2);
					state_code_val = atoi(state_code);
					if (state_code_val > 0 && state_code_val < 38)
					{
                                        	gst_exempt = 0;
					}
					else
					{
                                        	gst_exempt = 1;
					}
                                }
                        }
                        /*END-Changes to check LCO is GSTIN registered or not and to populate gst_exempt flag-- Chandrakala*/

			*ret_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, MSO_FLD_LCO_OBJ, lco_pdp, ebufp);
			PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_CONN_TYPE, &gst_type, ebufp);
			//Added for GST changes
                        PIN_FLIST_FLD_SET(*ret_flistpp, MSO_FLD_TAX_TYPE, &gst_exempt, ebufp);

		}
	}
	
	if (gst_type == -1)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MATCH, PIN_FLD_CONN_TYPE, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_utils_get_profile error", ebufp);
		goto CLEANUP;
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

/*******************************************************************************
* Function: fm_mso_search_upgrade_offer_details 
* Description: This function is used to find the upgrade offers  
*******************************************************************************/
void
fm_mso_search_upgrade_offer_details(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	poid_t                  *act_pdp = NULL;
	poid_t                  *s_pdp = NULL;
	int32                   flags = 512;
	int32                   *oflags = NULL;
	int32                   *bill_when = NULL;
	int32                   *type = NULL;
	int64                   db = 1;
	int64                   id = -1;
	char			s_template[250] = "";
	char			*city = NULL;
	pin_flist_t             *ret_flistp = NULL;
	time_t                  current_t = pin_virtual_time(NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	//act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	//oflags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	//type  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE, 1, ebufp);
	//db = PIN_POID_GET_DB(act_pdp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_upgrade_offer_details input list", i_flistp);
	city = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CITY, 1, ebufp);
	bill_when = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_WHEN, 1, ebufp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	//PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, city, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, "*", ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);

	strcpy(s_template, "select X from /mso_cfg_upgrade_offers where ( F1 = V1 or F2 = V2 ) and F3 = V3 ");
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_upgrade_offer_details search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_search_upgrade_offer_details - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_upgrade_offer_details search output flist", ret_flistp);
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

/**************************************************
* Function  : fm_mso_get_purchase_install_products()
* Decription: function to fetch purchased install
*	products based on status passed
***************************************************/
void
fm_mso_get_subscr_purchased_products(
        pcm_context_t           *ctxp,
        poid_t			*act_pdp,
        poid_t			*srvc_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *sort_flistp = NULL;
	poid_t                  *s_pdp = NULL;
	int32                   flags = 256;
	int64                   db = 0;
	int64                   id = -1;
	char                    *s_template = " select X from /purchased_product 1, /product 2 where 1.F1 = V1 and 1.F2 = V2"
						" and 1.F4 = 2.F3 and 2.F5 IS NOT NULL order by created_t desc ";
	pin_flist_t		*ret_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	db = PIN_POID_GET_DB(act_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscr_purchased_products search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_subscr_purchased_products - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_subscr_purchased_products search output flist", ret_flistp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

/**************************************************
* Function  : fm_mso_get_event_type_base()
* Decription: function to fetch event type of 
*	product passed
***************************************************/
void
fm_mso_get_event_type_base(
      pcm_context_t           *ctxp,
        poid_t                  *prod_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	poid_t                  *s_pdp = NULL;
	int32                   flags = 256;
	int64                   db = 0;
	int64                   id = -1;
	char                    *s_template = "select X from /rate_plan 1, /product 2 where 2.F1 = 1.F2 and 2.F3 = V3";
	pin_flist_t		*ret_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	db = PIN_POID_GET_DB(prod_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, prod_pdp, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_PUT(res_flistp, PIN_FLD_EVENT_TYPE, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_event_type search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_event_type - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_event_type output flist", ret_flistp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

/**************************************************
* Function  : fm_mso_get_event_from_plan()
* Decription: function to fetch event type of 
*	product passed
***************************************************/
void
fm_mso_get_event_from_plan(
      pcm_context_t           *ctxp,
        poid_t                  *plan_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *srch_in_flistp = NULL;
	pin_flist_t             *ser_flistp = NULL;
	pin_flist_t             *usage_map = NULL;
	pin_flist_t             *arg_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	poid_t                  *s_pdp = NULL;
	int32                   flags = 256;
	int64                   db = 0;
	int64                   id = -1;
	char                    *s_template = "select X from /product 1, /deal 2, /plan 3 where 3.F1 = V1 and 2.F2 = 3.F3 and 2.F4 = 1.F5 and 1.F6 is not NULL ";
	pin_flist_t		*ret_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	db = PIN_POID_GET_DB(plan_pdp);
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, s_template , ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, plan_pdp, ebufp);

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
	
	res_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PRIORITY, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_NAME, NULL, ebufp);
	usage_map = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_USAGE_MAP, 0, ebufp );
	PIN_FLIST_FLD_SET(usage_map, PIN_FLD_EVENT_TYPE, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_event_from_plan search input list",srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_event_from_plan - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_event_from_plan output flist", ret_flistp);
	if (*ret_flistpp != NULL)
	{
	}
	else
	{
		*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

/**************************************************
* Function  : fm_mso_utils_get_helpdesk_details()
* Decription: function to fetch helpdesk details
* based on the city 
***************************************************/
void
fm_mso_utils_get_helpdesk_details(
        pcm_context_t           *ctxp,
        char			*city,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32                   flags = 512;
	int64                   db = 1;
	int32			id = -1;
	char                    *s_template = " select X from /mso_cfg_helpdesk where F1 = V1 ";
	pin_flist_t		*ret_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, city, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_helpdesk_details search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_utils_get_helpdesk_details - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_helpdesk_details search output flist", ret_flistp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

/**************************************************
* Function  : fm_mso_utils_get_mso_purchase_details()
* Decription: function to fetch helpdesk details
* based on the city 
***************************************************/
void
fm_mso_utils_get_mso_purchase_details(
        pcm_context_t           *ctxp,
        poid_t			*acct_pdp,
        poid_t			*srvc_pdp,
        poid_t			*offer_pdp,
	int32			*pckg_id,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *tmp_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32                   flags = 512;
	int32			package_id  = -1;
	int64                   db = 1;
	int32			id = -1;
	char                    *s_template = " select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 ";
	pin_flist_t		*ret_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	package_id = *pckg_id;

	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PACKAGE_ID, &package_id, ebufp);
	
	//args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, offer_pdp, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_mso_purchase_details search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_utils_get_mso_purchase_details - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_mso_purchase_details search output flist", ret_flistp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

void
fm_mso_utils_get_event_details(
	pcm_context_t           *ctxp,
	poid_t			*act_pdp,
	poid_t			*srvc_pdp,
	char			*event_type,
	time_t			*created_t,	
	int			sort,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *sort_flistp = NULL;
	poid_t                  *s_pdp = NULL;
	int32                   flags = 256;
	int64                   db = 0;
	int64                   id = -1;
	char                    s_template[200] = "select X from /event where F1 = V1 and F2 = V2 and F3 >= V3 and F4.type like V4 order by created_t ";
	
	pin_flist_t		*ret_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_RESET(ebufp);

	db = PIN_POID_GET_DB(act_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	if (sort == 0 ){ //ASC
		strcat(s_template, "ASC ");
	}else{//DESC default
		strcat(s_template, "DESC ");
	}
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	if (srvc_pdp){
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	}
	else{ // In case SERVICE_OBJ is null, it would check for 0 
		PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(0, " ", 0, ebufp), ebufp);
	}

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREATED_T, created_t, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, event_type, id, ebufp), ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_event_details search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_event_details - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_event_details search output flist", ret_flistp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

}

/**********************************************************
* This is to get the charge applied for given offering obj
**********************************************************/
void
fm_mso_utils_get_event_balimpacts(
	pcm_context_t           *ctxp,
	poid_t			*act_pdp,
	poid_t			*srvc_pdp,
	poid_t			*offer_obj,
	time_t			*created_t,
	char			*event_type,
	pin_flist_t             **ret_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *tmp_flistp = NULL;
	poid_t                  *s_pdp = NULL;
	int32                   flags = 512;
	int                   s_flags = 20;
	int64                   db = 0;
	int64                   id = -1;
	char                    s_template[200] = "select X from /event where F1 = V1 and F2 = V2 and F3 >= V3 and F4 = V4 and F5 > V5 and F6.type like V6 order by created_t ";
	char                    s_template1[200] = "select X from /event/billing/product/fee/cycle where F1 = V1 and F2 = V2 and F3 >= V3 and F4 = V4 and (F5 > V5 or (F5 = V5 and F7 = V7 and F8 = V8))  and F6.type like V6 order by created_t ";
	pin_decimal_t		*zero_decimalp = pbo_decimal_from_str("0", ebufp);
	
	pin_flist_t		*ret_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	db = PIN_POID_GET_DB(act_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	if (srvc_pdp){
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	}
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREATED_T, created_t, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp,  PIN_FLD_BAL_IMPACTS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_OFFERING_OBJ, offer_obj, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp,  PIN_FLD_BAL_IMPACTS, 0, ebufp);
	PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_AMOUNT, zero_decimalp, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, event_type, id, ebufp), ebufp);

	if(strstr(event_type,"/event/billing/product/fee/cycle"))
	{
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template1, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp,  PIN_FLD_CYCLE_INFO, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_event_balimpacts search input list1", s_flistp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp,  PIN_FLD_CYCLE_INFO,  ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SCALE, zero_decimalp  , ebufp);
	}

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(res_flistp,  PIN_FLD_BAL_IMPACTS, 0, ebufp);
	/*PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_OFFERING_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_AMOUNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RESOURCE_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RATE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TAX_CODE, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);*/

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_event_balimpacts search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_event_details - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_event_balimpacts search output flist", ret_flistp);
	*ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

}



/*******************************************************************
 * fm_mso_bb_pt_pkg_type() 
 *
 * Enhancement for ISP Heirarchy . This function returns package type 2 for deposits. 
 * Else returns default value of 0
 *******************************************************************/
int32
fm_mso_bb_pt_pkg_type(
	pcm_context_t		*ctxp,
	poid_t			*prd_pdp,
        pin_flist_t             *events_flistp,
	pin_errbuf_t            *ebufp)
{
	int32			pkg_type = 0;
	poid_t			*item_pdp = NULL;
	char			*item_type;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	
	item_pdp = (poid_t *)PIN_FLIST_FLD_GET(events_flistp,PIN_FLD_ITEM_OBJ,1,ebufp);

	if (item_pdp && item_pdp != NULL )
	{
		item_type = (char *)PIN_POID_GET_TYPE(item_pdp);
	            if( item_type && !strncmp( item_type, "/item/mso_hw_deposit/", 21 )){
			 pkg_type = 2;

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Setting pkg_type to 2");	
		    }
            else{
			 pkg_type = 0;

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Setting pkg_type to 0");	
            }
	
	}
	

CLEANUP:
        return pkg_type;
}

/**********************************************************
* This is to create a service object
**********************************************************/

void
fm_mso_create_service(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{

        poid_t                  *srvc_pdp = NULL;
        poid_t                  *act_pdp = NULL;
        poid_t                  *prof_pdp = NULL;
        poid_t                  *dvc_pdp = NULL;
        char    *login = NULL;
        char    *pswd = NULL;
        int32                   status_flag = 0;
        int32                  status = 10100;
        void                    *vp = NULL;
        int32           *status_svc = NULL;

        pin_flist_t             *svc_flistp = NULL;
        pin_flist_t             *ret_flistp = NULL;
        pin_flist_t             *status_arr = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                        return;
                PIN_ERRBUF_CLEAR(ebufp);

        /***********************************************************
        * Debug what we got.
        ***********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_service: input flist",in_flistp);
        act_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        srvc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
        //prof_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROFILE_OBJ, 0, ebufp);
        //dvc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_DEVICE_OBJ, 0, ebufp);
        status_arr = PIN_FLIST_ELEM_GET(in_flistp,PIN_FLD_STATUSES, PIN_ELEMID_ANY, 1, ebufp);
        status_svc = PIN_FLIST_FLD_GET(status_arr, PIN_FLD_STATUS, 1, ebufp);
        //status_flag = PIN_FLIST_FLD_GET (in_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp);
        login = PIN_FLIST_FLD_GET (in_flistp, PIN_FLD_LOGIN, 0, ebufp);
        pswd = PIN_FLIST_FLD_GET (in_flistp, PIN_FLD_PASSWD_CLEAR, 0, ebufp);

        svc_flistp= PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, svc_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, svc_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BAL_GRP_OBJ, svc_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
        //PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROFILE_OBJ, svc_flistp, PIN_FLD_PROFILE_OBJ, ebufp);
        //PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DEVICE_OBJ, svc_flistp, PIN_FLD_DEVICE_OBJ, ebufp);
        if (status_svc) {
                status_arr= PIN_FLIST_ELEM_ADD(svc_flistp,PIN_FLD_STATUSES,0, ebufp);
                PIN_FLIST_FLD_SET(status_arr, PIN_FLD_STATUS, status_svc, ebufp);
                PIN_FLIST_FLD_SET(status_arr, PIN_FLD_STATUS_FLAGS, &status_flag, ebufp);
        } else {
                PIN_FLIST_FLD_SET(svc_flistp, PIN_FLD_STATUS, &status, ebufp);
        }
        PIN_FLIST_FLD_SET(svc_flistp, PIN_FLD_STATUS_FLAGS, &status_flag, ebufp);


        if ( login ) {
                PIN_FLIST_FLD_SET(svc_flistp, PIN_FLD_LOGIN, login, ebufp);
        }

        if ( pswd ) {
                PIN_FLIST_FLD_SET(svc_flistp, PIN_FLD_PASSWD_CLEAR, pswd, ebufp);
        }


        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_service: input flist for PCM_OP_CUST_CREATE_SERVICE", svc_flistp);
        PCM_OP(ctxp, PCM_OP_CUST_CREATE_SERVICE, 0, svc_flistp, &ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&svc_flistp,NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_create_service: Error while calling PCM_OP_CUST_CREATE_SERVICE", ebufp);
                PIN_FLIST_DESTROY_EX(&ret_flistp,NULL);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_service: output flist for PCM_OP_CUST_CREATE_SERVICE", ret_flistp)
        *ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        return;

}

/**********************************************************
* This is to create a  billinfo object
**********************************************************/

void
fm_mso_create_billinfo(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t     *input_flistpp = NULL;
        pin_flist_t     *billinfo_flstp = NULL;
        pin_flist_t     *ret_flistp = NULL;
        //pin_flist_t   *srch_flistp = NULL;
        int64   db = 0;
        poid_t  *act_pdp =  NULL;
        poid_t  *bi_pdp =  NULL;
        poid_t  *pi_pdp = NULL;
        int32   currency = 356;
        int32   pay_type = 10001;
        //poid_t        *srch_pdp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                        return;
                PIN_ERRBUF_CLEAR(ebufp);
        act_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ , 0, ebufp);
        input_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, input_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(input_flistpp, PIN_FLD_PROGRAM_NAME, "TEST", ebufp);
        billinfo_flstp = PIN_FLIST_ELEM_ADD(input_flistpp, PIN_FLD_BILLINFO, 1, ebufp);
        db = PIN_POID_GET_DB(act_pdp);
        bi_pdp = PIN_POID_CREATE(db, "/billinfo", -1,ebufp);

        /**
        **Search to get existing billinfo details .The result details will be utilised tocreate new billinfo for Netflix service
        **/
        //srch_pdp = PIN_POID_CREATE(db, /search, -1, ebufp);
        //srch_flistp = PIN_FLIST_CREATE(ebufp);

        PIN_FLIST_FLD_SET(billinfo_flstp, PIN_FLD_POID, bi_pdp, ebufp);
        PIN_FLIST_FLD_SET(billinfo_flstp, PIN_FLD_BILLINFO_ID, "NETFLIX BILL INFO", ebufp);
        PIN_FLIST_FLD_SET(billinfo_flstp, PIN_FLD_CURRENCY_SECONDARY, 0, ebufp);
        PIN_FLIST_FLD_SET(billinfo_flstp, PIN_FLD_CURRENCY, &currency, ebufp);
        PIN_FLIST_FLD_SET(billinfo_flstp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
        PIN_FLIST_FLD_SET(billinfo_flstp, PIN_FLD_PAYINFO_OBJ, pi_pdp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_billinfo:PCM_OP_CREATE_BILLINFO INPUTFLIST", input_flistpp);
        PCM_OP(ctxp,PCM_OP_CUST_CREATE_BILLINFO, 0, input_flistpp, &ret_flistp, ebufp);
        PIN_DESTORY_EX(&input_flistpp,NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_create_billinfo:Error while calling PCM_OP_CREATE_BILLINFO", ebufp);
                return;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_billinfo:PCM_OP_CREATE_BILLINFO outplist", ret_flistp);
        *ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

        return;
}

/**********************************************************
* This is to create a  bal_grp  object
**********************************************************/

void
fm_mso_create_balgrp(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{

         pin_flist_t     *input_flistp = NULL;
         pin_flist_t     *bal_inf_flstp = NULL;
         pin_flist_t     *ret_flistp = NULL;
         poid_t  *act_pdp = NULL;
         poid_t *bal_pdp = NULL;
         time_t  pvt ;
         int64 db = 0;

         if (PIN_ERRBUF_IS_ERR(ebufp))
                         return;
                 PIN_ERRBUF_CLEAR(ebufp);

         act_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0 ,ebufp);
         input_flistp = PIN_FLIST_CREATE(ebufp);
         db = PIN_POID_GET_DB(act_pdp);
         bal_pdp=PIN_POID_CREATE(db, "/balance_group", -1, ebufp);
         //PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_POID, act_pdp, ebufp);
         PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_POID, bal_pdp, ebufp);
         PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

         PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BILLINFO_OBJ, input_flistp, PIN_FLD_BILLINFO_OBJ, ebufp);
         //PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, bal_inf_flstp, PIN_FLD_SERVICE_OBJ, ebufp);
         PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_balgrp:PCM_OP_CUST_CREATE_BAL_GRP INPUTFLIST", input_flistp);
         PCM_OP(ctxp, PCM_OP_CUST_CREATE_BAL_GRP, 0 , input_flistp, &ret_flistp, ebufp);

         //PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_balgrp:PCM_OP_CREATE_BILLINFO PCM_OP_CUST_SET_BAL_GRP  outplist", ret_flistp);
         PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_create_balgrp:PCM_OP_CUST_CREATE_BAL_GRP  outplist", ret_flistp);
         *ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
         //PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

         return;

}

/**********************************************************
* This is to purchase  a  deal
**********************************************************/

void
fm_mso_purchase_deal(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t     *input_flistp = NULL;
        pin_flist_t     *ret_flistp = NULL;
        pin_flist_t     *dl_inf_flistp  = NULL;
        pin_flist_t     *prd_arr_flstp = NULL;
        poid_t  *act_pd = NULL;
        pin_decimal_t   *qty =  pbo_decimal_from_str("1",ebufp);
        pin_decimal_t   *usage_dsct =  pbo_decimal_from_str("0",ebufp);
        pin_decimal_t   *cycle_dsct =  pbo_decimal_from_str("0",ebufp);
        pin_decimal_t   *pur_dsct =  pbo_decimal_from_str("0",ebufp);
        int32           status = 1;
        int32           end_details = 2;
        int32           start_details = 1;
        int32           status_flg = 16777216;
        int32           *start_offset;
        int32           *start_unit;
        int64           db = 0;
        poid_t          *product_obj = NULL;
        time_t          *end_t;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR(ebufp);

        act_pd = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        end_t = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_END_T,1,ebufp);
        input_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_POID, act_pd, ebufp);
        if(end_t)
        {
                PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_END_T, end_t, ebufp);
        }
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, input_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
        PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_PROGRAM_NAME, "Netflix Purchase", ebufp);
        dl_inf_flistp = PIN_FLIST_SUBSTR_ADD(input_flistp, PIN_FLD_DEAL_INFO, ebufp);
        prd_arr_flstp = PIN_FLIST_ELEM_ADD(dl_inf_flistp, PIN_FLD_PRODUCTS, 0, ebufp);

        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PRODUCT_OBJ, prd_arr_flstp, PIN_FLD_PRODUCT_OBJ, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_QUANTITY, qty, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_CYCLE_FEE_AMT, NULL, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_PURCHASE_FEE_AMT, NULL, ebufp);

        start_offset = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_CYCLE_START_OFFSET,1,ebufp);
        if(start_offset)
        {
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_USAGE_START_OFFSET, start_offset, ebufp);
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_CYCLE_START_OFFSET, start_offset, ebufp);
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_PURCHASE_START_OFFSET, start_offset, ebufp);
        }
        else
        {
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_USAGE_START_OFFSET, 0, ebufp);
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_CYCLE_START_OFFSET, 0, ebufp);
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_PURCHASE_START_OFFSET, 0, ebufp);
        }

        start_unit = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_CYCLE_START_UNIT,1,ebufp);
        if(start_unit)
        {
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_USAGE_START_UNIT, start_unit, ebufp);
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_PURCHASE_START_UNIT, start_unit, ebufp);
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_CYCLE_START_UNIT, start_unit, ebufp);
        }
        else
        {
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_USAGE_START_UNIT, 0, ebufp);
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_PURCHASE_START_UNIT, 0, ebufp);
                PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_CYCLE_START_UNIT, 0, ebufp);
        }


        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_USAGE_END_DETAILS, &end_details, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_PURCHASE_END_DETAILS, &end_details, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_CYCLE_END_DETAILS, &end_details, ebufp);

        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_PURCHASE_START_DETAILS, &start_details, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_USAGE_START_DETAILS, &start_details, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_CYCLE_START_DETAILS, &start_details, ebufp);

        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_PURCHASE_START_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_USAGE_START_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_CYCLE_START_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_PURCHASE_END_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_USAGE_END_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_CYCLE_END_T, NULL, ebufp);

        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CYCLE_END_OFFSET, prd_arr_flstp, PIN_FLD_CYCLE_END_OFFSET, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CYCLE_END_OFFSET, prd_arr_flstp, PIN_FLD_PURCHASE_END_OFFSET, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CYCLE_END_OFFSET, prd_arr_flstp, PIN_FLD_USAGE_END_OFFSET, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CYCLE_END_UNIT, prd_arr_flstp, PIN_FLD_CYCLE_END_UNIT, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CYCLE_END_UNIT, prd_arr_flstp, PIN_FLD_USAGE_END_UNIT, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CYCLE_END_UNIT, prd_arr_flstp, PIN_FLD_PURCHASE_END_UNIT, ebufp);

        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_USAGE_DISCOUNT, usage_dsct, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_DESCR, "Netflix Charges-SUBSCRIPTION", ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_CYCLE_DISCOUNT, cycle_dsct, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_PURCHASE_DISCOUNT, pur_dsct, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_STATUS, &status, ebufp);
        PIN_FLIST_FLD_SET(prd_arr_flstp, PIN_FLD_STATUS_FLAGS, &status_flg, ebufp);

        PIN_FLIST_FLD_SET(dl_inf_flistp, PIN_FLD_NAME, "Netflix Charges-DEAL", ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DEAL_OBJ, dl_inf_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PLAN_OBJ, dl_inf_flistp, PIN_FLD_PLAN_OBJ, ebufp);
        PIN_FLIST_FLD_SET(dl_inf_flistp, PIN_FLD_START_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(dl_inf_flistp, PIN_FLD_END_T, NULL, ebufp);
        PIN_FLIST_FLD_SET(dl_inf_flistp, PIN_FLD_FLAGS, 0, ebufp);
        PIN_FLIST_FLD_SET(dl_inf_flistp, PIN_FLD_DESCR, "Netflix Charges-DEAL", ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Subscription Purchase deal Input Flist");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_deal:PCM_OP_SUBSCRIPTION_PURCHASE_DEAL INPUTFLIST", input_flistp);

        PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_PURCHASE_DEAL, 0 , input_flistp, &ret_flistp, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Subscription Purchase deal return Flist");
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_deal:PCM_OP_SUBSCRIPTION_PURCHASE_DEAL  output_flist", ret_flistp);

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_purchase_deal:Error while calling PCM_OP_SUBSCRIPTION_PURCHASE_DEAL", ebufp);
                return;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_purchase_deal output_flist", ret_flistp);
        *ret_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        //PIN_FLIST_DESTORY_EX(&input_flistp,NULL);
        return;

}

void fm_mso_utils_get_act_bal_grp
(
    pcm_context_t       *ctxp,
    poid_t              *act_pdp,
    poid_t              **ret_bal_grp_poidp,
    pin_errbuf_t        *ebufp)
{
        pin_flist_t      *srch_i_flistp = NULL;
        pin_flist_t      *srch_o_flistp = NULL;
        pin_flist_t      *srch_res_flistp = NULL;
        pin_flist_t      *results_flistp = NULL;
        pin_flist_t   *args_flistp = NULL;
        char            template[] = "select X from /balance_group where F1 = V1 ";
        int             database = 0;
        poid_t          *srch_pdp = NULL;
        poid_t          *service_pdp = NULL;
        int             srch_flags = 256;

        if(act_pdp && strcmp(PIN_POID_GET_TYPE(act_pdp),"/account")==0) {
                srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(PCM_GET_USERID(ctxp)),"/search",-1,ebufp);

                srch_i_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(srch_i_flistp,PIN_FLD_POID,(void *)srch_pdp,ebufp);
                PIN_FLIST_FLD_SET(srch_i_flistp,PIN_FLD_FLAGS,(void *)&srch_flags,ebufp);
                PIN_FLIST_FLD_SET(srch_i_flistp,PIN_FLD_TEMPLATE,(void *)template,ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp,PIN_FLD_ARGS,1,ebufp);
                PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_ACCOUNT_OBJ,(void *)act_pdp,ebufp);
                srch_res_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp,PIN_FLD_RESULTS,0,ebufp);
                PIN_FLIST_FLD_SET(srch_res_flistp,PIN_FLD_POID,NULL,ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_act_bal_grp search input flist", srch_i_flistp);

                PCM_OP(ctxp,PCM_OP_SEARCH,0,srch_i_flistp,&srch_o_flistp,ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error during search",ebufp);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_get_act_bal_grp search input flist", srch_o_flistp);
                results_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp,PIN_FLD_RESULTS,0,1,ebufp);
                if(results_flistp != NULL) {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Valid Balance Group Found");
                        *ret_bal_grp_poidp = PIN_FLIST_FLD_TAKE(results_flistp,PIN_FLD_POID,0,ebufp);
                }
                else {
                        *ret_bal_grp_poidp = NULL;
                }
        }
        else{
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Invalid Poid_type");
        }
cleanup :
        PIN_FLIST_DESTROY_EX(&srch_o_flistp,ebufp);
        PIN_FLIST_DESTROY_EX(&srch_i_flistp,ebufp);
        return;
}

void
fm_mso_validate_tal_plan(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        char                            s_template1[300] = " select X from /config/product/tal  where F1 = V1 ";
        char                            s_template2[300] = " select X from /mso_cfg_credit_limit  where F1 = V1 ";
        pin_flist_t                     *s_flistp = NULL;
        pin_flist_t                     *args_flistp= NULL;
        pin_flist_t                     *s_oflistp=NULL;
        pin_flist_t                     *plan_flist=NULL;
        pin_flist_t                     *plan_arr=NULL;
        pin_flist_t                     *results_flistp = NULL;
        pin_flist_t                     *city_flistp = NULL;

        poid_t                          *s_pdp = NULL;
        int32                           flags = 512;
        int32                           db = 1;
        int32                           id = -1;
        int32                   *bill_when = NULL;
        poid_t                          *plan_pdp=NULL;

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside fm_mso_validate_tal_plan");
        plan_flist = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PLAN_LISTS,PIN_ELEMID_ANY,1, ebufp);
        plan_pdp = PIN_FLIST_FLD_GET(plan_flist, PIN_FLD_PLAN_OBJ, 1, ebufp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        plan_arr = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PLAN, 0, ebufp);
        PIN_FLIST_FLD_SET(plan_arr, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template1, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search config_product_tal Input flist:",s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 512, s_flistp, &s_oflistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search config_product_tal Input flist:",s_oflistp);
        if (s_oflistp) {
                results_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                if (results_flistp) {
                        *r_flistpp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_PLAN, PIN_ELEMID_ANY, 1, ebufp);
                }
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search config_product_tal plan flist:",*r_flistpp);
        PIN_FLIST_DESTROY_EX(&s_flistp,NULL);


        //Search for bill_when for the tal_plan
        if(*r_flistpp) {
                s_flistp = PIN_FLIST_CREATE(ebufp);
                s_oflistp = NULL;
                s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
                PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,s_template2,ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
                city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
                PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, NULL, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Input flist:",s_flistp);
                PCM_OP(ctxp, PCM_OP_SEARCH, 512, s_flistp, &s_oflistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit OutFlist flist:",s_oflistp);
                results_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                city_flistp = PIN_FLIST_ELEM_GET(results_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, 1, ebufp);
                bill_when = PIN_FLIST_FLD_GET(city_flistp, PIN_FLD_BILL_WHEN,1, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_BILL_WHEN, bill_when, ebufp);
        }

return;
}

/*******************************************************
* Function  : fm_mso_hier_group_get_parent()
* Decription: function to fetch parent service details 
*	for the service object passed
*******************************************************/
void
fm_mso_hier_group_get_parent(
	pcm_context_t		*ctxp,
        poid_t                  *serv_pdp,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *args_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	poid_t                  *s_pdp = NULL;
	int32                   flags = 256;
	int32			active_status = 10100;
	int64                   db = 0;
	int64                   id = -1;
	char                    *s_template = "select X from /group/mso_hierarchy where F1 = V1 and F2 = V2 ";
	pin_flist_t		*ret_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	db = PIN_POID_GET_DB(serv_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_MEMBERS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_OBJECT, serv_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &active_status, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_MEMBERS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_OBJECT, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hier_group_get_parent search input list", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_hier_group_get_parent - Error in calling SEARCH", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_hier_group_get_parent output flist", ret_flistp);
	res_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (res_flistp)
	{
		tmp_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_MEMBERS, 0, 0, ebufp);
		*ret_flistpp = PIN_FLIST_COPY(tmp_flistp, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
}

void
fm_mso_fetch_offer_purchased_plan(
	pcm_context_t           *ctxp,
        poid_t                  *acc_poidp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*search_flistp = NULL;
	pin_flist_t		*search_res_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;

	poid_t			*search_pdp = NULL;
        char			search_template[256] = "select X from /mso_cfg_promotional_offers 1, /mso_purchased_plan 2 where 2.F1 = V1 and 2.F2 = V2 and 1.F3 = 2.F4  ";

        char			search_template1[256] = "select X from /mso_cfg_promotional_offers where F1 = V1 ";

	char			*plan_typep = "base plan";
	int64			db = -1;
        int32			search_flags = 0;
	int32			mso_plan_status = 2;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return ;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_fetch_offer_purchased_plan:input account poid", acc_poidp);
	
        db = PIN_POID_GET_DB(acc_poidp);

        search_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        search_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(search_flistp, PIN_FLD_POID, search_pdp, ebufp);
        PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_FLAGS, &search_flags, ebufp);
	if (strcmp(PIN_POID_GET_TYPE(acc_poidp), "/account") == 0)
	{
        	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template, ebufp);

        	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acc_poidp, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, plan_typep, ebufp);

        	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_SUBSCRIPTION_PLAN_OBJ, NULL, ebufp);

        	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);

		//args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 5, ebufp);
		//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &mso_plan_status, ebufp);
	}
	else
	{
        	PIN_FLIST_FLD_SET(search_flistp, PIN_FLD_TEMPLATE, search_template1, ebufp);

        	args_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_SUBSCRIPTION_PLAN_OBJ, acc_poidp, ebufp);
	}
        result_flistp = PIN_FLIST_ELEM_ADD(search_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_fetch_offer_purchased_plan: search input flistp", search_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_res_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&search_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching MSO Purchased plan offer", ebufp);
                PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_fetch_offer_purchased_plan: search output flist", search_res_flistp);
        result_flistp = PIN_FLIST_ELEM_GET(search_res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if (result_flistp)
	{
		*o_flistpp = PIN_FLIST_COPY(result_flistp, ebufp);
        } 
        PIN_FLIST_DESTROY_EX(&search_res_flistp, NULL);
	return;
}

/*********************************************************************
* Function  	: fm_mso_get_acct_billinfo()
* Input		: Account POID
* Output	: void; ebuf set if errors encountered
* Decription	: Function to billinfo details based on the 
* 		  Account Object passed.
*********************************************************************/	
void fm_mso_get_acct_billinfo(
	pcm_context_t	*ctxp,
	poid_t		*acnt_pdp,
	pin_flist_t	**bill_infop,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*srch_flistp = NULL;
	pin_flist_t	*srch_oflistp = NULL;
	pin_flist_t	*arg_flist = NULL;
	pin_flist_t	*result_flist = NULL;

	int32		srch_flag = 768;
	int64		db = 1;
	
	char		*template = " select X from /billinfo where F1 = V1 ";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_acct_billinfo", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acct_billinfo :");
	
	if(acnt_pdp)
	{			
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

		result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILLINFO_ID, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_AR_BILLINFO_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_PARENT_BILLINFO_OBJ, NULL, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acct_billinfo search billinfo input list", srch_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling Search Opcode", ebufp);
			return;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_acct_billinfo search billinfo output list", srch_oflistp);

		result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
		if (result_flist)
		{
			*bill_infop = PIN_FLIST_COPY(result_flist, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flist, NULL);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Bill Info Object not found");
			goto CLEANUP;
		}
		
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account Object NULL");
		goto CLEANUP;
	}
	
CLEANUP:
	PIN_FLIST_DESTROY_EX(&result_flist, NULL);
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	
	return;
}

/************************************************************************
 * validate_hybrid_acnt_type()
 *
 * Inputs: flist with BusinessType, BillInfoID are mandatory fields 
 *
 * Output: void; ebuf set if errors encountered
 *
 * Description:
 * This function is validates the given BusinessType and BillInfoID of 
 * and returns the AccountType. 
 ************************************************************************/
void 
validate_hybrid_acnt_type(
	pcm_context_t	*ctxp,
	int32 		*business_typep,
	char		*billinfo_idp,
	char		**acnt_type,
	pin_errbuf_t	*ebufp)
{
	int32   	account_type = 0;
	int32   	account_model = 0;
	int32   	subscriber_type = 0;
	char 		*acnt_type_frm_bustype = NULL;
	char 		*acnt_type_frm_billinfoid = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling validate_hybrid_acnt_type", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "validate_hybrid_acnt_type :");
	
	if(business_typep != NULL && *business_typep != 0)
	{
		fm_mso_get_business_type_values(ctxp, *business_typep, &account_type, &account_model, &subscriber_type, ebufp);
				
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "account_type >>");
		/*PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)itoa(account_type));*/
		
		if (account_type == ACCT_TYPE_LCO)
		{			
			acnt_type_frm_bustype = ACTTYPE_LCO;			
		}
		else if (account_type == ACCT_TYPE_SUBSCRIBER)
		{
			acnt_type_frm_bustype = ACTTYPE_CATV;
		}
		else if ( (account_type == ACCT_TYPE_CORP_SUBS_BB) || (account_type == ACCT_TYPE_RESERVED_BB) || 
				  (account_type == ACCT_TYPE_RETAIL_BB) || (account_type == ACCT_TYPE_SME_SUBS_BB) )
		{
			acnt_type_frm_bustype = ACTTYPE_ISP;			
		}
		else 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account Type NULL");			
		}
		
		if(acnt_type_frm_bustype != NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_type_frm_bustype >>");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, acnt_type_frm_bustype);
		}
	}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Business Type is Null", ebufp);		
	}
		
	if(billinfo_idp && strlen(billinfo_idp) != 0)
	{
		if (strcmp(billinfo_idp, ACTTYPE_ISP) == 0)
		{
			acnt_type_frm_billinfoid = ACTTYPE_ISP;			
		}
		else if (strcmp(billinfo_idp, ACTTYPE_CATV) == 0)
		{
			acnt_type_frm_billinfoid = ACTTYPE_CATV;			
		}
		else
		{
			acnt_type_frm_billinfoid = "";
		}
		
		if(acnt_type_frm_billinfoid != NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "acnt_type_frm_billinfoid >>");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, acnt_type_frm_billinfoid);
		}		
	}
	else
	{
	//	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Bill Info ID is Null", ebufp);		
	}
	
	if ((acnt_type_frm_bustype != NULL) && (acnt_type_frm_billinfoid != NULL) &&
		  strcmp(acnt_type_frm_bustype, acnt_type_frm_billinfoid) == 0)
	{
		*acnt_type = acnt_type_frm_bustype;
	}
	else if (((acnt_type_frm_bustype != NULL) && (strcmp(acnt_type_frm_bustype, ACTTYPE_LCO) != 0)) && 
			  (acnt_type_frm_billinfoid == NULL || strlen(acnt_type_frm_billinfoid) == 0))
	{
		*acnt_type = acnt_type_frm_bustype;		
	}
	else if ((acnt_type_frm_bustype == NULL) && (acnt_type_frm_billinfoid != NULL))
	{
		*acnt_type = acnt_type_frm_billinfoid;		
	}
	else if ((acnt_type_frm_bustype == NULL) && (acnt_type_frm_billinfoid == NULL))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Account Types for the given BillInfoID and BusinessType are invalid", ebufp);	
		*acnt_type = ACTTYPE_INVALID;	
	}
	else
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Given BillInfoID and BusinessType are of different Account Types", ebufp);	
		*acnt_type = ACTTYPE_INVALID;			
	}
	
    return;
}

int32 
fm_mso_is_den_sp(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*acct_flistp = NULL;
	pin_flist_t		*out_flistp = NULL;

	pin_cookie_t		cookie = NULL;

	poid_t			*pdp = NULL;
	poid_t          *jv_pdp = NULL;

	int32			*business_type = NULL;
	int32			profile_srch_type = MSO_FLAG_SRCH_BY_SELF_ACCOUNT;
	int32			den_sp = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return den_sp;
	PIN_ERRBUF_CLEAR(ebufp);

	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
	if (pdp)
	{
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_PROFILE_DESCR, WHOLESALE, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_SEARCH_TYPE, &profile_srch_type, ebufp);
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_OBJECT, pdp, ebufp);

		fm_mso_get_profile_info(ctxp, srch_flistp, &srch_out_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

		if (!srch_out_flistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "There is no profile object");
			goto CLEANUP;
		}	

		acct_flistp = PIN_FLIST_SUBSTR_GET(srch_out_flistp, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
		if (acct_flistp)
		{
			pdp = PIN_FLIST_FLD_GET(acct_flistp, MSO_FLD_COMPANY_OBJ, 1, ebufp);
            if (!PIN_POID_IS_NULL(pdp))
            {
					fm_mso_get_account_info(ctxp, pdp, &out_flistp, ebufp);
					business_type = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_BUSINESS_TYPE, 0, ebufp);
                    if (*business_type == 33000000)
                    {
                        den_sp = 1;
                    }
                    else if(*business_type == 44000000)
                    {
                        den_sp = 2;
                    }
                    else
                    {
                        den_sp = 0;
                    }
                    PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
            }
		}
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	}
CLEANUP:
	return den_sp;
}

void
fm_mso_cust_create_bill_adj_sequence(
        pcm_context_t   *ctxp,
        poid_t          *account_pdp,
        int32           flag,                   //1 - Adjustment , 2 - Bill , 3 - Both
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *create_seq_in_flistp = NULL;
        pin_flist_t     *arg_flistp = NULL;

        int64           db = 1;

        poid_t          *seq_pdp = NULL;

        pin_cookie_t    cookie = NULL;

        int32           head_num = 0;

        char            *adj_head_str = ":-:ADJUSTMENT";
        char            *bill_head_str = ":-:BILL";
        char            *adj_name = "ADJUSTMENT";
        char            *bill_name = "BILL";

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_cust_create_bill_adj_sequence");

        if (PIN_ERR_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "error before fm_mso_cust_create_bill_adj_sequence", ebufp);
                return;
        }

        if (flag == 1 || flag == 3)
        {
                seq_pdp = PIN_POID_CREATE(db, "/data/sequence", -1, ebufp);
                create_seq_in_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(create_seq_in_flistp, PIN_FLD_POID, seq_pdp, ebufp);
                PIN_FLIST_FLD_SET(create_seq_in_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
                PIN_FLIST_FLD_SET(create_seq_in_flistp, PIN_FLD_HEADER_NUM, &head_num , ebufp);
                PIN_FLIST_FLD_SET(create_seq_in_flistp, PIN_FLD_HEADER_STR, adj_head_str , ebufp);
                PIN_FLIST_FLD_SET(create_seq_in_flistp, PIN_FLD_NAME, adj_name , ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adjustment Sequence input flist", create_seq_in_flistp);
                PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_seq_in_flistp, r_flistpp, ebufp);
                PIN_FLIST_DESTROY_EX(&create_seq_in_flistp, NULL);
                if (PIN_ERR_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating adj sequence", ebufp);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adjustment Sequence output flist", *r_flistpp);
        }
        if(flag == 2 || flag == 3)
        {
                PIN_FLIST_DESTROY_EX(r_flistpp, NULL);
                seq_pdp = PIN_POID_CREATE(db, "/data/sequence", -1, ebufp);
                create_seq_in_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(create_seq_in_flistp, PIN_FLD_POID, seq_pdp, ebufp);
                PIN_FLIST_FLD_SET(create_seq_in_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
                PIN_FLIST_FLD_SET(create_seq_in_flistp, PIN_FLD_HEADER_NUM, &head_num , ebufp);
                PIN_FLIST_FLD_SET(create_seq_in_flistp, PIN_FLD_HEADER_STR, bill_head_str , ebufp);
                PIN_FLIST_FLD_SET(create_seq_in_flistp, PIN_FLD_NAME, bill_name , ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Billing Sequence input flist", create_seq_in_flistp);
                PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_seq_in_flistp, r_flistpp, ebufp);
                PIN_FLIST_DESTROY_EX(&create_seq_in_flistp, NULL);
                if (PIN_ERR_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in creating bill sequence", ebufp);
                        return;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Billing Sequence output flist", *r_flistpp);
        }
        return;
}

/*************************************************************
* Function  : fm_mso_cust_get_pp_type()
* Decription: Function to fetch PP_TYPE of a CATV Customer
*************************************************************/
void
fm_mso_cust_get_pp_type(
	pcm_context_t	    *ctxp,
        poid_t              *acct_pdp,
        pin_flist_t         **ret_flistpp,
        pin_errbuf_t        *ebufp)
{
	pin_flist_t	*srch_in_flistp = NULL;
	pin_flist_t	*arg_flist = NULL;
	pin_flist_t	*result_flist = NULL;
	pin_flist_t	*srch_out_flistp = NULL;
	pin_flist_t	*profile_substr = NULL;

	int32		srch_flag = 512;	
	poid_t		*srch_pdp = NULL;

	char		*template = " select X from /profile 1, /profile/customer_care 2 where 1.F1.id = V1 and 1.F2 = 2.F3 ";
	int64		db = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_cust_get_pp_type", ebufp);
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_cust_get_pp_type");

	db = PIN_POID_GET_DB(acct_pdp);
	
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);
	
	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pp_type search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pp_type search output list", srch_out_flistp);
	
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pp_type result_flist flist", result_flist);
	if (result_flist)
	{
		profile_substr = PIN_FLIST_SUBSTR_GET(result_flist, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_pp_type profile_substr flist", profile_substr);
	    if (!profile_substr)
        {
            profile_substr = PIN_FLIST_SUBSTR_GET(result_flist, MSO_FLD_WHOLESALE_INFO, 1, ebufp);
        }
		if(profile_substr != NULL) 
		{
			*ret_flistpp = PIN_FLIST_COPY(profile_substr, ebufp);
		}			
	}
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

/************************************************************
* Function  : fm_mso_cust_get_acc_from_sno()
* Decription: Function to retrive account and service objects
* of a given serial number. 
*************************************************************/
void
fm_mso_cust_get_acc_from_sno(
	pcm_context_t	*ctxp,
	pin_flist_t     *i_flistp,
    pin_flist_t     **ret_flistpp,
    pin_errbuf_t    *ebufp)
{
	pin_flist_t	*srch_in_flistp = NULL;
	pin_flist_t	*result_flist = NULL;
	pin_flist_t	*srch_out_flistp = NULL;
	pin_flist_t	*services_flistp = NULL;
	poid_t		*acc_pdp = NULL;
	int32		srch_flag = 122;	
	poid_t		*srch_pdp = NULL;

	int64		db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(acc_pdp);
	
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, srch_in_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, srch_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_STB_ID, srch_in_flistp, MSO_FLD_SERIAL_NO, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

	PCM_OP(ctxp, MSO_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_get_acc_from_sno search output list", srch_out_flistp);
	
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (result_flist)
	{
		services_flistp = PIN_FLIST_ELEM_GET(result_flist, PIN_FLD_SERVICES, 0, 1, ebufp);
		
		if (services_flistp != NULL) 
		{
			*ret_flistpp = PIN_FLIST_COPY(services_flistp, ebufp);
	        PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_DEVICE_ID, *ret_flistpp, PIN_FLD_DEVICE_ID, ebufp);
	        PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_ACCOUNT_NO, *ret_flistpp, PIN_FLD_ACCOUNT_NO, ebufp);
		}			
	}
CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

/*******************************************************************
 * fm_inv_pol_update_curr_total()
 *
 * This function calculates current total and updates bill's current
 * total with new one.
 *******************************************************************/
pin_decimal_t *
get_ar_curr_total(
        pcm_context_t   *ctxp,
        poid_t          *ar_billinfo_pdp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *i_flistp = NULL;
        pin_flist_t     *o_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
        pin_flist_t     *temp_flistp = NULL;
        poid_t          *s_pdp = NULL;
        pin_decimal_t   *zerop = NULL;
        pin_decimal_t   *overduep = NULL;
        pin_decimal_t   *p_totalp = NULL;
        time_t          start_t = 0;
        void            *vp = NULL;
        int32           s_flags  = 1;
        char            templatep[2001];

        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        i_flistp = PIN_FLIST_CREATE(ebufp);

        /********************************************************
         * Prepare the input flist to get the previous_total from
         * outstanding items.
         ********************************************************/

        s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(ar_billinfo_pdp),
                                        "/search", -1, ebufp);

        PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        sprintf(templatep,
        "select sum( F2 ) from /item where  F1 = V1 ");
        PIN_FLIST_FLD_SET (i_flistp, PIN_FLD_TEMPLATE, (void *)templatep, ebufp);

        temp_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_AR_BILLINFO_OBJ, ar_billinfo_pdp, ebufp);

        zerop = pbo_decimal_from_str("0.0",ebufp);

        temp_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_PUT(temp_flistp, PIN_FLD_ITEM_TOTAL, (void *)zerop, ebufp);

        temp_flistp = PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_AMOUNT, NULL, ebufp);

        /**********************************************************
         * Do search to caculate over due amount
         **********************************************************/
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_update_curr_total search flist", i_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, i_flistp, &o_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
        if (PIN_ERR_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_inv_pol_update_curr_total search failed", ebufp);
                goto CLEANUP;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_inv_pol_update_curr_total output flist", o_flistp);

        res_flistp = PIN_FLIST_ELEM_GET(o_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,
            1, ebufp);
        if (res_flistp)
        {
                overduep = (pin_decimal_t *) PIN_FLIST_ELEM_TAKE(res_flistp,
                                PIN_FLD_AMOUNT, PIN_ELEMID_ANY, 1, ebufp);
                if (pbo_decimal_is_null(overduep, ebufp))
                {
                        overduep = pbo_decimal_from_str("0.0",ebufp);
                }
        }

CLEANUP:
        PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
        return overduep;
}
