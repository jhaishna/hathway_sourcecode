/*******************************************************************
 *
 *    Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *    This material is the confidential property of Oracle Corporation
 *    or its licensors and may be used, reproduced, stored or transmitted
 *    only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: pin_mta_test.c:CUPmod7.3PatchInt:1:2007-Feb-07 06:51:33 %";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pcm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_mta.h"
#include "pin_flds.h"
#include "mso_ops_flds.h"

#define NUMBER_OF_SECONDS_IN_DAY 86400 

#define OSD_NUMBER 253
#define OSD_DURATION 30
#define OSD_FLAGS 23

/*******************************************************************
 * During real application development, there is no necessity to implement 
 * empty MTA call back functions. Application will recognize implemented
 * functions automatically during run-time.
 *******************************************************************/
static int fm_mso_get_cust_type(
    pcm_context_t		*ctxp,
    poid_t  			*acct_pdp,
    pin_flist_t			**out_flist,
    pin_errbuf_t		*ebufp);

void
fm_mso_get_custcity(
	pcm_context_t   *ctxp,
	poid_t          *acct_obj,
	char          **cust_city,
	pin_errbuf_t    *ebufp);

void
fm_mso_get_prod_name(
	pcm_context_t   *ctxp,
	poid_t          *pdt_pdp,
	char            **prod_name,
	pin_errbuf_t    *ebufp);

void
fm_mso_get_grace_config(
	pcm_context_t   *ctxp,
	int             *pp_type,
	char            *cust_city,
	pin_flist_t     **out_flistp,
	pin_errbuf_t    *ebufp);

void
fm_mso_get_device_details(
	pcm_context_t   *ctxp,
	poid_t			*srvc_pdp,
	pin_flist_t     **out_flistp,
	pin_errbuf_t    *ebufp);

void
fm_mso_schedule_osd_ntf(
	pcm_context_t   *ctxp,
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp);

void
fm_mso_nds_osd(
	pcm_context_t   *ctxp,
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp);

void
fm_mso_cisco_osd(
	pcm_context_t   *ctxp,
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp);

int32
fm_mso_catv_pt_pkg_type(
        pcm_context_t           *ctxp,
        poid_t                  *prd_pdp,
        pin_errbuf_t            *ebufp);

void
mso_cancel_alc_addon_pdts(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_errbuf_t            *ebufp);

void fm_mso_utils_get_catv_children(
        pcm_context_t           *ctxp,
        poid_t                  *act_pdp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

int32 fm_mso_get_catv_conn_type(
        pcm_context_t           *ctxp,
        poid_t                  *srv_pdp,
        pin_errbuf_t            *ebufp);

void
mso_cancel_alc_addon_addconn_pdts(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *new_serv_pdp,
	pin_errbuf_t            *ebufp);

void mso_gp_cancel_prov(
    pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_errbuf_t            *ebufp);

/*******************************************************************
 * Configuration of application
 * Called prior MTA_CONFIG policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_config(
        pin_flist_t        *param_flistp,
        pin_flist_t        *app_flistp,
        pin_errbuf_t    *ebufp)
{
    int32           rec_id = 0;
    pin_cookie_t    cookie = 0;
    pin_cookie_t    prev_cookie = 0;
    pin_flist_t     *flistp = 0;
    char            *option = 0;
    char            *grace_type   = NULL;
    int32           mta_flags = 0;
    int32           err = 0;
    int32           i = 0;
    void            *vp = 0;
    int32          i_param_count = 0;
    char msg[20];

    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config parameters flist", 
                       param_flistp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist", 
                           app_flistp);
    
	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FLAGS, 0,ebufp);
	if(vp)
		mta_flags = *((int32*)vp);

        /***********************************************************
         * The new flag MTA_FLAG_VERSION_NEW has been introduced to
         * differentiate between new mta & old mta applications as
         * only new applications have the ability to drop PIN_FLD_PARAMS
         * for valid parameters . It is only for new apps that
         * checking err_params_cnt is valid, otherwise for old applications
         * this check without a distincion for new applications would hamper
         * normal functioning.
         ***********************************************************/
    mta_flags = mta_flags | MTA_FLAG_VERSION_NEW ;
    i_param_count = PIN_FLIST_ELEM_COUNT(param_flistp, PIN_FLD_PARAMS, ebufp);

    if ( i_param_count >= 1 && i_param_count <= 2)
    {
        while((flistp = PIN_FLIST_ELEM_GET_NEXT(param_flistp, PIN_FLD_PARAMS, &rec_id, 1, &cookie, ebufp))!= NULL) 
        {
        
            option = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_NAME, 0, ebufp);
            /***************************************************
             * Test options options.
             ***************************************************/
            if(option && (strcmp(option, "-grace_type") == 0)) 
            {    

                grace_type = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

                
                if(!grace_type) {
                    mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                }
                else if (!((strcmp(grace_type,"default")==0)||(strcmp(grace_type,"add_on_grace")==0)))
                {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Invalid grace_type... Input either default or add_on_grace for -grace_type");
                    mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                }
                else
                {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Valid grace_type");
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, grace_type);
                    PIN_FLIST_FLD_COPY (flistp, PIN_FLD_PARAM_VALUE, app_flistp,  PIN_FLD_OBJ_TYPE, ebufp);
                }

                PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
                cookie = prev_cookie;
            }
            else if (option && (strcmp(option,"-account")==0))
            {
                grace_type = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

                
                if(!grace_type) {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Invalid -account option... Enter the Account_no along with -account option");
                    mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                }
                else
                {
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Valid account option");
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, grace_type);
                    PIN_FLIST_FLD_COPY (flistp, PIN_FLD_PARAM_VALUE, app_flistp,  PIN_FLD_HOTLIST_FILENAME, ebufp);
                }

                PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
                cookie = prev_cookie;

            }
            else 
            {
                prev_cookie = cookie;
            }
        }
    }
    else
    {
        mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Invalid input Parameters Passed ");
    }

    grace_type = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_OBJ_TYPE, 1, ebufp);
    if(!grace_type) {
        mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
    }

    PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist", 
                       app_flistp);

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_config error", ebufp);
    }
    return;
}



/*******************************************************************
 * Usage information for the specific app.
 * Called prior MTA_USAGE policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_usage(
        char    *prog)
{

    pin_errbuf_t        ebuf;
    pin_flist_t        *ext_flistp = NULL;
    char                *usage_str = NULL;
    char                *format = "\nUsage:     %s [-verbose] -grace_type <default or add_on_grace> [-account] account_no \n\n";
            
    PIN_ERRBUF_CLEAR (&ebuf);
    
    usage_str = (char*)pin_malloc( strlen(format) + strlen(prog) + 1 );

    if (usage_str == NULL) {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No Memory error");
        return;
    }

    sprintf(usage_str, format ,prog);

    ext_flistp = pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO,
                               &ebuf);

    PIN_FLIST_FLD_SET (ext_flistp, PIN_FLD_DESCR, usage_str, &ebuf)

    if (PIN_ERR_IS_ERR(&ebuf)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_usage error", &ebuf);
    }
    if (usage_str) {
        pin_free(usage_str);
    }
    return;
}

/*******************************************************************
 * Usage information for the specific app.
 * Called after MTA_USAGE policy opcode
 * Information passed from customization layer
 * can be processed and displayed here
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_usage(
        pin_flist_t         *param_flistp,
        pin_flist_t         *app_flistp,
        pin_errbuf_t        *ebufp)
{
    pin_flist_t             *ext_flistp = NULL;
    void                    *vp = 0;

    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage parameters flist", 
                       param_flistp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage application info flist", 
                       app_flistp);

    ext_flistp =
        pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO,
                                               ebufp);

    vp = PIN_FLIST_FLD_GET (ext_flistp, PIN_FLD_DESCR, 1, ebufp);

    if(vp) {
        printf("%s",(char*) vp);
    }

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_post_usage error", ebufp);
    }
    return;
}

/*******************************************************************
 * Application defined search criteria.
 * Called prior MTA_INIT_SEARCH policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_init_search(
        pin_flist_t         *app_flistp,
        pin_flist_t         **s_flistpp,
        pin_errbuf_t        *ebufp)
{

	pin_flist_t	*s_flistp = NULL;
//	char		template[500] = {"select X from /purchased_product 1, /product 2, /profile/customer_care 3, /account 4, /mso_cfg_grace_period_ntf 5 where 1.F1.type = V1 and 1.F2 = V2 and 1.F3 <> V3 and (1.F4 >= V4 and 1.F5 < V5) and 1.F6 = 2.F7 and 2.F8 like V8 and 1.F9 = 3.F10 and 3.F11 = V11 and 1.F12 = 4.F13 and 4.F14 = 5.F15 and 3.F16 = 5.F17 and 5.F18 > V18 "};
//	char		template[500] = {"select X from /purchased_product 1, /product 2, /profile/customer_care 3, /account 4, /mso_cfg_grace_period_ntf 5 where 1.F1.type = V1 and 1.F2 = V2 and 1.F3 <> V3 and (1.F4 >= V4 and 1.F5 < V5) and 1.F6 = 2.F7 and 2.F8 like V8 and 1.F9 = 3.F10 and 1.F11 = 4.F12 and 4.F13 = 5.F14 and 3.F15 = 5.F16 and (5.F17 > V17 or 5.F18 > V18)"};
    char        template[400];
    char        *object_type = NULL;
    char        *account_no = NULL;
	int32		s_flags = 768;
	poid_t		*s_pdp = NULL;
	poid_t		*max_pdp = NULL;
	int64		id = -1;
	int64		db = 0;
	int64		max_id = 1;
	void		*vp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
    pin_flist_t *elem_flistp = NULL;
	int32		i = 0;
	int32		pp_status = 1;
    int32		status_flags = 0;
    int32       add_on_status_flags = 0xFFF;
	time_t		now = 0;
	time_t		exp_t = 0;
	time_t		day_start = 0;
	time_t		day_end = 0;
	struct tm	*tm1 = 0;
	int32		pp_type = 1; // Secondory Point

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	*s_flistpp = 0;

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", app_flistp);
    
	object_type = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_OBJ_TYPE, 0, ebufp);
	account_no = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_HOTLIST_FILENAME, 1, ebufp);

	now = pin_virtual_time((time_t *)NULL);
    
	exp_t = now;
	tm1 = localtime(&exp_t);
	tm1->tm_sec = 0;
	tm1->tm_min = 0;
	tm1->tm_hour = 0;
	tm1->tm_isdst = -1;
	day_end = mktime(tm1);
   
	/****************************************************************
	 *Apply grace period for next two days but add_on grace same day.
	 ****************************************************************/ 
	if (object_type && (strcmp(object_type, "default") == 0))
    	{
		day_end = day_start + (2 * NUMBER_OF_SECONDS_IN_DAY);
	}

	s_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
	if(vp)
		db = PIN_POID_GET_DB ((poid_t*)vp);

	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
//	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	/* PIN_FLD_SERVICE_OBJ */
	s_pdp = PIN_POID_CREATE(db, "/service/catv", 1, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_SERVICE_OBJ, s_pdp, ebufp);

	/* PIN_FLD_STATUS */
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &pp_status, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS_FLAGS, &status_flags, ebufp);

	/* PIN_FLD_PURCHASE_END_T */
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PURCHASE_END_T, &day_start, ebufp);

	/* PIN_FLD_PURCHASE_END_T */
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PURCHASE_END_T, &day_end, ebufp);

	/* PIN_FLD_PRODUCT_OBJ */
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);

	/* PIN_FLD_POID */
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);

	/* PIN_FLD_USAGE_MAP --> PIN_FLD_EVENT_TYPE */
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
	elem_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_USAGE_MAP, 0, ebufp);
	PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_EVENT_TYPE, "%fdp%", ebufp);

    memset(template, '\0', 400);

    if (object_type && (strcmp(object_type,"default")==0))
    {
        if (account_no && !(strcmp(account_no,"hotlist")==0 || strcmp(account_no,"")==0))
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Default Mode with Account No");
            /* ACCOUNT_NO  */
	        tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 19, ebufp);
	        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

            /* PIN_FLD_STATUS_FLAGS  add_on_grace (0xFFF) added to avoid re-applying the default grace on top of add_on_grace */
	        tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 20, ebufp);
	        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS_FLAGS, &add_on_status_flags, ebufp);

            strcpy(template,"select X from /purchased_product 1, /product 2, /profile/customer_care 3, /account 4, /mso_cfg_grace_period_ntf 5 where 1.F1.type = V1 and 1.F2 = V2 and 1.F3 <> V3 and (1.F4 >= V4 and 1.F5 < V5) and 1.F6 = 2.F7 and 2.F8 like V8 and 1.F9 = 3.F10 and 1.F11 = 4.F12 and 4.F13 = 5.F14 and 3.F15 = 5.F16 and (5.F17 > V17 or 5.F18 > V18) and 4.F19 = V19 and 1.F20 <> V20");

        }
        else
        {
            /* PIN_FLD_STATUS_FLAGS add_on_grace (0xFFF) added to avoid re-applying the default grace on top of add_on_grace */
	        tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 19, ebufp);
	        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS_FLAGS, &add_on_status_flags, ebufp);

            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Default Mode without Account No");
            strcpy(template,"select X from /purchased_product 1, /product 2, /profile/customer_care 3, /account 4, /mso_cfg_grace_period_ntf 5 where 1.F1.type = V1 and 1.F2 = V2 and 1.F3 <> V3 and (1.F4 >= V4 and 1.F5 < V5) and 1.F6 = 2.F7 and 2.F8 like V8 and 1.F9 = 3.F10 and 1.F11 = 4.F12 and 4.F13 = 5.F14 and 3.F15 = 5.F16 and (5.F17 > V17 or 5.F18 > V18) and 1.F19 <> V19");
        
        }

    }
    else if (object_type && (strcmp(object_type,"add_on_grace")==0))
    {
        if (account_no && !(strcmp(account_no,"hotlist")==0 || strcmp(account_no,"")==0))
        {
		/* PIN_FLD_ACCOUNT_OBJ of PURCHASED_PRODUCT*/
		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 9, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	
		/* PIN_FLD_POID of ACCOUNT*/
		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 10, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);

            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Add_on Mode with Account No");
            /* ACCOUNT_NO  */
	        tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 11, ebufp);
	        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);

            strcpy(template,"select X from /purchased_product 1, /product 2, /account 3 where 1.F1.type = V1 and 1.F2 = V2 and 1.F3 >= V3 and (1.F4 > V4 and 1.F5 <= V5) and 1.F6 = 2.F7 and 2.F8 like V8 and 1.F9 = 3.F10 and 3.F11 = V11 ");

        }
        else
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Add_on Mode without Account No");
            strcpy(template,"select /*+ parallel(purchased_product_t, 16) */ X from /purchased_product 1, /product 2 where 1.F1.type = V1 and 1.F2 = V2 and 1.F3 >= V3 and (1.F4 > V4 and 1.F5 <= V5) and 1.F6 = 2.F7 and 2.F8 like V8 ");
        
        }

    }
    else
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"MTA Invocation Error");
        goto CLEANUP;
    }


    
    PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	vp = 0;
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, vp, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PRODUCT_OBJ, vp, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_PURCHASE_END_T, vp, ebufp);
    // Added this condition to validate the add_on_grace and default option
    if (object_type && strcmp(object_type, "add_on_grace")==0)
    {
        PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_QUANTITY, 0, ebufp);
    }
    

	/***********************************************************
	 * Search results
	 ***********************************************************/

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_init_search error", ebufp);

		PIN_FLIST_DESTROY_EX (&s_flistp, NULL);
	}else{
		*s_flistpp = PIN_FLIST_COPY(s_flistp, ebufp);

		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", *s_flistpp);
	}
CLEANUP:    
    PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}


/*******************************************************************
 * Function called when new job is avaialble to worker
 * Called prior MTA_WORKER_JOB policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_worker_job(
        pcm_context_t       *ctxp,
        pin_flist_t        *srch_res_flistp,
        pin_flist_t        **op_in_flistpp,
        pin_flist_t        *ti_flistp,
        pin_errbuf_t        *ebufp)
{

    pin_flist_t     *app_flistp = NULL;
    pin_buf_t       *nota_buf     = NULL;

    pin_flist_t     *s_out_flistp = NULL;
    pin_flist_t     *res_flistp = NULL;
    pin_flist_t     *input_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *flistp = NULL;
    pin_flist_t     *pdt_flistp = NULL;
    pin_flist_t     *out_flistp = NULL;
    poid_t          *acct_obj = NULL;
    void            *vp = NULL;
    int32           status_flag = 0xFFFF;
    int32           add_on_status_flag = 0xFFF;
    int32           opcode = PCM_OP_BILL_SET_PRODINFO;
    int32	    ntf_flag = 0;
    int             pp_type = 2; 
    int             *qty = NULL;
    int             grace_days = 5;
    int             *add_on_grace_days = NULL;
    int             total_days = 0;
    time_t          exp_t = 0;
    time_t          purchase_end_date = 0;
    time_t          grace_date = 0;
    struct tm       *tm1 = 0;
    char            *cust_city = NULL;


    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job search results flist", 
                       srch_res_flistp);
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job opcode_in  flist",
                       *op_in_flistpp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job thread info flist", 
                       ti_flistp);

    exp_t = *(time_t *)PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);;
    tm1 = localtime(&exp_t);
    tm1->tm_sec = 0;
    tm1->tm_min = 0;
    tm1->tm_hour = 0;
    tm1->tm_isdst = -1;
    purchase_end_date = mktime(tm1);
//    day_end = day_start + NUMBER_OF_SECONDS_IN_DAY;

    PIN_FLIST_FLD_SET(srch_res_flistp, PIN_FLD_OPCODE, &opcode, ebufp);
    acct_obj = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);

        input_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, input_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_PROGRAM_NAME, "mso_fdp_set_grace_period", ebufp);
    
        pdt_flistp = PIN_FLIST_ELEM_ADD(input_flistp, PIN_FLD_PRODUCTS, 0, ebufp);
        PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_OFFERING_OBJ, ebufp);
        PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_PRODUCT_OBJ, pdt_flistp, PIN_FLD_PRODUCT_OBJ, ebufp);

        qty = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_QUANTITY, 1, ebufp);
        if (qty)
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Execution with Add-on Grace Period");
            grace_date = 1479753000 + (5 * NUMBER_OF_SECONDS_IN_DAY);
            PIN_FLIST_FLD_SET(pdt_flistp, PIN_FLD_STATUS_FLAGS, &add_on_status_flag, ebufp);
	    PIN_FLIST_FLD_SET(pdt_flistp, PIN_FLD_USAGE_END_T, &purchase_end_date, ebufp);
        }
        
        PIN_FLIST_FLD_SET(pdt_flistp, PIN_FLD_PURCHASE_END_T, &grace_date, ebufp);
  
        PIN_FLIST_FLD_SET(srch_res_flistp, PIN_FLD_VALID_TO_DETAILS, &grace_days, ebufp);
	PIN_FLIST_FLD_SET(srch_res_flistp, PIN_FLD_FLAGS, &ntf_flag, ebufp);
        //PIN_FLIST_FLD_COPY(out_flistp, PIN_FLD_FLAGS, srch_res_flistp, PIN_FLD_FLAGS, ebufp);

        *op_in_flistpp = PIN_FLIST_COPY(input_flistp, ebufp);
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job opcode input flist", *op_in_flistpp);
    
        PIN_FLIST_DESTROY_EX(&input_flistp, NULL);

    /********************************************
    *      prepare flist for opcode call in 
    *           pin_mta_worker_opcode
    *********************************************/

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
            "pin_mta_worker_job prepared flist for main opcode", *op_in_flistpp);

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_worker_job error", ebufp);
    }
    return;
}

/*******************************************************************
 * Main application opcode is called here
 *******************************************************************/
PIN_EXPORT void 
pin_mta_worker_opcode(
        pcm_context_t       *ctxp,
        pin_flist_t         *srch_res_flistp,
        pin_flist_t         *op_in_flistp,
        pin_flist_t         **op_out_flistpp,
        pin_flist_t         *ti_flistp,
        pin_errbuf_t        *ebufp)
{
    int32			*opcode = NULL;
    int			    *ntf_flag = NULL;
    int32			is_base_pack = -1;
    poid_t          *prd_pdp = NULL;
    int             *qty = NULL;

    if (PIN_ERR_IS_ERR(ebufp)) {
        return;
    }
    PIN_ERRBUF_CLEAR (ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search results flist", 
                       srch_res_flistp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", 
                       op_in_flistp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode thread info flist", 
                       ti_flistp);
    
    if (srch_res_flistp)
    {
        ntf_flag = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_FLAGS, 1, ebufp);
        if (ntf_flag && (*ntf_flag == 1))
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "OSD NOTIFICATION FLAG - ENABLED");
            fm_mso_schedule_osd_ntf(ctxp, srch_res_flistp, ebufp);
        }
        else
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "OSD NOTIFICATION FLAG - DISABLED");
    }
	
    /*******************************************
    *     Get the opcode number to be called
    *******************************************/
    opcode = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_OPCODE, 0, ebufp);
    
    /*******************************************
    *            Execute opcode
    *******************************************/
    PCM_OP (ctxp, *opcode, 0, op_in_flistp, op_out_flistpp, ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist of opcode is  ",
                *op_out_flistpp);

    /*******************************************
    *     Update the status of the object
    *******************************************/
    prd_pdp = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
        
    /****************************************************
	 * Check whether product is base pack or not
	 ****************************************************/
    // Checking the Quantity Parameter, as the quantity parameter will be passed only in the add_on_grace mode

    qty = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_QUANTITY, 1, ebufp);
    if (qty)
    {
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Add-On Mode, Hence Prov. Cancellation Check needed");
	    if (prd_pdp)
        {
            is_base_pack = fm_mso_catv_pt_pkg_type(ctxp, prd_pdp, ebufp);
        	if (is_base_pack == 0)
	        {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Base Pack");
                PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, srch_res_flistp, PIN_FLD_USERID, ebufp);
            	PIN_FLIST_FLD_SET(srch_res_flistp, PIN_FLD_PROGRAM_NAME, "mso_fdp_set_grace_period", ebufp);
    	        mso_cancel_alc_addon_pdts(ctxp, srch_res_flistp, ebufp);
        	}
            else 
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Non-Base Pack");
            }

            if (PIN_ERR_IS_ERR(ebufp))
		    PIN_ERR_CLEAR_ERR(ebufp);

            //Cancel the current Product also from Provisioning
            mso_gp_cancel_prov(ctxp, srch_res_flistp, ebufp);
        }

    }
    
    if (PIN_ERR_IS_ERR(ebufp))
		PIN_ERR_CLEAR_ERR(ebufp);



    return;
}

/**************************************************
* Function: fm_mso_get_cust_type()
* Retrieves the end customer type from Profile Obj.
* 
***************************************************/
static int fm_mso_get_cust_type(
    pcm_context_t		*ctxp,
    poid_t  			*acct_pdp,
    pin_flist_t			**out_flist,
    pin_errbuf_t		*ebufp)
{
    pin_flist_t     *search_inflistp = NULL;
    pin_flist_t     *search_outflistp = NULL;
    pin_flist_t     *args_flistp = NULL;
    pin_flist_t     *results_flistp = NULL;
    pin_flist_t     *tmp_flistp = NULL;
    pin_flist_t     *res_flistp = NULL;
    pin_flist_t     *flistp = NULL;
    poid_t          *s_pdp = NULL;
    void            *vp = NULL;

    int64           db = 1;
    char            template[500] = {"select X from /profile/customer_care where F1 = V1 "};
    int32           s_flags = 768;
    int             *pp_type = NULL;

    if (PIN_ERR_IS_ERR(ebufp))
		PIN_ERR_CLEAR_ERR(ebufp);
   

    db = PIN_POID_GET_DB(acct_pdp);

    // Extracting the Customer's PP_TYPE from profile object
    search_inflistp = PIN_FLIST_CREATE(ebufp);

    s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(search_inflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
    PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_FLAGS, &s_flags, ebufp);
    PIN_FLIST_FLD_SET(search_inflistp, PIN_FLD_TEMPLATE, template, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

    results_flistp = PIN_FLIST_ELEM_ADD(search_inflistp, PIN_FLD_RESULTS, 0, ebufp);
    vp = 0;
    PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, vp, ebufp);
    PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);
    tmp_flistp = PIN_FLIST_SUBSTR_ADD(results_flistp, PIN_FLD_CUSTOMER_CARE_INFO, ebufp);
    PIN_FLIST_FLD_SET(tmp_flistp, MSO_FLD_PP_TYPE, vp, ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "Profile Search Input Flist", search_inflistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, search_inflistp, &search_outflistp, ebufp);
    if (PIN_ERR_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
        PIN_ERR_CLEAR_ERR(ebufp);
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Profile Search Output Flist", search_outflistp);

    PIN_FLIST_DESTROY_EX(&search_inflistp, NULL);
    if (search_outflistp)
    {
        res_flistp = PIN_FLIST_ELEM_GET(search_outflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if (res_flistp)
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Profile Object Found");
            flistp = PIN_FLIST_SUBSTR_GET(res_flistp, PIN_FLD_CUSTOMER_CARE_INFO, 1, ebufp);

            pp_type = PIN_FLIST_FLD_GET(flistp, MSO_FLD_PP_TYPE, 1, ebufp);

            if (pp_type)
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Customer Type Found...");
                return *pp_type;
            }
            else
            {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Invalid Customer Type...");
                //Returning ERROR value
                return 2;
            }
        }
        else
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Profile Object Not Found");
    }
  
}


/**************************************************
* Function: fm_mso_get_custcity()
* Retrieves the end customer city from Account Obj.
* 
***************************************************/
void
fm_mso_get_custcity(
	pcm_context_t   *ctxp,
	poid_t          *acct_obj,
	char          **cust_city,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t     *read_flds_input = NULL;
	pin_flist_t     *read_flds_output = NULL;
    pin_flist_t     *nm_info_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_custcity", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
	}

	read_flds_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_POID, acct_obj, ebufp);
    nm_info_flistp = PIN_FLIST_ELEM_ADD(read_flds_input, PIN_FLD_NAMEINFO, 1, ebufp);
	PIN_FLIST_FLD_SET(nm_info_flistp, PIN_FLD_CITY, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_custcity READ input list", read_flds_input);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_input, &read_flds_output, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_input, NULL);

	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_custcity READ output list", read_flds_output);

	if (read_flds_output)
	{
        nm_info_flistp = PIN_FLIST_ELEM_GET(read_flds_output, PIN_FLD_NAMEINFO, 1, 0, ebufp);
		*cust_city = PIN_FLIST_FLD_TAKE(nm_info_flistp, PIN_FLD_CITY, 0, ebufp);
	}

	PIN_FLIST_DESTROY_EX(&read_flds_output, NULL);
	return;
}


/**************************************************
* Function: fm_mso_get_grace_config()
* Retrieves the config Obj.
* 
***************************************************/
void
fm_mso_get_grace_config(
	pcm_context_t   *ctxp,
	int             *pp_type,
	char            *cust_city,
    pin_flist_t     **out_flistp,
	pin_errbuf_t    *ebufp)
{
    pin_flist_t     *srch_input_flistp = NULL;
    pin_flist_t     *srch_output_flistp = NULL;
    pin_flist_t     *args_flistp = NULL;
    pin_flist_t     *res_flistp = NULL;
    pin_flist_t     *flistp = NULL;
    poid_t          *s_pdp = NULL;

    int64           db = 1;
    char            template[500] = {"select X from /mso_cfg_grace_period_ntf where F1 = V1 and F2 = V2"};
    int32           s_flags = 768;

    if (PIN_ERR_IS_ERR(ebufp))
        return;

    srch_input_flistp = PIN_FLIST_CREATE(ebufp);

    s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(srch_input_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
    PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
    PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 1, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CITY, cust_city, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, MSO_FLD_PP_TYPE, pp_type, ebufp);

    res_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_VALID_TO_DETAILS, 0, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_VALIDITY_IN_DAYS, 0, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_FLAGS, 0, ebufp);
    

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_cfg_grace_period_ntf Search Input Flist", srch_input_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_input_flistp, &srch_output_flistp, ebufp);
    if (PIN_ERR_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
        PIN_ERR_CLEAR_ERR(ebufp);
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_cfg_grace_period_ntf Search Output Flist", srch_output_flistp);

    PIN_FLIST_DESTROY_EX(&srch_input_flistp, NULL);
    if (srch_output_flistp)
    {
        res_flistp = PIN_FLIST_ELEM_GET(srch_output_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if (res_flistp)
        {
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Config Object Found");
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_cfg_grace_period_ntf RESULTS Flist", res_flistp);
            *out_flistp = PIN_FLIST_COPY(res_flistp, ebufp);
       }
       else 
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Config Object Not Found...");
       
    }

    return;
}

/**************************************************
* Function: fm_mso_schedule_osd_ntf()
* Schedules the OSD.
* 
***************************************************/
void
fm_mso_schedule_osd_ntf(
	pcm_context_t   *ctxp,
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t		*dev_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*device_pdp = NULL;
	int				*dev_count = NULL;
	int				elem_id = 0;
	pin_cookie_t	cookie = NULL;
	char			*device_id = NULL;
	char			*manufacturer = NULL;
	
	PIN_ERRBUF_CLEAR (ebufp);

	srvc_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	acct_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);

	fm_mso_get_device_details(ctxp, srvc_pdp, &dev_flistp, ebufp);
	if (dev_flistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Device Object Found");
		dev_count = PIN_FLIST_FLD_GET(dev_flistp, PIN_FLD_COUNT, 1, ebufp);
		if (*dev_count == 0)
		{
		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Device Object Not Found!!!");
			return;
		}
		else if (*dev_count == 1)
		{
			// Only 1 device for service, CISCO box
			res_flistp = PIN_FLIST_ELEM_GET(dev_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
			if (res_flistp)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Device Object Found");
				device_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 1, ebufp);
				manufacturer = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_MANUFACTURER, 1, ebufp);
				if (manufacturer && (strstr(manufacturer, "CISCO")))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "CISCO DEVICE");
					PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_DEVICE_ID, in_flistp, PIN_FLD_DEVICE_ID, ebufp);
					fm_mso_cisco_osd(ctxp, in_flistp, ebufp);

				}
			}
		}
		else
		{
			// More than 1 device for service, NDS boxes
			while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(dev_flistp, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp )) != NULL)
			{
				device_pdp = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_POID, 1, ebufp);
				manufacturer = PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_MANUFACTURER, 1, ebufp);
				if (manufacturer && (strstr(manufacturer, "NDS")))
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "NDS DEVICE");
					if (device_pdp && (strcmp(PIN_POID_GET_TYPE(device_pdp), "/device/vc") == 0))
					{
						PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_DEVICE_ID, in_flistp, PIN_FLD_DEVICE_ID, ebufp);
						fm_mso_nds_osd(ctxp, in_flistp, ebufp);
					}
				}
			}
		}
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Device Object Not Found!!!");
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

}


/**************************************************
* Function: fm_mso_get_device_details()
* Retrieves the device details from Service Obj.
* 
***************************************************/
void
fm_mso_get_device_details(
	pcm_context_t   *ctxp,
	poid_t			*srvc_pdp,
	pin_flist_t     **out_flistp,
	pin_errbuf_t    *ebufp)
{
    pin_flist_t     *srch_input_flistp = NULL;
    pin_flist_t     *srch_output_flistp = NULL;
    pin_flist_t     *args_flistp = NULL;
	pin_flist_t     *srvc_flistp = NULL;
    pin_flist_t     *res_flistp = NULL;
    pin_flist_t     *flistp = NULL;
    poid_t          *s_pdp = NULL;
	int				dev_count = 0;
	int				state_id = 2;
    int64           db = 0;
    char            *template = {"select X from /device where F1 = V1 and F2 = V2"};
    int32           s_flags = 768;

    if (PIN_ERR_IS_ERR(ebufp))
        return;

    srch_input_flistp = PIN_FLIST_CREATE(ebufp);
    db = PIN_POID_GET_DB(srvc_pdp);

    s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    PIN_FLIST_FLD_PUT(srch_input_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
    PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
    PIN_FLIST_FLD_SET(srch_input_flistp, PIN_FLD_TEMPLATE, template, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 1, ebufp);
    srvc_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_SERVICES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(srvc_flistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
    args_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_ARGS, 2, ebufp);
    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATE_ID, &state_id, ebufp);

    res_flistp = PIN_FLIST_ELEM_ADD(srch_input_flistp, PIN_FLD_RESULTS, 0, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);
    PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_DEVICE_ID, "", ebufp);
	PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_MANUFACTURER, "", ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "Device Search Input Flist", srch_input_flistp);
    PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_input_flistp, &srch_output_flistp, ebufp);
    if (PIN_ERR_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling PCM_OP_SEARCH", ebufp);
        PIN_ERR_CLEAR_ERR(ebufp);
    }
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"mso_cfg_grace_period_ntf Search Output Flist", srch_output_flistp);

    PIN_FLIST_DESTROY_EX(&srch_input_flistp, NULL);
    if (srch_output_flistp)
    {
		*out_flistp = srch_output_flistp;
		dev_count = PIN_FLIST_ELEM_COUNT(srch_output_flistp, PIN_FLD_RESULTS, ebufp);
		if ( dev_count == 0 )
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "No Devices are mapped to Services");
			PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_COUNT, &dev_count, ebufp);
		}
		else if (dev_count == 1)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Only One Device Mapping found for the service...");
			PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_COUNT, &dev_count, ebufp);
		}
		else
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "More than one Device Mapping found for the services...");
			PIN_FLIST_FLD_SET(*out_flistp, PIN_FLD_COUNT, &dev_count, ebufp);
		}
	}
	PIN_ERR_CLEAR_ERR(ebufp);

    return;
}

void
fm_mso_nds_osd(
	pcm_context_t   *ctxp,
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t		*schedule_in_flistp = NULL;
	pin_flist_t		*schedule_out_flistp = NULL;
	pin_flist_t		*data_flistp = NULL;
	int				action_mode = 7;
	int				*grace_days = NULL;
	int				grace_days_int = 0;
	int				i = 0;
	int				osd_number = OSD_NUMBER;
	int				flags = OSD_FLAGS;
	int				duration = OSD_DURATION;
	poid_t			*pdt_pdp = NULL;
	char			*prod_name = NULL;
	char			msg[500]="";
	char			buf[20]="";
	time_t          exp_t = 0;
    time_t          purchase_end_date = 0;
    time_t          schedule_date = 0;
    time_t          grace_date = 0;
    struct tm       *tm1 = 0;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_nds_osd input list", in_flistp);

    exp_t = *(time_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);;
    tm1 = localtime(&exp_t);
    tm1->tm_sec = 0;
    tm1->tm_min = 0;
    tm1->tm_hour = 10;
    tm1->tm_isdst = -1;
    purchase_end_date = mktime(tm1);
//	strftime(buf, sizeof(buf), "%d-%m-%Y", tm1);

	pdt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
	grace_days = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_VALID_TO_DETAILS, 1, ebufp);
	if (!grace_days)
	{
		grace_days_int = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Grace Period Config Object Not Found, Hence No extension applicable !!!");
	}
	else
	{
		grace_days_int = *grace_days;
	}

	fm_mso_get_prod_name(ctxp, pdt_pdp, &prod_name, ebufp);
	
	schedule_in_flistp = PIN_FLIST_CREATE(ebufp);
	while (i < grace_days_int)
	{

		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, schedule_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, schedule_in_flistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_SET(schedule_in_flistp, PIN_FLD_PROGRAM_NAME, "Scheduling OSD - NDS", ebufp);
		PIN_FLIST_FLD_SET(schedule_in_flistp, PIN_FLD_ACTION_MODE, &action_mode, ebufp);
		
		schedule_date = purchase_end_date + (i * NUMBER_OF_SECONDS_IN_DAY);
		PIN_FLIST_FLD_SET(schedule_in_flistp, PIN_FLD_WHEN_T, &schedule_date, ebufp);

		data_flistp = PIN_FLIST_ELEM_ADD(schedule_in_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, data_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, data_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, data_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DEVICE_ID, data_flistp, PIN_FLD_ADDRESS, ebufp);
		PIN_FLIST_FLD_SET(data_flistp, MSO_FLD_OSD_NUMBER, &osd_number, ebufp);
		PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_FLAGS, &flags, ebufp);
		PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_DURATION_SECONDS, &duration, ebufp);
		PIN_FLIST_FLD_SET(data_flistp, MSO_FLD_NETWORK_NODE, "NDS", ebufp);
		PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_DESCR, "OSD - NDS", ebufp);
		PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_TITLE, "SUBSCRIPTION NOTIFICATION", ebufp);
		
		if (prod_name && (strcmp(prod_name, "")!=0))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Prod Name Available");
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, prod_name);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"ABCD");
//			sprintf(msg, "Dear Hathway customers, FDP Plan %s have been reactivated due expiry date %s. Pls co-ordinate with your cable operator/Hathway Collection executive for any assistance", prod_name, buf);
			sprintf(msg, "Dear Hathway customers, FDP Plan %s will be deactivated, pls renew immediately Pls co-ordinate with your cable operator/Hathway Collection executive for any assistance.", prod_name);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"1");
		}
		else
		{
//			sprintf(msg, "Dear Hathway customers, FDP Plan have been reactivated due expiry date %s. Pls co-ordinate with your cable operator/Hathway Collection executive for any assistance", buf);
			sprintf(msg, "Dear Hathway customers, FDP Plan will be deactivated, pls renew immediately Pls co-ordinate with your cable operator/Hathway Collection executive for any assistance.");
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"2");
		}
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"3");
		PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_MESSAGE, msg, ebufp);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"4");
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_nds_osd schedule OSD input list", schedule_in_flistp);
		PCM_OP(ctxp, MSO_OP_CUST_CREATE_SCHEDULE, 0, schedule_in_flistp, &schedule_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_nds_osd schedule OSD output list", schedule_out_flistp);
		i++;
        strcpy(msg,"");
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"test3");
	}
	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "OSD Notification Schedule Failed !!!", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
	}

	return;

}


void
fm_mso_cisco_osd(
	pcm_context_t   *ctxp,
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t		*schedule_in_flistp = NULL;
	pin_flist_t		*schedule_out_flistp = NULL;
	pin_flist_t		*data_flistp = NULL;
	pin_flist_t		*stb_flistp = NULL;
	pin_flist_t		*msg_title_flistp = NULL;
	pin_flist_t		*msg_content_flistp = NULL;
	int				action_mode = 7;
	int				*grace_days = NULL;
	int				grace_days_int = 0;
	int				i = 0;
	int				osd_number = OSD_NUMBER;
	int				flags = OSD_FLAGS;
	int				duration = OSD_DURATION;
	poid_t			*pdt_pdp = NULL;
	char			*prod_name = NULL;
	char			msg[200]="";
	char			buf[20]="";
	time_t          exp_t = 0;
    time_t          purchase_end_date = 0;
    time_t          schedule_date = 0;
    time_t          grace_date = 0;
    struct tm       *tm1 = 0;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cisco_osd input list", in_flistp);

    exp_t = *(time_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PURCHASE_END_T, 1, ebufp);;
    tm1 = localtime(&exp_t);
    tm1->tm_sec = 0;
    tm1->tm_min = 0;
    tm1->tm_hour = 10;
    tm1->tm_isdst = -1;
    purchase_end_date = mktime(tm1);
//	strftime(buf, sizeof(buf), "%d-%m-%Y", tm1);

	pdt_pdp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
	grace_days = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_VALID_TO_DETAILS, 1, ebufp);
	if (!grace_days)
	{
		grace_days_int = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Grace Period Config Object Not Found, Hence No extension applicable !!!");
	}
	else
	{
		grace_days_int = *grace_days;
	}
	fm_mso_get_prod_name(ctxp, pdt_pdp, &prod_name, ebufp);
	
	schedule_in_flistp = PIN_FLIST_CREATE(ebufp);
	while (i < grace_days_int)
	{

		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, schedule_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, schedule_in_flistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_SET(schedule_in_flistp, PIN_FLD_PROGRAM_NAME, "Scheduling OSD - CISCO", ebufp);
		PIN_FLIST_FLD_SET(schedule_in_flistp, PIN_FLD_ACTION_MODE, &action_mode, ebufp);
		
		schedule_date = purchase_end_date + (i * NUMBER_OF_SECONDS_IN_DAY);
		PIN_FLIST_FLD_SET(schedule_in_flistp, PIN_FLD_WHEN_T, &schedule_date, ebufp);

		data_flistp = PIN_FLIST_ELEM_ADD(schedule_in_flistp, PIN_FLD_DATA_ARRAY, 0, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, data_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, data_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, data_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(data_flistp, MSO_FLD_OSD_NUMBER, &osd_number, ebufp);
		PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_FLAGS, &flags, ebufp);
		PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_DURATION_SECONDS, &duration, ebufp);
		PIN_FLIST_FLD_SET(data_flistp, MSO_FLD_NETWORK_NODE, "CISCO", ebufp);
		PIN_FLIST_FLD_SET(data_flistp, PIN_FLD_DESCR, "OSD - CISCO", ebufp);
		stb_flistp = PIN_FLIST_ELEM_ADD(data_flistp, MSO_FLD_STB_ID_LIST, 0, ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_DEVICE_ID, stb_flistp, MSO_FLD_STB_ID, ebufp);

		PIN_FLIST_FLD_SET(data_flistp, MSO_FLD_MSG_GRP_NAME, "TEST", ebufp);
		msg_title_flistp = PIN_FLIST_ELEM_ADD(data_flistp, MSO_FLD_MSG_TITLE_LIST, 0, ebufp);
		PIN_FLIST_FLD_SET(msg_title_flistp, PIN_FLD_TITLE, "SUBSCRIPTION NOTIFICATION", ebufp);
		
		msg_content_flistp = PIN_FLIST_ELEM_ADD(data_flistp, MSO_FLD_MSG_CONTENT_LIST, 0, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cisco_osd schedule OSD input list", schedule_in_flistp);
		if (prod_name)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Prod Name Available");
//			sprintf(msg, "Dear Hathway customers, FDP Plan %s have been reactivated due expiry date %s. Pls co-ordinate with your cable operator/Hathway Collection executive for any assistance", prod_name, buf);
			sprintf(msg, "Dear Hathway customers, FDP Plan %s will be deactivated, pls renew immediately Pls co-ordinate with your cable operator/Hathway Collection executive for any assistance.", prod_name);
		}
		else
		{
//			sprintf(msg, "Dear Hathway customers, FDP Plan have been reactivated due expiry date %s. Pls co-ordinate with your cable operator/Hathway Collection executive for any assistance", buf);
			sprintf(msg, "Dear Hathway customers, FDP Plan will be deactivated, pls renew immediately Pls co-ordinate with your cable operator/Hathway Collection executive for any assistance.");
		}
		PIN_FLIST_FLD_SET(msg_content_flistp, PIN_FLD_MESSAGE, msg, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cisco_osd schedule OSD input list", schedule_in_flistp);
		PCM_OP(ctxp, MSO_OP_CUST_CREATE_SCHEDULE, 0, schedule_in_flistp, &schedule_out_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cisco_osd schedule OSD output list", schedule_out_flistp);

		i++;
	}
	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "OSD Notification Schedule Failed !!!", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
	}
	return;
}

 
/**************************************************
* Function: fm_mso_get_prod_name()
* Retrieves the Name of the Product.
* 
***************************************************/
void
fm_mso_get_prod_name(
	pcm_context_t   *ctxp,
	poid_t          *pdt_pdp,
	char            **prod_name,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t     *read_flds_input = NULL;
	pin_flist_t     *read_flds_output = NULL;
    pin_flist_t     *nm_info_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_prod_name", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
	}

	read_flds_input = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_POID, pdt_pdp, ebufp);
    PIN_FLIST_FLD_SET(read_flds_input, PIN_FLD_NAME, "", ebufp);
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prod_name READ input list", read_flds_input);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flds_input, &read_flds_output, ebufp);
	PIN_FLIST_DESTROY_EX(&read_flds_input, NULL);

	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prod_name READ output list", read_flds_output);

	if (read_flds_output)
	{
		*prod_name = PIN_FLIST_FLD_TAKE(read_flds_output, PIN_FLD_NAME, 0, ebufp);
	}

	PIN_FLIST_DESTROY_EX(&read_flds_output, NULL);
	return;
}

/*******************************************************************
 * fm_mso_catv_pt_pkg_type()
 *
 * This function retrieves product package type from config PT and
 * returns the same.
 *******************************************************************/
int32
fm_mso_catv_pt_pkg_type(
        pcm_context_t           *ctxp,
        poid_t                  *prd_pdp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
        pin_flist_t             *arg_flistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        char                    template2[1024] = "select X from /mso_cfg_catv_pt 1, /product 2 where 1.F1 = 2.F2 and 2.F3 = V3 ";
        char                    *ptp = "";
        int32                   srch_flag = 768;
        int32                   pkg_type = 0;

        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        srch_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID,
        PIN_POID_CREATE(PIN_POID_GET_DB(prd_pdp), "/search", -1, ebufp), ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2, ebufp);
        arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, ptp,ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_PROVISIONING_TAG, ptp,ebufp);

        arg_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp);
        PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, prd_pdp, ebufp);

        r_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_catv_pt_pkg_type input flist", srch_flistp);

        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);

        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERR_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_catv_pt_pkg_type : error getting Package_type", ebufp);
                pkg_type = 0;
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                        "fm_mso_catv_pt_pkg_type output flist", srch_out_flistp);

        r_flistp = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
        if (r_flistp)
        {
                pkg_type = *(int32 *)PIN_FLIST_FLD_GET(r_flistp, MSO_FLD_PKG_TYPE, 0, ebufp);
        }
        else
        {
                pkg_type = -1;
        }

CLEANUP:
        PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
        return pkg_type;
}

void
mso_cancel_alc_addon_pdts(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_errbuf_t            *ebufp)
{
	pin_cookie_t    cookie = NULL;
    pin_cookie_t    cookie1 = NULL;
	pin_flist_t		*srch_in_flist = NULL;
	pin_flist_t		*srch_out_flist = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*prov_out_flistp = NULL;
	pin_flist_t		*write_flds_inflistp = NULL;
	pin_flist_t		*write_flds_outflistp = NULL;
	pin_flist_t		*pdt_flistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*child_serv_flist = NULL;
	poid_t			*ppt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*prd_pdp = NULL;
	poid_t			*srv_pdp = NULL;
	poid_t			*act_pdp = NULL;
	poid_t			*new_serv_pdp = NULL;
	char			template[200];
	int32			srch_flag = 768;
	int32			new_status_flags = 0xFFF;
	int32			is_base_pack = -1;
	int			    active_status = 1;
	int             elem_id = 0;
    int             elem_id1 = 0;
	int             cancel_plan_flag = 4;
	int32			main_conn_type = -1;
	int32			tmp_conn_type = -1;
	int32			cancel_flag = -1;
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	ppt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	srv_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);

	main_conn_type = fm_mso_get_catv_conn_type(ctxp, srv_pdp, ebufp);

	srch_in_flist = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(ppt_pdp), "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flist, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &active_status, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 3, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, args_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

    sprintf(template, "select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 ");

	PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_TEMPLATE, template, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_pdts search input flist", srch_in_flist);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flist, &srch_out_flist, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_pdts search output flist", srch_out_flist);

	PIN_FLIST_DESTROY_EX(&srch_in_flist, NULL);
	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in mso_cancel_alc_addon_pdts search", ebufp);
		goto CLEANUP;
	}

	prov_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, prov_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, prov_in_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, prov_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &cancel_plan_flag, ebufp);
	
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flist, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Adding the Product for Provisioning Cancellation");
		//Creating the Provisioning Action to Enable the Ala-Carte and Add-on Plans
		pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Input flist", prov_in_flistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Output flist", prov_out_flistp);

    PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
	if (PIN_ERR_IS_ERR(ebufp))
	{
        PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
    	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
		goto CLEANUP;
	}

	/***************************************************************
	 * Update purchased_product status_flags to 4095 
	 **************************************************************/
	elem_id = 0;
	cookie = NULL;
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flist, 
		PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		write_flds_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);
	
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

		PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
		if (PIN_ERR_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in mso_cancel_alc_addon_pdts: wflds", ebufp);
			break;
		}
	}

    if (main_conn_type == 0)
    {
       	act_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	    fm_mso_utils_get_catv_children(ctxp, act_pdp, &child_serv_flist, ebufp);
		while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(child_serv_flist, PIN_FLD_RESULTS, &elem_id1, 1, &cookie1, ebufp)) != NULL)
		{
			new_serv_pdp = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_POID, 1, ebufp);
            PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling Cancellation for Child Connection");
			mso_cancel_alc_addon_addconn_pdts(ctxp, i_flistp, new_serv_pdp, ebufp);
		}
	}
	PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flist, NULL);
	return;
}

/*******************************************************************
 * fm_mso_get_catv_conn_type()
 *
 * This function retrieves CATV service connection type and
 * returns the same.
 *******************************************************************/
int32
fm_mso_get_catv_conn_type(
        pcm_context_t           *ctxp,
        poid_t                  *srv_pdp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *rfld_iflistp = NULL;
        pin_flist_t             *rfld_oflistp = NULL;
        pin_flist_t             *temp_flistp = NULL;
        int32                   conn_type = 0;

        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        rfld_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID, srv_pdp, ebufp);
        temp_flistp = PIN_FLIST_SUBSTR_ADD(rfld_iflistp, MSO_FLD_CATV_INFO, ebufp);
        PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_CONN_TYPE, &conn_type, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_get_catv_conn_type: Read Flds In flist", rfld_iflistp);

        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp, &rfld_oflistp, ebufp);

        PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);

        if (PIN_ERR_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_get_catv_conn_type error: Read Flds Error", ebufp);
                conn_type = -1;
                goto CLEANUP;
        }

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_get_catv_conn_type: Read Flds Out flist", rfld_oflistp);

        temp_flistp = PIN_FLIST_SUBSTR_GET(rfld_oflistp, MSO_FLD_CATV_INFO, 1, ebufp);
        if (temp_flistp)
        {
                conn_type = *(int32 *)PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_CONN_TYPE, 0, ebufp);
        }
        else
        {
                conn_type = -1;
        }

CLEANUP:
        PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
        return conn_type;
}

/**************************************************************************
 * fm_mso_utils_get_catv_children()
 *
 * For getting CATV child service objects
 **************************************************************************/
void fm_mso_utils_get_catv_children(
        pcm_context_t           *ctxp,
        poid_t                  *act_pdp,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        poid_t                  *s_pdp = NULL;
        int32                   flags = 256;
        int64                   db = 0;
        int64                   id = -1;
        int32                   child_con_type = 1;
        char                    *s_template = "select X from /service/catv where F1 = V1 and F2 = V2 and F3 = V3 ";
        int32                   serv_status = 10100;

        if (PIN_ERR_IS_ERR(ebufp))
                return;
        PIN_ERR_CLEAR_ERR(ebufp);

        db = PIN_POID_GET_DB(act_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);

        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, act_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &serv_status, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
        args_flistp = PIN_FLIST_SUBSTR_ADD(args_flistp, MSO_FLD_CATV_INFO, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CONN_TYPE, &child_con_type, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
                PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_POID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Child Service Input flist:",s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Child Service Output flist:",*o_flistpp);

        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        return;
}

void
mso_cancel_alc_addon_addconn_pdts(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t             *new_serv_pdp,
	pin_errbuf_t            *ebufp)
{
	pin_cookie_t    cookie = NULL;
	pin_flist_t		*srch_in_flist = NULL;
	pin_flist_t		*srch_out_flist = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*prov_out_flistp = NULL;
	pin_flist_t		*write_flds_inflistp = NULL;
	pin_flist_t		*write_flds_outflistp = NULL;
	pin_flist_t		*pdt_flistp = NULL;
	poid_t			*srch_pdp = NULL;
	char			template[200];
	int32			srch_flag = 768;
	int32			new_status_flags = 0xFFF;
	int             active_status = 1;
	int             elem_id = 0;
	int             cancel_plan_flag = 4;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);


	srch_in_flist = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(new_serv_pdp), "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_in_flist, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_FLAGS, &srch_flag, ebufp);

    args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);

	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &active_status, ebufp);

	/***************************************************************
	 * Check if main connection get account level purchased products
	 * otherwise service level purchased products 
	 **************************************************************/
	args_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, new_serv_pdp, ebufp);
	sprintf(template, "select X from /purchased_product where F1 = V1 and F2 = V2 and F3 = V3 ");
	
	PIN_FLIST_FLD_SET(srch_in_flist, PIN_FLD_TEMPLATE, template, ebufp);

	result_flistp = PIN_FLIST_ELEM_ADD(srch_in_flist, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_addconn_pdts search input flist", srch_in_flist);

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_in_flist, &srch_out_flist, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_addconn_pdts search output flist", srch_out_flist);

	PIN_FLIST_DESTROY_EX(&srch_in_flist, NULL);
	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in mso_cancel_alc_addon_addconn_pdts search", ebufp);
		goto CLEANUP;
	}

	prov_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, prov_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_SERVICE_OBJ, new_serv_pdp, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, prov_in_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, prov_in_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &cancel_plan_flag, ebufp);
	
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flist, PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		//Creating the Provisioning Action to Enable the Ala-Carte and Add-on Plans
		pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_addconn_pdts Provisioning Input flist", prov_in_flistp);
	PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_cancel_alc_addon_addconn_pdts Provisioning Output flist", prov_out_flistp);

	PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
	if (PIN_ERR_IS_ERR(ebufp))
	{
	PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_cancel_alc_addon_addconn_pdts : Error in MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
		goto CLEANUP;
	}

	/***************************************************************
	 * Update purchased_product status_flags to 4095 
	 **************************************************************/
	elem_id = 0;
	cookie = NULL;
	while ((results_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_out_flist, 
		PIN_FLD_RESULTS, &elem_id, 1, &cookie, ebufp)) != NULL)
	{
		write_flds_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(results_flistp, PIN_FLD_POID, write_flds_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(write_flds_inflistp, PIN_FLD_STATUS_FLAGS, &new_status_flags, ebufp);
	
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, write_flds_inflistp, &write_flds_outflistp, ebufp);

		PIN_FLIST_DESTROY_EX(&write_flds_inflistp, NULL);
		PIN_FLIST_DESTROY_EX(&write_flds_outflistp, NULL);
		if (PIN_ERR_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in mso_cancel_alc_addon_addconn_pdts: wflds", ebufp);
			break;
		}
	}
	PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_out_flist, NULL);
	return;
}

void mso_gp_cancel_prov(
    pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_errbuf_t            *ebufp)
{
  	pin_flist_t		*prov_in_flistp = NULL;
	pin_flist_t		*prov_out_flistp = NULL;
    pin_flist_t		*pdt_flistp = NULL;

	int             cancel_plan_flag = 4;
    int             elem_id = 0;

    prov_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, prov_in_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, prov_in_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, prov_in_flistp, PIN_FLD_USERID, ebufp);
    PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_PROGRAM_NAME, "mso_fdp_set_grace_period", ebufp);
	PIN_FLIST_FLD_SET(prov_in_flistp, PIN_FLD_FLAGS, &cancel_plan_flag, ebufp);
    pdt_flistp = PIN_FLIST_ELEM_ADD(prov_in_flistp, PIN_FLD_PRODUCTS, elem_id, ebufp);
    PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, pdt_flistp, PIN_FLD_POID, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Input flist", prov_in_flistp);
    PCM_OP(ctxp, MSO_OP_PROV_CREATE_CATV_ACTION, 0, prov_in_flistp, &prov_out_flistp, ebufp);
    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Provisioning Output flist", prov_out_flistp);

    PIN_FLIST_DESTROY_EX(&prov_out_flistp, NULL);
    if (PIN_ERR_IS_ERR(ebufp))
    {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in MSO_OP_PROV_CREATE_CATV_ACTION", ebufp);
        goto CLEANUP;
    }

CLEANUP:
    PIN_FLIST_DESTROY_EX(&prov_in_flistp, NULL);
	return;
}

