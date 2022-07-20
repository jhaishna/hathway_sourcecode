/*******************************************************************
 * File:	fm_mso_ar_adjustgetment_details.c14-11-2013
 * Opcode:	MSO_OP_AR_ADJUSTMENT , 13151
 * Owner:	Asish Joshi
 * Created:	17-NOV-2013
 * Description: 
 *

Sample input flist event adjustment
0 PIN_FLD_POID                POID [0] 0.0.0.1 /account 368570 16
0 PIN_FLD_PROGRAM_NAME         STR [0] "MSO_OP_AR_ADJUSTMENT"
0 PIN_FLD_ADJUSTMENT_INFO    ARRAY [0] allocated 20, used 2
1     PIN_FLD_RESOURCE_ID      INT [0] 356
1     PIN_FLD_AMOUNT        DECIMAL [0] -1.5
0 PIN_FLD_EVENTS             ARRAY [0] allocated 20, used 1
1     PIN_FLD_POID            POID [0] 0.0.0.1 /event/billing/product/fee/cycle/cycle_forward_monthly 284940637361865203 0
0 PIN_FLD_USERID              POID [0] 0.0.0.1 /account 26216 0
0 PIN_FLD_DESCR                STR [0] "Service down"
0 PIN_FLD_FLAGS             INT [0] 2
0 pbo_decimal_is_zero         INT [0] 2

// Document History
//
// DATE         /  CHANGE                                       /  Author
//17-NOV-2013   / Initial Version                               / Asish Joshi
//17-Oct-2014   / Added BB Customzations and Error Handling     / Harish Kulkarni
*******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_do_adjustment.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "pcm_ops.h"
#include "mso_ops_flds.h"
#include "ops/ar.h"
#include "ops/bill.h"
#include "pin_decimal.h"
#include "mso_lifecycle_id.h"


#define SUCCESS 0
#define FAILURE 1
#define READWRITE 1
#define LOCAL_TRANS_OPEN_SUCCESS 0
#define BILLINFO_ID_BB "BB"
#define BILLINFO_ID_CATV "CATV"
#define EVENT_ADJUSTMENT "EVENT_ADJUSTMENT"
#define BILL_ADJUSTMENT "BILL_ADJUSTMENT"
#define ITEM_ADJUSTMENT "ITEM_ADJUSTMENT"
#define ACCOUNT_ADJUSTMENT "ACCOUNT_ADJUSTMENT"
#define SERV_TYPE_BB 1
#define SERV_TYPE_CATV 0
#define RENTAL_HSN "9973"
#define SUBSCRIPTION_HSN "9984"

/*******************************************************************
 * MSO_OP_AR_ADJUSTMENT 
 *******************************************************************/

EXPORT_OP void
mso_op_ar_adjustment(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_ar_adjustment_details(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_check_csr_adj_limit(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	pin_decimal_t		*ip_adj_amount,
	int			*trans_breaced,
	int			*monthly_breached,
	pin_errbuf_t		*ebufp);

static void
fm_mso_update_mso_ar_limit(
	pcm_context_t		*ctxp,
	poid_t			*mso_limit_pdp,
	time_t			first_of_month,
	pin_decimal_t		*adj_amount,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_return_first_of_month(
	time_t			adj_date,
	time_t			*ret_date);

static void 
fm_mso_search_mso_ar_limit(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	pin_flist_t		**mso_ar_limitpp,
	pin_errbuf_t		*ebufp);

PIN_EXPORT void 
fm_mso_ar_create_adjustment(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*res_flistp,
	int32			*service_type,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp) ;

PIN_EXPORT void 
fm_mso_ar_create_adjustment_object(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*res_flistp,
	pin_flist_t		*bill_details_flistp,
	pin_flist_t		*receipt_flistp,
	int32			*service_type,
	pin_decimal_t		*amount,
	pin_decimal_t		*rem_amount,
	pin_errbuf_t		*ebufp);
	
void fm_mso_part_receipt_no(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	int32				*service_type,
	pin_decimal_t		*amount,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);
	
//static void
//mso_op_adjustment_gen_lc_event(
//	pcm_context_t           *ctxp,
//	pin_flist_t             *i_flistp,
//	pin_flist_t		*s_oflistp,
//	char                    *disp_name,
//	pin_errbuf_t            *ebufp);

PIN_IMPORT int32 fm_mso_trans_open(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	int32			flag,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_trans_commit(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_trans_abort(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_utils_get_billinfo_bal_details(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	char			*billinfo_id,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void 
fm_inv_pol_get_state_code_from_cache(
	pcm_context_t	*ctxp,
	char			*state,
	pin_flist_t		**state_flistpp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT
void fm_mso_utils_gen_event(
	pcm_context_t   *ctxp,
	poid_t          *acct_pdp,
	poid_t          *serv_pdp,
	char            *program,
	char            *descr,
	char            *event_type,
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp);

PIN_IMPORT
void
fm_mso_utils_read_any_object(
	pcm_context_t           *ctxp,
	poid_t                  *objp,
	pin_flist_t             **out_flistpp,
	pin_errbuf_t            *ebufp);

extern void
fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

	
void
fm_mso_get_bill_amount_available(
	pcm_context_t	*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

void 
fm_mso_bill_adj(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);

void
fm_mso_get_reason(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp);

	
PIN_IMPORT int32 
fm_mso_utils_gst_sequence_no(
	pcm_context_t	*ctxp,
	poid_t			*account_pdp,
	char			*name,
	pin_errbuf_t	*ebufp);
	
PIN_IMPORT void
fm_mso_get_subscriber_profile(
	pcm_context_t	*ctxp,
	poid_t		*account_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void
fm_mso_get_bills(
	pcm_context_t	*ctxp,
	char		*bill_no,
	poid_t		*account_pdp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void
fm_mso_utils_get_open_bill_items(
    pcm_context_t       *ctxp,
    poid_t              *ar_bill_pdp,
    int32               all_items_flag,
    pin_flist_t         **o_flistpp,
    pin_errbuf_t        *ebufp);


PIN_IMPORT void
fm_mso_get_account_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
* Added for GSTIN Changes to get invoice's created_t value
*******************************************************************/
time_t getInvoiceCreatedT(
        pcm_context_t   *ctxp,
        poid_t          *bill_pdp,
        pin_errbuf_t    *ebufp);

/*******************************************************************
* Added for GSTIN Changes to get gst_flag value from invoice buffer
*******************************************************************/
int getGSTINflag(
        pcm_context_t   *ctxp,
        poid_t          *bill_pdp,
        pin_errbuf_t    *ebufp);
/*******************************************************************
* Added for GSTIN Changes to read /profile/gst_info object
*******************************************************************/
void getProfileGSTInfo(
        pcm_context_t *ctxp,
        poid_t *pdp,
        pin_flist_t **profile_gst_flistp,
        pin_errbuf_t *ebufp);


/*******************************************************************
 * MSO_OP_AR_ADJUSTMENT  
 *******************************************************************/
void 
mso_op_ar_adjustment(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	
	poid_t			*inp_pdp = NULL;

	int32			*flag_adj_type = NULL;
	int32			*tax_adj_type = NULL;
	int32			*flag = NULL;
	int			status = 0;
	pin_decimal_t		*amount = NULL;
	pin_decimal_t		*abs_amount = NULL;
	pin_decimal_t 		*valid_amount = pbo_decimal_from_str("999999.99", ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	
	PIN_ERRBUF_CLEAR(ebufp);

	// Insanity Check 

	if (opcode != MSO_OP_AR_ADJUSTMENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_ar_adjustment error",
			ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_AR_ADJUSTMENT Input Flist:", i_flistp);
	
	//Check if adjustment amount is greater than 99999.99
	amount = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_AMOUNT,0,ebufp);
	abs_amount  = pbo_decimal_abs(amount,ebufp);
	
	if(abs_amount && valid_amount &&  pbo_decimal_compare(abs_amount,valid_amount,ebufp) > 0)
	{
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_ar_adjustment_details Validation : Adjustment more than 99999.99 not allowed", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_ar_adjustment_details error");
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID,
                PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ), ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53085", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
                                                "Validation : Adjustment more than 99999.99 not allowed", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		 if (abs_amount && !pbo_decimal_is_null(abs_amount, ebufp))
                       pbo_decimal_destroy(&abs_amount);

	         if (valid_amount && !pbo_decimal_is_null(valid_amount, ebufp))
                       pbo_decimal_destroy(&valid_amount);

			
		return;
        }

	// Call the main function to process the adjustments
	fm_mso_ar_adjustment_details(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_ar_adjustment_details error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		/*status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_ar_adjustment_details error");
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, 
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ), ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53080", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
						"fm_mso_ar_adjustment_details error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);*/
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"fm_mso_ar_adjustment_details output flist", *r_flistpp);
	}
	
	return;
}

static void 
fm_mso_ar_adjustment_details(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*d_iflistp = NULL;
	pin_flist_t		*s_oflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*temp1_flistp = NULL;
	pin_flist_t		*temp2_flistp = NULL;
	pin_flist_t		*temp11_flistp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*rslt_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*tax_flistp = NULL;
	
	poid_t			*user_id = NULL;
	poid_t			*bill_pdp = NULL;
	poid_t			*a_pdp = NULL;
	char			*acnt_no = NULL;
	char			*login = NULL;
        char                            *acct_type = NULL;
	int			tax_flag = 0;
	int64			db = -1;
	int32			*flag_adj_type = NULL;
	int32			*output_result = NULL;
	int32			dispute_success = 0;
	int32			dispute_failure = 1;
	int32			*tax_adj_type = NULL;	
	int32			*flag = NULL;
	int32			status = SUCCESS;
	int32			without_tax = 1;
	int32			account_type = 0;
	int32			amount = 0;
	int32			reso_id = 356;
	poid_t			*ip_userid = NULL;
	pin_decimal_t		*ip_amount = NULL;
	pin_decimal_t		*tax_rate = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*temp_rate = NULL;
	pin_decimal_t		*calc_amount = NULL;
	pin_decimal_t		*orig_amount = NULL;
	pin_decimal_t		*hundred = pbo_decimal_from_str("100.0", ebufp);
	int			csr_trans_limit_breach = 0;
	int			csr_mnth_limit_breach = 0;
	int			local = 1;	
	int                     mode = 0;
	int32			*serv_type = NULL;
	poid_t			*bal_grp_pdp = NULL;
	int32                   precision = 6;
	int32                   round_mode = ROUND_HALF_UP;		
	int32			with_tax = 2;
	void			*vp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		result_flistp = PIN_FLIST_CREATE(ebufp);
	 	status = FAILURE;		
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53080" , ebufp);
					
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail output flist", result_flistp);
		*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
		goto CLEANUP;
	}

	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

        acct_type = (char *)PIN_POID_GET_TYPE(a_pdp);
        if (acct_type && ((strcmp(acct_type,"/account")==0))) {
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "inside account obj!  ...");
        }
        else {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "outside account!  ...");
        a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        }

//	local = fm_mso_trans_open(ctxp, a_pdp, READWRITE, ebufp);
	local = fm_mso_trans_open(ctxp, a_pdp, 3 , ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		result_flistp = PIN_FLIST_CREATE(ebufp);
		status = FAILURE;
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Error opening transaction", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53080" , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail output flist", result_flistp);
		*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
		goto CLEANUP;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Opened");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_AR_ADJUSTMENT input flist", i_flistp );
	mso_retrieve_gst_rate(ctxp, SUBSCRIPTION_HSN, a_pdp, &tax_flistp, ebufp);
	if(tax_flistp)
	{
		temp_rate = PIN_FLIST_FLD_GET(tax_flistp, MSO_FLD_IGST_RATE, 1, ebufp);     //IGST
		if(!pbo_decimal_is_null(temp_rate, ebufp))
			pbo_decimal_add_assign(tax_rate, temp_rate, ebufp);

		temp_rate = PIN_FLIST_FLD_GET(tax_flistp, MSO_FLD_SGST_RATE, 1, ebufp);     //SGST
                if(!pbo_decimal_is_null(temp_rate, ebufp))
                        pbo_decimal_add_assign(tax_rate, temp_rate, ebufp);

		temp_rate = PIN_FLIST_FLD_GET(tax_flistp, MSO_FLD_CGST_RATE, 1, ebufp);                     //CGST
                if(!pbo_decimal_is_null(temp_rate, ebufp))
                        pbo_decimal_add_assign(tax_rate, temp_rate, ebufp);

		pbo_decimal_multiply_assign(tax_rate, hundred, ebufp);
		pbo_decimal_add_assign(tax_rate, hundred, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&tax_flistp, NULL);

	orig_amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	calc_amount = pbo_decimal_divide(orig_amount, tax_rate, ebufp);
	pbo_decimal_multiply_assign(calc_amount, hundred, ebufp);
//	pbo_decimal_round_assign(calc_amount, precision, round_mode, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_AMOUNT, calc_amount, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_TAX_FLAGS, &with_tax, ebufp);

	if(!pbo_decimal_is_null(hundred, ebufp))
		pbo_decimal_destroy(&hundred);
	if(!pbo_decimal_is_null(tax_rate, ebufp))
		pbo_decimal_destroy(&tax_rate);
	if(!pbo_decimal_is_null(calc_amount, ebufp))
		pbo_decimal_destroy(&calc_amount);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_AR_ADJUSTMENT modified flist", i_flistp );

	serv_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp);
	ip_amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	ip_userid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp) || (pbo_decimal_sign(ip_amount, ebufp) == 0))
		goto CLEANUP;

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 1, ebufp);

	if ( PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BOOLEAN, 1, ebufp) != NULL )
	{
			mode = *(int *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BOOLEAN, 1, ebufp);
	}

	if (vp && *(int32*)(vp) == 32 && mode == 1) 
	{
		fm_mso_bill_adj(ctxp, i_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "inside bill flist", i_flistp );
	}

	if(serv_type && ip_amount && pbo_decimal_sign(ip_amount, ebufp) == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside CRDR");
		//login = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LOGIN, 0, ebufp );
		if(PIN_ERRBUF_IS_ERR(ebufp))
			goto CLEANUP;

		/* Check the CSR Adjustment Limit */
		fm_mso_check_csr_adj_limit(ctxp, ip_userid, ip_amount,
				&csr_trans_limit_breach, &csr_mnth_limit_breach, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		 {
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR,
					"Unknown Error checking CSR adjustment limit", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Error flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		 }
		else if(csr_trans_limit_breach == 1 || csr_mnth_limit_breach == 1)
		{
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			if(csr_trans_limit_breach == 1)
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR,
					"CSR adjustment transaction limit breached", ebufp);
			if(csr_mnth_limit_breach == 1)
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR,
					"CSR adjustment monthly transaction limit breached", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Error flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This is a debit note request for Broadband.");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_AR_DEBIT_NOTE input flist", i_flistp );
		PCM_OP(ctxp, MSO_OP_AR_DEBIT_NOTE, 0, i_flistp, &s_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_AR_DEBIT_NOTE output flist", s_oflistp );

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Adjustment Failed", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adjustment Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		}

		output_result = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_STATUS, 1, ebufp );
		if(*output_result!=0) //It failed
		{
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Debit note Failed", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		}
		*r_flistpp = PIN_FLIST_COPY(s_oflistp, ebufp);
		goto CLEANUP;
	}

	flag_adj_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp );

	/********************************************************
	 * Avoid debit adjustment in case of broadband Account &
	 * Bill adjustment. Also, avoid credit adjustment in case
	 * of broadband Item head adjustment.
	 ********************************************************/
	if(serv_type && ip_amount && *serv_type == SERV_TYPE_BB && 
			flag_adj_type && (( *flag_adj_type == 1 && pbo_decimal_sign(ip_amount, ebufp) == -1) ||
				( *flag_adj_type != 1 && pbo_decimal_sign(ip_amount, ebufp) == 1 ))){
		result_flistp = PIN_FLIST_CREATE(ebufp);
		status = FAILURE;
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53080" , ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Debit adjustment Not allowed", ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail outflist flist", result_flistp);
		*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
		goto CLEANUP;
	}		

	//flag_adj_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp );
	tax_adj_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TAX_FLAGS, 0, ebufp );
	if(PIN_ERRBUF_IS_ERR(ebufp))
		goto CLEANUP;
	
	if (*tax_adj_type==2)
	{
		tax_flag = PIN_AR_WITH_TAX;
	}
	else{
		tax_flag = PIN_AR_WITHOUT_TAX;
	}
	
	/* Check the CSR adjustment Limit */
	if(*(flag_adj_type)== 0 || *(flag_adj_type)== 1 || *(flag_adj_type)== 3)
	{
		ip_amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - adjustment Amount missing",ebufp);
			goto CLEANUP;
		}
	}
	else if(*(flag_adj_type)==2)
	{
		temp11_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ADJUSTMENT_INFO, PIN_ELEMID_ANY, 1, ebufp );
		ip_amount = PIN_FLIST_FLD_GET(temp11_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - adjustment Amount missing",ebufp);
			goto CLEANUP;
		}

	}
	if(pbo_decimal_sign(ip_amount, ebufp) == -1)
	{
		/* Check the CSR Adjustment Limit */
		fm_mso_check_csr_adj_limit(ctxp, ip_userid, ip_amount,
				&csr_trans_limit_breach, &csr_mnth_limit_breach, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		 {
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR,
					"Unknown Error checking CSR adjustment limit", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Error flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		 }
		else if(csr_trans_limit_breach == 1 || csr_mnth_limit_breach == 1)
		{
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			if(csr_trans_limit_breach == 1)
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR,
					"CSR adjustment transaction limit breached", ebufp);
			if(csr_mnth_limit_breach == 1)
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR,
					"CSR adjustment monthly transaction limit breached", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Error flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		}
	}

	if(*(flag_adj_type)==0) // then do bill adjustment
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside bill adjustment");
		//Input Flist mandatory fields check
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - Bill poid missing",ebufp);
			goto CLEANUP;
			}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - PIN_FLD_PROGRAM_NAME missing",ebufp);
			goto CLEANUP;
			}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - PIN_FLD_DESCR missing",ebufp);
			goto CLEANUP;
			}
		ip_amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - PIN_FLD_AMOUNT missing",ebufp);
			goto CLEANUP;
			}
		// Debit adjustments are not allowed for BB
		if(*serv_type == SERV_TYPE_BB && pbo_decimal_sign(ip_amount, ebufp) == 1)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Debit Adjustments are not allowed for BB", ebufp);
			goto CLEANUP;	
		}

	/* Adjustment Reasons
		1. Customer not satisfied with service
		2. Customer unaware of charges
		3. Debited account by mistake
		4. Technical and support charges
		5. Service charges
		6. Others	

	*/
		
		
		d_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, d_iflistp, PIN_FLD_AMOUNT, ebufp); //amount entered must be <= bill due amount
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, d_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, d_iflistp, PIN_FLD_DESCR, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_DOMAIN_ID, d_iflistp, PIN_FLD_REASON_DOMAIN_ID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, d_iflistp, PIN_FLD_REASON_ID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, d_iflistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_FLAGS, &tax_flag, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_BILL_ADJUSTMENT input flist", d_iflistp );
		PCM_OP(ctxp, PCM_OP_AR_BILL_ADJUSTMENT, 0, d_iflistp, &s_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_BILL_ADJUSTMENT output flist", s_oflistp );
		PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		 {
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Bill Adjustment Failed", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BILL_ADJUSTMENT Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		 }  
				
		output_result = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_RESULT, 1, ebufp );
		if(*output_result!=1) //It failed
		{
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BILL_ADJUSTMENT Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;

		}
		else
		{
			//mso_op_adjustment_gen_lc_event(ctxp, i_flistp, s_oflistp, BILL_ADJUSTMENT, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ORG_DISPLAY_NAME, BILL_ADJUSTMENT, ebufp);
			//if (serv_type && *serv_type == SERV_TYPE_BB)
			//{
				fm_mso_ar_create_adjustment(ctxp, i_flistp, s_oflistp, serv_type, &ret_flistp, ebufp); 
				if (PIN_ERRBUF_IS_ERR(ebufp) || ret_flistp == NULL)
                        	{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
							"Adjustment Failure in create_adjustment obj");
					PIN_ERRBUF_CLEAR(ebufp);
                                	result_flistp = PIN_FLIST_CREATE(ebufp);
                                	status = FAILURE;
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
                                	PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, 
										PIN_FLD_ERROR_DESCR, ebufp);
                                	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
                                	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53082" , ebufp);
                                	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
						"ADJUSTMENT Fail outflist flist", result_flistp);
                                	*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
                                	PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
                                	goto CLEANUP;
				}
				status = *(int32 *)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
				if(status == 1)
				{
					*r_flistpp=PIN_FLIST_COPY(ret_flistp, ebufp);
					goto CLEANUP;
				}
			//}
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, s_oflistp, ID_ADJUSTMENT, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Adjustment Failure in Lifecycle");
				PIN_ERRBUF_CLEAR(ebufp);
				result_flistp = PIN_FLIST_CREATE(ebufp);
				status = FAILURE;
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
				PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail outflist flist", result_flistp);
				*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
				goto CLEANUP;
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_BILL_ADJUSTMENT outflist flist", s_oflistp);
			// Preparing output flist for item adjustment
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, "Adjustment Success:", ebufp);
			status = SUCCESS;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BILL_ADJUSTMENT outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		}
	}


	if(*(flag_adj_type)==1) // then do item adjustment
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside item adjustment ");
		//Input Flist mandatory fields check
/*		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - item poid missing",ebufp);
			return;
			}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - adjustment Amount missing",ebufp);
			return;
			}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - PIN_FLD_PROGRAM_NAME missing",ebufp);
			return;
			} */
		
		ip_amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - PIN_FLD_AMOUNT missing",ebufp);
			goto CLEANUP;
		}

		// Debit adjustments are not allowed for BB
		if(*serv_type == SERV_TYPE_BB && pbo_decimal_sign(ip_amount, ebufp) == 1)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Debit Adjustments are not allowed for BB", ebufp);
			goto CLEANUP;
		}

		d_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, d_iflistp, PIN_FLD_AMOUNT, ebufp); //amount entered must be <= bill due amount
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, d_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, d_iflistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TAX_FLAGS, d_iflistp, PIN_FLD_FLAGS, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, d_iflistp, PIN_FLD_DESCR, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ITEM_ADJUSTMENT input flist", d_iflistp );

		PCM_OP(ctxp, PCM_OP_AR_ITEM_ADJUSTMENT, 0, d_iflistp, &s_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ITEM_ADJUSTMENT output flist", s_oflistp );
		PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				 {
					PIN_ERRBUF_CLEAR(ebufp);
					status = FAILURE;
					result_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Item Adjustment Failed", ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53082" , ebufp);
					
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ITEM_ADJUSTMENT Fail outflist flist", result_flistp);
					*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
					goto CLEANUP;
				 }  
				

				output_result = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_RESULT, 1, ebufp );
				if(*output_result == 0) //It failed
				{
					status = FAILURE;
					result_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
					PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53082" , ebufp);
					
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ITEM_ADJUSTMENT Fail outflist flist", result_flistp);
					*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
					goto CLEANUP;

				}
				else
				{
					//mso_op_adjustment_gen_lc_event(ctxp, i_flistp, s_oflistp, ITEM_ADJUSTMENT, ebufp);
					PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ORG_DISPLAY_NAME, ITEM_ADJUSTMENT, ebufp);
                        		//if (serv_type && *serv_type == SERV_TYPE_BB)
                        		//{
                                		fm_mso_ar_create_adjustment(ctxp, i_flistp, s_oflistp, serv_type, &ret_flistp, ebufp);
				      		if (PIN_ERRBUF_IS_ERR(ebufp) || ret_flistp == NULL)
                                		{
                                        		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                                                        	"Adjustment Failure in create_adjustment obj");
                                        		PIN_ERRBUF_CLEAR(ebufp);
                                        		result_flistp = PIN_FLIST_CREATE(ebufp);
                                        		status = FAILURE;
                                        		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
                                        		PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp,
                                                                       	PIN_FLD_ERROR_DESCR, ebufp);
                                        		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, 
									result_flistp, PIN_FLD_POID, ebufp);
                                        		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53082" , ebufp);
                                        		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                                		"ADJUSTMENT Fail outflist flist", result_flistp);
                                        		*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
                                        		PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
                                        		goto CLEANUP;
                                		}
						status = *(int32 *)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
						if(status == 1)
						{
							*r_flistpp=PIN_FLIST_COPY(ret_flistp, ebufp);
							goto CLEANUP;
						}
                        		//}

					fm_mso_create_lifecycle_evt(ctxp, i_flistp, s_oflistp, ID_ADJUSTMENT, ebufp);

					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Adjustment Failure in Lifecycle");
						PIN_ERRBUF_CLEAR(ebufp);
						result_flistp = PIN_FLIST_CREATE(ebufp);
						status = FAILURE;
						PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
						PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53082" , ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail outflist flist", result_flistp);
						*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
						PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
						goto CLEANUP;
					}

					// Preparing output flist for item adjustment
					result_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, "Adjustment Success:", ebufp);
					status = SUCCESS;
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
				
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ITEM_ADJUSTMENT outflist flist", result_flistp);
					*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
					goto CLEANUP;
				}
	}


	if(*(flag_adj_type)==2) // then do event adjustment
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside event adjustment ");

		temp1_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, 0, ebufp );
		temp11_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ADJUSTMENT_INFO, PIN_ELEMID_ANY, 0, ebufp );

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53080", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "EVENT_ADJUSTMENT Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		}
		
		//Input Flist mandatory fields check
		
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - Event poid missing",ebufp);
			goto CLEANUP;
			}
		ip_amount = PIN_FLIST_FLD_GET(temp11_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - adjustment Amount missing",ebufp);
			goto CLEANUP;
			}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - PIN_FLD_PROGRAM_NAME missing",ebufp);
			goto CLEANUP;
			}
				
		// Debit adjustments are not allowed for BB
		if(*serv_type == SERV_TYPE_BB && pbo_decimal_sign(ip_amount, ebufp) == 1)
		{
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Debit Adjustments are not allowed for BB", ebufp);
			goto CLEANUP;
		}
					   
		d_iflistp = PIN_FLIST_CREATE(ebufp);	
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, d_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, d_iflistp, PIN_FLD_DESCR, ebufp);
		temp_flistp = PIN_FLIST_ELEM_ADD(d_iflistp, PIN_FLD_ADJUSTMENT_INFO, 0, ebufp );
			PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_RESOURCE_ID, &reso_id, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TAX_FLAGS, temp_flistp, PIN_FLD_FLAGS, ebufp);
//			PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_FLAGS, &tax_adj_type, ebufp);
			PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_AMOUNT, PIN_FLIST_FLD_GET(temp11_flistp, PIN_FLD_AMOUNT, 0, ebufp), ebufp);
		temp2_flistp = PIN_FLIST_ELEM_ADD(d_iflistp, PIN_FLD_EVENTS, 0, ebufp );
			PIN_FLIST_FLD_SET(temp2_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(temp1_flistp, PIN_FLD_POID, 0, ebufp), ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, d_iflistp, PIN_FLD_DESCR, ebufp);
			

		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_EVENT_ADJUSTMENT input flist", d_iflistp );

		PCM_OP(ctxp, PCM_OP_AR_EVENT_ADJUSTMENT, 0, d_iflistp, &s_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_EVENT_ADJUSTMENT output flist", s_oflistp );
		PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);

				if (PIN_ERRBUF_IS_ERR(ebufp))
				 {
					PIN_ERRBUF_CLEAR(ebufp);
					status = FAILURE;
					result_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
					PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53083" , ebufp);
					
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "EVENT_ADJUSTMENT Fail outflist flist", result_flistp);
					*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
					goto CLEANUP;
				 }  
				
				output_result = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_RESULT, 1, ebufp );
				if(*output_result!=1) //It failed
				{
					status = FAILURE;
					result_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Event Adjustment Failed", ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53083" , ebufp);
					
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "EVENT_ADJUSTMENT Fail outflist flist", result_flistp);
					*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
					goto CLEANUP;

				}
				else
				{
					//mso_op_adjustment_gen_lc_event(ctxp, i_flistp, s_oflistp, EVENT_ADJUSTMENT, ebufp);
					PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ORG_DISPLAY_NAME, EVENT_ADJUSTMENT, ebufp);
                                       // if (serv_type && *serv_type == SERV_TYPE_BB)
                                       // {
                                                fm_mso_ar_create_adjustment(ctxp, i_flistp, s_oflistp, serv_type, &ret_flistp, ebufp);
                                                if (PIN_ERRBUF_IS_ERR(ebufp) || ret_flistp == NULL)
                                                {
                                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                                                                "Adjustment Failure in create_adjustment obj");
                                                        PIN_ERRBUF_CLEAR(ebufp);
                                                        result_flistp = PIN_FLIST_CREATE(ebufp);
                                                        status = FAILURE;
                                                        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
                                                        PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp,
                                                                        PIN_FLD_ERROR_DESCR, ebufp);
                                                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID,
                                                                        result_flistp, PIN_FLD_POID, ebufp);
                                                        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53082" , ebufp);
                                                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                                                "ADJUSTMENT Fail outflist flist", result_flistp);
                                                        *r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
                                                        PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
                                                        goto CLEANUP;
                                                }
						status = *(int32 *)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
						if(status == 1)
						{
							*r_flistpp=PIN_FLIST_COPY(ret_flistp, ebufp);
							goto CLEANUP;
						}
	
                                     //   }

					fm_mso_create_lifecycle_evt(ctxp, i_flistp, s_oflistp, ID_ADJUSTMENT, ebufp);

					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Adjustment Failure in Lifecycle");
						PIN_ERRBUF_CLEAR(ebufp);
						result_flistp = PIN_FLIST_CREATE(ebufp);
						status = FAILURE;
						PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
						PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail outflist flist", result_flistp);
						*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
						PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
						goto CLEANUP;
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_EVENT_ADJUSTMENT outflist flist", s_oflistp);
					// Preparing output flist for item adjustment
					result_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, "Adjustment Success:", ebufp);
					status = SUCCESS;
					PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
				
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "EVENT_ADJUSTMENT outflist flist", result_flistp);
					*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
					PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
					goto CLEANUP;
				}
	}


	if(*(flag_adj_type)==3) // then do account adjustment
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside account adjustment ");
		
		//Input Flist mandatory fields check
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - Account poid missing",ebufp);
			goto CLEANUP;
			}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - adjustment Amount missing",ebufp);
			goto CLEANUP;
			}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - PIN_FLD_PROGRAM_NAME missing",ebufp);
			goto CLEANUP;
			}
		if(*serv_type == 1)
		{
				/* It's a Broadband service. Get the broadband balance details */
			fm_mso_utils_get_billinfo_bal_details(ctxp, a_pdp, BILLINFO_ID_BB,
								&srch_flistp, ebufp);
		}
		else if(*serv_type == 0)
		{
				/* It's a CATV service. Get the catv balance details */
			fm_mso_utils_get_billinfo_bal_details(ctxp, a_pdp, BILLINFO_ID_CATV,
								&srch_flistp, ebufp);
		}

		if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Unknown err while searching balance", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53084" , ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Unknown err while searching balance", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		}
		rslt_flistp = PIN_FLIST_ELEM_GET(srch_flistp, PIN_FLD_EXTRA_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		/*if(rslt_flistp == NULL) {
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Accout has incorrect balance details", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53084" , ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Accout has incorrect balance details", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		}*/
                if (rslt_flistp)
                        bal_grp_pdp = PIN_FLIST_FLD_GET(rslt_flistp, PIN_FLD_POID, 1, ebufp);
                else
                        bal_grp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
		
		d_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, d_iflistp, PIN_FLD_AMOUNT, ebufp); //amount entered must be <= bill due amount
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, d_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, d_iflistp, PIN_FLD_DESCR, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_DOMAIN_ID, d_iflistp, PIN_FLD_STR_VERSION, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, d_iflistp, PIN_FLD_STRING_ID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, d_iflistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_FLAGS, &tax_flag, ebufp);
		if(bal_grp_pdp)
			PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_pdp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ACCOUNT_ADJUSTMENT input flist", d_iflistp );
		PCM_OP(ctxp, PCM_OP_AR_ACCOUNT_ADJUSTMENT, 0, d_iflistp, &s_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ACCOUNT_ADJUSTMENT output flist", s_oflistp );

		PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		 {
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Account Adjustment Failed", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53084" , ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ACCOUNT_ADJUSTMENT Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		 }  

		temp_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp );
		if(PIN_ERRBUF_IS_ERR(ebufp)) //It failed
		{
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53084" , ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ACCOUNT_ADJUSTMENT Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;

		}
		else
		{
			//mso_op_adjustment_gen_lc_event(ctxp, i_flistp, s_oflistp, ACCOUNT_ADJUSTMENT, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ORG_DISPLAY_NAME, ACCOUNT_ADJUSTMENT, ebufp);
                      //  if (serv_type && *serv_type == SERV_TYPE_BB)
                      //	{
                        	fm_mso_ar_create_adjustment(ctxp, i_flistp, s_oflistp, serv_type, &ret_flistp, ebufp);
                  		if (PIN_ERRBUF_IS_ERR(ebufp) || ret_flistp == NULL)
                               	{
                               		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
                                  		"Adjustment Failure in create_adjustment obj");
                               		PIN_ERRBUF_CLEAR(ebufp);
					if(ret_flistp)
					{
						*r_flistpp=PIN_FLIST_COPY(ret_flistp, ebufp);
					}
					else
					{
                               			result_flistp = PIN_FLIST_CREATE(ebufp);
	                               		status = FAILURE;
        	                       		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
                	               		PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp,
                        	                               PIN_FLD_ERROR_DESCR, ebufp);
                               			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID,
                                        	               result_flistp, PIN_FLD_POID, ebufp);
                               			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53082" , ebufp);
                               			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                                        	         "ADJUSTMENT Fail outflist flist", result_flistp);
	                               		*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
        	                       		PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
					}
                	               	goto CLEANUP;
                             	}
				status = *(int32 *)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);
				if(status == 1)
				{
					*r_flistpp=PIN_FLIST_COPY(ret_flistp, ebufp);
					goto CLEANUP;
				}	
			

                 //	}

			fm_mso_create_lifecycle_evt(ctxp, i_flistp, s_oflistp, ID_ADJUSTMENT, ebufp);
			
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Adjustment Failure in Lifecycle");
				PIN_ERRBUF_CLEAR(ebufp);
				result_flistp = PIN_FLIST_CREATE(ebufp);
				status = FAILURE;
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
				PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53081" , ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail outflist flist", result_flistp);
				*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
				goto CLEANUP;
			}

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ACCOUNT_ADJUSTMENT outflist flist", s_oflistp);
			// Preparing output flist for item adjustment
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, "Account Adjustment Success:", ebufp);
			status = SUCCESS;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ACCOUNT_ADJUSTMENT outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			goto CLEANUP;
		}
	}

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, 
			"Mandatory fields missing/Debit adjustment Not allowed Error", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53080" , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail output flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
		}
		else if( pbo_decimal_sign(ip_amount, ebufp) == 0 ){
			result_flistp = PIN_FLIST_CREATE(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR,
				"Zero amount adjustment is not allowed", ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE,"53087" , ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ADJUSTMENT Fail output flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
		}

		if (status == FAILURE)
		{
			if(local == LOCAL_TRANS_OPEN_SUCCESS )
			{
				fm_mso_trans_abort(ctxp, a_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Aborted");
			}
		}
		else
		{
			if(local == LOCAL_TRANS_OPEN_SUCCESS )
			{
				fm_mso_trans_commit(ctxp, a_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Committed");
			}
		}

	return;
}



/************************************************
* fm_mso_check_adj_limit()
* Harish Kulkarni
* Function to check the limit of the CSR to make the
* adjustments for the customer.
*************************************************/
static void fm_mso_check_csr_adj_limit(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	pin_decimal_t		*ip_adj_amount,
	int			*trans_breaced,
	int			*monthly_breached,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*mso_ar_limitp = NULL;
	pin_flist_t		*mso_adj_arrp = NULL;
	poid_t			*mso_ar_limit_pdp = NULL;
	pin_decimal_t		*trans_olimit = NULL;
	pin_decimal_t		*mnthly_olimit = NULL;
	pin_decimal_t		*current_mnth_ovalue = NULL;
	pin_decimal_t		*trans_limit = NULL;
	pin_decimal_t		*mnthly_limit = NULL;
	pin_decimal_t		*current_mnth_value = NULL;
	pin_decimal_t		*adj_amount = NULL;
	time_t			*mso_current_mnth = NULL;
	time_t			current_pvt = pin_virtual_time(NULL);
	time_t			first_of_month = 0;


	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_check_csr_adj_limit");

	/*****************************************************
	* Search the /mso_ar_limit to get the adjustment limit
	******************************************************/
	fm_mso_search_mso_ar_limit(ctxp, acct_pdp, &mso_ar_limitp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp) || mso_ar_limitp == NULL)
		goto CLEANUP;

	mso_adj_arrp = PIN_FLIST_ELEM_GET(mso_ar_limitp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (mso_adj_arrp == NULL)
		goto CLEANUP;

	mso_ar_limit_pdp = PIN_FLIST_FLD_GET(mso_adj_arrp, PIN_FLD_POID, 0, ebufp);
	trans_olimit = PIN_FLIST_FLD_GET(mso_adj_arrp, MSO_FLD_ADJ_LIMIT, 0, ebufp);
	mnthly_olimit = PIN_FLIST_FLD_GET(mso_adj_arrp, MSO_FLD_MONTHLY_ADJ_LIMIT, 0, ebufp);
	current_mnth_ovalue = PIN_FLIST_FLD_GET(mso_adj_arrp, MSO_FLD_ADJ_CURRENT_VALUE, 1, ebufp);
	mso_current_mnth = PIN_FLIST_FLD_GET(mso_adj_arrp, MSO_FLD_CURRENT_MONTH, 1, ebufp);

	/* Make sure mandatory fields are present in the /mso_ar_limit for each CSR accout */
	if (PIN_ERRBUF_IS_ERR(ebufp))
		goto CLEANUP; 

	/* Get the absolute values of the decimals */
	if(! pbo_decimal_is_null(trans_olimit, ebufp))
		trans_limit = pbo_decimal_abs(trans_olimit, ebufp);
	if(! pbo_decimal_is_null(mnthly_olimit, ebufp))
		mnthly_limit = pbo_decimal_abs(mnthly_olimit, ebufp);
	if(! pbo_decimal_is_null(current_mnth_ovalue, ebufp))
		current_mnth_value = pbo_decimal_abs(current_mnth_ovalue, ebufp);
	if(! pbo_decimal_is_null(ip_adj_amount, ebufp))
		adj_amount = pbo_decimal_abs(ip_adj_amount, ebufp);

	/* Set the trans_limit flag if transaction amount is more than transaction limit */
	if ( (! pbo_decimal_is_null(trans_limit, ebufp)) &&
		pbo_decimal_compare(adj_amount, trans_limit, ebufp) > 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction Limit breached");
		*trans_breaced = 1;
		goto CLEANUP;
	}

	/* Call the fucntion to get the unix time for 1st of the current month */
	fm_mso_return_first_of_month(current_pvt, &first_of_month);

	/* If it's a first time adjustment done by CSR */
	if (mso_current_mnth == (time_t *)NULL || *mso_current_mnth == 0 )
	{

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CSR is doing very first transaction");
		if ( (!pbo_decimal_is_null(mnthly_limit, ebufp))&&
			pbo_decimal_compare(adj_amount, mnthly_limit, ebufp) > 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Monthly Limit breached");
			*monthly_breached = 1;
			goto CLEANUP;
		}
		else
		{
			/***************************************************************
			* Set the current month 1st and monthly adjustment 
			* aggregted values to the /mso_ar_limit for the current CSR 
			***************************************************************/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating the /mso_ar_limit for the current CSR");
			fm_mso_update_mso_ar_limit(ctxp, mso_ar_limit_pdp, first_of_month, 
							pbo_decimal_negate(adj_amount, ebufp), ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
				goto CLEANUP;
		}
	}
	else
	{

		if (*mso_current_mnth != first_of_month)
		{ /* If the adjustment done is in the new month */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CSR is doing first transaction in new month");
			if ( (!pbo_decimal_is_null(mnthly_limit, ebufp)) &&
				pbo_decimal_compare(adj_amount, mnthly_limit, ebufp) > 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Monthly Limit breached");
				*monthly_breached = 1;
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating the /mso_ar_limit for the current CSR");
				fm_mso_update_mso_ar_limit(ctxp, mso_ar_limit_pdp, first_of_month, 
							pbo_decimal_negate(adj_amount, ebufp), ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
					goto CLEANUP;
			}
		}
		else
		{/* If the adjustment done is in the same month */
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CSR has done trnsactions in this month already");
			if (pbo_decimal_is_null(current_mnth_value, ebufp))
				current_mnth_value = pbo_decimal_from_str("0.0", ebufp);

			if (    (!pbo_decimal_is_null(mnthly_limit, ebufp)) && 
				(!pbo_decimal_is_null(current_mnth_value, ebufp)) &&
				pbo_decimal_compare(
				pbo_decimal_add(adj_amount, current_mnth_value, ebufp), mnthly_limit, ebufp) > 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Monthly Limit breached");
				*monthly_breached = 1;
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Updating the /mso_ar_limit for the current CSR");
				fm_mso_update_mso_ar_limit(ctxp, mso_ar_limit_pdp, first_of_month, 
					pbo_decimal_negate(pbo_decimal_add(adj_amount, current_mnth_value, ebufp), ebufp),
					ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
					goto CLEANUP;
			}
		}
	}

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&mso_ar_limitp, NULL);

		if(!pbo_decimal_is_null(trans_limit, ebufp))
			pbo_decimal_destroy(&trans_limit);
		if(!pbo_decimal_is_null(mnthly_limit, ebufp))
			pbo_decimal_destroy(&mnthly_limit);
		if(!pbo_decimal_is_null(current_mnth_value, ebufp))
			pbo_decimal_destroy(&current_mnth_value);
		if(!pbo_decimal_is_null(adj_amount, ebufp))
			pbo_decimal_destroy(&adj_amount);

	return;
}

/************************************************
* Function to update the /mso_ar_limit object
*************************************************/
static void
fm_mso_update_mso_ar_limit(
	pcm_context_t		*ctxp,
	poid_t			*mso_limit_pdp,
	time_t			first_of_month,
	pin_decimal_t		*adj_amount,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*wf_iflistp = NULL;
	pin_flist_t		*wf_oflistp = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_search_mso_ar_limit");

	wf_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(wf_iflistp, PIN_FLD_POID, mso_limit_pdp, ebufp);
	PIN_FLIST_FLD_SET(wf_iflistp, MSO_FLD_ADJ_CURRENT_VALUE, adj_amount, ebufp);
	PIN_FLIST_FLD_SET(wf_iflistp, MSO_FLD_CURRENT_MONTH, &first_of_month, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_mso_ar_limit write_flds input:", wf_iflistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wf_iflistp, &wf_oflistp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_mso_ar_limit write_flds output:", wf_oflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		goto CLEANUP;

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&wf_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&wf_oflistp, NULL);
	return;
}

/************************************************
* fm_mso_return_first_of_month()
* Harish Kulkarni
* Function to return the first date of the month 
* in Unix time.
*************************************************/
static void fm_mso_return_first_of_month(
	time_t		adj_date,
	time_t		*ret_date
)
{
	int		years,months,days;
	struct tm	*current_t;
	struct tm	first_mnth_t;
	time_t		new_time;
	char		tmp[512];

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_return_first_of_month");

	current_t = localtime(&adj_date);
	years = current_t->tm_year;
	months = current_t->tm_mon;
	days = current_t->tm_mday;

	sprintf(tmp, "Year = %d, Month = %d, Days = %d", years,months,days);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, tmp);

	first_mnth_t.tm_year = years;
	first_mnth_t.tm_mon = months;
	first_mnth_t.tm_mday = 1;
	first_mnth_t.tm_hour  = 0;
	first_mnth_t.tm_min  = 0;
	first_mnth_t.tm_sec = 0;
	first_mnth_t.tm_isdst = -1;

	*ret_date = mktime(&first_mnth_t);

	return;
}

/************************************************
* fm_mso_search_mso_ar_limit()
* Harish Kulkarni
* Function to get the /mso_ar_limit based on the
* CSR account poid.
*************************************************/
static void fm_mso_search_mso_ar_limit(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	pin_flist_t		**mso_ar_limitpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*rslt_flisp = NULL;
	pin_flist_t		*s_oflistp = NULL;
	poid_t			*s_pdp = NULL;
	int32			s_flags = 256;
	int64			db = 0;
	char			*s_template = " select X from /mso_ar_limit where F1 = V1 ";
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_search_mso_ar_limit");
	
	s_flistp = PIN_FLIST_CREATE(ebufp);
	db = PIN_POID_GET_DB(acct_pdp);
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	rslt_flisp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_mso_ar_limit search input:", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_oflistp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_mso_ar_limit search output:", s_oflistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
		goto CLEANUP;

	*mso_ar_limitpp = PIN_FLIST_COPY(s_oflistp, ebufp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
	return;
}

void fm_mso_ar_create_adjustment(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*res_flistp,
	int32			*service_type,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp) 
{
	poid_t			*acct_pdp = NULL;
	poid_t			*bill_pdp = NULL;
	poid_t			*ev_pdp = NULL;
	
	
	pin_flist_t		*get_bills_out_flistp = NULL;
	pin_flist_t		*bill_flistp = NULL;
	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*tax_flistp = NULL;
	pin_flist_t		*bill_details_flistp = NULL;
	pin_flist_t		*evt_flistp = NULL;
	pin_flist_t		*reslt_flistp = NULL;
	pin_flist_t             *read_iflistp = NULL;
         pin_flist_t             *read_oflistp = NULL;
	pin_flist_t		*acct_info_flistp = NULL;
	pin_flist_t		*rcpt_flistp = NULL;
	
	char			*bill_no = NULL;
	char			*legacy_account_no = NULL;
	char			*tax_code = NULL;
	char			*rate_tag = NULL;
        char                            *acct_type = NULL;
	
	int32			elem_id = 0;
	int32			i_elem_id = 0;
	int32			got_bill = -1;
	int32			failure = 1;
	int32			success = 0;
	int32 			is_migrated = 0;
	int32			*is_pre_gst = NULL;
	int32			res_id = 356;
    int32           is_done = 0;

	pin_cookie_t		cookie = NULL;
	pin_cookie_t		i_cookie = NULL;
	
	pin_decimal_t		*bill_due = NULL;
	pin_decimal_t		*remaining_bal = NULL;
	pin_decimal_t		*zero = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*amount = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*unallocated_bal = NULL;
	pin_decimal_t		*orig_rem_amount = NULL;
	pin_decimal_t		*adjusted_amount = NULL;
	pin_decimal_t		*temp_amt = NULL;
	pin_decimal_t		*percent = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*temp_percent = NULL;
	pin_decimal_t		*tol = pbo_decimal_from_str("0.02", ebufp);

//	int32                   precision = 2;
//	int32                   round_mode = ROUND_UNNECESSARY;

	time_t			*bill_date = NULL;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating Adjustment");

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "input flistp", i_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "res flistp", res_flistp);

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

        acct_type = (char *)PIN_POID_GET_TYPE(acct_pdp);
        if (acct_type && ((strcmp(acct_type,"/account")==0))) {
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "inside account obj!  ...");
        }
        else {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "outside account!  ...");
        acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        }

	fm_mso_get_account_info(ctxp, acct_pdp, &acct_info_flistp, ebufp);
	legacy_account_no = PIN_FLIST_FLD_GET(acct_info_flistp, MSO_FLD_LEGACY_ACCOUNT_NO, 1, ebufp);
	if(service_type && *service_type == SERV_TYPE_BB && legacy_account_no && strcmp(legacy_account_no, "") != 0)
	{
		is_migrated  = 1;
	}
	bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO, 1, ebufp);
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	rcpt_flistp = PIN_FLIST_CREATE(ebufp);
	
        reslt_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

        if(reslt_flistp)
        {
				if (PIN_FLIST_ELEM_COUNT(reslt_flistp, PIN_FLD_BAL_IMPACTS, ebufp) == 0)	 {
						ev_pdp = PIN_FLIST_FLD_GET(reslt_flistp, PIN_FLD_EVENT_OBJ, 0, ebufp);
						read_iflistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_SET(read_iflistp, PIN_FLD_POID, ev_pdp, ebufp);

						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Event read input list", read_iflistp);
						PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_iflistp, &read_oflistp, ebufp);
						PIN_FLIST_DESTROY_EX(&read_iflistp, NULL);	
						reslt_flistp=PIN_FLIST_COPY(read_oflistp, ebufp);

				}
	
	
                while ((evt_flistp = PIN_FLIST_ELEM_GET_NEXT(reslt_flistp, PIN_FLD_BAL_IMPACTS, &elem_id, 1, &cookie, ebufp))!= NULL)
                {
                        PIN_ERR_LOG_MSG(3, "check 1");
                        tax_code = PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
                        temp_amt = PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                        rate_tag = PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_RATE_TAG, 0, ebufp);
                        res_id = *(int32 *) PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_RESOURCE_ID, 0, ebufp);

						if (res_id != 356)
						{
							continue;
						}

                        if(rate_tag && strcmp(rate_tag, "Tax") == 0 && tax_code && strcmp(tax_code, "CGST") == 0)
                        {
				temp_percent = PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_PERCENT, 0, ebufp);
				pbo_decimal_add_assign(percent, temp_percent, ebufp);
				PIN_FLIST_FLD_COPY(evt_flistp, PIN_FLD_PERCENT, rcpt_flistp, MSO_FLD_CGST_RATE, ebufp);	
                        }
                        else if(rate_tag && strcmp(rate_tag, "Tax") == 0 && tax_code && strcmp(tax_code, "SGST") == 0)
                        {
                                temp_percent = PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_PERCENT, 0, ebufp);
                                pbo_decimal_add_assign(percent, temp_percent, ebufp);
				PIN_FLIST_FLD_COPY(evt_flistp, PIN_FLD_PERCENT, rcpt_flistp, MSO_FLD_SGST_RATE, ebufp);
                        }
                        else if(rate_tag && strcmp(rate_tag, "Tax") == 0 && tax_code && strcmp(tax_code, "IGST") == 0)
                        {
                                temp_percent = PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_PERCENT, 0, ebufp);
                                pbo_decimal_add_assign(percent, temp_percent, ebufp);
				PIN_FLIST_FLD_COPY(evt_flistp, PIN_FLD_PERCENT, rcpt_flistp, MSO_FLD_IGST_RATE, ebufp);
                        }
			PIN_FLIST_FLD_PUT(rcpt_flistp, MSO_FLD_TAX_RATE, percent, ebufp);
                       	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(temp_amt, ebufp));
                        if(!pbo_decimal_is_null(temp_amt, ebufp))
                                pbo_decimal_add_assign(amount, temp_amt, ebufp);
                }
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(amount, ebufp));

	fm_mso_part_receipt_no(ctxp, i_flistp, service_type, amount, &rcpt_flistp, ebufp);
	
	if(bill_no)
	{
		fm_mso_get_bills(ctxp, bill_no, acct_pdp, &get_bills_out_flistp, ebufp);
		bill_flistp = PIN_FLIST_ELEM_GET(get_bills_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if(!bill_flistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Bill Not Found");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "61011", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Bill Not Found", ebufp);
			goto CLEANUP;
		}
		bill_due = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_CURRENT_TOTAL, 0, ebufp);
		bill_pdp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_POID, 0, ebufp);
		bill_date = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_END_T, 0, ebufp);
		is_pre_gst = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_FLAGS, 1, ebufp);
        fm_mso_utils_get_open_bill_items(ctxp, bill_pdp, 1, &item_flistp, ebufp);
        i_cookie = NULL;
        i_elem_id = 0;
        PIN_ERR_LOG_FLIST(3, "Bill items flist", item_flistp);
        while((tax_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp, PIN_FLD_TAXES,
            &i_elem_id, 1, &i_cookie, ebufp)) != (pin_flist_t *)NULL)
        {
            PIN_FLIST_ELEM_SET(bill_flistp, tax_flistp, PIN_FLD_TAXES, i_elem_id, ebufp);
        }
		fm_mso_get_bill_amount_available(ctxp, bill_flistp, &bal_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp) || bal_flistp == NULL)
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error getting bill amount");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51001", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error getting bill amount", ebufp);
			goto CLEANUP;
		}
        i_cookie = NULL;
        i_elem_id = 0;
		remaining_bal = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
        while ((tax_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_TAXES,
                &i_elem_id, 1, &i_cookie, ebufp)) != (pin_flist_t *)NULL)
        {
        remaining_bal = PIN_FLIST_FLD_GET(tax_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);    
        tax_code = PIN_FLIST_FLD_GET(tax_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
		if(!pbo_decimal_is_null(amount, ebufp) && pbo_decimal_sign(amount, ebufp) == -1)
		{
			if (pbo_decimal_compare(remaining_bal, zero, ebufp) <= 0 || pbo_decimal_compare(pbo_decimal_negate(amount, ebufp),
													 pbo_decimal_add(remaining_bal, tol, ebufp), ebufp) >= 1)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Amount is greator than invoice available due");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "61001", ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Amount is greator than invoice available due", ebufp);
				PIN_FLIST_DESTROY_EX(&bal_flistp, NULL);
				goto CLEANUP;
			}
			else
			{
				got_bill = 1;
			}
		}
		else
		{
			got_bill = 1;
		}
		
		if(got_bill == 1)
		{
			unallocated_bal = pbo_decimal_copy(zero, ebufp);
			pbo_decimal_add_assign(unallocated_bal, pbo_decimal_add(remaining_bal, amount, ebufp), ebufp);
			bill_details_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_POID, bill_pdp, ebufp);
			PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_BILL_NO, bill_no, ebufp);
			PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_BILL_DATE_T, bill_date, ebufp);
			PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_CURRENT_BAL, unallocated_bal, ebufp);
			PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_ADJUSTED, amount, ebufp);
			PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_FLAGS, is_pre_gst, ebufp);
			PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_TAX_CODE, tax_code, ebufp);
			fm_mso_ar_create_adjustment_object(ctxp, i_flistp, res_flistp, bill_details_flistp, rcpt_flistp, service_type, amount, NULL, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error creating Adjustment LC");
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51003", ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error creating Adjustment LC", ebufp);
				goto CLEANUP;
			}
			pbo_decimal_destroy(&unallocated_bal);
			PIN_FLIST_DESTROY_EX(&bill_details_flistp, NULL);
		}
	}/* while loop */
    }
	else
	{
		PIN_ERR_LOG_MSG(3, "check 2");
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		//get latest unallocated Bill
		fm_mso_get_bills(ctxp, NULL, acct_pdp, &get_bills_out_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error searching Bills");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51001", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error getting Bill for Allocation", ebufp);
			goto CLEANUP;
		}
		if(pbo_decimal_sign(amount, ebufp) == -1)
		{
			PIN_ERR_LOG_MSG(3, "check 3");
			orig_rem_amount = pbo_decimal_copy(amount, ebufp);
			elem_id = 0;
			cookie = NULL;
			while ((bill_flistp = PIN_FLIST_ELEM_GET_NEXT(get_bills_out_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp))!= NULL)
			{
				PIN_ERR_LOG_MSG(3, "check 4");
				bill_due = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_CURRENT_TOTAL, 0, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(bill_due, ebufp));
				bill_pdp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_POID, 0, ebufp);
				bill_no = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_BILL_NO, 0, ebufp);
				bill_date = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_END_T, 0, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bill_no);
                fm_mso_utils_get_open_bill_items(ctxp, bill_pdp, 1, &item_flistp, ebufp);
			    PIN_ERR_LOG_FLIST(3, "Bill items flist", item_flistp);
                if (item_flistp == NULL)
                {
                    continue;
                }
                i_cookie = NULL;
                i_elem_id = 0;
                while((tax_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp, PIN_FLD_TAXES,
                        &i_elem_id, 1, &i_cookie, ebufp)) != (pin_flist_t *)NULL)
                {
                    PIN_FLIST_ELEM_SET(bill_flistp, tax_flistp, PIN_FLD_TAXES, i_elem_id, ebufp);
                }
				//get current available amount for allocation of the bill
				fm_mso_get_bill_amount_available(ctxp, bill_flistp, &bal_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp) || bal_flistp == NULL)
				{
					PIN_ERRBUF_CLEAR(ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error getting amount available");
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51001", ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error getting bill amount", ebufp);
					goto CLEANUP;
				}
                i_cookie = NULL;
                i_elem_id = 0;
                while ((tax_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_TAXES,
                    &i_elem_id, 1, &i_cookie, ebufp)) != (pin_flist_t *)NULL)
                {
                remaining_bal = PIN_FLIST_FLD_GET(tax_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);    
                tax_code = PIN_FLIST_FLD_GET(tax_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
				is_pre_gst = PIN_FLIST_FLD_GET(tax_flistp, PIN_FLD_FLAGS, 1, ebufp);
				if (remaining_bal && !pbo_decimal_is_null(remaining_bal, ebufp) && pbo_decimal_compare(remaining_bal, zero, ebufp) >= 1)
				{
					got_bill = 1;
					if (orig_rem_amount && pbo_decimal_compare(pbo_decimal_negate(orig_rem_amount, ebufp), remaining_bal, ebufp) < 0)
					{
						adjusted_amount = pbo_decimal_copy(orig_rem_amount, ebufp);
                        is_done = 1;
					}
					else
					{
						adjusted_amount = pbo_decimal_negate(remaining_bal, ebufp);
					}
					//create adjustment object
					unallocated_bal = pbo_decimal_copy(zero, ebufp);
					if(pbo_decimal_compare(pbo_decimal_negate(orig_rem_amount, ebufp), remaining_bal, ebufp) < 0)
					{
						PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(unallocated_bal, ebufp));
						PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(remaining_bal, ebufp));							
						PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(adjusted_amount, ebufp));
						pbo_decimal_add_assign(unallocated_bal, pbo_decimal_add(remaining_bal, adjusted_amount, ebufp), ebufp);
					}
					bill_details_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_POID, bill_pdp, ebufp);
					PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_BILL_NO, bill_no, ebufp);
					PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_BILL_DATE_T, bill_date, ebufp);
					PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_CURRENT_BAL, unallocated_bal, ebufp);
					PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_ADJUSTED, adjusted_amount, ebufp);
					PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_FLAGS, is_pre_gst, ebufp);
			        PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_TAX_CODE, tax_code, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"check3");
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "receipt output list", rcpt_flistp);
					fm_mso_ar_create_adjustment_object(ctxp, i_flistp, res_flistp, bill_details_flistp, rcpt_flistp, service_type, amount, NULL, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp))
					{
						PIN_ERRBUF_CLEAR(ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error creating Adjustment LC");
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51003", ebufp);
						PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error creating Adjustment LC", ebufp);
						goto CLEANUP;
					}
					pbo_decimal_add_assign(orig_rem_amount, remaining_bal, ebufp);
					PIN_FLIST_DESTROY_EX(&bill_details_flistp, NULL);
					pbo_decimal_destroy(&unallocated_bal);
					pbo_decimal_destroy(&adjusted_amount);
				}
				if (orig_rem_amount && pbo_decimal_compare(orig_rem_amount, zero, ebufp) >= 0)
				{
					break;
				}
			} /* inner while loop */
             if (is_done)
             {
                  PIN_ERR_LOG_MSG(3, "Its done");
                  break;
             }
            } /* outer while loop */
			//pbo_decimal_round_assign(orig_rem_amount, precision, round_mode, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(orig_rem_amount, ebufp));

			if(!pbo_decimal_is_null(orig_rem_amount, ebufp) && pbo_decimal_compare(orig_rem_amount, zero, ebufp) == -1)
			{
				PIN_ERR_LOG_MSG(3, "Still Amount Left");
				if(is_migrated == 1)
				{
					PIN_ERR_LOG_MSG(3, "migrated customer");
					fm_mso_ar_create_adjustment_object(ctxp, i_flistp, res_flistp, NULL, rcpt_flistp, service_type, amount, orig_rem_amount, ebufp);
				}
				else
				{
					PIN_ERR_LOG_MSG(3, "non migrated customer");
					got_bill = -1;
				}
				
			}
		}
		else
		{
			bill_flistp = PIN_FLIST_ELEM_GET(get_bills_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			if(bill_flistp)
			{
				bill_due = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_CURRENT_TOTAL, 0, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(bill_due, ebufp));
				bill_pdp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_POID, 0, ebufp);
				bill_no = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_BILL_NO, 0, ebufp);
				bill_date = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_END_T, 0, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bill_no);
                fm_mso_utils_get_open_bill_items(ctxp, bill_pdp, 1, &item_flistp, ebufp);
                i_cookie = NULL;
                i_elem_id = 0;
			    PIN_ERR_LOG_FLIST(3, "Bill items flist", item_flistp);
                while((tax_flistp = PIN_FLIST_ELEM_GET_NEXT(item_flistp, PIN_FLD_TAXES,
                        &i_elem_id, 1, &i_cookie, ebufp)) != (pin_flist_t *)NULL)
                {
                    PIN_FLIST_ELEM_SET(bill_flistp, tax_flistp, PIN_FLD_TAXES, i_elem_id, ebufp);
                }
				//get current available amount for allocation of the bill
				fm_mso_get_bill_amount_available(ctxp, bill_flistp, &bal_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp) || bal_flistp == NULL)
				{
					PIN_ERRBUF_CLEAR(ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error getting amount left");
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51001", ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error getting bill amount", ebufp);
					goto CLEANUP;
				}
                i_cookie = NULL;
                i_elem_id = 0;
                while ((tax_flistp = PIN_FLIST_ELEM_GET_NEXT(bal_flistp, PIN_FLD_TAXES,
                    &i_elem_id, 1, &i_cookie, ebufp)) != (pin_flist_t *)NULL)
                {
				remaining_bal = PIN_FLIST_FLD_GET(tax_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
                tax_code = PIN_FLIST_FLD_GET(tax_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
                if (strcmp(tax_code, "9984") != 0)
                {
                    continue;
                }
				is_pre_gst = PIN_FLIST_FLD_GET(tax_flistp, PIN_FLD_FLAGS, 1, ebufp);
				unallocated_bal = pbo_decimal_copy(zero, ebufp);
				if(!pbo_decimal_is_null(remaining_bal, ebufp))
				{
					pbo_decimal_add_assign(unallocated_bal, pbo_decimal_add(remaining_bal, amount, ebufp), ebufp);
				}
				bill_details_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_POID, bill_pdp, ebufp);
				PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_BILL_NO, bill_no, ebufp);
				PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_BILL_DATE_T, bill_date, ebufp);
				PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_CURRENT_BAL, unallocated_bal, ebufp);
				PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_ADJUSTED, amount, ebufp);
				PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_FLAGS, is_pre_gst, ebufp);
			    PIN_FLIST_FLD_SET(bill_details_flistp, PIN_FLD_TAX_CODE, tax_code, ebufp);
				fm_mso_ar_create_adjustment_object(ctxp, i_flistp, res_flistp, bill_details_flistp, rcpt_flistp, service_type, amount, NULL, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERRBUF_CLEAR(ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error creating Adjustment LC");
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "51003", ebufp);
					PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error creating Adjustment LC", ebufp);
					goto CLEANUP;
				}
				pbo_decimal_destroy(&unallocated_bal);
				PIN_FLIST_DESTROY_EX(&bill_details_flistp, NULL);
				got_bill = 1;
			}
		} /* innner while */
        } /* else*/
	}
	if(got_bill == -1 && is_migrated == 1)
	{
		
	}
	else if (got_bill == -1 && is_migrated == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No Bills for allocation");
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "61002", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "No Bills for allocation", ebufp);
		goto CLEANUP;
	}

	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &success, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "00", ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Success", ebufp);	

CLEANUP:
	if(zero && !pbo_decimal_is_null(zero, ebufp))
	{
		pbo_decimal_destroy(&zero);
	}
	if(tol && !pbo_decimal_is_null(tol, ebufp))
	{
		pbo_decimal_destroy(&tol);
	}
	PIN_FLIST_DESTROY_EX(&get_bills_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rcpt_flistp, NULL);
	return;
}

void
fm_mso_get_bill_amount_available(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*srch_in_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*profile_substr = NULL;
	pin_flist_t		*nameinfo_flistp = NULL;
	pin_flist_t		*taxes_oflistp = NULL;
	pin_flist_t		*taxes_flist = NULL;
    pin_cookie_t    cookie = NULL;   
    pin_cookie_t    i_cookie = NULL;   
	int32			srch_flag = 256;
    int32           rec_id = 0;
    int32           found = 0;
	int32			flags;

	poid_t			*srch_pdp = NULL;
	poid_t			*lco_obj = NULL;
	poid_t			*bill_pdp = NULL;

	pin_decimal_t		*bill_due = NULL;
	pin_decimal_t		*calc_due = NULL;
	pin_decimal_t		*calc_per = pbo_decimal_from_str("115.0",ebufp);
	pin_decimal_t		*hundred = pbo_decimal_from_str("100.0", ebufp);
	pin_decimal_t		*current_bal = NULL;
	pin_decimal_t		*subord_total = NULL;
	pin_decimal_t		*curr_total = NULL;

	char			*template = "select X from /mso_adjustment where F1 = V1 order by hsn_code, created_t desc ";
	char			*bill_no = NULL;
	char			*bname = NULL;
    char            *tax_codep = NULL;
    char            *inner_tax_codep = NULL;
	int64			db = -1;
    int32           elem_id = 0;
    int32           i_elem_id = 0;
	
	time_t			*bill_date = NULL;
	int32			is_pre_gst = 0;
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_bill_amount_available", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_get_bill_amount_available", in_flistp);

	bill_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	subord_total = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SUBORDS_TOTAL, 0, ebufp);
	curr_total = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_CURRENT_TOTAL, 0, ebufp);
	//bill_due = pbo_decimal_add(subord_total, curr_total, ebufp);
	bill_date = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_END_T, 0, ebufp);
	bill_no = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILL_NO, 0, ebufp);
	bname = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_NAME, 0, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bname);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bill_no);


	if(strstr(bill_no, "I") == bill_no || (bname && strstr(bname,"PIN Bill NOW") &&  bill_date && *bill_date >= 1498847400)  )
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bname);
		flags=2;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bill_no);
	}

PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bname);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, bill_no);

	
	db = PIN_POID_GET_DB(bill_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILL_OBJ, bill_pdp, ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_amount_available search input list", srch_in_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling search", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_amount_available search output list", srch_out_flistp);

	*r_flistpp = PIN_FLIST_CREATE(ebufp);
    while ((result_flist = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp, PIN_FLD_RESULTS,
        &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
    {
		current_bal = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_CURRENT_BAL, 0, ebufp);
		tax_codep = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_TAX_CODE, 0, ebufp);
        if ((inner_tax_codep && strcmp(tax_codep, inner_tax_codep) != 0) || !inner_tax_codep)
        {
            arg_flist = PIN_FLIST_ELEM_ADD(*r_flistpp, PIN_FLD_TAXES, rec_id, ebufp);
            PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_TAX_CODE, tax_codep, ebufp);
	        if (current_bal && !pbo_decimal_is_null(current_bal, ebufp))
	        {
		        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CURRENT_BAL, current_bal, ebufp);
	        }
        }
        rec_id++;
        inner_tax_codep = tax_codep;
    } 

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_amount_available return flist 1", *r_flistpp);
    cookie = NULL;
    elem_id = 0;
    rec_id = 0;
    while ((result_flist = PIN_FLIST_ELEM_TAKE_NEXT(in_flistp, PIN_FLD_TAXES,
        &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
    {
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_amount_available inner input", result_flist);
        tax_codep = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_TAX_CODE, 0, ebufp);
        bill_due = PIN_FLIST_FLD_GET(result_flist, PIN_FLD_AMOUNT, 0, ebufp);

        i_cookie = NULL;
        i_elem_id = 0;
        found = 0;
        while ((arg_flist = PIN_FLIST_ELEM_GET_NEXT(*r_flistpp, PIN_FLD_TAXES,
            &i_elem_id, 1, &i_cookie, ebufp)) != (pin_flist_t *)NULL)
        {
            inner_tax_codep = PIN_FLIST_FLD_GET(arg_flist, PIN_FLD_TAX_CODE, 0, ebufp);
            if (strcmp(tax_codep, inner_tax_codep) == 0)
            {
                PIN_FLIST_DESTROY_EX(&result_flist, NULL);
	            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_amount_available found in mso_adj ");
                found = 1;
                rec_id++;
                break;
            }
        }
        if (found == 0)
        { 
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_amount_available not found in mso_adj ");
            if (!taxes_oflistp)
            {
                taxes_oflistp = PIN_FLIST_CREATE(ebufp);
            }
            taxes_flist = PIN_FLIST_ELEM_ADD(taxes_oflistp, PIN_FLD_TAXES, rec_id, ebufp);
            PIN_FLIST_FLD_SET(taxes_flist, PIN_FLD_TAX_CODE, tax_codep, ebufp);
            if(bill_date && *bill_date >= 1498847400 &&  (  strstr(bill_no, "I") == bill_no || flags == 2 ) )
            {
                is_pre_gst = 0;
                PIN_FLIST_FLD_SET(taxes_flist, PIN_FLD_CURRENT_BAL, bill_due, ebufp);
                PIN_FLIST_FLD_SET(taxes_flist, PIN_FLD_FLAGS, &is_pre_gst, ebufp); 
            }
            else
            {
                is_pre_gst = 1;
                calc_due = pbo_decimal_divide(bill_due, calc_per, ebufp);
                pbo_decimal_multiply_assign(calc_due, hundred, ebufp);
                PIN_FLIST_FLD_SET(taxes_flist, PIN_FLD_CURRENT_BAL, calc_due, ebufp);
                PIN_FLIST_FLD_SET(taxes_flist, PIN_FLD_FLAGS, &is_pre_gst, ebufp);
            }
            rec_id++;
            PIN_FLIST_DESTROY_EX(&result_flist, NULL);
        }
    } /* outer while */
    if (taxes_oflistp)
    {
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "taxes flistp", taxes_oflistp);
        PIN_FLIST_CONCAT(*r_flistpp, taxes_oflistp, ebufp);
        PIN_FLIST_DESTROY_EX(&taxes_oflistp, NULL);
    }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "return flistp", *r_flistpp);
	CLEANUP:

	if(!pbo_decimal_is_null(hundred, ebufp))
		pbo_decimal_destroy(&hundred);
	
	if(!pbo_decimal_is_null(calc_per, ebufp))
                pbo_decimal_destroy(&calc_per);

	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}


void fm_mso_ar_create_adjustment_object(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*res_flistp,
	pin_flist_t		*bill_details_flistp,
	pin_flist_t		*receipt_flistp,
	int32			*service_type,
	pin_decimal_t		*amount,
	pin_decimal_t		*rem_amount,
	pin_errbuf_t		*ebufp) 
{
	poid_t			*acct_pdp = NULL;
	poid_t			*user_id = NULL;
	poid_t			*rcpt_pdp = NULL;
	poid_t			*item_pdp = NULL;
	poid_t			*evt_pdp = NULL;
	poid_t			*parent_pdp = NULL;
	
	pin_flist_t		*rcpt_flistp = NULL;
	pin_flist_t		*b_flistp = NULL;
	pin_flist_t		*evt_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;

	int64			id = -1;
	int64			db = 0;
	char			adj_descr [2024] = "";
	char			charging_id [2024] = "";
	void			*vp = NULL;
	void			*vp1 = NULL;
	char			*part_receipt = NULL;
	char			receipt[20] = {0};
	char			*charge_head = NULL;
	char			*hsn_code = NULL;
	
	int32			*pp_type = NULL;
	int32			seq_no = -1;
	int32                   precision = 2;
	int32                   round_mode = ROUND_HALF_UP;
	int32			*is_pre_gst = NULL;

	pin_decimal_t		*adjusted = NULL;
	pin_decimal_t		*tax_rate = NULL;
	pin_decimal_t		*tot_percent = NULL;
	pin_decimal_t		*hundred = pbo_decimal_from_str("100.0", ebufp);
	pin_decimal_t		*one_per = NULL;
	pin_decimal_t		*base_amt = NULL;
	pin_decimal_t		*igst_tax = NULL;
	pin_decimal_t		*sgst_tax = NULL;
	pin_decimal_t		*cgst_tax = NULL;
	//Added for GST_REASON_ID update
	int32                   gst_inv_flag = 1; //0-GSTIN from profile, 1-GSTIN from Inv Buffer
        int32                   *refer_profile = NULL;
        int32                   *refer_inv_buffer = NULL;
        int32                   err = 0;
        pin_flist_t             *profile_gst_flistp = NULL;
	pin_flist_t             *reason_flistp = NULL;
	
	time_t                  *bill_date = NULL;
        time_t                  inv_created_t ;
        time_t                  *profile_gst_info_created_t = NULL;
        int32                   gst_flag = 0;
        char                    *bill_no = NULL;
        char                    msg[256] = "";
        char                            *acct_type = NULL;
        poid_t                  *bill_pdp = NULL;


	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Creating Adjustment Object");
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

        acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	    hsn_code = PIN_FLIST_FLD_GET(bill_details_flistp, PIN_FLD_TAX_CODE, 1, ebufp);

        acct_type = (char *)PIN_POID_GET_TYPE(acct_pdp);
        if (acct_type && ((strcmp(acct_type,"/account")==0))) {
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "inside account obj!  ...");
        }
        else {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "outside account!  ...");
        acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        }


	pp_type = PIN_FLIST_FLD_GET(receipt_flistp, MSO_FLD_PP_TYPE, 0, ebufp);
	part_receipt = PIN_FLIST_FLD_GET(receipt_flistp, PIN_FLD_RECEIPT_NO, 0, ebufp);
	parent_pdp = PIN_FLIST_FLD_GET(receipt_flistp, PIN_FLD_PARENT, 0, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, receipt);
	seq_no = fm_mso_utils_gst_sequence_no(ctxp, parent_pdp, "ADJUSTMENT", ebufp);

	/* Changes related to GST . Sequence number to have 8 digits for PP and 6 digits for SP */
	if((pp_type && *pp_type == 0) || (service_type && *service_type == SERV_TYPE_BB))
	{
		sprintf(receipt, "%s%08d", part_receipt, seq_no);
	}
	else if (pp_type && *pp_type == 1)
	{
		sprintf(receipt, "%s%06d", part_receipt, seq_no);
	}
	PIN_ERR_LOG_MSG(3, receipt);
	user_id = CM_FM_USERID(ebufp);
	db = PIN_POID_GET_DB(acct_pdp);
	rcpt_flistp = PIN_FLIST_CREATE(ebufp);
	rcpt_pdp = PIN_POID_CREATE(db, "/mso_adjustment", id, ebufp);
	PIN_FLIST_FLD_PUT(rcpt_flistp, PIN_FLD_POID, rcpt_pdp, ebufp);
	PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_AMOUNT, amount, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, rcpt_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, rcpt_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_FLAGS, rcpt_flistp, PIN_FLD_FLAGS, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TAX_FLAGS, rcpt_flistp, PIN_FLD_TAX_FLAGS, ebufp); 

	evt_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (evt_flistp)
		evt_pdp = PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_POID, 0, ebufp);

	b_flistp =  PIN_FLIST_ELEM_GET(evt_flistp, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp);
	if (b_flistp)
		item_pdp = PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);

	PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_ITEM_OBJ, item_pdp, ebufp);
	PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_EVENT_OBJ, evt_pdp, ebufp);
	PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_USERID, user_id, ebufp);
	vp1 = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_CODE, 1, ebufp);
	if (vp)
	{
		sprintf(adj_descr, "Adjustment applied for:%s: %s", (char*)vp, (char*)vp1);
		sprintf(charging_id, "%s", (char*)vp);
	}
	else
	{
		fm_mso_get_reason(ctxp, i_flistp, &reason_flistp, ebufp);

		if(reason_flistp)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "return flistp", reason_flistp);
			vp = PIN_FLIST_FLD_GET(reason_flistp, PIN_FLD_REASON_CODE, 0, ebufp);
			sprintf(adj_descr, "Adjustment applied for:%s: %s", (char*)vp ,(char*)vp1);
			sprintf(charging_id, "%s", (char*)vp);
		}
        else
		{
				sprintf(adj_descr, "Adjustment applied for: %s", (char*)vp1);
				sprintf(charging_id, "Others");
		}

		
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 1, ebufp);
		/* Adjustment Reasons
		1. Customer not satisfied with service
		2. Customer unaware of charges
		3. Debited account by mistake
		4. Technical and support charges
		5. Service charges
		6. Others
		*/
/* Commenting 
		if(vp && *(int32*)(vp) == 0)
		{
			sprintf(adj_descr, "Disconnection Credit");
			sprintf(charging_id, "Disconnection Credit");
		}
		else if (vp && *(int32*)(vp) == 1)
		{
			sprintf(adj_descr, "Adjustment applied for:Settlement against Retention: %s", (char*)vp1);
			sprintf(charging_id,"Settlement against Retention");
		}
		else if (vp && *(int32*)(vp) == 2)

		{
			sprintf(adj_descr, "Adjustment applied for:Double Top Up: %s", (char*)vp1);
			sprintf(charging_id, "Double Top Up");
		}
		else if (vp && *(int32*)(vp) == 3)
		{
			sprintf(adj_descr, "Adjustment applied for:Plan Change Done: %s", (char*)vp1);
			sprintf(charging_id, "Plan Change Done");
		}
		else if (vp && *(int32*)(vp) == 4)
		{
			sprintf(adj_descr, "Adjustment applied for:Wrongly Activated&Terminated: %s", (char*)vp1);
			sprintf(charging_id, "Wrongly Activated&Terminated");
		}
		else if (vp && *(int32*)(vp) == 5)
		{
			sprintf(adj_descr, "Adjustment applied for:Non Usage: %s", (char*)vp1);
			sprintf(charging_id, "Non Usage");
		}
		else if (vp && *(int32*)(vp) == 6)
		{
			sprintf(adj_descr, "Adjustment applied for:Auto Activation Done: %s", (char*)vp1);
			sprintf(charging_id, "Auto Activation Done");
		}
		else if (vp && *(int32*)(vp) == 7)
                {
                        sprintf(adj_descr, "Adjustment applied for:Delay In Plan Change: %s", (char*)vp1);
                        sprintf(charging_id, "Delay In Plan Change");
		}
		else if (vp && *(int32*)(vp) == 8)
                {
                        sprintf(adj_descr, "Adjustment applied for:Wrong Plan Given: %s", (char*)vp1);
                        sprintf(charging_id, "Wrong Plan Given");
		}
		else if (vp && *(int32*)(vp) == 9)
                {
                        sprintf(adj_descr, "Adjustment applied for:New Wi-Fi router deposit - NWIFID: %s", (char*)vp1);
                        sprintf(charging_id, "New Wi-Fi router deposit - NWIFID");
		}
		else if (vp && *(int32*)(vp) == 10)
                {
                        sprintf(adj_descr, "Adjustment applied for:TDS  - TDS: %s", (char*)vp1);
                        sprintf(charging_id, "TDS  - TDS");
		}
		else if (vp && *(int32*)(vp) == 11)
		{
			sprintf(adj_descr, "Adjustment applied for:Penalty - Penalty: %s", (char*)vp1);
			sprintf(charging_id, "Penalty - Penalty");
		}
		else if (vp && *(int32*)(vp) == 12)
		{
			sprintf(adj_descr, "Adjustment applied for:CPE charges - CPEADJ: %s", (char*)vp1);
			sprintf(charging_id, "CPE charges - CPEADJ");
		}
		else if (vp && *(int32*)(vp) == 13)
                {
                        sprintf(adj_descr, "Adjustment applied for:LPF Charges - LPEADJ: %s", (char*)vp1);
                        sprintf(charging_id, "LPF Charges - LPEADJ");
		}
		else if (vp && *(int32*)(vp) == 14)
                {
                        sprintf(adj_descr, "Adjustment applied for:Static IP - AMC: %s", (char*)vp1);
                        sprintf(charging_id, "Static IP - AMC");
		}
		else if (vp && *(int32*)(vp) == 15)
              {
                        sprintf(adj_descr, "Adjustment applied for:Modem Deposit - DEP: %s", (char*)vp1);
                        sprintf(charging_id, "Modem Deposit - DEP");
		}
		else if (vp && *(int32*)(vp) == 16)
              {
                        sprintf(adj_descr, "Adjustment applied for:Additional uses waiver - AUCW: %s", (char*)vp1);
                        sprintf(charging_id, "Additional uses waiver - AUCW");
		}
              else if (vp && *(int32*)(vp) == 17)
              {
                        sprintf(adj_descr, "Adjustment applied for:CrNoteagainstDrNote(HO): %s", (char*)vp1);
                        sprintf(charging_id, "CrNoteagainstDrNote(HO)");
		}
		else if (vp && *(int32*)(vp) == 18)
              {
                        sprintf(adj_descr, "Adjustment applied for:Discount related: %s", (char*)vp1);
                        sprintf(charging_id, "Discount related");
		}
		else if (vp && *(int32*)(vp) == 19)
              {
                        sprintf(adj_descr, "Adjustment applied for:Initial payment refund(Subscribition): %s", (char*)vp1);
                        sprintf(charging_id, "Initial payment refund(Subscribition)");
		}
		else if (vp && *(int32*)(vp) == 20)
              {
                        sprintf(adj_descr, "Adjustment applied for:Cr Note for GPON migration: %s", (char*)vp1);
                        sprintf(charging_id, "Cr Note for GPON migration");
		}
		else if (vp && *(int32*)(vp) == 21)
              {
                        sprintf(adj_descr, "Adjustment applied for: Service Issue: %s", (char*)vp1);
                        sprintf(charging_id, "Service Issue");
		}
		else if (vp && *(int32*)(vp) == 22)
              {
                        sprintf(adj_descr, "Adjustment applied for:GST: %s", (char*)vp1);
                        sprintf(charging_id, "GST");
		}
		else if (vp && *(int32*)(vp) == 23)
              {
                        sprintf(adj_descr, "Adjustment applied for: Cheque bounce billing reversal: %s", (char*)vp1);
                        sprintf(charging_id, "Cheque bounce billing reversal");
		}
		else if (vp && *(int32*)(vp) == 24)
              {
                        sprintf(adj_descr, "Adjustment applied for: Outstanding reversal for refund: %s", (char*)vp1);
                        sprintf(charging_id, "Outstanding reversal for refund.");
		}
		else if (vp && *(int32*)(vp) == 25)
                {
                        sprintf(adj_descr, "Adjustment applied for:Delay in Termination: %s", (char*)vp1);
                        sprintf(charging_id, "Delay in Termination");
                }
                else if (vp && *(int32*)(vp) == 26)
                {
                        sprintf(adj_descr, "Adjustment applied for: Dedupp wrong account activated: %s", (char*)vp1);
                        sprintf(charging_id, "Dedupp wrong account activated");
                }
                else if (vp && *(int32*)(vp) == 27)
                {
                        sprintf(adj_descr, "Adjustment applied for: Billing reversal against wrong payment: %s", (char*)vp1);
                        sprintf(charging_id, "Billing reversal against wrong payment");
                }
                else if (vp && *(int32*)(vp) == 28)
                {
                        sprintf(adj_descr, "Adjustment applied for: Cash Back : %s", (char*)vp1);
                        sprintf(charging_id, "Cash Back Adjustment");
                }
                else if (vp && *(int32*)(vp) == 29)
                {
                        sprintf(adj_descr, "Adjustment applied for: OS Reversal against testing : %s", (char*)vp1);
                        sprintf(charging_id, "OS Reversal against testing ");
                }
                else if (vp && *(int32*)(vp) == 30)
                {
                        sprintf(adj_descr, "Adjustment applied for: HR Rental charges: %s", (char*)vp1);
                        sprintf(charging_id, "HR Rental charges");
                }
                else if (vp && *(int32*)(vp) == 31)
                {
                        sprintf(adj_descr, "Adjustment applied for: Deactivate due to wrongly days deduction: %s", (char*)vp1);
                        sprintf(charging_id, "Deactivate due to wrongly days deduction");
                }
				else if (vp && *(int32*)(vp) == 32)
                {
                        sprintf(adj_descr, "Adjustment applied for: Cashback: %s", (char*)vp1);
                        sprintf(charging_id, "Cashback");
                }
                else if (vp && *(int32*)(vp) == 33)
                {
                        sprintf(adj_descr, "Adjustment applied for: convenience fee waiver: %s", (char*)vp1);
                        sprintf(charging_id, "convenience fee waiver");
                }
                    
		else
		{
			sprintf(adj_descr, "Adjustment applied for: %s", (char*)vp1);
			sprintf(charging_id, "Others");
		}
*/
	}

	PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_CHARGING_ID, charging_id, ebufp);
	PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_RECEIPT_NO, receipt, ebufp);
	PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_DESCR, adj_descr, ebufp);
	if(bill_details_flistp)
	{
		PIN_FLIST_FLD_COPY(bill_details_flistp, PIN_FLD_POID, rcpt_flistp, PIN_FLD_BILL_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(bill_details_flistp, PIN_FLD_BILL_NO, rcpt_flistp, PIN_FLD_BILL_NO, ebufp);
		PIN_FLIST_FLD_COPY(bill_details_flistp, PIN_FLD_BILL_DATE_T, rcpt_flistp, PIN_FLD_BILL_DATE_T, ebufp);
		PIN_FLIST_FLD_COPY(bill_details_flistp, PIN_FLD_ADJUSTED, rcpt_flistp, PIN_FLD_ADJUSTED, ebufp);
		PIN_FLIST_FLD_COPY(bill_details_flistp, PIN_FLD_CURRENT_BAL, rcpt_flistp, PIN_FLD_CURRENT_BAL, ebufp);
		/**Adding GSTIN Flag in /mso_adjustement START */
                bill_pdp = PIN_FLIST_FLD_GET(bill_details_flistp, PIN_FLD_POID, 0, ebufp);
                bill_date= PIN_FLIST_FLD_GET(bill_details_flistp, PIN_FLD_BILL_DATE_T, 0, ebufp);

                pin_conf("fm_mso_ar", "refer_profile", PIN_FLDT_INT, (caddr_t*)&refer_profile, &err );
                pin_conf("fm_mso_ar", "refer_inv_buffer", PIN_FLDT_INT, (caddr_t*)&refer_inv_buffer, &err );
		
		if(refer_profile != NULL && *(int32*)refer_profile != 0) {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "refer_profile is enabled");
                        getProfileGSTInfo(ctxp, acct_pdp, &profile_gst_flistp, ebufp);
                        inv_created_t = getInvoiceCreatedT(ctxp, bill_pdp, ebufp);
                        if(profile_gst_flistp != NULL) {
                                profile_gst_info_created_t = PIN_FLIST_FLD_GET(profile_gst_flistp, PIN_FLD_CREATED_T, 0, ebufp);
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "got profile created date");

                                if(*profile_gst_info_created_t > inv_created_t) {
                                        gst_inv_flag = 1;
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "profile created after bill date");
                                }
                                else {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "profile created before bill date");
                                        gst_inv_flag = 0;
                                        gst_flag = 1;
                                        PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_REASON_ID, &gst_flag, ebufp);
                                }
                                PIN_FLIST_DESTROY_EX(&profile_gst_flistp, NULL);
                        }
                }

		if(gst_inv_flag != 0 && (refer_inv_buffer == NULL || *refer_inv_buffer == 1)) {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "referring invoice buffer");
                        gst_flag = getGSTINflag(ctxp, bill_pdp, ebufp);
                        sprintf(msg, "gst_flag : %d",gst_flag);
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
                        PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_REASON_ID, &gst_flag, ebufp);
                }

                /**Adding GSTIN Flag in /mso_adjustement END */
	
		is_pre_gst = PIN_FLIST_FLD_GET(bill_details_flistp, PIN_FLD_FLAGS, 1, ebufp);
		if (is_pre_gst && *is_pre_gst == 0)
		{
			adjusted = PIN_FLIST_FLD_GET(bill_details_flistp, PIN_FLD_ADJUSTED, 0, ebufp);
			tax_rate = PIN_FLIST_FLD_GET(receipt_flistp, MSO_FLD_TAX_RATE, 0, ebufp);
			tot_percent = pbo_decimal_multiply(tax_rate, hundred, ebufp);
			pbo_decimal_add_assign(tot_percent, hundred, ebufp);
		
			one_per = pbo_decimal_divide(adjusted, tot_percent, ebufp);
			if(!pbo_decimal_is_null(tot_percent, ebufp))
                	        pbo_decimal_destroy(&tot_percent);

			base_amt = pbo_decimal_copy(adjusted, ebufp);
		
			tax_rate = NULL;
			tax_rate = PIN_FLIST_FLD_GET(receipt_flistp, MSO_FLD_IGST_RATE, 1, ebufp);     //IGST
			if(!pbo_decimal_is_null(tax_rate, ebufp) && (hsn_code && strcmp(hsn_code, "HW_DEPOSIT")))
			{
				igst_tax = pbo_decimal_multiply(tax_rate, hundred, ebufp);
				pbo_decimal_multiply_assign(igst_tax, one_per, ebufp);
				pbo_decimal_round_assign(igst_tax, precision, round_mode, ebufp);
				PIN_FLIST_FLD_PUT(rcpt_flistp, MSO_FLD_IGST_RATE, igst_tax, ebufp); 
				pbo_decimal_subtract_assign(base_amt, igst_tax, ebufp);		
			}
		
			tax_rate = NULL;
       	         	tax_rate = PIN_FLIST_FLD_GET(receipt_flistp, MSO_FLD_SGST_RATE, 1, ebufp);     //IGST
	                if(!pbo_decimal_is_null(tax_rate, ebufp) && (hsn_code && strcmp(hsn_code, "HW_DEPOSIT")))
        	        {
                	        sgst_tax = pbo_decimal_multiply(tax_rate, hundred, ebufp);
                        	pbo_decimal_multiply_assign(sgst_tax, one_per, ebufp);
				pbo_decimal_round_assign(sgst_tax, precision, round_mode, ebufp);
	                        PIN_FLIST_FLD_PUT(rcpt_flistp, MSO_FLD_SGST_RATE, sgst_tax, ebufp);
				pbo_decimal_subtract_assign(base_amt, sgst_tax, ebufp);
                	}

			tax_rate = NULL;
        	        tax_rate = PIN_FLIST_FLD_GET(receipt_flistp, MSO_FLD_CGST_RATE, 1, ebufp);     //IGST
                	if(!pbo_decimal_is_null(tax_rate, ebufp) && (hsn_code && strcmp(hsn_code, "HW_DEPOSIT")))
               	 	{
                        	cgst_tax = pbo_decimal_multiply(tax_rate, hundred, ebufp);
	                        pbo_decimal_multiply_assign(cgst_tax, one_per, ebufp);
				pbo_decimal_round_assign(cgst_tax, precision, round_mode, ebufp);
                	        PIN_FLIST_FLD_PUT(rcpt_flistp, MSO_FLD_CGST_RATE, cgst_tax, ebufp);
				pbo_decimal_subtract_assign(base_amt, cgst_tax, ebufp);
                	}
			PIN_FLIST_FLD_PUT(rcpt_flistp, PIN_FLD_CHARGE_AMT, base_amt, ebufp);
	
			if(!pbo_decimal_is_null(one_per, ebufp))
				pbo_decimal_destroy(&one_per);
		}
		else
		{
			PIN_FLIST_FLD_COPY(bill_details_flistp, PIN_FLD_CHARGE_AMT, rcpt_flistp, PIN_FLD_CHARGE_AMT, ebufp);
		}
	}
	else
	{
		PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_ADJUSTED, rem_amount, ebufp);
		PIN_FLIST_FLD_PUT(rcpt_flistp, PIN_FLD_CHARGE_AMT, rem_amount, ebufp);
	}

	if(hsn_code)
	{
		PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_TAX_CODE, hsn_code, ebufp); 
	}
	else
	{
        hsn_code = PIN_FLIST_FLD_GET(bill_details_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
        charge_head = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGING_ID, 1, ebufp);
                if(charge_head && strstr(charge_head, "mso_hw_rental_"))
		{
            if (hsn_code)
            {
			    PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_TAX_CODE, hsn_code, ebufp);
            }
            else
            {
                PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_TAX_CODE, RENTAL_HSN, ebufp);
            }
		}
		else
		{
            if (hsn_code)
            {
			    PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_TAX_CODE, hsn_code, ebufp);
            }
            else
            {
			    PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_TAX_CODE, SUBSCRIPTION_HSN, ebufp);
            }
		}
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_Adjustments Input Flist", rcpt_flistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, rcpt_flistp, &ret_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_Adjustments Output Flist", ret_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in creating adjustment object");
		goto CLEANUP;
	}
	
CLEANUP:
	PIN_FLIST_DESTROY_EX(&rcpt_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return;
}

void fm_mso_part_receipt_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int32			*service_type,
	pin_decimal_t		*amount,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*parent_info_flistp = NULL;
	pin_flist_t		*profile_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*state_flistp = NULL;
	pin_flist_t		*lco_info_flistp = NULL;
	pin_flist_t		*jv_info_flistp = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*parent_pdp = NULL;
	poid_t			*jv_pdp = NULL;
	poid_t			*lco_pdp = NULL;

	char			*debit_str = "D";
	char			*credit_str = "C";
	char			*company = NULL;
	char			*pp_str = "P";
	char			*sp_str = "S";
	char			receipt[17];
	char			*state_name = NULL;
	char			*state_code = NULL;
	char			*bu_code = NULL;
        char                            *acct_type = NULL;

	int32			pp_type ;
	int32			month = -1;
	int32			year = -1;
	
	time_t			curr_time = pin_virtual_time(NULL);
	
	struct tm		*timeeff;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Generating Part of Receipt Number");
	timeeff = localtime(&curr_time);
	month = timeeff->tm_mon+1;
	year  = timeeff->tm_year;
	year = year % 100;
	/* Converting calendar year to Financial year for GST implicateion in sequence number */
	// checking if month is less than April
	 if(month > 3 )
	 year = year+1;
	
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

        acct_type = (char *)PIN_POID_GET_TYPE(acct_pdp);
        if (acct_type && ((strcmp(acct_type,"/account")==0))) {
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "inside account obj!  ...");
        }
        else {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "outside account!  ...");
        acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        }

	fm_mso_get_subscriber_profile(ctxp, acct_pdp, &profile_flistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp) || profile_flistp == NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error is getting LCO");
		goto CLEANUP;
	}
	pp_type = *(int32 *)PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_PP_TYPE, 0, ebufp);
	jv_pdp = PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_JV_OBJ, 0, ebufp);
	fm_mso_get_account_info(ctxp, jv_pdp, &jv_info_flistp, ebufp);
	lco_pdp = PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
	fm_mso_get_account_info(ctxp, lco_pdp, &lco_info_flistp, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_GET(lco_info_flistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 0, ebufp);
       	state_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_STATE, 0, ebufp);
	bu_code = PIN_FLIST_FLD_GET(lco_info_flistp, PIN_FLD_AAC_PROMO_CODE, 0, ebufp);
	company = PIN_FLIST_FLD_GET(jv_info_flistp, PIN_FLD_AAC_PROMO_CODE, 0, ebufp);

       	if(PIN_ERRBUF_IS_ERR(ebufp))
      	{
       		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in getting company");
              	goto CLEANUP;;
    	}
	PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(amount, ebufp));
	
	if((pp_type == 0) || (service_type && *service_type == SERV_TYPE_BB))
	{
		parent_pdp = PIN_FLIST_FLD_TAKE(profile_flistp, MSO_FLD_JV_OBJ, 0, ebufp);
		fm_inv_pol_get_state_code_from_cache(ctxp, state_name, &state_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "state flistp", state_flistp);
		state_code = PIN_FLIST_FLD_GET(state_flistp, PIN_FLD_CODE, 0, ebufp);

		/* Changes related to GST. Setting year only as part of receipt number */
        	if(pbo_decimal_sign(amount, ebufp) == -1){
			sprintf(receipt, "%s%s%s%s%02d", credit_str, company, state_code, pp_str,  year);
        	}
        	else
		{
			sprintf(receipt, "%s%s%s%s%02d", debit_str, company, state_code, pp_str,  year);
        	}
	}
	else if ( pp_type == 1)
	{
		parent_pdp = PIN_FLIST_FLD_TAKE(profile_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
		if(pbo_decimal_sign(amount, ebufp) == -1){
			sprintf(receipt, "%s%s%s%02d%s", credit_str, company, sp_str, year,  bu_code);
		}
		else
		{
			sprintf(receipt, "%s%s%s%02d%s", debit_str, company, sp_str,  year, bu_code);
		}
	}
	PIN_ERR_LOG_MSG(3, receipt);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_RECEIPT_NO, receipt, ebufp);
	PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_PARENT, parent_pdp, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_PP_TYPE, (void*)&pp_type, ebufp);
	 PIN_ERR_LOG_FLIST(3, "receipt input flistp", *r_flistpp);	
	
CLEANUP:
	PIN_FLIST_DESTROY_EX(&profile_flistp, NULL);
	return;
}
int getGSTINflag(pcm_context_t *ctxp, poid_t * bill_pdp, pin_errbuf_t *ebufp) {

        char *p, *temp, *gst_temp, *gst_temp2;
        int gst_flag = 0;

        int32           flags = 256;

        poid_t          *s_pdp = NULL;
        poid_t          *p_pdp = NULL;

        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *p_flistp = NULL;
        pin_flist_t     *buf_flistp = NULL;
        pin_flist_t     *r_s_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;

        pin_buf_t       *pin_bufp = NULL;

        char            *template =  "select X from /invoice where F1 = V1 ";
        char            debugmsg[100];

        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp  = PIN_POID_FROM_STR("0.0.0.1 /search -1 0", NULL, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);

        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_OBJ, bill_pdp, ebufp);
        //sprintf(debugmsg,"Bill_no- %s",bill_no);
        //PIN_ERR_LOG_MSG(3,debugmsg);
        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&flags, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(3, "invoice format search input flistp", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_s_flistp, ebufp);
        PIN_ERR_LOG_FLIST(3, "invoice format search output flistp", r_s_flistp);

        if (PIN_FLIST_ELEM_COUNT(r_s_flistp, PIN_FLD_RESULTS, ebufp) > 0) {
                r_flistp = PIN_FLIST_ELEM_GET( r_s_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
                buf_flistp = PIN_FLIST_ELEM_GET( r_flistp, PIN_FLD_FORMATS, 0, 0, ebufp );
                pin_bufp = (pin_buf_t *)PIN_FLIST_FLD_GET( buf_flistp, PIN_FLD_BUFFER, 0, ebufp );
                /*Extract the GSTIN from the Invoice Buffer String */
                p = strtok_r(pin_bufp->data, "\n", &temp);
                do {
                        if( strstr(p, "MSO_FLD_GSTIN") ) {
                                gst_temp = p + 16;
                                if(gst_temp != NULL && *gst_temp != '<') {
                                        gst_temp2 = strstr(gst_temp, "</M");
                                        gst_temp2 = '\0';
                                        gst_flag = (strlen(gst_temp) > 0) ? 1 : 0 ;
                                }
                                sprintf(debugmsg,"GSTIN-%s",gst_temp);
                                PIN_ERR_LOG_MSG(3,debugmsg);

                                break;
                        }
                } while ((p = strtok_r(NULL, "\n", &temp)) != NULL);

        }
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);

        return gst_flag;
}

void getProfileGSTInfo(pcm_context_t *ctxp, poid_t *pdp, pin_flist_t **profile_gst_flistp, pin_errbuf_t *ebufp){
         int32           flags = 256;

        poid_t          *s_pdp = NULL;
        poid_t          *p_pdp = NULL;

        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        char            *template =  "select X from /profile where F1 = V1 and profile_t.poid_type = '/profile/gst_info' ";
        time_t          *created_t=NULL;
        pin_flist_t     *s_r_flistp = NULL;
        pin_flist_t     *tmp_flist = NULL;
        int64           db=-1;

        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp  = PIN_POID_FROM_STR("0.0.0.1 /search -1 0", NULL, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);
        db = PIN_POID_GET_DB(s_pdp);
        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, pdp, ebufp);

        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&flags, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_CREATED_T, NULL, ebufp);
        tmp_flist = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_GST_INFO, 0, ebufp);
        PIN_FLIST_FLD_SET(tmp_flist, PIN_FLD_ACCOUNT_NO, NULL, ebufp);

        PIN_ERR_LOG_FLIST(3, "created date input flistp", s_flistp);
        PCM_OP(ctxp,PCM_OP_SEARCH, 0, s_flistp, &s_r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(3, "created date output flistp", s_r_flistp);

        if(PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in getting created date details");
                goto cleanup;
        }

        if (PIN_FLIST_ELEM_COUNT(s_r_flistp, PIN_FLD_RESULTS, ebufp) > 0 ){
                *profile_gst_flistp = PIN_FLIST_ELEM_TAKE(s_r_flistp,PIN_FLD_RESULTS,0,0, ebufp);
        }

cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&s_r_flistp, NULL);

}

time_t getInvoiceCreatedT(pcm_context_t *ctxp, poid_t *bill_pdp, pin_errbuf_t *ebufp) {

        char *p, *temp, *gst_temp, *gst_temp2;
        int gst_flag = 0;

        int32           flags = 256;

        poid_t          *s_pdp = NULL;
        poid_t          *p_pdp = NULL;

        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *p_flistp = NULL;
        pin_flist_t     *r_s_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;

        char            *template =  "select X from /invoice where F1 = V1 ";
        char            debugmsg[100];
        time_t          *inv_created_t_p = NULL;
        time_t          inv_created_t;

        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp  = PIN_POID_FROM_STR("0.0.0.1 /search -1 0", NULL, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp,PIN_FLD_POID, s_pdp, ebufp);

        PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_TEMPLATE, template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_OBJ, bill_pdp, ebufp);
	
	PIN_FLIST_FLD_SET(s_flistp,PIN_FLD_FLAGS, (void*)&flags, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREATED_T, 0, ebufp);

        PIN_ERR_LOG_FLIST(3, "invoice search input flistp", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_s_flistp, ebufp);
        PIN_ERR_LOG_FLIST(3, "invoice search output flistp", r_s_flistp);

        if (PIN_FLIST_ELEM_COUNT(r_s_flistp, PIN_FLD_RESULTS, ebufp) > 0) {
                r_flistp = PIN_FLIST_ELEM_GET( r_s_flistp, PIN_FLD_RESULTS, 0, 1, ebufp );
                inv_created_t_p = PIN_FLIST_FLD_GET( r_flistp, PIN_FLD_CREATED_T, 0, ebufp );
                if(inv_created_t_p != NULL)
                        inv_created_t = *inv_created_t_p;

        }
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_s_flistp, NULL);

        return inv_created_t;
}



void fm_mso_bill_adj(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_in_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *tmp_flistp = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *result_flistp = NULL;
        pin_flist_t             *wfl_iflistp = NULL;
        pin_flist_t             *wfl_oflistp = NULL;
        int32                   srch_flag = 256;
        time_t                  current_t = 0;
        int                     elem_id = 0;
        pin_cookie_t            cookie = NULL;
        int                     db = 1;
        poid_t          *a_pdp = NULL;
		char                            *acct_type = NULL;
		pin_decimal_t 		*valid_amount = pbo_decimal_from_str("0.00", ebufp);
		

        char                    *template = "select X from /billinfo 1, /item 2 where 1.F1 = V1 and billinfo_t.pay_type=10001 and billinfo_t.bill_info_id='CATV' and 2.F2 = V2 and 1.F3 = 2.F4 and 2.F5 = V5 and 2.F6 <> V6 ";

		current_t = pin_virtual_time(NULL);
		

		a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

        acct_type = (char *)PIN_POID_GET_TYPE(a_pdp);
        if (acct_type && ((strcmp(acct_type,"/account")==0))) {
         PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "inside account obj!  ...");
        }
        else {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                                "outside account!  ...");
        a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        }

        srch_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 3, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, a_pdp, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 4, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_AR_BILLINFO_OBJ, a_pdp, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 5, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_STATUS, &db, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 6, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ITEM_TOTAL, valid_amount, ebufp);


		result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp );
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bill_adj search input flist", srch_in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bill_adj output flist", srch_out_flistp);

        if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
        {
                PIN_ERRBUF_CLEAR(ebufp);
                return;
        }
        else
        {
			wfl_iflistp = PIN_FLIST_CREATE(ebufp);
			while((result_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flistp,PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp ))!= NULL)
			{
				//wfl_iflistp = (pin_flist_t *)NULL;
				//PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bill_adj result_flistp flist", result_flistp);
				PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_ACCOUNT_OBJ, wfl_iflistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(result_flistp, PIN_FLD_POID,wfl_iflistp, PIN_FLD_BILLINFO_OBJ, ebufp);
				PIN_FLIST_FLD_SET(wfl_iflistp, PIN_FLD_PROGRAM_NAME, "Bill Now - Adj Flow", ebufp)
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bill_adj write input flist", wfl_iflistp);
				PCM_OP(ctxp, PCM_OP_BILL_MAKE_BILL_NOW, 0, wfl_iflistp, &wfl_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bill_adj write output flist", wfl_oflistp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
						PIN_ERRBUF_CLEAR(ebufp);
						return;
				}


			}
        }


                                                        

        PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&wfl_iflistp, NULL);
        PIN_FLIST_DESTROY_EX(&wfl_oflistp, NULL);
        return ;
}


void fm_mso_get_reason(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	    pin_flist_t             **r_flistp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_in_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *result_flist = NULL;
        int32                   srch_flag = 512;
        time_t                  current_t = 0;
        int                     db = 1;
	void 			*vp=NULL;
		int32                     id = 999;
        poid_t          *a_pdp = NULL;
		

        char                    *template = "select X from /mso_cfg_reason_codes where F1 = V1 ";

			
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 1, ebufp);
 	if (vp){
	id = *(int32 *)vp;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside R");

        srch_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_in_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_in_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_REASON_ID, &id, ebufp);

        result_flist = PIN_FLIST_ELEM_ADD(srch_in_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
        PIN_FLIST_FLD_SET(result_flist, PIN_FLD_REASON_CODE, NULL, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_reason search input flist", srch_in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flistp, &srch_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_reason output flist", srch_out_flistp);

        if (PIN_ERRBUF_IS_ERR(ebufp) || PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp) == 0)
        {
                PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
                return;
        }
        else
        {
			*r_flistp = PIN_FLIST_CREATE(ebufp);
			*r_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_reason output flist", *r_flistp);
			//*r_flistp = PIN_FLIST_COPY(srch_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_in_flistp, NULL);
			//PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
			return;

        }
	}
                                                        
}

