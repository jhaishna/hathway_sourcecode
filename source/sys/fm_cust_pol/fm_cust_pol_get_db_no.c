/*******************************************************************
 *
* Copyright (c) 1999, 2019, Oracle and/or its affiliates. All rights reserved.
 *	
 *	This material is the confidential property of Oracle Corporation
 *	or its licensors and may be used, reproduced, stored or transmitted
 *	only in accordance with a valid Oracle license or sublicense agreement.	
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_cust_pol_get_db_no.c /cgbubrm_mainbrm.portalbase/2 2019/02/25 22:33:19 agangrad Exp $";
#endif

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "cm_cache.h"
#include "pinlog.h"
#include "fm_bill_utils.h"
#include "pin_os_random.h"

/*******************************************************************
 * Contains the PCM_OP_CUST_POL_GET_DB_NO policy operation.
 *
 * Retrieves the corresponsding database number to work with based
 * on the selection criteria in the case of multi database system.
 *
 *******************************************************************/

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
void
EXPORT_OP  op_cust_pol_get_db_no(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static int64
fm_cust_pol_get_db_no(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_errbuf_t    *ebufp);

static int64
fm_cust_pol_verify_db_no(
	pcm_context_t   *ctxp,
	int64 	db_no,
	pin_flist_t *db_flistp,
	char *fld_name,
	pin_errbuf_t *ebufp);

static int64
fm_cust_pol_get_db_no_with_logical_partition(
        pcm_context_t   *ctxp,
        int64           dbno,
	pin_flist_t     *db_flistp,
        pin_errbuf_t    *ebufp);

static int32
fm_cust_pol_get_db_entries_by_priority(
	pcm_context_t   *ctxp,
	pin_flist_t     *db_flistp,
	pin_flist_t     **o_flistpp,
	pin_errbuf_t    *ebufp);

static int32
fm_cust_pol_gen_drand_no(
        int32    max_db_no);

pin_flist_t *
fm_cust_pol_get_db_no_cache_entry();


/*******************************************************************
 * Global for holding /config/distribution
 *******************************************************************/
static pin_flist_t *
fm_cust_pol_config_db_no_flistp = NULL;

/*******************************************************************
 * fm_cust_pol_init_config_multi_db():
 *
 *      Reads the /config/distribution object from the database
 *      and caches its contents for future reference.
 *
 *	In a timesten configured system with multiple partitions
 *	assumption is that logical partition number starts with 0 
 *	and increments sequentially.
 *******************************************************************/
void
fm_cust_pol_init_config_multi_db(
        pcm_context_t           *ctxp,
	int64                   database,
	pin_errbuf_t            *ebufp)
{
        pin_cookie_t            cookie = NULL;
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *pr_flistp = NULL;
        pin_flist_t             *temp_flistp = NULL;
        pin_flist_t             *flistp = NULL;
        poid_t                  *s_pdp = NULL;
	char			*template = NULL;

        int32                   s_flags = SRCH_DISTINCT;
        int32                   element_id = 0;
        
	if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);
 
        /*********************************************************
         * Setup for search.
         *********************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = (poid_t *)PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);

	/**************************************************************
	 * Initialize the search template	
	 **************************************************************/
	template = "select X from /config/$1 where F1 = V1 ";
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, (void *)template, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PARAMETERS,
			(void *)"distribution", ebufp);

	/***********************************************************
	 * Add the search criteria.
	 ***********************************************************/
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	s_pdp = (poid_t *)PIN_POID_CREATE(database, "/config/distribution",
			 -1, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

	/***********************************************************
	 * Add the result fields we want to read.
	 ***********************************************************/
	PIN_FLIST_ELEM_SET(s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);
	
        /**************************************************************
         * Perform the search
         **************************************************************/
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
 
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_cust_pol_init_config_multi_db search error", ebufp);
                PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
                return;
        }
 
        /***********************************************************
	 * Sort by priority first.
         ***********************************************************/
	pr_flistp = PIN_FLIST_CREATE(ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(pr_flistp, PIN_FLD_DISTRIBUTION, 
			0, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_DB_PRIORITY, (void *)NULL, ebufp);

	/***********************************************************
	 * Do the sorting
	 ***********************************************************/
	flistp = PIN_FLIST_ELEM_TAKE_NEXT(r_flistp, PIN_FLD_RESULTS,
                                          &element_id, 1, &cookie, ebufp);
	PIN_FLIST_SORT_REVERSE(flistp, pr_flistp, 0, ebufp);
	PIN_FLIST_DESTROY_EX (&pr_flistp, NULL);
        
        /***********************************************************
         * Save the static pointer
         ***********************************************************/
	fm_cust_pol_config_db_no_flistp = flistp;
	
        /***********************************************************
         * Cleanup.
         ***********************************************************/
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
 
        /***********************************************************
         * Errors?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_cust_pol_init_config_multi_db error", ebufp);
        }
 
        return;
}

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_GET_DB_NO operation. 
 * This routine does the following:
 * It checks if the input contains the following in the order specified 
 *      PIN_FLD_PARENT if so it uses the POID_DB of this poid 
 *      PIN_FLD_PARENT_BILLINFO_OBJ if so it uses the POID_DB of this poid 
 *      PIN_FLD_BRAND if so it uses the POID_DB of this poid 
 *      PIN_FLD_SPONSOR if so it uses the POID_DB of this poid 
 *      PIN_FLD_GROUP_OBJ if so it uses the POID_DB of this poid 
 *      PIN_FLD_DEVICES under SERVICES if so it uses the POID_DB of this poid 
 * If timesten is enabled with multiple logical partitions 
 * we compute the POID_DB using the SCHEMA number from the above poids plus 
 * randomly generating the logical partition number.
 * If any of the above fields are not passed then we look into the
 * config/distribution object and we get the entries by priority.
 * if we get multiple entries by priority we randomly selects one entry
 * which has valid active status of database and return that poid_db. 
 *******************************************************************/
void
op_cust_pol_get_db_no(
        cm_nap_connection_t	*connp,
	u_int			opcode,
        u_int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	poid_t			*pdp = NULL;
	int64			db_no = 0;


	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*o_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_GET_DB_NO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_get_db_no", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_get_db_no input flist", i_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	db_no = fm_cust_pol_get_db_no(ctxp, i_flistp, ebufp);

	/***********************************************************
	 * Prep the output flist.
	 ***********************************************************/
	*o_flistpp = PIN_FLIST_CREATE(ebufp);
	pdp = PIN_POID_CREATE(db_no, "", 0, ebufp);
	PIN_FLIST_FLD_PUT(*o_flistpp, PIN_FLD_POID, (void *)pdp, ebufp);
 
	/***********************************************************
	 * Set the results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*o_flistpp = (pin_flist_t *)NULL;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_get_db_no error", ebufp);
	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_get_db_no return flist",*o_flistpp);
	}

	return;
}
/*******************************************************************
 * fm_cust_pol_get_db_no():
 *
 *      Reads the cache and the /config/distribution object  
 *      and returns the next available database for account creation.
 *
 *******************************************************************/
static int64
fm_cust_pol_get_db_no(
        pcm_context_t   *ctxp,
        pin_flist_t     *i_flistp,
        pin_errbuf_t    *ebufp)
{
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	acookie = NULL;
        pin_flist_t     *flistp = NULL;
        pin_flist_t     *db_flistp = NULL;
        pin_flist_t     *o_flistp = NULL;
        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *a_flistp = NULL;
        pin_flist_t     *temp_flistp = NULL;
	poid_t		*pdp = NULL;
	poid_t		*p_pdp = NULL;
	int32		*statusp = NULL;
	int32		*db_no_p = NULL;
	int64		db_no = 0;
	int64		db_lp_no = 0;
	int64		db_db_no = 0;
	int64           *logical_partitions = NULL;

        int32           err = PIN_ERR_NONE;
	int32		element_id = 0;
	int32		elem_id = 0;
	int32		aelemid = 0;
	int32		rand_no = 0;
	int32		count = 0;
	int32		elem_count = 0;
	int             cnt_logical_partitions = 0;
 

        if (PIN_ERR_IS_ERR(ebufp))
                return ((int64) 0);
        PIN_ERR_CLEAR_ERR(ebufp);
 
        /***********************************************************
         * Check to see whether we got errors.
         ***********************************************************/
        if (!fm_cust_pol_config_db_no_flistp) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NOT_FOUND, 0, 0, PIN_ERR_NOT_FOUND);
                PIN_FLIST_LOG_ERR("fm_cust_pol_get_db_no: "
                        "flist cache NULL ptr err", ebufp);
                return((int64) 0);
        }

        /***********************************************************
         * Get the database object from the cache.
         ***********************************************************/
	db_flistp = fm_cust_pol_config_db_no_flistp;

	cookie = NULL;

        PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
                        "i_flistp input flist for DB-", i_flistp);

	if (cm_fm_is_logical_partition()) {
		a_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_ACCTINFO,
                        &aelemid, 1, &acookie, ebufp);

                if (a_flistp != (pin_flist_t *)NULL) {
                        p_pdp = (poid_t *)PIN_FLIST_FLD_GET(a_flistp,
                                        PIN_FLD_PARENT, 1, ebufp);
			if (!PIN_POID_IS_NULL(p_pdp)) {
				db_no = PIN_POID_GET_DB(p_pdp);
				return fm_cust_pol_verify_db_no(ctxp, db_no, db_flistp,
					"PIN_FLD_PARENT", ebufp);	
			}
                }
	}

        /*****************************************************************
        * Go though each PIN_FLD_PARENT_BILLINFO_OBJ to validate that all
        * parent's DB point to the same DB.
        *****************************************************************/
        while ((s_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_BILLINFO,
                                        &elem_id, 1, &cookie, ebufp))) {

                poid_t  *dev_pdp;

                dev_pdp = PIN_FLIST_FLD_GET(s_flistp,
                                        PIN_FLD_PARENT_BILLINFO_OBJ, 1, ebufp);

                if (!PIN_POID_IS_NULL(dev_pdp)) {
                        db_no = PIN_POID_GET_DB(dev_pdp);
                        break;
                }
        }
        if ( db_no ) {
		return fm_cust_pol_verify_db_no(ctxp, db_no, db_flistp,
			"PIN_FLD_PARENT_BILLINFO_OBJ", ebufp);
        }

        /***********************************************************
         * Use the db id specified in PIN_FLD_BRAND_OBJ, if present 
         ***********************************************************/
	pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BRAND_OBJ, 1, ebufp);
	if ( pdp != NULL ) {
		db_no = PIN_POID_GET_DB( pdp );
		return fm_cust_pol_verify_db_no(ctxp, db_no, db_flistp,
			"PIN_FLD_BRAND_OBJ", ebufp);	
	}

        /***********************************************************
         * Use the db id specified in PIN_FLD_GROUP_OBJ of the 
	 * PIN_FLD_SPONSOR, if present 
         ***********************************************************/
	s_flistp = PIN_FLIST_ELEM_GET( i_flistp, PIN_FLD_SPONSOR, 
					PIN_ELEMID_ANY, 1, ebufp );
	if ( s_flistp ) {
		/* if there is a sponsor array there better be a group in it */
		pdp = PIN_FLIST_FLD_GET( s_flistp, PIN_FLD_GROUP_OBJ, 0, 
					ebufp );

		if ( pdp != NULL ) {
			db_no = PIN_POID_GET_DB( pdp );
		 	return fm_cust_pol_verify_db_no(ctxp, db_no, db_flistp,
				"PIN_FLD_GROUP", ebufp);	
		}
	}

        /***************************************************************************
         * Get the db number from the group info substruct associated with a
         * parent
         ***************************************************************************/
        s_flistp = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_GROUP_INFO, 1, ebufp);
        if ( s_flistp ) {
                /* if there is a group info substruct there better be a parent in it */
                pdp = PIN_FLIST_FLD_GET( s_flistp, PIN_FLD_PARENT, 0,
                                        ebufp );

                if ( pdp != NULL ) {
                        db_no = PIN_POID_GET_DB( pdp );
			return fm_cust_pol_verify_db_no(ctxp, db_no, db_flistp,
				"PIN_FLD_PARENT", ebufp);
                }
        }

	/**************************************************************
	 * Get the db number from the service array associated with the 
	 * devices 
	 * In the case of TimesTen, the devices are located in once schema
	 * so the device poid donot have logical partition and as
	 * well as the DB number that we want to create an account in.
	 * Need to revisit this later -
	 *  Now we get random DBNO as well as random logical parition.
	 * based on the config distribution details.
	 *************************************************************/
	if (!(is_timesten_used)) {
		cookie = NULL;
		while ((s_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_SERVICES,
			&elem_id, 1, &cookie, ebufp))) {
			pin_flist_t *dev_flistp;
			pin_cookie_t cookie2p = NULL;
			int32 dev_elemid = 0;
			while ((dev_flistp = PIN_FLIST_ELEM_GET_NEXT(s_flistp, 
				PIN_FLD_DEVICES,&dev_elemid, 1, &cookie2p, ebufp))) {
				poid_t  *dev_pdp;
				dev_pdp = PIN_FLIST_FLD_GET(dev_flistp, 
					PIN_FLD_DEVICE_OBJ, 1, ebufp);
				if (dev_pdp != NULL) {
					db_no = PIN_POID_GET_DB(dev_pdp);
					return fm_cust_pol_verify_db_no(ctxp, db_no, 
						db_flistp, "PIN_FLD_DEVICE", ebufp);
				}
			}
		}
	}

        /***********************************************************
         * Now read the limit and current count for each database 
         ***********************************************************/
	count = fm_cust_pol_get_db_entries_by_priority(ctxp, db_flistp,
			&o_flistp, ebufp);
	pinlog("fm_cust_pol_get_db_no.c", __LINE__, LOG_FLAG_DEBUG, "count : %d", 
			count, err);

	if ( count > 1) {

		PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
			"o_flistp -", o_flistp);

		rand_no = fm_cust_pol_gen_drand_no(count);
		pinlog("fm_cust_pol_get_db_no.c", __LINE__, 
			LOG_FLAG_DEBUG, "rand no: %d", rand_no, err);

		cookie = (pin_cookie_t)NULL;
		while ((temp_flistp = PIN_FLIST_ELEM_GET_NEXT(o_flistp, 
			PIN_FLD_DISTRIBUTION, &elem_id, 1, &cookie, 
			ebufp)) != (pin_flist_t *)NULL) {
			if (elem_count == rand_no) {
				break;
			} else {
				elem_count++;
			}
		}
		PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
			"temp_flistp -", temp_flistp);
		db_no_p = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_DB_NO, 
			0, ebufp);

		if (db_no_p) {
			db_db_no  = pin_poid_get_db_no_32 (*db_no_p);
			cm_fm_get_logical_partitions(db_db_no, &logical_partitions, 
					&cnt_logical_partitions, ebufp);
		}
		/***********************************************************
		* In case of TimesTen we will get the random logical partition
		* and construct the db_no with the logical partition and schema
		************************************************************/
		if (cnt_logical_partitions >= 1) {
			db_lp_no = pin_poid_get_lp_no_32 (*db_no_p);

			/***********************************************
			 * If the config/distribution already contains
			 * the logical parition info then we consider.
			 * We assume that the priority is based on 
			 * the combination of DBNO + LPNO
			 * we need to create database with lp_no and db_no
			 * because the database is a int64 where as
			 * DB_NO from config_distribution is int32.
			 ***********************************************/
			 db_no = PIN_CREATE_DATABASE_NO(db_db_no, db_lp_no);
		} else {
			db_no = (int64) (db_no_p ? *db_no_p : 0);
		}
		PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
		if (logical_partitions) {
			pin_free(logical_partitions);
		}

		return (db_no);

	} else if (count == 1) {
		PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
			"o_flistp -", o_flistp);

		cookie = (pin_cookie_t)NULL;
		flistp = PIN_FLIST_ELEM_GET_NEXT(o_flistp,
			PIN_FLD_DISTRIBUTION, &elem_id, 0, &cookie,
			ebufp);

		db_no_p = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DB_NO, 
			0, ebufp);
		if (db_no_p) {
			db_db_no  = pin_poid_get_db_no_32 (*db_no_p);
			cm_fm_get_logical_partitions(db_db_no, &logical_partitions, 
					&cnt_logical_partitions, ebufp);
		}
		/***********************************************************
		* In case of TimesTen we will get the random logical partition
		* and construct the db_no with the logical partition and schema
		************************************************************/
		if (cnt_logical_partitions >= 1) {
			db_lp_no = pin_poid_get_lp_no_32 (*db_no_p);

			/***********************************************
			 * If the config/distribution already contains
			 * the logical parition info then we consider.
			 * We assume that the priority is based on 
			 * the combination of DBNO + LPNO
			 * we need to create database with lp_no and db_no
			 * because the database is a int64 where as
			 * DB_NO from config_distribution is int32.
			 ***********************************************/
			 db_no = PIN_CREATE_DATABASE_NO(db_db_no, db_lp_no);

		} else {
			db_no = (int64) (db_no_p ? *db_no_p : 0);
		}

        	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
		if (logical_partitions) {
			pin_free(logical_partitions);
		}

		return (db_no);
	}

	cookie = (pin_cookie_t)NULL;
        /***********************************************************
         * We still couldn't find a valid database, we just pick the
	 * first available database without checking for limits.
         ***********************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(db_flistp, 
		PIN_FLD_DISTRIBUTION, &element_id, 1, &cookie, 
		ebufp)) != (pin_flist_t *)NULL) {

        	/***************************************************
         	 * First check the status of the database.
         	 ***************************************************/
		statusp = (int32 *)PIN_FLIST_FLD_GET(flistp,
				PIN_FLD_DB_STATUS, 0, ebufp);
		if(statusp){
			pinlog("fm_cust_pol_get_db_no.c", __LINE__, LOG_FLAG_DEBUG, "database elem_id: %d and status : %d",
                        	element_id, (int) *statusp, err);
		}
		if (statusp && (*statusp == PIN_DB_STATUS_UNAVAILABLE)) {
		pinlog("fm_cust_pol_get_db_no.c", __LINE__, LOG_FLAG_DEBUG, "Skipping Unavailable database elem_id: %d",
                        element_id, (int) *statusp, err);
			continue;
		}

        	/***************************************************
         	 * Get the database number.
         	 ***************************************************/
		db_no_p = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DB_NO, 
				0, ebufp);

		if (db_no_p) {
			db_db_no  = pin_poid_get_db_no_32 (*db_no_p);
			cm_fm_get_logical_partitions(db_db_no, &logical_partitions,
                                        &cnt_logical_partitions, ebufp);
		}
		/***********************************************************
		* In case of TimesTen we will get the random logical partition
		* and construct the db_no with the logical partition and schema
		************************************************************/
		if (cnt_logical_partitions >= 1) {
			db_lp_no  = pin_poid_get_lp_no_32 (*db_no_p);
			db_no = PIN_CREATE_DATABASE_NO(db_db_no, db_lp_no);
		} else {
			db_no = (int64) (db_no_p ? *db_no_p : 0);
		}
        	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
		if (logical_partitions) {
			pin_free(logical_partitions);
		}
		pinlog("fm_cust_pol_get_db_no.c", __LINE__, LOG_FLAG_DEBUG, "Selected DB Number : %d",
                        db_no, (int) *statusp, err);
		return (db_no);
	}

        /***********************************************************
         * We still couldn't find a valid database, set the error 
         ***********************************************************/
	pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_NOT_FOUND, PIN_FLD_DB_NO, 0, 
			PIN_ERR_NOT_FOUND);
                PIN_FLIST_LOG_ERR("fm_cust_pol_get_db_no: "
                        "Could not find a valid database", ebufp);

        /***********************************************************
         * Cleanup?
         ***********************************************************/
       	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
	if (logical_partitions) {
                pin_free(logical_partitions);
        }


        /***********************************************************
         * Errors?
         ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_cust_pol_get_db_no error", ebufp);
        }

	return ((int64)0);
}

/*******************************************************************
 * This routine walks through the list of databases available and
 * gets only the valid databases while checking the database status. 
 * verify_db_no -> db_no is a PIN_POID_GET_DB ( poid)
 * In the case of TimesTen, in addition to validating the database status
 * we will also get random logical partition and create a poid db.
 *******************************************************************/

static int64
fm_cust_pol_verify_db_no(
	pcm_context_t   *ctxp,
        int64		db_no,
        pin_flist_t	*db_flistp,
        char		*fld_name,
        pin_errbuf_t	*ebufp)
{
        pin_cookie_t    cookie = NULL;
        pin_flist_t     *flistp = NULL;
	char		msg[200];
        int32           *db_no_p = NULL;
	int32		d_db_no = 0;
	int64		db_lp_no = 0;
        int32           element_id = 0;
        int32           *statusp = NULL;
	int64		*logical_partitions = NULL;
	int32		schema_no = 0;
	int             cnt_logical_partitions = 0;

        if (PIN_ERR_IS_ERR(ebufp))
		return ((int64) 0);
        PIN_ERR_CLEAR_ERR(ebufp);

	schema_no = (int32) PIN_GET_SCHEMA_NO(db_no);
	if (is_timesten_used) {
		cm_fm_get_logical_partitions(schema_no, &logical_partitions, 
					&cnt_logical_partitions, ebufp);
	     /*******************************************
	     * We are not using logical_paritions anywhere
	     * only using count of logical partitions
	     ********************************************/
	     if (logical_partitions) {
		pin_free(logical_partitions);
	     }
	}
	cookie = (pin_cookie_t)NULL;
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(db_flistp,
        	PIN_FLD_DISTRIBUTION, &element_id, 1, &cookie,
		ebufp)) != (pin_flist_t *)NULL) {
		db_no_p = PIN_FLIST_FLD_GET(flistp,
			PIN_FLD_DB_NO, 0, ebufp);
		/*************************************************
		* In the case of timesten 
		* DB_NO, LP_NO and other two reserved entries 
		* are placed into the single PIN_FLDT_INT
		* we need to use an api to get the db_no
		* Without timesten the db_no_p contains just the 
		* databse number ex: 1 or 2 or 3 
		* and not 0.1.0.1 in integer format
		***************************************************/
		if ((db_no_p) && (cnt_logical_partitions > 1)) {
			d_db_no = pin_poid_get_db_no_32 (*db_no_p);
		       if (d_db_no && ( schema_no == d_db_no )) {
			    /************************************************
			    * Compute the DB_NO with random logical partition
			    *************************************************/
			    db_no  =
                                   fm_cust_pol_get_db_no_with_logical_partition(ctxp,
                                               schema_no, db_flistp, ebufp);

			     if (PIN_ERR_IS_ERR(ebufp)) {
				return ((int64) 0);
			     } else {
                                return (db_no);
			    }
			}
		} else {

		   if (db_no_p && ( db_no == (int64) *db_no_p )) {
			statusp = (int32 *)PIN_FLIST_FLD_GET
				(flistp, PIN_FLD_DB_STATUS,
				0, ebufp);
			if (statusp && (*statusp != PIN_DB_STATUS_UNAVAILABLE)) {
				return (db_no);
			}
			/* Specified db id status not valid */
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_DB_STATUS, 0, 0);
			pin_snprintf(msg, sizeof(msg), "fm_cust_pol_get_db_no: Database specified "
				"in %s is unavailable", fld_name);
			PIN_FLIST_LOG_ERR(msg, ebufp);
			return ((int64) 0);
		   }
		}
	}

	/* Specified db id not found in cache */
	pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_NOT_FOUND, 0, 0, PIN_ERR_NOT_FOUND);
	pin_snprintf(msg, sizeof(msg), "fm_cust_pol_get_db_no: unusable database number "
		"specified in %s ", fld_name);
	PIN_FLIST_LOG_ERR(msg, ebufp);
	return ((int64) 0);
}

/*******************************************************************
 * This routine walks through the list of databases available and
 * gets only the valid databases and those of the same priority.
 *******************************************************************/

static int32
fm_cust_pol_get_db_entries_by_priority(
        pcm_context_t   *ctxp,
        pin_flist_t     *db_flistp,
        pin_flist_t     **o_flistpp,
        pin_errbuf_t    *ebufp)
{
	pin_cookie_t    cookie = NULL;
        pin_flist_t     *cfg_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *flistp = NULL;
        pin_flist_t     *t_flistp = NULL;
	int32           *statusp = NULL;
	int32		*max_count = NULL;
	int32		*cur_count = NULL;
	int32           *db_priority = NULL;
        int32           max_db_priority = 0;
	void		*vp = NULL;

	int32           element_id = 0;
	int32           count = 0;
        int64           primary_db_no = 0;
        int64           ctx_db_no = 0;

	pcm_context_t   *vctxp = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*o_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	t_flistp = PIN_FLIST_CREATE(ebufp);
        /***********************************************************
         * Now get the list of valid databases with same priority. 
         ***********************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(db_flistp, 
		PIN_FLD_DISTRIBUTION, &element_id, 1, &cookie, 
		ebufp)) != (pin_flist_t *)NULL) {

        	/***************************************************
         	* If there is limit set for this database, get the
	 	* current count by reading the /config/distribution.
         	***************************************************/
		if (cfg_flistp == (pin_flist_t *)NULL) {

			r_flistp = PIN_FLIST_CREATE(ebufp);
			vp = PIN_FLIST_FLD_GET(db_flistp, 
				PIN_FLD_POID, 0, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, 
				PIN_FLD_POID, vp, ebufp);
			PIN_FLIST_ELEM_SET(r_flistp, NULL, 
				PIN_FLD_DISTRIBUTION, PIN_ELEMID_ANY, ebufp);

			/***************************************************
			* If using external txn to create customers in a 
			* multi-db env we need to open a new context to 
			* read the config object from primary db only if
			* external context if different from primary context
			***************************************************/

			cm_fm_get_primary_db_no(&primary_db_no, ebufp);

			ctx_db_no = cm_fm_get_current_db_no(ctxp);

			if ( primary_db_no != ctx_db_no ) {
				PCM_CONTEXT_OPEN(&vctxp, (pin_flist_t *)0, ebufp);
			} else {
				vctxp = ctxp;
			}

			PCM_OP(vctxp, PCM_OP_READ_FLDS, 0, r_flistp, 
				&cfg_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

			/***************************************************
			* Close the previously opened context.
                        ***************************************************/

                        if ( primary_db_no != ctx_db_no ) {
                                PCM_CONTEXT_CLOSE(vctxp, 0, ebufp);
			}

		PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
			"cfg_flistp -", cfg_flistp);
		}

        	/***************************************************
         	* Do we have have room in this database?
         	***************************************************/
		r_flistp = PIN_FLIST_ELEM_GET(cfg_flistp, PIN_FLD_DISTRIBUTION,
			element_id, 0, ebufp);

        	/***************************************************
         	 * First check the status of the database.
         	 ***************************************************/
		statusp = (int32 *)PIN_FLIST_FLD_GET(r_flistp,
				PIN_FLD_DB_STATUS, 0, ebufp);
		if (statusp && (*statusp != PIN_DB_STATUS_OPEN)) {
			continue;
		}

        	/***************************************************
         	 * Is there a limit set for this database? If not
		 * we have found the database.
         	 ***************************************************/
		max_count = (int32 *)PIN_FLIST_FLD_GET(r_flistp,
				PIN_FLD_MAX_ACCOUNT_SIZE, 0, ebufp);

        	/***************************************************
         	 * Did we exceed the limit already?
         	 ***************************************************/
		cur_count = (int32 *)PIN_FLIST_FLD_GET(r_flistp,
				PIN_FLD_CURR_ACCOUNT_SIZE, 0, ebufp);
		if (cur_count && max_count && (*cur_count >= *max_count) &&
			(*max_count != -1)) {
			continue;
		}

        	/***************************************************
         	 * Now from here on we have only valid databases to
		 * work with. 
		 * Now check to see if the priorities are the same.
         	 ***************************************************/
		db_priority = (int32 *)PIN_FLIST_FLD_GET(r_flistp,
				PIN_FLD_DB_PRIORITY, 0, ebufp);

                if (db_priority && (*db_priority >= max_db_priority)) {
                        max_db_priority = *db_priority;
			PIN_FLIST_ELEM_SET(t_flistp, r_flistp,
				PIN_FLD_DISTRIBUTION, element_id, ebufp);
                }
	}
        /******************************************************************
         * Now from here on we have only valid databases to work with.
         * Walk through the list, and delete any databases with a priority
         * lower than max_db_priority
         *******************************************************************/
        *o_flistpp = PIN_FLIST_CREATE(ebufp);
        cookie = NULL;
        
        while ((flistp =
               PIN_FLIST_ELEM_GET_NEXT(t_flistp, PIN_FLD_DISTRIBUTION,
                                       &element_id, 1, &cookie, ebufp))) {
                db_priority = (int32 *)PIN_FLIST_FLD_GET(flistp,
				PIN_FLD_DB_PRIORITY, 0, ebufp);
                if (db_priority && (*db_priority == max_db_priority)) {
                        PIN_FLIST_ELEM_SET(*o_flistpp, flistp,
                                           PIN_FLD_DISTRIBUTION, element_id,
                                           ebufp);
                        count++;
                }
        }
        PIN_FLIST_DESTROY_EX(&t_flistp, NULL);        
       	PIN_FLIST_DESTROY_EX(&cfg_flistp, NULL);
	return (count);
}

/*******************************************************************
 * This routine finds the cache entry representing the available
 * databases and returns it to the caller.  In a single-DB system, 
 * the returned value may be null.
 *******************************************************************/
pin_flist_t *
fm_cust_pol_get_db_no_cache_entry()
{
   pin_flist_t* p_cache_flist= (pin_flist_t*)NULL;
   pin_errbuf_t ebuf;
   
   PIN_ERR_CLEAR_ERR(&ebuf);
   
   if (fm_cust_pol_config_db_no_flistp) {
      p_cache_flist= PIN_FLIST_COPY(fm_cust_pol_config_db_no_flistp, &ebuf);
      
      if (PIN_ERR_IS_ERR(&ebuf)) {
         PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_pol_get_db_no_cache_entry error", &ebuf);
         p_cache_flist= (pin_flist_t*)NULL;
      }
   }
  
   return p_cache_flist;
}


/****************************************************
 * This routine is called from verify_db_no when
 * either PIN_FLD_PARENT or PIN_FLD_SPONSOR or
 * ACCTG_INFO.POID etc... are passed in the input.
 *
 * This routine gets the logical available partitions 
 * by priority with in a schema.
 *
 * If there are no partitions available then errors out.
 *
 * If there is only one partition available use it.
 *
 * If there are multiple partitions with same priority
 * this routine constructs db_no with random 
 * logical partition + database no
 *
 * passed dbno contains just 1(just schema no)
 *   not complete 0.0.0.1
 * Return construct_db_no is full poid 0.x.0.y
*****************************************************/
static int64
fm_cust_pol_get_db_no_with_logical_partition(
        pcm_context_t   *ctxp,
        int64           dbno,
	pin_flist_t     *db_flistp,
        pin_errbuf_t    *ebufp)
{

        int64           lp_no = 0;
	int64           db_db_no = 0;
	int64		construct_db_no = 0;
	pin_flist_t     *flistp = NULL;
        pin_flist_t     *t_flistp = NULL;
	pin_flist_t     *hp_flistp = NULL;


	pin_cookie_t    cookie = NULL;
	
	int32           element_id = 0;
	int32           *statusp = NULL;
	int32           *db_no_p = NULL;
	int32           *max_countp = NULL;
        int32           *cur_countp = NULL;
	int32           *db_priorityp = NULL;
        int32           max_db_priorityp = 0;

	int32		count = 0;
	int32		elem_count = 0;
	char            msg[200];

        PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
		"config distribution db_flistp _logical-", db_flistp);


	t_flistp = PIN_FLIST_CREATE(ebufp);
	/***************************************************
	* Filter the distribution list to get only
	* available partitions with in the given schema.
	***************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(db_flistp,
                PIN_FLD_DISTRIBUTION, &element_id, 1, &cookie,
                ebufp)) != (pin_flist_t *)NULL) {
		 statusp = (int32 *)PIN_FLIST_FLD_GET(flistp,
                                PIN_FLD_DB_STATUS, 0, ebufp);
		if (statusp && (*statusp == PIN_DB_STATUS_UNAVAILABLE)) {
			continue;
		} else {
			db_no_p = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DB_NO,
                                0, ebufp);
			if(db_no_p){
				db_db_no  = pin_poid_get_db_no_32 (*db_no_p);
			}
			if (dbno == db_db_no) {
				max_countp = (int32 *)PIN_FLIST_FLD_GET(flistp,
                                PIN_FLD_MAX_ACCOUNT_SIZE, 0, ebufp);
				 cur_countp = (int32 *)PIN_FLIST_FLD_GET(flistp,
                                PIN_FLD_CURR_ACCOUNT_SIZE, 0, ebufp);
				if (cur_countp && max_countp && (*cur_countp >= *max_countp) &&
					(*max_countp != -1)) {
					continue;
				}
				db_priorityp = (int32 *)PIN_FLIST_FLD_GET(flistp,
                                PIN_FLD_DB_PRIORITY, 0, ebufp);

				if (db_priorityp && (*db_priorityp >= max_db_priorityp)) {
					max_db_priorityp = *db_priorityp;

					PIN_FLIST_ELEM_SET(t_flistp, flistp,
						PIN_FLD_DISTRIBUTION, element_id, ebufp);


				}
			}
		}
	}
        cookie = NULL;
	/**************************************************************
	* Get only the highest priority partitions with in given schema
	***************************************************************/
	hp_flistp = PIN_FLIST_CREATE(ebufp);
	while ((flistp =
               PIN_FLIST_ELEM_GET_NEXT(t_flistp, PIN_FLD_DISTRIBUTION,
                                       &element_id, 1, &cookie, ebufp))) {
                db_priorityp = (int32 *)PIN_FLIST_FLD_GET(flistp,
                                PIN_FLD_DB_PRIORITY, 0, ebufp);
                if (db_priorityp && (*db_priorityp == max_db_priorityp)) {
                        PIN_FLIST_ELEM_SET(hp_flistp, flistp,
                                           PIN_FLD_DISTRIBUTION, element_id,
                                           ebufp);
                        count++;
                }
        }
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
		"high priority hp_flistp ", hp_flistp);

	 /**********************************************************
	 * We got available partitions per schema sort by priority
	 **********************************************************/
	 if (count == 0 ) {
		/* No logical partition available */
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_DB_STATUS, 0, 0);
		sprintf(msg, "fm_cust_pol_get_db_no: Logical Partitions "
				"in SCHEMA %" I64_PRINTF_PATTERN "d is "
				"unavailable", dbno);
		PIN_FLIST_LOG_ERR(msg, ebufp);
		return ((int64) 0);

	 } else if (count > 1) {
		/***********************************************************
		 * Get a random number from same priority partitions
		 * get_random_no returns 0 to 3 if we send count as 4
		 *
		 * We can enhance it later to match with parent/sponsor/brand
		 * partition and create in that partition .
		 *
		 * For now lets take the random available partition 
		 ***********************************************************/
		lp_no = fm_cust_pol_gen_drand_no(count);
		cookie = (pin_cookie_t)NULL;
		element_id = 0;
		while ((flistp = PIN_FLIST_ELEM_GET_NEXT(hp_flistp, 
			PIN_FLD_DISTRIBUTION, &element_id, 1, &cookie, 
			ebufp)) != (pin_flist_t *)NULL) {
			if (elem_count == lp_no) {
				break;
			} else {
				elem_count++;
			}
		}

		db_no_p = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DB_NO, 
			0, ebufp);
		if(db_no_p){
			lp_no = pin_poid_get_lp_no_32 (*db_no_p);
		}
		construct_db_no = PIN_CREATE_DATABASE_NO(dbno, lp_no);

	 } else if (count == 1) {
		cookie = (pin_cookie_t)NULL;
                flistp = PIN_FLIST_ELEM_GET_NEXT(hp_flistp,
                        PIN_FLD_DISTRIBUTION, &element_id, 0, &cookie,
                        ebufp);

                db_no_p = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DB_NO,
                        0, ebufp);
		if(db_no_p){
                        lp_no = pin_poid_get_lp_no_32 (*db_no_p);
                }
		
		construct_db_no = PIN_CREATE_DATABASE_NO(dbno, lp_no);


	 }

	PIN_FLIST_DESTROY_EX(&t_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&hp_flistp, NULL);

        return (construct_db_no);
}

/*******************************************************
  * This routine generates a random number with new seed
  * everytime using pin_virtual_time.
  * The distribution is more even using lrand than rand.
********************************************************/
static int32 fm_cust_pol_gen_drand_no(
        int32    numdb)
{
	struct timeval tv;
	int32 tmp=0;

	gettimeofday(&tv, (struct timezone *) 0);
	tmp = (int32) ((u_int64 ) tv.tv_usec  % numdb);
	return  tmp;

}
