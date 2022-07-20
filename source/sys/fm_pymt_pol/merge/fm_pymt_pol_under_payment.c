/*
 *
 *      Copyright (c) 1999 - 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_pymt_pol_under_payment.c:BillingVelocityInt:2:2006-Sep-05 21:54:09 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_PYMT_POL_UNDER_PAYMENT operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#define FILE_SOURCE_ID  "fm_pymt_pol_under_payment.c(1.8)"
#define SERVICE_TYPE_BB "/service/telco/broadband"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_pymt_pol_under_payment(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

EXPORT_OP void
fm_pymt_get_sorted_charge_head_flist(
	pcm_context_t   *ctxp,
	pin_flist_t     *i_flistp,
	pin_flist_t     **o_flistpp,
	pin_errbuf_t    *ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

PIN_IMPORT void fm_mso_utils_get_service_from_balgrp(
	pcm_context_t		*ctxp,
	poid_t			*balgrp_pdp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT
void fm_mso_utils_get_pending_bill_items(
	pcm_context_t           *ctxp,
	poid_t                  *billinfo_pdp,
	pin_flist_t             **o_flistpp,
	pin_errbuf_t            *ebufp);

PIN_IMPORT
void fm_mso_utils_read_any_object(
	pcm_context_t           *ctxp,
	poid_t                  *objp,
	pin_flist_t             **out_flistpp,
	pin_errbuf_t            *ebufp);

// MSO Customization Start
/*PIN_IMPORT void mso_retrieve_items(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);
*/
// MSO Customization End
/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_UNDER_PAYMENT operation.
 * Just pick only the items that can be paid off with the given
 * PIN_FLD_AMOUNT and throw away rest of the items selected.
 *******************************************************************/
void
op_pymt_pol_under_payment(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_decimal_t		*duep = NULL;
	pin_decimal_t		*amtp = NULL;

	int32			rec_id = 0;
	u_int			result = 0;
	pin_decimal_t		*total_allocated = NULL;
	pin_decimal_t		*total_recvd = NULL;
	pin_decimal_t		*item_amt = NULL;
	pin_decimal_t		*decp = NULL;
	u_int			status = PIN_SELECT_STATUS_UNDER_PAYMENT;
	// MSO Customization Start
	pin_flist_t			*item_in_flistp = NULL;
	pin_flist_t			*item_out_flistp = NULL;
	pin_flist_t			*temp_flistp = NULL;
	pin_flist_t			*misc_flistp = NULL;
	pin_flist_t			*non_misc_flistp = NULL;
	pin_flist_t                     *sort_flistp = NULL;
	poid_t				*item_pdp = NULL;
	poid_t				*svc_pdp = NULL;
	char				*item_type = NULL;
	char				*svc_type = NULL;
	int32				stack_opcode = 0;
	int32				rec_id1 = 0;
	int32				rec_id2 = 0;
	int32				rec_id3 = 0;
	int32				elem_count = 0;
	pin_cookie_t		cookie1 = NULL;
	pin_cookie_t		cookie2 = NULL;
	pin_cookie_t		cookie3 = NULL;
	int32				count = 0;
	int32				misc_elem = 0;
	int32				non_misc_elem = 0;
	poid_t			*balgrp_pdp = NULL;
	poid_t			*srvice_pdp = NULL;
	pin_flist_t		*service_flistp = NULL;
	pin_flist_t		*rslt_flistp = NULL;
	char			*service_type = NULL;	

	// MSO Customization End
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_UNDER_PAYMENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_under_payment opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_under_payment input flist", i_flistp);

	/***********************************************************
	 * Get the total money received from the input flist.
	 ***********************************************************/
	total_allocated	 = pbo_decimal_from_str("0.00", ebufp);
	amtp = (pin_decimal_t *)PIN_FLIST_FLD_GET(i_flistp, 
			PIN_FLD_AMOUNT, 0, ebufp);
	total_recvd = pbo_decimal_copy(amtp, ebufp);

	/***********************************************************
	 * Check for any credits with in the items selected and 
	 * include that money also in the toal received.
	 ***********************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ITEMS,
		&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		duep = (pin_decimal_t *)PIN_FLIST_FLD_GET(flistp,
			PIN_FLD_DUE, 0, ebufp);
		if (pbo_decimal_sign(duep, ebufp) < 0) {
			pbo_decimal_subtract_assign(total_recvd, duep, ebufp);
		}
	}

	cookie = NULL;
	r_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	// MSO Customization Start
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside Customization");

	balgrp_pdp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);	
	if(balgrp_pdp != NULL)
	{
		fm_mso_utils_get_service_from_balgrp(ctxp, balgrp_pdp, &service_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Error while getting service details error", ebufp);
			PIN_FLIST_DESTROY_EX(&service_flistp, NULL);
			return;
		}
		rslt_flistp = PIN_FLIST_ELEM_GET(service_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
		if(rslt_flistp != NULL)
		{
			srvice_pdp = PIN_FLIST_FLD_GET(rslt_flistp, PIN_FLD_POID, 0, ebufp);
			service_type = (char *)PIN_POID_GET_TYPE(srvice_pdp);
		}
	}

	temp_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	if(service_type !=NULL && (strcmp(service_type, SERVICE_TYPE_BB)!=0) )
	{
	  misc_flistp = PIN_FLIST_CREATE(ebufp);
	  non_misc_flistp = PIN_FLIST_CREATE(ebufp);

	  elem_count = PIN_FLIST_ELEM_COUNT(temp_flistp, PIN_FLD_ITEMS, ebufp);
	  PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1");
	  while ((flistp = PIN_FLIST_ELEM_GET_NEXT(temp_flistp, PIN_FLD_ITEMS,
		&rec_id1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL) {
		item_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_POID, 1, ebufp);
		item_type = (char*) PIN_POID_GET_TYPE(item_pdp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "2");
		if ( item_type && (strcmp((char*)item_type, "/item/misc") == 0) )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "3");
			svc_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
			svc_type = (char *) PIN_POID_GET_TYPE(svc_pdp);
			if (svc_type && (strcmp((char*)svc_type, "") == 0) )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "4");
				PIN_FLIST_ELEM_SET(misc_flistp, flistp, PIN_FLD_ITEMS, misc_elem, ebufp);
				misc_elem = misc_elem + 1;
			}
			else
			{
				PIN_FLIST_ELEM_SET(non_misc_flistp, flistp, PIN_FLD_ITEMS, non_misc_elem, ebufp);
				non_misc_elem = non_misc_elem + 1;
			}
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "5");
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
		PIN_FLIST_ELEM_SET(temp_flistp, flistp, PIN_FLD_ITEMS, count, ebufp);
		count = count + 1;
	  }

	  while ((flistp = PIN_FLIST_ELEM_GET_NEXT(non_misc_flistp, PIN_FLD_ITEMS,
		&rec_id3, 1, &cookie3, ebufp)) != (pin_flist_t *)NULL) {
		PIN_FLIST_ELEM_SET(temp_flistp, flistp, PIN_FLD_ITEMS, count, ebufp);
		count = count + 1;
	  }
	  PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Updated temp flist",temp_flistp);
	}

	/*
         * Re-arrange the ITEMS according to the payment knock-out priority rule
	 * for Broadband Customers
         */
	if(service_type !=NULL && (strcmp(service_type, SERVICE_TYPE_BB)==0) )
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"It's a broadband service. Knocking off priority");
        	//sort_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
		fm_pymt_get_sorted_charge_head_flist(ctxp, temp_flistp, &sort_flistp, ebufp);
        	r_flistp = PIN_FLIST_COPY(sort_flistp, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"It's a CATV service");
		i_flistp = PIN_FLIST_COPY(temp_flistp, ebufp);	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Updated input flist",i_flistp);
        	sort_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	}

        cookie = NULL;
		rec_id = 0;

	// MSO Customization End
	/***********************************************************
	 * Go thru the input opem items and allocate the money.
	 ***********************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(sort_flistp, PIN_FLD_ITEMS,
		&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		/***************************************************
	 	 * Check to see, whether we can squeeze in this item
	 	 ****************************************************/
		duep = (pin_decimal_t *)PIN_FLIST_FLD_GET(flistp, 
				PIN_FLD_DUE, 0, ebufp);
		amtp = (pin_decimal_t *)PIN_FLIST_FLD_GET(flistp, 
				PIN_FLD_AMOUNT, 1, ebufp);

		if (pbo_decimal_is_null(duep, ebufp) ||
			(pbo_decimal_sign(duep, ebufp) < 0))
			continue;

		if (pbo_decimal_compare(total_allocated,
					 total_recvd, ebufp) >= 0 ){
			PIN_FLIST_ELEM_DROP(r_flistp, PIN_FLD_ITEMS, 
				rec_id, ebufp);
			continue;
		}

		if ((!pbo_decimal_is_null(amtp, ebufp)) ){
			if ((pbo_decimal_sign(duep, ebufp) > 0) && 
				(pbo_decimal_sign(amtp, ebufp) == 0)) {
				PIN_FLIST_ELEM_DROP(r_flistp, PIN_FLD_ITEMS, 
					rec_id, ebufp);
				continue;
			}
		}

		pbo_decimal_destroy(&item_amt);
		if (!pbo_decimal_is_null(amtp, ebufp)) {
			item_amt = pbo_decimal_negate(amtp, ebufp);
		} else {
			item_amt = pbo_decimal_copy(duep, ebufp);
		}

		pbo_decimal_destroy(&decp);
		decp = pbo_decimal_add(total_allocated, item_amt, ebufp);
		if (pbo_decimal_compare(decp, total_recvd, ebufp) > 0) {
			pbo_decimal_destroy(&item_amt);
			item_amt = pbo_decimal_subtract(total_recvd,
				  total_allocated, ebufp);
		} 

		pbo_decimal_add_assign(total_allocated, item_amt, ebufp);
		pbo_decimal_negate_assign(item_amt, ebufp);

		flistp = PIN_FLIST_ELEM_GET(r_flistp, 
				PIN_FLD_ITEMS, rec_id, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_AMOUNT, 
				(void *)item_amt, ebufp);
	
	}

	if (flags & PIN_BILLFLG_SELECT_FINAL) {
		result = PIN_SELECT_RESULT_PASS;
	} else {
		result = PIN_SELECT_RESULT_FAIL;
		status |= PIN_SELECT_STATUS_MANUAL_ALLOC;
	}

	/***********************************************************
	 * Set the result & result status of select items.
	 ***********************************************************/
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_SELECT_RESULT, 
			(void *)&result, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_SELECT_STATUS, 
			(void *)&status, ebufp);
	/***********************************************************
	 * Cleanup.
	 ***********************************************************/
	pbo_decimal_destroy(&item_amt);
	pbo_decimal_destroy(&total_recvd);
	pbo_decimal_destroy(&decp);
	pbo_decimal_destroy(&total_allocated);
	PIN_FLIST_DESTROY(sort_flistp, NULL);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_under_payment error", ebufp);
		PIN_FLIST_DESTROY(r_flistp, NULL);
		*o_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*o_flistpp = r_flistp;

		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_pymt_pol_under_payment return flist", *o_flistpp);

	}
	PIN_FLIST_DESTROY_EX(&temp_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&misc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&non_misc_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&service_flistp, NULL);
	return;
}
/**************************************************************
* Function: fm_pymt_get_sorted_charge_head_flist()
* Author: Harish Kulkarni
* Descr: This function returns the open items ordered by oldest bill
* and inturn charge head wise
***************************************************************/
void fm_pymt_get_sorted_charge_head_flist(
	pcm_context_t   *ctxp,
	pin_flist_t     *i_flistp,
	pin_flist_t     **o_flistpp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t		*in_flistp = NULL;
	pin_flist_t		*sort_flistp = NULL;
	pin_flist_t		*bal_grp_flistp = NULL;
	pin_flist_t		*ar_sel_out_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int32			rec_id = 0;
	pin_cookie_t		cookie = NULL;
	poid_t			*bal_grp_pdp = NULL;
	poid_t			*billinfo_pdp = NULL;
	poid_t			*in_item_pdp = NULL;
	poid_t			*out_item_pdp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
			return;
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_pymt_get_sorted_charge_head_flist input flist", i_flistp);

	/***********************************************************
	 * Get the Billinfo object for the balance group
	 ***********************************************************/
	bal_grp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BAL_GRP_OBJ, 0, ebufp);

	/* Get the Billinfo poid */
	fm_mso_utils_read_any_object(ctxp, bal_grp_pdp, &bal_grp_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error in getting the balance group details!!");
		goto CLEANUP;
	}
	billinfo_pdp = PIN_FLIST_FLD_GET(bal_grp_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp );
	
	/* Get all the ordered open items for the billinfo */
	fm_mso_utils_get_pending_bill_items(ctxp, billinfo_pdp, &ar_sel_out_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error in selecting the open Items", ebufp);
		goto CLEANUP;
	}

	/* Sort the Input Flist according to the Olderst Bill and Charge-Head wise */
	in_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
	sort_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

	cookie = NULL;
	rec_id = 0;
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_ITEMS,
			&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {
		PIN_FLIST_ELEM_DROP( sort_flistp, PIN_FLD_ITEMS, rec_id, ebufp);
	}

	cookie = NULL;
	rec_id = 0;
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(ar_sel_out_flistp, PIN_FLD_ITEMS,
			&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_FLIST_ELEM_COPY(ar_sel_out_flistp, PIN_FLD_ITEMS, rec_id, 
			sort_flistp, PIN_FLD_ITEMS, rec_id, ebufp);
	}
	*o_flistpp = PIN_FLIST_COPY(sort_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_pymt_get_sorted_charge_head_flist output flist", *o_flistpp);
	CLEANUP:
		PIN_FLIST_DESTROY_EX(&bal_grp_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&ar_sel_out_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&sort_flistp, NULL);

	return;
}

