/*******************************************************************
 *
 *    Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *    This material is the confidential property of Oracle Corporation
 *    or its licensors and may be used, reproduced, stored or transmitted
 *    only in accordance with a valid Oracle license or sublicense agreement.
 *    ====================================================================
 *    This application is created to resubmit suspended cdr for rating based 
 *    on parameter provided as parameter to MTA. This will remove cdrs from
 *    suspend and send for processing. Main parameter is error code.
 *
 *    Author: Kaliprasad Mahunta
 *    Date: 24-july-2015
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


#define  MODE_NON_VERBOSE       0

static void mso_resbmit_susp_cdr_cmd_opt(
        pin_flist_t     *param_flistp,
        pin_flist_t     *app_flistp,
        pin_errbuf_t    *ebufp);

/*******************************************************************
 * Configuration of application
 * Called prior MTA_CONFIG policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_config(
	pin_flist_t		*param_flistp,
	pin_flist_t		*app_flistp,
	pin_errbuf_t	*ebufp)
{
	int32			rec_id = 0;
	pin_cookie_t	cookie = 0;
	pin_cookie_t	prev_cookie = 0;
	pin_flist_t		*flistp = 0;
	char			*option = 0;
	char			*object_type   = NULL;
	char			*obj_type   = NULL;
	int32			mta_flags = 0;
	int32			err = 0;
	int32			i = 0;
	void			*vp = 0;
	int32			i_param_count = 0;
	char			delimiter = '/';
	char			msg[20];

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

	//get comand line option to search suspend cdr
	mso_resbmit_susp_cdr_cmd_opt(param_flistp,app_flistp,ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_mta_config error", ebufp);
	}
	return;
}

//function to parse command line and throw error if anything missing
static void mso_resbmit_susp_cdr_cmd_opt(
	pin_flist_t	*param_flistp,
	pin_flist_t	*app_flistp,
	pin_errbuf_t	*ebufp)
{

	int32           mode = MODE_NON_VERBOSE;
	int32           mta_flags = 0;
	int32		param_flag = 0;
        int32           rec_id = 0;
        char           *error_code = 0;
	void            *vp = NULL;
        pin_cookie_t    cookie = 0;
        pin_cookie_t    prev_cookie = 0;
        time_t          start_date_passed=0;
        time_t          end_date_passed=1;
	pin_flist_t     *flistp;
	char            *option = 0;
	char            *account_no = NULL;
	char		msg[20];
        char           *date_ptr = 0;
	struct tm       stm ;
	char		*day;

	if(PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG ,"mso_resbmit_susp_cdr_cmd_opt param_flistp",param_flistp);

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
		if(strcmp(option, "-err_code") == 0) {

                        vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

                        if(vp == 0) {
                                fprintf( stderr, "\n\tError Code not specified\n" );
                                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        }else{
                            error_code = (char *)(vp);
                            PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
                        }

                        PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
			param_flag = 1;
			start_date_passed = 1;
			cookie = prev_cookie;

                }else if (strcmp(option, "-start") == 0) {

			start_date_passed = 0;

                        date_ptr = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

                        if(date_ptr == 0) {
                                fprintf( stderr, "\n\tstart time not specified\n" );
                                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        }else{

	                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Start Date Passed STR is:");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, date_ptr);
				day = strtok(date_ptr,"-");
				if(day != NULL){
					stm.tm_mday = atoi(day);
				}
				day = strtok(NULL,"-");
				if(day != NULL){
					stm.tm_mon = atoi(day)-1;
				}
				day = strtok(NULL,"-");
				if(day != NULL){
					stm.tm_year = atoi(day)-1900;
				}
				stm.tm_hour = 0;
				stm.tm_min = 0;
				stm.tm_sec = 0;
	                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Start tm year Passed STR is:");
				//strptime(date_ptr, "DD-MM-YYYY", &stm);
				start_date_passed = mktime(&stm);
                        	//start_date_passed = (time_t) atoi(date_ptr);
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Start Date Passed INT is:");
                        	sprintf(msg,"%ld",start_date_passed);
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);

                        	PIN_FLIST_FLD_SET (app_flistp,  PIN_FLD_START_T, &start_date_passed, ebufp);

                        }
                        PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
			param_flag = 1;
			cookie = prev_cookie;
                }else if (strcmp(option, "-end") == 0){

                        date_ptr = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

                        if(date_ptr == 0) {
                                fprintf( stderr, "\n\tEnd Time not specified\n" );
                                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        }else{

	                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "End Date Passed STR is:");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, date_ptr);
				day = strtok(date_ptr,"-");
				if(day != NULL){
					stm.tm_mday = atoi(day);
				}
				day = strtok(NULL,"-");
				if(day != NULL){
					stm.tm_mon = atoi(day)-1;
				}
				day = strtok(NULL,"-");
				if(day != NULL){
					stm.tm_year = atoi(day)-1900;
				}
				stm.tm_hour = 0;
				stm.tm_min = 0;
				stm.tm_sec = 0;
				end_date_passed = mktime(&stm);
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "End Date Passed INT is:");
                        	sprintf(msg,"%d",end_date_passed);
                        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
                        	if ( end_date_passed == 0) {
                                	end_date_passed = pin_virtual_time((time_t *)NULL);
                        	}
                        	PIN_FLIST_FLD_SET (app_flistp,  PIN_FLD_END_T, &end_date_passed, ebufp);

                        }
                        PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
			param_flag = 1;
			cookie = prev_cookie;
                }else if(strcmp(option, "-account") == 0) {

                        vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

                        if(vp == 0) {
                                fprintf( stderr, "\n\tAccount number not specified\n" );
                                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        }else{

                        	PIN_FLIST_FLD_PUT(app_flistp, PIN_FLD_ACCOUNT_OBJ,
					PIN_POID_CREATE(1, "/account", atof(vp), ebufp), ebufp);
                        }
                        PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
			param_flag = 1;
			cookie = prev_cookie;
                }else {

			//fprintf( stderr, "\n\tInvalid Option\n" );
			prev_cookie = cookie;
                }

	}

        if(param_flag == 0 || start_date_passed == 0 || start_date_passed > end_date_passed) {
                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                //fprintf( stderr, "\n\tInvalid/missing command line options..\n" );
		//exit(1) ;
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

	pin_errbuf_t		ebuf;
	pin_flist_t		*ext_flistp = NULL;
	char			*usage_str = NULL;
	//char			*format = "\nUsage:     %s [-verbose] -object_type </mso_mta_job/xxx> \n\n";
	char			*format = "\nUsage \t %s [-v] \n"
				"\t\t -h help option \n"
				"\t\t -err_code <Error code>.\n"
				"\t\t -start <DD-MM-YYYY>\n"
				"\t\t -end <DD-MM-YYYY>\n"
				"\t\t -account <Account Poid if want to run for particular account>\n";
            
	PIN_ERRBUF_CLEAR (&ebuf);
    
	usage_str = (char*)pin_malloc( strlen(format) + strlen(prog) + 1 );

	if (usage_str == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No Memory error");
		return;
	}

	sprintf(usage_str, format ,prog);
	//fprintf( stderr,usage_str);

	ext_flistp = pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO, &ebuf);
	PIN_FLIST_FLD_SET(ext_flistp, PIN_FLD_DESCR, usage_str, &ebuf);

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
	pin_flist_t		*param_flistp,
	pin_flist_t		*app_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*ext_flistp = NULL;
	void			*vp = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage parameters flist", 
		param_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_usage application info flist", 
		app_flistp);

	ext_flistp =  pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO, ebufp);

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
	pin_flist_t		*app_flistp,
	pin_flist_t		**s_flistpp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
 	char			*template_err = "select X from /mso_aaa_suspended_bb_event where F1 = V1 and F2 = V2 and F3 <> V3 ";
	char			template_time [200];
	//char			*template_time = "select X from /mso_aaa_suspended_bb_event where CREATED_T >= V1 and CREATED_T <= V2";
	//char			*template_err_time = "select X from /mso_aaa_suspended_bb_event where F1 = V1 and CREATED_T >= V2 and CREATED_T <= V3";
	char			template_err_time [200];
	int32			s_flags = 256;
	int32			err_code_flag = 0;
	poid_t			*s_pdp = NULL;
	poid_t			*acc_pdp = NULL;
	char			*error_code = 0;
	int64			db = 0;
	void			*vp = NULL;
	time_t			*start_time = 0;
	time_t			*end_time = 0;
	int32			status = 1;  //Too not to pick new and already processed jobs
	int32			is_aged = 1; //1-Aged CDR, 0-Normal


	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);
	*s_flistpp = 0;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", app_flistp);
	
	s_flistp = PIN_FLIST_CREATE(ebufp);
	/***********************************************************
	 * Simple search flist
	 ***********************************************************
	0 PIN_FLD_POID                      POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS                      INT [0] 256
	0 PIN_FLD_TEMPLATE                   STR [0] "select X from /mso_aaa_suspended_bb_event where F1 = V1"
	0 PIN_FLD_RESULTS                  ARRAY [0] allocated 2, used 2
	0 PIN_FLD_ARGS                     ARRAY [1] allocated 1, used 1
	1     PIN_FLD_ERROR_CODE             STR [0] "56011"
	***********************************************************/

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
	if(vp)
		db = PIN_POID_GET_DB ((poid_t*)vp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1");
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);

	error_code = PIN_FLIST_FLD_GET(app_flistp,PIN_FLD_ERROR_CODE, 1, ebufp);
	start_time = PIN_FLIST_FLD_GET(app_flistp,PIN_FLD_START_T, 1, ebufp);
	end_time = PIN_FLIST_FLD_GET(app_flistp,PIN_FLD_END_T, 1, ebufp);
	acc_pdp = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);

	/***********************************************************
	 * Search Parameters
	 ***********************************************************/

	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);

	if(error_code && *error_code !=0){

		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
                tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &status, ebufp);
                tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TYPE, &is_aged, ebufp);

		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template_err, ebufp);
		err_code_flag = 1;
	}

	if((start_time && *start_time !=0) && err_code_flag == 0){
		
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &status, ebufp);
                tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TYPE, &is_aged, ebufp);

		sprintf(template_time,"select X from /mso_aaa_suspended_bb_event where CREATED_T >= %ld and CREATED_T <= %ld and F1 = V1 and F2 <> V2 ",*start_time,*end_time);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, template_time);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template_time, ebufp);
	}
        else
	if((start_time && *start_time !=0) && acc_pdp && err_code_flag == 0){
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acc_pdp, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_START_T, start_time, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_END_T, end_time, ebufp);
		sprintf(template_err_time,"select X from /mso_aaa_suspended_bb_event where F1 = V1 and  CREATED_T >= %ld and CREATED_T <= %ld",*start_time,*end_time);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, template_time);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template_err_time, ebufp);
	}
        else
	if((start_time && *start_time !=0) && err_code_flag == 1){
		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_START_T, start_time, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_END_T, end_time, ebufp);
                tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &status, ebufp);
                tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
                PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TYPE, &is_aged, ebufp);

		sprintf(template_err_time,"select X from /mso_aaa_suspended_bb_event where F1 = V1 and  CREATED_T >= %ld and CREATED_T <= %ld and F4 = V4 and F5 <> V5 ",*start_time,*end_time);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, template_err_time);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template_err_time, ebufp);
	}
	else
	{
	}
	
	/***********************************************************
	 * Search results
	 ***********************************************************/

	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET( tmp_flistp, PIN_FLD_POID, NULL, ebufp );
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
		PIN_FLIST_DESTROY_EX (&s_flistp,0);
	}
	else
	{
		*s_flistpp = PIN_FLIST_COPY(s_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
	}
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}


/*******************************************************************
 * Search results are modified to consider extra outouts into worker opcode
 * Called after MTA_TUNE policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_tune( pin_flist_t	*app_flistp,
	pin_flist_t	*srch_res_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*rest_flistp=NULL;
	pin_flist_t		*multirest_flistp=NULL;
	pin_cookie_t	cookie = NULL;
	int				elem_id = 0;
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune application info flist", 
		app_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results flist", 
		srch_res_flistp);
	multirest_flistp = PIN_FLIST_ELEM_GET(srch_res_flistp,PIN_FLD_MULTI_RESULTS,PIN_ELEMID_ANY,1,ebufp);
	while((rest_flistp = PIN_FLIST_ELEM_GET_NEXT(multirest_flistp, PIN_FLD_RESULTS,
		&elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL)
	{
		PIN_FLIST_ELEM_COPY(multirest_flistp, PIN_FLD_EXTRA_RESULTS, elem_id + 1, rest_flistp, PIN_FLD_EXTRA_RESULTS, 0, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results modify flist", rest_flistp);
	}	


	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			 "pin_mta_post_tune error", ebufp);
	}
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results output flist", srch_res_flistp);

	return;
}

/*******************************************************************
 * Main application opcode is called here
 *******************************************************************/
PIN_EXPORT void
pin_mta_worker_opcode(
	pcm_context_t		*ctxp,
	pin_flist_t		*srch_res_flistp,
	pin_flist_t		*op_in_flistp,
	pin_flist_t		**op_out_flistpp,
	pin_flist_t		*ti_flistp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t	*in_flistp = NULL;
	pin_flist_t	*del_in_flistp = NULL;
	pin_flist_t	*del_out_flistpp = NULL;
	pin_flist_t	*rd_iflistp = NULL;
	pin_flist_t	*rd_oflistp = NULL;
	pin_flist_t     *upd_flistp = NULL;
	pin_flist_t     *upd_o_flistp = NULL;
	pin_buf_t       *pin_bufp  = NULL;
	int		flags = 1;
	int32		*rating_status = NULL;
	char		*program_name = "mso_resubmit_susp_cdr";
	int32		upd_status = 0;
	int32		is_aged = 1;  //1-Aged CDR
        time_t          current_t = 0;
        time_t          cdr_t = 0;
	void		*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}

	PIN_ERRBUF_CLEAR (ebufp);
	del_in_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search flist", srch_res_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode input flist", op_in_flistp);

	rd_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY( op_in_flistp, PIN_FLD_POID, rd_iflistp, PIN_FLD_POID, ebufp );
	PCM_OP( ctxp, PCM_OP_READ_OBJ, 0, rd_iflistp, &rd_oflistp, ebufp );
	if( PIN_ERR_IS_ERR(ebufp) ){
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
			"pin_mta_worker_opcode: Suspended cdr read error", ebufp );
		goto cleanup;
	}
	
	vp=PIN_FLIST_FLD_GET(rd_oflistp, PIN_FLD_END_T, 1, ebufp);
	if (vp)
		cdr_t = *(time_t *)vp;
        current_t = pin_virtual_time(NULL);	
	upd_flistp = PIN_FLIST_CREATE(ebufp);
	if (current_t-cdr_t > 172800)
	{
	   PIN_ERR_LOG_MSG(3, "Aged CDR");
           PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_POID, upd_flistp, PIN_FLD_POID, ebufp);
           PIN_FLIST_FLD_SET(upd_flistp, PIN_FLD_TYPE, &is_aged, ebufp);
	   PIN_FLIST_FLD_SET(upd_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp );
           PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, upd_flistp, &upd_o_flistp, ebufp);
	}
	PIN_ERR_LOG_MSG(3, "Normal CDR");
		
/*	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_POID, del_in_flistp, PIN_FLD_POID, ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode delete obj input flist", del_in_flistp);
	PCM_OP(ctxp, PCM_OP_DELETE_OBJ, 0, del_in_flistp, &del_out_flistpp, ebufp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode delete obj output flist", del_out_flistpp);
*/
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode buf flist", rd_oflistp);
	pin_bufp = (pin_buf_t *) PIN_FLIST_FLD_GET(rd_oflistp, PIN_FLD_INPUT_FLIST, 0, ebufp );

	/********************************************
	 *      prepare flist for opcode call in 
	 *           pin_mta_worker_opcode
	 *********************************************/

	PIN_ERR_LOG_MSG(3,(char *)pin_bufp->data);
	PIN_STR_TO_FLIST((char *)pin_bufp->data, 1, &in_flistp, ebufp);
	PIN_FLIST_FLD_SET( in_flistp, PIN_FLD_FLAGS, (void *)&flags, ebufp );
	if( PIN_ERR_IS_ERR(ebufp)){
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_mta_worker_opcode error", ebufp);
		goto cleanup;
	}
	PIN_FLIST_FLD_SET( in_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp );
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job prepared flist for main opcode", in_flistp);
	

	/*******************************************
	 *     Calling aaa rating Opcode
	 *******************************************/

	PCM_OP(ctxp, MSO_OP_RATE_AAA_RATE_BB_EVENT, 0, in_flistp, op_out_flistpp, ebufp);


	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_mta_worker_opcode error", ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_ERROR, "pin_mta_worker_job failed flist", in_flistp);
		goto cleanup;
	}else{
		rating_status = PIN_FLIST_FLD_GET(*op_out_flistpp, 
						PIN_FLD_RATING_STATUS, 1, ebufp);
		if (rating_status && *rating_status == 0)
		{
		    PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_POID, del_in_flistp, PIN_FLD_POID, ebufp);

                    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode delete obj input flist", del_in_flistp);
                    PCM_OP(ctxp, PCM_OP_DELETE_OBJ, 0, del_in_flistp, &del_out_flistpp, ebufp);
                    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode delete obj output flist", del_out_flistpp);
		}
		else
		{
		    PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_POID, upd_flistp, PIN_FLD_POID, ebufp);
		    PIN_FLIST_FLD_COPY(*op_out_flistpp, PIN_FLD_ERROR_CODE, upd_flistp, PIN_FLD_ERROR_CODE, ebufp);
		    PIN_FLIST_FLD_COPY(*op_out_flistpp, PIN_FLD_ERROR_DESCR, upd_flistp, PIN_FLD_ERROR_DESCR, ebufp);
		    PIN_FLIST_FLD_SET(upd_flistp, PIN_FLD_STATUS, &upd_status, ebufp);
		    PIN_FLIST_FLD_SET(upd_flistp, PIN_FLD_PROGRAM_NAME, program_name, ebufp );
		    PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, upd_flistp, &upd_o_flistp, ebufp);
		}
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
		"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
	}

	cleanup:
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&rd_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&rd_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&upd_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&upd_o_flistp, NULL);
	return;
}

