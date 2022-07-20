/**************************************************************************
* Copyright 2012 Bell Aliant
*
* Created By: Sreekanth Yarraguntla - June 2012
*
* Description: 
*
* USAGE: provisioning_service_reactivation [-i] <Input File> [-h or -help]
*
*****************************************************************************/
/*****************************************************************************
*******************************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: pin_mta_test.c:CUPmod7.3PatchInt:1:2007-Feb-07 06:51:33 %";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcm.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_cust.h"
//#include "fm_bill_utils.h"
#include "ops/pymt.h"
#include "pin_mta.h"

/*******************************************************************
 * During real application development, there is no necessity to implement 
 * empty MTA call back functions. Application will recognize implemented
 * functions automatically during run-time.
 *******************************************************************/
#define MAXFLDS 200
#define MAXFLDSIZE 50

FILE	*INPUT_FILE = NULL;
FILE	*OUTPUT_FILE = NULL;
int	rec_count = 0;
int	success_count = 0;


void parse(
	char 	*record, 
	char 	arr[][50], 
	int 	*fldcnt);

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
	int32		rec_id = 0;
	pin_cookie_t	cookie = 0;
	pin_cookie_t	prev_cookie = 0;
	pin_flist_t	*flistp = 0;
	char		*option = 0;
	int32		mta_flags = 0;
	void		*vp = 0;
	int32		i = 0;
	char		*value = NULL;	
	char		*INPUT_FILENAME = NULL;
	char		str[1024];
	char		*output_file = NULL;
	char		fdate[80];
	char		output_fname[255]="";
	char		buffer[1024];
	time_t		pvt = (time_t)0;
	int		err=0;


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
	mta_flags |= MTA_FLAG_RETURN_WORKER_ERROR;
	while((flistp = PIN_FLIST_ELEM_GET_NEXT(param_flistp, PIN_FLD_PARAMS, &rec_id,
								   1, &cookie, ebufp))!= NULL){
		
		option = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_NAME, 0, ebufp);
		value = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
		/***************************************************
		 * Test options options.
		 ***************************************************/
		if(strcmp(option, "-i") == 0) {
			if (value == 0 || *value == 0) {

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Missing input file name for -i option");

				fprintf(stderr, "%s\n", "Missing input filename for -i option");

				mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
		    	} else {
			 
				INPUT_FILENAME = strdup(value);

				snprintf(str, 1024, "Command line switch -i read. Input file set to '%s'.", INPUT_FILENAME);

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str);

		    	}	

		} else if ((strcmp(option, "-h") == 0) || (strcmp(option, "-help") == 0))
			mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
		else {
			mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;

			snprintf(str, 1024, "Unknown option '%s'", option);
		  	PIN_ERR_LOG_MSG (PIN_ERR_LEVEL_ERROR, str);
			fprintf(stderr, "%s\n", str);
		}
			
	}
	if (INPUT_FILENAME == NULL)  
		mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
	else {
		INPUT_FILE = fopen(INPUT_FILENAME, "r+");

		if (INPUT_FILE == NULL)
		{
			sprintf(str, "Could not open input file '%s'", INPUT_FILENAME);
			fprintf(stderr, "%s", str);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, str);
			mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
		}
	}

	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);
	
	pin_conf("provision_reactivation_mta", "output_file", PIN_FLDT_STR,
					&output_file, &err);

	pvt = pin_virtual_time((time_t *)NULL);
	strftime(fdate, sizeof(fdate), "-%Y%m%d", localtime(&pvt));
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, fdate);
		
	sprintf(output_fname, "%s", output_file);
	strcat(output_fname, fdate);
	strcat(output_fname, ".csv");

	OUTPUT_FILE = fopen(output_fname, "w");
	fprintf(OUTPUT_FILE, "Login,Status,Deal Name\n");
	fflush(OUTPUT_FILE);


	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_config error", ebufp);
	}
	return;
}

/*******************************************************************
 * Post configuration of application
 * Called after MTA_CONFIG policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_config(
		pin_flist_t		*param_flistp,
		pin_flist_t		*app_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_config parameters flist", 
					   param_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_config application info flist", 
					   app_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_config error", ebufp);
	}
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
	char		usage_str[512] = {""};

	PIN_ERRBUF_CLEAR (&ebuf);

	sprintf(usage_str,"\nUsage: %s \n"
			   "\t-i <Input File>\n",
			   "\t-h or -help\n", prog);
	ext_flistp =
		pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO,
											   &ebuf);

	PIN_FLIST_FLD_SET (ext_flistp, PIN_FLD_DESCR, usage_str, &ebuf)

	if (PIN_ERR_IS_ERR(&ebuf)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_usage error", &ebuf);
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

	ext_flistp =
		pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO,ebufp);
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
 * Initialization of application
 * Called prior MTA_INIT_APP policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_init_app(
		pin_flist_t		*app_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_app application info flist", 
					   app_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_init_app error", ebufp);
	}
	return;
}

/*******************************************************************
 * Post initialization of application
 * Called after MTA_INIT_APP policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_init_app(
		pin_flist_t		*app_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_init_app application info flist", 
					   app_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_init_app error", ebufp);
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

	int32 		line_counter = 0;
	int32 		fldcnt=0;
	int32		err=0;

	char   	buffer[1024];
	char          arr[MAXFLDS][MAXFLDSIZE]={0x0};
	char		a_pdp_str[32];
	char 		*input_file = NULL;
	char 		*output_file = NULL;
	char		fdate[80];
	char		output_fname[255]="";

	void		*vp = NULL;

	poid_t		*a_pdp = NULL;
	poid_t		*s_pdp = NULL;

	pin_flist_t	*result_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*actfind_flistp = NULL;
	pin_flist_t	*ractfind_flistp = NULL;
	
	time_t		pvt = (time_t)0;

	pcm_context_t 	*ctxp = pin_mta_main_thread_pcm_context_get(ebufp);
	
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);
	
	results_flistp = PIN_FLIST_CREATE(ebufp);

	int			a_cntr = 0;

	*s_flistpp = 0;

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", 
					   app_flistp);
			
	while (fgets(buffer, sizeof buffer, INPUT_FILE) != NULL)
	{

		parse(buffer, arr, &fldcnt);
		rec_count = rec_count+1;
		a_pdp = PIN_POID_CREATE(1, "/account", (u_int64)(atoi(arr[4])), ebufp);
		s_pdp = PIN_POID_CREATE(1, arr[3],(u_int64)(atoi(arr[2])), ebufp);
		PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, PIN_POID_CREATE(1, "/service", -1,ebufp), ebufp);
		result_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_RESULTS, a_cntr++, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_LOGIN, arr[1], ebufp);
		PIN_FLIST_FLD_PUT(result_flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);
		PIN_FLIST_FLD_PUT(result_flistp, PIN_FLD_SERVICE_OBJ, s_pdp, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DEAL_OBJ, PIN_POID_CREATE(1, "/deal", (u_int64)(atoi(arr[5])), ebufp), ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_PRODUCT_OBJ, PIN_POID_CREATE(1, "/product", (u_int64)(atoi(arr[7])), ebufp), ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_NAME, arr[6], ebufp);
		
		
	}
	fclose(INPUT_FILE);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_init_search error", ebufp);

		PIN_FLIST_DESTROY_EX (&results_flistp, ebufp);
	}else{
		*s_flistpp = results_flistp;

		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", 
						   results_flistp);
	}
	/*fprintf(OUTPUT_FILE, "%s\n", buffer);
	fflush(OUTPUT_FILE); */

	return;
}

/*******************************************************************
 * Application defined search criteria.
 * Called after MTA_INIT_SEARCH policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_init_search(
		pin_flist_t		*app_flistp,
		pin_flist_t		*search_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_init_search application info flist", 
					   app_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_init_search search flist", 
					   search_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_init_search error", ebufp);
	}
	return;
}

/*******************************************************************
 * Search results may be updated, modified or enriched
 * Called prior MTA_TUNE policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_tune(
		pin_flist_t		*app_flistp,
		pin_flist_t		*srch_res_flistp,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t	*multi_res_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	int32		i = 0;
	int32		mta_flags = 0;
	void		*vp = 0;
	pin_cookie_t	s_cookie = 0;
	int32		s_rec_id = 0;
	pin_cookie_t	m_cookie = 0;
	int32		m_rec_id = 0;
	int64		db = 0;
	int64		id = 0;
	char		*type = 0;
	poid_t		*pdp = 0;
	poid_t		*n_pdp = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_tune application info flist", 
					   app_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_tune search results flist", 
					   srch_res_flistp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_OPS_ERROR, MTA_OPTIONAL,ebufp);
	if(vp)
		i = *((int32*)vp);

	if (i) {

		while((multi_res_flistp = 
			   PIN_FLIST_ELEM_GET_NEXT(srch_res_flistp, PIN_FLD_MULTI_RESULTS,
									   &s_rec_id, 1, &s_cookie, ebufp)) != NULL) {

			m_cookie = 0;
			while((tmp_flistp = 
				   PIN_FLIST_ELEM_GET_NEXT(multi_res_flistp, PIN_FLD_RESULTS,
										   &m_rec_id, 1, &m_cookie, ebufp)) != NULL) {

				vp = PIN_FLIST_FLD_TAKE (tmp_flistp, PIN_FLD_POID, 0, ebufp);

				if(vp) {
					pdp = (poid_t*)vp;
					db = PIN_POID_GET_DB (pdp);
					id = PIN_POID_GET_ID (pdp);
					type = (char *)PIN_POID_GET_TYPE (pdp);
					id = id * i;
					n_pdp = PIN_POID_CREATE (db, type,id, ebufp);
					PIN_POID_DESTROY (pdp, ebufp);

					PIN_FLIST_FLD_PUT (tmp_flistp, PIN_FLD_POID, n_pdp, ebufp);
				}
			}
		}
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_tune error", ebufp);
	}
	return;
}

/*******************************************************************
 * Search results may be updated, modified or enriched
 * Called after MTA_TUNE policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_tune(
		pin_flist_t		*app_flistp,
		pin_flist_t		*srch_res_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune application info flist", 
					   app_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_tune search results flist", 
					   srch_res_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_tune error", ebufp);
	}
	return;
}

/*******************************************************************
 * Function executed after seach results were processed
 * Called prior MTA_JOB_DONE policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_job_done(
		pin_flist_t		*app_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_job_done application info flist", 
					   app_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_job_done error", ebufp);
	}
	return;
}

/*******************************************************************
 * Function executed after seach results were processed
 * Called after MTA_JOB_DONE policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_job_done(
		pin_flist_t		*app_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_job_done application info flist", 
					   app_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_job_done error", ebufp);
	}
	return;
}

/*******************************************************************
 * Function executed at application exit
 * Called prior MTA_EXIT policy opcode
 *******************************************************************/
PIN_EXPORT void
pin_mta_exit(
		pin_flist_t		*app_flistp,
		pin_errbuf_t	*ebufp)
{
	char	record_count[255];
	char	suc_count[255];
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_exit application info flist", 
					   app_flistp);
	sprintf(record_count, "Total Records Processed: %d", rec_count);

	fprintf(OUTPUT_FILE, "\n\n=========================================================\n");
	fprintf(OUTPUT_FILE, "\nDetailed Report is given below\n\n");
	fprintf(OUTPUT_FILE,"%s\n", record_count);
	sprintf(suc_count, "Successful Records Count: %d", success_count);
	fprintf(OUTPUT_FILE,"%s\n", suc_count);
	fprintf(OUTPUT_FILE, "\n=========================================================\n");
	fflush(OUTPUT_FILE);
	fclose(OUTPUT_FILE);
	
	rec_count = 0;
	success_count = 0;
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_exit error", ebufp);
	}
	return;
}

/*******************************************************************
 * Function executed at application exit
 * Called after MTA_EXIT policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_exit(
		pin_flist_t		*app_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_exit application info flist", 
					   app_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_exit error", ebufp);
	}
	return;
}

/*******************************************************************
 * Initialization of worker thread
 * Called prior MTA_WORKER_INIT policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_worker_init(
		pcm_context_t	*ctxp,
		pin_flist_t		*ti_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_init thread info flist", 
					   ti_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_worker_init error", ebufp);
	}
	return;
}

/*******************************************************************
 * Post initialization of worker thread
 * Called after MTA_WORKER_INIT policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_worker_init(
		pcm_context_t	*ctxp,
		pin_flist_t		*ti_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_init thread info flist", 
					   ti_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_worker_init error", ebufp);
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
		pin_flist_t		*srch_res_flistp,
		pin_flist_t		**op_in_flistpp,
		pin_flist_t		*ti_flistp,
		pin_errbuf_t	*ebufp)
{

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job search results flist", 
					   srch_res_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job thread info flist", 
					   ti_flistp);


	*op_in_flistpp = PIN_FLIST_COPY (srch_res_flistp, ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job prepared flist for main opcode", 
					   *op_in_flistpp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_worker_job error", ebufp);
	}
	return;
}


/*******************************************************************
 * Function called when new job is avaialble to worker
 * Called after MTA_WORKER_JOB policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_worker_job(
		pcm_context_t	*ctxp,
		pin_flist_t		*srch_res_flistp,
		pin_flist_t		*op_in_flistp,
		pin_flist_t		*ti_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_job search results flist", 
					   srch_res_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_job thread info flist", 
					   ti_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_job prepared flist for main opcode", 
					   op_in_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_worker_job error", ebufp);
	}
	return;
}

/*******************************************************************
 * Main application opcode is called here
 * Here our main logic will be written to update the services
 *******************************************************************/
PIN_EXPORT void 
pin_mta_worker_opcode(
		pcm_context_t	*ctxp,
		pin_flist_t		*srch_res_flistp,
		pin_flist_t		*op_in_flistp,
		pin_flist_t		**op_out_flistpp,
		pin_flist_t		*ti_flistp,
		pin_errbuf_t	*ebufp)
{

	pin_flist_t		*update_serv_ipflist = NULL;
	pin_flist_t		*update_serv_opflist = NULL;
	pin_flist_t		*service_array_flistp = NULL;
	pin_flist_t		*update_serv_active_ipflist = NULL;
	pin_flist_t		*update_serv_active_opflist = NULL;
	pin_flist_t		*dealinfo_iflistp = NULL;
	pin_flist_t		*deal_purchase_iflistp = NULL;
	pin_flist_t		*deal_purchase_oflistp = NULL;
	pin_flist_t		*deal_products_flistp = NULL;
	pin_flist_t		*products_oflistp = NULL;
	pin_flist_t		*services_oflistp = NULL;

    	poid_t			*nova_cancel_poid = NULL;
	poid_t		       *account_poid = NULL;
	poid_t			*service_poid = NULL;
	poid_t			*deal_poid = NULL;
	poid_t			*prod_poid = NULL;
	poid_t			*offering_obj = NULL;

	char			search_template[256];
	char			pvt_string[50];
	char			*name = NULL;
	char			*output_file = NULL;
	char			fdate[80];
	char			output_fname[255]="";
	char			buffer[1024];
	char			*login = NULL;


	int			flag = 768;
	int			status_flag = 4;
	int			p_status = 1; 
	int			c_status = 10103;
	int			r_status = 10100;
	int			count = 0;
	int			cnt = 0;
	int			err = 0;
	int32			date_tstamp = 0;
	int32			vp = 0;
	
	pin_decimal_t		*quantity = NULL;


	time_t          pvt	= (time_t)0;


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

	
	service_poid = (poid_t *)PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	account_poid = (poid_t *)PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	deal_poid = (poid_t *)PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_DEAL_OBJ, 0, ebufp);
	prod_poid = (poid_t *)PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_PRODUCT_OBJ, 0, ebufp);
	name = (char *)PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_NAME, 0, ebufp);
	login = (char *)PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_LOGIN, 0, ebufp);
	sprintf(buffer, "%s", login);
 	/**
	===========================================================================
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 840096556 232
	0 PIN_FLD_PROGRAM_NAME    STR [0] "MTA APP FOR PROVISION"
	0 PIN_FLD_DESCR           STR [0] "[Upgraded Plan] "
	0 PIN_FLD_SERVICES      ARRAY [0] allocated 20, used 3
	1     PIN_FLD_STATUS_FLAGS    INT [0] 4
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /service/broadband 1059876219 36
	1     PIN_FLD_STATUS         ENUM [0] 10103
   	===========================================================================
	*/
	update_serv_ipflist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(update_serv_ipflist, PIN_FLD_POID, account_poid, ebufp);
	PIN_FLIST_FLD_SET(update_serv_ipflist, PIN_FLD_PROGRAM_NAME, "PROV SERV REACT MTA", ebufp);
	PIN_FLIST_FLD_SET(update_serv_ipflist, PIN_FLD_DESCR, "[Voluntary Disconnect]", ebufp);
	service_array_flistp = PIN_FLIST_ELEM_ADD(update_serv_ipflist, PIN_FLD_SERVICES, 0, ebufp);
	PIN_FLIST_FLD_SET(service_array_flistp, PIN_FLD_POID, service_poid, ebufp);
	PIN_FLIST_FLD_SET(service_array_flistp, PIN_FLD_STATUS, &c_status, ebufp);
	PIN_FLIST_FLD_SET(service_array_flistp, PIN_FLD_STATUS_FLAGS, &status_flag, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provisioning_reactive_service closing services input flist", 
							update_serv_ipflist);
	/**
	*  Calling PCM_OP_CUST_UPDATE_SERVICES to update services
	*/
	PCM_OP(ctxp, 86, 0, update_serv_ipflist, &update_serv_opflist, ebufp);
	PIN_FLIST_DESTROY_EX(&update_serv_ipflist, NULL);
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		* Log something and return nothing.
		***************************************************/
		strcat(buffer, ",Error occured while cancelling the Service");
		fprintf(OUTPUT_FILE, "%s\n", buffer);
		fflush(OUTPUT_FILE);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"Error in updating the services", ebufp);
		
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provisioning_reactive_service closing services output flist", 
							update_serv_opflist);
	    
	/**
	==========================================================================
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 840096556 234
	0 PIN_FLD_PROGRAM_NAME    STR [0] "MTA PROV APP"
	0 PIN_FLD_DESCR           STR [0] "[Temporarily Inactive] "
	0 PIN_FLD_SERVICES      ARRAY [0] allocated 20, used 3
	1     PIN_FLD_STATUS_FLAGS    INT [0] 4
	1     PIN_FLD_POID           POID [0] 0.0.0.1 /service/broadband 1059876219 39
	1     PIN_FLD_STATUS         ENUM [0] 10100
	==========================================================================
	*/
	if(update_serv_opflist != NULL){
		services_oflistp = PIN_FLIST_ELEM_GET(update_serv_opflist, PIN_FLD_SERVICES,PIN_ELEMID_ANY, 0, ebufp);
		if(services_oflistp != NULL){
			update_serv_active_ipflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(update_serv_active_ipflist, PIN_FLD_POID, account_poid, ebufp);
			PIN_FLIST_FLD_SET(update_serv_active_ipflist, PIN_FLD_PROGRAM_NAME, "PROV SERV REACT MTA", ebufp);
			PIN_FLIST_FLD_SET(update_serv_active_ipflist, PIN_FLD_DESCR, "[Temporarily Inactive]", ebufp);
			service_array_flistp = PIN_FLIST_ELEM_ADD(update_serv_active_ipflist, PIN_FLD_SERVICES, 0, ebufp);
			PIN_FLIST_FLD_SET(service_array_flistp, PIN_FLD_POID, service_poid, ebufp);
			PIN_FLIST_FLD_SET(service_array_flistp, PIN_FLD_STATUS, &r_status, ebufp);
			PIN_FLIST_FLD_SET(service_array_flistp, PIN_FLD_STATUS_FLAGS, &status_flag, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provisioning_reactive_service reactivate service input flist",
									update_serv_active_ipflist);
	
			/**
			*  Calling PCM_OP_CUST_UPDATE_SERVICES to update /service
			*/
			PCM_OP(ctxp, 86, 0, update_serv_active_ipflist, &update_serv_active_opflist, ebufp);
			PIN_FLIST_DESTROY_EX(&update_serv_active_ipflist, NULL);
			if (PIN_ERR_IS_ERR(ebufp)) {

				/***************************************************
				* Log something and return nothing.
				***************************************************/
				strcat(buffer, ",Error occured while reactivating the Service");
				fprintf(OUTPUT_FILE, "%s\n", buffer);
				fflush(OUTPUT_FILE);
	
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Error in reactivating service", ebufp);
			
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provisioning_reactive_service reactivate output flist", 
									update_serv_active_opflist);
			if(update_serv_active_opflist != NULL){
				services_oflistp = PIN_FLIST_ELEM_GET(update_serv_active_opflist, PIN_FLD_SERVICES,PIN_ELEMID_ANY, 0, ebufp);
				if(services_oflistp != NULL){
					/*******************************************************************
					* =====Input flist for PCM_OP_SUBSCRIPTION_PURCHASE_DEALS===========
					*0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 840096556 235
					*0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/broadband 1059876219 40
					*0 PIN_FLD_PROGRAM_NAME    STR [0] "MTA PROV APP"
					*0 PIN_FLD_DEAL_INFO    SUBSTRUCT [0] allocated 20, used 7
					*1     PIN_FLD_PRODUCTS      ARRAY [0] allocated 32, used 32
					*2         PIN_FLD_PURCHASE_END_T TSTAMP [0] (0) <null>
					*2         PIN_FLD_PURCHASE_START_T TSTAMP [0] (0) <null>
					*2         PIN_FLD_QUANTITY     DECIMAL [0] 1
					*2         PIN_FLD_PRODUCT_OBJ    POID [0] 0.0.0.1 /product 818204862 0
					*2         PIN_FLD_USAGE_DISCOUNT DECIMAL [0] 0
					*2         PIN_FLD_CYCLE_DISCOUNT DECIMAL [0] 0
					*2         PIN_FLD_PURCHASE_DISCOUNT DECIMAL [0] 0
					*2         PIN_FLD_STATUS         ENUM [0] 1
					*2         PIN_FLD_STATUS_FLAGS    INT [0] 0
					*2         PIN_FLD_USAGE_END_T  TSTAMP [0] (0) <null>
					*2         PIN_FLD_USAGE_START_T TSTAMP [0] (0) <null>
					*2         PIN_FLD_CYCLE_END_T  TSTAMP [0] (0) <null>
					*2         PIN_FLD_CYCLE_START_T TSTAMP [0] (0) <null>
					*1     PIN_FLD_NAME            STR [0] "High-Speed Lite"
					*1     PIN_FLD_POID           POID [0] 0.0.0.1 /deal 818208446 0
					***********************************************************************/
					deal_purchase_iflistp = PIN_FLIST_CREATE(ebufp);
					PIN_FLIST_FLD_SET(deal_purchase_iflistp, PIN_FLD_POID, account_poid, ebufp);
					PIN_FLIST_FLD_SET(deal_purchase_iflistp, PIN_FLD_PROGRAM_NAME, "PROV SERV REACT MTA", ebufp);
					PIN_FLIST_FLD_SET(deal_purchase_iflistp, PIN_FLD_SERVICE_OBJ, service_poid, ebufp);
					dealinfo_iflistp = PIN_FLIST_SUBSTR_ADD(deal_purchase_iflistp, PIN_FLD_DEAL_INFO, ebufp);
					PIN_FLIST_FLD_SET(dealinfo_iflistp, PIN_FLD_POID, deal_poid, ebufp);
					PIN_FLIST_FLD_SET(dealinfo_iflistp, PIN_FLD_NAME, name, ebufp);
					PIN_FLIST_FLD_SET(dealinfo_iflistp, PIN_FLD_START_T, date_tstamp, ebufp);
					PIN_FLIST_FLD_SET(dealinfo_iflistp, PIN_FLD_END_T, date_tstamp, ebufp);
					deal_products_flistp = PIN_FLIST_ELEM_ADD(dealinfo_iflistp, PIN_FLD_PRODUCTS, 0, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_PRODUCT_OBJ, prod_poid, ebufp);
					quantity = pbo_decimal_from_str("0", ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_QUANTITY, quantity, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_PURCHASE_START_T, date_tstamp, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_PURCHASE_END_T, date_tstamp, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_USAGE_START_T, date_tstamp, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_USAGE_END_T, date_tstamp, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_CYCLE_END_T, date_tstamp, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_CYCLE_START_T, date_tstamp, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_USAGE_DISCOUNT, quantity, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_CYCLE_DISCOUNT, quantity, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_PURCHASE_DISCOUNT, quantity, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_STATUS, &p_status, ebufp);
					PIN_FLIST_FLD_SET(deal_products_flistp, PIN_FLD_STATUS_FLAGS, &vp, ebufp);
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provisioning_reactive_service purchasing deal input flist", 
											deal_purchase_iflistp);
	
					/**
					*  Calling PCM_OP_SUBSCRIPTION_PURCHASE_DEAL to purchase deal
					*/
					PCM_OP(ctxp, 108, 0, deal_purchase_iflistp, &deal_purchase_oflistp, ebufp);
					PIN_FLIST_DESTROY_EX(&deal_purchase_iflistp, NULL);
		
					if (PIN_ERR_IS_ERR(ebufp)) {
	
						/***************************************************
						* Log something and return nothing.
						***************************************************/
						strcat(buffer, ",Error occured while Purchasing the Deal");
						fprintf(OUTPUT_FILE, "%s\n", buffer);
						fflush(OUTPUT_FILE);
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"Error in purchasing deal", ebufp);	
					}
					else{
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "provisioning_reactive_service purchasing deal output flist", 
												deal_purchase_oflistp);
						if(deal_purchase_oflistp != NULL){
							products_oflistp = PIN_FLIST_ELEM_GET(deal_purchase_oflistp, PIN_FLD_PRODUCTS,PIN_ELEMID_ANY, 0, ebufp);
							if(products_oflistp != NULL){
								offering_obj = PIN_FLIST_FLD_GET(products_oflistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);
								if(PIN_POID_GET_ID(offering_obj) > 1){
									PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "DEAL PURCHASED SUCCESSFULLY");
									strcat(buffer, ",SUCCESS,");
									strcat(buffer, name);
									fprintf(OUTPUT_FILE,"%s\n", buffer);
									fflush(OUTPUT_FILE);
									success_count = success_count+1;
								}
							}
						}
					
					}
				
				}
			}
		}
	}
	
	PIN_FLIST_DESTROY_EX(&deal_purchase_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&update_serv_active_opflist, NULL);
	PIN_FLIST_DESTROY_EX(&update_serv_opflist, NULL);
	return;
}

/*******************************************************************
 * Function called when worker completed assigned job
 * Called prior MTA_WORKER_JOB_DONE policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_worker_job_done(
		pcm_context_t	*ctxp,
		pin_flist_t		*srch_res_flistp,
		pin_flist_t		*op_in_flistp,
		pin_flist_t		*op_out_flistp,
		pin_flist_t		*ti_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done search results flist", 
					   srch_res_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done prepared flist for main opcode", 
					   op_in_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done output flist from main opcode", 
					   op_out_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job_done thread info flist", 
					   ti_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_worker_job_done error", ebufp);
	}
	return;
}

/*******************************************************************
 * Function called when worker completed assigned job
 * Called after MTA_WORKER_JOB_DONE policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_worker_job_done(
		pcm_context_t	*ctxp,
		pin_flist_t		*srch_res_flistp,
		pin_flist_t		*op_in_flistp,
		pin_flist_t		*op_out_flistp,
		pin_flist_t		*ti_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_job_done search results flist", 
					   srch_res_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_job_done prepared flist for main opcode", 
					   op_in_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_job_done output flist from main opcode", 
					   op_out_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_job_done thread info flist", 
					   ti_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_worker_job_done error", ebufp);
	}
	return;
}

/*******************************************************************
 * Worker thread exit function
 * Called prior MTA_WORKER_EXIT policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_worker_exit(
		pcm_context_t	*ctxp,
		pin_flist_t		*ti_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_exit thread info flist", 
					   ti_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_worker_exit error", ebufp);
	}
	return;
}

/*******************************************************************
 * Worker thread exit function
 * Called after MTA_WORKER_EXIT policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_post_worker_exit (
		pcm_context_t	*ctxp,
		pin_flist_t		*ti_flistp,
		pin_errbuf_t	*ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_worker_exit thread info flist", 
					   ti_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_worker_exit error", ebufp);
	}
	return;
}


/*******************************************************************
 * Obsolete callback API , supported for backward compatibility
 * There are several drawbacks in keeping using obsolete function,
 * one of them obsolete functions do not match MTA policy opcodes ...
 *******************************************************************/
/*******************************************************************
 * User-defined initialization.
 * Add user specified arguments or parameters to arg_flistp.
 * Should not be used , since there is better approach with pin_mta_config
 * and pin_mta_post_config, pin_mta_init_app and pin_mta_post_init_app.
 *******************************************************************/
PIN_EXPORT void
pin_mta_init_job(
		int				argc,
		char			*argv[],
		int				*work_modep,
		pin_flist_t		**arg_flistpp,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t	*arg_flistp = NULL;
	int32 i = argc;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	*arg_flistpp = 0;

	*work_modep = MTA_WORKMODE_SINGLE;

	arg_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET (arg_flistp, PIN_FLD_ATTRIBUTE, &i , ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_init_job error", ebufp);

		PIN_FLIST_DESTROY_EX (&arg_flistp,0);
	}else {
		*arg_flistpp = arg_flistp;

		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_job arguments flist", 
						   arg_flistp);
	}

	return;
}

/*******************************************************************
 * User-defined hotlist initialization.
 * Should not be used , since there is better approach with search flist
 * option reading from a file.
 *******************************************************************/
PIN_EXPORT void
pin_mta_init_hotlist(
		pin_flist_t		*app_flistp,
		pin_flist_t		**hot_flistpp,
		pin_errbuf_t	*ebufp )
{

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_hotlist application info flist", 
					   app_flistp);

	*hot_flistpp = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_init_hotlist error", ebufp);
	}
	return;
}

/*******************************************************************
 * Do the real job for each work unit.
 * Should not be used, better approach pin_mta_worker_opcode 
 * and rest of pin_mta_worker_* functions
 *******************************************************************/	
PIN_EXPORT void
pin_mta_exec_work_unit(
		pcm_context_t	*ctxp,
		pin_flist_t		*app_flistp,
		pin_flist_t		*i_flistp,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	int32		i = 1;
	int32		mta_flags = 0;
	void		*vp = 0;
	pin_cookie_t	cookie = 0;
	int32		rec_id = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_exec_work_unit application info flist", 
					   app_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_exec_work_unit input flist", 
					   i_flistp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FLAGS, 0,ebufp);
	if(vp)
		mta_flags = *((int32*)vp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_INCR_AMOUNT, 1,ebufp);
	if(vp)
		i = *((int32*)vp);


	if(mta_flags & MTA_FLAG_BATCH_MODE) {

		while((tmp_flistp = 
			   PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS,
									   &rec_id, 1, &cookie, ebufp)) != NULL) {

			PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_HEADER_NUM, &i, ebufp);
			PCM_OP (ctxp, PCM_OP_INC_FLDS, 0, tmp_flistp, &r_flistp, ebufp);


			PIN_FLIST_DESTROY_EX (&r_flistp, ebufp);
		}


	} else {
		tmp_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

		PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_HEADER_NUM, &i, ebufp);

		PCM_OP (ctxp, PCM_OP_INC_FLDS, 0, tmp_flistp, &r_flistp, ebufp);

		PIN_FLIST_DESTROY_EX (&tmp_flistp, 0);
		PIN_FLIST_DESTROY_EX (&r_flistp, ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_exec_work_unit error", ebufp);
	}
	return;
}

/*******************************************************************
 * Cleanup. 
 * Should not be used, better approach is to use pin_mta_exit and 
 * pin_mta_post_exit; in such cases, no need to destroy app_flist :-)
 * doing this to test backward compatibility, just in case ...
 * and destroying passed copy of app_flistp ...
 *******************************************************************/
PIN_EXPORT void
pin_mta_cleanup_job(
		pin_flist_t		*app_flistp,
		pin_errbuf_t	*ebufp)
{

	pin_flist_t	*app_flistp_x = app_flistp;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_clean_up application info flist", 
					   app_flistp);

	PIN_FLIST_DESTROY_EX (&app_flistp_x, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_cleanup_job error", ebufp);
	}
	return;
}
void parse(char *record, char arr[][50], int *fldcnt)
{
	char line1[1024];
	char line2[1024];
	char line3[1024];
	char *stptr;
	int flag = 0;
	int idx = 0;
	int lcount = 0;
	int count =0;
	
	strcpy(line2,record);
	stptr = line2;

	while (*stptr != '\0')
	{
		lcount++;
		idx = 0;
		while (*stptr != '\0' && *stptr != ',')
		{
			line3[idx] = *stptr;
			idx++;
			stptr++;
		}
		line3[idx] = '\0';
		if (*stptr != '\0' && *stptr == ',')
			stptr++;
		strcpy(line2,stptr);
		stptr = line2;

		strcpy(arr[lcount],line3);
	}
	*fldcnt=lcount+1;
}
