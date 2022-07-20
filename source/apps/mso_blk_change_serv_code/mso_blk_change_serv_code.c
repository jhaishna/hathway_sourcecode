/*******************************************************************
 *
 *    Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *    This material is the confidential property of Oracle Corporation
 *    or its licensors and may be used, reproduced, stored or transmitted
 *    only in accordance with a valid Oracle license or sublicense agreement.
 *    ====================================================================
 *    This application is created to create bulk LCOs
 *    Author: Kunal Bhardwaj
 *    Date: 31-Dec-2015
 *    Version: 1.0 
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: mso_blk_change_serv_code.c:CUPmod7.3PatchInt:1:2015-Dec-31 18:51:33 %";
#endif

#include <stdio.h>
#include <string.h>

#include "pcm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "pin_mta.h"
#include "pin_flds.h"
#include "mso_ops_flds.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_cust.h"
#include "mso_utils.h"
#include "ops/bal.h"

#define  NEW_RECORD     0  
#define  SUCCESS_RECORD     1
#define  ERROR_RECORD     2
#define  RETRY        3 

/*******************************************************************
 * Configuration of application
 * Called prior MTA_CONFIG policy opcode
 *******************************************************************/
 void
fm_mso_get_sequence_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);
		
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
	int				retry = RETRY;
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
	i_param_count = PIN_FLIST_ELEM_COUNT(param_flistp, PIN_FLD_PARAMS, ebufp);

	object_type = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_OBJ_TYPE, 1, ebufp);
	if(!object_type) {
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

	pin_errbuf_t		ebuf;
	pin_flist_t		*ext_flistp = NULL;
	char			*usage_str = NULL;
	char			*format = "\nUsage:     %s [-verbose] > \n\n";
            
	PIN_ERRBUF_CLEAR (&ebuf);
    
	usage_str = (char*)pin_malloc( strlen(format) + strlen(prog) + 1 );

	if (usage_str == NULL) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"No Memory error");
		return;
	}

	sprintf(usage_str, format ,prog);

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
	pin_flist_t		*tmp1_flistp = NULL;
	pin_flist_t		*extraRes_flistp = NULL;
	char			*template = "select X from /mso_blk_change_serv where F1 = V1 ";
	char			obj_type[100];
	char			*object_type = NULL;
	char			*t1 = NULL;
	int32			s_flags = 256;
	poid_t			*s_pdp = NULL;
	poid_t			*i_pdp = NULL;
	poid_t			*binfo_pdp = NULL;
	int32			*days = NULL;
	int32			new_record = NEW_RECORD;
	int32			errp = 0;
	int32			link_direction=1;
	int64			id = -1;
	int64			db = 0;
	void			*vp = NULL;
	time_t			now_t = 0;
	time_t			start_t = (time_t)0;
	time_t			end_t = (time_t)0;


	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);
	now_t = pin_virtual_time((time_t *)NULL);
	*s_flistpp = 0;

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", 
		app_flistp);
	
	s_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Simple search flist
	 ***********************************************************
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS           INT [0] 256
	0 PIN_FLD_ARGS          ARRAY [1] allocated 20, used 1
	1	MSO_FLD_STATUS      ENUM [0] 0 
	0 PIN_FLD_TEMPLATE        STR [0] "select X from /mso_blk_change_serv F1 = V1"
	0 PIN_FLD_RESULTS       ARRAY [*] allocated 20, used 5
	***********************************************************/

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
	if(vp)
	  db = PIN_POID_GET_DB ((poid_t*)vp);

	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

	PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	/***********************************************************
	 * Search Start Time & End Time
	 ***********************************************************/
	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_STATUS, &new_record, ebufp);
	PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	/***********************************************************
	 * Search results
	 ***********************************************************/
	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS,0, ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "pin_mta_init_search error", ebufp);
		PIN_FLIST_DESTROY_EX (&s_flistp,0);
	}
	else
	{
		*s_flistpp = PIN_FLIST_COPY(s_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search search flist", s_flistp);
	}
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
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
	pin_flist_t	    *in_flistp=NULL;
	pin_flist_t         *i_flistp = NULL;
	pin_flist_t         *o_flistp = NULL;
	pin_flist_t         *read_in_flistp = NULL;
	pin_flist_t         *read_out_flistp = NULL;
	pin_flist_t         *task_flistp = NULL;
	pin_flist_t         *avp_flistp = NULL;
	pin_flist_t         *bal_flistp = NULL;
	pin_flist_t         *bal_in_flistp = NULL;
	pin_flist_t         *bal_out_flistp = NULL;
	pin_flist_t         *balance_flistp = NULL;	
	pin_flist_t         *sub_bal_flistp = NULL;
	pin_flist_t         *order_seq_i_flistp = NULL;
	pin_flist_t         *order_seq_o_flistp = NULL;
	
	pin_flist_t         * order_i_flistp = NULL;
	
	pin_flist_t         *upd_in_flistp = NULL;
	pin_flist_t         *upd_out_flistp = NULL;
	
	poid_t		    *job_poid = NULL;
	poid_t		    *prov_pdp = NULL;
	char		    *prog_name = "Bulk_Change_Service_Code";
	char			error_descr[200];
	char		    *action = "UPDATE_SUBSCRIBER-CAPPING_DOCSIS";
	char		    *task_name = "FUP_UPDATE_QPS";
	
	char                *avp1 = "ServiceType";
	char                *avp2 = "isQuota";
	char                *quotacode = "DATA";
	char			*unlimited_quota = "1073740750258176";
	char			*total_bal_str = NULL;
	char             initial_amount[200];
	char            *flistbuf = NULL;
	
	int32		    flags = 102;
	int32		    bal_flags = 0;
	int32		    status=1;
	int32		    error_code=0;
	int32		    *service_status=0;
	int32			*plan_type = NULL;
	int32			rec_id=0;
	int32			isquota_flag=-1;
	int32			ord_status  = 1;
	int32			pri = 1;
	int64           db_no = 0;
	int32           flistlen=0;
	
	pin_cookie_t	cookie = NULL;
	
	void                *vp = NULL;
	
	
	pin_decimal_t 		*total_bal = NULL;
	pin_decimal_t 		*curr_bal = NULL;
	pin_decimal_t 		*res_bal = NULL;
	
	pin_decimal_t 		*mb_bytes = NULL;
	pin_decimal_t 		*tmp_val = NULL;
		
	time_t              pvt;
	time_t 				*valid_to = NULL;
	
	pin_buf_t           *nota_buf     = NULL;
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}

	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search flist", srch_res_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode input flist", op_in_flistp);
	in_flistp = PIN_FLIST_COPY(op_in_flistp, ebufp);
	job_poid = PIN_FLIST_FLD_GET (in_flistp, PIN_FLD_POID, 1, ebufp);
	db_no = PIN_POID_GET_DB(job_poid);
	upd_in_flistp = PIN_FLIST_CREATE(ebufp);
	
	/********************************************
	 *      prepare flist for opcode call in 
	 *           pin_mta_worker_opcode
	 *********************************************/

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_job prepared flist for main opcode", in_flistp);

	/*********************************************
	0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 4446998593 9
	0 PIN_FLD_SERVICE_OBJ    POID [0] 0.0.0.1 /service/telco/broadband 4447003710 56
	0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 1290705 8
	0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|csradmin"
	0 PIN_FLD_FLAGS           INT [0] 114 
	0 MSO_FLD_PURCHASED_PLAN_OBJ   POID [0] 0.0.0.1 /mso_purchased_plan 4447007023 0
	0 PIN_FLD_SUB_BAL_IMPACTS  ARRAY [0]    NULL array ptr
	0 PIN_FLD_PROVISIONING_TAG    STR [0] "D3_POST_U15360_D15360_0_U_0_0_0"
	0 PIN_FLD_PLAN_OBJ       POID [0] 0.0.0.1 /plan 3869959732 2
	0 MSO_FLD_TASK          ARRAY [2] allocated 20, used 9
	1     MSO_FLD_TASK_NAME       STR [0] "CHANGE_SERVICES_QPS"
	1     MSO_FLD_NETWORKID       STR [0] "kunal51613523"
	1     MSO_FLD_NETWORK_NODE    STR [0] "QPS01"
	1     MSO_FLD_OLD_SERVICECODE    STR [0] "FUP-D15360-U3072-T-D1024-U1024"
	1     MSO_FLD_OLD_BALANCECODE    STR [0] "FUP-D15360-U3072-T-D1024-U1024"
	1     MSO_FLD_SERVICE_CODE    STR [0] "PP-D15360-U15360-PT-D15360-U15360"
	1     MSO_FLD_SUB_AVP       ARRAY [1] allocated 20, used 2
	2         MSO_FLD_CODE            STR [0] "ServiceType"
	2         MSO_FLD_VALUE           STR [0] "POST"
	1     MSO_FLD_SUB_AVP       ARRAY [2] allocated 20, used 2
	2         MSO_FLD_CODE            STR [0] "isQuota"
	2         MSO_FLD_VALUE           STR [0] "false"
	1     MSO_FLD_CREATEBALANCE  ARRAY [1] allocated 20, used 4
	2         MSO_FLD_CODE            STR [0] "PP-D15360-U15360"
	2         MSO_FLD_QUOTACODE       STR [0] "DATA"
	2         MSO_FLD_STARTDATE       STR [0] "2016-05-31T18:40:33.000+0530"
	2         MSO_FLD_INITIALAMOUNT    STR [0] "1073740750258176"
	0 PIN_FLD_INDICATOR       INT [0] 0
	0 PIN_FLD_ORDER_ID        STR [0] "PROV-167886492"
	0 PIN_FLD_ACTION          STR [0] "CHANGE_SERVICES_DOCSIS"
	*********************************************/
	
	read_in_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, read_in_flistp, PIN_FLD_POID, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, read_in_flistp, &read_out_flistp, ebufp);
        if (PIN_ERR_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Service Read Error", ebufp);
                status = 2;
		error_code = 100;
		strcpy(error_descr, "Service Search Failed");
                PIN_ERR_CLEAR_ERR(ebufp);
                PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
                PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_CODE, &error_code, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
                *op_out_flistpp = PIN_FLIST_COPY(o_flistp, ebufp);
                PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
                        "pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp);
                goto UPDATE_REC;
        }

	service_status = PIN_FLIST_FLD_GET(read_out_flistp, PIN_FLD_STATUS, 1, ebufp); 
	
	if (service_status && *service_status != 10103)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active or Inactive Service");		
	}
	else
	{
	        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Service Status error", ebufp);
                status = 2;
		error_code=101;
		strcpy(error_descr, "Invalid Service Status");
                PIN_ERR_CLEAR_ERR(ebufp);
                PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
                PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
                PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_CODE, &error_code, ebufp);
                PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
                *op_out_flistpp = PIN_FLIST_COPY(o_flistp, ebufp);
                PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
                        "pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp);
                goto UPDATE_REC;
	}
	/*
        #PCM_OP_BAL_GET_BALANCES
        0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 2317687 0
        0 PIN_FLD_FLAGS           INT [0] 4
        0 PIN_FLD_BALANCES      ARRAY [*]     NULL array ptr
	*/
	
	plan_type = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_TYPE, 0, ebufp);
	if (plan_type && (*plan_type == 1 ||  *plan_type == 2)){
		pvt = pin_virtual_time(NULL);
		bal_in_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, bal_in_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(bal_in_flistp, PIN_FLD_FLAGS, &bal_flags, ebufp);
		PIN_FLIST_ELEM_ADD(bal_in_flistp, PIN_FLD_BALANCES, PIN_ELEMID_ANY, ebufp);
		
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "bal_in_flistp flist", bal_in_flistp);
		PCM_OP(ctxp, PCM_OP_BAL_GET_BALANCES, 0, bal_in_flistp, &bal_out_flistp, ebufp);
		
		if (PIN_ERR_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting Balances", ebufp);
			PIN_ERR_CLEAR_ERR(ebufp);
			status = 2;
			error_code = 102;
			strcpy(error_descr, "Error in getting customer balance");
			PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
			PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
			PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_CODE, &error_code, ebufp);
			PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
			*op_out_flistpp = PIN_FLIST_COPY(bal_out_flistp, ebufp);
			PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
				"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
			goto UPDATE_REC;
		}
		
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist", bal_out_flistp);
	
		if(plan_type && *plan_type == 1 ){
				balance_flistp = PIN_FLIST_ELEM_GET(bal_out_flistp, PIN_FLD_BALANCES, 1100010, 1, ebufp);
		}
		else if(plan_type && *plan_type == 2){
				balance_flistp = PIN_FLIST_ELEM_GET(bal_out_flistp, PIN_FLD_BALANCES, 1000104, 1, ebufp);
		}
		
		if(balance_flistp)
        {
			total_bal = pbo_decimal_from_str("0.0",ebufp);
			mb_bytes = pbo_decimal_from_str("1048576.0",ebufp);
			while ( (sub_bal_flistp = PIN_FLIST_ELEM_GET_NEXT (balance_flistp,
									   PIN_FLD_SUB_BALANCES, &rec_id, 1,&cookie, ebufp)) != (pin_flist_t *)NULL )
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Sub Bal flist", sub_bal_flistp);

				valid_to = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_VALID_TO, 0,ebufp);
				if(*valid_to > pvt )
				{
						//ncr_found = 1;
						curr_bal = PIN_FLIST_FLD_GET(sub_bal_flistp, PIN_FLD_CURRENT_BAL, 0,ebufp);
						tmp_val = pbo_decimal_multiply(curr_bal,mb_bytes,ebufp);
						pbo_decimal_add_assign(total_bal,tmp_val,ebufp);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Total Balance");
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));
						// Added below to remove the decimal
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,pbo_decimal_to_str(total_bal,ebufp));
				}
			}
			res_bal = NULL;
			res_bal = PIN_FLIST_FLD_GET(balance_flistp, PIN_FLD_RESERVED_AMOUNT, 0,ebufp);
			tmp_val = pbo_decimal_multiply(res_bal,mb_bytes,ebufp);
			pbo_decimal_subtract_assign(total_bal,tmp_val,ebufp);
			pbo_decimal_round_assign(total_bal, 0, ROUND_UP, ebufp);
			total_bal_str = pbo_decimal_to_str(total_bal,ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Final Bal");
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,total_bal_str);
			strcpy(initial_amount, total_bal_str);
        }
	}
	else
	{
		strcpy(initial_amount, unlimited_quota);
	}
	
	if (PIN_ERR_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before creating buf flist", ebufp);
		status = 2;
		error_code=103;
		strcpy(error_descr, "Error before creating buf flist");
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error before creating buf flist", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_CODE, &error_code, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);

		*op_out_flistpp = PIN_FLIST_COPY(o_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
		goto UPDATE_REC;
	}
	
    order_seq_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, order_seq_i_flistp, PIN_FLD_POID, ebufp);
    PIN_FLIST_FLD_SET(order_seq_i_flistp, PIN_FLD_NAME, "MSO_PROV_SEQUENCE", ebufp);
    //call the function to read the provisioning order sequence
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"mso_blk_change_serv_code input flist", order_seq_i_flistp); 
    fm_mso_get_sequence_no(ctxp, order_seq_i_flistp, &order_seq_o_flistp, ebufp);
    if (PIN_ERR_IS_ERR(ebufp))
    {
		error_code = 104;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting sequence", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		status = 2;
		strcpy(error_descr, "Error before creating buf flist");
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_CODE, &error_code, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
		*op_out_flistpp = PIN_FLIST_COPY(order_seq_o_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
		goto UPDATE_REC;
    }


	i_flistp = PIN_FLIST_CREATE(ebufp);
	
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
	//PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, i_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, 
			i_flistp, MSO_FLD_PURCHASED_PLAN_OBJ, ebufp);
	//PIN_FLIST_ELEM_ADD(i_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROVISIONING_TAG, 
				i_flistp, PIN_FLD_PROVISIONING_TAG, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PLAN_OBJ,
                                i_flistp, PIN_FLD_PLAN_OBJ, ebufp);
	task_flistp = PIN_FLIST_ELEM_ADD(i_flistp, MSO_FLD_TASK, 1, ebufp);
	PIN_FLIST_FLD_SET(task_flistp, MSO_FLD_TASK_NAME, task_name, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_LOGIN,
                        task_flistp, MSO_FLD_NETWORKID, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_NETWORK_NODE,
                        task_flistp, MSO_FLD_NETWORK_NODE, ebufp);	
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_OLD_SERVICECODE,
                        task_flistp, MSO_FLD_OLD_SERVICECODE, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_OLD_SERVICECODE,
                        task_flistp, MSO_FLD_OLD_BALANCECODE, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_CODE,
                        task_flistp, MSO_FLD_SERVICE_CODE, ebufp);
	avp_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_SUB_AVP, 1, ebufp);
	PIN_FLIST_FLD_SET(avp_flistp, MSO_FLD_CODE, avp1, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_TYPE,
                        avp_flistp, MSO_FLD_VALUE, ebufp);
        avp_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_SUB_AVP, 2, ebufp);
        PIN_FLIST_FLD_SET(avp_flistp, MSO_FLD_CODE, avp2, ebufp);
		isquota_flag = *(int32 *)PIN_FLIST_FLD_GET (in_flistp, MSO_FLD_ISQUOTA_FLAG, 1, ebufp);    
		if (isquota_flag){
			PIN_FLIST_FLD_SET(avp_flistp, MSO_FLD_VALUE, "true", ebufp);
		}
		else{
			PIN_FLIST_FLD_SET(avp_flistp, MSO_FLD_VALUE, "false", ebufp);
		}
			
       	bal_flistp = PIN_FLIST_ELEM_ADD(task_flistp, MSO_FLD_CREATEBALANCE, 1, ebufp);
        PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_QUOTA_CODE,
                        bal_flistp, MSO_FLD_CODE, ebufp);
        PIN_FLIST_FLD_SET(bal_flistp, MSO_FLD_QUOTA_CODE, quotacode, ebufp);
/*	if (initial_amount != NULL ){
		PIN_FLIST_FLD_SET(bal_flistp, MSO_FLD_INITIALAMOUNT, initial_amount, ebufp);
	}
	else{
		PIN_FLIST_FLD_SET(bal_flistp, MSO_FLD_INITIALAMOUNT, "0", ebufp);
	}
*/		
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_INDICATOR,
                        i_flistp, PIN_FLD_INDICATOR, ebufp);	
	PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, i_flistp, PIN_FLD_ORDER_ID, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ACTION, action, ebufp);
	

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "main buffer input flist", i_flistp);
	    /*
     * Create flist for order creation
     */
    PIN_FLIST_TO_STR(i_flistp, &flistbuf, &flistlen, ebufp );
    if (PIN_ERR_IS_ERR(ebufp))
    {
		error_code = 108;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "error creating buffer", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		status = 2;
		strcpy(error_descr, "error creating buffer");
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_CODE, &error_code, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
		*op_out_flistpp = PIN_FLIST_COPY(order_seq_o_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
		goto UPDATE_REC;
    }

    order_i_flistp = PIN_FLIST_CREATE(ebufp);
    prov_pdp = PIN_POID_CREATE(db_no, "/mso_prov_bb_order", -1, ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_POID, prov_pdp, ebufp);
	//PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, order_i_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_PROGRAM_NAME, prog_name, ebufp);
    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, order_i_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
    PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SERVICE_OBJ, order_i_flistp, PIN_FLD_SERVICE_OBJ, ebufp);

    PIN_FLIST_FLD_COPY(order_seq_o_flistp, PIN_FLD_STRING, order_i_flistp, PIN_FLD_ORDER_ID, ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_STATUS, &ord_status, ebufp); //PROVISIONING
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_ACTION, action, ebufp);
    PIN_FLIST_FLD_SET(order_i_flistp, PIN_FLD_RANK, (void *)&pri, ebufp);

    nota_buf = ( pin_buf_t *) pin_malloc( sizeof( pin_buf_t ));
    if ( nota_buf == NULL ) 
    {
		error_code = 105;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "couldn't allocate nemory for nota_buf", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		status = 2;
		strcpy(error_descr, "couldn't allocate nemory for nota_buf");
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_CODE, &error_code, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
		*op_out_flistpp = PIN_FLIST_COPY(order_seq_o_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
		goto UPDATE_REC;
    }   

    nota_buf->flag   = 0;
    nota_buf->size   = flistlen ;
    nota_buf->offset = 0;
    nota_buf->data   = ( caddr_t)flistbuf;
    nota_buf->xbuf_file = ( char *) NULL;

    PIN_FLIST_FLD_PUT( order_i_flistp, PIN_FLD_INPUT_FLIST, ( void *) nota_buf, ebufp );

    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_create_bb_action create input flist", order_i_flistp);
    PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, order_i_flistp, op_out_flistpp, ebufp);
    PIN_FLIST_FLD_COPY(order_i_flistp, PIN_FLD_ORDER_ID, *op_out_flistpp, PIN_FLD_ORDER_ID, ebufp);
     PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
        "fm_mso_prov_create_bb_action order create output flist", *op_out_flistpp);

	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		error_code = 106;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in getting sequence", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		status = 2;
		strcpy(error_descr, "Error in getting sequence");
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_CODE, &error_code, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
		*op_out_flistpp = PIN_FLIST_COPY(order_seq_o_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
		goto UPDATE_REC;
	}

	vp = PIN_FLIST_FLD_GET (order_i_flistp, PIN_FLD_ORDER_ID, 1, ebufp);
	
	if (vp) {
		status=1;
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_COPY(order_i_flistp, PIN_FLD_ORDER_ID, upd_in_flistp, PIN_FLD_ORDER_ID, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		*op_out_flistpp = PIN_FLIST_COPY(o_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
		goto UPDATE_REC;
	}
	else{
		status = 2;
		error_code=107;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Provisioning failed", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		strcpy(error_descr, "Provisioning failed");
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_CODE, &error_code, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);

		*op_out_flistpp = PIN_FLIST_COPY(o_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
		goto UPDATE_REC;
	}

	
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist", *op_out_flistpp);
	
UPDATE_REC:
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, upd_in_flistp, &upd_out_flistp, ebufp);
	
	if(!pbo_decimal_is_null(total_bal,ebufp)) pbo_decimal_destroy(&total_bal);
	
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&upd_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&upd_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bal_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bal_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&read_out_flistp, NULL);	
	PIN_FLIST_DESTROY_EX(&order_seq_o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_seq_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&order_i_flistp, NULL);
	return;
}

void
fm_mso_get_sequence_no(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp)
{
        pcm_context_t           *new_ctxp = NULL;
        pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *rs_flistp = NULL;
        pin_flist_t             *flistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *inc_flistp = NULL;
        char                    *name = NULL;
        poid_t                  *i_pdp = NULL;
        poid_t                  *pdp = NULL;
        poid_t                  *s_pdp = NULL;
        u_int64                 id = (u_int64)-1;
        u_int64                 db = 0;
        int32                   s_flags = 256;
        int32                   *hdr_nump = NULL;
        int32                   hdr_num = 0;
        int32                   hdr_inc = 1;
        char                    *hdr_str = NULL;
        char                    *t1 = NULL;
        char                    *t2 = NULL;
        char                    str[100];
		int64		database;

        srch_flistp = PIN_FLIST_CREATE(ebufp);

        i_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
        db = PIN_POID_GET_DB(i_pdp);
        name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 0, ebufp);
        if (PIN_ERR_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR,"sequence creation error flist", i_flistp);
                PIN_ERR_CLEAR_ERR(ebufp);
                PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_POID, i_pdp, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53999", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Sequence Creation Error", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS,&hdr_inc, ebufp);
                return;
        }
        pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, (void *)pdp, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, "select X from /data/sequence where F1 = V1 ", ebufp);
        flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, (char *)name, ebufp);
        flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sequence Object Find Input Flist", srch_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &rs_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"Sequence Object Find Output Flist", rs_flistp);

        r_flistp = PIN_FLIST_ELEM_GET(rs_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        s_pdp = PIN_FLIST_FLD_TAKE(r_flistp, PIN_FLD_POID, 0, ebufp);

        if (PIN_ERR_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "sequence creation error flist", rs_flistp);
                PIN_ERR_CLEAR_ERR(ebufp);
                PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_POID, i_pdp, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53999", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Sequence Creation Error", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &hdr_inc, ebufp);
                return;
        }
        else if (r_flistp)
        {
                hdr_str = PIN_FLIST_FLD_TAKE(r_flistp, PIN_FLD_HEADER_STR, 1, ebufp);
                if (hdr_str)
                {
                        t1 = strtok(hdr_str,":");
                        t2 = strtok(NULL, ":");
                }
                PIN_FLIST_DESTROY_EX(&rs_flistp, NULL);
                r_flistp = NULL;
                inc_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_PUT(inc_flistp, PIN_FLD_POID, s_pdp, ebufp);
                PIN_FLIST_FLD_SET(inc_flistp, PIN_FLD_HEADER_NUM, &hdr_inc, ebufp);
                //Updating the incremented value for the sequencing object
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Calling INC_FLDS");
                //PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);
				PCM_CONNECT(&new_ctxp, &database, ebufp);
                PCM_OP(ctxp, PCM_OP_INC_FLDS, PCM_OPFLG_LOCK_OBJ, inc_flistp, &r_flistp, ebufp);
                PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
                hdr_nump = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_HEADER_NUM, 0, ebufp);

                if (hdr_nump)
                {
                        hdr_num = *hdr_nump;
                }

                //str = pin_malloc(100);
                //memset(str,'\0',sizeof(str));
                memset(str,'\0',strlen(str));
                if (t1 && t2)
                {
                        sprintf(str, "%s%s%d", t2, t1, hdr_num);
                }
//              *r_flistpp = PIN_FLIST_COPY(r_flistp, ebufp);
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_CONCAT(*r_flistpp, r_flistp, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STRING, (void *)str, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SEPARATOR, (void *)t1, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_SEQUENCE_ID, (void *)t2, ebufp);

                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,  "SEQUENCE RETURN flist", *r_flistpp);
        }

        PIN_FLIST_DESTROY_EX(&inc_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);


        if (hdr_str)
        {
                free(hdr_str);
        }

        PIN_ERR_CLEAR_ERR(ebufp);
        return;

}
