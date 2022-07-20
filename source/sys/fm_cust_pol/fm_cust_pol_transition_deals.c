/*******************************************************************
 *
 *      Copyright (c) 2004-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)%Portal Version: fm_cust_pol_transition_deals.c:BillingVelocityInt:2:2006-Sep-05 04:28:07 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_CUST_POL_TRANSITION_DEALS policy operation.
 *
 * Retrieves a list of deals that are available for transition
 * for the given deal. 
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

void
EXPORT_OP  op_cust_pol_transition_deals(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);


static void
fm_cust_pol_transition_deals(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);


static void
fm_cust_pol_transition_deals_search(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	time_t			end_t,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_TRANSITION_DEALS operation. 
 *******************************************************************/
void
op_cust_pol_transition_deals(
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
	if (opcode != PCM_OP_CUST_POL_TRANSITION_DEALS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_transition_deals", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_transition_deals input flist", i_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_transition_deals(ctxp, i_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Set the results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*o_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY(r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_transition_deals error", ebufp);
	} else {
		*o_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_transition_deals return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_transition_deals()
 *
 *	Retrieves the appropriate plan_list object from the db.
 *	Then performs a read_obj of each plan in the list. The
 *	result is an array of available plans for the relevant
 *	aac method (mecca, admin, etc).
 *
 *******************************************************************/
static void
fm_cust_pol_transition_deals(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
        pin_errbuf_t	*ebufp)
{
	pin_flist_t	*d_flistp = NULL;
	time_t		*tp = NULL;
	time_t		end_t;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

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
	 * Retrieve all deals from the transition for the given deal 
	 */
	fm_cust_pol_transition_deals_search(ctxp, i_flistp, end_t, &d_flistp,
		 ebufp);

	*o_flistpp = d_flistp; 

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_transition_deals error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_transition_deals_search():
 *
 *	Search for all deal in transition object for the given deal
 *	and valid for the given timestamp (deal end_t of 0 = valid).
 *
 *******************************************************************/
static void
fm_cust_pol_transition_deals_search(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	time_t		end_t,
	pin_flist_t	**o_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t     *rf_flistp = NULL;
        pin_flist_t     *rd_flistp = NULL;
        pin_flist_t     *ro_flistp = NULL;
        pin_flist_t     *p_flistp = NULL;
        pin_flist_t     *d_flistp = NULL;
	poid_t		*pdp = NULL;
	poid_t		*d_pdp = NULL;
	poid_t		*b_pdp = NULL;
	poid_t          *p_pdp = NULL;
	
	int64		id = (int64)-1;
	int64		db = 0;
	int32		*trans_typep = NULL;
	int32		s_flags = 0;
	void		*vp = NULL;
	int		rec_id = 0;
	int             rec_id1 = 0;
	int32		cred = 0;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t    cookie1 = NULL;	

	if (PIN_ERR_IS_ERR(ebufp)) 
		return;
	PIN_ERR_CLEAR_ERR(ebufp); 

	d_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	
	db = PIN_POID_GET_DB(d_pdp);
	
	/*********************************************************
	 * Create the search flist		
	 *********************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);

	pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

	/*********************************************************
	 * Set the search template 
	 *********************************************************/
	vp = "select X from /deal 1, /transition 2 where 2.F1 = V1 and 2.F2 = V2 and ( 2.F3 <= V3 or 2.F4 = V4 ) and ( 2.F5 >= V5 or 2.F6 = V6 ) and 2.F7 = 1.F8 and 2.F9 = V9 "; 
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, (void *)vp, ebufp);

	/*********************************************************
	 * Add the argument Deal Poid 
	 *********************************************************/
	t_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_FROM_OBJ, (void *)d_pdp, ebufp);

	/*********************************************************
	 * Add the argument Transition Type 
	 *********************************************************/
	trans_typep = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TRANSITION_TYPE, 0,
		 ebufp);
	t_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_TYPE, (void *)trans_typep, ebufp);
	
	t_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_START_T, (void *)&end_t, ebufp);

	t_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_START_T, (void *)NULL, ebufp);

	t_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_END_T, (void *)&end_t, ebufp);

	t_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_END_T, (void *)NULL, ebufp);

	t_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_TO_OBJ, (void *)NULL, ebufp);

	t_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_POID, (void *)NULL, ebufp);

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
         * deals that do not belong to this brand using the brand owner
         * in the query.
         */
	b_pdp = cm_fm_perm_get_brand_account(ebufp);
	if (PIN_POID_IS_NULL(b_pdp)) {
		b_pdp = fm_utils_lineage_root_poid(db, ebufp);
	} else {
		if (PIN_POID_GET_ID(b_pdp) == 1) {
			b_pdp = PIN_POID_CREATE(db, PIN_POID_GET_TYPE(b_pdp),
				PIN_POID_GET_ID(b_pdp), ebufp);
		} else {
			b_pdp = PIN_POID_COPY(b_pdp, ebufp);
		}
	} 
		
	t_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
	PIN_FLIST_FLD_PUT(t_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)b_pdp, ebufp);

	/*********************************************************
	 * Add the results array for the search 
	 *********************************************************/
	PIN_FLIST_ELEM_SET(s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp); 

	/*
	 * Do the search
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_cust_pol_transition_deals_search search flist", s_flistp);
	cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
        CM_FM_END_OVERRIDE_SCOPE(cred);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_cust_pol_transition_deals_search return flist", r_flistp);

	rd_flistp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(rd_flistp, PIN_FLD_POID, vp, ebufp);

	while (t_flistp = PIN_FLIST_ELEM_TAKE_NEXT(r_flistp, PIN_FLD_RESULTS, 
		&rec_id, 1, &cookie, ebufp)) {
		rec_id1 =0;
                cookie1 = NULL;
                while ((p_flistp = PIN_FLIST_ELEM_GET_NEXT(t_flistp,
                                PIN_FLD_PRODUCTS, &rec_id1, 1, &cookie1,
                                                        ebufp))!= NULL) {
                        p_pdp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_PRODUCT_OBJ,
                                                                 0, ebufp);
                        rf_flistp = PIN_FLIST_CREATE(ebufp);
                        PIN_FLIST_FLD_SET(rf_flistp, PIN_FLD_POID, p_pdp,
                                                                        ebufp);
                        PIN_FLIST_FLD_SET(rf_flistp, PIN_FLD_NAME, NULL, ebufp);
                        PIN_FLIST_FLD_SET(rf_flistp, PIN_FLD_DESCR, NULL,
                                                                        ebufp);
                        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rf_flistp,
                                                        &ro_flistp, ebufp);
                        vp = PIN_FLIST_FLD_GET(ro_flistp, PIN_FLD_NAME, 0,
                                                                        ebufp);
                        PIN_FLIST_FLD_SET(p_flistp, PIN_FLD_NAME, vp, ebufp);
                        vp = PIN_FLIST_FLD_GET(ro_flistp, PIN_FLD_DESCR, 1,
                                                                        ebufp);
                        if (vp) {
                                PIN_FLIST_FLD_SET(p_flistp, PIN_FLD_DESCR, vp,
                                                                        ebufp);
                        }
                        PIN_FLIST_DESTROY_EX(&rf_flistp, NULL);
                        PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
                }

		PIN_FLIST_ELEM_PUT(rd_flistp, t_flistp, PIN_FLD_DEALS, rec_id,
			 ebufp);
	}

	*o_flistpp = rd_flistp;	

	/*
	 * Clean up.
	 */
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);	

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_transition_deals_search error", ebufp);
	}

	return;
}
