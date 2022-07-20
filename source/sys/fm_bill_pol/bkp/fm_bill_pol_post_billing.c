/*
 *
* Copyright (c) 2002, 2016, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_bill_pol_post_billing.c:BillingVelocityInt:3:2006-Sep-05 21:55:03 %";
#endif

#include <stdio.h> 
#include <string.h> 
#include <time.h> 
#include "pcm.h"
#include "ops/bill.h"
#include "ops/cust.h"
#include "pin_bill.h"
#include "pin_pymt.h"
#include "pin_cust.h"
#include "fm_utils.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_bill_utils.h"
#include "psiu_business_params.h"
#include "fm_utils_bparams_cache.h"
#include "ops/collections.h"
#include "mso_lifecycle_id.h"

#define FILE_LOGNAME "fm_bill_pol_post_billing.c(3)"

/*******************************************************************
 * Contains the PCM_OP_BILL_POL_POST_BILLING operation. 
 *
 * This policy provides a hook at the end of the billing process for 
 * the single account. The default implementation contains the call
 * to the PCM_OP_BILL_SUSPEND_BILLING opcode in order to stop billing 
 * for the closed account.
 *******************************************************************/

EXPORT_OP void
op_bill_pol_post_billing(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_bill_pol_stop_billing();

static void
fm_mso_bill_pol_set_billtime_total(
        pcm_context_t   *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp);

static void
fm_mso_get_previous_bill(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp);

// MSO Customization Starts
static void fm_mso_billing_notification(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        poid_t                          *bill_pdp,
        pin_errbuf_t            *ebufp);

static void 
fm_mso_exit_collection_scenario(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);


#define NOTIFICATION 1

#define BMAIL 1

#include "mso_ops_flds.h"
// MSO Customization Ends

/*******************************************************************
 * Main routine for the PCM_OP_BILL_POL_POST_BILLING
 *******************************************************************/
void
op_bill_pol_post_billing(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
        pcm_context_t   *ctxp = connp->dm_ctx;
        void                    *vp = NULL;
        //MSO Customization Starts
        poid_t                  *bill_pdp = NULL;
        char                    *type = NULL;
        // MSO Customization Ends

	*r_flistpp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/*******************************************************************
	 * Insanity check.
	 *******************************************************************/
	if (opcode != PCM_OP_BILL_POL_POST_BILLING) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_post_billing bad opcode error", 
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: What we got.
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_bill_pol_post_billing input flist", i_flistp);


	/*******************************************************************
	 * Call the default implementation in order to stop billing of the 
	 * closed accounts
	 *******************************************************************/
	fm_bill_pol_stop_billing(ctxp, flags, i_flistp, r_flistpp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"op_bill_pol_post_billing error", ebufp);
	} else {

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_bill_pol_post_billing output flist", *r_flistpp);
                //MSO Customization Starts
                bill_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_LAST_BILL_OBJ, 1, ebufp);
                type = (char *)PIN_POID_GET_TYPE(bill_pdp);
                if ( type && (strcmp(type, "") != 0) )
                {
                        fm_mso_billing_notification(ctxp,i_flistp, bill_pdp, ebufp);
                }

                //MSO Customization Ends
        }
        return;
}

/*******************************************************************
 * fm_bill_pol_stop_billing():
 *
 * This checks the following conditions:
 * 1) The stop_bill_closed_accounts stanza from the pin.conf file (for CM)
 *    should not be zero
 * 2) Account has a closed status
 * 3) The billing is provided for the last (final) billing state
 * 4) Total due for open and pending items is zero
 * 5) This is a last accounting cycle in the multi-month billing cycle
 *
 * If all these conditions are satisfied then the function calls the
 * PCM_OP_BILL_SUSPEND_BILLING opcode to stop billing.
 *******************************************************************/
static void
fm_bill_pol_stop_billing(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*i_pol_flistp = NULL;
	pin_flist_t		*o_pol_flistp = NULL;
	void			*vp = NULL;
	pin_status_t		*statusp = NULL;
	int32			*cycles_leftp = NULL;
        int32			*bill_whenp = NULL;
        int32			*bill_typep = NULL;
        int32			f = PIN_BILL_FLAGS_DUE_TO_STOP_BILL_CLOSE_ACCT;
	int32		        config_stop_bill_closed_accounts = 0;
	pin_bill_billing_state_t *billing_statep = NULL;
	pin_decimal_t		*due = NULL;
	pin_decimal_t		*due_tot = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, vp, ebufp);

	/***********************************************************
	 * Check the conditions to stop billing
	************************************************************/
	config_stop_bill_closed_accounts = fm_utils_bparams_cache_get_int(ctxp,
		PSIU_BPARAMS_BILLING_PARAMS, PSIU_BPARAMS_STOP_BILL_CLOSED_ACCOUNTS,
			0, ebufp);

	if (!config_stop_bill_closed_accounts) {
		goto cleanup;
	}

	billing_statep = (pin_bill_billing_state_t *)PIN_FLIST_FLD_GET(i_flistp,
					PIN_FLD_BILLING_STATE, 0, ebufp);
	if (billing_statep && (*billing_statep != PIN_ACTG_CYCLE_OPEN)) {
		goto cleanup;
	}

	statusp = (pin_status_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATUS,
							0, ebufp);
	if (statusp && (*statusp != PIN_STATUS_CLOSED)) {
		goto cleanup;
	}

	due_tot = pin_decimal ("0.0", ebufp);
	due = (pin_decimal_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OPEN_BAL,
							1, ebufp);
	if (due) {
		pin_decimal_add_assign(due_tot, due, ebufp);
	}
	due = (pin_decimal_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PENDING_BAL,
							1, ebufp);
	if (due) {
		pin_decimal_add_assign(due_tot, due, ebufp);
	}
	if (!pbo_decimal_is_zero(due_tot, ebufp)) {
		goto cleanup;
	}

	bill_whenp = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_WHEN,
					0, ebufp);
	cycles_leftp = (int32 *)PIN_FLIST_FLD_GET(i_flistp, 
					PIN_FLD_BILL_ACTGCYCLES_LEFT, 0, ebufp);
	if (!bill_whenp || !cycles_leftp) {
			goto cleanup;
	}
	if (*bill_whenp != *cycles_leftp) {
			goto cleanup;
	}

	bill_typep = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PAY_TYPE,
		 1, ebufp);

	if (bill_typep && *bill_typep == PIN_PAY_TYPE_SUBORD) {
		f = PIN_BILL_FLAGS_DUE_TO_STOP_BILL_CLOSE_ACCT_SUBORD;
	} 	

	/***********************************************************
	 * Call the PCM_OP_BILL_SUSPEND_BILLING opcode to stop billing
	************************************************************/
	i_pol_flistp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(i_pol_flistp, PIN_FLD_POID, vp, ebufp);
	PIN_FLIST_FLD_SET(i_pol_flistp, PIN_FLD_BILLING_STATUS_FLAGS, 
				(void *)&f, ebufp);
	PCM_OP(ctxp, PCM_OP_BILL_SUSPEND_BILLING, flags, i_pol_flistp, 
				&o_pol_flistp, ebufp);

cleanup:

	PIN_FLIST_DESTROY_EX(&i_pol_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&o_pol_flistp, ebufp);
	PIN_DECIMAL_DESTROY_EX(&due_tot);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
		"fm_bill_pol_stop_billing", ebufp);
        }

	return;
}


//MSO Customization Starts
static void fm_mso_billing_notification(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        poid_t                          *bill_pdp,
        pin_errbuf_t            *ebufp)
        {
                pin_flist_t             *in_flistp = NULL;
                pin_flist_t             *r_flistp = NULL;
                pin_flist_t             *bin_flistp = NULL;
                pin_flist_t             *br_flistp = NULL;
                pin_flist_t             *notif_flistp = NULL;
                int32                   notif = NOTIFICATION;
                int32                   bmail = BMAIL;
		char			*billinfo_id = NULL;
		poid_t			*svc_pdp = NULL;
		void			*vp = NULL;
		time_t			time = 0;
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
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TOTAL_DUE, NULL, ebufp);
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_DUE_T, NULL, ebufp);
                PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_END_T, NULL, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill Object Read input flist", in_flistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, in_flistp, &r_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill Object Read output flist", r_flistp);
		//Calling Lifecycle_event
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_PROGRAM_NAME, "pin_bill_accts", ebufp);
		fm_mso_create_lifecycle_evt(ctxp, r_flistp, NULL, ID_BILLING, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}

		//Removal of collection scenario object
		fm_mso_exit_collection_scenario(ctxp, i_flistp, NULL, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_exit_collection_scenario error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
		}

		bin_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, bin_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(bin_flistp, PIN_FLD_BILLINFO_ID, NULL, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill Object Read input flist", bin_flistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, bin_flistp, &br_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill Object Read output flist", br_flistp);

		billinfo_id = PIN_FLIST_FLD_GET(br_flistp, PIN_FLD_BILLINFO_ID, 0, ebufp);
		if(strcmp(billinfo_id, "BB") == 0) {
		    svc_pdp = PIN_POID_CREATE(1, "/service/telco/broadband", -1, ebufp);
		} else {
		    svc_pdp = PIN_POID_CREATE(1, "/service/catv", -1, ebufp);
		}


                //Preparing input and Calling Notfication API
                notif_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_ACCOUNT_OBJ, notif_flistp, PIN_FLD_POID, ebufp);
                PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
                PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_FLAGS, &notif, ebufp);
                PIN_FLIST_FLD_SET(notif_flistp, PIN_FLD_SUBJECT, "Hathway - Bill Generated", ebufp);
                PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_BILL_NO, notif_flistp, PIN_FLD_BILL_NO, ebufp);
                PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_TOTAL_DUE, notif_flistp, PIN_FLD_AMOUNT, ebufp);
                //PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_DUE_T, notif_flistp, PIN_FLD_DUE_T, ebufp);
                //PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_END_T, notif_flistp, PIN_FLD_END_T, ebufp);
		if( strcmp(billinfo_id, "BB") == 0) {
			vp = PIN_FLIST_FLD_GET( r_flistp, PIN_FLD_END_T, 0, ebufp );
			if( vp ){
				time = *(time_t *)vp;
				time = time - 1;
			}
			PIN_FLIST_FLD_SET( notif_flistp, PIN_FLD_END_T, (void *)&time, ebufp );
			vp = PIN_FLIST_FLD_GET( r_flistp, PIN_FLD_DUE_T, 0, ebufp );
			if( vp ){
				time = *(time_t *)vp;
				time = time - 1;
			}
			PIN_FLIST_FLD_SET( notif_flistp, PIN_FLD_DUE_T, (void *)&time, ebufp );
		}
		else{
			PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_DUE_T, notif_flistp, PIN_FLD_DUE_T, ebufp);
			PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_END_T, notif_flistp, PIN_FLD_END_T, ebufp);
		}
			
                PIN_FLIST_FLD_SET(notif_flistp, MSO_FLD_SEND_BMAIL, &bmail, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notifiation input flist", notif_flistp);
                PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, notif_flistp, &r_flistp, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification output flist", r_flistp);

                PIN_FLIST_DESTROY_EX(&in_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&r_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&bin_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&br_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&notif_flistp, ebufp);
                if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Notification Error", ebufp);
                        PIN_ERRBUF_RESET(ebufp);
                }

                return;
        }



/*******************************************************************
 * fm_mso_bill_pol_set_billtime_total():
 *
 * updates billtime_total in the bill_t table
 *******************************************************************/
static void
fm_mso_bill_pol_set_billtime_total(
        pcm_context_t   *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp)
{

        poid_t                  *billinfo_obj = NULL;
        poid_t                  *s_pdp = NULL;
        pin_decimal_t           *old_billtime_total = NULL;
        pin_decimal_t           *current_total = NULL;
        pin_decimal_t           *amountp = pbo_decimal_from_str("0.0",ebufp);
        pin_decimal_t           *amt = NULL;
        pin_decimal_t           *total_due = NULL;
        pin_decimal_t           *disputed = NULL;
        pin_decimal_t           *write_off = NULL;
        pin_decimal_t           *current_billtime_amt = NULL;
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *flistp = NULL;
        pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *out_flistpp = NULL;
        pin_flist_t             *read_curr_bill_outflistp = NULL;
        pin_flist_t             *read_curr_bill_inflistp = NULL;
        pin_flist_t             *temp_flistp = NULL;
        pin_flist_t             *write_inflistp = NULL;
        pin_flist_t             *write_outflistp = NULL;
        pin_flist_t             *rd_flistp = NULL;
        pin_flist_t             *rd_oflistp = NULL; 
        int64                   database = 0;
        int32                   s_flags = 768;
        int32                   rec_id = 0;
        pin_cookie_t            cookie = NULL;
        time_t                  *last_bill_t = NULL;
        time_t                  no_bill_t = 0;
        time_t                  current_t = 0;
        void                    *vp = NULL;

        billinfo_obj = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        database = (int64) PIN_POID_GET_DB(billinfo_obj);

        fm_mso_get_previous_bill(ctxp, i_flistp, &out_flistpp, ebufp);

        if(out_flistpp)
        {
                vp = PIN_FLIST_FLD_GET(out_flistpp, PIN_FLD_END_T, 0, ebufp);
                if (vp)
                last_bill_t = ((time_t *)vp);
                
                if(*last_bill_t == no_bill_t)
                {
                	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "I am Inside");
                   rd_flistp = PIN_FLIST_CREATE(ebufp); 
                   PIN_FLIST_FLD_SET(rd_flistp, PIN_FLD_POID, billinfo_obj, ebufp);
                   PIN_FLIST_FLD_SET(rd_flistp, PIN_FLD_EFFECTIVE_T, NULL, ebufp);
                   
                   PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rd_flistp, &rd_oflistp, ebufp);
                   PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read the billinfo input flist", rd_flistp);
                   PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read the billinfo output list", rd_oflistp); 
                   if (PIN_ERRBUF_IS_ERR(ebufp))
                   {
                       PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in accessing billinfo or the billinfo does not exist", ebufp);
                       PIN_ERRBUF_RESET(ebufp);
                       goto CleanUp;
                   }

                   vp = PIN_FLIST_FLD_GET(rd_oflistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);
                   last_bill_t = ((time_t *)vp);
                }
                   
                
                
                old_billtime_total = (pin_decimal_t *)PIN_FLIST_FLD_GET(out_flistpp, MSO_FLD_BILLTIME_TOTAL, 0, ebufp);
                if(pbo_decimal_is_null(old_billtime_total, ebufp)==1)
                {
                        old_billtime_total = pbo_decimal_from_str("0.0",ebufp);
                }
        }
        else
        {
                old_billtime_total = pbo_decimal_from_str("0.0",ebufp);
        }

        /*****************************************************************
        * searching payment and adjustment
        *******************************************************************/

        s_flistp = PIN_FLIST_CREATE(ebufp);

        s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select x from /item where F1 > V1 and F2 <= V2 and F3 = V3 and ( item_t.poid_type = '/item/payment' or item_t.poid_type = '/item/adjustment' or item_t.poid_type = '/item/payment/reversal') ", ebufp);
        
        temp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_EFFECTIVE_T, last_bill_t, ebufp);

        temp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACTG_LAST_T, temp_flistp, PIN_FLD_EFFECTIVE_T, ebufp);

	temp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_obj, ebufp);

        temp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS,PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp,  PIN_FLD_ITEM_TOTAL, NULL, ebufp );
        PIN_FLIST_FLD_SET(temp_flistp,  PIN_FLD_EFFECTIVE_T, NULL, ebufp );
	PIN_FLIST_FLD_SET(temp_flistp,  PIN_FLD_BILLINFO_OBJ, NULL, ebufp );

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "payment/Adjustment search input list", s_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "payment/Adjustment search input list", r_flistp);

        while((flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS, &rec_id, 1, &cookie, ebufp))!=(pin_flist_t*)NULL)
        {
                amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_ITEM_TOTAL, 0, ebufp);
                amountp = pbo_decimal_add(amountp,amt,ebufp);
        }


        /*****************************************************************
        * reading the current bill Object
        *******************************************************************/

        read_curr_bill_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_LAST_BILL_OBJ, read_curr_bill_inflistp, PIN_FLD_POID, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bill_pol_set_billtime_total read current_bill input list",
                read_curr_bill_inflistp);
        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_curr_bill_inflistp, &read_curr_bill_outflistp, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bill_pol_set_billtime_total read current_bill out flist",
                read_curr_bill_outflistp);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in accessing bill or the bill does not exist", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                goto CleanUp;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bill_pol_set_billtime_total read account output flist",
                read_curr_bill_outflistp);
        current_total = (pin_decimal_t *)PIN_FLIST_FLD_GET(read_curr_bill_outflistp, PIN_FLD_CURRENT_TOTAL, 1, ebufp);
        disputed = (pin_decimal_t *)PIN_FLIST_FLD_GET(read_curr_bill_outflistp, PIN_FLD_DISPUTED, 1, ebufp);
        write_off = (pin_decimal_t *)PIN_FLIST_FLD_GET(read_curr_bill_outflistp, PIN_FLD_WRITEOFF, 1, ebufp);

        total_due = pbo_decimal_add(current_total,disputed,ebufp);
        total_due = pbo_decimal_add(total_due,write_off,ebufp);
        total_due = pbo_decimal_add(total_due,old_billtime_total,ebufp);  
        current_billtime_amt = pbo_decimal_add(amountp,total_due,ebufp);

        /*****************************************************************
        * Writting the bill Object
        *******************************************************************/

        write_inflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_LAST_BILL_OBJ, write_inflistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(write_inflistp, MSO_FLD_BILLTIME_TOTAL, current_billtime_amt, ebufp);
        current_t = pin_virtual_time(NULL);
        PIN_FLIST_FLD_SET(write_inflistp, PIN_FLD_BILL_DATE_T, &current_t, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_bill_pol_set_billtime_total read account input list",
                write_inflistp);
        PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_inflistp, &write_outflistp, ebufp);
		
		 if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in writing bill or the bill does not exist", ebufp);
                PIN_ERRBUF_RESET(ebufp);
                goto CleanUp;
        }

CleanUp:
        PIN_FLIST_DESTROY_EX(&write_inflistp, NULL);
        PIN_FLIST_DESTROY_EX(&write_outflistp, NULL);
        PIN_FLIST_DESTROY_EX(&out_flistpp, NULL);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&read_curr_bill_inflistp, NULL);
        PIN_FLIST_DESTROY_EX(&read_curr_bill_outflistp, NULL);
        PIN_FLIST_DESTROY_EX(&rd_oflistp, NULL);
        PIN_FLIST_DESTROY_EX(&rd_flistp, NULL);  
		
		
}

/*******************************************************************
 * fm_mso_get_previous_bill():
 *
 * Returns last months bill details
 *******************************************************************/
static void
fm_mso_get_previous_bill(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **out_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *bi_flistp = NULL;
        pin_flist_t             *last_bill_inflistp = NULL;
        pin_flist_t             *last_bill_outflistp = NULL;
        pin_flist_t             *last_bill_obj_flistp = NULL;
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        poid_t                  *last_bill_obj = NULL;
        poid_t                  *billinfo_obj = NULL;
        char                    *search_template = "select X from /bill where F1 = V1 and F2 <= V2 order by due_t desc ";
        int64                   db = -1;
        poid_t                  *s_pdp = NULL;
        int32                   srch_flag = 768;
        time_t                  bill_end_t = 0;
        void                    *vp = NULL;



        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;


        *out_flistpp = PIN_FLIST_CREATE(ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_previous_bill input flist", in_flistp);



        vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_END_T, 0, ebufp);
        bill_end_t = *((time_t *)vp);

        billinfo_obj = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);

        db = PIN_POID_GET_DB(billinfo_obj);
        s_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, search_template , ebufp);
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_OBJ, billinfo_obj , ebufp);
        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
        PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_END_T, &bill_end_t , ebufp);
        result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
        //PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search input flist for bill_t", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                goto cleanup;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist for bill_t", srch_out_flistp);
        if (srch_out_flistp)
        {
                if (last_bill_obj_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 1, 1, ebufp))
                {
                        *out_flistpp = PIN_FLIST_COPY(last_bill_obj_flistp, ebufp);
                }
                else
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "2nd last bill not found, hence not changing PIN_FLD_PREVIOUS_TOTAL");
                }
        }


        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_previous_bill output flist", *out_flistpp);
cleanup:
        PIN_FLIST_DESTROY_EX(&last_bill_inflistp, NULL);
        PIN_FLIST_DESTROY_EX(&last_bill_outflistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
}

//MSO Customization Ends

static void
fm_mso_exit_collection_scenario(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*bal_grp_out_flist = NULL;
	pin_flist_t		*coll_process_ifilst = NULL;
	pin_flist_t		*coll_process_ofilst = NULL;
	pin_flist_t		*srch_iflist = NULL;
	pin_flist_t		*srch_oflist = NULL;
	pin_flist_t		*wflds_oflist = NULL;
	pin_flist_t		*arg_flist = NULL;


	poid_t			*scenario_obj = NULL;

	int32			processing_time = 1;
	int32			srch_flags = 256;

	int64			db =1;

	time_t			time_now = pin_virtual_time(NULL);

	pin_decimal_t		*exit_amount_new    = pbo_decimal_from_str("9999999999.0",ebufp);
	pin_decimal_t		*exit_amount_actual = NULL;

	void			*vp = NULL;
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;



	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_exit_collection_scenario input flist", in_flistp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, in_flistp, &bal_grp_out_flist, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in reading balance group ", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "bal_grp_out_flist", bal_grp_out_flist);

	if (bal_grp_out_flist)
	{
		scenario_obj = PIN_FLIST_FLD_GET(bal_grp_out_flist, PIN_FLD_SCENARIO_OBJ, 1, ebufp);
		if (scenario_obj && (PIN_POID_GET_ID(scenario_obj) > 0))
		{
			//Search /config/collections/scenario object to have a highPIN_FLD_EXIT_AMOUNT
			srch_iflist = PIN_FLIST_CREATE(ebufp); 
			PIN_FLIST_FLD_PUT(srch_iflist, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
			PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_FLAGS, &srch_flags, ebufp);
			PIN_FLIST_FLD_SET(srch_iflist, PIN_FLD_TEMPLATE, " select X from /config/collections/scenario 1, /collections_scenario 2 where 1.F1 = 2.F2 and 2.F3.id = V3 ", ebufp );

			arg_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 2, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_CONFIG_SCENARIO_OBJ, NULL, ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_ARGS, 3, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, scenario_obj, ebufp);

			arg_flist = PIN_FLIST_ELEM_ADD(srch_iflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
			PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);
			
			vp = (pin_flist_t*)PIN_FLIST_SUBSTR_ADD(arg_flist, PIN_FLD_SCENARIO_INFO, ebufp);
			PIN_FLIST_FLD_SET((pin_flist_t*)vp, PIN_FLD_EXIT_AMOUNT, NULL, ebufp);


			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "srch_iflist", srch_iflist);
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflist, &srch_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&srch_iflist, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in searching config/collections/scenario", ebufp);
				goto cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "coll_process_ofilst", srch_oflist);

			//Update /config/collections/scenario object to have a highPIN_FLD_EXIT_AMOUNT
			if (srch_oflist)
			{
				arg_flist = PIN_FLIST_ELEM_GET(srch_oflist, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp );
			}

			if (arg_flist)
			{
				vp =  (pin_flist_t*)PIN_FLIST_SUBSTR_GET(arg_flist, PIN_FLD_SCENARIO_INFO, 1, ebufp);
				if (vp)
				{
					exit_amount_actual = PIN_FLIST_FLD_TAKE((pin_flist_t*)vp, PIN_FLD_EXIT_AMOUNT, 1, ebufp);
					PIN_FLIST_FLD_SET((pin_flist_t*)vp, PIN_FLD_EXIT_AMOUNT, exit_amount_new, ebufp);
				}
				//Call write_flds
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update config inputflist", arg_flist);
				PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, arg_flist, &wflds_oflist, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating config/collections/scenario", ebufp);
					goto cleanup;
				}
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "update config outputflist", wflds_oflist);
			}


			coll_process_ifilst = PIN_FLIST_CREATE(ebufp); 
			PIN_FLIST_FLD_COPY(bal_grp_out_flist, PIN_FLD_ACCOUNT_OBJ, coll_process_ifilst, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(coll_process_ifilst, PIN_FLD_PROGRAM_NAME, "pin_collections_process", ebufp);
			PIN_FLIST_FLD_SET(coll_process_ifilst, PIN_FLD_PROCESSING_TIME, &processing_time, ebufp);
			PIN_FLIST_FLD_COPY(bal_grp_out_flist, PIN_FLD_POID, coll_process_ifilst, PIN_FLD_BILLINFO_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(bal_grp_out_flist, PIN_FLD_SCENARIO_OBJ, coll_process_ifilst, PIN_FLD_SCENARIO_OBJ, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "coll_process_ifilst", coll_process_ifilst);
			PCM_OP(ctxp, PCM_OP_COLLECTIONS_PROCESS_ACCOUNT, 0, coll_process_ifilst, &coll_process_ofilst, ebufp);
			PIN_FLIST_DESTROY_EX(&coll_process_ifilst, NULL);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in executing PCM_OP_COLLECTIONS_PROCESS_ACCOUNT", ebufp);
				goto cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "coll_process_ofilst", coll_process_ofilst);

			//Revert /config/collections/scenario object to have a the old value
			vp =  (pin_flist_t*)PIN_FLIST_SUBSTR_GET(arg_flist, PIN_FLD_SCENARIO_INFO, 1, ebufp);
			if (vp)
			{
				PIN_FLIST_FLD_SET((pin_flist_t*)vp, PIN_FLD_EXIT_AMOUNT, exit_amount_actual, ebufp);
			}
			//Call write_flds
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "revert config inputflist", arg_flist);
			PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, arg_flist, &wflds_oflist, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in updating config/collections/scenario", ebufp);
				goto cleanup;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "revert config outputflist", wflds_oflist);
		}
	}

cleanup:
	PIN_FLIST_DESTROY_EX(&bal_grp_out_flist, NULL);
	PIN_FLIST_DESTROY_EX(&coll_process_ofilst, NULL); 
	PIN_FLIST_DESTROY_EX(&srch_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&wflds_oflist, NULL);

	if (!pbo_decimal_is_null(exit_amount_actual, ebufp))
	{
		pbo_decimal_destroy(&exit_amount_actual);
	}

	if (!pbo_decimal_is_null(exit_amount_new, ebufp))
	{
		pbo_decimal_destroy(&exit_amount_new);
	}
}

