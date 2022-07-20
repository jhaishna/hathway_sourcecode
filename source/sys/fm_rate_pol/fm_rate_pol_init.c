/***********************************************************************
 *
 *      Copyright (c) 2000-2006 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 ***********************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_rate_pol_init.c:BillingVelocityInt:3:2006-Sep-05 04:30:40 %";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pcm.h"
#include "pinlog.h"
#include "pin_errs.h"
#include "cm_fm.h"
#include "mso_rate.h"
#include "cm_cache.h"

/*******************************************************************
 * Global for holding credit threshold configuration
 *******************************************************************/

PIN_EXPORT cm_cache_t *fm_rate_pol_tax_codes_ptr = (cm_cache_t *)0;
PIN_EXPORT credit_threshold_config_t credit_thresholds[10];
PIN_EXPORT int num_of_resources_configured;

void split_token(char *str, credit_threshold_config_t credit_thresholds[10]);

void parse_credit_threshold_config(char p[10][50],int num_of_resources_configured, credit_threshold_config_t credit_thresholds[20]);

extern void
fm_rate_pol_init_tax_codes_cache(
        pcm_context_t   *ctxp,
        int64           database,
        pin_errbuf_t    *ebufp);

/***********************************************************************
 *Forward declaration
 ***********************************************************************/
EXPORT_OP void fm_rate_pol_init(int32 *errp);
char debug_msg[250];

/*******************************************************************
 * Routines defined elsewere.
 *******************************************************************/
/***********************************************************************
 *fm_rate_pol_init: One time initialization for fm_rate_pol
 ***********************************************************************/

void
fm_rate_pol_init(int32 *errp)
{
    	int32                   errp1 = 0;
    	char                    *str = NULL;	
    	pin_flist_t		*bal_in_flistp = NULL;
	pcm_context_t   *ctxp = NULL;
	poid_t          *pdp = NULL;
	int64           database;
	pin_errbuf_t    ebuf;
	int             err;
	
    pin_conf("fm_mso", "credit_threshold", PIN_FLDT_STR, (caddr_t*)&str, &errp1);    

    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Credit Threshold configuration found is ");
    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,str);
 
    /***********************************************************
    * 
    * 
    ***********************************************************/
    	split_token(str, credit_thresholds);

	PIN_ERRBUF_CLEAR(&ebuf);
	PIN_ERR_LOG_MSG(3, "Check 1");
	PCM_CONTEXT_OPEN(&ctxp, (pin_flist_t *)0, &ebuf);
	PIN_ERR_LOG_MSG(3, "Check 2");
        if(PIN_ERRBUF_IS_ERR(&ebuf)) {
		PIN_ERR_LOG_MSG(3, "Check Error");
                pin_set_err(&ebuf, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_DM_CONNECT_FAILED, 0, 0, ebuf.pin_err);
                PIN_FLIST_LOG_ERR("fm_rate_pol_init pcm_context_open err",
                                &ebuf);
                *errp = PIN_ERR_DM_CONNECT_FAILED;
                return;
        }
	pdp = PCM_GET_USERID(ctxp);
	PIN_ERR_LOG_MSG(3, "Check 3");
        database = PIN_POID_GET_DB(pdp);
	PIN_ERR_LOG_MSG(3, "Check 4");
        PIN_POID_DESTROY(pdp, NULL);
	PIN_ERR_LOG_MSG(3, "Check 5");

	fm_rate_pol_init_tax_codes_cache(ctxp, database, &ebuf);

        if(PIN_ERRBUF_IS_ERR(&ebuf)) {
                PIN_FLIST_LOG_ERR("fm_rate_pol_init_tax_codes_cache error",
                        &ebuf);
        }


	PCM_CONTEXT_CLOSE(ctxp, 0, &ebuf);
        *errp = ebuf.pin_err;
    	return;
}

/**
 * 
 * config format will be
 * 
 * - fm_mso credit_threshold 1000009-70,80,90:1000010-70,80,90
 * 
 * 
 */
void split_token(char *str, credit_threshold_config_t credit_thresholds[10])
{
	int   num_of_res_conf = 0;	
	char temp_str[10][50];
	char *p  = strtok (str, ":");		
	while(p != NULL) {		
		//printf(" Split after : is %s\n", p);
		memset(temp_str[num_of_res_conf],0,50);
		memcpy(temp_str[num_of_res_conf++], p, strlen(p));
		sprintf(debug_msg," Temp Str is %s\n", temp_str[num_of_res_conf-1]);
    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		p = strtok(NULL, ":");
	}
	parse_credit_threshold_config(temp_str,num_of_res_conf, credit_thresholds);
	num_of_resources_configured = num_of_res_conf;
}

void parse_credit_threshold_config(char p[10][50],int num_of_res_conf, credit_threshold_config_t credit_thresholds[20]) {
	
	int   num_of_percents_configured = 0;	
	pin_errbuf_t	ebuf;
	int i,j;
	char *c = NULL;
	char temp_str[5];
	PIN_ERRBUF_CLEAR(&ebuf);
	char   *generic_char = NULL;

	for (i = 0 ; i < num_of_res_conf ; i++) {
	c    = strtok (p[i], "-,");	

	if(c != NULL) {
		sprintf(debug_msg," Setting resource id as %s",c);
    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		credit_thresholds[i].resource_id = atoi(c);
	} else {
		return;
	}
	c = strtok(NULL, ",");
	num_of_percents_configured = 0;
	while(c != NULL) {		
		sprintf(debug_msg,"Setting threshold as [%s]",c);
    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		strcpy(temp_str, c);
		sprintf(debug_msg, "trying setting threshold [%d]", atoi(temp_str));
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		credit_thresholds[i].threshold_config[num_of_percents_configured++] = pbo_decimal_from_str(temp_str, &ebuf);
		//sprintf(debug_msg, "after setting threshold is %s",pbo_decimal_to_str(credit_thresholds[i].threshold_config[num_of_percents_configured-1], &ebuf));
		generic_char = pbo_decimal_to_str(credit_thresholds[i].threshold_config[num_of_percents_configured-1], &ebuf);
		sprintf(debug_msg, "after setting threshold is %s",generic_char);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
		c = strtok(NULL, ",");
		if (generic_char) {
                        pin_free(generic_char);
			generic_char = NULL;
                }
	}
	credit_thresholds[i].num_of_config = num_of_percents_configured;
	sprintf(debug_msg,"Credit Threshold parse for this resource complete!\n");
    		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);

	}

	for (i = 0; i < num_of_res_conf ; i++){
		sprintf(debug_msg,"Credit Threshold for %d is:",credit_thresholds[i].resource_id);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
			for(j = 0; j < credit_thresholds[i].num_of_config; j++) {
//				sprintf(debug_msg, "threshold config[%d] is %s", j, pbo_decimal_to_str(credit_thresholds[i].threshold_config[j], &ebuf));
				generic_char = pbo_decimal_to_str(credit_thresholds[i].threshold_config[j], &ebuf);
				sprintf(debug_msg, "threshold config[%d] is %s", j, generic_char);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, debug_msg);
				if (generic_char) {
                        		pin_free(generic_char);
					generic_char = NULL;
                		 }

			}
	}
	
}

void
fm_rate_pol_init_tax_codes_cache(
        pcm_context_t   *ctxp,
        int64           database,
        pin_errbuf_t    *ebufp)
{
        poid_t          *s_pdp = NULL;
        pin_flist_t     *s_flistp = NULL;
        pin_flist_t     *r_flistp = NULL;
        pin_flist_t     *arg_flistp = NULL;
        pin_flist_t     *c_flistp = NULL;

        pin_cookie_t    cookie = NULL;
        int32           err = PIN_ERR_NONE;
        int32           cnt = 0;
        int32           recid;
        int32           s_flags = SRCH_DISTINCT;
        time_t          current_t = 0;

        char            key_str [100];
        char            *tax_code = NULL;

	PIN_ERR_LOG_MSG(3, "Inside tax codes cache");
        current_t = pin_virtual_time(NULL);
        s_flistp = PIN_FLIST_CREATE(ebufp);

        s_pdp = PIN_POID_CREATE(database, "/search", -1, ebufp);
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /mso_cfg_gst_rates where F1.db = V1 and F2 <= V2 and F3 > V3 ", ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_EFFECTIVE_T, &current_t, ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_END_T, &current_t, ebufp);
        PIN_FLIST_ELEM_SET(s_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_cust_pol_init_config_gst_rate_cache search input flist", s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_pol_init_config_gst_rate_cache: error loading /mso_cfg_gst_rates object",ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read mso_cfg_gst_rates", r_flistp);

        /* Bail if there's an error (such as no such object!) */
        if (PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERRBUF_CLEAR(ebufp);
                goto CLEANUP;
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
                goto CLEANUP;
        }

        /*
         * This cache doesn't need configuration since the sizes of the
         * entries are computed
         */
        fm_rate_pol_tax_codes_ptr = cm_cache_init("fm_rate_pol_init_tax_codes_cache", cnt, cnt * 4096, 8, CM_CACHE_KEY_IDDBSTR, &err);

        if (err != PIN_ERR_NONE) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Error: Couldn't initialize tax codes in fm_rate_pol_init_tax_codes_cache");
                pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_NO_MEM, 0, 0, err);
                goto CLEANUP;
        }

        if (!fm_rate_pol_tax_codes_ptr) {
                goto CLEANUP;
        }
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Will read entries to put into cache");
        //PIN_FLIST_ELEM_TAKE(r_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        while (c_flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS, &recid, 1, &cookie, ebufp)) {
                cm_cache_key_iddbstr_t  kids;
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Read entry", c_flistp);
                tax_code = PIN_FLIST_FLD_GET(c_flistp, PIN_FLD_TAX_CODE, 0, ebufp);

                kids.id = 0;    /* Not relevant for us */
                kids.db = 0;    /* Not relevant for us */
                kids.str = tax_code;

                if (kids.str) {
                        cm_cache_add_entry(fm_rate_pol_tax_codes_ptr, &kids, c_flistp, &err);
                        switch(err) {
                        case PIN_ERR_NONE:
                                break;
                        case PIN_ERR_OP_ALREADY_DONE:
                                pinlog(__FILE__, __LINE__, LOG_FLAG_WARNING,
                                "fm_rate_pol_init_tax_codes_cache: cache already done: <%s>", kids.str);
                                break;
                        default:
                                pinlog(__FILE__, __LINE__, LOG_FLAG_ERROR, "fm_rate_pol_init_tax_codes_cache: error adding cache <%s>", kids.str);
                                break;
                        }
                }
        }
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_rate_pol_init_tax_codes_cache error", ebufp);
        }

CLEANUP:
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
}

