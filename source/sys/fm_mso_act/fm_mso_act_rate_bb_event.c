/*******************************************************************
 * File:	fm_mso_act_rate_bb_event.c
 * Opcode:	MSO_OP_ACT_RATE_BB_EVENT 
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
	0 PIN_FLD_USERID          POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_POID            POID [0] 0.0.0.1 /account 88888
	0 PIN_FLD_ACCOUNT_NO       STR [0] "988766"
	0 PIN_FLD_LOGIN            STR [0] ""
	0 PIN_FLD_PASSWD_CLEAR     STR [0] ""
********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_act_rate_bb_event.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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

#define ACCT_TYPE_CSR_ADMIN         1
#define ACCT_TYPE_CSR               2
#define ACCT_TYPE_LCO               3
#define ACCT_TYPE_JV                4
#define ACCT_TYPE_DTR               5
#define ACCT_TYPE_SUB_DTR           6
#define ACCT_TYPE_OPERATION         7
#define ACCT_TYPE_FIELD_SERVICE     8
#define ACCT_TYPE_LOGISTICS         9
#define ACCT_TYPE_COLLECTION_AGENT 10
#define ACCT_TYPE_SALES_PERSON     11
#define ACCT_TYPE_HTW              13
#define ACCT_TYPE_SUBSCRIBER       99



/*******************************************************************
 * MSO_OP_ACT_RATE_BB_EVENT 
 *
 * 
 *******************************************************************/

EXPORT_OP void
op_mso_act_rate_bb_event(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_act_rate_bb_event(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_ACT_RATE_BB_EVENT  
 *******************************************************************/
void 
op_mso_act_rate_bb_event(
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
	if (opcode != MSO_OP_ACT_RATE_BB_EVENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_act_rate_bb_event error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_act_rate_bb_event input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_act_rate_bb_event(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_act_rate_bb_event error", ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_act_rate_bb_event output flist", *r_flistpp);
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_act_rate_bb_event(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*input_flist = NULL;
	pin_flist_t		*broadband_info = NULL;
	pin_flist_t		*event_substr = NULL;
	
	poid_t			*acnt_pdp = NULL;
	poid_t			*serv_pdp = NULL;
	poid_t			*user_pdp = NULL;
	poid_t			*event_pdp = NULL;
		
	time_t			*evt_start_t =0;
	time_t			*evt_end_t =0;

	int32			usage_category = 0;
	
	char			*event_type_broadband= "/event/broadband/usage";

	int64		db = -1;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_act_rate_bb_event error",ebufp);
		return;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_act_rate_bb_event input Flist", i_flistp);

	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	db = PIN_POID_GET_DB(acnt_pdp);

	serv_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 0, ebufp);
	evt_start_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_START_T, 0, ebufp);
	evt_end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);
	

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_act_rate_bb_event input error",ebufp);
		return;
	}
	input_flist = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(input_flist, PIN_FLD_POID, (void *)acnt_pdp, ebufp);
	event_substr = PIN_FLIST_SUBSTR_ADD(input_flist, PIN_FLD_EVENT, ebufp);

	event_pdp = PIN_POID_CREATE(db, event_type_broadband, (int64)-1, ebufp);
	PIN_FLIST_FLD_SET(event_substr, PIN_FLD_POID, (void *)event_pdp, ebufp);
	PIN_FLIST_FLD_SET(event_substr, PIN_FLD_ACCOUNT_OBJ, (void *)acnt_pdp, ebufp);
	PIN_FLIST_FLD_SET(event_substr, PIN_FLD_SERVICE_OBJ, (void *)serv_pdp, ebufp);
	PIN_FLIST_FLD_SET(event_substr, PIN_FLD_START_T, (void *)evt_start_t, ebufp);
	PIN_FLIST_FLD_SET(event_substr, PIN_FLD_END_T, (void *)evt_end_t, ebufp);
	PIN_FLIST_FLD_SET(event_substr, PIN_FLD_NAME, (void *)"Activity Session Log", ebufp);
	PIN_FLIST_FLD_SET(event_substr, PIN_FLD_PROGRAM_NAME, (void *)"Universal Event Loader", ebufp);
	PIN_FLIST_FLD_SET(event_substr, PIN_FLD_SYS_DESCR, (void *)"Session: /event/broadband/usage", ebufp);
	
	/* get user id */
	user_pdp = CM_FM_USERID(ebufp);
	if(user_pdp)
	{
		PIN_FLIST_FLD_SET(event_substr, PIN_FLD_USERID, (void *)user_pdp, ebufp);
	}
 
	broadband_info = PIN_FLIST_SUBSTR_GET(i_flistp, PIN_FLD_EXTENDED_INFO, 0, ebufp);
	PIN_FLIST_FLD_SET(broadband_info, PIN_FLD_USAGE_CATEGORY, &usage_category, ebufp);
	PIN_FLIST_SUBSTR_SET(event_substr, broadband_info, PIN_FLD_BROADBAND_USAGE, ebufp );

	PIN_FLIST_SUBSTR_SET(event_substr, NULL, PIN_FLD_BROADBAND, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_act_rate_bb_event input error",ebufp);
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_act_rate_bb_event prepared flist for PCM_OP_ACT_USAGE", input_flist);

	PCM_OP(ctxp, PCM_OP_ACT_USAGE, 0, input_flist, r_flistpp, ebufp);
	//*r_flistpp = PIN_FLIST_COPY(input_flist, ebufp);  //not to be copied in actual return
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_act_rate_bb_event err calling PCM_OP_ACT_USAGE error",ebufp);
		goto CLEANUP;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_act_rate_bb_event", *r_flistpp);
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&input_flist, NULL);
	return;
}

