/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 * Version 1.0: 08-10-2013: HarishKulkarni: Added the MSO Customization of
 * Refund Processing using MSO_OP_PYMT_PROCESS_REFUND opcode and refund object creation
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_process_refund.c /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_PROCESS_REFUND operation. 
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
#include "ops/bal.h"
#include "mso_ops_flds.h" 
#include "pin_currency.h"
#include "mso_lifecycle_id.h"

#define DEP_STATUS_CLOSED 1
#define REF_STATUS_NEW 0
#define SEQ_TYPE_REFUND "MSO_SEQUENCE_TYPE_REFUND"
#define EVENT_MSO_REFUND_DEBIT "/event/billing/mso_refund_debit"
#define REFUND_DESCR "mso_pymt_process_refund"
#define PROCESS_REFUND "PROCESS_REFUND"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_pymt_process_refund(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

static void mso_retrieve_refunded_deposit(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void mso_create_refund_object(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        poid_t                  *d_pdp,
        poid_t                  *a_pdp,
        poid_t                  *s_pdp,
        pin_flist_t             *adj_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

static void mso_op_pymt_process_refund(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);


static void mso_op_get_acct_service_details(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	poid_t				**a_pdpp,
	poid_t				**s_pdpp,
	poid_t				**b_pdpp,
	pin_errbuf_t		*ebufp);

static void
mso_fld_proc_refund_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	poid_t			*a_pdp,
	pin_flist_t		*ref_flistp,
	pin_decimal_t		*tot_amount,
	pin_flist_t		*refret_flistp,
	char                    *disp_name,
	pin_errbuf_t            *ebufp);

static int mso_op_validate_bal_grp(
	pcm_context_t		*ctxp,
	poid_t			*a_pdp,
	poid_t			*b_pdp,
	pin_decimal_t		*amount,
	pin_errbuf_t		*ebufp);

PIN_EXPORT void
mso_pymt_process_refund_adjustment(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        poid_t                  *acct_pdp,
        poid_t                  *service_pdp,
        poid_t                  *bal_grp_pdp,
	char			*sys_descr,
	pin_decimal_t		*amount,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

//PIN_IMPORT
//void fm_mso_utils_gen_event(
//	pcm_context_t   *ctxp,
//	poid_t          *acct_pdp,
//	poid_t          *serv_pdp,
//	char            *program,
//	char            *descr,
//	char            *event_type,
//	pin_flist_t     *in_flistp,
//	pin_errbuf_t    *ebufp);

PIN_IMPORT
void fm_mso_utils_read_any_object(
	pcm_context_t           *ctxp,
	poid_t                  *objp,
	pin_flist_t             **out_flistpp,
	pin_errbuf_t            *ebufp);


PIN_IMPORT void 
fm_mso_utils_create_pre_rated_impact(
	pcm_context_t		*ctxp,
	poid_t				*acct_pdp,
	poid_t				*service_pdp,
	char				*program_name,
	char				*sys_description,
	char				*event_type,
	pin_decimal_t		*amount,
	pin_flist_t		**au_oflistpp,
	pin_errbuf_t		*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PYMT_PROCESS_REFUND operation.
 *******************************************************************/
void
mso_pymt_process_refund(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	int			status = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_PROCESS_REFUND) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_pymt_process_refund opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_process_refund input flist", i_flistp);

	/***********************************************************
	 * Call the routine to process refund.
	 ***********************************************************/
	mso_op_pymt_process_refund(ctxp, i_flistp, &r_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unknown Error occured in MSO_OP_PYMT_PROCESS_REFUND.");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"Unknown Error occured in MSO_OP_PYMT_PROCESS_REFUND.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	*o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_process_refund return flist", *o_flistpp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	return;
}

void mso_op_pymt_process_refund(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*dep_oflistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*acct_flistp = NULL;
	pin_flist_t		*adj_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*lc_in_flist = NULL;

	poid_t			*a_pdp = NULL;
	poid_t			*d_pdp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*b_pdp = NULL;
	poid_t			*i_pdp = NULL;
	int32			*ref_type = NULL;
	char			msg[256];
	char			*dep_no = NULL;
	char			*acct_number = NULL;
	int			status = 0;
	pin_decimal_t		*amount = NULL;
	int			has_valid_credit = 0;
	char			*description = NULL;
	char			sys_description[256];
	char			*program_name = NULL;
	void			*vp = NULL;
	// Added to enrich sys description 
	char 		*bank_name = NULL;
	char 		*check_no  = NULL;
	char		check_date[30] = "";
	time_t		eff_t		= 0;
	struct 		tm lc;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Unknown input error!!");
		goto CLEANUP;
	}

	amount = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_REFUND_AMOUNT, 0, ebufp);
	ref_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE, 1, ebufp);
	dep_no = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DEPOSIT_NO, 1, ebufp);
	acct_number = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	description = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	// Getting bank details
	bank_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BANK_NAME, 1, ebufp);
	check_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHECK_NO, 1, ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);
	if(vp)
	eff_t = *(time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);
	// Getting bank details end
	memset(sys_description,'\0',256);
	if(description)
	{
		sprintf(sys_description,"%s%s","PROCESS REFUND: ",description);		
	}
	else
	{
		sprintf(sys_description,"%s","PROCESS REFUND");
	}
	// Added to enrich description	
	if(bank_name && check_no && check_date )
	{
		PIN_ERR_LOG_MSG(3, "Entering Cheque block");		
		lc = *localtime(&eff_t);
		strftime(check_date, 80, "%x", &lc);
		sprintf(sys_description +strlen(sys_description)," BN:%s CN:%s DT:%s ", bank_name, check_no, check_date);
		PIN_ERR_LOG_MSG(3, sys_description);
	}
	// Added to enrich description	 
	/* Get the account & service poid from the input flist */
	mso_op_get_acct_service_details(ctxp, i_flistp, &a_pdp, &s_pdp, &b_pdp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Error while searching account and service details.");
		goto CLEANUP;
	}

	// The refund to be processed only if account has enough credit balance.
	has_valid_credit = mso_op_validate_bal_grp(ctxp, a_pdp, b_pdp, amount, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Error while validating the balance group details.");
		goto CLEANUP;
	}
	if (has_valid_credit == 0)
	{
		status = 1;
		sprintf(msg, "Account does not have enough credit to process refund.");
		goto CLEANUP;

	}
	if(dep_no != NULL && (strcmp(dep_no,"") != 0))
	{
		/* Get the Deposit and account details */
		mso_retrieve_refunded_deposit(ctxp, i_flistp, &dep_oflistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			sprintf(msg, "Error while getting deposit details");
			goto CLEANUP;
		}
		res_flistp = PIN_FLIST_ELEM_GET(dep_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if (res_flistp == NULL)
		{
			status = 1;
			sprintf(msg, "No Deposit details available for the refund");
			goto CLEANUP;
		}
		d_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);
	}
	/* Create a Pre-Rated Debit impact of the input refund amount */
//	fm_mso_utils_create_pre_rated_impact(ctxp, a_pdp, s_pdp, program_name,sys_description,
//					EVENT_MSO_REFUND_DEBIT, amount, &act_usagep, ebufp);
	mso_pymt_process_refund_adjustment(ctxp, i_flistp, a_pdp, s_pdp, b_pdp, sys_description, amount, 
						&adj_flistp, ebufp); 

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Error while creating refund debit impact.");
		goto CLEANUP;
	}
	/* Call the function to create a refund object */
	mso_create_refund_object(ctxp, i_flistp, d_pdp, a_pdp, s_pdp, adj_flistp, &ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Error while creating refund object");
		goto CLEANUP;
	}



	/* Create a lifecycle event for Deposit Refund */
	//mso_fld_proc_refund_gen_lc_event(ctxp, i_flistp, a_pdp, act_usagep, amount, ret_flistp, PROCESS_REFUND, ebufp);
	lc_in_flist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(lc_in_flist, PIN_FLD_POID, a_pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_NO, lc_in_flist, PIN_FLD_ACCOUNT_NO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, lc_in_flist, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, lc_in_flist, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(ret_flistp, MSO_FLD_DEPOSIT_NO, lc_in_flist, MSO_FLD_DEPOSIT_NO, ebufp);
	PIN_FLIST_FLD_PUT(lc_in_flist, PIN_FLD_AMOUNT, (pbo_decimal_negate(
	(PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_REFUND_AMOUNT, 0, ebufp)),
	ebufp)), ebufp)
	vp = (pin_flist_t*)PIN_FLIST_ELEM_ADD(lc_in_flist, PIN_FLD_DEPOSITS, 0, ebufp);
	PIN_FLIST_FLD_SET(vp, PIN_FLD_SERVICE_OBJ, s_pdp, ebufp);
	PIN_FLIST_FLD_COPY(ret_flistp, MSO_FLD_DEPOSIT_NO, vp, MSO_FLD_DEPOSIT_NO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, vp, PIN_FLD_POID, ebufp);

	fm_mso_create_lifecycle_evt(ctxp, lc_in_flist, NULL, ID_REFUND, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}


	sprintf(msg, "Refund Process Success !!");
	err_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(ret_flistp, PIN_FLD_POID, err_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, msg, ebufp);
	*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Return flist is:",*r_flistpp);

	CLEANUP:
		if (PIN_ERRBUF_IS_ERR(ebufp) || status == 1)
		{
			status = 1;
			PIN_ERRBUF_CLEAR(ebufp);
			err_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, msg);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53022", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, msg, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		}
		PIN_FLIST_DESTROY_EX(&dep_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&adj_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&lc_in_flist, NULL);   
		
		if (a_pdp != (poid_t *)NULL)
		{
			PIN_POID_DESTROY(a_pdp,NULL);
		}
		if (s_pdp != (poid_t *)NULL)
		{
			PIN_POID_DESTROY(s_pdp,NULL);
		}
	
	return;
}

/**************************************************************************
* Function:     mso_create_refund_object()
* Owner:        Hrish Kulkarni
* Decription:   For creating the /mso_refund object
*
**************************************************************************/
static void mso_create_refund_object(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	poid_t			*d_pdp,
	poid_t			*a_pdp,
	poid_t			*s_pdp,
	pin_flist_t		*adj_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*co_iflistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*seq_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	poid_t			*rfnd_pdp = NULL;
	poid_t			*i_pdp = NULL;
	poid_t			*evt_pdp = NULL;
	int64			db = 0;
	int64			id = -1;
	int32			flags = REF_STATUS_NEW;
	char			*ref_str = NULL;

	db = PIN_POID_GET_DB(a_pdp);

        res_flistp = PIN_FLIST_ELEM_GET(adj_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
	if (res_flistp)
	{
        	bal_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_BAL_IMPACTS, 0, 0, ebufp);
		if (bal_flistp)
        		i_pdp = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);
        	evt_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);
	}

	co_iflistp = PIN_FLIST_CREATE(ebufp);
	rfnd_pdp = PIN_POID_CREATE(db, "/mso_refund", id, ebufp);
	PIN_FLIST_FLD_PUT(co_iflistp, PIN_FLD_POID, rfnd_pdp, ebufp);
	PIN_FLIST_FLD_SET(co_iflistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
	PIN_FLIST_FLD_SET(co_iflistp, PIN_FLD_SERVICE_OBJ, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(co_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
	if (d_pdp)
		PIN_FLIST_FLD_SET(co_iflistp, PIN_FLD_DEPOSIT_OBJ, d_pdp, ebufp);
	PIN_FLIST_FLD_SET(co_iflistp, PIN_FLD_ITEM_OBJ, i_pdp, ebufp);
	PIN_FLIST_FLD_SET(co_iflistp, PIN_FLD_EVENT_OBJ, evt_pdp, ebufp);

	/* Get the Sequence for refund reference number */
	seq_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(seq_flistp, PIN_FLD_POID, a_pdp, ebufp);
	PIN_FLIST_FLD_SET(seq_flistp, PIN_FLD_NAME, SEQ_TYPE_REFUND, ebufp);
	fm_mso_utils_sequence_no(ctxp, seq_flistp, &r_flistp, ebufp);
	ref_str = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STRING, 0, ebufp);
	PIN_FLIST_FLD_SET(co_iflistp, MSO_FLD_REFUND_NO, ref_str, ebufp);

	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_REFUND_DATE, co_iflistp, MSO_FLD_REFUND_DATE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_REFUND_AMOUNT, co_iflistp, MSO_FLD_REFUND_AMOUNT, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_ACCOUNT_NO, co_iflistp, PIN_FLD_BANK_ACCOUNT_NO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_ACCOUNT_OWNER, co_iflistp, MSO_FLD_ACCOUNT_OWNER, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_NAME, co_iflistp, PIN_FLD_BANK_NAME, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_CREDIT_TYPE, co_iflistp, MSO_FLD_CREDIT_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, co_iflistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TYPE, co_iflistp, PIN_FLD_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, co_iflistp, PIN_FLD_DESCR, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHECK_NO, co_iflistp, PIN_FLD_CHECK_NO, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REFERENCE_ID, co_iflistp, PIN_FLD_REFERENCE_ID, ebufp);
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Refund Object Creation Input flist", co_iflistp);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, co_iflistp, r_flistpp, ebufp);
	if (*r_flistpp)
	{
		PIN_FLIST_FLD_SET(*r_flistpp, MSO_FLD_DEPOSIT_NO, ref_str, ebufp );
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Refund Object Creation Output flist", *r_flistpp);
	
	PIN_FLIST_DESTROY_EX(&co_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&seq_flistp, NULL);

	return;
}

/**************************************************************************
* Function:     mso_retrieve_refunded_deposit()
* Owner:        Hrish Kulkarni
* Decription:   This module retrieves the deposit details which was refunded
*
**************************************************************************/
static void mso_retrieve_refunded_deposit(
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
	int32			i_status = DEP_STATUS_CLOSED; // 0-Open,1-Closed
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

static void mso_op_get_acct_service_details(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	poid_t			**a_pdpp,
	poid_t			**s_pdpp,
	poid_t			**b_pdpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*s_iflistp = NULL;
	pin_flist_t			*s_oflistp = NULL;
	pin_flist_t			*arg_flistp = NULL;
	pin_flist_t			*res_flistp = NULL;
	poid_t				*s_pdp = NULL;
	poid_t				*pdp = NULL;
	poid_t				*acct_pdp = NULL;
	poid_t				*serv_pdp = NULL;
	poid_t				*bal_pdp = NULL;
	int32				flags = 256;
	char				*catv_template = "select X from /service/catv 1, /account 2 "
									"where 1.F1 = 2.F2 and 2.F3 = V3 ";
	char				*bb_template = "select X from /service 1, /account 2 "
									"where 1.F1 = 2.F2 and 2.F3 = V3 and service_t.poid_type = '/service/telco/broadband'";
	char				*acct_no = NULL;
	int64				db = 0;
	int32				*serv_type = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"mso_op_get_acct_serice_details input error!!");
		goto CLEANUP;
	}

	serv_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0 , ebufp);
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0 , ebufp);
	acct_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	db = PIN_POID_GET_DB(pdp);
	s_iflistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	PIN_FLIST_FLD_PUT(s_iflistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
	if (*serv_type == 0) //catv
	{
		PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, catv_template, ebufp);
	}
	else if(*serv_type == 1) //broadband
	{
		PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, bb_template, ebufp);
	}
	arg_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_NO, acct_no, ebufp);

	res_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_op_get_acct_serice_details search input flist", s_iflistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_op_get_acct_serice_details search output flist", s_oflistp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "mso_op_get_acct_serice_details search error!!");
		goto CLEANUP;
	}
	PIN_FLIST_DESTROY_EX(&s_iflistp, ebufp);

	res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (res_flistp !=NULL)
	{
		serv_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0 , ebufp);
		acct_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACCOUNT_OBJ, 0 , ebufp);
		bal_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_BAL_GRP_OBJ, 0 , ebufp);
		*s_pdpp = PIN_POID_COPY(serv_pdp, ebufp);
		*a_pdpp = PIN_POID_COPY(acct_pdp, ebufp);
		*b_pdpp = PIN_POID_COPY(bal_pdp, ebufp);
	}

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&s_iflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&s_oflistp, ebufp);

	return;
}

static void
mso_fld_proc_refund_gen_lc_event(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	poid_t			*a_pdp,
	pin_flist_t		*ref_flistp,
	pin_decimal_t		*tot_amount,
	pin_flist_t             *refret_flistp,
	char                    *disp_name,
	pin_errbuf_t            *ebufp)
{
	pin_flist_t             *lc_iflistp = NULL;
	pin_flist_t             *lc_temp_flistp = NULL;
	pin_flist_t             *ro_flistp = NULL;
	pin_flist_t             *ret_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	poid_t                  *user_id = NULL;
	poid_t			*ref_pdp = NULL;
	char			*ref_number = NULL;
	char                    *event = "/event/activity/mso_lifecycle/refund";

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_fld_refund_gen_lc_event Input FList:", i_flistp);
	ref_pdp = PIN_FLIST_FLD_GET(refret_flistp, PIN_FLD_POID, 0, ebufp );
	/* Get the Refund Number */
	fm_mso_utils_read_any_object(ctxp, ref_pdp, &ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in getting the refund object details!!");
		goto CLEANUP;
	}	
	ref_number = PIN_FLIST_FLD_GET(ret_flistp, MSO_FLD_REFUND_NO, 0, ebufp );
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp );
	lc_iflistp = PIN_FLIST_CREATE(ebufp);
	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_REFUND, 0, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, disp_name, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_USERID, user_id, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, tot_amount, ebufp);
	PIN_FLIST_FLD_SET(lc_temp_flistp, MSO_FLD_DEPOSIT_NO, ref_number, ebufp);
	res_flistp = PIN_FLIST_ELEM_GET(ref_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if(res_flistp)
		PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_gen_event Event FList:", lc_iflistp);

	fm_mso_utils_gen_event(ctxp, a_pdp, NULL, "MSO_OP_PYMT_PROCESS_REFUND",
				disp_name, event, lc_iflistp, ebufp);

	CLEANUP:
	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return;
}

// The refund to be processed only if account has enough credit balance.
static int mso_op_validate_bal_grp(
	pcm_context_t		*ctxp,
	poid_t			*a_pdp,
	poid_t			*b_pdp,
	pin_decimal_t		*amount,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*read_bal_flistp = NULL;
	pin_flist_t		*read_bal_res_flistp = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_decimal_t		*bal_amount = NULL;
	pin_decimal_t		*neg_amount = NULL;
	pin_decimal_t           *diff_amount = pbo_decimal_copy(amount, ebufp);
	pin_decimal_t    	*diff = pbo_decimal_from_str("0.02", ebufp);
	pin_decimal_t    	*zero_dec = pbo_decimal_from_str("0.00", ebufp);
	int32			bal_flags = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"mso_op_validate_bal_grp input error!!");
		goto CLEANUP;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"In function mso_op_validate_bal_grp");

	read_bal_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_POID, a_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_BAL_GRP_OBJ, b_pdp, ebufp);
	PIN_FLIST_FLD_SET(read_bal_flistp, PIN_FLD_FLAGS, &bal_flags, ebufp);
	PIN_FLIST_ELEM_SET(read_bal_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"mso_op_validate_bal_grp read balance group input list", read_bal_flistp);
	PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES , 0, read_bal_flistp, &read_bal_res_flistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
		"mso_op_validate_bal_grp read balance group error");
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"mso_op_validate_bal_grp read balance group result list", read_bal_res_flistp);

	bal_flistp = PIN_FLIST_ELEM_GET(read_bal_res_flistp, PIN_FLD_BALANCES, PIN_CURRENCY_INR, 1, ebufp);
	if (bal_flistp != NULL)
	{
		bal_amount = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);
		neg_amount = pbo_decimal_negate(amount, ebufp);
		pbo_decimal_add_assign(diff_amount, bal_amount, ebufp);

		if ( diff_amount && pbo_decimal_compare(diff_amount, zero_dec, ebufp) < 0)
		{
			pbo_decimal_negate_assign(diff_amount, ebufp);
		}

		if (neg_amount != NULL && pbo_decimal_compare(bal_amount, neg_amount, ebufp) <= 0) //bal_amount <= negate(amount))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account has enough credit balance");
			PIN_FLIST_DESTROY_EX(&read_bal_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);
			pbo_decimal_destroy(&neg_amount);
			return 1;
		}

		if (diff_amount != NULL && pbo_decimal_compare(diff, diff_amount, ebufp) >= 0)
		{
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account has enough credit balance 0.02 ");
                        PIN_FLIST_DESTROY_EX(&read_bal_flistp, NULL);
                        PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);
                        pbo_decimal_destroy(&neg_amount);
                        return 1;
		}
                

	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account does not have enough credit balance");

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&read_bal_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&read_bal_res_flistp, NULL);

	return 0;
}

PIN_EXPORT void
mso_pymt_process_refund_adjustment(
        pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
        poid_t                	*acct_pdp,
        poid_t                 	*service_pdp,
	poid_t			*bal_grp_pdp,
	char			*sys_descr,
	pin_decimal_t		*amount,
	pin_flist_t		**r_flistpp,
        pin_errbuf_t            *ebufp)
{
	pin_flist_t		*adj_in_flistp = NULL;
	pin_flist_t             *adj_out_flistp = NULL;
	char			*program_name = NULL;
        poid_t			*user_id = NULL;
	int32			str_ver =  1;
	int32			str_id = 1;
	int32			tax_flag = PIN_AR_WITHOUT_TAX; 

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error before mso_pymt_process_refund_adjustment");
                goto CLEANUP;
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_pymt_process_refund_adjustment input flist", i_flistp);
	program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);

        adj_in_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
        PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_AMOUNT, amount, ebufp);
        PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp);
        PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_DESCR, sys_descr, ebufp);
        PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_STRING_ID, &str_id, ebufp);
        PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_STR_VERSION, &str_ver, ebufp);
        PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_USERID, user_id, ebufp);
        PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_FLAGS, &tax_flag, ebufp);
	PIN_FLIST_FLD_SET(adj_in_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_pdp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "PCM_OP_AR_ACCOUNT_ADJUSTMENT input flist", adj_in_flistp);

        PCM_OP(ctxp, PCM_OP_AR_ACCOUNT_ADJUSTMENT, 0, adj_in_flistp, &adj_out_flistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error before mso_pymt_process_refund_adjustment");
                goto CLEANUP;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "PCM_OP_AR_ACCOUNT_ADJUSTMENT output flist", adj_out_flistp);

	*r_flistpp = PIN_FLIST_COPY(adj_out_flistp, ebufp);

	CLEANUP:	
        PIN_FLIST_DESTROY_EX(&adj_in_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&adj_out_flistp, NULL);
	return;
}

