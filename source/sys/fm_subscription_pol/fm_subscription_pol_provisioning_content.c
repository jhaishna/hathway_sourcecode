/*******************************************************************
 *
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates.All rights reserved. 
 *      
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_subscription_pol_provisioning_content.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 06:17:31 nishahan Exp $";
#endif

#include <stdio.h>	/* for FILE * in pcm.h */
#include "ops/act.h"
#include "ops/bill.h"
#include "ops/subscription.h"
#include <string.h>
#include "pcm.h"
#include "cm_fm.h"
#include "pin_bill.h"
#include "pin_channel.h"
#include "fm_utils.h"

#define FILE_SOURCE_ID    "fm_subscription_pol_provisioning_content.c(1)"
/********************************************************************
 * contains:
 *	plp_link_to_content_profile
 *
 * DO NOT CHANGE THESE!
 *
 *******************************************************************/
static void
read_config(
	pcm_context_t	*ctxp, 
	const poid_t	*i_poidp, 
	pin_flist_t	**return_flp, 
	pin_errbuf_t	*ebufp
);

/*
 * Update a content service with the new service profile.
 */
void
plp_link_to_content_profile(pcm_context_t *ctxp, poid_t *svc_obj_p, int32 buy,
	char *content_tag, pin_errbuf_t *ebufp)
{
	/*
	 * FLISTs that start with "d_" must be destroyed.
	 */
	char            	*ctag_name = NULL;
	int			opcode; 
	int64			db_id;
	int32			flags = 0;
	int32			mflags = PCM_OPFLG_ADD_ENTRY;
	int32			profile_cnt = 0;
	int32           	elem_id;
	pin_flist_t		*d_flistp = NULL;
	pin_flist_t		*d_mod_flistp = NULL;
	pin_flist_t		*d_ret_mod_flistp = NULL;
	pin_flist_t		*d_ret_rflds_flistp = NULL;
	pin_flist_t     	*d_read_flistp = NULL;
	pin_flist_t     	*entry_flp = NULL;
	pin_flist_t     	*d_r_flistp = NULL;
	pin_flist_t		*profile_flistp = NULL;
	pin_flist_t		*d_config_flistp = NULL;
	pin_flist_t		*fd_flistp = NULL;
	pin_flist_t		*svc_flistp = NULL;
	pin_cookie_t    	cookie = NULL;
	poid_t        		*config_poid         = NULL;
	poid_t        		*profile_poid         = NULL;
	poid_t			*temp_pdp = NULL;
	
	/*
	 * This routine does the following:
	 * On buy:
	 *   Reads the /config/content object to get the poid
	 *   of the service profile corresponding to the tag 
	 *   passed in.
	 *   Inserts this poid into the service objects 
	 *   profile poid array.
	 *   
	 * On cancel:
	 *    Undoes the "buy".
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	if (buy) {
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,
			"plp_link_to_content (BUY) Service Poid=",
			svc_obj_p);
	}
	else {
		PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG,
			"plp_link_to_content (CANCEL) Service Poid=",
			svc_obj_p);
	}

	/**************************************************************** 
	 * read the config object   
	 *****************************************************************/
	db_id = pin_poid_get_db(svc_obj_p);
	config_poid = PIN_POID_CREATE(db_id, "/config/content",
					 -1, ebufp);
	read_config(ctxp, config_poid, &d_config_flistp, ebufp);

	/**************************************************************** 
	 * Walk array of provisioning tags, comparing to our input to
	 * get the poid of the service profile for the given tag 
	 *****************************************************************/

	while ((entry_flp = PIN_FLIST_ELEM_GET_NEXT(d_config_flistp,
		PIN_FLD_PROD_PROVISIONING_TAGS, &elem_id, 1,
		&cookie, ebufp)) != (pin_flist_t *)NULL) {
		
		/* Grab tag in current entry */
		ctag_name = (char *) PIN_FLIST_FLD_GET(entry_flp,
			PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		if (ctag_name == NULL) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_WARNING,
				"NULL tag name in content config "
				"object");
			continue; /* next element */
		}

		/* Compare tag in current entry to input */
		if (strcmp(ctag_name, content_tag) == 0) {
			profile_poid = (poid_t *) PIN_FLIST_FLD_GET(entry_flp,
			PIN_FLD_PROFILE_OBJ, 0, ebufp);
			break;
		}
	} /* while */


	/*******************************************************************
	 * read the service object to get the array of service profiles
	 ********************************************************************/
	d_read_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(d_read_flistp, PIN_FLD_POID, svc_obj_p, ebufp);
	
	fd_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_ELEM_SET(fd_flistp, NULL, PIN_FLD_PROFILE_OBJ,
						PIN_ELEMID_ANY, ebufp);

	PIN_FLIST_ELEM_PUT(d_read_flistp, fd_flistp, PIN_FLD_PROFILES,
				PIN_ELEMID_ANY, ebufp);
	
	PCM_OP(ctxp, PCM_OP_READ_FLDS, flags, d_read_flistp, &d_r_flistp, ebufp);


	/******************************************************************
	 * If this is a "buy", then I need to calculate the correct 
	 * element id (index) into the PIN_FLD_PROFILES array. In case
	 * entries have been deleted, I need to walk the array to find
	 * an empty entry. If none are found, I use the
	 * "current count" + 1.
	 *
	 * If this is not a "buy", then I need to find the matching 
	 * element id (index) in the PIN_FLD_SERVICES array.
	 * This is stored in "profile_cnt"
	 ******************************************************************/
	
	if (buy) {
		opcode = PCM_OP_WRITE_FLDS;
		profile_cnt = 1; /* Default index */
		elem_id = 0;
		cookie = NULL; /* Start at the beginning */
		/* Search for an empty entry */		
		while ((profile_flistp = PIN_FLIST_ELEM_GET_NEXT(
			d_r_flistp, PIN_FLD_PROFILES, &elem_id, 1,
			&cookie, ebufp)) != NULL) {
			
			if (profile_cnt != elem_id) {
				break;  /* Found it! */
			}
			profile_cnt++;
		}
		/*
		 *If we didn't find an emtpy slot, profile_cnt will be pointing
		 * to the next entry.  So we don't have to check for this
		 * condition since it is already what we want. 
		 */
	}
	else {
		opcode = PCM_OP_DELETE_FLDS;
		profile_cnt = 0; /* Default index */
		elem_id = 0;
		cookie = NULL; /* Start at the beginning */
		/* Search for the matching entry */
		while ((profile_flistp = PIN_FLIST_ELEM_GET_NEXT(
			d_r_flistp, PIN_FLD_PROFILES, &elem_id, 1,
			&cookie, ebufp)) != NULL) {

			temp_pdp = (poid_t*) PIN_FLIST_FLD_GET(profile_flistp, 
				PIN_FLD_PROFILE_OBJ, 0, ebufp);

			if (PIN_POID_COMPARE(temp_pdp, profile_poid, 0, 
							ebufp) == 0) {
				profile_cnt = elem_id;
				break;  /* Found it! */
			}
		}
		if (0 == profile_cnt) {
			/* Didn't find it.  Log it */
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				"plp_link_to_content CANCEL did not find "
				"matching service profile POID in PIN_FLD_PROFILES",
				d_r_flistp);
		}
	}
	/*
	 * Now modify the /service/content with the new PIN_FLD_PROFILES array.
	 */
	d_mod_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(d_mod_flistp, PIN_FLD_POID, svc_obj_p, ebufp);
	svc_flistp = PIN_FLIST_ELEM_ADD(d_mod_flistp, PIN_FLD_PROFILES, 
					profile_cnt, ebufp);
	if (buy) {
		PIN_FLIST_FLD_SET(svc_flistp, PIN_FLD_PROFILE_OBJ, profile_poid,
					ebufp);
	}
	/*
	 * Debug: What we're about to modify
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"plp_link_to_content MODIFY PIN_FLD_PROFILES input flist",
		d_mod_flistp);

	PCM_OP(ctxp, opcode, mflags, d_mod_flistp,
	       &d_ret_mod_flistp, ebufp);
	/*
	 * Debug: What we're got back from the modify
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"plp_link_to_content MODIFY PIN_FLD_PROFILES return flist",
		d_ret_mod_flistp);
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"plp_link_to_content: Error detected:", ebufp);
	}

	PIN_POID_DESTROY(config_poid, NULL);
	PIN_FLIST_DESTROY(d_flistp, NULL);
	PIN_FLIST_DESTROY(d_r_flistp, NULL);
	PIN_FLIST_DESTROY(d_mod_flistp, NULL);
	PIN_FLIST_DESTROY(d_ret_mod_flistp, NULL);
	PIN_FLIST_DESTROY(d_ret_rflds_flistp, NULL);
	PIN_FLIST_DESTROY(d_config_flistp, NULL);
	PIN_FLIST_DESTROY(d_read_flistp, NULL);
}

/*******************************************************************
 * read_config()
 * Reads the config object
 * input:
 *   i_poidp: Contains a type-only poid for DB and config obj type
 *
 * output:
 *   return_flp contains the config object
 *   Caller is responsible for cleaning up memory!
 *
 *******************************************************************/
static void
read_config(
	pcm_context_t	*ctxp, 
	const poid_t	*i_poidp, 
	pin_flist_t	**return_flp, 
	pin_errbuf_t	*ebufp) 
{
	pin_flist_t	*search_flp = NULL;
	pin_flist_t	*r_flp = NULL;
	pin_flist_t	*config_flp = NULL;
	pin_flist_t	*args_flp = NULL;
	poid_t		*srchpp = NULL;
	int64           database;
	int32           elem_id;
	pin_cookie_t    cookie;

	/* Sanity check */ 
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, 
		"read_config(): Reading config obj:",
		(poid_t *) i_poidp);

	/*
	 * Create input for search opcode
	 */

	/* Allocate the flist for searching. */
	search_flp = PIN_FLIST_CREATE(ebufp);

	/* Add DB number to search flist */
	database = PIN_POID_GET_DB((poid_t *) i_poidp);
	srchpp = PIN_POID_CREATE(database, "/search", (int64)500, ebufp);
	PIN_FLIST_FLD_PUT(search_flp, PIN_FLD_POID, (void *)srchpp, ebufp);

	/* Tell search we're looking for the requested type */
	args_flp = PIN_FLIST_ELEM_ADD(search_flp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flp, PIN_FLD_POID, (void *)i_poidp, ebufp);

	/* Tell search to go ahead and grab everything it finds */
	PIN_FLIST_ELEM_PUT(search_flp, (void*)NULL, PIN_FLD_RESULTS, 0, ebufp); 

	/* log the search input flist */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			  "read_config(): Search Flist", 
			  search_flp);

	/*
	 * Do the search
	 */

	/* Call search with constructed input */
	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_READ_UNCOMMITTED, 
		search_flp, &r_flp, ebufp);

	/* Clean up search input */
	PIN_FLIST_DESTROY(search_flp, NULL); 

	/* Log result */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			  "read_config(): "
			  "Search Return", r_flp);

	/*
	 * Process search return
	 */

	/* Grab config object (should only be one) */
	/* (Note the take: caller responsible for memory) */
	cookie = (pin_cookie_t)NULL;
	config_flp = PIN_FLIST_ELEM_TAKE_NEXT(r_flp, PIN_FLD_RESULTS,
					   &elem_id, 0, &cookie, ebufp);

	/* Clean up search output */
	PIN_FLIST_DESTROY(r_flp, NULL); 

	/*
	 * Set return value & exit
	 */

	/* Were we able to find the object? */
	if (!PIN_ERR_IS_ERR(ebufp)) {
		/* Yes: return config object */
		*return_flp = config_flp;
	} else {
		/* No: log unfound object & set return to null */
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,
				 "read_config(): "
				 "Unable to find requested config object "
				 "in DB", 
				 ebufp);
		PIN_FLIST_DESTROY(config_flp, NULL);
		*return_flp = NULL;
	}

	/* Log return value */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			  "read_config(): "
			  "Return Flist", *return_flp);

	return;
}	
