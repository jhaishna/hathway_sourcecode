/*******************************************************************
 * File:	fm_mso_cust_modify_data_access.c
 * Opcode:	MSO_OP_CUST_MODIFY_DATA_ACCESS 
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
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_modify_data_access.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
 * MSO_OP_CUST_MODIFY_DATA_ACCESS 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_cust_modify_data_access(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_modify_data_access(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_MODIFY_DATA_ACCESS  
 *******************************************************************/
void 
op_mso_cust_modify_data_access(
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
	if (opcode != MSO_OP_CUST_MODIFY_DATA_ACCESS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_modify_data_access error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_modify_data_access input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_modify_data_access(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_modify_data_access error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_modify_data_access output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_modify_data_access(
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
	pin_flist_t		*update_flistp = NULL;
	pin_flist_t		*update_flist_out = NULL;

	poid_t			*acnt_pdp = NULL;
	poid_t			*cust_cred_pdp = NULL;

	int32			srch_flag = 256;
	int32			modify_success =0;

	int64			db = -1;

	char			*data_access_str = NULL;
	char			*template = "select X from /mso_customer_credential where F1.id = V1 " ;

	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_modify_data_access input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_cust_modify_data_access", ebufp);
		goto CLEANUP;
	}

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp ); 
	db = PIN_POID_GET_DB(acnt_pdp);
	data_access_str  = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_DATA_ACCESS, 0, ebufp );

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_POID, PIN_POID_CREATE(db, "/search", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template, ebufp);

	arg_flist =  PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
	PIN_FLIST_FLD_SET(arg_flist, PIN_FLD_ACCOUNT_OBJ, acnt_pdp, ebufp);

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search input list", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search output flist", srch_out_flistp);
	result_flist = PIN_FLIST_ELEM_GET(srch_out_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
	if (result_flist)
	{
		update_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(result_flist, PIN_FLD_POID, update_flistp, PIN_FLD_POID, ebufp );
		PIN_FLIST_FLD_SET(update_flistp, MSO_FLD_DATA_ACCESS, data_access_str, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_ERROR, "update_flistp", update_flistp);
		PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 0, update_flistp, &update_flist_out, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
			goto CLEANUP;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "search output flist", update_flist_out);
		PIN_FLIST_DESTROY_EX(&update_flist_out, NULL);

		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, ret_flistp, PIN_FLD_USERID, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &modify_success, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, MSO_FLD_DATA_ACCESS, data_access_str, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "Data Access level/value modified successfully", ebufp);

	}
	
	CLEANUP:
	*r_flistp = PIN_FLIST_COPY(ret_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&srch_out_flistp, NULL);
	return;
}

