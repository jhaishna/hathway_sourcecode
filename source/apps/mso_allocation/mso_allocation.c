/*******************************************************************
 *
 *    Copyright (c) 1999-2007 Oracle. All rights reserved.
 *
 *    This material is the confidential property of Oracle Corporation
 *    or its licensors and may be used, reproduced, stored or transmitted
 *    only in accordance with a valid Oracle license or sublicense agreement.
 *    ====================================================================
 *    This application is created to do Automatic Payment Allocation for 
 *    Customer Accounts (Migrated / Created) having credit before billing
 *    and billing process not allocating the items.
 *
 *    Author: Vilva Sabarikanth
 *    Date: 14-May-2014
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
	i_param_count = PIN_FLIST_ELEM_COUNT(param_flistp, PIN_FLD_PARAMS, ebufp);

	if ( i_param_count == 1)
	{
		while((flistp = PIN_FLIST_ELEM_GET_NEXT(param_flistp, PIN_FLD_PARAMS, &rec_id, 1, &cookie, ebufp))!= NULL) 
		{
			option = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_NAME, 0, ebufp);
			/***************************************************
			 * Test options options.
			 ***************************************************/
			if(option && (strcmp(option, "-object_type") == 0)) 
			{    

				object_type = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 1, ebufp);

				if(!object_type) {
					mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
				}
				else
				{
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, object_type);
					PIN_FLIST_FLD_COPY (flistp, PIN_FLD_PARAM_VALUE, app_flistp,  PIN_FLD_OBJ_TYPE, ebufp);
				}

				PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
				cookie = prev_cookie;
			}
			else 
			{
				prev_cookie = cookie;
			}
		}
	}
	else
	{
		mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR,"Invalid input Parameters Passed ");
	}

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
	char			*template = "select X from /billinfo 1, /item 2 where 1.F1 = 2.F2 and 2.F3 = V3 and 2.F4 < 0";
	int32			s_flags = 256;
	poid_t			*s_pdp = NULL;
	poid_t			*pdp = NULL;
	int32			open_items = 2;
	int32			link_direction=1;
	pin_decimal_t	*zero = NULL;
	int64			id = -1;
	int64			id1 = 1;
	int64			db = 0;
	void			*vp = NULL;


	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);
	*s_flistpp = 0;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "pin_mta_init_search application info flist", app_flistp);
	
	s_flistp = PIN_FLIST_CREATE(ebufp);
	zero = pbo_decimal_from_str("0.0", ebufp);
	/***********************************************************
	 * Simple search flist
	 ***********************************************************
	0 PIN_FLD_POID                      POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS                      INT [0] 256
	0 PIN_FLD_TEMPLATE                   STR [0] "select X from /billinfo 1, /item 2 where 1.F1 = 2.F2 and 2.F3 = V3 and 2.F4 < 0"
	0 PIN_FLD_RESULTS                  ARRAY [0] allocated 2, used 2
	1     PIN_FLD_POID                  POID [0] 0.0.0.1 /billinfo 1 0
	1     PIN_FLD_ACCOUNT_OBJ           POID [0] 0.0.0.1 /account 1 0
	0 PIN_FLD_ARGS                     ARRAY [1] allocated 1, used 1
	1     PIN_FLD_POID                  POID [0] 0.0.0.1 /billinfo 1 0
	0 PIN_FLD_ARGS                     ARRAY [2] allocated 1, used 1
	1     PIN_FLD_BILLINFO_OBJ          POID [0] 0.0.0.1 /billinfo 1 0
	0 PIN_FLD_ARGS                     ARRAY [3] allocated 1, used 1
	1     PIN_FLD_STATUS                ENUM [0] 2
	0 PIN_FLD_ARGS                     ARRAY [4] allocated 1, used 1
	1     PIN_FLD_DUE                DECIMAL [0] 0
	***********************************************************/

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 0, ebufp);
	if(vp)
		db = PIN_POID_GET_DB ((poid_t*)vp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "1");
	s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, template, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	/***********************************************************
	 * Search Parameters
	 ***********************************************************/
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "2");
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	pdp = PIN_POID_CREATE(db, "/billinfo", id1, ebufp);
	PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_POID, pdp, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 2, ebufp);
	pdp = PIN_POID_CREATE(db, "/billinfo", id1, ebufp);
	PIN_FLIST_FLD_PUT(tmp_flistp, PIN_FLD_BILLINFO_OBJ, pdp, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_STATUS, &open_items, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DUE, zero, ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "3");
	
	/***********************************************************
	 * Search results
	 ***********************************************************/

	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	
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
	pin_flist_t			*srch_res_flistp,
	pin_flist_t			*op_in_flistp,
	pin_flist_t			**op_out_flistpp,
	pin_flist_t			*ti_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t			*in_flistp=NULL;

	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		return;
	}

	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode search flist", srch_res_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode input flist", op_in_flistp);
	in_flistp = PIN_FLIST_COPY(op_in_flistp, ebufp);

	/********************************************
	 *      prepare flist for opcode call in 
	 *           pin_mta_worker_opcode
	 *********************************************/

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_job prepared flist for main opcode", in_flistp);

	/*******************************************
	 *     Calling Payment Allocation Opcode
	 *******************************************/

	PCM_OP(ctxp, MSO_OP_AR_ALLOCATE, 0, in_flistp, op_out_flistpp, ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist", *op_out_flistpp);

	if (PIN_ERR_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_mta_worker_opcode error", ebufp);
	}else{
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist from main opcode", 
			*op_out_flistpp);
	}

	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
	return;
}


