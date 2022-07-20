/*******************************************************************
 *
* Copyright (c) 2004, 2013, Oracle and/or its affiliates. All rights reserved.
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_subscription_pol_prep_fold.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 06:17:24 nishahan Exp $";
#endif

#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "ops/bill.h"
#include "ops/subscription.h"
#include "cm_fm.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_rate.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"

#define FILE_SOURCE_ID  "fm_subscription_pol_prep_fold.c(7)"

EXPORT_OP void
op_subscription_pol_prep_fold(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_SUBSCRIPTION_POL_PREP_FOLD  command
 * This is called by PCM_OP_SUBSCRIPTION_CANCEL_PRODUCT to prepare 
 * the input flist and call the PCM_OP_SUBSCRIPTION_CYCLE_FOLD opcode
 * to fold resources contributed to by just the cancelled product 
 *******************************************************************/
void
op_subscription_pol_prep_fold(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)

{
	pcm_context_t	*ctxp = connp->dm_ctx;

  	pin_flist_t	*rs_flistp	 = NULL;
	pin_flist_t	*cf_flistp	 = NULL;
	pin_flist_t	*prod_flist	 = NULL;
	pin_flist_t	*bi_flistp	 = NULL;
	pin_flist_t	*results_flistp	 = NULL;
	pin_flist_t	*s_flistp	 = NULL;
	pin_flist_t	*args_flistp	 = NULL;
	pin_flist_t	*prod_flistp	 = NULL;
	pin_flist_t	*cycle_flistp	 = NULL;
	pin_flist_t	*r_flistp	 = NULL;
	pin_flist_t	*res_flistp	 = NULL;
	pin_flist_t	*bg_flistp	 = NULL;
	pin_flist_t	*bal_flistp	 = NULL;
	pin_flist_t 	*ret_bg_flistp	 = NULL;
	pin_flist_t	*ret_flistp	 = NULL;
	pin_flist_t   	*tmp_flistp	 = NULL;
	pin_flist_t	*bal_out_flistp	 = NULL;

	time_t		*end_t		 = NULL;
	time_t          bal_end_t        = 0;
        time_t          temp_t           = 0; 
        void	        *vp              = NULL;

        poid_t	        *a_pdp		 = NULL;
	poid_t	        *s_pdp		 = NULL;
	poid_t	        *p_pdp		 = NULL;
	poid_t	        *e_pdp		 = NULL;
	poid_t	        *bg_pdp	         = NULL;
	poid_t	        *bill_pdp	 = NULL;
	poid_t	        *service_pdp	 = NULL;
	
	int32	        s_flags	         = SRCH_EXACT;

	int32	        bi_id            = 0;
	int32	        evt_id           = 0;
	int32	        resource         = 0;

	int32	        rerate_flag      = 0;
	int32	        refold_flag      = 0;

	/*  Has balance group poid been retrieved ??? */
	int32	        bg_got		 = PIN_BOOLEAN_FALSE;

	pin_cookie_t	sbi_cookie       = NULL;
	pin_cookie_t	evt_cookie       = NULL;


        PIN_ERR_CLEAR_ERR(ebufp);
	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_SUBSCRIPTION_POL_PREP_FOLD) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_subscription_pol_prep_fold", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_prep_fold input flist", in_flistp);

	a_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp); 


	/**********************************************************************
	 * Retrieve fields from the input flist and prepare the input 
	 * to cycle_fold
	 *********************************************************************/	
        cf_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(cf_flistp, PIN_FLD_POID, a_pdp, ebufp);

	service_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if (service_pdp) {
		PIN_FLIST_FLD_SET(cf_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
	}

        vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PROGRAM_NAME,0,ebufp);
	PIN_FLIST_FLD_SET(cf_flistp, PIN_FLD_PROGRAM_NAME, vp, ebufp);

	end_t = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_END_T, 1, ebufp);
	if (end_t == NULL){
		temp_t = pin_virtual_time(NULL);
                end_t  = &temp_t;
        }
	
	/***********************************************************
         * Set the end_t as end_t - 1 after midnight rounding, 
	 * this is required for FOLD to pick up the buckets to fold 
	 * Also to be in consistence with regular bill end_t 
	 * and bill_now end_t that is being modified in  
	 * fm_bill_mb_apply_cycle() of fm_bill_utils_bill.c 
         * In case of product cancellation, we should not
         * do midnight rounding since the bucket truncation is not
         * midnight rounded, but we still need to do end_t - 1 so
         * that get_balances does not throw out the bucket as expired.
         **********************************************************/
	if(!(fm_utils_op_is_ancestor(connp->coip,
                        PCM_OP_SUBSCRIPTION_CANCEL_PRODUCT)) &&
	   !(fm_utils_op_is_ancestor(connp->coip,
			PCM_OP_CUST_SET_STATUS)) ) {
                bal_end_t = fm_utils_time_round_to_midnight(*end_t);
                bal_end_t = bal_end_t - 1;
        } else {
                bal_end_t = *end_t - 1;
	}
        PIN_FLIST_FLD_SET(cf_flistp, PIN_FLD_END_T, &bal_end_t, ebufp);

	prod_flist = PIN_FLIST_ELEM_GET(in_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY,
								 0, ebufp);
	p_pdp = PIN_FLIST_FLD_GET(prod_flist, PIN_FLD_OFFERING_OBJ, 0, ebufp);

	bi_flistp = PIN_FLIST_ELEM_ADD(cf_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(bi_flistp, PIN_FLD_RESULTS, 0, ebufp);
	
	/***********************************************************
	 * Get the resources to fold - search for cycle_forward events
         * that got triggered by the product cancellation 
	 **********************************************************/
        s_flistp = PIN_FLIST_CREATE(ebufp);

        s_pdp = PIN_POID_CREATE(pin_poid_get_db(a_pdp), "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);


        /***************************************************************
	 * 20 - PRO_LAST + INVERTED
	 * 28 - PRO_FIRST +PRO_LAST +INVERTED
	 * 524548 - PRO_FORCED + PRO_NORMAL + INVERTED
	 * 524556 - PRO_FORCED + PRO_NORMAL + PRO_FIRST + INVERTED
	 * 134217748 - ALIGN + PRO_LAST + INVERTED 
	 * 134217756 - ALIGN + PRO_FIRST + PRO_LAST + INVERTED
         * 134742276 - ALIGN + PRO_FORCED + PRO_NORMAL + INVERTED
         * 134742284 - ALIGN + PRO_FORCED + PRO_NORMAL + PRO_FIRST + INVERTED
         * 33554452 - CUT + PRO_LAST + INVERTED
	 * 33554460 - CUT + PRO_LAST + PRO_FIRST + INVERTED
	 * 34078980 - CUT+ PRO_FORCED + PRO_NORMAL + INVERTED
         * 34078988 - CUT+ PRO_FORCED + PRO_NORMAL + PRO_FIRST + INVERTED
         * 570425364 - CUT + PRO_LAST + INVERTED + STOP_ROLLOVER
	 * 570425372 - CUT + PRO_LAST + PRO_FIRST + INVERTED + STOP_ROLLOVER
	 * 570949892 - CUT+ PRO_FORCED + PRO_NORMAL + INVERTED + STOP_ROLLOVER
         * 570949900 - CUT+ PRO_FORCED + PRO_NORMAL + PRO_FIRST + INVERTED
         *                                                      + STOP_ROLLOVER
	 * 67108884 - MAINTAIN + PRO_LAST + INVERTED
	 * 67108892 - MAINTAIN + PRO_LAST + PRO_FIRST + INVERTED
         * 67633412 - MAINTAIN + PRO_FORCED + PRO_NORMAL + INVERTED
         * 67633420 - MAINTAIN + PRO_FORCED + PRO_NORMAL + PRO_FIRST + INVERTED
         * 67637519 - MAINTAIN + PRO_FORCED + CYCLE_CHARGED + PRO_NORMAL + PRO_FIRST + INVERTED
         * 67637511 - MAINTAIN + PRO_FORCED + CYCLE_CHARGED + PRO_NORMAL + INVERTED
	 **************************************************************/

	vp  = (void *)"select X from "
                "/event/billing/product/fee/cycle "
                " where  F1 = V1  and F2 = V2 and F3 like V3 and F4 = V4 and "
                " F5 in ( 20, 28, 524556, 524548, 134217748, 134217756, "
                " 134742276, 13474484, 34078980, 34078988, 33554452, 33554460, "
		" 570425364, 570425372, 570949892, 570949900, "
		" 67108884, 67108892, 67633412, 67633420, 67637519, 67637511 ) ";

        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,vp,ebufp); 

        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	prod_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, PIN_FLD_PRODUCT, ebufp);
        PIN_FLIST_FLD_SET(prod_flistp, PIN_FLD_OFFERING_OBJ, p_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        e_pdp = PIN_POID_CREATE(pin_poid_get_db(a_pdp),
                                "/event/billing/product/fee/cycle/%", -1, ebufp);
        PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, e_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_END_T, end_t, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
        cycle_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, PIN_FLD_CYCLE_INFO,
                                                         ebufp);
	vp = (int32 *)0;
        PIN_FLIST_FLD_SET(cycle_flistp, PIN_FLD_FLAGS, vp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_ELEM_SET(args_flistp, NULL, PIN_FLD_BAL_IMPACTS, PCM_RECID_ALL , ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_subs_pol_prep_fold search for events input" , s_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);


        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_subs_pol_prep_fold search for events output" , r_flistp);


        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
					"fm_subs_pol_prep_fold error", ebufp);
                goto Cleanup;
        }

	 /********************************************************************
	  * For each of the events of type "/event/billing/product/fee/cycle/"
	  * retrieved  .
	  *******************************************************************/
        while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp,
                  PIN_FLD_RESULTS, &evt_id, 1, &evt_cookie, ebufp)) != NULL) {
		sbi_cookie = NULL;
                /**************************************************************
                 * For each bal_impact
                 *************************************************************/
                while ((bi_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp,
                        PIN_FLD_BAL_IMPACTS, &bi_id, 1, &sbi_cookie, ebufp))
                                                                != NULL) {
                        /****************************************************
			 * Get the value of PIN_FLD_BAL_GRP_OBJ field 
			 * which is later on used in the input flist of 
			 * PCM_OP_SUBSCRIPTION_CYCLE_FOLD . After getting 
			 * the value of PIN_FLD_BAL_GRP_OBJ set the bg_got
			 * to PIN_BOOLEAN_TRUE which signifies  that the 
			 * balance_group poid has been got .
			 ****************************************************/
			if (bg_got == PIN_BOOLEAN_FALSE){
                        	bg_pdp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_BAL_GRP_OBJ,
                                                                 0, ebufp);

	                	bg_flistp = PIN_FLIST_CREATE(ebufp);
                		PIN_FLIST_FLD_SET(bg_flistp, PIN_FLD_POID, 
								bg_pdp, ebufp);
                		PIN_FLIST_FLD_SET(bg_flistp, PIN_FLD_ACCOUNT_OBJ, 
								a_pdp, ebufp);
                		PIN_FLIST_FLD_SET(bg_flistp, PIN_FLD_BILLINFO_OBJ,
								 NULL, ebufp);
                		PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE,
						 bg_flistp, &ret_bg_flistp, ebufp);

				bill_pdp = PIN_FLIST_FLD_GET(ret_bg_flistp, 
						PIN_FLD_BILLINFO_OBJ,0,ebufp);
				bg_got = PIN_BOOLEAN_TRUE;
	
			}
                        vp = PIN_FLIST_FLD_GET(bi_flistp, PIN_FLD_RESOURCE_ID,
                                                                0, ebufp);
			if (vp) {
				resource = * ((int32* )vp);
			}
			/******************************************************
			 * read the balances for this balance group and resource
			 * add the BALANCES to the input flist of CYCLE_FOLD
			 ******************************************************/
                	bal_flistp = PIN_FLIST_CREATE(ebufp);
                	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_POID, bg_pdp, 
									ebufp);
                	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_ACCOUNT_OBJ, 
								a_pdp, ebufp);
                	PIN_FLIST_FLD_SET(bal_flistp, PIN_FLD_BILLINFO_OBJ,
								 bill_pdp, ebufp);
                	tmp_flistp = PIN_FLIST_ELEM_ADD(bal_flistp, PIN_FLD_BALANCES,
                                				resource, ebufp);
                	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CREDIT_PROFILE,
                                				NULL, ebufp);
                	PCM_OP(ctxp, PCM_OP_READ_FLDS, PCM_OPFLG_CACHEABLE, bal_flistp,
                                			&bal_out_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&bal_flistp,NULL);	

			bal_flistp = PIN_FLIST_ELEM_GET(bal_out_flistp, 
						PIN_FLD_BALANCES, resource, 1, ebufp);
			if (bal_flistp){
				PIN_FLIST_ELEM_SET(results_flistp, bal_flistp, 
						PIN_FLD_BALANCES, resource, ebufp);
			}
			PIN_FLIST_DESTROY_EX(&bal_out_flistp,NULL);	
		}
	}
	if (bg_got == PIN_BOOLEAN_TRUE) {
        	vp = PIN_FLIST_FLD_GET(in_flistp,
			PIN_FLD_RERATE_FLAGS, 1, ebufp);
		if (vp) {
			rerate_flag = *(int32 *)vp;

			if ((rerate_flag & PIN_RATE_FLG_RERATE) &&
	    	    	    (rerate_flag & PIN_RATE_FLG_ITEM_BILLED)) {

			    refold_flag |= PIN_RATE_FLG_RERATE;
			    refold_flag |= PIN_RATE_FLG_ITEM_BILLED;
			    PIN_FLIST_FLD_SET(cf_flistp,
				PIN_FLD_RERATE_FLAGS, &refold_flag, ebufp);
			}
		}
		PIN_FLIST_FLD_SET(cf_flistp, PIN_FLD_BILLINFO_OBJ, bill_pdp, ebufp); 
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_BAL_GRP_OBJ, bg_pdp, ebufp);
		if (service_pdp) {
			PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
		}

		PIN_FLIST_FLD_SET(cf_flistp, PIN_FLD_OFFERING_OBJ, p_pdp, ebufp);
	
		/***************************************************
 	 	* Call PCM_OP_SUBSCRIPTION_CYCLE_FOLD
 	 	***************************************************/
		PCM_OP(ctxp, PCM_OP_SUBSCRIPTION_CYCLE_FOLD, flags,
                                cf_flistp, &ret_flistp, ebufp);
	}


Cleanup:
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_prep_fold error", ebufp);
	} else {
        	/***********************************************************
	         * Prepare results .
	         ***********************************************************/
		if (ret_flistp){
			*ret_flistpp = ret_flistp;
		}
		else{
        	/***********************************************************
	         * If PCM_OP_SUBSCRIPTION_CYCLE_FOLD does not return results
		 * then return the input poid
	         ***********************************************************/
		        rs_flistp = PIN_FLIST_CREATE(ebufp);
		        PIN_FLIST_FLD_SET(rs_flistp, PIN_FLD_POID, (void *)a_pdp, ebufp);
		        *ret_flistpp = rs_flistp;
		}

		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_subscription_pol_prep_fold return flist", 
			*ret_flistpp);
	}

	PIN_FLIST_DESTROY_EX(&cf_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bg_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp,NULL);
	/************************************************************
	* Because of the flag bg_got, memory for ret_bg_flistp
        * is getting allocated only once,so destroying the flist
        * only once  . 
	************************************************************/
	PIN_FLIST_DESTROY_EX(&ret_bg_flistp,NULL);
	return;
}
