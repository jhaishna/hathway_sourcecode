/*****************************************************************************************************
 * File:	fm_mso_cust_price_calculator.c
 * Opcode:	MSO_OP_CUST_CALC_PRICE 
 * Owner:	Nagaraj
 * Created:	14-AUG-2013
 * Description: This opcode is used to calculate the price incurred on purchasing plan.
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 * Modified on 3-NUN-15 to handle non rated INR value also
 *
 * Input Flist
 *
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 88888
	0 PIN_FLD_PLAN			 ARRAY [0] 
	0 PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 88888
	0 PIN_FLD_PLAN			 ARRAY [1] 
	0 PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 99999
	0 PIN_FLD_ACTION		 ENUM [0] 1
	
 * Modified By: Jyothirmayi Kachiraju
 * Date: 19th Nov 2019
 * Modification details : Support for new events created for 360, 330, 300, 270, 240, 210, 150 days.
******************************************************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_calc_price.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"
#include "mso_errs.h"
#include "pin_rate.h"
#include "pin_decimal.h"

/*******************************************************************
 * MSO_OP_CUST_CALC_PRICE 
 *
 * 
 *******************************************************************/

extern pin_flist_t *VAT_Table;

extern pin_flist_t *Tax_Table;

#define TOK_LEN          80
#define STR_LEN          256
#define NCF_PKG_TYPE	3
#define ALL_PKG_TYPE	10

EXPORT_OP void
op_mso_cust_calc_price(
	cm_nap_connection_t	*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_calc_price(
    pcm_context_t       *ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);

extern 
void fm_mso_search_plan_details(
	pcm_context_t                   *ctxp,
	pin_flist_t                     *i_flistp,
	pin_flist_t                     **o_flistpp,
	pin_errbuf_t                    *ebufp);

extern
void fm_mso_search_tax_code(
	pcm_context_t                   *ctxp,
	pin_flist_t                     *i_flistp,
	poid_t                          *acct_pdp,
	int32                           *bill_when,
	pin_decimal_t                   *ch_amt,
	pin_decimal_t                   *cf_amt,
	pin_decimal_t                   *pf_amt,
	char                            *plan_name,
	pin_decimal_t                   **tot_amt,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t                    *ebufp);

static void fm_mso_get_vat_rate(
	pcm_context_t                   *ctxp,
	poid_t                          *a_pdp,
	pin_decimal_t                   *ch_amt,
	pin_flist_t                     **o_flistpp,
	pin_errbuf_t                    *ebufp);

static void
fm_mso_apply_vat(
	pcm_context_t       *ctxp,
	char                *taxcode,
	int		    flags,
	pin_decimal_t       *amt_gross,
	pin_flist_t         **r_flistpp,
	pin_errbuf_t        *ebufp);

extern void
mso_getTaxCodelist(
        pcm_context_t           *ctxp,
        char                    *tax_code,
	int32			gst_type,
        pin_flist_t             **taxCodelist_flistp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void
fm_mso_fetch_offer_purchased_plan(
	pcm_context_t		*ctxp,
	poid_t			*acc_poidp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_mso_get_account_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_mso_retrieve_et(
        pcm_context_t           *ctxp,
        poid_t                  *srvc_pdp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

void calculate_st(pcm_context_t       *ctxp,
        char                *taxcode,
        pin_decimal_t       *amt_gross,
        pin_decimal_t       **tax_amt,
	pin_flist_t		**taxapplied_flistp,
        pin_errbuf_t        *ebufp);

// functions for catv plan price calculation

void fm_mso_cust_catv_calc_price( 
	pcm_context_t       *ctxp,
        pin_flist_t       *i_flistp,
        pin_flist_t        **r_flistpp,
        pin_errbuf_t        *ebufp);

void fm_mso_get_plan_price (
        pcm_context_t   *ctxp,
        poid_t          *plan_pdp,
	poid_t		*deal_pdp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

void fm_mso_get_pkg_type(
        pcm_context_t   *ctxp,
        pin_flist_t  	*in_flistp,
        pin_flist_t     **ret_flistp,
        pin_errbuf_t    *ebufp);

int32 fm_mso_get_prd_scale(
        pcm_context_t   *ctxp,
        poid_t          *prd_pdp,
        time_t          charged_to_t,
        pin_errbuf_t   *ebufp);

void fm_mso_get_et_amt(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**r_flistpp,
	pin_errbuf_t	*ebufp);

void fm_mso_get_srv_dtls(
	 pcm_context_t   *ctxp,
        pin_flist_t     *out_flistp,
        pin_errbuf_t    *ebufp);

void fm_mso_get_plan_dtls(
         pcm_context_t   *ctxp,
        pin_flist_t     *out_flistp,
        pin_errbuf_t    *ebufp);

static void
mso_gst_apply_tax(
        pcm_context_t   *ctxp,
        pin_decimal_t   *tax_rate,
        pin_decimal_t   *amt_gross,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);


static void calculate_gst(
	pcm_context_t       *ctxp,
	poid_t				*acct_pdp,
	char                *tax_code,
	pin_decimal_t       *amt_gross,
	pin_decimal_t       **tax_amt,
	pin_flist_t        	**taxapplied_flistp,
	pin_errbuf_t        *ebufp);

void fm_mso_get_plan_tax_code (
        pcm_context_t   *ctxp,
		poid_t			*plan_pdp,
        char			*tax_code,
		pin_flist_t		**r_flistpp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT
 char* mso_get_taxcode(
        pcm_context_t           *ctxp,
        poid_t                          *srvc_pdp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void
mso_et_apply_tax(
        pcm_context_t   *ctxp,
        char                    *taxcode,
        pin_decimal_t   *amt_gross,
        pin_decimal_t   *scale,
        pin_flist_t             **out_flistpp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT void mso_retrieve_et_code(
        pcm_context_t   *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT void mso_retrieve_gst_rate(
        pcm_context_t   *ctxp,
        char            *tax_code,
        poid_t          *acct_pdp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT void
fm_mso_utils_get_profile(
        pcm_context_t   *ctxp,
        poid_t          *account_pdp,
        pin_flist_t     **ret_flistpp,
        pin_errbuf_t    *ebufp);
extern void
fm_mso_utils_read_any_object(
        pcm_context_t           *ctxp,
        poid_t                  *objp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp);

const int SUCCESS = 0;
const int FAILURE = 1;

#define SVC_ACTIVATION  1
#define TOP_UP			2 
#define RENEW			3
#define ADD_PLAN		4
#define CHANGE_PLAN		5

#define IGST "IGST"
#define SGST "SGST"
#define CGST "CGST"
#define ADD_CESS "Additional_Cess"

/*******************************************************************
 * MSO_OP_CUST_CALC_PRICE  
 *******************************************************************/
void 
op_mso_cust_calc_price(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t   *ctxp = connp->dm_ctx;
	pin_flist_t		*ret_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *in_flistp = NULL;
    pin_cookie_t    cookie = NULL;
	int32			status = 0;
	int32			*srvc_type = NULL;
    int32           elem_id = 0;
    int32           plan_list_cnt = 0;

	*r_flistpp		= NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_CALC_PRICE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_calc_price error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_calc_price input flist", i_flistp);

	srvc_type =  PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 1, ebufp);	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	if(srvc_type && *srvc_type == 0 )
	{
        plan_list_cnt = PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_PLAN_LISTS, ebufp);
        if (plan_list_cnt > 0)
        {
            in_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, in_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
            PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);

            *r_flistpp = PIN_FLIST_COPY(in_flistp, ebufp);
        }
		PIN_ERR_LOG_MSG(3, "Enterting here");
        while ((tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN_LISTS,
            &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        {
            PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_PLAN_OBJ, in_flistp, PIN_FLD_PLAN_OBJ, ebufp);
            fm_mso_cust_catv_calc_price(ctxp, in_flistp, &ret_flistp, ebufp);
            if (PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_calc_price plan_list error", ebufp);
                break;
            }
            if (ret_flistp)
            {
                PIN_FLIST_ELEM_SET(*r_flistpp, ret_flistp, PIN_FLD_PLAN_LISTS, elem_id, ebufp);
                PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
                ret_flistp = NULL;
            }
        }

        if (plan_list_cnt == 0)
        {
            fm_mso_cust_catv_calc_price(ctxp, i_flistp, &ret_flistp, ebufp);
        }
	}
	else
	{
		fm_mso_cust_calc_price(ctxp, i_flistp, &ret_flistp, ebufp);
	}
	

 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		status = FAILURE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_calc_price error", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"op_mso_cust_calc_price input flist", i_flistp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
	}
	else
	{
        if (plan_list_cnt == 0)
        {
            *r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
        }
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_calc_price output flist", *r_flistpp);
	}

	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return;
}




static void
fm_mso_cust_calc_price(
    pcm_context_t       *ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp) 
{
	pin_flist_t	*op_iflistp = NULL;
	pin_flist_t	*op_oflistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*total_bal_impacts_flist = NULL;
	pin_flist_t	*plan_flistp = NULL;
	pin_flist_t	*planlist_flistp = NULL;
	pin_flist_t	*data_array_flistp = NULL;
	pin_flist_t	*hybrid_offer_info = NULL;
	pin_flist_t	*acnt_info = NULL;
	int32		calc_only_mode = 1;
	int32		INR = 356;
	int32		resource_id = 0;
	int32		rec_id2 = 0;
	int32		rec_id = 0;
	int32		operation = 0;
	int32		status = FAILURE;
	pin_cookie_t    cookie = NULL;
	pin_cookie_t    cookie2 = NULL;
	pin_decimal_t	*total_charge = NULL;
	pin_decimal_t   *zero = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t	*temp_amount = NULL;
	pin_decimal_t	*tax_per_amtp = NULL;
	pin_decimal_t	*catv_pymt_amtp = NULL;
	int32		*status_exe = NULL;
	pin_decimal_t	*tax_amt = NULL;
	pin_decimal_t	*temp_amt_amc = NULL;
	//Added for disc changes
	pin_decimal_t   *temp_disc_amt = NULL;
        pin_decimal_t   *total_disc = NULL;
	poid_t		*prod_pdp = NULL;
	poid_t		*plan_pdp = NULL;
	poid_t		*acc_pdp = NULL;
	char		*rate_tag = NULL;
	char		*tag_acc_nop = NULL;
	double		prod_pri_dbl = 0;
	double          AMC_PRODUCT_CM = 700;
	double          AMC_PRODUCT_DCM = 770;
	pin_flist_t	*ret_flistp = NULL;	
	pin_decimal_t	*sub_charge = NULL;
	pin_decimal_t   *total_chr = NULL;
        pin_decimal_t   *total_ref = NULL;
        pin_flist_t     *ref_flistp = NULL;

        pin_decimal_t   *total_staticip_amt = NULL;
        pin_decimal_t   *temp_staticip_amt = NULL;
        int32           staticipflag = 0;
	int32		acc_no_val = 0;

	/* Algo:
	1. check for PIN_FLG_FLAGS in input flist. 
	if value invalid, set status to FAILURE and return.
	2. based on the operation type, call the appropriate opcode/function.*/

	operation = *(int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp);

	if(operation < SVC_ACTIVATION && operation > CHANGE_PLAN) {
		status = FAILURE;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"fm_mso_cust_calc_price: invalid operation type");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"fp_mso_cust_calc_price input flist", i_flistp);
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
	}

	//op_iflistp = PIN_FLIST_CREATE(ebufp);
	data_array_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DATA_ARRAY , 0, 0, ebufp);
	op_iflistp = PIN_FLIST_COPY(data_array_flistp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, op_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);

	if(operation == SVC_ACTIVATION) {
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, op_iflistp, PIN_FLD_POID, ebufp);
		fm_mso_search_plan_details(ctxp, op_iflistp, r_flistpp, ebufp);
	} else {
		PIN_FLIST_FLD_SET(op_iflistp, PIN_FLD_MODE, &calc_only_mode, ebufp);
		// Moved the result creation 

		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		plan_flistp = PIN_FLIST_ELEM_ADD(*r_flistpp, PIN_FLD_PLAN, 0, ebufp);
		PIN_FLIST_FLD_COPY(op_iflistp, PIN_FLD_PLAN_OBJ, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);

		switch(operation) {
			case TOP_UP:
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"MSO_OP_CUST_TOP_UP_BB_PLAN (read-only) input flist", op_iflistp);
				PCM_OP(ctxp, MSO_OP_CUST_TOP_UP_BB_PLAN, 0, op_iflistp, &op_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"MSO_OP_CUST_TOP_UP_BB_PLAN (read-only) output flist", op_oflistp);
				PIN_FLIST_FLD_COPY(op_iflistp, PIN_FLD_PLAN_OBJ, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
				break;
			case RENEW:
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"MSO_OP_CUST_RENEW_BB_PLAN (read-only) input flist", op_iflistp);
				PCM_OP(ctxp, MSO_OP_CUST_RENEW_BB_PLAN, 0, op_iflistp, &op_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"MSO_OP_CUST_RENEW_BB_PLAN (read-only) output flist", op_oflistp);
				PIN_FLIST_FLD_COPY(op_iflistp, PIN_FLD_PLAN_OBJ, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
				break;
			case ADD_PLAN:	
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"MSO_OP_CUST_ADD_PLAN (read-only) input flist", op_iflistp);
				PCM_OP(ctxp, MSO_OP_CUST_ADD_PLAN, 0, op_iflistp, &op_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"MSO_OP_CUST_ADD_PLAN (read-only) output flist", op_oflistp);
				planlist_flistp = PIN_FLIST_ELEM_GET(op_iflistp, PIN_FLD_PLAN_LISTS , 0, 0, ebufp);
				PIN_FLIST_FLD_COPY(planlist_flistp, PIN_FLD_PLAN_OBJ, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
				break;
			case CHANGE_PLAN:
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"MSO_OP_CUST_CHANGE_PLAN (read-only) input flist", op_iflistp);
				PCM_OP(ctxp, MSO_OP_CUST_CHANGE_PLAN, 0, op_iflistp, &op_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"MSO_OP_CUST_CHANGE_PLAN (read-only) output flist", op_oflistp);
				planlist_flistp = PIN_FLIST_ELEM_GET(op_iflistp, PIN_FLD_PLAN_LISTS , 1, 0, ebufp);
				PIN_FLIST_FLD_COPY(planlist_flistp, PIN_FLD_PLAN_OBJ, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
				break;
		}

		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"err returned from main opcode", ebufp);
                	PIN_ERRBUF_RESET(ebufp);
			status = FAILURE;
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "64001", ebufp);
                	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Failed in main function", ebufp);
			goto CLEANUP;
		}

		if (op_oflistp == NULL)
		{
			status = FAILURE;
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "64002", ebufp);
                	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Main function returns null", ebufp);
			goto CLEANUP;
		}

		/* Extract the charge amount from the output flist. The output flist is essentially the 
		purchase deal output flist.*/
//		*r_flistpp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
//		plan_flistp = PIN_FLIST_ELEM_ADD(*r_flistpp, PIN_FLD_PLAN, 0, ebufp);
//		PIN_FLIST_FLD_COPY(op_iflistp, PIN_FLD_PLAN_OBJ, plan_flistp, PIN_FLD_PLAN_OBJ, ebufp);
		sub_charge = pbo_decimal_from_str("0.0",ebufp);
		total_charge = NULL;
		while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(op_oflistp, PIN_FLD_RESULTS,
                  &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {
          rec_id2 = 0;
          cookie2 = NULL;
			temp_staticip_amt= NULL;
          while((total_bal_impacts_flist = PIN_FLIST_ELEM_GET_NEXT(res_flistp, PIN_FLD_BAL_IMPACTS,
             &rec_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL) {

              PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PIN_FLD_BAL_IMPACTS flist", total_bal_impacts_flist);
              resource_id = *(int32 *)PIN_FLIST_FLD_GET(total_bal_impacts_flist, PIN_FLD_RESOURCE_ID, 0, ebufp);
	     
	      prod_pdp = PIN_FLIST_FLD_GET(total_bal_impacts_flist, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
	      rate_tag = PIN_FLIST_FLD_GET(total_bal_impacts_flist, PIN_FLD_RATE_TAG, 1, ebufp);
	      if(prod_pdp)
	      {
              	fm_mso_get_product_priority(ctxp, prod_pdp, &ret_flistp, ebufp);
	      	prod_pri_dbl = pbo_decimal_to_double( PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_PRIORITY, 0, ebufp),ebufp);
	      }
			      if(resource_id == INR) {
                                  temp_amount = PIN_FLIST_FLD_GET(total_bal_impacts_flist, PIN_FLD_AMOUNT, 0, ebufp);
				  temp_disc_amt = PIN_FLIST_FLD_GET(total_bal_impacts_flist, PIN_FLD_DISCOUNT, 1, ebufp);
                                  if(pbo_decimal_is_null(total_charge, ebufp)) {
                                          total_charge = pbo_decimal_copy(temp_amount, ebufp);
                                  } else {
                                          total_charge = pbo_decimal_add(temp_amount,total_charge, ebufp);
                                  }
                                // Changes for including STATIC Ip charges

                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Checking staticipflag");
                                staticipflag = 0;
                                if(PIN_FLIST_FLD_GET(total_bal_impacts_flist, MSO_FLD_IP_MASK_VALUE, 1, ebufp) != NULL ) staticipflag = 1;

                                if (staticipflag == 1)
                                {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Staticipflag is 1");
                                        // Retreive only if flag is not set
                                        if((rate_tag && strcmp(rate_tag,"Tax") != 0))
                                        {
                                         temp_staticip_amt = PIN_FLIST_FLD_GET(total_bal_impacts_flist, PIN_FLD_AMOUNT, 0, ebufp);

                                        if(temp_staticip_amt != NULL && !pbo_decimal_is_null(temp_staticip_amt, ebufp) )
                                                temp_staticip_amt = pbo_decimal_round( temp_staticip_amt ,
                                                2, ROUND_HALF_UP, ebufp);

                                        // Round he static amount

                                          if(pbo_decimal_is_null(total_staticip_amt, ebufp)) {
                                                  total_staticip_amt = pbo_decimal_copy(temp_staticip_amt, ebufp);
                                          } else {
                                                  total_staticip_amt = pbo_decimal_add(total_staticip_amt,temp_staticip_amt, ebufp);
                                          }
                                                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"erro calc", ebufp);}
                                        }

                                }

                                        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"erro calc", ebufp);}

                                if(total_staticip_amt != NULL && !pbo_decimal_is_null(total_staticip_amt, ebufp))
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(total_staticip_amt, ebufp));

                                // Static IP change . This also needs to be ported

				        //Added to populate discount info
				if(temp_disc_amt != NULL )
				{
                                	if(pbo_decimal_is_null(total_disc, ebufp)){
                                        	total_disc = pbo_decimal_copy(temp_disc_amt, ebufp);
                                	}
                                	else
                                	{
                                       		total_disc=pbo_decimal_add(temp_disc_amt,total_disc, ebufp);
                                	}
                                }

			      }
			      if(resource_id == INR && prod_pri_dbl != AMC_PRODUCT_CM && prod_pri_dbl != AMC_PRODUCT_DCM && !(rate_tag && strcmp(rate_tag,"Tax") == 0))
			      {
                                  if(staticipflag != 1)
                                        {
                                                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"erro calc", ebufp); }
                                          temp_amount = PIN_FLIST_FLD_GET(total_bal_impacts_flist, PIN_FLD_AMOUNT, 0, ebufp);
                                          if(pbo_decimal_is_null(sub_charge, ebufp)) {
                                                  sub_charge = pbo_decimal_copy(temp_amount, ebufp);
                                                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"erro calc", ebufp); }
                                          } else {
                                                  sub_charge = pbo_decimal_add(temp_amount,sub_charge, ebufp);
                                                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"erro calc", ebufp); }
                                          }
                                        }

			       }	
			       else if(resource_id == INR && (prod_pri_dbl == AMC_PRODUCT_CM || prod_pri_dbl == AMC_PRODUCT_DCM)) {
				       temp_amt_amc = PIN_FLIST_FLD_GET(total_bal_impacts_flist, PIN_FLD_AMOUNT, 0, ebufp);
				       prod_pri_dbl = 0;
			       }
			       else if(resource_id == INR && ((rate_tag) && (strcmp(rate_tag,"Tax") == 0))) {
			      	  temp_amount = PIN_FLIST_FLD_GET(total_bal_impacts_flist, PIN_FLD_AMOUNT, 0, ebufp);
				  if(pbo_decimal_is_null(tax_amt, ebufp)) {
				  	tax_amt = pbo_decimal_copy(temp_amount, ebufp);
				  } else {
					 tax_amt = pbo_decimal_add(temp_amount,tax_amt, ebufp);
				  }
						
			  }
		  }
	   }
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"erro calc", ebufp);
			
		}

//		zero = pbo_decimal_from_str("0.0",ebufp);
//Modified on 3-NUN-15 to handle non rated INR value also
		status_exe = PIN_FLIST_FLD_GET(op_oflistp, PIN_FLD_STATUS,1, ebufp);
		if((status_exe && (*status_exe == SUCCESS)) || !pbo_decimal_is_null(total_charge, ebufp))
		{
			status = SUCCESS;
		//Payload for change plan scenario
		   if(operation == CHANGE_PLAN)
		 {
			if(!pbo_decimal_is_null(sub_charge, ebufp)) {
			    PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT, sub_charge, ebufp);
			}
			else {
			    PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT, zero, ebufp);
			}
			if(!pbo_decimal_is_null(sub_charge, ebufp)) {
			   if(!pbo_decimal_is_null(total_disc, ebufp)) {
                            	PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_AMOUNT_ORIG, pbo_decimal_add(sub_charge, total_disc, ebufp), ebufp);
			   }
			   else
				{
					PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_ORIG, sub_charge, ebufp);
				}
                        }
                        else {
                            PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_ORIG, zero, ebufp);
                        }	
		        if(!pbo_decimal_is_null(temp_amt_amc, ebufp)) {
			   PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_FIXED_AMOUNT, temp_amt_amc, ebufp);
			}
			else {
			   PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_FIXED_AMOUNT, zero, ebufp);
		        }
		        if(!pbo_decimal_is_null(tax_amt, ebufp)) {
			   PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_TAXED, tax_amt, ebufp);
		        }
		    	else {
			    PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_TAXED, zero, ebufp);
			}
			if(!pbo_decimal_is_null(total_disc, ebufp)) {
                                PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_DISCOUNT, pbo_decimal_negate(total_disc, ebufp), ebufp);
                        }
                        else
                        {
                                PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_DISCOUNT, zero, ebufp);
                        }

                        if(!pbo_decimal_is_null(total_staticip_amt, ebufp)) {
                                PIN_FLIST_FLD_SET(plan_flistp, MSO_FLD_MISC_AMT, total_staticip_amt, ebufp);
                        }

                        //Block added to populate refund changes
                        ref_flistp = PIN_FLIST_ELEM_GET(op_oflistp, MSO_FLD_REFUND, 0, 1, ebufp);
                        if(ref_flistp)
                        {
                                total_ref = PIN_FLIST_FLD_GET(ref_flistp, PIN_FLD_AMOUNT, 0, ebufp);
                                PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_TOTAL_DUE, pbo_decimal_add(total_charge, total_ref, ebufp), ebufp);
                                PIN_FLIST_ELEM_COPY(op_oflistp, MSO_FLD_REFUND, 0, *r_flistpp, MSO_FLD_REFUND, 0, ebufp);
                        }
			else
			{
				PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TOTAL_DUE, total_charge, ebufp);
			}
		}
		//Payload remains same for ADD_PLAN/RENEW/TOPUP
		else
			{
			 if(!pbo_decimal_is_null(total_charge, ebufp)) {
                            PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT, total_charge, ebufp);
                        }
                        else {
                            PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT, zero, ebufp);
                        }
                        if(!pbo_decimal_is_null(sub_charge, ebufp)) {
                            PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_ORIG, sub_charge, ebufp);
                        }
                        else {
                            PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_ORIG, zero, ebufp);
                        }
                        if(!pbo_decimal_is_null(temp_amt_amc, ebufp)) {
                           PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_FIXED_AMOUNT, temp_amt_amc, ebufp);
                        }
                        else {
                           PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_FIXED_AMOUNT, zero, ebufp);
                        }
                        if(!pbo_decimal_is_null(tax_amt, ebufp)) {
                           PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_TAXED, tax_amt, ebufp);
                        }
                        else {
                            PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_TAXED, zero, ebufp);
                        }
			if(!pbo_decimal_is_null(total_disc, ebufp)) {
                                PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_DISCOUNT, pbo_decimal_negate(total_disc, ebufp), ebufp);
                        }
                        else
                        {
                                PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_DISCOUNT, zero, ebufp);
                        }

                        if(!pbo_decimal_is_null(total_staticip_amt, ebufp)) {
                                PIN_FLIST_FLD_SET(plan_flistp, MSO_FLD_MISC_AMT, total_staticip_amt, ebufp);
                        }

		}		
                        PIN_FLIST_FLD_COPY(op_oflistp, PIN_FLD_STATUS, *r_flistpp, PIN_FLD_STATUS, ebufp);

		}
		else
		{
			    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Copying from  flist", op_oflistp);
			    status = FAILURE;
			    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS,&status,ebufp);
			    PIN_FLIST_FLD_COPY(op_oflistp, PIN_FLD_ERROR_CODE, *r_flistpp, PIN_FLD_ERROR_CODE, ebufp);
			    PIN_FLIST_FLD_COPY(op_oflistp, PIN_FLD_ERROR_DESCR, *r_flistpp, PIN_FLD_ERROR_DESCR, ebufp);		
		}

	}

	if (operation == TOP_UP || operation == RENEW || operation == CHANGE_PLAN)
	{
		acc_pdp = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_POID, 0, ebufp);
		fm_mso_get_account_info(ctxp, acc_pdp, &acnt_info, ebufp);
		tag_acc_nop = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_AAC_PACKAGE, 1, ebufp);
		if (tag_acc_nop && strlen(tag_acc_nop) != 10 || sscanf(tag_acc_nop, "%d", &acc_no_val) != 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Tagging Account Number Format");
		}
		else if (tag_acc_nop == NULL)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "There is no tagging account number");
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "This is the proper tagging account number");
			plan_pdp = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_PLAN_OBJ, 0, ebufp);
			fm_mso_fetch_offer_purchased_plan(ctxp, plan_pdp, &hybrid_offer_info, ebufp);
			if (hybrid_offer_info)
			{
				catv_pymt_amtp = PIN_FLIST_FLD_GET(hybrid_offer_info, MSO_FLD_NCF_CHARGE, 0, ebufp);
				tax_per_amtp = pbo_decimal_from_str("1.18",ebufp);
				temp_amount = pbo_decimal_divide(catv_pymt_amtp, tax_per_amtp, ebufp);
				pbo_decimal_round_assign(temp_amount, 2, ROUND_HALF_UP, ebufp);

				/**********************************************************************
				 * Add Tax component of CATV plan amount
				 *********************************************************************/
				temp_disc_amt = pbo_decimal_subtract(catv_pymt_amtp, temp_amount, ebufp);
				pbo_decimal_add_assign(temp_disc_amt, tax_amt, ebufp); 
				PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_AMOUNT_TAXED, temp_disc_amt, ebufp);

				/**********************************************************************
				 * Add original component of CATV plan amount
				 *********************************************************************/
				pbo_decimal_add_assign(temp_amount, PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_AMOUNT_ORIG, 0, ebufp), ebufp);
				PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_AMOUNT_ORIG, temp_amount, ebufp);

			 	if (!pbo_decimal_is_null(total_charge, ebufp)) 
				{
                                	PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_AMOUNT, pbo_decimal_add(total_charge, catv_pymt_amtp, ebufp), ebufp);
				}
				PIN_FLIST_FLD_COPY(hybrid_offer_info, MSO_FLD_NCF_CHARGE,
					plan_flistp, PIN_FLD_MIN_CHARGE, ebufp);
				PIN_FLIST_DESTROY_EX(&hybrid_offer_info, NULL);
			}
		}
		PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	} 
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PIN_FLD_BAL_IMPACTS maha flist", *r_flistpp);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&op_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&op_oflistp, NULL);
        if (!pbo_decimal_is_null(tax_amt, ebufp))
        {
		pbo_decimal_destroy(&tax_amt);
        }
        if (!pbo_decimal_is_null(sub_charge, ebufp))
        {
		pbo_decimal_destroy(&sub_charge);
        }
        if (!pbo_decimal_is_null(total_charge, ebufp))
        {
		pbo_decimal_destroy(&total_charge);
        }
        if (!pbo_decimal_is_null(total_disc, ebufp))
        {
		pbo_decimal_destroy(&total_disc);
        }
        if (!pbo_decimal_is_null(total_staticip_amt, ebufp))
        {
		pbo_decimal_destroy(&total_staticip_amt);
        }
}



/**************************************************************************
* Function:     fm_mso_search_plan_details()
* Owner:        Hrish Kulkarni
* Decription:   For getting the Tax codes for the plan.
**************************************************************************/
extern void fm_mso_search_plan_details(
	pcm_context_t                   *ctxp,
	pin_flist_t                     *i_flistp,
	pin_flist_t                     **o_flistpp,
	pin_errbuf_t                    *ebufp)
{

	pin_flist_t                     *s_flistp = NULL;
	pin_flist_t                     *s_oflistp = NULL;
	pin_flist_t                     *args_flistp = NULL;
	pin_flist_t                     *res_flistp = NULL;
	pin_flist_t                     *city_flistp = NULL;
	pin_flist_t                     *ret_flistp = NULL;
	pin_flist_t                     *plan_flistp = NULL;
	pin_flist_t                     *plan_list_flistp = NULL;
	pin_flist_t                     *taxes_flistp = NULL;

	poid_t                          *s_pdp = NULL;
	poid_t                          *plan_pdp = NULL;
	poid_t                          *acct_pdp = NULL;
	int32                           flags = 512;
	int32                           *tax_flags = NULL;
	int64                           db = 1;
	int64                           id = -1;
	char                            *s_template = " select X from /mso_cfg_credit_limit "
					"where F1 = V1 and ( F2 = V2 or F3 = V3 ) and ( F4 = V4 or F5 = V5 ) ";
	int32                           *bill_when = NULL;
	int32                           non_payterm = 0; // For OTC product
	pin_decimal_t                   *ch_amt = NULL;
	pin_decimal_t                   *cf_amt = NULL;
	pin_decimal_t                   *pf_amt = NULL;
	pin_decimal_t                   *tot_amt = NULL;
	char                            *city = NULL;
	char                            *plan_name = NULL;
	pin_cookie_t                    cookie = NULL;
	int32                           elem_id = 0;
	int32				status = FAILURE;
	int32				all_plans_failure = 1;
	char				*amc_opcode = NULL;	
	pin_flist_t			*rett_flistp = NULL;
	pin_decimal_t			*zero = NULL;
	char                            *s_template2 = " select X from /au_mso_cfg_credit_limit "
                                        "where F1 = V1 and ( F2 = V2 or F3 = V3 ) and ( F4 = V4 or F5 = V5 ) and F6 <= V6 order by created_t desc";
	time_t				*start_t = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Error before calling fm_mso_search_plan_details", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_plan_details input flist", i_flistp);

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	tax_flags = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	ret_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acct_pdp, ebufp);
	amc_opcode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp);
	while((plan_list_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_PLAN,
			&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		tot_amt = NULL;
		status = FAILURE;
		plan_pdp = PIN_FLIST_FLD_GET(plan_list_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
		plan_name = PIN_FLIST_FLD_GET(plan_list_flistp, MSO_FLD_PLAN_NAME, 0, ebufp);
		bill_when = PIN_FLIST_FLD_GET(plan_list_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
		city = PIN_FLIST_FLD_GET(plan_list_flistp, PIN_FLD_CITY, 0, ebufp);
		start_t = PIN_FLIST_FLD_GET(plan_list_flistp, PIN_FLD_PURCHASE_START_T, 1, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_search_plan_details: Mandatory fields missing!!", ebufp);
				return;
		}
		s_flistp = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

		/*******************************************************
		PCM_OP_SEARCH Input:
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
		0 PIN_FLD_FLAGS           INT [0] 768
		0 PIN_FLD_TEMPLATE        STR [0] "select X from /mso_cfg_credit_limit where 
						F1 = V1 and ( F2 = V2 or F3 = V3 ) and ( F4 = V4 or F5 = V5 ) "
		0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
		1     MSO_FLD_PLAN_NAME       STR [0] "HARDWARE RENTAL - CABLE MODEM - PLAN"
		0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
		1     MSO_FLD_CITIES        ARRAY [*] allocated 20, used 1
		2         PIN_FLD_CITY            STR [0] "MUMBAI"
		0 PIN_FLD_ARGS          ARRAY [3] allocated 20, used 1
		1     MSO_FLD_CITIES        ARRAY [*] allocated 20, used 1
		2         PIN_FLD_CITY            STR [0] "*"
		0 PIN_FLD_ARGS          ARRAY [4] allocated 20, used 1
		1     MSO_FLD_CITIES        ARRAY [*] allocated 20, used 1
		2         PIN_FLD_BILL_WHEN       INT [0] 6
		0 PIN_FLD_ARGS          ARRAY [5] allocated 20, used 1
		1     MSO_FLD_CITIES        ARRAY [*] allocated 20, used 1
		2         PIN_FLD_BILL_WHEN       INT [0] 0
		0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 1
		1     MSO_FLD_CITIES        ARRAY [*]
		2         PIN_FLD_BILL_WHEN       INT [0] 0
		2         PIN_FLD_CHARGE_AMT   DECIMAL [0] NULL
		2         PIN_FLD_CITY            STR [0] ""
		2         PIN_FLD_CYCLE_FEE_AMT   DECIMAL [0] NULL
		2         PIN_FLD_PURCHASE_FEE_AMT   DECIMAL [0] NULL

		PCM_OP_SEARCH Output:
		0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
		0 PIN_FLD_RESULTS       ARRAY [*] allocated 20, used 2
		1     PIN_FLD_POID           POID [0] 0.0.0.1 /mso_cfg_credit_limit 2896858 0
		1     MSO_FLD_CITIES        ARRAY [1] allocated 20, used 3
		2         PIN_FLD_BILL_WHEN       INT [0] 6
		2         PIN_FLD_CHARGE_AMT   DECIMAL [0] 3300
		2         PIN_FLD_CITY            STR [0] "MUMBAI"
		2         PIN_FLD_CYCLE_FEE_AMT   DECIMAL [0] 1500
		2         PIN_FLD_PURCHASE_FEE_AMT   DECIMAL [0] 1800
		*******************************************************/
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		if(amc_opcode && !strcmp(amc_opcode, "Cancel_Rounding_Call"))
                {
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template2, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
			args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
                        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREATED_T, start_t, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PLAN_NAME, plan_name, ebufp);
		}
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CITY, city, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CITY, "*", ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(args_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, &non_payterm, ebufp);
				
		res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CREATED_T, NULL, ebufp);
		city_flistp = PIN_FLIST_ELEM_ADD(res_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CHARGE_AMT, NULL, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CITY, "", ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_CYCLE_FEE_AMT, NULL, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_PURCHASE_FEE_AMT, NULL, ebufp);
		PIN_FLIST_FLD_SET(city_flistp, PIN_FLD_AMOUNT_ORIG, NULL, ebufp);		

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Input flist:",s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_plan_details PCM_OP_SEARCH error.", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_cfg_credit_limit Output flist:",s_oflistp);

		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " After reading PIN_FLD_RESULTS");
		plan_flistp = PIN_FLIST_ELEM_ADD(ret_flistp, PIN_FLD_PLAN, elem_id, ebufp);
		PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(plan_flistp, MSO_FLD_PLAN_NAME, plan_name, ebufp);
		PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_BILL_WHEN, bill_when, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " After initializing ret_flistp");

		if (res_flistp != NULL)
		{
			city_flistp = PIN_FLIST_ELEM_GET(res_flistp, MSO_FLD_CITIES, PIN_ELEMID_ANY, 1, ebufp);
			bill_when = PIN_FLIST_FLD_GET(city_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
			cf_amt = PIN_FLIST_FLD_GET(city_flistp, PIN_FLD_CYCLE_FEE_AMT, 1, ebufp);
			pf_amt = PIN_FLIST_FLD_GET(city_flistp, PIN_FLD_PURCHASE_FEE_AMT, 1, ebufp);
			ch_amt = PIN_FLIST_FLD_GET(city_flistp, PIN_FLD_CHARGE_AMT, 1, ebufp);
			/*changes made for AMC and also for the checking the actual amount for discount*/
			//amc_opcode = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 1, ebufp);
			if(amc_opcode != NULL &&  !strcmp(amc_opcode, "DISC AMT"))
			{
				rett_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(rett_flistp, PIN_FLD_CYCLE_FEE_AMT, ch_amt, ebufp);
				*o_flistpp = rett_flistp;
				return;
			}
			else if(amc_opcode != NULL && !strcmp(amc_opcode, "AMC CALL"))
                        {
                                rett_flistp = PIN_FLIST_CREATE(ebufp);
                                PIN_FLIST_FLD_SET(rett_flistp, PIN_FLD_CYCLE_FEE_AMT, cf_amt, ebufp);
                                *o_flistpp = rett_flistp;
                                return;
                        }
			else if(amc_opcode != NULL && (!strcmp(amc_opcode, "Activation_Rounding_Call") || !strcmp(amc_opcode,"Cancel_Rounding_Call")))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " check1");
				*o_flistpp = PIN_FLIST_COPY(s_oflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&s_oflistp,NULL);
                                return;
			}
			// Call the function to get the tax code and tax amount
			fm_mso_search_tax_code(ctxp, res_flistp, acct_pdp, bill_when, ch_amt, 
						cf_amt, pf_amt, plan_name, &tot_amt, &taxes_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
					"fm_mso_search_plan_details: Error in funtion fm_mso_search_tax_code()");
				// Will not be throwing err buffer. Will just set the status to failure to indicate this.
				PIN_ERRBUF_RESET(ebufp);
				PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_STATUS, &status, ebufp);
				goto CLEANUP;
			}
			status = SUCCESS;
			all_plans_failure = 0;
			PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_AMOUNT, tot_amt, ebufp);
		} else {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Setting status");
			PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_STATUS, &status, ebufp);
			
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After Setting status");
		}
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
	}


	if(!all_plans_failure) {
	    PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &all_plans_failure, ebufp);
	} else {
	    PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &all_plans_failure, ebufp);
	    PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51685", ebufp);
	    PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error in calcuating price for plans", ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ret_flistp is:", ret_flistp);	
	*o_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "o_flistpp is:", *o_flistpp);	

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

	return;
}

/**************************************************************************
* Function:     fm_mso_search_tax_code()
* Owner:        Hrish Kulkarni
* Decription:   For getting the Tax codes for the plan.
**************************************************************************/
extern
void fm_mso_search_tax_code(
	pcm_context_t                   *ctxp,
	pin_flist_t                     *i_flistp,
	poid_t                          *acct_pdp,
	int32                           *bill_when,
	pin_decimal_t                   *ch_amt,
	pin_decimal_t                   *cf_amt,
	pin_decimal_t                   *pf_amt,
	char                            *plan_name,
	pin_decimal_t                   **tot_amt,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t                    *ebufp)
{
	pin_flist_t                     *s_flistp = NULL;
	pin_flist_t                     *s_oflistp = NULL;
	pin_flist_t                     *args_flistp = NULL;
	pin_flist_t                     *res_flistp = NULL;
	pin_flist_t                     *res_oflistp = NULL;
	pin_flist_t                     *res_iflistp = NULL;
	pin_flist_t                     *prod_flistp = NULL;
	pin_flist_t                     *prod_arg_flistp = NULL;
	pin_flist_t                     *serv_flistp = NULL;
	pin_flist_t                     *vat_flistp = NULL;
	pin_flist_t                     *u_flistp = NULL;

	poid_t                          *s_pdp = NULL;
	poid_t                          *prod_pdp = NULL;
	poid_t                          *prodi_pdp = NULL;
	poid_t                          *prodo_pdp = NULL;
	int32                           flags = 256;
	int32                           elem_count = 0;
	int64                           db = 1;
	int64                           id = -1;
	char                            *s_template = " select X from /rate_plan 1, /product 2, /deal 3, "
						"/plan 4 where 1.F1 = 2.F2 and 3.F3 = 2.F4 and "
						"3.F5 = 4.F6 and 4.F7 = V7 and 1.F8 not like V8 and 1.F9 <> V9 ";
	char                            *tax_code = NULL;
	pin_cookie_t                    cookie = NULL;
	int32                           elem_id = 0;
	pin_cookie_t                    cookie1 = NULL;
	int32                           elem_id1 = 0;
	pin_cookie_t                    cookie2 = NULL;
	int32                           elem_id2 = 0;
	int32                           cpy_status = 0;
	int                             cf_flag = 0;
	int                             pf_flag = 0;
	int                             st_flag = 0;
	int                             vat_flag = 0;
	int                             cntr = 0;
	char                            *st_string = "MSO_Service_Tax";
	char                            *vat_string = "MSO_VAT";
	pin_decimal_t                   *vat_rate = NULL;
	pin_decimal_t                   *tax_amt = NULL;
	pin_decimal_t                   *prod_priority = NULL;
	pin_decimal_t                   *st_tax_amt = NULL;
	pin_decimal_t                   *vat_tax_amt = NULL;
	pin_decimal_t			*otc_tax_amt = NULL;
	pin_decimal_t			*rc_tax_amt = NULL;
	char                            tmp[256];
	double                          prod_priority_double = 0;

	pin_flist_t			*taxcodes_flistp = NULL;
	pin_flist_t			*taxedamt_flistp = NULL;
	pin_flist_t			*taxes_flistp = NULL;
	pin_flist_t			*taxchg_flistp  = NULL;
	int				taxmap_flags = 0;
	int				tax_elemid = 0;
	pin_cookie_t			tax_cookie = NULL;
	char				*taxes_taxcode = NULL;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"Error before calling fm_mso_search_tax_code");
			return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_search_tax_code input flist", i_flistp);

	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	/*******************************************************
	PCM_OP_SEARCH Input:
	0 PIN_FLD_POID POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_FLAGS INT [0] 768
	0 PIN_FLD_TEMPLATE STR [0] "select X from /rate_plan 1, /product 2, /deal 3, /plan 4 
	where 1.F1 = 2.F2 and 3.F3 = 2.F4 and 3.F5 = 4.F6 and 4.F7 = V7 and 1.F8 is not null "
	0 PIN_FLD_ARGS ARRAY [1]
	1   PIN_FLD_PRODUCT_OBJ POID [0] NULL
	0 PIN_FLD_ARGS ARRAY [2]
	1   PIN_FLD_POID  POID [0] NULL
	0 PIN_FLD_ARGS ARRAY [3]
	1   PIN_FLD_PRODUCTS ARRAY [*]
	2     PIN_FLD_PRODUCT_OBJ  POID [0] NULL
	0 PIN_FLD_ARGS ARRAY [4]
	1   PIN_FLD_POID  POID [0] NULL
	0 PIN_FLD_ARGS ARRAY [5]
	1   PIN_FLD_POID  POID [0] NULL
	0 PIN_FLD_ARGS ARRAY [6]
	1   PIN_FLD_SERVICES  ARRAY [*]
	2     PIN_FLD_DEAL_OBJ   POID [0] NULL
	0 PIN_FLD_ARGS ARRAY [7]
	1   PIN_FLD_NAME STR [0] "POWER STREAM2 BLR 50Mbps 6MonU"
	0 PIN_FLD_ARGS ARRAY [8]
	1   PIN_FLD_TAX_CODE STR [0] ""
	0 PIN_FLD_RESULTS ARRAY [0]
	1   PIN_FLD_TAX_CODE STR [0] NULL
	1   PIN_FLD_PRODUCT_OBJ  POID [0] NULL

	PCM_OP_SEARCH Output:
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 3
	1     PIN_FLD_TAX_CODE        STR [0] "MSO_Service_Tax"
	1     PIN_FLD_PRODUCT_OBJ    POID [0] 0.0.0.1 /product 2899530 0
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /rate_plan 2898250 0
	0 PIN_FLD_RESULTS       ARRAY [1] allocated 20, used 3
	1     PIN_FLD_TAX_CODE        STR [0] "MSO_Service_Tax"
	1     PIN_FLD_PRODUCT_OBJ    POID [0] 0.0.0.1 /product 2899530 5
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /rate_plan 2887895 5
	
	*******************************************************/
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	prod_arg_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(prod_arg_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
	serv_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(serv_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_NAME, plan_name, ebufp);

//	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
//	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TAX_CODE, "", ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_TAX_CODE, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EVENT_TYPE, "%mso_grant", ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EVENT_TYPE, "/event/session/telco/broadband", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Tax Code Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_tax_code PCM_OP_SEARCH error.");
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Tax Code Output flist:",s_oflistp);
	elem_count = PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp);
	sprintf(tmp,"elem_count is: %d", elem_count);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
	if (elem_count == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"There are no taxes associated with the product!");
		*tot_amt = pbo_decimal_copy(ch_amt, ebufp);
		goto CLEANUP;
	}
	//if (!pbo_decimal_is_null(cf_amt, ebufp) && !pbo_decimal_is_zero(cf_amt, ebufp))
	if (!pbo_decimal_is_null(cf_amt, ebufp))
	{
		cf_flag = 1;
	}
	//if (!pbo_decimal_is_null(pf_amt, ebufp) && !pbo_decimal_is_zero(pf_amt, ebufp))
	if (!pbo_decimal_is_null(pf_amt, ebufp))
	{
		pf_flag = 1;
	}
	if (elem_count == 1)
	{
		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		tax_code = (char *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
		calculate_gst(ctxp, acct_pdp, tax_code, ch_amt, &tax_amt, &taxchg_flistp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(1, "error message 2", ebufp);
		}
		*tot_amt = pbo_decimal_copy(tax_amt, ebufp);
		PIN_ERR_LOG_FLIST(3, "Returned Tax Flistp", taxchg_flistp);
						if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(1, "error message 3", ebufp);
		}
	}
	else
	{
		u_flistp = PIN_FLIST_CREATE(ebufp);
		cpy_status = PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_POID, u_flistp, PIN_FLD_POID, ebufp);
		while((res_oflistp = PIN_FLIST_ELEM_GET_NEXT(s_oflistp, PIN_FLD_RESULTS,
			&elem_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			prodo_pdp = PIN_FLIST_FLD_GET(res_oflistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
			if(cntr == 0)
			{
				cpy_status = PIN_FLIST_ELEM_COPY(s_oflistp, PIN_FLD_RESULTS, elem_id1, 
						u_flistp, PIN_FLD_RESULTS, elem_id1, ebufp);
				cntr++;
				continue;
			}
			elem_id2 = 0;
			cookie2 = NULL;	
			while((res_iflistp = PIN_FLIST_ELEM_GET_NEXT(u_flistp, PIN_FLD_RESULTS,
				&elem_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL)
			{
				prodi_pdp = PIN_FLIST_FLD_GET(res_iflistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
				if(PIN_POID_COMPARE(prodo_pdp, prodi_pdp, 0, ebufp) != 0)
				{
					cpy_status = PIN_FLIST_ELEM_COPY(s_oflistp, PIN_FLD_RESULTS, elem_id1, 
						u_flistp, PIN_FLD_RESULTS, elem_id1, ebufp);
				}
			}

		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Unique u_flistp: ", u_flistp);
				PIN_ERR_LOG_FLIST(3, "Returned Tax Flistp", taxchg_flistp);
						if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(1, "error message new 1", ebufp);
		}

		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Both ST and VAT are applicable");
		elem_id = 0;
		cookie = NULL;
		while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(u_flistp, PIN_FLD_RESULTS,
				&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "res_flistp:", res_flistp);
			tax_code = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
			prod_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
			if(cf_flag == 1 && pf_flag == 1)
			{
				// Get the product priority details
				fm_mso_utils_read_any_object(ctxp, prod_pdp, &prod_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
							"fm_mso_search_tax_code: Error in reading product poid.");
					goto CLEANUP;
				}
				prod_priority = PIN_FLIST_FLD_GET(prod_flistp, PIN_FLD_PRIORITY, 1, ebufp);
				if (!pbo_decimal_is_null(prod_priority, ebufp))
					prod_priority_double = pbo_decimal_to_double(prod_priority, ebufp);

				// Check if the priority is for One-Time-Charges. If so, apply the tax based on
				// the tax code and the /mso_cfg_credit_limit.PIN_FLD_PURCHASE_FEE_AMT
				if ((prod_priority_double >= BB_HW_DEPOSIT_RANGE_START &&
					prod_priority_double <= BB_HW_DEPOSIT_RANGE_END) ||
					(prod_priority_double >= BB_HW_OUTRIGHT_RANGE_START &&
					prod_priority_double <= BB_HW_OUTRIGHT_RANGE_END)
				)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Processing the OTC product");


					if (tax_code)
					{
						// ST Amount
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Processing the ST ");
						taxmap_flags = 1;
						calculate_gst(ctxp, acct_pdp, tax_code, pf_amt, &tax_amt, &taxchg_flistp, ebufp);
						//fm_mso_apply_vat(ctxp, tax_code, taxmap_flags, pf_amt, &taxedamt_flistp, ebufp);
						//st_tax_amt = PIN_FLIST_FLD_TAKE(taxedamt_flistp, PIN_FLD_AMOUNT_GROSS, 1, ebufp);
						otc_tax_amt = pbo_decimal_copy(tax_amt, ebufp);
					}
				}
				else
				{
					// Check if the priority is for Recurring-Charges. If so, apply the tax based on
					// the tax code and the /mso_cfg_credit_limit.PIN_FLD_CYCLE_FEE_AMT
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Processing the Recurring product");
					if (tax_code)
					{
						// ST Amount
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Processing the ST ");
						taxmap_flags= 1;
						calculate_gst(ctxp, acct_pdp, tax_code, cf_amt, &tax_amt, &taxchg_flistp, ebufp);
						//fm_mso_apply_vat(ctxp, tax_code, taxmap_flags, cf_amt, &taxedamt_flistp, ebufp);
						//st_tax_amt = PIN_FLIST_FLD_TAKE(taxedamt_flistp, PIN_FLD_AMOUNT_GROSS, 1, ebufp);
						rc_tax_amt = pbo_decimal_copy(tax_amt, ebufp);
					}
				}
				PIN_FLIST_DESTROY_EX(&prod_flistp, NULL);
			}
		}
		if(!pbo_decimal_is_null(otc_tax_amt, ebufp) && !pbo_decimal_is_null(rc_tax_amt, ebufp))
		{
			*tot_amt = pbo_decimal_add(otc_tax_amt, rc_tax_amt, ebufp);
		}
		else if (!pbo_decimal_is_null(otc_tax_amt, ebufp))
		{
			*tot_amt = pbo_decimal_copy(otc_tax_amt, ebufp);
		}
		else if (!pbo_decimal_is_null(rc_tax_amt, ebufp))
		{
			*tot_amt = pbo_decimal_copy(rc_tax_amt, ebufp);
		}

//------------------------------------
	}
	PIN_ERR_LOG_FLIST(3, "Returned Tax Flistp", taxchg_flistp);
	*ret_flistp = taxchg_flistp;
	PIN_ERR_LOG_FLIST(3, "Final Tax Flistp", *ret_flistp);
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&prod_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&vat_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&u_flistp, NULL);
		
		if (!pbo_decimal_is_null(otc_tax_amt, ebufp))
		{
			pbo_decimal_destroy(&otc_tax_amt);
		}
		if (!pbo_decimal_is_null(rc_tax_amt, ebufp))
		{
			pbo_decimal_destroy(&rc_tax_amt);
		}
	return;
}

/**************************************************************************
* Function:     fm_mso_get_vat_rate()
* Owner:        Hrish Kulkarni
* Decription:   For getting the Tax codes for the plan.
**************************************************************************/
static void fm_mso_get_vat_rate(
	pcm_context_t                   *ctxp,
	poid_t                          *a_pdp,
	pin_decimal_t                   *ch_amt,
	pin_flist_t                     **o_flistpp,
	pin_errbuf_t                    *ebufp)
{
	pin_flist_t                     *s_flistp = NULL;
	pin_flist_t                     *s_oflistp = NULL;
	pin_flist_t                     *args_flistp = NULL;
	pin_flist_t                     *res_flistp = NULL;
	poid_t                          *s_pdp = NULL;
	int32                           flags = 768;
	int64                           db = 1;
	int64                           id = -1;
	char                            *s_template = " select X from /mso_cfg_vat_mapping 1, /account 2 "
					"where 2.F1 = V1 and mso_cfg_vat_mapping_t.tax_zone = account_t.vat_zone||'_BB' ";
	char                            *tax_code = NULL;

	int				taxmap_flags = 0;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Error before calling fm_mso_get_vat_rate", ebufp);
		return;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_get_vat_rate");

	/*******************************************************
	PCM_OP_SEARCH Input:
	0 PIN_FLD_POID POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_FLAGS INT [0] 256
	0 PIN_FLD_TEMPLATE STR [0] "select X from /mso_cfg_vat_mapping 1, /account 2 
					where 2.F1 = V1 and 
					mso_cfg_vat_mapping_t.tax_zone = account_t.vat_zone||'_BB' "
	0 PIN_FLD_ARGS ARRAY [1]
	1   PIN_FLD_POID POID [0] 0.0.0.1 /account 111001
	0 PIN_FLD_RESULTS ARRAY [0] 
	1   PIN_FLD_TAX_CODE   STR [0] ""

	PCM_OP_SEARCH Output:
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 2
	1     PIN_FLD_TAX_CODE        STR [0] "VAT_14_5"
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /mso_cfg_vat_mapping 2110056 0
	*******************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, a_pdp, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_TAX_CODE, "", ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search VAT Zone Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"fm_mso_get_vat_rate PCM_OP_SEARCH error.");
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search VAT Zone Output flist:",s_oflistp);
	res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	tax_code = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_TAX_CODE, 0, ebufp);

	// Get the vat amount
	fm_mso_apply_vat(ctxp, tax_code, taxmap_flags, ch_amt, o_flistpp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"fm_mso_get_vat_rate: fm_mso_apply_vat() function error.");
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_vat_rate: fm_mso_apply_vat Output flist:",*o_flistpp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);

	return;

}

/**************************************************************************
* Function:     fm_mso_apply_vat()
* Owner:        Hrish Kulkarni
* Decription:   For getting the Tax amount based on the cached VAT Table.
**************************************************************************/
static void
fm_mso_apply_vat(
	pcm_context_t       *ctxp,
	char                *taxcode,
	int		    flags,	
	pin_decimal_t       *amt_gross,
	pin_flist_t         **r_flistpp,
	pin_errbuf_t        *ebufp)
{

	pin_flist_t             *t_flistp = NULL;   /* in taxes array */
	pin_flist_t             *flistp = NULL;
	pin_flist_t             *ret_flistp = NULL;
	pin_cookie_t            cookie = NULL;
	int32                   elemid = 0;
	int32                   elemcnt = 0;
	int32                   j_level = 0;
	char                    *j_list = NULL;
	char                    *descr = NULL;
	char                    *str_tax_rate = NULL;
	void                    *vp = NULL;
	char                    *locale = NULL;
	char                    city[TOK_LEN];
	char                    state[TOK_LEN];
	char                    zip[TOK_LEN];
	char                    county[TOK_LEN];
	char                    country[TOK_LEN];
	char                    code[TOK_LEN];
	char                    name[STR_LEN];
	char                    msg[STR_LEN];
	pin_decimal_t           *tax_rate=NULL;
	pin_decimal_t           *tot_tax=NULL;
	pin_decimal_t           *tax_amt=NULL;
	pin_decimal_t           *amt_taxed = NULL;
	pin_decimal_t           *amt_exempt = NULL;
	pin_decimal_t           *amt_with_tax = NULL;
	int32                   tax_pkg = PIN_RATE_TAXPKG_CUSTOM;
	int32                   tax_rule = 0;
	int32                   loc_mode = 0;
	int32                   round_mode = ROUND_HALF_UP;
	time_t                  *startTm = NULL;
	time_t                  *endTm = NULL;
	time_t                  *transDt = NULL;
	time_t                  now = 0;
	int64                   db = 1;
	int64                   id = -1;
	poid_t                  *dmy_pdp = NULL;
	
	pin_flist_t		*main_tablep = NULL;

	ret_flistp = PIN_FLIST_CREATE(ebufp);
	dmy_pdp = PIN_POID_CREATE(db, "/dummy", id, ebufp);
	PIN_FLIST_FLD_PUT(ret_flistp, PIN_FLD_POID, dmy_pdp, ebufp);

	if (transDt == (time_t*)NULL) {
			/* not available. use current time */
			now = pin_virtual_time((time_t*)NULL);
			transDt = &now;
	}
	str_tax_rate = pbo_decimal_to_str(amt_gross,ebufp);
	sprintf(msg, "Amount to be Taxed is %s.", str_tax_rate);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

	if(flags == 0){
		main_tablep = PIN_FLIST_COPY(VAT_Table,ebufp);
	} else if(flags == 1){
		main_tablep = PIN_FLIST_COPY(Tax_Table,ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Cached extern taxcode map flist",main_tablep);
	elemid = 0; cookie = NULL;
	/* compute tax for each matching taxcode in the table */
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(main_tablep,
			PIN_FLD_TAXES, &elemid, 1, &cookie, ebufp)) != (pin_flist_t*)NULL) {

			/* get the taxcode */
			vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_TAX_CODE,
					1, ebufp);

			/* taxcode match? */
			if (vp) {
					if (strcmp((char*)vp, taxcode) != 0) {
							/* taxcode did not match */
							sprintf(msg,"Checking taxcode %s with %s. Skipping ...",
									taxcode, (char*)vp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
							continue;
							/*******/
					} else {
							/* taxcode matches */
							sprintf(msg, "Checking taxcode %s with %s. Matched!",
									taxcode, (char*)vp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					}
			}
			else {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, "no tax code available");
					continue;
			}
			/* get validity dates */
			startTm = (time_t*) PIN_FLIST_FLD_GET(flistp, PIN_FLD_START_T, 1, ebufp);
			endTm = (time_t*) PIN_FLIST_FLD_GET(flistp, PIN_FLD_END_T, 1, ebufp);

			/* tax rate valid? */
			if (startTm && endTm && *startTm && *endTm &&
					((*transDt < *startTm) || (*transDt >= *endTm))) {
					/* tax rate is not valid */
					sprintf(msg,
							"Taxcode %s has expired rate. Skipping ...\n",
							taxcode);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					continue;
					/*******/
			} else {
					/* tax rate is valid */
					sprintf(msg, "Taxcode %s has valid rate!\n",
							taxcode);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}

			/* get jurisdiction level */
			vp =  PIN_FLIST_FLD_GET(flistp,
					PIN_FLD_TYPE, 1, ebufp);
			if (vp) {
					j_level = *(int32 *)vp;
			}

			descr = (char*) PIN_FLIST_FLD_GET(flistp, PIN_FLD_DESCR, 1, ebufp);
			/* get tax rate */
			vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_PERCENT,
					1, ebufp);
			if (vp == (void*)NULL) {
					/* no tax rate?  default to zero */
					tax_rate = pbo_decimal_from_str("0.0",ebufp);
			} else {
					tax_rate = pbo_decimal_copy((pin_decimal_t*)vp,
							ebufp);
			}

					str_tax_rate = pbo_decimal_to_str(tax_rate,ebufp);
					sprintf(msg, "Tax rate for taxcode %s is %s.",
							taxcode,str_tax_rate);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

					/* get tax rule */
					vp =  PIN_FLIST_FLD_GET(flistp, PIN_FLD_TAX_WHEN, 1, ebufp);
					if (vp) {
							tax_rule = *(int32*)vp;
					}
					else {
							tax_rule = 0;
					}
					amt_taxed = pbo_decimal_copy(amt_gross, ebufp);
					if(PIN_ERRBUF_IS_ERR(ebufp))
					{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Assignment ERROR");
							return;
					}

							/* The tax_rate is a percentage */
							str_tax_rate = pbo_decimal_to_str(amt_taxed,ebufp);
							sprintf(msg, "Amount Gross is %s.", str_tax_rate);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
							tax_amt = pbo_decimal_multiply(amt_taxed, tax_rate, ebufp);
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
									"Standard tax rule applied.");
							str_tax_rate = pbo_decimal_to_str(tax_amt,ebufp);
							sprintf(msg, "Tax Amount = %s.", str_tax_rate);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			}
					if(PIN_ERRBUF_IS_ERR(ebufp))
					{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Applying ERROR");
							return;
					}
			amt_with_tax = pbo_decimal_add(tax_amt, amt_gross, ebufp);	
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_TAX, tax_amt, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_PERCENT, tax_rate, ebufp);
			PIN_FLIST_FLD_PUT(ret_flistp, PIN_FLD_AMOUNT_GROSS, amt_with_tax, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_AMOUNT_TAXED, amt_taxed, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, descr, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_TYPE, &j_level, ebufp);
			*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"VAT Processing Return FLIST", *r_flistpp);

			if (str_tax_rate)
			{
				free(str_tax_rate);
			}
			if (tax_amt)
			{
				pbo_decimal_destroy(&tax_amt);
			}

			if (tax_rate)
			{
				pbo_decimal_destroy(&tax_rate);
			}
			if (amt_taxed)
			{
				pbo_decimal_destroy(&amt_taxed);
			}
			PIN_FLIST_DESTROY_EX(&ret_flistp, ebufp);
}



void calculate_st(pcm_context_t       *ctxp,
        char                *tax_code,
        pin_decimal_t       *amt_gross,
	pin_decimal_t	    **tax_amt,
	pin_flist_t		**taxapplied_flistp,
        pin_errbuf_t        *ebufp) 
{
    pin_flist_t	*taxcodes_flistp = NULL;
    pin_flist_t	*taxes_flistp = NULL;
    pin_flist_t	*taxedamt_flistp = NULL;
    pin_flist_t	*tax_flistp = NULL;
    pin_decimal_t	*taxes_taxcode = NULL;
    int32		tax_elemid = 0;
    pin_cookie_t	tax_cookie = NULL;
    int32		taxmap_flags = 0;
    char                            *st_string = "MSO_Service_Tax";
    char		debug_msg[250];

    *tax_amt = pbo_decimal_from_str("0.0", ebufp);


	if(tax_code && (strcmp(tax_code,"") != 0))
	{
		if(strcmp(tax_code, st_string) == 0) {
		    taxmap_flags = 1;
		}
		taxcodes_flistp = PIN_FLIST_CREATE(ebufp);
		mso_getTaxCodelist(ctxp,tax_code, 0, &taxcodes_flistp,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Taxes to apply",taxcodes_flistp);
		if((taxcodes_flistp != NULL) && (PIN_FLIST_ELEM_COUNT(taxcodes_flistp,PIN_FLD_TAXES,ebufp)>0))
		{
		    tax_elemid = 0; tax_cookie = NULL;
			*taxapplied_flistp = PIN_FLIST_CREATE(ebufp);
		    while((taxes_flistp = PIN_FLIST_ELEM_GET_NEXT(taxcodes_flistp, PIN_FLD_TAXES,
			&tax_elemid, 1, &tax_cookie, ebufp )) != NULL)
		    {
			taxes_taxcode = PIN_FLIST_FLD_GET(taxes_flistp,PIN_FLD_TAX_CODE,0,ebufp);
			fm_mso_apply_vat(ctxp, taxes_taxcode, taxmap_flags, amt_gross, &taxedamt_flistp, ebufp);
			*tax_amt = pbo_decimal_add(*tax_amt, PIN_FLIST_FLD_GET(taxedamt_flistp, PIN_FLD_TAX, 0, ebufp), ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Taxed amount",taxedamt_flistp);
			if(taxedamt_flistp){
				tax_flistp = PIN_FLIST_ELEM_ADD(*taxapplied_flistp, PIN_FLD_TAXES, tax_elemid, ebufp);
				PIN_FLIST_FLD_SET(tax_flistp, PIN_FLD_TAX_CODE, taxes_taxcode, ebufp);
				PIN_FLIST_CONCAT(tax_flistp, taxedamt_flistp, ebufp);
			}
			PIN_FLIST_DESTROY_EX(&taxedamt_flistp,NULL);
		    }
		}
	}
    sprintf(debug_msg, "Tax amount calculated is %s", pbo_decimal_to_str(*tax_amt, ebufp));
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);  
    *tax_amt = pbo_decimal_add(*tax_amt, amt_gross, ebufp);
    sprintf(debug_msg, "Total amount calculated is %s", pbo_decimal_to_str(*tax_amt, ebufp));
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);  
    return;
}


void calculate_gst(
	pcm_context_t     	*ctxp,
	poid_t			*acct_pdp,
	char                	*tax_code,
	pin_decimal_t       	*amt_gross,
	pin_decimal_t       	**tax_amt,
	pin_flist_t        	**taxapplied_flistp,
	pin_errbuf_t        	*ebufp)
{
    	pin_flist_t 		*taxcodes_flistp = NULL;
    	pin_flist_t 		*taxes_flistp = NULL;
    	pin_flist_t 		*taxedamt_flistp = NULL;
    	pin_flist_t 		*tax_flistp = NULL;
    	pin_flist_t 		*gst_rate_flistp = NULL;
	pin_flist_t		*gst_flistp = NULL;
    

	pin_decimal_t       	*taxes_taxcode = NULL;
	pin_decimal_t		*gst_rate = NULL;

    	int32               	tax_elemid = 0;
    	pin_cookie_t        	tax_cookie = NULL;
    	int32               	taxmap_flags = 0;
    	char                	*st_string = "MSO_Service_Tax";
    	char                	msg[250];
        char                    *statep = NULL;
        pin_decimal_t           *add_cessp = NULL;
	int32			gst_type = -1;
        int32                   precision = 2;
        int32                   round_mode = ROUND_HALF_UP;


    	*tax_amt = pbo_decimal_from_str("0.0", ebufp);
	if(tax_code && (strcmp(tax_code,"") != 0))
	{
		if(strcmp(tax_code, st_string) == 0) {
			taxmap_flags = 1;
		}
        	fm_mso_utils_get_profile(ctxp, acct_pdp, &gst_flistp, ebufp);
        	PIN_ERR_LOG_FLIST(3, "Gst FListp", gst_flistp);
        	if(gst_flistp)
        	{
                	gst_type = *(int32 *)PIN_FLIST_FLD_GET(gst_flistp, PIN_FLD_CONN_TYPE, 0, ebufp);
        	}
        	if(PIN_ERRBUF_IS_ERR(ebufp) || gst_type == -1)
        	{
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error is getting GST Type");
                	goto CLEANUP;
        	}

		mso_retrieve_gst_rate(ctxp, tax_code, acct_pdp, &gst_rate_flistp, ebufp);
        statep = PIN_FLIST_FLD_GET(gst_rate_flistp, PIN_FLD_STATE, 1, ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in getting GST Rates");
                        goto CLEANUP;
                }

		taxcodes_flistp = PIN_FLIST_CREATE(ebufp);
        if (statep && strstr(statep, "KERALA"))
        {
            gst_type = 3;
        }
		mso_getTaxCodelist(ctxp, tax_code, gst_type, &taxcodes_flistp, ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Error in getting tax code");
                        goto CLEANUP;
                }

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Taxes to apply", taxcodes_flistp);
		if(taxcodes_flistp)
		{
			*taxapplied_flistp = PIN_FLIST_CREATE(ebufp);
			while((taxes_flistp = PIN_FLIST_ELEM_GET_NEXT(taxcodes_flistp, PIN_FLD_TAXES, &tax_elemid, 1, &tax_cookie, ebufp )) != NULL)
			{
				taxes_taxcode = PIN_FLIST_FLD_GET(taxes_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
                		if (taxes_taxcode && (strcmp(taxes_taxcode, IGST)==0 || strcmp(taxes_taxcode, CGST)==0 || strcmp(taxes_taxcode, SGST)==0 || strcmp(taxes_taxcode, ADD_CESS)==0))
                		{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "GST Tax Code Matched");
					if(taxes_taxcode && strcmp(taxes_taxcode, IGST)==0)
					{
						gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_IGST_RATE, 0, ebufp);     //IGST
					}
					else if(taxes_taxcode && strcmp(taxes_taxcode, SGST)==0)
					{
						gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_SGST_RATE, 0, ebufp);     //SGST
					}
					else if(taxes_taxcode && strcmp(taxes_taxcode, CGST)==0)
					{
						gst_rate = PIN_FLIST_FLD_GET(gst_rate_flistp, MSO_FLD_CGST_RATE, 0, ebufp);                     //CGST
					}
				}

				sprintf(msg,"Tax Rate is : %s ", pbo_decimal_to_str(gst_rate, ebufp));
			    if (taxes_taxcode && strcmp(taxes_taxcode, ADD_CESS)==0)
                {
                    add_cessp = pbo_decimal_from_str("0.01", ebufp);
                    mso_gst_apply_tax(ctxp, add_cessp, amt_gross, &taxedamt_flistp, ebufp);
                    pbo_decimal_destroy(&add_cessp);
                }
                else
                {
				    mso_gst_apply_tax(ctxp, gst_rate, amt_gross, &taxedamt_flistp, ebufp);
                }
				if(taxedamt_flistp)
				{
					pbo_decimal_add_assign(*tax_amt,
 						pbo_decimal_round(PIN_FLIST_FLD_GET(taxedamt_flistp, PIN_FLD_TAX, 0, ebufp), precision, round_mode, ebufp),
						ebufp);
					tax_flistp = PIN_FLIST_ELEM_ADD(*taxapplied_flistp, PIN_FLD_TAXES, tax_elemid, ebufp);
					PIN_FLIST_FLD_SET(tax_flistp, PIN_FLD_TAX_CODE, taxes_taxcode, ebufp);
					PIN_FLIST_CONCAT(tax_flistp, taxedamt_flistp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Taxed amount", taxedamt_flistp);
					PIN_FLIST_DESTROY_EX(&taxedamt_flistp,NULL);
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Temp Ret Flistp", *taxapplied_flistp);
			}
		}
	}
    	sprintf(msg, "Tax amount calculated is %s", pbo_decimal_to_str(*tax_amt, ebufp));
    	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
    	pbo_decimal_add_assign(*tax_amt, amt_gross, ebufp);
    	sprintf(msg, "Total amount calculated is %s", pbo_decimal_to_str(*tax_amt, ebufp));
    	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Tax Return Flistp", *taxapplied_flistp);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&gst_flistp, NULL);
    	return;
}

static void
mso_gst_apply_tax(
        pcm_context_t   *ctxp,
        pin_decimal_t   *tax_rate,
        pin_decimal_t   *amt_gross,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{
        int32                   j_level = 0;
        char                    *descr = "GST Tax Component";
        pin_decimal_t           *tax_amt=NULL;
        pin_decimal_t           *total_amt = NULL;
        char                    msg [255];

        sprintf(msg, "Amount to be Taxed is %s.", pbo_decimal_to_str(amt_gross, ebufp));
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

        tax_amt = pbo_decimal_multiply(amt_gross, tax_rate, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Standard tax rule applied.");
        sprintf(msg, "Tax Amount = %s.", pbo_decimal_to_str(tax_amt, ebufp));
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
        if(PIN_ERRBUF_IS_ERR(ebufp))
        {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Tax Calculation ERROR");
			return;
        }

		total_amt = pbo_decimal_add(amt_gross, tax_amt, ebufp);

		*r_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_TAX, tax_amt, ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_PERCENT, tax_rate, ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT_GROSS, total_amt, ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT_TAXED, amt_gross, ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DESCR, descr, ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_TYPE, &j_level, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"GST Processing Return FLIST", *r_flistpp);
        if (!pbo_decimal_is_null(tax_amt, ebufp))
        {
                pbo_decimal_destroy(&tax_amt);
        }
        if (!pbo_decimal_is_null(total_amt, ebufp))
        {
                pbo_decimal_destroy(&total_amt);
        }
        return;
}


void fm_mso_cust_catv_calc_price(
        pcm_context_t       *ctxp,
        pin_flist_t       *i_flistp,
        pin_flist_t        **r_flistpp,
        pin_errbuf_t        *ebufp)
{
	pin_flist_t	*srch_i_flistp = NULL;
	pin_flist_t	*srch_o_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*plan_flistp = NULL;
	pin_flist_t	*out_flistp = NULL;
	pin_flist_t	*pdt_disc_search_flistp = NULL;
	pin_flist_t	*taxchg_flistp = NULL;
	pin_flist_t	*taxes_flistp = NULL;
	pin_flist_t	*alc_addon_pdts_main_flistp = NULL;

	pin_flist_t	*tax_code_flistp = NULL;
	poid_t		*plan_pdp = NULL;
	poid_t		*acnt_pdp = NULL;
	time_t		bp_valid_flag = 0;
    time_t      *purchase_endtp = NULL;
	time_t		pvt = 0;
	double		scale = 1;
	int32		pkg_type = -1;
	int32		conn_type = -1;
	int32		failure = FAILURE;
	int32		success = 0;
    int32       *ret_status1 = NULL;
	int32		ret_status = -1;
	int32	 	pp_type = -1;
	int32		discount_flag = -1;
	int32		validity_align = 0;
	int32		*date_alignp = NULL;
	int32		days = 0;
	char		*das_type = NULL;
	char		*cust_city = NULL;
	char		*tax_code = NULL;
	poid_t		*srvc_pdp = NULL;
	poid_t		*prd_pdp = NULL;
	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*discount_flistp = NULL;
	pin_flist_t	*et_flistp = NULL;
	pin_decimal_t	*st = NULL;
	pin_decimal_t	*base_price = NULL;
	pin_decimal_t	*disc_amt = NULL;
	pin_decimal_t	*neg_amt = NULL;	
	pin_decimal_t	*discount_amt = NULL;
	pin_decimal_t	*disc_base = NULL;
	pin_decimal_t	*final_disc = NULL;
	pin_decimal_t	*mrp = NULL;
	pin_decimal_t	*disc_per = NULL;
	pin_decimal_t	*tmp_disc = NULL;
	pin_decimal_t	*deal_disc = NULL;
	pin_decimal_t	*prd_cnt = NULL;
	pin_decimal_t	*et_amt = NULL;
	pin_decimal_t	*amt_orig = NULL;
	pin_decimal_t	*scalep = NULL;
	int32		prod_count = 0;
	char		msg[10] ="";
	int             tax_elemid = 0;
        pin_cookie_t    tax_cookie = NULL;
        char            *taxes_taxcode = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);	

	PIN_ERR_LOG_FLIST(3, "Calc catv price input flist", i_flistp);

	neg_amt = pbo_decimal_from_str("-1.0", ebufp);
	
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);	
	plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
       if (PIN_POID_IS_NULL(plan_pdp) || PIN_POID_IS_NULL(srvc_pdp) || PIN_POID_IS_NULL(acnt_pdp)) {
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "54001", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Mandatory parameters missing", ebufp);
                goto CLEANUP;
        }
        fm_mso_get_srv_dtls(ctxp, i_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "54002", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Service close/ invalid device details", ebufp);
                goto CLEANUP;
        }

	// Getting the plan price.
	fm_mso_get_plan_price(ctxp, plan_pdp, NULL, &plan_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "54003", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Failed in plan price calculation", ebufp);
        	goto CLEANUP;
	}

	fm_mso_get_plan_tax_code (ctxp, plan_pdp, tax_code, &tax_code_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERRBUF_RESET(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "54003", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Failed in Getting Tax Code", ebufp);
        goto CLEANUP;
	}
	if(tax_code_flistp)
    {
		tax_code = PIN_FLIST_FLD_GET(tax_code_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
    }
	
/*	if (tax_code == NULL) {
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "54003", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Invalid Tax Code", ebufp);
        	goto CLEANUP;
	}*/

	date_alignp = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_VALIDITY_FLAGS, 1, ebufp);
	if (date_alignp)
	{
		validity_align = *date_alignp;
	}

    purchase_endtp = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
	if (validity_align || purchase_endtp)
	{
        pvt = fm_utils_time_round_to_midnight(pin_virtual_time((time_t *)NULL));
        if (purchase_endtp && *purchase_endtp > pvt)
        {
            days = (*purchase_endtp - pvt)/86400;
            scale = (double) days / 30;
            PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_SCALE, pbo_decimal_from_double(scale, ebufp), ebufp);
        }
        else
        {
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Date alignment is set. Get base pack status");
                fm_mso_get_base_pdts(ctxp, acnt_pdp, srvc_pdp, &alc_addon_pdts_main_flistp, ebufp);

                if (alc_addon_pdts_main_flistp && alc_addon_pdts_main_flistp != NULL)
                {       
                        ret_status = *(int32*)PIN_FLIST_FLD_GET(alc_addon_pdts_main_flistp, PIN_FLD_STATUS, 1, ebufp);
                        if (ret_status == failure)
                        {       
                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Package is Inactive.");
                        }
                }

                bp_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 1, ebufp);

                if (bp_valid_flag == 0)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Package is Inactive.");
                }
                else if (bp_valid_flag == 1)
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Package is multi payterm.");
                }
		        else
                {
                        pvt = fm_utils_time_round_to_midnight(pin_virtual_time((time_t *)NULL));
                        if (bp_valid_flag > pvt)
                        {
                                days = (bp_valid_flag - pvt)/86400;
                                        scale = (double) days / 30;
                                        PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_SCALE,
                                                pbo_decimal_from_double(scale, ebufp), ebufp);
                                        PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_PURCHASE_END_T, &bp_valid_flag, ebufp);

                        }
                }
            }
            if(days > 0)
            {
                PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_DAYS, &days, ebufp);
            }
	}
	PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	base_price = PIN_FLIST_FLD_TAKE(plan_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	scalep = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_SCALE, 1, ebufp);
	if (scalep)
	{
		pbo_decimal_multiply_assign(base_price, scalep, ebufp);
		pbo_decimal_round_assign(base_price, 2, ROUND_HALF_UP, ebufp);
	}
	PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_AMOUNT, base_price, ebufp);
	base_price = PIN_FLIST_FLD_GET(plan_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	PIN_ERR_LOG_FLIST(3, "PLan price flist", plan_flistp);
	if(pbo_decimal_is_zero(base_price, ebufp)== 1)
	{
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &success, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_DISCOUNT, base_price, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT_GROSS, base_price, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_TAX, base_price, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_TOTAL_DUE, base_price, ebufp);
		//Added PIN_FLD_AMOUNT  to maintain sanity of return flist
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT, base_price, ebufp);
		PIN_ERR_LOG_FLIST(3, "Zero pack charge : ", *r_flistpp);
		goto CLEANUP;
	}
	//Block for setting MRP
	calculate_gst(ctxp, acnt_pdp, tax_code, base_price, &mrp, &taxchg_flistp, ebufp);
	PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_AMOUNT_ORIG, pbo_decimal_round(mrp, 2, ROUND_HALF_UP, ebufp), ebufp);
	PIN_ERR_LOG_FLIST(3, "Plan flist after setting MRP", plan_flistp);
	//Getting pack type
	fm_mso_get_pkg_type(ctxp, plan_flistp, &out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERRBUF_RESET(ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "54004", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Failed in plan type fetch", ebufp);
		goto CLEANUP;
         }
	PIN_ERR_LOG_FLIST(3, "fm_mso_get_pkg_type: return flist", out_flistp);
	pkg_type = *(int32 *) PIN_FLIST_FLD_GET(out_flistp, MSO_FLD_PKG_TYPE, 0, ebufp);
	prd_pdp = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
	sprintf(msg,"%d", prod_count);
	prd_cnt = pbo_decimal_from_str(msg, ebufp);

    // Et tax calculation block
    if(pkg_type == 0)
    {
        PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_PRODUCT_OBJ, prd_pdp, ebufp);
        PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_GROSS, base_price, ebufp);
        PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TOTAL_DUE, base_price, ebufp);
        fm_mso_get_et_amt(ctxp, plan_flistp, &et_flistp, ebufp);
        if(PIN_ERRBUF_IS_ERR(ebufp)) {
                    PIN_ERRBUF_RESET(ebufp);
                    *r_flistpp = PIN_FLIST_CREATE(ebufp);
                    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
                    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
                    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "54005", ebufp);
                    PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Failed in ET calculation", ebufp);
            goto CLEANUP;
        }
        et_amt = PIN_FLIST_FLD_GET(plan_flistp, MSO_FLD_ET_AMOUNT, 0, ebufp);
        pbo_decimal_add_assign(base_price, et_amt, ebufp);
    }

	//Block for setting MRP
	calculate_gst(ctxp, acnt_pdp, tax_code, base_price, &mrp, &taxchg_flistp, ebufp);
	PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_AMOUNT_ORIG, pbo_decimal_round(mrp, 2, ROUND_HALF_UP, ebufp), ebufp);
	PIN_ERR_LOG_FLIST(3, "Plan flist after setting MRP", plan_flistp);
	//check for service level discount
	/*pdt_disc_search_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, srvc_pdp, ebufp);
	PIN_FLIST_FLD_SET(pdt_disc_search_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
	PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
	fm_mso_get_service_level_discount(ctxp, pdt_disc_search_flistp, &discount_flistp, ebufp);*/
	if (out_flistp)
	{
		ret_status1 = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_STATUS, 1, ebufp);
        if (ret_status1)
        {
            ret_status = *(int32 *) ret_status1;
        }
		if (ret_status == failure)
		{
			if (PIN_ERRBUF_IS_ERR(ebufp))
			PIN_ERRBUF_RESET(ebufp);
	
			pdt_disc_search_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, plan_pdp, ebufp);
			// Retrieving the PP Type
			pp_type = fm_mso_get_cust_type(ctxp, acnt_pdp, ebufp);
			if (pp_type != 2)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"Valid PP Type Found... ");
				PIN_FLIST_FLD_SET(pdt_disc_search_flistp,
				MSO_FLD_PP_TYPE, &pp_type, ebufp);
			}
	
			//Retrieve the Customer City from Account Object
			fm_mso_get_custcity(ctxp, acnt_pdp, &cust_city, ebufp);
			if (cust_city)
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
					PIN_FLD_DISCOUNT_FLAGS, 1, ebufp);
					if (discount_flag == 0 || discount_flag == 1 )
					{
						disc_amt = PIN_FLIST_FLD_GET(discount_flistp,
						PIN_FLD_DISCOUNT, 1, ebufp);
					}
	
				}
			}
		}
	}
	if(discount_flag != -1)
	{
		PIN_ERR_LOG_MSG(3, "Discount found");
		if(discount_flag ==0)
		{
			PIN_ERR_LOG_MSG(3, "% discount configuration");
			disc_per = PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
			tmp_disc = pbo_decimal_round(pbo_decimal_multiply(base_price, disc_per, ebufp), 2, ROUND_HALF_UP, ebufp);
			discount_amt = pbo_decimal_multiply(tmp_disc, neg_amt, ebufp);

		}
		else
		{
			prod_count = *(int32 *) PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_COUNT, 0, ebufp);
			sprintf(msg, "%d", prod_count);
			prd_cnt = pbo_decimal_from_str(msg, ebufp);

			PIN_ERR_LOG_MSG(3, "Flat discount configuration");
			disc_amt = PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
			tmp_disc = pbo_decimal_multiply(disc_amt, prd_cnt, ebufp);
			discount_amt = pbo_decimal_multiply(tmp_disc, neg_amt, ebufp);
		}
		disc_base = pbo_decimal_add(base_price, discount_amt, ebufp);
		//calculate_gst(ctxp, acnt_pdp, tax_code, discount_amt, &final_disc, &taxchg_flistp, ebufp);
		PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_DISCOUNT, pbo_decimal_round(discount_amt, 2, ROUND_HALF_UP, ebufp), ebufp);		
		PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_GROSS, disc_base, ebufp);	
		PIN_ERR_LOG_FLIST(3, "discount flist", plan_flistp);
		calculate_gst(ctxp, acnt_pdp, tax_code, disc_base, &st, &taxchg_flistp, ebufp);		
		PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TAX, pbo_decimal_round(pbo_decimal_subtract(st, disc_base, ebufp), 2, ROUND_HALF_UP, ebufp), ebufp);
		PIN_ERR_LOG_FLIST(3, " ST calculated flist", plan_flistp);
		PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_TOTAL_DUE, pbo_decimal_round(st, 2, ROUND_HALF_UP, ebufp), ebufp);
		pbo_decimal_destroy(&tmp_disc);
		pbo_decimal_destroy(&discount_amt);
		pbo_decimal_destroy(&disc_base);
	}		
	else
	{	
		deal_disc = PIN_FLIST_FLD_GET(out_flistp, PIN_FLD_CYCLE_DISCOUNT, 1, ebufp);
		if (deal_disc && pbo_decimal_is_zero(deal_disc, ebufp))	
		{
			PIN_ERR_LOG_MSG(3, "No discount configurations available for this plan");
			calculate_gst(ctxp, acnt_pdp, tax_code, base_price, &st, &taxchg_flistp, ebufp);
			PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_GROSS, base_price, ebufp);
			PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_DISCOUNT, pbo_decimal_from_str("0.0", ebufp), ebufp);
			PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_TAX, pbo_decimal_round(pbo_decimal_subtract(st, base_price, ebufp), 2, ROUND_HALF_UP, ebufp), ebufp);
			PIN_ERR_LOG_FLIST(3, " ST calculated flist", plan_flistp);
                        PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_TOTAL_DUE, pbo_decimal_round(st, 2, ROUND_HALF_UP, ebufp), ebufp);

		}
		else
		{
			PIN_ERR_LOG_MSG(3, "Service/deal discount available for this plan");			
			disc_base = pbo_decimal_add(base_price, deal_disc, ebufp);
			//calculate_gst(ctxp, acnt_pdp, tax_code, discount_amt, &final_disc, &taxchg_flistp, ebufp);
			PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_DISCOUNT, pbo_decimal_round(deal_disc, 2, ROUND_HALF_UP, ebufp), ebufp);
                        PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_GROSS, disc_base, ebufp);
                        PIN_ERR_LOG_FLIST(3, "before calling st calculation", plan_flistp);
                        calculate_gst(ctxp, acnt_pdp, tax_code, disc_base, &st, &taxchg_flistp, ebufp);
                        PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_TAX, pbo_decimal_round(pbo_decimal_subtract(st, disc_base, ebufp), 2, ROUND_HALF_UP, ebufp), ebufp);
                        PIN_ERR_LOG_FLIST(3, " ST calculated flist", plan_flistp);
                        PIN_FLIST_FLD_PUT(plan_flistp, PIN_FLD_TOTAL_DUE, pbo_decimal_round(st, 2, ROUND_HALF_UP, ebufp), ebufp);
			pbo_decimal_destroy(&disc_base);
		}
	}
	
	if(taxchg_flistp !=NULL)
	{
		while((taxes_flistp = PIN_FLIST_ELEM_GET_NEXT(taxchg_flistp, PIN_FLD_TAXES, &tax_elemid, 1, &tax_cookie, ebufp )) != NULL)
		{	
		 	taxes_taxcode = PIN_FLIST_FLD_GET(taxes_flistp, PIN_FLD_TAX_CODE, 0, ebufp);
			if (taxes_taxcode && (strcmp(taxes_taxcode, IGST)==0 || strcmp(taxes_taxcode, CGST)==0 || strcmp(taxes_taxcode, SGST)==0 ||strcmp(taxes_taxcode, ADD_CESS)==0))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "GST Tax Code Matched");
				if(taxes_taxcode && strcmp(taxes_taxcode, IGST)==0)
				{
					PIN_FLIST_FLD_SET(plan_flistp, MSO_FLD_IGST_RATE, 
					pbo_decimal_round(PIN_FLIST_FLD_GET(taxes_flistp,PIN_FLD_TAX, 0, ebufp), 2, ROUND_HALF_UP, ebufp),  ebufp);     //IGST
				}
				else if(taxes_taxcode && strcmp(taxes_taxcode, SGST)==0)
				{
					 PIN_FLIST_FLD_SET(plan_flistp, MSO_FLD_SGST_RATE, 
					 pbo_decimal_round(PIN_FLIST_FLD_GET(taxes_flistp,PIN_FLD_TAX, 0, ebufp), 2, ROUND_HALF_UP, ebufp), ebufp);     //SGST
				}
				else if(taxes_taxcode && strcmp(taxes_taxcode, CGST)==0)
				{
					PIN_FLIST_FLD_SET(plan_flistp, MSO_FLD_CGST_RATE, 
					pbo_decimal_round(PIN_FLIST_FLD_GET(taxes_flistp,PIN_FLD_TAX, 0, ebufp), 2, ROUND_HALF_UP, ebufp), ebufp);
				}
                else if(taxes_taxcode && strcmp(taxes_taxcode, ADD_CESS)==0)
                {
                    PIN_FLIST_FLD_PUT(plan_flistp, MSO_FLD_MISC_AMT,
                          pbo_decimal_round(PIN_FLIST_FLD_GET(taxes_flistp,PIN_FLD_TAX, 0, ebufp), 2, ROUND_HALF_UP, ebufp), ebufp);
                }

			}
		}
	}	

    if (et_amt && pbo_decimal_is_zero(et_amt, ebufp) == 0)
    {
        if (disc_base && pbo_decimal_is_zero(disc_base, ebufp) == 0)
        {
            pbo_decimal_subtract_assign(disc_base, et_amt, ebufp);
            PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_GROSS, disc_base, ebufp);
        }
        else 
        {
            pbo_decimal_subtract_assign(base_price, et_amt, ebufp);
            PIN_FLIST_FLD_SET(plan_flistp, PIN_FLD_AMOUNT_GROSS, base_price, ebufp);
        }    

    }
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_NAME, plan_flistp, PIN_FLD_NAME, ebufp);
	*r_flistpp = PIN_FLIST_COPY(plan_flistp, ebufp);

	CLEANUP:
	pbo_decimal_destroy(&neg_amt);
	//pbo_decimal_destroy(&base_price);
	PIN_FLIST_DESTROY_EX(&plan_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);
	
	return;


}

void fm_mso_get_et_amt(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t	*ebufp)
{
	poid_t	*srvc_pdp = NULL;
	poid_t	*plan_pdp = NULL;
	poid_t	*prd_pdp = NULL;
	char	*tax_code = NULL;
	char	*et_tax_code = NULL;
	char	scale_str[10] = "";
	char	msg[500] = "";
	int32	scale = 0;
	pin_decimal_t	*prod_scale = NULL;
	pin_decimal_t	*amt_gross = NULL;
	pin_decimal_t	*et_amt = NULL;
	pin_decimal_t	*total_due = NULL;
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*et_in_flistp = NULL;
	pin_flist_t	*et_code_flistp = NULL;
    time_t  charged_to_t = 0;	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return;
	}
	PIN_ERR_LOG_FLIST(3, "et calc input flist", i_flistp);	
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	prd_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
	//getting tax_code
	tax_code = mso_get_taxcode(ctxp, srvc_pdp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		goto CLEANUP;
	}
	et_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, et_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_TAX_CODE, tax_code, ebufp);
	PIN_ERR_LOG_FLIST(3 ,"ET tax code input flist", et_in_flistp);
	
	mso_retrieve_et_code(ctxp, et_in_flistp, &et_code_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                goto CLEANUP;
        }
	PIN_ERR_LOG_FLIST(3 ,"ET tax code output flist", et_code_flistp);
	et_tax_code = PIN_FLIST_FLD_GET(et_code_flistp, PIN_FLD_TAX_CODE, 1, ebufp);

	amt_gross = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT_GROSS, 0, ebufp);

    fm_mso_retrieve_et(ctxp, srvc_pdp, &r_flistp, ebufp);
    if (r_flistp)
    {
        charged_to_t = *(time_t *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);
    }
	scale = fm_mso_get_prd_scale(ctxp, prd_pdp, charged_to_t, ebufp);
	PIN_ERR_LOG_MSG(3, "Reaching here");
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                goto CLEANUP;
        }

			
	sprintf(scale_str,"%d", scale);
			
	prod_scale = pbo_decimal_from_str(scale_str, ebufp);
	
	PIN_ERR_LOG_MSG(3, "Reaching point 2");
	sprintf(msg, "scale = %d , taxcode = %s", scale, tax_code);
	PIN_ERR_LOG_MSG(3, msg);
	//Et calc 
	mso_et_apply_tax(ctxp, et_tax_code, amt_gross, prod_scale, &et_in_flistp, &et_code_flistp, ebufp);
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		goto CLEANUP;
	}
	et_amt = PIN_FLIST_FLD_GET(et_code_flistp, PIN_FLD_TAX, 0, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_ET_AMOUNT, pbo_decimal_round(et_amt, 2, ROUND_HALF_UP, ebufp), ebufp);
	total_due = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TOTAL_DUE, 0, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_TOTAL_DUE, pbo_decimal_round(pbo_decimal_add(total_due, et_amt, ebufp), 2, ROUND_HALF_UP, ebufp), ebufp);
	
	PIN_ERR_LOG_FLIST(3, "Et tax final flist", i_flistp);
	PIN_ERR_LOG_FLIST(3, "Et tax return flist", et_code_flistp);
		
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&et_in_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&et_code_flistp, ebufp);
		if(!pbo_decimal_is_null(prod_scale, ebufp))
		{
			pbo_decimal_destroy(&prod_scale);
		}
		return;
	
}

int32 fm_mso_get_prd_scale(
	pcm_context_t	*ctxp,
	poid_t	*prd_pdp,
    time_t          et_charged_to_t,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*srch_iflistp = NULL;
	pin_flist_t	*srch_oflistp = NULL;
	pin_flist_t	*usage_flistp = NULL;
	pin_flist_t	*scale_iflistp = NULL;
	pin_flist_t	*scale_oflistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*events_flistp = NULL;
	pin_flist_t	*temp_flistp = NULL;
	poid_t		*srch_pdp = NULL;
	int32		db = 1;
	int32		cnt = -1;
	double		scale = 0.0; 
	time_t		charge_to = 0;
	time_t		current_t = fm_utils_time_round_to_midnight(pin_virtual_time(NULL));
    char        scale_str[10] = "";
    char        *event_typep = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
    {
		PIN_ERRBUF_CLEAR(ebufp);
    }
	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_POID, prd_pdp, ebufp);
	usage_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_USAGE_MAP, 0, ebufp);
	PIN_FLIST_FLD_SET(usage_flistp, PIN_FLD_EVENT_TYPE, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(3, "Product read flds flist", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, srch_iflistp, &srch_oflistp,  ebufp);
    PIN_FLIST_DESTROY_EX(&srch_iflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {	
		PIN_ERR_LOG_MSG(1, "Error during product readflds");
		goto CLEANUP;
	}
        /*****************************************************************
         * NTO: Check if event type is 30 days event then scale is 1
         *****************************************************************/
        usage_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 1,  ebufp);
        if (usage_flistp)
        {
                event_typep = PIN_FLIST_FLD_GET(usage_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);
        }
        
        if (et_charged_to_t == 0)
        {
            if (event_typep && strstr(event_typep, "cycle_forward_thirty_days"))
            {
                cnt = 1;
            }
            else if (event_typep && strstr(event_typep, "cycle_forward_sixty_days"))
            {
                cnt = 2;
            }
            else if (event_typep && strstr(event_typep, "cycle_forward_ninety_days"))
            {
                cnt = 3;
            }
            else if (event_typep && strstr(event_typep, "cycle_forward_onetwenty_days"))
            {
                cnt = 4;
            }
            else if (event_typep && strstr(event_typep, "cycle_forward_oneeighty_days"))
            {
                cnt = 6;
            }
		    // Jyothirmayi Kachiraju === New Event Creation
		    else if (event_typep && strstr(event_typep, "cycle_forward_onefifty_days"))
            {
                cnt = 5;
            }
		    else if (event_typep && strstr(event_typep, "cycle_forward_twoten_days"))
            {
                cnt = 7;
            }
		    else if (event_typep && strstr(event_typep, "cycle_forward_twoforty_days"))
            {
                cnt = 8;
            }
		    else if (event_typep && strstr(event_typep, "cycle_forward_twoseventy_days"))
            {
                cnt = 9;
            }
		    else if (event_typep && strstr(event_typep, "cycle_forward_threehundred_days"))
            {
                cnt = 10;
            }
		    else if (event_typep && strstr(event_typep, "cycle_forward_threethirty_days"))
            {
                cnt = 11;
            }
		    else if (event_typep && strstr(event_typep, "cycle_forward_threesixty_days"))
            {
                cnt = 12;
            }
            else
            {}
        }
        else
        {
		    scale_iflistp = PIN_FLIST_COPY(srch_oflistp, ebufp);
            charge_to = fm_utils_time_round_to_midnight(pin_virtual_time(NULL));
		    PIN_FLIST_FLD_SET(scale_iflistp, PIN_FLD_CHARGED_TO_T, &charge_to, ebufp);
	
		    PIN_ERR_LOG_FLIST(3, "input flist for event map", scale_iflistp);
		    fm_mso_search_event_map(ctxp, scale_iflistp, &scale_oflistp, ebufp);	
            PIN_FLIST_DESTROY_EX(&scale_iflistp, ebufp);

		    if (PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_MSG(1, "Error during product readflds");
                goto CLEANUP;
            }

		    PIN_ERR_LOG_FLIST(3, "Read scale outflistp", scale_oflistp);
            charge_to = *(time_t *)PIN_FLIST_FLD_GET(scale_oflistp, PIN_FLD_CHARGED_TO_T, 0, ebufp);

	    	PIN_FLIST_DESTROY_EX(&scale_oflistp, NULL);
            if (et_charged_to_t > charge_to)
            {
                PIN_ERR_LOG_MSG(3, "ET Already charged and hence scale is 0");
                cnt = 0;
            }
            else
            {
                if (et_charged_to_t < current_t)
                {
                    et_charged_to_t = current_t;
                }
                PIN_ERR_LOG_MSG(3, "ET not charged and hence scale calculating");
                charge_to = charge_to - 86400;
                sprintf(scale_str,"%d", charge_to);
                PIN_ERR_LOG_MSG(3, scale_str);

                sprintf(scale_str,"%d", et_charged_to_t);
                PIN_ERR_LOG_MSG(3, scale_str);

                scale = (charge_to - et_charged_to_t);
                sprintf(scale_str,"%f", scale);
                PIN_ERR_LOG_MSG(3, scale_str);

                scale = scale/86400;
                sprintf(scale_str,"%f", scale);
                PIN_ERR_LOG_MSG(3, scale_str);

                scale = scale/30;
                sprintf(scale_str,"%f", scale);
                PIN_ERR_LOG_MSG(3, scale_str);

                cnt = round(scale);
                sprintf(scale_str,"%d", cnt);
                PIN_ERR_LOG_MSG(3, scale_str);
            }
    }		
	 	
CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		return cnt;
}

void fm_mso_get_plan_price (
        pcm_context_t   *ctxp,
	poid_t		*plan_pdp,
	poid_t		*deal_pdp,
        pin_flist_t     **r_flistpp,
        pin_errbuf_t    *ebufp)
{
        pin_flist_t     *srch_i_flistp = NULL;
        pin_flist_t     *srch_o_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
	pin_flist_t	*prod_flistp = NULL;
	pin_flist_t	*quan_flistp = NULL;
	pin_flist_t	*bal_flistp = NULL;
	poid_t		*srch_pdp = NULL;
	int32		db_no = 1;
	int32		res_id = 0;
	int32		elemid = 0;
	int32		flags = 256;
	int32		dpo_flags = 0;
	pin_cookie_t    cookie = NULL;
	void 		*vp = NULL;
	char		srch_template[2000];
	char		msg[250];
	char		*descrp = NULL;
	pin_decimal_t	*fix_amt = NULL;
	pin_decimal_t	*scale_amt  = NULL;
	pin_decimal_t	*base_price = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/******************************************************
	 * Check this is a DPO pack or not
	 *****************************************************/
	srch_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_POID, plan_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_DESCR, NULL, ebufp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, srch_i_flistp, &srch_o_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_plan_price descr",
	srch_o_flistp);

	descrp = PIN_FLIST_FLD_GET(srch_o_flistp, PIN_FLD_DESCR, 0, ebufp);
	if (strstr(descrp, "DPO-"))
	{
		dpo_flags = 1;
	}

	PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);

	if (dpo_flags)
	{
		srch_i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_POID, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &dpo_flags, ebufp);

		PCM_OP(ctxp, MSO_OP_CUST_GET_PKG_CHANNELS, 0, srch_i_flistp, &srch_o_flistp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_plan_price pkg_channels",
			srch_o_flistp);

		if (srch_o_flistp)
		{	
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, plan_pdp, ebufp);
			base_price = PIN_FLIST_FLD_TAKE(srch_o_flistp, MSO_FLD_NCF_CHARGE, 0, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_NCF_CHARGE, base_price, ebufp);

			fix_amt = PIN_FLIST_FLD_TAKE(srch_o_flistp, MSO_FLD_SUBSCRIPTION_CHARGE, 0, ebufp);
			pbo_decimal_add_assign(base_price, fix_amt, ebufp);
			PIN_FLIST_FLD_PUT(*r_flistpp, MSO_FLD_SUBSCRIPTION_CHARGE, fix_amt, ebufp);
			PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_AMOUNT, base_price, ebufp);

			PIN_ERR_LOG_FLIST(3, "Base price flist ", *r_flistpp);
			goto CLEANUP;
		}
	}	

	strcpy(srch_template,
		"select X from /rate 1, /rate_plan 2, /deal 3, /plan 4  where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = 4.F6 and 4.F7 = V7");
	
	base_price = pbo_decimal_from_str("0.0", ebufp);
	srch_pdp = PIN_POID_CREATE(db_no, "/search", -1, ebufp);
	srch_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &flags, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RATE_PLAN_OBJ, plan_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, plan_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, plan_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 4, ebufp);
	prod_flistp  = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_PRODUCT_OBJ, plan_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, plan_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 6, ebufp);
	prod_flistp = PIN_FLIST_ELEM_ADD(args_flistp,  PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_DEAL_OBJ, plan_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 7, ebufp);	
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, plan_pdp, ebufp);

	if (!PIN_POID_IS_NULL(deal_pdp))
	{
		args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 8, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, deal_pdp, ebufp);
		strcat(srch_template, "and 3.F8 = V8 ");
	}
	
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, srch_template, ebufp);	
	res_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

	PIN_ERR_LOG_FLIST(3, "fm_mso_get_plan_price :search input flist", srch_i_flistp);
	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp , &srch_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_MSG(1, "Error during plan price search");
		goto CLEANUP;
	}
	
	PIN_ERR_LOG_FLIST(3, "fm_mso_get_plan_price :search output flist", srch_o_flistp);
	results_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	
	if(results_flistp)
	{
		
		while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_o_flistp, PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp )) != NULL)	
		{
			PIN_ERR_LOG_FLIST(3, "Results flistp ", res_flistp);
			res_id = 0;
			quan_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_QUANTITY_TIERS, 0, 0, ebufp);
			bal_flistp = PIN_FLIST_ELEM_GET(quan_flistp, PIN_FLD_BAL_IMPACTS, 0, 0, ebufp);
			res_id = *(int32 *) PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_ELEMENT_ID, 0, ebufp);
			if (res_id == 356)
			{
				scale_amt = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_SCALED_AMOUNT, 0, ebufp);
				pbo_decimal_add_assign(base_price, scale_amt, ebufp);
				fix_amt = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_FIXED_AMOUNT, 0, ebufp);
				pbo_decimal_add_assign(base_price, fix_amt, ebufp);
				sprintf(msg, "Added amount : %s", pbo_decimal_to_str(base_price, ebufp));
				PIN_ERR_LOG_MSG(3, msg);
			}
		}			
	
	}
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, plan_pdp, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_AMOUNT, base_price, ebufp);
	PIN_ERR_LOG_FLIST(3, "Base price flist ", *r_flistpp);
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
	return;
}

void fm_mso_get_plan_tax_code (
        pcm_context_t   *ctxp,
	poid_t		*plan_pdp,
        char		*tax_code,
	pin_flist_t	**r_flistpp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t     *s_flistp = NULL;
	pin_flist_t     *s_oflistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *serv_flistp = NULL;
	pin_flist_t     *prod_arg_flistp = NULL;

	poid_t		*s_pdp = NULL;
	int32		db = 1;
	int32		flags = 256;
	void 		*vp = NULL;
//	char		*s_template = "select X from /rate_plan 1, /deal 2, /plan 3 where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = V5 and 1.F6 is not null ";
    char        *s_template = "select X from /rate_plan 1, /deal 2, /plan 3 where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = V5 ";	
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	prod_arg_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(prod_arg_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	serv_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(serv_flistp, PIN_FLD_DEAL_OBJ, NULL, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, plan_pdp, ebufp);

/*	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_TAX_CODE, "", ebufp);*/

	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_TAX_CODE, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Tax Code Input flist:", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"fm_mso_search_tax_code PCM_OP_SEARCH error.");
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Tax Code Output flist:", s_oflistp);
	
	results_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	
	if(results_flistp)
	{
		*r_flistpp = PIN_FLIST_COPY(results_flistp, ebufp);
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
	return;
}
	
void  fm_mso_get_pkg_type(
	pcm_context_t 	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	**ret_flistp,
	pin_errbuf_t 	*ebufp)
{
	pin_flist_t	*srch_i_flistp = NULL;
	pin_flist_t	*srch_o_flistp = NULL;
	pin_flist_t	*args_flistp = NULL;
    pin_flist_t     *res_flistp = NULL;
    pin_flist_t     *results_flistp = NULL;
	pin_flist_t	*prod_flistp = NULL;
	pin_flist_t	*plan_flistp = NULL;
	pin_flist_t	*pdt_disc_search_flistp = NULL;
	pin_flist_t	*discount_flistp = NULL;
	pin_flist_t	*parent_serv_flistp = NULL;
    pin_flist_t *alc_addon_pdts_main_flistp = NULL;
	poid_t		*srch_pdp  = NULL;
	poid_t		*parent_acc_pdp = NULL;
	poid_t		*prd_pdp = NULL;
	poid_t		*deal_pdp = NULL;
	poid_t		*plan_pdp = NULL;
	poid_t		*service_pdp = NULL;
	poid_t		*new_serv_pdp = NULL;
	pin_decimal_t	*cycle_discount = NULL;
	pin_decimal_t	*amtp = NULL;
	pin_decimal_t	*total_discp = NULL;
	pin_decimal_t	*tmp_discp = NULL;
	pin_decimal_t	*disc_per = NULL;
	pin_decimal_t	*disc_amt = NULL;
	pin_decimal_t	*scalep = NULL;
	pin_decimal_t	*onep = NULL;
	pin_decimal_t	*ncf_amtp = NULL;
	pin_decimal_t	*sub_amtp = NULL;
	pin_decimal_t	*total_ncf_discp = NULL;
	pin_decimal_t	*total_sub_discp = NULL;
	pin_cookie_t    o_cookie = NULL;
	pin_cookie_t    o_cookie1 = NULL;
	pin_cookie_t	cookie = NULL;
	char 		*template = "select X from /deal 1, /plan 2 where 1.F1 = 2.F2 and 2.F3 = V3";
    time_t      bp_parent_valid_flag = 0;
	int32		db_no = 1;
	int32           elemid = 0;
	int32		o_elemid = 0;
	int32		o_elemid1 = 0;
	int32		flags  = 256;
	int32		pkg_type = -1;
	int32		tmp_pkg_type = -1;
	int32		prod_count = 0;
	int32		ret_status = -1;
	int32		discount_flag = -1;
	int32		service_disc_status = 1;
	int32		failure = 1;
    int32       skip_discount = 0;

	plan_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	ncf_amtp = PIN_FLIST_FLD_TAKE(in_flistp, MSO_FLD_NCF_CHARGE, 1, ebufp);
	sub_amtp = PIN_FLIST_FLD_TAKE(in_flistp, MSO_FLD_SUBSCRIPTION_CHARGE, 1, ebufp);
	scalep = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SCALE, 1, ebufp);
	onep = pbo_decimal_from_str("1.0", ebufp);
	total_discp = pbo_decimal_from_str("0.0", ebufp);
	total_ncf_discp = pbo_decimal_from_str("0.0", ebufp);
	total_sub_discp = pbo_decimal_from_str("0.0", ebufp);

	if (ncf_amtp)
	{
        fm_mso_hier_group_get_parent(ctxp, service_pdp, &parent_serv_flistp, ebufp);
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
		    PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, service_pdp, ebufp);
		    tmp_pkg_type = NCF_PKG_TYPE;
		    pkg_type = 0;
		    PIN_FLIST_FLD_SET(pdt_disc_search_flistp, MSO_FLD_PKG_TYPE, &tmp_pkg_type, ebufp);
		    PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Looking for Service Level Discount Config");
			
		    fm_mso_get_service_level_discount(ctxp, pdt_disc_search_flistp, 1, &discount_flistp, ebufp);
		    PIN_FLIST_DESTROY_EX(&pdt_disc_search_flistp, NULL);

        }	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_pkg_type: discount",
                                        discount_flistp);
		if (discount_flistp == NULL)
		{
                	PIN_ERR_LOG_MSG(3, "There is no service level discount flist for this DPO plan");
			goto CLEANUP;
		}
		discount_flag = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp,
                                                PIN_FLD_STATUS, 0, ebufp);
		if (discount_flag != 1)
		{
			pdt_disc_search_flistp = PIN_FLIST_CREATE(ebufp);
			plan_flistp = PIN_FLIST_ELEM_ADD(pdt_disc_search_flistp, PIN_FLD_RESULTS, 0, ebufp);	
			PIN_FLIST_FLD_SET(plan_flistp, MSO_FLD_PKG_TYPE, NULL, ebufp);
			PIN_FLIST_SORT(discount_flistp, pdt_disc_search_flistp, 1, ebufp);		
			PIN_FLIST_DESTROY_EX(&pdt_disc_search_flistp, NULL);
			service_disc_status = 0;
		}	
		else
		{
			PIN_FLIST_DESTROY_EX(&discount_flistp, NULL);
                	PIN_ERR_LOG_MSG(3, "There is no service level discount for this DPO plan");
			//goto CLEANUP;
		}
		while (discount_flistp && (results_flistp = PIN_FLIST_ELEM_GET_NEXT(discount_flistp, PIN_FLD_RESULTS,
			&o_elemid, 1, &o_cookie, ebufp )) != NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_pkg_type: discount element",
                                        results_flistp);
			pkg_type = *(int32 *)PIN_FLIST_FLD_GET(results_flistp, MSO_FLD_PKG_TYPE, 0, ebufp);
			
			/***************************************************************
			 * Skip BC and ALC pkg in the DPO pack; 
			 **************************************************************/	
			if (pkg_type == 1 || pkg_type == 2)
			{
				continue;
			}
			discount_flag = *(int32 *)PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_DISCOUNT_FLAGS, 0, ebufp);
			if (discount_flag == 0)
			{
				/***************************************************************
				 * Handling percentage discount
				 *************************************************************/
				disc_per = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
				if (!pbo_decimal_compare(onep, disc_per, ebufp))
				{
					PIN_ERR_LOG_MSG(3, "100% service level discount configured");

					if (pkg_type == ALL_PKG_TYPE)
					{
						if (sub_amtp && pbo_decimal_is_zero(sub_amtp, ebufp) == 0)
						{
							pbo_decimal_add_assign(total_discp, ncf_amtp, ebufp);
							pbo_decimal_add_assign(total_ncf_discp, ncf_amtp, ebufp);
							pbo_decimal_subtract_assign(ncf_amtp, ncf_amtp, ebufp);
						}
						pbo_decimal_add_assign(total_discp, sub_amtp, ebufp);
						pbo_decimal_add_assign(total_sub_discp, sub_amtp, ebufp);
						pbo_decimal_subtract_assign(sub_amtp, sub_amtp, ebufp);
					}
					else if (pkg_type == NCF_PKG_TYPE)
					{
						pbo_decimal_add_assign(total_discp, ncf_amtp, ebufp);
						pbo_decimal_add_assign(total_ncf_discp, ncf_amtp, ebufp);
						pbo_decimal_subtract_assign(ncf_amtp, ncf_amtp, ebufp);
					}	
				}	
				else
				{
					if (pkg_type == NCF_PKG_TYPE)
					{
						tmp_discp = pbo_decimal_round(pbo_decimal_multiply(ncf_amtp, disc_per, ebufp),
                                                                        2, ROUND_HALF_UP, ebufp);
						pbo_decimal_add_assign(total_discp, tmp_discp, ebufp);
						pbo_decimal_add_assign(total_ncf_discp, tmp_discp, ebufp);
						pbo_decimal_subtract_assign(ncf_amtp, tmp_discp, ebufp);
						pbo_decimal_destroy(&tmp_discp);                                       
					}
					else if (pkg_type == ALL_PKG_TYPE)
					{
						if (ncf_amtp && pbo_decimal_is_zero(ncf_amtp, ebufp) == 0)
						{
							tmp_discp = pbo_decimal_round(pbo_decimal_multiply(ncf_amtp, disc_per, ebufp),
                                        			2, ROUND_HALF_UP, ebufp);
							pbo_decimal_add_assign(total_discp, tmp_discp, ebufp);
							pbo_decimal_add_assign(total_ncf_discp, tmp_discp, ebufp);
							pbo_decimal_subtract_assign(ncf_amtp, tmp_discp, ebufp);
							pbo_decimal_destroy(&tmp_discp);
						}
						tmp_discp = pbo_decimal_round(pbo_decimal_multiply(sub_amtp, disc_per, ebufp),
							2, ROUND_HALF_UP, ebufp);
						pbo_decimal_add_assign(total_discp, tmp_discp, ebufp);
						pbo_decimal_add_assign(total_sub_discp, tmp_discp, ebufp);
						pbo_decimal_subtract_assign(sub_amtp, tmp_discp, ebufp);
						pbo_decimal_destroy(&tmp_discp);
					}
				}
			
			}	
			else if (discount_flag == 1)
			{
				/***************************************************************
				 * Handling fixed discount 
				 ***************************************************************/
				disc_amt = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
				if (disc_amt && pbo_decimal_is_zero(disc_amt, ebufp) == 0)
				{
					if (pkg_type == NCF_PKG_TYPE)
					{
						pbo_decimal_subtract_assign(ncf_amtp, disc_amt, ebufp);
						pbo_decimal_add_assign(total_ncf_discp, disc_amt, ebufp);
					}
					else if (pkg_type == ALL_PKG_TYPE)
					{
						pbo_decimal_subtract_assign(sub_amtp, disc_amt, ebufp);
						pbo_decimal_add_assign(total_sub_discp, disc_amt, ebufp);
					}
					pbo_decimal_add_assign(total_discp, disc_amt, ebufp);
				}
			}
		}
		amtp = pbo_decimal_add(ncf_amtp, sub_amtp, ebufp);
        	PIN_FLIST_FLD_PUT(in_flistp, MSO_FLD_NCF_CHARGE, ncf_amtp, ebufp);
        	PIN_FLIST_FLD_PUT(in_flistp, MSO_FLD_SUBSCRIPTION_CHARGE, sub_amtp, ebufp);
		pbo_decimal_negate_assign(total_discp, ebufp);
		PIN_FLIST_DESTROY_EX(&discount_flistp, NULL);	
		//goto CLEANUP;
	}


	srch_pdp = PIN_POID_CREATE(db_no, "/search", -1, ebufp);
        srch_i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, plan_pdp, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp);
        prod_flistp = PIN_FLIST_ELEM_ADD(args_flistp,  PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_DEAL_OBJ, plan_pdp, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, plan_pdp, ebufp);
	res_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_pkg_type: search input flist", srch_i_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp , &srch_o_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(1, "Error during product  search");
                goto CLEANUP;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_pkg_type :search output flist", srch_o_flistp);


	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_o_flistp, PIN_FLD_RESULTS,
		&o_elemid, 1, &o_cookie, ebufp )) != NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Results flistp1 ", results_flistp);

		deal_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 0, ebufp);
		prod_count = PIN_FLIST_ELEM_COUNT(results_flistp, PIN_FLD_PRODUCTS, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_cust_calc_price error",
                        ebufp);
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_cust_calc_price error",
                        ebufp);
			break;

		}

		fm_mso_get_plan_price(ctxp, plan_pdp, deal_pdp, &plan_flistp, ebufp);

		
		amtp = PIN_FLIST_FLD_TAKE(plan_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		if (scalep)
		{
			pbo_decimal_multiply_assign(amtp, scalep, ebufp);
			pbo_decimal_round_assign(amtp, 2, ROUND_HALF_UP, ebufp);
		}

		elemid = 0;
		cookie = NULL;
		while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(results_flistp, PIN_FLD_PRODUCTS, &elemid, 1, &cookie, ebufp )) != NULL)
        	{
        		PIN_ERR_LOG_FLIST(3, "Results flistp ", res_flistp);
			discount_flag = -1;
			prd_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
			pkg_type = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp);

			if (pkg_type == 0)
			{
				*ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, plan_pdp, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_COUNT, &prod_count, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_PRODUCT_OBJ, prd_pdp, ebufp);
				PIN_FLIST_FLD_SET(*ret_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
			}
		
			if (pkg_type == 0)
			{
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Package is not NTO2");
				tmp_pkg_type = NCF_PKG_TYPE;
                fm_mso_hier_group_get_parent(ctxp, service_pdp, &parent_serv_flistp, ebufp);
			}
			else
			{
				tmp_pkg_type = pkg_type;
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
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Package of main conn is Inactive.");
                        skip_discount = 1;
                    }
                }

                bp_parent_valid_flag = fm_mso_base_pack_validation(ctxp, alc_addon_pdts_main_flistp, 0, ebufp);
                if (bp_parent_valid_flag == 0)
                {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Package of main conn is Inactive.");
                    skip_discount = 1;
                }
                PIN_FLIST_DESTROY_EX(&alc_addon_pdts_main_flistp, NULL);
            }

            if (!skip_discount)
            {
			    pdt_disc_search_flistp = PIN_FLIST_CREATE(ebufp);
			    PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_POID, service_pdp, ebufp);
			    PIN_FLIST_FLD_SET(pdt_disc_search_flistp, MSO_FLD_PKG_TYPE, &tmp_pkg_type, ebufp);
			    PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_PLAN_OBJ, plan_pdp, ebufp);
			    PIN_FLIST_FLD_SET(pdt_disc_search_flistp, PIN_FLD_DEAL_OBJ, deal_pdp, ebufp);
			    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Looking for Service Level Discount Config");
			
			    fm_mso_get_service_level_discount(ctxp, pdt_disc_search_flistp, 0, &discount_flistp, ebufp);
			    PIN_FLIST_DESTROY_EX(&pdt_disc_search_flistp, NULL);
	        }	
			if (discount_flistp && discount_flistp != NULL)
			{
				ret_status = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_STATUS, 1, ebufp);
				if (ret_status != failure)
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service Level Discount Config Found");
					discount_flag = *(int32 *)PIN_FLIST_FLD_GET(discount_flistp,
						PIN_FLD_DISCOUNT_FLAGS, 0, ebufp);
					service_disc_status = 0;
					if (discount_flag == 0)
					{
						disc_per = PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 0, ebufp);
					}
					else if (discount_flag == 1)
					{
						disc_amt = pbo_decimal_negate(PIN_FLIST_FLD_GET(discount_flistp, PIN_FLD_DISCOUNT, 0, ebufp),
								ebufp);
					}
					else
					{
					}
				}
			}			
			cycle_discount = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_CYCLE_DISCOUNT, 0, ebufp);

			/**********************************************************
			 * Cumulative discount amount calculation
			 **********************************************************/
			if (pbo_decimal_is_zero(cycle_discount, ebufp) == 0 && pbo_decimal_is_zero(amtp, ebufp) == 0)
			{
				tmp_discp = pbo_decimal_round(pbo_decimal_multiply(amtp, cycle_discount, ebufp), 2, ROUND_HALF_UP, ebufp);
				pbo_decimal_negate_assign(tmp_discp, ebufp);
				if (discount_flag == 0)
				{
					if (!pbo_decimal_compare(onep, disc_per, ebufp))
					{
						PIN_ERR_LOG_MSG(3, "100% service level discount configured");
						pbo_decimal_destroy(&tmp_discp);
						tmp_discp = pbo_decimal_from_str("0.0", ebufp);
					}
					pbo_decimal_add_assign(total_discp, tmp_discp, ebufp);
					pbo_decimal_add_assign(tmp_discp, amtp, ebufp);
					pbo_decimal_negate_assign(tmp_discp, ebufp);
					pbo_decimal_multiply_assign(tmp_discp, disc_per, ebufp);
					pbo_decimal_round_assign(tmp_discp, 2, ROUND_HALF_UP, ebufp);	
				}
				else if (discount_flag == 1)
				{
					pbo_decimal_add_assign(tmp_discp, disc_amt, ebufp);
					pbo_decimal_destroy(&disc_amt);
				}
				pbo_decimal_add_assign(total_discp, tmp_discp, ebufp);
				pbo_decimal_destroy(&tmp_discp);
			}
			else if (pbo_decimal_is_zero(amtp, ebufp) == 0 && discount_flag != -1)
			{
				tmp_discp = pbo_decimal_from_str("0.0", ebufp);
				if (discount_flag == 0)
				{
					pbo_decimal_add_assign(tmp_discp, amtp, ebufp);
					pbo_decimal_negate_assign(tmp_discp, ebufp);
					pbo_decimal_multiply_assign(tmp_discp, disc_per, ebufp);
					pbo_decimal_round_assign(tmp_discp, 2, ROUND_HALF_UP, ebufp);	
				}
				else if (discount_flag == 1)
				{
					pbo_decimal_add_assign(tmp_discp, disc_amt, ebufp);
					pbo_decimal_destroy(&disc_amt);
				}
				pbo_decimal_add_assign(total_discp, tmp_discp, ebufp);
				pbo_decimal_destroy(&tmp_discp);
			}
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, pbo_decimal_to_str(total_discp, ebufp));
			PIN_FLIST_DESTROY_EX(&discount_flistp, ebufp);
		}
		PIN_FLIST_DESTROY_EX(&plan_flistp, ebufp);
		pbo_decimal_destroy(&amtp);
	}

CLEANUP:
	if (*ret_flistp == NULL)
	{
		*ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_POID, plan_pdp, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_COUNT, &prod_count, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_PRODUCT_OBJ, prd_pdp, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistp, MSO_FLD_PKG_TYPE, &pkg_type, ebufp);
	}
	PIN_FLIST_FLD_PUT(*ret_flistp, PIN_FLD_CYCLE_DISCOUNT, total_discp, ebufp);
	PIN_FLIST_FLD_SET(*ret_flistp, PIN_FLD_STATUS, &service_disc_status, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(1, "Error during plan search");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "op_mso_cust_calc_price error",
                        ebufp);
        }

	PIN_ERR_LOG_FLIST(3, "PLan configuration return flist", *ret_flistp);	
	PIN_FLIST_DESTROY_EX(&srch_o_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_i_flistp, ebufp);
	pbo_decimal_destroy(&onep);
	pbo_decimal_destroy(&total_ncf_discp);
	pbo_decimal_destroy(&total_sub_discp);

	return ;
}
                       
void fm_mso_get_srv_dtls(
	pcm_context_t *ctxp,
	pin_flist_t	*in_flistp,
	pin_errbuf_t	*ebufp)
{
        pin_flist_t     *srch_i_flistp = NULL;
        pin_flist_t     *srch_o_flistp = NULL;
        pin_flist_t     *args_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
        pin_flist_t     *results_flistp = NULL;
	pin_flist_t	*alias_list = NULL;
	char 		*template = "select X from /service 1  where 1.F1=V1 and 1.F2!=V2 and 1.F3=V3";
	int32		flags = 256;
	int32		db_no = 1;
	poid_t		*srch_pdp = NULL;
	poid_t		*acct_pdp = NULL;
	poid_t		*srvc_pdp = NULL;
	int32		closed = 10103;
	
        acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	srch_pdp = PIN_POID_CREATE(db_no, "/search", -1, ebufp);
        srch_i_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &closed, ebufp);	
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, srvc_pdp, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
	
	PIN_ERR_LOG_FLIST(3, "fm_mso_get_srv_dtls : Search inflistp", srch_i_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp, &srch_o_flistp, ebufp);	
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(1, "Error during service search");
                goto CLEANUP;
        }
	PIN_ERR_LOG_FLIST(3, "fm_mso_get_srv_dtls : Search outflistp", srch_o_flistp);
	res_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	
	if(res_flistp)
	{
		PIN_ERR_LOG_FLIST(3, "Matching serice account", res_flistp);
	}
	else
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_ARG, PIN_FLD_SERVICE_OBJ, 0, 0);
	}
			
	CLEANUP:
	//PIN_FLIST_DESTROY_EX(&srch_o_flistp, NULL);
        //PIN_FLIST_DESTROY_EX(&srch_i_flistp, NULL);
	
	return;
	
}


