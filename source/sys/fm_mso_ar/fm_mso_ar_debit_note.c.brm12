/*******************************************************************
 * File:	fm_mso_ar_debit_note.c
 * Opcode:	MSO_OP_AR_DEBIT_NOTE 
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
 *	0 PIN_FLD_POID            	POID [0] 0.0.0.1 /account 1234
 *	0 PIN_FLD_USERID		POID [0] 0.0.0.1 /account 4321
 *	0 PIN_FLD_CURRENCY              INT [0] 356
 *	0 PIN_FLD_AMOUNT            	DECIMAL [0] 123.4
 *	0 PIN_FLD_CHARGING_ID     	STR [0] "OUTRIGHT_CHARGE_FOR_CR"
 *	0 MSO_FLD_SERVICE_TYPE     	ENUM [0] 0 [0-CATV,1-BB]
 *
 * Document History
 *
 * DATE         /  CHANGE                                       / Author
 * 17-Oct-2014  / Initial Version                               / Harish Kulkarni
 * 27-AUG-2014    Modify for bill_obj population 
 *                in /mso_account_balance                       / Gautam Adak
********************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_ar_debit_note.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_lifecycle_id.h"
#include "ops/ar.h"
#include "ops/bill.h"
#include "pcm_ops.h"

#define READWRITE 1
#define LOCAL_TRANS_OPEN_SUCCESS 0
#define EVENT_CR_DR_TYPE "/event/billing/mso_cr_dr_note"      
#define SEQ_TYPE_RECEIPT "MSO_SEQUENCE_TYPE_NOTE"
#define SERVICE_TAX "MSO_Service_Tax"
#define BILLINFO_ID_BB "BB"
#define BILLINFO_ID_CATV "CATV"
#define CREDIT_DEBIT_NOTE "CREDIT_DEBIT_NOTE"
#define SERV_TYPE_BB 1
#define SERV_TYPE_CATV 0
#define SUBSCRIPTION_HSN "9984"
#define RENTAL_HSN "9973"
/*******************************************************************
 * MSO_OP_AR_DEBIT_NOTE 
 *******************************************************************/

EXPORT_OP void
mso_op_ar_debit_note(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_ar_debit_note(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

void fm_mso_ar_create_note_obj(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             *res_flistp,
	poid_t			*service_pdp,
	char			*charging_id,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT void fm_mso_ar_create_adjustment(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*res_flistp,
	int32			*service_type,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp) ;

extern 
void
fm_mso_get_billinfo(
	pcm_context_t		*ctxp,
	poid_t			*bal_grp_obj,
	poid_t			**billinfo,
	pin_errbuf_t		*ebufp);
//
//static void
//mso_op_crdr_note_gen_lc_event(
//	pcm_context_t           *ctxp,
//	pin_flist_t             *i_flistp,
//	pin_decimal_t		*amount,
//	pin_flist_t		*au_flistp,
//	char                    *disp_name,
//	pin_errbuf_t            *ebufp);

static void fm_mso_ar_get_service_details(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        int32                   serv_type,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp);

PIN_IMPORT
void fm_mso_utils_gen_event(
	pcm_context_t   *ctxp,
	poid_t          *acct_pdp,
	poid_t          *serv_pdp,
	char            *program,
	char            *descr,
	char            *event_type,
	pin_flist_t     *in_flistp,
	pin_errbuf_t    *ebufp);

PIN_IMPORT
void
fm_mso_utils_read_any_object(
	pcm_context_t           *ctxp,
	poid_t                  *objp,
	pin_flist_t             **out_flistpp,
	pin_errbuf_t            *ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

PIN_IMPORT int32 fm_mso_trans_open(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	int32			flag,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_trans_commit(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	pin_errbuf_t		*ebufp);

PIN_IMPORT void fm_mso_trans_abort(
	pcm_context_t		*ctxp,
	poid_t			*pdp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * MSO_OP_AR_DEBIT_NOTE  
 *******************************************************************/
void 
mso_op_ar_debit_note(
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
	int			status = 0;
	int			local = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_AR_DEBIT_NOTE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_ar_debit_note error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"mso_op_ar_debit_note input flist", i_flistp);
	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, 3, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		status = 1;
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error opening transaction", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE,"53080" , ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Debit Note Fail output flist", *r_flistpp);
		goto CLEANUP;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"Transaction Opened");

	fm_mso_ar_debit_note(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp) || *r_flistpp == NULL) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"mso_op_ar_debit_note error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "mso_op_ar_debit_note error");
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, 
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ), ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
						"mso_op_ar_debit_note error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"mso_op_ar_debit_note output flist",  *r_flistpp); 
		status = *(int32 *)PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 0, ebufp);	
	}

CLEANUP:
	if (status == 1)
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction Aborted");
		}
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
		{
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction Committed");
		}
	}
	return;
}



/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_ar_debit_note(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
//	pcm_context_t		*user_ctxp = NULL;
	pin_flist_t		*input_flistp = NULL;
	pin_flist_t		*event_flistp = NULL;
	pin_flist_t             *res_flistp = NULL;
	pin_flist_t             *res_oflistp = NULL;
	pin_flist_t		*adj_oflistp = NULL;
	pin_flist_t             *bal_impact_flistp = NULL;
	pin_flist_t             *total_flistp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*result_flistp = NULL;
	pin_flist_t             *cp_flistp = NULL;
	pin_flist_t		*tax_flistp = NULL;
	
	poid_t			*acnt_pdp = NULL;
	poid_t			*user_pdp = NULL;
	poid_t			*event_pdp = NULL;
	poid_t			*bal_grp_obj = NULL;
	poid_t			*billinfo_obj = NULL;
	poid_t			*service_pdp = NULL;
		
        char                    evt_descr [255];
        char                    remarks [255];
        char                    adj_descr [255];
        char                    *charging_id = NULL;
        char                    *descr = NULL;

	pin_decimal_t		*amount = NULL;
	pin_decimal_t		*temp_rate = NULL;
	pin_decimal_t		*tax_rate = pbo_decimal_from_str("0.0", ebufp);
	pin_decimal_t		*hundred = pbo_decimal_from_str("100.0", ebufp);
	pin_decimal_t		*orig_amount = NULL;
	pin_decimal_t		*calc_amount = NULL;

	time_t                  *evt_end_t =NULL;
	time_t                  pvt = pin_virtual_time(NULL);

	int64			db = 0;
	int32			currency = 0;

	int32			*serv_type = NULL;
	int32			status = 0;
	int32			tax_flag = 2; //To apply tax always for CRDR note
	int32			str_id = 1;
	int32			str_ver = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_ar_debit_note error",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Input Error");
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
						"Input Error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
	}
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ar_debit_note input Flist", i_flistp);

	/* Open User Context */
//	fm_mso_utils_open_context(i_flistp, ctxp, &user_ctxp, ebufp);
	user_pdp = CM_FM_EFFECTIVE_USERID(ebufp);
	
	acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	db = PIN_POID_GET_DB(acnt_pdp);

	//evt_end_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 0, ebufp);
	evt_end_t = &pvt;
	serv_type = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_SERVICE_TYPE, 0, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp) || *serv_type == SERV_TYPE_CATV) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_ar_debit_note input error",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Input Error");
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
						"Input Error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "r_flistpp:", *r_flistpp);
		goto CLEANUP;
	}
	/* It's a Broadband service. Get the broadband balance details */
	fm_mso_ar_get_service_details(ctxp, acnt_pdp, *serv_type, &srch_flistp, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp)){
		PIN_ERRBUF_RESET(ebufp);
	}
	result_flistp = PIN_FLIST_ELEM_GET(srch_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if(result_flistp != NULL) {
		service_pdp = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_POID, 1, ebufp);
		bal_grp_obj = PIN_FLIST_FLD_GET(result_flistp, PIN_FLD_BAL_GRP_OBJ, 1, ebufp);
	}
	//if (bal_grp_obj)
//	{
//		fm_mso_get_billinfo(ctxp, bal_grp_obj, &billinfo_obj, ebufp);
//	}
	cp_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
        charging_id = (char *) PIN_FLIST_FLD_GET(cp_flistp, PIN_FLD_CHARGING_ID, 1, ebufp);
        descr = (char *) PIN_FLIST_FLD_GET(cp_flistp, PIN_FLD_DESCR, 1, ebufp);

        if(charging_id && strcmp(charging_id, "") != 0)
                strcpy(evt_descr, charging_id);
        else
                strcpy(evt_descr, "Others");

        if(descr && strcmp(descr, "") != 0)
                strcpy(remarks, descr);
        else
                strcpy(remarks, "Others");

        sprintf(adj_descr, "Debit Note charges for : %s : %s", evt_descr, remarks);

	mso_retrieve_gst_rate(ctxp, SUBSCRIPTION_HSN, acnt_pdp, &tax_flistp, ebufp);
	PIN_ERR_LOG_FLIST(3, "tax_ flistp", tax_flistp);
	if(tax_flistp)
	{
		PIN_ERR_LOG_MSG(3, "inside tax calc");
		temp_rate = PIN_FLIST_FLD_GET(tax_flistp, MSO_FLD_IGST_RATE, 1, ebufp);     //IGST
		if(!pbo_decimal_is_null(temp_rate, ebufp))
			pbo_decimal_add_assign(tax_rate, temp_rate, ebufp);

		temp_rate = PIN_FLIST_FLD_GET(tax_flistp, MSO_FLD_SGST_RATE, 1, ebufp);     //SGST
		if(!pbo_decimal_is_null(temp_rate, ebufp))
			pbo_decimal_add_assign(tax_rate, temp_rate, ebufp);

		temp_rate = PIN_FLIST_FLD_GET(tax_flistp, MSO_FLD_CGST_RATE, 1, ebufp);                     //CGST
		if(!pbo_decimal_is_null(temp_rate, ebufp))
			pbo_decimal_add_assign(tax_rate, temp_rate, ebufp);

		PIN_ERR_LOG_MSG(3, "Tax rate 0");
                PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(tax_rate, ebufp));

		pbo_decimal_multiply_assign(tax_rate, hundred, ebufp);
		PIN_ERR_LOG_MSG(3, "Tax rate 1");
                PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(tax_rate, ebufp));

		pbo_decimal_add_assign(tax_rate, hundred, ebufp);
		PIN_ERR_LOG_MSG(3, "Tax rate 2");
		PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(tax_rate, ebufp));
	}
	PIN_FLIST_DESTROY_EX(&tax_flistp, NULL);

	orig_amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
	PIN_ERR_LOG_MSG(3, "Orig Amt");
	PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(orig_amount, ebufp));
	calc_amount = pbo_decimal_divide(orig_amount, tax_rate, ebufp);
	PIN_ERR_LOG_MSG(3, "Calc Amt 1");
	PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(calc_amount, ebufp));
	pbo_decimal_multiply_assign(calc_amount, hundred, ebufp);
	PIN_ERR_LOG_MSG(3, "Calc Amt 2");
	PIN_ERR_LOG_MSG(3, pbo_decimal_to_str(calc_amount, ebufp));
	PIN_ERR_LOG_FLIST(3, " Input flistp", i_flistp);
	PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_AMOUNT, calc_amount, ebufp);
	PIN_ERR_LOG_FLIST(3, "Modified Input flistp", i_flistp);

        if(!pbo_decimal_is_null(hundred, ebufp))
                pbo_decimal_destroy(&hundred);
        if(!pbo_decimal_is_null(tax_rate, ebufp))
                pbo_decimal_destroy(&tax_rate);
        if(!pbo_decimal_is_null(calc_amount, ebufp))
                pbo_decimal_destroy(&calc_amount);

	input_flistp = PIN_FLIST_CREATE(ebufp);
     	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, input_flistp, PIN_FLD_POID, ebufp);
       	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, input_flistp, PIN_FLD_AMOUNT, ebufp);
       	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, input_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_DESCR, &adj_descr, ebufp);
	PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_STRING_ID, &str_id, ebufp);
	PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_STR_VERSION, &str_ver, ebufp);
       	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, input_flistp, PIN_FLD_USERID, ebufp);
       	PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_FLAGS, &tax_flag, ebufp);
	if(bal_grp_obj)
       		PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_BAL_GRP_OBJ, bal_grp_obj, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_ar_debit_note prepared input flist", input_flistp);
        PCM_OP(ctxp, PCM_OP_AR_ACCOUNT_ADJUSTMENT, 0, input_flistp, &res_flistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_mso_ar_debit_note err calling PCM_OP_AR_ACCOUNT_ADJUSTMENT error",ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		acnt_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Failed to Post Debit Note", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
        }
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
                "fm_mso_ar_debit_note output flist", res_flistp);

/*	PIN_FLIST_FLD_SET(input_flistp, PIN_FLD_POID, (void *)acnt_pdp, ebufp);

	evt_descr = (char *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CHARGING_ID, 0, ebufp);
	remarks = (char *) PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 1, ebufp);
	
	event_flistp = PIN_FLIST_SUBSTR_ADD(input_flistp, PIN_FLD_EVENT, ebufp);
	
	event_pdp = PIN_POID_CREATE(db, EVENT_CR_DR_TYPE, (int64)-1, ebufp);
	PIN_FLIST_FLD_PUT(event_flistp, PIN_FLD_POID, (void *)event_pdp, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)acnt_pdp, ebufp);
	if(*serv_type == 1)
		{PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_NAME, (void *)"BB_CR_DR_NOTE", ebufp);}
	else
		{PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_NAME, (void *)"CATV_CR_DR_NOTE", ebufp);}

	//PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_PROGRAM_NAME, (void *)"Credit/Debit Note Generation", ebufp);
	PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,event_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_DESCR, (void *)evt_descr, ebufp);
	memset(sys_descr,'\0',sizeof(sys_descr));
	sprintf(sys_descr,"Debit Note charges for : %s : %s",evt_descr,remarks);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_SYS_DESCR, (void *)sys_descr, ebufp);

	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_START_T, (void *)evt_end_t, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_END_T, (void *)evt_end_t, ebufp);
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
	PIN_FLIST_FLD_PUT(event_flistp, PIN_FLD_BILLINFO_OBJ, billinfo_obj, ebufp);

	amount = (pin_decimal_t *)PIN_FLIST_FLD_GET(i_flistp,PIN_FLD_AMOUNT, 0, ebufp);
	currency = *(int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CURRENCY, 1, ebufp);
	total_flistp=PIN_FLIST_ELEM_ADD(event_flistp, PIN_FLD_TOTAL, currency, ebufp);
	PIN_FLIST_FLD_SET(total_flistp,PIN_FLD_AMOUNT,(void *)amount,ebufp)
	PIN_FLIST_FLD_SET(event_flistp, PIN_FLD_USERID, (void *)user_pdp, ebufp);
 
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_ar_debit_note prepared flist for PCM_OP_ACT_USAGE", input_flistp);

	PCM_OP(user_ctxp, PCM_OP_ACT_USAGE, 0, input_flistp, &res_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) { 
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_mso_ar_debit_note err calling PCM_OP_ACT_USAGE error",ebufp);
		goto CLEANUP;
	}*/

	/* create CR/DR Note OBject */
	fm_mso_ar_create_note_obj(ctxp, i_flistp, res_flistp, service_pdp, evt_descr, &res_oflistp, ebufp);  	

        if (PIN_ERRBUF_IS_ERR(ebufp) || res_oflistp == NULL)
        {
                PIN_ERRBUF_CLEAR(ebufp);
                status = 1;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Credir/Debit Note Object Creation Error");
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acnt_pdp, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53003", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
                                        "Credir/Debit Note Object Creation Error", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
                goto CLEANUP;
        }

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ar_create_note_obj Output:", res_oflistp);


	fm_mso_ar_create_adjustment(ctxp, i_flistp, res_flistp, serv_type, &adj_oflistp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp) || adj_oflistp == NULL)
        {
                PIN_ERRBUF_CLEAR(ebufp);
                status = 1;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Adjustment Object Creation Error");
                *r_flistpp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acnt_pdp, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53003", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Adjustment Object Creation Error", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
                goto CLEANUP;
        }
	// Call the function for creation of Lifecycle Event
	//mso_op_crdr_note_gen_lc_event(ctxp, i_flistp, amount, res_flistp, CREDIT_DEBIT_NOTE, ebufp);	  
	fm_mso_create_lifecycle_evt(ctxp, i_flistp, res_oflistp, ID_CREDIT_DEBIT_NOTE, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Credir/Debit Note Lifecycle Error");
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, acnt_pdp, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
					"Credir/Debit Note Lifecycle Error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
		goto CLEANUP;
	}
	
	*r_flistpp = PIN_FLIST_COPY(res_oflistp, ebufp);
	
	CLEANUP:
	PIN_FLIST_DESTROY_EX(&input_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&res_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&adj_oflistp, NULL);
	/* Close User Context */
//        fm_mso_utils_close_context(user_ctxp, ebufp);
	return;
}

void fm_mso_ar_create_note_obj(
	pcm_context_t   	*ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t             *res_flistp,
	poid_t			*service_pdp,
	char			*charging_id,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t    	*ebufp) {

        poid_t                  *acct_pdp = NULL;
        poid_t                  *user_id = NULL;
	poid_t                  *rcpt_pdp = NULL;
	poid_t                  *item_pdp = NULL;
	poid_t			*evt_pdp = NULL;
        pin_flist_t             *in_flistp = NULL;
        pin_flist_t             *r_flistp = NULL;
        pin_flist_t             *rcpt_flistp = NULL;
	pin_flist_t             *b_flistp = NULL;
	pin_flist_t             *evt_flistp = NULL;
	pin_flist_t             *ret_flistp = NULL;
        int64                   id = (int64)-1;
	int64			status = 0;
        u_int64                 db = 0;

        in_flistp = PIN_FLIST_CREATE(ebufp);

	//Generating the Sequence for Receipts
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	user_id = CM_FM_USERID(ebufp);	
	db = PIN_POID_GET_DB(acct_pdp);
        PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_POID, acct_pdp, ebufp);
       	PIN_FLIST_FLD_SET(in_flistp, PIN_FLD_NAME, SEQ_TYPE_RECEIPT, ebufp);
       	fm_mso_utils_sequence_no(ctxp, in_flistp, &r_flistp, ebufp);
		
	rcpt_flistp = PIN_FLIST_CREATE(ebufp);
        rcpt_pdp = PIN_POID_CREATE(db, "/mso_cr_dr_note", id, ebufp);
        PIN_FLIST_FLD_PUT(rcpt_flistp, PIN_FLD_POID, rcpt_pdp, ebufp);
        PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);
        PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_STRING, rcpt_flistp, PIN_FLD_RECEIPT_NO, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, rcpt_flistp, PIN_FLD_AMOUNT, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, rcpt_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
        PIN_FLIST_FLD_COPY(i_flistp, MSO_FLD_SERVICE_TYPE, rcpt_flistp, MSO_FLD_SERVICE_TYPE, ebufp);

	evt_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	evt_pdp = PIN_FLIST_FLD_GET(evt_flistp, PIN_FLD_POID, 0, ebufp); 
	b_flistp =  PIN_FLIST_ELEM_GET(evt_flistp, PIN_FLD_BAL_IMPACTS, PIN_ELEMID_ANY, 1, ebufp);
	item_pdp = PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);

        PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_ITEM_OBJ, item_pdp, ebufp);
        PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_EVENT_OBJ, evt_pdp, ebufp);
        PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_USERID, user_id, ebufp);
	PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_CHARGING_ID, charging_id, ebufp);
	PIN_FLIST_FLD_SET(rcpt_flistp, PIN_FLD_SERVICE_OBJ, service_pdp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CR DR Note Input Flist", rcpt_flistp);
        //Creating Debit Note 
        PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, rcpt_flistp, &ret_flistp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CR DR Note Output Flist", ret_flistp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERRBUF_CLEAR(ebufp);
        	status = 1;
        	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Credit/Debit Note Creation Error");
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
        	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, user_id, ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Credit/Debit Note Creation Error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53002", ebufp);
                PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
                goto CLEANUP;
     	}
	status = 0;
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_DESCR, "Credit/Debit Note Success", ebufp)
        PIN_FLIST_FLD_COPY(r_flistp, PIN_FLD_STRING, ret_flistp, PIN_FLD_RECEIPT_NO, ebufp);
	PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &status, ebufp)
	*r_flistpp = PIN_FLIST_COPY(ret_flistp, ebufp);

CLEANUP:
	PIN_FLIST_DESTROY_EX(&in_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
        PIN_FLIST_DESTROY_EX(&rcpt_flistp, NULL);
}

//static void
//mso_op_crdr_note_gen_lc_event(
//	pcm_context_t           *ctxp,
//	pin_flist_t             *i_flistp,
//        pin_decimal_t           *amount,
//        pin_flist_t             *au_flistp,
//	char                    *disp_name,
//	pin_errbuf_t            *ebufp)
//{
//	pin_flist_t             *lc_iflistp = NULL;
//	pin_flist_t             *lc_temp_flistp = NULL;
//	pin_flist_t             *ro_flistp = NULL;
//	pin_flist_t             *res_flistp = NULL;
//	poid_t                  *user_id = NULL;
//	poid_t                  *acct_pdp = NULL;
//	char                    *event = "/event/activity/mso_lifecycle/credit_debit_note";
//
//	if (PIN_ERRBUF_IS_ERR(ebufp))
//			return;
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_crdr_note_gen_lc_event Input FList:", i_flistp);
//
//	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp );
//	user_id = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_USERID, 0, ebufp );
//
//	lc_iflistp = PIN_FLIST_CREATE(ebufp);
//	lc_temp_flistp = PIN_FLIST_ELEM_ADD(lc_iflistp, MSO_FLD_CRDR_NOTE, 0, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_NAME, disp_name, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_USERID, user_id, ebufp);
//	PIN_FLIST_FLD_SET(lc_temp_flistp, PIN_FLD_AMOUNT, amount, ebufp);
//	res_flistp = PIN_FLIST_ELEM_GET(au_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
//	if(res_flistp)
//		PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, lc_temp_flistp, PIN_FLD_EVENT_OBJ, ebufp);
//
//	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_gen_event Event FList:", lc_iflistp);
//
//	fm_mso_utils_gen_event(ctxp, acct_pdp, NULL, "MSO_OP_AR_DEBIT_NOTE",
//							disp_name, event, lc_iflistp, ebufp);
//
//	PIN_FLIST_DESTROY_EX(&lc_iflistp, NULL);
//	PIN_FLIST_DESTROY_EX(&ro_flistp, NULL);
//	return;
//}

static void fm_mso_ar_get_service_details(
        pcm_context_t           *ctxp,
        poid_t                  *acct_pdp,
        int32                   serv_type,
        pin_flist_t             **o_flistpp,
        pin_errbuf_t            *ebufp)
{
        pin_flist_t             *s_flistp = NULL;
        pin_flist_t             *args_flistp = NULL;
        pin_flist_t             *res_flistp = NULL;
        poid_t                  *s_pdp = NULL;
        poid_t                  *serv_pdp = NULL;
        int32                   flags = 256;
        int64                   db = 0;
        int64                   id = -1;
        char                    *s_template = " select X from /service where F1 = V1 and F2.type = V2 order by service_t.created_t desc ";

        if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Error before calling fm_mso_ar_get_service_details", ebufp);
                return;
        }

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside function fm_mso_ar_get_service_details");
        PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "acct_pdp", acct_pdp );

        db = PIN_POID_GET_DB(acct_pdp);
        s_flistp = PIN_FLIST_CREATE(ebufp);
        s_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
        if (serv_type == 1)
        {
                serv_pdp = PIN_POID_CREATE(db, "/service/telco/broadband", id, ebufp);
        }
        if (serv_type == 0)
        {
                serv_pdp = PIN_POID_CREATE(db, "/service/catv", id, ebufp);
        }

        /*******************************************************
        PCM_OP_SEARCH Input:
        0 PIN_FLD_POID POID [0] 0.0.0.1 /search -1
        0 PIN_FLD_FLAGS INT [0] 256
        0 PIN_FLD_TEMPLATE STR [0] " select X from /service where F1 = V1 and F2 = V2 "
        0 PIN_FLD_ARGS ARRAY [1]
        1   PIN_FLD_ACCOUNT_OBJ POID [0] 0.0.0.1 /account 2321559
        0 PIN_FLD_ARGS ARRAY [2]
        1   PIN_FLD_POID POID [0] 0.0.0.1 /service/telco/broadband -1
        0 PIN_FLD_RESULTS ARRAY [0] NULL
        *******************************************************/
        PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &flags, ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, s_template, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ACCOUNT_OBJ, acct_pdp, ebufp);

        args_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
        PIN_FLIST_FLD_PUT(args_flistp, PIN_FLD_POID, serv_pdp, ebufp);

        res_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);

        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Input flist:",s_flistp);
        PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, o_flistpp, ebufp);
        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Search Service Output flist:",*o_flistpp);

        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
        return;
}

