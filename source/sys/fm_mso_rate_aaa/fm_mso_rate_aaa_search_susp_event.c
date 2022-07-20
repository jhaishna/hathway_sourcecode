/*******************************************************************
 * File:	fm_mso_rate_aaa_search_susp_event.c
 * Opcode:	MSO_OP_RATE_AAA_SEARCH_SUSP_EVENT 
 * Owner:	
 * Created:	
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * Input Flist
 *
1. Search Based on Session ID
        0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
        0 PIN_FLD_FLAGS               INT [0] 0
        0 PIN_FLD_ AUTHORIZATION_ID                STR [0] “SESSION-1”
2. Search by account number
        0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
        0 PIN_FLD_FLAGS               INT [0] 1
        0 PIN_FLD_ACCOUNT_NO          STR [0] "58006"
3. Search by Acct status Type
        0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
        0 PIN_FLD_FLAGS               INT [0] 2
        0 PIN_FLD_REQ_MODE          ENUM  [0] "2"
4. Search by User Name
        0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
        0 PIN_FLD_FLAGS               INT [0] 3
        0 PIN_FLD_UER_NAME         STR  [0] "SCOTT"
5. Search by Error Code
        0 PIN_FLD_POID               POID [0] 0.0.0.1 /search -1 0
        0 PIN_FLD_FLAGS               INT [0] 4
        0 PIN_FLD_REASON          INT  [0] "2"

********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_rate_aaa_search_susp_event.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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

#define SEARCH_BY_SESSION_ID  		0
#define SEARCH_BY_ACCT_NO     		1
#define SEARCH_BY_ACCT_STATUS_TYPE 	2
#define SEARCH_BY_USER_NAME  		3
#define SEARCH_BY_ERROR_CODE		4

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
op_mso_rate_aaa_search_susp_event(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_rate_aaa_search_susp_event(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_CUST_SEARCH_BILL  
 *******************************************************************/
void 
op_mso_rate_aaa_search_susp_event(
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
	if (opcode != MSO_OP_RATE_AAA_SEARCH_SUSP_EVENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_rate_aaa_search_susp_event error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_rate_aaa_search_susp_event input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_rate_aaa_search_susp_event(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_rate_aaa_search_susp_event error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_rate_aaa_search_susp_event output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_rate_aaa_search_susp_event(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*srch_out_flistp = NULL;
	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*result_flist = NULL;

	void			*srch_pdp = NULL;
	poid_t			*svc_pdp = NULL;

	int32			*flag_ptr = NULL;
	int32			flag = -1;
	int32			search_fail = 1;
	int32			srch_flag = 256;
	int32			elem_count = 0;
	int32			count;

	int32			*no_of_bills = NULL;

	int64			db = 1;

	time_t			*start_t = NULL;
	time_t			*end_t = NULL;
	time_t			zero_time =0;
	
	char			*acnt_no = NULL;
	char			*template = "select X from /mso_aaa_suspended_bb_event where F1 = V1 " ;
	char			*datetemplate = "select X from /mso_aaa_suspended_bb_event where F1 = V1 and F2 >= V2 and F3 <= V3 " ;

	void			*vp = NULL;
	pin_flist_t		*args_flistp = NULL;

	
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_rate_aaa_search_susp_event input flist", i_flistp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling fm_mso_rate_aaa_search_susp_event", ebufp);
		goto CLEANUP;
	}

	flag_ptr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp ); 
	svc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );

	/*******************************************************************
	* Mandatory fields validation
	*******************************************************************/
	/* Move these pre-proc values appropriately if new ones are added. */
	if (flag_ptr && (*flag_ptr <SEARCH_BY_SESSION_ID || *flag_ptr > SEARCH_BY_ERROR_CODE) )
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Search Bill: Invalid Flag", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_POID, svc_pdp, ebufp );
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &search_fail, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51200", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Search Suspense Event: Invalid Flag Specified", ebufp);
		goto CLEANUP;
	}
	flag = *flag_ptr;
	srch_flistp = PIN_FLIST_CREATE(ebufp);
        srch_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
    	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);	
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);
		
	switch(flag)
        {
		case SEARCH_BY_SESSION_ID:
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AUTHORIZATION_ID, arg_flist, PIN_FLD_AUTHORIZATION_ID, ebufp);
			break;
		case SEARCH_BY_ACCT_NO:
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, arg_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);
			break;
		case SEARCH_BY_ACCT_STATUS_TYPE:
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REQ_MODE, arg_flist, PIN_FLD_REQ_MODE, ebufp);
			break;
		case SEARCH_BY_USER_NAME:
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USER_NAME, arg_flist, PIN_FLD_USER_NAME, ebufp);
			break;
		case SEARCH_BY_ERROR_CODE:
			arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ERROR_CODE, arg_flist, PIN_FLD_ERROR_CODE, ebufp);
			break;		
		}   
	
	// Changed on 6-Oct-2015 
	// If start date is present then end date is mandatory and we search the suspense events
	// based on created_t >= start_t and created_t <= end_t

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 1, ebufp );
	if (vp)
	{
                args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp,PIN_FLD_ARGS, 2, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREATED_T,(void *)vp, ebufp);

		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp );

        	if (PIN_ERRBUF_IS_ERR(ebufp))
        	{
                	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "End Date not available in Suspense Search ", ebufp);
                	goto CLEANUP;
        	}
                args_flistp = PIN_FLIST_ELEM_ADD(srch_flistp,PIN_FLD_ARGS, 3, ebufp);
                PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_CREATED_T,(void *)vp, ebufp);

        	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, datetemplate , ebufp);
        }

	result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(result_flist, PIN_FLD_POID, NULL, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_login input flist", srch_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_login output flist", srch_out_flistp);


        CLEANUP:
        *r_flistp = srch_out_flistp;
        return;
}


/*void fm_mso_rate_aaa_srch_by_sess_id(			
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
		pin_flist_t             *srch_flistp = NULL;
        pin_flist_t             *arg_flist = NULL;
        pin_flist_t             *result_flist = NULL;
        pin_flist_t             *srch_out_flistp = NULL;
		poid_t                  *mso_pdp = NULL;
		int32                   srch_flag = 256;
        char                    *template = "select X from /mso_aaa_suspended_bb_event where F1 = V1 " ;
        int64                   db = -1;
        if (PIN_ERRBUF_IS_ERR(ebufp))
                return;
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_login input flist", in_flistp);
		
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_POID, srch_flistp, PIN_FLD_POID, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flag, ebufp);
        PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, template , ebufp);

        arg_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp );
        PIN_FLIST_FLD_COPY(in_flistp, PIN_FLD_ACCOUNT_OBJ, arg_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);
		result_flist = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp );

		PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_out_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling SEARCH", ebufp);
                goto CLEANUP;
        }
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_srch_login output flist", srch_out_flistp);


        CLEANUP:
        *ret_flistp = srch_out_flistp;
        return;
}

void fm_mso_rate_aaa_srch_by_acct_no(			
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
}

void fm_mso_rate_aaa_srch_by_acct_status_type(			
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
}

void fm_mso_rate_aaa_srch_by_user_name(			
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
}

void fm_mso_rate_aaa_srch_by_nas_id(			
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
}

void fm_mso_rate_aaa_srch_by_rating_status(			
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
}

void fm_mso_rate_aaa_srch_by_reason(			
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             **ret_flistp,
        pin_errbuf_t            *ebufp)
{
}*/

