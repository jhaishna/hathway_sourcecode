
/*******************************************************************
 * File:        fm_mso_commission_process_commission.c
 * Opcode:      MSO_OP_COMMISSION_PROCESS_COMMISSION
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_commission_process_commission.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "pin_rate.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_commission.h"

/**********************************
External subroutines - start
**********************************/
extern int32
fm_mso_utils_get_account_type(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_utils_prepare_status_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        int                     flag,
        char                    *status_descr,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_commission_record_failure (
        pcm_context_t           *ctxp,
        char                    *event_type,
        pin_flist_t             *i_flistp,
        pin_flist_t             *err_flistp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_utils_prepare_error_flist(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        char                    *error_code,
        char                    *error_descr,
        pin_errbuf_t            *ebufp);

extern void 
fm_mso_utils_get_bal_grp (
    pcm_context_t       	*ctxp,
    poid_t              	*act_pdp,
    char			*flags,
    poid_t             		**ret_bal_grp_poidp,
    poid_t             		**ret_serv_poidp,
    pin_errbuf_t       		*ebufp);

extern int
fm_mso_commission_acct_has_agreement(
        pcm_context_t           *ctxp,
        poid_t                  *lco_acct_objp,
        pin_errbuf_t            *ebufp) ;


extern void
fm_mso_utils_get_item_type (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_get_bals_credit_limit_bal (
        pcm_context_t           *ctxp,
        poid_t                  *account_poidp,
        poid_t                  *bal_grp_poidp,
        pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_util_get_customer_lco_info(
        pcm_context_t           *ctxp,
        poid_t                  *acnt_pdp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp);

extern void
fm_mso_utils_read_object (
        pcm_context_t           *ctxp,
        int32                   flags,
        poid_t                  *objp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

extern
void fm_mso_utils_get_service_details(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	int32			serv_type,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

extern int 
fm_mso_commission_get_vald_lco_acct_comm_mdl (
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/**********************************
External subroutines - end
**********************************/

EXPORT_OP void
op_mso_commission_process_commission (
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);


static void
fm_mso_commission_process_commission (
	pcm_context_t		 *ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);


static void
fm_mso_commission_aggregate_charge_tax_amt (
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static int
fm_mso_commission_prep_commission_info (
	pcm_context_t		*ctxp,
	pin_flist_t		*lcp_flistp,
	pin_flist_t		*data_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);		

static void
fm_mso_commission_extract_transfer_items (
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)	;


static int
fm_mso_commission_lco_bal_trans	(
	pcm_context_t		*ctxp,
	int32			commission_flag,
	int32			from_bucket,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static int
fm_mso_commission_lco_bal_impact (
	pcm_context_t		*ctxp,
	int32			commission_flag,
	int32			from_bucket,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static int
fm_mso_commission_get_lco_share	(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**lco_share_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_commission_create_report	(
	pcm_context_t		*ctxp,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern int
fm_mso_get_bu_commission_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flist,
	pin_flist_t		**out_flist,
	int32			flags,
	pin_errbuf_t		*ebufp);


static void
fm_mso_commission_calc_tax_share (
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_commission_process_dt_sdt_commission (
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	int32			flags,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_get_open_credit_item_due	(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_decimal_t		*total_duep,
	pin_errbuf_t		*ebufp);


void
op_mso_commission_process_commission (
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)	
{

	pcm_context_t		*ctxp =	connp->dm_ctx;
	int32			*status	= NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check
	 *******************************************************************/
	if (opcode != MSO_OP_COMMISSION_PROCESS_COMMISSION) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_commission_process_commission error",ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_commission_process_commission input flist ", i_flistp);

	fm_mso_commission_process_commission(ctxp, flags, i_flistp, r_flistpp, ebufp);
	if(*r_flistpp)
		status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp) || (status && *status == 1))
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_commission_process_commission error: ", i_flistp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_commission_process_commission output flist", *r_flistpp);
	return;
}

static void
fm_mso_commission_process_commission(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*lco_flistp=NULL;
	pin_flist_t		*event_chrgs_flistp=NULL;
	pin_flist_t		*err_flistp=NULL;
	pin_flist_t		*comm_flistp=NULL;
	pin_flist_t		*r_comm_flistp=NULL;
	pin_flist_t		*dtr_comm_flistp=NULL;
	pin_flist_t		*r_dtr_comm_flistp=NULL;
	pin_flist_t		*item_r_flistp=NULL;
	pin_flist_t		*item_flistp=NULL;
	pin_flist_t		*lco_bal_trans_r_flistp=NULL;
	//pin_flist_t		*lco_bal_impact_r_flistp=NULL;
	pin_flist_t		*comm_obj_flistp=NULL;
	pin_flist_t		*r_lco_bal_grp_flistp=NULL;
	pin_flist_t		*bal_flistp=NULL;
	//pin_flist_t		*lco_calc_comm_flistp=NULL;
	pin_flist_t		*share_info_flistp = NULL;
	//pin_flist_t		*amount_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*adj_flistp = NULL;
	pin_flist_t		*adj_o_flistp = NULL;
	pin_flist_t		*r_item_type_flistp = NULL;
	pin_flist_t		*lco_share_flistp = NULL;
	pin_flist_t		*charge_head_flistp = NULL;
	//pin_flist_t		*tmp_chrg_head = NULL;
	pin_flist_t		*dtr_flistp = NULL;
	pin_flist_t		*sdt_flistp = NULL;
	pin_flist_t		*service_info = NULL;
//	pin_flist_t		*bb_info = NULL;
//	pin_flist_t		*results = NULL;

	poid_t			*evt_pdp=NULL;
	poid_t			*serv_pdp=NULL;
	poid_t			*acct_pdp=NULL;
	poid_t			*lco_acct_objp=NULL;
	poid_t			*lco_bal_grp_objp=NULL;
	poid_t			*lco_comm_grp_objp=NULL;
	poid_t			*lco_incollect_serv_objp=NULL;
	poid_t			*lco_outcollect_serv_objp=NULL;
	poid_t			*dtr_acct_objp=NULL;
	poid_t			*sdt_acct_objp=NULL;
	poid_t			*dtr_comm_grp_objp=NULL;
	poid_t			*dtr_outcollect_serv_objp=NULL;
	poid_t			*sdt_comm_grp_objp = NULL;
	poid_t			*sdt_outcollect_serv_objp = NULL;

	int32			acct_type=0;
	int32			commission_rule=0;
	int32			*commission_model=NULL;
	int32			*commission_service=NULL;
	int32			commission_flag=0; /*0 - No commission, 1 - Commission */
	int32			elem_id = 0;
	int32			ielem_id = 0;
	//int32			error_flag = 0;
	int32			ret_val = 0;
	int32			active_cust_count_based_flag = 0;
	//int32			status = 0;
	int32			*comm_status = NULL;
	int32			adj_flag = PIN_AR_WITHOUT_TAX;
//	int32			zero = 0;
	int32			charge_head_based_evt_for_dt = 0;
	int32			charge_head_based_evt_for_lco = 0;
	int32			charge_head_based_evt_for_sdt = 0 ;
	int32			lco_has_valid_comm_model =0 ;
	int32			lco_has_valid_comm_srvc = 0;
	int32			sdt_has_valid_comm_model =0 ;
	int32			sdt_has_valid_comm_srvc = 0;
	int32			dt_has_valid_comm_model =0 ;
	int32			dt_has_valid_comm_srvc = 0;
	int32			*dtr_comm_status=NULL;
	int32			acnt_type_lco = ACCT_TYPE_LCO;
	int32			acnt_type_sdt = ACCT_TYPE_SUB_DTR; 
	int32			acnt_type_dtr = ACCT_TYPE_DTR;
	int32			*sdt_commission_model      = NULL ;
	int32			*sdt_commission_service    = NULL ;
	int32			*dt_commission_model       = NULL ;
	int32			*dt_commission_service     = NULL ;
	int32			comm_applicable_for       = -1 ;
	int32			comm_rev_flag = 0 ;


	//char			*lco_acct_no=NULL;
	char			*evt_type=NULL;
	char			*item_type=NULL;
	char			*serv_type=NULL;
	char			msg[1024];
//	char			tmp[1024];

 	//int64			db=1;

	//pin_decimal_t		*lco_comm_share = NULL;
	//pin_decimal_t		*lco_tax_share = NULL;
	//pin_decimal_t		*hath_share = NULL;
	pin_decimal_t		*lco_credit_limit = NULL;
	pin_decimal_t		*lco_cur_bal = NULL;
	pin_decimal_t		*lco_commission_charge = NULL;
	pin_decimal_t		*lco_bal_after_commission;// = pin_decimal("0.0", ebufp);
	pin_decimal_t		*zerop = pin_decimal("0.00", ebufp);
	pin_decimal_t		*comm_amt = NULL;
	pin_decimal_t		*event_tot = NULL;
	pin_decimal_t		*adj_amt = NULL;

	void			*vp = NULL;
	

	pin_cookie_t		cookie=NULL;
	pin_cookie_t		icookie=NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	if (flags == LCO_COMMISSION)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO_COMMISSION fm_mso_commission_process_commission input flist 1", i_flistp);
	}
	if (flags == DTR_COMMISSION)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DTR_COMMISSION fm_mso_commission_process_commission input flist 1", i_flistp);
	}
	if (flags == SDT_COMMISSION)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SDT_COMMISSION fm_mso_commission_process_commission input flist 1", i_flistp);
	}


	/*Check the event poid & type - Start*/
	evt_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
	evt_type = (char *) PIN_POID_GET_TYPE(evt_pdp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);
	/*Check the event poid & type - End*/

	/*Retrieve LCO Account and its commission model & sevice and set the commission rule-START*/
	if (evt_type != NULL && strstr(evt_type, "/event/billing/settlement/ws/outcollect/active_cust_count_based")) 
	{
		//Get the LCO Account POID and Commission Model & Service from input
		lco_acct_objp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
		commission_model = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
		commission_service = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_COMMISION_SERVICE, 0, ebufp);

		//Set the commission rule 
		active_cust_count_based_flag = 1;

		/***************************************************************************
		 Apply the commission in case of active customer based commission - START
		 **************************************************************************/ 
		if (active_cust_count_based_flag == 1) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Applying commission for Non-Postpaid BB LCO Commission Model");

			//Preparing the commission information flist
			comm_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
			PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_LCO_COMM_BUCKET_OBJ, comm_flistp, MSO_FLD_LCO_COMM_BUCKET_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_LCO_OUTCOLLECT_SERV_OBJ, comm_flistp, MSO_FLD_LCO_OUTCOLLECT_SERV_OBJ, ebufp);

			//Apply the commission 
			PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_lco, ebufp);
			PCM_OP(ctxp, MSO_OP_COMMISSION_APPLY_COMMISSION, 0, comm_flistp, &r_comm_flistp, ebufp);
			comm_status = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_RATING_STATUS, 0, ebufp);
			if (comm_status != NULL && *comm_status != SUCCESSFUL_RATING && *comm_status != NO_MATCHING_SELECTOR_DATA) {
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), comm_flistp, r_comm_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_COMMISSION_FAILURE, 0, 0, 0);
				goto cleanup;
			}
			if (*comm_status != NO_MATCHING_SELECTOR_DATA) {
				comm_amt = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_AMOUNT, 0, ebufp);
				PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_CHARGE, comm_amt, ebufp);
				PIN_FLIST_FLD_COPY(r_comm_flistp, PIN_FLD_EVENT_OBJ, comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
				fm_mso_commission_create_report(ctxp, LCO_COMMISSION, comm_flistp, &comm_obj_flistp, ebufp);
			}
			goto success;

			/*if (r_comm_flistp != NULL) {
				fm_mso_commission_record_failure (ctxp, evt_type, i_flistp, r_comm_flistp, ebufp);
				pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_COMMISSION_FAILURE, 0, 0, 0);
				goto cleanup;
			}*/
			goto success;
		}
		/*************************************************************************
		 Apply the commission in case of active customer based commission - END
		 *************************************************************************/
		commission_rule = ACTIVE_CUST_COUNT_BASED_COMMISSION;
	}
	else if (evt_type != NULL && strstr(evt_type, "/event/billing/settlement/ws/outcollect/charge_head_based")) 
	{
		//Get the LCO Account POID and Commission Model & Service from input
		dtr_acct_objp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DT_OBJ, 1, ebufp);

		if (flags == LCO_COMMISSION)
		{
			charge_head_based_evt_for_lco = 1;
		}
		else if ( flags == SDT_COMMISSION  )
		{
			charge_head_based_evt_for_sdt = 1;
		}
		else if ( flags == DTR_COMMISSION  )
		{
			charge_head_based_evt_for_dt = 1;
		}
		/**************************************************************************
		Apply the commission in case of direct LCO commission - START
		**************************************************************************/
		if (charge_head_based_evt_for_lco == 1) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Applying commission for LCO(direct)");

			comm_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

			//Apply the commission
			PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_lco, ebufp);
			PCM_OP(ctxp, MSO_OP_COMMISSION_APPLY_COMMISSION, 0, comm_flistp, &r_comm_flistp, ebufp);
			comm_status = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_RATING_STATUS, 1, ebufp);
			if (comm_status && *comm_status != SUCCESSFUL_RATING && *comm_status != NO_MATCHING_SELECTOR_DATA) {
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), comm_flistp, r_comm_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_COMMISSION_FAILURE, 0, 0, 0);
				goto cleanup;
			}
			if (*comm_status != NO_MATCHING_SELECTOR_DATA) {
				comm_amt = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_AMOUNT, 0, ebufp);
				PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_CHARGE, comm_amt, ebufp);
				PIN_FLIST_FLD_COPY(r_comm_flistp, PIN_FLD_EVENT_OBJ, comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
				fm_mso_commission_create_report(ctxp, LCO_COMMISSION, comm_flistp, &comm_obj_flistp, ebufp);
			}
			goto success;
		}
		/****************************************************************************
		Apply the commission in case of direct LCO commission - END
		****************************************************************************/

 		/**********************************************************************
		 Apply the commission in case of sub-distributor commission - START
		 *********************************************************************/
		if (charge_head_based_evt_for_sdt == 1) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Applying commission for SDT");

			comm_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
			//PIN_FLIST_FLD_COPY(dtr_flistp, MSO_FLD_DTR_COMM_BUCKET_OBJ, comm_flistp, MSO_FLD_DTR_COMM_BUCKET_OBJ, ebufp);
			//PIN_FLIST_FLD_COPY(dtr_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, comm_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, ebufp);

			//Apply the commission
			PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_sdt, ebufp);
			PCM_OP(ctxp, MSO_OP_COMMISSION_APPLY_COMMISSION, 0, comm_flistp, &r_comm_flistp, ebufp);
			comm_status = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_RATING_STATUS, 0, ebufp);
			if (comm_status != NULL && *comm_status != SUCCESSFUL_RATING && *comm_status != NO_MATCHING_SELECTOR_DATA) {
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), comm_flistp, r_comm_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_DTR_COMMISSION_FAILURE, 0, 0, 0);
				goto cleanup;
			}
			if (*comm_status != NO_MATCHING_SELECTOR_DATA) {
				comm_amt = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_AMOUNT, 0, ebufp);
				PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_CHARGE, comm_amt, ebufp);
				PIN_FLIST_FLD_COPY(r_comm_flistp, PIN_FLD_EVENT_OBJ, comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
				fm_mso_commission_create_report(ctxp, SDT_COMMISSION, comm_flistp, &comm_obj_flistp, ebufp);
			}
			goto success;
		}
		/************************************************************************
		 Apply the commission in case of sub-distributor commission - END
		 ***********************************************************************/


		/**********************************************************************
		 Apply the commission in case of distributor commission - START
		 *********************************************************************/
		if (charge_head_based_evt_for_dt == 1) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Applying commission for DTR");

			comm_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
			//PIN_FLIST_FLD_COPY(dtr_flistp, MSO_FLD_DTR_COMM_BUCKET_OBJ, comm_flistp, MSO_FLD_DTR_COMM_BUCKET_OBJ, ebufp);
			//PIN_FLIST_FLD_COPY(dtr_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, comm_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, ebufp);

			//Apply the commission
			PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_dtr, ebufp);
			PCM_OP(ctxp, MSO_OP_COMMISSION_APPLY_COMMISSION, 0, comm_flistp, &r_comm_flistp, ebufp);
			comm_status = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_RATING_STATUS, 0, ebufp);
			if (comm_status != NULL && *comm_status != SUCCESSFUL_RATING && *comm_status != NO_MATCHING_SELECTOR_DATA) {
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), comm_flistp, r_comm_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_DTR_COMMISSION_FAILURE, 0, 0, 0);
				goto cleanup;
			}
			if (*comm_status != NO_MATCHING_SELECTOR_DATA) {
				comm_amt = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_AMOUNT, 0, ebufp);
				PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_CHARGE, comm_amt, ebufp);
				PIN_FLIST_FLD_COPY(r_comm_flistp, PIN_FLD_EVENT_OBJ, comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
				fm_mso_commission_create_report(ctxp, DTR_COMMISSION, comm_flistp, &comm_obj_flistp, ebufp);
			}
			goto success;
		}
		/************************************************************************
		 Apply the commission in case of distributor commission - END
		 ***********************************************************************/
		commission_rule = CHARGE_HEAD_BASED_COMMISSION;
	}
	else 
	{
		commission_rule = CHARGE_HEAD_BASED_COMMISSION;

		//Get service type to check whether current service is applicable for commission against commission service
		serv_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		serv_type = (char *) PIN_POID_GET_TYPE(serv_pdp);
		acct_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		vp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ITEM_OBJ, 1, ebufp);
		if (vp)
		{
			item_type = (char *) PIN_POID_GET_TYPE((poid_t*)vp);
		}

		//Get account type to ignore commission transaction if account is of type LCO
		acct_type = fm_mso_utils_get_account_type(ctxp, acct_pdp, ebufp);
		if (acct_type == ACCT_TYPE_LCO ) 
		{
			fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ,
				"COMMISSION - LCO Payment, hence skipping the commission", ebufp);
			goto cleanup;
		}
		if ( acct_type == ACCT_TYPE_SUB_DTR || acct_type == ACCT_TYPE_DTR) 
		{
			fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ,
				"COMMISSION - DT/SDT item transfer, hence skipping the commission", ebufp);
			goto cleanup;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commission input flist 2 ");
		
		/********************************************************************************
		Retrieve LCO Account POID and Commission Model & Service for the current account
		*********************************************************************************/
		ret_val = fm_mso_commission_get_vald_lco_acct_comm_mdl (ctxp, i_flistp, &lco_flistp, ebufp) ;
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			//Cleanup the error and copy the error flist which got created from fm_mso_commission_get_vald_lco_acct_comm_mdl
			//PIN_ERRBUF_CLEAR(ebufp);
			//err_flistp = PIN_FLIST_COPY(lco_flistp, ebufp);
			goto cleanup;
		}
		else if (ret_val !=1 ) //Success
		{
			//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "lco_flistp", lco_flistp);
			lco_acct_objp      = PIN_FLIST_FLD_GET(lco_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
			dtr_acct_objp      = PIN_FLIST_FLD_GET(lco_flistp, MSO_FLD_DT_OBJ, 0, ebufp);
			sdt_acct_objp     = PIN_FLIST_FLD_GET(lco_flistp, MSO_FLD_SDT_OBJ, 1, ebufp);
			commission_model   = PIN_FLIST_FLD_GET(lco_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
			commission_service = PIN_FLIST_FLD_GET(lco_flistp, MSO_FLD_COMMISION_SERVICE, 0, ebufp);
			PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_COMMISSION_RULE,         &commission_rule, ebufp);
		}

		/******************************************
		SET Flags- Start
		******************************************/
		if (commission_model && *commission_model >=1 && *commission_model <=6  )
		{
			lco_has_valid_comm_model = 1;
		}

		if (commission_service && *commission_service >=0 && *commission_service <=2)
		{
			lco_has_valid_comm_srvc = 1;
		}
		/******************************************
		SET Flags- End
		******************************************/

		/************************************************
		 LCO: Retrieve BALANCE_BUCKET & COMMISSION_BUCKET
		 -Start
		************************************************/
		if (lco_has_valid_comm_model ==1 && lco_has_valid_comm_srvc )
		{
			fm_mso_utils_get_bal_grp(ctxp,lco_acct_objp, BALANCE_BUCKET, &lco_bal_grp_objp, &lco_incollect_serv_objp, ebufp);
			if (lco_bal_grp_objp == NULL) 
			{
				fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp, "61001", "COMMISSION - Balance Bucket not found",ebufp);
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, err_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_BAL_BUCKET_NOT_FOUND, 0, 0, 0);
				goto cleanup;
			}
			
			fm_mso_utils_get_bal_grp(ctxp,lco_acct_objp, COMMISSION_BUCKET, &lco_comm_grp_objp, &lco_outcollect_serv_objp, ebufp);
			if (lco_comm_grp_objp == NULL) 
			{
				fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp, "61002", "COMMISSION - Commission Bucket not found",ebufp);
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, err_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_COMM_BUCKET_NOT_FOUND, 0, 0, 0);
				goto cleanup;
			}

			PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_LCO_OBJ, lco_acct_objp, ebufp);
			PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_DT_OBJ,  dtr_acct_objp, ebufp);
			PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_SDT_OBJ, sdt_acct_objp, ebufp);

			PIN_FLIST_FLD_PUT(lco_flistp, MSO_FLD_LCO_BAL_BUCKET_OBJ,      lco_bal_grp_objp, ebufp);
			PIN_FLIST_FLD_PUT(lco_flistp, MSO_FLD_LCO_COMM_BUCKET_OBJ,     lco_comm_grp_objp, ebufp);
			PIN_FLIST_FLD_PUT(lco_flistp, MSO_FLD_LCO_INCOLLECT_SERV_OBJ,  lco_incollect_serv_objp, ebufp);
			PIN_FLIST_FLD_PUT(lco_flistp, MSO_FLD_LCO_OUTCOLLECT_SERV_OBJ, lco_outcollect_serv_objp, ebufp);
			//PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_COMMISSION_RULE,         &commission_rule, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO Commission Balance Group Buckets Retrieved", lco_flistp);
			
		}
		/************************************************
		 LCO: Retrieve BALANCE_BUCKET & COMMISSION_BUCKET
		 -End
		************************************************/

		/***************************************
		Retrieve SDT Commission Model
		****************************************/
		if ( sdt_acct_objp && (int64)PIN_POID_GET_ID(sdt_acct_objp) > 0  )
		{
			PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_SDT_OBJ, sdt_acct_objp, ebufp);
			ret_val = fm_mso_get_bu_commission_info (ctxp, i_flistp, &sdt_flistp, ACCT_TYPE_SUB_DTR, ebufp) ;
			if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
				//Cleanup the error and copy the error flist which got created from fm_mso_commission_get_vald_lco_acct_comm_mdl
				//PIN_ERRBUF_CLEAR(ebufp);
				//err_flistp = PIN_FLIST_COPY(lco_flistp, ebufp);
				goto cleanup;
			}
			else if (ret_val !=1 ) //Success
			{
				//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "lco_flistp", lco_flistp);
				sdt_commission_model   = PIN_FLIST_FLD_GET(sdt_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
				sdt_commission_service = PIN_FLIST_FLD_GET(sdt_flistp, MSO_FLD_COMMISION_SERVICE, 0, ebufp);
				PIN_FLIST_FLD_SET(sdt_flistp, MSO_FLD_COMMISSION_RULE,         &commission_rule, ebufp);
			}
		}
		/******************************************
		SET Flags- Start
		******************************************/
		if (sdt_commission_model && *sdt_commission_model >=1 && *sdt_commission_model <=6 )
		{
			sdt_has_valid_comm_model = 1;
		}
		if (sdt_commission_service && *sdt_commission_service >=0 && *sdt_commission_service <=2)
		{
			sdt_has_valid_comm_srvc = 1;
		}
		/******************************************
		SET Flags- End
		******************************************/

		/************************************************
		 SDT: Retrieve COMMISSION_BUCKET
		 -Start
		************************************************/
		if (sdt_has_valid_comm_model == 1 && sdt_has_valid_comm_srvc ==1 )
		{
			fm_mso_utils_get_bal_grp(ctxp, sdt_acct_objp, COMMISSION_BUCKET, &sdt_comm_grp_objp, &sdt_outcollect_serv_objp, ebufp);
			if (sdt_comm_grp_objp == NULL) 
			{
				fm_mso_utils_prepare_error_flist(ctxp, sdt_flistp, &err_flistp, "61003", "COMMISSION - SDT Commission Bucket not found",ebufp);
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, err_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_DTR_COMM_BUCKET_NOT_FOUND, 0, 0, 0);
				goto cleanup;
			}
			PIN_FLIST_FLD_SET(sdt_flistp, MSO_FLD_LCO_OBJ, lco_acct_objp, ebufp);
			PIN_FLIST_FLD_SET(sdt_flistp, MSO_FLD_DT_OBJ,  dtr_acct_objp, ebufp);
			PIN_FLIST_FLD_SET(sdt_flistp, MSO_FLD_SDT_OBJ, sdt_acct_objp, ebufp);

			PIN_FLIST_FLD_PUT(sdt_flistp, MSO_FLD_SDT_COMM_BUCKET_OBJ,     sdt_comm_grp_objp, ebufp);
			PIN_FLIST_FLD_PUT(sdt_flistp, MSO_FLD_SDT_OUTCOLLECT_SERV_OBJ, sdt_outcollect_serv_objp, ebufp);
			//PIN_FLIST_FLD_PUT(sdt_flistp, MSO_FLD_SDT_BAL_BUCKET_OBJ,      sdt_bal_grp_objp, ebufp);
			PIN_FLIST_FLD_PUT(sdt_flistp, MSO_FLD_SDT_OUTCOLLECT_SERV_OBJ, sdt_outcollect_serv_objp, ebufp);

//			PIN_FLIST_FLD_SET(sdt_flistp, MSO_FLD_COMMISSION_RULE, &commission_rule, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SDT Commission Balance Group Bucket Retrieved", sdt_flistp);
		}
		/************************************************
		 SDT: Retrieve COMMISSION_BUCKET
		 -End
		************************************************/

		/***************************************
		Retrieve DT Commission Model
		****************************************/
		if ( dtr_acct_objp && (int64)PIN_POID_GET_ID(dtr_acct_objp) > 0  )
		{
			PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_DT_OBJ, dtr_acct_objp, ebufp);
			ret_val = fm_mso_get_bu_commission_info (ctxp, i_flistp, &dtr_flistp, ACCT_TYPE_DTR, ebufp) ;
			if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
				//Cleanup the error and copy the error flist which got created from fm_mso_commission_get_vald_lco_acct_comm_mdl
				//PIN_ERRBUF_CLEAR(ebufp);
				//err_flistp = PIN_FLIST_COPY(lco_flistp, ebufp);
				goto cleanup;
			}
			else if (ret_val !=1 ) //Success
			{
				//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "lco_flistp", lco_flistp);
				dt_commission_model = PIN_FLIST_FLD_GET(dtr_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
				dt_commission_service = PIN_FLIST_FLD_GET(dtr_flistp, MSO_FLD_COMMISION_SERVICE, 0, ebufp);
				PIN_FLIST_FLD_SET(dtr_flistp, MSO_FLD_COMMISSION_RULE,         &commission_rule, ebufp);
			}
		}
		/******************************************
		SET Flags- Start
		******************************************/
		if (dt_commission_model && *dt_commission_model >=1 && *dt_commission_model <=6 )
		{
			dt_has_valid_comm_model = 1;
		}
		if (dt_commission_service && *dt_commission_service >=0 && *dt_commission_service <=2)
		{
			dt_has_valid_comm_srvc = 1;
		}
		/******************************************
		SET Flags- End
		******************************************/
		/************************************************
		 SDT: Retrieve COMMISSION_BUCKET
		 -Start
		************************************************/
		if (dt_has_valid_comm_model == 1 && dt_has_valid_comm_srvc ==1 )
		{
			fm_mso_utils_get_bal_grp(ctxp, dtr_acct_objp, COMMISSION_BUCKET, &dtr_comm_grp_objp, &dtr_outcollect_serv_objp, ebufp);
			if (dtr_comm_grp_objp == NULL) 
			{
				fm_mso_utils_prepare_error_flist(ctxp, dtr_flistp, &err_flistp, "61003", "COMMISSION - DTR Commission Bucket not found",ebufp);
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, err_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_DTR_COMM_BUCKET_NOT_FOUND, 0, 0, 0);
				goto cleanup;
			}
			PIN_FLIST_FLD_SET(dtr_flistp, MSO_FLD_LCO_OBJ, lco_acct_objp, ebufp);
			PIN_FLIST_FLD_SET(dtr_flistp, MSO_FLD_DT_OBJ,  dtr_acct_objp, ebufp);
			PIN_FLIST_FLD_SET(dtr_flistp, MSO_FLD_SDT_OBJ, sdt_acct_objp, ebufp);

			PIN_FLIST_FLD_PUT(dtr_flistp, MSO_FLD_DTR_COMM_BUCKET_OBJ,     dtr_comm_grp_objp, ebufp);
			PIN_FLIST_FLD_PUT(dtr_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, dtr_outcollect_serv_objp, ebufp);
			//PIN_FLIST_FLD_SET(dtr_flistp, MSO_FLD_COMMISSION_RULE, &commission_rule, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DTR Commission Balance Group Bucket Retrieved", dtr_flistp);
		}
		/************************************************
		 SDT: Retrieve COMMISSION_BUCKET
		 -End
		************************************************/

		/**********************************
		set flag:
		comm_applicable_for
		**********************************/
		if (dt_has_valid_comm_model && sdt_has_valid_comm_model && lco_has_valid_comm_model )
		{
			comm_applicable_for = LCO_SDT_AND_DT;
		}
		else if (dt_has_valid_comm_model && sdt_has_valid_comm_model)
		{
			comm_applicable_for = SDT_AND_DT;
		}
		else if (dt_has_valid_comm_model && lco_has_valid_comm_model)
		{
			comm_applicable_for = LCO_AND_DT;
		}
		else if (sdt_has_valid_comm_model && lco_has_valid_comm_model)
		{
			comm_applicable_for = LCO_AND_SDT;
		}
		else if (dt_has_valid_comm_model )
		{
			comm_applicable_for = DT_ONLY;
		}
		else if (sdt_has_valid_comm_model)
		{
			comm_applicable_for = SDT_ONLY;
		}
		else if ( lco_has_valid_comm_model)
		{
			comm_applicable_for = LCO_ONLY;
		}
		

		/*********************************************************************
		Exit when commission is not applicable  for LCO/DT/SDT

		IMP NOTE:
		This exit criteria can be done based on commission_model on customer's 
		service bb info as that will improve performane by not hitting 
		/profile/wholesale for LCO/SDT/DT instead 
		 hitting /service/telco/broadband however the field always need to be 
		 in sync with commission model of LCO/SDT/DT and care to be taken 
		 for LCO/SDT and DT modification
		**********************************************************************/
		if (comm_applicable_for == -1 )
		{
			goto cleanup;
		}
	}

//
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commission input flist 3 ", i_flistp);
//	//Handle error in case of invalid commission model and commission service 
//	if (charge_head_based_evt_for_dt != 1) 
//	{
//		if (commission_model == NULL || commission_service == NULL) 
//		{
//			fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp,"61000", "COMMISSION - Commission Model or Commission Service is not valid",ebufp);
//			//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_INVALID_COMM_MDL_SERV, 0, 0, 0);
//			fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), lco_flistp, err_flistp, ebufp);
//			goto cleanup;
//		}
//
//		//Prepare the lco account information flistp
//		lco_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_LCO_OBJ, lco_acct_objp, ebufp);
//		PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_DT_OBJ,  dtr_acct_objp, ebufp);
//		PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_SDT_OBJ, sdt_acct_objp, ebufp);
//
//		PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_COMMISION_MODEL, commission_model, ebufp);
//		PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_COMMISION_SERVICE, commission_service, ebufp);
//		PIN_FLIST_FLD_SET(lco_flistp, MSO_FLD_COMMISSION_RULE, &commission_rule, ebufp);
//	}
	


	/*************************************************************************************
	 Checking whether LCO commission is applicable for the current event's service-START
	 *************************************************************************************/
	if (active_cust_count_based_flag != 1 && 
	    charge_head_based_evt_for_dt !=1 
	   ) 
	{
		commission_flag = 0;
		switch ( *commission_service ) 
		{
		    case 0 : 
			commission_flag = 1; 
			break;
		    case 1 : 
			if (serv_pdp != NULL && strstr(serv_type, "/service/catv")){ 
				commission_flag = 1; 
			} 
			break;
		    case 2 : 
			if (serv_pdp != NULL && strstr(serv_type, "/service/telco/broadband")) {
				commission_flag = 1; 
			} 
			break;
		}
		
		if (evt_pdp != NULL && strstr(evt_type,"/event/billing/item/transfer"))
			commission_flag = 1;

		if (commission_flag != 1) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"Commission service is not applicable for the current event's service");
			goto cleanup;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Commission Service Validated");
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Skipping Commission Service Validation");
	}
	/*************************************************************************************
	 Checking whether LCO commission is applicable for the current event's service-END
	 *************************************************************************************/

	/******************************************************
	Applying commission for all the models	   
	******************************************************/

	/**********************************************
	Commission for POSTPAID_BB_MODEL
	**********************************************/
	if ( (commission_model     && *commission_model     == POSTPAID_BB_MODEL ) ||
	     (sdt_commission_model && *sdt_commission_model == POSTPAID_BB_MODEL ) ||
	     (dt_commission_model  && *dt_commission_model   == POSTPAID_BB_MODEL )
	   ) 
	{

		if (item_type && strcmp(item_type, "/item/adjustment")==0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Adjustment item not elligible for commission according to the current commission model");
			fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ,
				"COMMISSION - Item is not eligible for commission according to the current commission model", ebufp);
			goto cleanup;
		}

		if (evt_pdp != NULL && strstr(evt_type,"/event/billing/item/transfer")) 
		{
			fm_mso_commission_extract_transfer_items(ctxp, i_flistp, &item_r_flistp, ebufp);
			if (item_r_flistp == NULL) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No BB charge head eligible for commission");
				goto cleanup;
			}
		}
		// This is only for handling cycle refund case if any commission applied and to revert the commission based on the prorated amount
		else if (PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_ITEMS, ebufp) > 0 ){
			while ( PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ITEMS, &ielem_id, 1, &icookie, ebufp) != NULL){
				item_r_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_ELEM_COPY(i_flistp, PIN_FLD_ITEMS, ielem_id, item_r_flistp, PIN_FLD_ITEMS, ielem_id, ebufp);
			}
			comm_rev_flag = 1;
		}
		else 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Event is not eligible for commission according to the current commission model");
			fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ,
				"COMMISSION - Event is not eligible for commission according to the current commission model", ebufp);
			goto cleanup;
		}
		//commission_flag=1;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PROCESS COMMISSION ITEMS FLIST", item_r_flistp);
		if (item_r_flistp != NULL) 
		{
			while ( (item_flistp = PIN_FLIST_ELEM_GET_NEXT(item_r_flistp, PIN_FLD_ITEMS, &elem_id, 1, &cookie, ebufp))!=NULL) 
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "item list", item_flistp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SESSION_OBJ, item_flistp, PIN_FLD_SESSION_OBJ, ebufp);
				PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_FLAGS, &comm_rev_flag, ebufp); // FLAGS set for handling commission reverse for CF refund.
				fm_mso_commission_prep_commission_info(ctxp, lco_flistp, item_flistp, &comm_flistp, ebufp);
				charge_head_flistp = PIN_FLIST_SUBSTR_GET(comm_flistp, MSO_FLD_CHARGE_HEAD_INFO, 0, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ITEM_OBJ, charge_head_flistp, PIN_FLD_ITEM_OBJ, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, comm_flistp, PIN_FLD_PROGRAM_NAME, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO COMMISSION_FLIST START", comm_flistp);

				/******************************************************
				Apply LCO commision charges - Start
				******************************************************/
				if (comm_applicable_for == LCO_ONLY ||
				    comm_applicable_for == LCO_AND_SDT ||
				    comm_applicable_for == LCO_AND_DT ||
				    comm_applicable_for == LCO_SDT_AND_DT
				   )
				{

					PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_lco, ebufp);
					PCM_OP(ctxp, MSO_OP_COMMISSION_APPLY_COMMISSION, 0, comm_flistp, &r_comm_flistp, ebufp);
					comm_status = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_RATING_STATUS, 0, ebufp);
					if (comm_status != NULL && *comm_status != SUCCESSFUL_RATING && *comm_status != NO_MATCHING_SELECTOR_DATA) 
					{
						fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), comm_flistp, r_comm_flistp, ebufp);
						//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_COMMISSION_FAILURE, 0, 0, 0);
						//goto cleanup;
					}
					else if (comm_status && *comm_status != NO_MATCHING_SELECTOR_DATA) 
					{
						comm_amt = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_AMOUNT, 0, ebufp);
						PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_CHARGE, comm_amt, ebufp);
						PIN_FLIST_FLD_COPY(r_comm_flistp, PIN_FLD_EVENT_OBJ, comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
						fm_mso_commission_create_report(ctxp, LCO_COMMISSION, comm_flistp, &comm_obj_flistp, ebufp);
					}
				}
				/***********************************************
				Apply LCO commision charges - End 
				**********************************************/
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO COMMISSION_FLIST END", comm_flistp);

				/****************************************
				Apply DTR commision charges - Start 
				****************************************/
				if( comm_applicable_for == DT_ONLY ||
				    comm_applicable_for == LCO_AND_DT ||
				    comm_applicable_for == SDT_AND_DT ||
				    comm_applicable_for == LCO_SDT_AND_DT)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "DISTRIBUTOR COMMISSION_FLIST START", comm_flistp);
					PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_DT_OBJ, dtr_acct_objp, ebufp);
					PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, dtr_outcollect_serv_objp, ebufp );

					fm_mso_commission_process_dt_sdt_commission(ctxp, comm_flistp, &r_dtr_comm_flistp, MSO_FLD_DT_OBJ,  ebufp);
					if (r_dtr_comm_flistp != NULL) 
					{
						dtr_comm_status = PIN_FLIST_FLD_GET(r_dtr_comm_flistp, PIN_FLD_STATUS, 0, ebufp);
						
						if (dtr_comm_status != NULL && *dtr_comm_status == 0) 
						{
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commision dtr succeeded", r_dtr_comm_flistp);
						}
						else 
						{
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commision dtr failed", r_dtr_comm_flistp);
							PIN_FLIST_DESTROY_EX(&r_dtr_comm_flistp, NULL);
							//goto cleanup;
						}
						PIN_FLIST_DESTROY_EX(&r_dtr_comm_flistp, NULL);
					}
					/*Apply DTR commision charges - End */
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DISTRIBUTOR COMMISSION APPLIED");
				}

				/**************************************************
				 Apply Sub-Distributor commision charges - Start 
				***************************************************/
				if( comm_applicable_for == SDT_ONLY ||
				    comm_applicable_for == LCO_AND_SDT ||
				    comm_applicable_for == SDT_AND_DT ||
				    comm_applicable_for == LCO_SDT_AND_DT)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SUB-DISTRIBUTOR COMMISSION_FLIST START", comm_flistp);
					PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_SDT_OBJ, sdt_acct_objp, ebufp);
					PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_SDT_OUTCOLLECT_SERV_OBJ, sdt_outcollect_serv_objp, ebufp );

					fm_mso_commission_process_dt_sdt_commission(ctxp, comm_flistp, &r_dtr_comm_flistp, MSO_FLD_SDT_OBJ, ebufp);
					if (r_dtr_comm_flistp != NULL) 
					{
						dtr_comm_status = PIN_FLIST_FLD_GET(r_dtr_comm_flistp, PIN_FLD_STATUS, 0, ebufp);
						
						if (dtr_comm_status != NULL && *dtr_comm_status == 0) 
						{
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commision dtr succeeded", r_dtr_comm_flistp);
						}
						else {
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commision dtr failed", r_dtr_comm_flistp);
							PIN_FLIST_DESTROY_EX(&r_dtr_comm_flistp, NULL);
							//goto cleanup;
						}
						PIN_FLIST_DESTROY_EX(&r_dtr_comm_flistp, NULL);
					}
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "SUB-DISTRIBUTOR COMMISSION APPLIED");
				} 
				/*************************************************** 
				End of Sub-Distributor Commission 
				***************************************************/
				// Drop the ITEMS array for refund as all done
				if (PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_ITEMS, ebufp) > 0 )
					PIN_FLIST_ELEM_DROP(i_flistp, PIN_FLD_ITEMS, elem_id, ebufp);
			}
		}
		PIN_FLIST_DESTROY_EX(&item_r_flistp, ebufp);
	}
	/**********************************************
	Commission for PREPAID_GROSS_MODEL
	**********************************************/
	else if (*commission_model == PREPAID_GROSS_MODEL) 
	{
		/*Do Balance Transfer for the gross amount from LCO Balance Bucket*/
		//-- Get the entire amount including charge + tax 
		//-- Call balance transfer to transfer the amount from LCO Balance to Customer Account
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Commission Model : Prepaid Gross Billing");
		fm_mso_commission_aggregate_charge_tax_amt(ctxp, i_flistp, &event_chrgs_flistp, ebufp);
		event_tot = PIN_FLIST_FLD_GET(event_chrgs_flistp, PIN_FLD_TOTALS, 0, ebufp);
		sprintf(msg, "Totals : %s", pbo_decimal_to_str(event_tot, ebufp));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

		if(event_tot && pbo_decimal_compare(event_tot, zerop, ebufp) == 0) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Event is not having balance impact. Nothing to commission");
			fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ,
			"COMMISSION - Event is not having balance impact. Nothing to commission", ebufp);
			return;
		}

		//Get the item type / charge head type
		fm_mso_utils_get_item_type(ctxp, i_flistp, &r_item_type_flistp, ebufp);
		if (r_item_type_flistp != NULL) {
			PIN_FLIST_FLD_COPY(r_item_type_flistp, PIN_FLD_ITEM_TYPE, event_chrgs_flistp, PIN_FLD_ITEM_TYPE, ebufp);
			PIN_FLIST_DESTROY_EX (&r_item_type_flistp, NULL);
		}
		
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SESSION_OBJ, event_chrgs_flistp, PIN_FLD_SESSION_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, event_chrgs_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		fm_mso_commission_prep_commission_info(ctxp, lco_flistp, event_chrgs_flistp, &comm_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "COMMISSION_FLIST", comm_flistp);

		//-- Check for credit limit breach
		fm_mso_get_bals_credit_limit_bal(ctxp, lco_bal_grp_objp, lco_bal_grp_objp, &r_lco_bal_grp_flistp, ebufp);
		if (r_lco_bal_grp_flistp != NULL) 
		{
			bal_flistp = PIN_FLIST_ELEM_GET(r_lco_bal_grp_flistp, PIN_FLD_BALANCES, 356, 0, ebufp);
			lco_cur_bal = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
			lco_credit_limit = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CREDIT_LIMIT, 0, ebufp);
			lco_commission_charge = PIN_FLIST_FLD_GET(event_chrgs_flistp, PIN_FLD_TOTALS, 0, ebufp);
			lco_bal_after_commission = pbo_decimal_add(lco_cur_bal, lco_commission_charge, ebufp);
			if (lco_cur_bal != NULL && lco_credit_limit != NULL && lco_bal_after_commission != NULL) 
			{
				sprintf(msg, "LCO Balance : %s LCO Credit limit : %s LCO Balance (AFC) : %s",
					pbo_decimal_to_str(lco_cur_bal, ebufp), 
					pbo_decimal_to_str(lco_credit_limit,ebufp), 
					pbo_decimal_to_str(lco_bal_after_commission, ebufp));
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}

			if(lco_cur_bal == NULL) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO - Balance is ZERO");
				fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp,"61006", "LCO COMMISSION - LCO Balance Bucket not having balance",ebufp);
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, err_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_INSUFFICIENT_BALANCE, 0,0,0);
				goto cleanup;
			}
			else if(lco_bal_after_commission && (pbo_decimal_compare(lco_bal_after_commission, zerop, ebufp) == 1)) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO - Insufficient Balance");
				fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp, "61007", "LCO COMMISSION - LCO Balance Bucket having insufficient balance",ebufp);
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, err_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_INSUFFICIENT_BALANCE, 0,0,0);
				goto cleanup;
			}
		}

		ret_val = 0; //1: success 0:Fail
		ret_val = fm_mso_commission_lco_bal_trans (ctxp, GROSS_AMT, FROM_LCO_BALANCE_BUCKET, comm_flistp, &lco_bal_trans_r_flistp, ebufp);
		if ( ret_val == 0) 
		{
			err_flistp = lco_bal_trans_r_flistp;
			goto cleanup;
		}

		/*Apply the commision charges - Start */
		PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_lco, ebufp);
		PCM_OP(ctxp, MSO_OP_COMMISSION_APPLY_COMMISSION, 0, comm_flistp, &r_comm_flistp, ebufp);
		comm_status = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_RATING_STATUS, 0, ebufp);
		
		if (comm_status != NULL && *comm_status != SUCCESSFUL_RATING && *comm_status != NO_MATCHING_SELECTOR_DATA) 
		{
			fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), comm_flistp, r_comm_flistp, ebufp);
			//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_COMMISSION_FAILURE, 0, 0, 0);
			goto cleanup;
		}
		if (*comm_status != NO_MATCHING_SELECTOR_DATA) 
		{
			comm_amt = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_AMOUNT, 0, ebufp);
			PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_CHARGE, comm_amt, ebufp);
			PIN_FLIST_FLD_COPY(r_comm_flistp, PIN_FLD_EVENT_OBJ, comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
			fm_mso_commission_create_report(ctxp, LCO_COMMISSION, comm_flistp, &comm_obj_flistp, ebufp);
		}
		/*Apply the commision charges - End */

	}
	/**********************************************
	Commission for PREPAID_NET_MODEL
	**********************************************/
	else if (*commission_model == PREPAID_NET_MODEL) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Commission Model : Prepaid Net Billing");
		fm_mso_commission_aggregate_charge_tax_amt(ctxp, i_flistp, &event_chrgs_flistp, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SESSION_OBJ, event_chrgs_flistp, PIN_FLD_SESSION_OBJ, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Charge and Tax Aggregated");

		event_tot = PIN_FLIST_FLD_GET(event_chrgs_flistp, PIN_FLD_TOTALS, 0, ebufp);
		sprintf(msg, "Totals : %s", pbo_decimal_to_str(event_tot, ebufp));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

		if(pbo_decimal_compare(event_tot, zerop, ebufp) == 0) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Event is not having balance impact. Nothing to commission");
			fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ,
			"COMMISSION - Event is not having balance impact. Nothing to commission", ebufp);
			return;
		}

		//Get the item type / charge head type
		fm_mso_utils_get_item_type(ctxp, i_flistp, &r_item_type_flistp, ebufp);
		if (r_item_type_flistp != NULL) 
		{
			PIN_FLIST_FLD_COPY(r_item_type_flistp, PIN_FLD_ITEM_TYPE, event_chrgs_flistp, PIN_FLD_ITEM_TYPE, ebufp);
			PIN_FLIST_DESTROY_EX (&r_item_type_flistp, NULL);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Item Type Identified");

		//Preparing the data required for commission  
		fm_mso_commission_prep_commission_info(ctxp, lco_flistp, event_chrgs_flistp, &comm_flistp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Commission flist prepared");

		/*************************************************
		  Calculate the LCO Commission Share
		 ************************************************/
		if ( fm_mso_commission_get_lco_share(ctxp, comm_flistp, &lco_share_flistp, ebufp) != 0) 
		{
			err_flistp = lco_share_flistp;
			goto cleanup;
		}
		else 
		{
			PIN_FLIST_FLD_COPY(lco_share_flistp, PIN_FLD_AMOUNT, comm_flistp, MSO_FLD_COMMISSION_CHARGE, ebufp);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - LCO Share Calculated");

		/***********************************************
		2. Calculate the Hathway share and lco share tax
		************************************************/
		fm_mso_commission_calc_tax_share(ctxp, comm_flistp, &share_info_flistp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Tax Share Calculated");

		PIN_FLIST_ELEM_COPY(share_info_flistp, PIN_FLD_AMOUNTS, HATHWAY_SHARE, comm_flistp, PIN_FLD_AMOUNTS, HATHWAY_SHARE, ebufp);
		PIN_FLIST_ELEM_COPY(share_info_flistp, PIN_FLD_AMOUNTS, LCO_SHARE, comm_flistp, PIN_FLD_AMOUNTS, LCO_SHARE, ebufp);
		PIN_FLIST_ELEM_COPY(share_info_flistp, PIN_FLD_AMOUNTS, LCO_SHARE_TAX, comm_flistp, PIN_FLD_AMOUNTS, LCO_SHARE_TAX, ebufp);

		/*************************************************
		 3. Transfer hathway share from LCO Balance Bucket
		 *************************************************/
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG1");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_lco_bal_trans before flist", comm_flistp);

		ret_val=0;
		ret_val=fm_mso_commission_lco_bal_trans(ctxp, HATHWAY_SHARE, FROM_LCO_BALANCE_BUCKET,comm_flistp, &lco_bal_trans_r_flistp, ebufp);
		if ( ret_val == 0) 
		{
			err_flistp = lco_bal_trans_r_flistp;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG2");
			goto cleanup;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG3");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Balance Transfered for Hathway Share");
		PIN_FLIST_DESTROY_EX(&lco_bal_trans_r_flistp, NULL);

		/***********************************************
		4. Transfer LCO share from LCO Commission Bucket
		***********************************************/
		ret_val = 0;
		ret_val = fm_mso_commission_lco_bal_trans(ctxp, LCO_SHARE, FROM_LCO_COMMISSION_BUCKET, comm_flistp, &lco_bal_trans_r_flistp, ebufp);
		if ( ret_val == 0) 
		{
			err_flistp = lco_bal_trans_r_flistp;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG4");
			goto cleanup;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Balance Transfered for LCO Share");
		PIN_FLIST_DESTROY_EX(&lco_bal_trans_r_flistp, NULL);

		tmp_flistp = PIN_FLIST_ELEM_GET(share_info_flistp, PIN_FLD_AMOUNTS, LCO_SHARE_TAX, 1, ebufp);
		if(tmp_flistp)
		{
			adj_amt = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_AMOUNT, 1, ebufp);
		}
		
		if(!pbo_decimal_is_null(adj_amt,ebufp) && !pbo_decimal_is_zero(adj_amt, ebufp))
		{
			/***************************************************
			5. Transfer LCO Share tax from LCO Balance Bucket
			****************************************************/
			ret_val=0;
			ret_val=fm_mso_commission_lco_bal_trans(ctxp, LCO_SHARE_TAX, FROM_LCO_BALANCE_BUCKET,comm_flistp, &lco_bal_trans_r_flistp, ebufp);
			if ( ret_val == 0) 
			{
				err_flistp = lco_bal_trans_r_flistp;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG5");
				goto cleanup;
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Balance Transfered for Tax Share");

			/**************************************************************
			6. Post an adjustment for the remaining commission tax amount 
			into the LCO Balance Bucket
			**************************************************************/
		
			adj_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(comm_flistp, PIN_FLD_POID, adj_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(adj_flistp,PIN_FLD_PROGRAM_NAME,"LCO Commission - Adjustment", ebufp);
			PIN_FLIST_FLD_SET(adj_flistp, PIN_FLD_AMOUNT, pbo_decimal_negate(adj_amt,ebufp), ebufp);
			PIN_FLIST_FLD_COPY(comm_flistp, MSO_FLD_LCO_BAL_BUCKET_OBJ, adj_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "LCO Adjustment input flist",adj_flistp);
			PIN_FLIST_FLD_SET(adj_flistp,PIN_FLD_FLAGS,&adj_flag, ebufp);
			PCM_OP(ctxp, PCM_OP_AR_ACCOUNT_ADJUSTMENT, 0, adj_flistp, &adj_o_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error During Adjustment ",ebufp);
				goto cleanup ;
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Adjustment passed to adjust Tax Share");
	
			PIN_FLIST_DESTROY_EX(&adj_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&adj_o_flistp, NULL);
		}	
		PIN_FLIST_DESTROY_EX(&lco_bal_trans_r_flistp, NULL);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "comm_flistp:", comm_flistp);

		/*Apply the commision charges - Start */
		PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_lco, ebufp);
		PCM_OP(ctxp, MSO_OP_COMMISSION_APPLY_COMMISSION, 0, comm_flistp, &r_comm_flistp, ebufp);
		comm_status = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_RATING_STATUS, 0, ebufp);
		if(comm_status != NULL && *comm_status == SUCCESSFUL_RATING)
		{
			comm_amt = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_AMOUNT, 0, ebufp);
			/* Transfer the balance from Commission bucket to LCO Bucket */
			ret_val =0;
			ret_val = fm_mso_commission_lco_bal_trans(ctxp, LCO_TO_HW_SHARE, FROM_LCO_COMMISSION_BUCKET, comm_flistp, &lco_bal_trans_r_flistp, ebufp);
			if ( ret_val == 0) 
			{
				err_flistp = lco_bal_trans_r_flistp;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"DEBUG6");
				goto cleanup;
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,  "LCO Commission - Balance Transfered from LCO Share to HW");
			PIN_FLIST_DESTROY_EX(&lco_bal_trans_r_flistp, NULL);
		}
		else if (comm_status && *comm_status != SUCCESSFUL_RATING && *comm_status != NO_MATCHING_SELECTOR_DATA) 
		{
			fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), comm_flistp, r_comm_flistp, ebufp);
			fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ,
				"LCO COMMISSION - Charge head not applicable for commission", ebufp);
			//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_COMMISSION_FAILURE, 0, 0, 0);
			goto cleanup;
		}

		if (*comm_status != NO_MATCHING_SELECTOR_DATA) 
		{
			comm_amt = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_AMOUNT, 0, ebufp);
			PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_CHARGE, comm_amt, ebufp);
			PIN_FLIST_FLD_COPY(r_comm_flistp, PIN_FLD_EVENT_OBJ, comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
			fm_mso_commission_create_report(ctxp, LCO_COMMISSION, comm_flistp, &comm_obj_flistp, ebufp);
		}
		/*Apply the commision charges - End */
	}
	/**********************************************
	Commission for POSTPAID_GROSS_MODEL
	**********************************************/
	else if (*commission_model == POSTPAID_GROSS_MODEL) 
	{
		/*Do Balance Transfer for the gross amount from LCO Balance Bucket*/
		//-- Get the entire amount including charge + tax
		//-- Call balance transfer to transfer the amount from LCO Balance to Customer Account
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Commission Model : Postpaid Gross Billing");

		fm_mso_commission_aggregate_charge_tax_amt(ctxp, i_flistp, &event_chrgs_flistp, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SESSION_OBJ, event_chrgs_flistp, PIN_FLD_SESSION_OBJ, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Charge and Tax Aggregated");

		event_tot = PIN_FLIST_FLD_GET(event_chrgs_flistp, PIN_FLD_TOTALS, 0, ebufp);
		sprintf(msg, "Totals : %s", pbo_decimal_to_str(event_tot, ebufp));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

		if(pbo_decimal_compare(event_tot, zerop, ebufp) == 0) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Event is not having balance impact. Nothing to commission");
			fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ,
			"COMMISSION - Event is not having balance impact. Nothing to commission", ebufp);
			return;
		}

		//Get the item type / charge head type
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"wrong tagging check");
		fm_mso_utils_get_item_type(ctxp, i_flistp, &r_item_type_flistp, ebufp);
		if (r_item_type_flistp != NULL) 
		{
			PIN_FLIST_FLD_COPY(r_item_type_flistp,PIN_FLD_ITEM_TYPE, event_chrgs_flistp, PIN_FLD_ITEM_TYPE, ebufp);
			PIN_FLIST_DESTROY_EX (&r_item_type_flistp, NULL);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Item Type Identified");

		fm_mso_commission_prep_commission_info(ctxp, lco_flistp, event_chrgs_flistp, &comm_flistp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Commission flist prepared");

		//-- Get the commission charge
		lco_commission_charge = PIN_FLIST_FLD_GET(event_chrgs_flistp, PIN_FLD_TOTALS, 0, ebufp);

		//-- Check for credit limit breach
		fm_mso_get_bals_credit_limit_bal(ctxp, lco_bal_grp_objp, lco_bal_grp_objp, &r_lco_bal_grp_flistp, ebufp);
		if (r_lco_bal_grp_flistp != NULL) 
		{
			bal_flistp = PIN_FLIST_ELEM_GET(r_lco_bal_grp_flistp, PIN_FLD_BALANCES, 356, 1, ebufp);
			if (bal_flistp != NULL) 
			{
				lco_cur_bal = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CURRENT_BAL, 1, ebufp);
				lco_credit_limit = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CREDIT_LIMIT, 1, ebufp);
				lco_bal_after_commission = pbo_decimal_add(lco_cur_bal, lco_commission_charge, ebufp);

				if (pbo_decimal_is_null(lco_cur_bal,ebufp) == 0 && pbo_decimal_is_null(lco_credit_limit,ebufp) == 0) 
				{
					sprintf(msg, "LCO Balance : %s LCO Credit limit : %s LCO Balance (AFC) : %s", 
						pbo_decimal_to_str(lco_cur_bal, ebufp), 
						pbo_decimal_to_str(lco_credit_limit,ebufp), 
						pbo_decimal_to_str(lco_bal_after_commission, ebufp));
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			
					if(pbo_decimal_compare(lco_credit_limit, lco_bal_after_commission, ebufp) == -1) 
					{
						fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp, "61008", "LCO COMMISSION - Credit Limit Exceed for LCO", ebufp);
						fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp),i_flistp, err_flistp, ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Credit Limit Exceed");
						//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,MSO_ERR_LCO_CREDIT_LIMIT_EXCEEDED,0,0,0);
						goto cleanup;
					}
				}
			}
			else 
			{
				fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp, "61009", "LCO COMMISSION - No Credit Limit for LCO", ebufp);
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, err_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_CREDIT_LIMIT_NOT_SET, 0,0,0);
				goto cleanup;
			}
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Credit Limit Verified");

		//Impact the balance from LCO Incollect Balance group to Customer's Balance Group
		if ( fm_mso_commission_lco_bal_impact
			(ctxp, GROSS_AMT, FROM_LCO_BALANCE_BUCKET, comm_flistp, &lco_bal_trans_r_flistp, ebufp) == 0) 
		{
			err_flistp = lco_bal_trans_r_flistp;
			goto cleanup;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Balance Impacted Successfully");

		/*Apply the commision charges - Start */
		PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_lco, ebufp);
		PCM_OP(ctxp, MSO_OP_COMMISSION_APPLY_COMMISSION, 0, comm_flistp, &r_comm_flistp, ebufp);
		comm_status = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_RATING_STATUS, 0, ebufp);
		if (comm_status != NULL && *comm_status != SUCCESSFUL_RATING && *comm_status != NO_MATCHING_SELECTOR_DATA) 
		{
			fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, r_comm_flistp, ebufp);
			//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_COMMISSION_FAILURE, 0, 0, 0);
			goto cleanup;
		}
		if (*comm_status != NO_MATCHING_SELECTOR_DATA) 
		{
			comm_amt = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_AMOUNT, 0, ebufp);
			PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_CHARGE, comm_amt, ebufp);
			PIN_FLIST_FLD_COPY(r_comm_flistp, PIN_FLD_EVENT_OBJ, comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
			fm_mso_commission_create_report(ctxp, LCO_COMMISSION, comm_flistp, &comm_obj_flistp, ebufp);
		}
		/*Apply the commision charges - End */
	}
	/**********************************************
	Commission for POSTPAID_NET_MODEL
	**********************************************/
	else if (*commission_model == POSTPAID_NET_MODEL) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Commission Model : Postpaid Net Billing");

		fm_mso_commission_aggregate_charge_tax_amt(ctxp, i_flistp, &event_chrgs_flistp, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SESSION_OBJ, event_chrgs_flistp, PIN_FLD_SESSION_OBJ, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Charge and Tax Aggregated");

		event_tot = PIN_FLIST_FLD_GET(event_chrgs_flistp, PIN_FLD_TOTALS, 0, ebufp);
		sprintf(msg, "Totals : %s", pbo_decimal_to_str(event_tot, ebufp));
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

		if(pbo_decimal_compare(event_tot, zerop, ebufp) == 0) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Event is not having balance impact. Nothing to commission");
			fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ,
			"COMMISSION - Event is not having balance impact. Nothing to commission", ebufp);
			return;
		}


		//Get the item type / charge head type
		fm_mso_utils_get_item_type(ctxp, i_flistp, &r_item_type_flistp, ebufp);
		if (r_item_type_flistp != NULL) 
		{
			PIN_FLIST_FLD_COPY(r_item_type_flistp, PIN_FLD_ITEM_TYPE, event_chrgs_flistp, PIN_FLD_ITEM_TYPE, ebufp);
			PIN_FLIST_DESTROY_EX (&r_item_type_flistp, NULL);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Item Type Identified");

		fm_mso_commission_prep_commission_info(ctxp, lco_flistp, event_chrgs_flistp, &comm_flistp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Commission flist prepared");

		//-- Get the commission charge
		lco_commission_charge = PIN_FLIST_FLD_GET(event_chrgs_flistp, PIN_FLD_TOTALS, 0, ebufp);

		//-- Check for credit limit breach
		fm_mso_get_bals_credit_limit_bal(ctxp, lco_bal_grp_objp, lco_bal_grp_objp, &r_lco_bal_grp_flistp, ebufp);
		if (r_lco_bal_grp_flistp != NULL) 
		{
			bal_flistp = PIN_FLIST_ELEM_GET(r_lco_bal_grp_flistp, PIN_FLD_BALANCES, 356, 1, ebufp);
			if (bal_flistp != NULL) 
			{
				lco_cur_bal = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CURRENT_BAL, 1, ebufp);
				lco_credit_limit = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CREDIT_LIMIT, 1, ebufp);
				lco_bal_after_commission = pbo_decimal_add(lco_cur_bal, lco_commission_charge, ebufp);

				if (pbo_decimal_is_null(lco_cur_bal,ebufp) != 0 && pbo_decimal_is_null(lco_credit_limit,ebufp) != 0) 
				{
					sprintf(msg, "LCO Balance : %s LCO Credit limit : %s LCO Balance (AFC) : %s",
						pbo_decimal_to_str(lco_cur_bal, ebufp),
						pbo_decimal_to_str(lco_credit_limit,ebufp),
						pbo_decimal_to_str(lco_bal_after_commission, ebufp));
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

					if(pbo_decimal_compare(lco_credit_limit, lco_bal_after_commission, ebufp) == -1) 
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Credit Limit Exceed");
						pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_CREDIT_LIMIT_EXCEEDED,0,0,0);
						goto cleanup;
					}
				}
			}
			else 
			{
				fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp, "61010", "LCO COMMISSION - No Credit Limit for LCO", ebufp);
				fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, err_flistp, ebufp);
				//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_CREDIT_LIMIT_NOT_SET, 0,0,0);
				goto cleanup;
			}
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Credit Limit Verified");

		if ( fm_mso_commission_get_lco_share(ctxp, comm_flistp, &lco_share_flistp, ebufp) != 0) 
		{
			err_flistp = lco_share_flistp;
			goto cleanup;
		}
		else {
			PIN_FLIST_FLD_COPY(lco_share_flistp, PIN_FLD_AMOUNT, comm_flistp, MSO_FLD_COMMISSION_CHARGE, ebufp);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - LCO Share Calculated");

		fm_mso_commission_calc_tax_share(ctxp, comm_flistp, &share_info_flistp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Tax Share Calculated");

		PIN_FLIST_ELEM_COPY(share_info_flistp, PIN_FLD_AMOUNTS, HATHWAY_SHARE, comm_flistp, PIN_FLD_AMOUNTS, HATHWAY_SHARE, ebufp);
		PIN_FLIST_ELEM_COPY(share_info_flistp, PIN_FLD_AMOUNTS, LCO_SHARE, comm_flistp, PIN_FLD_AMOUNTS, LCO_SHARE, ebufp);
		PIN_FLIST_ELEM_COPY(share_info_flistp, PIN_FLD_AMOUNTS, LCO_SHARE_TAX, comm_flistp, PIN_FLD_AMOUNTS, LCO_SHARE_TAX, ebufp);

		/*******************************************************************************
		Impact the balance from LCO Incollect Balance group to Customer's Balance Group
		*******************************************************************************/
		if ( fm_mso_commission_lco_bal_impact (ctxp, HATHWAY_SHARE, FROM_LCO_BALANCE_BUCKET, 
		           comm_flistp, &lco_bal_trans_r_flistp, ebufp) == 0
		    ) 
		{
			err_flistp = lco_bal_trans_r_flistp;
			goto cleanup;
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "LCO Commission - Hathway Share Impacted Successfully");

		/*Apply the commision charges - Start */
		PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_lco, ebufp);
		PIN_FLIST_FLD_SET(comm_flistp, PIN_FLD_PROGRAM_NAME, "OAP|Program", ebufp);
		PCM_OP(ctxp, MSO_OP_COMMISSION_APPLY_COMMISSION, 0, comm_flistp, &r_comm_flistp, ebufp);
		comm_status = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_RATING_STATUS, 0, ebufp);
		if (comm_status != NULL && *comm_status != SUCCESSFUL_RATING && *comm_status != NO_MATCHING_SELECTOR_DATA) 
		{
			fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, r_comm_flistp, ebufp);
			//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, MSO_ERR_LCO_COMMISSION_FAILURE, 0, 0, 0);
			goto cleanup;
		}
		if (*comm_status != NO_MATCHING_SELECTOR_DATA) 
		{
			comm_amt = PIN_FLIST_FLD_GET(r_comm_flistp, PIN_FLD_AMOUNT, 0, ebufp);
			PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_COMMISSION_CHARGE, comm_amt, ebufp);
			PIN_FLIST_FLD_COPY(r_comm_flistp, PIN_FLD_EVENT_OBJ, comm_flistp, PIN_FLD_EVENT_OBJ, ebufp);
			fm_mso_commission_create_report(ctxp, LCO_COMMISSION, comm_flistp, &comm_obj_flistp, ebufp);
		}
		/*Apply the commision charges - End */
	}
	else 
	{
		fm_mso_utils_prepare_error_flist(ctxp, lco_flistp, &err_flistp, "61011", "LCO COMMISSION - Invalid Commission Model",ebufp);
		fm_mso_commission_record_failure (ctxp, (char*)PIN_POID_GET_TYPE(evt_pdp), i_flistp, err_flistp, ebufp);
	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commission input flist 12 ", i_flistp);
	/*Collect the required information from the event-END*/


success:
	//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SESSION_OBJ, comm_flistp, PIN_FLD_SESSION_OBJ, ebufp);
	/*fm_mso_commission_create_report(ctxp, comm_flistp, &comm_obj_flistp, ebufp);*/
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commission completed successfully");

	fm_mso_utils_prepare_status_flist(ctxp, i_flistp, &err_flistp, PIN_FLD_ACCOUNT_OBJ,
		"COMMISSION - Process Completed Succesfully", ebufp);

//	/*Apply DTR commision charges - Start */
//	if (charge_head_based_evt_for_lco != 1) 
//	{
//		if (charge_head_based_evt_for_dt == 0) 
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commision DTR object is NULL");
//		}
//		else if (*commission_model == PREPAID_GROSS_MODEL || *commission_model == PREPAID_NET_MODEL ||
//			 *commission_model == POSTPAID_GROSS_MODEL || *commission_model == POSTPAID_NET_MODEL 
//		        ) 
//		{
//			PIN_FLIST_FLD_SET(comm_flistp, MSO_FLD_DT_OBJ, dtr_acct_objp, ebufp);
//			fm_mso_commission_process_dt_sdt_commission(ctxp, comm_flistp, &r_dtr_comm_flistp, MSO_FLD_DT_OBJ, ebufp);
//			if (r_dtr_comm_flistp != NULL) 
//			{
//				dtr_comm_status = PIN_FLIST_FLD_GET(r_dtr_comm_flistp, PIN_FLD_STATUS, 0, ebufp);
//				PIN_FLIST_DESTROY_EX(&r_dtr_comm_flistp, NULL);
//				if (dtr_comm_status != NULL && *dtr_comm_status == 0) 
//				{
//					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commision dtr succeeded", r_dtr_comm_flistp);
//				}
//				else 
//				{
//					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_commision dtr failed", r_dtr_comm_flistp);
//					return;
//				}
//			}
//		}
//	}
//	/*Apply DTR commision charges - End */

cleanup:
	if (err_flistp != NULL) 
	{
		*r_flistpp = err_flistp;
	}
	if (*r_flistpp == NULL && (commission_model != NULL && *commission_model != POSTPAID_BB_MODEL) ) 
	{
		*r_flistpp = lco_bal_trans_r_flistp;
	}

	PIN_FLIST_DESTROY_EX(&lco_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&comm_obj_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&event_chrgs_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&comm_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&dtr_comm_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_comm_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&share_info_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&service_info, NULL);
	pbo_decimal_destroy(&zerop);
}



static void 
fm_mso_commission_aggregate_charge_tax_amt (
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp) 
{

	pin_flist_t		*balImpact_flistp = NULL;
	char			*rate_tag = NULL;
	int32           	*resource = NULL;
	pin_decimal_t           *event_base_amt = NULL;
	pin_decimal_t           *event_tax_amt = NULL;
	pin_decimal_t           *event_tot_amt = NULL;
	int32           	rec_id = 0;
	pin_cookie_t            cookie = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_commission_aggregate_charge_tax_amt", ebufp);
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	event_base_amt = pbo_decimal_from_str("0.0",ebufp);
	event_tax_amt  = pbo_decimal_from_str("0.0",ebufp);
	event_tot_amt  = pbo_decimal_from_str("0.0",ebufp);

	/* Collect the charge and tax amount separately - START */
	while ((balImpact_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BAL_IMPACTS, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) 
	{
		rate_tag = PIN_FLIST_FLD_GET(balImpact_flistp, PIN_FLD_RATE_TAG, 0, ebufp);
		resource = PIN_FLIST_FLD_GET(balImpact_flistp, PIN_FLD_RESOURCE_ID, 0, ebufp);
		if ((resource && PIN_BEID_IS_CURRENCY(*resource)) && 
		     rate_tag && strcmp(rate_tag, "Tax")!=0 && 
		     strcmp(rate_tag, "DUMMY_MSO_TAX")!=0
		   ) 
		{
			pbo_decimal_add_assign(event_base_amt,
				PIN_FLIST_FLD_GET(balImpact_flistp, PIN_FLD_AMOUNT, 0, ebufp), ebufp);
			pbo_decimal_add_assign(event_tot_amt,
				PIN_FLIST_FLD_GET(balImpact_flistp, PIN_FLD_AMOUNT, 0, ebufp), ebufp);
		}
		else if((resource && PIN_BEID_IS_CURRENCY(*resource)) && 
			 rate_tag && strcmp(rate_tag, "Tax")==0 && 
			 strcmp(rate_tag, "DUMMY_MSO_TAX")!=0) 
		{
			pbo_decimal_add_assign(event_tax_amt,
				PIN_FLIST_FLD_GET(balImpact_flistp, PIN_FLD_AMOUNT, 0, ebufp), ebufp);
			pbo_decimal_add_assign(event_tot_amt,
				PIN_FLIST_FLD_GET(balImpact_flistp, PIN_FLD_AMOUNT, 0, ebufp), ebufp);
		}
	}
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(*r_flistpp,PIN_FLD_AMOUNT, event_base_amt, ebufp);
	PIN_FLIST_FLD_PUT(*r_flistpp,PIN_FLD_TAX, event_tax_amt, ebufp);
	PIN_FLIST_FLD_PUT(*r_flistpp,PIN_FLD_TOTALS, event_tot_amt, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, *r_flistpp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BAL_GRP_OBJ, *r_flistpp, PIN_FLD_BAL_GRP_OBJ, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_aggregate_charge_tax_amt return flist", *r_flistpp);
	/* Collect the charge and tax amount separately - END */

}

static int 
fm_mso_commission_prep_commission_info (
	pcm_context_t           *ctxp,
	pin_flist_t             *lco_flistp,
	pin_flist_t             *data_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp)
{
	pin_decimal_t		*transfer_amount = NULL;
	pin_decimal_t		*t_credit_duep = NULL;
	pin_decimal_t		*item_totalp = NULL;
	pin_decimal_t		*total_chg_amt  = NULL;
	pin_decimal_t		*qty_apply  = NULL;
	int32			*comm_model = NULL;
	int32			*comm_rule = NULL;
	int32			comm_status = 0;
	int32			*comm_rev_flag = NULL;
	time_t			comm_effective_time = 0;
	//struct tm             *tmp_time_t;
	poid_t			*item_pdp = NULL;

	pin_flist_t		*comm_flistp = NULL;
	pin_flist_t		*charge_head_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_commission_prep_commission_info", ebufp);
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_prep_commission_info lco flistp",  lco_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_prep_commission_info data flistp", data_flistp);

	/*Get the commission rule*/
	comm_model = PIN_FLIST_FLD_GET(lco_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
	comm_rule = PIN_FLIST_FLD_GET(lco_flistp, MSO_FLD_COMMISSION_RULE, 0, ebufp);
	comm_rev_flag = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_FLAGS, 1, ebufp);
	item_pdp = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_POID, 1, ebufp);

	/*Prepare the commission information flist*/
	comm_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_LCO_OBJ, comm_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_LCO_OBJ, comm_flistp, MSO_FLD_LCO_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_SDT_OBJ, comm_flistp, MSO_FLD_SDT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_DT_OBJ,  comm_flistp, MSO_FLD_DT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_LCO_BAL_BUCKET_OBJ, comm_flistp, MSO_FLD_LCO_BAL_BUCKET_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_LCO_COMM_BUCKET_OBJ, comm_flistp, MSO_FLD_LCO_COMM_BUCKET_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_LCO_INCOLLECT_SERV_OBJ, comm_flistp, MSO_FLD_LCO_INCOLLECT_SERV_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_LCO_OUTCOLLECT_SERV_OBJ, comm_flistp, MSO_FLD_LCO_OUTCOLLECT_SERV_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_COMMISION_MODEL, comm_flistp, MSO_FLD_COMMISION_MODEL, ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_COMMISION_SERVICE, comm_flistp, MSO_FLD_COMMISION_SERVICE, ebufp);
	PIN_FLIST_FLD_COPY(lco_flistp, MSO_FLD_COMMISSION_RULE, comm_flistp, MSO_FLD_COMMISSION_RULE, ebufp);

/*	if (*comm_model == POSTPAID_BB_MODEL) {
		comm_effective_time = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);
		tmp_time_t = localtime(comm_effective_time);
		tmp_time_t->tm_mday -= 1; 
		*comm_effective_time = mktime(tmp_time_t);
		//comm_effective_time = pin_virtual_time(NULL);
		PIN_FLIST_FLD_SET(comm_flistp, PIN_FLD_EFFECTIVE_T, comm_effective_time, ebufp)
	}*/
	comm_effective_time = pin_virtual_time(NULL);
	PIN_FLIST_FLD_SET(comm_flistp, PIN_FLD_EFFECTIVE_T, &comm_effective_time, ebufp)
	PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_ACCOUNT_OBJ, comm_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_SESSION_OBJ, comm_flistp, PIN_FLD_SESSION_OBJ, ebufp);
	PIN_FLIST_FLD_SET(comm_flistp, PIN_FLD_PROGRAM_NAME, "OAP|LCO Commission", ebufp)

	//Adding the commission
	if (comm_rule && *comm_rule == CHARGE_HEAD_BASED_COMMISSION ) 
	{
		PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_POID, comm_flistp, PIN_FLD_ITEM_OBJ, ebufp);
		charge_head_flistp = PIN_FLIST_SUBSTR_ADD (comm_flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp);
		PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_ITEM_TYPE, charge_head_flistp, MSO_FLD_CHARGE_HEAD, ebufp);
		transfer_amount = PIN_FLIST_FLD_TAKE(data_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		item_totalp = PIN_FLIST_FLD_GET(data_flistp, PIN_FLD_ITEM_TOTAL, 1, ebufp);
		// Added in case of technology movement and credit CF items before payment
		if ((comm_rev_flag && *comm_rev_flag != 1) && (strstr(PIN_POID_GET_TYPE(item_pdp), "/item/cycle_forward"))){
			t_credit_duep = pbo_decimal_from_str("0", ebufp);
			fm_mso_get_open_credit_item_due(ctxp, data_flistp, t_credit_duep, ebufp);
			/************************************************************************************************
			* Below is to calculate the commission to be applied incase if there is any refund is already 
			* given and it is like (ACTUAL_CHG - REFUND_CHG)/(ACTUAL_CHG)*PAYMENT_AMT
			************************************************************************************************/
			if ( !pbo_decimal_is_null(t_credit_duep, ebufp) && !pbo_decimal_is_zero(t_credit_duep, ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entered Partial Payment with Refund exists section" );
				total_chg_amt = pbo_decimal_from_str("0", ebufp);
				total_chg_amt = pbo_decimal_add( item_totalp, t_credit_duep, ebufp);
				qty_apply = pbo_decimal_divide(total_chg_amt, item_totalp, ebufp);
				pbo_decimal_multiply_assign(transfer_amount, qty_apply, ebufp);
				pbo_decimal_round_assign(transfer_amount, 2, ROUND_HALF_UP, ebufp);
				//pbo_decimal_negate_assign(transfer_amount, ebufp);
				if (!pbo_decimal_is_null(total_chg_amt, ebufp))
					pbo_decimal_destroy(&total_chg_amt);
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "new transfer amount" );
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(transfer_amount, ebufp));
			
		}
		PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_ITEM_TOTAL, charge_head_flistp, MSO_FLD_CHARGE_HEAD_TOTAL, ebufp);
		PIN_FLIST_CONCAT(charge_head_flistp, data_flistp, ebufp);
		PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_BAL_GRP_OBJ, comm_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);

		if (*comm_model != POSTPAID_BB_MODEL ) {
		    PIN_FLIST_FLD_PUT(charge_head_flistp, MSO_FLD_COLLECTION_AMT, transfer_amount, ebufp);
		    PIN_FLIST_FLD_COPY(data_flistp, PIN_FLD_SERVICE_OBJ, comm_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		}
		else {
		    PIN_FLIST_FLD_PUT(charge_head_flistp, MSO_FLD_COLLECTION_AMT, pbo_decimal_negate(transfer_amount,ebufp), ebufp);
		}
		comm_status = 1; //Completed - Commission will be applied immediately
		PIN_FLIST_FLD_SET(comm_flistp,PIN_FLD_STATUS, &comm_status, ebufp);
		PIN_FLIST_FLD_SET(comm_flistp,PIN_FLD_STATUS_MSG, "Completed", ebufp);
	}

	*r_flistpp = comm_flistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_prep_commission_info output flistp", *r_flistpp);
}


static void
fm_mso_commission_extract_transfer_items (
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp) {

	pin_flist_t             *action_info_flistp = NULL;
	pin_flist_t             *item_transfer_flistp = NULL;
	pin_flist_t             *item_flistp = NULL;
	pin_flist_t             *item_r_flistp = NULL;
	pin_flist_t             *tmp_flistp = NULL;
	pin_buf_t               *pin_bufp = NULL;
	int32			elemid=0;
	int32			count=0;
	pin_cookie_t            cookie = NULL;
	poid_t			*item_pdp = NULL;
	poid_t			*serv_pdp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_extract_transfer_items input flist", i_flistp);

	/* Read the buffer and convert into flist - Start*/
	action_info_flistp = PIN_FLIST_ELEM_GET( i_flistp, PIN_FLD_ACTION_INFO, 0, 1, ebufp );
	if (action_info_flistp != NULL) {
		pin_bufp = (pin_buf_t *)PIN_FLIST_FLD_GET(action_info_flistp, PIN_FLD_BUFFER, 0, ebufp );
		if ( pin_bufp != NULL && pin_bufp->data != NULL ) 
		{
			pin_str_to_flist( pin_bufp->data, 0, &item_transfer_flistp, ebufp );
	
			if( PIN_ERRBUF_IS_ERR( ebufp )) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Error putting buffer onto flist", ebufp);
				goto cleanup;
			}
		}
	}
	/* Read the buffer and convert into flist - End*/

	/* Loop through the items and filter only the items belongs to /service/telco/broadband - Start*/
	*r_flistpp = PIN_FLIST_CREATE (ebufp);
	while ( (item_flistp = PIN_FLIST_ELEM_GET_NEXT(item_transfer_flistp, PIN_FLD_ITEMS, &elemid, 1, &cookie, ebufp)) != NULL ) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "item list", item_flistp);
		item_pdp = PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);	
		fm_mso_utils_read_object(ctxp, ITEM_OBJECT, item_pdp, &item_r_flistp, ebufp);
		serv_pdp = PIN_FLIST_FLD_GET(item_r_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
		if ( serv_pdp != NULL && 
		     strstr ( PIN_POID_GET_TYPE(serv_pdp),"/service/telco/broadband")) 
		{

			if ( item_pdp != NULL && !(
				strstr ( PIN_POID_GET_TYPE(item_pdp),"/item/Service_Tax") || 
				strstr ( PIN_POID_GET_TYPE(item_pdp),"/item/Education_Cess") || 
				strstr ( PIN_POID_GET_TYPE(item_pdp),"/item/Secondary_Higher_Education_Cess") || 
				strstr ( PIN_POID_GET_TYPE(item_pdp),"/item/VAT") || 
				strstr ( PIN_POID_GET_TYPE(item_pdp),"/item/adjustment") || 
				strstr ( PIN_POID_GET_TYPE(item_pdp),"/item/cycle_tax"))) 
			{
				tmp_flistp = PIN_FLIST_ELEM_ADD (*r_flistpp, PIN_FLD_ITEMS, count, ebufp);
				PIN_FLIST_FLD_COPY (item_r_flistp, PIN_FLD_POID, tmp_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY (item_r_flistp, PIN_FLD_SERVICE_OBJ, tmp_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_FLIST_FLD_SET  (tmp_flistp, PIN_FLD_ITEM_TYPE, (void*)PIN_POID_GET_TYPE (
					PIN_FLIST_FLD_GET  (item_r_flistp, PIN_FLD_POID, 1, ebufp)), ebufp);
				PIN_FLIST_FLD_COPY (item_r_flistp, PIN_FLD_ITEM_TOTAL, tmp_flistp, PIN_FLD_ITEM_TOTAL, ebufp);
				PIN_FLIST_FLD_COPY (item_r_flistp, PIN_FLD_DUE, tmp_flistp, PIN_FLD_DUE, ebufp);
				PIN_FLIST_FLD_COPY (item_flistp, PIN_FLD_AMOUNT, tmp_flistp, PIN_FLD_AMOUNT, ebufp);
				PIN_FLIST_FLD_COPY (item_r_flistp, PIN_FLD_EFFECTIVE_T, tmp_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
				PIN_FLIST_FLD_COPY (item_r_flistp, PIN_FLD_ACCOUNT_OBJ, tmp_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
				count++;
			}
		}
		PIN_FLIST_DESTROY_EX(&item_r_flistp, NULL);
	}
	/* Loop through the items and filter only the items belongs to /service/telco/broadband - End*/
cleanup:
	if (count == 0) 
	{
		PIN_FLIST_DESTROY_EX(r_flistpp, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_extract_transfer_items output flist", *r_flistpp);
	PIN_FLIST_DESTROY_EX(&item_transfer_flistp, NULL);
}

static int
fm_mso_commission_lco_bal_trans (
	pcm_context_t           *ctxp,
	int32                   commission_flag,
	int32                   from_bucket,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp) 
{

	pin_flist_t             *flistp = NULL;
	pin_flist_t             *lco_bal_trans_r_flistp = NULL;
	pin_flist_t             *err_flistp = NULL;
	//pin_flist_t             *plan_info_flistp = NULL;
	//pin_flist_t             *tmp_flistp = NULL;
	pin_flist_t             *amount_flistp = NULL;
	pin_flist_t             *bal_flistp = NULL;
	int32                   *result = NULL;
	int32                   *comm_model = NULL;
	int32                   err_code = 0;
	int32                   tmp_enum1 = 356;
	int32                   tmp_enum3 = 1;
	int32                   tmp_enum4 = 0;
	pin_decimal_t           *amountp = NULL;
	pin_decimal_t           *zerop = pbo_decimal_from_str("0.0",ebufp);

	// Input Flist:
	//        PIN_POID                /account - Funds are to be transferred from
	//        TRANSFER_TARGET         /bal_grp - Funds are to be transferred to
	//        AMOUNT
	//        PROGRAM_NAME
	//        CURRENCY
	//        BAL_INFO [0]            Source
	//        BAL_INFO [0]            Target

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_commission_lco_bal_trans");

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_commission_lco_bal_trans", ebufp);
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_commission_lco_bal_trans input flist",i_flistp);

	/* Prepare the input flist for balance transfer start*/
	flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_OBJ, flistp, PIN_FLD_POID, ebufp);
	if(commission_flag == LCO_TO_HW_SHARE)
	{
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_OBJ, flistp, PIN_FLD_TRANSFER_TARGET, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, flistp, PIN_FLD_TRANSFER_TARGET, ebufp);
	}
	bal_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_BAL_INFO, 0, ebufp);
	if(commission_flag == LCO_TO_HW_SHARE)
	{
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_COMM_BUCKET_OBJ, bal_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);	
		bal_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_BAL_INFO, 1, ebufp); /*Target Balance*/
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_BAL_BUCKET_OBJ, bal_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
	}
	else
	{
		if (from_bucket == FROM_LCO_BALANCE_BUCKET) 
		{
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_BAL_BUCKET_OBJ, bal_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp); 
		}
		else 
		{
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_BAL_BUCKET_OBJ, bal_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
		}
		bal_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_BAL_INFO, 1, ebufp); /*Target Balance*/
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BAL_GRP_OBJ, bal_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
	}
	PIN_FLIST_FLD_SET(flistp,PIN_FLD_CURRENCY, (void*)&tmp_enum1, ebufp);
	PIN_FLIST_FLD_SET(flistp,PIN_FLD_FLAGS, (void*)&tmp_enum3, ebufp);
	/* Prepare the input flist for balance transfer end*/

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before Switch");
	/* Retrieve the amount to be transfered start*/
	switch (commission_flag) {
	    case GROSS_AMT:
		amount_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_CHARGE_HEAD_INFO, 0, ebufp);
		PIN_FLIST_FLD_COPY(amount_flistp, PIN_FLD_TOTALS, flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_PROGRAM_NAME, "LCO Commission-Bal Transfer (Gross)", ebufp);
	    break;
	    case HATHWAY_SHARE:
		amount_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_AMOUNTS, HATHWAY_SHARE, 1, ebufp);
		PIN_FLIST_FLD_COPY(amount_flistp, PIN_FLD_AMOUNT, flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_PROGRAM_NAME, "LCO Commission-Bal Transfer (Hathway_Share)", ebufp);
	    break;
	    case LCO_SHARE:
		amount_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_AMOUNTS, LCO_SHARE, 1, ebufp);
		PIN_FLIST_FLD_COPY(amount_flistp, PIN_FLD_AMOUNT, flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_PROGRAM_NAME, "LCO Commission-Bal Transfer (LCO_Share)", ebufp);
	    break;
	    case LCO_SHARE_TAX:
		amount_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_AMOUNTS, LCO_SHARE_TAX, 1, ebufp);
		PIN_FLIST_FLD_COPY(amount_flistp, PIN_FLD_AMOUNT, flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_PROGRAM_NAME, "LCO Commission-Bal Transfer (LCO_Share_Tax)", ebufp);
	    break;
	    case LCO_TO_HW_SHARE:
		amount_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_AMOUNTS, LCO_SHARE, 1, ebufp);
		PIN_FLIST_FLD_COPY(amount_flistp, PIN_FLD_AMOUNT, flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_PROGRAM_NAME, "LCO Commission-Bal Transfer (LCO_To_Hathway)", ebufp);
	    break;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After Switch");
	tmp_enum4=from_bucket;
	if (from_bucket != FROM_LCO_COMMISSION_BUCKET) 
	{
		amountp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_AMOUNT, 0, ebufp);
	    
		if ( pbo_decimal_compare(amountp, zerop, ebufp) == -1) {
			tmp_enum4=0 ; //Allowing system to transfer negative balance
		}
	}
	comm_model = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp); 
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After comm_model");
	if (*comm_model > 2) {
		tmp_enum4=0 ; //Allowing system to transfer negative balance
	}
	PIN_FLIST_FLD_SET(flistp,PIN_FLD_VERIFY_BALANCE, (void*)&tmp_enum4, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_TRANSFERED, amountp, ebufp);
	/* Retrieve the amount to be transfered end*/
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before Calling MSO_OP_COMMISSION_LCO_BAL_TRANSFER");

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_commission_lco_bal_trans input flist new ",flistp);
	PCM_OP(ctxp, MSO_OP_COMMISSION_LCO_BAL_TRANSFER, from_bucket, flistp, &lco_bal_trans_r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&flistp, NULL);

	if(lco_bal_trans_r_flistp!=NULL) 
	{
		result = PIN_FLIST_FLD_GET(lco_bal_trans_r_flistp, PIN_FLD_RESULT, 0, ebufp);
		if (*result != 0) 
		{
			switch(*result) {
				case 1 : err_code = MSO_ERR_LCO_INSUFFICIENT_BALANCE; break;
				case 2 : err_code = MSO_ERR_INVALID_TRANSFER_AMOUNT; break;
			}
			fm_mso_utils_prepare_error_flist(ctxp, i_flistp, &err_flistp,"61015", "LCO COMMISSION - Insufficient LCO Balance", ebufp);
			//pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, err_code, 0, 0, 0);
			PIN_FLIST_DESTROY_EX(&lco_bal_trans_r_flistp, NULL);
			*r_flistpp = err_flistp;
			return 0;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_commission_process_commission LCO Balance Transfer Successful", lco_bal_trans_r_flistp);
	}

	*r_flistpp = lco_bal_trans_r_flistp;

	if(!pbo_decimal_is_null(zerop,ebufp)) 
	{
		pbo_decimal_destroy(&zerop);
	}

	return 1;
}

static int
fm_mso_commission_lco_bal_impact (
	pcm_context_t           *ctxp,
	int32                   commission_flag,
	int32                   from_bucket,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp) {


	pin_flist_t		*flistp = NULL;
	pin_flist_t		*amount_flistp = NULL;
	pin_flist_t		*lco_bal_impact_r_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;

	pin_decimal_t		*amountp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_commission_lco_bal_impact", ebufp);
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_lco_bal_impact input flist", i_flistp);

	/* Prepare the reequired input flist to do balance impact START*/
	flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_OBJ, flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_INCOLLECT_SERV_OBJ, flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EFFECTIVE_T, flistp, PIN_FLD_START_T, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EFFECTIVE_T, flistp, PIN_FLD_END_T, ebufp);
	tmp_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_CHARGE_HEAD_INFO, 0, ebufp);
	PIN_FLIST_SUBSTR_SET(flistp, tmp_flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp);

	tmp_flistp = PIN_FLIST_SUBSTR_GET(flistp, MSO_FLD_CHARGE_HEAD_INFO, 0, ebufp);
	if(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, 1,ebufp))
	    PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	if(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_ITEM_TYPE, 1, ebufp))
	    PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_ITEM_TYPE, ebufp);
	if(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_SESSION_OBJ, 1, ebufp))
	    PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_SESSION_OBJ, ebufp);
	if(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp))
	    PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
	if(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_TOTALS, 1, ebufp))
	    PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_TOTALS, ebufp);
	if(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_TAX, 1, ebufp))
	    PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_TAX, ebufp);
	if(PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_AMOUNT, 1, ebufp))
	    PIN_FLIST_FLD_DROP(tmp_flistp, PIN_FLD_AMOUNT, ebufp);
	/* Prepare the reequired input flist to do balance impact END*/

	/* Retrieve the amount to be transfered start*/
	switch (commission_flag) {
	    case GROSS_AMT:
		amount_flistp = PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_CHARGE_HEAD_INFO, 0, 0, ebufp);
		PIN_FLIST_FLD_COPY(amount_flistp, PIN_FLD_TOTALS, flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_PROGRAM_NAME, "LCO Commission-Bal Impact (Gross)", ebufp);
	    break;
	    case HATHWAY_SHARE:
		amount_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_AMOUNTS, HATHWAY_SHARE, 1, ebufp);
		PIN_FLIST_FLD_COPY(amount_flistp, PIN_FLD_AMOUNT, flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_PROGRAM_NAME, "LCO Commission-Bal Impact (Hathway_Share)", ebufp);
	    break;
	    case LCO_SHARE:
		amount_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_AMOUNTS, LCO_SHARE, 1, ebufp);
		PIN_FLIST_FLD_COPY(amount_flistp, PIN_FLD_AMOUNT, flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_PROGRAM_NAME, "LCO Commission-Bal Impact (LCO_Share)", ebufp);
	    break;
	    case LCO_SHARE_TAX:
		amount_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_AMOUNTS, HATHWAY_SHARE, 1, ebufp);
		PIN_FLIST_FLD_COPY(amount_flistp, PIN_FLD_AMOUNT, flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_SET(flistp,PIN_FLD_PROGRAM_NAME, "LCO Commission-Bal Impact (LCO_Share_Tax)", ebufp);
	    break;
	}
	/* Retrieve the amount to be transfered end*/
	amountp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_AMOUNT, 0, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_TRANSFERED, amountp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_lco_bal_impact input flist new ", flistp);
	PCM_OP(ctxp, MSO_OP_COMMISSION_LCO_BAL_IMPACT, from_bucket, flistp, &lco_bal_impact_r_flistp, ebufp);

	if(lco_bal_impact_r_flistp!=NULL) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_commission_process_commission LCO Balance Impact Successful", lco_bal_impact_r_flistp);
	}
	*r_flistpp = lco_bal_impact_r_flistp;
	PIN_FLIST_DESTROY_EX(&flistp, NULL);

	return 1;
}



static void
fm_mso_commission_create_report (
	pcm_context_t           *ctxp,
	int             	flags,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistpp,
	pin_errbuf_t            *ebufp)
{
	/* Variable Declaration Start */
	pin_flist_t             *flistp=NULL;
	pin_flist_t             *r_flistp=NULL;
	pin_flist_t             *tmp_flistp=NULL;
	pin_flist_t             *tmp_flistp1=NULL;
	poid_t                  *comm_objp=NULL;
	
	int32                   *comm_rule=NULL;
	int32			acnt_type_lco = ACCT_TYPE_LCO;
	int32			acnt_type_dt  = ACCT_TYPE_DTR;
	int32			acnt_type_sdt = ACCT_TYPE_SUB_DTR;

	/* Variable Declaration End */

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_commission_create_report", ebufp);
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_create_report input flist", i_flistp);

	flistp = PIN_FLIST_CREATE(ebufp);

	/*Collect the required data from the input flist end*/
	comm_rule = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_COMMISSION_RULE, 0, ebufp);
	if (comm_rule != NULL)  {
	    switch (*comm_rule) {
		case ACTIVE_CUST_COUNT_BASED_COMMISSION: 
		    comm_objp = PIN_POID_CREATE(PIN_POID_GET_DB(PCM_GET_USERID(ctxp)), 
			"/mso_commission_rpt/count_based", -1, ebufp);
		break;
		case CHARGE_HEAD_BASED_COMMISSION: 
		    comm_objp = PIN_POID_CREATE(PIN_POID_GET_DB(PCM_GET_USERID(ctxp)), 
			"/mso_commission_rpt/chrg_head_based", -1, ebufp);
		break;
	    } 
	}
	PIN_FLIST_FLD_PUT(flistp,PIN_FLD_POID, comm_objp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_OBJ, flistp, MSO_FLD_LCO_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SDT_OBJ, flistp, MSO_FLD_SDT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DT_OBJ, flistp, MSO_FLD_DT_OBJ, ebufp);

	if (flags == LCO_COMMISSION) {
	    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_COMM_BUCKET_OBJ, flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
	    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_OUTCOLLECT_SERV_OBJ, flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	    PIN_FLIST_FLD_SET(flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_lco, ebufp);
	}
	else if (flags == DTR_COMMISSION)
	{
	    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DTR_COMM_BUCKET_OBJ, flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
	    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	    PIN_FLIST_FLD_SET(flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_dt, ebufp);
	}
	else if (flags == SDT_COMMISSION)
	{
	    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DTR_COMM_BUCKET_OBJ, flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
	    PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	    PIN_FLIST_FLD_SET(flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_sdt, ebufp);
	}

	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EFFECTIVE_T, flistp, PIN_FLD_EFFECTIVE_T, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EVENT_OBJ, flistp, PIN_FLD_EVENT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_COMMISION_MODEL, flistp, MSO_FLD_COMMISION_MODEL, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_COMMISION_SERVICE, flistp, MSO_FLD_COMMISION_SERVICE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_COMMISSION_RULE, flistp, MSO_FLD_COMMISSION_RULE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_COMMISSION_CHARGE, flistp, MSO_FLD_COMMISSION_CHARGE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATUS, flistp, PIN_FLD_STATUS, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATUS_MSG, flistp, PIN_FLD_STATUS_MSG, ebufp);

	if (*comm_rule != ACTIVE_CUST_COUNT_BASED_COMMISSION) {
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SESSION_OBJ, flistp, PIN_FLD_SESSION_OBJ, ebufp);

		tmp_flistp = PIN_FLIST_SUBSTR_ADD(flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp);
		tmp_flistp1 = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_CHARGE_HEAD_INFO, 1, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp1, MSO_FLD_CHARGE_HEAD, tmp_flistp, MSO_FLD_CHARGE_HEAD, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp1, MSO_FLD_CHARGE_HEAD_TOTAL, tmp_flistp, MSO_FLD_CHARGE_HEAD_TOTAL, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp1, MSO_FLD_COLLECTION_AMT, tmp_flistp, MSO_FLD_COLLECTION_AMT, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp1, PIN_FLD_ITEM_OBJ, tmp_flistp, PIN_FLD_ITEM_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp1, PIN_FLD_POID, tmp_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp1, PIN_FLD_ACCOUNT_OBJ, tmp_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	}
	else {
		tmp_flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_CUSTOM_INFO, ebufp);
		tmp_flistp1 = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_CUSTOM_INFO, 1, ebufp);
		PIN_FLIST_FLD_COPY(tmp_flistp1, PIN_FLD_COUNT, tmp_flistp, PIN_FLD_COUNT, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_create_report create_obj input flist", flistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, flistp, &r_flistp, ebufp);
	if ( PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_commission_create_report error in creating object", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_create_report create_obj output flist", r_flistp);

	*r_flistpp = r_flistp;
cleanup:
	PIN_FLIST_DESTROY_EX(&flistp, NULL);

}

static int
fm_mso_commission_get_lco_share (
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **lco_share_flistp,
	pin_errbuf_t            *ebufp)
{
	//pin_decimal_t           *hath_share = NULL;
	//pin_decimal_t           *lco_tax_share = NULL;

	pin_flist_t             *lco_calc_comm_flistp = NULL;
	//pin_flist_t             *calc_tax_share_flp = NULL;
	//pin_flist_t             *ret_flistp = NULL;
	//pin_flist_t             *bal_flistp = NULL;
	int32             	*rating_status = NULL;
	int32			success = 0;
	int32			failed = 1;
	int32			acnt_type_lco = ACCT_TYPE_LCO;
	int			return_val = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_commission_get_lco_share", ebufp);
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_lco_share input flist", i_flistp);

	/* Retrieve the LCO Commission start*/
	PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_COMMISSION_FOR, &acnt_type_lco, ebufp);
	PCM_OP(ctxp, MSO_OP_COMMISSION_APPLY_COMMISSION, PCM_OPFLG_CALC_ONLY, i_flistp, &lco_calc_comm_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_lco_share apply commission return flist", lco_calc_comm_flistp);

	if (lco_calc_comm_flistp != NULL) 
	{
		rating_status = PIN_FLIST_FLD_GET(lco_calc_comm_flistp, PIN_FLD_RATING_STATUS, 0, ebufp);
		*lco_share_flistp = PIN_FLIST_CREATE(ebufp);
		if (*rating_status == SUCCESSFUL_RATING) 
		{
			PIN_FLIST_FLD_COPY(lco_calc_comm_flistp, PIN_FLD_RATING_STATUS, *lco_share_flistp, PIN_FLD_STATUS, ebufp);
			PIN_FLIST_FLD_COPY(lco_calc_comm_flistp, PIN_FLD_AMOUNT, *lco_share_flistp, PIN_FLD_AMOUNT, ebufp);
			return_val = 0;
		}
		else if (*rating_status == NO_MATCHING_SELECTOR_DATA) 
		{
			PIN_FLIST_FLD_SET(*lco_share_flistp, PIN_FLD_STATUS, &success, ebufp);
			PIN_FLIST_FLD_SET(*lco_share_flistp, PIN_FLD_ERROR_DESCR, "LCO COMMISSION - Charge head not applicable for commission", ebufp);
			return_val = 1;
			goto cleanup;
		}
		else 
		{
			PIN_FLIST_FLD_SET(*lco_share_flistp, PIN_FLD_STATUS, &failed, ebufp);
			PIN_FLIST_FLD_COPY(lco_calc_comm_flistp, PIN_FLD_ERROR_DESCR, *lco_share_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			return_val = 1;
			goto cleanup;
		}
	}
	/* Retrieve the LCO Commission end*/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_get_lco_share output flist", *lco_share_flistp);
cleanup:
	PIN_FLIST_DESTROY_EX(&lco_calc_comm_flistp, NULL);
	return return_val;
}

static void
fm_mso_commission_calc_tax_share (
	pcm_context_t       *ctxp,
	pin_flist_t         *i_flistp,
	pin_flist_t         **r_flistpp,
	pin_errbuf_t        *ebufp)
{
	pin_flist_t             *tmp_flistp = NULL;
	pin_flist_t             *charge_head_flistp = NULL;
	pin_flist_t             *r_flistp = NULL;
	pin_decimal_t           *tax_p = NULL;
	pin_decimal_t           *total_p = NULL;
	pin_decimal_t           *amount_p = NULL;
	pin_decimal_t           *percp = NULL;
	pin_decimal_t           *total_lco_share = NULL;
	//pin_decimal_t           *lco_share_dcp = NULL;
	pin_decimal_t           *lco_tax_p = NULL;
	pin_decimal_t           *hath_share_p = NULL;
	pin_decimal_t           *lco_share = NULL;
	pin_decimal_t           *lco_share_neg = NULL;
	void                    *vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_commission_calc_tax_share", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_calc_tax_share input flist", i_flistp);

	lco_share_neg = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_COMMISSION_CHARGE, 1, ebufp);
	lco_share = pbo_decimal_negate(lco_share_neg, ebufp);

	charge_head_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, MSO_FLD_CHARGE_HEAD_INFO, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(charge_head_flistp,PIN_FLD_TAX,0,ebufp);
	if(vp) tax_p = pbo_decimal_copy(vp,ebufp);
	vp = PIN_FLIST_FLD_GET(charge_head_flistp,PIN_FLD_AMOUNT,0,ebufp);
	if(vp) amount_p = pbo_decimal_copy(vp,ebufp);
	vp = PIN_FLIST_FLD_GET(charge_head_flistp,PIN_FLD_TOTALS,0,ebufp);
	if(vp) total_p = pbo_decimal_copy(vp,ebufp);

	percp = pbo_decimal_divide(lco_share,amount_p,ebufp);
	lco_tax_p = pbo_decimal_multiply(tax_p,percp,ebufp);
	total_lco_share = pbo_decimal_add(lco_tax_p,lco_share,ebufp);
	hath_share_p = pbo_decimal_subtract(total_p,total_lco_share,ebufp);

	r_flistp = PIN_FLIST_CREATE(ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_AMOUNTS, HATHWAY_SHARE, ebufp);
	PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_AMOUNT, hath_share_p, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_AMOUNTS, LCO_SHARE, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_AMOUNT, lco_share, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_AMOUNTS, LCO_SHARE_TAX, ebufp);
	PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_AMOUNT, lco_tax_p, ebufp);
	*r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);

	if (!pbo_decimal_is_null(tax_p, ebufp)) {
		pbo_decimal_destroy(&tax_p);
	}
	if (!pbo_decimal_is_null(amount_p, ebufp)) {
		pbo_decimal_destroy(&amount_p);
	}
	if (!pbo_decimal_is_null(total_p, ebufp)) {
		pbo_decimal_destroy(&total_p);
	}
	if (!pbo_decimal_is_null(percp, ebufp)) {
		pbo_decimal_destroy(&percp);
	}
	if (!pbo_decimal_is_null(lco_share, ebufp)) {
		pbo_decimal_destroy(&lco_share);
	}
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_calc_tax_share return flist", *r_flistpp);
	return;
}

static void
fm_mso_commission_process_dt_sdt_commission (
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	int32			flags,
	pin_errbuf_t		*ebufp) {

	pin_flist_t	     *dtr_comm_flistp = NULL;
	pin_flist_t	     *charge_head_flistp = NULL;
	poid_t	     	     *evt_pdp = NULL;
	int32	     	     *comm_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_commission_process_dt_sdt_commission", ebufp);
		return;
	}

	//Prepare the commission flist for distributor account
	dtr_comm_flistp = PIN_FLIST_CREATE(ebufp);
	evt_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(PCM_GET_USERID(ctxp)), 
		"/event/billing/settlement/ws/outcollect/charge_head_based", -1, ebufp);
	PIN_FLIST_FLD_PUT(dtr_comm_flistp, PIN_FLD_POID, evt_pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DT_OBJ, dtr_comm_flistp, MSO_FLD_DT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SDT_OBJ, dtr_comm_flistp, MSO_FLD_SDT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_LCO_OBJ, dtr_comm_flistp, MSO_FLD_LCO_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, dtr_comm_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_COMMISSION_RULE, dtr_comm_flistp, MSO_FLD_COMMISSION_RULE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EFFECTIVE_T, dtr_comm_flistp, PIN_FLD_EFFECTIVE_T, ebufp);

	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, dtr_comm_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SESSION_OBJ, dtr_comm_flistp, PIN_FLD_SESSION_OBJ, ebufp);
	
	charge_head_flistp = PIN_FLIST_SUBSTR_GET (i_flistp, MSO_FLD_CHARGE_HEAD_INFO, 1, ebufp);
	PIN_FLIST_SUBSTR_SET(dtr_comm_flistp, charge_head_flistp, MSO_FLD_CHARGE_HEAD_INFO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATUS, dtr_comm_flistp, PIN_FLD_STATUS, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_STATUS_MSG, dtr_comm_flistp, PIN_FLD_STATUS_MSG, ebufp);
	
	if (flags == MSO_FLD_DT_OBJ )
	{
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, dtr_comm_flistp, MSO_FLD_DTR_OUTCOLLECT_SERV_OBJ, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_dt_commission input flist", dtr_comm_flistp);
		PCM_OP(ctxp, MSO_OP_COMMISSION_PROCESS_COMMISSION, DTR_COMMISSION, dtr_comm_flistp, r_flistpp, ebufp);
	}
	else if (flags == MSO_FLD_SDT_OBJ)
	{
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SDT_OUTCOLLECT_SERV_OBJ, dtr_comm_flistp, MSO_FLD_SDT_OUTCOLLECT_SERV_OBJ, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_sdt_commission input flist", dtr_comm_flistp);
		PCM_OP(ctxp, MSO_OP_COMMISSION_PROCESS_COMMISSION, SDT_COMMISSION, dtr_comm_flistp, r_flistpp, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&dtr_comm_flistp, NULL);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_commission_process_dt_sdt_commission output flist", *r_flistpp);
	if (*r_flistpp != NULL) {
	    comm_status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 0, ebufp);
	    if (comm_status != NULL && *comm_status == 0) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_commission_process_dt_sdt_commission commission succeeded", *r_flistpp);
	    }
	    else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_mso_commission_process_dt_sdt_commission commission failed", *r_flistpp);
		return;
	    }
	}

}

static void 
fm_mso_get_open_credit_item_due (
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_decimal_t		*total_duep,
        pin_errbuf_t            *ebufp)
{	
	pin_flist_t     *search_flistp = NULL;
	pin_flist_t     *search_oflistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *res_flistp = NULL;
	
	poid_t		*search_pdp = NULL;
	
	int32           flag = 1;
	int32           database = 1;
	//int32           i_status = PIN_ITEM_STATUS_CLOSED;

	pin_decimal_t	*due = NULL; 
	
	char            *template = "select SUM(F3) from /item where F1 = V1 and F2 = V2 and F3 < V3 and F4.type = V4 ";

	if( PIN_ERRBUF_IS_ERR( ebufp ) )
 		return;
	PIN_ERRBUF_CLEAR( ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"open credit items input flist", i_flistp);

	search_flistp = PIN_FLIST_CREATE( ebufp );
	search_pdp = PIN_POID_CREATE( database, "/search", -1, ebufp );
	PIN_FLIST_FLD_PUT( search_flistp, PIN_FLD_POID, search_pdp, ebufp );
	
	PIN_FLIST_FLD_SET( search_flistp, PIN_FLD_FLAGS, &flag, ebufp );
	PIN_FLIST_FLD_SET( search_flistp, PIN_FLD_TEMPLATE, template, ebufp );
	
	args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 2, ebufp );;
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, args_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	due = pbo_decimal_from_str("0.0", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_PUT( args_flistp, PIN_FLD_ITEM_TOTAL, (void *)due, ebufp );
	args_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_ARGS, 4, ebufp );
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, args_flistp, PIN_FLD_POID, ebufp);
	
	res_flistp = PIN_FLIST_ELEM_ADD( search_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET( res_flistp, PIN_FLD_AMOUNT, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"open credit items search input", search_flistp);
	PCM_OP( ctxp, PCM_OP_SEARCH, 0, search_flistp, &search_oflistp, ebufp );
	PIN_FLIST_DESTROY_EX( &search_flistp, NULL );	

	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error during open credit item search", ebufp);
		return;
	}	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"open credit items search output", search_oflistp);
	res_flistp = PIN_FLIST_ELEM_GET(search_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"open credit items results input", res_flistp);
	due = (pin_decimal_t *)PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_AMOUNT, PIN_ELEMID_ANY, 1, ebufp);
	if (!pbo_decimal_is_null(due, ebufp))
		pbo_decimal_add_assign(total_duep, due, ebufp);
	
	PIN_FLIST_DESTROY_EX(&search_oflistp, NULL);
	return;
}
