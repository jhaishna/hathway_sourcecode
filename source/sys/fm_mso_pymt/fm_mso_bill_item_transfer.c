/*
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 * Version 1.0: 27-09-2014: Harish Kulkarni: Added the customization for broadband
 * customers for allocating the AR charg headwise per bill.
 */

#ifndef lint
static char Sccs_id[] = "@(#)$Id: fm_mso_bill_item_transfer.c /cgbubrm_7.5.0. 2013/09/26 04:39:34 vsabarik Exp $";
#endif

/*******************************************************************
 * Contains the MSO_OP_BILL_ITEM_TRANSFER operation. 
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

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
mso_op_bill_item_transfer(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the MSO_OP_BILL_ITEM_TRANSFER operation.
 *******************************************************************/
void
mso_op_bill_item_transfer(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx; 
	
	pin_flist_t	*bill_read_flistp = NULL;
	pin_flist_t	*bill_to_flistp = NULL;
	pin_flist_t	*item_flistp = NULL;
	pin_flist_t	*item_update_iflistp = NULL;
	pin_flist_t	*item_update_oflistp = NULL;
	pin_flist_t	*bill_update_iflistp = NULL;
	pin_flist_t	*oitem_update_iflistp = NULL;
	pin_flist_t	*oitem_update_oflistp = NULL;
	pin_flist_t	*bill_elem_flistp = NULL;
	pin_flist_t	*bill_upi_flistp = NULL;
	pin_flist_t	*bill_upo_flistp = NULL;
	
	poid_t		*pr_bill_obj = NULL;
	poid_t		*old_bill_obj = NULL;

	int32		rec_id = 0;
	int32		brec_id = 0;
	int32		status = 4;
	int32		bill_count = 0;
	int32		bill_match = 0;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	bcookie = NULL;

	pin_decimal_t	*from_orig_due = NULL;
	pin_decimal_t	*to_orig_due = NULL;
	pin_decimal_t	*to_orig_recvd = NULL;
	pin_decimal_t	*to_orig_adjusted = NULL;  
	pin_decimal_t	*to_bill_due = NULL;
	pin_decimal_t	*to_bill_recvd = NULL;
	pin_decimal_t	*to_bill_adjusted = NULL;  
	pin_decimal_t	*temp_transfer_amt = NULL;
	pin_decimal_t	*transfer_amt = NULL;
	pin_decimal_t	*sum_transfer_amt = NULL;
	pin_decimal_t	*zero_amt = NULL;
	
	time_t          time_now = 0;

	poid_t		*from_item_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERRBUF_CLEAR(ebufp);
	
	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != MSO_OP_BILL_ITEM_TRANSFER) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_bill_item_transfer opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_op_bill_item_transfer input flist", i_flistp);

	time_now = pin_virtual_time((time_t *)NULL);
	sum_transfer_amt = pbo_decimal_from_str("0.00",ebufp);
	zero_amt = pbo_decimal_from_str("0.00",ebufp);

	from_item_pdp = PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_ITEM_OBJ,1,ebufp);
	
	transfer_amt = PIN_FLIST_FLD_TAKE(i_flistp,PIN_FLD_TRANSFERED,1,ebufp);
	from_orig_due = PIN_FLIST_FLD_TAKE(i_flistp,PIN_FLD_DUE,1,ebufp); 
 
	rec_id =0;
	cookie = NULL;
	while ((item_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ITEMS, &rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL){

		bill_match = 0;

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"To Item being processed is", item_flistp);

		to_orig_due = PIN_FLIST_FLD_TAKE(item_flistp,PIN_FLD_DUE,1,ebufp);		
		to_orig_recvd = PIN_FLIST_FLD_TAKE(item_flistp,PIN_FLD_RECVD,1,ebufp);		
		to_orig_adjusted = PIN_FLIST_FLD_TAKE(item_flistp,PIN_FLD_ADJUSTED,1,ebufp);		

		temp_transfer_amt = PIN_FLIST_FLD_GET(item_flistp,PIN_FLD_AMOUNT,1,ebufp);		

		pbo_decimal_add_assign(sum_transfer_amt, temp_transfer_amt, ebufp);
		pbo_decimal_add_assign(to_orig_due, temp_transfer_amt, ebufp);

		item_update_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(item_flistp,PIN_FLD_POID,item_update_iflistp,PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_PUT(item_update_iflistp,PIN_FLD_DUE,to_orig_due,ebufp);

		old_bill_obj = PIN_FLIST_FLD_GET(item_flistp,PIN_FLD_BILL_OBJ,1,ebufp);

		if(bill_update_iflistp != NULL){

		brec_id =0;
        	bcookie = NULL;
        	while ((bill_elem_flistp = PIN_FLIST_ELEM_GET_NEXT(bill_update_iflistp, PIN_FLD_BILLS, &brec_id, 1, &bcookie, ebufp)) != (pin_flist_t *)NULL){

			pr_bill_obj = PIN_FLIST_FLD_GET(bill_elem_flistp,PIN_FLD_POID,1,ebufp);

			if(pr_bill_obj && old_bill_obj && PIN_POID_COMPARE(old_bill_obj,pr_bill_obj,0,ebufp) == 0){
		
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"current and old item belongs to same bill");

				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"bill_elem_flistp for same bill",bill_elem_flistp);

				to_bill_due = PIN_FLIST_FLD_TAKE(bill_elem_flistp,PIN_FLD_DUE,1,ebufp);		
				to_bill_recvd = PIN_FLIST_FLD_TAKE(bill_elem_flistp,PIN_FLD_RECVD,1,ebufp);		
				to_bill_adjusted = PIN_FLIST_FLD_TAKE(bill_elem_flistp,PIN_FLD_ADJUSTED,1,ebufp);		
				pbo_decimal_add_assign(to_bill_due, temp_transfer_amt, ebufp);

				PIN_FLIST_FLD_PUT(bill_elem_flistp,PIN_FLD_DUE,to_bill_due,ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"bill_elem_flistp for same bill",bill_elem_flistp);
				bill_match = 1;
				break;
			}
		}
		}


		// bill not found in existing flist. so create new element
		if(old_bill_obj && (bill_update_iflistp == NULL || bill_match == 0)){

			//create new bill update flist as the current item is the first item being processed
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"New bill for items in item transfer input");
			bill_read_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(item_flistp,PIN_FLD_BILL_OBJ,bill_read_flistp,PIN_FLD_POID,ebufp);
			PIN_FLIST_FLD_SET(bill_read_flistp,PIN_FLD_DUE,NULL,ebufp);
			PIN_FLIST_FLD_SET(bill_read_flistp,PIN_FLD_RECVD,NULL,ebufp);
			PIN_FLIST_FLD_SET(bill_read_flistp,PIN_FLD_ADJUSTED,NULL,ebufp);
		 
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"From Bill read input flist", bill_read_flistp);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, bill_read_flistp, &bill_to_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"From Bill read output flist",bill_to_flistp);

		        if(PIN_ERRBUF_IS_ERR(ebufp)) {
       			        pin_set_err(ebufp, PIN_ERRLOC_DM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NOT_FOUND, PIN_FLD_BILL_OBJ, 0, opcode);
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "op_mso_search error", ebufp);
				goto Cleanup;
        		}


			to_bill_due = PIN_FLIST_FLD_TAKE(bill_to_flistp,PIN_FLD_DUE,1,ebufp);		
			to_bill_recvd = PIN_FLIST_FLD_TAKE(bill_to_flistp,PIN_FLD_RECVD,1,ebufp);		
			to_bill_adjusted = PIN_FLIST_FLD_TAKE(bill_to_flistp,PIN_FLD_ADJUSTED,1,ebufp);		

			PIN_FLIST_DESTROY_EX(&bill_read_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&bill_to_flistp, NULL);
		
			pbo_decimal_add_assign(to_bill_due, temp_transfer_amt, ebufp);

			if(bill_update_iflistp == NULL){
				bill_update_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_POID,bill_update_iflistp,PIN_FLD_POID,ebufp);
			}
			bill_elem_flistp = PIN_FLIST_ELEM_ADD(bill_update_iflistp,PIN_FLD_BILLS,bill_count,ebufp);
			PIN_FLIST_FLD_COPY(item_flistp,PIN_FLD_BILL_OBJ,bill_elem_flistp,PIN_FLD_POID,ebufp);
			PIN_FLIST_FLD_PUT(bill_elem_flistp,PIN_FLD_DUE,to_bill_due,ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"bill_elem_flistp after new is",bill_update_iflistp);
			bill_count++;

		}
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ERROR in Item Transfer");
			goto Cleanup;
		}

		if (pbo_decimal_compare(to_orig_due, zero_amt, ebufp) == 0 ){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Updating STATUS & CLOSED_t of item");
			PIN_FLIST_FLD_SET(item_update_iflistp,PIN_FLD_STATUS,&status,ebufp);
			PIN_FLIST_FLD_SET(item_update_iflistp,PIN_FLD_CLOSED_T,&time_now,ebufp);
		}

		if(strcmp((char *)PIN_POID_GET_TYPE(from_item_pdp),PIN_OBJ_TYPE_ITEM_ADJUSTMENT) == 0){
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Updating ADJUSTED of to item");
			pbo_decimal_add_assign(to_orig_adjusted, temp_transfer_amt, ebufp);
			pbo_decimal_add_assign(to_bill_adjusted, temp_transfer_amt, ebufp);
			PIN_FLIST_FLD_PUT(item_update_iflistp,PIN_FLD_ADJUSTED,to_orig_adjusted,ebufp);
		}else{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Updating RECVD of to item");
			pbo_decimal_add_assign(to_orig_recvd, temp_transfer_amt, ebufp);
			pbo_decimal_add_assign(to_bill_recvd, temp_transfer_amt, ebufp);
			PIN_FLIST_FLD_PUT(item_update_iflistp,PIN_FLD_RECVD,to_orig_recvd,ebufp);
		}

		if(old_bill_obj){
			PIN_FLIST_FLD_PUT(bill_elem_flistp,PIN_FLD_RECVD,to_bill_recvd,ebufp);
			PIN_FLIST_FLD_PUT(bill_elem_flistp,PIN_FLD_ADJUSTED,to_bill_adjusted,ebufp);
		}
		
		if(bill_update_iflistp){
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"bill_update_iflistp after processing item",bill_update_iflistp);
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"item_update_iflistp input flist is",item_update_iflistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, item_update_iflistp, &item_update_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"item_update_oflistp input flist is",item_update_oflistp);

		
		PIN_FLIST_DESTROY_EX(&item_update_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&item_update_oflistp, NULL);
		
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ERROR in Item Transfer");
			goto Cleanup;
		}
	}

	if(bill_update_iflistp){
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"bill_update_iflistp after processing item",bill_update_iflistp);
	}

	pbo_decimal_add_assign(transfer_amt, sum_transfer_amt, ebufp);
	pbo_decimal_subtract_assign(from_orig_due, sum_transfer_amt, ebufp);
	
	oitem_update_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_ITEM_OBJ,oitem_update_iflistp,PIN_FLD_POID,ebufp);
	PIN_FLIST_FLD_PUT(oitem_update_iflistp,PIN_FLD_DUE,from_orig_due,ebufp);
	PIN_FLIST_FLD_PUT(oitem_update_iflistp,PIN_FLD_TRANSFERED,transfer_amt,ebufp); 
	
	if (pbo_decimal_compare(from_orig_due, zero_amt, ebufp) == 0 ){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Updating STATUS & CLOSED_t of from item");
		PIN_FLIST_FLD_SET(oitem_update_iflistp,PIN_FLD_STATUS,&status,ebufp);
		PIN_FLIST_FLD_SET(oitem_update_iflistp,PIN_FLD_CLOSED_T,&time_now,ebufp);
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"oitem_update_iflistp input flist is",oitem_update_iflistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, oitem_update_iflistp, &oitem_update_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"oitem_update_oflistp input flist is",oitem_update_oflistp);

	PIN_FLIST_DESTROY_EX(&oitem_update_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&oitem_update_oflistp, NULL);

	brec_id =0;
       	bcookie = NULL;
       	while (bill_update_iflistp && (bill_upi_flistp = PIN_FLIST_ELEM_GET_NEXT(bill_update_iflistp, PIN_FLD_BILLS, &brec_id, 1, &bcookie, ebufp)) != (pin_flist_t *)NULL){

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"bill_upi_flistp input flist is",bill_upi_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, flags, bill_upi_flistp, &bill_upo_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"bill_upi_flistp input flist is",bill_upo_flistp);
		PIN_FLIST_DESTROY_EX(&bill_upo_flistp, NULL);
	}

	PIN_FLIST_DESTROY_EX(&bill_update_iflistp, NULL);
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "ERROR in Item Transfer");
		goto Cleanup;
	}

	*o_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);

Cleanup: 

 	if (!pbo_decimal_is_null(sum_transfer_amt, ebufp))
	{
		pbo_decimal_destroy(&sum_transfer_amt);
	}
	
	
	if (!pbo_decimal_is_null(zero_amt, ebufp))
	{
		pbo_decimal_destroy(&zero_amt);
	}
	
	return;
	
}
