/*******************************************************************
 *
 *    Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *    This material is the confidential property of Oracle Corporation
 *    or its licensors and may be used, reproduced, stored or transmitted
 *    only in accordance with a valid Oracle license or sublicense agreement.
 *    ====================================================================
 *    This application is created to send payment reminder to customer
 *    Author: Vilva S
 *    Date: 28-Dec-2013
 *    Version: 1.0 
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: pin_mta_test.c:CUPmod7.3PatchInt:1:2007-Feb-07 06:51:33 %";
#endif

#include <stdio.h>

#include "pcm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_mta.h"
#include "pin_flds.h"
#include "mso_ops_flds.h"
#include "mso_ntf.h"
#include "ops/bal.h"

#define  NEW_RECORD     0  
#define  SUCCESS_RECORD     1
#define  ERROR_RECORD     2
#define  RETRY         2

#define NOTIFICATION 2
#define  MODE_NON_VERBOSE       0



static void
get_used_mbs(
	pcm_context_t		*ctxp,
	pin_flist_t		*bal_out_flistp,
	pin_decimal_t		*mb_consumed,
	pin_decimal_t		*credit_limit,
	pin_decimal_t		*credit_floor,
	pin_errbuf_t		*ebufp );

static void
get_total_granted_quota(
	pcm_context_t		*ctxp,
	int64			db_no,
	char			*grantor_list,
	pin_decimal_t		*credit_limit,
	pin_decimal_t		*credit_floor,
	pin_errbuf_t		*ebufp );

static void mso_prepaid_lim_usage_cmd_opt(
        pin_flist_t     *param_flistp,
        pin_flist_t     *app_flistp,
        pin_errbuf_t    *ebufp);

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
    char            *object_type   = NULL;
    char            *obj_type   = NULL;
    int32           mta_flags = 0;
    int32           err = 0;
    int32           i = 0;
    void            *vp = 0;
    int             retry = RETRY;
    int32          i_param_count = 0;
    char            delimiter = '/';
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
    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_config application info flist", 
                       app_flistp);
    mso_prepaid_lim_usage_cmd_opt(param_flistp, app_flistp, ebufp);

    if (PIN_ERR_IS_ERR(ebufp)) {
        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                         "pin_mta_config error", ebufp);
    }
    return;
}

static void mso_prepaid_lim_usage_cmd_opt(
        pin_flist_t     *param_flistp,
        pin_flist_t     *app_flistp,
        pin_errbuf_t    *ebufp)
{
        int32           mode = MODE_NON_VERBOSE;
        int32           mta_flags = 0;
        int32           param_flag = 0;
        int32           rec_id = 0;
        void            *vp = NULL;
        pin_cookie_t    cookie = 0;
        pin_cookie_t    prev_cookie = 0;
        pin_flist_t     *flistp;
        char            *option = 0;
        char           	*account_obj = NULL;

        if(PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ,"mso_clean_reservations_cmd_opt param_flistp",param_flistp);

        /******************************************************
         * By default the application should run in non-verbose
         * mode.
         ******************************************************/
        PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_MODE, &mode, ebufp);
        vp = PIN_FLIST_FLD_GET(app_flistp,PIN_FLD_FLAGS,1,ebufp);
        if(vp){
                mta_flags=*(int32 *) vp;
        }
        /******************************************************************
         *   Walk the PIN_FLD_PARAMS array. Get the PIN_FLD_PARAM_NAME.
         *   The param name is a command line option.
         ******************************************************************/
        while((flistp = PIN_FLIST_ELEM_GET_NEXT(param_flistp,PIN_FLD_PARAMS,&rec_id,1, &cookie, ebufp))!=NULL){

                option=PIN_FLIST_FLD_GET(flistp, PIN_FLD_PARAM_NAME, 1, ebufp);

                if (!option) {
                        prev_cookie = cookie;
                        continue;
                }
                if(strcmp(option, "-account") == 0) {

                        account_obj = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

                        if(!account_obj) {
                                fprintf( stderr, "\n\tAccount Poid not specified\n" );
                                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        }else{
                             	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account No Passed STR is:");
                              	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, account_obj);
                                PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_ACCOUNT_NO, account_obj, ebufp);
                        }
                        PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
                        param_flag = 1;
                        cookie = prev_cookie;
                }else {
                        prev_cookie = cookie;
                }

        }

        PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);
}

/*******************************************************************
 * Usage information for the specific app.
 * Called prior MTA_USAGE policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_usage(
        char    *prog)
{

	pin_errbuf_t    ebuf;
	pin_flist_t        *ext_flistp = NULL;
	char            *usage_str = NULL;

	char            *format = "\nUsage:     %s  \n\n";
	    
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
        pin_flist_t        *param_flistp,
        pin_flist_t        *app_flistp,
        pin_errbuf_t    *ebufp)
{
	pin_flist_t    *ext_flistp = NULL;
	void        *vp = 0;

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
        pin_flist_t	*app_flistp,
        pin_flist_t	**s_flistpp,
        pin_errbuf_t	*ebufp)
{

	pin_flist_t	*s_flistp    = NULL;
	pin_flist_t     *tmp_flistp = NULL;
	pin_flist_t     *tmp1_flistp = NULL;
	pin_flist_t     *extraRes_flistp = NULL;
	pin_flist_t	*elem_flistp = NULL;
	char            *template = "select X from /purchased_product 1, /account 2, /product 3 where 2.F1 = 1.F2 and 1.F3 = V3 and 1.F4 = 3.F5 and ((3.F6 >= V6 and 3.F7 <= V7) or (3.F8 >= V8 and 3.F9 <= V9) or (3.F10 >= V10 and 3.F11 <= V11) or (3.F12 >= V12 and 3.F13 <= V13)) and 3.F14 like V14 ";
	char            *template_acct = "select X from /purchased_product 1, /account 2, /product 3 where 2.F1 = 1.F2 and 1.F3 = V3 and 1.F4 = 3.F5 and ((3.F6 >= V6 and 3.F7 <= V7) or (3.F8 >= V8 and 3.F9 <= V9) or (3.F10 >= V10 and 3.F11 <= V11) or (3.F12 >= V12 and 3.F13 <= V13)) and 3.F14 like V14 and 1.F15 = V15 ";	
	char            obj_type[100];
	char            *object_type = NULL;
	char            *t1 = NULL;
	char		*bill_info_id = "BB";
	int32           s_flags = 256;
	int32           active_status = 1;
	poid_t          *s_pdp = NULL;
	poid_t          *i_pdp = NULL;
	poid_t          *binfo_pdp = NULL;
	poid_t		*account_pdp = NULL;
	int64           id = -1;
	int64           db = 0;
	void            *vp = NULL;
	time_t		now_t = 0;
	time_t		start_t = (time_t)0;
	time_t		end_t = (time_t)0;
	pin_decimal_t	*sme_pri_start = pbo_decimal_from_str("1000.0", ebufp);
	pin_decimal_t	*sme_pri_end = pbo_decimal_from_str("1100.0", ebufp);
	pin_decimal_t	*ret_pri_start = pbo_decimal_from_str("2000.0", ebufp);
	pin_decimal_t	*ret_pri_end = pbo_decimal_from_str("2100.0", ebufp);
	pin_decimal_t	*cyb_pri_start = pbo_decimal_from_str("3000.0", ebufp);
	pin_decimal_t	*cyb_pri_end = pbo_decimal_from_str("3100.0", ebufp);
	pin_decimal_t	*cor_pri_start = pbo_decimal_from_str("4000.0", ebufp);
	pin_decimal_t	*cor_pri_end = pbo_decimal_from_str("4100.0", ebufp);
	int		count = 0;
	char		*account_id = NULL;


	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);
	now_t = pin_virtual_time((time_t *)NULL);
	*s_flistpp = 0;

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", 
                       app_flistp);
	
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Reading pin_conf for collection reminder");
	
   	s_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_ERR_LOG_MSG(3, "After poid create");

	account_id = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);

        if (account_id)
                account_pdp = PIN_POID_CREATE(db, "/account", atof(account_id), ebufp);

	/***********************************************************
	* Simple search flist
	***********************************************************
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search/pin -1 0
	0 PIN_FLD_FLAGS           INT [0] 0
	0 PIN_FLD_TEMPLATE        STR [0] "select X from /account 1, /purchased_product 2, /product 3 
		where 1.F1 = 2.F2 and 2.F3 = V3 and 2.F4 = 3.F5 and ((3.F6 >= V6 and 3.F7 <= V7) 
		or (3.F8 >= V8 and 3.F9 <= V9) or (3.F10 >= V10 and 3.F11 <= V11) 
		or (3.F12 >= V12 and 3.F13 <= V13)) "
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	0 PIN_FLD_POID		POID [0] NULL
	0 PIN_FLD_ARGS          ARRAY [2] allocated 20, used 1
	1     PIN_FLD_ACCOUNT_OBJ    POID [0] NULL
	0 PIN_FLD_ARGS          ARRAY [3] allocated 20, used 1
	1     PIN_FLD_STATUS    INT [0] 1
	0 PIN_FLD_ARGS          ARRAY [4] allocated 20, used 1
	1     PIN_FLD_PRODUCT_OBJ    POID [0] NULL
	0 PIN_FLD_ARGS          ARRAY [5] allocated 20, used 1
	1     PIN_FLD_POID    POID [0] NULL
	0 PIN_FLD_ARGS          ARRAY [6] allocated 20, used 1
	1     PIN_FLD_PRIORITY    DECIMAL [0] 1000.0
	0 PIN_FLD_ARGS          ARRAY [7] allocated 20, used 1
	1     PIN_FLD_PRIORITY    DECIMAL [0] 1100.0
	0 PIN_FLD_ARGS          ARRAY [8] allocated 20, used 1
	1     PIN_FLD_PRIORITY    DECIMAL [0] 2000.0
	0 PIN_FLD_ARGS          ARRAY [9] allocated 20, used 1
	1     PIN_FLD_PRIORITY    DECIMAL [0] 2100.0
	0 PIN_FLD_ARGS          ARRAY [10] allocated 20, used 1
	1     PIN_FLD_PRIORITY    DECIMAL [0] 3000.0
	0 PIN_FLD_ARGS          ARRAY [11] allocated 20, used 1
	1     PIN_FLD_PRIORITY    DECIMAL [0] 3100.0
	0 PIN_FLD_ARGS          ARRAY [12] allocated 20, used 1
	1     PIN_FLD_PRIORITY    DECIMAL [0] 4000.0
	0 PIN_FLD_ARGS          ARRAY [13] allocated 20, used 1
	1     PIN_FLD_PRIORITY    DECIMAL [0] 4100.0
	0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 5
	1     PIN_FLD_POID   POID [0] NULL poid pointer
	***********************************************************/
		
	PIN_ERR_LOG_MSG(3, "Before poid");
    	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
    	if(vp)
        	db = PIN_POID_GET_DB ((poid_t*)vp);

    	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
    	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
		PIN_ERR_LOG_MSG(3, "Before flags");
    	PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
    	PIN_ERR_LOG_MSG(3, "after flags");
	/***********************************************************
     	* Search Start Time & End Time
     	***********************************************************/
    	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_ERR_LOG_MSG(3, "after 1");
    	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 2, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
		PIN_ERR_LOG_MSG(3, "after 2");
    	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 3, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, &active_status, ebufp);
		PIN_ERR_LOG_MSG(3, "after status");
    	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 4, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
		PIN_ERR_LOG_MSG(3, "after 4");
    	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 5, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, NULL, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before priority");
		tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 6, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PRIORITY, sme_pri_start, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 7, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PRIORITY, sme_pri_end, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 8, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PRIORITY, ret_pri_start, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 9, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PRIORITY, ret_pri_end, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 10, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PRIORITY, cyb_pri_start, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 11, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PRIORITY, cyb_pri_end, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 12, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PRIORITY, cor_pri_start, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 13, ebufp);
    	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_PRIORITY, cor_pri_end, ebufp);
        /* PIN_FLD_USAGE_MAP --> PIN_FLD_EVENT_TYPE */
        tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 14, ebufp);
        elem_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_USAGE_MAP, 0, ebufp);
        PIN_FLIST_FLD_SET(elem_flistp, PIN_FLD_EVENT_TYPE, "%fdp/mso_sb_dc%", ebufp);


 	if (account_pdp)
        {
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template_acct, ebufp);
                tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 15, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
        }
        else{
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template, ebufp);
        }

    	/***********************************************************
     	* Search results
     	***********************************************************/
	/*s_flistp = PIN_FLIST_CREATE( ebufp );
	PIN_FLIST_FLD_SET( s_flistp, PIN_FLD_FILENAME, (void *)"testfile", ebufp );
	count = 2;
	PIN_FLIST_FLD_SET( s_flistp, PIN_FLD_COUNT, (void *)&count, ebufp );*/
   	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS,0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
    	if (PIN_ERR_IS_ERR(ebufp)) 
    	{
        	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
        	PIN_FLIST_DESTROY_EX (&s_flistp,0);
    	}
    	else
    	{
        	*s_flistpp = s_flistp;
        	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
    	}

    return;
}


/*******************************************************************
 * Search results are modified to consider extra outouts into worker opcode
 * Called after MTA_TUNE policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_tune(
		pin_flist_t		*app_flistp,
		pin_flist_t		*srch_res_flistp,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t             *rest_flistp=NULL;
	pin_flist_t             *multirest_flistp=NULL;
	pin_cookie_t		cookie = NULL;
	int			elem_id = 0;
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune application info flist", 
					   app_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results flist", 
					   srch_res_flistp);
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results output flist", srch_res_flistp);

	return;
}

/*******************************************************************
 * Main application opcode is called here
 *******************************************************************/
PIN_EXPORT void
pin_mta_worker_opcode(
	pcm_context_t  	*ctxp,
	pin_flist_t     *srch_res_flistp,
	pin_flist_t     *op_in_flistp,
	pin_flist_t     **op_out_flistpp,
	pin_flist_t     *ti_flistp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t	*bal_in_flistp 		= NULL;
	pin_flist_t	*bal_out_flistp 	= NULL;
	pin_flist_t	*balances_flistp 	= NULL;
	pin_flist_t	*notif_iflistp 		= NULL;
	pin_flist_t	*notif_oflistp 		= NULL;
	int32		bal_flag 		= 4;
	poid_t		*acc_pdp 		= NULL;
	poid_t		*svc_pdp 		= NULL;
	char		msg[250] 		= "";
	pin_decimal_t	*credit_floor 		= pin_decimal( "0", ebufp );
	pin_decimal_t	*credit_limit 		= pin_decimal( "0", ebufp );
	pin_decimal_t	*mb_consumed 		= pin_decimal("0", ebufp);
	pin_decimal_t	*quota 			= NULL;
	time_t		start_t 		= pin_virtual_time((time_t *)NULL);
	time_t		end_t 			= pin_virtual_time((time_t *)NULL) + 1;
	int32		notif_flag 		= NTF_BB_PREPAID_LIM_USAGE;

	if( PIN_ERR_IS_ERR(ebufp)){
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search flist", srch_res_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode input flist", op_in_flistp);

	acc_pdp = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	svc_pdp = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);

	/* get the balance for additional MB */
	bal_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_POID, acc_pdp, ebufp);
	PIN_FLIST_ELEM_SET(bal_in_flistp, NULL, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
	//balances_flistp = PIN_FLIST_ELEM_ADD(bal_in_flistp, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
	//PIN_FLIST_FLD_SET(balances_flistp, PIN_FLD_CURRENT_BAL, NULL, ebufp);
	//PIN_FLIST_FLD_SET(balances_flistp, PIN_FLD_RESERVED_AMOUNT, NULL, ebufp);
	PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_END_T, &end_t, ebufp);
	PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_START_T, &start_t, ebufp);
	PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
	PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_FLAGS, &bal_flag, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "GET_BALANCES input flist", bal_in_flistp);
	PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES, 0, bal_in_flistp, &bal_out_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
            "GET_BALANCES output flist", bal_out_flistp);
	if( PIN_ERR_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
				"pin_mta_worker_opcode"
				"Get balances error", ebufp );
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
			"pin_mta_worker_opcode:"
			"Get balances output", bal_out_flistp );

	/****
	 * Call function to get the consumed mb's,
	 * credit limit & credit floor values.
	 ***/
	get_used_mbs( ctxp, bal_out_flistp, mb_consumed, credit_limit, credit_floor, ebufp );
	if( PIN_ERR_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"pin_mta_worker_opcode: "
			"Error in getting used mb's value", ebufp );
		goto cleanup;
	}

	if(pbo_decimal_is_null(credit_limit, ebufp) || pbo_decimal_is_null(credit_floor, ebufp)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Credit Limit is null. So, not checking the balance for this account. Exiting..");
		goto cleanup;
	}

	quota = pbo_decimal_subtract(credit_limit, credit_floor, ebufp);

	sprintf(msg, "Quota is %s",pbo_decimal_to_str(quota, ebufp));
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	sprintf(msg, "Remaining in MB is %s",pbo_decimal_to_str(mb_consumed, ebufp));
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	//mb_consumed = pbo_decimal_add(quota, mb_consumed, ebufp);
	pbo_decimal_add_assign( mb_consumed, quota, ebufp );
	sprintf(msg, "Usage in MB is %s",pbo_decimal_to_str(mb_consumed, ebufp));
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);

	//Preparing input and Calling Notfication API
	notif_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_SERVICE_OBJ, svc_pdp, ebufp);
	PIN_FLIST_FLD_SET( notif_iflistp, PIN_FLD_POID, acc_pdp, ebufp);
	PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_FLAGS, &notif_flag, ebufp);
	PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_TOTAL_VALUE, quota, ebufp);
	PIN_FLIST_FLD_SET(notif_iflistp, PIN_FLD_USED_QUANTITY, mb_consumed, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notifiation input flist", notif_iflistp);
	PCM_OP(ctxp, MSO_OP_NTF_CREATE_NOTIFICATION, 0, notif_iflistp, &notif_oflistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Notification output flist", notif_oflistp);

	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			 "pin_mta_worker_opcode error in notification ", ebufp);
	}else{
	    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist from main opcode", 
			   *op_out_flistpp);
	}

	cleanup:

	PIN_FLIST_DESTROY_EX( &bal_in_flistp, NULL );
	PIN_FLIST_DESTROY_EX( &bal_out_flistp, NULL );

	if( credit_limit ){
		pbo_decimal_destroy( &credit_limit );
	}
	if( credit_floor ){
		pbo_decimal_destroy( &credit_floor );
	}
	if( mb_consumed ){
		pbo_decimal_destroy( &mb_consumed );
	}
	if( !pbo_decimal_is_null(quota, ebufp) ){
		pbo_decimal_destroy( &quota );
	}

	return;
}



/*************************************************
 * This function get the consumed
 * data from sub-balances of free mb
 * Added by: Sudhish Jugran
 *************************************************/
static void
get_used_mbs(
	pcm_context_t		*ctxp,
	pin_flist_t		*bal_out_flistp,
	pin_decimal_t		*mb_consumed,
	pin_decimal_t		*credit_limit,
	pin_decimal_t		*credit_floor,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t		*balances_flistp	= NULL;
	pin_flist_t		*sub_bal_flistp		= NULL;
	pin_flist_t		*temp_flistp		= NULL;

	pin_cookie_t		cookie			= NULL;

	poid_t			*grantor_obj		= NULL;

	pin_decimal_t		*current_bal		= NULL;
	pin_decimal_t		*reserved_bal		= NULL;

	int64			grantor_id		= 0;
	int64			db_no			= 0;
	int32			elemid			= 0;
	int			free_mb			= 1100010;

	char			*temp_grantor		= NULL;
	char			*temp			= NULL;
	char			grantor_array[20]	= ""; 
	

	if( PIN_ERR_IS_ERR(ebufp)){
		return;
	}

	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
		"get_used_mbs: input flist",
		bal_out_flistp );

	temp_flistp = PIN_FLIST_CREATE(ebufp);
	balances_flistp = PIN_FLIST_ELEM_GET( bal_out_flistp, 
			PIN_FLD_BALANCES, free_mb, 0, ebufp );
	current_bal = PIN_FLIST_FLD_GET(balances_flistp, 
			PIN_FLD_CURRENT_BAL, 1, ebufp);
	reserved_bal = PIN_FLIST_FLD_GET(balances_flistp, 
			PIN_FLD_RESERVED_AMOUNT, 1, ebufp);
	pbo_decimal_add_assign( mb_consumed, reserved_bal, ebufp );
	pbo_decimal_add_assign( mb_consumed, current_bal, ebufp );

	while( (sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT( balances_flistp,
			PIN_FLD_SUB_BALANCES, &elemid, 1, &cookie, ebufp )) 
			!= (pin_flist_t *)NULL ){

		grantor_obj = PIN_FLIST_FLD_GET( sub_bal_flistp,
				PIN_FLD_GRANTOR_OBJ, 0, ebufp );
		grantor_id = PIN_POID_GET_ID( grantor_obj );
		if( !db_no ){
			db_no = PIN_POID_GET_DB( grantor_obj );
		}

		memset( grantor_array, '\0', 20*sizeof(char));

		if( !temp_grantor ){
			sprintf( grantor_array, "%lld", grantor_id );
			temp_grantor = (char *)pin_malloc((strlen(grantor_array)+1));
			if( !temp_grantor ){
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NO_MEM, 0, 0, 0);
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_DEBUG,
					"get_used_mbs: Not enough memory",
					ebufp );
				break;
			}
			strcpy( temp_grantor, grantor_array);
		}
		else{
			sprintf( grantor_array, ",%ld", grantor_id );
			temp_grantor = (char *)pin_realloc( temp_grantor, 
				(strlen(temp_grantor) + strlen(grantor_array) + 2));
			if( !temp_grantor ){
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NO_MEM, 0, 0, 0);
				PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_DEBUG,
					"get_used_mbs: Not enough memory",
					ebufp );
				break;
			}
			strcat( temp_grantor, grantor_array );
		}
	}
	cookie = NULL;
	elemid = 0;

	get_total_granted_quota( ctxp, db_no, temp_grantor, credit_limit,
						credit_floor, ebufp );
	if( PIN_ERR_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"get_used_mbs: Error in getting quota details",
			ebufp );
	}

	if( temp_grantor ){
		pin_free(temp_grantor);
	}

	return;
}


/*************************************************
 * This function finds the credit limit
 * and credit floor values of the plan from
 * /mso_cfg_credit_limit class.
 * Added by: Sudhish Jugran
 *************************************************/
static void
get_total_granted_quota( 
	pcm_context_t		*ctxp,
	int64			db_no,
	char			*grantor_list,
	pin_decimal_t		*credit_limit,
	pin_decimal_t		*credit_floor,
	pin_errbuf_t		*ebufp )
{
	pin_flist_t		*srch_iflistp		= NULL;
	pin_flist_t		*srch_oflistp		= NULL;
	pin_flist_t		*cities_flistp		= NULL;
	pin_flist_t		*result_flistp		= NULL;
	pin_flist_t		*cl_flistp		= NULL;
	pin_flist_t		*args_flistp		= NULL;
	pin_flist_t		*balances_flistp	= NULL;

	pin_decimal_t		*cf_temp		= NULL;
	pin_decimal_t		*cl_temp		= NULL;

	pin_cookie_t		cookie			= NULL;

	poid_t			*srch_pdp		= NULL;

	char			srch_tmplt[500]		= "";

	int			srch_flag		= SRCH_EXACT;
	int			free_mb			= 1100010;
	int			is_found		= 0;

	int32			elemid			= 0;

	if( PIN_ERR_IS_ERR(ebufp)){
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_DEBUG, grantor_list );

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE( db_no, "/search",
			-1, ebufp );

	PIN_FLIST_FLD_PUT( srch_iflistp, PIN_FLD_POID,
			(void *)srch_pdp, ebufp );
	PIN_FLIST_FLD_SET( srch_iflistp, PIN_FLD_FLAGS,
			(void *)&srch_flag, ebufp );


	sprintf( srch_tmplt, "select X from /mso_cfg_credit_limit 1, "
		"/plan 2, /purchased_product 3 where 1.F1 = 2.F2 "
		"and 2.F3 = 3.F4 and purchased_product_t.poid_id0 "
		"in ( %s ) ", grantor_list );

	PIN_FLIST_FLD_SET( srch_iflistp, PIN_FLD_TEMPLATE,
			(void *)srch_tmplt, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD( srch_iflistp,
			PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET( args_flistp, MSO_FLD_PLAN_NAME,
			(void *)"", ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD( srch_iflistp,
			PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_NAME,
			(void *)"", ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD( srch_iflistp,
			PIN_FLD_ARGS, 3, ebufp );
	PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_POID,
			NULL, ebufp );

	args_flistp = PIN_FLIST_ELEM_ADD( srch_iflistp,
			PIN_FLD_ARGS, 4, ebufp );
	PIN_FLIST_FLD_SET( args_flistp, PIN_FLD_PLAN_OBJ,
			NULL, ebufp );

	PIN_FLIST_ELEM_SET( srch_iflistp, NULL,
			PIN_FLD_RESULTS, 0, ebufp );

	/****
	 * Log the input flist
	 ****/
	PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
		"get_total_granted_quota: "
		"mso credit profile search input flist",
		srch_iflistp );

	PCM_OP( ctxp, PCM_OP_SEARCH, 0, srch_iflistp,
			&srch_oflistp, ebufp );
	if( PIN_ERR_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"get_total_granted_quota :"
			"mso credit profile search error",
			ebufp );
	}
	else{
		/****
		 * Log output flist
		 ****/
		PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
			"get_total_granted_quota: "
			"mso credit profile search output flist",
			srch_oflistp );

		while( (result_flistp = PIN_FLIST_ELEM_GET_NEXT( srch_oflistp, 
				PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp))
				!= (pin_flist_t *)NULL ){
			if( result_flistp ){
				cities_flistp = PIN_FLIST_ELEM_GET(result_flistp, 
						MSO_FLD_CITIES, 0, 1, ebufp);
				if( cities_flistp ){
					cl_flistp = PIN_FLIST_ELEM_GET(cities_flistp, 
						MSO_FLD_CREDIT_PROFILE, free_mb, 1, ebufp);
				}
				if( cl_flistp ){
					cf_temp = PIN_FLIST_FLD_GET(cl_flistp, 
						PIN_FLD_CREDIT_FLOOR, 0, ebufp);
					cl_temp = PIN_FLIST_FLD_GET(cl_flistp, 
						PIN_FLD_CREDIT_LIMIT, 0, ebufp);

					pbo_decimal_add_assign( credit_limit,
								cl_temp, ebufp );
					pbo_decimal_add_assign( credit_floor,
								cf_temp, ebufp );
				}
				else{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
						"No Free MB configured for this plan!!");
				}
			}
		}
		elemid = 0;
		cookie = NULL;
	}
	PIN_FLIST_DESTROY_EX( &srch_iflistp, NULL );
	PIN_FLIST_DESTROY_EX( &srch_oflistp, NULL );

	return;
}
