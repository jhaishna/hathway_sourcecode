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
static const char Sccs_id[] = "@(#) %full_filespec: fm_mso_ntf_init.c~2:csrc:1 %";
#endif

#include <stdio.h>
#include <string.h>

#include <pcm.h>
#include <pinlog.h>

#define FILE_LOGNAME "fm_mso_ntf_init.c(1)"

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
EXPORT_OP void fm_mso_ntf_init(int32 *errp);

PIN_EXPORT cm_cache_t	*mso_prov_bb_config_ptr = (cm_cache_t*) 0;
//PIN_EXPORT cm_cache_t	*mso_prov_bb_cmts_ptr = (cm_cache_t*) 0;

void tokensplit(char *str, int32 config[]);

static void
fm_mso_ntf_bb_config_cache(
	pcm_context_t *ctxp,
	pin_errbuf_t  *ebufp);



PIN_EXPORT int32	sms_config[20];
PIN_EXPORT int32	email_config[20];

/***********************************************************************
 *fm_mso_ntf_init: One time initialization for fm_bill_pol
 ***********************************************************************/
void fm_mso_ntf_init(int32 *errp)
{
        pcm_context_t   *ctxp = NULL;
        pin_errbuf_t    ebuf;
        int             err;
		int32			errp1 = 0;
		char			*str = NULL;
        /***********************************************************
         * open the context.
         ***********************************************************/
        PIN_ERRBUF_CLEAR(&ebuf);
        PCM_CONTEXT_OPEN(&ctxp, (pin_flist_t *)0, &ebuf);

        if(PIN_ERRBUF_IS_ERR(&ebuf)) {
                pin_set_err(&ebuf, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_DM_CONNECT_FAILED, 0, 0, ebuf.pin_err);
                PIN_FLIST_LOG_ERR("fm_mso_ntf_init pcm_context_open err", 
				&ebuf);

                *errp = PIN_ERR_DM_CONNECT_FAILED;
                return;
        }

	// MSO Customization Start
	/* Get the SMS Notification Config from the pin.conf */
	pin_conf("fm_mso_ntf", "enable_sms_notif", PIN_FLDT_STR, (caddr_t*)&str, &errp1);

	if (errp1 != PIN_ERR_NONE) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"SMS Notification Params config error!");
		/*****/
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "SMS Notification config!");
		tokensplit(str, sms_config);
	}
	if (str != NULL) {
		free(str);
	}

//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char*)sms_config);

	/* Get the EMAIL Notification Config from the pin.conf */
	pin_conf("fm_mso_ntf", "enable_email_notif", PIN_FLDT_STR, (caddr_t*)&str, &errp1);

	if (errp1 != PIN_ERR_NONE) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, 
			"EMAIL Notification Params config error!");
		/*****/
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "EMAIL Notification config!");
		tokensplit(str, email_config);
	}

	if (str != NULL) {
		free(str);
	}

	fm_mso_ntf_bb_config_cache(ctxp, &ebuf);
	if (PIN_ERRBUF_IS_ERR(&ebuf)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_ntf_bb_config_cache load error ", &ebuf);
	}


//	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char*)email_config);
// MSO Customization End


        PCM_CONTEXT_CLOSE(ctxp, 0, &ebuf);
        *errp = ebuf.pin_err;
        return;
}


void tokensplit(char *str, int32 config[20])
{

char *p    = strtok (str, ",");
int n_spaces = 0, i;
char msg[20];
//int32 config[20];
/* split string and append tokens to 'res' */
i = 1;

while (i<16) {
//  res = realloc (res, sizeof (char*) * ++n_spaces);

//  if (res == NULL)
//    exit (-1); /* memory allocation failed */

//  res[n_spaces-1] = p;
	p = strtok (NULL, ",");  
	sprintf (msg, "config[%d] = %s\n", i, (char *)p);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

	if (strcmp((char*)p,"0")==0)
	{
		config[i] = 0;
	}
	else if (strcmp((char*)p,"1")==0)
	{
		config[i] = 1;
	}
	else if(strcmp((char*)p,"2")==0)
	{
		config[i] = 2;
	}
	else if(strcmp((char*)p,"3")==0)
	{
		config[i] = 3;
	}
i++;

}

/* realloc one extra element for the last NULL */

/*res = realloc (res, sizeof (char*) * (n_spaces+1));
res[n_spaces] = 0;


for (i = 1; i < (n_spaces); ++i)
	{
		sprintf (msg, "res[%d] = %s\n", i, (char *)res[i]);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
		if (strcmp((char*)res[i],"0")==0)
		{
			config[i] = 0;
		}
		else
		{
			config[i] = 1;
		}
	}
*/

/* free the memory allocated */
/*if (res)
{
	free (res);
}*/
}

/**********************************************************************
* Read the config objects /mso_cfg_bb_pt into cm cache 
**********************************************************************/
static void
fm_mso_ntf_bb_config_cache(
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
                        "fm_mso_ntf_bb_config_cache: error loading /mso_config_bb_pt object",
						ebufp);
             goto cleanup;
     }

	cnt = PIN_FLIST_COUNT(r_flistp, ebufp);

	if (cnt <= 1) {
			goto cleanup;
	}

	PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry res_flist", res_flistp)

	/*
	 * Get the number of items in the flist so we can create an
	 * appropriately sized cache
	 */
	cnt  = 0;
	if (r_flistp) {
		cnt = PIN_FLIST_COUNT(r_flistp, ebufp);
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
	mso_prov_bb_config_ptr = cm_cache_init("mso_ntf_bb_config_object", no_of_buckets,
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

	if (!mso_prov_bb_config_ptr) 
	{
		goto cleanup;
	}

	kids.id  = 0; 
	kids.db  = 0;
	kids.str = "/mso_config_bb_pt";
	cm_cache_add_entry(mso_prov_bb_config_ptr, &kids, r_flistp, &err);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_ntf_bb_config_object cache error", ebufp);
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_ntf_bb_config_object cache loaded successfully");
	}
	
 cleanup:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
		PIN_POID_DESTROY(pdp, NULL);
}

///**********************************************************************
//* Read the config objects /mso_cfg_cmts_master into cm cache 
//**********************************************************************/
//static void
//fm_mso_prov_bb_cmts_cache(
//	pcm_context_t *ctxp,
//	pin_errbuf_t  *ebufp)
//{
//	poid_t                  *s_pdp = NULL;
//	poid_t			*pdp = NULL;
//	pin_flist_t             *s_flistp = NULL;
//	pin_flist_t             *r_flistp = NULL;
//	pin_flist_t             *res_flistp = NULL;
//	pin_flist_t             *arg_flistp = NULL;
//	int32                   err = PIN_ERR_NONE;
//	int32                   cnt;
//	int64			database;
//	cm_cache_key_iddbstr_t  kids;
//	int32                   s_flags = 256;
//	int			no_of_buckets = 1;
//
//		
//	s_flistp = PIN_FLIST_CREATE(ebufp);
//
//	/***********************************************************
//	 * assume db matches userid found in pin.conf
//	 ***********************************************************/
//	pdp = PCM_GET_USERID(ctxp);
//	database = PIN_POID_GET_DB(pdp);
//
//	s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
//	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
//	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
//	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, 
//			"select X from /mso_cfg_cmts_master where F1.db = V1 ", ebufp);
//	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
//	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
//
//	PIN_FLIST_ELEM_SET( s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
//                        "cm_cache_find_entry input flist", s_flistp);
//	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
//                        "cm_cache_find_entry output flist", r_flistp)
//	if (PIN_ERRBUF_IS_ERR(ebufp)) {
//             PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//                        "fm_mso_prov_bb_cmts_cache: error loading /mso_cfg_cmts_master object",
//						ebufp);
//             goto cleanup;
//     }
//
//	cnt = PIN_FLIST_COUNT(r_flistp, ebufp);
//
//	if (cnt <= 1) {
//			goto cleanup;
//	}
//
//	PIN_FLIST_FLD_DROP(r_flistp, PIN_FLD_POID, ebufp);
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "cm_cache_find_entry res_flist", res_flistp)
//
//	/*
//	 * Get the number of items in the flist so we can create an
//	 * appropriately sized cache
//	 */
//	cnt  = 0;
//	if (r_flistp) {
//		cnt = PIN_FLIST_COUNT(r_flistp, ebufp);
//	}
//
//	/*
//	 * If there's no data, there's no point initializing it, is there.
//	 * Especially since this is optional!
//	 */
//	if (cnt < 1) {
//		goto cleanup;
//	}
//
//	/*
//	 * This cache doesn't need configuration since the sizes of the
//	 * entries are computed
//	 */
//	mso_prov_bb_cmts_ptr = cm_cache_init("mso_prov_bb_cmts_object", no_of_buckets,
//									cnt * 2 * 1024, 1024, CM_CACHE_KEY_IDDBSTR, &err);
//
//	if (err != PIN_ERR_NONE) 
//	{
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error: Couldn't initialize "
//				"cache in fm_mso_prov_bb_cmts_cache");
//		pin_set_err(ebufp, PIN_ERRLOC_FM, 
//				PIN_ERRCLASS_SYSTEM_DETERMINATE,
//				PIN_ERR_NO_MEM, 0, 0, err);
//		goto cleanup;
//	}
//
//	if (!mso_prov_bb_cmts_ptr) 
//	{
//		goto cleanup;
//	}
//
//	kids.id  = 0; 
//	kids.db  = 0;
//	kids.str = "/mso_cfg_cmts_master";
//	cm_cache_add_entry(mso_prov_bb_cmts_ptr, &kids, r_flistp, &err);
//
//	if (PIN_ERRBUF_IS_ERR(ebufp)) 
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_prov_bb_cmts_object cache error", ebufp);
//	}
//	else 
//	{
//		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_prov_bb_cmts_object cache loaded successfully");
//	}
//	
// cleanup:
//        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
//        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
//        PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
//	PIN_POID_DESTROY(pdp, NULL);
//}
