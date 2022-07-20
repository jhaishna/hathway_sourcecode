/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 * Version 1.0: 08-10-2013: Vilva Sabarikanth: Added the MSO Customization of
 * Deposit Payments using MSO_OP_PYMT_DEPOSIT_REFUND opcode and deposit object creation
 * for deposit refunds based on refund rule.
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_deposit_refund.c /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_DEPOSIT_REFUND operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
#include <time.h>
#include <math.h>
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_rate.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_ar.h"
#include "ops/bill.h"
#include "mso_ops_flds.h" 
#include "mso_lifecycle_id.h"

#define HW_OPTION1 "HardwarePlan_STP_OPTION_1"
#define HW_OPTION2 "HardwarePlan_STP_OPTION_2"
#define HW_OPTION3 "HardwarePlan_STP_OPTION_3"
#define HW_OPTION4 "HardwarePlan_STP_OPTION_4"

#define ITEM_STATUS_AVAIL 0
#define ITEM_STATUS_PENDING 1
#define ITEM_STATUS_OPEN 2
#define ITEM_STATUS_CLOSED 4
#define DEP_STATUS_CLOSE 1

#define MAX_STEPS 36
#define MIN_VAT_DURATION 6

#define SEQ_TYPE_REFUND "MSO_SEQUENCE_TYPE_REFUND"
#define SEQ_TYPE_ADJUSTMENT "MSO_SEQUENCE_TYPE_ADJUSTMENT"

#define CURRENCY 356

#define CHEQUE_MODE 1

#define NOTIFICATION 5
#define EVENT_MSO_REFUND_CREDIT "/event/billing/mso_refund_credit"

#define FULL_REFUND 0
#define STEP_REFUND 1
#define PERCENT_REFUND 3
#define WITH_TAX 1
#define WITHOUT_TAX 0
#define DAYS 0
#define MONTHS 1
#define YEARS 2

#define DEPOSIT_REFUND "DEPOSIT_REFUND"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_pymt_deposit_refund(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

static void mso_retrieve_deposit(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);
		
static void mso_find_refund_rule(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	time_t				*deposit_start_t,
	time_t				now_t,
	pin_decimal_t		*dep_amount,
	int32					*VAT_return,
	pin_decimal_t		**step_amtpp,
	pin_errbuf_t		*ebufp);

//static void
//mso_fld_refund_gen_lc_event(
//	pcm_context_t           *ctxp,
//	pin_flist_t             *i_flistp,
//	poid_t			*a_pdp,
//	pin_flist_t             *au_flistp,
//	pin_decimal_t		*dep_total,
//	char			*dep_number,
//	char                    *disp_name,
//	pin_errbuf_t            *ebufp);

PIN_IMPORT void mso_item_status_update(
	pcm_context_t		*ctxp,
	poid_t				*item_flistp,
	pin_flist_t			**s_flistp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void 
fm_mso_utils_create_pre_rated_impact(
	pcm_context_t		*ctxp,
	poid_t				*acct_pdp,
	poid_t				*service_pdp,
	char				*program_name,
	char				*sys_description,
	char				*event_type,
	pin_decimal_t		*amount,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

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
void fm_mso_utils_read_any_object(
	pcm_context_t           *ctxp,
	poid_t                  *objp,
	pin_flist_t             **out_flistpp,
	pin_errbuf_t            *ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PYMT_DEPOSIT_REFUND operation.
 *******************************************************************/
void
mso_pymt_deposit_refund(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*amt_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*it_flistp = NULL;
	pin_flist_t		*t_flistp = NULL;
	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*svc_flistp = NULL;
	pin_flist_t		*notif_flistp = NULL;
	pin_flist_t		*item_arrp = NULL;
	pin_flist_t		*au_flistp = NULL;

	u_int64			db = 1;
	u_int64         id = (u_int64)-1;
	pin_cookie_t	cookie = NULL;
	int32			recid = 0;
	int32           len = 0;
	int32			sub = 0;
	int32			elem = 0;	
	int32			first = 0;
	int32			c_type = 1;
	int32			*flag = NULL;
	int32			status = 0;
	int32			notif = NOTIFICATION;
	int				currency = CURRENCY;
	int32			d_status = DEP_STATUS_CLOSE;
	int32			i_status = ITEM_STATUS_CLOSED;
	poid_t			*i_pdp = NULL;
	poid_t			*a_pdp = NULL;
	poid_t			*d_pdp = NULL;
	poid_t			*ref_pdp = NULL;
	poid_t			*rfnd_pdp = NULL;
	poid_t			*adj_pdp = NULL;
	poid_t			*adj_pdp1 = NULL;
	poid_t			*ref_pdp1 = NULL;
	poid_t			*t_pdp = NULL;
	poid_t			*user_id = NULL;
	poid_t			*srvc_pdp = NULL;
	time_t          time_now = 0;
	time_t			*c_time = NULL;
	pin_decimal_t	*step_amount = NULL;
	pin_decimal_t	*tot_amount = NULL;
	pin_decimal_t	*ded_amount_ip = NULL;
	pin_decimal_t	*ded_amount = NULL;
	pin_decimal_t	*dep_amount = NULL;
	pin_decimal_t	*amt = NULL;
	pin_decimal_t	*zero_amt = NULL;
	pin_decimal_t	*item_amt = NULL;
	char            poid_buf[PCM_MAX_POID_TYPE + 1];
	char			poid_str[100];
    char			*code = NULL;
	char			*ref_str = NULL;
	char			*acc_no = NULL;
	char			*owner = NULL;
	char			*name = NULL;
	char			*dep_number = NULL;
	char			*description = NULL;
	char			sys_description[256];
	char			*program_name = NULL;
	char			*decimal_str = NULL;
	int32			*impact_type = NULL;
	int32			VAT_return = 0;

	tot_amount = pbo_decimal_from_str("0.0", ebufp);
	amt = pbo_decimal_from_str("0.0", ebufp);
	zero_amt = pbo_decimal_from_str("0.0", ebufp);
	step_amount = pbo_decimal_from_str("0.0", ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_DEPOSIT_REFUND) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_pymt_deposit_refund opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_op_pymt_deposit_refund input flist", i_flistp);

	/*************************************************************
	 * Identifying Deposit Objects 
	 *************************************************************/
	amt_flistp = PIN_FLIST_CREATE(ebufp);
	in_flistp = PIN_FLIST_CREATE(ebufp);
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
	t_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	description = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	memset(sys_description,'\0',256);
	if(description)
	{
		sprintf(sys_description,"%s%s","DEPOSIT REFUND: ",description);		
	}
	else
	{
		sprintf(sys_description,"%s","DEPOSIT REFUND");
	}

	flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DEPOSITS, PIN_ELEMID_ANY, 0, ebufp);
	dep_number = PIN_FLIST_FLD_GET(flistp, MSO_FLD_DEPOSIT_NO, 0, ebufp);


	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, t_pdp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Deposit Object retrieval Input flist", flistp);
	mso_retrieve_deposit(ctxp, flistp, &amt_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Deposit Object retrieval return flist", amt_flistp);
	rs_flistp = PIN_FLIST_FLD_GET(amt_flistp, PIN_FLD_RESULTS, 0, ebufp);
	d_pdp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_POID, 0, ebufp);
	a_pdp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	dep_amount = PIN_FLIST_FLD_GET(rs_flistp, MSO_FLD_DEPOSIT_AMOUNT, 0, ebufp);
	code = PIN_FLIST_FLD_GET(rs_flistp, MSO_FLD_REFUND_RULE, 0, ebufp);	
	srvc_pdp = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	c_time = PIN_FLIST_FLD_GET(rs_flistp, MSO_FLD_DEPOSIT_DATE, 0, ebufp);

	if (!PIN_ERRBUF_IS_ERR(ebufp)) {
//		sub = (int32 )(*c_time / 1);
		time_now = pin_virtual_time((time_t *)NULL);
		/***********************************************************************************
			Identify the date difference from deposit payment date to
			refund initiation date in months. If the number of months is
			greater than 36 then assigning the max value(36) for the refund rule.
			If the refund initiated within a month, setting the refund rule to 1st month
		 ************************************************************************************/
/*		elem = (int32)ceil(((time_now - sub)/86400)/30);
		if(elem > MAX_STEPS)
			elem = MAX_STEPS;
*/
		/*************************************************************
		 * Identifying Deposit Refund Rule with Exact Amount
		 *************************************************************/
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, t_pdp, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_CODE, (char *)code, ebufp);

		r_flistp = NULL;
		/* Hardware Plan Option 1 & 2 is refunded completely, hence need not 
		 * pass the calculated element id. The Refund Rule Config Object is 
		 * also configured in the same fashion.
		 */
		 /*
		if( (0 == strcmp(code, HW_OPTION1))||(0 == strcmp(code, HW_OPTION2)))
		{
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_STEP, (void *)&first, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_STEP, (void *)&elem, ebufp);
		}
		*/

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Retrieving Refund Rule Input flist", in_flistp);
		mso_find_refund_rule(ctxp, in_flistp, c_time, time_now, 
							dep_amount, &VAT_return, &step_amount, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, t_pdp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
				"No Refund Rule Matching for Refunding the Deposits");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53050", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"No Refund Rule Matching for Refunding the Deposits", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			goto Cleanup;
		}

		pbo_decimal_add_assign(tot_amount, step_amount, ebufp);
	}
	else
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, t_pdp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Deposit Object(/mso_deposit) Not Found");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53052", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No Deposit Found", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		goto Cleanup;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Initial Step Refund amount is:");
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(tot_amount, ebufp));
	decimal_str = pbo_decimal_to_str(tot_amount, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, decimal_str);
	pin_free(decimal_str);
	decimal_str = NULL;
	/* If the refund is requsted in less than 6 months, then no need to return VAT amount */
	if (VAT_return == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Vat Return in Set");
		while((flistp = PIN_FLIST_ELEM_GET_NEXT(rs_flistp, PIN_FLD_ITEMS,
			&recid, 1, &cookie, ebufp)) != (pin_flist_t *) NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Items flistp:",flistp);
			impact_type = PIN_FLIST_FLD_GET(flistp, PIN_FLD_IMPACT_TYPE, 0, ebufp);
			item_amt = PIN_FLIST_FLD_GET(flistp, PIN_FLD_AMOUNT, 0, ebufp);
			if (*impact_type == PIN_IMPACT_TYPE_TAX)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"This is a Tax Impact");
				pbo_decimal_add_assign(amt, item_amt, ebufp);
			}
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"The VAT amount refundable is:");
		//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(amt, ebufp));
		decimal_str = pbo_decimal_to_str(amt, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,decimal_str);
		pin_free(decimal_str);
		decimal_str = NULL;
		pbo_decimal_add_assign(tot_amount, amt, ebufp);
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
		"Step refund amount after adding VAT(if any applicable):");
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(tot_amount, ebufp));
	decimal_str = pbo_decimal_to_str(tot_amount, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,decimal_str);
	pin_free(decimal_str);
	decimal_str = NULL;

	if (pbo_decimal_compare(tot_amount, zero_amt, ebufp)  <= 0 )
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, t_pdp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"There is no refund amount left after deducting VAT!!");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53052", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
			"There is no refund amount left after deducting VAT!!", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		goto Cleanup;
	}

	ded_amount_ip = (pin_decimal_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp);
	if(pbo_decimal_is_null(ded_amount_ip,ebufp))
	{
		ded_amount = pbo_decimal_from_str("0.0", ebufp);
	}
	else
	{
		ded_amount = pbo_decimal_copy(ded_amount_ip, ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"The Hardware Damage Deduction amount is:");
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(ded_amount, ebufp));
        decimal_str = pbo_decimal_to_str(ded_amount, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,decimal_str);
        pin_free(decimal_str);
        decimal_str = NULL;

	if (pbo_decimal_compare(tot_amount, ded_amount, ebufp) == 1)
	{
		pbo_decimal_subtract_assign(tot_amount, ded_amount, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Step amount after all deductions is:");
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(tot_amount, ebufp));
		decimal_str = pbo_decimal_to_str(tot_amount, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,decimal_str);
		pin_free(decimal_str);
		decimal_str = NULL;

		pbo_decimal_round_assign(tot_amount, 2, ROUND_DOWN, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Step amount after rounding:");
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(tot_amount, ebufp));
		decimal_str = pbo_decimal_to_str(tot_amount, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,decimal_str);
		pin_free(decimal_str);
		decimal_str = NULL;

		pbo_decimal_negate_assign(tot_amount, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Applying the credit refund impact of:");
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(tot_amount, ebufp));
		decimal_str = pbo_decimal_to_str(tot_amount, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,decimal_str);
		pin_free(decimal_str);
		decimal_str = NULL;

		/* Call the function to make the Credit charge impact */
		fm_mso_utils_create_pre_rated_impact(ctxp, a_pdp, srvc_pdp, program_name,sys_description,
						EVENT_MSO_REFUND_CREDIT, tot_amount, &au_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
				PIN_ERRBUF_CLEAR(ebufp);
				status = 1;
				err_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, t_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error while applying credit impact");
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53052", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
								"Error while applying credit impact", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				goto Cleanup;
		}

		/* Create a lifecycle event for Deposit Refund */

		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_AMOUNT, tot_amount, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_DEPOSIT_NO, dep_number, ebufp);
		fm_mso_create_lifecycle_evt(ctxp, i_flistp, NULL, ID_REFUND, ebufp );
		//mso_fld_refund_gen_lc_event(ctxp, i_flistp, a_pdp, au_flistp, tot_amount, dep_number, DEPOSIT_REFUND, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
				PIN_ERRBUF_CLEAR(ebufp);
				status = 1;
				err_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, t_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error while applying credit impact");
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53052", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
								"Error while applying credit impact", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				goto Cleanup;
		}
		//Preparing Input and Calling Notification API
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling Notification API");
		notif_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_POID, a_pdp, ebufp);
		if(user_id !=NULL)
			PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_USERID, user_id, ebufp);
		PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
		PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_FLAGS, &notif, ebufp);
		PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_AMOUNT, tot_amount, ebufp);
		PIN_FLIST_FLD_SET(notif_flistp, MSO_FLD_REFUND_NO, dep_number, ebufp);
		PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_ACK_TIME, &time_now, ebufp);
		//PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_REASON, flag, ebufp);

		it_flistp = NULL;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification input flist", notif_flistp);
		PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, notif_flistp, &it_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification output flist", it_flistp);
		PIN_FLIST_DESTROY_EX(&it_flistp, NULL);

		//Calling the Deposit Status Update function to close the deposit object
		item_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_POID, d_pdp, ebufp);
		PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_STATUS, (void *)&d_status, ebufp);
		PIN_FLIST_FLD_SET(item_flistp, MSO_FLD_REFUND_DATE, &time_now, ebufp);

		it_flistp = NULL;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Deposit Object Status Update Input flist", item_flistp);
		mso_item_status_update(ctxp, item_flistp, &it_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Deposit Object Status Update Output flist", it_flistp);

		PIN_FLIST_DESTROY_EX(&item_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&it_flistp, NULL);
		item_flistp = NULL;
		it_flistp = NULL;

		cookie = NULL;
		recid = 0;

		//Calling the Status Update function to close the deposit payment items
		while((flistp = PIN_FLIST_ELEM_GET_NEXT(rs_flistp, PIN_FLD_ITEMS,
			&recid, 1, &cookie, ebufp)) != (pin_flist_t *) NULL)
		{
			i_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);
			
			item_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_POID, i_pdp, ebufp);
			PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_STATUS, (void *)&i_status, ebufp);
			PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_DUE, zero_amt, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Deposit Payment Item Status Update Input flist", item_flistp);
			mso_item_status_update(ctxp, item_flistp, &it_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"Deposit Payment Item Status Update Output flist", it_flistp);
			PIN_FLIST_DESTROY_EX(&item_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&it_flistp, NULL);
			item_flistp = NULL;
			it_flistp = NULL;
		}
	}
	else
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, t_pdp, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"There is no refund amount left after all deducations!!");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53052", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
			"There is no refund amount left after all deducations!!", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		goto Cleanup;
	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, t_pdp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Deposit Object(/mso_deposit) Not Found");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53052", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "No Deposit Found", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			goto Cleanup;
	}
	else 
	{
		status = 0;
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, "Deposit Refunded Successfully", ebufp);

		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_FLD_COPY(notif_flistp, PIN_FLD_AMOUNT, *o_flistpp, PIN_FLD_AMOUNT, ebufp );
		PIN_FLIST_FLD_COPY(notif_flistp, MSO_FLD_REFUND_NO, *o_flistpp, MSO_FLD_REFUND_NO, ebufp );
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_pymt_deposit_refund return flist", *o_flistpp);

		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	}
	
Cleanup:
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&amt_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&it_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&notif_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&au_flistp, NULL);

	if (status == 1)
	{
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	}
	if (!pbo_decimal_is_null(tot_amount, ebufp))
	{
		pbo_decimal_destroy(&tot_amount);
	}
	if (!pbo_decimal_is_null(amt, ebufp))
	{
		pbo_decimal_destroy(&amt);
	}
	if (!pbo_decimal_is_null(zero_amt, ebufp))
	{
		pbo_decimal_destroy(&zero_amt);
	}
	if (!pbo_decimal_is_null(step_amount, ebufp))
	{
		pbo_decimal_destroy(&step_amount);
	}
	if (!pbo_decimal_is_null(ded_amount, ebufp))
	{
		pbo_decimal_destroy(&ded_amount);
	}

	return;
	
}

/* This module retrieves the refund rule details based from the Input Flist passed to MSO_OP_PYMT_DEPOSIT_REFUND */
static void mso_find_refund_rule(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	time_t				*deposit_start_t,
	time_t				now_t,
	pin_decimal_t		*dep_amount,
	int32					*VAT_return,
	pin_decimal_t		**step_amtpp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)-1;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	int32			s_flags = 256;
	poid_t			*pdp = NULL;
	poid_t			*t_pdp = NULL;
	int64			db= 0;
	char			*plan_code=NULL;
	pin_cookie_t	cookie = NULL;
	int32			recid = 0;

	int32			*rule_type = NULL; // 0-FULL_REFUND,1-STEP_REFUND,3-PERCENT_REFUND
	int32			*refund_with_tax = NULL; // 1-WITH_TAX,2-WITHOUT_TAX
	int32			*step_unit = NULL; // 0-DAYS,1-MONTHS,2-YEARS
	pin_decimal_t	*step_amt = NULL;
	pin_decimal_t	*percent = NULL;
	pin_decimal_t	*percent_amt = NULL;
	int32			*start_step = NULL;
	int32			*end_step = NULL;
	int32			date_diff = 0;
	int32			date_diff_days = 0;
	int32			date_diff_months = 0;
	int32			date_diff_years = 0;
	char			tmp[1024];
	char			*decimal_str = NULL;
	char			*decimal_str2 = NULL;


	percent = pbo_decimal_from_str("100.00", ebufp);
	t_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(t_pdp);
	plan_code = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CODE, 0, ebufp);

	s_flistp = PIN_FLIST_CREATE(ebufp);

	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_cfg_refund_rule where F1 = V1 ", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, MSO_FLD_RULE_NAME, (void *)plan_code, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"refund rule config search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &rs_flistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		goto CLEANUP;
	}
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	s_flistp = NULL;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"refund rule config search output flist", rs_flistp);
	result_flistp = PIN_FLIST_ELEM_GET(rs_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	if (result_flistp == NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No refund rule found for the deposit !!");
		goto CLEANUP;
	}

	rule_type = PIN_FLIST_FLD_GET(result_flistp, MSO_FLD_RULE_TYPE, 0, ebufp);
	if (*rule_type == FULL_REFUND)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Full Refund");
		s_flistp = PIN_FLIST_ELEM_GET(result_flistp, MSO_FLD_RULE_STEPS,PIN_ELEMID_ANY,0,ebufp);
		step_amt = PIN_FLIST_FLD_GET(s_flistp, MSO_FLD_STEP_AMOUNT, 0, ebufp);
		*step_amtpp = pbo_decimal_copy(step_amt, ebufp);
		*VAT_return = WITH_TAX;
		goto CLEANUP;
	}
	else if (*rule_type == STEP_REFUND)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Step Refund");
		/* Get the time difference between deposit creation and present time */
		date_diff = now_t - *deposit_start_t;

		/* Loop through each MSO_FLD_RULE_STEPS to fetch the valid step */
		while((s_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, MSO_FLD_RULE_STEPS,
			&recid, 1, &cookie, ebufp)) != (pin_flist_t *) NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"RULE_STEP Flist:",s_flistp);
			step_unit = PIN_FLIST_FLD_GET(s_flistp, MSO_FLD_STEP_UNIT, 0, ebufp);
			start_step = PIN_FLIST_FLD_GET(s_flistp, MSO_FLD_START_STEP, 0, ebufp);
			end_step = PIN_FLIST_FLD_GET(s_flistp, MSO_FLD_END_STEP, 0, ebufp);
			step_amt = PIN_FLIST_FLD_GET(s_flistp, MSO_FLD_STEP_AMOUNT, 0, ebufp);
			refund_with_tax = PIN_FLIST_FLD_GET(s_flistp, PIN_FLD_FLAGS, 0, ebufp);
			if (*step_unit == DAYS)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Unit is Days");
				date_diff_days = (int32) ceil(date_diff/86400);
				/* Check if the time_difference falls in the range */
				if (date_diff_days >= *start_step && date_diff_days < *end_step)
				{
					*step_amtpp = pbo_decimal_copy(step_amt, ebufp);
					*VAT_return = *refund_with_tax;
					goto CLEANUP;
				}
			}
			else if (*step_unit == MONTHS)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Unit is Months");
				date_diff_months = (int32) ceil((date_diff/86400)/30);
				sprintf(tmp,"now_t=%ld,deposit_start_t=%ld,date_diff=%ld,"
					"date_diff_months=%ld,start_step=%ld,end_step=%ld",
					now_t,*deposit_start_t,date_diff,date_diff_months,*start_step,*end_step);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
				/* Check if the time_difference falls in the range */
				if (date_diff_months >= *start_step && date_diff_months < *end_step)
				{
					*step_amtpp = pbo_decimal_copy(step_amt, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
					*VAT_return = *refund_with_tax;
					decimal_str = pbo_decimal_to_str(*step_amtpp, ebufp);
					sprintf(tmp,"step_amtpp=%s,VAT_return=%ld",
						decimal_str,*VAT_return);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
					pin_free(decimal_str);
					decimal_str = NULL;
					goto CLEANUP;
				}
			}
			else if (*step_unit == YEARS)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Unit is Years");
				date_diff_years = (int32) ceil(((date_diff/86400)/30)/12);
				/* Check if the time_difference falls in the range */
				if (date_diff_years >= *start_step && date_diff_years < *end_step)
				{
					*step_amtpp = pbo_decimal_copy(step_amt, ebufp);
					*VAT_return = *refund_with_tax;
					goto CLEANUP;
				}
			}
		}
	}
	else if (*rule_type == PERCENT_REFUND)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Percent Refund");
		/* Get the time difference between deposit creation and present time */
		date_diff = now_t - *deposit_start_t;

		/* Loop through each MSO_FLD_RULE_STEPS to fetch the valid step */
		while((s_flistp = PIN_FLIST_ELEM_GET_NEXT(result_flistp, MSO_FLD_RULE_STEPS,
			&recid, 1, &cookie, ebufp)) != (pin_flist_t *) NULL)
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"RULE_STEP Flist:",s_flistp);
			step_unit = PIN_FLIST_FLD_GET(s_flistp, MSO_FLD_STEP_UNIT, 0, ebufp);
			start_step = PIN_FLIST_FLD_GET(s_flistp, MSO_FLD_START_STEP, 0, ebufp);
			end_step = PIN_FLIST_FLD_GET(s_flistp, MSO_FLD_END_STEP, 0, ebufp);
			step_amt = PIN_FLIST_FLD_GET(s_flistp, MSO_FLD_STEP_AMOUNT, 0, ebufp);
			refund_with_tax = PIN_FLIST_FLD_GET(s_flistp, PIN_FLD_FLAGS, 0, ebufp);
			if (*step_unit == DAYS)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Unit is Days");
				date_diff_days = (int32) ceil(date_diff/86400);
				/* Check if the time_difference falls in the range */
				if (date_diff_days >= *start_step && date_diff_days < *end_step)
				{
					percent_amt = pbo_decimal_divide(step_amt, percent,ebufp);
					*step_amtpp = pbo_decimal_multiply(dep_amount, percent_amt, ebufp);
					*VAT_return = *refund_with_tax;
					goto CLEANUP;
				}
			}
			else if (*step_unit == MONTHS)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Unit is Months");
				date_diff_months = (int32) ceil((date_diff/86400)/30);
				sprintf(tmp,"now_t=%ld,deposit_start_t=%ld,date_diff=%ld,"
					"date_diff_months=%ld,start_step=%ld,end_step=%ld",
					now_t,*deposit_start_t,date_diff,date_diff_months,*start_step,*end_step);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
				/* Check if the time_difference falls in the range */
				if (date_diff_months >= *start_step && date_diff_months < *end_step)
				{
					percent_amt = pbo_decimal_divide(step_amt, percent,ebufp);
					*step_amtpp = pbo_decimal_multiply(dep_amount, percent_amt, ebufp);
					decimal_str = pbo_decimal_to_str(*step_amtpp, ebufp);
					sprintf(tmp,"final step_amt =%s",decimal_str);
					*VAT_return = *refund_with_tax;
					decimal_str2 = pbo_decimal_to_str(*step_amtpp, ebufp);
					sprintf(tmp,"step_amtpp=%s,VAT_return=%ld",
						decimal_str2,*VAT_return);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,tmp);
					pin_free(decimal_str);
					pin_free(decimal_str2);
					decimal_str = NULL;
					decimal_str2 = NULL;
					goto CLEANUP;
				}
			}
			else if (*step_unit == YEARS)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Unit is Years");
				date_diff_years = (int32) ceil(((date_diff/86400)/30)/12);
				/* Check if the time_difference falls in the range */
				if (date_diff_years >= *start_step && date_diff_years < *end_step)
				{
					percent_amt = pbo_decimal_divide(step_amt, percent,ebufp);
					*step_amtpp = pbo_decimal_multiply(dep_amount, percent_amt, ebufp);
					*VAT_return = *refund_with_tax;
					goto CLEANUP;
				}
			}
		}
	}


	CLEANUP:
		PIN_FLIST_DESTROY_EX(&rs_flistp, ebufp);
		if (!pbo_decimal_is_null(percent_amt, ebufp))
		{
			pbo_decimal_destroy(&percent_amt);
		}
		if (!pbo_decimal_is_null(percent, ebufp))
		{
			pbo_decimal_destroy(&percent);
		}
	return;
}

/* This module retrieves the deposit amount from the product purchased for deposit */
static void mso_retrieve_deposit(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)0;
	u_int64			db = 0;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int32			s_flags = 256;
	int32			i_status = ITEM_STATUS_AVAIL;
	poid_t			*a_pdp = NULL;
	poid_t			*pdp = NULL;
		
	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0 , ebufp);
	db = PIN_POID_GET_DB(a_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_deposit where F1 = V1 and F2 = V2 ", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_DEPOSIT_NO, flistp, MSO_FLD_DEPOSIT_NO, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, &i_status, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Deposit Object search input flist", s_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, ret_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Deposit Object search output flist", *ret_flistpp);

	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	return;
}

//static void
//mso_fld_refund_gen_lc_event(
//	pcm_context_t           *ctxp,
//	pin_flist_t             *i_flistp,
//	poid_t                  *a_pdp,
//	pin_flist_t             *au_flistp,
//	pin_decimal_t		*dep_total,
//	char			*dep_number,
//	char                    *disp_name,
//	pin_errbuf_t            *ebufp)
//{
//	pin_flist_t             *lc_iflistp = NULL;
//	pin_flist_t             *lc_temp_flistp = NULL;
//	pin_flist_t             *ro_flistp = NULL;
//	pin_flist_t             *res_flistp = NULL;
//	poid_t                  *user_id = NULL;
//	poid_t                  *event_pdp = NULL;
//	char                    *event = "/event/activity/mso_lifecycle/refund";
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//			return;
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_fld_refund_gen_lc_event Input FList:", i_flistp);
//
//	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp );
//	lc_iflistp = PIN_FLIST_CREATE(ebufp);
//	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_REFUND, 0, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, disp_name, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_USERID, user_id, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, dep_total, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_DEPOSIT_NO, dep_number, ebufp);
//	res_flistp = PIN_FLIST_ELEM_GET(au_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
//	if(res_flistp)
//		PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event Event Flist:", lc_iflistp);
//
//	fm_mso_utils_gen_event(ctxp, a_pdp, NULL, "MSO_OP_PYMT_DEPOSIT_REFUND",
//							disp_name, event, lc_iflistp, ebufp);
//
//	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
//	PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
//	return;
//}

