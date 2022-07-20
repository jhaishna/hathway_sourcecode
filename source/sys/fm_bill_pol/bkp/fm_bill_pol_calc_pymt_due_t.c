/*
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_bill_pol_calc_pymt_due_t.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:32:43 nishahan Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pcm.h"
#include "ops/bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_bill.h"
#include "pin_pymt.h"
#include "pinlog.h"
#include "pin_os.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "mso_ntf.h"
#include "mso_ops_flds.h"
// TERM15D Added for MSO Customization 
#define TERM15D 101
#define TERM13D 102
#define TERM0D 103
#define	TERM21D 104

enum {DEFAULT = 0, TERM1, TERM2, TERM3};
enum {SUNDAY = 0, SATURDAY=6};

/**** Functions defined elsewhere ******/
extern time_t fm_utils_cycle_actgfuturet();
extern void fm_utils_get_calendar();

EXPORT_OP void
op_bill_pol_calc_pymt_due_t(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_bill_pol_calc_pymt_due_t(
	cm_nap_connection_t     *connp,
	pcm_context_t	*ctxp,
	int32		flags,
	pin_flist_t	*in_flistp,
	pin_flist_t	**out_flistpp,
	pin_errbuf_t	*ebufp);

static void 
fm_bill_pol_default_calc_due_t(
	time_t          *due_tp,
	time_t          *eff_tp,
	time_t          r_due_t,
	int             due_dom,
	pin_errbuf_t    *ebufp);

static void
fm_bill_pol_get_nthweekdayofmonth(
	int 	dow,
	int 	week,
	time_t *a_time,
	pin_errbuf_t    *ebufp);

static void
fm_bill_pol_add_n_bus_days(
	pcm_context_t	*ctxp,
	int 		n_bus_days,
	char 		*cal_name,
	time_t 		*a_time,
	pin_errbuf_t    *ebufp);

static void
fm_bill_pol_get_non_billing_days(
	pcm_context_t	*ctxp,
	int		year,
	char 		*cal_name,
	time_t 		**calendarpp,
	pin_errbuf_t    *ebufp);

static void 
fm_bill_pol_update_collect_date(
        pcm_context_t   *ctxp,
	int32		flags,
        pin_flist_t     *i_flistp,
	time_t          *eff_tp,
        time_t          due_t,
        pin_errbuf_t    *ebufp);

static void 
fm_bill_pol_bill_notification(
	cm_nap_connection_t     *connp,
	pcm_context_t           *ctxp,
        u_int                   flags,
        pin_flist_t             *in_flistp,
	time_t          	*due_t,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp);

static void fm_mso_billing_notification(
        pcm_context_t           *ctxp,
        poid_t                          *bill_pdp,
        poid_t                          *svc_pdp,
	time_t                  *due_t,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_BILL_POL_CALC_PYMT_DUE_T opcode
 *******************************************************************/
void
op_bill_pol_calc_pymt_due_t(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	void			*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	*ret_flistpp = NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_BILL_POL_CALC_PYMT_DUE_T)
	{
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_bill_pol_calc_pymt_due", ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_calc_pymt_due_t: Input flist", i_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_bill_pol_calc_pymt_due_t(connp, ctxp, flags, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (! PIN_ERR_IS_ERR(ebufp)) 
	{
		*ret_flistpp = PIN_FLIST_CREATE(ebufp);
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_POID, vp, ebufp);
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_BILLINFO_OBJ, vp, ebufp);
		vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_DUE_T, 0, ebufp);
		PIN_FLIST_FLD_SET(*ret_flistpp, PIN_FLD_DUE_T, vp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_bill_pol_calc_pymt_due_t: Output flist", *ret_flistpp);
	}

	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	return;
}

/*******************************************************************
 * fm_bill_pol_calc_pymt_due_t()
 *******************************************************************/
static void
fm_bill_pol_calc_pymt_due_t(
	cm_nap_connection_t     *connp,
	pcm_context_t	*ctxp,
	int32		flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	**out_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*pi_i_flistp = NULL;
	pin_flist_t	*pi_o_flistp = NULL;
	pin_flist_t	*bi_i_flistp = NULL;
	pin_flist_t	*bi_o_flistp = NULL;
	pin_flist_t     *t_flistp = NULL;
	pin_flist_t     *r_flistp = NULL;		
	poid_t		*bi_pdp = NULL;
	poid_t		*acc_pdp = NULL;
	poid_t		*pi_pdp = NULL;
	void		*vp = NULL;
	time_t		due_t = (time_t)0;
	time_t		r_due_t = (time_t)0;
	time_t		*eff_tp = (time_t *)NULL;
	u_int		ptt;
	int32		due_date_adjust = 0;
	int32           *pay_typep = NULL;		
	int		n_days = 0;
	int		due_dom = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);	

	/***********************************************************
	* Create outgoing flist
	***********************************************************/
	*out_flistpp = PIN_FLIST_CREATE(ebufp);

	acc_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	if (!acc_pdp)
	{
		PIN_FLIST_LOG_ERR("fm_bill_pol_calc_pymt_due_t: "
			"Mandatory value Account Poid missing in the input flist",
			ebufp);
		return;
	}

	bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	if (!bi_pdp)
	{
		PIN_FLIST_LOG_ERR("fm_bill_pol_calc_pymt_due_t: "
			"Mandatory value Billinfo Poid missing in the input flist",
			ebufp);
		return;
	}

	/***********************************************************
	* Get the end date of the billing cycle. This date will be
	* used in conjunction with the payment term to calculate the
	* the payment due date.
	***********************************************************/
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);
	if (!vp)
	{
		PIN_FLIST_LOG_ERR("fm_bill_pol_calc_pymt_due_t: "
		"Bill cycle end date missing in flist error", ebufp);
		return;
	}
	else 
	{
		eff_tp = ((time_t *)vp);
		
		/* If a whole day is not passed, get back to the 00:00 hrs of the day */
		if (!((*eff_tp) % 86400)) {
			*eff_tp = ((*eff_tp) / 86400) * 86400;
		}
	}

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTG_LAST_T, 0, ebufp);
	if (!vp)
	{
		PIN_FLIST_LOG_ERR("fm_bill_pol_calc_pymt_due_t: "
		"Bill cycle PIN_FLD_ACTG_LAST_T missing in flist error", ebufp);
		return;

	}
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DUE_DATE_ADJUSTMENT , 1, ebufp);
	if (vp) {
		due_date_adjust = *((int32 *)vp);
	}

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_RELATIVE_DUE_T, 1, ebufp);
	if (vp) {
		r_due_t = *(time_t *) vp;
	}

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DUE_DOM, 1, ebufp);
	if (vp) {
		due_dom = *(int *) vp;
	}

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PAYMENT_TERM, 1, ebufp);

	if (!vp)
	{
		/***********************************************************
		* Get the payinfo object for a given billinfo object, if not
		* provided in the input flist
		***********************************************************/
		bi_i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(bi_i_flistp, PIN_FLD_POID, (void *) bi_pdp, ebufp);
		PIN_FLIST_FLD_SET(bi_i_flistp, PIN_FLD_PAYINFO_OBJ, (void *) NULL, ebufp);
		PIN_FLIST_FLD_SET(bi_i_flistp, PIN_FLD_PAY_TYPE,(void *) NULL, ebufp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, bi_i_flistp, &bi_o_flistp, ebufp);
		if (PIN_ERR_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_bill_pol_calc_pymt_due_t: "
				"Error while executing opcode PCM_OP_READ_FLDS", 
				ebufp);
		}

		pi_pdp = (poid_t *)PIN_FLIST_FLD_GET(bi_o_flistp, PIN_FLD_PAYINFO_OBJ, 1, 
							ebufp);
		pay_typep = (int32 *)PIN_FLIST_FLD_GET(bi_o_flistp, PIN_FLD_PAY_TYPE, 1,
                 ebufp);
		/*************************************************************
		 * If pay_type is subord then please get the Payment Term from
		 * payinfo object of the parent.
		 ************************************************************/

		if (pay_typep && (*pay_typep == PIN_PAY_TYPE_SUBORD)) {
                
		          t_flistp = PIN_FLIST_CREATE(ebufp);
                          vp = PIN_FLIST_FLD_GET(i_flistp,
                          PIN_FLD_AR_BILLINFO_OBJ, 0, ebufp);
                          PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, vp, ebufp);
                          PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_PAYINFO_OBJ, NULL,
                                    ebufp);
                          PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE,
                                    t_flistp, &r_flistp, ebufp);
                          PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
                          pi_pdp = PIN_FLIST_FLD_GET(r_flistp,
                          PIN_FLD_PAYINFO_OBJ, 0,ebufp);
			  PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
                }

		/***********************************************************
		* Read the payment term for the given payinfo POID.
		***********************************************************/
		pi_i_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(pi_i_flistp, PIN_FLD_POID, 
							(void *) pi_pdp, ebufp);
		PIN_FLIST_FLD_SET(pi_i_flistp, PIN_FLD_PAYMENT_TERM, 
							(void *) NULL, ebufp);
		PIN_FLIST_FLD_SET(pi_i_flistp, PIN_FLD_PAYMENT_OFFSET, 
							(void *) NULL, ebufp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, pi_i_flistp,
							&pi_o_flistp, ebufp);
		if (PIN_ERR_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_bill_pol_calc_pymt_due_t: "
				"Error while executing opcode PCM_OP_READ_FLDS", 
				ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, 
				"Switching to default Payment Term");
                        ptt = 0;
		}
		else
		{
			vp = PIN_FLIST_FLD_GET(pi_o_flistp, PIN_FLD_PAYMENT_TERM, 1, ebufp);
			if (!vp)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, 
					"Switching to default Payment Term");
				ptt = 0;
			}
			else {
				ptt = *(u_int *) vp;
			}
		}
	}
	else {
		ptt = *(u_int *) vp;
	}

	/***********************************************************
	 * Check if the payment term is a default one. If so, do the
	 * calculation as done in the earlier version of the code
	 * (by using either the relative_due_t or due_dom. Else if
	 * the term is NOT default, do the payment term calculation as
	 * per your requirement. Sample implementation for the payment
	 * terms using
	 * 17 DAYS,
	 * 14 Business DAYS,
	 * 3rd Tuesday of Next month
	 * are given below.
	 * Finally, if an adjustment (in days) to the due date is
	 * provided, ADD that to the calculated due date.
	 ***********************************************************/
	 due_t = *eff_tp;
	 switch (ptt)
	 {
		case TERM1: /* Add 17 days to the bill cycle end date */
			fm_utils_add_n_days(17, &due_t);
			break;

		case TERM2: /* Add 14 business days to the bill cycle end date */
			fm_bill_pol_add_n_bus_days(ctxp, 14, "default", &due_t, ebufp);
			if (PIN_ERR_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_bill_pol_calc_pymt_due_t: "
					"Error while using Payment Term 2", ebufp);
				goto cleanup;
			}
			/***********************************************************
			* If due_t is not changed, it only will mean that the there 
			* are no /config/calendar objects available during CM init.
			* In this case, use the default implementation.
			***********************************************************/
			if (due_t == *eff_tp)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING, 
					"Switching to default calculation");
				fm_bill_pol_default_calc_due_t(&due_t, eff_tp, 
					r_due_t, due_dom, ebufp);
			}

			break;

		case TERM3: /* Make the due date as 3rd Tuesday of the month */
			fm_bill_pol_get_nthweekdayofmonth( 2, 3, &due_t, ebufp);
			if (PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_bill_pol_calc_pymt_due_t: "
					"Error while getting nth weekday of month", 
					ebufp);
				goto cleanup;
			}
			break;
		//MSO Customization - Start
		case TERM15D: /* Add 15 days to the bill cycle end date */
			fm_utils_add_n_days(15, &due_t);
			break;
		case TERM13D: /* Add 13 days to the postpaid bill cycle end date for BB*/
			fm_utils_add_n_days(13, &due_t);
			break;
		case TERM21D:
			PIN_ERR_LOG_MSG(3,"21 days case");
			fm_utils_add_n_days(21, &due_t);
                         break;
		case TERM0D: /* Add 0 days to the prepaid bill cycle end date for BB*/
			fm_utils_add_n_days(0, &due_t);
			break;
		//MSO Customization - End
		/************************************************************
		* Default Implementation
		*
		* 86400 = 60 * 60 * 24 (1day)
		* If whole day is passed(not 1 day 2 hrs), then use the
		* fm_utils_add_n_days to correctly calculate the due_t.
		************************************************************/
		default:
			//fm_bill_pol_default_calc_due_t(&due_t, eff_tp, r_due_t, 
		//		due_dom, ebufp);
			fm_utils_add_n_days(15, &due_t);		
			if (PIN_ERR_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_bill_pol_calc_pymt_due_t: "
					"Error while using Default Payment Term", ebufp);
				goto cleanup;
			}
	}
	if (due_date_adjust) {
		fm_utils_add_n_days(due_date_adjust, &due_t);
	}

	due_t = fm_utils_time_round_to_midnight(due_t);

	/************************************************************
	* Set the PIN_FLD_DUE_T field in the config flist.
	************************************************************/
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, (void *)acc_pdp, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_BILLINFO_OBJ, (void *)bi_pdp, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_DUE_T, (void *)&due_t, ebufp);

	/*************** Setting Collection Date ************************/

	fm_bill_pol_update_collect_date(ctxp, flags, i_flistp, eff_tp, 
		due_t, ebufp);
	

	//fm_bill_pol_bill_notification(connp, ctxp, flags, i_flistp, &due_t, out_flistpp, ebufp);


	/*********************************************************
	* Cleanup...
	*********************************************************/
	cleanup:

	PIN_FLIST_DESTROY_EX(&bi_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bi_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pi_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pi_o_flistp, NULL);

	/*********************************************************
        * Errors..?
        *********************************************************/
        if (PIN_ERR_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_bill_pol_calc_pymt_due_t error", ebufp);
        }
	return;
}

static void fm_bill_pol_update_collect_date(
	pcm_context_t	*ctxp,
	int32		flags,
        pin_flist_t     *i_flistp,
        time_t          *eff_tp,
        time_t          due_t,
	pin_errbuf_t    *ebufp)
{

	pin_flist_t	*pi_i_flistp = NULL;
	pin_flist_t	*pi_o_flistp = NULL;
	pin_flist_t	*bi_i_flistp = NULL;
	pin_flist_t	*bi_o_flistp = NULL;
	poid_t		*pi_pdp = NULL;
	void		*vp = NULL;

	pin_flist_t     *bi_flistp = NULL;
	pin_flist_t     *r_flistp = NULL;
	poid_t		*bi_pdp = NULL;
	int32           opcode = 0;
	time_t          collect_t = (time_t)0;
	time_t          *collect_tp = (time_t *)NULL; 	
	int		ndays = 0;
	int		offset = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }

	PIN_ERR_CLEAR_ERR(ebufp);


	bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	if (!bi_pdp)
	{
		PIN_FLIST_LOG_ERR("fm_bill_pol_update_collect_date: "
			"Mandatory value Billinfo Poid missing in the input flist",
			ebufp);
		return;
	}

	/***********************************************************
	 * Get the payinfo object for a given billinfo object
	 ***********************************************************/
	bi_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(bi_i_flistp, PIN_FLD_POID, (void *) bi_pdp, ebufp);
	PIN_FLIST_FLD_SET(bi_i_flistp, PIN_FLD_PAYINFO_OBJ, (void *) NULL, 
								ebufp);
	PIN_FLIST_FLD_SET(bi_i_flistp, PIN_FLD_COLLECTION_DATE, (void *) NULL,
                						ebufp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, bi_i_flistp, &bi_o_flistp, ebufp);

	pi_pdp = (poid_t *)PIN_FLIST_FLD_GET(bi_o_flistp, PIN_FLD_PAYINFO_OBJ, 1,
									ebufp);
	collect_tp = (time_t *)PIN_FLIST_FLD_GET(bi_o_flistp, PIN_FLD_COLLECTION_DATE, 1,
                        						ebufp);
	/*********************************************************************
	 *Non paying child has null payinfo poid. Ignore for non paying child.
	 *********************************************************************/
	if (PIN_POID_IS_NULL(pi_pdp)||(PIN_POID_GET_ID(pi_pdp) < 0)) {
		/****** No need to log message ********/
		PIN_FLIST_DESTROY_EX(&bi_i_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&bi_o_flistp, NULL);
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_FLIST_LOG_ERR("fm_bill_pol_update_collect_date ",
                        	ebufp);
		}
		return;
		/*******************************/
	}

	/*******************************************************
	 * Read the Offset value for the given payinfo POID.
	 ***********************************************************/
	pi_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(pi_i_flistp, PIN_FLD_POID, (void *) pi_pdp, ebufp);
	PIN_FLIST_FLD_SET(pi_i_flistp, PIN_FLD_PAYMENT_OFFSET, (void *) NULL, 
		ebufp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, pi_i_flistp, &pi_o_flistp, ebufp);

	vp = PIN_FLIST_FLD_GET(pi_o_flistp, PIN_FLD_PAYMENT_OFFSET, 1, ebufp);
	if (!vp)
	{
		offset = -1;
	} else {
		offset = *(u_int *) vp;
	}


	/****** Check offset value **************************
	 * if offset value is -1 , collection on bill_creation_date
	 * if offset value is 0 , collection date date will be due date
	 * if offset value is xx, collection date xx days before due date
	 ****************************************************/
	if (offset < 0){
                  collect_t = *eff_tp;
	}
	else if (offset == 0) {
		collect_t = due_t;
	}
	else {
		collect_t = due_t;
		ndays = -(offset);
		fm_utils_add_n_days(ndays, &collect_t);
		if (collect_t < *eff_tp) {
			collect_t = *eff_tp;
		}
	}
	
	if(!(collect_tp && (*collect_tp > *eff_tp) && (*collect_tp < collect_t ))) {
		bi_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_POID, (void *) bi_pdp, ebufp);
		PIN_FLIST_FLD_SET(bi_flistp, PIN_FLD_COLLECTION_DATE, 
			(void *)&collect_t, ebufp);
		opcode = PCM_OP_WRITE_FLDS;
		PCM_OP(ctxp, opcode, 0, bi_flistp, &r_flistp, ebufp);
	}
	/*
	 * Clean Up.
	 */
	PIN_FLIST_DESTROY_EX(&bi_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bi_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bi_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pi_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&pi_o_flistp, NULL);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_update_collect_date error", ebufp);
	}
}

/************************************************************
* fm_bill_pol_default_calc_due_t: Default Implementation for
* Calculating payment due date.
*
* 86400 = 60 * 60 * 24 (1day)
* If whole day is passed(not 1 day 2 hrs), then use the
* fm_utils_add_n_days to correctly calculate the due_t.
************************************************************/
static void
fm_bill_pol_default_calc_due_t(
	time_t          *due_tp,
	time_t          *eff_tp,
	time_t          r_due_t,
	int             due_dom,
	pin_errbuf_t    *ebufp)
{
	int  n_days = 0;

	if (!(r_due_t % 86400))
	{
		n_days = (int)(r_due_t / 86400);
		*due_tp = *eff_tp;
		fm_utils_add_n_days(n_days, due_tp);
	}
	else
		*due_tp = *eff_tp + r_due_t;

	if (due_dom != 0)
		*due_tp = fm_utils_cycle_actgfuturet(due_dom, 0, *due_tp, ebufp);

}

/******************************************************************************
 * fm_bill_pol_compare_dates()
 * Helper function that compares 2 dates. Dates on in time_t (derivative of integer
 * format) so it can be numerically compared. It is an helper function for 
 * qsorting an arrary of dates in ascending order. If the first parameter is
 * greater than second, then first (date) value falls after second (date).
 * Else if first is les than second, second (date) value falls after first (date).
 * Otherwise, they are the same.
 ******************************************************************************/
int fm_bill_pol_compare_dates(const void *p1, const void *p2)
{
	time_t i = *((time_t *)p1);
	time_t j = *((time_t *)p2);

	if (i > j) {
		return (1);
	}
	if (i < j){
		return (-1);
	}
	return (0);
}

/******************************************************************************
 * fm_bill_pol_check_duplicate_date()
 * This function checks if there are any duplicate dates in given calendar array
 * If the date already exists, then it returns 1, otherwise zero.
******************************************************************************/
int fm_bill_pol_check_duplicate_date(struct tm tm, time_t **calendarpp, int added_so_far)
{
	time_t tmp_time;
	int j = 0;

	tmp_time = mktime(&tm);
	
	/***************************************************
	 * check if a date is duplicate of a recurring date 
	 * if so, do not store the duplicate value.
	***************************************************/
	for (j=0; j< added_so_far; j ++)
	{
		if ((*calendarpp)[j] == tmp_time) {
			return 1;
		}
	}
	return 0;
}


/******************************************************************************
 * fm_bill_pol_get_non_billing_days()
 *
 * Fill the calendar array with list of non-billing days. Though the object 
 * contain the dates in DD MM YYYY|0 format, the dates will be converted to
 * (time_t) time in seconds since the Epoch (00:00:00 UTC, January 1, 1970).
 * This fills in the array for 2 consecutive years starting form a given year
 ******************************************************************************/
static void
fm_bill_pol_get_non_billing_days(
	pcm_context_t	*ctxp,
	int		year,
	char		*cal_name,
	time_t		**calendarpp,
	pin_errbuf_t	*ebufp)
{
	struct tm 	tm;
	pin_flist_t 	*cfg_flistp = NULL;
	pin_flist_t 	*date_flistp = NULL;
	pin_cookie_t	cookie = NULL;
	int 		i = 0;
	int 		day_of_week = 0;
	int		non_bday_count = 0;
	int 		elem_count = 0;
	int32		err = 0;
	int32		rec_id = 0;
	int32 		cal_month = -1;
	int32 		cal_day = -1;
	int32 		cal_year = -1;
	void *		vp = NULL;

        fm_utils_get_calendar(cal_name, &cfg_flistp, ebufp);

	/****************************************************************************** 
	 * Allocate memory assuming the worst case (all non-billing days are recurring) 
	 * for the previous year, current year and the next 
	*******************************************************************************/
	elem_count = PIN_FLIST_ELEM_COUNT(cfg_flistp, PIN_FLD_CALENDAR_DATE, ebufp);
	*calendarpp = (time_t *) malloc (3 * elem_count * (sizeof (time_t)));

	if (*calendarpp == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_get_non_billing_days: memory error", ebufp);
		goto cleanup;
	}
	/* Get user-defined non-billing days and update the calendar array */
	while ((date_flistp = PIN_FLIST_ELEM_GET_NEXT(cfg_flistp, PIN_FLD_CALENDAR_DATE, 
		&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		vp = PIN_FLIST_FLD_GET(date_flistp, PIN_FLD_CALENDAR_DOM, 
					0, ebufp);
		if (vp) {
			cal_day = *(int *) vp;
		}

		vp = PIN_FLIST_FLD_GET(date_flistp, PIN_FLD_CALENDAR_MOY, 
					0, ebufp);
		if (vp) {
			cal_month = *(int *) vp;
		}

		vp = PIN_FLIST_FLD_GET(date_flistp, PIN_FLD_CALENDAR_YEAR, 
					0, ebufp);
		if (vp) {
			cal_year = *(int *) vp;
		}
		
		if (cal_day == -1 || cal_month == -1 || cal_year == -1)
		{
			free(*calendarpp);
			*calendarpp = NULL;
			goto cleanup;
		}

		tm.tm_mon = cal_month - 1;
		tm.tm_mday = cal_day;
		tm.tm_sec = 0;
		tm.tm_min = 0;
		tm.tm_hour = 0;
		tm.tm_isdst = -1;
		
		/*************************************
		 * If it is recurring non-billing day
		*************************************/
		if (cal_year == 0) /* Recurring non-billing day */
		{
			/* So, update the array for the given year */
			tm.tm_year = year - 1900;
			if(fm_bill_pol_check_duplicate_date(tm, calendarpp, i) == 0) {
				(*calendarpp)[i] = mktime(&tm);
				i += 1;
			}

			/* and the next year*/
			tm.tm_year = year + 1 - 1900;
			if(fm_bill_pol_check_duplicate_date(tm, calendarpp, i) == 0) {
				(*calendarpp)[i] = mktime(&tm);
				i += 1;
			}

		}
		else if (cal_year >= year -1 && cal_year <= year +1)	
		{
			tm.tm_year = cal_year - 1900;
			if(fm_bill_pol_check_duplicate_date(tm, calendarpp, i) == 0) {
				(*calendarpp)[i] = mktime(&tm);
				i += 1;
			}
		}
	}

	/* If we have allocated more than required, resize the array */
	if (*calendarpp && i < (3 * elem_count)&& (i != 0)) {
		*calendarpp = (time_t *) realloc (*calendarpp, i * (sizeof (time_t)));
	}
	if (*calendarpp == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_get_non_billing_days realloc: memory error", ebufp);
		goto cleanup;
	}
	
	/* Sort the calendar in ascending order */
	qsort(*calendarpp, i, sizeof(time_t), fm_bill_pol_compare_dates);

cleanup:	
	PIN_FLIST_DESTROY_EX(&cfg_flistp, NULL);

	return;
}

/*******************************************************************
 * fm_bill_pol_add_n_bus_days()
 *
 * Adds n business days to the give day using the holiday calendar.
 * Note that the resultant day can fall on next year (at max)
 *******************************************************************/
static void
fm_bill_pol_add_n_bus_days(
	pcm_context_t	*ctxp,
	int 		n_bus_days,
	char 		*cal_name,
	time_t		*a_time,
	pin_errbuf_t    *ebufp)
{

	time_t		*calendar = NULL;
	struct tm	*tm = NULL;
	time_t 		day; 
	int 		i = 0;

	if (!a_time || !n_bus_days)
		return;

	/* Fill in the non-billing day calendar for the current year and the next year */
	tm = localtime(a_time);
	if (tm == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_IS_NULL, 0, 0, 0);
		
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_add_n_bus_days: time is Null", ebufp);
		goto cleanup;
	}
	day = *a_time;
	fm_bill_pol_get_non_billing_days(ctxp, 1900 + tm->tm_year, cal_name, 
		&calendar, ebufp);

	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_add_n_bus_days: "
			"Error in getting Non-billing days", ebufp);
		goto cleanup;
	}

	/*******************************************************************
	 * Find out the first non-billing day that falls on or after the end_t 
	 * For ex: Bill Cycle Date 5-Jun-04, 
	 * Non-billing days are: 26-Jan-04, 1-May-04, 4-Jul-04, 25-Dec-04
	 * then when the for loop ends, i will point to the index of 4-Jul-04.
	 * of the calendar array.
	 *******************************************************************/
	if (calendar)
	{
		for (day = *a_time, i=0; day > calendar[i]; i++)
				;
	}
	
	/*******************************************************************
	 * Keep adding days until we've taken all 'n' business days account are 
	 * accounted for. Skip all the weekend days that pass by.
	 *******************************************************************/
	while (n_bus_days > 0)
	{
		fm_utils_add_n_days(1, &day);
		tm = localtime(&day);
		if (tm == NULL) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_IS_NULL, 0, 0, 0);

			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_bill_pol_add_n_bus_days: time is Null", ebufp);
			goto cleanup;
		}

		/* Skip weekends days */
		if (tm->tm_wday == SATURDAY || tm->tm_wday == SUNDAY) 
		{
			continue;
		}
		/* Skip the Non-Billing day too, but increment the index */
		else if (calendar && (day == calendar[i]))
		{
		 	i++;	
			continue;
		}
		/* If we find one business day, start counting down */
		else 
		{
			n_bus_days--; 
		}
	}
	*a_time = day;

cleanup:
	/*We do not need the calendar array anymore. Free it */
	if (calendar != NULL)
	{
		free (calendar);
		calendar = NULL;
	}

	return;
}

/***********************************************************************
 * fm_bill_pol_get_nthweekdayofmonth()
 *
 * Gets the date for something like 3rd Tuesday of Month (or)
 * 2nd Friday of Month. If the day is already past, it gets it for the
 * next month.
 * Note that the resultant day can fall on next year (at max)
 ***********************************************************************/
static void
fm_bill_pol_get_nthweekdayofmonth(
	int 	dow, /* Day of the week starting from Sunday [0..6] */
	int 	week, /* What week within the month. 1, 2, 3, 4 or 5 */
	time_t	*a_time,
	pin_errbuf_t    *ebufp)
{
	enum { JANUARY=0, DECEMBER=11 };
	const int	DAYS_OF_WEEK = 7;
	int 		nthweekdayofmonth = 0; /* Calculated value goes here */

	/* This holds the day of the week (in number) for the 1st day of a month */
	int 		dow4firstdayofmonth; 
	int 		i = 0;
	struct tm	*tm = NULL;

	/* Find the day of the week for the 1st of the atime's month*/
	tm = localtime(a_time);
	if (tm == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_IS_NULL, 0, 0, 0);
		
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_get_nthweekdayofmonth: time is Null", ebufp);
		return;
	}
	tm->tm_mday = 1;
	tm->tm_isdst = -1;
	mktime(tm);
	dow4firstdayofmonth = tm->tm_wday;

	/*******************************************************************
	 * Find the day of the month: To do this, find the Day of the week
	 * to which the 1st day of the month (dow4firstdayofmonth) falls on.
	 * Once that is obtained, compare that with the Day of the Week
	 * parameter dow.
	 *
	 * For example:
	 * If dow4firstdayofmonth is TUESDAY and dow is THURSDAY
	 * then
	 * dow4firstdayofmonth is said to be < dow.
	 * Day difference is -2
	 * else if dow4firstdayofmonth is THURSDAY and dow is THURSDAY
	 * then
	 * dow4firstdayofmonth is said to be = dow.
	 * Day difference is 0
	 * else if dow4firstdayofmonth is THURSDAY and dow is TUESDAY
	 * then
	 * dow4firstdayofmonth is said to be > dow.
	 * Day difference is +2
	 * fi
	 *
	 * If the Day difference is <= 0
	 * then
	 * The required date can be obtained with the formula
	 * (7 * (No. of weeks - 1)) + (Day difference)
	 * else
	 * (7 * No. of weeks) + (Day difference)
	 * fi
	 *******************************************************************/
	if (dow4firstdayofmonth <= dow) {
		nthweekdayofmonth = (DAYS_OF_WEEK * (week - 1)) 
			+ (dow - dow4firstdayofmonth);
	}
	else {
		nthweekdayofmonth = (DAYS_OF_WEEK * (week)) 
			+ (dow - dow4firstdayofmonth);
	}

	nthweekdayofmonth += 1; /* Calculation done */
	tm = localtime(a_time);
	if (tm == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_IS_NULL, 0, 0, 0);
		
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_get_nthweekdayofmonth: time is Null", ebufp);
		return;
	}
	tm->tm_isdst = -1;
	mktime(tm);

	/*******************************************************************
	* Find the day of the month for atime. If the resultant day is not
	* in future w.r.to bill cycle end date then do similar calculation
	* using month next to the atime's month
	********************************************************************/
	if (tm->tm_mday >= nthweekdayofmonth)
	{
		/* Get Next Month */
		if (tm->tm_mon == DECEMBER)
		{
			tm->tm_mon = JANUARY;
			tm->tm_year += 1;
		}
		else {
			tm->tm_mon += 1;
		}

		/* Make time for the first day of Next Month */
		tm->tm_mday = 1;
		tm->tm_isdst = -1;
		mktime(tm);
		dow4firstdayofmonth = tm->tm_wday;
		if (dow4firstdayofmonth <= dow) {
			nthweekdayofmonth = (DAYS_OF_WEEK * (week - 1)) 
				+ (dow - dow4firstdayofmonth);
		}
		else {
			nthweekdayofmonth = (DAYS_OF_WEEK * (week)) 
				+ (dow - dow4firstdayofmonth);
		}

		nthweekdayofmonth += 1;

		/* Make time for the resultant date */
		tm->tm_mday = nthweekdayofmonth;
		tm->tm_isdst = -1;
		*a_time = mktime(tm);
	}
	else /* Otherwise, the resultant date is already in future */
	{
		tm->tm_mday = nthweekdayofmonth;
		tm->tm_isdst = -1;
		*a_time = mktime(tm);
	}
}



static void fm_bill_pol_bill_notification(
	cm_nap_connection_t     *connp,
	pcm_context_t           *ctxp,
        u_int                   flags,
        pin_flist_t             *in_flistp,
	time_t			*due_t,
        pin_flist_t             **ret_flistpp,
        pin_errbuf_t            *ebufp) {

       poid_t  *acct_pdp = NULL;
        poid_t  *svc_pdp = NULL;
        poid_t  *bill_pdp = NULL;
        poid_t  *billinfo_pdp = NULL;
        pin_flist_t     *out_flistp = NULL;
        pin_flist_t     *res_flistp = NULL;
	char		*billinfo_id = NULL;

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_bill_pol_bill_pre_commit input flist", in_flistp);


	billinfo_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);

	fm_mso_utils_read_any_object(ctxp, billinfo_pdp, &out_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_bill_pol_bill_pre_commit  BILLINFO FLIST ", out_flistp);
	if(fm_utils_op_is_ancestor(connp->coip,MSO_OP_BILL_MAKE_BILL_NOW))
	{
		bill_pdp = PIN_FLIST_FLD_TAKE(out_flistp, PIN_FLD_ACTUAL_LAST_BILL_OBJ, 0, ebufp);
	}
	else
	{
		bill_pdp = PIN_FLIST_FLD_TAKE(out_flistp, PIN_FLD_BILL_OBJ, 0, ebufp);
	}
	billinfo_id = PIN_FLIST_FLD_TAKE(out_flistp, PIN_FLD_BILLINFO_ID, 0, ebufp);

	PIN_FLIST_DESTROY_EX(&out_flistp, NULL);

	out_flistp = NULL;

        if (!PIN_POID_IS_NULL(bill_pdp) && (billinfo_id && !strcmp(billinfo_id, "BB")))
        {
	    svc_pdp = PIN_POID_CREATE(1, "/service/telco/broadband", -1, ebufp);
            fm_mso_billing_notification(ctxp, bill_pdp, svc_pdp,  due_t, ebufp);
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_bill_pol_bill_pre_commit output flist", *ret_flistpp);

        return;

}


//MSO Customization Starts
static void fm_mso_billing_notification(
        pcm_context_t           *ctxp,
        poid_t                          *bill_pdp,
        poid_t                          *svc_pdp,
	time_t			*due_t,
        pin_errbuf_t            *ebufp)
{
                pin_flist_t             *in_flistp = NULL;
                pin_flist_t             *r_flistp = NULL;
                pin_flist_t             *notif_flistp = NULL;
                int32                   notif = NTF_BILLING;

                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                "Error Prior to Billing Notificaiton", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
        }

                in_flistp = PIN_FLIST_CREATE(ebufp);
                r_flistp = PIN_FLIST_CREATE(ebufp);
                // Reading the Bill Object Fields required for Notification
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, bill_pdp, ebufp);
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_BILL_NO, "", ebufp);
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_DUE, NULL, ebufp);
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_END_T, NULL, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill Object Read input flist", in_flistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, in_flistp, &r_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill Object Read output flist", r_flistp);

                //Preparing input and Calling Notfication API
                notif_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_ACCOUNT_OBJ, notif_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_FLAGS, &notif, ebufp);
                PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_SUBJECT, "Hathway - Bill Generated", ebufp);
                PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_BILL_NO, notif_flistp, PIN_FLD_BILL_NO, ebufp);
                PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_DUE, notif_flistp, PIN_FLD_AMOUNT, ebufp);
                PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_END_T, notif_flistp, PIN_FLD_END_T, ebufp);
                PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_DUE_T, due_t, ebufp);

                PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notifiation input flist", notif_flistp);
                PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, notif_flistp, &r_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification output flist", r_flistp);

                PIN_FLIST_DESTROY_EX(&in_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&notif_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Notification Error", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                }

                return;
}
//MSO Customization Ends

