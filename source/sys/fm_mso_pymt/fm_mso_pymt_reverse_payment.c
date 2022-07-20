/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 * Version 1.0: 01-10-2013: Vilva Sabarikanth: Added the MSO Customization of
 * preparing the Input FLIST for PCM_OP_BILL_REVERSE opcode 
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_reverse_payment.c /cgbubrm_7.5.0 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_REVERSE_PAYMENT operation. 
 *******************************************************************/


#include <stdio.h> 
#include <string.h> 
#include <time.h>
 
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
#include "psiu_business_params.h"
#include "mso_ops_flds.h"
#include "mso_lifecycle_id.h"
#include "mso_ntf.h"


#define PYMT_EVT_CASH "/event/billing/payment/cash"
#define PYMT_EVT_TDS "/event/billing/payment/cash/tds"
#define PYMT_EVT_PORTAL "/event/billing/payment/cash/portal"
#define PYMT_EVT_SP_AUTOPAY "/event/billing/payment/cash/sp_autopay"
#define PYMT_EVT_CHECK "/event/billing/payment/check"
#define CHK_BOUNCE_EVENT "/event/billing/mso_penalty"

#define PAY_TYPE_CASH 10011
#define PAY_TYPE_CHECK 10012
#define PAY_TYPE_ONLINE 10013
#define PAY_TYPE_TDS 10018
#define PAY_TYPE_SP_AUTOPAY 10019
#define PAY_TYPE_ECS 10020
#define PAY_TYPE_PORTAL_TRANSFER 10021

#define REASON_ID_NO_PENALTY 		  0
#define REASON_ID_CHK_BOUNCE  		  1
#define REASON_ID_INVALID_ACCOUNT	  2
#define REASON_ID_PYMT_CORRECTION	  4
#define REASON_ID_OTHERS		  3


#define PYMT_REVERSAL_NOTIFICATION 4
#define CHECK_REVERSAL_NOTIFICATION 14
#define BILLINFO_ID_WS "WS_SETTLEMENT"
#define CODE1 "Account Mismatch"
#define CODE2 "Cheque Bounce"
#define CODE3 "Duplicate"
#define CODE4 "Others"
#define LCO_CUST_CARE 3
#define READWRITE 1

#define LOCAL_TRANS_OPEN_SUCCESS 0

#define SERVICE_TYPE_BB "/service/telco/broadband"
#define SERVICE_TYPE_CATV "/service/catv"
#define PAYMENT_REVERSAL "PAYMENT_REVERSAL"
#define CQ_BOUNCE_PENALTY "CQ_BOUNCE_PENALTY"
#define BAL_TRANS_LCO_SOURCE "BAL_TRANS_LCO_SOURCE"
#define BAL_TRANS_LCO_DEST "BAL_TRANS_LCO_DEST"
#define NON_MIGRATED 0
#define MIGRATED 1

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_pymt_reverse_pymt(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

void mso_get_repeipt_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

void mso_find_trans_id(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

void mso_pymt_reverse(
	pcm_context_t	*ctxp,
	pin_flist_t		*inp_flistp,
	pin_flist_t		*i_flistp,
	int				*reason_code,
	pin_decimal_t	*amount,
	char			*cheque_no,
	time_t			*cheque_dt,
	pin_decimal_t	*paid_amt,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

void
fm_mso_retrieve_service_obj(
	pcm_context_t		*ctxp, 
	poid_t				*acct_pdp,
	int32				*service_type,
	pin_flist_t			**ret_flistpp, 
	pin_errbuf_t		*ebufp);

void mso_update_receipt_obj(
	pcm_context_t		*ctxp, 
	poid_t				*rcpt_pdp,
	pin_flist_t			**ret_flistpp, 
	pin_errbuf_t		*ebufp);

static void fm_mso_pymt_process_lco_reversal(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			*ip_flistp,
	poid_t				*bal_grp_pdp,
	pin_errbuf_t			*ebufp);

static void
mso_process_lco_pymt_reversal(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	poid_t			*acct_pdp,
	pin_decimal_t		*amount,
	int32			*serv_type,
	pin_errbuf_t		*ebufp);

static void
mso_op_pymt_reverse_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *s_oflistp,
	char                    *disp_name,
	pin_decimal_t		*ip_amount,
	pin_errbuf_t            *ebufp);

PIN_IMPORT void	fm_mso_get_profile_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void	fm_mso_utils_balance_transfer(
	pcm_context_t		*ctxp,
	poid_t			*s_acct_pdp,
	poid_t			*d_acct_pdp,
	poid_t			*s_bal_pdp,
	poid_t			*d_bal_pdp,
	pin_decimal_t		*amount,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void	fm_mso_utils_read_any_object(
	pcm_context_t		*ctxp,
	poid_t			*objp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void	fm_mso_utils_get_service_details(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	int32			serv_type,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT
void fm_mso_utils_gen_event(
	pcm_context_t   	*ctxp,
	poid_t          	*acct_pdp,
	poid_t          	*serv_pdp,
	char            	*program,
	char            	*descr,
	char            	*event_type,
	pin_flist_t     	*in_flistp,
	pin_errbuf_t    	*ebufp);

PIN_IMPORT void
fm_mso_utils_create_pre_rated_impact(
	pcm_context_t           *ctxp,
	poid_t                  *acct_pdp,
	poid_t                  *service_pdp,
	char                    *program_name,
	char			*sys_description,
	char                    *event_type,
	pin_decimal_t           *amount,
	pin_flist_t             **au_oflistpp,
	pin_errbuf_t            *ebufp);

PIN_IMPORT void fm_mso_utils_get_billinfo_bal_details(
	pcm_context_t           *ctxp,
	poid_t                  *acct_pdp,
	char                    *billinfo_id,
	pin_flist_t             **o_flistpp,
	pin_errbuf_t            *ebufp);

extern void
fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * Main routine for the MSO_OP_PYMT_REVERSE_PAYMENT operation.
 *******************************************************************/
void
mso_pymt_reverse_pymt(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;

	pin_flist_t		*s_r_flistp = NULL;
	pin_flist_t		*p_r_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;

	poid_t			*user_id = NULL;
	poid_t			*rcpt_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*d_pdp = NULL;

	pin_decimal_t		*amount = NULL;
	pin_decimal_t		*paid_amount = NULL;

	int32			elemid = 0;
	int32			status = 0;
	int32			*ret_status = 0;
	int32			id = 1;
	int32			*migrated_pymt = NULL;
	int32			*serv_type = NULL;
	int32			*pay_type = NULL;
	//NETF REVERSAL
	pin_flist_t	*netf_in_flist = NULL;
	pin_decimal_t	 *net_pymt_amt = pbo_decimal_from_str("0", ebufp);
	int		*netf_status;

	int			*reason_code = NULL;
	int			local = 1;
	int64			db = 1;

	pin_cookie_t	cookie = NULL;

	char			*cheque_no = NULL;
	
	time_t			*cheque_dt = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_REVERSE_PAYMENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_pymt_reverse_payment opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_reverse_payment input flist", i_flistp);

	
	d_pdp = PIN_POID_CREATE(db, "/account", id, ebufp);
	local = fm_mso_trans_open(ctxp, d_pdp, 1, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Opened");
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error opening the transaction", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53020", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in opening transactions", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	reason_code = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 1 , ebufp);
	amount      = (void *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp);

	if (!reason_code)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Reason Code is missing in the input");
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53020", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Reason Code is missing in the input", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}
	if(*reason_code == 1 && amount == (pin_decimal_t *) NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Penalty amount is missing in the input");
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,  "53020", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Penalty amount is missing in the input", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}
	/***********************************************************
	 * Identifying the Event POID
	 ***********************************************************/
//	r_flistp = PIN_FLIST_CREATE(ebufp);
	
	mso_get_repeipt_info(ctxp, i_flistp, &r_flistp, ebufp);
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	s_r_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Receipt Details Not Found");
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53020", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Receipt Details Not Found", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		goto CLEANUP;
	}
	else if(strstr(PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_RECEIPT_NO,1,ebufp),"N-"))
	{
			net_pymt_amt = (pin_decimal_t *)pbo_decimal_negate(PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_AMOUNT, 1, ebufp),ebufp);
			netf_in_flist = PIN_FLIST_CREATE(ebufp);
			netf_in_flist = PIN_FLIST_COPY(s_r_flistp,ebufp);
			rcpt_pdp = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_POID, 1, ebufp);
			PIN_FLIST_FLD_COPY(s_r_flistp, PIN_FLD_ACCOUNT_OBJ, netf_in_flist, PIN_FLD_POID, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"NETFLIX REVERSAL FLIST INPUT FLIST", netf_in_flist);
			fm_mso_split_netflix_pymt(ctxp, netf_in_flist ,net_pymt_amt, &p_r_flistp, ebufp);	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"NETFLIX REVERSAL FLIST OUTPUT FLIST", p_r_flistp);
			netf_status = PIN_FLIST_FLD_GET(p_r_flistp, PIN_FLD_STATUS, 1, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp) || netf_status == NULL || *netf_status == 1)
			{
				PIN_ERRBUF_CLEAR(ebufp);
				status = 1;
				err_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, user_id, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53921", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Netflix Payment Reversal Failed", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
				goto CLEANUP;
			}
			else
			{
				mso_update_receipt_obj(ctxp, rcpt_pdp, &r_flistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"IS REVERSED FLAG WRITE OUTPUT FLIST", r_flistp);
				*o_flistpp = PIN_FLIST_COPY(p_r_flistp, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, netf_in_flist, PIN_FLD_REASON_ID, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_RECEIPT_NO, netf_in_flist, PIN_FLD_RECEIPT_NO, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"NETFLIX REVERSAL FLIST INPUT FLIST", netf_in_flist);
				//mso_op_netfr_gen_lc_event(ctxp,netf_in_flist,ebufp);
		              fm_mso_create_lifecycle_evt(ctxp, netf_in_flist, r_flistp, ID_NETFLIX_REVERSAL, ebufp );
		              if(PIN_ERRBUF_IS_ERR(ebufp))
		              {
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation Error");
					PIN_ERRBUF_CLEAR(ebufp);
				}
				goto CLEANUP;
			}
	}
	else
	{
		r_flistp = NULL;
		rcpt_pdp = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_POID, 1, ebufp);
		pay_type = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
		cheque_no = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_CHECK_NO, 1, ebufp);
		paid_amount = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_AMOUNT, 1, ebufp);
		cheque_dt = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);
		serv_type = PIN_FLIST_FLD_GET(s_r_flistp, MSO_FLD_SERVICE_TYPE, 1, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_SERVICE_TYPE, serv_type, ebufp);
		acct_pdp = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
		migrated_pymt = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_STATUS_FLAGS, 1, ebufp);
		if(migrated_pymt != NULL && *migrated_pymt == MIGRATED)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "It's a Migrated Payment");
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "It's a Migrated Payment", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			goto CLEANUP;
		}

		/*********************************************************
		Pay type and reason code sync -validation
		**********************************************************/
		if( pay_type && *pay_type != PAY_TYPE_CHECK && *reason_code == REASON_ID_CHK_BOUNCE
		  )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Invalid Combination, 'Check Bounce' applicable with cheque payment only");
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Invalid Combination, 'Check Bounce' applicable with cheque payment only", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			goto CLEANUP;
		}

		/*************************************************************
		 * Identifying the transaction ID
		 *************************************************************/
		mso_find_trans_id(ctxp, s_r_flistp, &r_flistp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"transaction id return flist", r_flistp);

		/***********************************************************
		 * Preparing the Input FLIST for PCM_OP_BILL_REVERSE
		 ***********************************************************/
		reason_code = (void *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON_ID, 0 , ebufp);
		amount = (void *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp);
		PIN_FLIST_FLD_COPY( s_r_flistp, PIN_FLD_PAY_TYPE, i_flistp, PIN_FLD_PAY_TYPE, ebufp);
		//Added to populate the bank name
		PIN_FLIST_FLD_COPY(s_r_flistp, PIN_FLD_BANK_NAME, i_flistp, PIN_FLD_BANK_NAME, ebufp);
		mso_pymt_reverse(ctxp, i_flistp, r_flistp, reason_code, amount, cheque_no, cheque_dt, paid_amount, &p_r_flistp, ebufp);
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
	    	{
//			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "mso_reverse_payment error raising reversal events", ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Receipt Details Not Found");
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Reversal Failed", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			goto CLEANUP;
	    	}
		else
		{
			//r_flistp = NULL;
			PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
			if (serv_type && *serv_type != 0)
			{	
				mso_process_lco_pymt_reversal(ctxp, i_flistp, acct_pdp, paid_amount, serv_type, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
					"Error while processing LCO payment reversal");
					PIN_ERRBUF_CLEAR(ebufp);
					status = 1;
					err_flistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, user_id, ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
					"Error while processing LCO payment reversal", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
					*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
					goto CLEANUP;
		    		}
			}
			mso_update_receipt_obj(ctxp, rcpt_pdp, &r_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Receipt Details(is_reversed flag) Update Failed");
				PIN_ERRBUF_CLEAR(ebufp);
				status = 1;
				err_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, user_id, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"Error while processing LCO payment reversal", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
				goto CLEANUP;
		    	}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"p_r_flistp", p_r_flistp);
			*o_flistpp = PIN_FLIST_COPY(p_r_flistp, ebufp);
		}
		PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_PAY_TYPE, ebufp);
	}
	/***********************************************************
	 * Results.
	 ***********************************************************/
	CLEANUP:
	if (PIN_ERRBUF_IS_ERR(ebufp))
    	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"mso_reverse_payment error raising reversal events");
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
				fm_mso_trans_abort(ctxp, d_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Aborted");
		}

		*o_flistpp = NULL;
    	}
    	else
	{	
		ret_status = PIN_FLIST_FLD_GET(*o_flistpp, PIN_FLD_STATUS, 1, ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_pymt_reverse_payment return flist", *o_flistpp);
		if (ret_status && *ret_status == 1)
		{
			//PIN_ERR_LOG_MSG(3,"debug0");
			if(local == LOCAL_TRANS_OPEN_SUCCESS )
			{
			//PIN_ERR_LOG_MSG(3,"debug1");
				fm_mso_trans_abort(ctxp, d_pdp, ebufp);
			//PIN_ERR_LOG_MSG(3,"debug2");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Aborted");
			//PIN_ERR_LOG_MSG(3,"debug3");
			}
		}
		else
		{
			//PIN_ERR_LOG_MSG(3,"debug4");
			if(local == LOCAL_TRANS_OPEN_SUCCESS )
			{
			//PIN_ERR_LOG_MSG(3,"debug5");
				fm_mso_trans_commit(ctxp, d_pdp, ebufp);
			//PIN_ERR_LOG_MSG(3,"debug6");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Committed");
			//PIN_ERR_LOG_MSG(3,"debug7");
			}
		}

	}
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&p_r_flistp, NULL);
	if(d_pdp)
		PIN_POID_DESTROY(d_pdp, ebufp);
	return;

}


/* This Module will retrieve the Payment Event details for the Customer 
 * from Receipt No in the /mso_receipt object. */
void mso_get_repeipt_info(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	u_int64			id = (u_int64)-1;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int32			s_flags = 256;
	int32			status = 0;
	poid_t			*pdp = NULL;
	poid_t			*user_id = NULL;
	u_int64			db = 0;
	char			*rcpt_no = NULL;


//	rcpt_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_RECEIPT_NO, 0, ebufp);
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	db = (u_int64)PIN_POID_GET_DB(user_id);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_receipt where F1 = V1 ", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_RECEIPT_NO, flistp, PIN_FLD_RECEIPT_NO, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_EVENT_OBJ, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_CHECK_NO, (void*)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_AMOUNT, (void*)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_NO, (void*)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_EFFECTIVE_T, (void*)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, MSO_FLD_SERVICE_TYPE, (void*)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void*)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS_FLAGS, (void*)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PAY_TYPE, (void*)NULL, ebufp); 
	//Added for chque reversal
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_BANK_NAME, (void*)NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"event search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"event search output flist",*r_flistpp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		status = 1;
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Event Search Error");
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Event Search Error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		return;
	}
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
}


/* This module will retrieve the transaction ID of the Payments from the Event POID */
void mso_find_trans_id(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	u_int64			id = (u_int64)0;
	u_int64			db = 0;
	pin_flist_t		*s_flistp = NULL;
	poid_t			*event_pdp = NULL;
	char			*event_type = NULL;
	char			*temp = NULL;
	poid_t			*pdp = NULL;
	int32			s_flags = 256;
	int32			status = 0;
	pin_flist_t		*st_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	
	event_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EVENT_OBJ, 0 , ebufp);
	if (!PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		db=(u_int64)PIN_POID_GET_DB(event_pdp);
		event_type = (void*)PIN_POID_GET_TYPE(event_pdp);
        
		temp = pin_malloc(200);
        memset(temp,'\0',sizeof(temp));

		s_flistp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		sprintf(temp,"select X from %s where F1 = V1", (char *)event_type);

		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, temp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)event_pdp, ebufp);

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		st_flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_PAYMENT, ebufp);
		PIN_FLIST_FLD_PUT(st_flistp, PIN_FLD_PAY_TYPE, (void *)NULL, ebufp);
		PIN_FLIST_FLD_PUT(st_flistp, PIN_FLD_TRANS_ID, (void *)NULL, ebufp);
   
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Trans-ID search input flist", s_flistp);

		/* Get the transaction id from the payment event obj */
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Trans-ID search output flist", *r_flistpp);

		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			status = 1;
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Transaction Search Error");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, event_pdp, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53022", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Transaction Search Error", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
			return;
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Receipt Details Not Found");
		return;
	}
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	if (temp)
	{
		pin_free(temp);
	}
}


/* This Module prepare the Input Flist & invoke the PCM_OP_BILL_REVERSE opcode */
void mso_pymt_reverse(
	pcm_context_t		*ctxp,
	pin_flist_t		*inp_flistp,
	pin_flist_t		*i_flistp,
	int			*reason_code,
	pin_decimal_t		*amount,
	char			*cheque_no,
	time_t			*cheque_dt,
	pin_decimal_t		*paid_amt,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	u_int64			id = (u_int64)1;
	u_int64			db = 0;
	pin_flist_t		*ip_flistp = NULL;
	poid_t			*event_pdp = NULL;
	poid_t			*pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*balgrp_pdp = NULL;
	poid_t			*srvice_pdp = NULL;
	pin_flist_t		*sr_flistp = NULL;
	pin_flist_t		*st_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*notif_flistp = NULL;
	pin_flist_t		*cheque_notif_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*inh_flistp = NULL;	
	pin_flist_t		*balgrp_flistp = NULL;
	pin_flist_t		*adj_in_flistp = NULL;
	pin_flist_t		*reas_flistp = NULL;
	pin_flist_t		*service_flistp = NULL;
	pin_flist_t		*rslt_flistp = NULL;
	pin_flist_t		*act_usagep = NULL;
	pin_flist_t		*bill_rev_rflist = NULL;

	int32			*pay_type = NULL;
	int32			*service_type_val = 0;
	int32			*res = NULL;
	int32			reason = 0;
	int32			status = 0;
	int32			notif1 = PYMT_REVERSAL_NOTIFICATION;
	int32			notif2 = NTF_CHEQUE_REVERSAL;
	int32			destroy_flistp = 0;
	int			reason_domain = 2;
	char			*trans_id = NULL;
	char			*event_type = NULL;
	char			*service_type = NULL;
	char			*descr = NULL;
	char			*program_name = NULL;
	char			*description = NULL;
	char			sys_description[256];
	time_t			time_now = 0;

	PIN_ERRBUF_CLEAR(ebufp);
	ip_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_pymt_reverse input flist", inp_flistp);

	sr_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	event_pdp = PIN_FLIST_FLD_GET(sr_flistp, PIN_FLD_POID, 0 , ebufp);
	service_type_val = PIN_FLIST_FLD_GET(inp_flistp, MSO_FLD_SERVICE_TYPE, 0 , ebufp);
	program_name = PIN_FLIST_FLD_GET(inp_flistp, PIN_FLD_PROGRAM_NAME, 0 , ebufp);

	description = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	memset(sys_description,'\0',256);
	if(description)
	{
		sprintf(sys_description,"%s%s","PAYMENT REVERSAL: ",description);		
	}
	else
	{
		sprintf(sys_description,"%s","PAYMENT REVERSAL");
	}
	db=(u_int64)PIN_POID_GET_DB(event_pdp);
	event_type = (void *)PIN_POID_GET_TYPE(event_pdp);

	st_flistp = PIN_FLIST_SUBSTR_GET(sr_flistp, PIN_FLD_PAYMENT, 0, ebufp);
	pay_type = PIN_FLIST_FLD_GET(st_flistp, PIN_FLD_PAY_TYPE, 0, ebufp);
	trans_id = PIN_FLIST_FLD_GET(st_flistp, PIN_FLD_TRANS_ID, 0, ebufp);
	pdp = PIN_POID_CREATE(db, "/account", id, ebufp);
	PIN_FLIST_FLD_PUT(ip_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(ip_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
	//PIN_FLIST_FLD_SET(ip_flistp, PIN_FLD_PROGRAM_NAME, "MSO_OP_BILL_REVERSE_PAYMENT", ebufp);
	if (strcmp(event_type, PYMT_EVT_CASH )==0)
	{
		
		if(description)
		{
			sprintf(sys_description,"%s FOR: %s","CASH PAYMENT REVERSAL",description);
		}
		else
		{
			sprintf(sys_description,"%s","CASH PAYMENT REVERSAL");
		}
		PIN_FLIST_FLD_SET(ip_flistp, PIN_FLD_DESCR, sys_description, ebufp);
	}else if (strcmp(event_type, PYMT_EVT_CHECK)==0)
	{
		//sprintf(sys_description,"%s",": Cash Payment Reversal");
		if(description)
		{
			sprintf(sys_description,"%s FOR: %s","CHEQUE PAYMENT REVERSAL",description);
		}
		else
		{
			sprintf(sys_description,"%s","CHEQUE PAYMENT REVERSAL");
		}
		PIN_FLIST_FLD_SET(ip_flistp, PIN_FLD_DESCR, sys_description, ebufp);
	}else if (strcmp(event_type, PYMT_EVT_TDS)==0)
	{
		if(description)
		{
			sprintf(sys_description,"%s FOR: %s","TDS PAYMENT REVERSAL",description);
		}
		else
		{
			sprintf(sys_description,"%s","TDS PAYMENT REVERSAL");
		}
		PIN_FLIST_FLD_SET(ip_flistp, PIN_FLD_DESCR, sys_description, ebufp);	
	}
	else if (strcmp(event_type, PYMT_EVT_PORTAL)==0)
	{
		if(description)
		{
			sprintf(sys_description,"%s FOR: %s","PORTAL PAYMENT REVERSAL",description);
		}
		else
		{
			sprintf(sys_description,"%s","PORTAL PAYMENT REVERSAL");
		}
		PIN_FLIST_FLD_SET(ip_flistp, PIN_FLD_DESCR, sys_description, ebufp);	
	}
	else 
	{
		if(description)
		{
			sprintf(sys_description,"%s FOR: %s","ONLINE PAYMENT REVERSAL",description);
		}
		else
		{
			sprintf(sys_description,"%s","ONLINE PAYMENT REVERSAL");
		}
		PIN_FLIST_FLD_SET(ip_flistp, PIN_FLD_DESCR, sys_description, ebufp);
	}

	flistp = PIN_FLIST_ELEM_ADD(ip_flistp, PIN_FLD_REVERSALS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PAYMENT_TRANS_ID, (void *)trans_id, ebufp);
	//PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, "MSO - Transaction Reversal", ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, sys_description, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PAY_TYPE, pay_type, ebufp);
	in_flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_INHERITED_INFO, ebufp);
	if ((strcmp(event_type, PYMT_EVT_CASH)==0) || (strcmp(event_type, PYMT_EVT_TDS)==0) || (strcmp(event_type, PYMT_EVT_PORTAL)==0) || (strcmp(event_type, PYMT_EVT_SP_AUTOPAY)==0))
	{
		inh_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_CASH_INFO, 1, ebufp);
	}else if (strcmp(event_type, PYMT_EVT_CHECK)==0)
	{
		inh_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_CHECK_INFO, 1, ebufp);
	}else 
		inh_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_WIRE_TRANSFER_INFO, 1, ebufp);
	
	time_now = pin_virtual_time(NULL);
//	PIN_FLIST_FLD_SET(inh_flistp, PIN_FLD_REASON_CODE, (void *)reason_code, ebufp);
	PIN_FLIST_FLD_SET(inh_flistp, PIN_FLD_EFFECTIVE_T, &time_now, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ERROR PAYMENTINFO Info");
		return;
	}

	reas_flistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_REASONS, 0, ebufp);
	PIN_FLIST_FLD_SET(reas_flistp, PIN_FLD_REASON_ID, reason_code, ebufp);
	PIN_FLIST_FLD_SET(reas_flistp, PIN_FLD_REASON_DOMAIN_ID, &reason_domain, ebufp);

	
	 /***********************************************************
	 * Invoking PCM_OP_BILL_REVERSE
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Payment Reversal Input flist", ip_flistp);

	PCM_OP(ctxp, PCM_OP_BILL_REVERSE, 0 , ip_flistp, &bill_rev_rflist, ebufp);

	err_flistp = PIN_FLIST_CREATE(ebufp);
	// Payment Reversal Failure Case
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Reversal Error");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, event_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53023", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Reversal Error", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto Cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Payment reversal output flist", bill_rev_rflist);
	
	if (bill_rev_rflist)
	{
		rs_flistp = PIN_FLIST_ELEM_GET(bill_rev_rflist, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (rs_flistp)
		{
			res   = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_RESULT, 1, ebufp);
			descr = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_DESCR, 1, ebufp);
		}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Payment reversal result flist", rs_flistp);
	//PIN_ERRBUF_CLEAR(ebufp);
	
	if (res && *res == 0) 
	{
		// Payment Reversal Success Case
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, event_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, descr, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "E1");
		//Service Object Retrieval for Notifiaction
		balgrp_flistp = PIN_FLIST_ELEM_GET(rs_flistp, PIN_FLD_BAL_IMPACTS, 0, 0, ebufp);
//		balgrp_pdp = PIN_FLIST_FLD_GET(balgrp_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
		acct_pdp = PIN_FLIST_FLD_GET(balgrp_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
//		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "BAL GRP", balgrp_pdp);
//		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "ACCT PDP", acct_pdp);

		// Lifecycle Event for Payment Reversal
		PIN_FLIST_FLD_SET(rs_flistp, PIN_FLD_NAME, PAYMENT_REVERSAL, ebufp );
		PIN_FLIST_FLD_SET(rs_flistp, PIN_FLD_AMOUNT, NULL, ebufp );

		fm_mso_create_lifecycle_evt(ctxp, inp_flistp, rs_flistp, ID_PAYMENT_REVERSAL, ebufp );
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			//status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation Error");
			PIN_ERRBUF_CLEAR(ebufp);
		}
		fm_mso_retrieve_service_obj(ctxp, acct_pdp,service_type_val, &rs_flistp, ebufp);
		
		flistp = PIN_FLIST_FLD_TAKE(rs_flistp, PIN_FLD_RESULTS, 0, ebufp);
		destroy_flistp = 1;
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERRBUF_CLEAR(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Reversed, No Service Object Found for Notification!");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, event_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, "Payment Reversed, No Service Object Found for Notification!", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			return;
        	}

                PIN_FLIST_DESTROY_EX(&rs_flistp, ebufp);

		// Preparing Input Flist and calling Notification API
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling Notification API");
		notif_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_FLAGS, &notif1, ebufp);
		PIN_FLIST_FLD_COPY(flistp, PIN_FLD_POID, notif_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(balgrp_flistp, PIN_FLD_AMOUNT, notif_flistp, PIN_FLD_AMOUNT, ebufp);
		PIN_FLIST_FLD_COPY(bill_rev_rflist, PIN_FLD_BATCH_ID, notif_flistp, PIN_FLD_TRANS_ID, ebufp);
		PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_MODE, pay_type, ebufp);
		if (*pay_type == PAY_TYPE_CHECK)
		{
			PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_CHECK_NO, cheque_no, ebufp);
			//added for cheque payment reversal
			if(reason_code && *reason_code == REASON_ID_CHK_BOUNCE)
			{
				PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_EFFECTIVE_T, cheque_dt, ebufp);
				PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_AMOUNT, paid_amt, ebufp);
				PIN_FLIST_FLD_COPY(inp_flistp, PIN_FLD_BANK_NAME, notif_flistp, PIN_FLD_BANK_NAME, ebufp);
				PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_FLAGS, &notif2, ebufp);
			}
		}
		PIN_FLIST_FLD_COPY(bill_rev_rflist, PIN_FLD_END_T, notif_flistp, PIN_FLD_ACK_TIME, ebufp);
		PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_REASON, reason_code, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification input flist", notif_flistp);
		PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, notif_flistp, &rs_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification output flist", rs_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, event_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, "Payment Reversed, Notification Failed!", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		}
		/*	
		if (*pay_type == PAY_TYPE_CHECK)
		{
			cheque_notif_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(cheque_notif_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_COPY(flistp, PIN_FLD_POID, cheque_notif_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_FLD_SET(cheque_notif_flistp, PIN_FLD_FLAGS, &notif2, ebufp);
			PIN_FLIST_FLD_SET(cheque_notif_flistp, PIN_FLD_AMOUNT, paid_amt, ebufp);
			PIN_FLIST_FLD_SET(cheque_notif_flistp, PIN_FLD_CHECK_NO, cheque_no, ebufp);
			PIN_FLIST_FLD_SET(cheque_notif_flistp, PIN_FLD_EFFECTIVE_T, cheque_dt, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification input flist", cheque_notif_flistp);
			PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, cheque_notif_flistp, &rs_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification output flist", rs_flistp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, event_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, "Payment Reversed, Notification Failed!", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
			}
		}
		*/
		//Applying the fine amount if the its Cheque Bounce
		if (*reason_code == 1)
		{
			//  Input FLIST Prepared for OP_ACT_USAGE Opcode
			balgrp_pdp = PIN_FLIST_FLD_GET(balgrp_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
			if(balgrp_pdp != NULL)
			{
				fm_mso_utils_get_service_from_balgrp(ctxp, balgrp_pdp, &service_flistp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"Error while getting service details error", ebufp);
					PIN_FLIST_DESTROY_EX(&service_flistp, NULL);
					goto Cleanup;
				}
				rslt_flistp = PIN_FLIST_ELEM_GET(service_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
				if(rslt_flistp != NULL)
				{
					srvice_pdp = PIN_FLIST_FLD_GET(rslt_flistp, PIN_FLD_POID, 0, ebufp);
					service_type = (char *)PIN_POID_GET_TYPE(srvice_pdp);
				}
			}

			/* Create a Pre-Rated Debit impact of the input refund amount */
			fm_mso_utils_create_pre_rated_impact(ctxp, acct_pdp, srvice_pdp, program_name,sys_description,
					CHK_BOUNCE_EVENT, amount, &act_usagep, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error while creating check bounce impact.");
				goto Cleanup;
			}

			// Life cycle Event for Check Bounce Penalty
			PIN_FLIST_FLD_SET(act_usagep, PIN_FLD_NAME, CQ_BOUNCE_PENALTY, ebufp );
			PIN_FLIST_FLD_SET(act_usagep, PIN_FLD_AMOUNT, NULL, ebufp );
			fm_mso_create_lifecycle_evt(ctxp, inp_flistp, act_usagep, ID_PAYMENT_REVERSAL, ebufp );
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				//status = 1;
				PIN_ERRBUF_CLEAR(ebufp);
			}
		}
	}
	else
	{
		// Payment Reversal Failure Case
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Payment Reversal Error");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, event_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53023", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, descr, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Copying the error flist if present");
	*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
	//*r_flistpp = err_flistp;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After Copying the error flist if present", *r_flistpp);
Cleanup:
	PIN_FLIST_DESTROY_EX(&ip_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&adj_in_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&notif_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&service_flistp, ebufp);
	if (rs_flistp) 
		PIN_FLIST_DESTROY_EX(&rs_flistp, ebufp);
	if (destroy_flistp)
	{
		PIN_FLIST_DESTROY_EX(&flistp, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&bill_rev_rflist, ebufp);
//	PIN_POID_DESTROY(acct_pdp, ebufp);
//	PIN_POID_DESTROY(balgrp_pdp, ebufp);
	return ;
}

// This module retrieves the Service object using Account POID
void fm_mso_retrieve_service_obj(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	int32			*service_type,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)-1;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int32			s_flags = 512;
	int64			db = 0;
	poid_t			*pdp = NULL;
	poid_t			*svc_pdp = NULL;

	db = PIN_POID_GET_DB(acct_pdp);
	if(*service_type == 1)
	{
		svc_pdp = PIN_POID_CREATE(db, SERVICE_TYPE_BB, id, ebufp);
	}
	else
	{
		svc_pdp = PIN_POID_CREATE(db, SERVICE_TYPE_CATV, id, ebufp);
	}

	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /service where F1 = V1 and F2.type = V2 ", ebufp);


        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, svc_pdp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, (void *)NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search output flist",*r_flistpp);
		
	PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
	PIN_POID_DESTROY(svc_pdp, ebufp);
	return;
}


// This module updates the is_reversed column of payment receipt table, if payment is reversed successfully
void mso_update_receipt_obj(
	pcm_context_t		*ctxp, 
	poid_t			*rcpt_pdp,
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*in_flistp = NULL;
	int			flag = 1;

	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, rcpt_pdp, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_FLAGS, &flag, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Update Receipt input flist", in_flistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, in_flistp, ret_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Update Receipt output flist",*ret_flistpp);

	PIN_FLIST_DESTROY_EX(&in_flistp, ebufp);
return;
}

static void
mso_process_lco_pymt_reversal(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	poid_t		*acct_pdp,
	pin_decimal_t	*amount,
	int32		*serv_type,
	pin_errbuf_t	*ebufp
)
{
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*serv_flistp = NULL;
	pin_flist_t		*charge_flistp = NULL;
	pin_flist_t		*serv_res_flistp = NULL;
	pin_flist_t		*bb_info_flistp = NULL;
	int32			*serv_objtype = NULL;
	poid_t			*bal_grp_pdp = NULL;

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside the function mso_process_lco_pymt_reversal");

	fm_mso_utils_get_service_details(ctxp, acct_pdp, *serv_type, &serv_flistp, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After fm_mso_utils_get_service_details");
	if (PIN_ERRBUF_IS_ERR(ebufp) || serv_flistp == NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error gettig the service details !!");
		goto Cleanup;
	}
	serv_res_flistp = PIN_FLIST_ELEM_GET(serv_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (serv_res_flistp == NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No services are attached to the account !!");
		goto Cleanup;
	}
	bb_info_flistp = PIN_FLIST_ELEM_GET(serv_res_flistp, MSO_FLD_BB_INFO, PIN_ELEMID_ANY, 1, ebufp);
	if (bb_info_flistp == NULL)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No MSO_FLD_BB_INFO under the service !!");
		goto Cleanup;
	}

	serv_objtype = PIN_FLIST_FLD_GET(bb_info_flistp, MSO_FLD_COMMISION_MODEL, 0, ebufp);
	bal_grp_pdp = PIN_FLIST_FLD_GET(serv_res_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);
	if (*serv_objtype == 1 || *serv_objtype == 2)
	{
		in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		charge_flistp = PIN_FLIST_ELEM_ADD(in_flistp, PIN_FLD_CHARGES, 0, ebufp);
		PIN_FLIST_FLD_SET(charge_flistp, PIN_FLD_AMOUNT, amount, ebufp);
		fm_mso_pymt_process_lco_reversal(ctxp, in_flistp, i_flistp, bal_grp_pdp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error processing LCO balance transfer !!");
			goto Cleanup;
		}
	}

	Cleanup:
		PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&serv_flistp, NULL);

	return;
}

/**************************************************************************
* Function:		fm_mso_pymt_process_lco_reversal()
* Owner:		Hrish Kulkarni
* Decription:	For	transfering	balance	from LCO to	source account.
**************************************************************************/
static void fm_mso_pymt_process_lco_reversal(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			*ip_flistp,
	poid_t				*bal_grp_pdp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*profile_iflistp = NULL;
	pin_flist_t			*profile_oflistp = NULL;
	pin_flist_t			*custcare_flistp = NULL;
	pin_flist_t			*lco_acct_flistp = NULL;
	pin_flist_t			*charge_flistp = NULL;
	pin_flist_t			*ret_flistp = NULL;
	pin_flist_t			*result_flistp = NULL;
	pin_flist_t			*lco_acnt_info = NULL;

	pin_decimal_t			*amount = NULL;
	pin_decimal_t			*dsc_amount = NULL;
	poid_t				*acct_pdp = NULL;
	poid_t				*lco_acctp = NULL;
	poid_t				*lco_balp = NULL;
	int32				srch_type = LCO_CUST_CARE;

	void				*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"Error before calling fm_mso_pymt_process_lco_reversal", ebufp);
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_pymt_process_lco_reversal");

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0,	ebufp);
	charge_flistp =	PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_CHARGES, PIN_ELEMID_ANY, 0, ebufp);
	amount = PIN_FLIST_FLD_GET(charge_flistp, PIN_FLD_AMOUNT, 0, ebufp);

	profile_iflistp= PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(profile_iflistp, PIN_FLD_POID, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(profile_iflistp, PIN_FLD_OBJECT, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(profile_iflistp, PIN_FLD_PROFILE_DESCR, "/profile/customer_care", ebufp);
	PIN_FLIST_FLD_SET(profile_iflistp, PIN_FLD_SEARCH_TYPE, &srch_type,	ebufp);

	/**********************************************************
	* Read the profile of account to get the LCO details
	***********************************************************/
	fm_mso_get_profile_info(ctxp, profile_iflistp, &profile_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error reading the /profile class !!");
		goto CLEANUP;
	}
	if (profile_oflistp!= NULL)
	{
		custcare_flistp = PIN_FLIST_SUBSTR_GET(profile_oflistp,	PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
		lco_acctp = PIN_FLIST_FLD_GET(custcare_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
		if (lco_acctp)
		{
			fm_mso_get_account_info(ctxp, lco_acctp, &lco_acnt_info, ebufp);
			vp = PIN_FLIST_ELEM_ADD(ip_flistp, MSO_FLD_LCOINFO, 0, ebufp);
			PIN_FLIST_FLD_SET(vp, PIN_FLD_AMOUNT, amount, ebufp );
			PIN_FLIST_FLD_COPY(lco_acnt_info, PIN_FLD_ACCOUNT_NO, vp, PIN_FLD_ACCOUNT_NO, ebufp );
			PIN_FLIST_DESTROY_EX(&profile_iflistp, NULL);
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"LCO object link does not exist !!");
		pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_NONEXISTANT_ELEMENT, 0, 0, 0);
		goto CLEANUP;
	}
	/**********************************************
	* Get the account balance group	of the LCO account
	***********************************************/
	fm_mso_utils_get_billinfo_bal_details(ctxp, lco_acctp, BILLINFO_ID_WS,
						&lco_acct_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		goto CLEANUP;
	}
	result_flistp = PIN_FLIST_ELEM_GET(lco_acct_flistp, PIN_FLD_EXTRA_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if(result_flistp == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"LCO Incollect Balance Group Does not exist !!");
		pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_NONEXISTANT_ELEMENT, 0, 0, 0);
		goto CLEANUP;
	}
	lco_balp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);

	/**********************************************************
	* Do the balance transfer from end_customer	to LCO account
	***********************************************************/
	fm_mso_utils_balance_transfer(ctxp, lco_acctp, acct_pdp, lco_balp, bal_grp_pdp,
				pbo_decimal_abs(amount,ebufp), &ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error transfering the balance !!");
		goto CLEANUP;
	}

	// Lifecycle event for source account
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
	dsc_amount = pbo_decimal_negate(amount,ebufp);	

	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_NAME, BAL_TRANS_LCO_SOURCE, ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_AMOUNT, dsc_amount, ebufp );
	fm_mso_create_lifecycle_evt(ctxp, ip_flistp, ret_flistp, ID_PAYMENT_REVERSAL, ebufp );
	//mso_op_pymt_reverse_gen_lc_event(ctxp, ip_flistp, ret_flistp, BAL_TRANS_LCO_SOURCE, 
	//			dsc_amount, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Lifecycle event creation !!");
		goto CLEANUP;
	}
	// Lifecycle event for Destination(LCO) account
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_NAME, BAL_TRANS_LCO_DEST, ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_AMOUNT, amount, ebufp );
	fm_mso_create_lifecycle_evt(ctxp, ip_flistp, ret_flistp, ID_PAYMENT_REVERSAL, ebufp );
	//mso_op_pymt_reverse_gen_lc_event(ctxp, ip_flistp, ret_flistp, BAL_TRANS_LCO_DEST, amount, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Lifecycle event creation !!");
		goto CLEANUP;
	}
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&profile_iflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&profile_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&lco_acct_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, ebufp);
		if(dsc_amount)
			pbo_decimal_destroy(&dsc_amount);
	return;
}

///****************************************************************
//* This function creates the lifecycle event for payment reversal 
//****************************************************************/
//static void
//mso_op_pymt_reverse_gen_lc_event(
//	pcm_context_t           *ctxp,
//	pin_flist_t             *i_flistp,
//	pin_flist_t             *s_oflistp,
//	char                    *disp_name,
//	pin_decimal_t		*ip_amount,
//	pin_errbuf_t            *ebufp)
//{
//	pin_flist_t             *lc_iflistp = NULL;
//	pin_flist_t             *lc_temp_flistp = NULL;
//	pin_flist_t		*balimp_flistp = NULL;
//	pin_flist_t		*sub_balimp_flistp = NULL;
//	pin_flist_t		*sbalnces_flistp = NULL;
//	pin_flist_t             *res_flistp = NULL;
//	poid_t                  *acct_pdp = NULL;
//	char                    *event = "/event/activity/mso_lifecycle/payment_reversal";
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//		return;
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_pymt_reverse_gen_lc_event Input Flist:", i_flistp);
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_pymt_reverse_gen_lc_event s_oflistp Flist:", s_oflistp);
//
//	lc_iflistp = PIN_FLIST_CREATE(ebufp);
//	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_RVERSAL, 0, ebufp);
//
//	if(strcmp(disp_name, PAYMENT_REVERSAL) == 0)
//	{
//		acct_pdp = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
//		PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);
//		balimp_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_BAL_IMPACTS, 0, 1, ebufp);
//		if(balimp_flistp)
//			PIN_FLIST_FLD_COPY(balimp_flistp, PIN_FLD_AMOUNT, lc_temp_flistp, PIN_FLD_AMOUNT, ebufp);
//	}
//	else if(strcmp(disp_name, CQ_BOUNCE_PENALTY) == 0)
//	{
//		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, 
//							PIN_ELEMID_ANY, 1, ebufp);
//		if(res_flistp)
//			sub_balimp_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_SUB_BAL_IMPACTS, 
//							PIN_ELEMID_ANY, 1, ebufp);
//		if(sub_balimp_flistp)
//		{
//			sbalnces_flistp = PIN_FLIST_ELEM_GET(sub_balimp_flistp, PIN_FLD_SUB_BALANCES, 
//							PIN_ELEMID_ANY, 1, ebufp);
//			if(sbalnces_flistp)
//				PIN_FLIST_FLD_COPY(sbalnces_flistp, PIN_FLD_AMOUNT, lc_temp_flistp, 
//						PIN_FLD_AMOUNT, ebufp);
//		}
//		PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);
//		acct_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
//	}
//	else if(strcmp(disp_name, BAL_TRANS_LCO_SOURCE) ==0)
//	{
//		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_EVENTS, 0, 1, ebufp);
//		if(res_flistp)
//			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_EVENT_OBJ, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);
//		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, ip_amount, ebufp);
//		acct_pdp = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
//	}
//	else if(strcmp(disp_name, BAL_TRANS_LCO_DEST) ==0)
//	{
//		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_EVENTS, 1, 1, ebufp);
//		if(res_flistp)
//			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_EVENT_OBJ, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);
//		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, ip_amount, ebufp);
//		acct_pdp = PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
//	}
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, disp_name, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, lc_temp_flistp, PIN_FLD_USERID, ebufp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Event Flist:", s_oflistp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event Event Flist:", lc_iflistp);
//
//	fm_mso_utils_gen_event(ctxp, acct_pdp, NULL, "MSO_OP_PYMT_REVERSE_PAYMENT",
//							disp_name, event, lc_iflistp, ebufp);
//
//	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
//
//	return;
//}



