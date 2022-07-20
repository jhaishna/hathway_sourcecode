/*******************************************************************
 * File:	fm_mso_ar_get_details.c14-11-2013
 * Opcode:	MSO_OP_AR_GET_DETAILS 
 * Owner:	Asish Joshi
 * Created:	17-SEPT-2013
 * Description: 

 * Modification History:
 * Modified by : Gautam Adak
 * Purpose     : Memory leaks due to using same flist for output multiple times without destroying


 
 Sample I/P Flist for wrapper opcode

0 PIN_FLD_DESCR           STR [0] "Search Bill/Item/Event details for a bill"
0 PIN_FLD_FLAGS           INT [0] 0 [0-Bill, 1- Item, 2- Event search]
0 MSO_FLD_SERVICE_TYPE   ENUM [0] 0
0 PIN_FLD_PROGRAM_NAME    STR [0] "MSO_OP_AR_GET_DETAILS"
0 PIN_FLD_BILL_NO         STR [0] "BILL-218"  [ Either give the bill num like "BILL-218" Or, ""]
0 PIN_FLD_POID     POID [0] 0.0.0.1 /account 368570 0
0 PIN_FLD_START_T      TSTAMP [0] (1411065000) Fri Sep 19 00:00:00 2010
0 PIN_FLD_END_T        TSTAMP [0] (1411065000) Fri Sep 19 00:00:00 2010

Out Put
For Bill Flag =0
0 PIN_FLD_POID           POID [0] 0.0.0.1 /bill 447399 4
0 PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 368570 0
0 PIN_FLD_ADJUSTED     DECIMAL [0] 0
0 PIN_FLD_BILL_NO         STR [0] "BILL-218"
0 PIN_FLD_CURRENT_TOTAL DECIMAL [0] 10
0 PIN_FLD_DUE          DECIMAL [0] 10
0 PIN_FLD_DUE_T        TSTAMP [0] (1400783400) Fri May 23 00:00:00 2014
0 PIN_FLD_PARENT         POID [0] 0.0.0.0  0 0
0 PIN_FLD_RECVD        DECIMAL [0] 0
0 PIN_FLD_TOTAL_DUE    DECIMAL [0] 73.09

For Item Flag=1
0 PIN_FLD_POID           POID [0] 0.0.0.1 /search -1 0
0 PIN_FLD_RESULTS       ARRAY [0] allocated 20, used 4
1     PIN_FLD_POID           POID [0] 0.0.0.1 /item/misc 449447 1
1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 368570 0
1     PIN_FLD_DUE          DECIMAL [0] 0
1     PIN_FLD_BILL_OBJ       POID [0] 0.0.0.1 /bill 447399 0
0 PIN_FLD_RESULTS       ARRAY [1] allocated 20, used 4
1     PIN_FLD_POID           POID [0] 0.0.0.1 /item/cycle_forward 449127 2
1     PIN_FLD_ACCOUNT_OBJ    POID [0] 0.0.0.1 /account 368570 13
1     PIN_FLD_DUE          DECIMAL [0] 10
1     PIN_FLD_BILL_OBJ       POID [0] 0.0.0.1 /bill 447399 0

For Event Flag=2
0 PIN_FLD_POID           POID [0] 0.0.0.1 /account 368570 13
0 PIN_FLD_ADJUSTMENT_INFO  ARRAY [1] allocated 20, used 1
1     PIN_FLD_RESOURCE_ID     INT [0] 356
0 PIN_FLD_EVENTS        ARRAY [0] allocated 20, used 1
1     PIN_FLD_POID           POID [0] 0.0.0.1 /event/billing/product/fee/cycle/cycle_forward_monthly 285468402943186535 0

 * Document History
 *
 * DATE          /  CHANGE                                       /  Author
 * 17-SEP-2013   / Initial Version                               / Asish Joshi
 * 17-Oct-2014   / Added BB Customzations and Error Handling     / Harish Kulkarni
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Podrtal Version: fm_mso_ar_get_details.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "pcm_ops.h"
#include "mso_ops_flds.h"
#include "ops/ar.h"
#include "ops/bill.h"


/**************************************
* External Functions start
***************************************/
/**************************************
* External Functions end
***************************************/

 
/**************************************
* Local Functions start
***************************************/
static void
fm_mso_get_last_bill_details(
	pcm_context_t		*ctxp,
	poid_t				*acnt_pdp,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

	static void
fm_mso_get_last_item_details(
	pcm_context_t		*ctxp,
	poid_t				*bill_pdp,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

	static void
fm_mso_get_last_event_details(
	pcm_context_t		*ctxp,
	poid_t				*bill_pdp,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_bal_grp_details(
	pcm_context_t		*ctxp,
	poid_t				*acnt_pdp,
	poid_t			*bnfo_pdp,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_user_id(
	pcm_context_t		*ctxp,
	poid_t				*user_id,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_service_type(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_validate_acnt_no(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_bill_details(
	pcm_context_t		*ctxp,
	pin_flist_t			*i_flistp,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp);



/*******************************************************************
 * MSO_OP_AR_GET_DETAILS 
 * 
 *******************************************************************/

EXPORT_OP void
mso_op_ar_get_details(
	cm_nap_connection_t	*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_ar_get_details(
	pcm_context_t		*ctxp,
	int32				flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_AR_GET_DETAILS  
 *******************************************************************/
void 
mso_op_ar_get_details(
	cm_nap_connection_t	*connp,
	int32				opcode,
	int32				flags,
	pin_flist_t			*i_flistp,
	pin_flist_t			**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	poid_t			*inp_pdp = NULL;
	pin_flist_t		*r_flistp = NULL;
	int32			srch_customer_failure = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERRBUF_CLEAR(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53070", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
										
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Data incorrect error outflist flist", r_flistp);
		*r_flistpp=PIN_FLIST_COPY(r_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
//		PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
		return;	
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_AR_GET_DETAILS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,	PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,	"op_mso_ar_get_details error",	ebufp);

		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53070", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
							
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Data incorrect error outflist flist", r_flistp);
		*r_flistpp=PIN_FLIST_COPY(r_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
//		PIN_FLIST_DESTROY_EX(&i_flistp, NULL);

		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_ar_get_details input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_ar_get_details(ctxp, flags, i_flistp, r_flistpp, ebufp);
	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_ar_get_details error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53070", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
										
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Data incorrect error outflist flist", r_flistp);
		*r_flistpp=PIN_FLIST_COPY(r_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
//		PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"fm_mso_ar_get_details output flist", *r_flistpp);
	}
	
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_ar_get_details(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*acnt_info = NULL;
	pin_flist_t		*out_flist = NULL;
	pin_flist_t		*bill_oflist = NULL;
	pin_flist_t		*nameinfo_installation = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*srvc_info = NULL; 
	pin_flist_t		*validate_out_flist = NULL; 
	pin_flist_t		*purch_prod_details = NULL; 
	pin_flist_t		*device_details = NULL;
	pin_flist_t		*bill_details = NULL;
	pin_flist_t		*err_ret_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;

	poid_t			*acnt_pdp = NULL;
	poid_t			*bill_pdp = NULL;
	poid_t			*bnfo_pdp = NULL;
	poid_t			*user_id = NULL;
	char			*acnt_no = NULL;
	char			*data_access = NULL;
	char			*access_str = NULL;
	char			*access_level = NULL;
	char			*access_value = NULL;
	char			*existing_access_value = NULL;
	char			msg[200];
	char			msg1[200];
	char			*service_type = NULL;
	int64			db = -1;
	int64			last_bill_obj_id0 = 0;
	int32			*flag = NULL;
	int32			check_data_protection = 1;
	int32			srch_customer_failure = 1;
	int32			srch_customer_success = 0;
	int32			account_type = 0;
	int32			svrc_type = 2;
	int32			serv_value = 2;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ar_get_details input flist", i_flistp);
	
	err_ret_flistp = PIN_FLIST_CREATE(ebufp);
	// user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 1, ebufp);
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
	svrc_type = *((int32*)PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp));
	flag = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp );
	db = PIN_POID_GET_DB(acnt_no);
	

	/*******************************************************************
	* Validate account_no passed for adjustment
	*******************************************************************/
	fm_mso_validate_acnt_no(ctxp, i_flistp, &validate_out_flist, ebufp);
	
	if (!validate_out_flist || (PIN_ERRBUF_IS_ERR(ebufp)))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53070", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
										
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Data incorrect error outflist flist", r_flistp);
		*r_flistpp=PIN_FLIST_COPY(r_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		//PIN_FLIST_DESTROY_EX(&validate_out_flist, NULL);
		goto CLEANUP;;	
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "validate_out_flist", validate_out_flist);
	PIN_FLIST_DESTROY_EX(&validate_out_flist, NULL);
	
	/*******************************************************************
	* Validate service_type passed for adjustment
	*******************************************************************/
	fm_mso_validate_service_type(ctxp, i_flistp, &validate_out_flist, ebufp);
	
	//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_validate_service_type before if ", ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_validate_service_type before if ");

	if ((!validate_out_flist) || (PIN_ERRBUF_IS_ERR(ebufp)))
	{
		//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Inside fm_mso_ar_get_details fm_mso_validate_service_type if  ", ebufp);

		PIN_ERRBUF_CLEAR(ebufp);
		r_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53070", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
							
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Data incorrect error outflist flist", r_flistp);
		*r_flistpp=PIN_FLIST_COPY(r_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		//PIN_FLIST_DESTROY_EX(&validate_out_flist, NULL);
		goto CLEANUP;
	}
	else 
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_ar_get_details fm_mso_validate_service_type else  ");
		service_type = (char*)PIN_FLIST_FLD_GET(validate_out_flist, PIN_FLD_BILLINFO_ID, 1, ebufp );

		if (service_type && (strcmp(service_type, "BB") ==0 || strcmp(service_type, "bb") ==0))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_ar_get_details fm_mso_validate_service_type strcmp BB ");
			serv_value= 1;
		}
		else if(service_type && (strcmp(service_type, "CATV") ==0 || strcmp(service_type, "catv") ==0 || strcmp(service_type, "Catv") ==0))
		{
			//printf("Service Type = %c", service_type);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_ar_get_details fm_mso_validate_service_type strcmp CATV ");
			serv_value= 0;
			
		}

		if (serv_value!=svrc_type)  // service not same as that of customer 
		{
			//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Inside fm_mso_ar_get_details fm_mso_validate_service_type strcmp serv_value ", ebufp);
			if ( PIN_ERRBUF_IS_ERR(ebufp) )
			{
				PIN_ERRBUF_CLEAR(ebufp);
			}
			r_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_DESCR, "Service Mismatch", ebufp);
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_ERROR_CODE, "53071", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, r_flistp, PIN_FLD_POID, ebufp);
								
			*r_flistpp=PIN_FLIST_COPY(r_flistp,ebufp);
			PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
			//PIN_FLIST_DESTROY_EX(&validate_out_flist, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Eror Flist is: ",*r_flistpp);
			goto CLEANUP;;	
		}
	}
	//printf("serv_value svrc_type = %d and %d", serv_value, svrc_type );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "validate_out_flist", validate_out_flist);
	bnfo_pdp = (char*)PIN_FLIST_FLD_GET(validate_out_flist, PIN_FLD_POID, 1, ebufp );
	
	
	if(*(flag)==3) //checking for account level adjustment - fm_mso_get_bill_details not required
	{
		goto ACCOUNT_FLAG;
	}
	
	// Get lastest bill obj
		
	fm_mso_get_bill_details(ctxp, i_flistp, &bill_oflist, ebufp ); // get the lastest bill object
	
	
	if (!bill_oflist || (PIN_ERRBUF_IS_ERR(ebufp)))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_CODE, "53072", ebufp);
		PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_DESCR, "Error in getting Last Bill 0bject doesnot exist", ebufp);
		PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
		*r_flistpp=PIN_FLIST_COPY(err_ret_flistp,ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
//		PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Eror Flist is: ",*r_flistpp);	
		goto CLEANUP;
	}

	
	bill_pdp = PIN_FLIST_FLD_GET(bill_oflist, PIN_FLD_POID, 0, ebufp);
	if (PIN_POID_GET_ID(bill_pdp) == 0)
	{
		PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_CODE, "53072", ebufp);
		PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_DESCR, "Error in getting Last Bill 0bject doesnot exist", ebufp);
		PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
                *r_flistpp=PIN_FLIST_COPY(err_ret_flistp,ebufp);
                PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
//              PIN_FLIST_DESTROY_EX(&i_flistp, NULL);
                PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Eror Flist is: ",*r_flistpp);

		goto CLEANUP;
	}
	
	ACCOUNT_FLAG:
	switch(*(flag))
	{ 

		case 0: 
			/* get bill details*/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside Switch Case 0");
			fm_mso_get_last_bill_details(ctxp, bill_pdp, &out_flist, ebufp );
			if (!out_flist || (PIN_ERRBUF_IS_ERR(ebufp)))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_CODE, "53073", ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_DESCR, "Last Bill Find - Error", ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
				PIN_FLIST_DESTROY_EX(&out_flist, NULL);
				*r_flistpp = PIN_FLIST_COPY(err_ret_flistp,ebufp);
				goto CLEANUP;
			}
			else
			{
				*r_flistpp= PIN_FLIST_COPY(out_flist,ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &srch_customer_success, ebufp);
				PIN_FLIST_DESTROY_EX(&out_flist, NULL);
				goto CLEANUP;
			}

			break; 

		case 1: 
			/* get item details*/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside Switch Case 1");
			fm_mso_get_last_item_details(ctxp, bill_pdp, &out_flist, ebufp );
			if (!out_flist || (PIN_ERRBUF_IS_ERR(ebufp)))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside Switch Case 1 if");
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_CODE, "53074", ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_DESCR, "Last Bill's Items Find - Error", ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
				PIN_FLIST_DESTROY_EX(&out_flist, NULL);
				*r_flistpp = PIN_FLIST_COPY(err_ret_flistp,ebufp);
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside Switch Case 1 else");
				*r_flistpp = PIN_FLIST_COPY(out_flist,ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &srch_customer_success, ebufp);
				PIN_FLIST_DESTROY_EX(&out_flist, NULL);
				
			}
			break; 

		case 2: 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside Switch Case 2");
			/* get event details*/
			fm_mso_get_last_event_details(ctxp, bill_pdp, &out_flist, ebufp );
			if (!out_flist || (PIN_ERRBUF_IS_ERR(ebufp)))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside Switch Case 2 if");
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_CODE, "53075", ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_DESCR, "Last Bill's Events Find - Error", ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
				PIN_FLIST_DESTROY_EX(&out_flist, NULL);
				*r_flistpp = PIN_FLIST_COPY(err_ret_flistp,ebufp);
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside Switch Case 2 else");
				*r_flistpp = PIN_FLIST_COPY(out_flist,ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &srch_customer_success, ebufp);
				PIN_FLIST_DESTROY_EX(&out_flist, NULL);
				
			}

			
			break;

		case 3: 
			/* get account and bal_grp_obj poid details*/
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Inside Switch Case 0");
			fm_mso_get_bal_grp_details(ctxp, acnt_pdp,bnfo_pdp,&out_flist, ebufp );
			if (!out_flist || (PIN_ERRBUF_IS_ERR(ebufp)))
			{
				PIN_ERRBUF_CLEAR(ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_CODE, "53076", ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_ERROR_DESCR, "Balance Group Find - Error", ebufp);
				PIN_FLIST_FLD_SET(err_ret_flistp, PIN_FLD_STATUS, &srch_customer_failure, ebufp);
				PIN_FLIST_DESTROY_EX(&out_flist, NULL);
				*r_flistpp = PIN_FLIST_COPY(err_ret_flistp,ebufp);
				goto CLEANUP;
			}
			else
			{
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"out_flist is:",out_flist);
				*r_flistpp= PIN_FLIST_COPY(out_flist,ebufp);
				PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &srch_customer_success, ebufp);
				PIN_FLIST_DESTROY_EX(&out_flist, NULL);
				goto CLEANUP;
			}

			break;

 
		default: 
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Improper Flag value");
			
			return;

	} 	/* end switch */

	
CLEANUP:
	PIN_FLIST_DESTROY_EX(&err_ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&validate_out_flist, NULL);
	PIN_FLIST_DESTROY_EX(&bill_oflist,  NULL);
	PIN_FLIST_DESTROY_EX(&out_flist,  NULL);
	return;
}
	



/**************************************************
* Function: 	fm_mso_validate_acnt_no()
* Owner:        Asish Joshi
* Decription:   
* 
***************************************************/
static void
fm_mso_validate_acnt_no(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;

	int64			db = -1;

	char			*template = "select X from /account where F1 = V1 " ;


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_acnt_no", ebufp);
		return;
	}
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, acnt_pdp, ebufp);
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	//PIN_FLIST_ELEM_SET(srch_flistp, NULL, PIN_FLD_RESULTS, 0, ebufp );

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_acnt_no read input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_acnt_no READ output list", srch_oflistp);
	
	result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
	
	if (result_flist )
	{
		*ret_flistp = result_flist;
	}
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_validate_service_type()
* Owner:        Asish Joshi
* Decription:   
* 
***************************************************/

static void
fm_mso_validate_service_type(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*result_flist2 = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	char			*Serv_type = NULL;
	int32			*service_type = NULL;
	int32			srch_flag = 512;	
	int64			db = -1;
	char			*template = "select X from /billinfo where F1 = V1 and F2 = V2 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_validate_service_type", ebufp);
		return;
	}
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
	service_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp );
	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	if(*service_type == 1)
	{
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_ID, "BB", ebufp);
	}
	else
	{
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_ID, "CATV", ebufp);
	}
	
	
	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILLINFO_ID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_service_type read input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ Billinfo for service_type", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_service_type READ output list", srch_oflistp);
	
	result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
	
	if (result_flist )
	{
		Serv_type = (char*)PIN_FLIST_FLD_GET(result_flist, PIN_FLD_BILLINFO_ID, 0, ebufp);
		
		if (strcmp(Serv_type, "CATV") || strcmp(Serv_type, "BB"))
		{
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_service_type read input list", result_flist);
			*ret_flistp = result_flist;
			//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG, "Inside fm_mso_validate_service_type Serv_type match if ", ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_validate_service_type read input list", *ret_flistp);
			
			goto CLEANUP;
		}
	}
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	return;
}

/**************************************************
* Function: 	fm_mso_get_bill_details()
* Owner:        Asish Joshi
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_bill_details(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*arg2_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*result2_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*srch2_oflistp = NULL;
	pin_flist_t		*srch2_flistp = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*read_bill_iflist = NULL;
	pin_flist_t		*read_billinfo_oflist = NULL;
	pin_flist_t		*temp_flist = NULL;
	poid_t			*acnt_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int32			link_direction = 1;
        int32                   process_failure = 1;
        int32                   process_success = 0;

	int64			db = -1;
	char			*template1 = "select X from /billinfo where F1 = V1 " ;
	char			*template2 = "select X from /bill where F1 = V1 ";
	char			*bill_no = NULL;
	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_bill_details", ebufp);
		return;
	}

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
	bill_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILL_NO, 1, ebufp );

	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

	if(bill_no && (strcmp(bill_no, "") != 0))
	{
		
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2 , ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILL_NO, bill_no, ebufp);
		result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILL_NO, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DUE, NULL, ebufp);
		
				
	}
	else
	{
		
		srch2_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_PUT(srch2_flistp, PIN_FLD_POID, srch_pdp, ebufp);
		PIN_FLIST_FLD_SET(srch2_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
		
		//get the latest bill object
		PIN_FLIST_FLD_SET(srch2_flistp, PIN_FLD_TEMPLATE, template1 , ebufp);
		arg2_flist = PIN_FLIST_ELEM_ADD(srch2_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg2_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);
		result2_flist = PIN_FLIST_ELEM_ADD(srch2_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(result2_flist, PIN_FLD_ACTUAL_LAST_BILL_OBJ, NULL, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details search billinfo details input list", srch2_flistp);
		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch2_flistp, &srch2_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details search billinfo details output list", srch2_oflistp);
		PIN_FLIST_DESTROY_EX(&srch2_flistp, NULL);
		


		// get the latest bill no
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template2 , ebufp);
		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		temp_flist = PIN_FLIST_ELEM_TAKE(srch2_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
		PIN_FLIST_FLD_COPY(temp_flist, PIN_FLD_ACTUAL_LAST_BILL_OBJ, arg_flist, PIN_FLD_POID, ebufp);
		result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILL_NO, NULL, ebufp);
		PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DUE, NULL, ebufp);
		
	}



	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details search bill details input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ BILLINFO", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bill_details search billinfo output list", srch_oflistp);
	 	
//	*ret_flistp = PIN_FLIST_CREATE(ebufp);
	result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
	if(result_flist)
	{
		*ret_flistp = PIN_FLIST_COPY(result_flist,ebufp);
	}
	else
	{
                pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERR_NOT_FOUND, PIN_FLD_BILL_NO, 0, 0, MSO_OP_AR_GET_DETAILS);
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,   "No bill Found for the account",  ebufp);
	}

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_flistp,  NULL);
		PIN_FLIST_DESTROY_EX(&result_flist, NULL);
	return;
}




/** New structured code for fm_mso_get_last_bill_details, fm_mso_get_last_item_details, fm_mso_get_last_event_details **/

/**************************************************
* Function: 	fm_mso_get_last_bill_details()
* Owner:        Asish Joshi
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_last_bill_details(
	pcm_context_t		*ctxp,
	poid_t		*bill_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*read_bill_iflist = NULL;
	pin_flist_t		*read_bill_oflist = NULL;
	pin_flist_t		*result_flistp = NULL;
	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int32			link_direction = 1;
	int32			process_failure = 1;
	int32			process_success = 0;
	int64			db = -1;
	int64			last_bill_obj_id0 = 0;
	char			*template = "select X from /bill where F1 = V1 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_last_bill_details", ebufp);
		return;
	}
	
		
	db = PIN_POID_GET_DB(bill_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID,bill_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ADJUSTED, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILL_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DUE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DUE_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_RECVD, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_TOTAL_DUE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CURRENT_TOTAL, NULL, ebufp);
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_last_bill_details search bill input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ BILL object", ebufp);
		result_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &process_failure, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Process Failed", ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53079", ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID, bill_pdp, ebufp);
															
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Unknown error- outflist flist", result_flistp);
		*ret_flistp=PIN_FLIST_COPY(result_flistp, ebufp);

		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_last_bill_details search bill output list", srch_oflistp);


 	
	//*ret_flistp = PIN_FLIST_CREATE(ebufp);
	result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
	if (result_flist)
	{
		*ret_flistp = PIN_FLIST_COPY(result_flist,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_last_bill_details search bill output list", result_flist);
	}		
	
	

	CLEANUP:

		PIN_FLIST_DESTROY_EX(&result_flist, NULL);
		PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	return;
}


/**************************************************
* Function: 	fm_mso_get_last_item_details()
* Owner:        Asish Joshi
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_last_item_details(
	pcm_context_t		*ctxp,
	poid_t		*bill_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*read_bill_iflist = NULL;
	pin_flist_t		*read_bill_oflist = NULL;
	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int32			link_direction = 1;
	int64			db = -1;
	int64			last_bill_obj_id0 = 0;
	char			*template = "select X from /item where F1 = V1 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_last_item_details", ebufp);
		return;
	}
	//PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_last_item_details acnt_pdp", acnt_pdp);

	
	//last_bill_obj_id0 = PIN_POID_GET_ID((PIN_FLIST_FLD_GET(result_flist, PIN_FLD_ACTUAL_LAST_BILL_OBJ, 0, ebufp)));
	
	//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Not_Error- Inside fm_mso_get_last_item_details", ebufp);

	db = PIN_POID_GET_DB(bill_pdp);
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILL_OBJ, bill_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DUE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILL_OBJ, NULL, ebufp);
	

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_last_item_details search bill input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);


	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ Item object", ebufp);
		goto CLEANUP;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_last_item_details search bill output list", srch_oflistp);

	
	*ret_flistp = PIN_FLIST_COPY(srch_oflistp,ebufp);
	//PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "End of fm_mso_get_last_item_details ", ebufp);
	
	CLEANUP:
		
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		

	return;
}

/**************************************************
* Function: 	fm_mso_get_last_event_details()
* Owner:        Asish Joshi
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_last_event_details(
	pcm_context_t		*ctxp,
	poid_t			*bill_pdp,
	pin_flist_t		**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*items_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*read_bill_oflist = NULL;
	pin_flist_t		*res_array_flistp = NULL;
	pin_flist_t		*i_flistp2 = NULL;
	pin_flist_t		*i_flistp = NULL;
	pin_flist_t		*s_oflistp2 = NULL;
	pin_flist_t		*res2_array_flistp = NULL;
	pin_flist_t		*res3_array_flistp = NULL;
	pin_flist_t		*event_flist = NULL;
	pin_flist_t		*adj_flist = NULL;
	pin_flist_t		*item_obj_flistp = NULL;
	pin_flist_t		*re_flistp = NULL;
	pin_cookie_t	cookie= NULL;
	pin_cookie_t	cookie1= NULL;
	pin_cookie_t	cookie2= NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*item_pdp = NULL;
	int32			srch_flag = 512;
	int32			link_direction = 1;
	int32			RESOURCE_ID =356;
	int32           elemid = 0;
	int32           elemid1 = 0;
	int32           elemid2 = 0;
	int32			elem = 0;
	int64			db = -1;
	int64			last_bill_obj_id0 = 0;
	char			*template = "select X from /event where F1 = V1 " ; 

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_last_event_details", ebufp);
		return;
	}
	
	
	fm_mso_get_last_item_details(ctxp, bill_pdp, &items_flistp, ebufp );

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ BILL object", ebufp);
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BILL_GET_ITEM_EVENT_CHARGE_DISCOUNT-fm_mso_get_last_item_details output list", items_flistp);
	//res_array_flistp = PIN_FLIST_CREATE(ebufp);
	re_flistp = PIN_FLIST_CREATE(ebufp);

	while (( res_array_flistp = PIN_FLIST_ELEM_GET_NEXT(items_flistp, PIN_FLD_RESULTS, &elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) 
	{
		 PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside while res_array_flistp ");
				
		 item_pdp = PIN_FLIST_FLD_GET(res_array_flistp, PIN_FLD_POID, 0, ebufp);

		// Setting input flist for PCM_OP_BILL_GET_ITEM_EVENT_CHARGE_DISCOUNT
		i_flistp2 = PIN_FLIST_CREATE(ebufp);

		PIN_FLIST_FLD_COPY(res_array_flistp, PIN_FLD_ACCOUNT_OBJ, i_flistp2, PIN_FLD_POID, ebufp);
						
		item_obj_flistp= PIN_FLIST_ELEM_ADD(i_flistp2, PIN_FLD_ITEMS, 1, ebufp);
		PIN_FLIST_FLD_COPY(res_array_flistp, PIN_FLD_POID, item_obj_flistp, PIN_FLD_ITEM_OBJ, ebufp);
									
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BILL_GET_ITEM_EVENT_CHARGE_DISCOUNT input list", i_flistp2);
		PCM_OP(ctxp, PCM_OP_BILL_GET_ITEM_EVENT_CHARGE_DISCOUNT, 0, i_flistp2, &s_oflistp2, ebufp);

		//Filtering and appending the output flist as per CSR display and input for PCM_OP_AR_EVENT_ADJUSTMENT 
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BILL_GET_ITEM_EVENT_CHARGE_DISCOUNT output list", s_oflistp2);

		PIN_FLIST_FLD_COPY(s_oflistp2, PIN_FLD_POID, re_flistp, PIN_FLD_POID, ebufp); //account_poid added

		adj_flist = PIN_FLIST_ELEM_ADD(re_flistp, PIN_FLD_ADJUSTMENT_INFO, elemid, ebufp );
		PIN_FLIST_FLD_SET(adj_flist, PIN_FLD_RESOURCE_ID, &RESOURCE_ID, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BILL_GET_ITEM_EVENT_CHARGE_DISCOUNT return list", re_flistp);
				
		elemid1 = 0;
		cookie1 = NULL;

		while(( res2_array_flistp = PIN_FLIST_ELEM_GET_NEXT(s_oflistp2, PIN_FLD_RESULTS, &elemid1, 1, &cookie1, ebufp)) != (pin_flist_t *)NULL)
		{
			elemid2 = 0;
			cookie2 = NULL;
			while (( res3_array_flistp = PIN_FLIST_ELEM_GET_NEXT(res2_array_flistp, PIN_FLD_EVENTS, &elemid2, 1, &cookie2, ebufp)) != (pin_flist_t *)NULL)
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside second  while res3_array_flistp ");
				event_flist = PIN_FLIST_ELEM_ADD(re_flistp, PIN_FLD_EVENTS, elem++, ebufp );
				PIN_FLIST_FLD_COPY(res3_array_flistp, PIN_FLD_EVENT_OBJ,event_flist, PIN_FLD_POID, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_BILL_GET_ITEM_EVENT_CHARGE_DISCOUNT return list Inside second  while", re_flistp);

			}
		}					
	}
	
	if (re_flistp)
	{
		*ret_flistp = PIN_FLIST_COPY(re_flistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_last_event_details search bill output list", re_flistp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"End of fm_mso_get_last_event_details ");
	}

	CLEANUP:
	
	PIN_FLIST_DESTROY_EX(&s_oflistp2, NULL);
	PIN_FLIST_DESTROY_EX(&i_flistp2, NULL);
	PIN_FLIST_DESTROY_EX(&items_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&re_flistp, NULL);
	
	return;
		
}


/**************************************************
* Function: 	fm_mso_get_bal_grp_details()
* Owner:        Asish Joshi
* Decription:   
*               
*		
* 
* 
***************************************************/
static void
fm_mso_get_bal_grp_details(
	pcm_context_t		*ctxp,
	poid_t				*acnt_pdp,
	poid_t				*bnfo_pdp,
	pin_flist_t			**ret_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*services = NULL;
	pin_flist_t		*read_bill_iflist = NULL;
	pin_flist_t		*read_bill_oflist = NULL;
	pin_flist_t		*result_flistp = NULL;
	poid_t			*srch_pdp = NULL;
	int32			srch_flag = 512;
	int32			process_failure = 1;
	int32			process_success = 0;
	int64			db = -1;
	int64			last_bill_obj_id0 = 0;
	char			*template = "select X from /balance_group where F1 = V1 and F2 = V2 " ;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling fm_mso_get_bal_grp_details", ebufp);
		return;
	}
		
	db = PIN_POID_GET_DB(acnt_pdp) ;
	srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ,acnt_pdp, ebufp);

	arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_OBJ,bnfo_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_NAME, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);
		
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bal_grp_details search bal_grp input list", srch_flistp);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		IN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error before calling READ Bal_grp", ebufp);
	}

	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_oflistp, ebufp);	

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling READ Bal_grp", ebufp);
		result_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &process_failure, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Process Failed", ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53079", ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_POID,acnt_pdp, ebufp);
															
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Unknown error- outflist flist", result_flistp);
		*ret_flistp=PIN_FLIST_COPY(result_flistp, ebufp);

		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bal_grp_details search bal_grp output list", srch_oflistp);
	
	*ret_flistp = PIN_FLIST_COPY(srch_oflistp,ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "End of fm_mso_get_bal_grp_details ");

 	
	/**
	result_flist = PIN_FLIST_ELEM_TAKE(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 0, ebufp );
	if (result_flist)
	{
		*ret_flistp = PIN_FLIST_COPY(result_flist,ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_bal_grp_details search bal_grp output list", result_flist);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "End of fm_mso_get_bal_grp_details ", ebufp);
	}	**/	
	

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

	return;
}
