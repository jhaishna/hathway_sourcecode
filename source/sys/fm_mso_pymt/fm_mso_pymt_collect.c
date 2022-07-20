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
 * preparing the Input FLIST for PCM_OP_PYMT_COLLECT opcode and payment receipt creation
 *
 * Change History
 * Gautam Adak : rewriting code 
 *
 * Modified By: Jyothirmayi Kachiraju
 * Date: 20th Apr 2020
 * Modification details : Added functionality to include the service provider id (Carrier ID) in the
 *			  				in the mso_ntf_sms_t table for identification.
 *
 **************************************************************************************************************/

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_collect.c /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_COLLECT operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
 
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
#include "ops/ar.h"
#include "mso_ops_flds.h"
#include "mso_lifecycle_id.h"
#include "mso_ntf.h"
#include "mso_cust.h"
#include "pin_os.h"
#include "pin_type.h"

#define SEQ_TYPE_RECEIPT "MSO_SEQUENCE_TYPE_RECEIPT"

#define PAY_TYPE_CASH 10011
#define PAY_TYPE_CHECK 10012
#define PAY_TYPE_ONLINE 10013
#define PAY_TYPE_TDS 10018
#define PAY_TYPE_SP_AUTOPAY 10019
#define PAY_TYPE_ECS 10020
#define PAY_TYPE_PORTAL_TRANSFER 10021

#define LOCAL_TRANS_OPEN_SUCCESS 0
#define LOCAL_TRANS_OPEN_FAIL 1

#define READONLY 0
#define READWRITE 1
#define LOCK_OBJ 2

#define WITHOUT_TAX 1
#define WITH_TAX 2

#define NOTIFICATION 3

#define STATUS_ACTIVE 10100
#define STATUS_INACTIVE 10102
#define SERV_TYPE_CATV 0
#define SERV_TYPE_BB 1
#define BILLINFO_ID_CATV "CATV"
#define BILLINFO_ID_BB "BB"
#define BILLINFO_ID_WS "WS_SETTLEMENT"
#define LCO_CUST_CARE 3
#define LCO_BULK_APPL_NAME "mso_bulk_pymt_posting"
#define SUSPEND 5
#define PAYMENT_COLLECT      "PAYMENT_COLLECT"
#define BAL_TRANS_LCO_SOURCE "BAL_TRANS_LCO_SOURCE"
#define BAL_TRANS_LCO_DEST   "BAL_TRANS_LCO_DEST"
#define CURRENCY_INR 356
#define NETFLIX_CURRENCY 1000358


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_mso_pymt_collect(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

void mso_find_billinfo_poid(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

void mso_retrieve_billed_items(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*s_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

void mso_prep_pymt_collect(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	int			lco_blk_payment,
	pin_flist_t		**ret_flistpp,
	poid_t			**billinfo_pdpp,
	pin_errbuf_t	*ebufp);

EXPORT_OP void mso_pymt_receipt_create(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*ip_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void mso_pymt_service_reactivation(
	pcm_context_t	*ctxp,
	poid_t			*billinfo_pdp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*srvc_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void mso_pymt_apply_late_fee(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

static void fm_mso_pymt_process_lco_payment(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	poid_t				*bal_grp_pdp,
	pin_errbuf_t		*ebufp);

static void
mso_op_payment_gen_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *s_oflistp,
        char                    *disp_name,
	pin_decimal_t		*trans_amt,
        pin_errbuf_t            *ebufp);

EXPORT_OP void
fm_mso_retrieve_service(
	pcm_context_t	*ctxp, 
	poid_t			*binfo_pdp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t	*ebufp);

void
fm_mso_retrieve_service_netflix(
	pcm_context_t	*ctxp, 
	poid_t			*act_pdp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t	*ebufp);

PIN_IMPORT void fm_mso_utils_sequence_no(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT int32 fm_mso_trans_open(
	pcm_context_t	*ctxp,
	poid_t			*pdp,
	int32			flag,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void fm_mso_trans_commit(
	pcm_context_t	*ctxp,
	poid_t			*pdp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void fm_mso_trans_abort(
	pcm_context_t	*ctxp,
	poid_t			*pdp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void fm_mso_retrieve_service_obj(
	pcm_context_t	*ctxp,
	poid_t			*acct_pdp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void fm_mso_utils_get_billinfo_bal_details(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        char                    *billinfo_id,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);
		
PIN_IMPORT void fm_mso_utils_get_billinfo_bal_details_woserv(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        char                    *billinfo_id,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);		

PIN_IMPORT void fm_mso_get_profile_info(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_utils_balance_transfer(
	pcm_context_t		*ctxp,
	poid_t			*s_acct_pdp,
	poid_t			*d_acct_pdp,
	poid_t			*s_bal_pdp,
	poid_t			*d_bal_pdp,
	pin_decimal_t		*amount,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_utils_read_any_object(
	pcm_context_t		*ctxp,
	poid_t			*objp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_mso_fetch_offer_purchased_plan(
	pcm_context_t		*ctxp,
	poid_t			*acc_poidp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_mso_get_acnt_from_acntno(
	pcm_context_t		*ctxp,
	char			*acnt_no,
	pin_flist_t		**acnt_details,
	pin_errbuf_t		*ebufp);
 
void fm_mso_utils_get_all_service_details(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	int32			serv_type,
	pin_flist_t		**o_flistpp,
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

PIN_IMPORT void
fm_mso_utils_get_lco_services_info(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             **r_flistp,
	char                    *ser_type,
	pin_errbuf_t            *ebufp);

extern void
fm_mso_check_csr_access_level(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *acnt_info,
	pin_flist_t             **ret_flist,
	pin_errbuf_t            *ebufp);

extern void
fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

extern void
fm_mso_get_account_info(
	pcm_context_t		*ctxp,
	poid_t			*acnt_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

void
fm_mso_retrieve_os_charges(
	pcm_context_t	*ctxp, 
	poid_t			*binfo_pdp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t	*ebufp);

int32
fm_mso_get_pp_type(
        pcm_context_t   *ctxp,
        poid_t          *act_pdp,
        pin_errbuf_t    *ebufp);

PIN_IMPORT int
fm_mso_cust_get_gst_profile(
        pcm_context_t           *ctxp,
        poid_t                  *account_pdp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

void
fm_mso_split_netflix_pymt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_decimal_t 	*pymt_amt,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp);

static int32
fm_mso_split_hybrid_pymt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_decimal_t	 	*catv_amtp,
	char			*catv_acc_nop,
	pin_errbuf_t		*ebufp);

void
mso_op_netfp_gen_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);

void
get_evt_lifecycle_template(
	pcm_context_t			*ctxp,
	int64				db,
	int32				string_id, 
	int32				string_version,
	char				**lc_template, 
	pin_errbuf_t			*ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PYMT_COLLECT operation.
 *******************************************************************/
void
op_mso_pymt_collect(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pcm_context_t		*new_ctxp = NULL;

	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*hybrid_offer_info = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*notif_flistp = NULL;
	pin_flist_t		*srvc_flistp = NULL;
	pin_flist_t		*s_r_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*rcpt_flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*l_flistp = NULL;
	pin_flist_t		*nr_flistp = NULL;
	pin_flist_t		*telco_feature_flistp = NULL;
	pin_flist_t		*validate_iflistp = NULL;
	pin_flist_t		*validate_oflistp = NULL;
	pin_flist_t     	*charge_flistps = NULL;
	pin_flist_t    		*os_r_flistp = NULL;
	pin_flist_t    		*os_r_flistp1 = NULL;
 	poid_t			*d_pdp = NULL;
	poid_t			*plan_pdp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*billinfo_pdp = NULL;
	poid_t			*user_poid = NULL;

	pin_decimal_t		*amount = NULL; 
	
	int			*cmdp = NULL;
	int			local = 1;
	int32			elemid = 0;
	int32			status = 0;
	int32			*pymt_status = NULL;
	int32			additional_status = 0;
	int32			*actg_type = NULL;
	int32			*flag = NULL;
	int32			*s_flags = NULL;
	int32			*svc_status = NULL;
	int32			id = 1;
	int32			tax = WITHOUT_TAX;
	int32			notif = NOTIFICATION;
	int64			db = 1;
	int32			errp = 0;
	int32			*serv_type = NULL;
	int32			*pay_type = NULL;
	int			    lco_blk_payment = 0;
	int32			*prov_status = NULL;
	int32			acnt_type = -1;
	int32			acc_no_val = 0;
    int32           subscriber_type = -1;
	int32			catv_pymt_status = 0;
    int             enable_flg = 1;

	pin_cookie_t		cookie = NULL;
	
	char			*prog_name = NULL;
	char			ret_msg[255]="";
	char			*tag_acc_nop = NULL;
	pin_decimal_t           *pymt_amt = NULL;
    pin_decimal_t           *pymt_amt1 = NULL;
    pin_decimal_t           *abs_pymt_amt = NULL;
    pin_decimal_t           *diff = NULL;
    pin_decimal_t           *abs_diff = NULL;
    pin_decimal_t           *tolerance = NULL;
    pin_decimal_t           *abs_tolerance = NULL;
    pin_decimal_t           *curr_bal = NULL;
    pin_decimal_t           *abs_curr_bal = NULL;
	pin_decimal_t		*cmp_amt = NULL;
	pin_decimal_t		*zero_p = NULL;
	pin_decimal_t		*catv_pymt_amtp = NULL;
	void			*vp = NULL;
       //Netflix Changes
	int32			*netf_flag = NULL;
	pin_decimal_t		*net_pymt_amt = NULL;
	pin_flist_t     	*netf_in_flist = NULL;
	pin_flist_t     	*netf_oflistp = NULL;
       int32	              *netf_status = NULL;
	int32				srvc_prov_id = -1;
	int32				carrier_id = -1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_COLLECT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_pymt_collect opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_mso_pymt_collect input flist", i_flistp);

	/***********************************************************
	 * Identifying the Billinfo POID & ACTG_TYPE
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	rcpt_flistp = PIN_FLIST_CREATE(ebufp);
	err_flistp = PIN_FLIST_CREATE(ebufp);
	d_pdp = PIN_POID_CREATE(db, "/account", id, ebufp);
	
	serv_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 1, ebufp);

	if  (serv_type && *serv_type == 0)
	{
			cmp_amt = pbo_decimal_from_str("399999.99", ebufp);
	}
	else
	{
			cmp_amt = pbo_decimal_from_str("1000000.00", ebufp);
	}
	charge_flistps = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_CHARGES, PIN_ELEMID_ANY, 0, ebufp);
	if (charge_flistps && charge_flistps != NULL )
	{
		pymt_amt = (pin_decimal_t *) PIN_FLIST_FLD_GET(charge_flistps, PIN_FLD_AMOUNT, 0, ebufp);		
		zero_p = pbo_decimal_from_str("0.0", ebufp);	
		if (pymt_amt && pymt_amt != NULL && cmp_amt && cmp_amt != NULL && pbo_decimal_compare(pymt_amt, cmp_amt, ebufp) > 0)
		{
	   	   if (serv_type && *serv_type == 0)
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Validation: Payment more than 399999.99 is not allowed", ebufp);
		   else
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Validation: Payment more than 1000000.00 is not allowed", ebufp);

          	   PIN_ERRBUF_RESET(ebufp);
           	   status = 1;
           	   PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
           	   PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53004", ebufp);
           	    if (serv_type && *serv_type == 0){
	                 PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment more than 399999.99 is not allowed", ebufp);
			    }
		    else{
				 PIN_FLIST_FLD_SET(err_flistp,PIN_FLD_ERROR_DESCR,"Payment more than 1000000.00 is not allowed", ebufp);
			    }
          	   PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		
	   	   if (cmp_amt && !pbo_decimal_is_null(cmp_amt, ebufp)) 
	   	       pbo_decimal_destroy(&cmp_amt);

                   goto CLEANUP;		
		}
		PIN_ERR_LOG_MSG(3, "Entering charges block:");
		if(pymt_amt && pymt_amt != NULL && zero_p && (pbo_decimal_compare(pymt_amt, zero_p, ebufp) <= 0))
		{
	       	   
 		   PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Validation :  Payment less than or equal to zero not allowed");
		   PIN_ERRBUF_RESET(ebufp);
                   status = 1;
                   PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
                   PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53005", ebufp);
		   PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment less than or equal to zero is not allowed", ebufp);
		   PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		   if (zero_p && !pbo_decimal_is_null(zero_p, ebufp))
                       pbo_decimal_destroy(&zero_p);
		  goto CLEANUP;
		}
	}	
	if(fm_utils_op_is_ancestor(connp->coip, MSO_OP_PYMT_LCO_COLLECT))
		lco_blk_payment = 1;
	
	PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	serv_type = (int32 *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp);
/*	mso_find_billinfo_poid(new_ctxp, i_flistp, &r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "billinfo return flist", r_flistp);

	s_r_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	binfo_pdp = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_POID, 0, ebufp);
	actg_type = (int32 *)PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_ACTG_TYPE, 0, ebufp);
	s_flags = (int32 *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if ((!(r_flistp)) || PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Billinfo Object or Accounting Type Not Found", r_flistp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53000", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Billinfo Object or Accounting Type Not Found", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
	}
*/
	/***********************************************************
	 * Preparing Input FLIST & Invoking PCM_OP_PYMT_COLLECT
	 ***********************************************************/
// Setting flat to 3 and passing account poid
	local = fm_mso_trans_open(new_ctxp, acct_pdp, 2, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Opened");
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error before calling mso_prep_pymt_collect for payments", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53004", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in opening transactions", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
	}

	/****************************************************************
	 * Netflix payment split if Flag is passed and !=0
	 ****************************************************************/
	netf_flag = PIN_FLIST_FLD_GET(charge_flistps, PIN_FLD_FLAGS, 1, ebufp);
	if(netf_flag && netf_flag !=NULL && *netf_flag !=0 && *netf_flag <3)
	{
		net_pymt_amt = (pin_decimal_t *) PIN_FLIST_FLD_GET(charge_flistps, PIN_FLD_AMOUNT_DST, 0, ebufp);
   		if (charge_flistps && charge_flistps != NULL && (!PIN_ERRBUF_IS_ERR(ebufp)))
		{
			netf_in_flist = PIN_FLIST_CREATE(ebufp);
			netf_in_flist = PIN_FLIST_COPY(i_flistp,ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"NETFLIX FLIST AFTER SPLITTING NETFLIX", netf_in_flist);
			fm_mso_split_netflix_pymt(new_ctxp, netf_in_flist ,net_pymt_amt, &netf_oflistp, ebufp);	
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_split_netflix_pymt OUTPUT FLIST", netf_oflistp);
			netf_status = PIN_FLIST_FLD_GET(netf_oflistp, PIN_FLD_STATUS, 1, ebufp);
			if(local == LOCAL_TRANS_OPEN_SUCCESS && *netf_status == 1)
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_ERR_LOG_MSG(3,"Netflix Payment Posting Failed.");
				fm_mso_trans_abort(new_ctxp, acct_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Aborted");
				*o_flistpp = PIN_FLIST_COPY(netf_oflistp,ebufp);
			    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"op_mso_pymt_collect_netflix return flist", *o_flistpp);
				goto CLEANUP_LAST;
			}
			else if(local == LOCAL_TRANS_OPEN_SUCCESS && *netf_status == 0 && *netf_flag ==1)
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_ERR_LOG_MSG(3,"Netflix Payment Posting Successful Commiting Transaction.");
				fm_mso_trans_commit(new_ctxp, acct_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Committed");
				*o_flistpp = PIN_FLIST_COPY(netf_oflistp,ebufp);
			    	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"op_mso_pymt_collect_netflix return flist", *o_flistpp);
				goto CLEANUP_LAST;
			}
			else
			{
				PIN_ERR_LOG_MSG(3,"Netflix Payment Posting Successful Continuing with Normal Flow if Required.");
			}
		}
		else
		{
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Could not post Netflix payment As netflix Amount is not specified.");
			netf_oflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(netf_oflistp, PIN_FLD_POID,acct_pdp,ebufp);
			PIN_FLIST_FLD_SET(netf_oflistp, PIN_FLD_ERROR_CODE,"53995",ebufp);
			PIN_FLIST_FLD_SET(netf_oflistp, PIN_FLD_ERROR_DESCR,"Could not post Netflix payment as Netflix Amount is Missing.", ebufp);
			PIN_FLIST_FLD_SET(netf_oflistp, PIN_FLD_STATUS, &status, ebufp);
			fm_mso_trans_abort(new_ctxp, acct_pdp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Aborted");
			*o_flistpp = PIN_FLIST_COPY(netf_oflistp,ebufp);
			PIN_FLIST_DESTROY_EX(&netf_oflistp, ebufp);
			goto CLEANUP_LAST;	
		}	
	}
	else 
   	{
    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Continuing the Normal flow for Cases if Flag is not Passed or passed 0 or Netflix Amount not passed");
	}

	/****************************************************************
	 * End of Netflix payment split if Flag is passed and !=1
	 ****************************************************************/

	validate_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, validate_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, validate_iflistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, validate_iflistp, PIN_FLD_USERID, ebufp);
	user_poid = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);
	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	// Check the CSR access for making the payment for the payment
	//Pawan: Added 2nd condition to avoid validation if account and 
	//	user poid are same and prog_name contains "selfcare"
	fm_mso_get_account_info(ctxp, acct_pdp, &acnt_info, ebufp);
	if (*serv_type == 1)
	{
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
			plan_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
			if (plan_pdp)
			{
				fm_mso_fetch_offer_purchased_plan(ctxp, plan_pdp, &hybrid_offer_info, ebufp);
			}
			else
			{
				fm_mso_fetch_offer_purchased_plan(ctxp, acct_pdp, &hybrid_offer_info, ebufp);
			}
			if (hybrid_offer_info)
			{
				prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
				catv_pymt_amtp = PIN_FLIST_FLD_GET(hybrid_offer_info, MSO_FLD_NCF_CHARGE, 0, ebufp);	
				if (pbo_decimal_compare(pymt_amt, catv_pymt_amtp, ebufp) > 0)	
				{
			        	pymt_amt1 = pbo_decimal_subtract(pymt_amt, catv_pymt_amtp, ebufp);
					sprintf(ret_msg, "%s|Total Payment Rs.%s, CATV - Rs.%s and ISP - Rs.%s", 
						prog_name, pbo_decimal_to_str(pymt_amt, ebufp),
					pbo_decimal_to_str(catv_pymt_amtp, ebufp), pbo_decimal_to_str(pymt_amt1, ebufp));
					PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_DESCR, ret_msg,  ebufp);
					catv_pymt_status = fm_mso_split_hybrid_pymt(ctxp, i_flistp, catv_pymt_amtp, 
						tag_acc_nop, ebufp);
					if (catv_pymt_status == 1)
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Successfully created CATV payment");
						PIN_FLIST_FLD_PUT(charge_flistps, PIN_FLD_AMOUNT, pymt_amt1, ebufp);		
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Failed to create CATV payment");
					}
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Skip CATV payment in Hybrid");
				}
				PIN_FLIST_DESTROY_EX(&hybrid_offer_info, NULL);
			}
		}
	}

	prog_name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
	if(*serv_type == 1 &&
		!(PIN_POID_COMPARE(acct_pdp,user_poid,0, ebufp)==0 &&
		  prog_name && (strstr(prog_name,"selfcare") || strstr(prog_name,"SCMOB") || strstr(prog_name,"SCFOS") || strstr(prog_name,"SCPAYTM") || strstr(prog_name,"SCMOBI") || strstr(prog_name,"AFFLE")))
		)
	{
		fm_mso_check_csr_access_level(ctxp, validate_iflistp , acnt_info, &validate_oflistp, ebufp);
		if (!validate_oflistp)
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in CSR access to this account : ORG_STRUCTURE: returns null", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			status = 1;
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53004", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "CSR has no access to this account :ORG_STRUCTURE", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			goto CLEANUP;
		}
	}

	vp = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_BUSINESS_TYPE, 1, ebufp);
	if(vp && vp != NULL )
	{
		acnt_type = (*(int32*)vp)/1000000;
	    subscriber_type = (*(int32*)vp)/10000;	
		// Fetching the last digit from Business Type
		srvc_prov_id = (*(int32*)vp)%10;
		
		// Service Provider Identification for SMS Notification
		if (srvc_prov_id == 0 || srvc_prov_id == 1)
		{
			carrier_id = 0; // Service Provider is Hathway
		}
		else if (srvc_prov_id == 2 || srvc_prov_id == 3)
		{
			carrier_id = 1; // Service Provider is DEN
		}		
	}

    /*********************************************************
     * Don't allow payment on child accounts
     ********************************************************/
    if (subscriber_type == 9940)
    {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"op_mso_pymt_collect error: Payment not allowed on child account");
            PIN_ERRBUF_RESET(ebufp);
            status = 1;
            PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
            PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53016", ebufp);
            PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment not allowed on child account", ebufp);
            PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
            PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
            goto CLEANUP;
    }

	//Validate if the account does not have any service
	serv_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 1, ebufp);

	if (acnt_type != ACCT_TYPE_LCO && serv_type && serv_type != NULL && *serv_type == 1) //broadband
	{
		fm_mso_utils_get_services_info(ctxp, i_flistp, &ret_flistp, "/service/telco/broadband", ebufp); 

		strcpy(ret_msg, "Payment not allowed until");
		strcat(ret_msg, " BB ");
		strcat(ret_msg, "service associated with account");
	}
	else if (acnt_type != ACCT_TYPE_LCO && serv_type && serv_type != NULL && *serv_type == 0) //CATV
	{
	}
	/*
		fm_mso_utils_get_services_info(ctxp, i_flistp, &ret_flistp, "/service/catv", ebufp);
		strcpy(ret_msg, "Payment not allowed until");
		strcat(ret_msg, " CATV ");
		strcat(ret_msg, "service associated with account");	

	if (ret_flistp)
	{
		rs_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if (!rs_flistp)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"op_mso_pymt_collect error: No service found");
			additional_status=0;
			PIN_ERRBUF_RESET(ebufp);
			status = 1;
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53006", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, ret_msg, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
			goto CLEANUP;			
		}
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	}
	*/
	mso_prep_pymt_collect(new_ctxp, i_flistp, lco_blk_payment, &ret_flistp, &billinfo_pdp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"pcm_op_pymt_collect output flist", ret_flistp);

	if(ret_flistp && ret_flistp != NULL)	
 	{	pymt_status = (int32 *)PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_STATUS, 0, ebufp);}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,
                "pcm_op_pymt_collect output flist", ret_flistp);
		goto CLEANUP;
		}

	if (pymt_status && pymt_status != NULL && *pymt_status == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"op_mso_pymt_collect error");
		additional_status=0;
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53006", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Posting Failed", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
	}

	/***********************************************************
	 * Payments Receipt Creation
	 ***********************************************************/
	mso_pymt_receipt_create(new_ctxp, ret_flistp, i_flistp, &rcpt_flistp, ebufp);
        if((ebufp)->pin_err == PIN_ERR_STORAGE) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error generating receipts", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53004", ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error : Reference Id must be unique", ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
                goto CLEANUP;
        }

	if(rcpt_flistp && rcpt_flistp != NULL){}
	else
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error generating receipts", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                status = 1;
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53004", ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error : Reference Id must be unique", ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
                goto CLEANUP;
	}

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error generating receipts", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53004", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error generating receipts", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
	}


	/***********************************************************
	 * Payment Lifecycle Event Creation -start
	 ***********************************************************/
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_NAME, PAYMENT_COLLECT, ebufp );
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_AMOUNT, NULL, ebufp );
	PIN_FLIST_FLD_COPY(rcpt_flistp, PIN_FLD_RECEIPT_NO, ret_flistp, PIN_FLD_RECEIPT_NO, ebufp);
	fm_mso_create_lifecycle_evt(new_ctxp, i_flistp, ret_flistp, ID_PAYMENT, ebufp );
	//mso_op_payment_gen_lc_event(new_ctxp, i_flistp, ret_flistp, PAYMENT_COLLECT, NULL, ebufp);

	vp = (pin_flist_t*)PIN_FLIST_ELEM_GET(i_flistp, MSO_FLD_LCOINFO, 0, 1, ebufp);
	if (vp && vp != NULL )
	{
		amount = PIN_FLIST_FLD_GET(vp, PIN_FLD_AMOUNT, 1, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_NAME, BAL_TRANS_LCO_SOURCE, ebufp );
		PIN_FLIST_FLD_COPY(vp, PIN_FLD_AMOUNT, ret_flistp, PIN_FLD_AMOUNT,  ebufp );
		fm_mso_create_lifecycle_evt(new_ctxp, i_flistp, ret_flistp, ID_PAYMENT, ebufp );
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Lifecycle event creation BAL_TRANS_LCO_SOURCE !!");
			goto CLEANUP;
		}
		
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_NAME, BAL_TRANS_LCO_DEST, ebufp );
		fm_mso_create_lifecycle_evt(new_ctxp, i_flistp, ret_flistp, ID_PAYMENT, ebufp );
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Lifecycle event creation BAL_TRANS_LCO_DEST !!");
			goto CLEANUP;
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error lifecycle event creation", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53004", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error lifecycle event creation", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
	}
	/***********************************************************
	 * Payment Lifecycle Event Creation - end
	 ***********************************************************/


	/***********************************************************
	 * Notification for Payments
	 ***********************************************************/
	 if (s_flags && s_flags != NULL && (*s_flags == 1))
	 {
		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Payment Made Before any Service Activation, Hence no Notification");
	 }
	 else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling Retrieve Services");
		//fm_mso_retrieve_service_obj(new_ctxp, acct_pdp, &r_flistp, ebufp);
		if (serv_type && serv_type != NULL && *serv_type != 0)
		{
		fm_mso_utils_get_all_service_details(ctxp, acct_pdp, *serv_type, &r_flistp, ebufp);
		if(r_flistp && r_flistp != NULL)
		srvc_flistp = PIN_FLIST_COPY(r_flistp, ebufp);
		}
		//rs_flistp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_RESULTS, 1, ebufp);
		rs_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

		if (!PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Calling Notification API");

			notif_flistp = PIN_FLIST_CREATE(ebufp);

			pay_type = (int32 *)PIN_FLIST_FLD_GET(rcpt_flistp, PIN_FLD_PAY_TYPE, 0, ebufp);
			if( pay_type && pay_type != NULL && *pay_type == PAY_TYPE_CASH)
			{
				notif = NTF_BB_PAYMENT_CASH;
			
			} else if( pay_type && pay_type != NULL && *pay_type == PAY_TYPE_CHECK) {
			
				PIN_FLIST_FLD_COPY(rcpt_flistp, PIN_FLD_CHECK_NO, notif_flistp, PIN_FLD_CHECK_NO, ebufp);
				notif = NTF_BB_PAYMENT_CHEQUE;			
			}
			//ADDED For sms template changes Aug 2017
                        else if(  pay_type && pay_type != NULL && *pay_type  == PAY_TYPE_ONLINE) {
                                notif = NTF_BB_PAYMENT_ONLINE;
                        }
                        // Changes end
			 else {

				notif = NTF_PAYMENT;
			}
			PIN_FLIST_FLD_COPY(rcpt_flistp, PIN_FLD_ACCOUNT_OBJ, notif_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(rcpt_flistp, PIN_FLD_AMOUNT, notif_flistp, PIN_FLD_AMOUNT, ebufp);
			PIN_FLIST_FLD_COPY(rcpt_flistp, PIN_FLD_RECEIPT_NO, notif_flistp, PIN_FLD_TRANS_ID, ebufp);
			PIN_FLIST_FLD_COPY(rcpt_flistp, PIN_FLD_CREATED_T, notif_flistp, PIN_FLD_ACK_TIME, ebufp);
			PIN_FLIST_FLD_COPY(rcpt_flistp, PIN_FLD_PAY_TYPE, notif_flistp, PIN_FLD_MODE, ebufp);
			
			if (rs_flistp && serv_type && serv_type != NULL && *serv_type != 0)
			{
					PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_POID, notif_flistp, PIN_FLD_SERVICE_OBJ, ebufp);				
			}
			else
			{
				s_pdp = PIN_POID_CREATE(db, "/service/catv", id, ebufp);
				PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_SERVICE_OBJ, s_pdp, ebufp);
			}
			PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_FLAGS, &notif, ebufp);
			
			// Service Provider Identification - Hathway or DEN 
			PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_CARRIER_ID, &carrier_id, ebufp);
			
			//PIN_FLIST_FLD_COPY(rcpt_flistp, PIN_FLD_CHECK_NO, notif_flistp, PIN_FLD_CHECK_NO, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification input flist", notif_flistp);
			PCM_OP(new_ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, notif_flistp, &nr_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification output flist", nr_flistp);
			if((!(r_flistp)) || PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error in Notification");
				PIN_ERRBUF_CLEAR(ebufp);
				status = 1;
				additional_status=1;
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53005", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error in Notification", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				PIN_FLIST_DESTROY_EX(&notif_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&nr_flistp, NULL);
				goto CLEANUP;
			}
			PIN_FLIST_DESTROY_EX(&notif_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&nr_flistp, NULL);

		}
	

		/****************************************************************
		 * Service Re-Activation on Full Payment, if enabled in pin.conf
		 ****************************************************************/
		/* Get the service_reactivation flag from the pin.conf */
		pin_conf("fm_mso_pymt", "service_reactivation", PIN_FLDT_INT, (caddr_t*)&flag, &errp);
		if (errp != PIN_ERR_NONE) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
				"Service Re-activation config error!");
			/*****/
		}
		else 
		{
			if (flag && flag != NULL && *flag == 0)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service Re-activation flag Disabled!");
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Service Re-activation flag Enabled!");
				if(rs_flistp && serv_type && serv_type != NULL && *serv_type == 1)
				{
					telco_feature_flistp = PIN_FLIST_ELEM_GET(rs_flistp, PIN_FLD_TELCO_FEATURES, 
								PIN_ELEMID_ANY, 1, ebufp);
					prov_status = PIN_FLIST_FLD_GET(telco_feature_flistp, PIN_FLD_STATUS, 0, ebufp);

					svc_status = (int32 *)PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_STATUS, 1, ebufp);
					if (svc_status && svc_status != NULL && *svc_status == STATUS_INACTIVE || *svc_status == STATUS_ACTIVE)
					{
						if(prov_status && prov_status != NULL && *prov_status == SUSPEND)
						{

                        			    os_r_flistp1 = PIN_FLIST_CREATE(ebufp);
			                            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Retrieving the Outstanding Charges...");
		     		                    fm_mso_retrieve_os_charges(ctxp, billinfo_pdp, &os_r_flistp1, ebufp);
    		                		    if (os_r_flistp1 && os_r_flistp1 != NULL)
    		                		    {
			                                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Getting the Tolerance Amount.");
                        			        tolerance = (pin_decimal_t *) PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_LIMIT, 1, ebufp);
							
			                                if(tolerance && tolerance != NULL )abs_tolerance = pbo_decimal_abs(tolerance, ebufp);
                        			        if (abs_tolerance && abs_tolerance != NULL)
			                                {
                        			            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Validating the Tolerance Amount.");
			                                    pymt_amt1 = (pin_decimal_t *) PIN_FLIST_FLD_GET(charge_flistps, PIN_FLD_AMOUNT, 1, ebufp);
			                                    if(pymt_amt1 && pymt_amt1 != NULL)abs_pymt_amt = pbo_decimal_abs(pymt_amt1, ebufp);

                        			   	    curr_bal = (pin_decimal_t *) PIN_FLIST_FLD_GET(os_r_flistp1, PIN_FLD_CURRENT_BAL, 1, ebufp);
				                            if(curr_bal && curr_bal != NULL)abs_curr_bal = pbo_decimal_abs(curr_bal, ebufp);

			                                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
								"Retrieved Current Balance and Payment Amount.");

			                                    if (abs_pymt_amt && abs_pymt_amt != NULL && abs_curr_bal != NULL && abs_curr_bal)
                        			            {
			                                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calculating the Difference");
                        			                diff = (pin_decimal_t *)pbo_decimal_subtract(abs_curr_bal, abs_pymt_amt, ebufp);
			                                        abs_diff = pbo_decimal_abs(diff, ebufp);

                        			                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Absoluted the Difference");

			                                        if (abs_tolerance && abs_tolerance != NULL && abs_diff  && abs_diff != NULL && (pbo_decimal_compare(abs_diff, abs_tolerance, ebufp) <= 0))
                        			                {
			                                            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Within the Tolerance Limit");
                        			                    // Setting the Flag for Reactivation without validating the outstanding for BB
			                                            PIN_FLIST_FLD_SET(rcpt_flistp, MSO_FLD_ID_TYPE, &enable_flg, ebufp);
                        			                    /* Broadband */
        			    		    		    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling Service Re-activation API");
				        		    	    mso_pymt_service_reactivation(new_ctxp, billinfo_pdp, 
						    	    		    rcpt_flistp, srvc_flistp, &r_flistp, ebufp); 

			//                                            PIN_FLIST_FLD_SET(rcpt_flistp, MSO_FLD_ID_TYPE, &enable_flg, ebufp);
                           			                 }
			                                         else
			                                            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
									"Beyond the Tolerance Limit, so no Service Re-activation!!!");

                                    			    }	
                                			}
			                                else
                        			        {
			                                    /* Broadband */
                        			            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
			                                        "Calling Service Re-activation API");
                        			            mso_pymt_service_reactivation(new_ctxp, billinfo_pdp, 
			                                        rcpt_flistp, srvc_flistp, &r_flistp, ebufp); 
                        			        }
    		                		}
	
							
					      }
	
						if(PIN_ERRBUF_IS_ERR(ebufp))
						{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in Re-activating Services");
							PIN_ERRBUF_CLEAR(ebufp);
							status = 1;
							additional_status=1;
							PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
							PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53006", ebufp);
							PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
									"Error in Re-activating Services", ebufp);
							PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
							goto CLEANUP;
						}
					}
				}
				else 
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
						"Skipped Service Re-activation, Services are ACTIVE");
				}
			}
		}
	}
    

	/****************************************************************
	 * Applying Payment Late fee based on the Flag in CM's pin.conf
	 ****************************************************************/
//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Starting Late Fee Section");
	/* get the late fee flag from the pin.conf */
/*	pin_conf("fm_mso_pymt", "late_fee", PIN_FLDT_INT, (caddr_t*)&flag, &errp);
	if (errp != PIN_ERR_NONE) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"Payment Late Fee config error!");
		return;

	}
	else 
	{
		if (*flag == 0)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Late Fee flag Disabled, Hence No Late Fee is Applicable!");
		}
		else
		{
			l_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling Late Fee Adjustment");
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, l_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(s_r_flistp, PIN_FLD_POID, l_flistp, PIN_FLD_BILLINFO_OBJ, ebufp);
			
			pin_conf("fm_mso_pymt", "late_fee_interest", PIN_FLDT_DECIMAL, (caddr_t*)&interest, &errp);
			PIN_FLIST_FLD_SET(l_flistp, PIN_FLD_AMOUNT, interest, ebufp);
	
			pin_conf("fm_mso_pymt", "total_days", PIN_FLDT_DECIMAL, (caddr_t*)&total_days, &errp);
			PIN_FLIST_FLD_SET(l_flistp, PIN_FLD_TOTALS, total_days, ebufp);
			PIN_FLIST_FLD_SET(l_flistp, PIN_FLD_FLAGS, &status, ebufp);
			PIN_FLIST_FLD_SET(l_flistp, PIN_FLD_TAX_FLAGS, &tax, ebufp);

			mso_pymt_apply_late_fee(ctxp, l_flistp, &l_flistp, ebufp);
		}
		
	}
*/

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_pymt_collect error", ebufp);
		
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, d_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53006", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Posting Failed", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
	}
/*	else
	{
			
		*o_flistpp = PIN_FLIST_COPY(rcpt_flistp,ebufp);
		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &status, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_pymt_collect return flist", *o_flistpp);

	}
*/
CLEANUP:
	// Additional Status is included, Even if Notification / Service Activation fails, 
	// still the pymt will be posted and committed.
	if (additional_status == 0)
	{
		PIN_ERR_LOG_MSG(3,"inside additional_status = 0");
		if (status == 1)
		{
			PIN_ERR_LOG_MSG(3,"inside status = 1");
			if(local == LOCAL_TRANS_OPEN_SUCCESS )
			{
				PIN_ERR_LOG_MSG(3,"trans fail inside status = 1");
				fm_mso_trans_abort(new_ctxp, acct_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Aborted");
			}
			*o_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
		}
		else
		{
			PIN_ERR_LOG_MSG(3,"inside status = 1 else");
			if(local == LOCAL_TRANS_OPEN_SUCCESS )
			{
				PIN_ERR_LOG_MSG(3,"trans success inside status = 1");
				fm_mso_trans_commit(new_ctxp, acct_pdp, ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Committed");
			}
			*o_flistpp = PIN_FLIST_COPY(rcpt_flistp,ebufp);
			PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &status, ebufp);
		}
	}
	else 
	{
		PIN_ERR_LOG_MSG(3,"inside additional_status = 0 else");
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			PIN_ERR_LOG_MSG(3,"trans success inside");
			fm_mso_trans_commit(new_ctxp, acct_pdp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Committed");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Payment posted, though Notification/Service Re-activation failed");
		}
		status = 0;
		*o_flistpp = PIN_FLIST_COPY(rcpt_flistp,ebufp);
		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_STATUS, &status, ebufp);
		
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		
	}
    
    // Retrieving the Current Balance after Payment for BB Customers for responses
	if (serv_type && serv_type != NULL && *serv_type == 1 && billinfo_pdp)
	{
    		os_r_flistp = PIN_FLIST_CREATE(ebufp);
    		fm_mso_retrieve_os_charges(ctxp, billinfo_pdp, &os_r_flistp, ebufp);
    		if (os_r_flistp && os_r_flistp != NULL )
    		{
        		PIN_FLIST_FLD_COPY(os_r_flistp, PIN_FLD_CURRENT_BAL, *o_flistpp, PIN_FLD_CURRENT_BAL, ebufp);
    		}
	}

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_mso_pymt_collect return flist", *o_flistpp);

	PIN_FLIST_DESTROY_EX(&rcpt_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&os_r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&l_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&validate_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&validate_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	if (zero_p && !pbo_decimal_is_null(zero_p, ebufp))
	{
	     pbo_decimal_destroy(&zero_p);
	}
	PIN_POID_DESTROY(d_pdp, NULL);
	PIN_POID_DESTROY(billinfo_pdp, NULL);
CLEANUP_LAST : pin_free(flag);
		 PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
		 return;
}

/* This module retrieves the Billinfo details based from the Input Flist passed to MSO_OP_PYMT_COLLECT */
/*void mso_find_billinfo_poid(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	u_int64			id = (u_int64)-1;
	pin_flist_t		*s_flistp = NULL;
	int32			s_flags = 256;
	int32			status = 0;
	poid_t			*pdp = NULL;
	int64			db= 0;
	poid_t			*acct_pdp = NULL;
	int32			*srvc_type = NULL;
	char			*srvc_str = NULL;
	int				flags = 1;
	poid_t			*s_bal_pdp = NULL;
	poid_t			*s_bill_pdp = NULL;
	poid_t			*s_svc_pdp = NULL;
	pin_flist_t		*flistp = NULL;

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	srvc_type = (int32 *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp);

	db = PIN_POID_GET_DB(acct_pdp);
	if(*srvc_type == 0)
	{
		s_svc_pdp=PIN_POID_CREATE(db, "/service/broadband", id, ebufp);	
	}
	else if(*srvc_type == 1)
	{
		s_svc_pdp=PIN_POID_CREATE(db, "/service/catv", id, ebufp);
	}

	s_flistp = PIN_FLIST_CREATE(ebufp);

	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	s_bal_pdp = PIN_POID_CREATE(db, "/balance_group", id, ebufp);
	s_bill_pdp = PIN_POID_CREATE(db, "/billinfo", id, ebufp);

	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /billinfo 1, /balance_group 2, /service 3 where 1.F1 = V1 and  3.F2 = V2 and 3.F3.type = V3 and 2.F4 = 3.F5 and 1.F6 = 2.F7 ", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acct_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acct_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)s_svc_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)s_bal_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_BAL_GRP_OBJ, (void *)s_bal_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)s_bill_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILLINFO_OBJ, (void *)s_bill_pdp, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACTG_TYPE, (void *)NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"billinfo search input flist", s_flistp);
	//Retrieves the Billinfo POID and accounting type
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"billinfo search output flist", *r_flistpp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Billinfo Search Error", *r_flistpp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53001", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Billinfo Search Error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		return;
	}
	if (PIN_FLIST_ELEM_COUNT(*r_flistpp, PIN_FLD_RESULTS,ebufp) == 0)
	{
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		s_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /billinfo where F1 = V1 ", ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acct_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACTG_TYPE, (void *)NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"billinfo search input flist", s_flistp);
		//Retrieves the Billinfo POID and accounting type
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_FLAGS, &flags, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"billinfo search output flist", *r_flistpp);
		
		if (PIN_ERRBUF_IS_ERR(ebufp)) {

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "Billinfo Search Error", *r_flistpp);
			PIN_ERRBUF_RESET(ebufp);
			status = 1;
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53001", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Billinfo Search Error", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
			return;
		}
	}

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_POID_DESTROY(s_bill_pdp, NULL);
	PIN_POID_DESTROY(s_bal_pdp, NULL);
	PIN_POID_DESTROY(pdp, NULL);
	return;
}
*/

/* This module prepares the Input Flist & Invokes the PCM_OP_PYMT_COLLECT */
void mso_prep_pymt_collect(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	int			lco_blk_payment,
	pin_flist_t		**ret_flistpp,
	poid_t			**billinfo_pdpp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)1;
	u_int64			db = 0;
	poid_t			*acct_pdp = NULL;
	poid_t			*pdp = NULL;
	poid_t			*user_id = NULL;
	poid_t			*bal_grp_pdp = NULL;
	poid_t			*billinfo_pdp = NULL;
    poid_t          *dev_upg_plan_pdp = NULL;
    poid_t          *serv_pdp = NULL;
    poid_t          *srvc_pdp = NULL;
	char			*ref_no = NULL;
	char			*bank_name = NULL;
	char			*bank_code = NULL;
	char			*gateway   = NULL;
	char			*bank_acc_no = NULL;
	char			*chq_no = NULL;
	char			*branch_name = NULL;
	char			channel[128]="";
	char			channel_and_mode[128]="";
	char			ref_sp_code[128]="";
	char			*tag_acc_nop = NULL;
	//char			*calling_application = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	time_t			eff_t=0;
	int32			rec_id = 0;
	int32			sr_id = 0;
	int32			actg_type = 2; //Balance Forward Accounting Method
	int32			*pay_ch = NULL;
	int32			*pay_type = NULL;
	int32			status = 0;
	int32			*result = NULL;
	int32			*trans_type = NULL;
	int32			*serv_type = NULL;
	pin_flist_t		*ch_flistp = NULL;
	pin_flist_t		*ip_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*pymt_flistp = NULL;
	pin_flist_t		*p_flistp = NULL;
	pin_flist_t		*inh_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*serv_flistp = NULL;
	pin_flist_t		*serv_res_flistp = NULL;
	pin_flist_t		*sub_info_flistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*reto_flistp = NULL;
	pin_flist_t		*lco_flistp = NULL;
    pin_flist_t     *add_flistp = NULL;
    pin_flist_t     *add_opflistp = NULL;
    pin_flist_t     *add_args_flistp = NULL;
    pin_flist_t     *r_flistp = NULL;
	int32			*serv_objtype = NULL;
	int32			*coll_type = NULL;
	int32           alloc_flag = 16777216;//apply the payment to the account without allocating it. 
	int			lco_type = 0;
	int32 			*billflagp = NULL;
	int32 			 billflag = 0 ;
	int32           	err = 0;
	int32                   *over_flagp = NULL;
	void 			*vp = NULL;


	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = (u_int64)PIN_POID_GET_DB(acct_pdp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pcm_op_pymt_collect pre-input flist", i_flistp);

	ip_flistp = PIN_FLIST_CREATE(ebufp);
	err_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/account", id, ebufp);
	PIN_FLIST_FLD_PUT(ip_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, ip_flistp, PIN_FLD_PROGRAM_NAME, ebufp);

	coll_type = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE, 1, ebufp);

	serv_type = (int32 *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Invalid service_type passed");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, 
					"Select either CATV/Broadband service type", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
		goto CLEANUP;
	}
	
	//Check if the payment is done for LCO
	fm_mso_utils_get_lco_services_info(ctxp, i_flistp, &lco_flistp, 
					"/service/settlement/ws/incollect", ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unknown err while searching service information");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
					"Unknown err while searching service information", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
		goto CLEANUP;
	}
	if (lco_flistp)
	{
		result_flistp = PIN_FLIST_ELEM_GET(lco_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if(result_flistp != NULL)
			lco_type = 1;
	}

	if(lco_type == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "It's a LCO payment/topup");
		/* It's a LCO service. Get the broadband balance details */
		fm_mso_utils_get_billinfo_bal_details(ctxp, acct_pdp, BILLINFO_ID_WS, 
							&srch_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unknown err while searching balance");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
						"Unknown err while searching balance", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
			goto CLEANUP;
		}
		result_flistp = PIN_FLIST_ELEM_GET(srch_flistp, PIN_FLD_EXTRA_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if(result_flistp == NULL) {
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Invalid Service/Balance details");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
						"Invalid Service/Balance details", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
			goto CLEANUP;
		}
		bal_grp_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp);
		billinfo_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_BILLINFO_OBJ, 1, ebufp);
	}
	else if(*serv_type == 1)
	{
		PIN_ERR_LOG_MSG(3,"Inside  BB Payment");
		/* It's a Broadband service. Get the broadband balance details */
		fm_mso_utils_get_billinfo_bal_details_woserv(ctxp, acct_pdp, BILLINFO_ID_BB, 
							&srch_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unknown err while searching balance");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
						"Unknown err while searching balance", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
			goto CLEANUP;
		}
		result_flistp = PIN_FLIST_ELEM_GET(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if(result_flistp == NULL) {
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Invalid Service/Balance details");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
						"Invalid Service/Balance details", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
			goto CLEANUP;
		}
		bal_grp_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);
		billinfo_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	}
	else if(*serv_type == 0)
	{
		/* It's a CATV service. Get the CATV balance details */
		/*
		fm_mso_utils_get_billinfo_bal_details(ctxp, acct_pdp, BILLINFO_ID_CATV, 
							&srch_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Unknown err while searching balance");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
						"Unknown err while searching balance", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
			goto CLEANUP;
		}
		result_flistp = PIN_FLIST_ELEM_GET(srch_flistp, PIN_FLD_EXTRA_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if(result_flistp == NULL) {
			PIN_ERRBUF_CLEAR(ebufp);
			status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Invalid Service/Balance details");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
						"Invalid Service/Balance details", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
			goto CLEANUP;
		}
		bal_grp_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);
		billinfo_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
		*/
	}

	pay_ch = (int32 *)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_PYMT_CHANNEL, 0, ebufp);
	if(pay_ch && *pay_ch == 0)
	{
		sprintf(channel, "%s", "OAP Initiated Payment");
	}
	else if(pay_ch && *pay_ch == 1)
	{
		sprintf(channel, "%s", "SSA Initiated Payment");
	}
	else if(pay_ch && *pay_ch == 2)
	{
		sprintf(channel, "%s", "UPASS Initiated Payment");
	}
	else if(pay_ch && *pay_ch == 3)
	{
		sprintf(channel, "%s", "LCO Bulk Payment");
	}
	else if(pay_ch && *pay_ch == 4)
	{
		sprintf(channel, "%s", "E-Seva Payment");
	}
	else if(pay_ch && *pay_ch == 5)
	{
		sprintf(channel, "%s", "BillDesk Payment");
	}
	else if(pay_ch && *pay_ch == 6)
	{
		sprintf(channel, "%s", "IVR Payment");
	}
	else if(pay_ch && *pay_ch == 7)
	{
		sprintf(channel, "%s", "BATCH CC Payment");
	}
	else if(pay_ch && *pay_ch == 8)
	{
		sprintf(channel, "%s", "Batch DD or SI Payment");
	}
        else if(pay_ch && *pay_ch == 9)
        {
                sprintf(channel, "%s", "MOSAMBEE");
        }
	// New payment channels addition 10-JAN-2017
	else if (pay_ch && *pay_ch == 10)
	{
		sprintf(channel, "%s", "Mobile Application");
	}
	else if (pay_ch && *pay_ch == 11)
        {
                sprintf(channel, "%s", "Mobikwik");
        }
	else if (pay_ch && *pay_ch == 12)
        {
                sprintf(channel, "%s", "Paytm");
        }
	else if (pay_ch && *pay_ch == 13)
	{
		sprintf(channel, "%s", "MPOS");
	}
	else if (pay_ch && *pay_ch == 14)
        {
                sprintf(channel, "%s", "NEFT");
        }
       	// New payment channels addition 14-JUN-2018
        else if (pay_ch && *pay_ch == 15)
        {
                sprintf(channel, "%s", "Amazon Pay");
        }
        else if (pay_ch && *pay_ch == 16)
        {
                sprintf(channel, "%s", "Netflix");
        }
        else if (pay_ch && *pay_ch == 17)
        {
                sprintf(channel, "%s", "Dealer Wallet");
        }
        else if (pay_ch && *pay_ch == 18)
        {
                sprintf(channel, "%s", "MYJIO");
        }
        else if (pay_ch && *pay_ch == 19)
        {
                sprintf(channel, "%s", "PAYU");
        }
        else if (pay_ch && *pay_ch == 20)
        {
                sprintf(channel, "%s", "UPI");
        }

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHANNEL, 1, ebufp);
	if (vp && strcmp((char*)vp,"") != 0)
	{
		sprintf(channel_and_mode, "%s|%s", channel, (char*)vp);
	}
	else
	{
		sprintf(channel_and_mode, "%s", channel);
		PIN_FLIST_FLD_SET(ip_flistp, PIN_FLD_CHANNEL, channel, ebufp);
	}

	PIN_FLIST_FLD_SET(ip_flistp, PIN_FLD_DESCR, channel_and_mode, ebufp);

	over_flagp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OVERRIDE_FLAGS, 1, ebufp);

	while((ch_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_CHARGES, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"ch_flistp start", ch_flistp);
		pay_type = (int32 *)PIN_FLIST_FLD_GET(ch_flistp, PIN_FLD_PAY_TYPE, 0, ebufp);
		pymt_flistp = PIN_FLIST_SUBSTR_GET(ch_flistp, PIN_FLD_PAYMENT, 0, ebufp);
		if (!pymt_flistp || pymt_flistp ==(pin_flist_t*)NULL){
			status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Posting Error: Missing PIN_FLD_PAYMENT");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Posting Error: Missing INFO", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
			goto CLEANUP;
		}
		inh_flistp = PIN_FLIST_SUBSTR_GET(pymt_flistp, PIN_FLD_INHERITED_INFO, 0, ebufp);
		if (!inh_flistp || inh_flistp ==(pin_flist_t*)NULL){
			status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Posting Error: Missing INHERITED_INFO");
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Posting Error: Missing INFO", ebufp);
			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
			*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
			goto CLEANUP;
		}
		if( pay_type && ( *pay_type == PAY_TYPE_CASH || *pay_type == PAY_TYPE_TDS || *pay_type == PAY_TYPE_SP_AUTOPAY || *pay_type == PAY_TYPE_PORTAL_TRANSFER) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PAY_TYPE_CASH|PAY_TYPE_TDS|PAY_TYPE_SP_AUTOPAY||PAY_TYPE_PORTAL_TRANSFER");
			p_flistp = PIN_FLIST_ELEM_GET(inh_flistp, PIN_FLD_CASH_INFO, PIN_ELEMID_ANY, 1, ebufp);
			if (!p_flistp || p_flistp ==(pin_flist_t*)NULL){
				status = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Posting Error: Missing CASH_INFO");
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Posting Error: Missing INFO", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
				goto CLEANUP;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "p_flistp data flist", p_flistp);
			ref_no = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_RECEIPT_NO, 1, ebufp);
			vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);
			if(vp)
				eff_t = *(time_t *) PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);
		}
		else if (pay_type && *pay_type == PAY_TYPE_CHECK)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PAY_TYPE_CHECK");
			p_flistp = PIN_FLIST_ELEM_GET(inh_flistp, PIN_FLD_CHECK_INFO, PIN_ELEMID_ANY, 1, ebufp);
			if (!p_flistp || p_flistp ==(pin_flist_t*)NULL){
				status = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Posting Error: Missing CHECK_INFO");
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Posting Error: Missing INFO", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
				goto CLEANUP;
			}
			tmp_flistp = PIN_FLIST_COPY(p_flistp, ebufp);
			ref_no = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_RECEIPT_NO, 1, ebufp);
			if (ref_no)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Valid Value");
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Nothing");
			}
			chq_no = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CHECK_NO, 1, ebufp);
			bank_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BANK_NAME, 1, ebufp);
			bank_code = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BANK_CODE, 1, ebufp);
			branch_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BRANCH_NO, 1, ebufp);
			bank_acc_no = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BANK_ACCOUNT_NO, 1, ebufp);
			vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);
			if(vp)
				eff_t = *(time_t *) PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);
			
			/* added to populate these fields in mso_account_balance_t */
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_BANK_NAME, bank_name, ebufp);	
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_CHECK_NO, chq_no, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_EFFECTIVE_T, &eff_t, ebufp);
			/* ends here */
			PIN_FLIST_FLD_DROP(p_flistp, PIN_FLD_BRANCH_NO, ebufp);
			PIN_FLIST_FLD_DROP(p_flistp, PIN_FLD_BANK_NAME, ebufp);
			PIN_FLIST_FLD_DROP(p_flistp, PIN_FLD_RECEIPT_NO, ebufp);
		}
		else if (pay_type && (*pay_type == PAY_TYPE_ONLINE || *pay_type == PAY_TYPE_ECS))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"PAY_TYPE_ONLINE/ECS");
			p_flistp = PIN_FLIST_ELEM_GET(inh_flistp, PIN_FLD_WIRE_TRANSFER_INFO, PIN_ELEMID_ANY, 1, ebufp);
			if (!p_flistp || p_flistp ==(pin_flist_t*)NULL){
				status = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Posting Error: Missing TRANSFER_INFO");
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Posting Error: Missing INFO", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
				goto CLEANUP;
			}
			tmp_flistp = PIN_FLIST_COPY(p_flistp, ebufp);
			ref_no = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_WIRE_TRANSFER_ID, 1, ebufp);
			if (ref_no)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Valid Value");
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Nothing");
			}
			chq_no = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_CHECK_NO, 1, ebufp);
			bank_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BANK_NAME, 1, ebufp);
			bank_code = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BANK_CODE, 1, ebufp);
			branch_name = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BRANCH_NO, 1, ebufp);
			bank_acc_no = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_BANK_ACCOUNT_NO, 1, ebufp);
			gateway = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_IP_ADDRESS, 1, ebufp);
			trans_type = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TRANSACTION_TYPE, 1, ebufp);
			
			vp = PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);
			if(vp)
				eff_t = *(time_t *) PIN_FLIST_FLD_GET(tmp_flistp, PIN_FLD_EFFECTIVE_T, 1, ebufp);
			
			if (gateway)
				PIN_FLIST_FLD_DROP(p_flistp, PIN_FLD_IP_ADDRESS, ebufp);
			if (bank_name)
				PIN_FLIST_FLD_DROP(p_flistp, PIN_FLD_BANK_NAME, ebufp);
			if (branch_name)
				PIN_FLIST_FLD_DROP(p_flistp, PIN_FLD_BRANCH_NO, ebufp);
			if (chq_no)
				PIN_FLIST_FLD_DROP(p_flistp, PIN_FLD_CHECK_NO, ebufp);		
		}

		PIN_FLIST_FLD_SET(ch_flistp, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);

		if (over_flagp && *over_flagp == 1)
		{
			PIN_FLIST_FLD_SET(ch_flistp, PIN_FLD_CHANNEL_ID, over_flagp, ebufp);
		}

		/******************************************************************************
        	* Checking Whether billing is in progress before calling  PCM_OP_PYMT_COLLECT
        	******************************************************************************/
		PIN_FLIST_FLD_SET(ch_flistp, PIN_FLD_ACTG_TYPE, &actg_type, ebufp);
			

                pin_conf("fm_mso_pymt", "billing_window_flag", PIN_FLDT_INT,
                          (caddr_t *)&billflagp, &(err));
                if (billflagp) 
		{
                      billflag= *billflagp;
                      free(billflagp);	
		}
		
		if (billflag == 1)
		{
			PIN_FLIST_FLD_SET(ch_flistp, PIN_FLD_FLAGS, &alloc_flag, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"COMING_PAYMENT");

		}
		if( *serv_type != 0 )
		{
			PIN_FLIST_FLD_SET(ch_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_pdp, ebufp);
			PIN_FLIST_FLD_SET(ch_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_pdp, ebufp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"ch_flistp end", ch_flistp);
		PIN_FLIST_ELEM_SET(ip_flistp, (pin_flist_t *)ch_flistp, PIN_FLD_CHARGES, rec_id, ebufp);
	}

	 /***********************************************************
	 * Invoking PCM_OP_PYMT_COLLECT
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"pcm_op_pymt_collect input flist", ip_flistp);

	PCM_OP(ctxp, PCM_OP_PYMT_COLLECT, 0 , ip_flistp, &reto_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"pcm_op_pymt_collect output flist", reto_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Posting Error");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Posting Error", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
		goto CLEANUP;
	}

	rs_flistp = PIN_FLIST_ELEM_GET(reto_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	result = (int32 *)PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_RESULT, 1, ebufp);
	if (result && *result == 0)
	{
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Posting Error");
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Posting Error", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
		goto CLEANUP;
	}
	else
	{
		/********************************************************************
		* If the payment is directly done by a end_customer under LCO, then the
		* same amount has to be debited from end_customer and credited to LCO.
		********************************************************************/
		/* If the opcode is called by mso_bulk_payment_posting MTA, do nothing */
	//	if(lco_blk_payment == 0 && lco_type == 0)
	//	{
			/*******************************
			* Get the service object details
			********************************/
			/*
			fm_mso_utils_get_all_service_details(ctxp, acct_pdp, *serv_type, &serv_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp) || serv_flistp == NULL)
			{
				PIN_ERRBUF_CLEAR(ebufp);
				status = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error searching the service");
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
							"Error searching the service", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
				goto CLEANUP;
			}
			serv_res_flistp = PIN_FLIST_ELEM_GET(serv_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			if (serv_res_flistp == NULL)
			{
				PIN_ERRBUF_CLEAR(ebufp);
				status = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No services attached to the account!");
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
							"No services attached to the account!", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
				goto CLEANUP;

			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "serv_res_flistp is: ", serv_res_flistp);
			if(*serv_type == 0)
			{
				sub_info_flistp = PIN_FLIST_SUBSTR_GET(serv_res_flistp, MSO_FLD_CATV_INFO,
							0, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "sub_info_flistp is: ", sub_info_flistp);
			}
			else if(*serv_type == 1)
			{
				sub_info_flistp = PIN_FLIST_SUBSTR_GET(serv_res_flistp, MSO_FLD_BB_INFO,
							0, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "sub_info_flistp is: ", sub_info_flistp);
			}
			serv_objtype = PIN_FLIST_FLD_GET(sub_info_flistp, MSO_FLD_COMMISION_MODEL, 1, ebufp);
			if (serv_objtype != NULL && (*serv_objtype == 1 || *serv_objtype == 2))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Its a LCO end customer payment");
				fm_mso_pymt_process_lco_payment(ctxp, i_flistp, bal_grp_pdp, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_ERRBUF_CLEAR(ebufp);
					status = 1;
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error processing LCO Balance Transfer");
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, acct_pdp, ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53002", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,
								"Error processing LCO Balance Transfer", ebufp);
					PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
					*ret_flistpp = PIN_FLIST_COPY(err_flistp,ebufp);
					goto CLEANUP;
				}
			}
		}
		*/
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, reto_flistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, reto_flistp, PIN_FLD_DESCR, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, reto_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, reto_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TYPE, reto_flistp, PIN_FLD_TYPE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, reto_flistp, PIN_FLD_START_T, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, reto_flistp, PIN_FLD_END_T, ebufp);
		tag_acc_nop = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_TAG, 1, ebufp);
		if (tag_acc_nop)
		{
			strcpy(ref_sp_code, tag_acc_nop);
			strcat(ref_sp_code, "|");
			strcat(ref_sp_code, ref_no);
			PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_REFERENCE_ID, ref_sp_code, ebufp);
		}
		else
		{
			PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_REFERENCE_ID, ref_no, ebufp);
		}
		if(trans_type)
			PIN_FLIST_FLD_SET(reto_flistp, MSO_FLD_TRANSACTION_TYPE, trans_type, ebufp);
		PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_PAY_TYPE, pay_type, ebufp);
		PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_EFFECTIVE_T, &eff_t, ebufp);		
		PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_CHECK_NO, chq_no, ebufp);
		PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_BANK_NAME, bank_name, ebufp);
		PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_BANK_CODE, bank_code, ebufp);
		PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_BRANCH_NO, branch_name, ebufp);
		PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_BANK_ACCOUNT_NO, bank_acc_no, ebufp);
		PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_STATUS, &status, ebufp);
		if (gateway)
		{
			PIN_FLIST_FLD_SET(reto_flistp, PIN_FLD_IP_ADDRESS, gateway, ebufp);
		}		
		*ret_flistpp = PIN_FLIST_COPY(reto_flistp,ebufp);
	}

CLEANUP:
/*	if (ref_no)
	{
		pin_free(ref_no);
	}
*/
        if( *serv_type != 0 )
        {
		*billinfo_pdpp = PIN_POID_COPY(billinfo_pdp, ebufp);
	}
	PIN_FLIST_DESTROY_EX(&ip_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&serv_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&reto_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&lco_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&tmp_flistp, NULL);
	return;
}

/* This module generates the Payment Receipt for all the Payment modes & Payment Channels */
void mso_pymt_receipt_create(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*ip_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	poid_t			*rcpt_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*item_pdp = NULL;
	poid_t			*evt_pdp = NULL;
	poid_t			*receipt_pdp = NULL;
	poid_t			*user_id = NULL;
	pin_flist_t		*bal_flistp = NULL;
	pin_flist_t		*s_r_flistp = NULL;
	pin_flist_t		*op_flistp = NULL;
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*rcpt_flistp = NULL;
	pin_flist_t		*acct_flistp = NULL;
	pin_flist_t		*acct_nm_flistp = NULL;
	pin_flist_t		*profile_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	

	pin_cookie_t		cookie = NULL;
	char			*rcpt_no = NULL;
	char			*evt_str = NULL;
	char			poid_buf[PCM_MAX_POID_TYPE + 1];
	char			*fname = NULL;
	char			*mname = NULL;
	char			*lname = NULL;
	char			name[128] = "";
	char			*addr = NULL;
	char			*city = NULL;
	char			*state = NULL;
	char			*country = NULL;
	char			*zip = NULL;
	char			f_addr[512] = "";
	char			*acct_no = NULL;
	char			channel[128]="";
	char			channel_and_mode[128]="";
	void			*vp = NULL;
	
	int32			len = 0;
	int32			rec_id = 0;
	int32                   *pay_ch = NULL;
	int32			status = 0;
	int32			*pay_type = NULL;
	int32			*srvc_type = NULL;
	int32			catv_service = 0;
	int32			pp_type = -1;
	int64			id = (int64)-1;
	int64			acc_no = 0;
	int32			ret_flag = -1;
	u_int64			db = 0;
	pin_decimal_t		*amt = NULL;
	pin_decimal_t		*t_amt = NULL;

	

	in_flistp = PIN_FLIST_CREATE(ebufp);
	r_flistp = PIN_FLIST_CREATE(ebufp);
	acct_flistp = PIN_FLIST_CREATE(ebufp);
	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Posting Error");
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, user_id, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Payment Posting Error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_pymt_receipt_create ip_flistp", ip_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_pymt_receipt_create i_flistp", i_flistp);

//	op_flistp = PIN_FLIST_CREATE(ebufp);
		s_r_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);

		acct_pdp = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		srvc_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp);
		/* retrieving pp type in case of catv */
		if (*srvc_type == catv_service)
		{
			pp_type = fm_mso_get_pp_type(ctxp, acct_pdp, ebufp);
		}
		db = (u_int64)PIN_POID_GET_DB(acct_pdp);
		evt_pdp = PIN_FLIST_FLD_GET(s_r_flistp, PIN_FLD_POID, 0, ebufp);
			
		bal_flistp = PIN_FLIST_ELEM_GET(s_r_flistp, PIN_FLD_BAL_IMPACTS, 0, 0, ebufp);
		item_pdp = PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Balances from results", bal_flistp);

		amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(bal_flistp, PIN_FLD_AMOUNT, 1, ebufp);
		if (pbo_decimal_is_null(amt, ebufp))
		{
			t_amt = pbo_decimal_from_str("0.0", ebufp);
		}
		else
		{
			t_amt = pbo_decimal_negate(amt, ebufp);
		}
//		acc_no = PIN_POID_GET_ID(acct_pdp);
		//Concantenating the Receipt_No as AccountID_CurrentTimestamp
//		sprintf(ref_no, "%d",acc_no);
//		sprintf(time_str, "_%d", pin_virtual_time(NULL));
//		strcat(ref_no, time_str);

		//Generating the Sequence for Receipts
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_NAME, SEQ_TYPE_RECEIPT, ebufp);
		fm_mso_utils_sequence_no(ctxp, in_flistp, &r_flistp, ebufp);
//		rcpt_no = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_STRING, 0, ebufp);

		//Reading the Account attributes
		PIN_FLIST_FLD_SET(acct_flistp, PIN_FLD_POID, (void *)acct_pdp, ebufp);
		PIN_FLIST_FLD_SET(acct_flistp, PIN_FLD_ACCOUNT_NO, "", ebufp);
		acct_nm_flistp = PIN_FLIST_ELEM_ADD(acct_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
		PIN_FLIST_FLD_SET(acct_nm_flistp, PIN_FLD_FIRST_NAME, "", ebufp);
		PIN_FLIST_FLD_SET(acct_nm_flistp, PIN_FLD_MIDDLE_NAME, "", ebufp);
		PIN_FLIST_FLD_SET(acct_nm_flistp, PIN_FLD_LAST_NAME, "", ebufp);
		PIN_FLIST_FLD_SET(acct_nm_flistp, PIN_FLD_ADDRESS, "", ebufp);
		PIN_FLIST_FLD_SET(acct_nm_flistp, PIN_FLD_CITY, "", ebufp);
		PIN_FLIST_FLD_SET(acct_nm_flistp, PIN_FLD_STATE, "", ebufp);
		PIN_FLIST_FLD_SET(acct_nm_flistp, PIN_FLD_COUNTRY, "", ebufp);
		PIN_FLIST_FLD_SET(acct_nm_flistp, PIN_FLD_ZIP, "", ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account Attributes Fetch Input Flist", acct_nm_flistp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, acct_flistp, &op_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account Attributes Fetch Output Flist", op_flistp);

//		acct_no = PIN_FLIST_FLD_GET(op_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		acct_nm_flistp = PIN_FLIST_ELEM_TAKE(op_flistp, PIN_FLD_NAMEINFO, 1, 0, ebufp);
		fname = PIN_FLIST_FLD_GET(acct_nm_flistp, PIN_FLD_FIRST_NAME, 0, ebufp);
		mname = PIN_FLIST_FLD_GET(acct_nm_flistp, PIN_FLD_MIDDLE_NAME, 0, ebufp);
		lname = PIN_FLIST_FLD_GET(acct_nm_flistp, PIN_FLD_LAST_NAME, 0, ebufp);
		sprintf(name, "%s %s %s", fname, mname, lname);

		addr = PIN_FLIST_FLD_GET(acct_nm_flistp, PIN_FLD_ADDRESS, 0, ebufp);
		city = PIN_FLIST_FLD_GET(acct_nm_flistp, PIN_FLD_CITY, 0, ebufp);
		state = PIN_FLIST_FLD_GET(acct_nm_flistp, PIN_FLD_STATE, 0, ebufp);
		country = PIN_FLIST_FLD_GET(acct_nm_flistp, PIN_FLD_COUNTRY, 0, ebufp);
		zip = PIN_FLIST_FLD_GET(acct_nm_flistp, PIN_FLD_ZIP, 0, ebufp);
//		memset(f_addr,'\0',sizeof(f_addr));
		sprintf(f_addr, "%s, %s, %s, %s - %s", addr, city, state, country, zip);
		
		rcpt_flistp = PIN_FLIST_CREATE(ebufp);
		rcpt_pdp = PIN_POID_CREATE(db, "/mso_receipt", id, ebufp);
		PIN_FLIST_FLD_PUT(rcpt_flistp, PIN_FLD_POID, rcpt_pdp, ebufp);
		PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
		PIN_FLIST_FLD_COPY(op_flistp, PIN_FLD_ACCOUNT_NO, rcpt_flistp, PIN_FLD_ACCOUNT_NO, ebufp);
		PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_STRING, rcpt_flistp, PIN_FLD_RECEIPT_NO, ebufp);
		
		PIN_FLIST_FLD_PUT(rcpt_flistp, PIN_FLD_AMOUNT, (void *)t_amt, ebufp);
		PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_NAME, name, ebufp);
		PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_ADDRESS, f_addr, ebufp);
		
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, rcpt_flistp, PIN_FLD_DESCR, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REFERENCE_ID, rcpt_flistp, PIN_FLD_REFERENCE_ID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PAY_TYPE, rcpt_flistp, PIN_FLD_PAY_TYPE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, rcpt_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, rcpt_flistp, MSO_FLD_SERVICE_TYPE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_EFFECTIVE_T, rcpt_flistp, PIN_FLD_EFFECTIVE_T, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_NAME, rcpt_flistp, PIN_FLD_BANK_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_CODE, rcpt_flistp, PIN_FLD_BANK_CODE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_IP_ADDRESS, rcpt_flistp, PIN_FLD_IP_ADDRESS, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BRANCH_NO, rcpt_flistp, PIN_FLD_BRANCH_NO, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_CHECK_NO, rcpt_flistp, PIN_FLD_CHECK_NO, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TYPE, rcpt_flistp, PIN_FLD_TYPE, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BANK_ACCOUNT_NO, rcpt_flistp, PIN_FLD_BANK_ACCOUNT_NO, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_START_T, rcpt_flistp, PIN_FLD_START_T, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_END_T, rcpt_flistp, PIN_FLD_END_T, ebufp);
		PIN_FLIST_FLD_COPY(ip_flistp, MSO_FLD_PYMT_CHANNEL, rcpt_flistp, MSO_FLD_PYMT_CHANNEL, ebufp);
		// Populating PP type in case of catv service 
		if(*srvc_type == catv_service )
		{
			PIN_FLIST_FLD_SET(rcpt_flistp, MSO_FLD_PP_TYPE, &pp_type, ebufp);
		}
		pay_ch = (int32 *)PIN_FLIST_FLD_GET(ip_flistp, MSO_FLD_PYMT_CHANNEL, 0, ebufp);
		if(pay_ch && *pay_ch == 0)
		{
			sprintf(channel, "%s", "OAP Initiated Payment");
		}
		else if(pay_ch && *pay_ch == 1)
		{
			sprintf(channel, "%s", "SSA Initiated Payment");
		}
		else if(pay_ch && *pay_ch == 2)
		{
			sprintf(channel, "%s", "UPASS Initiated Payment");
		}
		else if(pay_ch && *pay_ch == 3)
		{
			sprintf(channel, "%s", "LCO Bulk Payment");
		}
		else if(pay_ch && *pay_ch == 4)
		{
			sprintf(channel, "%s", "E-Seva Payment");
		}
		else if(pay_ch && *pay_ch == 5)
		{
			sprintf(channel, "%s", "BillDesk Payment");
		}
		else if(pay_ch && *pay_ch == 6)
		{
			sprintf(channel, "%s", "IVR Payment");
		}
		else if(pay_ch && *pay_ch == 7)
		{
			sprintf(channel, "%s", "BATCH CC Payment");
		}
		else if(pay_ch && *pay_ch == 8)
		{
			sprintf(channel, "%s", "Batch DD or SI Payment");
		}
        	else if(pay_ch && *pay_ch == 9)
        	{
                	sprintf(channel, "%s", "MOSAMBEE");
        	}
	       	//New payment channels added 10-JAN-2017 
        	else if (pay_ch && *pay_ch == 10)
        	{
                	sprintf(channel, "%s", "Mobile Application");
        	}
		else if (pay_ch && *pay_ch == 11)
	        {
        	        sprintf(channel, "%s", "Mobikwik");
        	}	
        	else if (pay_ch && *pay_ch == 12)
        	{
                	sprintf(channel, "%s", "Paytm");
        	}
	        else if (pay_ch && *pay_ch == 13)
	        {
        	        sprintf(channel, "%s", "MPOS");
       		}
		else if (pay_ch && *pay_ch == 14)
                {
                        sprintf(channel, "%s", "NEFT");
                }
                // New payment channels addition 14-JUN-2018
                else if (pay_ch && *pay_ch == 15)
                {
                        sprintf(channel, "%s", "Amazon Pay");
                }
                else if (pay_ch && *pay_ch == 16)
                {
                        sprintf(channel, "%s", "Netflix");
                }
        	  else if (pay_ch && *pay_ch == 17)
       	  	{
                 	   sprintf(channel, "%s", "Dealer Wallet");
        	}		
                else if (pay_ch && *pay_ch == 18)
                {
                           sprintf(channel, "%s", "MYJIO");
                }
                else if (pay_ch && *pay_ch == 19)
                {
                           sprintf(channel, "%s", "PAYU");
                }
                else if (pay_ch && *pay_ch == 20)
                {
                           sprintf(channel, "%s", "UPI");
                }



		vp = PIN_FLIST_FLD_GET(ip_flistp, PIN_FLD_CHANNEL, 1, ebufp);
		if (vp && strcmp((char*)vp,"") != 0)
		{
			sprintf(channel_and_mode, "%s|%s", channel, (char*)vp);
		}
		else
		{
			sprintf(channel_and_mode, "%s", channel);
		}
		PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_CHANNEL, channel_and_mode, ebufp);


		//PIN_FLIST_FLD_COPY(ip_flistp, PIN_FLD_CHANNEL, rcpt_flistp, PIN_FLD_CHANNEL, ebufp);
		PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_ITEM_OBJ, item_pdp, ebufp);
		PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_EVENT_OBJ, evt_pdp, ebufp);
		PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_USERID, user_id, ebufp);

		//get customer GSTIN number if available
        	ret_flag = fm_mso_cust_get_gst_profile(ctxp, acct_pdp, &profile_flistp, ebufp);
        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Error searching gst profile");
                	PIN_ERRBUF_CLEAR(ebufp);
                	return;
        	}

        	if(profile_flistp)
        	{
                	tmp_flistp = PIN_FLIST_ELEM_GET(profile_flistp, MSO_FLD_GST_INFO, PIN_ELEMID_ANY, 1, ebufp);
                	if(tmp_flistp)
                	{
                       	 	PIN_FLIST_FLD_COPY(tmp_flistp, PIN_FLD_ACCOUNT_NO, rcpt_flistp, MSO_FLD_GSTIN, ebufp);
                	}
        	}
		
		op_flistp = NULL;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Receipts Input Flist", rcpt_flistp);
		//Creating the Payment Receipt Object(/mso_receipt)
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, rcpt_flistp, &op_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Receipts Output Flist", op_flistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			status = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Payment Receipt Creation Error");
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, user_id, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53003", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Payment Receipt Creation Error", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
			return;
		}
//		receipt_pdp = PIN_FLIST_FLD_GET(op_flistp, PIN_FLD_POID, 0, ebufp);
//		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, (void *)receipt_pdp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Receipt Read Input Flist", op_flistp);
		PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, op_flistp, r_flistpp, ebufp);
		PIN_FLIST_FLD_DROP(*r_flistpp,PIN_FLD_MOD_T, ebufp);
		PIN_FLIST_FLD_DROP(*r_flistpp,PIN_FLD_READ_ACCESS, ebufp);
		PIN_FLIST_FLD_DROP(*r_flistpp,PIN_FLD_WRITE_ACCESS, ebufp);
		PIN_FLIST_FLD_DROP(*r_flistpp,PIN_FLD_EVENT_OBJ, ebufp);
		PIN_FLIST_FLD_DROP(*r_flistpp,PIN_FLD_ITEM_OBJ, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Receipt Read Output Flist", *r_flistpp);
//	op_flistp = NULL;

	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&op_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rcpt_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&acct_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
//	PIN_FLIST_DESTROY_EX(&bal_flistp, NULL);

//	PIN_POID_DESTROY(&acct_pdp,ebufp);

/*	if (f_addr != NULL)
	{
		free(f_addr);
	}
	if (name != NULL)
	{
		free(name);
	}
	if (!pbo_decimal_is_null(amt, ebufp))
	{
		pbo_decimal_destroy(&amt);
	}
*/
	return ;
}

/*
static void mso_pymt_apply_late_fee(
	pcm_context_t	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*adj_flistp = NULL;
    poid_t			*pdp = NULL;
	poid_t			*bill_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	int64			db = 0;
	int32			*d_status = NULL;
	int32			s_flags = 512;
	int32			days = 0;
	int32			bill_adj = 0;
	int32			errp = 0;
	int				tax = WITH_TAX;
	u_int64			id = (u_int64)-1;
	time_t			*due_t = NULL;
	time_t			now_t = 0;
	time_t			diff_t = 0;
	pin_decimal_t	*amt = NULL;
	pin_decimal_t	*interest = NULL;
	pin_decimal_t	*total_days = NULL;
	char			msg[20];
	void			*vp = NULL;
	
		amt = pbo_decimal_from_str("0.0", ebufp);
		interest = pbo_decimal_from_str("0.0", ebufp);
		total_days = pbo_decimal_from_str("0.0", ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Late Fee input flist", i_flistp);
		acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	
		db = PIN_POID_GET_DB(acct_pdp);
		s_flistp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /bill where F1 = V1 and F2 != V2 order by F3 desc", ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BILLINFO_OBJ, flistp, PIN_FLD_BILLINFO_OBJ,  ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILL_NO, "(null)", ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_CREATED_T, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_DUE_T, NULL, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TOTAL_DUE, NULL, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Bill search input flist", s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"Bill search output flist",*r_flistpp);

		rs_flistp = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_RESULTS, 1, ebufp);
		amt = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_TOTAL_DUE, 1, ebufp);
		due_t = PIN_FLIST_FLD_GET(rs_flistp, PIN_FLD_DUE_T, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,"No Bill Created for the Customer Account, Hence No Late fee!", ebufp);
			PIN_ERRBUF_CLEAR(ebufp);
			return;
		}
		sprintf(msg, "Due Date %d.", *due_t); 
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

		now_t = pin_virtual_time((time_t *)NULL);
		sprintf(msg, "Current Date is %d.", now_t); 
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

		if (now_t >= *due_t)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Eligible for Late Fee");
			diff_t = now_t - (*due_t);
			days = (diff_t /86400);
						
			vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
			interest = pbo_decimal_copy((pin_decimal_t*)vp, ebufp);

			vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TOTALS, 0, ebufp);
			total_days = pbo_decimal_copy((pin_decimal_t*)vp, ebufp);
			
			interest = pbo_decimal_divide(interest, total_days, ebufp);
			amt = pbo_decimal_multiply(amt, interest, ebufp);
			amt = pbo_decimal_multiply(amt, pbo_decimal_from_double(days, ebufp), ebufp);

			//Calling MSO_OP_AR_ADJUSTMENT
			adj_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(rs_flistp, PIN_FLD_POID, adj_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(adj_flistp, PIN_FLD_AMOUNT, amt, ebufp);
			PIN_FLIST_FLD_SET(adj_flistp, PIN_FLD_PROGRAM_NAME, "Late Fee Adjustment", ebufp);
			PIN_FLIST_FLD_SET(adj_flistp, PIN_FLD_DESCR, "Late Fee Adjustment", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_FLAGS, adj_flistp, PIN_FLD_FLAGS, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TAX_FLAGS, adj_flistp, PIN_FLD_TAX_FLAGS, ebufp);
			PIN_FLIST_FLD_SET(adj_flistp, PIN_FLD_USERID, acct_pdp, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,   "Late Fee adjustment input flist", adj_flistp);
	        PCM_OP(ctxp, MSO_OP_AR_ADJUSTMENT, 0, adj_flistp, r_flistpp, ebufp);
		    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "Late Fee adjustment output flist",*r_flistpp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not Eligible for Late Fee - Current Date is lesser than Due Date");
		}

//		PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);

	if (amt)
	{
		pbo_decimal_destroy(&amt);
	}
	if (interest)
	{
		pbo_decimal_destroy(&interest);
	}
	if (total_days)
	{
		pbo_decimal_destroy(&total_days);
	}
return;
}
*/

static void mso_pymt_service_reactivation(
	pcm_context_t	*ctxp,
	poid_t			*billinfo_pdp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*srvc_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t	*ebufp)
{
		//pcm_context_t   *ctxp_tmp = NULL;
		u_int64         id = (u_int64)-1;
		pin_flist_t     *s_flistp = NULL;
		pin_flist_t		*flistp = NULL;
		pin_flist_t		*res_flistp = NULL;
		pin_flist_t		*act_flistp = NULL;
		int32           s_flags = 1;
		int32           *s_type = NULL;
		int32           *status = NULL;
		int32			rec_id = 0;
        int32           *enable_flag = NULL;
		int32			activate = STATUS_ACTIVE;
		pin_cookie_t    cookie = NULL;
		int64           db = 0;
		poid_t          *binfo_pdp = NULL;
		pin_decimal_t	*amount = NULL;
		pin_decimal_t	*zero = NULL;
		pin_decimal_t   *unapplied_amt = NULL;
		pin_decimal_t   *pending_bill_amt = NULL;
		void			*vp = NULL;
		
		PIN_ERRBUF_CLEAR(ebufp);
		amount = pbo_decimal_from_str("0.0",ebufp);
		zero = pbo_decimal_from_str("15.0",ebufp);
		
		s_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, billinfo_pdp, ebufp);
//		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_AR_BILLINFO_OBJ, billinfo_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_INCLUDE_CHILDREN, &s_flags, ebufp);
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Get Balance Summary input flist", s_flistp);
//		PCM_OP(ctxp, PCM_OP_AR_GET_BAL_SUMMARY, 0, s_flistp, r_flistpp, ebufp);
		PCM_OP(ctxp, PCM_OP_AR_GET_ACCT_BAL_SUMMARY, 0, s_flistp, r_flistpp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Get Balance Summary output flist",*r_flistpp);

		vp = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_OPENBILL_DUE, 1, ebufp);
		amount = pbo_decimal_copy((pin_decimal_t*)vp, ebufp);
		vp = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_UNAPPLIED_AMOUNT, 1, ebufp);
		unapplied_amt = pbo_decimal_copy((pin_decimal_t*)vp, ebufp);
		pbo_decimal_add_assign( amount, unapplied_amt, ebufp );
		vp = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_PENDINGBILL_DUE, 1, ebufp);
		pending_bill_amt = pbo_decimal_copy((pin_decimal_t*)vp, ebufp); 
		pbo_decimal_add_assign( amount, pending_bill_amt, ebufp );
		if (amount)
		{
            // Getting the Flag for Service Reactivation based on Tolerance Amount for BB
            enable_flag = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_ID_TYPE, 1, ebufp);
			if((pbo_decimal_compare(amount, zero, ebufp) != 1) || (enable_flag && *enable_flag == 1))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Total Due Lesser than or Equal to Zero OR Tolerance based Service Enablement");
//				fm_mso_retrieve_service(ctxp, binfo_pdp, r_flistpp, ebufp);
		
				while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srvc_flistp, PIN_FLD_RESULTS, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
				{
					status = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_STATUS, 1, ebufp);
						
					if (*status == STATUS_ACTIVE || *status == STATUS_INACTIVE)
					{
							
						/*****************************************************
						PROGRAM_NAME Should be standard ie. CHANNEL|username
						"Service Re-Activation after Payment" should be stored 
 						in PIN_FLD_DESCR
						-Gautam
						******************************************************/
						act_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, act_flistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_SET(act_flistp, PIN_FLD_PROGRAM_NAME, "Service Re-Activation after Payment", ebufp);
						PIN_FLIST_FLD_SET(act_flistp, PIN_FLD_DESCR, "MSO_OP_PYMT_COLLECT", ebufp);
						PIN_FLIST_FLD_SET(act_flistp, PIN_FLD_STATUS, &activate, ebufp);
						PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, act_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, act_flistp, PIN_FLD_USERID, ebufp);

						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service Activation input flist", act_flistp);
						//ctxp_tmp = NULL;
						//PCM_CONTEXT_OPEN(&ctxp_tmp, (pin_flist_t *)0, ebufp);
						PCM_OP(ctxp, MSO_OP_CUST_REACTIVATE_SERVICE, 0, act_flistp, &s_flistp, ebufp);
						//PCM_CONTEXT_CLOSE(ctxp_tmp, 0, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service Activation output flist", s_flistp);
				
						PIN_FLIST_DESTROY_EX(&act_flistp, ebufp);
					}
					else
					{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No Service Reactivation Required");
					}
				}
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Total Due not Lesser than or Equal to Zero");
			}
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Amount Retrieval Error");
			PIN_ERRBUF_CLEAR(ebufp);
			return;
		}
	
		if (!pbo_decimal_is_null(amount, ebufp))
			pbo_decimal_destroy(&amount);
		if (!pbo_decimal_is_null(zero, ebufp))
			pbo_decimal_destroy(&zero);
		if (!pbo_decimal_is_null(unapplied_amt, ebufp))
			pbo_decimal_destroy(&unapplied_amt);
 		if (!pbo_decimal_is_null(pending_bill_amt, ebufp))
                        pbo_decimal_destroy(&pending_bill_amt);	

		PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);

return;
}

EXPORT_OP void fm_mso_retrieve_service(
	pcm_context_t		*ctxp,
	poid_t				*binfo_pdp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	u_int64			id = (u_int64)-1;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int32			s_flags = 512;
	int64			db = 0;
	poid_t			*pdp = NULL;

		db = PIN_POID_GET_DB(binfo_pdp);

		s_flistp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /service 1, /balance_group 2, /billinfo 3 where 1.F1 = 2.F2 and 2.F3 = 3.F4 and 3.F5 = V5", ebufp);

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

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_STATUS, (void *)NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search input flist", s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search output flist",*r_flistpp);
		
		PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
		return;
}

/**************************************************************************
* Function: 	fm_mso_pymt_process_lco_payment()
* Owner:        Hrish Kulkarni
* Decription:   For transfering balance from source to LCO account.
**************************************************************************/
static void fm_mso_pymt_process_lco_payment(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	poid_t				*bal_grp_pdp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*profile_iflistp = NULL;
	pin_flist_t			*profile_oflistp = NULL;
	pin_flist_t			*custcare_flistp = NULL;
	pin_flist_t			*lco_acct_flistp = NULL;
	pin_flist_t			*charge_flistp = NULL;
	pin_flist_t			*ret_flistp = NULL;
	pin_flist_t			*result_flistp = NULL;
	pin_flist_t			*lco_acnt_info = NULL;

	void				*vp = NULL;

	pin_decimal_t		*amount = NULL;
	pin_decimal_t		*abs_amount = NULL;
	poid_t				*acct_pdp = NULL;
	poid_t				*lco_acctp = NULL;
	poid_t				*lco_balp = NULL;
	int32				srch_type = LCO_CUST_CARE;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error before calling fm_mso_pymt_process_lco_payment", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_pymt_process_lco_payment");

	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	charge_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_CHARGES, PIN_ELEMID_ANY, 0, ebufp);
	amount = PIN_FLIST_FLD_GET(charge_flistp, PIN_FLD_AMOUNT, 0, ebufp);

	profile_iflistp= PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(profile_iflistp, PIN_FLD_POID, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(profile_iflistp, PIN_FLD_OBJECT, acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(profile_iflistp, PIN_FLD_PROFILE_DESCR, "/profile/customer_care", ebufp);
	PIN_FLIST_FLD_SET(profile_iflistp, PIN_FLD_SEARCH_TYPE, &srch_type, ebufp);

	/**********************************************************
	* Read the profile of account to get the LCO details
	***********************************************************/
	fm_mso_get_profile_info(ctxp, profile_iflistp, &profile_oflistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error reading the /profile class !!");
		goto CLEANUP;
	}
	if (profile_oflistp != NULL)
	{
		custcare_flistp = PIN_FLIST_SUBSTR_GET(profile_oflistp, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);
		lco_acctp = PIN_FLIST_FLD_GET(custcare_flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
		if (lco_acctp)
		{
			fm_mso_get_account_info(ctxp, lco_acctp, &lco_acnt_info, ebufp);
			vp = PIN_FLIST_ELEM_ADD(i_flistp, MSO_FLD_LCOINFO, 0, ebufp);
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
	* Get the account balance group of the LCO account
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
	* Do the balance transfer from end_customer to LCO account
	***********************************************************/
	abs_amount =  pbo_decimal_abs(amount,ebufp);
	fm_mso_utils_balance_transfer(ctxp, acct_pdp, lco_acctp, bal_grp_pdp, 
				lco_balp, abs_amount, &ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error transfering the balance !!");
		goto CLEANUP;
	}

	// Lifecycle event for source account
	//fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_PAYMENT, ebufp );
	//mso_op_payment_gen_lc_event(ctxp, i_flistp, ret_flistp, BAL_TRANS_LCO_SOURCE, amount, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Lifecycle event creation BAL_TRANS_LCO_SOURCE !!");
		goto CLEANUP;
	}
	// Lifecycle event for Destination(LCO) account
	//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_NAME, BAL_TRANS_LCO_DEST, ebufp );
	//PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_AMOUNT, amount, ebufp );
	//fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_PAYMENT, ebufp );
	//mso_op_payment_gen_lc_event(ctxp, i_flistp, ret_flistp, BAL_TRANS_LCO_DEST, 
	//				amount, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Error Lifecycle event creation BAL_TRANS_LCO_DEST !!");
		goto CLEANUP;
	}
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&profile_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&profile_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&lco_acct_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		pbo_decimal_destroy(&abs_amount);
	return;
}

/*******************************************************
* This function creates the lifecycle event for payment 
*******************************************************/
//static void
//mso_op_payment_gen_lc_event(
//	pcm_context_t           *ctxp,
//	pin_flist_t             *i_flistp,
//	pin_flist_t             *s_oflistp,
//	char                    *disp_name,
//	pin_decimal_t		*trans_amt,
//	pin_errbuf_t            *ebufp)
//{
//	pin_flist_t             *lc_iflistp = NULL;
//	pin_flist_t             *lc_temp_flistp = NULL;
//	pin_flist_t             *res_flistp = NULL;
//	pin_flist_t             *charge_flistp = NULL;
//	poid_t                  *acct_pdp = NULL;
//	char                    *event = "/event/activity/mso_lifecycle/payment";
//	pin_decimal_t		*dec_amount = NULL;
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//			return;
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_payment_gen_lc_event Input FList:", i_flistp);
//
//	lc_iflistp = PIN_FLIST_CREATE(ebufp);
//	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_PAYMENT, 0, ebufp);
//	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
//
//	if(strcmp(disp_name, PAYMENT_COLLECT) == 0)
//	{
//		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
//		if(res_flistp)
//			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);
//
//		charge_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_CHARGES, PIN_ELEMID_ANY, 1, ebufp);
//		if(charge_flistp)
//		{
//			dec_amount = pbo_decimal_negate(PIN_FLIST_FLD_GET(charge_flistp, PIN_FLD_AMOUNT,0,ebufp), ebufp);
//			PIN_FLIST_FLD_PUT(lc_temp_flistp, PIN_FLD_AMOUNT, dec_amount, ebufp);
//		}
//	}
//	else if(strcmp(disp_name, BAL_TRANS_LCO_SOURCE) ==0)
//	{
//		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_EVENTS, 0, 1, ebufp);
//		if(res_flistp)
//			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_EVENT_OBJ, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);
//		PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, trans_amt, ebufp);
//	}
//	else if(strcmp(disp_name, BAL_TRANS_LCO_DEST) ==0)
//	{
//		res_flistp = PIN_FLIST_ELEM_GET(s_oflistp, PIN_FLD_EVENTS, 1, 1, ebufp);
//		if(res_flistp)
//			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_EVENT_OBJ, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);
//
//		dec_amount = pbo_decimal_negate(trans_amt, ebufp);
//		PIN_FLIST_FLD_PUT(lc_temp_flistp, PIN_FLD_AMOUNT, dec_amount, ebufp);
//	}
//
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, disp_name, ebufp);
//	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, lc_temp_flistp, PIN_FLD_USERID, ebufp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Event Flist:", s_oflistp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_utils_gen_event Event Flist:", lc_iflistp);
//
//	fm_mso_utils_gen_event(ctxp, acct_pdp, NULL, "MSO_OP_PYMT_COLLECT",
//				disp_name, event, lc_iflistp, ebufp);
//
//	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
//
//	return;
//}


/**************************************************************************
* Function: 	fm_mso_utils_get_service_details()
* Owner:        Hrish Kulkarni
* Decription:   For getting service details of broadband / CATV accounts.
*
**************************************************************************/
void fm_mso_utils_get_all_service_details(
	pcm_context_t		*ctxp,
	poid_t			*acct_pdp,
	int32			serv_type,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*serv_pdp = NULL;
	int32			flags = 256;
	int32			srvc_status_active  = 10100;
 	int32			srvc_status_inactive = 10102;
	int64			db = 0;
	int64			id = -1;
	char			*s_template = " select X from /service where F1 = V1 and F2 = V2 order by created_t desc";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_utils_get_service_details", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_utils_get_service_details");
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "acct_pdp", acct_pdp );

	db = PIN_POID_GET_DB(acct_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	if (serv_type == 1)
	{
		serv_pdp = PIN_POID_CREATE(db, "/service/telco/broadband", id, ebufp);
	}
	if (serv_type == 0)
	{
		serv_pdp = PIN_POID_CREATE(db, "/service/catv", id, ebufp);
	}

	/*******************************************************
	PCM_OP_SEARCH Input:
	0 PIN_FLD_POID POID [0] 0.0.0.1 /search -1
	0 PIN_FLD_FLAGS INT [0] 256
	0 PIN_FLD_TEMPLATE STR [0] " select X from /service where F1 = V1 and F2 = V2 "
	0 PIN_FLD_ARGS ARRAY [1]
	1   PIN_FLD_ACCOUNT_OBJ POID [0] 0.0.0.1 /account 2321559
	0 PIN_FLD_ARGS ARRAY [2]
	1   PIN_FLD_POID POID [0] 0.0.0.1 /service/telco/broadband -1
	0 PIN_FLD_RESULTS ARRAY [0] NULL
	*******************************************************/
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, serv_pdp, ebufp);


	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Output flist:",*o_flistpp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}

void
fm_mso_retrieve_os_charges(
	pcm_context_t	*ctxp, 
	poid_t			*binfo_pdp, 
	pin_flist_t		**ret_flistpp, 
	pin_errbuf_t	*ebufp)
{
    pin_flist_t		*s_flistp = NULL;
    pin_flist_t		*s_res_flistp = NULL;
    pin_flist_t		*bal_flistp = NULL;
    pin_flist_t		*sub_bal_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
    pin_flist_t		*res_bal_flistp = NULL;
    pin_flist_t		*res_sbal_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int32			flags = 256;
	int64			db = 0;
	int64			id = -1;
	char			*s_template = " select X from /balance_group 1, /billinfo 2 where 2.F1 = V1 and 2.F2 = 1.F3";
	pin_decimal_t           *total_bal = NULL;
	pin_decimal_t		*curr_bal = NULL;
	int			elem_id = 0;
	pin_cookie_t		cookie = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error before calling fm_mso_retrieve_os_charges", ebufp);
		return;
	}
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_retrieve_os_charges");
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "billinfo pdp", binfo_pdp );

	db = PIN_POID_GET_DB(binfo_pdp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);
	
	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, binfo_pdp, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, NULL, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);


	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
    bal_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_BALANCES, CURRENCY_INR, ebufp);
    sub_bal_flistp = PIN_FLIST_ELEM_ADD(bal_flistp, PIN_FLD_SUB_BALANCES, PIN_ELEMID_ANY, ebufp);
    PIN_FLIST_FLD_SET(sub_bal_flistp, PIN_FLD_CURRENT_BAL, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Balance Group Input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &s_res_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Balance Group Output flist:",s_res_flistp);

    if (s_res_flistp)
    {
        res_flistp = PIN_FLIST_ELEM_GET(s_res_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);

        if (res_flistp)
        {
            res_bal_flistp=PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_BALANCES, CURRENCY_INR, 1, ebufp);
            if (res_bal_flistp)
            {
		while((res_sbal_flistp = PIN_FLIST_ELEM_GET_NEXT(res_bal_flistp, PIN_FLD_SUB_BALANCES,
                &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
        	{
                	//res_sbal_flistp=PIN_FLIST_ELEM_GET(res_bal_flistp, PIN_FLD_SUB_BALANCES, 0, 1, ebufp);
                	if (res_sbal_flistp)
                	{
		            if(pbo_decimal_is_null(total_bal, ebufp))
			    {
				total_bal = pbo_decimal_from_str("0.0",ebufp);	
			    }
			    curr_bal = PIN_FLIST_FLD_GET(res_sbal_flistp,PIN_FLD_CURRENT_BAL, 1, ebufp);
			    pbo_decimal_add_assign(total_bal,curr_bal,ebufp);	
			    //PIN_FLIST_FLD_COPY(res_sbal_flistp, PIN_FLD_CURRENT_BAL, *ret_flistpp, PIN_FLD_CURRENT_BAL, ebufp);
			}
		}
            }

        }
    }
    if(!pbo_decimal_is_null(total_bal, ebufp))
    {
	PIN_FLIST_FLD_PUT(*ret_flistpp, PIN_FLD_CURRENT_BAL, total_bal, ebufp);	
	//pbo_decimal_destroy(&total_bal);
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_retrieve_os_charges Output flist:",*ret_flistpp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
    PIN_FLIST_DESTROY_EX(&s_res_flistp, NULL);
	return;


}


int32
fm_mso_get_pp_type(
        pcm_context_t   *ctxp,
        poid_t          *act_pdp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t	*srch_i_flistp = NULL;
	pin_flist_t	*srch_o_flistp = NULL;
	pin_flist_t	*args_flistp   = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*profile_flistp = NULL;
	poid_t		*srch_pdp = NULL;
	int32		srch_flags = 256;
	int32		zero = 0;
	int32		*pp_type = NULL;
	char 		*template = "select X from /profile where F1 = V1 and profile_t.poid_type='/profile/customer_care' ";
	
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling profile_search", ebufp);
                return;
        }
	
	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(act_pdp), "/search", -1, ebufp);
	srch_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_i_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_i_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp,ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(srch_i_flistp, PIN_FLD_RESULTS, 0, ebufp);
	profile_flistp = PIN_FLIST_SUBSTR_ADD(results_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
	PIN_FLIST_FLD_SET(profile_flistp, MSO_FLD_PP_TYPE, &zero, ebufp);
	PIN_ERR_LOG_FLIST(3, "Profile search flist", srch_i_flistp);
	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_i_flistp, &srch_o_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_i_flistp, ebufp);
	 if (PIN_ERRBUF_IS_ERR(ebufp))
        {
		 PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error during  profile_search", ebufp);
                return;
        }
	PIN_ERR_LOG_FLIST(3, "Profile search out flist", srch_o_flistp);	
	results_flistp = PIN_FLIST_ELEM_GET(srch_o_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	profile_flistp = PIN_FLIST_SUBSTR_GET(results_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 0, ebufp);
	
	pp_type = PIN_FLIST_FLD_GET(profile_flistp, MSO_FLD_PP_TYPE, 0, ebufp);
	
	PIN_FLIST_DESTROY_EX(&srch_o_flistp, ebufp);
	return *pp_type;

}

void
fm_mso_split_netflix_pymt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_decimal_t	 	*npymt_amt,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	poid_t		*act_pdp = NULL;
	poid_t		*evt_pdp = NULL;
	poid_t		*binfo_pdp = NULL;
	poid_t		*balgrp_pdp = NULL;
	poid_t		*srvc_pdp = NULL;
	pin_decimal_t	 *zero_amt = pbo_decimal_from_str("0", ebufp);
	int		status;
	int32		*pay_type = NULL;
	char    	*log = "Billing Event Log";
	time_t		current_t = pin_virtual_time(NULL);
	int           netf_curr= NETFLIX_CURRENCY;
	pin_flist_t	*netf_flistp = NULL;
	pin_flist_t	*nf_flistp = NULL;
	pin_flist_t	*rf_flistp = NULL;
	pin_flist_t	*robj_iflistp = NULL;
	pin_flist_t	*robj_oflistp = NULL;
	pin_flist_t	*err_flistp = NULL;
	pin_flist_t	*bal_flistp = NULL;
	pin_flist_t	*tot_flistp = NULL;
	pin_flist_t	*wrt_oflistp = NULL;
	pin_flist_t	*seq_iflistp = NULL;
	pin_flist_t	*seq_oflistp = NULL;
	pin_flist_t	*recp_netf_flistp = NULL;
	pin_flist_t	*recp_netf_oflistp = NULL;
	char		*fname = NULL;
	char		*mname = NULL;
	char		*lname = NULL;
	char		*addr = NULL;
	char		*city = NULL;
	char		*state = NULL;
	char		*country = NULL;
	char		*zip = NULL;
	char 		msg[500]="";
	
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_split_netfix_pymt", ebufp);
                return;
        }

	PIN_ERR_LOG_FLIST(3, "Netflix Payment Input Flist", in_flistp);
	act_pdp = PIN_FLIST_FLD_GET(in_flistp,PIN_FLD_POID,1,ebufp);

	/**************************************************************************
	CALLING DATA COLLECTION FUNCTION FOR SANITY CHECKS AND  FLIST PREPARATION
	**************************************************************************/
	fm_mso_retrieve_service_netflix(ctxp,act_pdp,&rf_flistp,ebufp);
	if(rf_flistp !=NULL && PIN_FLIST_ELEM_COUNT(rf_flistp, PIN_FLD_RESULTS, ebufp) > 0)
	{
		nf_flistp = PIN_FLIST_ELEM_GET(rf_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		balgrp_pdp = PIN_FLIST_FLD_GET(nf_flistp, PIN_FLD_BAL_GRP_OBJ,1,ebufp);
		srvc_pdp = PIN_FLIST_FLD_GET(nf_flistp, PIN_FLD_POID,1,ebufp);
		robj_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID,balgrp_pdp, ebufp);
		PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_BILLINFO_OBJ,NULL, ebufp);
		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflistp, &robj_oflistp, ebufp);	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ OBJ BEFORE DATA PREPARARTION", robj_oflistp);
		binfo_pdp =PIN_FLIST_FLD_GET(robj_oflistp, PIN_FLD_BILLINFO_OBJ, 1, ebufp);
	}
	else
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Invalid Netflix Service/Balance_group details");
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID,act_pdp,ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"53992",ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Invalid Netflix Service/Balance_group details", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*ret_flistp = PIN_FLIST_COPY(err_flistp,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST AFTER ERROR", *ret_flistp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&rf_flistp, ebufp);
		return;
	}

	/**************************************************************************
	FLIST PREPARATION FOR PCM_OP_ACT_USAGE
	**************************************************************************/
	netf_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,netf_flistp,PIN_FLD_POID,ebufp);
	nf_flistp = PIN_FLIST_SUBSTR_ADD(netf_flistp,PIN_FLD_EVENT,ebufp);
	if (pbo_decimal_compare(npymt_amt, zero_amt, ebufp) != -1)
	{
		PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_POID, PIN_POID_CREATE(PIN_POID_GET_DB(act_pdp), NETF_PAY, -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_PROGRAM_NAME,NF_PROG_NAME, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_POID, PIN_POID_CREATE(PIN_POID_GET_DB(act_pdp), NETF_REV, -1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_PROGRAM_NAME,NF_RPROG_NAME, ebufp);
	}
	PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_NAME,log, ebufp);
	PIN_FLIST_FLD_SET(nf_flistp,PIN_FLD_USERID, PIN_POID_CREATE(PIN_POID_GET_DB(act_pdp), "/service/pcm_client", 1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(nf_flistp,PIN_FLD_USERID, PIN_POID_CREATE(PIN_POID_GET_DB(act_pdp), NETF_SESS, -1, ebufp), ebufp);
	PIN_FLIST_FLD_COPY(in_flistp,PIN_FLD_POID,nf_flistp,PIN_FLD_ACCOUNT_OBJ,ebufp);
	PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_START_T,&current_t, ebufp);
	PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_END_T,&current_t, ebufp);
	PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_SERVICE_OBJ,srvc_pdp, ebufp);
	if (pbo_decimal_compare(npymt_amt, zero_amt, ebufp) != -1)
	{
		PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_DESCR,NETF_DESCR, ebufp);
		PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_SYS_DESCR,NETF_DESCR, ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_DESCR,NETF_RDESCR, ebufp);
		PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_SYS_DESCR,NETF_RDESCR, ebufp);
	}
	PIN_FLIST_FLD_SET(nf_flistp, PIN_FLD_BILLINFO_OBJ,binfo_pdp, ebufp);
	bal_flistp = PIN_FLIST_ELEM_ADD(nf_flistp,PIN_FLD_BAL_IMPACTS,0,ebufp);
	status = 2;
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_IMPACT_TYPE, &status, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_RESOURCE_ID,&netf_curr, ebufp);
	status = 0;
	log = "";
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_RESOURCE_ID_ORIG, &status, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_TAX_CODE, log, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_RATE_TAG, log, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_RATE_OBJ, PIN_POID_CREATE(PIN_POID_GET_DB(act_pdp), "0", 0, ebufp), ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_DISCOUNT, zero_amt, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_PERCENT, zero_amt, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_QUANTITY, zero_amt, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_AMOUNT_DEFERRED, zero_amt, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_AMOUNT,(pin_decimal_t *)pbo_decimal_negate(npymt_amt,ebufp), ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_AMOUNT_ORIG, (pin_decimal_t *)NULL, ebufp);
	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_BAL_GRP_OBJ, balgrp_pdp, ebufp);
	tot_flistp = PIN_FLIST_ELEM_ADD(nf_flistp,PIN_FLD_TOTAL,NETFLIX_CURRENCY,ebufp);
	PIN_FLIST_FLD_SET(tot_flistp, PIN_FLD_AMOUNT, (pin_decimal_t *)pbo_decimal_negate(npymt_amt,ebufp), ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix Payment Record using PCM_OP_ACT_USAGE input flist", netf_flistp);
	PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, netf_flistp, &robj_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Netflix Payment Record using PCM_OP_ACT_USAGE output flist",robj_oflistp);

	if(robj_oflistp !=NULL && PIN_FLIST_ELEM_COUNT(robj_oflistp, PIN_FLD_RESULTS, ebufp) > 0 && !PIN_ERRBUF_IS_ERR(ebufp))
	{
		nf_flistp = PIN_FLIST_ELEM_GET(robj_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		evt_pdp = PIN_FLIST_FLD_GET(nf_flistp,PIN_FLD_POID,1,ebufp);
		tot_flistp = PIN_FLIST_ELEM_GET(nf_flistp, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp);
		binfo_pdp = PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_ITEM_OBJ,1,ebufp);
		PIN_FLIST_DESTROY_EX(&robj_iflistp, ebufp);
		status = 4;
		robj_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID, binfo_pdp, ebufp);
		PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_ITEM_TOTAL, npymt_amt, ebufp);
		PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "WRITE FLDS FOR ITEM_TOTAL INPUT", robj_iflistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, robj_iflistp, &wrt_oflistp, ebufp);	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "WRITE FLDS FOR ITEM_TOTAL OUTPUT", wrt_oflistp);
		/**************************************************************************
		FLIST PREPARATION FOR RECEIPT GENERATION FOR NETFLIX
		**************************************************************************/
		if(!PIN_ERRBUF_IS_ERR(ebufp) && pbo_decimal_compare(npymt_amt, zero_amt, ebufp) != -1)
		{
		       recp_netf_flistp = PIN_FLIST_CREATE(ebufp);
		       PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_POID, PIN_POID_CREATE(PIN_POID_GET_DB(act_pdp), "/mso_receipt", -1, ebufp), ebufp);
		       PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);
			PIN_FLIST_DESTROY_EX(&robj_iflistp, ebufp);
		       robj_iflistp = PIN_FLIST_CREATE(ebufp);
		       PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID,act_pdp, ebufp);
			PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_ACCOUNT_NO,NULL, ebufp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflistp, &robj_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "READ OBJ FOR ACCOUNT NO", robj_oflistp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_ACCOUNT_NO, PIN_FLIST_FLD_GET(robj_oflistp,PIN_FLD_ACCOUNT_NO,1,ebufp), ebufp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_AMOUNT, npymt_amt, ebufp);	
			//Reading the Account attributes
			bal_flistp = PIN_FLIST_CREATE(ebufp);
			tot_flistp = PIN_FLIST_CREATE(ebufp);
			wrt_oflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_POID, (void *)act_pdp, ebufp);
			PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_ACCOUNT_NO, "", ebufp);
			tot_flistp = PIN_FLIST_ELEM_ADD(bal_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
			PIN_FLIST_FLD_SET(tot_flistp, PIN_FLD_FIRST_NAME, "", ebufp);
			PIN_FLIST_FLD_SET(tot_flistp, PIN_FLD_MIDDLE_NAME, "", ebufp);
			PIN_FLIST_FLD_SET(tot_flistp, PIN_FLD_LAST_NAME, "", ebufp);
			PIN_FLIST_FLD_SET(tot_flistp, PIN_FLD_ADDRESS, "", ebufp);
			PIN_FLIST_FLD_SET(tot_flistp, PIN_FLD_CITY, "", ebufp);
			PIN_FLIST_FLD_SET(tot_flistp, PIN_FLD_STATE, "", ebufp);
			PIN_FLIST_FLD_SET(tot_flistp, PIN_FLD_COUNTRY, "", ebufp);
			PIN_FLIST_FLD_SET(tot_flistp, PIN_FLD_ZIP, "", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account Attributes Fetch for Netflix Receipt", bal_flistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, bal_flistp, &wrt_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Account Attributes Fetch for Netflix Receipt OP Flist", wrt_oflistp);
	              PIN_FLIST_DESTROY_EX(&tot_flistp, ebufp);
			tot_flistp = (pin_flist_t *)NULL;
			tot_flistp = PIN_FLIST_ELEM_TAKE(wrt_oflistp, PIN_FLD_NAMEINFO, PIN_ELEMID_ANY, 0, ebufp);
			sprintf(msg, "%s %s %s", PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_FIRST_NAME, 0, ebufp), PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_MIDDLE_NAME, 0, ebufp), PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_LAST_NAME, 0, ebufp));
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_NAME, msg, ebufp);	
			sprintf(msg, "%s, %s, %s, %s - %s", PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_ADDRESS, 0, ebufp), PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_CITY, 0, ebufp), PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_STATE, 0, ebufp), PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_COUNTRY, 0, ebufp), PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_ZIP, 0, ebufp));
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_ADDRESS, msg, ebufp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_DESCR,NETF_DESCR, ebufp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_PROGRAM_NAME,NF_PROG_NAME, ebufp);
			//Reinitiating to Null
			bal_flistp = (pin_flist_t *)NULL;
			tot_flistp = (pin_flist_t *)NULL;
			wrt_oflistp = (pin_flist_t *)NULL;
			tot_flistp = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_CHARGES, PIN_ELEMID_ANY, 1, ebufp);
			bal_flistp = PIN_FLIST_SUBSTR_GET(tot_flistp, PIN_FLD_PAYMENT, 1, ebufp);
			wrt_oflistp = PIN_FLIST_SUBSTR_GET(bal_flistp, PIN_FLD_INHERITED_INFO, 1, ebufp);
			pay_type = PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
			if (!bal_flistp || bal_flistp ==(pin_flist_t*)NULL || !wrt_oflistp || wrt_oflistp ==(pin_flist_t*)NULL || !tot_flistp || tot_flistp ==(pin_flist_t*)NULL)
			{
				status = 1;
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Could not post Netflix payment due to Missing INFO Array hence Aborting.");
				err_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID,act_pdp,ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"53994",ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Could not post Netflix payment due to Missing INFO Array hence Aborting.", ebufp);
				PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
				*ret_flistp = PIN_FLIST_COPY(err_flistp,ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST AFTER NETFLIX PAYMENT POSTING", *ret_flistp);
				PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		       	PIN_FLIST_DESTROY_EX(&recp_netf_flistp, ebufp);
				goto NETF_CLEAN;
			}
			else
			{
				if( pay_type && ( *pay_type == PAY_TYPE_CASH || *pay_type == PAY_TYPE_TDS || *pay_type == PAY_TYPE_SP_AUTOPAY))
				{
					PIN_FLIST_DESTROY_EX(&robj_iflistp, ebufp);
					robj_iflistp = PIN_FLIST_CREATE(ebufp);
					robj_iflistp = PIN_FLIST_ELEM_GET(wrt_oflistp, PIN_FLD_CASH_INFO, PIN_ELEMID_ANY, 1, ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_CHECK_NO,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_CHECK_NO, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BANK_ACCOUNT_NO,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BANK_ACCOUNT_NO, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BANK_NAME,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BANK_NAME, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BANK_CODE,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BANK_CODE, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BRANCH_NO,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BRANCH_NO, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_REFERENCE_ID,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_RECEIPT_NO, 1, ebufp), ebufp);
				}
				else if (*pay_type && *pay_type == PAY_TYPE_CHECK)
				{
					PIN_FLIST_DESTROY_EX(&robj_iflistp, ebufp);
					robj_iflistp = PIN_FLIST_CREATE(ebufp);
					robj_iflistp = PIN_FLIST_ELEM_GET(wrt_oflistp, PIN_FLD_CHECK_INFO, PIN_ELEMID_ANY, 1, ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_CHECK_NO,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_CHECK_NO, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BANK_ACCOUNT_NO,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BANK_ACCOUNT_NO, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BANK_NAME,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BANK_NAME, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BANK_CODE,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BANK_CODE, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BRANCH_NO,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BRANCH_NO, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_REFERENCE_ID,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_RECEIPT_NO, 1, ebufp), ebufp);
				}
				else if (*pay_type && (*pay_type == PAY_TYPE_ONLINE || *pay_type == PAY_TYPE_ECS))
				{
					PIN_FLIST_DESTROY_EX(&robj_iflistp, ebufp);
					robj_iflistp = PIN_FLIST_CREATE(ebufp);
					robj_iflistp = PIN_FLIST_ELEM_GET(wrt_oflistp, PIN_FLD_WIRE_TRANSFER_INFO, PIN_ELEMID_ANY, 1, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Receipt LAst Flist Details", robj_iflistp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_CHECK_NO,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_CHECK_NO, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BANK_ACCOUNT_NO,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BANK_ACCOUNT_NO, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BANK_NAME,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BANK_NAME, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BANK_CODE,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BANK_CODE, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_BRANCH_NO,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_BRANCH_NO, 1, ebufp), ebufp);
					PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_REFERENCE_ID,PIN_FLIST_FLD_GET(robj_iflistp, PIN_FLD_WIRE_TRANSFER_ID, 1, ebufp), ebufp);
				}		
			}
			PIN_FLIST_FLD_SET(recp_netf_flistp, MSO_FLD_SERVICE_TYPE,PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_SERVICE_TYPE, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_PAY_TYPE,PIN_FLIST_FLD_GET(tot_flistp, PIN_FLD_PAY_TYPE, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_END_T,PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_END_T, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_START_T,PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_START_T, 1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, MSO_FLD_PYMT_CHANNEL,PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp), ebufp);
			
			if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 0)
			{
				sprintf(msg, "%s", "OAP Initiated Payment");
			}
			else if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 1)
			{
				sprintf(msg, "%s", "SSA Initiated Payment");
			}
			else if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 2)
			{
				sprintf(msg, "%s", "UPASS Initiated Payment");
			}
			else if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 3)
			{
				sprintf(msg, "%s", "LCO Bulk Payment");
			}
			else if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 4)
			{
				sprintf(msg, "%s", "E-Seva Payment");
			}
			else if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 5)
			{
				sprintf(msg, "%s", "BillDesk Payment");
			}
			else if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 6)
			{
				sprintf(msg, "%s", "IVR Payment");
			}
			else if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 7)
			{
				sprintf(msg, "%s", "BATCH CC Payment");
			}
			else if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 8)
			{
				sprintf(msg, "%s", "Batch DD or SI Payment");
			}
			else if(PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 9)
			{
			   sprintf(msg, "%s", "MOSAMBEE");
			}
			else if (PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 10)
			{
				sprintf(msg, "%s", "Mobile Application");
			}
			else if (PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 11)
			{
				sprintf(msg, "%s", "Mobikwik");
			}
			else if (PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 12)
			{
				sprintf(msg, "%s", "Paytm");
			}
			else if (PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 13)
			{
				sprintf(msg, "%s", "MPOS");
			}
			else if (PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 14)
			{
			   sprintf(msg, "%s", "NEFT");
			}
			else if (PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 15)
			{
			   sprintf(msg, "%s", "Amazon Pay");
			}
			else if (PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 16)
			{
			   sprintf(msg, "%s", "Netflix");
			}
			else if (PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) !=(pin_flist_t *)NULL  && *(int *)PIN_FLIST_FLD_GET(in_flistp, MSO_FLD_PYMT_CHANNEL, 1, ebufp) == 17)
			{
			   sprintf(msg, "%s", "Dealer Wallet");
			}
			else
			{
			   sprintf(msg, "%s", "");
			}

			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_CHANNEL,msg, ebufp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_ITEM_OBJ, binfo_pdp, ebufp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_EVENT_OBJ, evt_pdp, ebufp);
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_USERID, PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_USERID, 1, ebufp), ebufp);		
			//Generating the Sequence for Receipts
			seq_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(seq_iflistp, PIN_FLD_POID, act_pdp, ebufp);
			PIN_FLIST_FLD_SET(seq_iflistp, PIN_FLD_NAME, SEQ_TYPE_RECEIPT, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "IP FLIST FOR NETFLIX SEQUENCE", seq_iflistp);
			fm_mso_utils_sequence_no(ctxp, seq_iflistp, &seq_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OP FLIST FOR NETFLIX SEQUENCE", seq_oflistp);
			sprintf(msg, "%s%d", "N-",*(int32 *)PIN_FLIST_FLD_GET(seq_oflistp, PIN_FLD_HEADER_NUM, 1, ebufp));
			PIN_FLIST_FLD_SET(recp_netf_flistp, PIN_FLD_RECEIPT_NO, msg, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "IP FLIST FOR NETFLIX RECEIPT GENERATION", recp_netf_flistp);
			PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, recp_netf_flistp, &recp_netf_oflistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OP FLIST FOR NETFLIX RECEIPT GENERATION", recp_netf_oflistp);
			
			if (!PIN_ERRBUF_IS_ERR(ebufp))
			{
			    mso_op_netfp_gen_lc_event(ctxp,recp_netf_flistp,ebufp);
			}
		
		       PIN_FLIST_DESTROY_EX(&recp_netf_flistp, ebufp);
	              PIN_FLIST_DESTROY_EX(&recp_netf_oflistp, ebufp);
	              PIN_FLIST_DESTROY_EX(&seq_iflistp, ebufp);
	              PIN_FLIST_DESTROY_EX(&seq_oflistp, ebufp);
		}
	}

       if(PIN_ERRBUF_IS_ERR(ebufp) && ((ebufp)->pin_err == PIN_ERR_STORAGE))
	{
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,"Error generating receipts", ebufp);
		  PIN_ERRBUF_CLEAR(ebufp);
		  err_flistp = PIN_FLIST_CREATE(ebufp);
                status = 1;
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID, act_pdp, ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "53004", ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Error : Reference Id must be unique", ebufp);
                PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		  *ret_flistp = PIN_FLIST_COPY(err_flistp,ebufp);
                goto NETF_CLEAN;
	}
	else if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Could not post Netflix payment hence Aborting.");
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID,act_pdp,ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE,"53993",ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Could not post Netflix payment hence Aborting.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*ret_flistp = PIN_FLIST_COPY(err_flistp,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST AFTER NETFLIX PAYMENT POSTING", *ret_flistp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		goto NETF_CLEAN;
	}
	else if(pbo_decimal_compare(npymt_amt, zero_amt, ebufp) == -1)
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Reversal Successful.");
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID,act_pdp,ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_AMOUNT,npymt_amt,ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Netflix Reversal Posting Successful.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*ret_flistp = PIN_FLIST_COPY(err_flistp,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST AFTER NETFLIX REVERSAL POSTING", *ret_flistp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		goto NETF_CLEAN;
	}
	else 
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Netflix Payment Posting Successful moving with Normal Flow.");
		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_POID,act_pdp,ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_SERVICE_OBJ,srvc_pdp,ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_AMOUNT,npymt_amt,ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR,"Netflix Payment Posting Successful.", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
		*ret_flistp = PIN_FLIST_COPY(err_flistp,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "OUTPUT FLIST AFTER NETFLIX PAYMENT POSTING", *ret_flistp);
		PIN_FLIST_DESTROY_EX(&err_flistp, ebufp);
		goto NETF_CLEAN;
	}

NETF_CLEAN:	    
	    PIN_FLIST_DESTROY_EX(&robj_iflistp, ebufp);
	    PIN_FLIST_DESTROY_EX(&robj_oflistp, ebufp);
	    return;
}	

void fm_mso_retrieve_service_netflix(
	pcm_context_t		*ctxp,
	poid_t			*act_pdp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	u_int64		id = (u_int64)-1;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int32			s_flags = 512;
	int64			db = 0;
	poid_t			*pdp = NULL;
	int                status_closed= PIN_STATUS_CLOSED;

		db = PIN_POID_GET_DB(act_pdp);

		s_flistp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /service 1, /balance_group 2 where 1.F1 = 2.F2 and 1.F3 = V3 and 1.F4 != V4 and service_t.poid_type = '/service/netflix'", ebufp);

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BAL_GRP_OBJ, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ,act_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS,&status_closed,ebufp);

		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_BAL_GRP_OBJ, (void *)NULL, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_STATUS, (void *)NULL, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search input flist", s_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Service search output flist",*r_flistpp);
		
		PIN_FLIST_DESTROY_EX(&s_flistp, ebufp);
		return;
}
	

void mso_op_netfp_gen_lc_event(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp)
{
	poid_t			*acc_pdp = NULL;
	pin_flist_t		*lc_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	char			*event = NETF_PAY_EVENT;
	char			*action_name = NETF_ACTION;
	char			msg[255]="";
	char			*prog_name = NF_PROG_NAME;
	char			*name = "PAYMENT_COLLECT";
	char			*lc_template = NULL;
	int			string_id = ID_NETFLIX_PAYMENT;
	int			db = 1;


		lc_flistp = PIN_FLIST_CREATE(ebufp);
		acc_pdp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_ACCOUNT_OBJ,1,ebufp);
		flistp = PIN_FLIST_ELEM_ADD(lc_flistp, MSO_FLD_PAYMENT, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_NO, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_RECEIPT_NO, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_RECEIPT_NO, 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_AMOUNT, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp), ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, name, ebufp);
		get_evt_lifecycle_template(ctxp, db, string_id, VER_PAYMENT_COLLECT, &lc_template, ebufp);
		if (lc_template)
		{
			sprintf(msg,lc_template, pbo_decimal_to_str(PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp),ebufp), (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp));
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Lifecycle Creation input flist", lc_flistp);
		fm_mso_utils_gen_lifecycle_evt(ctxp, acc_pdp, NULL, prog_name,msg, action_name, event, lc_flistp, ebufp);
		
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
		 	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error lifecycle event creation", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}
		PIN_FLIST_DESTROY_EX(&lc_flistp, NULL);
		return;
}

static int32
fm_mso_split_hybrid_pymt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_decimal_t	 	*catv_amtp,
	char			*catv_acc_nop,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*pay_oflistp = NULL;
	pin_flist_t		*chg_flistp = NULL;
	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*i_flistp = NULL;
	pin_decimal_t		*src_amtp = NULL;
	poid_t			*acnt_pdp = NULL;
	int32			ret_status = -1;
	int32			*pay_statusp = NULL;
	int32			service_type = 0;

        if (PIN_ERRBUF_IS_ERR(ebufp))
                return ret_status;
	PIN_ERRBUF_CLEAR(ebufp);

	i_flistp = PIN_FLIST_COPY(in_flistp, ebufp);
	chg_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_CHARGES, PIN_ELEMID_ANY, 0, ebufp);
	src_amtp = (pin_decimal_t *)PIN_FLIST_FLD_GET(chg_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	if (pbo_decimal_compare(src_amtp, catv_amtp, ebufp) <= 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Input payment amount is less than CATV plan amount");
		ret_status = 0;
		goto CLEANUP;
	}

	fm_mso_get_acnt_from_acntno(ctxp, catv_acc_nop, &acnt_info, ebufp);
	if (acnt_info)
	{
		acnt_pdp = PIN_FLIST_FLD_GET(acnt_info, PIN_FLD_POID, 0, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(chg_flistp, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(chg_flistp, PIN_FLD_AMOUNT, catv_amtp, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_SERVICE_TYPE, &service_type, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACCOUNT_TAG, catv_acc_nop, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Tagging CATV account number not found");
		ret_status = 0;
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PYMT_COLLECT calling input flist", i_flistp);

	PCM_OP(ctxp, MSO_OP_PYMT_COLLECT, 0, i_flistp, &pay_oflistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_PYMT_COLLECT calling output flist", pay_oflistp);
	pay_statusp = (int32 *)PIN_FLIST_FLD_GET(pay_oflistp, PIN_FLD_STATUS, 0, ebufp);
	if (pay_statusp && *pay_statusp == 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CATV Payment Posted Successfully");
		ret_status = 1;
	}

CLEANUP:
	PIN_FLIST_DESTROY_EX(&pay_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&acnt_info, NULL);
	return ret_status;
}