/*****************************************************************************
 *
* Copyright (c) 1999, 2013, Oracle and/or its affiliates. All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 ****************************************************************************/

#ifndef lint
static char Sccs_id[]="@(#)$Id: mso_bulk_entity_change.c /cgbubrm_7.3.2.rwsmod/17 2013/03/06 19:21:09 rnandi Exp $";
#endif

/*******************************************************************
 * This file contains the monthly billing application.
 *
 * It basically gathers all the parameters and calls routines
 * elsewhere that do the actual work of billing billinfo objects.
 *******************************************************************/

#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "pcm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "psiu_business_params.h"
#include "psiu_round.h"
#include "pin_bill.h"
#include "pin_pymt.h"
#include "pin_cust.h"
#include "pin_mta.h"
#include "pin_os_string.h"
#include "mso_ops_flds.h"
#include "mso_cust.h"


#define  NEW_RECORD         0  
#define  SUCCESS_RECORD     1
#define  ERROR_RECORD       2


/*******************************************************************
 * This file contains implemetation of mso_bulk_entity_change application
 * based on CAF/MTA framework.
 *******************************************************************/
static void pin_billing_tmp_inp_file();
static char *pin_billing_trim_comment_from_flist ();

/*******************************************************************
 * Global Data Area
 ******************************************************************/
 poid_t                  *last_acctp = NULL;

/*******************************************************************
 * Re-use some code from pin_billd untill billing utilities created
 * just for compilation and linking adding missing externals
 *******************************************************************/
void usage(char	*prog){ return;}
int verbose;
int test_mode;

/*******************************************************************
 * Local functions
 *******************************************************************/
void
pin_entity_change_cmd_opt (
		pin_flist_t	*param_flistp,
		pin_flist_t	*app_flistp,
		pin_errbuf_t	*ebufp);

void
pin_billing_init_search(
		pin_flist_t		*app_flistp,
		pin_flist_t		**s_flistpp,
		pin_errbuf_t	*ebufp);

/*******************************************************************
 * seq_no_parse_id_sep should be moved to business logic library
 *******************************************************************/
static void
seq_no_parse_id_sep(
		char	*header,
		char	*sequence_id,
		char	*separator);
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
	int32		midnight = 0;
	pcm_context_t *ctxp = 0;
	time_t		current_t = 0;
	void		*vp = 0;
	char		*xmlFileName = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
		"pin_mta_config parameters flist", 
					   param_flistp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, 
	   "pin_mta_config application info flist", 
					   app_flistp);


	/***********************************************************
	 * Get business parameters
	 ***********************************************************/
	ctxp = pin_mta_main_thread_pcm_context_get (ebufp);

//	midnight = psiu_bparams_get_int(ctxp,
//		PSIU_BPARAMS_BILLING_PARAMS,
//		PSIU_BPARAMS_BILLING_CYCLE_OFFSET,
//		ebufp);
//
//	if (midnight < 0 || midnight > 23) {
//		pin_set_err(ebufp, PIN_ERRLOC_APP,
//					PIN_ERRCLASS_SYSTEM_DETERMINATE,
//					PIN_ERR_INVALID_CONF, 
//				PIN_FLD_BILLING_CYCLE_OFFSET, 0, 0);
//	}else{
//		PIN_FLIST_FLD_SET (app_flistp,
//				PIN_FLD_BILLING_CYCLE_OFFSET,
//						   &midnight, ebufp);
//	}

	/***********************************************************
	 * Get command line options
	 ***********************************************************/
	pin_entity_change_cmd_opt(param_flistp, app_flistp, ebufp);
	
//	/***********************************************************
//	 * If -file <xml file>, 
//	 * get xml data to be used in search filtering
//	 * on the basis of billing segments and DOMs
//	 ***********************************************************/
//	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FILENAME, MTA_OPTIONAL, ebufp);
//	if(vp) {
//		xmlFileName = (char *)vp;
//		get_xml_bill_run_data(xmlFileName, &app_flistp, ebufp);
//		pin_billing_tmp_inp_file(app_flistp,ebufp);
//	}

	if (PIN_ERR_IS_ERR(ebufp)) {
  	   PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
	   "pin_mta_config error in getting params from pinconf or cmdline or bill run control xml file ", ebufp);
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
	char		usage_str[1024] = {""};
	char		prog_name[256];

	PIN_ERRBUF_CLEAR (&ebuf);
	
	pin_strlcpy(prog_name, prog, sizeof(prog_name));
	sprintf(usage_str, "\nUsage:\n\t%s %s\n\n",
		prog_name,
			"[-from_file  <Input File Name]");
			

	ext_flistp =
	 pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO,
						   &ebuf);

	PIN_FLIST_FLD_SET (ext_flistp, PIN_FLD_DESCR, usage_str, &ebuf);

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

	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
		   "pin_mta_post_usage parameters flist", 
					   param_flistp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
		   "pin_mta_post_usage application info flist", 
					   app_flistp);

	ext_flistp =
	pin_mta_global_flist_node_get_no_lock (PIN_FLD_EXTENDED_INFO,
	   ebufp);

	vp = PIN_FLIST_FLD_GET (ext_flistp, PIN_FLD_DESCR, 1, ebufp);

	if(vp) {
		printf("%s",(char*) vp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "pin_mta_post_usage error",ebufp);
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
	pin_flist_t     *oper_flistp = NULL;
	void            *vp = 0;
        int32     mta_flags = 1;
        int32     enable_ara = 0;
        pin_decimal_t *current_total = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
	   "pin_mta_init_app application info flist", 
					   app_flistp);

        vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FLAGS, 0, ebufp);
        if (vp) {
               mta_flags = *(int32*)vp;
        }

        /******************************************************************
         * Initialize OPR flist to capture ARA Release-1 data if split not
         * set.
         ******************************************************************/

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "pin_mta_init_app error", ebufp);
	}
	return;
}

/******************************************************************
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

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
	   "pin_mta_post_init_app application info flist", 
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
		pin_errbuf_t		*ebufp)
{

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	*s_flistpp = 0;

	pin_billing_init_search(app_flistp, s_flistpp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "pin_mta_init_search error", ebufp);
	}

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
           "pin_mta_init_search search info flist",
                                           *s_flistpp);

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
    pin_flist_t             *w_o_flist = NULL;
    pin_flist_t             *w_inflist = NULL;
    int32                   new_status = SUCCESS_RECORD;
    int32                   new_err_status = ERROR_RECORD;
    int32                   opcode = MSO_OP_INTEG_CREATE_JOB;
    int32                   *opcode_status = NULL;
    char                    *msg[128];
    char		    *error_descr = NULL;
    char		    *error_code	= NULL;
    char		    err_code[20];
    pin_err_location_t      err_location;
    pin_err_class_t         err_class;
    pin_err_t               pin_err;
    pin_fld_num_t           err_field;
    pin_rec_id_t            err_rec_id;
    pin_err_reserved_t      err_reserved;


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
    

    /*******************************************
    *     Get the opcode number to be called
    *******************************************/
    
//    sprintf(msg, " Opcode value is %d ",*opcode);
//    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, msg);
    
    /*******************************************
    *            Execute opcode
    *******************************************/
    PCM_OP (ctxp, opcode, 0, op_in_flistp, op_out_flistpp, ebufp);

    PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "pin_mta_worker_opcode output flist of opcode is  ",
                *op_out_flistpp);

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

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG,
		   "pin_mta_post_worker_exit thread info flist", 
				   ti_flistp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		 "pin_mta_post_worker_exit error", ebufp);
	}
	return;
}


void
pin_entity_change_cmd_opt (
		pin_flist_t	*param_flistp,
		pin_flist_t	*app_flistp,
		pin_errbuf_t	*ebufp)
{
	int32		rec_id = 0;
	pin_cookie_t	cookie = 0;
	pin_cookie_t	prev_cookie = 0;
	pin_flist_t	*flistp = 0;
	char		*option = 0;
	int32		mta_flags = 0;
	void		*vp = 0;
	void		*vp1 = 0;
	int32		i = 0;
	time_t		end_t = 0;
	int32		midnight_offset = 0;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	/***************************************************
	 * Default of account's status options.
	 ***************************************************/

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FLAGS, 0,ebufp);
	if(vp)
		mta_flags = *(int32*)vp;

	mta_flags = mta_flags | MTA_FLAG_VERSION_NEW;


	while((flistp = /*!=NULL*/
		   PIN_FLIST_ELEM_GET_NEXT(param_flistp, PIN_FLD_PARAMS, &rec_id,
		   1, &cookie, ebufp))!= NULL){
		
		option = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_NAME, 0, ebufp);
		if (!option) {
			return;	
		}
		/***************************************************
		 * Account's status options.
		 ***************************************************/
		if(strcmp(option, "-from_file") == 0) {
			vp = PIN_FLIST_FLD_GET (flistp, PIN_FLD_PARAM_VALUE, 0, ebufp);
			
			if (vp == 0) {
				
				mta_flags = mta_flags | MTA_FLAG_USAGE_MSG;
				pin_set_err(ebufp, PIN_ERRLOC_APP, 
					PIN_ERRCLASS_APPLICATION,
					PIN_ERR_INVALID_CONF, PIN_FLD_FILENAME, 0, 0);
			} else {

				PIN_FLIST_FLD_SET(app_flistp, PIN_FLD_INPUT_FILE_NAME, vp, ebufp);
			}
				
			PIN_FLIST_ELEM_DROP(param_flistp, PIN_FLD_PARAMS, rec_id, ebufp);
			cookie = prev_cookie; 
		}
		else {
			prev_cookie = cookie;
		}
	}
	
	PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FLAGS, &mta_flags, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"pin_entity_change_cmd_opt error", ebufp);
	}
	return;
}

void
pin_bulk_entity_change_config_validation (
		pin_flist_t	*app_flistp,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t	*flistp = 0;
	int32		mta_flags = 0;
	void		*vp = 0;
	int32		i = 0;
	int32		fetch_size = 0;
	int32		batch_size = 0;
	int32		step_size = 0;
	int32		pay_type = 0;
	time_t		end_t = 0;
	time_t		current_t = 0;
	int32		sponsorship = 0;
	char		*name = 0;
	int32		err = 0;
	int32		status = 0;
	int32		midnight_offset = 0;
	int		discount = 0;
	pcm_context_t   *ctxp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR (ebufp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FLAGS, 
					MTA_MANDATORY, ebufp);
	if(vp)
		mta_flags = *(int32*)vp;

	if(mta_flags & MTA_FLAG_USAGE_MSG){
		return;
	}

	/***********************************************************
	 * If fetch_size < per_batch, make fetch_size = batch_size
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FETCH_SIZE, 
					MTA_MANDATORY, ebufp);
	if(vp)
		fetch_size = *((int32*)vp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_BATCH_SIZE, 
					MTA_MANDATORY, ebufp);
	if(vp)
		batch_size = *((int32*)vp);

	if(fetch_size < batch_size) {
		fetch_size = batch_size;
		PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_FETCH_SIZE,
				   &fetch_size, ebufp);
	}

	/***********************************************************
	 * If step_size = 0, make step_size = batch_size
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_STEP_SIZE, 
					MTA_OPTIONAL, ebufp);
	if(vp)
		step_size = *((int32*)vp);

	if(step_size == 0) {
		step_size = batch_size;
		PIN_FLIST_FLD_SET (app_flistp, PIN_FLD_STEP_SIZE,
				   &step_size, ebufp);
	}

	/***********************************************************
	 * If the -end option is specified, then the date specified
	 * is converted by billd_convert_str_to_time_t function, adds
	 * extra 1 second. The below statement subtracts that 1sec 
	 * otherwise if -end option is not specified calculates the 
 	 * midnight hour. 
	 ***********************************************************/


	ctxp = pin_mta_main_thread_pcm_context_get (ebufp);

	/***********************************************************
	 * Check sponsorship option
	 ***********************************************************/
	/***********************************************************
     	* Check/set the discount option.
     	***********************************************************/
	/***********************************************************
	 * Check status field
	 ***********************************************************/
	if (!PIN_ERR_IS_ERR(ebufp)) {
		vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_STATUS, 
					MTA_MANDATORY, ebufp);
		if(vp)
			status = *((int32*)vp);

		if(status == 0) {
			pin_set_err(ebufp, PIN_ERRLOC_APP, 
				PIN_ERRCLASS_APPLICATION,
				PIN_ERR_INVALID_CONF, 
				PIN_FLD_STATUS, 0, 0); 
		}
	}
	/***********************************************************
	 * Check end_t > current_t
	 ***********************************************************/
	if (!PIN_ERR_IS_ERR(ebufp)) {
	
		current_t = pin_virtual_time ((time_t *)NULL);

		if ((end_t == 0) || (end_t > current_t)) {

			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "bad config: time");
			fprintf(stderr,"bad config: time\n");

			pin_set_err(ebufp, PIN_ERRLOC_APP, 
				PIN_ERRCLASS_APPLICATION,
				PIN_ERR_INVALID_CONF, 
				PIN_FLD_END_T, 0, 0); 
		}
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			 "pin_bulk_entity_change_config_validation error", ebufp);
	}
	return;
}

/*******************************************************************
 * Functions to parse separator and sequence_id from header_str.
 *******************************************************************/
static void
seq_no_parse_id_sep(
		char	*header,
		char	*sequence_id,
		char	*separator)
{
	char    delimiter;
	char    *buf;
	int     n = 0;
	int     i = 0;
	int     j = 0;
 
	if (!header) {
		return;
	}
 
	buf = header;
 
	/* skip leading spaces */
	while (buf[n] && isspace(buf[n])) {
		n++;
	}
 
	delimiter =  buf[n];
	n++;
 
	/* get the separator */
	while (buf[n] && (buf[n] != delimiter)) {
		separator[i++] = buf[n++];
	}
 
	/* skip the delimiter */
	n++;
 
	/* skip leading spaces */
	while (buf[n] && isspace(buf[n])) {
		n++;
	}
 
	/* get the sequence id */
	while (buf[n] && (buf[n] != delimiter)) {
		sequence_id[j++] = buf[n++];
	}
 
	separator[i] = '\0';
	sequence_id[j] = '\0';
	return;
}

/*******************************************************************
 * Search flist preparation
 *******************************************************************/
void
pin_billing_init_search(
		pin_flist_t		*app_flistp,
		pin_flist_t		**s_flistpp,
		pin_errbuf_t	*ebufp)
{
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*flistp_tmp = NULL;
	pin_flist_t	*flistp_tmp1 = NULL;	
	pin_flist_t	*prod_flistp = NULL;
	poid_t		*s_pdp = NULL;
	void		*vp = NULL;
	int64		database = 0;
	pin_status_t	status = PIN_STATUS_NOT_SET; 
	time_t		bill_time = 0;
	time_t		bill_time1 = 0;


	int32		fetch_size = 0;
	int32		pay_type = 0;
	int32		perr = 0;
	int32		enforce_billing = 0;
	int32		*flagp = NULL;
	char		template[1024] = {0};
	char		tmp_template[1024] = {0};
	char		*text1 = "";
	char		*text2 = "";
	char		*group_type = "/group/sharing/discounts";
	char		*rem_num = "";	

	char		str_dom_buf[1024] = {""};	
	int32		sponsorship = 0; 
	int32		sflags = 0;
	int32		err = PIN_ERR_NONE;
	int32		status_flag = PIN_BOOLEAN_FALSE;
	int32		srch_flags = SRCH_DISTINCT;
	int32		pcm_srch_flags = 0;
	pin_bill_billing_state_t bill_state = PIN_ACTG_CYCLE_ALL_CHARGES_DONE;
	pin_bill_billing_status_t bill_status = PIN_BILL_ACTIVE;
	int32		agr_num = 0;
	int32		mta_flags = 0;
	int32		enable_ara = 0;
	int32		apply_fees = 0;
	int32		finalize_bill = 0;
	pcm_context_t *ctxp = 0;
	int32		remit_flag = 0;
	int32		cb_delay = 0;
	int32		days = 0;
	int32		rem_sec = 0;
	char		*program = 0;
	int32		leaf = 0;
	int32		parent_flag = 0;
	int32		retry=0;
	int32		set_error_status=1;
	int32		remit = PIN_BOOLEAN_FALSE;
	int32		sponsorship_flag = 0;
	int		attrib = 1;
	int		discount = 0;
	int32		recId = 0;
	pin_cookie_t	cookie = 0;
	pin_flist_t	*flist_iter = NULL;
	int		dom_count = 0;
	int		bs_count = 0;
	int		i = 0;
	char* 		from_file = NULL;
	int	 	no_of_objs =2;	 		
	

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);


	/***********************************************************
	 * Get infor from application flist node
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_POID_VAL, 
				MTA_MANDATORY, ebufp);
	if(vp)
		database = PIN_POID_GET_DB ((poid_t*)vp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_STATUS, 
				MTA_OPTIONAL, ebufp);
	if(vp)
		status = *((int32*)vp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_INPUT_FILE_NAME, 
				MTA_OPTIONAL, ebufp);
	if(vp)
		from_file = (char*)vp;

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_END_T, 
				MTA_OPTIONAL, ebufp);
	if(vp){
		bill_time = *((time_t*)vp);
		bill_time1 = bill_time;
	}

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FETCH_SIZE, 
				MTA_MANDATORY, ebufp);
	if(vp)
		fetch_size = *((int32*)vp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_PAY_TYPE, 
				MTA_OPTIONAL, ebufp);
	if(vp)
		pay_type = *((int32*)vp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_FLAGS, 
				MTA_MANDATORY, ebufp);
	if(vp)
		mta_flags = *((int32*)vp);

        vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_ATTRIBUTE, 
                                MTA_OPTIONAL,ebufp);
        if(vp) 
                enable_ara = *((int32*)vp);


	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_REMIT_FLD_NO, 
				MTA_OPTIONAL, ebufp);
	if(vp)
		remit_flag = *((int32*)vp);

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_CFG_DELAY, 
				MTA_OPTIONAL, ebufp);
	if(vp) {
		cb_delay = *((int32*)vp);   /* cb_delay in seconds */
		rem_sec = cb_delay % 86400; /* 86400 = 60*60*24 */
		days = (cb_delay - rem_sec)/86400;
	}

	vp = PIN_FLIST_FLD_GET (app_flistp, PIN_FLD_NAME, 
				MTA_MANDATORY, ebufp);
	if(vp)
		program = (char*)vp;

	ctxp = pin_mta_main_thread_pcm_context_get (ebufp);

	/***********************************************************
	 * Allocate the search flist.
	 ***********************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * check if billinfo status is passed into to mso_bulk_entity_change
	 * application since this field will be an optional field.
	 ***********************************************************/
	if ((status == PIN_STATUS_ACTIVE) ||
		(status == PIN_STATUS_INACTIVE) ||
		(status == PIN_STATUS_CLOSED)) {
			status_flag = PIN_BOOLEAN_TRUE;
	}

	/***********************************************************
	 * Create search poid and flags.
	 ***********************************************************/

      if (from_file!=NULL) {

		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FILENAME,(void *) from_file, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_COUNT,(void *) &no_of_objs, ebufp);
		flistp_tmp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		vp = (void *) NULL;
		PIN_FLIST_FLD_SET(flistp_tmp, PIN_FLD_POID,vp, ebufp);
 		PIN_FLIST_FLD_SET (flistp_tmp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp); 
       } 

        /**********************************************************
	 * Set the results.
        ***********************************************************/
	 *s_flistpp = s_flistp;

	/***********************************************************
	 * Clean up.
	 ***********************************************************/
	 Cleanup:

	/* Error? */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"pin_billing_init_search error", ebufp);
	}

	return;
}

