/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 * Version 1.0: 17-08-2014: Harish Kulkarni: Added the MSO Customization of
 * Refund Processing using MSO_OP_PYMT_REVERSE_REFUND opcode and reverse
 * the refund amoount and update the /mso_refund object
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_reverse_refund.c /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_REVERSE_REFUND operation. 
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


#define REF_STATUS_OPEN 0
#define REF_STATUS_REVERSE 1
#define EVENT_MSO_REFUND_CREDIT "/event/billing/mso_refund_credit"
#define REFUND_REVERSAL "REFUND_REVERSAL"
#define REFUND_REASON_NOT_RECEIVED "Customer reported not received"
#define REFUND_REASON_NOT_ACCEPTED "Customer  not accepted"
#define REFUND_REASON_PROCESS_FAILED "Online batch processing failed"
#define REFUND_REASON_CHECK_BOUNCED "Refund Check bounced"
#define REFUND_REASON_OTHER "Other"

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_pymt_reverse_refund(
	cm_nap_connection_t	*connp,
	int					opcode,
	int					flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**o_flistpp,
	pin_errbuf_t		*ebufp);

static void mso_retrieve_refund_details(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void mso_update_refund_object(
	pcm_context_t		*ctxp,
	poid_t			*r_pdp,
	poid_t			*evt_pdp,
	poid_t			*item_pdp,
	pin_errbuf_t		*ebufp);

static void mso_op_pymt_reverse_refund(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void mso_op_get_acct_service_details(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        poid_t                  **bal_pdpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void
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

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PYMT_REVERSE_REFUND operation.
 *******************************************************************/
void
mso_pymt_reverse_refund(
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
	if (opcode != MSO_OP_PYMT_REVERSE_REFUND) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_pymt_reverse_refund opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_reverse_refund input flist", i_flistp);

	/***********************************************************
	 * Call the routine to process refund.
	 ***********************************************************/
	mso_op_pymt_reverse_refund(ctxp, i_flistp, &r_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		err_flistp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
					"Unknown Error occured in MSO_OP_PYMT_REVERSE_REFUND.");
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, err_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53021", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
				"Unknown Error occured in MSO_OP_PYMT_REVERSE_REFUND.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*o_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		goto CLEANUP;
	}

	*o_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_pymt_reverse_refund return flist", *o_flistpp);

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	return;
}

static void mso_op_pymt_reverse_refund(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*ref_oflistp = NULL;
	pin_flist_t			*res_flistp = NULL;
	pin_flist_t			*err_flistp = NULL;
	pin_flist_t			*ret_flistp = NULL;
	pin_flist_t			*adj_flistp = NULL;
	pin_flist_t			*bal_flistp =NULL;
	poid_t				*a_pdp = NULL;
	poid_t				*r_pdp = NULL;
	poid_t				*s_pdp = NULL;
	poid_t				*b_pdp = NULL;
	poid_t				*evt_pdp = NULL;
	poid_t				*item_pdp = NULL;
	char				msg[512];
	int					status = 0;
	int					*reason = NULL;
	pin_decimal_t		*ref_amount = NULL;
	pin_decimal_t		*rev_amount = NULL;
	char			*description = NULL;
	char			sys_description[256];
	char			sys_description1[256];
	char			*program_name = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Unnown input error!!");
		goto CLEANUP;
	}
	rev_amount = pbo_decimal_from_str("0.0",ebufp);	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_op_pymt_reverse_refund input flist", i_flistp);
	/* Get the Deposit and account details */
	mso_retrieve_refund_details(ctxp, i_flistp, &ref_oflistp, ebufp);
	description  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
	reason       = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON, 1, ebufp);
	memset(sys_description1,'\0',256);
	
	if(reason)
	{

		if(*reason == 1)
		{
			sprintf(sys_description1,"PROCESS REFUND REVERSE with reason %s",REFUND_REASON_NOT_RECEIVED);
		}
		else if (*reason == 2)
		{
			sprintf(sys_description1,"PROCESS REFUND REVERSE with reason %s",REFUND_REASON_NOT_ACCEPTED);
		}
		else if (*reason == 3)
		{
			sprintf(sys_description1,"PROCESS REFUND REVERSE with reason %s",REFUND_REASON_PROCESS_FAILED);
		}
		else if (*reason == 4)
		{
			sprintf(sys_description1,"PROCESS REFUND REVERSE with reason %s",REFUND_REASON_CHECK_BOUNCED);
		}
		else if (*reason == 5)
		{
			sprintf(sys_description1,"PROCESS REFUND REVERSE with reason %s",REFUND_REASON_OTHER);
		}
		else
		{
			sprintf(sys_description1,"PROCESS REFUND REVERSE with reason %s",REFUND_REASON_OTHER);
		}
	}
	else
	{
		
		sprintf(sys_description1,"%s","PROCESS REFUND REVERSE");
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,sys_description);

	program_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);

	if(description)
	{

		sprintf(sys_description,"%s and with remarks: %s",sys_description1,description);

	}
	else
	{
		sprintf(sys_description,"%s",sys_description1);
	}

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Error while getting refund details");
		goto CLEANUP;
	}
	res_flistp = PIN_FLIST_ELEM_GET(ref_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if (res_flistp == NULL)
	{
		status = 1;
		sprintf(msg, "No Refund details available for the reversal");
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Refund res_flistp:", res_flistp);
	ref_amount = PIN_FLIST_FLD_GET(res_flistp, MSO_FLD_REFUND_AMOUNT, 0, ebufp);
	r_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);
	a_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	s_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	rev_amount = pbo_decimal_negate(ref_amount, ebufp);
	/* Create a Pre-Rated Debit impact of the input refund amount */
//	fm_mso_utils_create_pre_rated_impact(ctxp, a_pdp, s_pdp, program_name,sys_description,
//					EVENT_MSO_REFUND_CREDIT, rev_amount, &au_oflistp, ebufp);
       	mso_op_get_acct_service_details(ctxp, a_pdp, &b_pdp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                sprintf(msg, "Error in getting Balance Details");
                goto CLEANUP;
        }

	mso_pymt_process_refund_adjustment(ctxp, i_flistp, a_pdp, s_pdp, b_pdp, sys_description, rev_amount, 
						&adj_flistp, ebufp);	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Error while doing refund debit impact.");
		goto CLEANUP;
	}
        res_flistp = PIN_FLIST_ELEM_GET(adj_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp);
        if (res_flistp)
        {
                bal_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_BAL_IMPACTS, 0, 0, ebufp);
                if (bal_flistp)
                        item_pdp = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);
                evt_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 0, ebufp);
        }

	/* Create a lifecycle event for Deposit Refund reversal*/
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_AMOUNT, ref_amount, ebufp );
	//mso_fld_refund_reverse_gen_lc_event(ctxp, i_flistp, a_pdp, au_oflistp, ref_amount, REFUND_REVERSAL, ebufp);

	/* Call the function to update a refund object */
	mso_update_refund_object(ctxp, r_pdp, evt_pdp, item_pdp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Error while updating refund object");
		goto CLEANUP;
	}

	fm_mso_create_lifecycle_evt(ctxp, i_flistp, adj_flistp, ID_REFUND_REVERSAL, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		sprintf(msg, "Error while creating lifecycle object");
		goto CLEANUP;
	}


	sprintf(msg, "Refund Reversal Success !!");
	err_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, r_pdp, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR, msg, ebufp);
	*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);

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
		PIN_FLIST_DESTROY_EX(&ref_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	        if (!pbo_decimal_is_null(rev_amount, ebufp))
        	{
               		pbo_decimal_destroy(&rev_amount);
        	}


	return;
}

/**************************************************************************
* Function:     mso_update_refund_object()
* Owner:        Hrish Kulkarni
* Decription:   For updating the /mso_refund object
*
**************************************************************************/
static void mso_update_refund_object(
	pcm_context_t		*ctxp,
	poid_t			*ref_pdp,
	poid_t			*evt_pdp,
	poid_t			*item_pdp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*co_iflistp = NULL;
	pin_flist_t		*co_oflistp = NULL;
	int32			flags = REF_STATUS_REVERSE;

	co_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(co_iflistp, PIN_FLD_POID, ref_pdp, ebufp);
	PIN_FLIST_FLD_SET(co_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(co_iflistp, MSO_FLD_REFUND_REV_EVENT_OBJ, evt_pdp, ebufp);
	PIN_FLIST_FLD_SET(co_iflistp, MSO_FLD_REFUND_REV_ITEM_OBJ, item_pdp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Refund Object Reverse Input flist", co_iflistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, co_iflistp, &co_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Refund Object Reverse Output flist", co_oflistp);
	
	PIN_FLIST_DESTROY_EX(&co_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&co_oflistp, NULL);

	return;
}

/**************************************************************************
* Function:     mso_retrieve_refund_details()
* Owner:        Hrish Kulkarni
* Decription:   This module retrieves the refund details.
*
**************************************************************************/
static void mso_retrieve_refund_details(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)-1;
	u_int64			db = 0;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int32			s_flags = 256;
	int32			i_status = REF_STATUS_OPEN; // 0-Open,1-Closed
	poid_t			*a_pdp = NULL;
	poid_t			*pdp = NULL;
		
	a_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0 , ebufp);
	db = PIN_POID_GET_DB(a_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_refund where F1 = V1 and F2 = V2 ", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_REFUND_NO, flistp, MSO_FLD_REFUND_NO, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_FLAGS, &i_status, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Refund Object search input flist", s_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, ret_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Refund Object search output flist", *ret_flistpp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}

///**************************************************************************
//* Function:     mso_fld_refund_reverse_gen_lc_event()
//* Owner:        Hrish Kulkarni
//* Decription:   This module creates the lifecycle event for refund reversal
//*
//**************************************************************************/
//static void
//mso_fld_refund_reverse_gen_lc_event(
//	pcm_context_t           *ctxp,
//	pin_flist_t             *i_flistp,
//	poid_t			*a_pdp,
//	pin_flist_t		*au_oflistp,
//	pin_decimal_t		*tot_amount,
//	char                    *disp_name,
//	pin_errbuf_t            *ebufp)
//{
//	pin_flist_t             *lc_iflistp = NULL;
//	pin_flist_t             *lc_temp_flistp = NULL;
//	pin_flist_t             *ro_flistp = NULL;
//	pin_flist_t             *res_flistp = NULL;
//	poid_t                  *user_id = NULL;
//	poid_t                  *acct_pdp = NULL;
//	char                    *event = "/event/activity/mso_lifecycle/refund_reversal";
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//			return;
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_fld_refund_reverse_gen_lc_event Input FList:", i_flistp);
//
//	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp );
//	lc_iflistp = PIN_FLIST_CREATE(ebufp);
//	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_REFREV, 0, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, disp_name, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_USERID, user_id, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, tot_amount, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_REFUND_NO, lc_temp_flistp, MSO_FLD_REFUND_NO, ebufp);
//	res_flistp = PIN_FLIST_ELEM_GET(au_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
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

static void mso_op_get_acct_service_details(
        pcm_context_t           *ctxp,
        poid_t	             	*acct_pdp,
        poid_t                  **bal_pdpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t         	*s_iflistp = NULL;
        pin_flist_t            	*s_oflistp = NULL;
        pin_flist_t         	*arg_flistp = NULL;
        pin_flist_t         	*res_flistp = NULL;
	poid_t			*srch_pdp = NULL;
        int32               	flags = 256;
        char                	*bb_template = "select X from /service where F1 = V1 order by service_t.created_t desc";
        int64               	db = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"mso_op_get_acct_serice_details input error!!");
                return;
        }

        db = PIN_POID_GET_DB(acct_pdp);
        s_iflistp = PIN_FLIST_CREATE(ebufp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

        PIN_FLIST_FLD_PUT(s_iflistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, bb_template, ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

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

        res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if (res_flistp !=NULL)
        {
                *bal_pdpp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_BAL_GRP_OBJ, 0 , ebufp);
        }

        CLEANUP:
                PIN_FLIST_DESTROY_EX(&s_iflistp, ebufp);
                PIN_FLIST_DESTROY_EX(&s_oflistp, ebufp);
        	return;
}

