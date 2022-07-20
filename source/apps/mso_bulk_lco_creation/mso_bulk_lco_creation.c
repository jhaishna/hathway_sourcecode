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
static  char    Sccs_id[] = "@(#)%Portal Version: mso_bulk_lco_creation.c:CUPmod7.3PatchInt:1:2015-Dec-31 18:51:33 %";
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


#define  NEW_RECORD     0  
#define  SUCCESS_RECORD     1
#define  ERROR_RECORD     2
#define  RETRY         2

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
	pin_flist_t			*ext_flistp = NULL;
	char				*usage_str = NULL;
	char				*format = "\nUsage:     %s [-verbose] -object_type </mso_mta_job/xxx> \n\n";
            
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
	char			*template = "select X from /mso_blk_lco_create where F1 = V1 ";
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
	0 PIN_FLD_TEMPLATE        STR [0] "select X from /mso_blk_lco_create F1 = V1"
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
	pin_flist_t			*in_flistp=NULL;
	pin_flist_t         *i_flistp = NULL;
	pin_flist_t         *o_flistp = NULL;
	
	pin_flist_t         *profile_flistp = NULL;
	pin_flist_t         *inh_flistp = NULL;
	pin_flist_t         *winfo_flistp = NULL;
	pin_flist_t         *nameinfo_flistp = NULL;
	pin_flist_t         *phone_flistp = NULL;
	pin_flist_t         *acctinfo_flistp = NULL;
	pin_flist_t         *bal_flistp = NULL;
	pin_flist_t         *upd_in_flistp = NULL;
	pin_flist_t         *upd_out_flistp = NULL;
	pin_flist_t         *seq_in_flistp = NULL;
	pin_flist_t         *seq_out_flistp = NULL;
	
	char			*name = NULL;
	char			*passwd = "XXXX";
	char			obj_type[50];
	int64			id = -1;
	int64			db = 1;
	
	poid_t			*plan_pdp = NULL;
	poid_t			*profile_pdp = NULL;
	poid_t			*acct_pdp = NULL;
	poid_t			*job_poid = NULL;

	char                	login[255];
	char			*fname = NULL;
	char			*null_str = "";
	char			*seq_no = NULL;
	int32			flags = 0;
	int32			comm_model = 0;
	int32			erp_ctr_id = 0;
	int32			comm_serv = 1;
	int32			phone_type = 5;
	int32			bus_type= 13000000;
	int32			curr = 356;
	int32			cont_pref = 0;
	void			*vp = NULL;
	int32			status = 1;
	int64			poid_id = 0;

	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}

	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search flist", srch_res_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode input flist", op_in_flistp);
	in_flistp = PIN_FLIST_COPY(op_in_flistp, ebufp);
	job_poid = PIN_FLIST_FLD_GET (in_flistp, PIN_FLD_POID, 1, ebufp);
	upd_in_flistp = PIN_FLIST_CREATE(ebufp);
	fname = PIN_FLIST_FLD_GET (in_flistp, PIN_FLD_FIRST_NAME, 1, ebufp);
	poid_id = PIN_POID_GET_ID(job_poid);
	memset(login, '\0', sizeof(login));
	sprintf(login, "BLKLCO%d", poid_id);
	
	/********************************************
	 *      prepare flist for opcode call in 
	 *           pin_mta_worker_opcode
	 *********************************************/

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_job prepared flist for main opcode", in_flistp);
	/*********************************************
	* MSO_OP_CUST_REGISTER_CUSTOMER
	* 0 PIN_FLD_POID           POID [0] 0.0.0.1 /plan -1 0
	* 0 PIN_FLD_USERID         POID [0] 0.0.0.1 /account 1290705 8			
	* 0 PIN_FLD_PASSWD_CLEAR    STR [0] "XXXX"
	* 0 PIN_FLD_LOGIN           STR [0] "first123120946"
	* 0 PIN_FLD_PROGRAM_NAME    STR [0] "OAP|csradmin"				
	* 0 MSO_FLD_CATV_ACCOUNT_OBJ   POID [0] NULL poid pointer
	* 0 PIN_FLD_FLAGS           INT [0] 0
	* 0 PIN_FLD_PROFILES      ARRAY [0] allocated 20, used 2
	* 1     PIN_FLD_INHERITED_INFO SUBSTRUCT [0] allocated 20, used 1
	* 2         MSO_FLD_WHOLESALE_INFO SUBSTRUCT [0] allocated 20, used 7
	* 3             MSO_FLD_COMMISION_MODEL   ENUM [0] 0
	* 3             MSO_FLD_ERP_CONTROL_ACCT_ID    INT [0] 0
	* 3             MSO_FLD_PP_TYPE        ENUM [0] 0					
	* 3             PIN_FLD_CUSTOMER_SEGMENT    INT [0] 1					
	* 3             MSO_FLD_PREF_DOM       ENUM [0] 1					
	* 3             PIN_FLD_PARENT         POID [0] 0.0.0.1 /account 3567230548 7 
	* 3             MSO_FLD_COMMISION_SERVICE   ENUM [0] 1
	* 1     PIN_FLD_PROFILE_OBJ    POID [0] 0.0.0.1 /profile/wholesale -1 0
	* 0 PIN_FLD_NAMEINFO      ARRAY [1] allocated 20, used 18
	* 1     PIN_FLD_EMAIL_ADDR      STR [0] "k@b.com"					
	* 1     PIN_FLD_COUNTRY         STR [0] "INDIA"					
	* 1     PIN_FLD_ZIP             STR [0] "591307"					
	* 1     PIN_FLD_STATE           STR [0] "KARNATAKA"					
	* 1     PIN_FLD_CITY            STR [0] "Belgaum"				
	* 1     PIN_FLD_ADDRESS         STR [0] "222"						
	* 1     PIN_FLD_COMPANY         STR [0] "hathway"					
	* 1     PIN_FLD_LAST_NAME       STR [0] "last"					
	* 1     PIN_FLD_MIDDLE_NAME     STR [0] ""						
	* 1     PIN_FLD_FIRST_NAME      STR [0] "first"						
	* 1     PIN_FLD_SALUTATION      STR [0] "Mr."						
	* 1     PIN_FLD_LASTSTAT_CMNT    STR [0] ""
	* 1     PIN_FLD_PHONES        ARRAY [5] allocated 20, used 2
	* 2         PIN_FLD_PHONE           STR [0] "1111111111"				
	* 2         PIN_FLD_TYPE           ENUM [0] 5
	* 1     MSO_FLD_LANDMARK        STR [0] ""						
	* 1     MSO_FLD_BUILDING_NAME    STR [0] "LAXMI EXTENSION"				
	* 1     MSO_FLD_STREET_NAME     STR [0] "LAXMI EXTENSION"			
	* 1     MSO_FLD_LOCATION_NAME    STR [0] "LAXMI EXTENSION"
	* 1     MSO_FLD_AREA_NAME       STR [0] "GOKAK"						
	* 0 MSO_FLD_ROLES           STR [0] ""
	* 0 PIN_FLD_ACCTINFO      ARRAY [0] allocated 20, used 10
	* 1     MSO_FLD_AREA            STR [0] "KA|KA-D58-C2|KA-D58-C865-A11755"		
	* 1     PIN_FLD_POID           POID [0] 0.0.0.1 /account -1 0
	* 1     PIN_FLD_BUSINESS_TYPE   ENUM [0] 13000000					
	* 1     PIN_FLD_CURRENCY        INT [0] 356
	* 1     MSO_FLD_RMAIL           STR [0] "k@b.com"					
	* 1     MSO_FLD_RMN             STR [0] "1111111111"					
	* 1     MSO_FLD_ET_ZONE         STR [0] ""
	* 1     MSO_FLD_CONTACT_PREF   ENUM [0] 0
	* 1     PIN_FLD_BAL_INFO      ARRAY [0]     NULL array ptr
	* 1     MSO_FLD_VAT_ZONE        STR [0] ""
	*********************************************/
	
	i_flistp = PIN_FLIST_CREATE(ebufp);
	
	plan_pdp = PIN_POID_CREATE(db, "/plan", id, ebufp);
	PIN_FLIST_FLD_PUT(i_flistp, PIN_FLD_POID, (void *)plan_pdp, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_USERID, i_flistp, PIN_FLD_USERID, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_PASSWD_CLEAR, passwd, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_LOGIN, login, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PROGRAM_NAME, i_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_CATV_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_FLAGS, &flags, ebufp);
	profile_flistp = PIN_FLIST_ELEM_ADD (i_flistp, PIN_FLD_PROFILES, 0, ebufp);
	inh_flistp = PIN_FLIST_SUBSTR_ADD (profile_flistp, PIN_FLD_INHERITED_INFO, ebufp);
	winfo_flistp = PIN_FLIST_SUBSTR_ADD (inh_flistp, MSO_FLD_WHOLESALE_INFO, ebufp);
	PIN_FLIST_FLD_SET(winfo_flistp, MSO_FLD_COMMISION_MODEL, &comm_model, ebufp);
	PIN_FLIST_FLD_SET(winfo_flistp, MSO_FLD_ERP_CONTROL_ACCT_ID, &erp_ctr_id, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_PP_TYPE, winfo_flistp, MSO_FLD_PP_TYPE, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CUSTOMER_SEGMENT, 
					winfo_flistp, PIN_FLD_CUSTOMER_SEGMENT, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_PREF_DOM, winfo_flistp, MSO_FLD_PREF_DOM, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PARENT, winfo_flistp, PIN_FLD_PARENT, ebufp);
	PIN_FLIST_FLD_SET(winfo_flistp, MSO_FLD_COMMISION_SERVICE, &comm_serv, ebufp);
	
	profile_pdp = PIN_POID_CREATE(db, "/profile/wholesale", id, ebufp);
	PIN_FLIST_FLD_PUT(profile_flistp, PIN_FLD_PROFILE_OBJ, (void *)profile_pdp, ebufp);
	
	nameinfo_flistp = PIN_FLIST_ELEM_ADD (i_flistp, PIN_FLD_NAMEINFO, 1, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_EMAIL_ADDR, nameinfo_flistp, PIN_FLD_EMAIL_ADDR, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_COUNTRY, nameinfo_flistp, PIN_FLD_COUNTRY, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ZIP, nameinfo_flistp, PIN_FLD_ZIP, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_STATE, nameinfo_flistp, PIN_FLD_STATE, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_CITY, nameinfo_flistp, PIN_FLD_CITY, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ADDRESS, nameinfo_flistp, PIN_FLD_ADDRESS, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_COMPANY, nameinfo_flistp, PIN_FLD_COMPANY, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_LAST_NAME, nameinfo_flistp, PIN_FLD_LAST_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_MIDDLE_NAME,nameinfo_flistp, PIN_FLD_MIDDLE_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_FIRST_NAME, nameinfo_flistp, PIN_FLD_FIRST_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_SALUTATION, nameinfo_flistp, PIN_FLD_SALUTATION, ebufp);
	PIN_FLIST_FLD_SET(nameinfo_flistp, PIN_FLD_LASTSTAT_CMNT, null_str, ebufp);
	
	phone_flistp = PIN_FLIST_ELEM_ADD (nameinfo_flistp, PIN_FLD_PHONES, 5, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PHONE, phone_flistp, PIN_FLD_PHONE, ebufp);
	PIN_FLIST_FLD_SET(phone_flistp, PIN_FLD_TYPE, &phone_type, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_LANDMARK, nameinfo_flistp, MSO_FLD_LANDMARK, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_BUILDING_NAME, nameinfo_flistp, 
							MSO_FLD_BUILDING_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_STREET_NAME, nameinfo_flistp, 
							MSO_FLD_STREET_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_LOCATION_NAME, nameinfo_flistp, 
							MSO_FLD_LOCATION_NAME, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_AREA_NAME, nameinfo_flistp, MSO_FLD_AREA_NAME, ebufp);

	PIN_FLIST_FLD_SET(i_flistp, MSO_FLD_ROLES, null_str, ebufp);
	acctinfo_flistp = PIN_FLIST_ELEM_ADD (i_flistp, PIN_FLD_ACCTINFO, 0, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, MSO_FLD_AREA, acctinfo_flistp, MSO_FLD_AREA, ebufp);
	
	acct_pdp = PIN_POID_CREATE(db, "/account", id, ebufp);
	PIN_FLIST_FLD_PUT(acctinfo_flistp, PIN_FLD_POID, (void *)acct_pdp, ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_BUSINESS_TYPE, &bus_type, ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, PIN_FLD_CURRENCY, &curr, ebufp);
	
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_EMAIL_ADDR, acctinfo_flistp, MSO_FLD_RMAIL, ebufp);
	PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_PHONE, acctinfo_flistp, MSO_FLD_RMN, ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, MSO_FLD_ET_ZONE, null_str, ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, MSO_FLD_CONTACT_PREF, &cont_pref, ebufp);
	bal_flistp = PIN_FLIST_ELEM_ADD (acctinfo_flistp, PIN_FLD_BAL_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(acctinfo_flistp, MSO_FLD_VAT_ZONE, null_str, ebufp);
	
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "main opcode input flist", i_flistp);
	PCM_OP(ctxp, MSO_OP_CUST_REGISTER_CUSTOMER, 0, i_flistp, &o_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Account Creation Error", ebufp);
		status = 2;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_COPY(o_flistp, PIN_FLD_ERROR_CODE, 
				upd_in_flistp, PIN_FLD_ERROR_CODE, ebufp);
		PIN_FLIST_FLD_COPY(o_flistp, PIN_FLD_ERROR_DESCR, 
				upd_in_flistp, PIN_FLD_ERROR_DESCR, ebufp);
		*op_out_flistpp = PIN_FLIST_COPY(o_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
		goto UPDATE_REC;
	}

	vp = PIN_FLIST_FLD_GET (o_flistp, PIN_FLD_ACCOUNT_NO, 1, ebufp);
	if (vp) {
		status=1;
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_ACCOUNT_NO, (char *)vp, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		*op_out_flistpp = PIN_FLIST_COPY(o_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
		goto UPDATE_REC;
	}
	else{
		status = 2;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Account Creation failed", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_COPY(o_flistp, PIN_FLD_ERROR_CODE, upd_in_flistp, 
							PIN_FLD_ERROR_CODE, ebufp);
		PIN_FLIST_FLD_COPY(o_flistp, PIN_FLD_ERROR_DESCR, upd_in_flistp, 
							PIN_FLD_ERROR_DESCR, ebufp);
		*op_out_flistpp = PIN_FLIST_COPY(o_flistp, ebufp);
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
			"pin_mta_worker_opcode output flist from main opcode", *op_out_flistpp); 
		goto UPDATE_REC;
	}

	
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist", *op_out_flistpp);
	
	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_mta_worker_opcode error", ebufp);
		status = 2;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_POID, job_poid, ebufp);
		PIN_FLIST_FLD_SET(upd_in_flistp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_COPY(o_flistp, PIN_FLD_ERROR_CODE, upd_in_flistp, 
								PIN_FLD_ERROR_CODE, ebufp);
		PIN_FLIST_FLD_COPY(o_flistp, PIN_FLD_ERROR_DESCR, upd_in_flistp, 
								PIN_FLD_ERROR_DESCR, ebufp);
		goto UPDATE_REC;
	}

UPDATE_REC:
	PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, upd_in_flistp, &upd_out_flistp, ebufp);
	
	PIN_FLIST_DESTROY_EX(&seq_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&seq_out_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&o_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&upd_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&upd_out_flistp, NULL);
	return;
}
