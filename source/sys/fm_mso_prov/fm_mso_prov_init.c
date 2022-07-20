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
static const char Sccs_id[] = "@(#) %full_filespec: fm_mso_prov_init.c~2:csrc:1 %";
#endif


#define FILE_LOGNAME "fm_mso_prov_init.c(1)"

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
#include "mso_ops_flds.h"


/***********************************************************************
 *Forward declaration
 ***********************************************************************/
EXPORT_OP void fm_mso_prov_init(int32 *errp);


/*******************************************************************
 * Globals for /mso_cfg_catv_pt objects in memory/cache stored objects.
 *******************************************************************/
PIN_EXPORT cm_cache_t	*mso_prov_catv_config_ptr = (cm_cache_t*) 0; 
PIN_EXPORT cm_cache_t	*mso_prov_bb_config_ptr = (cm_cache_t*) 0; 
PIN_EXPORT cm_cache_t	*mso_prov_bb_cmts_ptr = (cm_cache_t*) 0;
PIN_EXPORT cm_cache_t   *mso_prov_cap_plans_ptr = (cm_cache_t*) 0;
PIN_EXPORT cm_cache_t   *mso_cfg_catv_channel_master_config_ptr = (cm_cache_t*) 0;
//Caching
// Increasing caching interval to 21000 from 3600
PIN_EXPORT int32 refresh_pt_interval = 36000;

PIN_EXPORT int32 *fup_capp_flag = NULL;


static void
fm_mso_prov_catv_config_cache(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp);

static void
fm_mso_prov_bb_cmts_cache(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp);

static void
fm_mso_prov_bb_config_cache(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp);

static void
fm_mso_prov_cap_plans_cache(
	pcm_context_t *ctxp,
        pin_errbuf_t  *ebufp);

PIN_EXPORT void
fm_mso_catv_channel_master_config_cache(
        pcm_context_t *ctxp,
        pin_errbuf_t  *ebufp);

/***********************************************************************
 *fm_mso_prov_init: One time initialization for mso_prov
 ***********************************************************************/
void fm_mso_prov_init(int32 *errp)
{
        pcm_context_t   *ctxp = NULL;
        pin_errbuf_t    ebuf;
        int             err;
	int		*flagp = NULL;
	int		perr;
 
        /***********************************************************
         * open the context.
         ***********************************************************/
        PIN_ERRBUF_CLEAR(&ebuf);
        PCM_CONTEXT_OPEN(&ctxp, (pin_flist_t *)0, &ebuf);

        if(PIN_ERRBUF_IS_ERR(&ebuf)) {
                pin_set_err(&ebuf, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_DM_CONNECT_FAILED, 0, 0, ebuf.pin_err);
                PIN_FLIST_LOG_ERR("fm_mso_prov_init pcm_context_open err", 
				&ebuf);

                *errp = PIN_ERR_DM_CONNECT_FAILED;
                return;
        }
	
	/*
	 * Get the configuration data: refresh_product_interval
	 */
	pin_conf("fm_rate", "refresh_pt_interval", PIN_FLDT_INT,
		(caddr_t *)&(flagp), &perr);

	if (flagp) {
		refresh_pt_interval = *flagp;
		free(flagp);
	} else {
		refresh_pt_interval = 3600;
	}
	
	/*added for fup cap mobe enable and disable*/
	pin_conf("fm_mso_prov", "fup_cap_flag", PIN_FLDT_INT, (caddr_t*)&fup_capp_flag, &perr);
	/* Cache the /mso_cfg_catv_pt object */
	fm_mso_prov_catv_config_cache(ctxp, &ebuf);
	if (PIN_ERRBUF_IS_ERR(&ebuf)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_prov_catv_config_cache load error ", &ebuf);
	}

        fm_mso_catv_channel_master_config_cache(ctxp, &ebuf);
        if (PIN_ERRBUF_IS_ERR(&ebuf)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_prov_catv_config_cache load error ", &ebuf);
        }

	fm_mso_prov_bb_cmts_cache(ctxp, &ebuf);
	if (PIN_ERRBUF_IS_ERR(&ebuf)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_prov_bb_cmts_cache load error ", &ebuf);
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Back in main");
	/* Cache the /mso_config_bb_pt object */
	fm_mso_prov_bb_config_cache(ctxp, &ebuf);
	if (PIN_ERRBUF_IS_ERR(&ebuf)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_prov_bb_config_cache load error ", &ebuf);
	}

	fm_mso_prov_cap_plans_cache(ctxp, &ebuf);
	if (PIN_ERRBUF_IS_ERR(&ebuf)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_prov_cap_plans_cache load error", &ebuf);
	}

        PCM_CONTEXT_CLOSE(ctxp, 0, &ebuf);
        *errp = ebuf.pin_err;
        return;
}


/**********************************************************************
* Read the config objects /mso_cfg_catv_pt into cm cache 
**********************************************************************/
static void
fm_mso_prov_catv_config_cache(
	pcm_context_t		*ctxp,
	pin_errbuf_t		*ebufp)
{
	poid_t			*s_pdp = NULL;
	poid_t			*pdp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	int32			err = PIN_ERR_NONE;
	int32			cnt;
	int64			database;
	cm_cache_key_iddbstr_t	kids;
	int32			s_flags = 256;
	int			no_of_buckets = 1;
	int32			elem_count = 0;
	time_t			now = 0;

		
	s_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, 
			"select X from /mso_cfg_catv_pt where F1.db = V1 ", ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

	PIN_FLIST_ELEM_SET( s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry output flist", r_flistp)
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_prov_catv_config_cache: error loading /mso_cfg_catv_pt object",
						ebufp);
             goto cleanup;
     }

	cnt = PIN_FLIST_COUNT(r_flistp, ebufp);

	if (cnt <= 1) {
			goto cleanup;
	}

	PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp);
	//Caching
	now = pin_virtual_time((time_t *)NULL);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_MOD_T,&now,ebufp);
	//
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry res_flist", r_flistp)

	/*
	 * Get the number of items in the flist so we can create an
	 * appropriately sized cache
	 */
	cnt  = 0;
	if (r_flistp) {
		elem_count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp);
		res_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, PIN_ELEMID_ANY, ebufp);
		cnt = PIN_FLIST_COUNT(res_flistp, ebufp);
		cnt = cnt * elem_count;
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
	mso_prov_catv_config_ptr = cm_cache_init("mso_prov_catv_config_object", no_of_buckets,
									cnt * 2 * 1024, 1024, CM_CACHE_KEY_IDDBSTR, &err);

	if (err != PIN_ERR_NONE) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error: Couldn't initialize "
				"cache in fm_mso_prov_catv_config_cache");
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, err);
		goto cleanup;
	}

	if (!mso_prov_catv_config_ptr) 
	{
		goto cleanup;
	}

	kids.id  = 0; 
	kids.db  = 0;
	kids.str = "/mso_cfg_catv_pt";
	cm_cache_add_entry(mso_prov_catv_config_ptr, &kids, r_flistp, &err);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_prov_catv_config_object cache error", ebufp);
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_prov_catv_config_object cache loaded successfully");
	}
	
 cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        //PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
	PIN_POID_DESTROY(pdp, NULL);
	return;
}

/**********************************************************************
* Read the config objects /mso_cfg_bb_pt into cm cache 
**********************************************************************/
static void
fm_mso_prov_bb_config_cache(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp)
{
	poid_t                  *s_pdp = NULL;
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
	int32                   elem_count = 0;
	int			no_of_buckets = 1;
	time_t			now = 0;

		
	s_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, 
			"select X from /mso_config_bb_pt where F1.db = V1 ", ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

	PIN_FLIST_ELEM_SET( s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry output flist", r_flistp)
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_prov_bb_config_cache: error loading /mso_config_bb_pt object",
						ebufp);
             goto cleanup;
     }

	cnt = PIN_FLIST_COUNT(r_flistp, ebufp);

	if (cnt <= 1) {
			goto cleanup;
	}

	PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp);
	//Caching
	now = pin_virtual_time((time_t *)NULL);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_MOD_T,&now,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry res_flist", r_flistp)

	/*
	 * Get the number of items in the flist so we can create an
	 * appropriately sized cache
	 */
	cnt  = 0;
	if (r_flistp) {
		elem_count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp);
		res_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, PIN_ELEMID_ANY, ebufp);
		cnt = PIN_FLIST_COUNT(res_flistp, ebufp);
		cnt = cnt * elem_count;
	}
	//if (r_flistp) {
	//	cnt = PIN_FLIST_COUNT(r_flistp, ebufp);
	//}

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
	mso_prov_bb_config_ptr = cm_cache_init("mso_prov_bb_config_object", no_of_buckets,
									cnt * 2 * 1024, 1024, CM_CACHE_KEY_IDDBSTR, &err);

	if (err != PIN_ERR_NONE) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error: Couldn't initialize "
				"cache in fm_mso_prov_bb_config_cache");
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, err);
		goto cleanup;
	}

	if (!mso_prov_catv_config_ptr) 
	{
		goto cleanup;
	}

	kids.id  = 0; 
	kids.db  = 0;
	kids.str = "/mso_config_bb_pt";
	cm_cache_add_entry(mso_prov_bb_config_ptr, &kids, r_flistp, &err);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_prov_bb_config_object cache error", ebufp);
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_prov_bb_config_object cache loaded successfully");
	}
	
 cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        //PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
	PIN_POID_DESTROY(pdp, NULL);
}

/**********************************************************************
* Read the config objects /mso_cfg_cmts_master into cm cache 
**********************************************************************/
static void
fm_mso_prov_bb_cmts_cache(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp)
{
	poid_t                  *s_pdp = NULL;
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
	char			msg[250];
	int32			elem_count = 0;
	time_t			now = 0;
	

		
	s_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * assume db matches userid found in pin.conf
	 ***********************************************************/
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, 
			"select X from /mso_cfg_cmts_master where F1.db = V1 ", ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

	PIN_FLIST_ELEM_SET( s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry output flist", r_flistp)
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_prov_bb_cmts_cache: error loading /mso_cfg_cmts_master object",
						ebufp);
             goto cleanup;
     }

	cnt = PIN_FLIST_COUNT(r_flistp, ebufp);

	if (cnt <= 1) {
			goto cleanup;
	}

	PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp);
	//Caching
	now = pin_virtual_time((time_t *)NULL);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_MOD_T,&now,ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry res_flist", r_flistp)

	/*
	 * Get the number of items in the flist so we can create an
	 * appropriately sized cache
	 */
	cnt  = 0;
	if (r_flistp) {
		elem_count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp);
		res_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, PIN_ELEMID_ANY, ebufp);
		cnt = PIN_FLIST_COUNT(res_flistp, ebufp);
		cnt = cnt * elem_count;
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
	sprintf(msg, " Size is %d", (cnt * 2 * 1024));
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	mso_prov_bb_cmts_ptr = cm_cache_init("mso_prov_bb_cmts_object", no_of_buckets,
									cnt * 2 * 1024, 1024, CM_CACHE_KEY_IDDBSTR, &err);

	if (err != PIN_ERR_NONE) 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error: Couldn't initialize "
				"cache in fm_mso_prov_bb_cmts_cache");
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_NO_MEM, 0, 0, err);
		goto cleanup;
	}

	if (!mso_prov_bb_cmts_ptr) 
	{
		goto cleanup;
	}

	kids.id  = 0; 
	kids.db  = 0;
	kids.str = "/mso_cfg_cmts_master";
	cm_cache_add_entry(mso_prov_bb_cmts_ptr, &kids, r_flistp, &err);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_prov_bb_cmts_object cache error", ebufp);
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_prov_bb_cmts_object cache loaded successfully");
	}
	
 cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        //PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
	PIN_POID_DESTROY(pdp, NULL);
}

static void
fm_mso_prov_cap_plans_cache(
	pcm_context_t	*ctxp,
	pin_errbuf_t	*ebufp)
{
	poid_t		*s_pdp = NULL;
	poid_t		*pdp = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*arg_flistp = NULL;
	int32		err = PIN_ERR_NONE;
	int32		cnt;
	int64		database;
	cm_cache_key_iddbstr_t	kids;
	int32		s_flags = 256;
	int			no_of_buckets = 1;
	char		msg[250];
	int32		elem_count = 0;
	time_t		now = 0;

	s_flistp = PIN_FLIST_CREATE(ebufp);
	pdp = PCM_GET_USERID(ctxp);
	database = PIN_POID_GET_DB(pdp);

	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_cfg_cap_plans where F1.db = V1 ", ebufp);
	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

	PIN_FLIST_ELEM_SET( s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry mso_cfg_cap_plans input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry mso_cfg_cap_plans output flist", r_flistp)
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_cfg_cap_plans: error loading /mso_cfg_cap_plans object", ebufp);
		goto cleanup;
	}

	cnt = PIN_FLIST_COUNT(r_flistp, ebufp);

	if (cnt <= 1) {
		goto cleanup;
	}

	PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp);
	//Caching
	now = pin_virtual_time((time_t *)NULL);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_MOD_T, &now, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry  mso_cfg_cap_plans res_flist", r_flistp)
	cnt = 0;
	if (r_flistp) {
		elem_count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp);
		res_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, PIN_ELEMID_ANY, ebufp);
		cnt = PIN_FLIST_COUNT(res_flistp, ebufp);
		cnt = cnt * elem_count;
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

	sprintf(msg, " Size is %d", (cnt * 2 * 1024));
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	mso_prov_cap_plans_ptr = cm_cache_init("mso_cfg_cap_plans", no_of_buckets, cnt * 2 * 1024, 1024, CM_CACHE_KEY_IDDBSTR, &err);

	if (err != PIN_ERR_NONE){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error: Couldn't initialize cache in mso_cfg_cap_plans");
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0, err);
		goto cleanup;
	}

	if (!mso_prov_cap_plans_ptr)
	{
		goto cleanup;
	}

	kids.id = 0;
	kids.db = 0;
	kids.str = "/mso_cfg_cap_plans";
	cm_cache_add_entry(mso_prov_cap_plans_ptr, &kids, r_flistp, &err);

	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_cfg_cap_plans cache error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_cfg_cap_plans cache loaded successfully");
	}

cleanup:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_POID_DESTROY(pdp, NULL);
}

/**********************************************************************
* Read the config objects /mso_cfg_catv_channel_master into cm cache
**********************************************************************/
void
fm_mso_catv_channel_master_config_cache(
        pcm_context_t           *ctxp,
        pin_errbuf_t            *ebufp)
{
        poid_t                  *s_pdp = NULL;
        poid_t                  *pdp = NULL;
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        pin_flist_t             *arg_flistp = NULL;
        int32                   err = PIN_ERR_NONE;
        int32                   cnt;
        int64                   database;
        cm_cache_key_iddbstr_t  kids;
        int32                   s_flags = 256;
        int                     no_of_buckets = 1;
        int32                   elem_count = 0;
        time_t                  now = 0;

        s_flistp = PIN_FLIST_CREATE(ebufp);

        /***********************************************************
         * assume db matches userid found in pin.conf
         ***********************************************************/
        pdp = PCM_GET_USERID(ctxp);
        database = PIN_POID_GET_DB(pdp);

        s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,
                        "select X from /mso_cfg_catv_channel_master where F1.db = V1 ", ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_BROADCASTER, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_CHANNEL_GENRE, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_CHANNEL_ID, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_CHANNEL_NAME, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_CHANNEL_TYPE, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_LANGUAGE, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_POPULARITY, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_PRICE, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_QUALITY, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, MSO_FLD_TRANSMISSION, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_DISTRIBUTOR, NULL, ebufp);
        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_TYPE, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "cm_cache_find_entry output flist", r_flistp)

        if (PIN_ERRBUF_IS_ERR(ebufp)) {
             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_prov_catv_config_cache: error loading /mso_cfg_catv_channel_master object",
                                                ebufp);
             goto cleanup;
     	}

        cnt = PIN_FLIST_COUNT(r_flistp, ebufp);

        if (cnt <= 1) {
                        goto cleanup;
        }

        PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp);
        //Caching
        now = pin_virtual_time((time_t *)NULL);
        PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_MOD_T,&now,ebufp);
        //
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry res_flist", r_flistp)

        /*
         * Get the number of items in the flist so we can create an
         * appropriately sized cache
         */
        cnt  = 0;
        if (r_flistp) {
                elem_count = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp);
                res_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, PIN_ELEMID_ANY, ebufp);
                cnt = PIN_FLIST_COUNT(res_flistp, ebufp);
                cnt = cnt * elem_count;
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
        mso_cfg_catv_channel_master_config_ptr = cm_cache_init("mso_cfg_channel_master_object", no_of_buckets,
                                                                        cnt * 2 * 1024, 1024, CM_CACHE_KEY_IDDBSTR, &err);

        if (err != PIN_ERR_NONE)
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error: Couldn't initialize "
                                "cache in fm_mso_prov_catv_config_cache");
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                PIN_ERR_NO_MEM, 0, 0, err);
                goto cleanup;
        }

        if (!mso_cfg_catv_channel_master_config_ptr)
        {
                goto cleanup;
        }

        kids.id  = 0;
        kids.db  = 0;
        kids.str = "/mso_cfg_catv_channel_master";
        cm_cache_add_entry(mso_cfg_catv_channel_master_config_ptr, &kids, r_flistp, &err);

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_cfg_channel_master_object cache error", ebufp);
        }
        else
        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_cfg_channel_master_object cache loaded successfully");
        }

		cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_POID_DESTROY(pdp, NULL);
        return;
}
