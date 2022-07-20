/* Continuus file information --- %full_filespec: fm_cust_pol_get_db_list.c~2:csrc:2 % */
/*
 * @(#) %full_filespec: fm_cust_pol_get_db_list.c~2:csrc:2 %
 *
 *      Copyright (c) 2000-2006 Oracle. All rights reserved.
 * 
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 */

#ifndef lint
static  char    Sccs_id[] = "@(#) %full_filespec: fm_cust_pol_get_db_list.c~2:csrc:2 %";
#endif

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "pcm.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "cm_cache.h"
#include "pinlog.h"

/*******************************************************************
 * Contains the PCM_OP_CUST_POL_GET_DB_LIST policy operation.
 *
 * Retrieves the list of databases that are available
 * to Portal for the purpose of creating objects.
 *
 *******************************************************************/

/*******************************************************************
 * Routines contained within.
 *******************************************************************/

EXPORT_OP void
op_cust_pol_get_db_list(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_get_db_list(
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern pin_flist_t* fm_cust_pol_get_db_no_cache_entry();

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_GET_DB_LIST operation. 
 *******************************************************************/
void
op_cust_pol_get_db_list(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			opflags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**r_flistpp,
        pin_errbuf_t		*ebufp)
{
	/***********************************************************
	 * If there are pending errors, then short circuit immediately
	 ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp))
                return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
         * Null out results until we're sure there's something to send back
	 ***********************************************************/
        *r_flistpp = NULL;

	/***********************************************************
	 * Sanity check - make sure opcode was routed properly.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_GET_DB_LIST) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_get_db_list", ebufp);
		return;
	}

	/***********************************************************
	 * For completeness, dump the input flist if debugging is enabled
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_get_db_list input flist", i_flistp);

	/***********************************************************
	 * Call the main function to do the real work
	 ***********************************************************/
	fm_cust_pol_get_db_list(i_flistp, r_flistpp, ebufp);

	/***********************************************************
	 * For completeness, dump the results flist if debugging is enabled
	 * and we actually have a results flist.
	 ***********************************************************/
	if(*r_flistpp != (pin_flist_t *)NULL) {
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_get_db_list output flist:",
			*r_flistpp);
	}

	return;
}

static void
fm_cust_pol_get_db_list(
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	poid_t			*pdp = NULL;
        pin_flist_t		*db_flistp = NULL;
        int32           	err = 0;

	/***********************************************************
	 * If there are pending errors, then short circuit immediately
	 ***********************************************************/
        if (PIN_ERR_IS_ERR(ebufp))
                return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Get the database object from the cache.
	 ***********************************************************/
	*r_flistpp = fm_cust_pol_get_db_no_cache_entry();
	if (*r_flistpp == (pin_flist_t *)NULL) {
		/* Entry not found, therefore this is a single-db system */
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		pdp = PIN_POID_CREATE(0, "", 0, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, (void *)pdp, ebufp);
		return;
	} 

	/***********************************************************
	 * We return the object as is.  As a customization you may
	 * wish to restrict which databases should be in the return
	 * FList.
	 ***********************************************************/
	return;
}
