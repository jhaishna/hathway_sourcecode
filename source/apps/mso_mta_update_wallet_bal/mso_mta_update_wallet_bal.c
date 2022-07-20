/**************************************************************************
* Description: 
* USAGE: mso_mta_update_wallet_bal [-i] <Input File> [-h or -help]
*
*****************************************************************************/
/*****************************************************************************
*******************************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: mso_mta_update_wallet_bal.c:CUPmod7.3PatchInt:1:2018-Dec-25 06:51:33 %";
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
#include "ops/bill.h"
#include "pin_mta.h"
#include "mso_ops_flds.h"

/*******************************************************************
 * During real application development, there is no necessity to implement 
 * empty MTA call back functions. Application will recognize implemented
 * functions automatically during run-time.
 *******************************************************************/
#define MAXFLDS 200
#define MAXFLDSIZE 100

FILE	*INPUT_FILE = NULL;
FILE	*OUTPUT_FILE = NULL;
int	rec_count = 0;
int	success_count = 0;
int32		*currency_p = NULL;
pin_decimal_t	*credit_limit_p = NULL;
pin_decimal_t	*credit_floor_p = NULL;


void parse(
	char 	*record, 
	char 	arr[][100], 
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
	int32		refund_flags = 0;
	char		*value = NULL;	
	char		*INPUT_FILENAME = NULL;
	char		str[1024];
	char		*output_file = NULL;
	char		fdate[80];
	char		output_fname[255]="";
	char		buffer[1024] ="";
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

				snprintf(str, 1024, "Command line switch -r read. File is for refund");

				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str);
		    	}	
		}
		else if(strcmp(option, "-r") == 0) {
			refund_flags = 1;

			snprintf(str, 1024, "Command line switch -i read. Input file set to '%s'.", INPUT_FILENAME);
			PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_FEE_FLAG, &refund_flags, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, str);

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
	
	pin_conf("mso_mta_update_wallet_bal", "output_file", PIN_FLDT_STR, &output_file, &err);
	pin_conf("mso_mta_update_wallet_bal", "refund_ncr", PIN_FLDT_INT, (caddr_t *)&currency_p, &err);
	pin_conf("mso_mta_update_wallet_bal", "credit_floor", PIN_FLDT_DECIMAL, (caddr_t *)&credit_floor_p, &err);
	pin_conf("mso_mta_update_wallet_bal", "credit_limit", PIN_FLDT_DECIMAL, (caddr_t *)&credit_limit_p, &err);

	pvt = pin_virtual_time((time_t *)NULL);
	strftime(fdate, sizeof(fdate), "-%Y%m%d", localtime(&pvt));
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, fdate);
		
	sprintf(output_fname, "%s", output_file);
	strcat(output_fname, fdate);
	strcat(output_fname, ".csv");

	OUTPUT_FILE = fopen(output_fname, "w");
	fprintf(OUTPUT_FILE, "account_poid,bal_grp_poid,currency,amount\n");
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
			   "\t-r <Refund Flag>\n",
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
	poid_t		*bg_pdp = NULL;

	pin_flist_t	*result_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*actfind_flistp = NULL;
	pin_flist_t	*ractfind_flistp = NULL;
	
	time_t		pvt = (time_t)0;
	int32		currency;

	pcm_context_t 	*ctxp = pin_mta_main_thread_pcm_context_get(ebufp);
	
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);
	
	results_flistp = PIN_FLIST_CREATE(ebufp);

	int			a_cntr = 0;
	char			data[80];

	*s_flistpp = 0;

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", 
					   app_flistp);
			
	while (fgets(buffer, sizeof buffer, INPUT_FILE) != NULL)
	{

		parse(buffer, arr, &fldcnt);
		//a_pdp = PIN_POID_CREATE(1, "/account", (u_int64)(atoi(arr[1])), ebufp);
		//bg_pdp = PIN_POID_CREATE(1, "/balance_group",(u_int64)(atoi(arr[2])), ebufp);
		//sprintf(data,"0.0.0.1 /account %s",arr[1]);
		//a_pdp = PIN_POID_FROM_STR(data, NULL, ebufp);
		//sprintf(data,"0.0.0.1 /balance_group %s",arr[2]);
		//bg_pdp = PIN_POID_FROM_STR(data, NULL, ebufp);
		rec_count = rec_count+1;
		PIN_FLIST_FLD_PUT(results_flistp, PIN_FLD_POID, PIN_POID_CREATE(1, "/account", -1,ebufp), ebufp);
		result_flistp = PIN_FLIST_ELEM_ADD(results_flistp, PIN_FLD_RESULTS, a_cntr++, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ACCOUNT_NO, arr[1], ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_AMOUNT, pbo_decimal_from_str(arr[2], ebufp), ebufp);
		
		vp = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_FEE_FLAG, 1, ebufp);
		if (vp)
		{	
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_FEE_FLAG, vp, ebufp);
		}
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
	void		*vp = NULL;
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

	if(credit_limit_p != NULL) {
		pbo_decimal_destroy(&credit_limit_p);
	}
	if(credit_floor_p != NULL) {
		pbo_decimal_destroy(&credit_floor_p);
	}
	if(currency_p != NULL) {
		free(currency_p);
	}
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_exit error", ebufp);
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

	pin_flist_t		*apply_ncr_bal_flistp = NULL;
	pin_flist_t		*apply_ncr_bal_r_flistp = NULL;
	char		        *account_no = NULL;
	char			*name = NULL;
	char			*output_file = NULL;
	char			fdate[80];
	char			output_fname[255]="";
	char			buffer[1024]="";


	pin_decimal_t		*amnt = NULL;
	pin_flist_t		*total_amtp = NULL;
	pin_flist_t		*debit_flistp = NULL;
	pin_flist_t 		*bal_imp_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*credit_limit_flistp = NULL;
	pin_flist_t		*credit_limit_r_flistp = NULL;
	pin_flist_t		*limit_flistp = NULL;
	pin_flist_t		*rules_flistp = NULL;
	poid_t			*dummy_pdp = NULL;
	poid_t			*account_pdp = NULL;
	poid_t			*item_pdp = NULL;
	int32			succ_flag = 0;
	int32			*status_p = NULL;
	int32			*ref_flags = NULL;
	int32			ref_bal_res_id = 2300001;
	poid_t			*evt_pdp = NULL;
	int64           	db = 1;	
	char           		*amnt_ptr = NULL;	
	char           		data[80] ;	

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

		
	account_no = (char*)PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
	amnt = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	ref_flags = PIN_FLIST_FLD_GET(op_in_flistp, PIN_FLD_FEE_FLAG, 1, ebufp);

	amnt_ptr = pbo_decimal_to_str(amnt, ebufp);
	sprintf(data,"%s,", account_no); strcat(buffer,data);
	sprintf(data,"%s", amnt_ptr); strcat(buffer,data);
	free(amnt_ptr);

	/**********************************************************************
	 * Create refund balance
	 **********************************************************************/
	if (ref_flags && *ref_flags == 1)
	{
		apply_ncr_bal_flistp = PIN_FLIST_CREATE(ebufp);
		dummy_pdp = PIN_POID_CREATE(db, "/dummy", -1, ebufp);
		PIN_FLIST_FLD_PUT(apply_ncr_bal_flistp, PIN_FLD_POID, dummy_pdp, ebufp);
		PIN_FLIST_FLD_SET(apply_ncr_bal_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
		PIN_FLIST_FLD_SET(apply_ncr_bal_flistp, PIN_FLD_PROGRAM_NAME, "mso_mta_update_wallet_bal", ebufp);
        	PIN_FLIST_FLD_SET(apply_ncr_bal_flistp, PIN_FLD_CURRENCY, &ref_bal_res_id, ebufp);
        	PIN_FLIST_FLD_SET(apply_ncr_bal_flistp, PIN_FLD_AMOUNT, amnt, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_mta_update_wallet_bal input flist", apply_ncr_bal_flistp);
	
		PCM_OP(ctxp, MSO_OP_CUST_APPLY_WALLET_BAL, 0, apply_ncr_bal_flistp, &apply_ncr_bal_r_flistp, ebufp);

		PIN_FLIST_DESTROY_EX(&apply_ncr_bal_flistp, NULL);    

		status_p = PIN_FLIST_FLD_GET(apply_ncr_bal_r_flistp, PIN_FLD_STATUS, 0, ebufp);
		if (PIN_ERR_IS_ERR(ebufp) || status_p != NULL && *status_p != 0) {
			strcat(buffer, ",Error occured while calling bill_debit ");
			fprintf(OUTPUT_FILE, "%s\n", buffer);
			fflush(OUTPUT_FILE);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in bill_debit ", ebufp);
		}

		PIN_FLIST_DESTROY_EX(&apply_ncr_bal_r_flistp, NULL);    
	}
 	/**
	===========================================================================
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /dummy -1 0
	0 PIN_FLD_ACCOUNT_NO            STR [0] "1000034827"
	0 PIN_FLD_PROGRAM_NAME          STR [0] "mso_mta_update_wallet_bal"
	0 PIN_FLD_CURRENCY      INT [0] 2300000
	0 PIN_FLD_AMOUNT        DECIMAL [0] -11000
   	===========================================================================
	*/
	apply_ncr_bal_flistp = PIN_FLIST_CREATE(ebufp);
	dummy_pdp = PIN_POID_CREATE(db, "/dummy", -1, ebufp);
	PIN_FLIST_FLD_PUT(apply_ncr_bal_flistp, PIN_FLD_POID, dummy_pdp, ebufp);
	PIN_FLIST_FLD_SET(apply_ncr_bal_flistp, PIN_FLD_ACCOUNT_NO, account_no, ebufp);
	PIN_FLIST_FLD_SET(apply_ncr_bal_flistp, PIN_FLD_PROGRAM_NAME, "mso_mta_update_wallet_bal", ebufp);
        PIN_FLIST_FLD_SET(apply_ncr_bal_flistp, PIN_FLD_CURRENCY, currency_p, ebufp);
        PIN_FLIST_FLD_SET(apply_ncr_bal_flistp, PIN_FLD_AMOUNT, amnt, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_mta_update_wallet_bal input flist", apply_ncr_bal_flistp);
	/**
	*  Calling MSO_OP_CUST_APPLY_WALLET_BAL to apply ncr balance
	*/
	PCM_OP(ctxp, MSO_OP_CUST_APPLY_WALLET_BAL, 0, apply_ncr_bal_flistp, &apply_ncr_bal_r_flistp, ebufp);

	account_pdp = PIN_FLIST_FLD_GET(apply_ncr_bal_r_flistp, PIN_FLD_POID, 0, ebufp);
	status_p = PIN_FLIST_FLD_GET(apply_ncr_bal_r_flistp, PIN_FLD_STATUS, 0, ebufp);

	if (PIN_ERR_IS_ERR(ebufp) || status_p != NULL && *status_p != 0) {
		strcat(buffer, ",Error occured while calling bill_debit ");
		fprintf(OUTPUT_FILE, "%s\n", buffer);
		fflush(OUTPUT_FILE);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in bill_debit ", ebufp);
	}
	else
	{
	        /*===========================================================================
		0 PIN_FLD_POID             POID [0] 0.0.0.1 /account 3178591
		0 PIN_FLD_DESCR             STR [0] ""
		0 PIN_FLD_PROGRAM_NAME      STR [0] "Customer Center"
		0 PIN_FLD_LIMIT           ARRAY [2300000] allocated 20, used 2
		1  PIN_FLD_CREDIT_FLOOR DECIMAL [0] 66000
		1  PIN_FLD_CREDIT_LIMIT DECIMAL [0] 0
        	===========================================================================
        	
        	credit_limit_flistp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_SET(credit_limit_flistp, PIN_FLD_POID, account_pdp, ebufp);
        	PIN_FLIST_FLD_SET(credit_limit_flistp, PIN_FLD_PROGRAM_NAME, "mso_mta_update_wallet_bal", ebufp);
        	PIN_FLIST_FLD_SET(credit_limit_flistp, PIN_FLD_DESCR, "mso_mta_update_wallet_bal", ebufp);
        	limit_flistp = PIN_FLIST_ELEM_ADD(credit_limit_flistp, PIN_FLD_LIMIT, *currency_p, ebufp);
        	PIN_FLIST_FLD_SET(limit_flistp, PIN_FLD_CREDIT_FLOOR, credit_floor_p, ebufp);
        	PIN_FLIST_FLD_SET(limit_flistp, PIN_FLD_CREDIT_LIMIT, credit_limit_p, ebufp);
	
		if (ref_flags && *ref_flags == 1)
		{
        		limit_flistp = PIN_FLIST_ELEM_ADD(credit_limit_flistp, PIN_FLD_LIMIT, ref_bal_res_id, ebufp);
        		PIN_FLIST_FLD_SET(limit_flistp, PIN_FLD_CREDIT_FLOOR, credit_floor_p, ebufp);
        		PIN_FLIST_FLD_SET(limit_flistp, PIN_FLD_CREDIT_LIMIT, credit_limit_p, ebufp);
		}
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_mta_update_wallet_bal set credit limit input flist",
                                                       	credit_limit_flistp); */
        	/**
        	*  Calling PCM_OP_BILL_SET_LIMIT_AND_CR to generate recurring charges
        	PCM_OP(ctxp, PCM_OP_BILL_SET_LIMIT_AND_CR, 0, credit_limit_flistp, &credit_limit_r_flistp, ebufp);
        	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_mta_update_wallet_bal set credit limit out flist", credit_limit_r_flistp);
        	PIN_FLIST_DESTROY_EX(&credit_limit_flistp, NULL);
        	PIN_FLIST_DESTROY_EX(&credit_limit_r_flistp, NULL);

        	if (PIN_ERR_IS_ERR(ebufp)) {
                	strcat(buffer, ",Error occured while calling set_credit_limit ");
                	fprintf(OUTPUT_FILE, "%s\n", buffer);
                	fflush(OUTPUT_FILE);
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                	"Error in seting_credit_limit ", ebufp);
		}
		*/

		succ_flag = 0;
		if(status_p != NULL && *status_p == 0) {
			strcat(buffer, ",SUCCESS");
			fprintf(OUTPUT_FILE,"%s\n", buffer);
			fflush(OUTPUT_FILE);
			success_count = success_count+1;
			succ_flag = 1;
		}
		if(succ_flag == 0) {
			strcat(buffer, "NCR Balance not applied");
                	fprintf(OUTPUT_FILE, "%s\n", buffer);
                	fflush(OUTPUT_FILE);
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                		"NCR Balance not applied", ebufp);
		}
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_mta_update_wallet_bal output flist", apply_ncr_bal_r_flistp);
	PIN_FLIST_DESTROY_EX(&apply_ncr_bal_flistp, NULL);    
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
void parse(char *record, char arr[][100], int *fldcnt)
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
