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
			"\t\t No parameter required. Insert all adjustment data to rds_base_dc_t.\n\n";

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
 	char            template1[100] = "select /*+ parallel(8) full(purchased_product_t)*/ X from /rds_base_dc ";
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

	{
/*		pin_conf("mso_bb_grace", "bb_grace_period", PIN_FLDT_INT, (caddr_t*)&grace_days, &perr);
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
                }*/

		s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
		PIN_FLIST_FLD_SET( s_flistp, PIN_FLD_TEMPLATE, template1, ebufp );
		
		args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		//PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, NULL, ebufp);
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
	int		svctype = 0;

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
//	if(poid_type && (strcmp(poid_type, "/purchased_product") == 0))
        {
		cycle_end_t = PIN_FLIST_FLD_GET(srch_res_flistp, PIN_FLD_CYCLE_END_T, 0, ebufp);

		create_iflist = PIN_FLIST_CREATE(ebufp);
		s_pdp = PIN_POID_CREATE(db, "/mso_adjustment", -1, ebufp);
		PIN_FLIST_FLD_PUT(create_iflist, PIN_FLD_POID, (void *)s_pdp, ebufp);
		PIN_FLIST_FLD_SET(create_iflist, MSO_FLD_SERVICE_TYPE, &svctype, ebufp);	
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_ACCOUNT_OBJ, create_iflist, PIN_FLD_ACCOUNT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_EVENT_OBJ, create_iflist, PIN_FLD_EVENT_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_ITEM_OBJ, create_iflist, PIN_FLD_ITEM_OBJ, ebufp);
		s_pdp = PIN_POID_CREATE(db, "/bill", 1, ebufp);
		PIN_FLIST_FLD_SET(create_iflist,PIN_FLD_BILL_OBJ,s_pdp,ebufp);
		PIN_FLIST_FLD_SET(create_iflist,PIN_FLD_CHARGING_ID,"Disconnection Credit",ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_BILL_NO,create_iflist,PIN_FLD_BILL_NO,ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_BILL_DATE_T,create_iflist,PIN_FLD_BILL_DATE_T,ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_AMOUNT,create_iflist,PIN_FLD_AMOUNT,ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_AMOUNT,create_iflist,PIN_FLD_ADJUSTED,ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_AMOUNT ,create_iflist,PIN_FLD_CURRENT_BAL,ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_BILL_DATE_T,create_iflist,PIN_FLD_EFFECTIVE_T,ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_BILL_DATE_T,create_iflist,PIN_FLD_START_T,ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp, PIN_FLD_BILL_DATE_T,create_iflist,PIN_FLD_END_T,ebufp);
		PIN_FLIST_FLD_SET(create_iflist,PIN_FLD_FLAGS,&svctype, ebufp);
		PIN_FLIST_FLD_SET(create_iflist,PIN_FLD_PROGRAM_NAME,"Manual Adjustment",ebufp);
		svctype = 1;
		PIN_FLIST_FLD_SET(create_iflist,PIN_FLD_TAX_FLAGS,&svctype, ebufp);
		s_pdp = PIN_POID_CREATE(db, "/service/pcm_client", 21341232, ebufp);
		PIN_FLIST_FLD_SET(create_iflist,PIN_FLD_USERID,(void *)s_pdp, ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp,MSO_FLD_CGST_RATE,create_iflist,MSO_FLD_CGST_RATE,ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp,MSO_FLD_SGST_RATE,create_iflist,MSO_FLD_CGST_RATE,ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp,MSO_FLD_IGST_RATE,create_iflist,MSO_FLD_CGST_RATE,ebufp);
		PIN_FLIST_FLD_COPY(srch_res_flistp,PIN_FLD_CHARGE_AMT,create_iflist,PIN_FLD_CHARGE_AMT,ebufp);

                /*pin_conf("mso_bb_grace", "bb_grace_period", PIN_FLDT_INT, (caddr_t*)&grace_days, &perr);*/
/*                timeeff = localtime(cycle_end_t);
                timeeff->tm_mday = timeeff->tm_mday + *grace_days;
                expiry = mktime (timeeff); */
		/*if (perr != PIN_ERR_NONE) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,
				"Error in adding grace days");
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_BAD_VALUE, 0, 0, 0);
			goto cleanup;
		}*/
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, " Calling create adjustment_obj input flist", create_iflist);

		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_iflist, &create_oflist, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							 "pin_mta_worker_job error", ebufp);
		}
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, " Calling create adjustment_obj output flist", create_iflist);
		goto cleanup;
		//PIN_FLIST_DESTROY_EX(&create_iflist, NULL);
		//PIN_FLIST_DESTROY_EX(&create_oflist, NULL);
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

