/*******************************************************************
 *
 *	Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *	This material is the confidential property of Oracle Corporation
 *	or its licensors and may be used, reproduced, stored or transmitted
 *	only in accordance with a valid Oracle license or sublicense agreement.
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
#include "ops/base.h"

/*******************************************************************
* Apllication constants 
********************************************************************/
#define  MODE_VERBOSE		1
#define  MODE_NON_VERBOSE	0
#define  CSV_FIELD_COUNT	6

/*******************************************************************
 * Macros for mapping field to ARRAY INDEX
 *******************************************************************/
#define ACCOUNT_ID	0
#define ACCOUNT_NO	1
#define LCO_ACC_ID	2
#define LCO_ACC_NO	3
#define REF_ID		4
#define	CURRENCY_BAL	5



/*******************************************************************
 * Functions used within
 *******************************************************************/
static void mso_pymt_autopay_cmd_opt();
static char *unquote ();
static char *trim ();
static char *get_tok ();
static char *trim_comment_from_flist ();

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
	mta_flags = mta_flags | MTA_FLAG_VERSION_NEW;
	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);

	/***********************************************************
	 * Get command line options for allication
	 ***********************************************************/
	mso_pymt_autopay_cmd_opt(param_flistp, app_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_config error", ebufp);
	}
	return;
}

static void
mso_pymt_autopay_cmd_opt(
	pin_flist_t	*param_flistp,
	pin_flist_t	*app_flistp,
	pin_errbuf_t	*ebufp)
{
	int32		mode = MODE_NON_VERBOSE;
	pin_flist_t	*flistp = NULL;
	pin_cookie_t	cookie = 0;
	pin_cookie_t	prev_cookie = 0;
	int32		rec_id = 0;
	int32		file_option_flag = 0;
	char		*option	= 0;
	int32		mta_flags = 0;
	void		*vp = NULL;
	char		*pch = NULL;
	char            sbuf[1024];
	char            *input_file_name = NULL;
	time_t 		rawtime;
	struct tm 	*timeinfo;


	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

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
	}

	/******************************************************************
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

		if(strcmp(option, "-v") == 0) {	
		/* Verbose option selected */

			mode = MODE_VERBOSE;
			PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_MODE,
				&mode, ebufp);

			PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS,
				rec_id,	ebufp);

			cookie = prev_cookie;

		}else if(strcmp(option, "-h") == 0) {	
		/* Help option selected */

			mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;

			PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS,
				rec_id, ebufp);

			PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS,
				&mta_flags, ebufp);
			return;

		}else if(strcmp(option, "-f") == 0) {	
		/* Specify file name */

			file_option_flag = 1;
			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1,
				ebufp);

			if(vp == 0) {
				fprintf( stderr,
					"\n\tFile name not specified "
					"after -f option.\n\n" );
				mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
			}else{

				PIN_FLIST_FLD_SET(app_flistp,
					PIN_FLD_INPUT_FILE_NAME, vp, ebufp);
			}

			PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS,
				rec_id, ebufp);

			cookie = prev_cookie;
		}
		else {  /* Discard any unknown option */
			prev_cookie = cookie;
		}
	}

	/* Check if the -f <input file> mandatory option is not supplied */
	if(file_option_flag == 0) {
		fprintf( stderr, "\n\tMandatory option -f missing\n\n" );
		mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
	}

	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);

	/* SET search flist file name and failed file name */
	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FILENAME, 
				(void *)"pin_mta_search.flist", ebufp);

	input_file_name = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_INPUT_FILE_NAME,
				1, ebufp);

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
	pch=strrchr(input_file_name,'/');
	if(pch){
		sprintf(sbuf, "output/failed_%02d%02d%04d%02d%02d%02d_%s", timeinfo->tm_mday,
			timeinfo->tm_mon + 1,timeinfo->tm_year + 1900,timeinfo->tm_hour,timeinfo->tm_min,
			timeinfo->tm_sec, pch + 1);
	}
	else{
                sprintf(sbuf, "output/failed_%02d%02d%04d%02d%02d%02d_%s", timeinfo->tm_mday,
                        timeinfo->tm_mon + 1,timeinfo->tm_year + 1900,timeinfo->tm_hour,timeinfo->tm_min,
                        timeinfo->tm_sec, input_file_name);
	}

	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_ERROR_FILE_NAME,
				(void *)sbuf, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"bulk_update_profile_cmd_opt error", ebufp);
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
	char		*usage_str = NULL;

	char		*format = "\nUsage:\t %s [-v] \n"
			"\t\t -h help option \n"
			"\t\t -f <input file name> This is mandatory.\n";

	PIN_ERRBUF_CLEAR (&ebuf);
	
	usage_str = (char*)pin_malloc( strlen(format) + strlen(prog) + 1 );

	if (usage_str == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No Memory error");
		return;
	}

	sprintf(usage_str, format ,prog);
	fprintf( stderr,usage_str);

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
 * Initialization of application
 * Called prior MTA_INIT_APP policy opcode
 *******************************************************************/
PIN_EXPORT void 
pin_mta_init_app(
		pin_flist_t	*app_flistp,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t	*oper_flistp = NULL;
	pin_flist_t	*result_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	pin_flist_t     *bulk_autopay_flistp = NULL;
	pin_flist_t     *r_bulk_autopay_flistp = NULL;
	pin_flist_t     *values_flistp = NULL;
	pin_flist_t     *common_flistp = NULL;
	FILE		*fread = NULL;
	FILE		*fwrite	= NULL;
	FILE		*ferror = NULL;
	int32		flist_len = 0;
	char		*str_flist = NULL;
	char		*input_file_name = NULL;
	char            *search_file_name = NULL;
	char            *error_file_name = NULL;
	void		*vp = NULL;
	int32		mode = 0;
	int32		row_no = 0;
	char		buf[1024];
	char		sbuf[1024];
	char		curr_rec[1024];
	char		delims[] = "|";
	char		*q = NULL;
	char		*tok = NULL;
	char 		*fields[CSV_FIELD_COUNT];
	int32 		nfield = 0;
	int32 		type = 0;
	int32		any_valid_record = 0;
	int32		failed_count = 0;
	int32		something_to_update = 0;
	int32		val_count = 0;
	poid_t		*poid_ptr = NULL;
	poid_t		*lco_pdp = NULL;
	int64		database = 0;
	pin_decimal_t	*dec_val = NULL;
	char		last_lco_no[64] = {0};
	time_t		coll_t = 0;
	pcm_context_t   *ctxp = 0;


	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"pin_mta_init_app application info flist", app_flistp);

	ctxp = pin_mta_main_thread_pcm_context_get (ebufp);

	/******************************************************
	 * Read verbose flag
	 ******************************************************/
	vp = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_MODE, 1, ebufp);
	if(vp){
		mode = *(int32 *) vp;
	}
	if(mode == MODE_VERBOSE){
		fprintf(stdout, "\n\tApplication execution started.\n");
	}

	/****************************************************
	 * Get the Operation Info by locking 
	 ****************************************************/
	oper_flistp = pin_mta_global_flist_node_get_with_lock(
			PIN_FLD_OPERATION_INFO, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"pin_mta_init_app application oper_flistp", oper_flistp);

	/*****************************************************
	 * Fields of the application scope.
	 *****************************************************/
	PIN_FLIST_FLD_SET(oper_flistp, PIN_FLD_PROCESS_NAME,
			(void *) "mso_pymt_autopay_mta" , ebufp);
	PIN_FLIST_FLD_SET(oper_flistp,PIN_FLD_PROGRAM_NAME,
			(void *) "mso_pymt_autopay_mta" , ebufp);

	/******************************************************
	 * Initialise the fields for storing the application
	 * statistics.
	 ******************************************************/
	vp = (void *)NULL;
    	PIN_FLIST_FLD_SET(oper_flistp, PIN_FLD_SUCCESSFUL_RECORDS, vp, ebufp);
    	PIN_FLIST_FLD_SET(oper_flistp, PIN_FLD_FAILED_RECORDS, vp, ebufp);
    	PIN_FLIST_FLD_SET(oper_flistp, PIN_FLD_NUM_TOTAL_RECORDS, vp, ebufp);

	/*****************************************************
	 * Open input CVS file to read.
	 *****************************************************/

	input_file_name = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_INPUT_FILE_NAME,
				1, ebufp);

	sprintf(sbuf, "\n\tOpening the input file: %s\n",
			input_file_name ?	(char *) input_file_name: "");

	if(mode == MODE_VERBOSE){
		fprintf(stdout, sbuf);
	}

	sprintf(sbuf,"\n\tChecking for invalid records.......\n");

	if(mode == MODE_VERBOSE){
		fprintf(stdout, sbuf);
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sbuf);

	if (input_file_name) {
		fread = fopen( (char *) input_file_name, "r" );
	}

	if ( fread == (FILE *) 0 ){

		pin_set_err(ebufp, PIN_ERRLOC_APP, 
			PIN_ERRCLASS_APPLICATION,
			PIN_ERR_FILE_IO,
			PIN_FLD_INPUT_FILE_NAME, 0, 0);

		sprintf(sbuf, "\n\tpin_mta_init_app error: Unable to open "
				"the input file: %s\n\n",
				(char *) input_file_name);

		fprintf(stderr, sbuf);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, sbuf, ebufp);

		exit(1);
	}

        /*****************************************************
         * Open intermediate  file to write.
         *****************************************************/

        search_file_name = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_FILENAME,
                                1, ebufp);

	if (search_file_name) {
		fwrite = fopen( (char *) search_file_name, "w" );
	}

	if ( fwrite == (FILE *) 0 ){

		pin_set_err(ebufp, PIN_ERRLOC_APP, 
			PIN_ERRCLASS_APPLICATION,
			PIN_ERR_FILE_IO,
			PIN_FLD_FILENAME, 0, 0);

		sprintf(sbuf, "\n\tpin_mta_init_app error: Unable to open "
				"the file %s for writing.\n\n",
				(char *) search_file_name);

		fprintf(stderr, sbuf);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, sbuf, ebufp);

		exit(2);
	}

	/*****************************************************
	 * Try to open a file for writing the erroneous CSV
	 * records
	 *****************************************************/

	error_file_name = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_ERROR_FILE_NAME,
				1, ebufp);

	sprintf(sbuf, "\n\tOpening the file %s for writing Invalid Records\n",
			error_file_name ?	(char *) error_file_name:"");

	if(mode == MODE_VERBOSE){
	//	fprintf(stdout, sbuf);
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sbuf);

	if (error_file_name) {
		ferror = fopen( (char *) error_file_name, "w" );
	}

	if ( ferror == (FILE *) 0 ){

		pin_set_err(ebufp, PIN_ERRLOC_APP, 
			PIN_ERRCLASS_APPLICATION,
			PIN_ERR_FILE_IO,
			PIN_FLD_ERROR_FILE_NAME, 0, 0);

		sprintf(sbuf, "\n\tpin_mta_init_app error: Unable to open "
				"the file %s for writing invalid records.\n\n",
				(char *) error_file_name);

		fprintf(stderr, sbuf);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, sbuf, ebufp);

		exit(2);
	}

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL,
			 MTA_MANDATORY, ebufp);
	if(vp){
		database = PIN_POID_GET_DB ((poid_t*)vp);
	}

	/************************************************************
	 * Create bulk flistp for the mso_lco_autopay_master class
	 ************************************************************/
	bulk_autopay_flistp = PIN_FLIST_CREATE(ebufp);
	lco_pdp = PIN_POID_CREATE(database, "/mso_lco_autopay_master", (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(bulk_autopay_flistp, PIN_FLD_POID, (void *)lco_pdp, ebufp);
	common_flistp = PIN_FLIST_ELEM_ADD(bulk_autopay_flistp, PIN_FLD_COMMON_VALUES, 0, ebufp);
	PIN_FLIST_FLD_SET ( common_flistp, MSO_FLD_AMOUNT_TOTAL, NULL,  ebufp);
	strcpy(last_lco_no, "DUMMY_ACCOUNT");
	

	row_no = 0; /****no rows read yet****/
	while (fgets(buf, sizeof(buf), fread) != NULL){
		row_no++;
		nfield = 0;
		strcpy(curr_rec, buf); /* Since buf gets modified */

		/**************************************************************
		 *  Build an array of fields with the values from the line read.
		 **************************************************************/
		for ( q = buf; ( tok = get_tok( q ) ) != NULL; q = NULL ){
			if( nfield == CSV_FIELD_COUNT ) {
				break;
			}
			fields[nfield++] = unquote(trim(tok));
		}
		/**************************************************************
		 *  Check to see if the CSV record is a valid length record.
		 *  If not, report an error and increment the failed_count.
		 **************************************************************/
		if(nfield != CSV_FIELD_COUNT ){

			pin_set_err(ebufp, PIN_ERRLOC_APP, 
				PIN_ERRCLASS_APPLICATION,
				PIN_ERR_BAD_ARG,
				PIN_FLD_RESULTS, 0, 0);

			sprintf(sbuf, "\tpin_mta_init_app error: Invalid record"
				" found at line %d for account %s\n", row_no, 
				(char *) fields[ACCOUNT_NO]);

			if(mode == MODE_VERBOSE){
				//fprintf(stdout, sbuf); 
			}

			/* Write failed CSV record to the error file */
			sprintf(sbuf, "<RECORD %d>\n"
					"\tRecord format not proper.\n",row_no);
			fprintf(ferror,sbuf);
			fprintf(ferror, curr_rec);

			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, sbuf, ebufp);
			PIN_ERRBUF_CLEAR(ebufp);

			failed_count++;
			continue;
		}

		/***************create search flist *****************/
		result_flistp = PIN_FLIST_CREATE( ebufp );
		flistp = PIN_FLIST_ELEM_ADD(result_flistp, PIN_FLD_RESULTS,
				row_no - 1, ebufp);

		poid_ptr = PIN_POID_CREATE( database, "/account", atoi64(fields[ACCOUNT_ID]), ebufp);
		PIN_FLIST_FLD_PUT( flistp, PIN_FLD_POID, (void *)poid_ptr, ebufp );
		PIN_FLIST_FLD_SET( flistp, PIN_FLD_ACCOUNT_NO,
					(void *) fields[ACCOUNT_NO], ebufp );
                poid_ptr = PIN_POID_CREATE( database, "/account", atoi64(fields[LCO_ACC_ID]), ebufp);
                PIN_FLIST_FLD_PUT( flistp, MSO_FLD_LCO_OBJ, (void *)poid_ptr, ebufp );
		PIN_FLIST_FLD_SET( flistp, MSO_FLD_ACCOUNT_OWNER,
					(void *) fields[LCO_ACC_NO], ebufp );

		if (last_lco_no && (strcmp(last_lco_no, fields[LCO_ACC_NO]) != 0)){
			strcpy(last_lco_no, fields[LCO_ACC_NO]);
			values_flistp = PIN_FLIST_ELEM_ADD(bulk_autopay_flistp, PIN_FLD_VALUES, ++val_count, ebufp);
			PIN_FLIST_FLD_SET(values_flistp, PIN_FLD_ACCOUNT_NO, (void *) fields[LCO_ACC_NO], ebufp);
			vp =  PIN_FLIST_FLD_GET ( flistp, MSO_FLD_LCO_OBJ, 0, ebufp);
			PIN_FLIST_FLD_SET(values_flistp, MSO_FLD_LCO_OBJ, vp, ebufp);
			PIN_FLIST_FLD_SET(values_flistp, PIN_FLD_REFERENCE_ID, (void *) fields[REF_ID], ebufp );
			coll_t = atoi64(fields[REF_ID]);
			PIN_FLIST_FLD_SET( values_flistp, MSO_FLD_PYMT_COLLECTION_T, (void *)&coll_t, ebufp);
		}
		PIN_FLIST_FLD_SET( flistp, PIN_FLD_REFERENCE_ID,
					(void *) fields[REF_ID], ebufp );
		dec_val = pbo_decimal_from_str( fields[CURRENCY_BAL], ebufp);
		PIN_FLIST_FLD_PUT( flistp, PIN_FLD_CURRENT_BAL,
                                                (void *) dec_val, ebufp);
		PIN_FLIST_FLD_SET( flistp, PIN_FLD_ELEMENT_ID,
					&row_no, ebufp );
		PIN_FLIST_FLD_SET( flistp, PIN_FLD_DESCR,
					(void *)curr_rec, ebufp );

		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
				"result_flistp parameters flist",
					result_flistp);

		if(str_flist != (char *) 0) {
			free(str_flist);
			str_flist = (char *) 0;
		}

		PIN_FLIST_TO_STR(result_flistp, &str_flist, &flist_len, ebufp);

		fputs(trim_comment_from_flist(str_flist), fwrite);
		any_valid_record = 1;
		PIN_FLIST_DESTROY_EX(&result_flistp, NULL);		
	}

	/*****************************************************
	 * Free the memory since no longer required.
	 *****************************************************/
	free(str_flist);
	str_flist = NULL;
	
	if (any_valid_record == 0){
		sprintf(sbuf, "\n\tInput file empty or records not proper.\n ");

		if(mode == MODE_VERBOSE){
			fprintf(stdout, sbuf); 
		}
	}

	/*********************************************************
	 * Call BULK CREATE opcode
	 *********************************************************/
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
				"bulk_autopay_flistp flistp", bulk_autopay_flistp);
	PCM_OP(ctxp, PCM_OP_BULK_CREATE_OBJ, 0, bulk_autopay_flistp, &r_bulk_autopay_flistp, ebufp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
                                "r_bulk_autopay_flistp flistp", r_bulk_autopay_flistp);
	PIN_FLIST_DESTROY_EX(&bulk_autopay_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_bulk_autopay_flistp, NULL);

	/******************************************************
	 * Store the No. of records that failed processing at
	 * this stage.
	 ******************************************************/
	PIN_FLIST_FLD_SET(oper_flistp, PIN_FLD_FAILED_RECORDS,
			&failed_count, ebufp);
	/******************************************************
	 * Total No. of records.
	 ******************************************************/
	PIN_FLIST_FLD_SET(oper_flistp, PIN_FLD_NUM_TOTAL_RECORDS,
			&row_no, ebufp);

	sprintf(sbuf,
		"\n\tRead %d record(s) from the CSV file, "
		"out of which %d was/were erroneous.\n", row_no, failed_count);

	if(mode == MODE_VERBOSE){
		fprintf(stdout, sbuf);
	}

	pin_mta_global_flist_node_release(PIN_FLD_OPERATION_INFO,ebufp);

	/******************************************************
	 * Close input file.
	 ******************************************************/
	if ( fread != (FILE *) 0 ){
		fclose(fread);
	}

	/**************************************************************
	 * Close search file.
	 **************************************************************/
	if ( fwrite != (FILE *) 0 ){
		fclose(fwrite);
	}

	/**************************************************************
	 * Close error file.
	 **************************************************************/
	if ( ferror != (FILE *) 0 ){
		fclose(ferror);
	}
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_app application info flist", 
					   app_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_init_app error", ebufp);
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

	pin_flist_t	*s_flistp = NULL;
	char		sbuf[255];
	void		*vp = NULL;
	pin_flist_t	*res_flistp = (pin_flist_t *) 0;
	char		*file_name = NULL;
	int32		field_count = CSV_FIELD_COUNT + 2;
	int32		mode = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", 
					   app_flistp);

	s_flistp = PIN_FLIST_CREATE(ebufp);
	/******************************************************
	 * Read and set the verbose flag
	 ******************************************************/
	vp = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_MODE, 1, ebufp);
	if(vp){
		mode = *(int32 *) vp;
	}

	file_name = (char *) PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_FILENAME,
				0, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FILENAME,
				(void *) file_name, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_COUNT,
				(void *) &field_count, ebufp);

	sprintf(sbuf, "\n\tDistributing the task among "
			"the worker threads.......\n");

	if(mode == MODE_VERBOSE){
		fprintf(stdout, sbuf);
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sbuf);


	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_init_search error", ebufp);

		PIN_FLIST_DESTROY_EX (&s_flistp,0);
	}else{
		*s_flistpp = s_flistp;

		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", 
						   s_flistp);
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
	pin_flist_t	*oper_flistp = NULL;
	pin_flist_t	*thread_flistp	= NULL;
	pin_flist_t	*err_flistp = NULL;
	pin_flist_t	*rec_flistp = NULL;
	pin_flist_t	*sort_flistp = NULL;
	pin_flist_t	*temp_flistp = NULL;
	void		*vp = NULL;
	int32		success_count = 0;
	int32		failed_count = 0;
	int32		total_count = 0;
	int32		rec_id	= 0;
	pin_cookie_t	cookie	= NULL;
	char		*err_file_name	= NULL;
	FILE		*ferr		= NULL;
	int32		mode		= 0;
	char		sbuf[1024];
	char		buf[1024];

	int32		rec_id1	= 0;
	pin_cookie_t	cookie1	= NULL;
	int32		mta_flags = 0;


	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_post_exit application info flist", 
					   app_flistp);

	/******************************************************
	 * Get the value of PIN_FLD_FLAGS if flag is set to
	 * MTA_FLAG_USAGE_MSG then return.
	 ******************************************************/
	vp = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_FLAGS, 0, ebufp);

	if(vp){
		mta_flags = *(int32 *)vp;
	}

	if(mta_flags & MTA_FLAG_USAGE_MSG){
		return;
	}	

	vp = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_MODE, 0, ebufp);
	if(vp){
		mode = *(int32 *)vp;
	}

	oper_flistp = pin_mta_global_flist_node_get_with_lock(
				PIN_FLD_OPERATION_INFO, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"xxxxxxx application oper_flistp", oper_flistp);

	vp = PIN_FLIST_FLD_GET(oper_flistp,
			PIN_FLD_SUCCESSFUL_RECORDS, 0, ebufp);
	if(vp){
		success_count += *(int32 *)vp;
	}

	vp = PIN_FLIST_FLD_GET(oper_flistp, PIN_FLD_FAILED_RECORDS,
		0, ebufp);
	if(vp){
		failed_count += *(int32 *)vp;
	}
	vp = PIN_FLIST_FLD_GET(oper_flistp, PIN_FLD_NUM_TOTAL_RECORDS,
		0, ebufp);
	if(vp){
		total_count = *(int32 *)vp;
	}


        /*****************************************************
         * Try to open a file for writing the erroneous CSV
         * records
         *****************************************************/

        err_file_name = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_ERROR_FILE_NAME,
                                1, ebufp);

        sprintf(sbuf, "\n\tOpening the file %s for writing Invalid Records\n",
                        err_file_name ? (char *) err_file_name:"");

        if(mode == MODE_VERBOSE){
                //fprintf(stdout, sbuf);
        }

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, sbuf);

        if (err_file_name) {
                ferr = fopen( (char *) err_file_name, "a" );
        }

        if ( ferr == (FILE *) 0 ){

                pin_set_err(ebufp, PIN_ERRLOC_APP,
                        PIN_ERRCLASS_APPLICATION,
                        PIN_ERR_FILE_IO,
                        PIN_FLD_ERROR_FILE_NAME, 0, 0);

                sprintf(sbuf, "\n\tpin_mta_post_exit error: Unable to open "
                                "the file %s for writing invalid records.\n\n",
                                (char *) err_file_name);

                fprintf(stderr, sbuf);

                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, sbuf, ebufp);

                exit(2);
        }


	/***************************************************************
	 * Loop through the thread specific info and consolidate
	 ***************************************************************/
	rec_id = 0;
	cookie = NULL;
	while ( (thread_flistp = PIN_FLIST_ELEM_GET_NEXT(oper_flistp,
			PIN_FLD_THREAD_INFO, &rec_id, 1,
			&cookie, ebufp)) != NULL ) {

		/********************************************
		 * Consolidate a list of the failed records.
		 ********************************************/

		cookie1 = NULL;
		rec_id1 = 0;
		while( (rec_flistp = PIN_FLIST_ELEM_GET_NEXT(thread_flistp,
			PIN_FLD_NUMBERS, &rec_id1, 1,
			&cookie1, ebufp)) != NULL ) {

			vp = PIN_FLIST_FLD_GET(rec_flistp,
				PIN_FLD_DESCR, 0, ebufp);
			strcpy(buf, (void *)vp);
                        /* Write failed CSV record to the error file */

                        sprintf(sbuf, "<RECORD %d>\n"
                                        "\tRecord failed in opcode.\n",rec_id1);
                        fprintf(ferr,sbuf);
			fprintf(ferr, buf);
			/***************increment failed_count***********/
			failed_count++;
		}
	}

	if( ferr != (FILE *) 0){
		fclose(ferr);
	}

	if(mode == MODE_VERBOSE){
		fprintf(stdout,
			"\n\tNo. of records successfully processed = %d\n",
			total_count - failed_count);
		fprintf(stdout,"\tNo. of records failed to process      = %d\n",
			failed_count);
		fprintf(stdout,"\tTotal No. of records processed        = %d\n",
			total_count);
	}

	pin_mta_global_flist_node_release(PIN_FLD_OPERATION_INFO,ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_post_exit error", ebufp);
	}
	return;
}

/*******************************************************************
 * Main application opcode is called here
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
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	int32		opcode = 0;
	int32		mode = 0;
	int32		*rec_no = 0;
	void		*vp = 0;
	pin_cookie_t	cookie = 0;
	int32		rec_id = 0;
	pin_flist_t	*app_flistp = NULL;
	pin_flist_t	*err_flistp = NULL;
	pin_errbuf_t	ebuf1;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);
	PIN_ERRBUF_CLEAR (&ebuf1);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search results flist", 
					   srch_res_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode prepared flist for main opcode", 
					   op_in_flistp);

	PIN_ERR_LOG_FLIST (1, "pin_mta_worker_opcode thread info flist", 
					   ti_flistp);

	app_flistp =
		pin_mta_global_flist_node_get_with_lock (PIN_FLD_APPLICATION_INFO,
												 ebufp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_MODE, 1,ebufp);
	if(vp){
		mode = *((int32*)vp);
	}

	pin_mta_global_flist_node_release (PIN_FLD_APPLICATION_INFO,
									   ebufp);

	opcode = MSO_OP_PYMT_AUTOPAY;
	PCM_OP(ctxp, opcode, 0, op_in_flistp,
					op_out_flistpp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "pin_mta_worker_opcode error", ebufp);

		rec_no = (int32 *) PIN_FLIST_FLD_GET(op_in_flistp,
				PIN_FLD_ELEMENT_ID, 0, &ebuf1);
		err_flistp = PIN_FLIST_ELEM_ADD(ti_flistp, PIN_FLD_NUMBERS,
			*rec_no, &ebuf1);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ELEMENT_ID,
					(void *)rec_no, &ebuf1);
		vp = PIN_FLIST_FLD_GET(op_in_flistp,
				PIN_FLD_DESCR, 0, &ebuf1);
		PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_DESCR,
						vp, &ebuf1);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
			"YYYYY", err_flistp);
	}else{
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
						   "pin_mta_worker_opcode output flist from main opcode", 
						   *op_out_flistpp);
	}

	return;
}

/******************************************************************************
 * Function     : unquote()
 * Description  : This function removes the quotes(") from the input string.
 ******************************************************************************/
static char *
unquote(char *p)
{
	if (p[0] == '"') {
		if (p[strlen(p)-1] == '"')
			p[strlen(p)-1] = '\0';
		p++;
	}
	return p;
}


/******************************************************************************
 * Function     : trim()
 * Description  : This function returns a copy of the string, with leading
 *              : and trailing whitespaces omitted.
 ******************************************************************************/

static char *
trim(char *str)
{
	int c;

	while ((c = str[strlen(str) - 1]) == ' ' || c == '\t'){
		str[strlen(str) - 1] = '\0';
	}

	while((c = *str) == ' ' || c == '\t'){
		str++;
	}

	return str;
}

/******************************************************************************
 * Function     : get_tok()
 * Parameters   : char  *str :  A string with comma separated values.
 * Returns      : (char *) A string token (or NULL) on each call.
 ******************************************************************************/
static char *
get_tok(char *str)
{
	static char *next	= NULL;

	if(str == NULL) {
		str = next;
	}
	else {
		next = str;
	}

	if(str == NULL) {
		return NULL;
	}

	while(*next && *next != '\n' && *next != '\r') {
		if(*next == '|') {
			*next = '\0';
			next++;
			return str;
		}
		next++;
	}

	if(*next == '\r' || *next == '\n') {
		*next = '\0';
	}

	next = NULL;

	return str;
}

/******************************************************************************
 * Function     : trim_comment_from_flist()
 * Description  : This function returns a copy of the flist string after
 *              : the comment header from the flist
 ******************************************************************************/
static char *
trim_comment_from_flist(char *str)
{
	int c;

	if(str[0] == '#'){
		while( (c = *str++) != '\n' && c != '\r' )
			/* do nothing */ ;
		if(c == '\r' && *str == '\n'){
			str++;
		}
	}

	return str;
}

