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
static  char    Sccs_id[] = "@(#)%Portal Version: fm_bill_pol_provisioning_ssg.c:BillingVelocityInt:2:2006-Sep-05 21:55:23 %";
#endif

#include <stdio.h>	/* for FILE * in pcm.h */
#include "ops/act.h"
#include "ops/bill.h"
#include <string.h>
#include "pcm.h"
#include "cm_fm.h"
#include "pin_bill.h"
#include "pin_channel.h"
#include "fm_utils.h"


/********************************************************************
 * contains:
 *	plp_link_to_service_profile
 *	plp_link_to_ssg
 *
 * DO NOT CHANGE THESE!
 *
 *******************************************************************/


/*
 * Update the service to point to the service profile.
 */
void
plp_link_to_service_profile(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *svc_profile_type, char *svc_profile_login, pin_errbuf_t *ebufp)
{
	/*
	 * FLISTs that start with "d_" must be destroyed.
	 */
	pin_flist_t		*d_find_flistp = NULL;
	pin_flist_t		*d_flistp = NULL;
	pin_flist_t		*d_ret_find_flistp = NULL;
	pin_flist_t		*d_ret_wflds_flistp = NULL;
	poid_t			*svc_pdp = NULL;
	poid_t			*svc_profile_pdp = NULL;
	char			service_type[40] = "/service/";
	int64			db_id;
	void			*vp;
	
	/*
	 * This routine does the following:
	 * On buy:
	 *   Modifies the passed service to point to the service
	 *   profile via PIN_FLD_PROFILE_OBJ.
	 *
	 * On cancel:
	 *   Modifies the passed service's PIN_FLD_PROFILE_OBJ 
	 *   to contain a NULL POID.
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	if (buy) {
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,
			"plp_link_to_service_profile (BUY) Service Poid=",
			svc_obj_p);
		svc_pdp = svc_obj_p;
	}
	else {
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,
			"plp_link_to_service_profile (CANCEL) Service Poid=",
			svc_obj_p);
		svc_pdp = NULL;
	}
	/*
	 * Now let's modify the passed service to point to the service
	 * profile via PIN_FLD_PROFILE_OBJ.
	 *
	 * Find the POID for the service profile.
	 */
	
	db_id = PIN_POID_GET_DB(svc_obj_p);
	d_find_flistp = PIN_FLIST_CREATE(ebufp);
	/* PIN_FLD_LOGIN */
	PIN_FLIST_FLD_SET(d_find_flistp, PIN_FLD_LOGIN, svc_profile_login, 
		ebufp);

	strcat(service_type, svc_profile_type);
	svc_pdp = PIN_POID_CREATE(db_id, service_type, -1, ebufp);
	/* PIN_FLD_POID */
	PIN_FLIST_FLD_PUT(d_find_flistp, PIN_FLD_POID, svc_pdp, ebufp);
	/*
	 * Call PCM_OP_ACT_FIND to do the real work.
	 */
	PCM_OP(ctxp, PCM_OP_ACT_FIND, 0, d_find_flistp, &d_ret_find_flistp,
		ebufp);
		
	/***********************************************************
	 * What did we find?
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(d_ret_find_flistp, PIN_FLD_ACCOUNT_OBJ,
		0, ebufp);
	if (vp == (void *)NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_APPLICATION, PIN_ERR_BAD_ARG,
			PIN_FLD_PROVISIONING_TAG, 0, 0);

	}

	svc_profile_pdp = PIN_FLIST_FLD_GET(d_ret_find_flistp, PIN_FLD_POID,
		0, ebufp);

	d_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_POID, svc_obj_p, ebufp);

	if (!buy) {
		svc_profile_pdp = PIN_POID_FROM_STR("0.0.0.0  0 0", NULL,
			ebufp);
	}
	/*
	 * Now set PIN_FLD_PROFILE_OBJ
	 */
	PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_PROFILE_OBJ,
	    (void *)svc_profile_pdp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
	    "plp_link_to_service_profile WFLDS flist", d_flistp);

	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, d_flistp, &d_ret_wflds_flistp,
		ebufp);
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"plp_link_to_service_profile: Error detected:", ebufp);
	}

	PIN_FLIST_DESTROY(d_find_flistp, NULL);
	PIN_FLIST_DESTROY(d_flistp, NULL);
	PIN_FLIST_DESTROY(d_ret_find_flistp, NULL);
	PIN_FLIST_DESTROY(d_ret_wflds_flistp, NULL);

	if (!buy) {
		PIN_POID_DESTROY(svc_profile_pdp, NULL);
	}
}

/*
 * Update a ssg service with the new secondary service.
 */
void
plp_link_to_ssg(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *ssg_svc_name, pin_errbuf_t *ebufp)
{
	/*
	 * FLISTs that start with "d_" must be destroyed.
	 */
	pin_flist_t		*d_flistp = NULL;
	pin_flist_t		*d_mod_flistp = NULL;
	pin_flist_t		*d_ret_mod_flistp = NULL;
	pin_flist_t		*d_ret_rflds_flistp = NULL;
	pin_flist_t		*d_ret_srch_flistp = NULL;
	pin_flist_t		*d_srch_flistp = NULL;
	pin_flist_t		*svc_flistp = NULL;
	pin_flist_t		*t_flistp = NULL;
	poid_t			*a_pdp = NULL;
	poid_t 			*srch_pdp = NULL;
	poid_t			*ssg_svc_pdp = NULL;
	poid_t			*svc_pdp = NULL;
	poid_t			*temp_pdp = NULL;
	int32			mflags = PCM_OPFLG_ADD_ENTRY;
	int32			rflags = 0;
	int32			sflags = SRCH_DISTINCT;
	void			*vp = NULL;
	int32			elemid = 0;
	int32			svc_cnt = 0;
	pin_cookie_t		cookie = NULL;
	char			service_type[40] = "/service/";
	int64			db_id;
	int			opcode;
	
	/*
	 * This routine does the following:
	 * On buy:
	 *   Reads the passed in service object to get the account object.
	 *   Find the account's /service/ssg object.
	 *   Count the number of elements in the PIN_FLD_SERVICES array
	 *   Add the passed in service to the PIN_FLD_SERVICE array of
	 *   the /service/ssg object.
	 *
	 * On cancel:
	 *   Undoes the "buy".
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	if (buy) {
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,
			"plp_link_to_ssg (BUY) Service Poid=",
			svc_obj_p);
		svc_pdp = svc_obj_p;
	}
	else {
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,
			"plp_link_to_ssg (CANCEL) Service Poid=",
			svc_obj_p);
		svc_pdp = NULL;
	}

	/******************************************************************
	 * Read the service object to get the account POID
	 ******************************************************************/
	
	db_id = PIN_POID_GET_DB(svc_obj_p);
	d_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_POID, (void *)svc_obj_p, ebufp);
	PIN_FLIST_FLD_SET(d_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"plp_link_to_ssg READ_FLDS input flist", d_flistp);

	PCM_OP(ctxp, PCM_OP_READ_FLDS, rflags, d_flistp, &d_ret_rflds_flistp,
		ebufp);

	PIN_FLIST_DESTROY(d_flistp, NULL);
	d_flistp = NULL;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"plp_link_to_ssg READ_FLDS output flist",
		d_ret_rflds_flistp);
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(d_ret_rflds_flistp,
			PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

	/******************************************************************
	 * Create the search flist to read the service object.
	 ******************************************************************/
	d_srch_flistp = PIN_FLIST_CREATE(ebufp);

	srch_pdp = PIN_POID_CREATE(db_id, "/search/pin", -1, ebufp);

	PIN_FLIST_FLD_PUT(d_srch_flistp, PIN_FLD_POID, (void *)srch_pdp,
		ebufp);
	PIN_FLIST_FLD_SET(d_srch_flistp, PIN_FLD_FLAGS, (void *)&sflags,
		ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"plp_link_to_ssg partial 1 s_flist", d_srch_flistp);

	/******************************************************************
	 * Set the search template.
	 ******************************************************************/
	vp = (void *)"select X from /service/ssg where F1 = V1 and F2 = V2 ";
	PIN_FLIST_FLD_SET(d_srch_flistp, PIN_FLD_TEMPLATE, vp, ebufp);

	/******************************************************************
	 * 1st arg. Matching Acct obj.
	 ******************************************************************/
	t_flistp = PIN_FLIST_ELEM_ADD(d_srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);

	/******************************************************************
	 * 2nd arg. Matching Name.
	 ******************************************************************/
	t_flistp = PIN_FLIST_ELEM_ADD(d_srch_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(t_flistp, PIN_FLD_NAME, ssg_svc_name, ebufp);

	/******************************************************************
	 * Add the RESULTS array.
	 * Return the entire object including the PIN_FLD_SERVICES
	 ******************************************************************/
	PIN_FLIST_ELEM_PUT(d_srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);
	/*
	 * Debug: What we're searching for
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"plp_link_to_ssg search input flist", d_srch_flistp);

	/******************************************************************
	 * Do the actual search.
	 ******************************************************************/
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, d_srch_flistp, &d_ret_srch_flistp,
		ebufp);
	/*
	 * Debug: What we've found
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"plp_link_to_ssg search return flist",
		d_ret_srch_flistp);

	/******************************************************************
	 * Get the /service/ssg PIN_FLD_SERVICES.
	 ******************************************************************/

	t_flistp = PIN_FLIST_ELEM_GET_NEXT(d_ret_srch_flistp, PIN_FLD_RESULTS, 
		&elemid, 1, &cookie, ebufp);

	ssg_svc_pdp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_POID, 0,
		ebufp);

	/******************************************************************
	 * If this is a "buy", then I need to calculate the correct 
	 * element id (index) into the PIN_FLD_SERVICES array. In case
	 * entries have been deleted, I need to walk the array to find
	 * an empty entry. If none are found, I use the
	 * "current count" + 1.
	 *
	 * If this is not a "buy", then I need to find the matching 
	 * element id (index) in the PIN_FLD_SERVICES array.
	 * This is stored in "svc_cnt"
	 ******************************************************************/
	
	if (buy) {
		opcode = PCM_OP_WRITE_FLDS;
		svc_cnt = 1; /* Default index */
		elemid = 0;
		cookie = NULL; /* Start at the beginning */
		/* Search for an empty entry */		
		while ((svc_flistp = PIN_FLIST_ELEM_GET_NEXT(
			t_flistp, PIN_FLD_SERVICES, &elemid, 1,
			&cookie, ebufp)) != NULL) {
			
			if (svc_cnt != elemid) {
				break;  /* Found it! */
			}
			svc_cnt++;
		}
		/*
		 *If we didn't find an emtpy slot, svc_cnt will be pointing
		 * to the next entry.  So we don't have to check for this
		 * condition since it is already what we want. 
		 */
	}
	else {
		opcode = PCM_OP_DELETE_FLDS;
		svc_cnt = 0; /* Default index */
		elemid = 0;
		cookie = NULL; /* Start at the beginning */
		/* Search for the matching entry */
		while ((svc_flistp = PIN_FLIST_ELEM_GET_NEXT(
			t_flistp, PIN_FLD_SERVICES, &elemid, 1,
			&cookie, ebufp)) != NULL) {

			temp_pdp = (poid_t*) PIN_FLIST_FLD_GET(svc_flistp, 
				PIN_FLD_OBJECT, 0, ebufp);

			if (PIN_POID_COMPARE(temp_pdp, svc_obj_p, 0, 
							ebufp) == 0) {
				svc_cnt = elemid;
				break;  /* Found it! */
			}
		}
		if (0 == svc_cnt) {
			/* Didn't find it.  Log it */
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"plp_link_to_ssg CANCEL did not find "
				"matching service POID in PIN_FLD_SERVICES",
				svc_flistp);
		}
	}
	/*
	 * Now modify the /service/ssg with the new PIN_FLD_SERVICES array.
	 */
	d_mod_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(d_mod_flistp, PIN_FLD_POID, ssg_svc_pdp, ebufp);
	svc_flistp = PIN_FLIST_ELEM_ADD(d_mod_flistp, PIN_FLD_SERVICES, 
					svc_cnt, ebufp);
	if (buy) {
		PIN_FLIST_FLD_SET(svc_flistp, PIN_FLD_OBJECT, svc_pdp,
			ebufp);
	}
	/*
	 * Debug: What we're about to modify
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"plp_link_to_ssg MODIFY PIN_FLD_SERVICES input flist",
		d_mod_flistp);

	PCM_OP(ctxp, opcode, mflags, d_mod_flistp,
	       &d_ret_mod_flistp, ebufp);
	/*
	 * Debug: What we're got back from the modify
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"plp_link_to_ssg MODIFY PIN_FLD_SERVICES return flist",
		d_ret_mod_flistp);
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"plp_link_to_ssg: Error detected:", ebufp);
	}

	PIN_FLIST_DESTROY(d_flistp, NULL);
	PIN_FLIST_DESTROY(d_mod_flistp, NULL);
	PIN_FLIST_DESTROY(d_ret_mod_flistp, NULL);
	PIN_FLIST_DESTROY(d_ret_rflds_flistp, NULL);
	PIN_FLIST_DESTROY(d_ret_srch_flistp, NULL);
	PIN_FLIST_DESTROY(d_srch_flistp, NULL);
}
