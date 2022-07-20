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
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_get_plans.c:BillingVelocityInt:3:2006-Sep-05 04:30:24 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_CUST_POL_GET_PLANS policy operation.
 *
 * Retrieves a list of plans for account creation/upgrade that
 * are available for the aac method in question (mecca, admin, etc).
 *
 *******************************************************************/

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"

EXPORT_OP void
op_cust_pol_get_plans(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_get_plans(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp);

static void
fm_cust_pol_get_plans_locate(
	pcm_context_t	*ctxp,
	int64		planlist_db,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp);

static void
fm_cust_pol_get_plans_search(
	pcm_context_t	*ctxp,
	int64		planlist_db,
	char		*name,
	char		*type,
	pin_flist_t	**o_flistpp,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_get_plans_retrieve(
	pcm_context_t	*ctxp,
	int64		brand_db,
	pin_flist_t	*i_flistp,
	pin_flist_t	*p_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp);

static void
fm_cust_pol_get_plans_read(
	pcm_context_t	*ctxp,
	int64		brand_db,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_GET_PLANS operation. 
 *******************************************************************/
void
op_cust_pol_get_plans(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*auth_flistp = NULL;
	int			*auth_result = 0;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_GET_PLANS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_get_plans", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_get_plans input flist", i_flistp);

#ifdef	BILL_GROUP_AUTHORIZATION

	/******************************************************************
	 * check to see the PIN_FLD_USERID is allowed to change password
	 * for this account.
	 ******************************************************************/
	fm_utils_verify_relation(connp, i_flistp, opcode, &auth_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_AUTHORIZATION_FAIL, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_get_plans authorization error", ebufp);
		PIN_FLIST_DESTROY_EX(&auth_flistp, NULL);
		return;
	} 

	auth_result = PIN_FLIST_FLD_GET(auth_flistp, PIN_FLD_RESULT, 0, ebufp);
	if (auth_result && *auth_result != PIN_BOOLEAN_TRUE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_AUTHORIZATION_FAIL, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_get_plans authorization error", ebufp);
		PIN_FLIST_DESTROY_EX(&auth_flistp, NULL);
		return;
	}
	PIN_FLIST_DESTROY_EX(&auth_flistp, NULL);

#endif	/* BILL_GROUP_AUTHORIZATION */

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_get_plans(ctxp, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Set the results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_get_plans error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_get_plans return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_plans()
 *
 *	Retrieves the appropriate plan_list object from the db.
 *	Then performs a read_obj of each plan in the list. The
 *	result is an array of available plans for the relevant
 *	aac method (mecca, admin, etc).
 *
 *******************************************************************/
static void
fm_cust_pol_get_plans(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pin_flist_t	*p_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	poid_t		*pdp = NULL;
	int64		brand_db = 0;
	int64		planlist_db = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Get the database of brand.
	 */
	PCM_OP(ctxp, PCM_OP_PERM_GET_CREDENTIALS, 0, i_flistp, &r_flistp, 
		ebufp);
	pdp = (poid_t *)PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_BRAND_OBJ, 0, 
					       ebufp);
	if ( PIN_POID_GET_ID(pdp) == 1 ) {
		/*
		 * this has the root brand so
		 * Get the database number of the input PIN_FLD_POID.
		 */
		pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, 
					       		ebufp);
	}
	brand_db = PIN_POID_GET_DB(pdp);
	PIN_FLIST_DESTROY_EX( &r_flistp, NULL );

	/*
	 * Figure out the database where the plan lists are located.
	 *
	 * The /group/plan_list is the only pricelist object that
	 * does not get replicated in a multidb configuration currently
	 * and resides ONLY in the primary database.
	 */
	if (cm_fm_is_multi_db() ) {
		cm_fm_get_primary_db_no(&planlist_db, ebufp);
	} else {
		planlist_db = brand_db;
	}

	/*
	 * Locate the desired plan_list
	 */
	fm_cust_pol_get_plans_locate(ctxp, planlist_db, i_flistp, &p_flistp, 
				     ebufp);

	/*
 	 * Read the plans on the list
	 */
	fm_cust_pol_get_plans_retrieve(ctxp, brand_db, i_flistp, p_flistp, 
				       o_flistpp, ebufp);

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_plans error", ebufp);
	}
	PIN_FLIST_DESTROY_EX(&p_flistp, NULL);
	return;
}

/*******************************************************************
 * fm_cust_pol_get_plans_locate()
 *
 *	Retrieves the appropriate plan_list object from the db.
 *
 *	For now we just search for the list that belongs to the
 *	aac access method being used. If we don't find a list, we
 *	search for the default list (which might also not exist).
 *	At some point, additional selection criteria might be used.
 *
 *******************************************************************/
static void
fm_cust_pol_get_plans_locate(
	pcm_context_t	*ctxp,
	int64		planlist_db,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	poid_t		*pdp = NULL;
	poid_t		*cfg_master_dbp = NULL;
	char		*type = NULL;
	char		*name = NULL;
	pin_flist_t	*r_flistp = NULL;

	int		err = PIN_ERR_NONE;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Null the results for safety
	 */
	*o_flistpp = (pin_flist_t *)NULL;

	/*
	 * Drive the search by the type of plan requested...
	 */
	pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	if (PIN_POID_IS_TYPE_ONLY(pdp)) {
		/*
		 * Assume this is a new account, get new plans
		 */
		type = "new";
	} else {
		/*
		 * Assume this is an existing account, get addon plans
		 */
		type = "addon";
	}

	/*
	 * ...and by the name (derived from aac_access)
	 */
	name = (char *)PIN_FLIST_FLD_GET(i_flistp,
		PIN_FLD_AAC_ACCESS, 1, ebufp);

	/*
	 * Try to find a plan_list for this name and type
	 */
	fm_cust_pol_get_plans_search(ctxp, planlist_db, name,
		type, o_flistpp, ebufp);

	/*
	 * If unsucessful, try finding the default plan_list
	 */
	if (*o_flistpp == (pin_flist_t *)NULL) {
		name = "default";
		fm_cust_pol_get_plans_search(ctxp, planlist_db,
			name, type, o_flistpp, ebufp);
	}

	/*
	 * What to do if still unsuccessful?
	 */
	if (*o_flistpp == (pin_flist_t *)NULL) {
		*o_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_POID,
			(void *)pdp, ebufp);
	}

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_plans_locate error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_plans_search():
 *******************************************************************/
static void
fm_cust_pol_get_plans_search(
	pcm_context_t	*ctxp,
	int64		database,
	char		*name,
	char		*type,
	pin_flist_t	**o_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_cookie_t	cookie = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	poid_t		*pdp = NULL;
	poid_t		*brand_poidp = NULL;
	int32		cred = 0;

	int32		element_id;
	u_int		uint_val;

	/*
	 * Safety check.
	 */
	if ((name == (char *)NULL) || (type == (char *)NULL)) {
		return;
	}

	/*
	 * Create the search flist
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
	 *
	 */
	s_flistp = PIN_FLIST_CREATE(ebufp);

	pdp = PIN_POID_CREATE(database, "/search", (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	uint_val = SRCH_DISTINCT;
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&uint_val, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, 
		(void *)  "select X from /group/$1 where " 
			  "F1 = V1 and F2 = V2 and F3 = V3 and F4 = V4", ebufp);

	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PARAMETERS, (void *)"plan_list", 
		ebufp); 

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp); 
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, (void *)name, ebufp); 

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp); 
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_TYPE_STR, (void *)type, ebufp); 

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp); 
	pdp = PIN_POID_CREATE(database, "/group/plan_list", (int64)-1, ebufp); 

	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)pdp, ebufp); 
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);

	brand_poidp = cm_fm_perm_get_brand_account(ebufp);
        if(PIN_POID_IS_NULL(brand_poidp)) {
		brand_poidp = fm_utils_lineage_root_poid(database, ebufp);
        } else {
		if (PIN_POID_GET_ID(brand_poidp) == 1) {
			brand_poidp = PIN_POID_CREATE(database,
					PIN_POID_GET_TYPE(brand_poidp), 
					PIN_POID_GET_ID(brand_poidp), ebufp);
		} else {
			brand_poidp = PIN_POID_COPY(brand_poidp, ebufp);
		}
        }

        PIN_FLIST_FLD_PUT(flistp, PIN_FLD_ACCOUNT_OBJ, (void *)brand_poidp, 
			ebufp);


	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	PIN_FLIST_ELEM_SET(flistp, (pin_flist_t *)NULL, PIN_FLD_MEMBERS,
		PIN_ELEMID_ANY, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "op_cust_pol_get_plans search flist", s_flistp);

	/*
	 * Do the search
	 */
        cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
        CM_FM_END_OVERRIDE_SCOPE(cred);

	/*
	 * We want what is in the result element (assume only one)
	 */
	cookie = NULL;
	*o_flistpp = PIN_FLIST_ELEM_TAKE_NEXT(r_flistp, PIN_FLD_RESULTS,
		&element_id, 1, &cookie, ebufp);

	/*
	 * Clean up.
	 */
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_plans_search error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_plans_retrieve()
 *
 *	Constructs the outgoing plan array by reading each plan
 *	on the given list and adding the results to the output
 *	as an element of the array.
 *
 *******************************************************************/
static void
fm_cust_pol_get_plans_retrieve(
	pcm_context_t	*ctxp,
	int64		brand_db,
	pin_flist_t	*i_flistp,
	pin_flist_t	*p_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pin_cookie_t	cookie = NULL;
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*r_flistp = NULL;

	int		element_id;

	void		*vp;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Safety check.
	 */
	if (i_flistp == (pin_flist_t *)NULL) {
		return;
	}

	/*
	 * Prep the output flist.
	 */
	*o_flistpp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(*o_flistpp, PIN_FLD_POID, vp, ebufp);

	/*
	 * Read each member plan and add the results to the out list
	 */
	cookie = NULL;
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(p_flistp, PIN_FLD_MEMBERS,
		 &element_id, 1, &cookie, ebufp)) != (pin_flist_t*) NULL) {

		/*
		 * Read this plan
		 */
		fm_cust_pol_get_plans_read(ctxp, brand_db, flistp, &r_flistp, 
					   ebufp);

		/*
		 * Add it to the output
		 */
		if ( r_flistp ) {
			PIN_FLIST_ELEM_PUT(*o_flistpp, r_flistp, PIN_FLD_PLAN,
					element_id, ebufp);
		}
	}

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_plans_retrieve error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_get_plans_read():
 *******************************************************************/
static void
fm_cust_pol_get_plans_read(
	pcm_context_t	*ctxp,
	int64		brand_db,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pin_flist_t	*flistp = NULL;
	poid_t		*p_plan_pdp = NULL;
	poid_t		*b_plan_pdp = NULL;
	int32       cred;
	int32       override_cred= PIN_BOOLEAN_FALSE;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Create the flist for reading the plan
	 * We should read the plan object from the database in which the
	 * brand lives. (Currently only the root brand can span databases).
	 * This is done by replacing the database number of the plan
	 * poid retrieved from the primary database. Note that this is an
	 * acceptable transformation since all the pricelist objects 
	 * always get created in the main database.
	 */
	flistp = PIN_FLIST_CREATE(ebufp);

	p_plan_pdp = (poid_t*) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_OBJECT, 
						 0, ebufp);

	b_plan_pdp = PIN_POID_CREATE(brand_db, 
				     PIN_POID_GET_TYPE(p_plan_pdp),
				     PIN_POID_GET_ID(p_plan_pdp),
				     ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, b_plan_pdp, ebufp);

	/*
	 * In a multi db system, if the brand_poid and plan_poid are in
	 * different databases, then we need to disable branding due to
	 * a low level design flaw in the brand scoping area.
	 */
    
	if (cm_fm_is_multi_db() ) {
		cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
		override_cred= PIN_BOOLEAN_TRUE;
	}

	/*
	 * Read the plan.
	 */
	PCM_OP(ctxp, PCM_OP_READ_OBJ,
		PCM_OPFLG_ORDER_BY_REC_ID, flistp, o_flistpp, ebufp);
	
	/*
	 * if this was multi-db and the error is it can't find the object poid
	 * then it may not have been replicated yet
	 */
	if ( ebufp->pin_err == PIN_ERR_NOT_FOUND && 
		ebufp->field == PIN_FLD_POID && 
		override_cred == PIN_BOOLEAN_TRUE ) {

		PIN_ERR_CLEAR_ERR(ebufp);
		*o_flistpp = NULL;
	}

	/*
	 * Re-enable branding if necessary.
	 */
	if (override_cred == PIN_BOOLEAN_TRUE) {
		CM_FM_END_OVERRIDE_SCOPE(cred);
		override_cred= PIN_BOOLEAN_FALSE;
	}
   
	/*
	 * Clean up.
	 */
	PIN_FLIST_DESTROY_EX(&flistp, NULL);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_get_plans_read error", ebufp);
	}

	return;
}

