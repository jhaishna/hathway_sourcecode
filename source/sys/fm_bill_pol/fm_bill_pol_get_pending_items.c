/*
 *
* Copyright (c) 2002, 2010, Oracle and/or its affiliates. All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static const char Sccs_id[] = "@(#)$Id: fm_bill_pol_get_pending_items.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:33:01 nishahan Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/bill.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_bill_utils.h"

EXPORT_OP void
op_bill_pol_get_pending_items(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void fm_bill_pol_get_pending_items();
static void fm_bill_pol_prepare_pending_items();

/*******************************************************************
 * PCM_OP_BILL_POL_GET_PENDING_ITEMS 
 *
 * This policy provides a hook to the Bill Now opcode for selecting 
 * the Pending Items to be included in the Bill. The default 
 * implementation select all the Pending Items to be included in the
 * Bill.
 *******************************************************************/
void 
op_bill_pol_get_pending_items(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	void		*vp = NULL;
	pcm_context_t	*ctxp = connp->dm_ctx;
	
	*r_flistpp = NULL;
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != PCM_OP_BILL_POL_GET_PENDING_ITEMS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_bill_pol_get_pending_items error",
			ebufp);
		return;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_bill_pol_get_pending_items input flist", i_flistp);

	/*******************************************************************
	 * Call the function in order to get pending items to be included
	 *  into the Bill Now. 
	 *******************************************************************/
	fm_bill_pol_get_pending_items(ctxp, i_flistp, r_flistpp, ebufp);
	
	 /******************************************************************
         * Set account poid and bill_type to the output flist.
         ******************************************************************/
	
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, vp, ebufp);
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"op_bill_pol_get_pending_items error", ebufp);
	}
	else {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_bill_pol_get_pending_items output flist", *r_flistpp);
	}
	return;
}

/*******************************************************************
 * This function gets pending items to be included in the Bill.
 *******************************************************************/
static void 
fm_bill_pol_get_pending_items(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp, 
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*temp_flistp = NULL;
	pin_flist_t	*o_flistp = NULL;
	pin_cookie_t	cookie = NULL;
	poid_t		*s_pdp = NULL;
	poid_t		*bi_pdp = NULL;
	poid_t		*acc_pdp = NULL;
	poid_t		*srv_pdp = NULL;
	poid_t		*srv_pdp1 = NULL;
	int32		rec_id = 0;
	int32		s_flags = 0;
	void		*vp = NULL;
	int64		database = 0;
	int32		status = 0;
	int		srv_cnt = 0;
	int		first_srv_flag = 0;
	char		s_template[BUFSIZ] = {0};
	char		tmp_s_template[BUFSIZ] = {0};

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/**************************************************************
	 * Check whether the input flist contains the list of pendning
	 * items. If so, prepare output flist for the policy, and return
	 **************************************************************/
	if (PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ITEMS,
						&rec_id, 1, &cookie, ebufp)) {
		fm_bill_pol_prepare_pending_items(ctxp, i_flistp, r_flistpp,
							ebufp);
		return;
	}

	/*********************************************************
	 * Get billinfo poid
	 *********************************************************/
	bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILLINFO_OBJ, 0,
									ebufp);
	/*********************************************************
	 * Get account poid
	 *********************************************************/
	acc_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0,
								  ebufp);
	/*********************************************************
	 * Create the search flist to search for the item objects
	 *********************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);

	database = pin_poid_get_db(bi_pdp);
	s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

        /***********************************************************
         * Start with the base search template. Add the search for list of
	 * services if they are presented in the input flist.
         ***********************************************************/
        strcpy(s_template, "select X from /item where F1 = V1 and F2 = V2 ");

	srv_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, 
						PIN_FLD_SERVICE_OBJ, 1, ebufp);
	if (srv_pdp) {
		/***********************************************
		 * Get all services from the service group
		 ***********************************************/
		fm_utils_bill_get_service_group(ctxp, srv_pdp,acc_pdp,&o_flistp,
									ebufp);
		/***********************************************************
		 * Different 'where' clauses will be used depending on 
		 * whether the services array contains a single service or 
		 * multiple ones.
		 ***********************************************************/
		srv_cnt = PIN_FLIST_ELEM_COUNT(o_flistp, PIN_FLD_SERVICES, 
									ebufp);

		if (1 == srv_cnt) {
			strncpy(tmp_s_template, s_template,
				sizeof(tmp_s_template)-1);
			pin_snprintf(s_template, sizeof(s_template), "%s %s",
				tmp_s_template, "and F3 = V3");
			cookie = NULL;
			temp_flistp = PIN_FLIST_ELEM_GET_NEXT(o_flistp, 
						PIN_FLD_SERVICES, &rec_id, 0, 
						&cookie, ebufp);
			srv_pdp1 = (poid_t *)PIN_FLIST_FLD_GET(temp_flistp, 
						PIN_FLD_SERVICE_OBJ, 0, ebufp);
			/* Add ARG 3 - SERVICE_OBJ */
			temp_flistp = PIN_FLIST_ELEM_ADD(s_flistp,PIN_FLD_ARGS,
								3, ebufp);
			PIN_FLIST_FLD_SET(temp_flistp,
				PIN_FLD_SERVICE_OBJ, (void *)srv_pdp1, ebufp);
		} else { /* Multiple services */

			/******************************************************
			 * SEARCH CONDITION:  service_obj_id0 in 
			 * (poid_id_1, poid_id_2, ...  poid_id_n)
			 *****************************************************/

			/******************************************************
			 * Append the initial part of 'in' clause to search 
			 * template 
			 *****************************************************/
			strncpy(tmp_s_template, s_template,
				sizeof(tmp_s_template)-1);	
			pin_snprintf(s_template, sizeof(s_template), "%s %s",
				tmp_s_template, "and service_obj_id0 in (");

			cookie = NULL;
			while ((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(o_flistp,
				PIN_FLD_SERVICES, &rec_id, 1, &cookie, ebufp))) {

				vp = (poid_t *)PIN_FLIST_FLD_GET(temp_flistp, 
						PIN_FLD_SERVICE_OBJ, 0, ebufp);

				if (first_srv_flag == 0) { /* First iteration */
					/*************************************
					 * NOTE: NO 'comma' before first poid ID
					 * in 'in' clause 
					 *************************************/
					first_srv_flag = 1;
				}
				else { /* NOT the first iteration */
					/*************************************
					 * Add 'comma' before adding this poid 
					 * ID to 'in' clause 
					 ************************************/
					strncpy(tmp_s_template, s_template,
						sizeof(tmp_s_template)-1);
					pin_snprintf(s_template, sizeof(s_template),
						"%s%s", tmp_s_template, ", ");
				}

				/* Add this poid ID to 'in' clause */
				strncpy(tmp_s_template, s_template,
					sizeof(tmp_s_template)-1);
				pin_snprintf(s_template, sizeof(s_template), 
					"%s%" I64_PRINTF_PATTERN "d",
					tmp_s_template, 
					PIN_POID_GET_ID((poid_t *)vp));
				}

			/*************************************
                	 * Append the last part of 'in' clause 
			 * to search template 
			 ************************************/
               		strncpy(tmp_s_template, s_template,
				sizeof(tmp_s_template)-1); 
			pin_snprintf(s_template, sizeof(s_template), "%s%s",
				tmp_s_template, ")");

        	} /* end else (multiple services) */
	}

	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, (void *)s_template, 
									ebufp);
	/*********************************************************
	 * Add the search arguments. This is a 2 arg =, = search
	 *********************************************************/
	temp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_BILLINFO_OBJ, (void *)bi_pdp,
			ebufp);

	status = PIN_ITEM_STATUS_PENDING;
	temp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_STATUS, (void *)&status, ebufp);

	/*********************************************************
	 * Add the results array for the search.
	 *********************************************************/
	temp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_ITEM_TOTAL, NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_ADJUSTED, NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_DISPUTED, NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_WRITEOFF, NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_RECVD, NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_TRANSFERED, NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_BILL_OBJ, NULL, ebufp);

	/**********************************************************
	 * do real search
	 **********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                       "fm_bill_pol_get_pending_items search flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, r_flistpp, ebufp);


	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);

	/********************************************************* 
	 * Errors..?
	 *********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_get_pending_items error",ebufp);
	}
	return;
}

/*******************************************************************
 * This function accepts list of poids of pending items, and  
 * read the item fields which should be retured by the policy
 *******************************************************************/

static void 
fm_bill_pol_prepare_pending_items(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp, 
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t     *s_flistp = NULL;
	pin_flist_t     *o_flistp = NULL;
	pin_flist_t     *temp_flistp = NULL;
	pin_cookie_t    cookie = NULL;
	int32           rec_id = 0;
	int32           rec_id1 = 0;
	void            *vp = NULL;
	int64           database = 1;
	int32           count = 0;
	int32		*status = NULL;
	int64		item_poid_id = 0;
	poid_t		*item_pdp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

	*r_flistpp = PIN_FLIST_CREATE(ebufp);

	count = PIN_FLIST_ELEM_COUNT(i_flistp, PIN_FLD_ITEMS, ebufp);
	if (count == 1) {
		temp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ITEMS,
				&rec_id, 1, &cookie, ebufp);
		vp = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_ITEM_OBJ, 1, ebufp);
		if (PIN_POID_IS_NULL((poid_t *)vp)) {
                	goto cleanup;	
		}
	}

	s_flistp = PIN_FLIST_CREATE(ebufp);
       	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ITEM_TOTAL, NULL, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ADJUSTED, NULL, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_DISPUTED, NULL, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_WRITEOFF, NULL, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_RECVD, NULL, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TRANSFERED, NULL, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_BILL_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_STATUS, NULL, ebufp);

	cookie = NULL;
	while ((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ITEMS,
				&rec_id, 1, &cookie, ebufp))) {

		vp = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_ITEM_OBJ, 1, ebufp);
		if (PIN_POID_IS_NULL((poid_t *)vp)) {
			pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_VALUE, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_bill_pol_prepare_pending_items error:"
			" Item Poid is NULL", ebufp);
                	goto cleanup;	
		}
		item_poid_id = PIN_POID_GET_ID(vp);
		item_pdp = PIN_POID_CREATE(database, "/item", item_poid_id, ebufp);		
        	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, item_pdp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                       "fm_bill_pol_get_pending_items read flds input flist", s_flistp);
        	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, s_flistp, &o_flistp, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                       "fm_bill_pol_get_pending_items read flds output flist", o_flistp);
	        	
		/***********************************************************
		 * If the item status is pending add it to the results .
		 * Otherwise return.
		 **********************************************************/
		status = (int32 *)PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_STATUS, 1,
									ebufp);
		PIN_FLIST_FLD_COPY(o_flistp, PIN_FLD_POID, temp_flistp, PIN_FLD_ITEM_OBJ, ebufp);
		if ((!status) || (*status != PIN_ITEM_STATUS_PENDING)) {
                       vp = PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_BILL_OBJ, 1, 
                                                                        ebufp);
                        if (!status || PIN_POID_IS_NULL((poid_t *)vp)){
			        pin_set_err(ebufp, PIN_ERRLOC_FM, 
			        PIN_ERRCLASS_SYSTEM_DETERMINATE,
			        PIN_ERR_BAD_VALUE, 0, 0, 0);
			        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			        "fm_bill_pol_prepare_pending_items error:"
			        "Item Status is not pending", ebufp);
                	        goto cleanup;	
                        }
                       PIN_FLIST_DESTROY_EX(&o_flistp, NULL); 
                         continue;
                      
		}
		PIN_FLIST_FLD_DROP(o_flistp, PIN_FLD_STATUS, ebufp);

		PIN_FLIST_ELEM_PUT (*r_flistpp, o_flistp, PIN_FLD_RESULTS, 
							rec_id1++, ebufp);
		o_flistp = NULL;
	}

cleanup:

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_bill_pol_prepare_pending_items error ", ebufp);
		PIN_FLIST_DESTROY_EX(r_flistpp, NULL);
	}
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
	return;
}
