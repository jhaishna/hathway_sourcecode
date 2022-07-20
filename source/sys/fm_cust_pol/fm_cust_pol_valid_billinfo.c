/*******************************************************************
 *
z* Copyright (c) 1999, 2013, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_valid_billinfo.c:RWSmod7.3.1Int:1:2007-Jun-28 05:36:36 %";
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <time.h>
 
#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_pymt.h"
#include "pin_currency.h"
#include "fm_utils.h"
#include "psiu_currency.h"
#include "pinlog.h"
#include "cm_cache.h"
EXPORT_OP void
op_cust_pol_valid_billinfo(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_valid_billinfo(
	cm_nap_connection_t	*connp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_valid_billinfo_common(
	cm_nap_connection_t	*connp,
	pin_flist_t		*new_flistp,
	pin_flist_t		*old_flistp,
	pin_flist_t		*par_flistp,
	pin_flist_t		*res_flistp,
	int32			is_creation,
	int32			is_subord,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_valid_billinfo_bill_when(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_allow_change,
	int32	   	is_subord,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_billinfo_currency(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_subord,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_billinfo_dom(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_allow_change,
	int32		is_subord,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_billinfo_actg_type(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*res_flistp,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_billinfo_exist(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*res_flistp,
	int32		is_creation,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_billinfo_ar(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_subord,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_billinfo_acct(
	cm_nap_connection_t	*connp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_subord,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_billinfo_billing_segment(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_allow_change,
	int32		is_subord,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_billinfo_dom_from_cache(
	pcm_context_t   *ctxp,
	pin_flist_t*	res_flistp,
	int32		future_dom,
	int32		billing_segment,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_validate_billing_segment_from_cache(
	pcm_context_t   *ctxp,
	pin_flist_t	*res_flistp,
	int32		billing_segment,
	pin_errbuf_t	*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
extern void
fm_cust_pol_validate_fld_value (
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*r_flistp,
	int32		pin_fld_field_num,
	int32		pin_fld_element_id,
	char		*cfg_name,
	int32		type,
	pin_errbuf_t	*ebufp);

extern pin_flist_t *
fm_cust_pol_valid_add_fail(pin_flist_t  *r_flistp,
	int32		field,
	int32		elemid,
	int32		result,
	char		*descr,
	void		*val,
	pin_errbuf_t	*ebufp);

extern void fm_cust_billinfo_read_parent(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**par_flistpp,
	int32		is_subord,
	pin_errbuf_t	*ebufp);

extern u_int fm_utils_cycle_is_valid_dom(
	u_int		dom,
	pin_errbuf_t	*ebufp);

extern pin_flist_t *
fm_cust_utils_get_config_dom_map_cache(
	pcm_context_t   *ctxp,
        int64           database,
        pin_errbuf_t    *ebufp);

extern pin_flist_t *
fm_cust_utils_get_config_billing_segment_cache(
	pcm_context_t   *ctxp,
	int64		database,
	pin_errbuf_t    *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_BILLINFO  command
 *******************************************************************/
void
op_cust_pol_valid_billinfo(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	*ret_flistpp = NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_VALID_BILLINFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_valid_billinfo", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_valid_billinfo input flist", in_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_valid_billinfo(connp, flags, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_valid_billinfo error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_billinfo return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo()
 *
 * Validate the given billinfo
 *
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo(
	cm_nap_connection_t	*connp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*old_flistp = NULL;
	pin_flist_t		*new_flistp = NULL;
	pin_flist_t		*par_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	void			*vp = NULL;
	int32			result = PIN_CUST_VERIFY_PASSED;
	int32			is_creation = PIN_BOOLEAN_FALSE;
	int32			is_subord = PIN_BOOLEAN_FALSE;
	poid_t		  	*bi_pdp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	res_flistp = PIN_FLIST_CREATE(ebufp);
	*o_flistpp = res_flistp;

	/***********************************************************
	 * Add the account poid
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, vp, ebufp);
	
	/***********************************************************
	 * For now, assume pass.
	 ***********************************************************/
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);

	/***********************************************************
	 * Detect whether the billinfo is created
	 ***********************************************************/
	new_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO,
						PIN_ELEMID_NEW, 0, ebufp);
	bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_POID, 0,
									ebufp);
	if (PIN_POID_GET_ID(bi_pdp) <= 0) {
		is_creation = PIN_BOOLEAN_TRUE;
	}

	/******************************************************************
	 * Detect whether the billinfo is subordinate
	 ******************************************************************/
	vp = PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
	if (vp && (*(int32 *)vp == PIN_PAY_TYPE_SUBORD)) {
		is_subord = PIN_BOOLEAN_TRUE;
	}

	/************************************************************ 
	 * Set billinfo poid and billinfo id to the result
	 ************************************************************/
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_BILLINFO_OBJ, (void *)bi_pdp, 
									ebufp);
	vp = PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_BILLINFO_ID, 1, ebufp);
	if (vp) {
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_BILLINFO_ID, vp, ebufp);
	}

	/***********************************************************
	 * Get parent for subord
	 ***********************************************************/
	if (is_subord) {
		par_flistp = PIN_FLIST_SUBSTR_TAKE(i_flistp, 
					PIN_FLD_PARENT_INFO, 1, ebufp);
		if (!par_flistp) {
			fm_cust_billinfo_read_parent(ctxp, i_flistp, 
					&par_flistp, is_subord, ebufp);
		}
	}

	/***********************************************************
	 * Check the common billing info.
	 ***********************************************************/
	old_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLINFO, 
						PIN_ELEMID_OLD, 1, ebufp);
	fm_cust_pol_valid_billinfo_common(connp, new_flistp, old_flistp,
		par_flistp, res_flistp, is_creation, is_subord, ebufp);

	PIN_FLIST_DESTROY_EX(&par_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_billinfo error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo_common():
 *
 * Validate the billing info for which the rules are common to all types 
 *
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo_common(
	cm_nap_connection_t	*connp,
	pin_flist_t		*new_flistp,
	pin_flist_t		*old_flistp,
	pin_flist_t		*par_flistp,
	pin_flist_t		*res_flistp,
	int32			is_creation,
	int32			is_subord,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	poid_t		*new_par_pdp = NULL;
	poid_t		*old_par_pdp = NULL;
	int32		*resultp = NULL;
	int32		is_recursive = PIN_BOOLEAN_FALSE;
	int32		is_allow_change = PIN_BOOLEAN_FALSE;
	int32		field = PIN_FLD_BILLINFO;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/************************************************************ 
	 * Check if validation is provided from the recursive call to
	 * subordinate billinfo objects
	 ************************************************************/
	if (old_flistp) {
		vp = PIN_FLIST_FLD_GET(old_flistp, PIN_FLD_POID, 1, ebufp);
		if (!vp) {
			is_recursive = PIN_BOOLEAN_TRUE;
		}
	}

	/************************************************************
	 * Determine whether changes are allowed for subord.
	 * There are allowed in the following cases:
	 * 1. We are making subord (i.e. old pay_type is not subord)
	 * 2. We are moving subord (i.e. its parent is changed) 
	 * 3. We are creating billinfo
	 * 4. We are making recursive call
	 ************************************************************/
	if (!is_subord || is_creation || is_recursive) {
		is_allow_change = PIN_BOOLEAN_TRUE;
	} else {
		vp = PIN_FLIST_FLD_GET(old_flistp, PIN_FLD_PAY_TYPE, 0, ebufp);
		if (vp && (*(int32 *)vp != PIN_PAY_TYPE_SUBORD)) {
			is_allow_change = PIN_BOOLEAN_TRUE;
		}
		new_par_pdp = (poid_t *)PIN_FLIST_FLD_GET(new_flistp, 
					PIN_FLD_PARENT_BILLINFO_OBJ, 1, ebufp);
		old_par_pdp = (poid_t *)PIN_FLIST_FLD_GET(old_flistp, 
					PIN_FLD_PARENT_BILLINFO_OBJ, 0, ebufp);
		if (new_par_pdp &&
		    PIN_POID_COMPARE(new_par_pdp, old_par_pdp, 0, ebufp) != 0) {
			is_allow_change = PIN_BOOLEAN_TRUE;
		}
	}

	/************************************************************ 
	 * Validate billinfo fields
	 ************************************************************/
	fm_cust_pol_valid_billinfo_exist(ctxp, new_flistp, old_flistp, 
						res_flistp, is_creation, ebufp);
	fm_cust_pol_valid_billinfo_currency(ctxp, new_flistp, old_flistp,
		par_flistp, res_flistp, is_subord, ebufp);
	fm_cust_pol_valid_billinfo_billing_segment(ctxp, new_flistp, old_flistp, 
		par_flistp, res_flistp, is_allow_change, is_subord, ebufp);
	fm_cust_pol_valid_billinfo_dom(ctxp, new_flistp, old_flistp, 
		par_flistp, res_flistp, is_allow_change, is_subord, ebufp);
	fm_cust_pol_valid_billinfo_bill_when(ctxp, new_flistp, old_flistp,
		par_flistp, res_flistp, is_allow_change, is_subord, ebufp);
	fm_cust_pol_valid_billinfo_actg_type(ctxp, new_flistp, res_flistp, ebufp);
	fm_cust_pol_valid_billinfo_ar(ctxp, new_flistp, old_flistp, par_flistp,
						res_flistp, is_subord, ebufp);
	fm_cust_pol_valid_billinfo_acct(connp, new_flistp, par_flistp, 
						res_flistp, is_subord, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_billinfo_common error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo_currency():
 *
 * Validate currency
 *
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo_currency(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_subord,
	pin_errbuf_t	*ebufp)
{
	int32		curr = 0;
	int32		*currp = NULL;
	int32		*sec_currp = NULL;
	int32		*resultp = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	currp = (int32 *)PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_CURRENCY, 1, 
									ebufp);
	if (currp) {
		curr = *currp;
		/*******************************************************
		 * We will do a broad check here basically making sure 
		 * it is within the range Portal defines for currency
		 * balance elements. This can be customized to restrict 
		 * to the currencies defined in the pin.beid file.
		 *******************************************************/
		if (!(curr >= PIN_CURRENCY_VAL_MIN && 
		       curr <= PIN_CURRENCY_VAL_MAX)) {
			vp = (void *)"Invalid/unsupported currency";
			fm_cust_pol_valid_add_fail(res_flistp,
				PIN_FLD_CURRENCY, (int32)NULL,
				PIN_CUST_VAL_ERR_INVALID, 
				(char *)vp, (void *)&curr, ebufp);
		}

		/************************************************************
		* We don't allow to change currency for any billinfo
		*************************************************************/
		if (old_flistp) {
			vp = PIN_FLIST_FLD_GET(old_flistp, 
						PIN_FLD_CURRENCY, 0, ebufp);
			if (vp && (*((int32 *)vp) != curr)) {
				vp = (void *)"Currency can not be changed";
				fm_cust_pol_valid_add_fail(res_flistp,
						PIN_FLD_CURRENCY, (int32)NULL,
						PIN_CUST_VAL_ERR_INVALID,
						(char *)vp, (void *)&curr,
						ebufp);
			}
		}
	}

	/************************************************************ 
	 * Compare with parent
	 ************************************************************/
	if (par_flistp && is_subord) {
		if (!currp && old_flistp) {
			currp = (int32 *)PIN_FLIST_FLD_GET(old_flistp,
						PIN_FLD_CURRENCY, 0, ebufp);
		}
		vp = PIN_FLIST_FLD_GET(par_flistp, PIN_FLD_CURRENCY, 0, ebufp);
		if (vp && currp && (*((int32 *)vp) != *(int32 *)currp)) {
			vp = (void *)"Currency for parent and subord are not the same";
			fm_cust_pol_valid_add_fail(res_flistp,
					PIN_FLD_CURRENCY, (int32)NULL,
					PIN_ERR_CURRENCY_MISMATCH,
					(char *)vp, (void *)currp, ebufp);
		}
	}

	/************************************************************ 
	 * Validate secondary currency
	 ************************************************************/
	sec_currp = (int32 *)PIN_FLIST_FLD_GET(new_flistp, 
			PIN_FLD_CURRENCY_SECONDARY, 1, ebufp);
	if (sec_currp && currp) { 
		if (!psiu_currency_is_supported_combination(curr,
				*sec_currp, ebufp)) {
			vp = (void *)"Invalid secondary currency";
			fm_cust_pol_valid_add_fail(res_flistp,
				PIN_FLD_CURRENCY_SECONDARY, (int32)NULL,
				PIN_CUST_VAL_ERR_INVALID, 
				(char *)vp, (void *)sec_currp, ebufp);
		}
	}
		
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_billinfo_currency error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo_dom()
 *
 * Validate FUTURE_DOM
 *
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo_dom(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_allow_change,
	int32		is_subord,
	pin_errbuf_t	*ebufp)
{
	int32		*future_domp = NULL;
	int32		*cycle_domp = NULL;
	int32		*billing_segmentp = NULL;
	int32		billing_segment = 0;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
	
	/***************************************************************
	 * Get the Billing Segment
	 ***************************************************************/
	billing_segmentp = (int32 *)PIN_FLIST_FLD_GET(new_flistp, 
					PIN_FLD_BILLING_SEGMENT, 1, ebufp);
	if (billing_segmentp) {
		billing_segment = *billing_segmentp;
	}
	
	 /***************************************************************
	 * Get the Future DOM
	 ***************************************************************/	
	future_domp = (int32 *)PIN_FLIST_FLD_GET(new_flistp, 
					PIN_FLD_ACTG_FUTURE_DOM, 1, ebufp);
	if (future_domp && !is_allow_change) {
		vp = (void *)"Cycle DOM is not allowed to change for subord";
		fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_ACTG_FUTURE_DOM,
				(int32)NULL, PIN_CUST_VAL_ERR_INVALID, 
				(char *)vp, (void *)future_domp, ebufp);
		return;
	}

	/************************************************************
	 * Check domain of future_dom from config object
	 ************************************************************/
	if (future_domp) {
		fm_cust_pol_validate_fld_value(ctxp, new_flistp, new_flistp, 
				res_flistp,
				PIN_FLD_ACTG_FUTURE_DOM,
				0,	      /* element_id   */
				"Actg_cycle",
				0,
				ebufp);
	}

	/************************************************************
	 * Compare cycle_dom with parent
	 ************************************************************/
	cycle_domp = (int32 *)PIN_FLIST_FLD_GET(new_flistp, 
					PIN_FLD_ACTG_CYCLE_DOM, 1, ebufp);
	if (par_flistp && is_subord) {
		if (!cycle_domp && old_flistp) {
			cycle_domp = (int32 *)PIN_FLIST_FLD_GET(old_flistp,
					PIN_FLD_ACTG_CYCLE_DOM, 0, ebufp);
		}
		vp = PIN_FLIST_FLD_GET(par_flistp, PIN_FLD_ACTG_CYCLE_DOM, 0, 
									ebufp);
		if (vp && cycle_domp && (*(int32 *)vp != *(int32*)cycle_domp)) {
			vp = (void *)"Cycle DOM for parent and subord are not the same";
			fm_cust_pol_valid_add_fail(res_flistp,
				PIN_FLD_ACTG_CYCLE_DOM, 
				(int32)NULL, PIN_CUST_VAL_ERR_INVALID, 
				(char *)vp, (void *)cycle_domp, ebufp);
		}
	}

	/************************************************************
	 * Check domain of cycle_dom
	 ************************************************************/
	if (cycle_domp && !fm_utils_cycle_is_valid_dom(*cycle_domp, ebufp)) {
		vp = (void *)"Cycle DOM is invalid";
		fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_ACTG_CYCLE_DOM,
				(int32)NULL, PIN_CUST_VAL_ERR_INVALID, 
				(char *)vp, (void *)cycle_domp, ebufp);
	}

	/************************************************************
	 * If cached data structure is available then validate the future_dom
	 ************************************************************/
	if(cycle_domp && billing_segment) {
		fm_cust_pol_valid_billinfo_dom_from_cache(ctxp, res_flistp, 
					*cycle_domp, billing_segment, ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_billinfo_dom error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo_bill_when()
 *
 * Validate BILL_WHEN
 *
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo_bill_when(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_allow_change,
	int32		is_subord,
	pin_errbuf_t	*ebufp)
{
	int32		*bill_whenp = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
		
	bill_whenp = (int32 *)PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_BILL_WHEN, 
								1, ebufp);
	if (bill_whenp && !is_allow_change) {
		vp = (void *)"Billing cycle is not allowed to change for subord";
		fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_BILL_WHEN, 
				(int32)NULL, PIN_CUST_VAL_ERR_INVALID, 
				(char *)vp, (void *)bill_whenp, ebufp);
		return;
	}

	/************************************************************
	 * Compare with parent
	 ************************************************************/
	if (par_flistp && is_subord) {
		if (!bill_whenp && old_flistp) {
			bill_whenp = (int32 *)PIN_FLIST_FLD_GET(old_flistp,
						PIN_FLD_BILL_WHEN, 0, ebufp);
		}
		vp = PIN_FLIST_FLD_GET(par_flistp, PIN_FLD_BILL_WHEN, 0, ebufp);
		if (vp && bill_whenp && (*(int32 *)vp != *(int32 *)bill_whenp)){
			vp = (void *)"Billing cycle for parent and subord are not the same";
			fm_cust_pol_valid_add_fail(res_flistp,
				PIN_FLD_BILL_WHEN, (int32)NULL,
				PIN_CUST_VAL_ERR_INVALID, 
				(char *)vp, (void *)bill_whenp, ebufp);
		}
	}
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_billinfo_bill_when error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo_actg_type()
 *
 * Validates the accounting type of the billinfo using the
 * /config_fld_validate object.
 *
 * @param ctxp	     :		pcm context.
 * @param new_flistp :		new billinfo flist. 
 * @param res_flistp :		validation result flist. 
 * @param ebufp      :		error buffer.
 *
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo_actg_type(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*res_flistp,
	pin_errbuf_t	*ebufp)
{
	int32		*actg_typep = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
		
	actg_typep = (int32 *)PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_ACTG_TYPE, 
								1, ebufp);
        /***********************************************************
         * Try to validate based on /config/fld_validate
         * (Actg_cycle_type) obj.
         ***********************************************************/
        if (actg_typep) {
                fm_cust_pol_validate_fld_value( ctxp, new_flistp, new_flistp, 
			res_flistp, PIN_FLD_ACTG_TYPE,
                        0,              /* element_id   */
                        "Actg_cycle_type",
                        0,
                        ebufp);
        }

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_billinfo_actg_type error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo_exist()
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo_exist(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*res_flistp,
	int32		is_creation,
	pin_errbuf_t	*ebufp)
{
	int32		pay_type = PIN_PAY_TYPE_UNDEFINED;
	char		*err_msg = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
		 
	/************************************************************
	 * Check existence of PIN_FLD_PAYINFO_OBJ if PAY_TYPE changed
	 ************************************************************/
	vp = PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
	if (vp) {
		pay_type = *(int32 *)vp;
	}
	if (vp && old_flistp) {
		vp = PIN_FLIST_FLD_GET(old_flistp, PIN_FLD_PAY_TYPE, 0, ebufp);
		if (vp && (*(int32 *)vp != pay_type)) {
		/************************************************************
		* Check existence of PAYINFO_OBJ if PAY_TYPE is not equal to
		* UNDEFINED, SUBORD, PREPAID, and VOUCHER.
		************************************************************/
		if ((pay_type != PIN_PAY_TYPE_UNDEFINED) &&
			(pay_type != PIN_PAY_TYPE_SUBORD) &&
			(pay_type != PIN_PAY_TYPE_PREPAID) &&
			(pay_type != PIN_PAY_TYPE_VOUCHER)) {
				vp = PIN_FLIST_FLD_GET(new_flistp, 
						PIN_FLD_PAYINFO_OBJ, 1, ebufp);
				if (PIN_POID_IS_NULL((poid_t *)vp)) {
					err_msg = "PAYINFO_OBJ is not specified";
					fm_cust_pol_valid_add_fail(res_flistp, 
						PIN_FLD_PAYINFO_OBJ, 
						(int32)NULL,
						PIN_CUST_VAL_ERR_MISSING, 
						err_msg, vp, ebufp);
				}
			}
		}
	}

	/************************************************************
	 * Check existence fields during creation of billinfo
	 ************************************************************/
	if (!is_creation) {
		goto cleanup;
	}

	/************************************************************
	 * Check existence BILL_INFO_ID
	 ************************************************************/
	vp = PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_BILLINFO_ID, 1, ebufp);
	if (!vp) {
		err_msg = "BILLINFO_ID is not specified";
		fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_BILLINFO_ID, 
				(int32)NULL, PIN_CUST_VAL_ERR_MISSING, 
				err_msg, vp, ebufp);
	}

	/************************************************************
	 * Check existence PAY_TYPE and PARENT_BILLINFO_OBJ
	 ************************************************************/
	vp = PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
	if (!vp) {
		err_msg = "PAY_TYPE is not specified";
		fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_PAY_TYPE, 
				(int32)NULL, PIN_CUST_VAL_ERR_MISSING, 
				err_msg, vp, ebufp);
	} else if (*(int32 *)vp == PIN_PAY_TYPE_SUBORD) {
		vp = PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_PARENT_BILLINFO_OBJ,
								1, ebufp);
		if (PIN_POID_IS_NULL((poid_t *)vp)) {
			err_msg = "BILLINFO_OBJ for parent is not specified";
			fm_cust_pol_valid_add_fail(res_flistp, 
				PIN_FLD_PARENT_BILLINFO_OBJ, 
				(int32)NULL, PIN_CUST_VAL_ERR_MISSING, 
				err_msg, vp, ebufp);
		}
	}

	/************************************************************
	 * Check existence of PAYINFO_OBJ if PAY_TYPE is not equal to
	 * UNDEFINED, SUBORD, PREPAID, and VOUCHER.
	 ************************************************************/
	if ((pay_type != PIN_PAY_TYPE_UNDEFINED) &&
		(pay_type != PIN_PAY_TYPE_SUBORD) &&
		(pay_type != PIN_PAY_TYPE_PREPAID) &&
		(pay_type != PIN_PAY_TYPE_VOUCHER)) {
		vp = PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_PAYINFO_OBJ, 1, 
									ebufp);
		if (PIN_POID_IS_NULL((poid_t *)vp)) {
			err_msg = "PAYINFO_OBJ is not specified";
			fm_cust_pol_valid_add_fail(res_flistp, 
					PIN_FLD_PAYINFO_OBJ, 
					(int32)NULL, PIN_CUST_VAL_ERR_MISSING, 
					err_msg, vp, ebufp);
		}
	}

	/************************************************************
	 * Check existence CURRENCY
	 ************************************************************/
	vp = PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_CURRENCY, 1, ebufp);
	if (!vp) {
		err_msg = "CURRENCY is not specified";
		fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_CURRENCY, 
				(int32)NULL, PIN_CUST_VAL_ERR_MISSING, 
				err_msg, vp, ebufp);
	}

cleanup:

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_billinfo_exist error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo_ar()
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo_ar(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_subord,
	pin_errbuf_t	*ebufp)
{
	poid_t		*ar_pdp = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);
		 
	ar_pdp = (poid_t *)PIN_FLIST_FLD_GET(new_flistp, 
					PIN_FLD_AR_BILLINFO_OBJ, 1, ebufp);
	if (!ar_pdp) {
		return;
	}
	if (PIN_POID_IS_NULL(ar_pdp)) {
		vp = (void *)"AR_BILLINFO_OBJ is not specified";
		fm_cust_pol_valid_add_fail(res_flistp, 
				PIN_FLD_AR_BILLINFO_OBJ, 
				(int32)NULL, PIN_CUST_VAL_ERR_INVALID, 
				(char *)vp, (void *)ar_pdp, ebufp);
	} else {
		if (!is_subord) {
		/***********************************************************
		* Check that AR = POID
		************************************************************/
			vp = PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_POID, 1, 
									ebufp);
			if (vp && PIN_POID_COMPARE((poid_t *)vp, ar_pdp, 0,
								ebufp) != 0) {
					vp = (void *)"AR_BILLINFO is not the same as POID";
					fm_cust_pol_valid_add_fail(res_flistp, 
						PIN_FLD_AR_BILLINFO_OBJ, 
						(int32)NULL, 
						PIN_CUST_VAL_ERR_INVALID, 
						(char *)vp, (void *)ar_pdp, 
						ebufp);
			}
		} else { /* billinfo is subord */
		/***********************************************************
		* Check that AR = AR(parent)
		************************************************************/
			vp = NULL;
			if (par_flistp) {
				vp = PIN_FLIST_FLD_GET(par_flistp, 
					PIN_FLD_AR_BILLINFO_OBJ, 1, ebufp);
			}
			if (vp && PIN_POID_COMPARE((poid_t *)vp, ar_pdp, 0,
								ebufp) != 0) {
					vp = (void *)"AR_BILLINFO for parent and subord are not the same";
					fm_cust_pol_valid_add_fail(res_flistp, 
						PIN_FLD_AR_BILLINFO_OBJ, 
						(int32)NULL, 
						PIN_CUST_VAL_ERR_INVALID, 
						(char *)vp, (void *)ar_pdp, 
						ebufp);
			}
		}
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_billinfo_ar error", ebufp);
	}
	return;
}


/*******************************************************************
 * fm_cust_pol_valid_billinfo_acct()
 *
 * This provides the following validations for the subord:
 * 1. Check that account for subord and account for parent are different
 * 2. Check that account for subord and account for parent are in the
 *    child-parent relation
 * 3. Check that "AR" account is the same as for any (one) subord billinfo 
 *
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo_acct(
	cm_nap_connection_t	*connp,
	pin_flist_t		*new_flistp,
	pin_flist_t		*par_flistp,
	pin_flist_t		*res_flistp,
	int32			is_subord,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	pin_flist_t*	s_flistp = NULL;
	pin_flist_t*	r_flistp = NULL;
	pin_flist_t*	tmp_flistp = NULL;
	pin_flist_t*	tmp1_flistp = NULL;
	poid_t		*a_pdp = NULL;
	poid_t		*ap_pdp = NULL;
	poid_t		*ar_pdp = NULL;
	poid_t		*ari_pdp = NULL;
	poid_t		*b_ari_pdp = NULL;
	poid_t		*b_ar_pdp = NULL;
	poid_t		*bi_pdp = NULL;
	poid_t		*s_pdp = NULL;
	int32		pay_type = PIN_PAY_TYPE_SUBORD;
	int32		s_flags = 0;
	int32		count = 0;
	int32		el_id = 0;
	pin_cookie_t	cookie = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	if (!is_subord || !par_flistp)
		return;
		 
	/***********************************************************
	 * Check that account for subord and account for parent are different
	************************************************************/
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_ACCOUNT_OBJ, 1,
									ebufp);
	ap_pdp = (poid_t *)PIN_FLIST_FLD_GET(par_flistp, PIN_FLD_ACCOUNT_OBJ, 1,
									ebufp);
	if (a_pdp && ap_pdp && (PIN_POID_COMPARE(a_pdp, ap_pdp, 0, 
								ebufp) == 0)) {
		vp = (void *)"Account for parent and subord are the same";
		fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_ACCOUNT_OBJ, 
						(int32)NULL, 
						PIN_CUST_VAL_ERR_INVALID, 
						(char *)vp, (void *)a_pdp,
						ebufp);
	} 

	/***********************************************************
	 * Check existence of the group object if we are not calling 
	 * ourselves from the COMMIT_CUSTOMER opcode.
	 * Note that this opcode takes care about setting the parent-child
	 * relation between accounts
	************************************************************/
	if (a_pdp && ap_pdp &&
		!fm_utils_op_is_ancestor(connp->coip, PCM_OP_CUST_COMMIT_CUSTOMER)) {
		s_flistp = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp), "/search", -1, 
									ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, 
									ebufp);

		vp = (void *)"select X from /group/billing where ( F1 = V1 and F2 = V2 ) ";
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, vp, ebufp);

		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, 
									ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PARENT, (void *)ap_pdp, 
									ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, 
									ebufp);
		tmp1_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_MEMBERS, 1,
									ebufp);
		PIN_FLIST_FLD_SET(tmp1_flistp, PIN_FLD_OBJECT, (void *)a_pdp,
									ebufp);

		PIN_FLIST_ELEM_SET(s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

		PCM_OP(ctxp, PCM_OP_SEARCH,
			PCM_OPFLG_COUNT_ONLY | PCM_OPFLG_READ_UNCOMMITTED, 
						s_flistp, &r_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS, &count, 1, 
								&cookie, ebufp);
		if (count == 0) {
			vp = (void *)"Accounts for subord and parent billinfo are not in the child-parent relation";
			fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_PARENT,
						(int32)NULL,
						PIN_CUST_VAL_ERR_INVALID,
						(char *)vp, (void *)ap_pdp,
						ebufp);
		}
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	}

	if (!a_pdp) {
		goto cleanup;
	}

	/***********************************************************
	 * Search for subords 
	************************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp), "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)a_pdp,ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PAY_TYPE, (void *)&pay_type,
									ebufp);

	bi_pdp  = (poid_t *)PIN_FLIST_FLD_GET(new_flistp, PIN_FLD_POID, 0, 
									ebufp);
	if (PIN_POID_GET_ID(bi_pdp) > 0) {
		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, 
									ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, (void *)bi_pdp, 
									ebufp);
		vp = (void *)"select X from /billinfo where ( F1 = V1 and F2 = V2 and F3 != V3 ) ";
	} else {
		vp = (void *)"select X from /billinfo where ( F1 = V1 and F2 = V2 ) ";
	}
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, vp, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_AR_BILLINFO_OBJ, NULL, ebufp);

	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_READ_UNCOMMITTED | PCM_OPFLG_CACHEABLE, 
						s_flistp, &r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

	cookie = NULL;
	if (!(tmp_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS,
						&el_id, 1, &cookie, ebufp))) {
		goto cleanup;
	}

	/***********************************************************
	 * Compare AR_BILLINFO 
	************************************************************/
	b_ar_pdp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_AR_BILLINFO_OBJ, 0, 
									ebufp);
	b_ari_pdp = PIN_FLIST_FLD_GET(par_flistp, PIN_FLD_AR_BILLINFO_OBJ, 0, 
									ebufp);
	if (b_ar_pdp && b_ari_pdp && 
			PIN_POID_COMPARE(b_ar_pdp, b_ari_pdp, 0, ebufp) == 0) {
		goto cleanup;
	}

cleanup:

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_POID_DESTROY(ar_pdp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_billinfo_acct error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo_billing_segment()
 *
 * Validate BILLING_SEGMENT
 *
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo_billing_segment(
	pcm_context_t	*ctxp,
	pin_flist_t	*new_flistp,
	pin_flist_t	*old_flistp,
	pin_flist_t	*par_flistp,
	pin_flist_t	*res_flistp,
	int32		is_allow_change,
	int32		is_subord,
	pin_errbuf_t	*ebufp)
{
	int32		*billing_segmentp = NULL;
	int32		billing_segment = 0;
	int32		err = 0;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/************************************************************
	 * Get the Billing Segment from the input flist
	 ************************************************************/
	billing_segmentp = (int32 *)PIN_FLIST_FLD_GET(new_flistp,
					PIN_FLD_BILLING_SEGMENT, 1, ebufp);
	if (billing_segmentp) {
		billing_segment = *billing_segmentp;
	
		/************************************************************
		* Validate using the cached flist. If cached flist is not 
		* available check for default value i.e; 0. If not default
		* then error
		************************************************************/
		if(billing_segment) {
			fm_cust_pol_validate_billing_segment_from_cache(ctxp, res_flistp, 
					 billing_segment, ebufp);
		}
		else {
			if(billing_segment != 0) {
				vp = (void *)"Billing Segment is not valid";
				fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_BILLING_SEGMENT,
					(int32)NULL, PIN_CUST_VAL_ERR_INVALID,
					(char *)vp, (void *)billing_segmentp, ebufp);
					return;
			}
		}
	}	

	if (billing_segmentp && !is_allow_change) {
		vp = (void *)"Billing Segment is not allowed to change for subord";
		fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_BILLING_SEGMENT,
				(int32)NULL, PIN_CUST_VAL_ERR_INVALID,
				(char *)vp, (void *)billing_segmentp, ebufp);
		return;
	}

	/************************************************************
	 * Compare billing_segment with parent
	 ************************************************************/
	if (par_flistp && is_subord) {
		if (!billing_segmentp && old_flistp) {
			billing_segmentp = (int32 *)PIN_FLIST_FLD_GET(old_flistp,
					PIN_FLD_BILLING_SEGMENT, 0, ebufp);
		}
		vp = PIN_FLIST_FLD_GET(par_flistp, PIN_FLD_BILLING_SEGMENT, 0,
									ebufp);
		if (vp && billing_segmentp && (*(int32 *)vp != *(int32*)billing_segmentp)) {
			vp = (void *)"BILLING_SEGMENT for parent and subord are not the same";
			fm_cust_pol_valid_add_fail(res_flistp,
				PIN_FLD_BILLING_SEGMENT,
				(int32)NULL, PIN_CUST_VAL_ERR_INVALID,
				(char *)vp, (void *)billing_segmentp, ebufp);
		}
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_billinfo_billing_segment error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo_billing_segment_from_cache()
 *
 * Validate BILLING_SEGMENT from cache
 *
 *******************************************************************/
static void
fm_cust_pol_validate_billing_segment_from_cache(
	pcm_context_t   *ctxp,
	pin_flist_t	*res_flistp,
	int32		billing_segment,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*cache_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	poid_t		*pdp = NULL;
	void		*vp = NULL;
	int64		database;
	int32		err = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	pdp = PCM_GET_USERID(ctxp);
        database = PIN_POID_GET_DB(pdp);
        PIN_POID_DESTROY(pdp, NULL);

	/***************************************************************
	* Get the config object data from cache 
	***************************************************************/

	cache_flistp = fm_cust_utils_get_config_billing_segment_cache(ctxp, database, ebufp);

	if(cache_flistp) {
		/*********************************************************************
		* Get the element in the Billing Segment Array at the given index
		* as Billing Segment acts as an enum value for the array index
		* If element not found then error
		*********************************************************************/
		flistp = PIN_FLIST_ELEM_GET(cache_flistp, PIN_FLD_BILLING_SEGMENTS,
			billing_segment, 1, ebufp);

		if (!flistp) {
			vp = (void *)"BILLING_SEGMENT is not available";
			fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_BILLING_SEGMENT,
				(int32)NULL, PIN_CUST_VAL_ERR_INVALID, 
				(char *)vp, (void *)&billing_segment, ebufp);
		}
	}
	else {
		if(billing_segment != 0) {
			vp = (void *)"BILLING_SEGMENT is not valid";
                        fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_BILLING_SEGMENT,
                                (int32)NULL, PIN_CUST_VAL_ERR_INVALID,
                                (char *)vp, (void *)&billing_segment, ebufp);
		}
	}

	PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_billinfo_dom_from_cache()
 *
 * Validate FUTURE_DOM from cache
 *
 *	 --------------------------------------------------
 *	| BS                 DOM           Status          |
 *	|--------------------------------------------------|
 *	| 101                Unassigned    N/A             |
 *	| 102                1             Open            |
 *	| 102                2             Restricted      |
 *	| 102                3             Closed          |
 *	 --------------------------------------------------
 *
 * From the above table, if a billinfo A has billing segment(BS) 101, 
 * then any DOM could be assigned for the billinfo. Also, A can 
 * be updated with any other DOM later.
 *
 * If a billinfo B has billing segment 102 and initial DOM 
 * assignment was 1 let's say. Then B cannot be assigned any of 
 * the following DOMs (3, 4 to 31) eventhough 4 to 31 is not 
 * explicitly closed.
 *******************************************************************/
static void
fm_cust_pol_valid_billinfo_dom_from_cache(
	pcm_context_t   *ctxp,
	pin_flist_t	*res_flistp,
	int32		future_dom,
	int32		billing_segment,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*cache_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	pin_cookie_t	cookie = NULL;  
	poid_t		*pdp = NULL;
	void		*vp = NULL;
	int32		recid = 0;
	int32		err = 0;
	int64		database;
	int32		match_found = 0;
	int32		combo_available = 0;
	
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	pdp = PCM_GET_USERID(ctxp);
        database = PIN_POID_GET_DB(pdp);
        PIN_POID_DESTROY(pdp, NULL);

	/***************************************************************
	* Get the config object data from cache 
	***************************************************************/

	cache_flistp = fm_cust_utils_get_config_dom_map_cache(ctxp, database, ebufp);
	
	if(cache_flistp) {
		while((flistp = PIN_FLIST_ELEM_GET_NEXT(cache_flistp, PIN_FLD_MAP, 
				&recid, 1, &cookie, ebufp)) != (pin_flist_t *) NULL) {

			vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_BILLING_SEGMENT, 0, ebufp);
			if(vp && *(int32 *)vp  == billing_segment) 
			{
				if (vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_ACTG_CYCLE_DOM, 0, ebufp))
				{
					/* At least one BS<-->DOM combination exist */
					combo_available = 1;
					if (*(int32 *)vp == future_dom)
					{
						match_found = 1;

						/* Check if this DOM is available */
						vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_STATUS, 0, ebufp);
						if(vp && *(int32 *)vp == PIN_ACTG_DOM_STATUS_CLOSE) {
							vp = (void *)"Cycle DOM Status is CLOSE";
							fm_cust_pol_valid_add_fail(res_flistp, 
								PIN_FLD_ACTG_FUTURE_DOM, (int32)NULL, 
								PIN_CUST_VAL_ERR_INVALID,(char *)vp, 
								(void *)&future_dom, ebufp);
						}
						break;
					}
				}
			}
		}

		/* BS  mapped to at least one DOM. But an exact BS<-->Future DOM match not found */
		if (combo_available == 1 && match_found == 0)
		{
			vp = (void *)"Cycle DOM is not available";
			fm_cust_pol_valid_add_fail(res_flistp, PIN_FLD_ACTG_FUTURE_DOM, (int32)NULL,
				PIN_CUST_VAL_ERR_INVALID,(char *)vp,
				(void *)&future_dom, ebufp);
		}
	}
	
	PIN_FLIST_DESTROY_EX(&cache_flistp, NULL);
	return;
}

