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
#include <time.h>
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
#include "fm_utils.h"
#include "fm_bill_utils.h"

/*******************************************************************
* Apllication constants
********************************************************************/
#define  MODE_VERBOSE		1
#define  MODE_NON_VERBOSE	0

	struct tm       *timeeff;
	int             perr = 0;
	int             *grace_days = NULL;
	int		day_adjusted = 0;

/*******************************************************************
 * Functions used within
 *******************************************************************/
static void
mso_parameter_check(
        pin_flist_t     *param_flistp,
        pin_flist_t     *app_flistp,
        pin_errbuf_t    *ebufp);

extern time_t
fm_utils_time_round_to_midnight(
        time_t  	a_time);

void
mso_get_mso_plan(
	pcm_context_t 	*ctxp, 
	poid_t		*purch_pdp, 
	pin_flist_t	**r_flistp, 
	pin_errbuf_t	*ebufp);

int32
mso_get_purchased_count(
        pcm_context_t                   *ctxp,
        poid_t                          *acc_pdp,
	pin_flist_t			**result_flistpp,
        pin_errbuf_t                    *ebufp);

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
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*tmp_flistp1 = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
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
	tmp_flistp = PIN_FLIST_ELEM_ADD(app_flistp, PIN_FLD_PARAMS, 1, ebufp);
	tmp_flistp1 = PIN_FLIST_ELEM_GET(param_flistp, PIN_FLD_PARAMS, PIN_ELEMID_ANY, 1, ebufp);
	PIN_FLIST_FLD_COPY(tmp_flistp1, PIN_FLD_PARAM_NAME, tmp_flistp, PIN_FLD_PARAM_NAME, ebufp);
	PIN_FLIST_FLD_COPY(tmp_flistp1, PIN_FLD_PARAM_VALUE, tmp_flistp, PIN_FLD_PARAM_VALUE, ebufp);
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_parameter_check application flist after adding params",
                app_flistp);
	/***********************************************************
	* Get command line options for allication
	***********************************************************/
	mso_parameter_check(param_flistp, app_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_mta_config error", ebufp);
	}
	return;

}

static void
mso_parameter_check(
	pin_flist_t	*param_flistp,
	pin_flist_t	*app_flistp,
	pin_errbuf_t	*ebufp)
{
	int32		mode = MODE_NON_VERBOSE;
	pin_flist_t	*flistp = NULL;
	char		*option = NULL;
	void		*vp = NULL;
	int32           mta_flags = 0;
	int32		Wrong_argument = 0;
	//char		*param_value = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_parameter_check parameters flist", 
		param_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_parameter_check application flist",
                app_flistp);

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

		
	flistp = PIN_FLIST_ELEM_GET(param_flistp, PIN_FLD_PARAMS, PIN_ELEMID_ANY, 1,  ebufp);
	if(flistp != NULL)
	{
		option = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_NAME, 1, ebufp);

		if(strcmp(option, "-fetch_acct") == 0) {

			//param_value = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
			PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_ACTION, "FETCH", ebufp);
			Wrong_argument = 1;
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "check1");
			PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, PIN_ELEMID_ANY, ebufp);
		} else if (strcmp(option, "-order_process") == 0 ) {

			PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_ACTION, "ORDER_PROCESS", ebufp);
			Wrong_argument = 1;
			PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, PIN_ELEMID_ANY, ebufp);
		} else if (strcmp(option, "-expiry") == 0) {
			PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_ACTION, "EXPIRY", ebufp);
			Wrong_argument = 1;
			PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, PIN_ELEMID_ANY, ebufp);
		}
		
	}

	/* Check if the -f <input file> mandatory option is not supplied */
	if(Wrong_argument == 0) {
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
			"\t\t Example: mso_bb_grace_job.c -fetch_acct \n";

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

	if (PIN_ERRBUF_IS_ERR(&ebuf)) {
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

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
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

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
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

	time_t		current_time = 1514485800;
 	char            *ptemplate = "select /*+ parallel(8) full(purchased_product_t)*/ X from /purchased_product 1,/product 2 where 1.F1 = 2.F2 and 2.F3 is not null and 1.F4.type = V4 and 1.F5 <= V5 and 1.F6 = V6 and 1.F7 <> V7 and 1.F8 >= V8 ";
	char                template1[256] = "select X from /mso_bb_grace_job where (F1 = V1 or F2 = V2) ";
	//char                ptemplate2[256] = "select X from /mso_bb_grace_job where (F1 = V1 or F2 = V2) ";
	time_t          current_date = 1514485800;

	time_t		zero = 0;
	time_t		now_t = 0;
	time_t		day_adjusted = 0;
	int		status = 1;
	int		status1 = 3;
	int		db = 0;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*param_flistp = NULL;
	pin_flist_t	*s_iflistp = NULL;
	pin_flist_t	*s_flistp = NULL;
	poid_t		*s_pdp = NULL;
	//int		s_flags = 768;	
	int           s_flags = 0;
	char		*action = NULL;
	char		msg[256];
	void		*vp = NULL;
	int32		op_status = 0;
	int32		op_fail = 2;
	int32		op_success = 1;
	int32		op_deac_fail = 6;
	int32		records = 0;
	char		*tmp_str = NULL;
/*	struct tm       *timeeff;
	int             perr = 0;
	int             *grace_days = NULL;
	int		expiry = 0;*/

	if (PIN_ERRBUF_IS_ERR(ebufp)) {	return;	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist",  app_flistp);

	action = PIN_FLIST_FLD_GET(app_flistp, PIN_FLD_ACTION, 0, ebufp);
	current_time = pin_virtual_time(NULL);
	//current_date = fm_utils_time_round_to_midnight(*(time_t *)current_time);

	sprintf ( msg,"  current_time: %d\n", current_time );
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
	param_flistp = PIN_FLIST_ELEM_GET(app_flistp, PIN_FLD_PARAMS, PIN_ELEMID_ANY, 1, ebufp);
	if(param_flistp)
	{
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search parm values",  param_flistp);
		tmp_str = PIN_FLIST_FLD_GET(param_flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);
		if(tmp_str)
		{
			records = atoi(tmp_str);
			sprintf (template1, "select X from /mso_bb_grace_job where (F1 = V1 or F2 = V2) and rownum <= %d ", records);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "record number is passed for the processing");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, template1);
		}
	}

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
        if(vp)
                db = PIN_POID_GET_DB ((poid_t*)vp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	if(action && (strcmp(action, "FETCH") == 0))
	{
		pin_conf("mso_bb_grace", "bb_grace_period", PIN_FLDT_INT, (caddr_t*)&grace_days, &perr);
	timeeff = localtime(&current_time);
                timeeff->tm_mday = timeeff->tm_mday - *grace_days + 1;
                day_adjusted = mktime (timeeff);
	sprintf ( msg,"  day_adjusted: %d\n", day_adjusted );
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,msg);
                if (perr != PIN_ERR_NONE) {
                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                        PIN_ERR_BAD_VALUE, 0, 0, 0);
                        goto cleanup;
                }

		s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
		PIN_FLIST_FLD_SET( s_flistp, PIN_FLD_TEMPLATE, ptemplate, ebufp );
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_OBJ, PIN_POID_CREATE(db, "/service/telco/broadband", -1, ebufp), ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 5, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CYCLE_END_T, &current_time, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 6, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &status, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 7, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CYCLE_END_T, &zero, ebufp);
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 8, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CYCLE_END_T, &day_adjusted, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
	}
	else if(action && (strcmp(action, "ORDER_PROCESS") == 0))
	{
		s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
                PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
                PIN_FLIST_FLD_SET( s_flistp, PIN_FLD_TEMPLATE, template1, ebufp );

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &op_status, ebufp);
	
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &op_fail, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	}
	else if(action && (strcmp(action, "EXPIRY") == 0))
	{
		s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
                PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
                PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
                PIN_FLIST_FLD_SET( s_flistp, PIN_FLD_TEMPLATE, template1, ebufp );

                args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &op_success, ebufp);

		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &op_deac_fail, ebufp);
	
		//args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
		//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_EXIT_T, &current_time, ebufp);

                args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	}
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"pin_mta_init_search error", ebufp);
		PIN_FLIST_DESTROY_EX (&s_flistp,0);
	}else
	{
		*s_flistpp = s_flistp;
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist output", *s_flistpp);
	}
cleanup:
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

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
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

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
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
	int32		set_status = 0;
	pin_flist_t	*create_iflist = NULL;
	pin_flist_t	*create_oflist = NULL;
	pin_flist_t	*args_flistp = NULL;
	poid_t		*s_pdp = NULL;
	int32		db = 1;
	poid_t		*poid_pdp = NULL;
	char		*poid_type = NULL;
	pin_flist_t	*r_flistp = NULL;
	poid_t		*purch_pdp = NULL;
	char		*descr = NULL;
	int		*purc_status = NULL;
	int		flag = 110 ;
	int32		upd_status = 3;
	pin_flist_t	*wrt_iflistp = NULL;
	pin_flist_t	*wrt_oflistp = NULL;
	pin_flist_t	*prov_iflistp = NULL;
	pin_flist_t	*prov_oflistp = NULL;
	int		*prov_call = NULL;
	pin_flist_t	*offr_flistp = NULL;
	pin_flist_t	*offr_oflistp = NULL;	
	pin_flist_t	*serv_out_flist = NULL;
	poid_t		*serv_pdp = NULL;
	int		*serv_status = NULL;
	poid_t		*acc_pdp = NULL;
	//struct tm       *timeeff;
	//int             perr = 0;
	//int             *grace_days = NULL;
//	int		expiry = 0;
	time_t		*cycle_end_t = NULL;
	time_t		*exit_t = NULL;
	time_t		current_time = 0;
	pin_flist_t	*ret_flistp = NULL;
	poid_t		*resul_pdp = NULL;
	pin_flist_t	*result_flistp = NULL;
	int		*status = NULL;
	/*struct tm       *timeeff;
	int             perr = 0;
	int             *grace_days = NULL;*/
	int		expiry = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
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
	poid_pdp = (poid_t *)PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_POID, 0, ebufp);
	poid_type = (char *)PIN_POID_GET_TYPE(poid_pdp);
	status = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_STATUS, 0, ebufp);
	if(poid_type && (strcmp(poid_type, "/purchased_product") == 0))
        {
		cycle_end_t = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_CYCLE_END_T, 0, ebufp);

		create_iflist = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(db, "/mso_bb_grace_job", -1, ebufp);
		PIN_FLIST_FLD_PUT(create_iflist, PIN_FLD_POID, (void *)s_pdp, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_PRODUCT_OBJ, create_iflist, PIN_FLD_PRODUCT_OBJ, ebufp);	
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_PLAN_OBJ, create_iflist, PIN_FLD_PLAN_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_CYCLE_END_T, create_iflist, PIN_FLD_PURCHASE_END_T, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_SERVICE_OBJ, create_iflist, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, create_iflist, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_POID, create_iflist, PIN_FLD_OFFERING_OBJ, ebufp);
		args_flistp = PIN_FLIST_ELEM_GET(create_iflist, PIN_FLD_CYCLE_FEES, PIN_ELEMID_ANY, 1,  ebufp);
		if(args_flistp)
		{
			PIN_FLIST_FLD_COPY(args_flistp, PIN_FLD_CHARGED_TO_T, create_iflist, PIN_FLD_CHARGED_TO_T, ebufp);
		}
		PIN_FLIST_FLD_SET(create_iflist, PIN_FLD_STATUS, &set_status, ebufp);
                /*pin_conf("mso_bb_grace", "bb_grace_period", PIN_FLDT_INT, (caddr_t*)&grace_days, &perr);*/
                timeeff = localtime(cycle_end_t);
                timeeff->tm_mday = timeeff->tm_mday + *grace_days;
                expiry = mktime (timeeff);
		/*if (perr != PIN_ERR_NONE) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"Error in adding grace days");
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, 0, 0, 0);
			goto cleanup;
		}*/
		PIN_FLIST_FLD_SET(create_iflist, PIN_FLD_EXIT_T, &expiry, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, " Calling create_obj input flist", create_iflist);

		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_iflist, &create_oflist, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							 "pin_mta_worker_job error", ebufp);
		}
		goto cleanup;
		//PIN_FLIST_DESTROY_EX(&create_iflist, NULL);
		//PIN_FLIST_DESTROY_EX(&create_oflist, NULL);
	}
	if(poid_type && (strcmp(poid_type, "/mso_bb_grace_job") == 0) && status && *status != 1)
        {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "order process loop");
		purch_pdp = (poid_t *)PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);
		serv_pdp = (poid_t *)PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
		acc_pdp = (poid_t *)PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		offr_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_POID, purch_pdp, ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_DESCR, NULL, ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_STATUS, NULL, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist rflds", offr_flistp);	
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, offr_flistp, &offr_oflistp, ebufp);	
		
		PIN_FLIST_DESTROY_EX(&offr_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                         "pin_mta_worker_job error", ebufp);
			PIN_ERRBUF_CLEAR (ebufp);
			upd_status = 2;
			goto SKIP;
                }
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist rflds", offr_oflistp);
		purch_pdp = PIN_FLIST_FLD_GET(offr_oflistp, PIN_FLD_POID, 0, ebufp);	
		descr = PIN_FLIST_FLD_GET(offr_oflistp, PIN_FLD_DESCR, 0, ebufp);	
		purc_status = PIN_FLIST_FLD_GET(offr_oflistp, PIN_FLD_STATUS, 0, ebufp);
		
		offr_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_POID, serv_pdp, ebufp);
                PIN_FLIST_FLD_SET(offr_flistp, PIN_FLD_STATUS, NULL, ebufp);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist rflds1", offr_flistp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, offr_flistp, &serv_out_flist, ebufp);
		PIN_FLIST_DESTROY_EX(&offr_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                         "pin_mta_worker_job error", ebufp);
                        PIN_ERRBUF_CLEAR (ebufp);
                        upd_status = 2;
                        goto SKIP;
                }
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist rflds1", serv_out_flist);
		serv_status = PIN_FLIST_FLD_GET(serv_out_flist, PIN_FLD_STATUS, 0, ebufp);
		if(purc_status && *purc_status == 3 && serv_status && *serv_status == 10102)	
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "prov order flist loop");
			mso_get_purchased_count(ctxp, acc_pdp, &ret_flistp,ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "mso_get_mso_plan search error ", ebufp);
                                PIN_ERRBUF_CLEAR (ebufp);
				//failed with error
				upd_status = 2;
                                goto SKIP;
                        }
			result_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, 0, 0,ebufp);
			if(result_flistp)
			{
				resul_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);
				if(PIN_POID_COMPARE(purch_pdp, resul_pdp, 0, ebufp) != 0)
				{
					//already the product is changed
					upd_status = 3;
					goto SKIP;
				}	
			}
			exit_t = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_EXIT_T, 0, ebufp);
			current_time = pin_virtual_time(NULL);
			if(exit_t && current_time >= *exit_t)
			{
				//already grace period is done
				upd_status = 4;	
				goto SKIP;
			}
			upd_status = 2;	
			mso_get_mso_plan(ctxp, purch_pdp, &r_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
                		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "mso_get_mso_plan search error ", ebufp);
				PIN_ERRBUF_CLEAR (ebufp);
                		goto SKIP;
        		}
			prov_iflistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, prov_iflistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_SERVICE_OBJ, prov_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_PLAN_OBJ, prov_iflistp, PIN_FLD_PLAN_OBJ, ebufp);
			PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_PURCHASE_END_T, prov_iflistp, PIN_FLD_PURCHASE_END_T, ebufp);
			PIN_FLIST_FLD_SET(prov_iflistp, PIN_FLD_ACTION_TYPE, "bb_grace_order", ebufp);
			PIN_FLIST_FLD_SET(prov_iflistp, PIN_FLD_FLAGS, &flag, ebufp);
			PIN_FLIST_FLD_SET(prov_iflistp, PIN_FLD_PROGRAM_NAME, "BB_GRACE_JOB", ebufp);	
			if(r_flistp)
			{
				PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_POID, prov_iflistp, MSO_FLD_PURCHASED_PLAN_OBJ, ebufp);
			}
			//PIN_FLIST_FLD_SET(prov_iflistp, PIN_FLD_USERID, PIN_POID_CREATE(db, "/account", 4412348913, ebufp), ebufp);
			PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, prov_iflistp, PIN_FLD_USERID, ebufp);

			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist for provisioning", prov_iflistp);
                	PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, prov_iflistp, &prov_oflistp, ebufp);

			PIN_FLIST_DESTROY_EX(&prov_iflistp, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist for provisioning", prov_oflistp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
				PIN_ERRBUF_CLEAR (ebufp);
				goto SKIP;
			}
			else if(prov_oflistp != NULL)
			{
				prov_call = PIN_FLIST_FLD_GET(prov_oflistp, PIN_FLD_STATUS, 1, ebufp);
				if(prov_call && (*prov_call != 0))
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Setting Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
					upd_status = 2;
					goto SKIP;
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Succss");
					// succes in provisioning
                                	upd_status = 1;
				}
			}
		}
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "if we miss any case error ", ebufp);
                                PIN_ERRBUF_CLEAR (ebufp);
                                //failed with error
                                upd_status = 2;
                                goto SKIP;
                }
	}
	if(poid_type && (strcmp(poid_type, "/mso_bb_grace_job") == 0) && status && (*status == 1 || *status == 6))
        {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, " Deactivation order loop");
		acc_pdp = (poid_t *)PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
		purch_pdp = (poid_t *)PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);
		mso_get_purchased_count(ctxp, acc_pdp, &ret_flistp,ebufp);
		 if (PIN_ERRBUF_IS_ERR(ebufp)) {
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                         "mso_get_mso_purc_plan search error ", ebufp);
                        PIN_ERRBUF_CLEAR (ebufp);
			// status during error
                        upd_status = 6;
                        goto SKIP;
                }
		result_flistp = PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, 0, 0,ebufp);
		if(result_flistp)
		{
			resul_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 0, ebufp);
			if(PIN_POID_COMPARE(purch_pdp, resul_pdp, 0, ebufp) != 0)
			{
				//already the product is changed
				upd_status = 7;
				goto SKIP;
			}
		}
		exit_t = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_EXIT_T, 0, ebufp);
		current_time = pin_virtual_time(NULL);
		if(exit_t && current_time < *exit_t)
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Grace period not expired");
			// no need to go for grace job as the grace period is not done
			//already grace period is done
			//upd_status = 8;
			goto cleanup;
		}
		upd_status = 6;
		//purch_pdp = (poid_t *)PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_OFFERING_OBJ, 0, ebufp);
		mso_get_mso_plan(ctxp, purch_pdp, &r_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					 "mso_get_mso_plan search error ", ebufp);
			PIN_ERRBUF_CLEAR (ebufp);
			upd_status = 6;
			goto SKIP;
		}
		flag = 104 ;	
		prov_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, prov_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_SERVICE_OBJ, prov_iflistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_PLAN_OBJ, prov_iflistp, PIN_FLD_PLAN_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_PURCHASE_END_T, prov_iflistp, PIN_FLD_PURCHASE_END_T, ebufp);
		PIN_FLIST_FLD_SET(prov_iflistp, PIN_FLD_ACTION_TYPE, "bb_grace_order", ebufp);
		PIN_FLIST_FLD_SET(prov_iflistp, PIN_FLD_FLAGS, &flag, ebufp);
		PIN_FLIST_FLD_SET(prov_iflistp, PIN_FLD_PROGRAM_NAME, "BB_GRACE_JOB", ebufp);
		if(r_flistp)
		{
			PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_POID, prov_iflistp, MSO_FLD_PURCHASED_PLAN_OBJ, ebufp);
		}
		//PIN_FLIST_FLD_SET(prov_iflistp, PIN_FLD_USERID, PIN_POID_CREATE(db, "/account", 4412348913, ebufp), ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, prov_iflistp, PIN_FLD_USERID, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Input flist for provisioning", prov_iflistp);
		PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, prov_iflistp, &prov_oflistp, ebufp);

		PIN_FLIST_DESTROY_EX(&prov_iflistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Output flist for provisioning", prov_oflistp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
			PIN_ERRBUF_CLEAR (ebufp);
			goto SKIP;
		}
		else if(prov_oflistp != NULL)
		{
			prov_call = PIN_FLIST_FLD_GET(prov_oflistp, PIN_FLD_STATUS, 1, ebufp);
			if(prov_call && (*prov_call != 0))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Setting Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
				upd_status = 6;
				goto SKIP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Succss");
				// succes in provisioning
				upd_status = 9;
			}
		}
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
                                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                                                 "if we miss any case error1 ", ebufp);
                                PIN_ERRBUF_CLEAR (ebufp);
                                //failed with error
                                upd_status = 6;
                                goto SKIP;
                }
		
        }
	SKIP:
	wrt_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(srch_res_flistp , PIN_FLD_POID, wrt_iflistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_STATUS, &upd_status, ebufp);

	PIN_ERR_LOG_FLIST(3, "Write fields in mta", wrt_iflistp);
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, wrt_iflistp, &wrt_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&wrt_iflistp, NULL);	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_FLIST(1, "Error during in write fields", wrt_oflistp);

	}
		
	cleanup:
	PIN_FLIST_DESTROY_EX(&prov_oflistp, NULL);
        PIN_FLIST_DESTROY_EX(&offr_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&wrt_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&serv_out_flist,NULL);
	PIN_FLIST_DESTROY_EX(&create_iflist, NULL);
        PIN_FLIST_DESTROY_EX(&create_oflist, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode end.");
	return;
}

void 
mso_get_mso_plan(
		pcm_context_t          *ctxp,
		poid_t          *purch_pdp,
		pin_flist_t     **r_flistpp,
		pin_errbuf_t            *ebufp)
{
	pin_flist_t	*mso_pp_in_flistp = NULL;
	pin_flist_t	*mso_pp_out_flistp = NULL;

	char 		mso_pp_srch_template[200] = "select X from /mso_purchased_plan where F1 = V1 ";
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*prod_flistp = NULL;
	poid_t		*srch_pdp = NULL;
	pin_flist_t	*result_flistp = NULL;
	int		db = 1;
	int		srch_flag = 256;

	mso_pp_in_flistp = PIN_FLIST_CREATE(ebufp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
        PIN_FLIST_FLD_SET(mso_pp_in_flistp, PIN_FLD_POID, srch_pdp, ebufp);
        PIN_FLIST_FLD_SET(mso_pp_in_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(mso_pp_in_flistp, PIN_FLD_TEMPLATE, mso_pp_srch_template, ebufp);
        args_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_ARGS, 1, ebufp);
        prod_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_PRODUCTS, PIN_ELEMID_ANY, ebufp);
        PIN_FLIST_FLD_SET(prod_flistp, MSO_FLD_PURCHASED_PRODUCT_OBJ, purch_pdp, ebufp);
        result_flistp = PIN_FLIST_ELEM_ADD(mso_pp_in_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_mso_plan input flist for base plan search", mso_pp_in_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, mso_pp_in_flistp, &mso_pp_out_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_mso_plan output flist for base plan search", mso_pp_out_flistp);
	PIN_FLIST_DESTROY_EX(&mso_pp_in_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						 "Search  error", ebufp);
		return;
	}	
	result_flistp = PIN_FLIST_ELEM_GET(mso_pp_out_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	*r_flistpp = PIN_FLIST_COPY(result_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&mso_pp_out_flistp, NULL);
	return;
}

int32
mso_get_purchased_count(
        pcm_context_t                   *ctxp,
        poid_t                          *acc_pdp,
	pin_flist_t			**ret_flistpp,
        pin_errbuf_t                    *ebufp)
{
        pin_flist_t                     *s_iflistp = NULL;
        pin_flist_t                     *s_oflistp = NULL;
        pin_flist_t                     *args_flistp = NULL;
        pin_flist_t                     *tmp_flistp = NULL;
        poid_t                          *s_pdp = NULL;
        int64                           db_no = 0;
        int32                           s_flags = SRCH_DISTINCT;
        char                            *pre_tmpl = "select X from /purchased_product 1, /product 2 where 1.F1 = V1 and 1.F2 = 2.F3 and 2.F4 is not null and 2.F5 <> V5 and 2.F6 <> V6 and 2.F7 <> V7 order by purchased_product_t.cycle_end_t desc ";
        int                             act_status = 1;
        int                             count = 0;
        pin_decimal_t                   *fup_top_rent_pri = pbo_decimal_from_str("2900",ebufp);
        pin_decimal_t                   *add_mb_sub_pri = pbo_decimal_from_str("2930",ebufp);
        pin_decimal_t                   *fup_top_sme_pri = pbo_decimal_from_str("1900",ebufp);
        int                             act_status_inac = 2;
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                return;
        }

                db_no = PIN_POID_GET_DB(acc_pdp);
                s_pdp = (poid_t *)PIN_POID_CREATE(db_no, "/search", -1, ebufp);
                s_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(s_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
                PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
                PIN_FLIST_FLD_SET(s_iflistp, PIN_FLD_TEMPLATE, (void *)pre_tmpl, ebufp);
                args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 1, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acc_pdp, ebufp);

                args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 3, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)NULL, ebufp);

                args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 2, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRODUCT_OBJ, (void *)NULL, ebufp);

		/*args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 4, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status, ebufp);*/

                args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 4, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, "", ebufp);

                args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 5, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, fup_top_rent_pri, ebufp);

                args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 6, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, add_mb_sub_pri, ebufp);

        /*        args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 8, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &act_status_inac, ebufp);*/

                args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_ARGS, 7, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PRIORITY, fup_top_sme_pri, ebufp);

                args_flistp = PIN_FLIST_ELEM_ADD(s_iflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);

                if(PIN_ERRBUF_IS_ERR(ebufp)){
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_get_purchased_count: Error", ebufp);
                        PIN_FLIST_DESTROY_EX(&s_iflistp,NULL);
                        return 0;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_purchased_count: Search Input Flist ", s_iflistp);
                //Call the PCM_OP_SEARCH
                PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_iflistp, &s_oflistp, ebufp);
                if(PIN_ERRBUF_IS_ERR(ebufp)){
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_get_purchased_count: after Search Error", ebufp);
                        PIN_FLIST_DESTROY_EX(&s_iflistp,NULL);
                        return 0;
                }
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_get_purchased_count: Search Output Flist ", s_oflistp);
                count = PIN_FLIST_ELEM_COUNT(s_oflistp, PIN_FLD_RESULTS, ebufp);
		*ret_flistpp = PIN_FLIST_COPY(s_oflistp, ebufp);	
                PIN_FLIST_DESTROY_EX(&s_iflistp,NULL);
                PIN_FLIST_DESTROY_EX(&s_oflistp,NULL);
                if (pbo_decimal_is_null(fup_top_rent_pri, ebufp))
                        pbo_decimal_destroy(&fup_top_rent_pri);
                if (pbo_decimal_is_null(add_mb_sub_pri, ebufp))
		        pbo_decimal_destroy(&add_mb_sub_pri);
                if (pbo_decimal_is_null(fup_top_sme_pri, ebufp))
                        pbo_decimal_destroy(&fup_top_sme_pri);
		//*ret_flistpp = PIN_FLIST_COPY(args_flistp, ebufp);	
                if(count >=1)
		{
                        return 1;
		}
                else
                        return 0;

}
