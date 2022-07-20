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
#include <time.h>

#include "pcm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_mta.h"
#include "pin_flds.h"
#include "mso_ops_flds.h"
#include "ops/rate.h"
#include "pin_rate.h"
#include "ops/tcf.h"
#include "pcm.h"

#define  MODE_NON_VERBOSE       0

static void mso_clean_reservations_cmd_opt(
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
	pin_cookie_t		cookie = 0;
	pin_cookie_t		prev_cookie = 0;
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
	mso_clean_reservations_cmd_opt(param_flistp,app_flistp,ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_mta_config error", ebufp);
	}
	return;
}

//function to parse command line and throw error if anything missing
static void mso_clean_reservations_cmd_opt(
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
        char         	*expiration = NULL;
	pin_flist_t     *flistp;
	char            *option = 0;
	char		msg[20];
        char           *date_ptr = 0;
	char			*account_no = NULL;
	struct tm       stm ;
	char		*day;
	time_t 		expiration_time = 0;
	time_t		expiration_seconds = 0;	
	time_t		curr_time = 0;
	int32		expiration_passed = 0;

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
                if (strcmp(option, "-expiration") == 0) {

			PIN_ERR_LOG_MSG(3, "Inside Expiration Option");
			expiration_passed = 1;

                        expiration = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

                        if(!expiration) {
                                fprintf( stderr, "\n\texpiration not specified\n" );
                                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        }else{
	                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Expiration Passed STR is:");
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, expiration);
	 			curr_time = pin_virtual_time(NULL);
                                expiration_seconds = atoi(expiration)*3600;
				expiration_time = curr_time - expiration_seconds; 

                        	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_END_T, &expiration_time, ebufp);
                        }
                        PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
			param_flag = 1;
			cookie = prev_cookie;
                }else if(strcmp(option, "-account") == 0) {

                        account_no = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

                        if(!account_no) {
                                fprintf( stderr, "\n\tAccount Poid not specified\n" );
                                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
                        }else{
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account No Passed STR is:");
							PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, account_no);
                        	PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
                        }
                        PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
			param_flag = 1;
			cookie = prev_cookie;
                }else {
			prev_cookie = cookie;
                }

	}

        if(param_flag == 0 || expiration_time == 0) {
                mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
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
				"\t\t -expiration <Hours>\n"
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
	pin_flist_t	*app_flistp,
	pin_flist_t	**s_flistpp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
 	char			*template = "select X from /mso_usage_session where F1 <> V1 and F2 <= V2 ";
	char                    *template_acct = "select X from /mso_usage_session where F1 <> V1 and F2 <= V2 and F3 = V3 ";
	int32			s_flags = 256;
	int32			err_code_flag = 0;
	poid_t			*s_pdp = NULL;
	poid_t			*pdp = NULL;
	char			*error_code = 0;
	int64			db = 0;
	void			*vp = NULL;
	time_t			*expiration_time = 0;
	int32			status = 2;  //Too not to pick already stopped sessions 
	poid_t			*account_pdp = NULL;
	char			*account_id = NULL;


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

	expiration_time = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_END_T, 1, ebufp);
	account_id = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);

	if (account_id)
		account_pdp = PIN_POID_CREATE(db, "/account", atof(account_id), ebufp);
	
	/***********************************************************
	 * Search Parameters
	 ***********************************************************/

	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &status, ebufp);

	if(!expiration_time && *expiration_time == 0){
		return;	
	}
		
    	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
       	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_END_T, expiration_time, ebufp);
	if (account_pdp)
	{
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template_acct, ebufp);
		tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, account_pdp, ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	}
	
	/***********************************************************
	 * Search results
	 ***********************************************************/

	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	
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
	pin_flist_t     *sess_info_flistp = NULL;
	pin_flist_t     *ext_info_flistp = NULL;
	pin_flist_t		*upd_iflistp = NULL;
	pin_flist_t		*upd_oflistp = NULL;
	int32		stop_rec = 2;
	int32		term_cause = 10;
	int32		*rating_status = NULL;
	char		*prg_name = "mso_clean_reservation";
	char		*event_type = "broadband";
	//pin_flist_t	*rd_iflistp = NULL;
	//pin_flist_t	*rd_oflistp = NULL;
	//pin_flist_t     *bb_info_flistp = NULL;
    //    time_t          current_t = 0;
	//time_t		end_t = 0;
	//void		*vp = NULL;
	//pin_decimal_t	*zero = pbo_decimal_from_str("0.0", ebufp);
	//pin_decimal_t	*bytes_uplink = NULL;
	//pin_decimal_t   *bytes_downlink = NULL;
	//pin_decimal_t   *rated_bytes_uplink = NULL;
	//pin_decimal_t   *rated_bytes_dnlink = NULL;
	//pin_decimal_t   *total_uplink = NULL;
	//pin_decimal_t   *total_downlink = NULL;

	//char		log_time[32];
	//char		*aggr_num = NULL;
	//char		*format = "%a %b %d %H:%M:%S IST %Y";
	
	//struct tm lt;


	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}

	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search flist", srch_res_flistp);
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode input flist", op_in_flistp);


    /*    rd_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_SERVICE_OBJ, rd_iflistp, PIN_FLD_POID, ebufp );
        PCM_OP( ctxp, PCM_OP_READ_OBJ, 0, rd_iflistp, &rd_oflistp, ebufp );
        if( PIN_ERR_IS_ERR(ebufp) ){
                PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
                        "pin_mta_worker_opcode: Service read error", ebufp );
                goto cleanup;
        }

	bb_info_flistp = PIN_FLIST_ELEM_GET(rd_oflistp, MSO_FLD_BB_INFO, PIN_ELEMID_ANY, 0, ebufp);
        aggr_num = PIN_FLIST_FLD_GET(bb_info_flistp, MSO_FLD_AGREEMENT_NO, 0, ebufp);

	bytes_uplink = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_BYTES_UPLINK, 0, ebufp);
	rated_bytes_uplink = PIN_FLIST_FLD_GET(op_in_flistp, MSO_FLD_BYTES_UPLINK_BEF_FUP, 0, ebufp);
	
	total_uplink = pbo_decimal_add(bytes_uplink, rated_bytes_uplink, ebufp);

	bytes_downlink = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_BYTES_DOWNLINK, 0, ebufp);
	rated_bytes_dnlink = PIN_FLIST_FLD_GET(op_in_flistp, MSO_FLD_BYTES_DOWNLINK_BEF_FUP, 0, ebufp);
	
	total_downlink = pbo_decimal_add(bytes_downlink, rated_bytes_dnlink, ebufp); 

	
	end_t = *(time_t *)PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_END_T, 0, ebufp);
	localtime_r(&end_t, &lt);	
    	if (strftime(log_time, sizeof(log_time), format, &lt) == 0) {
                PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
                        "pin_mta_worker_opcode: Error Parsing Date", ebufp );
                goto cleanup;
                return;
        }		
	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_POID, in_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, MSO_FLD_FRAMED_IP_ADDRESS, in_flistp, MSO_FLD_FRAMED_IP_ADDRESS, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_DURATION_SECONDS, in_flistp, PIN_FLD_DURATION_SECONDS, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_ACTIVE_SESSION_ID, in_flistp, PIN_FLD_AUTHORIZATION_ID, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, MSO_FLD_NE_IP_ADDRESS, in_flistp, MSO_FLD_NE_IP_ADDRESS, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, MSO_FLD_NE_ID, in_flistp, MSO_FLD_NE_ID, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_USER_NAME, in_flistp, PIN_FLD_USER_NAME, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, MSO_FLD_REMOTE_ID, in_flistp, MSO_FLD_REMOTE_ID, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_SERVICE_CODE, in_flistp, PIN_FLD_SERVICE_CODE, ebufp );
	PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_OUTPUT_GIGA_WORDS, zero, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_INPUT_GIGA_WORDS, zero, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_REQ_MODE, &stop_rec, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_AGREEMENT_NO, aggr_num, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TERMINATE_CAUSE, &term_cause, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, MSO_FLD_LOG_TIMESTAMP, log_time, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_BYTES_UPLINK, total_uplink, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_BYTES_DOWNLINK, total_downlink, ebufp);
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, prg_name, ebufp);*/

	in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_SERVICE_OBJ, in_flistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_PROGRAM_NAME, prg_name, ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_ACTIVE_SESSION_ID, in_flistp, PIN_FLD_AUTHORIZATION_ID, ebufp );
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_OBJ_TYPE, event_type, ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_START_T, in_flistp, PIN_FLD_START_T, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_END_T, in_flistp, PIN_FLD_END_T, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_QUANTITY, in_flistp, PIN_FLD_QUANTITY, ebufp );
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_REQ_MODE, &stop_rec, ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_BYTES_UPLINK, in_flistp, PIN_FLD_BYTES_UPLINK, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_BYTES_DOWNLINK, in_flistp, PIN_FLD_BYTES_DOWNLINK, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_ACCOUNT_OBJ, in_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp );
	ext_info_flistp = PIN_FLIST_SUBSTR_ADD(in_flistp, PIN_FLD_EXTENDED_INFO, ebufp);
	sess_info_flistp = PIN_FLIST_ELEM_ADD(ext_info_flistp, PIN_FLD_SESSION_INFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_USER_NAME, sess_info_flistp, PIN_FLD_USER_NAME, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, MSO_FLD_FRAMED_IP_ADDRESS, sess_info_flistp, MSO_FLD_FRAMED_IP_ADDRESS, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, MSO_FLD_NE_IP_ADDRESS, sess_info_flistp, MSO_FLD_NE_IP_ADDRESS, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, MSO_FLD_NE_ID, sess_info_flistp, MSO_FLD_NE_ID, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_SERVICE_CODE, sess_info_flistp, PIN_FLD_SERVICE_CODE, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, MSO_FLD_REMOTE_ID, sess_info_flistp, MSO_FLD_REMOTE_ID, ebufp );
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_DURATION_SECONDS, in_flistp, PIN_FLD_DURATION_SECONDS, ebufp );
	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_TERMINATE_CAUSE, &term_cause, ebufp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job prepared flist for main opcode", in_flistp);
	

	/*******************************************
	 *     Calling Stop Accounting Opcode
	 *******************************************/

	//PCM_OP(ctxp, MSO_OP_RATE_AAA_RATE_BB_EVENT, 0, in_flistp, op_out_flistpp, ebufp);
	PCM_OP(ctxp, PCM_OP_TCF_AAA_STOP_ACCOUNTING, 0, in_flistp, op_out_flistpp, ebufp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "Rating Output", *op_out_flistpp);

	if(*op_out_flistpp != NULL)
		rating_status = PIN_FLIST_FLD_GET(*op_out_flistpp, PIN_FLD_RATING_STATUS, 1, ebufp);
	
	if (PIN_ERR_IS_ERR(ebufp) || (rating_status && *rating_status != 0)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_worker_opcode error", ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
                "pin_mta_worker_opcode output flist", *op_out_flistpp);
		goto CLEANUP;
	}

	upd_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(op_in_flistp, PIN_FLD_POID, upd_iflistp, PIN_FLD_POID, ebufp );
	PIN_FLIST_FLD_SET(upd_iflistp, PIN_FLD_STATUS, &stop_rec, ebufp );
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "update usage_session input", upd_iflistp);
	PCM_OP( ctxp, PCM_OP_WRITE_FLDS, 0, upd_iflistp, &upd_oflistp, ebufp );
	if( PIN_ERR_IS_ERR(ebufp) ){
		PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
				"pin_mta_worker_opcode: Usage Session update error", ebufp );
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "update usage_session output", upd_oflistp);
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&upd_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&upd_oflistp, NULL);
	return;
}

