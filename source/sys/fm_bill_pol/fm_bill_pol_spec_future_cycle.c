/*
 *
* Copyright (c) 1996, 2013, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 * 
 * Version 1.0: 01-10-2013: Vilva Sabarikanth: Added the MSO Customization of
 * applying the Minimum CATV Bill Amount of Rs.150 per service
 */

#ifndef lint
static char Sccs_id[] = "@(#)%Portal Version: fm_bill_pol_spec_future_cycle.c:RWSmod7.3.1Int:3:2008-Mar-18 16:11:22 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_BILL_POL_SPEC_FUTURE_CYCLE operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
#include <time.h> 
#include "pcm.h"
#include "ops/bill.h"
#include "ops/ar.h"
#include "pin_bill.h"
#include "pin_rate.h"
#include "pin_pymt.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_cust.h"
#include "mso_ops_flds.h"
#include "mso_glid.h"

#define ITEM_TYPE_DEPOSIT  "/item/payment/mso_deposit"
#define ITEM_TYPE_ET  "/item/mso_et"
#define ITEM_TYPE_CYC_FWD_ANNUAL_PKG  "/item/cycle_forward/mso_sb_adn_fdp"
#define ITEM_TYPE_CYC_ARR_ANNUAL_AMC  "/item/cycle_arrear/mso_hw_amc"
#define ITEM_TYPE_MISC  "/item/misc"
#define ITEM_TYPE_PUR_ATP  "/item/mso_purchase_atp"
#define ITEM_TYPE_PUR_OTP  "/item/mso_purchase_outright"
#define ITEM_TYPE_PUR_HW  "/item/mso_sb_onetime_hw"
#define ITEM_TYPE_PUR_SRVC  "/item/mso_sb_onetime_srvc"
#define ITEM_TYPE_CYC_FWD_ET "/item/mso_et"
#define ITEM_TYPE_CYC_FWD_ANNUAL_AMC "/item/cycle_forward/mso_hw_amc"
#define ITEM_ST "/item/Service_Tax"
#define ITEM_ECESS "/item/Education_Cess"
#define ITEM_SHECESS "/item/Secondary_Higher_Education_Cess"
#define ITEM_SBC "/item/Swachh_Bharat_Cess"
#define ITEM_KKC "/item/Krish_Kalyan_Cess"
#define ITEM_VAT "/item/VAT"
#define ITEM_TYPE_CATV_COMMITMENT  "/item/mso_catv_commitment"
#define EVENT_TYPE_CATV_COMMITMENT "/event/billing/mso_catv_commitment"
#define ITEM_TYPE_RENTAL_FWD_HW "/item/cycle_arrear/mso_hw_rental"
#define ITEM_TYPE_RENTAL_ARR_HW "/item/cycle_forward/mso_hw_rental"
#define ITEM_TYPE_CYCLE_TAX "/item/cycle_tax"
#define ITEM_TYPE_PAYMENT "/item/payment"
#define ITEM_TYPE_PAYMENT_REVERSAL "/item/payment/reversal"
#define ITEM_TYPE_DISPUTE "/item/dispute"
#define ITEM_TYPE_ADJUSTMENT "/item/adjustment"

#define EVT_ADDON_1MONTH  "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_sb_adn_fdp"
#define EVT_ADDON_2MONTH  "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_adn_fdp"
#define EVT_ADDON_3MONTH  "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_sb_adn_fdp"
#define EVT_ADDON_6MONTH  "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_sb_adn_fdp"
#define EVT_ADDON_12MONTH "/event/billing/product/fee/cycle/cycle_forward_annual/mso_sb_adn_fdp"

#define EVT_PKG_1MONTH  "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_sb_pkg_fdp"
#define EVT_PKG_2MONTH  "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_pkg_fdp"
#define EVT_PKG_3MONTH  "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_sb_pkg_fdp"
#define EVT_PKG_6MONTH  "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_sb_pkg_fdp"
#define EVT_PKG_12MONTH "/event/billing/product/fee/cycle/cycle_forward_annual/mso_sb_pkg_fdp"

#define EVT_ALC_1MONTH  "/event/billing/product/fee/cycle/cycle_forward_monthly/mso_sb_alc_fdp"
#define EVT_ALC_2MONTH  "/event/billing/product/fee/cycle/cycle_forward_bimonthly/mso_sb_alc_fdp"
#define EVT_ALC_3MONTH  "/event/billing/product/fee/cycle/cycle_forward_quarterly/mso_sb_alc_fdp"
#define EVT_ALC_6MONTH  "/event/billing/product/fee/cycle/cycle_forward_semiannual/mso_sb_alc_fdp"
#define EVT_ALC_12MONTH "/event/billing/product/fee/cycle/cycle_forward_annual/mso_sb_alc_fdp"

#define SERVICE_TAX_CODE "MSO_Service_Tax"
#define MSO_ET_TAX_CODE "MSO_ET"


#define CATV_PROFILE "CATV"
#define BILLINFO_TYPE "/billinfo"

#define WITHOUT_TAX 1
#define WITH_TAX 2

#define CURRENCY 356

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
extern time_t fm_utils_cycle_actgfuturet();

extern pin_decimal_t *st_tax_rate;
extern pin_decimal_t *swachh_tax_rate;
extern pin_decimal_t *krish_tax_rate;

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_bill_pol_spec_future_cycle(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_bill_pol_spec_future_cycle_default();

static void
fm_bill_pol_spec_future_cycle_advance();

// MSO Customization Starts
void mso_bill_pol_catv_main(
	pcm_context_t		*ctxp,
	u_int			flags,
	poid_t			*binfo_pdp,
	poid_t			*service_pdp,	
	time_t			event_t,
	pin_opcode_t		op_num,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

void mso_bill_pol_retrieve_catv_unbilled_items(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	poid_t			*bill_obj_pdp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

void mso_bill_pol_catv_raise_correction(
	pcm_context_t		*ctxp,
	pin_decimal_t		*item_total,
	int32			event_t,
	time_t			*last_bill_t,
	time_t			*next_bill_t,
	poid_t			*billinfo_pdp,
	pin_flist_t		*sr_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

int mso_bill_pol_retrieve_purchased_product_status(
	pcm_context_t		*ctxp,
	poid_t			*svc_pdp,
	time_t			*last_bill_t,
	time_t			*next_bill_t,
	pin_errbuf_t		*ebufp);

int mso_bill_pol_retrieve_purchased_product_status_et(
	pcm_context_t		*ctxp,
	poid_t				*svc_pdp,
	pin_errbuf_t		*ebufp);

extern pin_decimal_t		*min_amount;
extern int32			*min_commit_flag;

PIN_EXPORT void 
mso_bill_pol_trigger_et_event(
	pcm_context_t		*ctxp,
	u_int			flags,
	pin_flist_t		*mso_et_flistp,
	pin_flist_t		*et_in_flistp,
	pin_flist_t		**et_out_flistp, //CATV_REFUND_API_CHANGE
	pin_errbuf_t		*ebufp);

int
mso_get_months_from_timestamp(
	time_t			timeStamp);

int
mso_get_years_from_timestamp(
	time_t			timeStamp);

int
mso_get_days_from_timestamp(
	time_t			timeStamp);

int
fm_mso_retrieve_base_fdp(
    pcm_context_t    *ctxp,
    poid_t          *svc_pdp,
    pin_flist_t     **fdp_flistp,
    pin_errbuf_t    *ebufp);

pin_decimal_t*
get_pdt_charge(
	pcm_context_t	*ctxp, 
	poid_t			*pdt_pdp, 
	pin_errbuf_t	*ebufp);

int
fm_mso_get_charging_dates(
	pcm_context_t   *ctxp,
    poid_t          *pdt_pdp,
    pin_errbuf_t    *ebufp);

PIN_EXPORT int32
fm_mso_catv_pt_pkg_type(
    pcm_context_t           *ctxp,
    poid_t                  *prd_pdp,
    pin_errbuf_t            *ebufp);

PIN_IMPORT
 char* mso_get_taxcode(
        pcm_context_t           *ctxp,
        poid_t                          *srvc_pdp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void mso_retrieve_et_code(
        pcm_context_t   *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp);


//MSO Customization Ends

/*******************************************************************
 * Main routine for the PCM_OP_BILL_POL_SPEC_FUTURE_CYCLE operation.
 *******************************************************************/
void
op_bill_pol_spec_future_cycle(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*ret_flistp = NULL;
	
	//MSO Customization Starts
	poid_t			*pdp = NULL;
	char			*profile = NULL;
	char			*poid_type = NULL;
//	char			msg[100];
	int32			errp = 0;
	time_t			*tmp = NULL;
	int32			event_t=0;
	//MSO_Custmization ends

	*r_flistpp = NULL;
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != PCM_OP_BILL_POL_SPEC_FUTURE_CYCLE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_spec_future_cycle bad opcode error", 
			ebufp);
		return;
	}

	/*
	 * Debug: What we got.
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_spec_future_cycle input flist", i_flistp);


	if (pin_conf_advance_bill_cycle) {
	/*
	 * The following example demonstrates a specific logic of setting of 
	 * the next_t and future_t fields.
	 * At the account creation, next_t is taken from the next day after 
	 * creation. Future_t is taken from the month after next_t.
	 * If next_t = Jan 28, and the year are not leap, then future_t = Mar 1.
	 */

		fm_bill_pol_spec_future_cycle_advance(i_flistp,
			r_flistpp, ebufp);
	} else {
	/*
	 * The code bellow is a default implementation
	 */
		fm_bill_pol_spec_future_cycle_default(i_flistp,
			r_flistpp, ebufp);
	}

	/*
	 * Error?
	 */

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"op_bill_pol_spec_future_cycle error", ebufp);
	} else {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_spec_future_cycle output flist", *r_flistpp);
	}

	
	//MSO Customization Starts
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_spec_future_cycle printing input flist", i_flistp);

//		if (*min_commit_flag == 0)
//		{
//			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Min. Commitment flag Disabled!");
//		}
//		else
//		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Entering CATV Custom");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_BILLINFO_ID, "", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				    	"Billinfo Object Input", i_flistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, i_flistp, &ret_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				    	"Billinfo Object Output", ret_flistp);
			pdp = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_POID, 1, ebufp);
			poid_type = (char *) PIN_POID_GET_TYPE(pdp);
			if (0 != strcmp(poid_type,BILLINFO_TYPE))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Non-Billing Process");
			}
			else
			{
				tmp = (time_t *)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_ACTG_NEXT_T, 1, ebufp);
//				sprintf(msg,"EVENT Time: %d",(time_t*)tmp);
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
				event_t = *tmp;
//				sprintf(msg,"EVENT Time: %d",(time_t*)event_t);
//				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

				profile = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_BILLINFO_ID, 1, ebufp);
				if(0 == strcmp(profile, CATV_PROFILE))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"CATV Customer");
					/* Added for Applying Minimum Bill Amount of Rs.150 for CATV Services */	
					//mso_bill_pol_catv_main(ctxp, flags, pdp, event_t, &ret_flistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp)) {
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
							"CATV Minimum Bill Amount Application Error", ret_flistp);
			        }
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Non-CATV Customer");
				}
			}
//		}
	
	PIN_FLIST_DESTROY(ret_flistp, ebufp);
	PIN_ERRBUF_CLEAR(ebufp);
	//MSO Customization Ends

	return;
}

/*******************************************************************
 * fm_bill_pol_spec_future_cycle_default():
 *
 *     Calculate fields PIN_FLD_ACTG_NEXT_T and PIN_FLD_ACTG_FUTURE_T 
 *
 *******************************************************************/
static void
fm_bill_pol_spec_future_cycle_default(
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;

	time_t			event_end_t = (time_t)0;
	time_t			next_t = (time_t)0;
	time_t			future_t = (time_t)0;

	int32			dom = 0;
	int32			cycle_len = 0;
	int32			*bill_typep = NULL;
	int			cycle_len_specified = 0;
	int			creation = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	*r_flistpp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, vp, ebufp);

	/*
	 * Get fields which are needed to calculate next_t and future_t
	 */
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
        if (vp) {
		event_end_t = *((time_t *)vp);
        }

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTG_CYCLE_DOM, 1, ebufp);
        if (vp) {
		dom = *((int32 *)vp);
        }

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTG_CYCLE_LEN, 1, ebufp);
        if (vp) {
		cycle_len = *((int32 *)vp);
		cycle_len_specified = 1;
        }

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTG_NEXT_T, 1, ebufp);
        if (vp) {
		next_t = *((time_t *)vp);
		creation = 0;
        }

	/*
	 * Calculate next_t
	 */
	if (creation) {
		next_t = fm_utils_cycle_actgfuturet(dom, cycle_len,
			event_end_t, ebufp);
	} 
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ACTG_NEXT_T, (void *)&next_t, 
		ebufp);

	/*
	 * Calculate future_t
	 */
	if (creation || !cycle_len_specified) {
	/* 
	 * This is a case of account creation or call from make_bill
	 */
		future_t = fm_utils_cycle_actgnextt(next_t, dom, ebufp);
		future_t = fm_utils_time_round_to_midnight(future_t);
	} else {
	/* This is a case when account already exists,
	 * and we are changing billing cycle
	 */
		future_t = fm_utils_cycle_actgfuturet(dom, cycle_len, next_t, 
			ebufp);
	} 
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ACTG_FUTURE_T, (void *)&future_t, 
		ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
		"fm_bill_pol_spec_future_cycle_default", ebufp);
        }

	return;
}

/*******************************************************************
 * fm_bill_pol_spec_future_cycle_advance():
 *
 *     Calculate fields PIN_FLD_ACTG_NEXT_T and PIN_FLD_ACTG_FUTURE_T 
 *
 *******************************************************************/
static void
fm_bill_pol_spec_future_cycle_advance(
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	void			*vp = NULL;

	time_t			event_end_t = (time_t)0;
	time_t			next_t = (time_t)0;
	time_t			future_t = (time_t)0;

	int32			dom = 0;
	int32			cycle_len = 0;
	int32			*bill_typep = NULL;
	int			cycle_len_specified = 0;
	int			adjust_next_t = 0;
	int			adjust_future_t = 0;
	int			subord = 0;
	int			next_dom = 0;
	int			curr_dom = 0;
	int			creation = 1;
	struct tm		*tm;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	*r_flistpp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, vp, ebufp);

	/*
	 * Get fields which are needed to calculate next_t and future_t
	 */
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
        if (vp) {
		event_end_t = *((time_t *)vp);
        }

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTG_CYCLE_DOM, 1, ebufp);
        if (vp) {
		dom = *((int32 *)vp);
        }

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTG_CYCLE_LEN, 1, ebufp);
        if (vp) {
		cycle_len = *((int32 *)vp);
		cycle_len_specified = 1;
        }

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTG_NEXT_T, 1, ebufp);
        if (vp) {
		next_t = *((time_t *)vp);
		creation = 0;
        }

	/*vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if (vp) {
		dom_advanced = *((int32 *)vp);
	}*/
	
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
        if (vp && (*((int32 *)vp) == PIN_PAY_TYPE_SUBORD)) {
		subord = 1;
	}

	/*
	 * Calculate next_t (only in the case of account creation)
	 */
	if (creation) {
		if (!subord) {
			curr_dom = fm_utils_cycle_get_dom(event_end_t, ebufp);
			if (dom >= curr_dom) {
			       next_t = event_end_t;
			       fm_utils_add_n_days(dom - curr_dom + 1, &next_t);
                               next_t = fm_utils_time_round_to_midnight(next_t);
			} else {
			       next_t = fm_utils_cycle_actgfuturet(dom,
                                       PIN_ACTG_CYCLE_SHORT, event_end_t, ebufp);
                                 fm_utils_add_n_days(1, &next_t);				
			}
		} else {  /* account is subordinate one */
		/*
		 * For subordinate account, dom is taken from parent.
		 * Therefore, dom can be equal to 29.
		 */	
			if (dom == 29) {
				adjust_next_t = 1;
				dom--;
				if (fm_utils_cycle_get_dom(event_end_t, 
					ebufp) > 1) {
			       		fm_utils_add_n_days(-1,
						&event_end_t);
				}
			}

			next_t = fm_utils_cycle_actgfuturet(dom, 
				PIN_ACTG_CYCLE_SHORT, event_end_t, ebufp);

			if (adjust_next_t) {
				fm_utils_add_n_days(1, &next_t);
			}

		} /* account is subordinate one */
	} /* (creation) */

	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ACTG_NEXT_T, (void *)&next_t, 
				ebufp);

	/*
	 * Calculate future_t
	 */
	if (creation || !cycle_len_specified) {
	/* This is a case of account creation or call from make_bill */
		next_dom = fm_utils_cycle_get_dom(next_t, ebufp);
		if (next_dom > 28) {
			fm_utils_add_n_days(-1, &next_t);
			adjust_future_t = 1;
		}

		future_t = fm_utils_cycle_actgnextt(next_t, dom, ebufp);
		if (adjust_future_t) {
			fm_utils_add_n_days(1, &future_t);
		} 
		/*
		 * Process of a special case when next_t = Mar 1.
		 * In this case, future_t should be Mar 29.
		 */
		if (next_dom == 1) {
			tm = localtime(&next_t);
			if (tm != NULL && tm->tm_mon == 2) {
				future_t = next_t;
				fm_utils_add_n_days(28, &future_t);
			}
		}
	} else { /* This is a case when account already exists, 
		  * and we are changing billing cycle 
		  */
		if (!subord) {
			dom++;
		}
		next_dom = fm_utils_cycle_get_dom(next_t, ebufp);
		if (next_dom == 29) {
			fm_utils_add_n_days(-1, &next_t);
			if (dom > 1) {
				dom--;
			} 
			adjust_future_t = 1;
		} else if (dom == 29) {
			dom--;
			if (next_dom > 1) {
				fm_utils_add_n_days(-1, &next_t);
			}
			adjust_future_t = 1;
		}

		future_t = fm_utils_cycle_actgfuturet(dom, cycle_len, next_t, 
			ebufp);

		if (adjust_future_t) {
			fm_utils_add_n_days(1, &future_t);
		}
	}

	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ACTG_FUTURE_T, (void *)&future_t, 
		ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_bill_pol_spec_future_cycle_advance", ebufp);
        }

	return;
}


// MSO Customization Starts

/* This is the main module for applying the Minimum Charge Amount of Rs.150(Configured in CM's pin.conf) 
 * for the CATV services, if the bill amount is lesser than Rs.150, it applies an adjustment of the 
 * remaining amount to the account to round off the amount to Rs.150 */
void mso_bill_pol_catv_main(
	pcm_context_t		*ctxp,
	u_int			flags,
	poid_t			*binfo_pdp,
	poid_t			*service_pdp,
	time_t			event_t,
	pin_opcode_t		op_num,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
 
/* This module calculates the total amount of all the unbilled items 
 * (for all the balance groups & sub services in a billinfo) and 
 * identify the differential amount for the adjustments */

	pin_flist_t			*s_flistp = NULL;
	pin_flist_t			*res_flistp = NULL;
	pin_flist_t			*i_flistp = NULL;
	pin_flist_t			*item_flistp = NULL;
	pin_flist_t			*flistp = NULL;
	pin_flist_t			*sub_flistp = NULL;
	pin_flist_t			*r_flistp = NULL;
	pin_flist_t			*srcp_flistp = NULL;
	pin_flist_t			*ar_flistp = NULL;
	pin_flist_t			*read_obj_iflistp = NULL;
	pin_flist_t			*read_obj_oflistp = NULL;
	pin_flist_t			*inv_info = NULL;
	pin_flist_t			*search_mso_et_iflistp = NULL;
	pin_flist_t			*search_mso_et_oflistp = NULL;
 	pin_flist_t			*args_flistp = NULL;
	pin_flist_t			*results = NULL;
	pin_flist_t			*mso_et_flistp = NULL;
	pin_flist_t         		*mso_et_flist_copy = NULL;
	pin_flist_t			*read_acct_iflistp = NULL;
	pin_flist_t			*read_acct_oflistp = NULL;
	pin_flist_t         		*et_in_flistp = NULL;
	pin_flist_t                     *et_out_flistp = NULL; //CATV_REFUND_API_CHANGE

	poid_t				*pdp = NULL;
	poid_t				*acct_pdp = NULL;
	poid_t				*adj_bal_pdp = NULL;
	poid_t				*bal_grp_pdp = NULL;
	poid_t				*bill_pdp = NULL;
	poid_t				*a_pdp = NULL;
	poid_t				*i_pdp = NULL;
	poid_t				*svc_pdp =NULL;
	poid_t				*search_pdp = NULL;
	poid_t				*payinfo_obj = NULL;
	poid_t				*bill_obj = NULL;
	poid_t				*pdt_pdp = NULL;

	u_int64				id = (u_int64)0;
	u_int64				db = PIN_POID_GET_DB(binfo_pdp);

	int32				s_flags = 256;
	int32 				count = 0;
	int32 				rec_id = 0;
	int32 				rec_id2 = 0;
	int32				errp = 0;
	int32				*conn_type = NULL;
	int32				min_commit_eligibility = 0;
	int32				base_plan = 0;
	int32				pay_indicator = 1;
	int32				*serv_status = NULL;
	int32				nb_local = 0;
	int32                           al_local = 0;
        int32                           lb_local = 0;

 	int64				no_of_days = 0;

	int 				pp_status = 0;
	int				scale = 0;
	int				charge_to_t_eligibility = 1;

	pin_decimal_t			*item_total = NULL;
	pin_cookie_t			cookie = NULL;
	pin_cookie_t			cookie2 = NULL;
//	pin_decimal_t			*min_amount = NULL;
	pin_decimal_t			*temp=NULL;
	pin_decimal_t			*zero_amount=NULL;
	pin_decimal_t			*subscription_amt = NULL;

	void				*vp = NULL;
	void				*service_vp = NULL;

	char				*item_type = NULL;
	char				msg[1000];
	char				*et_zone = NULL;
	char				*token = NULL;
	char				tmp[1000];

	time_t				*next_bill_t = NULL;
	time_t				*future_bill_t = NULL;
	time_t				*serv_created_t = NULL;
	time_t				zero = 0;
	time_t				zero1 = 0;
	time_t				*charge_to_t = &zero;
	time_t				*dummy = NULL;
	time_t				*event_charge_from_t = &zero1;
	time_t				*event_charge_to_t = &zero1;
	time_t				*actg_next_t = NULL;
	time_t                          *actg_last_t = NULL;
	time_t				*last_bill_t = NULL;
	time_t				*last_status_t = NULL;
	time_t				next_bt = 0;
	time_t                          last_bt = 0;
    	time_t              		calc_from_t = 0;
	time_t				calc_to_t = 0;
	time_t				*fdp_chrg_from_t = NULL;
	time_t				 fdp_t = 0;
	char                		*act = NULL;	


    struct tm			*tm1 = 0;
    time_t      		calc_bill_t = 0;
    time_t      		*fdp_pet = NULL;
    pin_flist_t 		*fdp_flistp = NULL;
    int         		base_fdp_active = 0;
	int32				diff = 0;

	PIN_ERRBUF_CLEAR(ebufp);

//	min_amount = pbo_decimal_from_str("0.0",ebufp);
	zero_amount = pbo_decimal_from_str("0.0",ebufp);
//	pin_conf("fm_bill_pol", "min_commitment_amount", PIN_FLDT_DECIMAL, (caddr_t*)&min_amount, &errp);
	charge_to_t = &zero;

	/* Check for Service type - prepaid or postpaid */
	read_obj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_obj_iflistp,PIN_FLD_POID,binfo_pdp,ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Billinfo input flist",read_obj_iflistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_obj_iflistp, &read_obj_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Billinfo output flist",read_obj_oflistp);

	next_bill_t = PIN_FLIST_FLD_TAKE(read_obj_oflistp,PIN_FLD_NEXT_BILL_T,1,ebufp);		//for future calculation of scale
	future_bill_t = PIN_FLIST_FLD_TAKE(read_obj_oflistp,PIN_FLD_FUTURE_BILL_T,1,ebufp);	//for future calculation of scale
	bill_obj = PIN_FLIST_FLD_TAKE(read_obj_oflistp,PIN_FLD_BILL_OBJ,1,ebufp);       //for future calculation of scale
	actg_next_t = PIN_FLIST_FLD_TAKE(read_obj_oflistp,PIN_FLD_ACTG_NEXT_T,1,ebufp);	//for checking ET trigger validity 
	last_bill_t = PIN_FLIST_FLD_TAKE(read_obj_oflistp,PIN_FLD_LAST_BILL_T,1,ebufp);	//for checking ET trigger validity 
	actg_last_t = PIN_FLIST_FLD_TAKE(read_obj_oflistp,PIN_FLD_ACTG_LAST_T,1,ebufp);

	next_bt = *next_bill_t;
	nb_local = *next_bill_t;;
	al_local = *actg_last_t;
        lb_local = *last_bill_t;
	last_bt = *last_bill_t;
	
	/* ET real time logic 
	payinfo_obj = PIN_FLIST_FLD_GET(read_obj_oflistp,PIN_FLD_PAYINFO_OBJ,1,ebufp);
	if(!PIN_POID_IS_NULL(payinfo_obj))
	{
		PIN_FLIST_FLD_SET(read_obj_iflistp,PIN_FLD_POID,payinfo_obj,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Payinfo input flist",read_obj_iflistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_obj_iflistp, &read_obj_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Read Payinfo output flist",read_obj_oflistp);

		if(read_obj_oflistp)
		{
			inv_info = PIN_FLIST_ELEM_GET(read_obj_oflistp,PIN_FLD_INV_INFO,PIN_ELEMID_ANY,1,ebufp);
			if(inv_info)
			{
				pay_indicator = PIN_FLIST_FLD_GET(inv_info,PIN_FLD_INDICATOR,1,ebufp);
			}
		}
	}
	ET real time logic */
	
	/* Search for all services of the billinfo getting billed */
	s_flistp = PIN_FLIST_CREATE(ebufp);
	ar_flistp = PIN_FLIST_CREATE(ebufp);

		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /service 1, /balance_group 2, /billinfo 3 where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = V5 and 1.F6 = V6 ", ebufp);

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)binfo_pdp, ebufp);

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
		pdp = PIN_POID_CREATE(db, "/service/catv", -1, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BAL_GRP_OBJ, (void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_CREATED_T, (void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_LAST_STATUS_T, (void *)NULL, ebufp);
		sub_flistp = PIN_FLIST_SUBSTR_ADD(flistp, MSO_FLD_CATV_INFO, ebufp);
		PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_CONN_TYPE, 0, ebufp);

		/* Get the /servive details for the /billinfo */
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"service search input flist", s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"service search output flist", *r_flistpp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			return;
		}

		
	i_flistp = PIN_FLIST_CREATE(ebufp);
	while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(*r_flistpp, PIN_FLD_RESULTS, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
	{
	svc_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_POID_COMPARE(service_pdp, svc_pdp, 0, ebufp) != 0)
          {
              continue;
          }
		/* Re-initialize for each service */
		charge_to_t_eligibility = 1;	
		no_of_days = 0;
		scale = 0;
		service_vp = (void *)PIN_POID_GET_TYPE(svc_pdp);
          if(strcmp((void*)service_vp, "/service/catv") ==0)
          {
		// Added to prevent duplicate ET 
		PIN_FLIST_DESTROY_EX(&mso_et_flist_copy, NULL);
		PIN_FLIST_DESTROY_EX(&search_mso_et_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&search_mso_et_oflistp, NULL);
			
		svc_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 1 , ebufp);
		acct_pdp = PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_ACCOUNT_OBJ,1,ebufp);
		bal_grp_pdp = PIN_FLIST_FLD_GET(res_flistp,PIN_FLD_BAL_GRP_OBJ,1,ebufp);
		flistp = PIN_FLIST_FLD_GET(res_flistp, MSO_FLD_CATV_INFO, 1 , ebufp);
		conn_type = PIN_FLIST_FLD_GET(flistp, PIN_FLD_CONN_TYPE, 1 , ebufp);
		serv_status = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_STATUS, 1 , ebufp);
		last_status_t = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_LAST_STATUS_T, 1, ebufp);

		/* Calculate no of months for ET calculation - start */
                search_mso_et_iflistp = PIN_FLIST_CREATE(ebufp);
                search_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
                PIN_FLIST_FLD_PUT(search_mso_et_iflistp, PIN_FLD_POID, (void *)search_pdp, ebufp);
                PIN_FLIST_FLD_SET(search_mso_et_iflistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
                PIN_FLIST_FLD_SET(search_mso_et_iflistp, PIN_FLD_TEMPLATE, "select X from /mso_et_charge_details 1 where 1.F1 = V1 ", ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(search_mso_et_iflistp,PIN_FLD_ARGS,1,ebufp);
		PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_SERVICE_OBJ,svc_pdp,ebufp);
		results = PIN_FLIST_ELEM_ADD(search_mso_et_iflistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_et_charge_details input flist", search_mso_et_iflistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_mso_et_iflistp, &search_mso_et_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search mso_et_charge_details output flist", search_mso_et_oflistp);
        mso_et_flistp = PIN_FLIST_ELEM_GET(search_mso_et_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1 , ebufp);
        if (mso_et_flistp)
        {
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_et_charge_details flist", mso_et_flistp);
            mso_et_flist_copy = PIN_FLIST_COPY(mso_et_flistp, ebufp);
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_et_charge_details flist copy ", mso_et_flist_copy);
        }
        
			//Checking for Base FDP Packs 
			fdp_flistp = PIN_FLIST_CREATE(ebufp);
			base_fdp_active = fm_mso_retrieve_base_fdp(ctxp, svc_pdp, &fdp_flistp, ebufp);
			if (base_fdp_active == 1)
			{
				if (fdp_flistp)
				{
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Base FDP Pack Available !!!", ebufp);
					//fdp_pet = PIN_FLIST_FLD_GET(fdp_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);
					fdp_pet = PIN_FLIST_FLD_GET(fdp_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
					calc_to_t = *fdp_pet;
					fdp_chrg_from_t = PIN_FLIST_FLD_GET(fdp_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
					sprintf(msg, "fdp_purchase_charge_to_t: %d", *fdp_pet);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					sprintf(msg, "fdp_purchase_charge_From_t: %d", *fdp_chrg_from_t);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
					pdt_pdp = PIN_FLIST_FLD_GET(fdp_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
					subscription_amt = get_pdt_charge(ctxp, pdt_pdp, ebufp);
					if(pbo_decimal_compare(subscription_amt, zero_amount, ebufp)==0)
					{
						base_fdp_active = 2;
					}

                    if (mso_et_flist_copy)
                    {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET Charge Details available.");
                        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ET Charge Details Flist", mso_et_flist_copy);
                        charge_to_t = PIN_FLIST_FLD_GET(mso_et_flist_copy, PIN_FLD_CHARGED_TO_T, 1, ebufp);
			if (charge_to_t && *charge_to_t > 0 && op_num != 11007)
			{
                        	calc_from_t = *charge_to_t;
			}
			else
			{
				 calc_from_t = *fdp_chrg_from_t;
			}
                        sprintf(msg, "ET charged_to_t: %d", *charge_to_t);
    					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
                        sprintf(msg, "ET calc charged_from_t: %d", calc_from_t);
    					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
			/*************************************************************
                         * Real time ET logic
                        *************************************************************/
		    	if (calc_from_t > calc_to_t)
                        {
                            scale = 0;
                            charge_to_t_eligibility = 0;
                        }
                        else
                        {
                            scale = round(((calc_to_t - calc_from_t)/86400)/30);
                        }
                    }
			else	
		   	{
			PIN_ERR_LOG_MSG(3, "For new service consider Charge_from_t");
			/*************************************************************
                         * Real time ET logic
                         *************************************************************/
			calc_from_t = *fdp_chrg_from_t;
			scale = round(((calc_to_t - calc_from_t)/86400)/30);
			}
				 }
                                else
                                {
                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base FDP Pack NOT Available !!!");
                                }
                        }
                /* Calculate no of months for ET calculation - end */

				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base FDP Pack NOT Available !!!");
				}



		/*********** Trigger ET ***********/
        if (PIN_ERRBUF_IS_ERR(ebufp))
            PIN_ERRBUF_CLEAR(ebufp);
    	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_et_flist display", mso_et_flistp);
            et_in_flistp = PIN_FLIST_CREATE(ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_END_T, charge_to_t, ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_NUMBER_OF_UNITS, &scale, ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_pdp, ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_EVENT_COUNT, &event_t, ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_STATUS_FLAGS, &charge_to_t_eligibility, ebufp);
	    PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_CHARGED_FROM_T, &calc_from_t, ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_CHARGED_TO_T, &calc_to_t, ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_BILL_OBJ, bill_obj, ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_INDICATOR, &pay_indicator, ebufp);
            PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_NEXT_BILL_T, &event_t, ebufp);
	    act = (char *) malloc(strlen("REAL TIME:ET CALCULATION")+1);
            strcpy(act, "REAL TIME:ET CALCULATION");
                       PIN_FLIST_FLD_PUT(et_in_flistp, PIN_FLD_ACTION, act, ebufp);
			PIN_FLIST_FLD_SET(et_in_flistp, PIN_FLD_CYCLE_FEE_FLAGS, &base_fdp_active, ebufp);
			// Adding offering obj_id0 
			if(fdp_flistp && base_fdp_active !=0 )
			{
				PIN_FLIST_FLD_COPY(fdp_flistp, PIN_FLD_POID, et_in_flistp, PIN_FLD_OFFERING_OBJ, ebufp);
			}
			
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ET_IN_FLISTP", et_in_flistp);
			//CATV_REFUND_API_CHANGE - Return flist added
			mso_bill_pol_trigger_et_event(ctxp, flags, mso_et_flistp, et_in_flistp, &et_out_flistp, ebufp);
			if ( !PIN_ERRBUF_IS_ERR(ebufp) && et_out_flistp !=  NULL ){
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ET Successfully Completed");
			}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Non-CATV Service Object");
		}
		//PIN_ERRBUF_RESET(ebufp);
		//item_total = pbo_decimal_from_str("0.0",ebufp);
	}
	if (!pbo_decimal_is_null(temp, ebufp))
	{
		pbo_decimal_destroy(&temp);
	}
	PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&i_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ar_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srcp_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&et_in_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&et_out_flistp, ebufp); //CATV_REFUND_API_CHANGE
	PIN_FLIST_DESTROY_EX(&search_mso_et_iflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&search_mso_et_oflistp, ebufp);
	
	//PIN_FLIST_DESTROY(res_flistp, ebufp);
	//PIN_FLIST_DESTROY(item_flistp, ebufp);
//	PIN_ERRBUF_CLEAR(ebufp);
	return;
}

/* This module retrieves all the unbilled items(pending status items) for the specific billinfo details */
void mso_bill_pol_retrieve_catv_unbilled_items(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	poid_t				*bill_obj_pdp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)1;
	u_int64			db = 0;
	pin_flist_t 	*s_flistp = NULL;
	pin_flist_t 	*sr_flistp = NULL;
	poid_t			*billinfo_pdp = NULL;
	poid_t			*svc_pdp = NULL;
	poid_t			*type1_pdp = NULL;
	poid_t			*type2_pdp = NULL;
	poid_t			*type3_pdp = NULL;
	poid_t			*type4_pdp = NULL;
	poid_t			*type5_pdp = NULL;
	poid_t			*type6_pdp = NULL;
	poid_t			*type7_pdp = NULL;
	poid_t			*type8_pdp = NULL;
	poid_t			*type9_pdp = NULL;
	poid_t			*type10_pdp = NULL;
	poid_t			*type11_pdp = NULL;
	poid_t			*type12_pdp = NULL;
	poid_t			*type13_pdp = NULL;
	poid_t			*type14_pdp = NULL;
    poid_t			*type15_pdp = NULL;
    poid_t			*type16_pdp = NULL;
//	pin_decimal_t	*zero_amount=NULL;
	int32			s_flags = 256;
	pin_flist_t		*flistp = NULL;
	poid_t			*pdp = NULL;
	int32 			p_items = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_bill_pol_retrieve_catv_unbilled_items", ebufp);
		return;
	}

	//PIN_ERRBUF_CLEAR(ebufp);
	billinfo_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILLINFO_OBJ, 0 , ebufp);
	svc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0 , ebufp);
//	zero_amount = pbo_decimal_from_str("0.0",ebufp);
	db=(u_int64)PIN_POID_GET_DB(billinfo_pdp);
        
	type1_pdp = PIN_POID_CREATE(db, ITEM_TYPE_DEPOSIT, id, ebufp);
	type2_pdp = PIN_POID_CREATE(db, ITEM_TYPE_ET, id, ebufp);
	type3_pdp = PIN_POID_CREATE(db, ITEM_TYPE_CYC_ARR_ANNUAL_AMC, id, ebufp);
	type4_pdp = PIN_POID_CREATE(db, ITEM_TYPE_MISC, id, ebufp);
	type5_pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_ATP, id, ebufp);
	type6_pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_OTP, id, ebufp);
	type7_pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_HW, id, ebufp);
	type8_pdp = PIN_POID_CREATE(db, ITEM_TYPE_PUR_SRVC, id, ebufp);
	type9_pdp = PIN_POID_CREATE(db, ITEM_ST, id, ebufp);
	type10_pdp = PIN_POID_CREATE(db, ITEM_ECESS, id, ebufp);
	type11_pdp = PIN_POID_CREATE(db, ITEM_SHECESS, id, ebufp);
	type12_pdp = PIN_POID_CREATE(db, ITEM_VAT, id, ebufp);
	//type13_pdp = PIN_POID_CREATE(db, ITEM_TYPE_CYC_FWD_ET, id, ebufp);
	type14_pdp = PIN_POID_CREATE(db, ITEM_TYPE_CYC_FWD_ANNUAL_AMC, id, ebufp);
    type15_pdp = PIN_POID_CREATE(db, ITEM_SBC, id, ebufp);
    type16_pdp = PIN_POID_CREATE(db, ITEM_KKC, id, ebufp);
	
	s_flistp = PIN_FLIST_CREATE(ebufp);
	sr_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
    PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /item where F1 = V1 and F2= V2 and F3 = V3 and (F4.type <> V4 and F5.type <> V5 and F6.type <> V6 and F7.type <> V7 and F8.type <> V8 and F9.type <> V9 and F10.type <> V10 and F11.type <> V11 and F12.type <> V12 and F13.type <> V13 and F14.type <> V14 and F15.type <> V15 and F16.type <> V16 and F17.type <>V17 and F18.type <> V18) and F19 = V19", ebufp);
//	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /item where F1 = V1 and F2= V2 and F3 = V3 and (F4.type <> V4 and F5.type <> V5 and F6.type <> V6 and F7.type <> V7 and F8.type <> V8 and F9.type <> V9 and F10.type <> V10 and F11.type <> V11 and F12.type <> V12 and F13.type <> V13 and F14.type <> V14 and F15.type <> V15 and F16.type <> V16 ) and F17 = V17", ebufp);
//	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /item where F1 = V1 and F2= V2 and F3 = V3 and (F4.type <> V4 and F5.type <> V5 and F6.type <> V6 and F7.type <> V7 and F8.type <> V8 and F9.type <> V9 and F10.type <> V10 and F11.type <> V11 and F12.type <> V12 and F13.type <> V13 and F14.type <> V14 and F15.type <> V15 and F16.type <> V16 and F17.type <> V17 ) and F18 = V18 and F19 >= 0", ebufp);
//	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /item where F1 = V1 and F2= V2 and F3 = V3 and (F4.type <> V4 and F5.type <> V5 and F6.type <> V6 and F7.type <> V7 and F8.type <> V8 and F9.type <> V9 and F10.type <> V10 and F11.type <> V11 and F12.type <> V12 and F13.type <> V13 and F14.type <> V14 and F15.type <> V15 and F16.type <> V16", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_OBJ, billinfo_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&p_items, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type1_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type2_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type3_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type4_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type5_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type6_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 10, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type7_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 11, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type8_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 12, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type9_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 13, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type10_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 14, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type11_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 15, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type12_pdp, ebufp);
//	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 16, ebufp);
//	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type13_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 16, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type14_pdp, ebufp);
    flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 17, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type15_pdp, ebufp);
    flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 18, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, type16_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 19, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILL_OBJ, bill_obj_pdp, ebufp);
//	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 19, ebufp);
//	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ITEM_TOTAL, zero_amount, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)NULL, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_BAL_GRP_OBJ, (void *)NULL, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_BILL_OBJ, (void *)NULL, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_ITEM_TOTAL, (void *)NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"Pending items search input flist", s_flistp);
	/* Get the pending item details for the service_obj */
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &sr_flistp, ebufp);
	*r_flistpp = PIN_FLIST_COPY(sr_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"Pending items search output flist", *r_flistpp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Pending items search Error");
		return;
	}
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&sr_flistp, ebufp);
	//PIN_ERRBUF_CLEAR(ebufp);
	return;
}

/* This module applies the Account Level Adjustment of the differential amount */
void mso_bill_pol_catv_raise_correction(
	pcm_context_t	*ctxp,
	pin_decimal_t	*item_total,
	int32			event_t,
	time_t			*last_bill_t,
	time_t			*next_bill_t,
	poid_t			*billinfo_pdp,
	pin_flist_t		*sr_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t	*ebufp)	
{
	
	pcm_context_t	*new_ctxp = NULL;
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*out_flistp  = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*bal_grp_pdp = NULL;
	poid_t			*evt_pdp = NULL;
	poid_t			*pdp = NULL;
	pin_flist_t		*event_flistp = NULL;
	pin_flist_t		*bal_impact_flistp = NULL;
	pin_flist_t		*total_flistp = NULL;
	pin_flist_t		*adj_in_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*elem_flistp = NULL;
	pin_flist_t		*tr_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_decimal_t	*adj_amount = NULL;
	
	int64			db = 0;
	int32			id = -1;
	int32			imp_type = 4;
	int32			pre_imp_type = PIN_IMPACT_TYPE_PRERATED;
	int32			tax = PIN_AR_WITH_TAX; 
	int32			errp = 0;
	int32			tax_when = 1;
	int32			glid =  GLID_MIN_COMMIT;
	int			currency = 356;
	poid_t			*obj;
	poid_t			*uid = NULL;
	poid_t			*sid = NULL;
	poid_t			*i_pdp = NULL;
	poid_t			*adj_pdp = NULL;
	int				unlimited = 0;   
	void			*vp = NULL;
//	time_t			now_t = pin_virtual_time((time_t *)NULL);
	time_t			tmp_t = 0;
	char			msg[100];

	
	//PIN_ERRBUF_CLEAR(ebufp);    

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_bill_pol_catv_raise_correction", ebufp);
		return;
	}

	
	adj_amount = pbo_decimal_from_str("0.0",ebufp);
	
	res_flistp = PIN_FLIST_ELEM_GET(sr_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (res_flistp)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"apply adjustment pre processing input flist res_flistp", res_flistp);
	
		acct_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		bal_grp_pdp =  PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
		adj_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);

		db = PIN_POID_GET_DB(acct_pdp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Items Retrieval Error");
			goto CLEANUP;
		}

		adj_amount = (pin_decimal_t *)pbo_decimal_subtract(item_total, min_amount, ebufp);
		if(pbo_decimal_is_null(adj_amount, ebufp)==1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Adj amount null error");
		}

		if(pbo_decimal_sign(adj_amount, ebufp) == -1)
		{
			if(pbo_decimal_is_null(adj_amount, ebufp)==1)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Adj amount null sign comparison");
			}
			adj_amount = (pin_decimal_t *)pbo_decimal_negate(adj_amount, ebufp);
		}
			
		if(pbo_decimal_is_null(adj_amount, ebufp)==1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Adj amount null error after subtraction");
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"1");
		// Applying the Minimum Commitment Adjustment as effective one hour prior to the accounting date
//		sprintf(msg,"EVENT Time: %d",event_t);
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

		tmp_t = event_t;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"2");

//		sprintf(msg,"EVENT Time: %d",(time_t*)tmp_t);
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
		
//  		Input FLIST Prepared for PCM_OP_AR_ACCOUNT_ADJUSTMENT Opcode
//		adj_in_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
//		PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_BAL_GRP_OBJ, adj_in_flistp, PIN_FLD_BAL_GRP_OBJ, ebufp);
//		PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_PROGRAM_NAME, "Automatic Adjustment", ebufp);
//		PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_DESCR, "Min. Bill Amount Adjustment", ebufp);
//		PIN_FLIST_FLD_PUT(adj_in_flistp, PIN_FLD_AMOUNT, (pin_decimal_t *)adj_amount, ebufp);
//		PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_FLAGS, (void *)&tax, ebufp);
//		PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_START_T, &due_t, ebufp);
//		PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_END_T, &due_t, ebufp);
//
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adjustment input flist", adj_in_flistp);
//		PCM_OP(ctxp, PCM_OP_AR_ACCOUNT_ADJUSTMENT, 0, adj_in_flistp, &r_flistp, ebufp);
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adjustment output flist", r_flistp);
//
//		res_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adjustment results flist", res_flistp);
//		flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_BAL_IMPACTS, 0, 1, ebufp);
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Adjustment bal_impacts flist", flistp);
//		i_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_ITEM_OBJ, 1, ebufp);
//		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "Adjustment item poid", i_pdp);
//
//		/* Doing Bill Item transfer for the proper allocation of the adjustment for the same bill itself*/
//
//		tr_flistp = PIN_FLIST_CREATE(ebufp);
//		PIN_FLIST_FLD_SET(tr_flistp, PIN_FLD_POID, acct_pdp, ebufp);
//		PIN_FLIST_FLD_SET(tr_flistp, PIN_FLD_ITEM_OBJ, i_pdp, ebufp);
//		PIN_FLIST_FLD_SET(tr_flistp, PIN_FLD_PROGRAM_NAME, "Min. Bill Amount Item Transfer", ebufp);
//		flistp = PIN_FLIST_ELEM_ADD(tr_flistp, PIN_FLD_ITEMS, 0, ebufp);
//		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, adj_pdp, ebufp);
//		PIN_FLIST_FLD_SET(flistp, PIN_FLD_AMOUNT, (pin_decimal_t *)adj_amount, ebufp);
//		PIN_FLIST_FLD_SET(flistp, PIN_FLD_CURRENCY, &currency, ebufp);
//
//		r_flistp = NULL;
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
//			"PCM_OP_BILL_ITEM_TRANSFER Input flist", tr_flistp);
//		PCM_OP(ctxp, PCM_OP_BILL_ITEM_TRANSFER, 0, tr_flistp, &r_flistp, ebufp);
//		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
//			"PCM_OP_BILL_ITEM_TRANSFER Output flist", r_flistp);
	        event_flistp = PIN_FLIST_CREATE(ebufp);
	        *ret_flistp = PIN_FLIST_CREATE(ebufp);
	        //pdp = PIN_POID_CREATE(db, "/", id, ebufp);
	        evt_pdp = PIN_POID_CREATE(db, EVENT_TYPE_CATV_COMMITMENT, id, ebufp);
        
	        PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_POID, acct_pdp, ebufp);
	        sub_flistp = PIN_FLIST_SUBSTR_ADD(event_flistp, PIN_FLD_EVENT, ebufp);
	        PIN_FLIST_FLD_PUT(sub_flistp, PIN_FLD_POID, evt_pdp, ebufp);
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"3");
	        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	        PIN_FLIST_FLD_COPY(sr_flistp, PIN_FLD_SERVICE_OBJ, sub_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_BILLINFO_OBJ,billinfo_pdp, ebufp);
	        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_NAME, "MSO_CATV_COMMITMENT", ebufp);
	        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PROGRAM_NAME, "pin_bill_accts", ebufp);
	        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_DESCR,"Min. Bill Commitment Charge", ebufp);
	        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SYS_DESCR,"Billing Charge :Minimum Commitment Charge", ebufp);
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"4");
	        uid = CM_FM_EFFECTIVE_USERID(ebufp);
	        if (!uid)
	        {
		        uid = CM_FM_USERID(ebufp);
	        }
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"5");
	        sid = CM_FM_BASE_SESSION(ebufp);
	        PIN_FLIST_FLD_PUT(sub_flistp, PIN_FLD_USERID, (void *)uid,ebufp);
	        if (sid != (poid_t *)NULL)
	        {
		        PIN_FLIST_FLD_PUT(sub_flistp, PIN_FLD_SESSION_OBJ,(void *)sid, ebufp);
        
	        }
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"6");
	        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_START_T, (void *)&tmp_t, ebufp);
	        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_END_T, (void *)&tmp_t, ebufp);
	        PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_EFFECTIVE_T, (void *)&tmp_t, ebufp);
	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"7");
	
		//Mapping the Balance Impacts element details
		bal_impact_flistp=PIN_FLIST_ELEM_ADD(sub_flistp, PIN_FLD_BAL_IMPACTS,0, ebufp);
            	PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_QUANTITY,(void *)adj_amount,ebufp);
            	PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_AMOUNT,(void *)adj_amount,ebufp);
        	PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_TAX_CODE,SERVICE_TAX_CODE,ebufp);
//		PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_TAX_WHEN,&tax_when,ebufp);
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_IMPACT_TYPE,&pre_imp_type,ebufp);
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_BAL_GRP_OBJ,(void *)bal_grp_pdp,ebufp);
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_ACCOUNT_OBJ,(void *)acct_pdp,ebufp);
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_RESOURCE_ID,(void *)&currency,ebufp);
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_GL_ID, &glid,ebufp);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"8");
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_RESOURCE_ID_ORIG,(void *)&currency,ebufp);
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_RATE_TAG,"MIN_COMMITMENT",ebufp);
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_DISCOUNT,(pin_decimal_t *)0,ebufp);
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_PERCENT,(pin_decimal_t *)0,ebufp);
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_AMOUNT_DEFERRED,(pin_decimal_t *)0,ebufp);
                PIN_FLIST_FLD_SET(bal_impact_flistp,PIN_FLD_AMOUNT_ORIG,(pin_decimal_t *)0,ebufp);
                total_flistp=PIN_FLIST_ELEM_ADD(sub_flistp, PIN_FLD_TOTAL,0, ebufp);
                PIN_FLIST_FLD_SET(total_flistp,PIN_FLD_AMOUNT,(void *)adj_amount,ebufp)
                PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"9");
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_FLIST_LOG_ERR("Error in Opening context pointer mso_bill_pol_catv_raise_correction", ebufp);
			PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
			return;
		}
		elem_flistp = PIN_FLIST_SUBSTR_ADD(sub_flistp,PIN_FLD_CYCLE_INFO,ebufp);
		sprintf(msg, "%d",tax_when);
		PIN_FLIST_FLD_SET(elem_flistp,PIN_FLD_SCALE,(pin_decimal_t*)pbo_decimal_from_str(msg,ebufp),ebufp);
		PIN_FLIST_FLD_SET(elem_flistp,PIN_FLD_CYCLE_START_T,last_bill_t,ebufp);
		PIN_FLIST_FLD_SET(elem_flistp,PIN_FLD_CYCLE_END_T,next_bill_t,ebufp);
		PIN_FLIST_FLD_SET(elem_flistp,PIN_FLD_ORIGINAL_SCALE,(pin_decimal_t*)pbo_decimal_from_str(msg,ebufp),ebufp);
		PIN_FLIST_FLD_SET(elem_flistp,PIN_FLD_FLAGS,&tax_when,ebufp);


		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "event_flistp i/p flist", event_flistp);
		PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, event_flistp, &out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "event_flistp o/p flist",  out_flistp);
		PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);

		*ret_flistp = out_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"CATV Minimum Commitment Correction output flist", *ret_flistp);
	}
CLEANUP:
	PIN_FLIST_DESTROY_EX(&adj_in_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&tr_flistp, ebufp);
	if (pbo_decimal_is_null(min_amount, ebufp))
	{
		pbo_decimal_destroy(&min_amount);
	}
	if (pbo_decimal_is_null(adj_amount, ebufp))
	{
		pbo_decimal_destroy(&adj_amount);
	}
//	PIN_ERRBUF_CLEAR(ebufp);
	return;
}

/* This module checks and returns whether the FDP & Priority 90  products is active for current billing cycle for Min. Commitment*/
int mso_bill_pol_retrieve_purchased_product_status(
	pcm_context_t		*ctxp,
	poid_t			*svc_pdp,
	time_t			*last_bill_t,
	time_t			*next_bill_t,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)1;
	u_int64			db = 0;
	pin_flist_t 		*s_flistp = NULL;
	pin_flist_t 		*sr_flistp = NULL;
	pin_flist_t 		*rs_flistp = NULL;
	pin_flist_t 		*pdt_in_flistp = NULL;
	pin_flist_t 		*pdt_out_flistp = NULL;
	int32			s_flags = 256;
	int32			rec_id = 0;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*t_flistp = NULL;
	pin_cookie_t    	cookie = NULL;
	char			*evt_type = NULL;
	poid_t			*pdp = NULL;
	int32 			active = 1;
	int32			inactive = 2;
	int32			cancelled = 3;
	int 			flags = 0;
	time_t			now_t = pin_virtual_time((time_t *)NULL);
	time_t			zero = 0;
	pin_decimal_t		*non_das = NULL;
	pin_decimal_t		*fdp_priority = NULL;
	pin_decimal_t		*base_lower = NULL;
	pin_decimal_t		*base_upper = NULL;
	pin_decimal_t		*temp = NULL;
	void			*vp = NULL;
	char			*template_str = "select X from /product 1, /purchased_product 2 where 1.F1 = 2.F2 and 2.F3 = V3 and "
			"((2.F4 = V4 and ( 2.F5 < V5 and ( 2.F6 > V6 or 2.F7 = V7 )))"  // Ststus  = 1
			"or (2.F8 = V8 and (2.F9 < V9 and 2.F10 > V10)) "		// Ststus  = 3
			"or (2.F11 = V11 and (2.F12 > V12 and (2.F13 > V13 or 2.F14 = V14)))) "		// Ststus  = 2
			"order by 1.F15 ";

        int32                           nb_local = 0;
        int32                           lb_local = 0;
	char                    msg[1000];
		char                    tmp[1000];
	//char			*descr = NULL;
	int32			pkg_type = -1;
	poid_t			*prd_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_bill_pol_retrieve_purchased_product_status", ebufp);
		return;
	}
	
	//PIN_ERRBUF_CLEAR(ebufp);
	
        nb_local = *next_bill_t;
        lb_local = *last_bill_t;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Our Dates below");
	sprintf(msg, "next_bill_t = %d", nb_local);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	sprintf(msg, "last_bill_t = %d", lb_local);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
	
	db=(u_int64)PIN_POID_GET_DB(svc_pdp);
	temp = pbo_decimal_from_str("0.0",ebufp);
	non_das = pbo_decimal_from_str("90.0",ebufp);
	fdp_priority = pbo_decimal_from_str("170.0",ebufp);
	base_lower = pbo_decimal_from_str("100.0",ebufp);
	base_upper = pbo_decimal_from_str("130.0",ebufp);

	s_flistp = PIN_FLIST_CREATE(ebufp);
	sr_flistp = PIN_FLIST_CREATE(ebufp);
	pdt_in_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
//	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /purchased_product where F1 = V1 and F2 = V2 and F3 >= V3 ", ebufp);
/*	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /product 1, /purchased_product 2 where 1.F1 = 2.F2  and 2.F3 = V3 and (2.F4 = V4 or (2.F5 > V5 and 2.F6 < V6)) order by 1.F7 ", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PRODUCT_OBJ, svc_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&active, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PURCHASE_END_T, last_bill_t, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PURCHASE_END_T, next_bill_t, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PRIORITY, temp, ebufp);
*/
//	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /product 1, /purchased_product 2 where 1.F1 = 2.F2  and 2.F3 = V3 and (2.F4 = V4 or (2.F5 = V5 and (2.F6 > V6 and 2.F7 < V7)) or (2.F8 = V8 and (2.F9 > V9 and 2.F10 < V10 ))) order by 1.F11 ", ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template_str, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PRODUCT_OBJ, svc_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);

	// Active Products
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&active, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PURCHASE_START_T, next_bill_t, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PURCHASE_END_T, last_bill_t, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PURCHASE_END_T, &zero, ebufp);

	// Cancelled Products
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&cancelled, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PURCHASE_START_T, next_bill_t, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 10, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PURCHASE_END_T, last_bill_t, ebufp);

	// Inactive Products
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 11, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&inactive, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 12, ebufp);
	t_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_CYCLE_FEES, 1, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_CHARGED_TO_T, last_bill_t, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 13, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PURCHASE_END_T, last_bill_t, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 14, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PURCHASE_END_T, &zero, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 15, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PRIORITY, temp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_PRIORITY, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PROVISIONING_TAG, (void *)NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Purchased Products search input flist", s_flistp);
	/* Get the pending item details for the service_obj */
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &sr_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"Products search output flist", sr_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Products search Error");
		return;
	}
	
	while((rs_flistp = PIN_FLIST_ELEM_GET_NEXT(sr_flistp, PIN_FLD_RESULTS, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
	{
		prd_pdp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_POID, 1, ebufp);
		//descr = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_DESCR, 1, ebufp);
		vp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_PRIORITY, 1, ebufp);
		temp = pbo_decimal_copy( (pin_decimal_t*)vp, ebufp);
		if(pbo_decimal_is_null(temp, ebufp)==1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PRIORITY Null error");
		}

		if (pbo_decimal_compare(temp,fdp_priority,ebufp) == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PRIORITY 170: FDP Package Available");
			//flags = 1;
			/**added changes to get the package type to identify the addon(pkg_type=1) and alacarte(pkg_type=2) products ******/
			pkg_type = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp);
			if(pkg_type == 1 || pkg_type == 2)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PRIORITY 170: FDP Package which is Alcarte or Addon Available");
				flags = 4;
			}
			else if(pkg_type == 3)
			{
				flags = 1;
                                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PRIORITY 170: FDP Package for which Subscription Available");
                                    goto CLEANUP;
			}
			else 
			{
				pin_set_err(ebufp, PIN_ERRLOC_CM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, MSO_FLD_PKG_TYPE, 0, 0);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Valid  package id is not found for the FDP packs ");	
				goto CLEANUP;
			}
		  
		}
		else if (pbo_decimal_compare(temp,non_das,ebufp) == 0)
		{
			flags = 2;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PRIORITY 90: Non_DAS Package Available");
		}
		else if ((pbo_decimal_compare(temp,base_lower,ebufp) >= 0) && (pbo_decimal_compare(base_upper,temp,ebufp) >= 0))
		{
			flags = 3;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "PRIORITY 100-130: Base Packages Available");
			goto CLEANUP;
		}
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&sr_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&pdt_in_flistp, ebufp);
	//PIN_ERRBUF_CLEAR(ebufp);
	return flags;
}

/* This module checks and returns whether the FDP is active for current billing cycle for ET calculations*/
int mso_bill_pol_retrieve_purchased_product_status_et(
	pcm_context_t		*ctxp,
	poid_t				*svc_pdp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)1;
	u_int64			db = 0;
	pin_flist_t 	*s_flistp = NULL;
	pin_flist_t 	*sr_flistp = NULL;
	pin_flist_t 	*rs_flistp = NULL;
	pin_flist_t 	*pdt_in_flistp = NULL;
	pin_flist_t 	*pdt_out_flistp = NULL;
	int32			s_flags = 256;
	int32			rec_id = 0;
	pin_flist_t		*flistp = NULL;
	pin_cookie_t    cookie = NULL;
	char			*evt_type = NULL;
	poid_t			*pdp = NULL;
	int32 			active = 1;
	int 			fdp_available = 0;
	time_t			now_t = pin_virtual_time((time_t *)NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_bill_pol_retrieve_purchased_product_status_et", ebufp);
		return 0;
	}
	//PIN_ERRBUF_CLEAR(ebufp);
	
	db=(u_int64)PIN_POID_GET_DB(svc_pdp);
	
	s_flistp = PIN_FLIST_CREATE(ebufp);
	sr_flistp = PIN_FLIST_CREATE(ebufp);
	pdt_in_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
//	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /purchased_product where F1 = V1 and F2 = V2 and F3 >= V3 ", ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /purchased_product where F1 = V1 and F2 = V2 ", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)&active, ebufp);
	/* Check only current status as per new design */
//	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
//	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PURCHASE_END_T, (void *)&now_t, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_PRODUCT_OBJ, (void *)NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Purchased Products search input flist", s_flistp);
	/* Get the pending item details for the service_obj */
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &sr_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"Purchased Products search output flist", sr_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Purchased Products search Error");
		return 0;
	}
	
	while((rs_flistp = PIN_FLIST_ELEM_GET_NEXT(sr_flistp, PIN_FLD_RESULTS, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
	{
		PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_PRODUCT_OBJ, pdt_in_flistp, PIN_FLD_POID, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"Product Read input flist", pdt_in_flistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, pdt_in_flistp, &pdt_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"Product Read output flist", pdt_out_flistp);

		flistp = PIN_FLIST_ELEM_GET(pdt_out_flistp, PIN_FLD_USAGE_MAP, 0, 1, ebufp);
		if (flistp)
		{
			evt_type = PIN_FLIST_FLD_GET(flistp, PIN_FLD_EVENT_TYPE, 0, ebufp);
			if (evt_type && ((strcmp(EVT_ALC_1MONTH, evt_type)==0)|| 
							(strcmp(EVT_ALC_2MONTH, evt_type)==0) || 
							(strcmp(EVT_ALC_3MONTH, evt_type)==0) || 
							(strcmp(EVT_ALC_6MONTH, evt_type)==0) || 
							(strcmp(EVT_ALC_12MONTH, evt_type)==0) ||
							(strcmp(EVT_PKG_1MONTH, evt_type)==0) ||
							(strcmp(EVT_PKG_2MONTH, evt_type)==0) ||
							(strcmp(EVT_PKG_3MONTH, evt_type)==0) ||
							(strcmp(EVT_PKG_6MONTH, evt_type)==0) ||
							(strcmp(EVT_PKG_12MONTH, evt_type)==0) ||
							(strcmp(EVT_ADDON_1MONTH, evt_type)==0) ||
							(strcmp(EVT_ADDON_2MONTH, evt_type)==0) ||
							(strcmp(EVT_ADDON_3MONTH, evt_type)==0) ||
							(strcmp(EVT_ADDON_6MONTH, evt_type)==0) ||
							(strcmp(EVT_ADDON_12MONTH, evt_type)==0)))
			{
				fdp_available = 1;
				goto CLEANUP;
			}
		}
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&sr_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&pdt_in_flistp, ebufp);
	//PIN_ERRBUF_CLEAR(ebufp);
	return fdp_available;
}

int
mso_get_months_from_timestamp(
        time_t          timeStamp)
{
        struct tm tmStruct;

        localtime_r((const time_t *)&timeStamp, &tmStruct);

        return tmStruct.tm_mon;
}


int
mso_get_years_from_timestamp(
        time_t          timeStamp)
{
        struct tm tmStruct;

        localtime_r((const time_t *)&timeStamp, &tmStruct);

        return tmStruct.tm_year;
}


int
mso_get_days_from_timestamp(
        time_t          timeStamp)
{
        struct tm tmStruct;

        localtime_r((const time_t *)&timeStamp, &tmStruct);

        return tmStruct.tm_mday;
}


void mso_bill_pol_trigger_et_event(
	pcm_context_t           *ctxp,
	u_int			flags,
	pin_flist_t		*mso_et_flistp,
	pin_flist_t		*et_in_flistp,
	pin_flist_t             **et_out_flistp, //CATV_REFUND_API_CHANGE
	pin_errbuf_t		*ebufp)
{
	int32                   p_items = 1;
	int32					count = 0;
	int64                   db = 0;
	int32                   id = -1;
	int32                   s_flags = 256;
	int32                   rec_id = 0;
	pin_cookie_t    	cookie = NULL;
	pin_decimal_t		*subscription_amt = NULL;
	pin_decimal_t		*mon_subscription_amt = NULL;
	pin_decimal_t		*tax_amt = NULL;
	pin_flist_t             *search_items_iflistp = NULL;
	poid_t			*search_pdp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*results = NULL;
	pin_flist_t		*search_items_oflistp = NULL;
	pin_flist_t		*item_flistp = NULL;
	poid_t			*item_pdp = NULL;
	char			*item_pdp_type = NULL;
	pin_decimal_t	*item_total = NULL;
	int				fdp_active = 0;
	pin_flist_t		*event_iflistp = NULL;
	poid_t			*evt_pdp = NULL;
	pin_flist_t		*sub_flistp = NULL;
	pin_flist_t		*elem_flistp = NULL;
	pin_flist_t		*event_oflistp = NULL;
	pin_flist_t		*wrt_flds_iflistp = NULL;
	pin_flist_t		*wrt_flds_oflistp = NULL;
	pin_flist_t		*create_obj_iflistp = NULL;
	pin_flist_t		*create_obj_oflistp = NULL;
	pin_flist_t		*cycle_fee_flistp = NULL;
	pin_flist_t		*et_tax_flistp = NULL;
	pin_flist_t		*et_code_flistp = NULL;
	poid_t			*obj_pdp = NULL;
	char			msg[500];
	pin_decimal_t	*zero = NULL;
	pin_decimal_t	*scale_decimal = NULL;
	pin_decimal_t	*calc_scale_decimal = NULL;
	pin_decimal_t	*tax_rate = NULL;
	pin_decimal_t	*hundred = pbo_decimal_from_str("100.0",ebufp);
	pin_decimal_t	*et_amt = NULL;
	int			currency = 356;
	int32			pre_imp_type = PIN_IMPACT_TYPE_PRERATED;
	//int32			pre_imp_type = PIN_IMPACT_TYPE_RATED;
	int32			glid =  GLID_ET;
	pin_flist_t		*bal_impact_flistp = NULL;
	pin_flist_t		*total_flistp = NULL;
	char			*sub_amt_str = NULL;
	char			*tax_amt_str = NULL;
//	time_t			pvt = pin_virtual_time((time_t *)NULL);
	int32			event_t = 0;
	int32			subscription_fee_flag = 0;

	poid_t			*acct_pdp = NULL;
	poid_t			*svc_pdp = NULL;
	poid_t			*bal_grp_pdp = NULL;
	poid_t			*pdt_pdp = NULL;
	poid_t			*bill_obj = NULL;
	poid_t			*offering_obj = NULL;
	time_t			*event_charge_from_t = NULL;
	time_t			*event_charge_to_t = NULL;
	time_t			*charge_to_t = NULL;
	time_t			*next_bill_t = NULL;
	time_t			*cycle_charge_from_t = NULL;
	time_t			*cycle_charge_to_t = NULL;
	int32			*pay_indicator = NULL;
	int32			actg_next_t = 0;
	int32			scale =0;
	int32			calc_scale = 0;
	int32			charge_to_t_eligibility = 1;
	int32			base_fdp_active = 0;
	int32			et_config = 0;
	char			*act = NULL;
	char			*et_tax_code = NULL;
	void			*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_bill_pol_trigger_et_event", ebufp);
		return;
	}
	

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Entering mso_bill_pol_trigger_et_event() ");
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ET_IN_FLISTP", et_in_flistp);
	charge_to_t = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_END_T, 1, ebufp);
	scale = *(int*)PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_NUMBER_OF_UNITS, 1, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	svc_pdp = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	bal_grp_pdp = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
	offering_obj = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
	actg_next_t = *(int *)PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_EVENT_COUNT, 1, ebufp);
	charge_to_t_eligibility = *(int *)PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp);
	event_charge_from_t = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_CHARGED_FROM_T, 1, ebufp);
	event_charge_to_t = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_CHARGED_TO_T, 1, ebufp);
	bill_obj = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_BILL_OBJ, 1, ebufp);
	pay_indicator = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_INDICATOR, 1, ebufp);
	next_bill_t = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_NEXT_BILL_T, 1, ebufp);
	act = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_ACTION, 1, ebufp);
	pdt_pdp = PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
	PIN_ERR_LOG_MSG(3, "Entry point 4");

	//Block to fetch if et configured is zero

	et_tax_code = mso_get_taxcode(ctxp, svc_pdp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                goto CLEANUP;
        }
        et_tax_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(et_tax_flistp, PIN_FLD_POID, acct_pdp, ebufp);
        PIN_FLIST_FLD_SET(et_tax_flistp, PIN_FLD_TAX_CODE, et_tax_code, ebufp);
        PIN_ERR_LOG_FLIST(3 ,"ET tax code input flist", et_tax_flistp);

        mso_retrieve_et_code(ctxp, et_tax_flistp, &et_code_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(3 ,"ET tax code output flist", et_code_flistp);
	if(et_code_flistp)
	{
		et_amt  = PIN_FLIST_FLD_GET(et_code_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		if(pbo_decimal_is_zero(et_amt,  ebufp) !=1)
		{
			et_config =1 ;
			PIN_ERR_LOG_MSG(3, "ET config Available for this account");
		}
	}


	fdp_active = *(int *)PIN_FLIST_FLD_GET(et_in_flistp, PIN_FLD_CYCLE_FEE_FLAGS, 1, ebufp);

	base_fdp_active = fdp_active;
	
	PIN_ERR_LOG_MSG(3, "Entry point 5");

	subscription_amt = pbo_decimal_from_str("0.0",ebufp);
	tax_amt = pbo_decimal_from_str("0.0",ebufp);
	zero = pbo_decimal_from_str("0.0",ebufp);
	tax_rate = pbo_decimal_from_str("0.0",ebufp);

	/* Retreive all pending items */
	db=(u_int64)PIN_POID_GET_DB(svc_pdp);
	search_items_iflistp = PIN_FLIST_CREATE(ebufp);
	search_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(search_items_iflistp, PIN_FLD_POID, (void *)search_pdp, ebufp);
	PIN_FLIST_FLD_SET(search_items_iflistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(search_items_iflistp, PIN_FLD_TEMPLATE, "select X from /item where F1 = V1 and F2 = V2 ", ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_items_iflistp,PIN_FLD_ARGS,1,ebufp);
	//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, (void *)&p_items, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_BILL_OBJ, bill_obj, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(search_items_iflistp,PIN_FLD_ARGS,2,ebufp);
	PIN_FLIST_FLD_SET(args_flistp,PIN_FLD_SERVICE_OBJ,svc_pdp,ebufp);
	results = PIN_FLIST_ELEM_ADD(search_items_iflistp,PIN_FLD_RESULTS,PIN_ELEMID_ANY,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Search bill items input flist",search_items_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_items_iflistp, &search_items_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Search bill items output flist",search_items_oflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Search bill items Error");
			return;
		}

	count = PIN_FLIST_ELEM_COUNT(search_items_oflistp,PIN_FLD_RESULTS,ebufp);	
	if(count == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"No bill items");
	}
	else
	{

		/* Accumulate subscription charge amt and tax amt from valid items */
		while((item_flistp = PIN_FLIST_ELEM_GET_NEXT(search_items_oflistp, PIN_FLD_RESULTS, &rec_id, 1, &cookie,  ebufp))!=(pin_flist_t*)NULL)
		{
			item_pdp = PIN_FLIST_FLD_GET(item_flistp,PIN_FLD_POID,0,ebufp);
			item_pdp_type = (void *)PIN_POID_GET_TYPE(item_pdp);
			item_total = PIN_FLIST_FLD_GET(item_flistp,PIN_FLD_ITEM_TOTAL,0,ebufp);

			if(strcmp((void*)item_pdp_type,ITEM_TYPE_CYC_ARR_ANNUAL_AMC) != 0
				&& strcmp((void*)item_pdp_type,ITEM_TYPE_CYC_FWD_ANNUAL_AMC) != 0
				&& strcmp((void*)item_pdp_type,ITEM_TYPE_PUR_HW) != 0
				&& strcmp((void*)item_pdp_type,ITEM_TYPE_RENTAL_FWD_HW) != 0
				&& strcmp((void*)item_pdp_type,ITEM_TYPE_RENTAL_ARR_HW) != 0
				&& strcmp((void*)item_pdp_type,ITEM_TYPE_ET) != 0
				&& strcmp((void*)item_pdp_type,ITEM_VAT) != 0)
			{
				if(strcmp((void*)item_pdp_type,ITEM_ST) == 0
					|| strcmp((void*)item_pdp_type,ITEM_ECESS) == 0
					|| strcmp((void*)item_pdp_type,ITEM_SHECESS) == 0
					|| strcmp((void*)item_pdp_type,ITEM_SBC) == 0
					|| strcmp((void*)item_pdp_type,ITEM_KKC) == 0)
				{
					pbo_decimal_add_assign(tax_amt,item_total,ebufp);	
					tax_amt_str = pbo_decimal_to_str(tax_amt,ebufp);
					sprintf(msg, "tax_amt = %s",tax_amt_str);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
				}
				else
				{
					pbo_decimal_add_assign(subscription_amt,item_total,ebufp);
					sub_amt_str = pbo_decimal_to_str(subscription_amt,ebufp);
					sprintf(msg, "subscription_amt = %s",sub_amt_str);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
					
					if(pbo_decimal_compare(item_total,zero,ebufp) == 1)
					{
						subscription_fee_flag = 1;
					}
				}
			}
		}


			/* Check if any FDP product is active or not 
			since we are doing this search already ,added this condition
			so that this block is entered when no FDP base pack available*/
			if(fdp_active ==0 )
			{
				fdp_active = mso_bill_pol_retrieve_purchased_product_status_et(ctxp, svc_pdp, ebufp);
			}
		  
		  
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error");
            return;
        }
	}

	// Validation to By-pass the Subscription flag for Base FDP packs validity beyond next billing date
	if (event_charge_to_t && next_bill_t && (*event_charge_to_t > *next_bill_t) && (base_fdp_active ==1))
	{
		subscription_fee_flag = 1;
	}

	/* Special scenario: Service is supended in entire bill cycle, but may be reactivated in next cycle.
	* For Prepaid, set charge_to_t to next_bill_t instead of future_bill_t if no charge item has been found.
	* This scenario applies for only flat ET, although it shouldn't have any impact on % based ET.
	*/


	sprintf(msg, "charge_to_t_eligibility = %d",charge_to_t_eligibility);	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	sprintf(msg, "fdp_active = %d",fdp_active);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	sub_amt_str = pbo_decimal_to_str(subscription_amt,ebufp);
	sprintf(msg, "subscription_amt = %s",sub_amt_str);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	tax_amt_str = pbo_decimal_to_str(tax_amt,ebufp);
	sprintf(msg, "tax_amt = %s",tax_amt_str);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	
	/* Trigger ET event only if FDP product is active or subscription_amt != 0 
	 * Add validations for karnataka_eligibility_flag & charge_to_t_eligibility */
	if(charge_to_t_eligibility == 1 ) //&& (fdp_active == 1 || pbo_decimal_is_zero(subscription_amt,ebufp) == 0))
	{
		/* We should apply ET only if there are any positive charges in the bill cycle */
		//Added condition to check if ET config is available before creatiing zero impact events
		if(subscription_fee_flag == 1 && et_config == 1 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"test1");
			event_iflistp = PIN_FLIST_CREATE(ebufp);
			evt_pdp = PIN_POID_CREATE(db, "/event/mso_et", id, ebufp);
			PIN_FLIST_FLD_SET(event_iflistp, PIN_FLD_POID, acct_pdp, ebufp);
			sub_flistp = PIN_FLIST_SUBSTR_ADD(event_iflistp, PIN_FLD_EVENT, ebufp);
			PIN_FLIST_FLD_PUT(sub_flistp, PIN_FLD_POID, evt_pdp, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_NAME, "MSO_ET", ebufp);
			if (scale >= 0)
			{
				PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_DESCR,"Entertainment Tax", ebufp);
				PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_PROGRAM_NAME, "pin_bill_accts", ebufp);
				PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SYS_DESCR,"Billing time: Entertainment Tax", ebufp);
			}
			else
			{
				PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_DESCR,"Entertainment Tax Refund", ebufp);
				PIN_FLIST_FLD_COPY(et_in_flistp, PIN_FLD_PROGRAM_NAME, sub_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
				PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SYS_DESCR,"Real time: Entertainment Tax Refund", ebufp);
			}
			
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_SYS_DESCR,"Billing time: Entertainment Tax", ebufp);	
			event_t = actg_next_t; // Trigger event at 1hr before actg_next_t else item will point to next bill.
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_START_T, (void *)&event_t, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_END_T, (void *)&event_t, ebufp);
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_EFFECTIVE_T, (void *)&event_t, ebufp);
			elem_flistp = PIN_FLIST_ELEM_ADD(sub_flistp, PIN_FLD_EVENTS, 0, ebufp);
			PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_AMOUNT, subscription_amt, ebufp);
			PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_TAX, tax_amt, ebufp);
			sprintf(msg, "%d",scale);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
			PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_SCALE, (pin_decimal_t*)pbo_decimal_from_str(msg,ebufp), ebufp);
			PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_CHARGED_FROM_T, event_charge_from_t, ebufp);
			PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_CHARGED_TO_T, event_charge_to_t, ebufp);
			PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_CYCLE_FEE_FLAGS, &subscription_fee_flag, ebufp);
			PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_FLAGS, &fdp_active, ebufp);
			PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_OFFERING_OBJ, offering_obj, ebufp);

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"test3");
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ET event creation input flist", event_iflistp);
			PCM_OP(ctxp, PCM_OP_ACT_USAGE, flags, event_iflistp, &event_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ET event creation output flist", event_oflistp);
			PIN_FLIST_DESTROY_EX(&event_iflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&event_oflistp, ebufp);

			if (PIN_ERRBUF_IS_ERR(ebufp)) 
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "ET event creation Error");
				return;
			}
			*et_out_flistp = PIN_FLIST_COPY(event_oflistp, ebufp); //CATV_REFUND_API_CHANGE
			PIN_FLIST_DESTROY_EX(&event_oflistp, ebufp);
		}

		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"test4");
		/* Write or create /mso_et_charge_details with updated details */
		if(mso_et_flistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"test5");
			if (scale != 0 && fdp_active != 0 )
			{
            
				/* Write charge_to_t in existing object */
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling WRITE_FLDS for Active FDP Cases");
				wrt_flds_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(mso_et_flistp,PIN_FLD_POID,wrt_flds_iflistp,PIN_FLD_POID,ebufp);
				PIN_FLIST_FLD_SET(wrt_flds_iflistp, PIN_FLD_CHARGED_TO_T, event_charge_to_t, ebufp);
				PIN_FLIST_FLD_SET(wrt_flds_iflistp, PIN_FLD_ACTION, act, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS input flist", wrt_flds_iflistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, wrt_flds_iflistp, &wrt_flds_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS output flist", wrt_flds_oflistp);

				PIN_FLIST_DESTROY_EX(&wrt_flds_iflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&wrt_flds_oflistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "PCM_OP_WRITE_FLDS Error");
					return;
				}
			}
			else if(scale >= 0 && fdp_active == 0 || (scale ==0 && (act && strstr(act, "CHANGE PLAN - ET REFUND"))))
			{
				/* Write charge_to_t in existing object for FDP Inactive Cases*/
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling WRITE_FLDS for Inactive FDP Cases");
				wrt_flds_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(mso_et_flistp,PIN_FLD_POID,wrt_flds_iflistp,PIN_FLD_POID,ebufp);
				PIN_FLIST_FLD_SET(wrt_flds_iflistp, PIN_FLD_CHARGED_TO_T, charge_to_t, ebufp);
				PIN_FLIST_FLD_SET(wrt_flds_iflistp, PIN_FLD_ACTION, act, ebufp);

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS input flist", wrt_flds_iflistp);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, wrt_flds_iflistp, &wrt_flds_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_WRITE_FLDS output flist", wrt_flds_oflistp);

				PIN_FLIST_DESTROY_EX(&wrt_flds_iflistp, ebufp);
				PIN_FLIST_DESTROY_EX(&wrt_flds_oflistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "PCM_OP_WRITE_FLDS Error");
					return;
				}
			}
		}
		else
		{
		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"test6");
			/* First time ET - create object */
			create_obj_iflistp = PIN_FLIST_CREATE(ebufp);
			obj_pdp = PIN_POID_CREATE(db, "/mso_et_charge_details", id, ebufp);
			PIN_FLIST_FLD_PUT(create_obj_iflistp, PIN_FLD_POID, obj_pdp, ebufp);
			PIN_FLIST_FLD_SET(create_obj_iflistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(create_obj_iflistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
			PIN_FLIST_FLD_SET(create_obj_iflistp, PIN_FLD_CHARGED_TO_T, charge_to_t, ebufp);
			PIN_FLIST_FLD_SET(create_obj_iflistp, PIN_FLD_ACTION, act, ebufp);			
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ input flist", create_obj_iflistp);
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ, flags, create_obj_iflistp, &create_obj_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_CREATE_OBJ output flist", create_obj_oflistp);

			PIN_FLIST_DESTROY_EX(&create_obj_iflistp, ebufp);
			PIN_FLIST_DESTROY_EX(&create_obj_oflistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "PCM_OP_CREATE_OBJ Error");
				return;
			}

		}
	}
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&et_tax_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&et_code_flistp, NULL);
		return;
}

pin_decimal_t*
get_pdt_charge(
	pcm_context_t	*ctxp, 
	poid_t			*pdt_pdp, 
	pin_errbuf_t	*ebufp)
{
    pin_flist_t     *srch_input_flistp = NULL;
    pin_flist_t     *srch_output_flistp = NULL;
    pin_flist_t     *args_flistp = NULL;
    pin_flist_t     *results_flistp = NULL;
    pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *qt_flistp = NULL;
	pin_flist_t     *bi_flistp = NULL;
	pin_flist_t     *res_qt_flistp = NULL;
	pin_flist_t     *res_bi_flistp = NULL;
	pin_decimal_t	*zero = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t	*total_amt = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t	*scaled_amt = pbo_decimal_from_str("0.0",ebufp);
	pin_decimal_t	*fixed_amt = pbo_decimal_from_str("0.0",ebufp);
	int64           db = 0;
    char            *template = {"select X from /rate 1, /rate_plan 2, /product 3 where 3.F1 = V1 and 3.F2 = 2.F3 and 2.F4 = 1.F5 and 1.F6 = V6"};
    char            *pt = "";
	poid_t			*s_pdp = NULL;
    int32           s_flags = 768;
	int				currency = 356;

	if (PIN_ERRBUF_IS_ERR(ebufp))
        return 0;

    srch_input_flistp = PIN_FLIST_CREATE(ebufp);
    db = PIN_POID_GET_DB(pdt_pdp);

    s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(srch_input_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
    PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
    PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, pdt_pdp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, pdt_pdp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, pdt_pdp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 4, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, pdt_pdp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 5, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_RATE_PLAN_OBJ, pdt_pdp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 6, ebufp);
	qt_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_QUANTITY_TIERS, PIN_ELEMID_ANY, ebufp);
	bi_flistp = PIN_FLIST_ELEM_ADD(qt_flistp, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_ELEMENT_ID, &currency, ebufp);

    res_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_RESULTS, 0, ebufp);
	qt_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_QUANTITY_TIERS, PIN_ELEMID_ANY, ebufp);
	bi_flistp = PIN_FLIST_ELEM_ADD(qt_flistp, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_FIXED_AMOUNT, zero, ebufp);
	PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_SCALED_AMOUNT, zero, ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "Product Rate Search Input Flist", srch_input_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_input_flistp, &srch_output_flistp, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
        PIN_ERRBUF_RESET(ebufp);
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Product Rate Search Output Flist", srch_output_flistp);

    PIN_FLIST_DESTROY_EX(&srch_input_flistp, NULL);
    if (srch_output_flistp)
    {
        results_flistp = PIN_FLIST_ELEM_GET(srch_output_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if (results_flistp)
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Rates Found");
			res_qt_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_QUANTITY_TIERS, PIN_ELEMID_ANY, 1, ebufp);
			res_bi_flistp = PIN_FLIST_ELEM_GET(res_qt_flistp, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp);
			fixed_amt = PIN_FLIST_FLD_GET(res_bi_flistp, PIN_FLD_FIXED_AMOUNT, 1, ebufp);
			scaled_amt = PIN_FLIST_FLD_GET(res_bi_flistp, PIN_FLD_SCALED_AMOUNT, 1, ebufp);
            total_amt = pbo_decimal_add(fixed_amt, scaled_amt, ebufp);
			return total_amt;
        }
        else
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Purchase Product NOT Available...");
    }

    PIN_FLIST_DESTROY_EX(&srch_output_flistp, NULL);
    return zero;
}

int
fm_mso_get_charging_dates(
	pcm_context_t   *ctxp,
    poid_t          *pdt_pdp,
    pin_errbuf_t    *ebufp)
{    
	pin_flist_t     *srch_input_flistp = NULL;
    pin_flist_t     *srch_output_flistp = NULL;
    pin_flist_t     *args_flistp = NULL;
    pin_flist_t     *results_flistp = NULL;
    pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *usage_flistp = NULL;
	pin_flist_t     *res_usage_flistp = NULL;
	pin_flist_t		*evt_map_flistp = NULL;
    pin_flist_t		*evts_flistp = NULL;
    pin_flist_t		*res_evt_map_flistp = NULL;
    pin_flist_t		*res_evts_flistp = NULL;
	int64           db = 0;
    char            *template = {"select X from /product where F1 = V1"};
    char            *template1 = {"select X from /config/event_map where F1 = V1 "};
    char            *etype = NULL;
	poid_t			*s_pdp = NULL;
    int32           s_flags = 768;
	int				currency = 356;
    int             count = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
        PIN_ERRBUF_CLEAR(ebufp);

    srch_input_flistp = PIN_FLIST_CREATE(ebufp);
    db = PIN_POID_GET_DB(pdt_pdp);

    s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(srch_input_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
    PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
    PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, pdt_pdp, ebufp);
     
    res_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	usage_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(usage_flistp, PIN_FLD_EVENT_TYPE, "", ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "Product Search Input Flist", srch_input_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_input_flistp, &srch_output_flistp, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
        PIN_ERRBUF_RESET(ebufp);
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Product Search Output Flist", srch_output_flistp);

    PIN_FLIST_DESTROY_EX(&srch_input_flistp, NULL);
    if (srch_output_flistp)
    {
        results_flistp = PIN_FLIST_ELEM_GET(srch_output_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if (results_flistp)
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Product Found");
			res_usage_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_USAGE_MAP, PIN_ELEMID_ANY, 1, ebufp);
            etype = PIN_FLIST_FLD_GET(res_usage_flistp, PIN_FLD_EVENT_TYPE, 1, ebufp);

            if (etype && (strcmp(etype, "")!=0))
            {
                srch_input_flistp = PIN_FLIST_CREATE(ebufp);
                db = PIN_POID_GET_DB(pdt_pdp);

                s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
                PIN_FLIST_FLD_PUT(srch_input_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
                PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
                PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_TEMPLATE, template1, ebufp);

                args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 1, ebufp);
                evt_map_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_EVENT_MAP, PIN_ELEMID_ANY, ebufp);
                evts_flistp = PIN_FLIST_ELEM_ADD(evt_map_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, ebufp);
                PIN_FLIST_FLD_SET(evts_flistp, PIN_FLD_EVENT_TYPE, etype, ebufp);
                
                res_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	            evt_map_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_EVENT_MAP, PIN_ELEMID_ANY, ebufp);
	            evts_flistp = PIN_FLIST_ELEM_ADD(evt_map_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, ebufp);
                PIN_FLIST_FLD_SET(evts_flistp, PIN_FLD_COUNT, 0, ebufp);

                PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "Event Mapping Search Input Flist", srch_input_flistp);
                PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_input_flistp, &srch_output_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp))
                {
                    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
                    PIN_ERRBUF_RESET(ebufp);
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Event Mapping Output Flist", srch_output_flistp);

                PIN_FLIST_DESTROY_EX(&srch_input_flistp, NULL);
                if (srch_output_flistp)
                {
                    results_flistp = PIN_FLIST_ELEM_GET(srch_output_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                    if (results_flistp)
                    {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Event Mapping Found");
                        res_evt_map_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_EVENT_MAP, PIN_ELEMID_ANY, 1, ebufp);
                        if (res_evt_map_flistp)
                        {
                            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Event Mapping Found");
                            res_evts_flistp = PIN_FLIST_ELEM_GET(res_evt_map_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, 1, ebufp);
                            if (res_evts_flistp)
                            {
                                count  = *(int *)PIN_FLIST_FLD_GET(res_evts_flistp, PIN_FLD_COUNT, 1, ebufp);
                                return count;

                            }
                        }
                    }
                    
                }

            }
            else
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Event Mapping NOT Available...");


        }
        else
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Product NOT Available...");
    }

    PIN_FLIST_DESTROY_EX(&srch_output_flistp, NULL);
    return 0;

}


int
fm_mso_retrieve_base_fdp(
	pcm_context_t   *ctxp,
	poid_t          *svc_pdp,
	pin_flist_t     **fdp_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t     *srch_input_flistp = NULL;
	pin_flist_t     *srch_output_flistp = NULL;
	pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *results_flistp = NULL;
	pin_flist_t     *res_flistp = NULL;
	pin_flist_t     *flistp = NULL;
	pin_flist_t     *cycle_flistp = NULL;
	pin_flist_t     *usg_flistp = NULL;
	poid_t          *s_pdp = NULL;
	//int             active_status = 1;
	int32			closed_status = 3;
	int             basepack_flag = 0;
	int64           db = 0;
	char            *template = {"select X from /purchased_product 1, /product 2, /mso_cfg_catv_pt 3 where 1.F1 = V1 and 1.F2 != V2 and 1.F3 = 2.F4 and 2.F5 = 3.F6 and 2.F7 like V7 and 3.F8 = V8 order by 1.F9 desc"};
	char            *pt = "";
	int32           s_flags = 768;
	


    if (PIN_ERRBUF_IS_ERR(ebufp))
        return 0;

    srch_input_flistp = PIN_FLIST_CREATE(ebufp);
    db = PIN_POID_GET_DB(svc_pdp);

    s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(srch_input_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
    PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
    PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &closed_status, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, svc_pdp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 4, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, svc_pdp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 5, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, pt, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 6, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, pt, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 7, ebufp);
    usg_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_USAGE_MAP, 0, ebufp);
    PIN_FLIST_FLD_SET(usg_flistp, PIN_FLD_EVENT_TYPE, "%fdp%", ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 8, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PKG_TYPE, &basepack_flag, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 9, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, svc_pdp, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PRODUCT_OBJ, svc_pdp, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_PURCHASE_END_T, 0, ebufp);
	cycle_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_CYCLE_FEES, 1, ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "fm_mso_retrieve_base_fdp Search Input Flist", srch_input_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_input_flistp, &srch_output_flistp, ebufp);
    if (PIN_ERRBUF_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
        PIN_ERRBUF_RESET(ebufp);
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_retrieve_base_fdp Search Output Flist", srch_output_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "fm_mso_retrieve_base_fdp Search Input Flist", srch_input_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_input_flistp, &srch_output_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_retrieve_base_fdp Search Output Flist", srch_output_flistp);
	PIN_FLIST_DESTROY_EX(&srch_input_flistp, NULL);
	if (srch_output_flistp)
	{
		results_flistp = PIN_FLIST_ELEM_GET(srch_output_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (results_flistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Purchase Product Available...");
			*fdp_flistp = PIN_FLIST_COPY(results_flistp, ebufp);
			cycle_flistp = PIN_FLIST_ELEM_GET(results_flistp, PIN_FLD_CYCLE_FEES, 1, 0, ebufp);
			PIN_FLIST_FLD_COPY(cycle_flistp, PIN_FLD_CHARGED_FROM_T, *fdp_flistp, PIN_FLD_CHARGED_FROM_T, ebufp);
			PIN_FLIST_FLD_COPY(cycle_flistp, PIN_FLD_CHARGED_TO_T, *fdp_flistp, PIN_FLD_CHARGED_TO_T, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_output_flistp, NULL);
			return 1;
		}
		else
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Purchase Product NOT Available...");
	}

	PIN_FLIST_DESTROY_EX(&srch_output_flistp, NULL);
	return 0;
}
// MSO Customization Ends
