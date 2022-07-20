/********************************************************************
 *
* Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_inv_pol_init.c:RWSmod7.3.1Int:1:2007-Aug-01 23:18:05 %";
#endif

/*******************************************************************
 * This file contains the fm_inv_pol_init() function which is called
 * during CM initialization.  This function caches the 
 * /config/invoice_templates object poids in a cm_cache for later
 * (faster) retrieval
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>

 
#include "pcm.h"
#include "ops/inv.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#include "pin_inv.h"
#include "cm_cache.h"
#include "mso_ops_flds.h"

#define FILE_SOURCE_ID          "fm_inv_pol_init.c(8)"

/*******************************************************************
 * Global for holding cache ptr
 *******************************************************************/
PIN_EXPORT cm_cache_t *fm_inv_pol_config_cache_ptr = (cm_cache_t *)0;
int fm_inv_pol_show_rerate_details = 0;
int fm_inv_pol_email_enabled = 0;
PIN_EXPORT int32 fm_inv_pol_service_enabled = 0;
int32 perf_features_flags = 0;

PIN_EXPORT void fm_inv_pol_init(
	int32			*errp);

PIN_EXPORT cm_cache_t *fm_inv_pol_city_adr_map_ptr = (cm_cache_t *)0;

PIN_EXPORT cm_cache_t *fm_inv_pol_state_code_ptr = (cm_cache_t *)0;
/***********************************************************************
 *Forward declaration
 ***********************************************************************/
static void
fm_inv_pol_init_cache_citywise_address(
	pcm_context_t	*ctxp,
	int64		database,
	pin_errbuf_t	*ebufp);

static void
fm_inv_pol_init_cache_state_code(
        pcm_context_t   *ctxp,
        int64           database,
        pin_errbuf_t    *ebufp);
/*******************************************************************
 * fm_inv_pol_init();
 *
 *	This routine caches the /config/invoice_templates objects.
 * 	(Actually only the object POIDs are cached for later
 * 	retrieval)
 *	If branding is disabled, then we need to cache only the
 *	config object belonging to the brand host (ROOT)
 *
 *******************************************************************/
void fm_inv_pol_init(
	int32			*errp) 
{
	pcm_context_t   *ctxp = NULL;
	pin_errbuf_t    ebuf;
	int64		database;
	poid_t			*pdp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*a_pdp = NULL;
	poid_t			*c_pdp = NULL;
	char 			s_template[BUFSIZ];
	int32			err = 0;

	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*ev_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*flistp = NULL;

	pin_cookie_t		cookie = NULL;
	int32			elemid = 0;

	int64			id = 0;
	int32			msize = 0;
	int32			strsize = 0;
	int32			s_flags = SRCH_DISTINCT;
	int32			flag = 0;
	int			*flagp = NULL;

	char			*strp = NULL;
	void			*vp = NULL;
	cm_cache_key_poid_t	cache_key;
	int32			nconfig;
	

	/***********************************************************
	 * Get pin.conf entries for later use.
	 * Default is to not show re-rated details.
	 ***********************************************************/
	pin_conf("fm_inv_pol", "show_rerate_details", PIN_FLDT_INT,
			(caddr_t *)&(flagp), errp);

	if (flagp) {
		fm_inv_pol_show_rerate_details = *flagp;
		free(flagp);
		flagp = NULL;
	} else {
		if (errp && (*errp == PIN_ERR_NOT_FOUND)) {
			*errp = PIN_ERR_NONE;
		}
	}

	/**************************************************************
	 * See if service centric invoicing is enabled.
	 **************************************************************/
	pin_conf("fm_inv_pol", "service_centric_invoice", PIN_FLDT_INT,
		(caddr_t *)&(flagp), &err);
	if (flagp != (int32 *)NULL) {
		fm_inv_pol_service_enabled = *flagp;
		pin_free(flagp);
		flagp = NULL;
	}
	/**************************************************************
         * See delivery pref for pdf in email is enabled
         **************************************************************/
        pin_conf("fm_inv_pol", "delivery_pref_email", PIN_FLDT_INT,
                (caddr_t *)&(flagp), &err);
        if (flagp != (int32 *)NULL) {
                fm_inv_pol_email_enabled = *flagp;
                pin_free(flagp);
                flagp = NULL;

        }

	/***********************************************************
	 * open the context and get the database number.
	 ***********************************************************/
	PIN_ERR_CLEAR_ERR(&ebuf);
	PCM_CONTEXT_OPEN(&ctxp, (pin_flist_t *)0, &ebuf);
	if(PIN_ERR_IS_ERR(&ebuf)) {
		pin_set_err(&ebuf, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_DM_CONNECT_FAILED, 0, 0, ebuf.pin_err);
		PIN_FLIST_LOG_ERR("fm_inv_init pcm_context_open err", &ebuf);
		if (errp) {
			*errp = PIN_ERR_DM_CONNECT_FAILED;
		}
		return;
	}

	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);
	PIN_POID_DESTROY(pdp, NULL);
	fm_inv_pol_init_cache_citywise_address(ctxp, database, &ebuf); 

        if (PIN_ERR_IS_ERR(&ebuf)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_inv_init: error loading "
                        "fm_inv_pol_init_cache_citywise_address object",
                        &ebuf);

                goto cleanup;
                /***********/
        }

	fm_inv_pol_init_cache_state_code(ctxp, database, &ebuf);

        if (PIN_ERRBUF_IS_ERR(&ebuf)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_inv_init: error loading "
                        "fm_inv_pol_init_cache_state_code object",
                        &ebuf);

                goto cleanup;
                /***********/
        }

	/***********************************************************
	 * Get all or root /config/invoice_templates objects
	 ***********************************************************/

	s_flistp = PIN_FLIST_CREATE(&ebuf);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, &ebuf);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, &ebuf);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, &ebuf);

	/* setup arguments */ 
	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, &ebuf);

	if (cm_fm_is_scoping_enabled()) {	
	  	a_pdp = PIN_POID_CREATE(database, "/account", -1, &ebuf);
	} else {			
	  	a_pdp = PIN_POID_CREATE(database, "/account", 1, &ebuf);
 	}
	PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)a_pdp,
			  &ebuf);

	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, &ebuf);
	s_pdp = PIN_POID_CREATE(database, "/config/invoice_templates", -1, &ebuf);
	PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_POID, (void *)s_pdp,
			  &ebuf);
		
	sprintf(s_template, "select X from /config "
		"where F1 = V1 and F2 = V2 ");
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, (void *)s_template,
			  &ebuf);
	
	res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 
		PCM_RECID_ALL, &ebuf);

	vp = NULL; 
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_ACCOUNT_OBJ, vp, &ebuf);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, vp, &ebuf);
	
	/* Also grab the flag for each invoice_templates object */
	flistp = PIN_FLIST_SUBSTR_ADD(res_flistp, PIN_FLD_INHERITED_INFO, 
		&ebuf);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_FLAGS, vp, &ebuf);
 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Search flist", s_flistp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, &ebuf);

	if (PIN_ERR_IS_ERR(&ebuf)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_init: error loading "
			"/config/invoice_events object", 
			&ebuf);

		goto cleanup;
		/***********/
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"Return flist", r_flistp);

	/* Housekeeping..  Free un-needed memory allocated so far */
	PIN_FLIST_DESTROY_EX (&s_flistp, NULL);

	/***********************************************************
	 * Allocate the CM Cache
	 * Cache Key  :  Brand Obj POID_DB.POID_ID0
	 *       Value:  /config/invoice_templates object POID
	 ***********************************************************/
	nconfig = PIN_FLIST_ELEM_COUNT( r_flistp, PIN_FLD_RESULTS,
			&ebuf );
					
	if (nconfig == 0) {
	  fm_inv_pol_config_cache_ptr = NULL;
	  goto cleanup;
	  /***********/
	} else {
                /*
                 * This cache doesn't need configuration because its size
                 * is computed
                 */
                fm_inv_pol_config_cache_ptr =
                        cm_cache_init("fm_inv_pol_config_cache",
                                      nconfig,
                                      nconfig * 1024,
                                      10, CM_CACHE_KEY_POID, &err);

		if (err != PIN_ERR_NONE) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_inv_init: error initing cm_cache",
				&ebuf);
			goto cleanup;
			/***********/
		}
	}

	/***********************************************************
	 * Loop thru all the result elements and grab the event types	
	 * and populate the cache
	 ***********************************************************/
	elemid = 0;
	while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp,
		 	PIN_FLD_RESULTS, &elemid, 1, &cookie, &ebuf)) != 
			(pin_flist_t *)NULL) {

		/* Grab the ACCOUNT_OBJ (which is the brand obj) */
		a_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_ACCOUNT_OBJ, 
				0, &ebuf);
		cache_key.id = PIN_POID_GET_ID ( a_pdp );
		/* Ignore the DB number for multi db case */
		cache_key.db = 0;

		/* Grab the POID (which is the config object POID) */
		c_pdp = PIN_FLIST_FLD_GET (res_flistp, PIN_FLD_POID, 
				0, &ebuf);

		/***********************************************************
		 * Store the config POID in the cache
		 ***********************************************************/
		ev_flistp = PIN_FLIST_CREATE (&ebuf);
		PIN_FLIST_FLD_SET (ev_flistp, PIN_FLD_POID, 
			(void *)c_pdp, &ebuf);

		/***********************************************************
		 * Also store the flag in the cache 			   *
		 ***********************************************************/
		flistp = PIN_FLIST_SUBSTR_GET(res_flistp, 
			PIN_FLD_INHERITED_INFO, 1, &ebuf);
		if (flistp != NULL) {
			vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_FLAGS, 1, &ebuf);
			if (vp) {
				flag = *(int32 *)vp;
			} else {
				flag = 0;
			}
		} else {
			flag = 0;
		}
		PIN_FLIST_FLD_SET(ev_flistp, PIN_FLD_FLAGS, (void *)&flag, 
			&ebuf);

		cm_cache_add_entry (fm_inv_pol_config_cache_ptr, 
				    (void *)&cache_key,
				    ev_flistp,
				    &err);
                switch (err) {
                        case PIN_ERR_NONE:
                                pinlog(FILE_SOURCE_ID, __LINE__, LOG_FLAG_DEBUG
,
                                "Added fm_inv_pol config cache entry: "
                                "cache key [%" I64_PRINTF_PATTERN "d.%" 
				 I64_PRINTF_PATTERN "d] : ",
				cache_key.db, cache_key.id);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
					"cache value :", ev_flistp);
                                break;
                        case PIN_ERR_OP_ALREADY_DONE:
                                pinlog(FILE_SOURCE_ID, __LINE__, LOG_FLAG_WARNING
,
                                "add fm_inv_pol config cache entry already done: "
                                "err [%d] ; cache key [%" I64_PRINTF_PATTERN 
				"d.%" I64_PRINTF_PATTERN "d]",err,
				cache_key.db, cache_key.id);
                                break;
                        default:
                                pin_set_err(&ebuf, PIN_ERRLOC_FM,
                                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                        PIN_ERR_NO_MEM, 0, 0, err);
                                pinlog(FILE_SOURCE_ID, __LINE__, LOG_FLAG_ERROR,
                                "bad fm_inv_pol config cache add entry: "
                                "err [%d] ; cache key [%" I64_PRINTF_PATTERN 
				"d.%" I64_PRINTF_PATTERN "d]",err,
				cache_key.db, cache_key.id);
                                break;
                }
		PIN_FLIST_DESTROY_EX(&ev_flistp, NULL);

	} /* end while */


	PIN_FLIST_DESTROY_EX (&r_flistp, NULL);
	PCM_CONTEXT_CLOSE(ctxp, 0, &ebuf);
	*errp = ebuf.pin_err;
	return;
cleanup: 
	PIN_FLIST_DESTROY_EX (&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX (&r_flistp, NULL);
	fm_inv_pol_config_cache_ptr = NULL;
	PCM_CONTEXT_CLOSE(ctxp, 0, &ebuf);
	*errp = ebuf.pin_err;
	return;
}

/***********************************************************************
 * fm_inv_pol_init_cache_citywise_address
 *
 ***********************************************************************/
static void
fm_inv_pol_init_cache_citywise_address(
	pcm_context_t	*ctxp,
	int64		database,
	pin_errbuf_t	*ebufp)
{
	poid_t		*s_pdp = NULL;
	poid_t		*a_pdp = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*arg_flistp = NULL;
	pin_flist_t	*c_flistp = NULL;

	pin_cookie_t	cookie = NULL;
	int32		err = PIN_ERR_NONE;
	int32		cnt = 0;
	int32		recid;
	int32		elemid = 0;
	int32		s_flags = SRCH_DISTINCT;
	int32		service_type = -1;

	s_flistp = PIN_FLIST_CREATE(ebufp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_config_network_info where F1.db = V1  ", ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_ELEM_SET(s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_inv_pol_init_cache_citywise_address search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_init_cache_citywise_address: "
			"error loading /mso_config_network_info object",
			ebufp);

		goto cleanup;
		/***********/
	}

	//res_flistp = PIN_FLIST_ELEM_TAKE(r_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);


	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read mso_config_network_info", r_flistp);

	/* Bail if there's an error (such as no such object!) */
	if (PIN_ERR_IS_ERR(ebufp) || !r_flistp){
		PIN_ERR_CLEAR_ERR(ebufp);
		goto cleanup;
	}

	/*
	 * Get the number of items in the flist so we can create an
	 * appropriately sized cache
	 */
	cnt = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp);

	/*
	 * If there's no data, there's no point initializing it, is there.
	 * Especially since this is optional!
	 */
	if (cnt == 0) {
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Got count as zero.. exiting");
		goto cleanup;
	}
	
	/*
	 * This cache doesn't need configuration since the sizes of the
	 * entries are computed
	 */
	fm_inv_pol_city_adr_map_ptr =
		cm_cache_init("fm_inv_pol_init_cache_citywise_address", cnt,
			      cnt * 4096, 8, CM_CACHE_KEY_IDDBSTR, &err);

	if (err != PIN_ERR_NONE) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"Error: Couldn't initialize "
				"citywise address cache in "
				"fm_inv_pol_init_cache_citywise_address");
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			    PIN_ERRCLASS_SYSTEM_DETERMINATE,
			    PIN_ERR_NO_MEM, 0, 0, err);
		goto cleanup;
	}

	if (!fm_inv_pol_city_adr_map_ptr) {
		goto cleanup;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Will read entries to put into cache");
	//PIN_FLIST_ELEM_TAKE(r_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	while (c_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp,
						  PIN_FLD_RESULTS,
						  &recid, 1, &cookie, ebufp)) {
		cm_cache_key_iddbstr_t  kids;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read entry", c_flistp);
		
		service_type = *(int32 *)PIN_FLIST_FLD_GET(c_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp);
		kids.id = service_type;    /* Not relevant for us */
		kids.db = 0;    /* Not relevant for us */
		kids.str = PIN_FLIST_FLD_GET(c_flistp, PIN_FLD_KEY_RIB, 0, ebufp); //state for CATV and city for ISP

		if (kids.str) {
			cm_cache_add_entry(fm_inv_pol_city_adr_map_ptr,
					   &kids, c_flistp, &err);
			switch(err) {
			case PIN_ERR_NONE:
				break;
			case PIN_ERR_OP_ALREADY_DONE:
				pinlog(__FILE__, __LINE__,
				       LOG_FLAG_WARNING,
				"fm_inv_pol_init_cache_citywise_address: "
				       "cache already done: <%s>", kids.str);
				break;
			default:
				pinlog(__FILE__, __LINE__,
				       LOG_FLAG_ERROR,
		  "fm_inv_pol_init_cache_citywise_address: "
				       "error adding cache <%s>", kids.str);
				break;
			}
		}
	}
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			   "fm_inv_pol_init_cache_citywise_address error",
				 ebufp);
	}

 cleanup:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
}

static void
fm_inv_pol_init_cache_state_code(
	pcm_context_t	*ctxp,
	int64		database,
	pin_errbuf_t	*ebufp)
{
	poid_t		*s_pdp = NULL;
	poid_t		*a_pdp = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*arg_flistp = NULL;
	pin_flist_t	*c_flistp = NULL;

	pin_cookie_t	cookie = NULL;
	int32		err = PIN_ERR_NONE;
	int32		cnt = 0;
	int32		recid;
	int32		elemid = 0;
	int32		s_flags = SRCH_DISTINCT;
	int32		service_type = -1;

	s_flistp = PIN_FLIST_CREATE(ebufp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_cfg_state_census_map where F1.db = V1  ", ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_ELEM_SET(s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_inv_pol_init_cache_state_code search input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_inv_pol_init_cache_state_code: "
			"error loading /mso_cfg_state_census_map object",
			ebufp);

		goto cleanup;
		/***********/
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read mso_cfg_state_census_map", r_flistp);

	/* Bail if there's an error (such as no such object!) */
	if (PIN_ERRBUF_IS_ERR(ebufp) || !r_flistp){
		PIN_ERRBUF_CLEAR(ebufp);
		goto cleanup;
	}

	/*
	 * Get the number of items in the flist so we can create an
	 * appropriately sized cache
	 */
	cnt = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp);
	if (cnt == 0) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Got count as zero.. exiting");
		goto cleanup;
	}

	/*
	 * This cache doesn't need configuration since the sizes of the
	 * entries are computed
	 */
	fm_inv_pol_state_code_ptr = cm_cache_init("fm_inv_pol_init_cache_state_code", cnt, cnt * 4096, 8, CM_CACHE_KEY_IDDBSTR, &err);

	if (err != PIN_ERR_NONE) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
			"Error: Couldn't initialize "
			"fm_inv_pol_init_cache_state_code");
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_NO_MEM, 0, 0, err);
		goto cleanup;
	}

	if (!fm_inv_pol_state_code_ptr) {
			goto cleanup;
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Will read entries to put into cache");
	while (c_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS, &recid, 1, &cookie, ebufp)) {
		cm_cache_key_iddbstr_t  kids;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read entry", c_flistp);

		kids.id = 0;    /* Not relevant for us */
		kids.db = 0;    /* Not relevant for us */
		kids.str = PIN_FLIST_FLD_GET(c_flistp, PIN_FLD_STATE, 0, ebufp); //state for CATV and city for ISP

		if (kids.str) {
			cm_cache_add_entry(fm_inv_pol_state_code_ptr, &kids, c_flistp, &err);
                        switch(err) {
                        case PIN_ERR_NONE:
                                break;
                        case PIN_ERR_OP_ALREADY_DONE:
                                pinlog(__FILE__, __LINE__,
                                       LOG_FLAG_WARNING,
                                "fm_inv_pol_init_cache_state_code: "
                                       "cache already done: <%s>", kids.str);
                                break;
                        default:
                                pinlog(__FILE__, __LINE__,
                                       LOG_FLAG_ERROR,
                  		"fm_inv_pol_init_cache_state_code: "
                                       "error adding cache <%s>", kids.str);
                                break;
                        }

		}
	}
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_inv_pol_init_cache_state_code error", ebufp);
	}

cleanup:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
}



