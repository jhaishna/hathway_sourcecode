/*******************************************************************
 * File:	fm_mso_rate_aaa_rate_bb_event.c
 * Opcode:	MSO_OP_RATE_AAA_RATE_BB_EVENT 
 * Owner:	Nagaraj
 * Created:	10-JULY-2014
 * Description: 
********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_act_rate_bb_aaa_event.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif


#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/tcf.h"
#include "ops/bal.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_errs.h"
#include "mso_rate.h"
#include "mso_prov.h"
#include "mso_ntf.h"
#include "ops/rate.h"
#include "pin_cust.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_act.h"
#include "pin_rate.h"
#include "ops/cust.h"
#include "ops/bill.h"
#include "fm_bill_utils.h"
#include "mso_glid.h"
#include "pin_currency.h"
#include "mso_cust.h"
#include "mso_commission.h"
#include "mso_lifecycle_id.h"

/* Acct Status Type */
#define ACCT_START 	1
#define ACCT_INTERIM 	3
#define ACCT_STOP	2

/* Rating Status */
#define SUCCESS 	0

/* FUP Status*/
#define FUP_NA  	0
#define WITHIN_FUP 	1
#define LIMIT_EXCEED 	2

/* Resource */
#define LIMITED_RESOURCE 1100010
#define FUP_RESOURCE	 1000104

/* Prov Status*/
#define PROV_ACTIVE  	2
#define PROV_FAILED  	3
#define PROV_DEACTIVE 	4 
#define PROV_HOLD 	5
#define PROV_SUSPENDED 	6
#define PROV_TERMINATED 7


/* NTF Status*/
#define	NTF_NO				0
#define	NTF_THROT_DEACT		100


int32 		success_status = 0;
int32 		failure_status = 1;
const char	*ONE_MB = "1048576.0";

int64		CREDIT_THRESHOLD_BREACH_FLAG = NTF_BB_DATA_LIMIT;
int64		CREDIT_LIMIT_BREACH_FLAG = NTF_BB_DATA_LIMIT_EXPIRED;
int64		FUP_LIMIT_THRESHOLD_BREACH_FLAG = NTF_BEFORE_FUP;
int64		FUP_LIMIT_BREACH_FLAG =	NTF_AFTER_FUP;

extern 		credit_threshold_config_t credit_thresholds[10];
extern 		int32 num_of_resources_configured;

EXPORT_OP void 
op_mso_rate_aaa_rate_bb_event(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_rate_aaa_rate_bb_event(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static int32 
fm_mso_rate_aaa_record_validate(
	pcm_context_t		*ctxp,
	char			*session_id,
	int32			record_type,
	int32			cdr_duration,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

void 
fm_mso_rate_aaa_get_service(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);
	
static void 
fm_mso_rate_aaa_create_usage_session( 
	pcm_context_t   	*ctxp,
	pin_decimal_t		*in_flistp,
	int32			record_type,
	poid_t          	*plan_pdp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_rate_aaa_update_usage_session(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	poid_t			*usage_sess_pdp,
	int32			record_type,
	int32			trans_flags,
	pin_errbuf_t		*ebufp);

void 
fm_mso_rate_aaa_update_daily_usage(
	pcm_context_t		*ctxp,
	pin_flist_t		*active_session_flistp,
	time_t			day_t,
	pin_flist_t		*aaa_in_flistp,
	poid_t          	*plan_pdp,
	pin_errbuf_t		*ebufp);		
				
int32 
fm_mso_rate_aaa_get_charged_dates(
	pcm_context_t  		*ctxp,
	poid_t          	*svc_pdp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t    	*ebufp);

void 
fm_mso_rate_aaa_suspend_event( 
	pcm_context_t		*ctxp,
	pin_flist_t     	*i_flistp,
	pin_flist_t     	*r_flistp,
	poid_t			*acct_pdp,
	pin_errbuf_t    	*ebufp);
	
static void 
fm_mso_rate_aaa_get_aso(
	pcm_context_t		*ctxp,
	poid_t			*serv_pdp,
	poid_t			*acct_pdp,
	time_t			valid_from,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);
	
void 
fm_mso_rate_aaa_threshold_check(
	pcm_context_t   	*ctxp,
	pin_flist_t		*svc_flistp,	
	poid_t			*acct_pdp,
	poid_t			*serv_pdp,
	int32			ntf_status,
	int32			fup_status,
	pin_errbuf_t  		*ebufp);
	
void 
fm_mso_rate_aaa_get_credit_profile(
	pcm_context_t   	*ctxp,
	poid_t			*acc_pdp, 
	pin_flist_t		*svc_flistp,
	pin_flist_t		**credit_profile_flistp,
	pin_errbuf_t  		*ebufp);
		
void 
fm_mso_rate_aaa_update_credit_limits(
	pin_flist_t		*res_flistp,
	pin_flist_t		**credit_profile_flistp,
	pin_decimal_t		*credit_profile_part,
	pin_errbuf_t  		*ebufp);

void
fm_mso_rate_aaa_get_balances(
	pcm_context_t  		*ctxp,
	poid_t         		*account_pdp,
	poid_t         		*serv_pdp,
	pin_flist_t    		**o_flistpp,
	int32			resource_id,
	pin_errbuf_t  		*ebufp);

int32 
fm_mso_rate_aaa_res_threshold_validation (
	int32			rec_id);

void 
fm_mso_rate_aaa_deactivate_customer(
	pcm_context_t   	*ctxp,
	poid_t			*acct_pdp,
	poid_t			*serv_pdp,
	pin_errbuf_t  		*ebufp);
	
void 
fm_mso_rate_aaa_throttle_customer(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	poid_t			*serv_pdp,
	int32			res_id,
	pin_errbuf_t  		*ebufp);

void 
fm_mso_rate_aaa_threshold_ntf(
	pin_flist_t 		*i_flistp,
	pcm_context_t   	*ctxp,
	pin_decimal_t 		*curr_bal, 
	pin_decimal_t 		*impact_amount, 
	int32 			rec_id, 
	pin_decimal_t 		*credit_range,
	pin_errbuf_t 		*ebufp);
	
extern void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);
	
void 
fm_mso_rate_aaa_create_ntf(
	pcm_context_t   *ctxp,
	poid_t			*acct_pdp,
	poid_t			*serv_pdp,	
	int32 			rec_id,
	pin_decimal_t	*percentage,
	int32			ntf_type,
	pin_errbuf_t 	*ebufp) ;

void
fm_mso_rate_aaa_update_bb_info(
	pcm_context_t   *ctxp,
	poid_t			*serv_pdp,	
	int32			update_value,
	int32			update_type,	//0 - fup status , 1 - NTF status
	pin_errbuf_t 	*ebufp);

void
fm_mso_rate_get_credit_limit_part(
        pcm_context_t   *ctxp,
        poid_t          *serv_pdp,
        poid_t          *acnt_pdp,
        poid_t          *plan_pdp,
	pin_decimal_t	**credit_profile_part,
        pin_errbuf_t    *ebufp);

void
fm_mso_rate_aaa_get_plan_name(
        pcm_context_t           *ctxp,
        poid_t                  *plan_pdp,
        pin_flist_t	       	**r_flistpp,
        pin_errbuf_t            *ebufp);


int32
fm_mso_rate_aaa_throttle_product_chk(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        poid_t                  *serv_pdp,
        pin_errbuf_t            *ebufp);

int32 
fm_mso_rate_aaa_get_pp(
	pcm_context_t  		*ctxp,
	poid_t          	*svc_pdp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t    	*ebufp);	

/*******************************************************************
 * MSO_OP_RATE_AAA_RATE_BB_EVENT  
 *******************************************************************/
void 
op_mso_rate_aaa_rate_bb_event(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t        	*ret_flistp = NULL;
	
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_RATE_AAA_RATE_BB_EVENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_rate_aaa_rate_bb_event error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_rate_aaa_rate_bb_event input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_rate_aaa_rate_bb_event(ctxp, flags, i_flistp, &ret_flistp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		*r_flistpp = NULL;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_mso_rate_aaa_rate_bb_event error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "op_mso_rate_aaa_rate_bb_event input flist", i_flistp);
	}
	else 
	{
		*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "op_mso_rate_aaa_rate_bb_event return flist", *r_flistpp);		
	}
	
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return;
}

/*******************************************************************
 * This is the default implementation for this opcode
 *******************************************************************/
static void 
fm_mso_rate_aaa_rate_bb_event(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*aaa_input_flist = NULL;
	pin_flist_t		*serv_ret_flistp = NULL;
	pin_flist_t		*val_out_flistp = NULL;
	pin_flist_t		*bb_info_substr_flistp = NULL;
	pin_flist_t		*extended_input_flist = NULL;
	pin_flist_t		*aaa_split_in_flistp = NULL;
	pin_flist_t		*aaa_split_out_flistp = NULL;
	pin_flist_t		*aaa_output_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*pp_ret_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	pin_flist_t		*telco_flistp = NULL;
	pin_flist_t		*svc_flistp = NULL;
	
	poid_t			*acct_pdp = NULL;
	poid_t			*serv_pdp = NULL;
	poid_t			*usage_sess_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	
	//char			error_code [5];
	//char			error_desc [255];	
	char			*session_id = NULL;
	char			*time_str = NULL;
	char			*bb_obj_type = "broadband";
	char			new_session_id [255];
	char			*prg_name = NULL;
	const char		*FOUR_GB = "4294967296.0";

	pin_decimal_t		*four_gb = pbo_decimal_from_str(FOUR_GB, ebufp);
	pin_decimal_t		*bytes_downlink = NULL;
	pin_decimal_t		*bytes_uplink = NULL;
	pin_decimal_t		*gigaword = NULL;
	pin_decimal_t		*input_bytes = NULL;
	pin_decimal_t		*zero_qty = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*aso_bytes_uplink = NULL;
	pin_decimal_t		*aso_bytes_dnlink = NULL;
	pin_decimal_t		*rated_bytes_uplink = NULL;
	pin_decimal_t 		*rated_bytes_downlink = NULL;
	pin_decimal_t		*qty = NULL;
	pin_decimal_t		*quantity = NULL;
	pin_decimal_t		*total_bytes_used = pin_decimal( "0.0", ebufp );

	int32			*rating_status = NULL;
	int32			*resubmit_flag = NULL;

	int32			fup_status = -1;
	int32			ntf_status = 0;
	int32			prov_status = 0;
	int32			cdr_duration = 0;
	int32			record_type = 0;
	int32			validation_flag = 0;
	int32			is_suspend = 0;
	int32			is_discard = 0;
	int32			is_error = 0;
	int32			duration = 0;
	int32			is_splitted = 0;
	int32			trans_status = -1;

	double			duration_seconds = 0;
	
	struct tm 		ltm = {0};
	struct tm 		daytm = {0};
	time_t			start_t = 0;
	time_t			end_t = 0;
	time_t			aso_start_t = 0;
	time_t			split_end_t = 0;
	time_t			bill_start_t = 0;
	time_t			bill_end_t = 0;
	time_t			new_session_start_t = 0;
	time_t			day_t = 0;
	
	void 			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"error before fm_mso_rate_aaa_rate_bb_event",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_BEF_RAT, ebufp);
	    	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_BEF_RAT_DESC, ebufp);
		is_suspend = 1;
		is_error = 1;
		goto CLEANUP;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event input Flist", i_flistp);
	
	cdr_duration = *(int32 *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DURATION_SECONDS, 0, ebufp);
	session_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AUTHORIZATION_ID, 0, ebufp);		
	record_type = *(int32 *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REQ_MODE, 0, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	prg_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	validation_flag = fm_mso_rate_aaa_record_validate(ctxp, session_id, record_type, cdr_duration, &val_out_flistp, ebufp);
	
	if (validation_flag == 1) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Duplicate Record");
		PIN_ERRBUF_CLEAR(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_DUP, ebufp);
	    	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_DUP_DESC, ebufp);
		is_suspend = 1;
		is_discard = 1;
		goto CLEANUP;
	}
	else if	(validation_flag == 2)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Higher Qty Already Rated");
		PIN_ERRBUF_CLEAR(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_HIGH_QTY, ebufp);
	    	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_HIGH_QTY_DESC, ebufp);
		is_suspend = 1;
		is_discard = 1;
		goto CLEANUP;
	}
	else if	(validation_flag == 3)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error in Validation");
		PIN_ERRBUF_CLEAR(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_VALIDATE, ebufp);
	   	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_VALIDATE_DESC, ebufp);
		is_suspend = 1;
		is_error = 1;
		goto CLEANUP;
	}
	
	aaa_input_flist = PIN_FLIST_CREATE(ebufp);
	fm_mso_rate_aaa_get_service(ctxp, i_flistp, &serv_ret_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp) || !serv_ret_flistp) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Service Not Found",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_NO_SERV, ebufp);
	    	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_NO_SERV_DESC, ebufp);
		is_suspend = 1;
		is_error = 1;
		goto CLEANUP;
	}

	time_str = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LOG_TIMESTAMP,0, ebufp);
	PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG, " Time string is ");
	PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG, time_str);
	strptime(time_str, "%a %b %d %H:%M:%S IST %Y", &ltm);
	end_t = mktime(&ltm);
	start_t = end_t - cdr_duration;
	qty = pbo_decimal_from_double(cdr_duration, ebufp);
	
	daytm.tm_mday = ltm.tm_mday;
	daytm.tm_mon = ltm.tm_mon;
	daytm.tm_year = ltm.tm_year;
	daytm.tm_hour = 0;
	daytm.tm_min = 0;
	daytm.tm_sec = 0;
	
	day_t = mktime(&daytm);
	
	gigaword = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_INPUT_GIGA_WORDS, 0, ebufp);
	input_bytes = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_BYTES_DOWNLINK, 0, ebufp);
	
	if (!pbo_decimal_is_zero(gigaword, ebufp)){
		bytes_downlink = pbo_decimal_multiply(four_gb, gigaword, ebufp);		
	}
	if( pbo_decimal_is_null(bytes_downlink, ebufp)) {
		bytes_downlink = pbo_decimal_copy(input_bytes, ebufp);
	}
	else {
		pbo_decimal_add_assign(bytes_downlink, input_bytes, ebufp);
	}
	gigaword = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_OUTPUT_GIGA_WORDS, 0, ebufp);
	input_bytes = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_BYTES_UPLINK, 0, ebufp);
	
	if (!pbo_decimal_is_zero(gigaword, ebufp)){
		bytes_uplink = pbo_decimal_multiply(four_gb,gigaword, ebufp);		
	}
	if(pbo_decimal_is_null(bytes_uplink, ebufp)) {
		bytes_uplink = pbo_decimal_copy(input_bytes, ebufp);
	}
	else {
		pbo_decimal_add_assign(bytes_uplink, input_bytes, ebufp);
	}

	if(!pbo_decimal_is_null(bytes_downlink, ebufp) && !pbo_decimal_is_zero(bytes_downlink, ebufp)){
		pbo_decimal_add_assign( total_bytes_used, bytes_downlink, ebufp );
	}
	if(!pbo_decimal_is_null(bytes_uplink, ebufp) && !pbo_decimal_is_zero(bytes_uplink, ebufp)){
		pbo_decimal_add_assign( total_bytes_used, bytes_uplink, ebufp );
	}
	
	trans_status = fm_mso_trans_open(ctxp, acct_pdp, 3, ebufp );
	if( trans_status != 0 || PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,"fm_mso_rate_aaa_rate_bb_event: Transaction open error", ebufp );
		trans_status = -1;
		PIN_ERRBUF_RESET(ebufp);
		is_suspend = 1;
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_TRANS, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_TRANS_DESC, ebufp);
		goto CLEANUP;
	}
	svc_flistp = PIN_FLIST_ELEM_GET(serv_ret_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	serv_pdp = PIN_FLIST_FLD_GET(svc_flistp, PIN_FLD_POID, 0, ebufp);
	bb_info_flistp = PIN_FLIST_SUBSTR_GET(svc_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	fup_status = *(int32 *)PIN_FLIST_FLD_GET(bb_info_flistp, MSO_FLD_FUP_STATUS, 0, ebufp);
	
	pp_ret_flistp = PIN_FLIST_CREATE(ebufp);

	fm_mso_rate_aaa_get_pp(ctxp, serv_pdp, &pp_ret_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Plan return flist", pp_ret_flistp);
	if(pp_ret_flistp)
	plan_pdp = PIN_FLIST_FLD_GET(pp_ret_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);

	vp = PIN_FLIST_FLD_GET(bb_info_flistp, MSO_FLD_NTF_STATUS, 1, ebufp);
	if(vp)
		ntf_status = *(int32 *)vp;
	telco_flistp = PIN_FLIST_SUBSTR_GET(svc_flistp, PIN_FLD_TELCO_FEATURES, 0, ebufp);
	prov_status = *(int32 *)PIN_FLIST_FLD_GET(telco_flistp, PIN_FLD_STATUS, 0, ebufp);
	
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, aaa_input_flist, PIN_FLD_MSID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, aaa_input_flist, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AUTHORIZATION_ID, aaa_input_flist, PIN_FLD_AUTHORIZATION_ID, ebufp);
	PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_POID, serv_pdp, ebufp);
	PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_OBJ_TYPE, bb_obj_type, ebufp);
	PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_START_T, &start_t, ebufp);
	PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_END_T, &end_t, ebufp);
	PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_QUANTITY, qty, ebufp);	
	PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_REQ_MODE, &record_type, ebufp);	
	PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_BYTES_DOWNLINK, bytes_downlink, ebufp);
	PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_BYTES_UPLINK, bytes_uplink, ebufp);
	PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	
	extended_input_flist = PIN_FLIST_SUBSTR_ADD(aaa_input_flist, PIN_FLD_EXTENDED_INFO, ebufp);
	bb_info_substr_flistp = PIN_FLIST_SUBSTR_ADD(extended_input_flist, PIN_FLD_SESSION_INFO, ebufp);

	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, bb_info_substr_flistp, PIN_FLD_ACCOUNT_NO, ebufp);	
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USER_NAME, bb_info_substr_flistp, PIN_FLD_USER_NAME, ebufp);	
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_FRAMED_IP_ADDRESS, bb_info_substr_flistp, MSO_FLD_FRAMED_IP_ADDRESS, ebufp);	
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_NE_IP_ADDRESS, bb_info_substr_flistp, MSO_FLD_NE_IP_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_NE_ID, bb_info_substr_flistp, MSO_FLD_NE_ID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DURATION_SECONDS, aaa_input_flist, PIN_FLD_DURATION_SECONDS, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_ID, bb_info_substr_flistp, PIN_FLD_SERVICE_ID, ebufp);	
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_CODE, bb_info_substr_flistp, PIN_FLD_SERVICE_CODE, ebufp);	
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_REMOTE_ID, bb_info_substr_flistp, MSO_FLD_REMOTE_ID, ebufp);	
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TERMINATE_CAUSE, aaa_input_flist, PIN_FLD_TERMINATE_CAUSE, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Original aaa_input_flist", aaa_input_flist);

	if (val_out_flistp)
	{
		vp = NULL;
		vp = PIN_FLIST_FLD_GET(val_out_flistp, PIN_FLD_FLAGS, 1, ebufp);
		if (vp)
			is_splitted = *(int32 *)vp;
		vp = NULL;
		vp = PIN_FLIST_FLD_GET(val_out_flistp, PIN_FLD_START_T, 0, ebufp);
		if(vp)
			aso_start_t = *(time_t *)vp;
	}
	
	if (is_splitted == 1)
	{
		PIN_ERR_LOG_MSG(3, "Splitted CDRs");
		rated_bytes_uplink = PIN_FLIST_FLD_GET(val_out_flistp, MSO_FLD_BYTES_UPLINK_BEF_FUP, 0, ebufp);
		rated_bytes_downlink = PIN_FLIST_FLD_GET(val_out_flistp, MSO_FLD_BYTES_DOWNLINK_BEF_FUP, 0, ebufp);
		if(!pbo_decimal_is_null(rated_bytes_uplink, ebufp)) {

			bytes_uplink = pbo_decimal_subtract(bytes_uplink, rated_bytes_uplink, ebufp);
		}
		if(!pbo_decimal_is_null(rated_bytes_downlink, ebufp)) {

			bytes_downlink = pbo_decimal_subtract(bytes_downlink, rated_bytes_downlink, ebufp);
		}
					
		duration = end_t - aso_start_t;
		qty = pbo_decimal_from_double(duration, ebufp);

		PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_START_T, &aso_start_t, ebufp);
		PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_QUANTITY, qty, ebufp);
		PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_BYTES_UPLINK, bytes_uplink, ebufp);
		PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_BYTES_DOWNLINK, bytes_downlink, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(3,"is_splitted else");
		//Active session present check for bill cycle time
		if((record_type != ACCT_START) && aso_start_t < day_t && fup_status != FUP_NA )
		{
			PIN_ERR_LOG_MSG(3,"inside diff day records");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			fm_mso_rate_aaa_get_charged_dates(ctxp, serv_pdp, &ret_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in getting charge dates",ebufp);
				PIN_ERRBUF_RESET(ebufp);
				*r_flistpp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_GET_DATE, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_GET_DATE_DESC, ebufp);
				is_suspend = 1;
				is_error = 1;
				goto CLEANUP;
			}
			if (ret_flistp)
			{
				vp = NULL;
				vp = PIN_FLIST_FLD_GET(ret_flistp,  PIN_FLD_CHARGED_FROM_T, 1, ebufp);
				if (vp)
					bill_start_t = *(time_t *)vp;
				vp = NULL;
				vp = PIN_FLIST_FLD_GET(ret_flistp,  PIN_FLD_CHARGED_TO_T, 1, ebufp);
				if (vp)
					bill_end_t = *(time_t *)vp;
			}
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event before condition", aaa_input_flist);
			if( bill_start_t != 0 && bill_end_t != 0 && aso_start_t != 0 && end_t != 0 && 
				(( aso_start_t > bill_start_t && aso_start_t < bill_end_t && end_t > bill_end_t) ||
				 ( aso_start_t < bill_start_t && aso_start_t < bill_end_t && end_t > bill_start_t)))
			{  
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event created flist ", val_out_flistp);

				//create flist for stop accounting as part of update cdr
				aaa_split_in_flistp = PIN_FLIST_COPY(aaa_input_flist,ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event after copy ", aaa_split_in_flistp);

				PIN_FLIST_FLD_COPY(val_out_flistp, PIN_FLD_BYTES_UPLINK, aaa_split_in_flistp, PIN_FLD_BYTES_UPLINK,ebufp);
				PIN_FLIST_FLD_COPY(val_out_flistp, PIN_FLD_BYTES_DOWNLINK,aaa_split_in_flistp, PIN_FLD_BYTES_DOWNLINK,ebufp);
				PIN_FLIST_FLD_COPY(val_out_flistp, PIN_FLD_START_T, aaa_split_in_flistp, PIN_FLD_START_T,ebufp);
				PIN_FLIST_FLD_COPY(val_out_flistp, PIN_FLD_DURATION_SECONDS, aaa_split_in_flistp, PIN_FLD_DURATION_SECONDS,ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event getting quantity", val_out_flistp);
				vp = NULL;
				vp = PIN_FLIST_FLD_GET(val_out_flistp, PIN_FLD_DURATION_SECONDS, 0, ebufp);
				if (vp)
				{
					duration_seconds=*(int32 *)vp;
					quantity = pbo_decimal_from_double(duration_seconds,ebufp);
					PIN_FLIST_FLD_SET(aaa_split_in_flistp, PIN_FLD_QUANTITY,(void *)quantity, ebufp);
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event setting quantity", val_out_flistp);

				//Calculate and set start and end time accordingly
				split_end_t = day_t - 1;
				PIN_FLIST_FLD_SET(aaa_split_in_flistp, PIN_FLD_END_T, (void *)&split_end_t, ebufp);	
				sprintf(new_session_id, "%s_1", session_id);
				PIN_FLIST_FLD_SET(aaa_split_in_flistp, PIN_FLD_AUTHORIZATION_ID, new_session_id, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event prepared split flist for AAA opcode ", aaa_split_in_flistp);
				PCM_OP(ctxp, PCM_OP_TCF_AAA_STOP_ACCOUNTING, 0, aaa_split_in_flistp, &aaa_split_out_flistp, ebufp);
				if(aaa_split_out_flistp)
				{
					rating_status = PIN_FLIST_FLD_GET(aaa_split_out_flistp, PIN_FLD_RATING_STATUS, 1, ebufp);
				}
				if(PIN_ERRBUF_IS_ERR(ebufp) || (rating_status && *rating_status != SUCCESS)) 
				{
					*r_flistpp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_SPLIT_RAT, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_SPLIT_RAT_DESC, ebufp);
					PIN_ERRBUF_CLEAR(ebufp);
					is_error = 1;
					PIN_FLIST_DESTROY_EX( &aaa_split_in_flistp, NULL);	
					PIN_FLIST_DESTROY_EX( &aaa_split_out_flistp, NULL);
					goto CLEANUP;
				}					
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event split return flist ", aaa_split_out_flistp);
				is_splitted = 1;
				PIN_ERR_LOG_MSG(3, "Before Update Active Session 1");
				new_session_start_t = day_t + 1;
				duration = end_t - new_session_start_t;
				qty = pbo_decimal_from_double(duration, ebufp);
				
				PIN_FLIST_FLD_SET(aaa_split_in_flistp, PIN_FLD_START_T, &new_session_start_t, ebufp);
				
				//update usage session for split cdr
				usage_sess_pdp = PIN_FLIST_FLD_GET(val_out_flistp, PIN_FLD_POID, 1, ebufp);	
				fm_mso_rate_aaa_update_usage_session(ctxp, aaa_split_in_flistp, usage_sess_pdp, record_type, 1, ebufp);		

								
				aso_bytes_uplink = PIN_FLIST_FLD_GET(val_out_flistp, PIN_FLD_BYTES_UPLINK, 0,ebufp);
				aso_bytes_dnlink = PIN_FLIST_FLD_GET(val_out_flistp, PIN_FLD_BYTES_DOWNLINK, 0,ebufp);
				
				if(!pbo_decimal_is_null(aso_bytes_uplink, ebufp))
						bytes_uplink = pbo_decimal_subtract(bytes_uplink, aso_bytes_uplink, ebufp);
				
				if(!pbo_decimal_is_null(aso_bytes_dnlink, ebufp)) 
						bytes_downlink = pbo_decimal_subtract(bytes_downlink, aso_bytes_dnlink, ebufp);
						
				PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_START_T, (void *)&new_session_start_t, ebufp);	
				PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_QUANTITY, qty, ebufp);	
				PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_BYTES_UPLINK, bytes_uplink, ebufp);
				PIN_FLIST_FLD_SET(aaa_input_flist, PIN_FLD_BYTES_DOWNLINK, bytes_downlink, ebufp);
				
				//reset bytes_uplink and bytes_downlink as current active session is rated
				PIN_FLIST_FLD_SET(val_out_flistp, PIN_FLD_BYTES_DOWNLINK, zero_qty, ebufp);
				PIN_FLIST_FLD_SET(val_out_flistp, PIN_FLD_BYTES_UPLINK, zero_qty, ebufp);

				PIN_FLIST_DESTROY_EX( &aaa_split_in_flistp, NULL);	
				PIN_FLIST_DESTROY_EX( &aaa_split_out_flistp, NULL);
			}
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		}
	}
	
	if (record_type == ACCT_STOP)
	{
		PCM_OP(ctxp, PCM_OP_TCF_AAA_STOP_ACCOUNTING, 0, aaa_input_flist, &aaa_output_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in Calling Rating Opcode",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_FAIL_RATING, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_FAIL_RATING_DESC, ebufp);
			is_suspend = 1;
			is_error = 1;
			goto CLEANUP;
		}
		if(aaa_output_flistp)
			rating_status = PIN_FLIST_FLD_GET(aaa_output_flistp, PIN_FLD_RATING_STATUS, 1, ebufp);
			
		if(rating_status && *rating_status != SUCCESS)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in Calling Rating Opcode",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);	
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_FAIL_RATING, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_FAIL_RATING_DESC, ebufp);
			PIN_ERRBUF_RESET(ebufp);
			is_error = 1;
			is_suspend = 1;
			goto CLEANUP;
		}
	}

	
	//create or update usage session
	vp = NULL;
	if (val_out_flistp)
	{
		PIN_ERR_LOG_MSG(3,"Inside val_out_flistp");
		PIN_ERR_LOG_FLIST(3, "Val Out Flistp",val_out_flistp);
		vp = PIN_FLIST_FLD_GET(val_out_flistp, PIN_FLD_POID, 1, ebufp);		
		if (vp)
		{
			PIN_ERR_LOG_MSG(3,"Inside update usage session");
			usage_sess_pdp = (poid_t *)vp;
			fm_mso_rate_aaa_update_usage_session(ctxp, aaa_input_flist, usage_sess_pdp, record_type, 0, ebufp);
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(3,"Inside create usage session");
		fm_mso_rate_aaa_create_usage_session(ctxp, aaa_input_flist, record_type, plan_pdp, ebufp);
	}
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Creating/Updating Usage Session",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_USG_SESS, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_USG_SESS_DESC, ebufp);
		is_suspend = 1;
		is_error = 1;
		goto CLEANUP;
	}

	//update daily usage
	fm_mso_rate_aaa_update_daily_usage(ctxp, val_out_flistp, day_t, aaa_input_flist, plan_pdp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Updating Daily Usage",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_DAILY_USG, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_DAILY_USG_DESC, ebufp);
		is_suspend = 1;
		is_error = 1;
		goto CLEANUP;
	}
	if (fup_status != LIMIT_EXCEED &&  prov_status == PROV_ACTIVE)
	{
		fm_mso_rate_aaa_threshold_check(ctxp, svc_flistp, acct_pdp, serv_pdp, ntf_status, fup_status, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in Threshold Check",ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure_status, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_ERR_THRESHOLD, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_ERR_THRESHOLD_DESC, ebufp);
			is_suspend = 1;
			is_error = 1;
			goto CLEANUP;
		}
	}
	
	if (record_type != ACCT_STOP)
	{
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_AGREEMENT_NO, *r_flistpp, MSO_FLD_AGREEMENT_NO, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &success_status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, RATING_SUCCESS, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, RATING_SUCCESS_DESC, ebufp);
	}
	else if (aaa_output_flistp)
	{
		*r_flistpp = PIN_FLIST_COPY(aaa_output_flistp, ebufp);
	}
	
	
	CLEANUP:
	PIN_ERRBUF_CLEAR(ebufp);
	if( trans_status == 0 ){
		if( is_error == 0 ){
			PIN_ERRBUF_CLEAR(ebufp);
			fm_mso_trans_commit( ctxp, acct_pdp, ebufp );
		}
		else{
			PIN_ERRBUF_CLEAR(ebufp);
			fm_mso_trans_abort( ctxp, acct_pdp, ebufp );
		}
	}
	
	if( is_suspend == 1 && prg_name && !strstr(prg_name, "mso_resubmit_susp_cdr") && 
	(resubmit_flag == NULL || (*(int32*)resubmit_flag != 1))){
		acct_pdp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		fm_mso_rate_aaa_suspend_event(ctxp, i_flistp, *r_flistpp, acct_pdp, ebufp);
	}
	
	PIN_FLIST_DESTROY_EX(&aaa_input_flist, NULL);
	PIN_FLIST_DESTROY_EX(&aaa_output_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&serv_ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&val_out_flistp, NULL);
	
	if (!pbo_decimal_is_null(total_bytes_used, ebufp))
		pbo_decimal_destroy(&total_bytes_used);
	if (!pbo_decimal_is_null(bytes_uplink, ebufp))
		pbo_decimal_destroy(&bytes_uplink);
	if (!pbo_decimal_is_null(bytes_downlink, ebufp))
		pbo_decimal_destroy(&bytes_downlink);
	if (!pbo_decimal_is_null(qty, ebufp))
		pbo_decimal_destroy(&qty);
	if (!pbo_decimal_is_null(four_gb, ebufp))
		pbo_decimal_destroy(&four_gb);
	if (!pbo_decimal_is_null(zero_qty, ebufp))
		pbo_decimal_destroy(&zero_qty);

	return;
}

//Return Values - 0 : Success, 1 : Duplicate Record, 2 : Higher Usage Already Rated, 3 : Error in validation
static int32 
fm_mso_rate_aaa_record_validate(
	pcm_context_t		*ctxp,
	char				*session_id,
	int32				record_type,
	int32				cdr_duration,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*srch_input_flist = NULL;
	pin_flist_t			*srch_result_flist = NULL;
	pin_flist_t			*arg_flistp = NULL;
	pin_flist_t			*result_flistp = NULL;
	poid_t				*srch_pdp = NULL;

	char				*usg_srch_templatep = "select X from /mso_usage_session where F1 = V1 ";
	int32				srch_flag = SRCH_DISTINCT;
	int32				aso_duration = 0;
	int32				validation_flag = 0;	
	int32				usage_session_status = 0;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_rate_aaa_record_validate : Error Before Validation");
		validation_flag = 3;
		goto CLEANUP;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	
	srch_input_flist = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(1, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_input_flist, PIN_FLD_POID, (void *)srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_input_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_input_flist, PIN_FLD_TEMPLATE, usg_srch_templatep, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_input_flist, PIN_FLD_ARGS, 1, ebufp);

	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACTIVE_SESSION_ID, session_id, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_input_flist, PIN_FLD_RESULTS, 0, ebufp);

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_record_validate usage session search input Flist", srch_input_flist);

	PCM_OP( ctxp, PCM_OP_SEARCH, 0, srch_input_flist, &srch_result_flist, ebufp);

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_record_validate usage session search output Flist", srch_result_flist);

	PIN_FLIST_DESTROY_EX( &srch_input_flist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"fm_mso_rate_aaa_record_validate: "
			"Error on getting session value from /mso_usage_session class", ebufp );
		*r_flistpp = NULL;
		validation_flag = 3;
		goto CLEANUP;
	}
	result_flistp = PIN_FLIST_ELEM_GET(srch_result_flist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	
	if (result_flistp)
	{
		usage_session_status = *(int32 *)PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_STATUS, 0, ebufp);
		if ((usage_session_status == ACCT_STOP) || (record_type == ACCT_START && usage_session_status == ACCT_INTERIM))
		{
			validation_flag = 1;
			goto CLEANUP;
		}			
		aso_duration = *(int32 *)PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_DURATION_SECONDS, 0, ebufp);
		if ((record_type == ACCT_STOP && aso_duration > cdr_duration) || 
			(record_type != ACCT_STOP && aso_duration >= cdr_duration))
		{
			validation_flag = 2;
			goto CLEANUP;
		}
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_DURATION_SECONDS, *r_flistpp, PIN_FLD_DURATION_SECONDS, ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_BYTES_DOWNLINK, *r_flistpp, PIN_FLD_BYTES_DOWNLINK, ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_BYTES_UPLINK, *r_flistpp, PIN_FLD_BYTES_UPLINK, ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_FLAGS, *r_flistpp, PIN_FLD_FLAGS, ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, MSO_FLD_BYTES_DOWNLINK_BEF_FUP, *r_flistpp, MSO_FLD_BYTES_DOWNLINK_BEF_FUP, ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, MSO_FLD_BYTES_UPLINK_BEF_FUP, *r_flistpp, MSO_FLD_BYTES_UPLINK_BEF_FUP, ebufp);
		PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_START_T, *r_flistpp, PIN_FLD_START_T, ebufp);
		PIN_ERR_LOG_FLIST(3, "Validation output flist", *r_flistpp);
	}
		
	CLEANUP:
	PIN_FLIST_DESTROY_EX( &srch_result_flist, NULL);
	return validation_flag;
}

void 
fm_mso_rate_aaa_get_service(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	int32 			srch_flag = 0;
	poid_t			*srch_pdp = NULL;
	char 			template[256] = "select X from /service/telco/broadband where F1 = V1 ";
	char			*aggr_num = NULL;
	

	if( PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_rate_aaa_get_service");
		*r_flistpp = NULL;
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	aggr_num = PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_AGREEMENT_NO, 1, ebufp);
	
	srch_pdp = PIN_POID_CREATE(1, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	bb_info_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_AGREEMENT_NO, aggr_num, ebufp);
	result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search input flist : ", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search output flist : ", srch_out_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) <= 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in getting service details");
		*r_flistpp = NULL;
		return;
	}
	
	*r_flistpp = PIN_FLIST_COPY (srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

static void 
fm_mso_rate_aaa_create_usage_session( 
	pcm_context_t   	*ctxp,
	pin_decimal_t		*in_flistp,
	int32			record_type,
	poid_t          	*plan_pdp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*usage_inp_flistp = NULL;
	pin_flist_t		*usage_ret_flistp = NULL;
	pin_flist_t		*extended_info_flist = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	
	int64			db = 1;
	poid_t			*usage_pdp = NULL;
	
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_rate_aaa_create_usage_session");
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	usage_inp_flistp = PIN_FLIST_CREATE(ebufp);
	usage_pdp = PIN_POID_CREATE(db, "/mso_usage_session", (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(usage_inp_flistp, PIN_FLD_POID, usage_pdp, ebufp);
	PIN_FLIST_FLD_SET(usage_inp_flistp, PIN_FLD_STATUS, &record_type, ebufp);
	PIN_FLIST_FLD_SET(usage_inp_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_AUTHORIZATION_ID, usage_inp_flistp, PIN_FLD_ACTIVE_SESSION_ID, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, usage_inp_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, usage_inp_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BYTES_DOWNLINK, usage_inp_flistp, PIN_FLD_BYTES_DOWNLINK, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BYTES_UPLINK, usage_inp_flistp, PIN_FLD_BYTES_UPLINK, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_START_T, usage_inp_flistp, PIN_FLD_START_T, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_END_T, usage_inp_flistp, PIN_FLD_END_T, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_NO, usage_inp_flistp, PIN_FLD_MSID, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_QUANTITY, usage_inp_flistp, PIN_FLD_QUANTITY, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DURATION_SECONDS, usage_inp_flistp, PIN_FLD_DURATION_SECONDS, ebufp);
	
	
	extended_info_flist = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_EXTENDED_INFO, 0, ebufp);
	bb_info_flistp = PIN_FLIST_SUBSTR_GET(extended_info_flist, PIN_FLD_SESSION_INFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(bb_info_flistp, PIN_FLD_USER_NAME, usage_inp_flistp, PIN_FLD_USER_NAME, ebufp);
	PIN_FLIST_FLD_COPY(bb_info_flistp, MSO_FLD_FRAMED_IP_ADDRESS, usage_inp_flistp, MSO_FLD_FRAMED_IP_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(bb_info_flistp, MSO_FLD_NE_IP_ADDRESS, usage_inp_flistp, MSO_FLD_NE_IP_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(bb_info_flistp, MSO_FLD_NE_ID, usage_inp_flistp, MSO_FLD_NE_ID, ebufp);
	PIN_FLIST_FLD_COPY(bb_info_flistp, PIN_FLD_SERVICE_CODE, usage_inp_flistp, PIN_FLD_SERVICE_CODE, ebufp);
	PIN_FLIST_FLD_COPY(bb_info_flistp, MSO_FLD_REMOTE_ID, usage_inp_flistp, MSO_FLD_REMOTE_ID, ebufp);
	PIN_FLIST_FLD_COPY(bb_info_flistp, PIN_FLD_TERMINATE_CAUSE, usage_inp_flistp, PIN_FLD_TERMINATE_CAUSE, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event input flist for creating usage session", usage_inp_flistp);
   	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, usage_inp_flistp, &usage_ret_flistp, ebufp);
    	PIN_FLIST_DESTROY_EX(&usage_inp_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_rate_aaa_rate_bb_event err calling PCM_OP_CREATE_OBJ Opcode. error:",ebufp);
	    goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event after creating usage session", usage_ret_flistp);
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&usage_ret_flistp, NULL);
	return;
}

static void
fm_mso_rate_aaa_update_usage_session(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	poid_t			*usage_sess_pdp,
	int32			record_type,
	int32			trans_flags,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*upd_iflistp = NULL;
	pin_flist_t		*upd_oflistp = NULL;

	if( PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_rate_aaa_update_usage_session");
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	upd_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET( upd_iflistp, PIN_FLD_POID, (void *)usage_sess_pdp, ebufp );
	PIN_FLIST_FLD_SET( upd_iflistp, PIN_FLD_STATUS, &record_type, ebufp );
	
	if (trans_flags == 0) 
	{
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DURATION_SECONDS, upd_iflistp, PIN_FLD_DURATION_SECONDS, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BYTES_DOWNLINK, upd_iflistp, PIN_FLD_BYTES_DOWNLINK, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BYTES_UPLINK, upd_iflistp, PIN_FLD_BYTES_UPLINK, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_QUANTITY, upd_iflistp, PIN_FLD_QUANTITY, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_END_T, upd_iflistp, PIN_FLD_END_T, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BYTES_DOWNLINK, upd_iflistp, MSO_FLD_BYTES_DOWNLINK_BEF_FUP, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_BYTES_UPLINK, upd_iflistp, MSO_FLD_BYTES_UPLINK_BEF_FUP, ebufp);
		PIN_FLIST_FLD_SET(upd_iflistp, PIN_FLD_FLAGS, &trans_flags, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_START_T, upd_iflistp, PIN_FLD_START_T, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DURATION_SECONDS, upd_iflistp, PIN_FLD_DURATION_SECONDS, ebufp);
	}

	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "update_usage_session: update input flist", upd_iflistp );
	PCM_OP( ctxp, PCM_OP_WRITE_FLDS, 0, upd_iflistp, &upd_oflistp, ebufp );
	PIN_FLIST_DESTROY_EX( &upd_iflistp, NULL );
	if( PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "update_usage_session: update session error", ebufp );
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "update_usage_session: update output flist", upd_oflistp );

	CLEANUP:
	PIN_FLIST_DESTROY_EX( &upd_oflistp, NULL );
	return;
}

void 
fm_mso_rate_aaa_update_daily_usage(
	pcm_context_t		*ctxp,
	pin_flist_t		*active_session_flistp,
	time_t			day_t,
	pin_flist_t		*aaa_in_flistp,
	poid_t          	*plan_pdp,
	pin_errbuf_t		*ebufp)
{
	int64			db = 1;
	pin_flist_t		*du_srch_input_flist = NULL;
	pin_flist_t		*du_srch_result_flist = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*du_flistp = NULL;
	pin_flist_t		*bb_daily_usage_flistp = NULL;
	pin_flist_t		*bb_daily_usage_o_flistp = NULL;

	poid_t			*du_pdp = NULL;
	poid_t			*bb_daily_usage_pdp = NULL;

	pin_decimal_t		*bytes_uplink = NULL;
	pin_decimal_t		*bytes_dnlink = NULL;
	pin_decimal_t		*aso_bytes_uplink = NULL;
	pin_decimal_t		*aso_bytes_dnlink = NULL;
	pin_decimal_t		*delta_dnlink = NULL;
	pin_decimal_t		*delta_uplink = NULL;
	pin_decimal_t		*delta_total = NULL;
	pin_decimal_t		*du_bytes_uplink = NULL;
	pin_decimal_t		*du_bytes_downlink = NULL;
	pin_decimal_t		*bytes_uplink_daily = NULL;
	pin_decimal_t		*bytes_downlink_daily = NULL;
	pin_decimal_t		*total_bytes_daily = NULL;

	int32			*duration = NULL;
	int32			*aso_duration = NULL;
	
	int32			delta_duration = 0;
	int32			ret_val = 0;
	int32			search_flags = SRCH_DISTINCT;
	int32			du_daily_duration = 0;

	char			*daily_usage_srch_template = "select X from /mso_bb_daily_usage where F1 = V1 and F2 = V2 ";
	char			dur_str[10];

	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Entered with ebuf set");
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "AAA input Flist is ",aaa_in_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Active Session Flist is ",active_session_flistp);

	duration = PIN_FLIST_FLD_GET(aaa_in_flistp, PIN_FLD_DURATION_SECONDS, 0, ebufp);
	bytes_uplink = PIN_FLIST_FLD_GET(aaa_in_flistp, PIN_FLD_BYTES_UPLINK, 0, ebufp);
	bytes_dnlink = PIN_FLIST_FLD_GET(aaa_in_flistp, PIN_FLD_BYTES_DOWNLINK, 0, ebufp);
	
	if (active_session_flistp)
	{
		PIN_ERR_LOG_MSG(3, "Inside Active Session");
		aso_duration = PIN_FLIST_FLD_GET(active_session_flistp, PIN_FLD_DURATION_SECONDS, 1, ebufp);
		aso_bytes_uplink = PIN_FLIST_FLD_GET(active_session_flistp, PIN_FLD_BYTES_UPLINK, 1, ebufp);
		aso_bytes_dnlink = PIN_FLIST_FLD_GET(active_session_flistp, PIN_FLD_BYTES_DOWNLINK, 1, ebufp);
		
	}
	if(!pbo_decimal_is_null(aso_bytes_uplink, ebufp))
		delta_uplink = pbo_decimal_subtract(bytes_uplink, aso_bytes_uplink, ebufp);
	else if(!pbo_decimal_is_null(bytes_uplink, ebufp))
		delta_uplink = pbo_decimal_copy(bytes_uplink, ebufp);
		
	if(!pbo_decimal_is_null(aso_bytes_dnlink, ebufp))
		delta_dnlink = pbo_decimal_subtract(bytes_dnlink, aso_bytes_dnlink, ebufp);
	else if(!pbo_decimal_is_null(bytes_dnlink, ebufp))
		delta_dnlink = pbo_decimal_copy(bytes_dnlink, ebufp);
		
	if (aso_duration && duration)
		delta_duration = *duration - *aso_duration;
	else if (duration)
		delta_duration = *duration;
		
	/* Search if a daily usage object exists for this date */
	du_srch_input_flist = PIN_FLIST_CREATE(ebufp);
	du_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(du_srch_input_flist, PIN_FLD_POID, du_pdp, ebufp);
	PIN_FLIST_FLD_SET(du_srch_input_flist, PIN_FLD_FLAGS, &search_flags, ebufp);
	PIN_FLIST_FLD_SET(du_srch_input_flist, PIN_FLD_TEMPLATE, daily_usage_srch_template, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(du_srch_input_flist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_EFFECTIVE_T, &day_t, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(du_srch_input_flist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_COPY(aaa_in_flistp, PIN_FLD_ACCOUNT_OBJ, arg_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_ELEM_ADD(du_srch_input_flist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_update_daily_usage: search input", du_srch_input_flist);
	PCM_OP(ctxp, PCM_OP_SEARCH, search_flags, du_srch_input_flist, &du_srch_result_flist, ebufp);
	PIN_FLIST_DESTROY_EX(&du_srch_input_flist, NULL);		
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		goto CLEANUP;
	}
	ret_val  = PIN_FLIST_ELEM_COUNT(du_srch_result_flist, PIN_FLD_RESULTS, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_update_daily_usage: search output ", du_srch_result_flist);
	if(ret_val) 
	{
		/* Found. Update it with delta values */
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_update_daily_usage: Updating the daily usage object");

		du_flistp = PIN_FLIST_ELEM_GET(du_srch_result_flist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
		bb_daily_usage_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(du_flistp, PIN_FLD_POID, bb_daily_usage_flistp, PIN_FLD_POID, ebufp);

		du_bytes_uplink = PIN_FLIST_FLD_GET(du_flistp, PIN_FLD_BYTES_UPLINK, 0, ebufp);
		du_bytes_downlink = PIN_FLIST_FLD_GET(du_flistp, PIN_FLD_BYTES_DOWNLINK, 0, ebufp);
		
		du_daily_duration = atoi(PIN_FLIST_FLD_GET(du_flistp, PIN_FLD_DURATION, 0, ebufp));
		du_daily_duration = du_daily_duration + delta_duration;

		/*********************************************************************************
		 * Limit daily seconds to 86400 due to sessoin start and ends between two days
		 *********************************************************************************/
		if (du_daily_duration > 86400)
		{
			du_daily_duration = 86400; 
		}

		sprintf(dur_str, "%d", du_daily_duration);
		
		bytes_uplink_daily = pbo_decimal_add(du_bytes_uplink, delta_uplink, ebufp);
		bytes_downlink_daily = pbo_decimal_add(du_bytes_downlink, delta_dnlink, ebufp);

		total_bytes_daily = pbo_decimal_add(bytes_uplink_daily, bytes_downlink_daily, ebufp);

		PIN_FLIST_FLD_SET(bb_daily_usage_flistp, PIN_FLD_BYTES_DOWNLINK, bytes_downlink_daily, ebufp);
		PIN_FLIST_FLD_SET(bb_daily_usage_flistp, PIN_FLD_BYTES_UPLINK, bytes_uplink_daily, ebufp);
		PIN_FLIST_FLD_SET(bb_daily_usage_flistp, MSO_FLD_TOTAL_DATA, total_bytes_daily, ebufp);
		PIN_FLIST_FLD_SET(bb_daily_usage_flistp, PIN_FLD_DURATION, dur_str, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event: Daily Usage Object Update input ", bb_daily_usage_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, bb_daily_usage_flistp, &bb_daily_usage_o_flistp, ebufp);	
		PIN_FLIST_DESTROY_EX(&bb_daily_usage_flistp, NULL);		
	} 
	else  
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event: Creating new daily usage object");
		bb_daily_usage_flistp = PIN_FLIST_CREATE(ebufp);
		bb_daily_usage_pdp = PIN_POID_CREATE(db, "/mso_bb_daily_usage", (int64)-1, ebufp);
		PIN_FLIST_FLD_PUT(bb_daily_usage_flistp, PIN_FLD_POID, bb_daily_usage_pdp, ebufp);
		PIN_FLIST_FLD_COPY(aaa_in_flistp, PIN_FLD_ACCOUNT_OBJ, bb_daily_usage_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_SET(bb_daily_usage_flistp, PIN_FLD_EFFECTIVE_T, &day_t, ebufp);			
		PIN_FLIST_FLD_SET(bb_daily_usage_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		
		sprintf(dur_str, "%d", delta_duration);
		delta_total = pbo_decimal_add(delta_dnlink, delta_uplink, ebufp);

		PIN_FLIST_FLD_SET(bb_daily_usage_flistp, PIN_FLD_BYTES_DOWNLINK, delta_dnlink, ebufp);
		PIN_FLIST_FLD_SET(bb_daily_usage_flistp, PIN_FLD_BYTES_UPLINK, delta_uplink, ebufp);
		PIN_FLIST_FLD_SET(bb_daily_usage_flistp, MSO_FLD_TOTAL_DATA, delta_total, ebufp);
		PIN_FLIST_FLD_SET(bb_daily_usage_flistp, PIN_FLD_DURATION, dur_str, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event: Daily Usage Object Create input ", bb_daily_usage_flistp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, bb_daily_usage_flistp, &bb_daily_usage_o_flistp, ebufp);			
		PIN_FLIST_DESTROY_EX(&bb_daily_usage_flistp, NULL);
	}

	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&du_srch_result_flist, NULL);
	PIN_FLIST_DESTROY_EX(&bb_daily_usage_o_flistp, NULL);

	if(delta_dnlink && !pbo_decimal_is_null(delta_dnlink, ebufp))
		pbo_decimal_destroy(&delta_dnlink);	
	if(delta_uplink && !pbo_decimal_is_null(delta_uplink, ebufp))
		pbo_decimal_destroy(&delta_uplink);	
	if(delta_total && !pbo_decimal_is_null(delta_total, ebufp))
		pbo_decimal_destroy(&delta_total);	
	if(bytes_uplink_daily && !pbo_decimal_is_null(bytes_uplink_daily, ebufp))
		pbo_decimal_destroy(&bytes_uplink_daily);	
	if(bytes_downlink_daily && !pbo_decimal_is_null(bytes_downlink_daily, ebufp))
		pbo_decimal_destroy(&bytes_downlink_daily);	
	if(total_bytes_daily && !pbo_decimal_is_null(total_bytes_daily, ebufp))
		pbo_decimal_destroy(&total_bytes_daily);
	return;
}

int32 
fm_mso_rate_aaa_get_charged_dates(
	pcm_context_t  	*ctxp,
	poid_t         	*svc_pdp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t   	*ebufp)
{
	pin_flist_t     *srch_iflistp = NULL;
	pin_flist_t     *srch_oflistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *cycle_flistp = NULL;
	poid_t          *srch_pdp = NULL;
	int64   	db = 1;
	char            *template = "select X from /purchased_product 1, /rate_plan 2  where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = 2.F4 and 2.F5 like V5 ";
	int32           status = 1; // active
	int32           srch_flags = 0;
	time_t        	*charge_to_t = NULL;
	time_t         	*charge_from_t = NULL;
	char           	*evt_type = "%grant%";

	if( PIN_ERRBUF_IS_ERR(ebufp)){
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EVENT_TYPE, evt_type, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_charged_dates: search input flist : ", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_charged_dates: search output flist : ", srch_oflistp);

	results_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(results_flistp) {
		cycle_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);
		if (cycle_flistp)
		{
			charge_to_t = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
			charge_from_t = PIN_FLIST_FLD_GET(cycle_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
			if (charge_to_t)
			{
				PIN_FLIST_FLD_COPY( cycle_flistp, PIN_FLD_CHARGED_TO_T, *ret_flistpp, PIN_FLD_CHARGED_TO_T, ebufp );
			} 
			if (charge_from_t)
			{
				PIN_FLIST_FLD_COPY( cycle_flistp, PIN_FLD_CHARGED_FROM_T, *ret_flistpp, PIN_FLD_CHARGED_FROM_T, ebufp );
			} 
		}
	}
	
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

void 
fm_mso_rate_aaa_suspend_event( 
	pcm_context_t		*ctxp,
	pin_flist_t     	*i_flistp,
	pin_flist_t     	*r_flistp,
	poid_t			*acct_pdp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*susp_inp_flistp = NULL;
	pin_flist_t		*susp_ret_flistp = NULL;
	pin_buf_t       	*nota_buf     = NULL;

	poid_t			*susp_evt_pdp = NULL;
	pcm_context_t		*new_ctxp = NULL;
	
	char            	*time_str = NULL;
	//char			*susp_event_type = "/mso_aaa_suspended_bb_event";
	char            	*flistbuf = NULL;

	pin_decimal_t   	*four_gb = NULL;
	pin_decimal_t   	*bytes_downlink = NULL;
	pin_decimal_t   	*bytes_uplink = NULL;
	pin_decimal_t   	*gigaword = NULL;
	pin_decimal_t   	*input_bytes = NULL;
	
	time_t         	 	start_t = 0;
	time_t          	end_t = 0;
	struct tm       	ltm = {0};
	
	int32           	session_time = 0;
	int32			status = 0; //1- Ready for Resubmit, 0- New or Resubmited
	int64			db = 1;
	int32			trans_id = -1;
    	int32             	flistlen=0;

	char			*error_code = NULL;
	char			*error_desc = NULL;
	PIN_ERRBUF_CLEAR(ebufp);

	//PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
	//trans_id = fm_mso_trans_open(new_ctxp, acct_pdp, 1, ebufp);
	
	/*if(PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_FLIST_LOG_ERR("suspend_event: pcm_context_open error", ebufp);
		goto CLEANUP;
	}*/
	
	error_code = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_ERROR_CODE, 1, ebufp);
	error_desc = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_ERROR_DESCR, 1, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Suspense Input", i_flistp);
	time_str = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LOG_TIMESTAMP,0, ebufp);
	strptime(time_str, "%a %b %d %H:%M:%S IST %Y", &ltm);
	end_t = mktime(&ltm);
	session_time = *(int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DURATION_SECONDS, 0, ebufp);
	start_t = end_t - session_time;
	four_gb = pbo_decimal_from_str("4294967296.0", ebufp);
    	gigaword = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_INPUT_GIGA_WORDS, 0, ebufp);
	input_bytes = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BYTES_DOWNLINK, 0, ebufp);
		
	if (!pbo_decimal_is_zero(gigaword, ebufp)){
		bytes_downlink = pbo_decimal_multiply(four_gb, gigaword, ebufp);
	}

	if(pbo_decimal_is_null(bytes_downlink, ebufp)) {
		bytes_downlink = pbo_decimal_copy(input_bytes, ebufp);
	} else {
		bytes_downlink = pbo_decimal_add(bytes_downlink, input_bytes, ebufp);
	}

	gigaword = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_OUTPUT_GIGA_WORDS, 0, ebufp);
	input_bytes = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BYTES_UPLINK, 0, ebufp);

	if (!pbo_decimal_is_zero(gigaword, ebufp)){
		bytes_uplink = pbo_decimal_multiply(four_gb, gigaword, ebufp);
	}
	if(pbo_decimal_is_null(bytes_uplink, ebufp)) {
		bytes_uplink = pbo_decimal_copy(input_bytes, ebufp);
	} else {
		bytes_uplink = pbo_decimal_add(bytes_uplink, input_bytes, ebufp);
	}	

	susp_inp_flistp = PIN_FLIST_CREATE(ebufp);
	susp_evt_pdp = PIN_POID_CREATE(db, "/mso_aaa_suspended_bb_event", (int64)-1, ebufp);
	PIN_FLIST_FLD_SET(susp_inp_flistp, PIN_FLD_POID, susp_evt_pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_AUTHORIZATION_ID, susp_inp_flistp,  PIN_FLD_AUTHORIZATION_ID,ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REQ_MODE, susp_inp_flistp, PIN_FLD_REQ_MODE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USER_NAME, susp_inp_flistp, PIN_FLD_USER_NAME, ebufp);
	PIN_FLIST_FLD_SET(susp_inp_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
	PIN_FLIST_FLD_SET(susp_inp_flistp, PIN_FLD_ERROR_DESCR, error_desc, ebufp);
	if(acct_pdp) {
	    PIN_FLIST_FLD_SET(susp_inp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	}
	PIN_FLIST_FLD_SET(susp_inp_flistp, PIN_FLD_START_T, &start_t, ebufp);
	PIN_FLIST_FLD_SET(susp_inp_flistp, PIN_FLD_END_T, &end_t, ebufp);
	PIN_FLIST_FLD_SET(susp_inp_flistp, PIN_FLD_BYTES_UPLINK, bytes_uplink, ebufp);
	PIN_FLIST_FLD_SET(susp_inp_flistp, PIN_FLD_BYTES_DOWNLINK, bytes_downlink, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, 
					susp_inp_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DURATION_SECONDS, 
					susp_inp_flistp, PIN_FLD_DURATION_SECONDS, ebufp);
	PIN_FLIST_FLD_SET(susp_inp_flistp, PIN_FLD_STATUS, &status, ebufp);
	
	PIN_FLIST_TO_STR(i_flistp, &flistbuf, &flistlen, ebufp );
	nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ) );
	
	if(nota_buf == NULL)
	{
		pin_set_err( ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, 
							PIN_ERR_NO_MEM, 0, 0 ,0 );
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR, "couldn't allocate nemory for nota_buf", ebufp );
		goto CLEANUP;
	}

	nota_buf->flag = 0;
	nota_buf->size = flistlen - 2;
	nota_buf->offset = 0;
	nota_buf->data = (caddr_t)flistbuf;
	nota_buf->xbuf_file = ( char *) NULL;
	PIN_FLIST_FLD_PUT(susp_inp_flistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event input flist for creating suspense event", susp_inp_flistp);

 	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, susp_inp_flistp, &susp_ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&susp_inp_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_rate_aaa_rate_bb_event err calling MSO_OP_RATE_AAA_CREATE_SUSP_EVENT Opcode. error:",ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_rate_bb_event after creating suspense event", susp_ret_flistp);


		
	CLEANUP:
/*	if( trans_id == 0 ){
		if( PIN_ERRBUF_IS_ERR(ebufp)){
			fm_mso_trans_abort(new_ctxp, acct_pdp, ebufp);
		}
		else{
			fm_mso_trans_commit(new_ctxp, acct_pdp, ebufp);
		}
	        PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);	
	}	*/
	
	PIN_FLIST_DESTROY_EX(&susp_ret_flistp, NULL);

	if(!pbo_decimal_is_null(four_gb, ebufp))
		pbo_decimal_destroy(&four_gb);	
	if(!pbo_decimal_is_null(bytes_downlink, ebufp))
		pbo_decimal_destroy(&bytes_downlink);
	if(!pbo_decimal_is_null(bytes_uplink, ebufp))
		pbo_decimal_destroy(&bytes_uplink);		
		
	return;
}

static void 
fm_mso_rate_aaa_get_aso(
	pcm_context_t		*ctxp,
	poid_t			*serv_pdp,
	poid_t			*acct_pdp,
	time_t			valid_from,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_input_flist = NULL;
	pin_flist_t		*srch_result_flist = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	
	poid_t			*srch_pdp = NULL;

	char			*usg_srch_templatep = "select X from /mso_usage_session where F1 = V1 and F2 = V2 and F3 <> V3 and F4 >= V4 ";
	int32			srch_flag = SRCH_DISTINCT;
	int32			status = 2;
	int32			rec_id = 0;
	
	pin_cookie_t		cookie = NULL;
	
	pin_decimal_t		*aso_total_bytes = NULL;
	pin_decimal_t		*bytes_uplink = NULL;
	pin_decimal_t		*bytes_dnlink = NULL;
	pin_decimal_t		*rated_uplink = NULL;
	pin_decimal_t		*rated_dnlink = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_rate_aaa_get_aso : Error Before getting ASO");
		return;
	}
	
	srch_input_flist = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(1, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_input_flist, PIN_FLD_POID, (void *)srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_input_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_input_flist, PIN_FLD_TEMPLATE, usg_srch_templatep, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_input_flist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_input_flist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_input_flist, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_STATUS, &status, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_input_flist, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_START_T, &valid_from, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_input_flist, PIN_FLD_RESULTS, 0, ebufp);
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_aso search input Flist", srch_input_flist);

	PCM_OP( ctxp, PCM_OP_SEARCH, 0, srch_input_flist, &srch_result_flist, ebufp);

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_aso search output Flist", srch_result_flist);

	PIN_FLIST_DESTROY_EX( &srch_input_flist, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"fm_mso_rate_aaa_get_aso: Error on getting sessions", ebufp );
		goto CLEANUP;
	}
	aso_total_bytes = pbo_decimal_from_str("0.0", ebufp);
	while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_result_flist, PIN_FLD_RESULTS, 
			&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		bytes_uplink = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_BYTES_UPLINK, 1, ebufp);
		bytes_dnlink = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_BYTES_DOWNLINK, 1, ebufp);
		rated_uplink = PIN_FLIST_FLD_GET(result_flistp, MSO_FLD_BYTES_UPLINK_BEF_FUP, 1, ebufp);
		rated_dnlink = PIN_FLIST_FLD_GET(result_flistp, MSO_FLD_BYTES_DOWNLINK_BEF_FUP, 1, ebufp);

		if(!pbo_decimal_is_null(bytes_uplink, ebufp))
			pbo_decimal_add_assign(aso_total_bytes, bytes_uplink, ebufp);
		if(!pbo_decimal_is_null(bytes_dnlink, ebufp))
			pbo_decimal_add_assign(aso_total_bytes, bytes_dnlink, ebufp);
		if(!pbo_decimal_is_null(rated_uplink, ebufp))
			pbo_decimal_subtract_assign(aso_total_bytes, rated_uplink, ebufp);
		if(!pbo_decimal_is_null(rated_dnlink, ebufp))
			pbo_decimal_subtract_assign(aso_total_bytes, rated_dnlink, ebufp);
	}
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(*r_flistpp, MSO_FLD_TOTAL_DATA, aso_total_bytes, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_rate_aaa_get_aso output flist", *r_flistpp);
	
	CLEANUP:	
	PIN_FLIST_DESTROY_EX( &srch_result_flist, NULL);
	return;
}

void 
fm_mso_rate_aaa_threshold_check(
	pcm_context_t   	*ctxp,
	pin_flist_t		*svc_flistp,	
	poid_t			*acct_pdp,
	poid_t			*serv_pdp,
	int32			ntf_status,
	int32			fup_status,
	pin_errbuf_t  		*ebufp)
{
	pin_flist_t		*credit_profile_flistp = NULL;
	pin_flist_t		*credit_limit_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	
	pin_decimal_t 		*credit_floor = NULL;
	pin_decimal_t 		*credit_limit = NULL;
	pin_decimal_t 		*credit_range = NULL;

	pin_decimal_t		*zero = pin_decimal( "0.0", ebufp );
	pin_decimal_t		*hundred = pin_decimal( "100.00", ebufp );
	pin_decimal_t 		*unlimited_quotoa = pbo_decimal_from_str("1999999998", ebufp);
	pin_decimal_t		*available_balance = NULL;
	pin_decimal_t		*percent_used = NULL;	
	pin_decimal_t		*threhold_percentage = NULL;
	
	int32			i = 0;
	int32			j = 0;
	int32			res_id = 0;
	int32			ntf_flag = 0;
	int32			reset_flag = 0;
	int32			ntf_type = -1;
	int32			ntf_value = -1;
	
	double			threshold_double = 0;
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(1, "Error before fm_mso_rate_aaa_threshold_check", ebufp);
		goto CLEANUP;
	}
	//get credit profile
	fm_mso_rate_aaa_get_credit_profile(ctxp, acct_pdp, svc_flistp, &credit_profile_flistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(1, "Error in getting Credit Profile", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "credit_profile_flistp", credit_profile_flistp);
	if(credit_profile_flistp)
	{
		credit_limit_flistp = PIN_FLIST_ELEM_GET(credit_profile_flistp, MSO_FLD_CREDIT_PROFILE, PIN_ELEMID_ANY, 1, ebufp);
		PIN_ERR_LOG_FLIST(3, "credit_limit_flistp", credit_limit_flistp);
		if(credit_limit_flistp) 
		{
			credit_floor = PIN_FLIST_FLD_GET(credit_limit_flistp, PIN_FLD_CREDIT_FLOOR, 1, ebufp);
			credit_limit = PIN_FLIST_FLD_GET(credit_limit_flistp, PIN_FLD_CREDIT_LIMIT, 1, ebufp);

			if((!pbo_decimal_is_null(credit_floor, ebufp)) && (!pbo_decimal_is_null(credit_limit, ebufp))) 
			{
				credit_range = pbo_decimal_subtract(credit_limit, credit_floor, ebufp);	

				if (!pbo_decimal_is_null(credit_range, ebufp) && !pbo_decimal_is_zero(credit_range, ebufp) && pbo_decimal_compare(credit_range, unlimited_quotoa, ebufp) < 0)
				{
					res_id = *(int32 *)PIN_FLIST_FLD_GET(credit_limit_flistp, PIN_FLD_RESOURCE_ID, 0, ebufp);
					fm_mso_rate_aaa_get_balances(ctxp, acct_pdp, serv_pdp, &bal_flistp, res_id, ebufp);
					if(PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(1, "Error in getting Balance", ebufp);
						goto CLEANUP;
					}
					if(bal_flistp)
					{
						available_balance = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CURRENT_BAL, 1, ebufp);
						if (!pbo_decimal_is_null(available_balance, ebufp) &&
							pbo_decimal_compare(available_balance, zero, ebufp) >= 0)
						{
							if(res_id == FUP_RESOURCE && fup_status == 1) 
							{
								PIN_ERR_LOG_MSG(3, "Fup Resource");
								if(fm_mso_rate_aaa_throttle_product_chk(ctxp, acct_pdp, serv_pdp, ebufp))
        							{
                							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Customer not eligible for throtelling ");
									goto CLEANUP; 
        							}
								fm_mso_rate_aaa_throttle_customer(ctxp, acct_pdp, serv_pdp, res_id, ebufp);
							}		
							else if(res_id == LIMITED_RESOURCE) 
							{
								PIN_ERR_LOG_MSG(3, "Limited Resource");
								fm_mso_rate_aaa_deactivate_customer(ctxp, acct_pdp, serv_pdp, ebufp);
							}
							if(PIN_ERRBUF_IS_ERR(ebufp))
							{
								PIN_ERR_LOG_EBUF(1, "Error in Deactivating/Throttling Customer", ebufp);
								goto CLEANUP;
							}
							ntf_value = NTF_THROT_DEACT;
							ntf_flag = 1;
						}
						else if (!pbo_decimal_is_null(available_balance, ebufp) &&
							pbo_decimal_compare(available_balance, zero, ebufp) < 0 && res_id == FUP_RESOURCE && fup_status == 1 
							&& fm_mso_rate_aaa_res_threshold_validation(res_id) && ntf_status != NTF_THROT_DEACT)
						{
							if(fm_mso_rate_aaa_throttle_product_chk(ctxp, acct_pdp, serv_pdp, ebufp))
							{
								PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Customer's SMS notification need to skip");
                                                                        goto CLEANUP;
							}
							pbo_decimal_negate_assign(available_balance, ebufp);
							percent_used = pbo_decimal_subtract(credit_range, available_balance, ebufp);						
							pbo_decimal_divide_assign(percent_used, credit_range, ebufp);
							pbo_decimal_multiply_assign(percent_used, hundred, ebufp);
							for (i = 0; i < num_of_resources_configured ; i++)
							{
								if (credit_thresholds[i].resource_id == res_id)
								{
									for(j = 0; j < credit_thresholds[i].num_of_config; j++) 
									{								
										threshold_double = pbo_decimal_to_double(credit_thresholds[i].threshold_config[j], ebufp);
										ntf_value = (int32)threshold_double;
										if (!pbo_decimal_is_null(percent_used, ebufp) &&
											!pbo_decimal_is_null(credit_thresholds[i].threshold_config[j], ebufp) && 
											pbo_decimal_compare(percent_used,
											credit_thresholds[i].threshold_config[j], ebufp) >= 0) 
										{
											if (ntf_status < ntf_value)
											{
												/*if (res_id == LIMITED_RESOURCE){
													ntf_type = CREDIT_THRESHOLD_BREACH_FLAG;
												}
												else if (res_id == FUP_RESOURCE){*/
													ntf_type = FUP_LIMIT_THRESHOLD_BREACH_FLAG;
												//}
												fm_mso_rate_aaa_create_ntf(ctxp, acct_pdp, serv_pdp, res_id, 
													credit_thresholds[i].threshold_config[j], ntf_type, ebufp) ;
												ntf_flag = 1;
											}
											reset_flag = 0;
											break;
										}
										else
										{
											reset_flag = 1;
											ntf_value = 0;
										}
									}
								}
							}
						}
						if(PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_EBUF(1, "Error in Checking Threshold", ebufp);
							
							goto CLEANUP;
						}
						//Update service object to set the NTF_STATUS.
						if((ntf_flag || reset_flag) && fup_status != 0)
							fm_mso_rate_aaa_update_bb_info(ctxp, serv_pdp, ntf_value, 1, ebufp);						
					}
				}
			}
		}
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&credit_profile_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bal_flistp, NULL);
	
	if( !pbo_decimal_is_null(credit_range, ebufp) )
		pbo_decimal_destroy(&credit_range);
	if( !pbo_decimal_is_null(zero, ebufp) )
		pbo_decimal_destroy(&zero);
	if( !pbo_decimal_is_null(hundred, ebufp) )
		pbo_decimal_destroy(&hundred);
	if( !pbo_decimal_is_null(percent_used, ebufp) )
		pbo_decimal_destroy(&percent_used);		
	if( !pbo_decimal_is_null(threhold_percentage, ebufp) )
		pbo_decimal_destroy(&threhold_percentage);		
	if( !pbo_decimal_is_null(unlimited_quotoa, ebufp) )
		pbo_decimal_destroy(&unlimited_quotoa);		

	return;
}


void 
fm_mso_rate_aaa_get_credit_profile(
	pcm_context_t   	*ctxp,
	poid_t			*acc_pdp, 
	pin_flist_t		*svc_flistp,
	pin_flist_t		**credit_profile_flistp,
	pin_errbuf_t  		*ebufp)
{
    	poid_t			*s_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*serv_pdp = NULL;

    	pin_flist_t		*srch_in_flistp = NULL;
   	pin_flist_t		*tmp_flistp = NULL;
    	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*srch_out_flistp2 = NULL;
    	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*res_flistp2 = NULL;
	pin_flist_t		*all_cities_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	pin_flist_t		*cities_flistp = NULL;
	pin_flist_t		*linkedobj_flistp = NULL;
	pin_flist_t		*extra_res_flistp = NULL;
	pin_flist_t		*plan_flistp = NULL;

    
	pin_cookie_t    	cookie = NULL;
	pin_cookie_t    	cookie2 = NULL;

	pin_decimal_t		*credit_profile_part = NULL;
	
    
	char			*plan_name = NULL;
	char			*all_cities = "*";
	char			*city = NULL;
	char			*plan_city = NULL;
	char 			*template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 ";
	//char			*template = "select X from /mso_purchased_plan 1, /plan 2 where 1.F1 = V1 and 1.F2 = 2.F3 and 1.F4 = V4 ";
    	char            	*plan_template = "select X from /mso_cfg_credit_limit where F1 = V1 ";
	char			str [200];
	char			*plan_type = NULL;
	
	int32			bill_when = 0;
	int32			plan_bill_when = 0;
	int32			city_found = 0;
	int32 			status = 2;
	int32			s_flags = 0;
    	int32			rec_id = 0;
    	int32			rec_id2 = 0;
    	int64 			db = 1;
	int32			*indicator = NULL;
	int32			direction  = -1;
	int32			inpr_status = 1;
	poid_t			*purc_pdp = NULL;
	time_t			*purc_end_t = NULL;
	int			*purc_status = NULL;
	time_t                  current_t = 0;
	pin_flist_t		*ret_flistpp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_rate_aaa_get_credit_profile", ebufp);
		return;
	}	
    
	/*  Algorithm:
	* Get all the plans for the purchased products which are in active state and are associated with this account.
	* For each of the plans, check if the credit limit config object exists.
	* Ideally, there should not be a config object existing for non-usage plans (like h/w rental, amc, etc..)
	* Add up all the credit limit and floors and derive the cumulative credit range.	
	*/

	*credit_profile_flistp = PIN_FLIST_CREATE(ebufp);
	serv_pdp = PIN_FLIST_FLD_GET(svc_flistp, PIN_FLD_POID, 0, ebufp);
	bb_info_flistp = PIN_FLIST_SUBSTR_GET(svc_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	city = PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_CITY, 0, ebufp);
	bill_when = *(int32 *)PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
	indicator = PIN_FLIST_FLD_GET(bb_info_flistp, PIN_FLD_INDICATOR, 0, ebufp);

	db = PIN_POID_GET_DB(acc_pdp);		
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_POID, s_pdp, ebufp);

	PIN_FLIST_FLD_SET (srch_in_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	PIN_FLIST_FLD_SET (srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);
		    
	tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_ARGS, 2, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, &status, ebufp);
	
	tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, &inpr_status, ebufp);   	
 	
    	tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);    

    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH input flist", srch_in_flistp);
    	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SEARCH output flist", srch_out_flistp);
    	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);

	current_t = pin_virtual_time(NULL);
	while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
                           &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
 	{
		tmp_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, 1, ebufp);
		purc_pdp = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 0, ebufp);
		fm_mso_get_poid_details(ctxp, purc_pdp, &ret_flistpp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while returning from function : fm_mso_get_poid_details", ebufp);
			return;
		}
		purc_status = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_STATUS, 0, ebufp);
		purc_end_t = PIN_FLIST_FLD_GET(ret_flistpp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
		if(purc_status && purc_end_t && (*purc_status == 3 || (*purc_end_t != 0 && *purc_end_t < current_t)))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Product not valid");
			PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
			continue;
		}
		PIN_FLIST_DESTROY_EX(&ret_flistpp, NULL);
		//plan_name = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_NAME, 0, ebufp);
		plan_type = NULL;
		plan_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
		plan_type = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_DESCR, 0, ebufp);
		plan_name = NULL;
		fm_mso_rate_aaa_get_plan_name(ctxp, plan_pdp, &plan_flistp, ebufp);
		if (plan_flistp)
			plan_name  = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_NAME, 0, ebufp);
		
		if (indicator && *indicator == 1 && plan_type && strcmp(plan_type, "base plan")  == 0)
		{
			fm_mso_rate_get_credit_limit_part(ctxp, serv_pdp, acc_pdp, plan_pdp, &credit_profile_part, ebufp);
		}
		else
		{
			credit_profile_part = pbo_decimal_from_str("1.00", ebufp);
		}	
		sprintf(str, "credit part %s plan name %s", pbo_decimal_to_str(credit_profile_part, ebufp), plan_name);
		PIN_ERR_LOG_MSG(3, str);
		srch_in_flistp = PIN_FLIST_CREATE(ebufp);
			//s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_POID, s_pdp, ebufp);
	        PIN_FLIST_FLD_SET (srch_in_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
        	PIN_FLIST_FLD_SET (srch_in_flistp, PIN_FLD_TEMPLATE, plan_template, ebufp);
        	tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_ARGS, 1, ebufp);
	        PIN_FLIST_FLD_SET (tmp_flistp, MSO_FLD_PLAN_NAME, plan_name, ebufp)
		tmp_flistp = PIN_FLIST_ELEM_ADD (srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PLAN LIMIT SEARCH input flist", srch_in_flistp);
	        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp2, ebufp);
       	 	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PLAN LIMIT SEARCH output flist", srch_out_flistp2);
		PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
		city_found = 0;
		res_flistp2 = PIN_FLIST_ELEM_GET(srch_out_flistp2, PIN_FLD_RESULTS, 0, 1, ebufp );

		if(!res_flistp2) {
			continue;
		}
			
		rec_id2 = 0;
		cookie2 = NULL;
		while((cities_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp2, MSO_FLD_CITIES,
                           &rec_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL) {
			// Pawan: Added below line to set all_cities_flistp to NULL since it takes reference 
			// from previous iteration flist
			plan_city = PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_CITY, 0, ebufp);
			plan_bill_when = *(int32 *)PIN_FLIST_FLD_GET(cities_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
			// logic to check if the city and payterm are applicable as per service settings.
			if(bill_when == plan_bill_when || plan_bill_when == 0) 
			{
				if(strcmp(city, plan_city) == 0) 
				{
					city_found = 1;
					fm_mso_rate_aaa_update_credit_limits(cities_flistp, credit_profile_flistp, credit_profile_part, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fm_mso_rate_aaa_update_credit_limits", ebufp);
					}	
				} 
				else if(strcmp(plan_city, all_cities) == 0) 
				{
					all_cities_flistp = PIN_FLIST_COPY(cities_flistp, ebufp);
				}
			}		
			if(city_found == 0 && all_cities_flistp != NULL) 
			{
				fm_mso_rate_aaa_update_credit_limits(all_cities_flistp, credit_profile_flistp, credit_profile_part, ebufp);	
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fm_mso_rate_aaa_update_credit_limits", ebufp);
				}				
				PIN_FLIST_DESTROY_EX(&all_cities_flistp, NULL);
			}
		}
		PIN_FLIST_DESTROY_EX(&srch_out_flistp2, NULL);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Final Credit Profile flist", *credit_profile_flistp);
	
	if(s_pdp)
		PIN_POID_DESTROY(s_pdp, ebufp);

    	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
}

void 
fm_mso_rate_aaa_update_credit_limits(
	pin_flist_t		*res_flistp,
	pin_flist_t		**credit_profile_flistp,
	pin_decimal_t		*credit_profile_part,
	pin_errbuf_t  		*ebufp)
{
	int32			rec_id = 0;
	pin_cookie_t    	cookie = NULL;
	pin_flist_t		*cl_flistp = NULL;
	pin_flist_t		*ncr_flistp = NULL;
	pin_decimal_t		*credit_limit = NULL;
	pin_decimal_t		*credit_floor = NULL;
        pin_decimal_t           *agg_cr_limit = NULL;
        pin_decimal_t           *agg_cr_floor = NULL;
        pin_decimal_t           *plan_cr_limit = NULL;
        pin_decimal_t           *plan_cr_floor = NULL;

	char			str [200];

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_rate_aaa_update_credit_limits", ebufp);
		return;
	}	
	
	PIN_ERR_LOG_FLIST(3, "fm_mso_rate_aaa_update_credit_limits input flist", res_flistp);
	PIN_ERR_LOG_FLIST(3, "credit_profile_flistp", *credit_profile_flistp)
	
	while((cl_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, MSO_FLD_CREDIT_PROFILE,
							   &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) 
	{
		if (rec_id == LIMITED_RESOURCE || rec_id == FUP_RESOURCE)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "result flist is ", cl_flistp);
			ncr_flistp = PIN_FLIST_ELEM_GET(*credit_profile_flistp, MSO_FLD_CREDIT_PROFILE, rec_id, 1, ebufp);
			if (ncr_flistp != NULL)
			{
				PIN_ERR_LOG_MSG(3, "NCR flistp is present");
				plan_cr_limit = PIN_FLIST_FLD_GET(cl_flistp, PIN_FLD_CREDIT_LIMIT, 0, ebufp);
				plan_cr_floor = PIN_FLIST_FLD_GET(cl_flistp, PIN_FLD_CREDIT_FLOOR, 0, ebufp);
				agg_cr_limit =  pbo_decimal_multiply(plan_cr_limit, credit_profile_part, ebufp);
				agg_cr_floor = pbo_decimal_multiply(plan_cr_floor, credit_profile_part, ebufp);
				credit_limit = PIN_FLIST_FLD_GET(ncr_flistp, PIN_FLD_CREDIT_LIMIT, 0,ebufp);
                                credit_floor = PIN_FLIST_FLD_GET(ncr_flistp, PIN_FLD_CREDIT_FLOOR, 0,ebufp);
				pbo_decimal_add_assign(agg_cr_limit, credit_limit, ebufp);
				pbo_decimal_add_assign(agg_cr_floor, credit_floor, ebufp); 
				
				PIN_FLIST_FLD_PUT(ncr_flistp, PIN_FLD_CREDIT_LIMIT, agg_cr_limit, ebufp);
				PIN_FLIST_FLD_PUT(ncr_flistp, PIN_FLD_CREDIT_FLOOR, agg_cr_floor, ebufp);
				PIN_FLIST_FLD_SET(ncr_flistp, PIN_FLD_RESOURCE_ID, &rec_id, ebufp);
			} 
			else 
			{
				PIN_ERR_LOG_MSG(3, "NCR flistp is not present");
				ncr_flistp = PIN_FLIST_ELEM_ADD(*credit_profile_flistp, MSO_FLD_CREDIT_PROFILE, rec_id, ebufp);
				plan_cr_limit = PIN_FLIST_FLD_GET(cl_flistp, PIN_FLD_CREDIT_LIMIT, 0, ebufp);
				plan_cr_floor = PIN_FLIST_FLD_GET(cl_flistp, PIN_FLD_CREDIT_FLOOR, 0, ebufp);
                                agg_cr_limit = pbo_decimal_multiply(plan_cr_limit, credit_profile_part, ebufp);
                                agg_cr_floor = pbo_decimal_multiply(plan_cr_floor, credit_profile_part, ebufp);
                                PIN_FLIST_FLD_PUT(ncr_flistp, PIN_FLD_CREDIT_LIMIT, agg_cr_limit, ebufp);
                                PIN_FLIST_FLD_PUT(ncr_flistp, PIN_FLD_CREDIT_FLOOR, agg_cr_floor, ebufp);
				PIN_FLIST_FLD_SET(ncr_flistp, PIN_FLD_RESOURCE_ID, &rec_id, ebufp);
			}
		}			
	}
	PIN_ERR_LOG_FLIST(3, "credit_profile_flistp", *credit_profile_flistp);
}

void
fm_mso_rate_aaa_get_balances(
	pcm_context_t  		*ctxp,
	poid_t         		*account_pdp,
	poid_t         		*serv_pdp,
	pin_flist_t    		**o_flistpp,
	int32			resource_id,
	pin_errbuf_t  		*ebufp)
{
	pin_flist_t       	*srch_in_flistp = NULL;
	pin_flist_t       	*srch_out_flistp =  NULL;
	pin_flist_t      	*arg_flistp = NULL;
	pin_flist_t       	*bal_flistp = NULL;
	pin_flist_t         	*sbal_flistp = NULL;
	pin_flist_t       	*result_flistp = NULL;
	pin_flist_t     	*bal_grp_flistp = NULL;
	pin_flist_t		*aso_out_flistp  = NULL;

	poid_t				*srch_pdp = NULL;
	pin_decimal_t		*current_bal = NULL;
	pin_decimal_t		*final_curr_bal = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*aso_bal = NULL;
	pin_decimal_t		*aso_bal_mb = NULL;
	pin_decimal_t		*one_mb = pbo_decimal_from_str(ONE_MB, ebufp);

	int64             	db = 1;
	int32             	srch_flag = 768;
	int32				elem_id = 0;

	pin_cookie_t    	cookie = NULL;

	char            	*template = "select X from /balance_group where F1 = V1 and F2 > V2 and F3 <= V3 ";
	time_t          	bal_srch_t = pin_virtual_time((time_t *)NULL);
	time_t			valid_from = 0;
	time_t			tmp_valid_from = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_rate_aaa_get_balances", ebufp);
		goto CLEANUP;
	}

	if (!account_pdp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_balances : missing account poid in input");
		goto CLEANUP;
	}

	db = PIN_POID_GET_DB(account_pdp);
	
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp)

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
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_balances : search balance_group input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_rate_aaa_get_balances : Error in calling PCM_OP_SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_balances : search bal_grp output flist", srch_out_flistp);
	bal_grp_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
	if (!bal_grp_flistp)
	{
		PIN_ERR_LOG_MSG(3, "No Balance");
		goto CLEANUP;
	}
	
	bal_flistp = PIN_FLIST_ELEM_GET(bal_grp_flistp, PIN_FLD_BALANCES, resource_id, 1, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "resource bal_flistp", bal_flistp);
	if (bal_flistp)
	{
		while((sbal_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_SUB_BALANCES, &elem_id, 1, &cookie, ebufp)) 
				!= (pin_flist_t *)NULL) 
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "resource sbal_flistp", sbal_flistp);
			tmp_valid_from = *(time_t *)PIN_FLIST_FLD_GET(sbal_flistp, PIN_FLD_VALID_FROM, 0, ebufp);
			
			if (valid_from == 0 || valid_from > tmp_valid_from)
				valid_from = tmp_valid_from;
			
			current_bal = PIN_FLIST_FLD_GET(sbal_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
			
			if(!pbo_decimal_is_null(current_bal, ebufp) && pbo_decimal_sign(current_bal, ebufp) < 0)
			{
				pbo_decimal_add_assign(final_curr_bal, current_bal, ebufp);
			}
		}
	}
	if (pbo_decimal_is_null(final_curr_bal, ebufp) && pbo_decimal_is_zero(final_curr_bal, ebufp))
	{
		PIN_ERR_LOG_MSG(3, "Zero Available Balance");
		goto CLEANUP;
	}
	/*if(!pbo_decimal_is_null(final_curr_bal, ebufp) && pbo_decimal_sign(final_curr_bal, ebufp) < 0)
	{
		pbo_decimal_negate_assign(final_curr_bal, ebufp);
	}*/
	fm_mso_rate_aaa_get_aso(ctxp, serv_pdp, account_pdp, valid_from, &aso_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_rate_aaa_get_balances : Error in calling PCM_OP_SEARCH", ebufp);
		goto CLEANUP;
	}
	
	if (aso_out_flistp)
	{
		aso_bal = PIN_FLIST_FLD_GET(aso_out_flistp, MSO_FLD_TOTAL_DATA, 0, ebufp);
		//convert in bytes to mb
		if(!pbo_decimal_is_null(aso_bal, ebufp) && !pbo_decimal_is_zero(aso_bal, ebufp))
		{
			aso_bal_mb = pbo_decimal_divide(aso_bal, one_mb, ebufp);
			pbo_decimal_add_assign(final_curr_bal, aso_bal_mb, ebufp);
		}
	}
	
	*o_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(*o_flistpp, PIN_FLD_CURRENT_BAL, final_curr_bal, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_balances out flistp", *o_flistpp);
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&aso_out_flistp, NULL);
	
	if(!pbo_decimal_is_null(aso_bal_mb, ebufp))
		pbo_decimal_destroy(&aso_bal_mb);
	if(!pbo_decimal_is_null(one_mb, ebufp))
		pbo_decimal_destroy(&one_mb);		
	return;
}

int32 
fm_mso_rate_aaa_res_threshold_validation (
	int32	rec_id)
{
	int32 	i = 0;
	
	for (i = 0; i < num_of_resources_configured ; i++){	
		if(credit_thresholds[i].resource_id == rec_id) {
			return 1;
		}
	}
	return 0;
}

void 
fm_mso_rate_aaa_deactivate_customer(
	pcm_context_t   	*ctxp,
	poid_t			*acct_pdp,
	poid_t			*serv_pdp,
	pin_errbuf_t  		*ebufp)
{
	pin_flist_t		*read_flds_in_flistp = NULL;
	pin_flist_t		*read_flds_out_flistp = NULL;
	pin_flist_t		*cancel_plan_flistp = NULL;
	pin_flist_t		*cancel_plan_oflistp = NULL;
	pin_flist_t		*plan_flistp = NULL;
	pin_flist_t		*deals_flistp = NULL;
	pin_flist_t		*srch_in_flistp = NULL;	
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*prd_flistp = NULL;
	
	poid_t			*pur_pdp = NULL;
	
	char			*template = "select X from /mso_purchased_plan where F1 = V1 and F2 = V2 and F3 = V3 and F4 = V4 order by mso_purchased_plan_t.created_t desc";
	char			*plan_type = "base plan";
	char			*program_name = "Usage Rating";
	char			*prov_tag = NULL;

	double			priority;
	int32			lim_pre_start = (BB_PREPAID_START + BB_LIMITED_RANGE_START);
	int32			lim_pre_end = (BB_PREPAID_START + BB_LIMITED_RANGE_END);
	int32			prov_status = MSO_PROV_DEACTIVE;
	int32			mso_prov_status = MSO_PROV_ACTIVE;
	int32			srch_flag =  768;
	int32			mode = QUOTA_EXHAUSTION;	
	int32			elem_id = 0;
	int64			db = 1;
	
	pin_cookie_t		cookie = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_rate_aaa_deactivate_customer", ebufp);
		goto CLEANUP;
	}
	
	//get base plan for the customer
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp)

	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_STATUS, &mso_prov_status, ebufp);
	
	arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 4, ebufp );
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_DESCR, plan_type, ebufp);
	
	result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_pp search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling mso_pp search", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_pp search output list", srch_out_flistp);
	result_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if(!result_flistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Active Base Pack Found");
		goto CLEANUP;
	}	
	while((prd_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) 
			!= (pin_flist_t *)NULL) 
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Product flistp", prd_flistp);
		prov_tag = PIN_FLIST_FLD_GET(prd_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
		
		if(prov_tag && strcmp(prov_tag, "") != 0)
		{
				pur_pdp = PIN_FLIST_FLD_GET(prd_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, 0, ebufp);
				break;
		}
	}
	priority = pbo_decimal_to_double(PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_PRIORITY, 0, ebufp), ebufp);
	priority = ((int32)priority%1000);

	if ( priority >= lim_pre_start && priority <= lim_pre_end) {

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Limited Prepaid plan.. Service suspension to be done.. ");
		read_flds_in_flistp =PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(read_flds_in_flistp, PIN_FLD_POID, pur_pdp, ebufp);
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
        	goto CLEANUP;
        }
		

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_READ_FLDS output flist", read_flds_out_flistp);

		cancel_plan_flistp = PIN_FLIST_CREATE(ebufp);

		/*deactivate_in_flistp = PIN_FLIST_CREATE(ebufp);*/
		PIN_FLIST_FLD_COPY(read_flds_out_flistp, PIN_FLD_ACCOUNT_OBJ, cancel_plan_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp, PIN_FLD_ACCOUNT_OBJ, cancel_plan_flistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp, PIN_FLD_ACCOUNT_OBJ, cancel_plan_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp, PIN_FLD_SERVICE_OBJ, cancel_plan_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
		
		plan_flistp = PIN_FLIST_ELEM_ADD(cancel_plan_flistp, PIN_FLD_PLAN_LISTS, 0, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp, PIN_FLD_PLAN_OBJ, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		deals_flistp = PIN_FLIST_ELEM_ADD(plan_flistp, PIN_FLD_DEALS, 0, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp, PIN_FLD_DEAL_OBJ, deals_flistp, PIN_FLD_DEAL_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(read_flds_out_flistp, PIN_FLD_PACKAGE_ID, deals_flistp, PIN_FLD_PACKAGE_ID, ebufp);
		PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_MODE, &mode, ebufp);
		PIN_FLIST_FLD_SET(cancel_plan_flistp, PIN_FLD_STATUS, &prov_status, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_DEACTIVATE_BB_SERVICE input flist", cancel_plan_flistp);
		PCM_OP(ctxp, MSO_OP_CUST_DEACTIVATE_BB_SERVICE, 0, cancel_plan_flistp, &cancel_plan_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&cancel_plan_flistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_CUST_DEACTIVATE_BB_SERVICE output flist", cancel_plan_oflistp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while deactviating the plan for limited prepaid account", ebufp);
			goto CLEANUP;
		}
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&cancel_plan_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_flds_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

void 
fm_mso_rate_aaa_throttle_customer(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	poid_t			*serv_pdp,
	int32			res_id,
	pin_errbuf_t  		*ebufp)
{
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*prov_out_flistp = NULL;
	int32			prov_flag = DOC_BB_UPDATE_CAP;
	int32			ntf_type = FUP_LIMIT_BREACH_FLAG;
	int32			*ret_status = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_rate_aaa_throttle_customer", ebufp);
		goto CLEANUP;
	}
	
	// Credit Limit Breach. Generate a notification.
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "FUP Limit Breached. Raising notification.. ");
	fm_mso_rate_aaa_create_ntf(ctxp, acct_pdp, serv_pdp, res_id, NULL, ntf_type, ebufp) ;
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
		goto CLEANUP;
	}
	
	//No need to trigger provisioning action as it is done by network element only.
	prov_in_flistp = PIN_FLIST_CREATE(ebufp);	
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &prov_flag, ebufp);
	PIN_FLIST_FLD_PUT(prov_in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ,
	PIN_POID_CREATE(PIN_POID_GET_DB(acct_pdp),"/mso_purchased_plan",-1,ebufp),ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist for provisioning action for FUP capping", prov_in_flistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, prov_in_flistp, &prov_out_flistp, ebufp);
	
	if(prov_out_flistp)
	{
		ret_status = PIN_FLIST_FLD_GET(prov_out_flistp, PIN_FLD_STATUS, 1, ebufp);
	}
	if(PIN_ERRBUF_IS_ERR(ebufp) || (ret_status && *ret_status != 0))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in Creating Provisioning for Throttle", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist from provisioning action for FUP capping", prov_out_flistp);
	
	//Update service object to set the FUP_STATUS to AFT_FUP.
	fm_mso_rate_aaa_update_bb_info(ctxp, serv_pdp, LIMIT_EXCEED, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in fm_mso_rate_aaa_update_bb_info", ebufp);
		goto CLEANUP;
	}
	fm_mso_create_lifecycle_evt(ctxp, prov_in_flistp, NULL, ID_FUP_DOWNGRADE ,ebufp );
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		goto CLEANUP;
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
	return;
}

void 
fm_mso_rate_aaa_create_ntf(
	pcm_context_t   *ctxp,
	poid_t			*acct_pdp,
	poid_t			*serv_pdp,	
	int32 			rec_id,
	pin_decimal_t	*percentage,
	int32			ntf_type,
	pin_errbuf_t 	*ebufp) 
{
	pin_flist_t		*notif_iflistp = NULL;
	pin_flist_t		*notif_oflistp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_rate_aaa_create_ntf", ebufp);
		return;
	}
	
	notif_iflistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_POID, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
	PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_FLAGS, &ntf_type, ebufp);
	
	if (ntf_type != FUP_LIMIT_BREACH_FLAG){
		PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_PERCENT, percentage, ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_RESOURCE_ID, &rec_id, ebufp);
	}
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_create_ntf  input list", notif_iflistp);	
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION , 0, notif_iflistp, &notif_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
    	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_NTF_CREATE_NOTIFICATION ", ebufp);
		PIN_ERRBUF_RESET(ebufp);
   	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_create_ntf output list", notif_oflistp);
	
	PIN_FLIST_DESTROY_EX(&notif_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&notif_oflistp, NULL);
	return;
}

void
fm_mso_rate_aaa_update_bb_info(
	pcm_context_t   *ctxp,
	poid_t			*serv_pdp,	
	int32			update_value,
	int32			update_type,	//0 - fup status , 1 - NTF status
	pin_errbuf_t 	*ebufp)
{
	pin_flist_t		*wrt_flds_in_flistp = NULL;
	pin_flist_t		*wrt_flds_out_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before fm_mso_rate_aaa_update_bb_info", ebufp);
		return;
	}
	
	wrt_flds_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(wrt_flds_in_flistp, PIN_FLD_POID, serv_pdp, ebufp);
	bb_info_flistp = PIN_FLIST_ELEM_ADD(wrt_flds_in_flistp, MSO_FLD_BB_INFO, 0, ebufp);
	
	if (update_type == 0){
		PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_FUP_STATUS, &update_value, ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(bb_info_flistp, MSO_FLD_NTF_STATUS, &update_value, ebufp);
	}
		
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist for updating bb_info", wrt_flds_in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wrt_flds_in_flistp, &wrt_flds_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&wrt_flds_in_flistp, NULL);	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist from updating bb_info", wrt_flds_out_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
    {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in update service", ebufp);
    }
	
	PIN_FLIST_DESTROY_EX(&wrt_flds_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&wrt_flds_out_flistp, NULL);
	return;
}

void
fm_mso_rate_get_credit_limit_part(
	pcm_context_t	*ctxp,
	poid_t		*serv_pdp,
	poid_t		*acnt_pdp,
	poid_t		*plan_pdp,
	pin_decimal_t	**credit_profile_part,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*srch_pp_flistp = NULL;
	pin_flist_t	*srch_pp_o_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;

	poid_t		*srch_pdp = NULL;
	
	time_t		*charged_from_t = NULL;
	time_t		*charged_to_t = NULL;

	struct tm	*ts;

	int32		diff;
	int32		day_in_month;
	int32		day;
	int32		month;
	int32		status_flags = 256;
	int32		pp_status = 3;
	int64		db = 1;

	char		*pp_srch_template = "select X from /purchased_product where F1 = V1 and F2 <> V2 and F3 = V3 and F4 = V4 and purchased_product_t.descr like '%-%GRANT' ";
	char		str [100];

	pin_decimal_t	*diff_dec = NULL;
	pin_decimal_t	*day_in_month_dec = NULL;
	pin_decimal_t	*temp_dec = NULL;

	db = PIN_POID_GET_DB(acnt_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_pp_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_pp_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_pp_flistp, PIN_FLD_FLAGS, &status_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_pp_flistp, PIN_FLD_TEMPLATE, pp_srch_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp,PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &pp_status, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);

	results_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp,PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search fup type input", srch_pp_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_pp_flistp, &srch_pp_o_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search pp output", srch_pp_o_flistp);
	results_flistp = PIN_FLIST_ELEM_GET(srch_pp_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);

	if (results_flistp)
	{
		PIN_ERR_LOG_MSG(3, "It's Grant Products");
		tmp_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1, ebufp);
		if (tmp_flistp)
		{
			PIN_ERR_LOG_MSG(3, "It's Cycle Fee Details");
			charged_from_t = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
			charged_to_t = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
			ts = localtime(charged_from_t);
			month = ts->tm_mon;
			day = ts->tm_mday;

			if (month == 0 || month == 2 || month == 4 || month == 6 || month == 7 || month == 9 || month == 11)
				day_in_month = 31;
			else if (month == 1)
				day_in_month = 28;
			else
				day_in_month = 30;

			diff =  day_in_month - day + 1;
			if (diff != day_in_month)
			{
				PIN_ERR_LOG_MSG(3, "Short Cycle");
				diff_dec = pbo_decimal_from_double((double)diff, ebufp);
				day_in_month_dec = pbo_decimal_from_double((double)day_in_month, ebufp);
				sprintf(str, "diff %s", pbo_decimal_to_str(diff_dec, ebufp));
				PIN_ERR_LOG_MSG(3, str);
				sprintf(str, "day_in_month_dec %s",  pbo_decimal_to_str(day_in_month_dec, ebufp));
				PIN_ERR_LOG_MSG(3, str);
	
				if (!pbo_decimal_is_zero(diff_dec, ebufp) && !pbo_decimal_is_zero(day_in_month_dec, ebufp))
					*credit_profile_part = pbo_decimal_divide(diff_dec, day_in_month_dec, ebufp);
			}
			else
			{
				PIN_ERR_LOG_MSG(3, "Full Cycle");
				*credit_profile_part = pbo_decimal_from_str("1.00", ebufp);
			}
			sprintf(str, "credit_profile_part %s", pbo_decimal_to_str(*credit_profile_part, ebufp));
				 PIN_ERR_LOG_MSG(3, str);
		}
	}
	
	return ;
}

void
fm_mso_rate_aaa_get_plan_name(
        pcm_context_t           *ctxp,
        poid_t                  *plan_pdp,
        pin_flist_t            	**r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *read_obj_input = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "Error before calling fm_mso_rate_aaa_get_plan_name", ebufp);
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_plan_name input flist");


        read_obj_input = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(read_obj_input, PIN_FLD_POID, plan_pdp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_rate_aaa_get_plan_name read input list", read_obj_input);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_obj_input, r_flistpp, ebufp);
        PIN_FLIST_DESTROY_EX(&read_obj_input, NULL);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error in calling PCM_OP_READ_OBJ", ebufp);
                return;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_rate_aaa_get_plan_name READ output list", *r_flistpp);
	return;
}

int32 
fm_mso_rate_aaa_throttle_product_chk(
	pcm_context_t		*ctxp, 
	poid_t			*acct_pdp, 
	poid_t 			*serv_pdp,
	pin_errbuf_t		*ebufp)
{
	poid_t		*srch_pdp = NULL;
	int32           pp_status = 1;
        int64           db = 1;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	time_t		*end_date = NULL;
	char            *pp_srch_template = "select X from /purchased_product 1,/mso_purchased_plan 2,/product 3 where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = V3 and 1.F4 = 2.F5 and 1.F6 = 3.F7 and 3.F8 like V8 and 2.F9 = V9";
	time_t          current_t = 0;
	int32           status_flags = 256;
	int32		ret_val = 0;
	pin_flist_t	*cycle_fees_flistp = NULL;
	pin_cookie_t    cookie = NULL;
	int             elem_id = 0;
	pin_flist_t	*srch_pp_flistp = NULL;
	pin_flist_t	*srch_pp_o_flistp = NULL;
	pin_flist_t	*temp_flistp = NULL;	
	time_t          *purc_end_t = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error before calling function fm_mso_rate_aaa_throttle_product_chk", ebufp);
                return;
        }

	db = PIN_POID_GET_DB(acct_pdp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        srch_pp_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(srch_pp_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_pp_flistp, PIN_FLD_FLAGS, &status_flags, ebufp);
        PIN_FLIST_FLD_SET(srch_pp_flistp, PIN_FLD_TEMPLATE, pp_srch_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp,PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &pp_status, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, serv_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 5, ebufp);
        temp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 6, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 7, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 8, ebufp);
        temp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_EVENT_TYPE, "%mso_grant", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp, PIN_FLD_ARGS, 9, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_DESCR, "base plan", ebufp);
        results_flistp = PIN_FLIST_ELEM_ADD(srch_pp_flistp,PIN_FLD_RESULTS, 0, ebufp );

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error before calling search in function fm_mso_rate_aaa_throttle_product_chk", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search purchased_product input for throtell chk", srch_pp_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_pp_flistp, &srch_pp_o_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search purchased_product output for throtell chk", srch_pp_o_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error after calling search in function fm_mso_rate_aaa_throttle_product_chk", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		goto CLEANUP;
        }	

	/*if(PIN_FLIST_ELEM_COUNT(srch_pp_o_flistp, PIN_FLD_RESULTS,ebufp) == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No products's available");
		ret_val = 1;
	}
	else
	{*/
		current_t = pin_virtual_time(NULL);
				
		results_flistp = PIN_FLIST_ELEM_GET(srch_pp_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if(results_flistp)
		{	
			purc_end_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
			while ((cycle_fees_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_CYCLE_FEES,
						&elem_id, 1, &cookie, ebufp)) != NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "In cycle loop");
				end_date = PIN_FLIST_FLD_GET(cycle_fees_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
				if(end_date && purc_end_t && current_t > *end_date && (*purc_end_t > current_t || *purc_end_t == 0))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not Eligible for throtelling");
					ret_val = 1;
				}
			}	
		}
	//}
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_pp_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_pp_o_flistp, NULL);
	return ret_val;
}

int32 
fm_mso_rate_aaa_get_pp(
	pcm_context_t  	*ctxp,
	poid_t         	*svc_pdp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t   	*ebufp)
{
	pin_flist_t     *srch_iflistp = NULL;
	pin_flist_t     *srch_oflistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *cycle_flistp = NULL;
	poid_t          *srch_pdp = NULL;
	int64   	db = 1;
	char            *template = "select X from /purchased_product 1, /product 2  where 1.F1 = V1 and 1.F2 = V2 and 1.F3 = 2.F4 and product_t.provisioning_Tag is not null ";
	int32           status = 1; // active
	int32           srch_flags = 0;

	if( PIN_ERRBUF_IS_ERR(ebufp)){
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_PLAN_OBJ, NULL, ebufp);
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_pp: search input flist : ", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_get_pp: search output flist : ", srch_oflistp);

	results_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if(results_flistp) {
		PIN_FLIST_FLD_COPY( results_flistp, PIN_FLD_PLAN_OBJ, *ret_flistpp, PIN_FLD_PLAN_OBJ, ebufp );
		} 
	
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

