/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 * Version 1.0: 05-09-2014: Kumar Gaurav: Added the MSO Customization of
 * MSO_OP_PYMT_ALLOCATE_PAYMENT opcode for allocating the payments after billing
 * as its not being done during billing process.
 *
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_pymt_allocate_payment.c /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_PYMT_ALLOCATE_PAYMENT operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
#include <time.h>
#include <math.h>
 
#include "pcm.h"
#include "ops/pymt.h"
#include "ops/ar.h"
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


#define CURRENCY 356
#define SERVICE_TYPE_BB "/service/telco/broadband"
/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_pymt_allocate_payment(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);
	
static void
fm_mso_pymt_allocate_payment(
        pcm_context_t   *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    *ebufp);

static void mso_pymt_allocate_reorder_items(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t			*ebufp);

static void 
fm_mso_item_transfer(
	pcm_context_t		*ctxp,
	int32                   flags,
	pin_flist_t		*trans_iflistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void
fm_pymt_get_sorted_charge_head_flist(
	pcm_context_t   *ctxp,
	pin_flist_t     *i_flistp,
	pin_flist_t     **o_flistpp,
	pin_errbuf_t    *ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_PYMT_ALLOCATE_PAYMENT operation.
 *******************************************************************/
void
mso_pymt_allocate_payment(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistpp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_PYMT_ALLOCATE_PAYMENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_pymt_allocate_payment opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"mso_op_pymt_allocate_payment input flist", i_flistp);
		
	
	/*************************************************************
	 * Payment Allocation
	 *************************************************************/	
	fm_mso_pymt_allocate_payment(ctxp, flags, i_flistp, &r_flistpp, ebufp);

	*o_flistpp = PIN_FLIST_COPY(r_flistpp, ebufp);

	return;
	
}


/*******************************************************************
 * Implementation of MSO_OP_PYMT_ALLOCATE_PAYMENT opcode.
 *******************************************************************/
static void
fm_mso_pymt_allocate_payment(
        pcm_context_t   *ctxp,
        int32                   flags,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    	*ebufp)
{
	pin_flist_t		*search_item_iflistp = NULL;
	pin_flist_t		*search_item_oflistp = NULL;
	pin_flist_t		*item_list_flistp = NULL;
	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*item2_flistp = NULL;
	pin_flist_t		*rflist = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*oflistp = NULL;
	pin_flist_t		*err_flistp = NULL;
	pin_flist_t		*transfer_flistp = NULL;
	pin_flist_t		*transfer_oflistp = NULL;
	pin_flist_t		*items_flistp = NULL;
	pin_flist_t		*sorted_item_list_flistp = NULL;
	pin_flist_t		*sorted_ret_item_list_flistp = NULL;
	pin_flist_t		*resflistp = NULL;
	pin_cookie_t		cookie = NULL;
	pin_cookie_t		cookie1 = NULL;
	pin_decimal_t		*zero = NULL; 
	pin_decimal_t		*item_due_from = NULL; 
	pin_decimal_t		*item_due_to = NULL; 
	pin_decimal_t		*temp_due = NULL; 
	pin_decimal_t		*trans_amt = NULL; 
	poid_t			*acc_pdp = NULL;
	poid_t			*ar_bi_pdp = NULL;
	poid_t			*bi_pdp = NULL;
	poid_t			*from_item = NULL;
	poid_t			*to_item = NULL;
	poid_t			*pdp = NULL;
	int32			rec_id = 0;
	int32			rec_id1 = 0;
	int32			s_flags = 512;
	int32			fail = 1;
	int32			success = 0;
	int32			status = 2;
	int32			check = 0;
	int32			trans_id = 0;
	int64			db = 1;
	u_int64			id = (u_int64)-1;
	char			*template = "select X from /item where F1 = V1 and F2 != V2 and F3 = V3 order by F4 asc ";
	int32			elem_count = 0;
	int32			count = 0;
	int32			*alloc_flag = NULL;
	int32			rec_id0 = 0;
	pin_cookie_t		cookie0 = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERRBUF_CLEAR (ebufp);

	zero = pbo_decimal_from_str("0.0", ebufp); 
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"fm_mso_pymt_allocate_payment input flist",i_flistp);

	/*********************************************************************
	# number of field entries allocated 20, used 3
	0 PIN_FLD_POID           POID [0] 0.0.0.0 /search -1 0
	0 PIN_FLD_STATUS         ENUM [0] 0
	0 PIN_FLD_ERROR_DESCR     STR [0] "Allocation Success"
	**********************************************************************/
	acc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	ar_bi_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AR_BILLINFO_OBJ, 0, ebufp);

	alloc_flag = (int32 *)PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_FLAGS,1,ebufp);	

	search_item_iflistp = PIN_FLIST_CREATE(ebufp);
	
	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_SET(search_item_iflistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_SET(search_item_iflistp, PIN_FLD_FLAGS, &s_flags, ebufp);

	PIN_FLIST_FLD_SET(search_item_iflistp, PIN_FLD_TEMPLATE, template, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(search_item_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_AR_BILLINFO_OBJ, ar_bi_pdp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(search_item_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DUE, zero, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(search_item_iflistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, &status, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(search_item_iflistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(search_item_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DUE, NULL, ebufp);
	// added this field in result to identify service type linked to the item
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp); 

	if(alloc_flag && *alloc_flag == 1){
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_BILL_OBJ, NULL, ebufp); 
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_DUE, NULL, ebufp); 
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_RECVD, NULL, ebufp); 
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ADJUSTED, NULL, ebufp); 
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TRANSFERED, NULL, ebufp); 
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"search input flist", search_item_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, flags, search_item_iflistp, &search_item_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"search output flist",search_item_oflistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Payment Items Error",search_item_oflistp);
		PIN_ERRBUF_RESET(ebufp);

		err_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(err_flistp, PIN_FLD_POID, pdp, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &fail, ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "52199", ebufp);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Items Error", ebufp);

		*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&err_flistp, NULL);

		goto CLEANUP;
	}
	
	item_list_flistp = PIN_FLIST_COPY(search_item_oflistp, ebufp);
	sorted_item_list_flistp = PIN_FLIST_CREATE(ebufp);
	//sorted_item_list_flistp = PIN_FLIST_COPY(search_item_oflistp, ebufp);
	PIN_FLIST_FLD_COPY(item_list_flistp, PIN_FLD_POID, sorted_item_list_flistp,PIN_FLD_POID, ebufp);

	// Add sorting here based on charge head/migrated item logic

	while ((resflistp = PIN_FLIST_ELEM_GET_NEXT(item_list_flistp, PIN_FLD_RESULTS,
		&rec_id0, 1, &cookie0, ebufp)) != (pin_flist_t *)NULL) {
		PIN_FLIST_ELEM_SET(sorted_item_list_flistp, resflistp, PIN_FLD_ITEMS, count, ebufp);
		count = count + 1;
	}
	/* Re-arrange the items on priority basis */
	mso_pymt_allocate_reorder_items(ctxp, sorted_item_list_flistp, &sorted_ret_item_list_flistp, ebufp);

	
	while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(item_list_flistp, PIN_FLD_RESULTS,
		&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL){
		
		trans_id = 0;
		item_due_from = PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_DUE, 0, ebufp);
		if (pbo_decimal_sign(item_due_from, ebufp) < 0 )
		{
			from_item = PIN_FLIST_FLD_GET(item_flistp, PIN_FLD_POID, 0, ebufp);
			transfer_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(transfer_flistp, PIN_FLD_POID, acc_pdp, ebufp);
			PIN_FLIST_FLD_SET(transfer_flistp, PIN_FLD_PROGRAM_NAME, 
				"fm_mso_pymt_allocate_payment", ebufp);
			PIN_FLIST_FLD_SET(transfer_flistp, PIN_FLD_ITEM_OBJ, from_item, ebufp);
			if(alloc_flag && *alloc_flag == 1){
				PIN_FLIST_FLD_COPY(item_flistp,PIN_FLD_DUE,transfer_flistp,PIN_FLD_DUE,ebufp);	
				PIN_FLIST_FLD_COPY(item_flistp,PIN_FLD_TRANSFERED,transfer_flistp,PIN_FLD_TRANSFERED,ebufp);	
			}

			rec_id1 =0;
			cookie1 = NULL;
			// changing the loop to oick items from sorted flist
			while ((item2_flistp = PIN_FLIST_ELEM_GET_NEXT(sorted_ret_item_list_flistp, PIN_FLD_ITEMS,
			&rec_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL){
				
				item_due_to = PIN_FLIST_FLD_GET(item2_flistp, PIN_FLD_DUE, 0, ebufp);
				if (pbo_decimal_sign(item_due_to, ebufp) > 0)
				{
					check = 1;
					to_item = PIN_FLIST_FLD_GET(item2_flistp, PIN_FLD_POID, 0, ebufp);
					
					items_flistp = PIN_FLIST_ELEM_ADD(transfer_flistp, PIN_FLD_ITEMS, 
						trans_id, ebufp);
					PIN_FLIST_FLD_SET(items_flistp, PIN_FLD_POID, to_item, ebufp);
					if(alloc_flag && *alloc_flag == 1){
						PIN_FLIST_FLD_COPY(item2_flistp,PIN_FLD_BILL_OBJ,items_flistp,PIN_FLD_BILL_OBJ,ebufp);	
						PIN_FLIST_FLD_COPY(item2_flistp,PIN_FLD_DUE,items_flistp,PIN_FLD_DUE,ebufp);	
						PIN_FLIST_FLD_COPY(item2_flistp,PIN_FLD_RECVD,items_flistp,PIN_FLD_RECVD,ebufp);	
						PIN_FLIST_FLD_COPY(item2_flistp,PIN_FLD_ADJUSTED,items_flistp,PIN_FLD_ADJUSTED,ebufp);	
					}
					trans_id = trans_id + 1;
					
					temp_due = pbo_decimal_add(item_due_from, item_due_to, ebufp);
					if (pbo_decimal_sign(temp_due, ebufp) >= 0)
					{
						trans_amt = pbo_decimal_copy(item_due_from, ebufp);
						PIN_FLIST_FLD_SET(items_flistp, PIN_FLD_AMOUNT, trans_amt, ebufp);
						if (PIN_ERRBUF_IS_ERR(ebufp)) {
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ERROR in Item Transfer");
							goto CLEANUP;
						}
						/*****************************************
						 *set item_list_flistp with updated Due
						 ****************************************/
						item_due_to = pbo_decimal_copy(temp_due, ebufp);
						PIN_FLIST_FLD_PUT(item2_flistp, PIN_FLD_DUE, item_due_to, ebufp);
						PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_DUE, zero, ebufp);	
						break;
					}
					else if(pbo_decimal_sign(temp_due,ebufp) == 0)
					{
						trans_amt = pbo_decimal_copy(item_due_from, ebufp);
						PIN_FLIST_FLD_SET(items_flistp, PIN_FLD_AMOUNT, trans_amt, ebufp);
						if (PIN_ERRBUF_IS_ERR(ebufp)) {
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ERROR in Item Transfer");
							goto CLEANUP;
						}
						/*****************************************
						 *set item_list_flistp with updated Due
						 ****************************************/
						PIN_FLIST_FLD_SET(item2_flistp, PIN_FLD_DUE, zero, ebufp);
						PIN_FLIST_FLD_SET(item_flistp, PIN_FLD_DUE, zero, ebufp);	
						break;
					}
					else
					{
						trans_amt = pbo_decimal_negate(item_due_to, ebufp);
						PIN_FLIST_FLD_SET(items_flistp, PIN_FLD_AMOUNT, trans_amt, ebufp);
						if (PIN_ERRBUF_IS_ERR(ebufp)) {
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ERROR in Item Transfer");
							goto CLEANUP;
						}
						/*****************************************
						 *set item_list_flistp with updated Due
						 ****************************************/
						item_due_from = pbo_decimal_copy(temp_due,ebufp);
						PIN_FLIST_FLD_SET(item2_flistp, PIN_FLD_DUE, zero, ebufp);
						PIN_FLIST_FLD_PUT(item_flistp, PIN_FLD_DUE, item_due_from, ebufp);	
					}
					pbo_decimal_destroy(&temp_due);
					pbo_decimal_destroy(&trans_amt);
				}
			}
			/*****************************************
			 * Call transfer function
			 ****************************************/
			if(alloc_flag && *alloc_flag == 1){

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Custom Item Transfer input flist", transfer_flistp);
				PCM_OP(ctxp, MSO_OP_BILL_ITEM_TRANSFER, 0, transfer_flistp, &transfer_oflistp, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Custom Item Transfer output flist",transfer_oflistp);
				PIN_FLIST_DESTROY_EX(&transfer_oflistp, NULL);
	

				if (PIN_ERRBUF_IS_ERR(ebufp)) {
                			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Payment Items allocation Error",search_item_oflistp);
                			PIN_ERRBUF_RESET(ebufp);

                			err_flistp = PIN_FLIST_CREATE(ebufp);
                			PIN_FLIST_FLD_PUT(err_flistp, PIN_FLD_POID, pdp, ebufp);
                			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &fail, ebufp);
                			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, "52199", ebufp);
                			PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, "Payment Items acclocation Error", ebufp);

                			*r_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
                			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Payment Items allocation Error flist", err_flistp);
                			PIN_FLIST_DESTROY_EX(&err_flistp, NULL);

                			goto CLEANUP;
        			}

			}else{
				fm_mso_item_transfer(ctxp, flags, transfer_flistp, &rflist, ebufp);
			}
		}
	}
	oflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(oflistp, PIN_FLD_POID, pdp, ebufp);
	PIN_FLIST_FLD_SET(oflistp, PIN_FLD_STATUS, &success, ebufp);
	if(check ==1)
	{
		PIN_FLIST_FLD_SET(oflistp,PIN_FLD_ERROR_DESCR,"Allocation Success",ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(oflistp, PIN_FLD_ERROR_DESCR, "There is oustanding item to knockoff",ebufp);
	}
	*r_flistpp = PIN_FLIST_COPY(oflistp, ebufp);
	
CLEANUP:
	PIN_FLIST_DESTROY_EX(&search_item_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&search_item_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&sorted_item_list_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&sorted_ret_item_list_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&transfer_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rflist, NULL);
	if(zero) pbo_decimal_destroy(&zero);
	return;
}

static void 
fm_mso_item_transfer(
	pcm_context_t		*ctxp,
	int32                   flags,
	pin_flist_t		*trans_iflistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*items_flistp = NULL;
	pin_flist_t		*trans_rflistp = NULL;

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                return;
        }
        PIN_ERRBUF_CLEAR (ebufp);
	
	/************************************************************************************************
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 3295252 0
	0 PIN_FLD_ITEM_OBJ       POID [0] 0.0.0.1 /item/cycle_forward/mso_sb_alc_paid 290394217161451917 4
	0 PIN_FLD_PROGRAM_NAME    STR [0] "Correction test"
	0 PIN_FLD_ITEMS         ARRAY [2] allocated 20, used 19
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /item/adjustment 289567384151832244 1
	1     PIN_FLD_AMOUNT       DECIMAL [0] -293.32
	************************************************************************************************/
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Item Transfer input flist", trans_iflistp);
	PCM_OP(ctxp, PCM_OP_BILL_ITEM_TRANSFER, flags, trans_iflistp, &trans_rflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Item Transfer output flist",trans_rflistp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ERROR in Item Transfer");
		goto Cleanup;
	}
Cleanup:
	PIN_FLIST_DESTROY_EX(&trans_rflistp, NULL);
	return;
}

static void mso_pymt_allocate_reorder_items(
	pcm_context_t			*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t			*ebufp)
{
	pin_flist_t			*misc_flistp = NULL;
	pin_flist_t			*non_misc_flistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*ret_flistp = NULL;
	pin_flist_t			*flistp = NULL;
	pin_cookie_t			cookie1 = NULL;
	pin_cookie_t			cookie2 = NULL;
	pin_cookie_t			cookie3 = NULL;
	int32				rec_id1 = 0;
	int32				rec_id2 = 0;
	int32				rec_id3 = 0;
	int32				misc_elem = 0;
	int32				non_misc_elem = 0;
	int32				elem_count = 0;
	int32				count = 0;
	poid_t				*item_pdp = NULL;
	poid_t				*svc_pdp = NULL;
	char				*item_type = NULL;
	char				*svc_type = NULL;
	pin_flist_t			*r_flistp = NULL;
	pin_flist_t                     *sort_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_pymt_allocate_reorder_items Input:", i_flistp);
	misc_flistp = PIN_FLIST_CREATE(ebufp);
	non_misc_flistp = PIN_FLIST_CREATE(ebufp);
	ret_flistp = PIN_FLIST_CREATE(ebufp);

	temp_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BILLINFO_OBJ, ret_flistp, PIN_FLD_POID, ebufp);
	elem_count = PIN_FLIST_ELEM_COUNT(temp_flistp, PIN_FLD_ITEMS, ebufp);
	//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Debug 1");
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(temp_flistp, PIN_FLD_ITEMS,
			&rec_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL) {
			item_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_POID, 1, ebufp);
			item_type = (char*) PIN_POID_GET_TYPE(item_pdp);
			//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Debug 2");
			if ( item_type && (strcmp((char*)item_type, "/item/misc") == 0) )
			{
				//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Debug 3");
				svc_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
				svc_type = (char *) PIN_POID_GET_TYPE(svc_pdp);
				if (svc_type && (strcmp((char*)svc_type, "") == 0) )
				{
					//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Debug 4");
					PIN_FLIST_ELEM_SET(misc_flistp, flistp, PIN_FLD_ITEMS, misc_elem, ebufp);
					misc_elem = misc_elem + 1;
				}
				else
				{
					PIN_FLIST_ELEM_SET(non_misc_flistp, flistp, PIN_FLD_ITEMS, non_misc_elem, ebufp);
					non_misc_elem = non_misc_elem + 1;
				}
				//PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Debug 5");
			}
			else
			{
				PIN_FLIST_ELEM_SET(non_misc_flistp, flistp, PIN_FLD_ITEMS, non_misc_elem, ebufp);
				non_misc_elem = non_misc_elem + 1;
			}
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Misc Items updated input flist", misc_flistp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Non-Misc Items updated input flist",non_misc_flistp);

	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(misc_flistp, PIN_FLD_ITEMS,
		&rec_id2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL) {
		PIN_FLIST_ELEM_SET(ret_flistp, flistp, PIN_FLD_ITEMS, count, ebufp);
		count = count + 1;
	}

	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(non_misc_flistp, PIN_FLD_ITEMS,
		&rec_id3, 1, &cookie3, ebufp)) != (pin_flist_t *)NULL) {
		PIN_FLIST_ELEM_SET(ret_flistp, flistp, PIN_FLD_ITEMS, count, ebufp);
		count = count + 1;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Updated ret_flistp: ",ret_flistp);
	/*
         * Re-arrange the ITEMS according to the payment knock-out priority rule
	 * for Broadband Customers
         */
	if(svc_type !=NULL && (strcmp(svc_type, SERVICE_TYPE_BB)==0) )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"It's a broadband service. Knocking off priority");
        	//sort_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
		//fm_pymt_get_sorted_charge_head_flist(ctxp, temp_flistp, &sort_flistp, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "sort flist",sort_flistp);
		r_flistp = PIN_FLIST_COPY(sort_flistp, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"It's a CATV service");
		i_flistp = PIN_FLIST_COPY(temp_flistp, ebufp);	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Updated input flist",i_flistp);
        	sort_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	}
	// End of BB customization
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);

	PIN_FLIST_DESTROY_EX(&misc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&non_misc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&temp_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);

	return;
}
