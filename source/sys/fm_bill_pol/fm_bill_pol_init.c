/*
 *
 *      Copyright (c) 2004 - 2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static const char Sccs_id[] = "@(#) %full_filespec: fm_bill_pol_init.c~2:csrc:1 %";
#endif

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#define FILE_LOGNAME "fm_bill_pol_init.c(1)"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pcm.h"
#include "ops/cust.h"
#include "pinlog.h"
#include "pin_errs.h"
#include "cm_fm.h"
#include "cm_cache.h"


/***********************************************************************
 *Forward declaration
 ***********************************************************************/
EXPORT_OP void fm_bill_pol_init(int32 *errp);

PIN_EXPORT char		*fm_bill_pol_dd_vendor = NULL;

// MSO Customization Start
PIN_EXPORT pin_decimal_t		*min_amount = NULL;
PIN_EXPORT int32				*min_commit_flag = NULL;

PIN_EXPORT pin_decimal_t		*st_tax_rate = NULL;
PIN_EXPORT pin_decimal_t		*ecess_tax_rate = NULL;
PIN_EXPORT pin_decimal_t		*shecess_tax_rate = NULL;
PIN_EXPORT pin_decimal_t                *swachh_tax_rate = NULL;
PIN_EXPORT pin_decimal_t                *krish_tax_rate = NULL;

// MSO Customization End
/*******************************************************************
 * Globals for /config/suppress objects in memory/cache stored objects.
 *******************************************************************/
PIN_EXPORT cm_cache_t	*fm_bill_suppression_config_ptr = (cm_cache_t*) 0; 

static void
fm_bill_pol_config_suppression_cache(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp);


/***********************************************************************
 *fm_bill_pol_init: One time initialization for fm_bill_pol
 ***********************************************************************/
void fm_bill_pol_init(int32 *errp)
{
        pcm_context_t   *ctxp = NULL;
        pin_errbuf_t    ebuf;
        int             err;
		int32			errp1 = 0;
        /***********************************************************
         * open the context.
         ***********************************************************/
        PIN_ERR_CLEAR_ERR(&ebuf);
        PCM_CONTEXT_OPEN(&ctxp, (pin_flist_t *)0, &ebuf);

        if(PIN_ERR_IS_ERR(&ebuf)) {
                pin_set_err(&ebuf, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_DM_CONNECT_FAILED, 0, 0, ebuf.pin_err);
                PIN_FLIST_LOG_ERR("fm_bill_pol_init pcm_context_open err", 
				&ebuf);

                *errp = PIN_ERR_DM_CONNECT_FAILED;
                return;
        }

	pin_conf("cm", "dd_vendor", PIN_FLDT_STR, &fm_bill_pol_dd_vendor, &err);

	// MSO Customization Start
	/* Get the min_commitment flag from the pin.conf */
	pin_conf("fm_bill_pol", "min_commitment", PIN_FLDT_INT, (caddr_t*)&min_commit_flag, &errp1);

//	min_amount = pbo_decimal_from_str("0.0",ebufp);
	pin_conf("fm_bill_pol", "min_commitment_amount", PIN_FLDT_DECIMAL, (caddr_t*)&min_amount, &errp1);

	if (errp1 != PIN_ERR_NONE) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"Minimum Commitment config error!");
		/*****/
	}

	// Reading the Service Tax, Edu Cess and SHE Cess from pin.conf
	pin_conf("fm_mso_rate", "service_tax", PIN_FLDT_DECIMAL, (caddr_t*)&st_tax_rate, &errp1);
	pin_conf("fm_mso_rate", "edu_cess", PIN_FLDT_DECIMAL, (caddr_t*)&ecess_tax_rate, &errp1);
	pin_conf("fm_mso_rate", "she_cess", PIN_FLDT_DECIMAL, (caddr_t*)&shecess_tax_rate, &errp1);
	pin_conf("fm_mso_rate", "swachh_cess", PIN_FLDT_DECIMAL, (caddr_t*)&swachh_tax_rate, &errp1);
	pin_conf("fm_mso_rate", "krish_cess", PIN_FLDT_DECIMAL, (caddr_t*)&krish_tax_rate, &errp1);
	if (errp1 != PIN_ERR_NONE) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"Service Taxes config error!");
		/*****/
	}

	// MSO Customization End
	/* Cache the /config/suppression config object */
	fm_bill_pol_config_suppression_cache(ctxp, &ebuf);
	if (PIN_ERR_IS_ERR(&ebuf)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, 
		"fm_bill_pol_config_suppression_cache load error ", &ebuf);
	}

        PCM_CONTEXT_CLOSE(ctxp, 0, &ebuf);
        *errp = ebuf.pin_err;
        return;
}


/**********************************************************************
* Read the config objects /config/suppression into cm cache 
**********************************************************************/
static void
fm_bill_pol_config_suppression_cache(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp)
{
	poid_t                  *s_pdp = NULL;
	poid_t                  *a_pdp = NULL;
	poid_t			*pdp = NULL;
	pin_flist_t             *s_flistp = NULL;
	pin_flist_t             *r_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *arg_flistp = NULL;

	int32                   err = PIN_ERR_NONE;
	int32                   cnt;
	int64			database;
	cm_cache_key_iddbstr_t  kids;
	int32                   s_flags = 256;
	int			no_of_buckets = 1;
	
	s_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * HACK: we need a db number. this will eventually
	 * be handled by the 'get beid object' method from
	 * likely the rating FM. But for now, assume db
	 * matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);
	PIN_POID_DESTROY(pdp, NULL);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, 
			"select X from /config where F1 = V1 ", ebufp);

	/* setup arguments */
	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);

	a_pdp = PIN_POID_CREATE(database, "/config/suppression", -1, ebufp);
	PIN_FLIST_FLD_PUT(arg_flistp, PIN_FLD_POID, (void *)a_pdp, ebufp);
	PIN_FLIST_ELEM_SET( s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_bill_config_suppression_object_from_db: "
			"error loading /config/suppression object",
                        ebufp);

                goto cleanup;
        }
	cnt = PIN_FLIST_COUNT(r_flistp, ebufp);

	if (cnt <= 1) {
		goto cleanup;
	}
	res_flistp = PIN_FLIST_ELEM_TAKE(r_flistp, PIN_FLD_RESULTS, 
					PIN_ELEMID_ANY, 1, ebufp);

	/*
	 * Get the number of items in the flist so we can create an
	 * appropriately sized cache
	 */
	cnt  = 0;
	if (res_flistp) {
		cnt = PIN_FLIST_COUNT(res_flistp, ebufp);
	}

	/*
	 * If there's no data, there's no point initializing it, is there.
	 * Especially since this is optional!
	 */
	if (cnt < 1) {
		goto cleanup;
	}
	
	/*
	 * This cache doesn't need configuration since the sizes of the
	 * entries are computed
	 */
	fm_bill_suppression_config_ptr =
		cm_cache_init("fm_bill_config_suppression_object", no_of_buckets,
				  cnt * 1024, 8, CM_CACHE_KEY_IDDBSTR, &err);

	if (err != PIN_ERR_NONE) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error: Couldn't initialize "
		"cache in fm_bill_pol_config_suppression_cache");
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, err);
		goto cleanup;
	}

	if (!fm_bill_suppression_config_ptr) {
		goto cleanup;
	}

	kids.id  = 0; 
	kids.db  = 0;
	kids.str = "/config/suppression";
	cm_cache_add_entry(fm_bill_suppression_config_ptr, 
				&kids, res_flistp, &err);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"fm_bill_config_suppression_object cache error",
				 ebufp);
	}
	else {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
			"config_suppression_object cache loaded successfully");
	}
	
 cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
}

