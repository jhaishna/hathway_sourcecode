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
static  char    Sccs_id[] = "@(#)%Portal Version: mso_bill_correction.c:CUPmod7.3PatchInt:1:2007-Feb-07 06:51:33 %";
#endif

#include <stdio.h>
#include "pcm.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_mta.h"
#include "pin_flds.h"
#include "ops/base.h"
#include "mso_ops_flds.h"
#include "ops/bill.h"


/*******************************************************************
* Apllication constants
********************************************************************/
#define  MODE_VERBOSE		1
#define  MODE_NON_VERBOSE	0

#define DAILY_USAGE  	 1
#define TRIAL_BILL 	 2
#define DEVICE_LIFECYCLE 3
#define SUSP_CDR	 4

/*******************************************************************
 * Functions used within
 *******************************************************************/
static void
mso_clean_obj_parse_cmd_opt(
        pin_flist_t     *param_flistp,
        pin_flist_t     *app_flistp,
        pin_errbuf_t    *ebufp);

/*******************************************************************
 * Configuration of application
 * Called prior MTA_CONFIG policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_config(
	pin_flist_t	*param_flistp,
	pin_flist_t	*app_flistp,
	pin_errbuf_t	*ebufp)
{
	void		*vp = 0;
	int32		mta_flags = 0;

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
	
	mta_flags = mta_flags | MTA_FLAG_VERSION_NEW;
	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);

	/***********************************************************
	* Get command line options for allication
	***********************************************************/
	mso_clean_obj_parse_cmd_opt(param_flistp, app_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_mta_config error", ebufp);
	}
	return;

}

static void
mso_clean_obj_parse_cmd_opt(
	pin_flist_t	*param_flistp,
	pin_flist_t	*app_flistp,
	pin_errbuf_t	*ebufp)
{
	int32		mode = MODE_NON_VERBOSE;
	pin_flist_t	*flistp = NULL;
	pin_cookie_t	cookie = 0;
	pin_cookie_t	prev_cookie = 0;
	int32		rec_id = 0;
	int32		param_duration_flag = 0;
	int32		param_type_flag = 0;
	char		*option = 0;
	int32		mta_flags = 0;
	void		*vp = NULL;
	struct tm	tm;
	time_t		end_t=0;
	int32		num_of_months = 0;
	int32		type_of_object = 0;


	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_clean_obj_parse_cmd_opt parameters flist", 
		param_flistp);

	/******************************************************
	 * By default the application should run in non-verbose
	 * mode.
	 ******************************************************/
	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_MODE,
		&mode, ebufp);

	/******************************************************
	 * Get the value of mta_flags in case it was already set
	 ******************************************************/
	vp = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_FLAGS, 1, ebufp);
	if(vp){
		mta_flags = *(int32 *) vp;
	} /******************************************************************
	 *   Walk the PIN_FLD_PARAMS array. Get the PIN_FLD_PARAM_NAME.
	 *   The param name is a command line option.
	 ******************************************************************/

	 while((flistp = PIN_FLIST_ELEM_GET_NEXT(param_flistp, PIN_FLD_PARAMS,
		   &rec_id, 1, &cookie, ebufp))!= NULL) {

		option = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_NAME, 0,
				ebufp);

		if (!option) {
			prev_cookie = cookie;
			continue;
			/*******/
		}

		if(strcmp(option, "-m") == 0) {

			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1,
				ebufp);

			if(vp == 0) {
				fprintf( stderr, "\n\tnumber of months not specified\n" );
				mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
			}else{
			    num_of_months = atoi(vp);
			    PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_NUMBER_OF_UNITS, &num_of_months, ebufp);
			}

			PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS,
				rec_id, ebufp);

			cookie = prev_cookie;
			param_duration_flag = 1;
		} else if (strcmp(option, "-t") == 0) {

                        vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1,
                                ebufp);

                        if(vp == 0) {
                                fprintf( stderr, "\n\ttype of object not specified\n" );
                                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        }else{
                            type_of_object = atoi(vp);
                            PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_TYPE, &type_of_object, ebufp);
                        }

                        PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS,
                                rec_id, ebufp);

                        cookie = prev_cookie;
			if(type_of_object < DAILY_USAGE || type_of_object > SUSP_CDR) {
                            fprintf( stderr, "\n\tInvalid type of object specified\n" );
		    	    param_type_flag = 0;
			} else {
		    	    param_type_flag = 1;
			}
		} else {  /* Discard any unknown option */
			prev_cookie = cookie;
		}
	}

	/* Check if the -f <input file> mandatory option is not supplied */
	if(param_duration_flag == 0 || param_type_flag == 0) {
		mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                fprintf( stderr, "\n\tInvalid/missing command line options..\n" );
	}

	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);

	return;
}

/*******************************************************************
 * Usage information for the specific app.
 * Called prior MTA_USAGE policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_usage(
char	*prog)
{

	pin_errbuf_t	ebuf;
	pin_flist_t	*ext_flistp = NULL;
	char		*usage_str = NULL;

	char            *format = "\nUsage \t %s [-v] \n"
			"\t\t -h help option \n"
			"\t\t -m <number of months> -t <type of object to clean>. Both these parameters are mandatory mandatory.\n\n"
			"\t\t type of object to clean\n"
			"\t\t 1 - Daily Usage Objects\n"
			"\t\t 2 - Trial Bill Report Objects\n"
			"\t\t 3 - Device Lifecycle Objects\n"
			"\t\t 4 - Suspended CDR Objects\n\n"
			"\t\t Example: mso_clean_bb_objects -m 2 -t 3\n";

	PIN_ERRBUF_CLEAR (&ebuf);

	usage_str = (char*)pin_malloc( strlen(format) + strlen(prog) + 1 );

	if (usage_str == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No Memory error");
		return;
	}

	sprintf(usage_str, format ,prog);
	//fprintf( stderr,usage_str);

	ext_flistp =
		pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO,
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
	pin_flist_t	*param_flistp,
	pin_flist_t	*app_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*ext_flistp = NULL;
	void		*vp = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage parameters flist", 
		param_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage application info flist", 
		app_flistp);

	ext_flistp = pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO, ebufp);

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

	pin_flist_t	*s_flistp = NULL;
	int32		s_flags = 0;
	poid_t		*s_pdp = NULL;
	int64		db = 0;
	void		*vp = NULL;
	void		*charge_typep = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*pay_order_arrp = NULL;
	pin_flist_t	*item_arrp = NULL;
	int32    	*db_list = 0;
	char		*str = "";
	time_t		current_time = pin_virtual_time((time_t *)NULL);
	time_t		exp_time = 0;
 	char            ptemplate[300] = "";
	struct tm       *tm1 = 0;
	int32		num_of_months = 0;
	int32		type_of_object = 0;

	
	if (PIN_ERR_IS_ERR(ebufp)) {	return;	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist",  app_flistp);

	type_of_object = *(int32 *)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_TYPE, 0, ebufp);

	current_time = pin_virtual_time(NULL);
	num_of_months = *(int32 *)PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_NUMBER_OF_UNITS, 0, ebufp);

	tm1 = localtime(&current_time);
        tm1->tm_sec = 0;
        tm1->tm_min = 0;
        tm1->tm_hour = 0;
        tm1->tm_isdst = -1;
	if(tm1->tm_mon < num_of_months ) {
	    tm1->tm_mon = tm1->tm_mon - num_of_months + 12;
	    tm1->tm_year -= 1;
	} else {
	    tm1->tm_mon = tm1->tm_mon - num_of_months;
	}
        exp_time = mktime(tm1);

	*s_flistpp = 0;
	s_flistp = PIN_FLIST_CREATE(ebufp);
	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
	if(vp)
		db = PIN_POID_GET_DB ((poid_t*)vp);
		
	s_pdp = PIN_POID_CREATE(db, "/search/pin", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1, ebufp);

	strcat(ptemplate, "select X from ");
	switch(type_of_object) {

	case DAILY_USAGE:
		strcat(ptemplate, "/mso_bb_daily_usage where F1 < V1 ");
        	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_EFFECTIVE_T, &exp_time, ebufp);
		break;
	case TRIAL_BILL:
		strcat(ptemplate, "/mso_trial_report where F1 < V1 ");
        	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CREATED_T, &exp_time, ebufp);
		break;
	case DEVICE_LIFECYCLE:
		strcat(ptemplate, "/mso_lifecycle where F1 < V1 ");
        	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CREATED_T, &exp_time, ebufp);
		break;
	case SUSP_CDR:
		strcat(ptemplate, "/mso_aaa_suspended_bb_event where F1 < V1 ");
        	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CREATED_T, &exp_time, ebufp);
		break;
	}
 	PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_TEMPLATE, ptemplate, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS, 0, ebufp);

	vp = 0;
	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_POID, vp, ebufp);
	
	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"pin_mta_init_search error", ebufp);
		PIN_FLIST_DESTROY_EX (&s_flistp,0);
	}else
	{
		*s_flistpp = s_flistp;
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
	}


    return;
}


/*******************************************************************
 * Function called when new job is avaialble to worker
 * Called prior MTA_WORKER_JOB policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_worker_job(
	pcm_context_t	*ctxp,
	pin_flist_t	*srch_res_flistp,
	pin_flist_t	**op_in_flistpp,
	pin_flist_t	*ti_flistp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*app_flistp = NULL;
	pin_buf_t	*nota_buf = NULL;
	void		*vp = NULL;

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
	pcm_context_t	*ctxp,
	pin_flist_t	*srch_res_flistp,
	pin_flist_t	*op_in_flistp,
	pin_flist_t	**op_out_flistpp,
	pin_flist_t	*ti_flistp,
	pin_errbuf_t	*ebufp)
	{

	pin_flist_t	*bill_corr_i_flistp = NULL;
	pin_flist_t	*app_flistp = NULL;
	poid_t		*routing_pdp = NULL;
	int32		s_flags = 2080;
	int32		reason_id = 2; //2 - price change 3 - manual
        int32		domain_id = 43;
        int32		inv_type = 257;
	pin_flist_t	*del_flistp = NULL;
	pin_flist_t	*del_o_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);


	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode start.");
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search results flist", 
		srch_res_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", 
		op_in_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode thread info flist", 
		ti_flistp);


	del_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_POID, del_flistp, PIN_FLD_POID, ebufp);
	
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, " Calling DELETE_OBJ with input flist",del_flistp);

	PCM_OP(ctxp, PCM_OP_DELETE_OBJ, 0, del_flistp, &del_o_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "pin_mta_worker_job error", ebufp);
        }
        PIN_FLIST_DESTROY_EX(&del_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&del_o_flistp, NULL);

        return;

cleanup:

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode end.");
	return;
}

