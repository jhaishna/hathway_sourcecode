/*******************************************************************
 *
 * Copyright (c) 2000, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/
#ifndef lint
static  char    Sccs_id[] = "@(#)$full_filespec: fm_cust_pol_set_brandinfo.c~12 $";
#endif

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "pin_cust.h"

#define TEMPLATE_UNIQUE_UNDER_BRAND "select X from /account 1, /group/billing 2 where 1.F1 = V1 AND 1.F2 = V2 AND 1.F3 = 2.F4 AND 2.F5 =  V5 AND 1.F6 != V6 "

#define TEMPLATE_UNIQUE_UNDER_BRANDHOST "select X from /account 1, /group 2 where 1.F1 = V1 AND 1.F2 = V2 AND 1.F3 = 2.F4  AND 1.F5 not like V5 AND 1.F6 != V6 "

EXPORT_OP void
op_cust_pol_set_brandinfo(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			opflags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_set_brandinfo(
	pcm_context_t		*ctxp,
	int32			opflags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_set_brandname(
	pcm_context_t		*ctxp,
	int32			opflags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_is_brandname_unique_global(
	pcm_context_t		*ctxp,
	poid_t			*poidp,
	char			*namep,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_is_brandname_unique_within_brand(
	pcm_context_t		*ctxp,
	poid_t			*poidp,
	char			*namep,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_read_err_return_list(
	poid_t			*poidp,
	char			*namep,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
extern pin_flist_t *
fm_cust_pol_valid_add_fail(pin_flist_t	*r_flistp,
			   int	field,
			   int	elemid,
			   int	result,
			   char		*descr,
			   void		*val,
			   pin_errbuf_t	*ebufp);

/*
 * Enum variable controlling whether or not brand names should be unique
 * throughout the system or under a parent brand. Eventually to be
 * controlled through a configuration object...
 */
typedef enum { NONE, SYSTEMWIDE_UNIQUE, BRANDWIDE_UNIQUE }
	brandName_Option_t;
/*   
 * The default is brand names are unique within a parent brand.
 */
static brandName_Option_t check_unique = BRANDWIDE_UNIQUE;

void
op_cust_pol_set_brandinfo(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			opflags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
        pcm_context_t           *ctxp = connp->dm_ctx;

	/*
	 * If there are pending errors, then short circuit immediately
	 */
        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        /*
         * Null out results until we're sure there's something to send back
         */
        *r_flistpp = NULL;

        /*
         * Sanity check -- make sure we've routed the opcode appropriately
         */
        if (opcode != PCM_OP_CUST_POL_SET_BRANDINFO) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_BAD_OPCODE, 0, 0, opcode);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "bad opcode in op_cust_pol_set_brandinfo", ebufp);
                return;
        }

	/*
	 * For completeness, dump the input flist if debugging is enabled.
	 */
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                          "op_cust_pol_set_brandinfo input flist:",
                          i_flistp);

	/*
 	 * Call the main function -- this is where all the real work is done.
	 */
	fm_cust_pol_set_brandinfo(ctxp, opflags, i_flistp, r_flistpp, ebufp);

	/*
	 * For completeness, dump the results flist if debugging is enabled
	 * and we actually have a results flist.
	 */
	if(*r_flistpp != (pin_flist_t *)NULL) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_set_brandinfo output flist:",
			*r_flistpp);
	}

	return;
}

static void
fm_cust_pol_set_brandinfo(
	pcm_context_t		*ctxp,
	int32			opflags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	/*
	 * If there are pending errors, then short circuit immediately
	 */
        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

	/* Attempt to set the brand name */
	fm_cust_pol_set_brandname(ctxp, opflags, i_flistp, r_flistpp, ebufp);

	/* If there's no pending errors, then just use what we got */
	if(!PIN_ERR_IS_ERR(ebufp)) {
		*r_flistpp = PIN_FLIST_COPY(i_flistp, ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_set_brandname()
 *
 * If the brand name is changed here in the policy code, we
 * have to similar changes in the policy code for setting
 * the gl_segment name for the brand in fm_cust_pol_prep_actginfo.c
 * since the brandname is used in the gl_segment name.
 *
 *******************************************************************/
/*ARGSUSED*/
static void
fm_cust_pol_set_brandname(
	pcm_context_t		*ctxp,
	int32			opflags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	char			*namep;
	poid_t			*poidp;
        poid_t                  *g_pdp;
	pin_flist_t		*sublistp;

	/* 
	 * For pending errors, return immediately.
	 */
	if(PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * For completeness, dump the input flist if debugging is enabled.
	 */
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                          "fm_cust_pol_set_brandname input flist:",
                          i_flistp);

	/*
	 * Get the name if there is one 
	 */
	poidp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	sublistp = (pin_flist_t *)PIN_FLIST_FLD_GET(i_flistp, 
		PIN_FLD_BRAND_INFO, 0, ebufp);
	namep = (char *)PIN_FLIST_FLD_GET(sublistp, PIN_FLD_NAME, 1, ebufp);
	if((namep == (char *)NULL) || (*namep == '\0')) {
		return;
	}	

	if(check_unique == BRANDWIDE_UNIQUE) {
		fm_cust_pol_is_brandname_unique_within_brand(ctxp, poidp,
			namep, r_flistpp, ebufp);
	}
	else if(check_unique == SYSTEMWIDE_UNIQUE) {
		fm_cust_pol_is_brandname_unique_global(ctxp, poidp, namep,
			r_flistpp, ebufp);
	}
}

/*
 * Search for a brand name within a brand.
 */
static void
fm_cust_pol_is_brandname_unique_within_brand(
	pcm_context_t		*ctxp,
	poid_t			*poidp,
	char			*namep,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*flistp = (pin_flist_t *)NULL;
	pin_flist_t		*rlistp = (pin_flist_t *)NULL;
	pin_flist_t		*sublistp;
	poid_t			*search_poidp;
	int32			int_val;
	int32			cred;
	pin_account_type_t	brand_type = PIN_ACCOUNT_TYPE_BRAND;
	pin_cookie_t		cookie = (pin_cookie_t)NULL;
	pin_flist_t             *b_flistp = NULL;
	pin_flist_t             *t_flistp = NULL;
	poid_t                  *b_pdp = NULL;
	poid_t                  *g_pdp = NULL;
	poid_t                  *rootp = NULL;
	int32                   elemid = 0;
	int			under_host = PIN_BOOLEAN_FALSE;
	void			*vp = NULL;

	/*
	 * If there are pending errors, then short circuit immediately
	 */
        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

	fm_utils_lineage_get_brand_hierarchy(ctxp, poidp, &b_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_is_brandname_unique_within_brand() : "
			"Call to get brand hierarchy failed ",
			ebufp);
	 	PIN_FLIST_DESTROY_EX (&b_flistp, NULL);
		fm_cust_pol_read_err_return_list(poidp, namep, r_flistpp,
			ebufp);
                goto cleanup;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Got brand hierarchy.  Return flist :", b_flistp);

	t_flistp = PIN_FLIST_ELEM_GET_NEXT(b_flistp,
		PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp);
	if (PIN_ERR_IS_ERR(ebufp) || t_flistp == (pin_flist_t *)NULL) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_is_brandname_unique_within_brand() : "
			"Call to get first brand failed ",
			ebufp);
		PIN_FLIST_DESTROY_EX (&b_flistp, NULL);
		fm_cust_pol_read_err_return_list(poidp, namep, r_flistpp,
			ebufp);
                goto cleanup;
	}
	
	/********************************************
	 * Get the first brand object from hierarchy
	 ********************************************/
	b_pdp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_BRAND_OBJ, 0, ebufp);

	/***************************************************************
	 * During the brand creation, the first brand object from the
	 * the hierarchy is the parent brand. But, for an existing 
	 * brand, the first object is self, the next one in hierarchy 
	 * is the parent brand.
	 ***************************************************************/
	if (!PIN_POID_COMPARE(b_pdp, poidp, 0, ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "first brand is self");
		t_flistp = PIN_FLIST_ELEM_GET_NEXT(b_flistp,
			PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp);
		if (PIN_ERR_IS_ERR(ebufp) || t_flistp == (pin_flist_t *)NULL) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_is_brandname_unique_within_brand() : "
			"Call to get second brand failed ",
			ebufp);
			PIN_FLIST_DESTROY_EX (&b_flistp, NULL);
			fm_cust_pol_read_err_return_list(poidp, namep, 	
				r_flistpp, ebufp);
                	goto cleanup;
		}
	 	/* Get the parent brand object */
		b_pdp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_BRAND_OBJ, 
			0, ebufp);
	}
	else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "first brand is parent");
	}

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "parent brand = ", b_pdp);

	/***************************************************************
	 * Check to see if we are creating brand under brandhost
	 ***************************************************************/
	rootp = PIN_POID_CREATE(PIN_POID_GET_DB(poidp), "/account", 1, ebufp);
	if (!PIN_POID_COMPARE(b_pdp, rootp, 0, ebufp)) {
		under_host = PIN_BOOLEAN_TRUE;  /* creating brand under 
						   brandhost */
	}
	else {	/* creating sub-brand under a brand */
		/* get the group_obj_id0 for the parent brand */
		cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT); 
		g_pdp = fm_utils_lineage_get_billing_group_by_parent(ctxp, 
			b_pdp, ebufp);
		CM_FM_END_OVERRIDE_SCOPE(cred); 
		if (PIN_ERR_IS_ERR(ebufp) || g_pdp == (poid_t*)NULL) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_is_brandname_unique_within_brand() : "
			"Call to get parent group failed ",
			ebufp);
			PIN_POID_DESTROY(rootp, NULL);
			PIN_FLIST_DESTROY_EX (&b_flistp, NULL);
			fm_cust_pol_read_err_return_list(poidp, namep, 
				r_flistpp, ebufp);
                	goto cleanup;
		}
	}
	PIN_POID_DESTROY(rootp, NULL);
	PIN_FLIST_DESTROY_EX (&b_flistp, NULL);
	
	/*
	 * Create search flist to try to figure out if someother brand already
	 * has this name within the parent brand.
	 */
	flistp = PIN_FLIST_CREATE(ebufp);
	search_poidp = PIN_POID_CREATE(PIN_POID_GET_DB(poidp), "/search", -1,
		ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)search_poidp, ebufp);
	int_val = SRCH_DISTINCT;
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_FLAGS, (void *)&int_val, ebufp);

	/***************************************************************
	 * We need different search templates for checking duplicate
	 * brandnames under the brandhost or another brand.
	 * The brandhost doesnot have an entry in the /group/billing
	 * storable class. The only way we can get the names of the 
	 * existing brands under the brandhost is to check the lineage
	 * tag. Arguments 1 to 3 are the same for both search templates,
	 * but arguments 4 & 5 are different. 
	 ***************************************************************/
	if (under_host == PIN_BOOLEAN_FALSE) {  
		/* creating sub-brands under a brand */
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TEMPLATE, 
			(void *)TEMPLATE_UNIQUE_UNDER_BRAND, ebufp);
	}
	else {                   /* creating brand under brandhost */
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_TEMPLATE, 
			(void *)TEMPLATE_UNIQUE_UNDER_BRANDHOST, ebufp);
	}
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	/*
	 * Arg 1 is the type of the account; only interested in brand accounts
	 */
	sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(sublistp, PIN_FLD_ACCOUNT_TYPE, (void *)&brand_type,
		ebufp);

	/*
	 * Arg 2 is the name of the brand.
	 */
	sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(sublistp, PIN_FLD_NAME, (void *)namep, ebufp);

	/*
	 * Arg 3 -- POID of account
	 */
	sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_PUT(sublistp, PIN_FLD_POID, NULL, ebufp);

	if (under_host == PIN_BOOLEAN_FALSE) {  
		/* creating sub-brands under a brand */
		/*
	 	 * Arg 4 -- OBJECT_ID of members from /group/billing 
	 	 */
		sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 4, ebufp);
		sublistp = PIN_FLIST_ELEM_ADD(sublistp, PIN_FLD_MEMBERS, 
			PIN_ELEMID_ANY, ebufp);
        	PIN_FLIST_FLD_PUT(sublistp, PIN_FLD_OBJECT, (void *)NULL, 
			ebufp);

		/*
	 	 * Arg 5 -- POID of the group
	 	 */
		sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(sublistp, PIN_FLD_POID, (void *)g_pdp, ebufp);
	}
	else {                   /* creating brand under brandhost */
		/*
	 	 * Arg 4 -- Parent of the group
	 	 */
		sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_PUT(sublistp, PIN_FLD_PARENT, NULL, ebufp);

		/*
	 	 * Arg 5 -- LINEAGE of account
	 	 */
		/* accounts under brandhost have only one ":" in their
		   lineage tag. We want to exclude others. */

		vp = (void *)"%:%:%";	
		sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(sublistp, PIN_FLD_LINEAGE, vp, ebufp);
	}

        /*
         * Arg 6 -- POID of current account
         */
        sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 6, ebufp);
        PIN_FLIST_FLD_SET(sublistp, PIN_FLD_POID, poidp, ebufp);

	/*
	 * For completeness, dump the input flist if debugging is enabled.
	 */
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                          "fm_cust_pol_is_brandname_unique_within_brand search input flist:",
                          flistp);

	/*
	 * Ready to search! Temporarily suspend search to ensure that
	 * all brands are traversed.
	 */
	cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
	PCM_OP(ctxp, PCM_OP_GLOBAL_SEARCH, PCM_OPFLG_COUNT_ONLY, 
		flistp, &rlistp, ebufp);
	CM_FM_END_OVERRIDE_SCOPE(cred);

	/*
	 * For completeness, dump the results flist if debugging is enabled
	 * and we actually have a results flist.
	 */
	if(rlistp != (pin_flist_t *)NULL) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"fm_cust_pol_is_brandname_unique_within_brand search output flist:",
			rlistp);
	}

	/* 
	 * See if there is a brand out there right now that already has
	 * this name. If so, flag it as an error.
	 */
	cookie = (pin_cookie_t)NULL;
	int_val = 0;
	PIN_FLIST_ELEM_GET_NEXT(rlistp, PIN_FLD_RESULTS, &int_val, 1, 
		&cookie, ebufp);
	if(int_val > 0) {
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, (void *)poidp, 
			ebufp);
		fm_cust_pol_valid_add_fail(*r_flistpp, 
			PIN_FLD_NAME,
			0,
			PIN_CUST_VAL_ERR_DUPE,
			PIN_CUST_DUPE_VALUE_ERR_MSG,
			(void *)namep, ebufp);

		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_DUPLICATE, PIN_FLD_NAME, 
			0, 0);
	}

cleanup:
	/*
	 * Cleanup and return
	 */
	PIN_FLIST_DESTROY_EX(&flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rlistp, NULL);
	PIN_POID_DESTROY(g_pdp, NULL);
}

/*
 * build the return list in case of a read error for func 
 * fm_cust_pol_is_brandname_unique_within_brand().
 */
static void
fm_cust_pol_read_err_return_list(
	poid_t			*poidp,
	char			*namep,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, (void *)poidp, 
		ebufp);
	fm_cust_pol_valid_add_fail(*r_flistpp, 
		PIN_FLD_NAME,
		0,
		PIN_CUST_VAL_ERR_MISSING,
		PIN_CUST_BAD_VALUE_ERR_MSG,
		(void *)namep, ebufp);

	pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_BAD_READ, PIN_FLD_NAME, 
		0, 0);
}


/*
 * Search for a brand name in the whole system.
 */
static void
fm_cust_pol_is_brandname_unique_global(
	pcm_context_t		*ctxp,
	poid_t			*poidp,
	char			*namep,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*flistp = (pin_flist_t *)NULL;
	pin_flist_t		*rlistp = (pin_flist_t *)NULL;
	pin_flist_t		*sublistp;
	poid_t			*search_poidp;
	int32			int_val;
	int32			cred;
	pin_account_type_t	brand_type = PIN_ACCOUNT_TYPE_BRAND;
	pin_cookie_t		cookie = (pin_cookie_t)NULL;

	/*
	 * If there are pending errors, then short circuit immediately
	 */
        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Create search flist to try to figure out if someone brand already
	 * has this name.
	 *
	 * NOTE: we really only need to search for accounts that are of type
	 * 'brand' and of a particular name. Unfortunately, neither of these
	 * fields are indexed by default, thus causing a performance problem.
	 *
	 * To get around this, we'll turn this into a complex search. We know
	 * that brands are the parent of a /group/billing object, so glue that
	 * in as well. That should be indexed...
	 */
	flistp = PIN_FLIST_CREATE(ebufp);
	search_poidp = PIN_POID_CREATE(PIN_POID_GET_DB(poidp), "/search", -1,
		ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)search_poidp, ebufp);
	int_val = SRCH_DISTINCT;
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_FLAGS, (void *)&int_val, ebufp);

	PIN_FLIST_FLD_SET(flistp, PIN_FLD_TEMPLATE, 
		(void *)"select X from /account 1, /group 2 where 1.F1 = V1 "
		"AND 1.F2 = V2 AND 1.F3 = 2.F4 AND 1.F5 != V5 ",
		ebufp);
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	/*
	 * Arg 1 is the type of the account; only interested in brand accounts
	 */
	sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(sublistp, PIN_FLD_ACCOUNT_TYPE, (void *)&brand_type,
		ebufp);

	/*
	 * Arg 2 is the name of the brand.
	 */
	sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(sublistp, PIN_FLD_NAME, (void *)namep, ebufp);

	/*
	 * Arg 3 -- POID of account
	 */
	sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_PUT(sublistp, PIN_FLD_POID, NULL, ebufp);

	/*
	 * Arg 4 -- Parent of the group
	 */
	sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_PUT(sublistp, PIN_FLD_PARENT, NULL, ebufp);

        /*
         * Arg 5 -- POID of current account
         */
        sublistp = PIN_FLIST_ELEM_ADD(flistp, PIN_FLD_ARGS, 5, ebufp);
        PIN_FLIST_FLD_SET(sublistp, PIN_FLD_POID, poidp, ebufp);

	/*
	 * Ready to search! Temporarily suspend search to ensure that
	 * all brands are traversed.
	 */
	cred = CM_FM_BEGIN_OVERRIDE_SCOPE(CM_CRED_SCOPE_OVERRIDE_ROOT);
	PCM_OP(ctxp, PCM_OP_GLOBAL_SEARCH, PCM_OPFLG_COUNT_ONLY, 
		flistp, &rlistp, ebufp);
	CM_FM_END_OVERRIDE_SCOPE(cred);

	/* 
	 * See if there is a brand out there right now that already has
	 * this name. If so, flag it as an error.
	 */
	PIN_FLIST_ELEM_GET_NEXT(rlistp, PIN_FLD_RESULTS, &int_val, 1, 
		&cookie, ebufp);
	if(int_val > 0) {
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, (void *)poidp, 
			ebufp);
		fm_cust_pol_valid_add_fail(*r_flistpp, 
			PIN_FLD_NAME,
			0,
			PIN_CUST_VAL_ERR_DUPE,
			PIN_CUST_DUPE_VALUE_ERR_MSG,
			(void *)namep, ebufp);

		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_DUPLICATE, PIN_FLD_NAME, 
			0, 0);
	}

	/*
	 * Cleanup and return
	 */
	PIN_FLIST_DESTROY_EX(&flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rlistp, NULL);
}
