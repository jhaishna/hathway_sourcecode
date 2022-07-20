/*******************************************************************
 *
 *	Copyright (c) 1999-2006 Oracle. All rights reserved.
 *
 *	This material is the confidential property of Oracle Corporation
 *	or its licensors and may be used, reproduced, stored or transmitted
 *	only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_get_deals.c:BillingVelocityInt:6:2006-Sep-14 22:02:35 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_CUST_POL_GET_DEALS policy operation.
 *
 * Retrieves a list of deals that are available for purchase
 * by the given account or service.
 *
 *******************************************************************/

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"

void
EXPORT_OP  op_cust_pol_get_deals(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);


static void
fm_cust_pol_get_deals(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);


static void
fm_cust_pol_get_deals_search(
	pcm_context_t		*ctxp,
	poid_t			*a_pdp,
	time_t			end_t,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);


static void
fm_cust_pol_get_deals_select(
	pcm_context_t		*ctxp,
	poid_t			*a_pdp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static int
fm_cust_pol_get_deals_match(
	poid_t			*a_pdp,
	pin_flist_t		*i_flistp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_get_deals_sort(
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

PIN_IMPORT void
fm_cust_convert_prod_disc_date_details_into_offset_unit(
	pcm_context_t   *ctxp,
	int32		infld,
	pin_flist_t	**d_flistpp,
	pin_errbuf_t	*ebufp);

PIN_IMPORT void
fm_cust_validity_decode_details(
	pcm_context_t   *ctxp,
	int32           details,
	int32           *unit_p,
	int32           *offset_p,
	pin_errbuf_t    *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_GET_DEALS operation. 
 *******************************************************************/
void
op_cust_pol_get_deals(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*o_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_GET_DEALS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_get_deals", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_get_deals input flist", i_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_get_deals(ctxp, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Set the results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*o_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_get_deals error", ebufp);
	} else {
		*o_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_get_deals return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_deals()
 *
 *	Retrieves the appropriate plan_list object from the db.
 *	Then performs a read_obj of each plan in the list. The
 *	result is an array of available plans for the relevant
 *	aac method (mecca, admin, etc).
 *
 *******************************************************************/
static void
fm_cust_pol_get_deals(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pin_flist_t	*d_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	poid_t		*pdp = NULL;
	time_t		*tp = NULL;
	time_t		end_t = 0;
	pin_cookie_t	cookie = NULL;
	int32		recid = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Use the incoming poid for db number
	 */
	pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	/*
	 * Also need the purchase timestamp
	 */
	tp = (time_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
	if (tp == (time_t *)NULL) {
		end_t = pin_virtual_time(NULL);
	} else {
		end_t = *tp;
	}

	/*
	 * Retrieve all deals from the db
	 */
	fm_cust_pol_get_deals_search(ctxp, pdp, end_t, &d_flistp, ebufp);

	/*
 	 * Select the deals for the given object
	 */
	fm_cust_pol_get_deals_select(ctxp, pdp, d_flistp, o_flistpp, ebufp);

	/*
 	 * Sort the deals that we found
	 */
	fm_cust_pol_get_deals_sort(*o_flistpp, ebufp);

	/* Decode the encoded dates into unit and offset 
	 * for each deal in the return flist.
	 */

	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(*o_flistpp,  PIN_FLD_DEALS,
		&recid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {
		
		fm_cust_convert_prod_disc_date_details_into_offset_unit(ctxp, 
			PIN_FLD_PRODUCTS, &flistp, ebufp);

		fm_cust_convert_prod_disc_date_details_into_offset_unit(ctxp, 
			PIN_FLD_DISCOUNTS, &flistp, ebufp);
	}

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_deals error", ebufp);
	}
        PIN_FLIST_DESTROY_EX(&d_flistp, ebufp);
	return;
}

/*******************************************************************
 * fm_cust_pol_get_deals_search():
 *
 *	Search for all deal objects in the database that are
 *	valid for the given timestamp (deal end_t of 0 = valid).
 *
 *******************************************************************/
static void
fm_cust_pol_get_deals_search(
	pcm_context_t	*ctxp,
	poid_t		*a_pdp,
	time_t		end_t,
	pin_flist_t	**o_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	poid_t		*pdp = NULL;
	poid_t		*brand_poidp = NULL;
	
	int64		id = (int64)258;
	int64		db = PIN_POID_GET_DB(a_pdp);

	time_t		dummy = 0;
	int32		cred;
	
	/*
	 * Create the search flist
	 */
	s_flistp = PIN_FLIST_CREATE(ebufp);

	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	pdp = PIN_POID_CREATE(db, "/deal%", (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_START_T, (void *)&end_t, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, (void *)&end_t, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, (void *)&dummy, ebufp);

	/*
	 * Per GET_PLANS:
	 *
	 * In a branding configuration, it is possible that the 
	 * account A belonging to a brand B might reside in a non-primary
	 * database. This implies that DM does not have information about
	 * the brand B owner to do the appropriate scoping. Therefore, we
	 * should disable the scoping temporarily for this search.
	 *
	 * Since we disable scoping, we need explicitly filter out the 
	 * plan lists that do not belong to this brand using the brand owner
	 * in the query.	 
	 */
	pdp = cm_fm_perm_get_brand_account(ebufp);
	if (PIN_POID_IS_NULL(pdp)) {
		pdp = fm_utils_lineage_root_poid(db, ebufp);
	} else {
		if (PIN_POID_GET_ID(pdp) == 1) {
			pdp = PIN_POID_CREATE(db, PIN_POID_GET_TYPE(pdp),
					      PIN_POID_GET_ID(pdp), ebufp);
		} else {
			pdp = PIN_POID_COPY(pdp, ebufp);
		}
	}
		
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_ACCOUNT_OBJ, pdp, ebufp);

	PIN_FLIST_ELEM_SET(s_flistp, (pin_flist_t *)NULL, 
		PIN_FLD_RESULTS, 0, ebufp);

	/*
	 * Do the search
	 */
	cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
        CM_FM_END_OVERRIDE_SCOPE(cred);
	
	/*
	 * Clean up.
	 */
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_deals_search error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_deals_retrieve()
 *
 *	Constructs the outgoing plan array by reading each plan
 *	on the given list and adding the results to the output
 *	as an element of the array.
 *
 *******************************************************************/
static void
fm_cust_pol_get_deals_select(
	pcm_context_t	*ctxp,
	poid_t		*a_pdp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pin_cookie_t	cookie = NULL;
	pin_flist_t	*flistp = NULL;
	int32		element_id;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Prep the output flist.
	 */
	*o_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_POID, (void *)a_pdp, ebufp);

	/*
	 * Read each deal from the results and see if it is a match
	 */
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS,
		&element_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		/*
		 * Is this deal a match? If so, keep it.
		 */
		if (fm_cust_pol_get_deals_match(a_pdp, flistp, ebufp)) {
			PIN_FLIST_ELEM_SET(*o_flistpp, flistp,
				PIN_FLD_DEALS, element_id, ebufp);
		}
	}

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_deals_select error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_deals_match():
 *
 *	Decide whether a given deal's permitted array allows the
 *	object type given to purchase it.
 *
 *******************************************************************/
static int
fm_cust_pol_get_deals_match(
	poid_t		*a_pdp,
	pin_flist_t	*i_flistp,
        pin_errbuf_t	*ebufp)
{
	char		*p_type = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return 0;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Compare the permitted string.
	 */
	p_type = (char *)PIN_FLIST_FLD_GET(i_flistp,
			PIN_FLD_PERMITTED, 0, ebufp);

	if (fm_utils_is_subtype(a_pdp, p_type)) {
		return (PIN_BOOLEAN_TRUE);
	}

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_deals_match error", ebufp);
	}

	return (PIN_BOOLEAN_FALSE);
}

/*******************************************************************
 * fm_cust_pol_get_deals_sort():
 *
 *	Sort the given list of deals by name.
 *
 *******************************************************************/
static void
fm_cust_pol_get_deals_sort(
	pin_flist_t	*o_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Construct the sort flist.
	 */
	s_flistp = PIN_FLIST_CREATE(ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_DEALS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, (void *)NULL, ebufp);

	/*
	 * Do the sort.
	 */
	PIN_FLIST_SORT(o_flistp, s_flistp, 0, ebufp);

	/*
	 * Clean up.
	 */
	PIN_FLIST_DESTROY(s_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_deals_sort error", ebufp);
	}

	return;
}

