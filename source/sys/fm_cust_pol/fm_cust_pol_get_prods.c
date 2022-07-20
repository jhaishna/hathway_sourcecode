/*******************************************************************
 *
 *      Copyright (c) 1999-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_get_prods.c:BillingVelocityInt:4:2006-Sep-05 04:30:29 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_CUST_POL_GET_PRODUCTS policy operation.
 *
 * Retrieves a list of products that are available for purchase
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

EXPORT_OP void
op_cust_pol_get_products(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_get_products(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp);

static void
fm_cust_pol_get_products_search(
	pcm_context_t	*ctxp,
	poid_t		*a_pdp,
	pin_flist_t	**o_flistpp,
	pin_errbuf_t	*ebufp);


static void
fm_cust_pol_get_products_select(
	pcm_context_t	*ctxp,
	poid_t		*a_pdp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp);

static void
fm_cust_pol_get_products_match(
	poid_t		*a_pdp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp);



/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_GET_PRODUCTS operation. 
 *******************************************************************/
void
op_cust_pol_get_products(
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
	if (opcode != PCM_OP_CUST_POL_GET_PRODUCTS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_get_products", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_get_products input flist", i_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_get_products(ctxp, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Set the results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*o_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_get_products error", ebufp);
	} else {
		*o_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_get_products return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_products()
 *
 *	Retrieves the appropriate plan_list object from the db.
 *	Then performs a read_obj of each plan in the list. The
 *	result is an array of available plans for the relevant
 *	aac method (mecca, admin, etc).
 *
 *******************************************************************/
static void
fm_cust_pol_get_products(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pin_flist_t	*d_flistp = NULL;
	poid_t		*pdp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Use the incoming poid for db number
	 */
	pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	/*
	 * Retrieve all products from the db
	 */
	fm_cust_pol_get_products_search(ctxp, pdp, &d_flistp, ebufp);

	/*
 	 * Select the products for the given object
	 */
	fm_cust_pol_get_products_select(ctxp, pdp, d_flistp, o_flistpp, ebufp);

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_products error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_products_search():
 *
 *	Search for all deal objects in the database.
 *
 *******************************************************************/
static void
fm_cust_pol_get_products_search(
	pcm_context_t	*ctxp,
	poid_t		*a_pdp,
	pin_flist_t	**o_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	poid_t		*pdp = NULL;

	u_int64		id = (u_int64)213;
	u_int64		db = PIN_POID_GET_DB(a_pdp);


	/*
	 * Create the search flist
	 */
	s_flistp = PIN_FLIST_CREATE(ebufp);

	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	pdp = PIN_POID_CREATE(db, "/product", (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_PERMITTED, (void *)NULL, ebufp);

	/*
	 * Do the search
	 */
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);

	/*
	 * Clean up.
	 */
	PIN_FLIST_DESTROY(s_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_products_search error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_products_retrieve()
 *
 *	Constructs the outgoing plan array by reading each plan
 *	on the given list and adding the results to the output
 *	as an element of the array.
 *
 *******************************************************************/
static void
fm_cust_pol_get_products_select(
	pcm_context_t	*ctxp,
	poid_t		*a_pdp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pin_cookie_t	cookie = NULL;
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*r_flistp = NULL;

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
	flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS,
		&element_id, 1, &cookie, ebufp);
	while (flistp != (pin_flist_t *)NULL) {

		/*
		 * Is this deal a match?
		 */
		fm_cust_pol_get_products_match(a_pdp, flistp, &r_flistp, ebufp);

		/*
		 * If so, keep it.
		 */
		if (r_flistp != (pin_flist_t *)NULL) {
			PIN_FLIST_ELEM_PUT(*o_flistpp, r_flistp,
				PIN_FLD_PRODUCTS, element_id, ebufp);
		}

		/*
		 * Get the next plan
		 */
		flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS,
			&element_id, 1, &cookie, ebufp);
	}

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_products_select error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_products_match():
 *
 *	Decide whether the given product 'matches' an object.
 *
 *******************************************************************/
static void
fm_cust_pol_get_products_match(
	poid_t		*a_pdp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	const char	*a_type = PIN_POID_GET_TYPE(a_pdp);
	char		*p_type = NULL;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Compare the permitted string
	 */
	p_type = (char *)PIN_FLIST_FLD_GET(i_flistp,
			PIN_FLD_PERMITTED, 0, ebufp);

	if ((p_type != (char *)NULL) &&
		    (strncmp(a_type, p_type, strlen(p_type)) != 0)) {

		*o_flistpp = (pin_flist_t *)NULL;
		return;
	}

	/*
	 * Yes. the permitted string matched.
	 */
	*o_flistpp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_POID, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 0, ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_NAME, vp, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 0, ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_DESCR, vp, ebufp);

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_products_match error", ebufp);
	}

	return;
}
