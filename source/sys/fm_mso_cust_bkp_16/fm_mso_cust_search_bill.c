/*******************************************************************
 * File:	fm_mso_cust_search_bill.c
 * Opcode:	MSO_OP_CUST_SEARCH_BILL 
 * Owner:	Gautam Adak
 * Created:	17-SEPT-2013
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 *
1. Search Login Availability 
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 0
	0 PIN_FLD_LOGIN               STR [0] "self_care_1"
2. Search by account number
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 1
	0 PIN_FLD_ACCOUNT_NO          STR [0] "58006"
3. Search by RMN
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 2
	0 MSO_FLD_RMN                 STR [0] "9983882151"
4.Search by FIRSTNAME Only
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 3
	0 PIN_FLD_FIRST_NAME          STR [0] "%GAU%"
5.Search by MIDDLENAME only
	0 PIN_FLD_POID              POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS              INT [0] 3
	0 PIN_FLD_MIDDLE_NAME        STR [0] "%TE%"
6. Search by LASTNAME only
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 3
	0 PIN_FLD_LAST_NAME           STR [0] "%ADAK%"
7.Search by FIRSTNAME and LASTNAME
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 3
	0 PIN_FLD_FIRST_NAME          STR [0] "%GAU%"
	0 PIN_FLD_LAST_NAME           STR [0] "%AD%"
8.Search by FIRSTTNAME and MIDDLENAME
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 3
	0 PIN_FLD_FIRST_NAME          STR [0] "%GAU%"
	0 PIN_FLD_MIDDLE_NAME         STR [0] "%KUM%"
9. Search by FIRSTNAME ,MIDDLENAME and LASTNAME
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 3
	0 PIN_FLD_FIRST_NAME          STR [0] "%GAU%"
	0 PIN_FLD_MIDDLE_NAME         STR [0] "%KUM%"
	0 PIN_FLD_LAST_NAME           STR [0] "%KUM%"
10.Search by Company
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 5
	0 PIN_FLD_COMPANY          STR [0] "%ORA%"
11.Search by Card Number
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 6
	0 PIN_FLD_NAME                STR [0] "180920130030"
12. Transaction History
	0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
	0 PIN_FLD_FLAGS               INT [0] 4
	0 PIN_FLD_ACCOUNT_OBJ        POID [0] 0.0.0.1 /account 307101 0
	0 PIN_FLD_START_T           TSTAMP [0] (1409300201)
	0 PIN_FLD_END_T             TSTAMP [0] (1409380201)
********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_search_bill.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_ops_flds.h"

#define SEARCH_LAST_BILLS  0
#define SEARCH_BY_DATE     1


/**************************************
* External Functions start
***************************************/

/**************************************
* External Functions end
***************************************/

/*******************************************************************
 * MSO_OP_CUST_SEARCH_BILL 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_search_bill(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_search_bill(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_SEARCH_BILL  
 *******************************************************************/
void 
op_mso_cust_search_bill(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	poid_t			*inp_pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_SEARCH_BILL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_search_bill error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_search_bill input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_search_bill(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_search_bill error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_search_bill output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_search_bill(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;

	poid_t			*srch_pdp = NULL;
	poid_t			*acnt_pdp = NULL;

	int32			*flag_ptr = NULL;
	int32			flag = -1;
	int32			search_fail = 1;
	int32			srch_flag = 256;
	int32			elem_count = 0;
	int32			count;
	int32			*service_type = NULL;

	int32			*no_of_bills = NULL;

	int64			db = -1;

	time_t			*start_t = NULL;
	time_t			*end_t = NULL;
	time_t			zero_time =0;
	char			billinfo_id[20];
	
	char			*acnt_no = NULL;
	char			*template_1 = "select X from /bill 1, /account 2, /billinfo 3 where 2.F1 = V1 and 1.F2 = 2.F3 and 2.F4 = 3.F5 and 3.F6 = V6 and 1.F7 > V7 order by end_t desc " ;
	char			*template_2 = "select X from /bill 1, /account 2, /billinfo 3 where 2.F1 = V1 and 1.F2 = 2.F3 and 2.F4 = 3.F5 and 3.F6 = V6 and 1.F7 >= V7 and 1.F8 <= V8 order by end_t desc " ;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_search_bill input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_search_bill", ebufp);
		goto CLEANUP;
	}

	flag_ptr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 1, ebufp ); 
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ); 
	acnt_no  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp );
	no_of_bills    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_COUNT, 1, ebufp );
	start_t  = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp );
	end_t    = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp );
	service_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 1, ebufp );
	memset(billinfo_id,'\0',20);
	if((service_type) && (*service_type == 1))
	{
		strcpy(billinfo_id,"BB");
	}
	else
	{
		strcpy(billinfo_id,"CATV");

	}

	db = PIN_POID_GET_DB(acnt_pdp);

	/*******************************************************************
	* Mandatory fields validation
	*******************************************************************/
	if (flag_ptr && (*flag_ptr <0 || *flag_ptr >1) )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Search Bill: Invalid Flag", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51200", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Search Bill: Invalid Flag", ebufp);
		goto CLEANUP;
	}
	flag = *flag_ptr;
	if ( flag == SEARCH_LAST_BILLS && (!no_of_bills))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Pass the no. of bills to be searched", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51201", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Pass the no. of bills to be searched", ebufp);
		goto CLEANUP;
	}
	if ( flag == SEARCH_BY_DATE && (!start_t || !end_t ))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Start/End date both mandatory for search by date", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51202", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Start/End date both mandatory for search by date", ebufp);
		goto CLEANUP;
	}

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);

	if (flag == SEARCH_LAST_BILLS )
	{
//"select X from /bill 1, /account 2, /billinfo 3 where 2.F1 = V1 and 1.F2 = 2.F3 and 2.F4 = 3.F5 and 3.F6 = V6 and 1.F7 > V7 order by end_t desc " ;
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_1 , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, acnt_no, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_ID, billinfo_id, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 7, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_END_T, &zero_time, ebufp);
	}

	if (flag == SEARCH_BY_DATE )
	{
//select X from /bill 1, /account 2 /billinfo 3 where 2.F1 = V1 and 1.F2 = 2.F3 and 2.F4 = 3.F5 and 3.F6 = V6 and 1.F7 >= V7 and 1.F8 <=V8 order by end_t desc
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template_2 , ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_NO, acnt_no, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 3, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 4, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_POID, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 5, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 6, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_BILLINFO_ID, billinfo_id, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 7, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_END_T, start_t, ebufp);

		arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 8, ebufp );
		PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_END_T, end_t, ebufp);
	}

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_END_T, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_BILL_NO, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_DUE, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_CURRENT_TOTAL, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_INVOICE_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search output flist", srch_out_flistp);
	if (srch_out_flistp)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Test_001");
		elem_count = PIN_FLIST_ELEM_COUNT(srch_out_flistp, PIN_FLD_RESULTS, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Test_001");
		if ( no_of_bills &&(elem_count > *no_of_bills))
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Test_001");
			for (count=*no_of_bills; count <elem_count ; count++ )
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Test_001");
				PIN_FLIST_ELEM_DROP(srch_out_flistp, PIN_FLD_RESULTS, count, ebufp );
			}
		}
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Test_001");
	PIN_FLIST_FLD_RENAME(srch_out_flistp, PIN_FLD_RESULTS, PIN_FLD_BILLS, ebufp );
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "After Modification", srch_out_flistp);
	ret_flistp = PIN_FLIST_COPY(srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	
	CLEANUP:
	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return;
}

