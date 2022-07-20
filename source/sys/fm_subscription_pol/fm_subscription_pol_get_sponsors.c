/*******************************************************************
 *
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates.All rights reserved. 
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_subscription_pol_get_sponsors.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 06:17:07 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_SUBSCRIPTION_POL_GET_SPONSORS policy operation.
 *
 * Retrieves a list of sponsors that are available for purchase
 * by the given account or service.
 *
 *******************************************************************/

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "pcm.h"
#include "ops/cust.h"
#include "ops/subscription.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_subscription.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"

#define FILE_SOURCE_ID	"fm_subscription_pol_get_sponsors.c(4)"

void
EXPORT_OP  op_subscription_pol_get_sponsors(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);


static void
fm_subs_pol_get_sponsors(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);


static void
fm_subs_pol_get_sponsors_search(
	pcm_context_t		*ctxp,
	poid_t			*a_pdp,
	time_t			end_t,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);


static void
fm_subs_pol_get_sponsors_select(
	pcm_context_t		*ctxp,
	poid_t			*a_pdp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static int
fm_subs_pol_get_sponsors_match(
	poid_t			*a_pdp,
	pin_flist_t		*i_flistp,
        pin_errbuf_t		*ebufp);

static void
fm_subs_pol_get_sponsors_sort(
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);


/*******************************************************************
 * Main routine for the PCM_OP_SUBSCRIPTION_POL_GET_SPONSORS operation. 
 *******************************************************************/
void
op_subscription_pol_get_sponsors(
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
	if (opcode != PCM_OP_SUBSCRIPTION_POL_GET_SPONSORS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_subscription_pol_get_sponsors", 
			ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_subscription_pol_get_sponsors input flist", i_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_subs_pol_get_sponsors(ctxp, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Set the results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*o_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_subscription_pol_get_sponsors error", ebufp);
	} else {
		*o_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_subscription_pol_get_sponsors return flist", 
			r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_subs_pol_get_sponsors()
 *
 *	Retrieves the appropriate plan_list object from the db.
 *	Then performs a read_obj of each plan in the list. The
 *	result is an array of available plans for the relevant
 *	aac method (mecca, admin, etc).
 *
 *******************************************************************/
static void
fm_subs_pol_get_sponsors(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pin_flist_t	*sp_flistp = NULL;
	poid_t		*pdp = NULL;
	time_t		*tp = NULL;
	time_t		end_t;

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
	 * Retrieve all sponsors from the db
	 */
	fm_subs_pol_get_sponsors_search(ctxp, pdp, end_t, &sp_flistp, ebufp);

	/*
 	 * Select the sponsors for the given object
	 */
	fm_subs_pol_get_sponsors_select(ctxp, pdp, sp_flistp, o_flistpp, ebufp);

	/*
 	 * Sort the sponsors that we found
	 */
	fm_subs_pol_get_sponsors_sort(*o_flistpp, ebufp);

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_subs_pol_get_sponsors error", ebufp);
	}
        PIN_FLIST_DESTROY_EX(&sp_flistp, ebufp);
	return;
}

/*******************************************************************
 * fm_subs_pol_get_sponsors_search():
 *
 *	Search for all sponsorship objects in the database that are
 *	valid for the given timestamp (sponsorship end_t of 0 = valid).
 *
 *******************************************************************/
static void
fm_subs_pol_get_sponsors_search(
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
	
	int64		db = PIN_POID_GET_DB(a_pdp);

	time_t		dummy = 0;
	int32		cred;
	int32		sflags = SRCH_DISTINCT;
	void		*vp = NULL;
	
        /******************************************************************
         * Set the search template.
         ******************************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&sflags,
                ebufp);

        vp = (void *)"select X from /sponsorship where F1 like V1 and F2 <= " \
		"V2 and ( F3 > V3 or F4 = V4 ) and F5 = V5 ";
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, vp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	pdp = PIN_POID_CREATE(db, "/sponsorship%", (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_START_T, (void *)&end_t, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, (void *)&end_t, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, (void *)&dummy, ebufp);

        /******************************************************************
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
         ******************************************************************/
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

        /******************************************************************
	 * Do the search
         ******************************************************************/
	cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
        CM_FM_END_OVERRIDE_SCOPE(cred);
	
        /******************************************************************
	 * Clean up.
         ******************************************************************/
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

        /******************************************************************
	 * Errors?
         ******************************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_subs_pol_get_sponsors_search error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_subs_pol_get_sponsors_retrieve()
 *
 *	Constructs the outgoing plan array by reading each plan
 *	on the given list and adding the results to the output
 *	as an element of the array.
 *
 *******************************************************************/
static void
fm_subs_pol_get_sponsors_select(
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
	 * Read each sponsorship from the results and see if it is a match
	 */
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS,
		&element_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		/*
		 * Is this sponsorship a match? If so, keep it.
		 */
		if (fm_subs_pol_get_sponsors_match(a_pdp, flistp, ebufp)) {
			PIN_FLIST_ELEM_SET(*o_flistpp, flistp,
				PIN_FLD_SPONSORS, element_id, ebufp);
		}
	}

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_subs_pol_get_sponsors_select error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_subs_pol_get_sponsors_match():
 *
 *	Decide whether a given sponsorship's permitted array allows the
 *	object type given to purchase it.
 *
 *******************************************************************/
static int
fm_subs_pol_get_sponsors_match(
	poid_t		*a_pdp,
	pin_flist_t	*i_flistp,
        pin_errbuf_t	*ebufp)
{

	if (PIN_ERR_IS_ERR(ebufp))
		return 0;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Your customizations to choose the sponsorship object based on
	 * your criteria will come here. By default every object is returned.
	 */


	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_subs_pol_get_sponsors_match error", ebufp);
	}

	return (PIN_BOOLEAN_TRUE);
}

/*******************************************************************
 * fm_subs_pol_get_sponsors_sort():
 *
 *	Sort the given list of sponsors by name.
 *
 *******************************************************************/
static void
fm_subs_pol_get_sponsors_sort(
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
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_SPONSORS, 0, ebufp);
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
			"fm_subs_pol_get_sponsors_sort error", ebufp);
	}

	return;
}

