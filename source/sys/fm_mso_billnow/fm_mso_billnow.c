
#ifndef lint
static const char Sccs_id[] = "@(#)%Podrtal Version: fm_mso_billnow.c:CUPmod7.3PatchInt:1:2014-July-22 08:57:18 %";
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
#include "ops/bill.h"
#include "pin_inv.h"
#include "pin_rate.h"
#include "mso_lifecycle_id.h"
#include "mso_cust.h"

#define READONLY 0
#define READWRITE 1

/*******************************************************************
 * Global variables
 *******************************************************************/
 
char bill_type[10];
char charge_type[10];

int32 ibill_type = -1;
int32 icharge_type = -1;
/**************************************
* External Functions start
***************************************/
/**************************************
* External Functions end
***************************************/

PIN_IMPORT void fm_mso_trans_commit(
	pcm_context_t   *ctxp,
	poid_t		*pdp,
	pin_errbuf_t	*ebufp);

/**************************************
* Local Functions start
***************************************/

EXPORT_OP void
mso_op_bill_make_bill_now(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

void mso_op_bill_make_bill_now_process(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

void mso_op_bill_make_trial_bill_now(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

void mso_op_create_input_for_billnow(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

void mso_op_process_trial_billnow(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

void mso_op_process_invoicing(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	pin_flist_t	*bill_flistp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);
void mso_op_create_trialbillnow_invoice_object(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	poid_t		*acct_poidp,
	pin_flist_t	*bill_flistp,
	pin_errbuf_t	*ebufp);
void mso_op_abort_transaction(
	pcm_context_t *ctxp,
	int flag,
	pin_errbuf_t *ebufp);
void mso_op_open_transaction(
	pcm_context_t *ctxp,
	int flag,
	pin_errbuf_t *ebufp);
void mso_op_commit_transaction(
	pcm_context_t	*ctxp,
	int		flag,
	pin_errbuf_t	*ebufp);
void
mso_op_create_trial_invoice_report(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	pin_flist_t	*tran_flistp1,
	pin_flist_t	*bill_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

void mso_op_read_bill_obj(
	pcm_context_t	*ctxp,
	pin_flist_t	*input_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp);

void mso_op_get_payinfo_flist(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	poid_t			*bi_poidp,
	pin_flist_t		**payinfo_flistpp,
	pin_errbuf_t		*ebufp);

void mso_op_create_err_flist(
	char			*error_descr,
	char			*error_code,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);
void mso_op_get_tax_events(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*item_flistp,
	pin_flist_t		**tax_event_flistpp,
	pin_errbuf_t		*ebufp);

void mso_op_get_tax_items(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**tax_item_flistpp,
	pin_errbuf_t		*ebufp);

void mso_op_create_inv_doc_reference(
	pcm_context_t		*ctxp,
	pin_flist_t		*inv_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_city_code(
	pcm_context_t		*ctxp, 
	poid_t			*acct_poidp, 
	char			**city_code, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_cmts(
	pcm_context_t		*ctxp, 
	poid_t			*srv_pdp, 
	char			**cmts, 
	int			*payterm,
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_payterm(
	pcm_context_t		*ctxp, 
	poid_t			*srv_pdp, 
	poid_t			*item_pdp, 
	int			*pay_term, 
	pin_errbuf_t		*ebufp);
			
static void
fm_mso_get_prod_details(
	pcm_context_t		*ctxp, 
	pin_flist_t		*event_flistp,
	poid_t			*acct_poidp, 
	poid_t			*srv_pdp, 
	char			**plan_name, 
	char			**plan_code, 
	char			**prov_tag, 
	char			**plan_type, 
	char			**disc_type, 
	poid_t			**prod_pdp, 
	pin_decimal_t		**disc_rate, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_event_flist(
	pcm_context_t		*ctxp, 
	poid_t			*item_pdp, 
	pin_flist_t		**event_flistp, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_event_details(
	pcm_context_t		*ctxp, 
	pin_flist_t		*evt_flistp, 
	pin_decimal_t		**charge, 
	pin_decimal_t		**discount, 
	char			**event_desc, 
	pin_errbuf_t		*ebufp);

static void
fm_mso_get_prov_tag(
	pcm_context_t		*ctxp, 
	poid_t			*plan_pdp, 
	poid_t			*srv_pdp, 
	char			**prov_tag, 
	char			**plan_type, 
	char			**plan_code, 
	char			**plan_name, 
	pin_errbuf_t		*ebufp);

static void
mso_op_trial_billnow(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp );

static void
mso_op_catv_bill_now(
	pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp );

/*******************************************************************
 * MSO_OP_AR_GET_DETAILS  
 *******************************************************************/
void 
mso_op_bill_make_bill_now(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now: start.");	
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"mso_op_bill_make_bill_now input flist", i_flistp);
	// Insanity Check 
	if (opcode != MSO_OP_BILL_MAKE_BILL_NOW) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_bill_make_bill_now error", ebufp);
		goto cleanup;
	}
	mso_op_bill_make_bill_now_process(connp, ctxp, i_flistp, o_flistpp,  ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(o_flistpp, NULL);
		*o_flistpp = (pin_flist_t *)NULL;		
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_bill_make_bill_now error", ebufp);
	}
	else 
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_bill_make_bill_now success return flist", *o_flistpp);
	}
	
cleanup:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now: end");
	return;
}

void mso_op_bill_make_bill_now_process(
	cm_nap_connection_t	*connp,
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{

	void			*vp = NULL;
	poid_t			*acct_poidp = NULL;
	poid_t			*bi_poidp = NULL;
	pin_flist_t		*r_flistp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now_process: start.");
	
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"mso_op_bill_make_bill_now_process input flist", i_flistp);

	vp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_BILL_TYPE, 0, ebufp);
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_bill_make_bill_now_process error", ebufp);
		goto cleanup;
	}

	if(vp)
	{
		strcpy(bill_type, (char *) vp);
	}
	vp = PIN_FLIST_FLD_GET(i_flistp, MSO_FLD_CHARGE_TYPE, 0, ebufp);
	if(vp)
	{
		strcpy(charge_type, (char *) vp);
	}

	if(strcmp(bill_type, "normal") == 0 && strcmp(charge_type, "normal") == 0)
	{
		//trigger bill now for normal items
		ibill_type = 0; icharge_type = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now_process: bill_type=NORMAL, charge_type=NORMAL");
		mso_op_bill_make_trial_bill_now(connp, ctxp, i_flistp, &r_flistp,  ebufp);	
	}
	else if(strcmp(bill_type, "normal") == 0 && strcmp(charge_type, "hardware") == 0)
	{
		//trigger trial bill for for normal items	
		ibill_type = 0; icharge_type = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now_process: bill_type=NORMAL, charge_type=HARDWARE");
		mso_op_bill_make_trial_bill_now(connp, ctxp,i_flistp, &r_flistp,  ebufp);	
	}
	else if(strcmp(bill_type, "normal") == 0 && strcmp(charge_type, "deposit") == 0)
        {
                //trigger trial bill for for normal items
                ibill_type = 0; icharge_type = 3;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now_process: bill_type=NORMAL, charge_type=DEPOSIT");
                mso_op_bill_make_trial_bill_now(connp, ctxp,i_flistp, &r_flistp,  ebufp);
	}	
	else if(strcmp(bill_type, "trial") == 0 && strcmp(charge_type, "normal") == 0)
	{
		ibill_type = 1; icharge_type = 0;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now_process: bill_type=TRIAL, charge_type=NORMAL");
		mso_op_bill_make_trial_bill_now(connp, ctxp, i_flistp, &r_flistp,  ebufp);	
	}
	else if(strcmp(bill_type, "trial") == 0 && strcmp(charge_type, "hardware") == 0)
	{
		ibill_type = 1; icharge_type = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now_process: bill_type=TRIAL, charge_type=HARDWARE");
		mso_op_bill_make_trial_bill_now(connp, ctxp, i_flistp, &r_flistp,  ebufp);		
	}
	else if(strcmp(bill_type, "trial") == 0 && strcmp(charge_type, "deposit") == 0)
        {
                ibill_type = 1; icharge_type = 3;
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now_process: bill_type=TRIAL, charge_type=DEPOSIT");
                mso_op_bill_make_trial_bill_now(connp, ctxp, i_flistp, &r_flistp,  ebufp);
        }
	
	else if(strcmp(bill_type, "normal") ==0 && strcmp(charge_type, "CATV") == 0)
	{
		mso_op_catv_bill_now(ctxp, i_flistp, &r_flistp, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now_process: bill_type=UNKNOWN, charge_type=UNKNOWN");
		//set error
		goto cleanup;
	}
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_bill_make_bill_now_process error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*ret_flistpp = NULL;
	}else
	{
		*ret_flistpp = r_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_bill_make_bill_now return flist", *ret_flistpp);
	}
	
cleanup:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_bill_now_process: end");
	return;
}

void mso_op_bill_make_trial_bill_now(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*read_bill_iflist = NULL;
	pin_flist_t	*read_bill_oflist = NULL;

	poid_t		*bill_obj = NULL;

	void		*vp = NULL;


	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_trial_bill_now: start.");
	
	PIN_ERRBUF_CLEAR(ebufp);


	if((ibill_type == 0 && icharge_type == 0) || (ibill_type == 0 && icharge_type == 1) || (ibill_type == 0 && icharge_type == 3))
	{
		mso_op_create_input_for_billnow(ctxp, i_flistp, &ret_flistp, ebufp);
		
		if(ret_flistp == NULL)
		{
			mso_op_create_err_flist("no valid bills", "1234", ret_flistpp, ebufp);
			goto cleanup;	
		}
		//mso_op_process_invoicing(connp, ctxp, ret_flistp, i_flistp, ret_flistpp, ebufp);
		/*******************************************
		 calling for Lifecycle
		*******************************************/
		vp  =  (pin_flist_t*)PIN_FLIST_ELEM_GET(ret_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
		if (vp)
		{
			bill_obj = PIN_FLIST_FLD_GET(vp, PIN_FLD_LAST_BILL_OBJ, 1, ebufp ); 
		}
		if (bill_obj)
		{
			read_bill_iflist = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_POID, bill_obj, ebufp);
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_BILL_NO, "", ebufp);
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_DUE, NULL, ebufp);
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_TOTAL_DUE, NULL, ebufp);
			PIN_FLIST_FLD_SET(read_bill_iflist, PIN_FLD_DUE_T, NULL, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill Object Read input flist", read_bill_iflist);
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_bill_iflist, &read_bill_oflist, ebufp);
			PIN_FLIST_DESTROY_EX(&read_bill_iflist, NULL);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Bill Object Read output flist", read_bill_oflist);
			//Calling Lifecycle_event
			PIN_FLIST_FLD_SET(read_bill_oflist, PIN_FLD_PROGRAM_NAME, "fm_mso_billnow", ebufp);
			fm_mso_create_lifecycle_evt(ctxp, read_bill_oflist, NULL, ID_BILLING, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			PIN_FLIST_DESTROY_EX(&read_bill_oflist, NULL);
		}
	}
	else if((ibill_type == 1 && icharge_type == 0) || (ibill_type == 1 && icharge_type == 1) || (ibill_type == 1 && icharge_type == 3))
	{
		mso_op_process_trial_billnow(connp, ctxp, i_flistp, &ret_flistp, ebufp);
	}
	else
	{
		//wrong inputs
	}

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_bill_make_trial_bill_now error", ebufp);
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		*ret_flistpp = NULL;
	}else{
		*ret_flistpp = ret_flistp;
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_trial_bill_now return flist", *ret_flistpp);
	}


cleanup:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_bill_make_trial_bill_now: end");
	return;

}

void mso_op_create_err_flist(
	char		*error_descr,
	char		*error_code,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*err_flistp = NULL;
	poid_t		*err_pdp = NULL;
	int64		db = 0;
	u_int64		id = (u_int64)-1;
	int32		status = 1;

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_mso_op_create_err_flist: start.");


	err_flistp = PIN_FLIST_CREATE(ebufp);
	err_pdp = PIN_POID_CREATE(db, "/search", id, ebufp);
	
	
	PIN_FLIST_FLD_PUT(err_flistp, PIN_FLD_POID, err_pdp, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_STATUS, &status, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_CODE, error_code, ebufp);
	PIN_FLIST_FLD_SET(err_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
	*ret_flistpp = PIN_FLIST_COPY(err_flistp, ebufp);
cleanup:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_mso_op_create_err_flist: end");
	PIN_FLIST_DESTROY_EX(&err_flistp, NULL);
	return;
}

void mso_op_process_trial_billnow(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*in_flistp = NULL;
	pin_flist_t	*report_flistp = NULL;
	poid_t		*acct_pdp = NULL;
	pcm_context_t	*new_ctxp = NULL;
	int		already_set = 0;
	int		type = 0;
	int		local = 1;
	poid_t		*d_pdp = NULL;
	char		msg[100]="";

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_process_trial_billnow: start.");

	// Open transaction
	if(ibill_type == 1)
	{
  		mso_op_open_transaction( ctxp, 0, ebufp);
		if( PIN_ERRBUF_IS_ERR(ebufp) ){
			PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
				"mso_op_process_trial_billnow: Trans open error", ebufp );
			goto cleanup;
		}
		mso_op_trial_billnow(ctxp, i_flistp, &ret_flistp, ebufp );
		if( ret_flistp ){
			PIN_FLIST_FLD_COPY( i_flistp, PIN_FLD_ACCOUNT_OBJ, ret_flistp, 
					PIN_FLD_ACCOUNT_OBJ, ebufp );
		}
	}
	else{
		mso_op_create_input_for_billnow(ctxp, i_flistp, &ret_flistp, ebufp);
	}

	if(ret_flistp == NULL)
	{
		mso_op_create_err_flist("no valid bills", "1234", ret_flistpp, ebufp);
		already_set = 1;
		goto cleanup;	
	}
	mso_op_process_invoicing(connp, ctxp, ret_flistp, i_flistp, &report_flistp, ebufp); 

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_process_trial_billnow error", ebufp);
		*ret_flistpp = NULL;
		goto cleanup;		
	}else{

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"mso_op_process_trial_billnow return flist",report_flistp);
	}
cleanup:
	// Abort transaction
	if(ibill_type == 1)
	{
		if( !PIN_ERRBUF_IS_ERR(ebufp) && report_flistp && already_set == 0){
			acct_pdp = PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp );
			mso_op_create_trialbillnow_invoice_object(connp, ctxp, acct_pdp, report_flistp, ebufp);
		}
		//PIN_FLIST_DESTROY_EX( &report_flistp, NULL );
		mso_op_abort_transaction( ctxp, 0, ebufp);
		

		if( report_flistp == NULL && already_set == 0 ){
			mso_op_create_err_flist("no charges for trial bill", "1235", ret_flistpp, ebufp);
		}
		else{
			mso_op_create_err_flist("Trial report successfully created", "0", ret_flistpp, ebufp );
		}
	}
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_process_trial_billnow: end");
	return;

}



/************************************************
 * Developer: Sudhish Jugran
 * Changes : Function to execute the trial bill of
 *	the account and create the mso_trial_report object.

 ***********************************************/
static void
mso_op_trial_billnow(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp )
{
        pin_flist_t             *trial_iflistp  = NULL;
        pin_flist_t             *trial_oflistp  = NULL;

	pin_decimal_t		*current_total	= NULL;

        time_t                  *start_t        = NULL;

        int                     state           = 2;

        if( PIN_ERRBUF_IS_ERR(ebufp)){
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        trial_iflistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_COPY( i_flistp, PIN_FLD_ACCOUNT_OBJ,
                trial_iflistp, PIN_FLD_POID, ebufp );
        PIN_FLIST_FLD_COPY( i_flistp, PIN_FLD_BILLINFO_OBJ,
                trial_iflistp, PIN_FLD_BILLINFO_OBJ, ebufp );
        PIN_FLIST_FLD_SET( trial_iflistp, PIN_FLD_PROGRAM_NAME,
                (void *)"pin_trial_bill_accts", ebufp );
        start_t = PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_START_T,
                1, ebufp );
        if( start_t ){
                PIN_FLIST_FLD_SET( trial_iflistp, PIN_FLD_START_T,
                        (void *)start_t, ebufp );
        }
        PIN_FLIST_FLD_COPY( i_flistp, PIN_FLD_END_T,
                trial_iflistp, PIN_FLD_END_T, ebufp );
        PIN_FLIST_FLD_SET( trial_iflistp, PIN_FLD_PREINVOICE_MODE,
                (void *)&state, ebufp );
        /******************************************
         * Log input flist
         *****************************************/
        PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
                "mso_op_trial_billnow : trial bill input flist",
                trial_iflistp );

        PCM_OP( ctxp, PCM_OP_BILL_MAKE_TRIAL_BILL, 0, trial_iflistp,
                &trial_oflistp, ebufp );

        if( PIN_ERRBUF_IS_ERR(ebufp)){
                PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,
                        "mso_op_trial_billnow : Trial bill error",
                        ebufp );
                PIN_FLIST_DESTROY_EX( &trial_oflistp, NULL );
        }
        else{
                PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG,
                        "mso_op_trial_billnow : Trial bill output flist",
                        trial_oflistp );
		current_total = PIN_FLIST_FLD_GET( trial_oflistp,
			PIN_FLD_CURRENT_TOTAL, 1, ebufp );
		if( current_total && pbo_decimal_sign(current_total, ebufp)){
               		*r_flistpp = trial_oflistp;
		}
		else{
			PIN_FLIST_DESTROY_EX( &trial_oflistp, NULL );
		}
        }

        PIN_FLIST_DESTROY_EX( &trial_iflistp, NULL );
}



void mso_op_create_input_for_billnow(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*trial_bill_in_flistp = NULL;
	pin_flist_t	*trial_bill_ret_flistp = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*search_ret_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*item_flistp = NULL;
	pin_flist_t	*item_flistp1 = NULL;
	pin_flist_t	*item_obj_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_cookie_t	r_cookie = NULL;
	poid_t		*s_pdp = NULL;
	poid_t		*acct_poidp = NULL;
	poid_t		*bi_poidp = NULL;
	pin_flist_t	*pay_order_arrp = NULL;
	pin_flist_t	*item_arrp = NULL;
	pin_flist_t	*tax_event_flistp = NULL;
	pin_flist_t	*tax_items_flistp = NULL;
	pin_flist_t	*dummy_item_flist = NULL;	
	int		r_elemid = 0;
	int32		s_flags = 0;
	int32		flags = 0;
	int64		db = 0;
	int		status = 0;
	int		elem_count = 0;
	char		ptemplate[BUFSIZ] = {0};
	void		*vp = NULL;
	time_t		next_t = (time_t)0;
	char		*str = "";
	time_t		pvt = pin_virtual_time((time_t *)NULL);

	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_create_input_for_billnow: start.");
	acct_poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	bi_poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTG_NEXT_T, 1, ebufp);
	if (vp) {
		next_t = *((time_t *)vp);
		next_t = next_t - 1;
	}
	
	if(icharge_type == 1)
	{
		/* Hardware items */
		strcpy(ptemplate, "select X from /item 1, /mso_cfg_pymt_priority 2  where 1.F1 = V1 and item_t.poid_type = 2.F2 and mso_cfg_chargehead_mapping_t.item_indicator = 1 and item_t.status = 1 and (item_t.item_total <> 0 or exists (select 1 from event_bal_impacts_t eb where eb.item_obj_id0 = item_t.poid_id0 and (eb.amount > 0 or eb.discount > 0) and resource_id = 356)) ");
	}
	else if(icharge_type == 3)
        {
                /* Subscription and Installation items. Exclude hw and usage items */
                strcpy(ptemplate, "select X from /item 1, /mso_cfg_pymt_priority 2  where 1.F1 = V1 and item_t.poid_type = 2.F2 and mso_cfg_chargehead_mapping_t.item_indicator = 3 and item_t.status = 1 and (item_t.item_total <> 0 or exists (select 1 from event_bal_impacts_t eb where eb.item_obj_id0 = item_t.poid_id0 and (eb.amount > 0 or eb.discount > 0) and resource_id = 356)) ");
        }	
	else
	{
		/* Subscription and Installation items. Exclude hw and usage items */
		strcpy(ptemplate, "select X from /item 1, /mso_cfg_pymt_priority 2  where 1.F1 = V1 and item_t.poid_type = 2.F2 and mso_cfg_chargehead_mapping_t.item_indicator = 2 and item_t.status = 1 and (item_t.item_total <> 0 or exists (select 1 from event_bal_impacts_t eb where eb.item_obj_id0 = item_t.poid_id0 and (eb.amount > 0 or eb.discount > 0) and resource_id = 356)) ");
	}

	s_flistp = PIN_FLIST_CREATE(ebufp);
	//item_flistp1 = PIN_FLIST_CREATE(ebufp);

	db = pin_poid_get_db(bi_poidp);
	s_pdp = (poid_t *)PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET (s_flistp, PIN_FLD_TEMPLATE, ptemplate, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET (tmp_flistp, PIN_FLD_ACCOUNT_OBJ, acct_poidp, ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_ARGS, 2, ebufp);
	pay_order_arrp = PIN_FLIST_ELEM_ADD (tmp_flistp, PIN_FLD_PAY_ORDER_INFO, PIN_ELEMID_ANY, ebufp);
	item_arrp = PIN_FLIST_ELEM_ADD (pay_order_arrp, PIN_FLD_ITEM_TYPES, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET (item_arrp, PIN_FLD_ITEM_TYPE, str , ebufp);

	tmp_flistp = PIN_FLIST_ELEM_ADD (s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ITEM_TOTAL, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_ADJUSTED, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_DISPUTED, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_WRITEOFF, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_RECVD, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_TRANSFERED, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_SERVICE_OBJ, NULL, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_BILL_OBJ, NULL, ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_input_for_billnow search input flist",  s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &search_ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp) ) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"error in mso_op_create_input_for_billnow in search on getting item poid ", ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_input_for_billnow search ret flist",  search_ret_flistp);

	elem_count = PIN_FLIST_ELEM_COUNT(search_ret_flistp, PIN_FLD_RESULTS, ebufp);
	if(elem_count > 0)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Inside elem_count");
		trial_bill_in_flistp = PIN_FLIST_CREATE(ebufp);
		//dummy_item_flist = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET (trial_bill_in_flistp, PIN_FLD_POID, acct_poidp, ebufp);
		PIN_FLIST_FLD_SET (trial_bill_in_flistp, PIN_FLD_BILLINFO_OBJ, bi_poidp, ebufp);
		
		if(ibill_type == 0){
			PIN_FLIST_FLD_SET (trial_bill_in_flistp, PIN_FLD_PROGRAM_NAME, "mso_bill_accts_now", ebufp);
			if(vp && vp != NULL && icharge_type == 1 || icharge_type == 3 && pvt >= *(time_t *)vp)
				PIN_FLIST_FLD_SET (trial_bill_in_flistp, PIN_FLD_END_T, (void *)&next_t, ebufp);
		}else{
			PIN_FLIST_FLD_SET (trial_bill_in_flistp, PIN_FLD_END_T, (void *)&next_t, ebufp);
			PIN_FLIST_FLD_SET (trial_bill_in_flistp, PIN_FLD_PROGRAM_NAME, "pin_trial_bill_accts", ebufp);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before while");
		if(icharge_type == 1 || icharge_type == 3)
        	{
			r_cookie = NULL;r_elemid = 0;
			while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(search_ret_flistp, PIN_FLD_RESULTS,&r_elemid,1,&r_cookie,ebufp)) != NULL)
			{
				item_flistp = PIN_FLIST_ELEM_ADD (trial_bill_in_flistp, PIN_FLD_ITEMS, r_elemid, ebufp);
				//item_obj_flistp = PIN_FLIST_ELEM_ADD (dummy_item_flist, PIN_FLD_ITEMS, r_elemid, ebufp);
		
				PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, item_flistp, PIN_FLD_ITEM_OBJ, ebufp);
				//PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, item_obj_flistp, PIN_FLD_ITEM_OBJ, ebufp);
			}
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Before while 2");
		r_elemid = 0;
		r_cookie = NULL;
		/*while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(dummy_item_flist, PIN_FLD_ITEMS,&r_elemid,1,&r_cookie,ebufp)) != NULL)
		{	mso_op_get_tax_events(ctxp, i_flistp, res_flistp, &tax_event_flistp, ebufp);
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After reading events");
			mso_op_get_tax_items(ctxp, tax_event_flistp, &tax_items_flistp, ebufp);		
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After reading items");
			if(tax_items_flistp)
				PIN_FLIST_CONCAT (item_flistp1, tax_items_flistp, ebufp);
		}
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "After reading events and items");
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_create_input_for_billnow error", ebufp);
			PIN_FLIST_DESTROY_EX(&trial_bill_ret_flistp, NULL);
			mso_op_create_err_flist("error during item event fetch", "12345", ret_flistpp, ebufp);
			goto cleanup;
		}*/
		//PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_input_for_billnow trial bill input flist before tax items",  trial_bill_in_flistp);
		//PIN_FLIST_CONCAT (trial_bill_in_flistp, item_flistp1, ebufp);
		//PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_input_for_billnow trial bill input flist after tax items",  trial_bill_in_flistp);
	
	
		PCM_OP (ctxp, PCM_OP_BILL_MAKE_BILL_NOW, 1, trial_bill_in_flistp, &trial_bill_ret_flistp, ebufp);

		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_create_input_for_billnow error", ebufp);
			PIN_FLIST_DESTROY_EX(&trial_bill_ret_flistp, NULL);
			*ret_flistpp = NULL;
			goto cleanup;
		}else
		{
			*ret_flistpp = PIN_FLIST_COPY(trial_bill_ret_flistp, ebufp);
		}
		PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_input_for_billnow: return_flistp", trial_bill_ret_flistp);
	}
cleanup:
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		//PIN_FLIST_DESTROY_EX(&item_flistp1, NULL);
		PIN_FLIST_DESTROY_EX(&item_obj_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&trial_bill_in_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&trial_bill_ret_flistp, NULL);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_create_input_for_billnow: end");
	return;
}

void mso_op_get_tax_items(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**tax_item_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*item_flistp = NULL;
	pin_flist_t		*evt_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*balimpact_flistp = NULL;
	pin_flist_t		*tax_item_flistp = NULL;
	int32			elemid = 0;
	pin_cookie_t		cookie = NULL;
	int32			elemid_1 = 0;
	pin_cookie_t		cookie_1 = NULL;
	void			*vp = NULL;
	int32			impact_type = 0;
	char			*rate_tag = NULL;
	char			*tax_code = NULL;
	int32			match_found = 0;


	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_get_tax_items: start");
	tax_item_flistp = PIN_FLIST_CREATE(ebufp);
	elemid = 0;
	cookie = NULL;
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_get_tax_items input flist", i_flistp);
	evt_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_EVENTS,
			&elemid, 0, &cookie,  ebufp);
	elemid_1 = 0;
	cookie_1 = NULL;

	
	while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(evt_flistp, PIN_FLD_RESULTS,
			&elemid_1, 1, &cookie_1,  ebufp)) != NULL) {

	elemid = 0;
	cookie = NULL;

	while((balimpact_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, PIN_FLD_BAL_IMPACTS,
		   &elemid, 1, &cookie, ebufp))!= NULL) 
	{
		vp = PIN_FLIST_FLD_GET(balimpact_flistp, PIN_FLD_IMPACT_TYPE, 1, ebufp);
		if (vp ) 
		{
			impact_type = *(int32 *)vp;

		}
		if (impact_type == PIN_IMPACT_TYPE_TAX) 
		{
			rate_tag = PIN_FLIST_FLD_GET(balimpact_flistp, PIN_FLD_RATE_TAG, 1, ebufp);
			tax_code = PIN_FLIST_FLD_GET(balimpact_flistp, PIN_FLD_TAX_CODE, 1, ebufp);
			if (strcmp(rate_tag, "Tax")==0) 
			{	match_found = 1;
				item_flistp = PIN_FLIST_ELEM_ADD (tax_item_flistp, PIN_FLD_ITEMS, elemid, ebufp);
				PIN_FLIST_FLD_COPY(balimpact_flistp, PIN_FLD_ITEM_OBJ, item_flistp, PIN_FLD_ITEM_OBJ, ebufp);
			}
		}
	}
	}
	
	if(match_found == 1)
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "match found");
		*tax_item_flistpp = PIN_FLIST_COPY(tax_item_flistp, ebufp);
	}
	else
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "match not found");
		*tax_item_flistpp = NULL;
	}

cleanup:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_get_tax_items: end");
	PIN_FLIST_DESTROY_EX(&tax_item_flistp, NULL);
	return;
}

void mso_op_get_tax_events(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*item_flistp,
	pin_flist_t		**tax_event_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t	*evt_i_flistp = NULL;
	pin_flist_t	*evt_o_flistp = NULL;
	pin_flist_t	*item_arrp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_get_tax_events: start");
	evt_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, evt_i_flistp, PIN_FLD_POID, ebufp);
	item_arrp = PIN_FLIST_ELEM_ADD(evt_i_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(item_flistp, PIN_FLD_ITEM_OBJ, item_arrp, PIN_FLD_ITEM_OBJ, ebufp);
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_get_tax_events: evt search input flist", evt_i_flistp);
	PCM_OP (ctxp, PCM_OP_BILL_ITEM_EVENT_SEARCH, 1, evt_i_flistp, &evt_o_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"mso_op_get_tax_events: "
		"error during event search",
		ebufp);
		*tax_event_flistpp =  NULL;
		goto cleanup;
	}

	*tax_event_flistpp = PIN_FLIST_COPY(evt_o_flistp, ebufp);

	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_get_tax_events: output flist:",*tax_event_flistpp);
cleanup:
	PIN_FLIST_DESTROY_EX(&evt_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&evt_o_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_get_tax_events: end");
	return;
}

void mso_op_process_invoicing(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	pin_flist_t	*bill_flistp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_cookie_t	r_cookie = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*inv_ret_flistp = NULL;
	pin_flist_t	*inv_input_flistp = NULL;
	pin_flist_t	*bill_det_flistp = NULL;
	pin_flist_t	*rd_iflistp = NULL;
	pin_flist_t	*rd_oflistp = NULL;
	pin_flist_t	*bulk_obj_flistp = NULL;
	poid_t		*acct_poidp = NULL;
	int		r_elemid = 0;	

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_process_invoicing: start");

	acct_poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
        if( ibill_type == 1 ){
                /***********************************
                 * get last bill obj from billinfo
                 ***********************************/
                rd_iflistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_COPY( bill_flistp, PIN_FLD_BILLINFO_OBJ, rd_iflistp, PIN_FLD_POID, ebufp );
                PIN_FLIST_FLD_SET( rd_iflistp, PIN_FLD_LAST_BILL_OBJ, 0, ebufp );
		PIN_FLIST_FLD_SET( rd_iflistp, PIN_FLD_BILL_OBJ, 0, ebufp );
                /***********************************
                 * Log input flist
                 **********************************/
                PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "so_op_process_invoicing: last bill obj read input flist", rd_iflistp );
                /**********************************
                 * Call read fields
                 *********************************/
                PCM_OP( ctxp, PCM_OP_READ_FLDS, 0, rd_iflistp, &rd_oflistp, ebufp );
                /*********************************
                 * Check Errors
                 ********************************/
                if( PIN_ERRBUF_IS_ERR(ebufp)){
                        PIN_ERR_LOG_EBUF( PIN_ERR_LEVEL_ERROR,"mso_op_process_invoicing: Last bill obj read error", ebufp );
                        goto cleanup;
                }
                PIN_ERR_LOG_FLIST( PIN_ERR_LEVEL_DEBUG, "mso_op_process_invoicing: Last bill obj read output", rd_oflistp );
                res_flistp = PIN_FLIST_ELEM_ADD( bill_flistp, PIN_FLD_RESULTS, 0, ebufp );
                PIN_FLIST_FLD_COPY( bill_flistp, PIN_FLD_BILLINFO_OBJ, res_flistp, PIN_FLD_BILLINFO_OBJ, ebufp );
                PIN_FLIST_FLD_COPY( rd_oflistp, PIN_FLD_LAST_BILL_OBJ, res_flistp, PIN_FLD_LAST_BILL_OBJ, ebufp );
                PIN_FLIST_DESTROY_EX( &rd_iflistp, NULL );
                PIN_FLIST_DESTROY_EX( &rd_oflistp, NULL );
        }


	if(bill_flistp != NULL && PIN_FLIST_ELEM_COUNT(bill_flistp, PIN_FLD_RESULTS, ebufp) > 0)
	{
		inv_input_flistp = PIN_FLIST_CREATE(ebufp);

		while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(bill_flistp, PIN_FLD_RESULTS,&r_elemid,1,&r_cookie,ebufp)) != NULL)
		{
			mso_op_read_bill_obj(ctxp, res_flistp, &bill_det_flistp, ebufp);
			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_LAST_BILL_OBJ, inv_input_flistp, PIN_FLD_POID, ebufp);

			PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_process_invoicing: inv input flist", inv_input_flistp);
			PCM_OP (ctxp, PCM_OP_INV_MAKE_INVOICE, 1, inv_input_flistp, &inv_ret_flistp, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"mso_op_process_invoicing: "
				"CREATE INVOICE error",
				ebufp);
				*ret_flistpp =  NULL;
				goto cleanup;
			}
			if(ibill_type == 1)
			{
				rd_iflistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY( inv_ret_flistp, PIN_FLD_POID, rd_iflistp, PIN_FLD_POID, ebufp );
				PCM_OP( ctxp, PCM_OP_READ_OBJ, 0, rd_iflistp, &rd_oflistp, ebufp );
				if( PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_process_invoicing: Invoice obj read error", ebufp );
					goto cleanup;
				}

				mso_op_create_trial_invoice_report(connp, ctxp, rd_oflistp, bill_det_flistp, &bulk_obj_flistp, ebufp);
				if( PIN_ERRBUF_IS_ERR(ebufp )){
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"mso_op_process_invoicing: "
						"Trial invoice object flist creation error", ebufp );
					goto cleanup;
				}

				PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_process_invoicing: Bulk object creation flist", bulk_obj_flistp );
				//mso_op_create_trialbillnow_invoice_object(connp, ctxp, inv_ret_flistp, acct_poidp, bill_det_flistp, ret_flistpp, ebufp);
				/*mso_op_create_trialbillnow_invoice_object(connp, ctxp, inv_ret_flistp, acct_poidp, bulk_obj_flistp, ret_flistpp, ebufp);
				
				if( PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
						"mso_op_process_invoicing: "
						"Trial invoice create obj error", ebufp );
					goto cleanup;
				}*/
				PIN_FLIST_DESTROY_EX(&bill_det_flistp, NULL);
				PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_process_invoicing: inv ret flist", inv_ret_flistp);
				//*ret_flistpp = inv_ret_flistp;
				*ret_flistpp = bulk_obj_flistp;
			}
			else
			{
				//mso_op_create_inv_doc_reference(ctxp, inv_ret_flistp, ebufp);
				*ret_flistpp = inv_ret_flistp;
			}
		}
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&inv_input_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&bill_det_flistp, NULL);
	//PIN_FLIST_DESTROY_EX(&bulk_obj_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_process_invoicing: end");
	return;
}




void mso_op_read_bill_obj(
        pcm_context_t   *ctxp,
	pin_flist_t     *input_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t    *ebufp)
{
	pin_flist_t	*robj_in_flistp = NULL;
	void		*vp = NULL;

	if (PIN_ERRBUF_IS_ERR(ebufp))
        {
                return;
        }
        PIN_ERRBUF_CLEAR(ebufp);

        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_read_bill_obj: start");
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_read_bill_obj: input flist ", input_flistp);
	robj_in_flistp = PIN_FLIST_CREATE(ebufp);

        vp = PIN_FLIST_FLD_GET(input_flistp, PIN_FLD_LAST_BILL_OBJ, 0, ebufp);
        PIN_FLIST_FLD_SET(robj_in_flistp, PIN_FLD_POID, vp, ebufp);
        PIN_FLIST_FLD_SET(robj_in_flistp, PIN_FLD_ACCOUNT_OBJ, NULL, ebufp);
        PIN_FLIST_FLD_SET(robj_in_flistp, PIN_FLD_BILL_NO, "", ebufp);
        PIN_FLIST_FLD_SET(robj_in_flistp, PIN_FLD_DUE, NULL, ebufp);
        PIN_FLIST_FLD_SET(robj_in_flistp, PIN_FLD_TOTAL_DUE, NULL, ebufp);
        PIN_FLIST_FLD_SET(robj_in_flistp, PIN_FLD_DUE_T, NULL, ebufp);

        PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, robj_in_flistp, ret_flistpp, ebufp);
        if (PIN_ERRBUF_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "mso_op_read_bill_obj: "
                        "READ_OBJ error",
                        ebufp);
                goto cleanup;
        }
        PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_read_bill_obj: robj_ret_flistp ", *ret_flistpp);
cleanup:
	PIN_FLIST_DESTROY_EX(&robj_in_flistp, NULL);
        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_read_bill_obj: end");
        return;

}

void mso_op_create_inv_doc_reference(
	pcm_context_t	*ctxp,
	pin_flist_t	*inv_flistp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*robj_in_flistp = NULL;
	pin_flist_t	*robj_ret_flistp = NULL;
	pin_flist_t	*create_obj_i_flist = NULL;
	poid_t		*doc_ref_poidp = NULL;
	void		*vp = NULL;
	char		file_path[2*BUFSIZ]="";
	char		temp[50]="";
	char		file_extn[10]=".pdf";
	int64		poid_id;
	poid_t		*pdp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_create_inv_doc_reference: start");
	
 	robj_in_flistp = PIN_FLIST_CREATE(ebufp);

	vp = (poid_t *)PIN_FLIST_FLD_GET(inv_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(robj_in_flistp, PIN_FLD_POID, (void *)vp, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, robj_in_flistp, &robj_ret_flistp, ebufp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_create_inv_doc_reference: "
			"READ_OBJ error",
			ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_inv_doc_reference: robj_ret_flistp ", robj_ret_flistp);

	create_obj_i_flist = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(robj_ret_flistp, PIN_FLD_BILL_OBJ, create_obj_i_flist, PIN_FLD_BILL_OBJ, ebufp);
	PIN_FLIST_FLD_COPY(robj_ret_flistp, PIN_FLD_BILL_NO, create_obj_i_flist, PIN_FLD_BILL_NO, ebufp);
	PIN_FLIST_FLD_COPY(robj_ret_flistp, PIN_FLD_ACCOUNT_OBJ, create_obj_i_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);	
	PIN_FLIST_FLD_COPY(robj_ret_flistp, PIN_FLD_POID, create_obj_i_flist, PIN_FLD_INVOICE_OBJ, ebufp);		
	PIN_FLIST_FLD_COPY(robj_ret_flistp, PIN_FLD_REPORT_NAME, create_obj_i_flist, PIN_FLD_REPORT_NAME, ebufp);
	PIN_FLIST_FLD_COPY(robj_ret_flistp, PIN_FLD_TEMPLATE_NAME, create_obj_i_flist, PIN_FLD_TEMPLATE_NAME, ebufp);
	PIN_FLIST_FLD_COPY(robj_ret_flistp, PIN_FLD_STATUS, create_obj_i_flist, PIN_FLD_STATUS, ebufp);
	poid_id = PIN_POID_GET_ID(vp);
	sprintf(temp,"%d",poid_id);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, temp);
	sprintf(file_path,"%s%s%s", "/test/path/inv/", temp, file_extn);
	PIN_FLIST_FLD_SET(create_obj_i_flist, PIN_FLD_PATH, file_path, ebufp);
	doc_ref_poidp = PIN_POID_CREATE(1, "/mso_inv_doc_reference", -1, ebufp);
	PIN_FLIST_FLD_PUT( create_obj_i_flist, PIN_FLD_POID, doc_ref_poidp, ebufp );	
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_inv_doc_reference: create obj flist ", create_obj_i_flist);
	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_obj_i_flist, &ret_flistp, ebufp);
	

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_create_inv_doc_reference: "
			"CREATE OBJ error",
			ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "create obj output ", ret_flistp);

cleanup:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_create_inv_doc_reference: end");
	PIN_FLIST_DESTROY_EX(&robj_in_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&robj_ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&create_obj_i_flist, NULL);
	PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return;

}
	

void mso_op_create_trialbillnow_invoice_object(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	poid_t		*acct_poidp,
	pin_flist_t	*bill_flistp,
	pin_errbuf_t	*ebufp)
{

	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*robj_in_flistp = NULL;
	pin_flist_t	*robj_ret_flistp = NULL;
	pin_flist_t	*create_obj_i_flistp = NULL;
	pin_flist_t	*create_obj_o_flistp = NULL;
	pin_flist_t	*values_flistp = NULL;
	pin_cookie_t	cookie = NULL;
	pcm_context_t	*new_ctxp = NULL;
	poid_t		*trial_inv_poidp = NULL;
	poid_t		*pdp = NULL;
	void		*vp = NULL;
	int		trans_id=0;
	int32		elemid = 0;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_create_trialbillnow_invoice_object: start");
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_trialbillnow_invoice_object: bill flist ", bill_flistp);

	PCM_CONTEXT_OPEN(&new_ctxp, (pin_flist_t *)0, ebufp);

//	trans_id = fm_utils_trans_open(new_ctxp, PCM_TRANS_OPEN_READWRITE, acct_poidp, ebufp);
	// Passing flag 3 while opening a transaction .
	trans_id = fm_utils_trans_open(new_ctxp, 3, acct_poidp, ebufp);

	if(PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_FLIST_LOG_ERR("mso_op_create_trialbillnow_invoice_object " 	"pcm_context_open error", ebufp);
		PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
		if( trans_id ) fm_utils_trans_close( new_ctxp, trans_id, ebufp );
		return;
	}
	
 	/*robj_in_flistp = PIN_FLIST_CREATE(ebufp);

	vp = PIN_FLIST_FLD_GET(inv_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(robj_in_flistp, PIN_FLD_POID, vp, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_OBJ, 0, robj_in_flistp, &robj_ret_flistp, ebufp);*/
	//create_obj_i_flistp = PIN_FLIST_COPY( bill_flistp, ebufp );
	/*create_obj_i_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY( bill_flistp, PIN_FLD_POID, create_obj_i_flistp, PIN_FLD_POID, ebufp );
	pdp = PIN_POID_CREATE( PIN_POID_GET_DB( acct_poidp ), "/mso_trial_report", -1, ebufp);
	PIN_FLIST_FLD_PUT( create_obj_i_flistp, PIN_FLD_POID, (void *)pdp, ebufp );
	PIN_FLIST_ELEM_COPY( bill_flistp, PIN_FLD_COMMON_VALUES, 0, create_obj_i_flistp, PIN_FLD_COMMON_VALUES, 0, ebufp );
	while( (values_flistp = PIN_FLIST_ELEM_GET_NEXT( bill_flistp, PIN_FLD_VALUES, &elemid, 1, &cookie, ebufp )) != NULL ){
		PIN_FLIST_ELEM_SET( create_obj_i_flistp, values_flistp, PIN_FLD_VALUES, elemid, ebufp );
	}*/
	PCM_OP(new_ctxp, PCM_OP_BULK_CREATE_OBJ, 0, bill_flistp, &create_obj_o_flistp, ebufp );
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_create_trialbillnow_invoice_object: "
			"CREATE_OBJ error",
			ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_trialbillnow_invoice_object: create_obj_o_flistp ", create_obj_o_flistp);


	/*  Do not need to create trial invoice.
	create_obj_i_flist = PIN_FLIST_CREATE(ebufp);
	create_obj_i_flist = PIN_FLIST_COPY(robj_ret_flistp, ebufp);
	trial_inv_poidp = PIN_POID_CREATE(1, "/invoice/trial_billnow", -1, ebufp);
	PIN_FLIST_FLD_PUT( create_obj_i_flist, PIN_FLD_POID, trial_inv_poidp, ebufp );	
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_trialbillnow_invoice_object: create obj flist ", create_obj_i_flist);
	PCM_OP(new_ctxp, PCM_OP_CREATE_OBJ, 0, create_obj_i_flist, &ret_flistp, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_create_trialbillnow_invoice_object: "
			"CREATE OBJ error",
			ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "create obj output ", ret_flistp);
	*/
	//mso_op_create_trial_invoice_report(connp, new_ctxp, robj_ret_flistp, bill_flistp, ebufp);
	//*ret_flistpp = ret_flistp;
	fm_mso_trans_commit(new_ctxp, acct_poidp, ebufp);
	//fm_utils_trans_commit(new_ctxp, ebufp );
	//if( trans_id ) fm_utils_trans_close( new_ctxp, trans_id, ebufp );
	PCM_CONTEXT_CLOSE(new_ctxp, 0, ebufp);
	//*ret_flistpp = PIN_FLIST_COPY(create_obj_o_flistp, ebufp);

cleanup:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_create_trialbillnow_invoice_object: end");
	//PIN_FLIST_DESTROY_EX(&create_obj_i_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&create_obj_o_flistp, NULL);
	return;

}

void
mso_op_create_trial_invoice_report(
	cm_nap_connection_t	*connp,
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*bill_flistp,
	pin_flist_t	**ret_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*create_obj_i_flist = NULL; 
	pin_flist_t	*ret_flistp = NULL; 
	pin_flist_t	*acct_details = NULL;
	pin_flist_t	*payinfo_details = NULL;
	pin_flist_t	*billinfo_out_flist = NULL;
	pin_flist_t	*payload_flistp = NULL;
	pin_flist_t	*formats_flistp = NULL;
	pin_flist_t	*stack_flistp=NULL;
	pin_flist_t	*temp_flistp = NULL;
	pin_flist_t	*temp2_flistp = NULL;
	pin_flist_t	*event_flistp = NULL;
	pin_flist_t	*common_values_flistp = NULL;
	pin_flist_t	*values_flistp = NULL;
	pin_flist_t	*total_flistp = NULL;
	int32		rec_id = 0;
	int32		rec_id2 = 0;
	int		pay_term = 0;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie2 = NULL;
	poid_t		*trial_billp = NULL;
	poid_t		*acct_poidp = NULL;
	poid_t		*srv_pdp = NULL;
	poid_t		*item_pdp = NULL;
	poid_t		*evt_pdp = NULL;
	poid_t		*prod_pdp = NULL;
	poid_t		*rerate_obj = NULL;
	poid_t		*act_obj = NULL;
	pin_buf_t	*nota_buf = NULL;
	void		*vp = NULL;
	time_t		*start_t = 0;
	time_t		*end_t = 0;
	time_t		*prod_start_t = NULL;
	time_t		*prod_end_t = NULL;
	char		*acc_no = NULL;
	char		*city_code = NULL;
	char		*item_type = NULL;
	char		*cmts = NULL;
	char		*plan_name = NULL;
	char		*plan_code = NULL;
	char		*prov_tag = NULL;
	char		*plan_type = NULL;
	char		*disc_type = NULL;
	char		*event_desc = NULL;
	char		*charge_type = NULL;
	char		*event_type = NULL;
	pin_decimal_t	*charge = NULL;
	pin_decimal_t	*discount = NULL;
	pin_decimal_t	*total_charge = NULL;
	pin_decimal_t	*disc_rate = NULL;
	int32		elemid = 1;
	int64		db = 0;
	int		payterm = 0;

	if(PIN_ERRBUF_IS_ERR(ebufp)) { return;}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_create_trial_invoice_report: start.");
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_trial_invoice_report input", i_flistp);
	formats_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_FORMATS, 0, 0, ebufp);
	if(formats_flistp)
	{
		nota_buf = (pin_buf_t *)PIN_FLIST_FLD_GET(formats_flistp,
			PIN_FLD_BUFFER, 0, ebufp);
	}
	if(nota_buf){
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, (char *)nota_buf->data);
		PIN_STR_TO_FLIST((char *)nota_buf->data, 1, &payload_flistp, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"mso_op_create_trial_invoice_report error: Error", ebufp);
			goto cleanup;
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
			"mso_op_create_trial_invoice_report: Payload Flist", payload_flistp);
	}
	if(PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_create_trial_invoice_report: Error", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_trial_invoice_report bill flist", bill_flistp);
	
	//Get all the values
	temp_flistp = PIN_FLIST_ELEM_GET(payload_flistp, PIN_FLD_BILLINFO, 0, 0, ebufp);
	if(temp_flistp)
	{
		//Get the start and end time
		start_t = (time_t *)PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_START_T, 0, ebufp);
		end_t = (time_t *)PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_END_T, 0, ebufp);
	}
	//Get the Account No
	temp_flistp = PIN_FLIST_ELEM_GET(payload_flistp, PIN_FLD_ACCTINFO, 0, 0, ebufp);
	if(temp_flistp)
	{
		acc_no = (char *)PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_ACCOUNT_NO, 0, ebufp);
		acct_poidp = (poid_t *)PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	}
	//Get the city code
	fm_mso_get_city_code(ctxp, acct_poidp, &city_code, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_create_trial_invoice_report: Error", ebufp);
		return;
	}
	//Iterate through items array
	while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (payload_flistp,
			PIN_FLD_AR_ITEMS, &rec_id, 1,&cookie, ebufp))
			!= (pin_flist_t *)NULL )
	{
		item_pdp = (poid_t *)PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_ITEM_OBJ, 0, ebufp);
		if(!PIN_POID_IS_NULL(item_pdp))
			item_type = (char *)PIN_POID_GET_TYPE(item_pdp);
		if(db == 0 ){
			db = PIN_POID_GET_DB( item_pdp );
		}
		if(PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"mso_op_create_trial_invoice_report: Error", ebufp);
			return;
		}
		if(strstr(item_type, "/item/cycle") ||
			strstr(item_type, "/item/purchase") ||
			strstr(item_type, "/item/VAT") ||
			strstr(item_type, "/item/HW_Service_Tax") ||
			strstr(item_type, "/item/HW_Secondary_Higher_Education_Cess") ||
			strstr(item_type, "/item/HW_Education_Cess") ||
			strstr(item_type, "/item/Service_Tax") ||
			strstr(item_type, "/item/Secondary_Higher_Education_Cess") ||
			strstr(item_type, "/item/Education_Cess") ||
			strstr(item_type, "/item/mso_usage") ||
			strstr(item_type, "/item/mso_hw_deposit"))
		{
			//Get the service poid
			if( srv_pdp == NULL ){
				srv_pdp = (poid_t *)PIN_FLIST_FLD_GET(temp_flistp, 
					PIN_FLD_SERVICE_OBJ, 0, ebufp);
				//get the CMTS
				fm_mso_get_cmts(ctxp, srv_pdp, &cmts, &payterm, ebufp);
			}
			//Get the event details
			if(strcmp(item_type, "/item/mso_usage") != 0)
			{
				//Get all the events for the item
				fm_mso_get_event_flist(ctxp, item_pdp, &event_flistp, ebufp);
				if(PIN_ERRBUF_IS_ERR(ebufp)) 
				{
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
						"mso_op_create_trial_invoice_report: Error", ebufp);
					return;
				}
				//Itrate through each event and create the trial report object
				rec_id2 = 0;
				cookie2 = NULL;
				while ( (temp2_flistp = PIN_FLIST_ELEM_GET_NEXT (event_flistp,
					PIN_FLD_RESULTS, &rec_id2, 1,&cookie2, ebufp))
					!= (pin_flist_t *)NULL )
				{
					charge = NULL;
					rerate_obj = PIN_FLIST_FLD_GET( temp2_flistp, PIN_FLD_RERATE_OBJ,
								1, ebufp );
					if( rerate_obj && !PIN_POID_IS_NULL(rerate_obj)){
						continue;
					}
					evt_pdp = (poid_t *)PIN_FLIST_FLD_GET(temp2_flistp, 
						PIN_FLD_POID, 0, ebufp);
					//Get the Event Details
					fm_mso_get_event_details(ctxp, temp2_flistp, &charge, 
						&discount, &event_desc, ebufp);
					if( charge == NULL ){
						continue;
					}
					//Get event type
					event_type = (char *)PIN_POID_GET_TYPE(evt_pdp);
					//get payterm
					/*if( event_type && strcmp(event_type, "/event/billing/product/fee/purchase") &&
							strcmp(event_type,"/event/billing/mso_cr_dr_note")){
						fm_mso_get_payterm(ctxp, srv_pdp, evt_pdp,
							&pay_term, ebufp);
					}*/
					//get product details
					fm_mso_get_prod_details(ctxp,temp2_flistp, acct_poidp, srv_pdp, &plan_name, &plan_code, 
						&prov_tag, &plan_type, &disc_type, &prod_pdp, &disc_rate, ebufp); 
				
					if(PIN_ERRBUF_IS_ERR(ebufp)) 
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
							"mso_op_create_trial_invoice_report: Error", ebufp);
						return;
					}
					total_charge = pbo_decimal_add(charge, discount, ebufp);

					if( event_type && strcmp(event_type, "/event/billing/product/fee/purchase") &&
							strcmp(event_type,"/event/billing/mso_cr_dr_note")){
						if( plan_type && !strcmp(plan_type,"ADVANCE") &&
							item_type && !strstr(item_type,"/item/cycle_forward/mso_hw_rental")){
							pay_term = payterm;
						}
						else{
							fm_mso_get_payterm(ctxp, srv_pdp, evt_pdp,
								&pay_term, ebufp);
						}
					}

					if( event_type && !strcmp(event_type, "/event/billing/product/fee/purchase")){
						prod_start_t = PIN_FLIST_FLD_GET( temp2_flistp, PIN_FLD_START_T, 0, ebufp );
						prod_end_t = PIN_FLIST_FLD_GET( temp2_flistp, PIN_FLD_END_T, 0, ebufp );
					}
					else{
						prod_start_t = PIN_FLIST_FLD_GET( temp2_flistp, PIN_FLD_EARNED_START_T, 0, ebufp );
						prod_end_t = PIN_FLIST_FLD_GET( temp2_flistp, PIN_FLD_EARNED_END_T, 0, ebufp );
					}
					
					//Do the create obj
					if( create_obj_i_flist == NULL ){
						create_obj_i_flist = PIN_FLIST_CREATE(ebufp);
						trial_billp = PIN_POID_CREATE(db, "/mso_trial_report", -1, ebufp);
						PIN_FLIST_FLD_PUT(create_obj_i_flist, PIN_FLD_POID, (void *)trial_billp, ebufp);
						common_values_flistp = PIN_FLIST_ELEM_ADD( create_obj_i_flist, PIN_FLD_COMMON_VALUES, 0, ebufp );
						//Set the account no
						PIN_FLIST_FLD_SET(common_values_flistp, PIN_FLD_ACCOUNT_NO, (void *)acc_no, ebufp);
						//Put the city code
						if(city_code)
						PIN_FLIST_FLD_PUT(common_values_flistp, MSO_FLD_CITY_CODE, (void *)city_code, ebufp);
						//Put the CMTS
						if(cmts)
						PIN_FLIST_FLD_PUT(common_values_flistp, MSO_FLD_CMTS_ID, (void *)cmts, ebufp);
						//Set the start date
						//PIN_FLIST_FLD_SET(common_values_flistp, PIN_FLD_START_T, (void *)start_t, ebufp);
						//Set the end date
						//PIN_FLIST_FLD_SET(common_values_flistp, PIN_FLD_END_T, (void *)end_t, ebufp);
						act_obj = PIN_POID_CREATE( db, "/account", 1, ebufp );
						PIN_FLIST_FLD_PUT( common_values_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)act_obj, ebufp );
					}
					if( create_obj_i_flist ){
						values_flistp = PIN_FLIST_ELEM_ADD( create_obj_i_flist, PIN_FLD_VALUES, elemid, ebufp );
						elemid++;
						//Set the payterm
						PIN_FLIST_FLD_SET(values_flistp, PIN_FLD_PAYMENT_TERM, (void *)&pay_term, ebufp);
						//Set the plan type
						if(plan_type)
						PIN_FLIST_FLD_PUT(values_flistp, MSO_FLD_PACKAGE_TYPE, (void *)plan_type, ebufp);
						//Set the charge
						PIN_FLIST_FLD_SET(values_flistp, PIN_FLD_CHARGE_AMT, (void *)total_charge, ebufp);
						//Set the discount
						PIN_FLIST_FLD_PUT(values_flistp, PIN_FLD_DISCOUNT, (void *)discount, ebufp);
						//Set the total charge
						PIN_FLIST_FLD_PUT(values_flistp, MSO_FLD_TOTAL_CHARGE, (void *)charge, ebufp);
						//Set the discount type
						if(disc_type)
						PIN_FLIST_FLD_PUT(values_flistp, MSO_FLD_DISCOUNT_TYPE, (void *)disc_type, ebufp);
						//Set the discount rate
						if(disc_rate)
						PIN_FLIST_FLD_PUT(values_flistp, MSO_FLD_DISCOUNT_RATE, (void *)disc_rate, ebufp);
						//Set the Charge Code
						PIN_FLIST_FLD_PUT(values_flistp, MSO_FLD_CHARGE_CODE_ID, (void *)item_type, ebufp);
						//Set the Event Descr
						PIN_FLIST_FLD_PUT(values_flistp, MSO_FLD_CHARGE_DESCR, (void *)event_desc, ebufp);
						//Set the Pricing Entity
						if(prod_pdp)
						PIN_FLIST_FLD_PUT(values_flistp, PIN_FLD_PRODUCT_OBJ, (void *)prod_pdp, ebufp);
						//Set the Package Code
						if(plan_code)
						PIN_FLIST_FLD_PUT(values_flistp, PIN_FLD_CODE, (void *)plan_code, ebufp);
						//Set the Package Name
						if(plan_name)
						PIN_FLIST_FLD_PUT(values_flistp, PIN_FLD_NAME, (void *)plan_name, ebufp);
						//Set the Charge ID
						PIN_FLIST_FLD_SET(values_flistp, PIN_FLD_EVENT_OBJ, (void *)evt_pdp, ebufp);
						PIN_FLIST_FLD_SET(values_flistp, PIN_FLD_START_T, (void *)prod_start_t, ebufp);
						PIN_FLIST_FLD_SET(values_flistp, PIN_FLD_END_T, (void *)prod_end_t, ebufp);
						//Set the Charge Type R - recurring/N - non-recurring/U - usage
						if(strstr(item_type, "/item/cycle")){
							PIN_FLIST_FLD_SET(values_flistp, MSO_FLD_CHARGE_TYPE, (void *)"R", ebufp);
						}
						else {
							PIN_FLIST_FLD_SET(values_flistp, MSO_FLD_CHARGE_TYPE, (void *)"N", ebufp);
						}
						//Set the Service Code
						if(prov_tag)
						PIN_FLIST_FLD_PUT(values_flistp, PIN_FLD_SERVICE_CODE, (void *)prov_tag, ebufp);
						if(PIN_ERRBUF_IS_ERR(ebufp)) 
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_create_trial_invoice_report: Error", ebufp);
							goto cleanup;
						}
						//Log the input flist
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
							"mso_op_create_trial_invoice_report: Create Obj Input", create_obj_i_flist);
						//Call the PCM_OP_CREATE_OBJ opcode
						/*PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_obj_i_flist, &ret_flistp, ebufp);
						if (PIN_ERRBUF_IS_ERR(ebufp)) {
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
								"mso_op_create_trial_invoice_report: CREATE OBJ error", ebufp);
							goto cleanup;
						}
						PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_trial_invoice_report ", ret_flistp);*/
					}
				}
			}
			else
			{
				//start_t = PIN_FLIST_FLD_GET( temp2_flistp, PIN_FLD_START_T, 0, ebufp );
				//end_t = PIN_FLIST_FLD_GET( temp2_flistp, PIN_FLD_END_T, 0, ebufp );
				if( create_obj_i_flist == NULL ){
					//Do the create obj for usage item
					create_obj_i_flist = PIN_FLIST_CREATE(ebufp);
					trial_billp = PIN_POID_CREATE(db, "/mso_trial_report", -1, ebufp);
					PIN_FLIST_FLD_PUT(create_obj_i_flist, PIN_FLD_POID, (void *)trial_billp, ebufp);
					common_values_flistp = PIN_FLIST_ELEM_ADD( create_obj_i_flist, PIN_FLD_COMMON_VALUES, 0, ebufp );
					//Set the account no
					PIN_FLIST_FLD_SET(common_values_flistp, PIN_FLD_ACCOUNT_NO, (void *)acc_no, ebufp);
					//Put the city code
					PIN_FLIST_FLD_PUT(common_values_flistp, MSO_FLD_CITY_CODE, (void *)city_code, ebufp);
					//Put the CMTS
					PIN_FLIST_FLD_PUT(common_values_flistp, MSO_FLD_CMTS_ID, (void *)cmts, ebufp);
					//Set the start date
					//PIN_FLIST_FLD_SET(common_values_flistp, PIN_FLD_START_T, (void *)start_t, ebufp);
					//Set the end date
					//PIN_FLIST_FLD_SET(common_values_flistp, PIN_FLD_END_T, (void *)end_t, ebufp);
					act_obj = PIN_POID_CREATE( db, "/account", 1, ebufp );
					PIN_FLIST_FLD_PUT( common_values_flistp, PIN_FLD_ACCOUNT_OBJ, (void *)act_obj, ebufp );
				}
				if( create_obj_i_flist ){
					values_flistp = PIN_FLIST_ELEM_ADD( create_obj_i_flist, PIN_FLD_VALUES, elemid, ebufp );
					elemid++;
					//Set the payterm
					//PIN_FLIST_FLD_PUT(create_obj_i_flist, PIN_FLD_PAYMENT_TERM, (void *)pay_term, ebufp);
					//Set the plan type
					//PIN_FLIST_FLD_PUT(create_obj_i_flist, MSO_FLD_PACKAGE_TYPE, (void *)plan_type, ebufp);
					//Set the charge
					//PIN_FLIST_FLD_SET(create_obj_i_flist, PIN_FLD_CHARGE_AMT, (void *)total_charge, ebufp);
					//Set the discount
					//PIN_FLIST_FLD_PUT(create_obj_i_flist, PIN_FLD_DISCOUNT, (void *)discount, ebufp);
					//Set the total charge
					//PIN_FLIST_FLD_PUT(create_obj_i_flist, MSO_FLD_TOTAL_CHARGE, (void *)charge, ebufp);
					//Set the discount type
					//PIN_FLIST_FLD_PUT(create_obj_i_flist, MSO_FLD_DISCOUNT_TYPE, (void *)disc_type, ebufp);
					//Set the discount rate
					//PIN_FLIST_FLD_SET(create_obj_i_flist, MSO_FLD_DISCOUNT_RATE, (void *)disc_rate, ebufp);
					PIN_FLIST_FLD_SET(values_flistp, PIN_FLD_START_T, (void *)start_t, ebufp);
					PIN_FLIST_FLD_SET(values_flistp, PIN_FLD_END_T, (void *)end_t, ebufp);
					//Set the Charge Code
					PIN_FLIST_FLD_PUT(values_flistp, MSO_FLD_CHARGE_CODE_ID, (void *)item_type, ebufp);
					//Set the Event Descr
					//PIN_FLIST_FLD_PUT(create_obj_i_flist, MSO_FLD_CHARGE_DESCR, (void *)event_desc, ebufp);
					//Set the Pricing Entity
					if(prod_pdp)
					PIN_FLIST_FLD_PUT(values_flistp, PIN_FLD_PRODUCT_OBJ, (void *)prod_pdp, ebufp);
					//Set the Package Code
					if(plan_code)
					PIN_FLIST_FLD_PUT(values_flistp, PIN_FLD_CODE, (void *)plan_code, ebufp);
					//Set the Package Name
					if(plan_name)
					PIN_FLIST_FLD_PUT(values_flistp, PIN_FLD_NAME, (void *)plan_name, ebufp);
					//Set the Charge ID
					//PIN_FLIST_FLD_SET(create_obj_i_flist, PIN_FLD_EVENT_OBJ, (void *)evt_pdp, ebufp);
					//Set the Charge Type R - recurring/N - non-recurring/U - usage
					PIN_FLIST_FLD_SET(values_flistp, MSO_FLD_CHARGE_TYPE, (void *)"U", ebufp);
					//Set the Service Code
					if(prov_tag)
					PIN_FLIST_FLD_PUT(values_flistp, PIN_FLD_SERVICE_CODE, (void *)prov_tag, ebufp);
					if(PIN_ERRBUF_IS_ERR(ebufp)) 
					{
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_create_trial_invoice_report: Error", ebufp);
						goto cleanup;
					}
					//Log the input flist
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
						"mso_op_create_trial_invoice_report: Create Obj Input", create_obj_i_flist);
					//Call the PCM_OP_CREATE_OBJ opcode
					/*PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_obj_i_flist, &ret_flistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp)) {
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
							"mso_op_create_trial_invoice_report: CREATE OBJ error", ebufp);
						goto cleanup;
					}*/
				}
			}
		}
	}
	if( create_obj_i_flist ){
		*ret_flistpp = create_obj_i_flist;
	}
//	if(PIN_ERRBUF_IS_ERR(ebufp)) 
//	{
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_create_trial_invoice_report: Error", ebufp);
//		return;
//	}
//	create_obj_i_flist = PIN_FLIST_CREATE(ebufp);
//	trial_billp = PIN_POID_CREATE(1, "/mso_trial_report", -1, ebufp);
//	PIN_FLIST_FLD_PUT( create_obj_i_flist, PIN_FLD_POID, trial_billp, ebufp );
//	PIN_FLIST_FLD_COPY(bill_flistp, PIN_FLD_TOTAL_DUE, create_obj_i_flist, PIN_FLD_AMOUNT, ebufp);
//	PIN_FLIST_FLD_COPY(bill_flistp, PIN_FLD_ACCOUNT_OBJ, create_obj_i_flist, PIN_FLD_ACCOUNT_OBJ, ebufp);
//	PIN_FLIST_FLD_COPY(bill_flistp, PIN_FLD_START_T, create_obj_i_flist, PIN_FLD_START_T, ebufp);
//	PIN_FLIST_FLD_COPY(bill_flistp, PIN_FLD_END_T, create_obj_i_flist, PIN_FLD_END_T, ebufp);
//	acct_poidp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
//	fm_mso_get_account_info(ctxp, acct_poidp, &acct_details, ebufp );
//	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_trial_invoice_report account_details", acct_details);
//	PIN_FLIST_FLD_COPY(acct_details, PIN_FLD_ACCOUNT_NO, create_obj_i_flist, PIN_FLD_ACCOUNT_NO, ebufp ); 
//	 fm_mso_read_billinfo(ctxp, (poid_t*)PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_AR_BILLINFO_OBJ, 0, ebufp), &billinfo_out_flist, ebufp);
//	 vp = PIN_FLIST_FLD_GET(billinfo_out_flist, PIN_FLD_BILL_WHEN, 0, ebufp);
//	 PIN_FLIST_FLD_SET(create_obj_i_flist, PIN_FLD_PAYMENT_TERM, vp, ebufp);
//	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_trial_invoice_report: create obj flist ", create_obj_i_flist);
//	PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_obj_i_flist, &ret_flistp, ebufp);
//	if (PIN_ERRBUF_IS_ERR(ebufp)) {
//		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
//			"mso_op_create_trial_invoice_report: "
//			"CREATE OBJ error",
//			ebufp);
//		goto cleanup;
//	}
//	PIN_ERR_LOG_FLIST (PIN_ERR_LEVEL_DEBUG, "mso_op_create_trial_invoice_report ", ret_flistp);
//	*ret_flistpp = ret_flistp;

cleanup:
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_create_trial_invoice_report: end.");
	PIN_FLIST_DESTROY_EX(&event_flistp, NULL);
	//PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
	return;
}

void
mso_op_open_transaction(
	pcm_context_t	*ctxp,
	int		flag,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*tran_flistp = NULL;
	poid_t		*tran_pdp = NULL;
	pin_flist_t	*tran_ret_flistp = NULL;

	if(PIN_ERRBUF_IS_ERR(ebufp)) { return;}
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_open_transaction: start.");

	tran_flistp = PIN_FLIST_CREATE(ebufp);
	tran_pdp = PIN_POID_CREATE(1, "/trans", -1, ebufp);
	PIN_FLIST_FLD_PUT(tran_flistp, PIN_FLD_POID, (void*)tran_pdp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_open_transaction: PCM_OP_TRANS_OPEN input  flist", tran_flistp);
	PCM_OP(ctxp, PCM_OP_TRANS_OPEN, PCM_TRANS_OPEN_READWRITE, tran_flistp, &tran_ret_flistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_open_transaction: Error after calling PCM_OP_TRANS_OPEN.", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_open_transaction: PCM_OP_TRANS_OPEN output flist", tran_ret_flistp);

	switch(flag)
	{
	case 1:
		if( ( PIN_ERRBUF_IS_ERR(ebufp) ) && (ebufp->pin_err == PIN_ERR_TRANS_ALREADY_OPEN) )
		{ PIN_ERRBUF_CLEAR(ebufp); }
		break;
	}

cleanup:
	//PIN_FLIST_DESTROY_EX(&tran_ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&tran_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_open_transaction: end.");
}

void
mso_op_abort_transaction(
	pcm_context_t		*ctxp,
	int			flag,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*tran_flistp = NULL;
	poid_t			*tran_pdp = NULL;
	pin_flist_t		*tran_ret_flistp = NULL;

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_abort_transaction: ebuf has error, clearing ebuf to continue", ebufp);
		PIN_ERRBUF_RESET(ebufp);
	}

	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_abort_transaction: start");

	PIN_ERRBUF_CLEAR(ebufp);
	tran_flistp = PIN_FLIST_CREATE(ebufp);
	tran_pdp = PIN_POID_CREATE(1, "/trans", -1, ebufp);
	PIN_FLIST_FLD_PUT(tran_flistp, PIN_FLD_POID, (void*)tran_pdp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_abort_transaction: PCM_OP_TRANS_ABORT input  flist", tran_flistp);
	PCM_OP(ctxp, PCM_OP_TRANS_ABORT, 0, tran_flistp, &tran_ret_flistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_abort_transaction: Error after calling PCM_OP_TRANS_ABORT.", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_abort_transaction: PCM_OP_TRANS_ABORT output flist", tran_ret_flistp);

	switch(flag)
	{
		case 1:
		if( ( PIN_ERRBUF_IS_ERR(ebufp) ) && (ebufp->pin_err == PIN_ERR_TRANS_ALREADY_OPEN) )
		{ PIN_ERRBUF_CLEAR(ebufp); }
		break;
	}

cleanup:
	PIN_FLIST_DESTROY_EX(&tran_ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&tran_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_abort_transaction: end");
}

void
mso_op_commit_transaction(
	pcm_context_t		*ctxp,
	int			flag,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*tran_flistp = NULL;
	pin_flist_t		*tran_ret_flistp = NULL;
	poid_t			*tran_pdp = NULL;

	if(PIN_ERRBUF_IS_ERR(ebufp)) { return; }
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_commit_transaction: start");

	tran_flistp = PIN_FLIST_CREATE(ebufp);
	tran_pdp = PIN_POID_CREATE(1, "/trans", -1, ebufp);
	PIN_FLIST_FLD_PUT(tran_flistp, PIN_FLD_POID, (void*)tran_pdp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_commit_transaction: PCM_OP_TRANS_COMMIT input  flist", tran_flistp);
	PCM_OP(ctxp, PCM_OP_TRANS_COMMIT, 0, tran_flistp, &tran_ret_flistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_commit_transaction: Error after calling PCM_OP_TRANS_COMMIT.", ebufp);
		goto cleanup;
	}

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_commit_transaction: PCM_OP_TRANS_COMMIT output flist", tran_ret_flistp);

	switch(flag)
	{
		case 1:
		if( ( PIN_ERRBUF_IS_ERR(ebufp) ) && (ebufp->pin_err == PIN_ERR_TRANS_NOT_OPEN) )
		{ PIN_ERRBUF_CLEAR(ebufp); }
		break;
	};

cleanup:
	PIN_FLIST_DESTROY_EX(&tran_ret_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&tran_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_commit_transaction: end");
}

/***********************************************************************************
*   mso_op_get_payinfo_flist
*************************************************************************************/
void
mso_op_get_payinfo_flist(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	poid_t			*bi_poidp,
	pin_flist_t		**payinfo_flistpp,
	pin_errbuf_t		*ebufp)
{	
	poid_t			*account_poidp = NULL;
	poid_t			*s_pdp = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*arg_flistp = NULL;
	int32			cnt;
	int32			s_flags = 256;
	pin_flist_t		*res_flistp = NULL;
	
	if(PIN_ERRBUF_IS_ERR(ebufp))	{ return; }
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_get_payinfo_flist: start");


	account_poidp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);

	s_flistp = PIN_FLIST_CREATE(ebufp);

	s_pdp = PIN_POID_CREATE(1, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, (void *)&s_flags, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, "select X from /payinfo 1 where 1.F1 = V1 ", ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, (void *)account_poidp, ebufp);

	arg_flistp = PIN_FLIST_ELEM_ADD( s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(arg_flistp, PIN_FLD_POID, NULL, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_get_payinfo_flist: search input flist:",s_flistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_get_payinfo_flist: search output flist:",r_flistp);

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "mso_op_get_payinfo_flist: Error searching the service !!", ebufp);
		goto cleanup;
	}

	cnt = PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp); 
	
	if ( cnt == 0 ) { *payinfo_flistpp = NULL; goto cleanup;	}
	if ( cnt >  1 ) { *payinfo_flistpp = NULL; goto cleanup;	}

	res_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);

	*payinfo_flistpp = PIN_FLIST_COPY(res_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_get_payinfo_flist: *payinfo_flistpp:", *payinfo_flistpp); 
cleanup:
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "mso_op_get_payinfo_flist: end");
}

static void
fm_mso_get_city_code(
	pcm_context_t		*ctxp, 
	poid_t			*acc_pdp, 
	char			**city_code, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*robj_oflistp = NULL;
	char			*area = NULL;
	char			*temp = NULL;
	char			*city = NULL;

	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;

	//Create the input flist
	robj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID, (void *)acc_pdp, ebufp);
	PIN_FLIST_FLD_SET(robj_iflistp, MSO_FLD_AREA, (void *)NULL, ebufp);
	//Cal the read fields
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflistp, &robj_oflistp, ebufp);

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_get_city_code: Read fields failed", ebufp);
		PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&robj_oflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_city_code: Read fields output", robj_oflistp);
	area = (char *)PIN_FLIST_FLD_GET(robj_oflistp, MSO_FLD_AREA, 0, ebufp);
	if(area)
	{
		temp = strdup(area);
		city = strtok(temp, "|");
		city = strtok(temp, "|");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "City Code");
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, city);
		*city_code = strdup(city);
		free(temp);
	}
	PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&robj_oflistp, NULL);
	return;
}

static void
fm_mso_get_cmts(
	pcm_context_t		*ctxp, 
	poid_t			*srv_pdp, 
	char			**cmts,
	int			*payterm,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*robj_iflistp = NULL;
	pin_flist_t		*robj_oflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;

	void			*vp = NULL;

	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;

	//Create the input flist
	robj_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(robj_iflistp, PIN_FLD_POID, (void *)srv_pdp, ebufp);
	temp_flistp = PIN_FLIST_SUBSTR_ADD(robj_iflistp, MSO_FLD_BB_INFO, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, MSO_FLD_NETWORK_ELEMENT, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_BILL_WHEN, (void *)NULL, ebufp );
	//Cal the read fields
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, robj_iflistp, &robj_oflistp, ebufp);

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"fm_mso_get_cmts: Read fields failed", ebufp);
		PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&robj_oflistp, NULL);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_cmts: Read fields output", robj_oflistp);
	temp_flistp = PIN_FLIST_SUBSTR_GET(robj_oflistp, MSO_FLD_BB_INFO, 0, ebufp);
	*cmts = (char *)PIN_FLIST_FLD_TAKE(temp_flistp, MSO_FLD_NETWORK_ELEMENT, 0, ebufp);
	vp = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_BILL_WHEN, 0, ebufp );
	if(vp){
		*payterm = *(int *)vp;
	}
	PIN_FLIST_DESTROY_EX(&robj_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&robj_oflistp, NULL);
	return;
}

static void
fm_mso_get_payterm(
	pcm_context_t		*ctxp, 
	poid_t			*srv_pdp, 
	poid_t			*evt_pdp, 
	int			*pay_term, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*src_iflistp = NULL;
	pin_flist_t		*src_oflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*temp2_flistp = NULL;
	pin_flist_t		*temp3_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int64			db = 0;
	int32			flag = SRCH_EXACT;
	char			*templ = "select X from /config/event_map where F1 = V1 and F2 = V2 ";
	char			*srv_typ = (char *)PIN_POID_GET_TYPE(srv_pdp);
	char			*evt_typ = (char *)PIN_POID_GET_TYPE(evt_pdp);
	void			*vp = NULL;
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_payterm: Event Type");
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, evt_typ);
	PIN_ERR_LOG_POID(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_payterm: Event Poid", evt_pdp);
	db = PIN_POID_GET_DB(srv_pdp);
	src_iflistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(src_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(src_iflistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	PIN_FLIST_FLD_SET(src_iflistp, PIN_FLD_TEMPLATE, (void *)templ, ebufp);
	//Add the 1st arg
	args_flistp = PIN_FLIST_ELEM_ADD(src_iflistp, PIN_FLD_ARGS, 1, ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_EVENT_MAP, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_PERMITTED, (void *)srv_typ, ebufp);
	//Add the 2nd arg
	args_flistp = PIN_FLIST_ELEM_ADD(src_iflistp, PIN_FLD_ARGS, 2, ebufp);
	temp3_flistp = PIN_FLIST_ELEM_ADD(args_flistp, PIN_FLD_EVENT_MAP, PIN_ELEMID_ANY, ebufp);
	temp2_flistp = PIN_FLIST_ELEM_ADD(temp3_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(temp2_flistp, PIN_FLD_EVENT_TYPE, (void *)evt_typ, ebufp);
	//Add the Result Array
	res_flistp = PIN_FLIST_ELEM_ADD(src_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	temp_flistp = PIN_FLIST_ELEM_ADD(res_flistp, PIN_FLD_EVENT_MAP, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_PERMITTED, (void *)NULL, ebufp);
	temp2_flistp = PIN_FLIST_ELEM_ADD(temp_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(temp2_flistp, PIN_FLD_EVENT_TYPE, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(temp2_flistp, PIN_FLD_COUNT, (void *)NULL, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_payterm: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_payterm: Search Input Flist", src_iflistp);
	//Call the PCM_OP_SEARCH
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, src_iflistp, &src_oflistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_payterm: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_payterm: Search Output Flist", src_oflistp);
	res_flistp = PIN_FLIST_ELEM_GET(src_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	if(res_flistp)
	{
		temp_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_EVENT_MAP, 
			PIN_ELEMID_ANY, 0, ebufp);
		if(temp_flistp)
		{
			temp2_flistp = PIN_FLIST_ELEM_GET(temp_flistp, PIN_FLD_EVENTS, 
				PIN_ELEMID_ANY, 0, ebufp);
			if(temp2_flistp)
			{
				vp = (int *)PIN_FLIST_FLD_GET(temp2_flistp,
					PIN_FLD_COUNT, 0, ebufp);
				if(vp){
					*pay_term = *(int *)vp;
				}
			}
		}
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_payterm: Error", ebufp);
		goto cleanup;
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&src_oflistp, NULL);
	PIN_FLIST_DESTROY_EX(&src_iflistp, NULL);
	return;
}

static void
fm_mso_get_prod_details(
	pcm_context_t		*ctxp, 
	pin_flist_t		*event_flistp,
	poid_t			*acc_pdp, 
	poid_t			*srv_pdp, 
	char			**plan_name, 
	char			**plan_code, 
	char			**prov_tag, 
	char			**plan_type, 
	char			**disc_type, 
	poid_t			**prod_pdp,
	pin_decimal_t		**disc_rate, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*src_iflistp = NULL;
	pin_flist_t		*src_oflistp = NULL;
	pin_flist_t		*pt_src_iflistp = NULL;
	pin_flist_t		*pt_src_oflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*pp_read_flistp = NULL;
	pin_flist_t		*pp_read_oflistp = NULL;
	pin_flist_t		*prod_read_flistp = NULL;
	pin_flist_t		*prod_read_oflistp = NULL;
	poid_t			*s_pdp = NULL;
	poid_t			*pp_obj = NULL;
	poid_t			*prod_obj = NULL;
	poid_t			*plan_pdp = NULL;
	int64			db = 0;
	int32			flag = SRCH_DISTINCT;
	char			*templ = "select X from /plan 1, /purchased_product 2 where 2.F1 = V1 and 2.F2 = 1.F3 ";
	char			*pt_templ = "select X from /mso_config_bb_pt where F1 = V1 ";
	char			*per = "P";
	char			*flat = "F";
	char			*none = "None";
	pin_decimal_t		*disc_per = NULL;
	pin_decimal_t		*disc_amt = NULL;
	int32			rec_id = 0;
	pin_cookie_t		cookie = NULL;
	int32			indicator = 0;
	char			*svc_code = NULL;

	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;

	db = PIN_POID_GET_DB(srv_pdp);
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);

	while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (event_flistp,
                                        PIN_FLD_BAL_IMPACTS, &rec_id, 1,&cookie, ebufp))
                                        != (pin_flist_t *)NULL )
        {
	    pp_obj = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_OFFERING_OBJ, 1, ebufp);
	    prod_obj = PIN_FLIST_FLD_GET(temp_flistp, PIN_FLD_PRODUCT_OBJ, 1, ebufp);
	    if(pp_obj != NULL && prod_obj != NULL) {
		*prod_pdp = PIN_POID_COPY(prod_obj, ebufp);
		break;
	    }
	}

	if(pp_obj != NULL && !PIN_POID_IS_NULL(pp_obj)) {
	    src_iflistp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_SET(src_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	    PIN_FLIST_FLD_SET(src_iflistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	    PIN_FLIST_FLD_SET(src_iflistp, PIN_FLD_TEMPLATE, (void *)templ, ebufp);
	    //Add the 1st arg
	    args_flistp = PIN_FLIST_ELEM_ADD(src_iflistp, PIN_FLD_ARGS, 1, ebufp);
	    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)pp_obj, ebufp);
	    //Add the 2nd arg
	    args_flistp = PIN_FLIST_ELEM_ADD(src_iflistp, PIN_FLD_ARGS, 3, ebufp);
	    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_POID, (void *)NULL, ebufp);
	    //Add the 3rd arg
	    args_flistp = PIN_FLIST_ELEM_ADD(src_iflistp, PIN_FLD_ARGS, 2, ebufp);
	    PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PLAN_OBJ, (void *)NULL, ebufp);

	    res_flistp = PIN_FLIST_ELEM_ADD(src_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	    PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_NAME, NULL, ebufp);
	    PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_CODE, NULL, ebufp);
	    if(PIN_ERRBUF_IS_ERR(ebufp))
	    {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_prod_details: Error", ebufp);
		goto cleanup;
	    }
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prod_details: Search Input Flist", src_iflistp);
	    //Call the PCM_OP_SEARCH
    	    PCM_OP(ctxp, PCM_OP_SEARCH, 0, src_iflistp, &src_oflistp, ebufp);
	    if(PIN_ERRBUF_IS_ERR(ebufp))
	    {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_prod_details: Error", ebufp);
		goto cleanup;
	    }
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prod_details: Search Output Flist", src_oflistp);
	    res_flistp = PIN_FLIST_ELEM_GET(src_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp);
	    if( res_flistp ){
		    *plan_name = strdup(PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_NAME, 0, ebufp));
		    *plan_code = strdup(PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_CODE, 0, ebufp));
	   }

	   PIN_FLIST_DESTROY_EX(&src_iflistp, NULL); 
	   PIN_FLIST_DESTROY_EX(&src_oflistp, NULL); 

	    pp_read_flistp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_SET(pp_read_flistp, PIN_FLD_POID, pp_obj, ebufp);
	    PIN_FLIST_FLD_SET(pp_read_flistp, PIN_FLD_CYCLE_DISCOUNT, NULL, ebufp);
	    PIN_FLIST_FLD_SET(pp_read_flistp, PIN_FLD_CYCLE_DISC_AMT, NULL, ebufp);
	    PIN_FLIST_FLD_SET(pp_read_flistp, PIN_FLD_CYCLE_DISCOUNT, NULL, ebufp);
	    PIN_FLIST_FLD_SET(pp_read_flistp, PIN_FLD_CYCLE_DISCOUNT, NULL, ebufp);
	    PIN_FLIST_FLD_SET( pp_read_flistp, PIN_FLD_PRODUCT_OBJ, NULL, ebufp );

	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased_product: Read Input Flist", pp_read_flistp);
            //Call the PCM_OP_SEARCH
            PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, pp_read_flistp, &pp_read_oflistp, ebufp);
            if(PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Purchased_product: Error", ebufp);
                goto cleanup;
            }
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Purchased_product: Read Output Flist", pp_read_oflistp);
		disc_per = (pin_decimal_t *)PIN_FLIST_FLD_GET(pp_read_oflistp,
			PIN_FLD_CYCLE_DISCOUNT, 0, ebufp);
		disc_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(pp_read_oflistp,
			PIN_FLD_CYCLE_DISC_AMT, 0, ebufp);
		if( disc_per && !pbo_decimal_is_zero(disc_per, ebufp)) 
		{
			*disc_type = strdup(per);
			*disc_rate = pbo_decimal_copy(disc_per, ebufp);
		}
		else if (disc_amt && !pbo_decimal_is_zero(disc_amt, ebufp))
		{
			*disc_type = strdup(flat);
			*disc_rate = pbo_decimal_copy(disc_amt, ebufp);
		}
		else
		{
			*disc_type = strdup(none);
			*disc_rate = pbo_decimal_copy(disc_amt, ebufp);
		}
	   PIN_FLIST_DESTROY_EX(&pp_read_flistp, NULL); 
	   //PIN_FLIST_DESTROY_EX(&pp_read_oflistp, NULL); 

	    prod_read_flistp = PIN_FLIST_CREATE(ebufp);
	    PIN_FLIST_FLD_COPY(pp_read_oflistp, PIN_FLD_PRODUCT_OBJ, prod_read_flistp, PIN_FLD_POID, ebufp );
	    PIN_FLIST_DESTROY_EX(&pp_read_oflistp, NULL);
	    //PIN_FLIST_FLD_SET(prod_read_flistp, PIN_FLD_POID, prod_obj, ebufp);
	    PIN_FLIST_FLD_SET(prod_read_flistp, PIN_FLD_PROVISIONING_TAG, NULL, ebufp);
	    PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Product: Read Input Flist", prod_read_flistp);
            //Call the PCM_OP_SEARCH
            PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, prod_read_flistp, &prod_read_oflistp, ebufp);
            if(PIN_ERRBUF_IS_ERR(ebufp))
            {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Product: Error", ebufp);
	    }
            PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Product: Read Output Flist", prod_read_oflistp);
	    *prov_tag = strdup(PIN_FLIST_FLD_GET(prod_read_oflistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp));
	   PIN_FLIST_DESTROY_EX(&prod_read_flistp, NULL); 
	   PIN_FLIST_DESTROY_EX(&prod_read_oflistp, NULL); 

	    if(*prov_tag != NULL && strcmp(*prov_tag, "") != 0) {
	    // Get the plan type from mso_config_bb_pt_t

	        pt_src_iflistp = PIN_FLIST_CREATE(ebufp);
	        PIN_FLIST_FLD_SET(pt_src_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	        PIN_FLIST_FLD_SET(pt_src_iflistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	        PIN_FLIST_FLD_SET(pt_src_iflistp, PIN_FLD_TEMPLATE, (void *)pt_templ, ebufp);
	        //Add the 1st arg
	        args_flistp = PIN_FLIST_ELEM_ADD(pt_src_iflistp, PIN_FLD_ARGS, 1, ebufp);
	        PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_PROVISIONING_TAG, (void *)*prov_tag, ebufp);
    
	        res_flistp = PIN_FLIST_ELEM_ADD(pt_src_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_SERVICE_CODE, NULL, ebufp);
	        PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_INDICATOR, NULL, ebufp);
	        if(PIN_ERRBUF_IS_ERR(ebufp))
	        {
		    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "read mso_config_bb_pt_t: Error", ebufp);
		    goto cleanup;
	        }
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read mso_config_bb_pt_t: Search Input Flist", pt_src_iflistp);
	        //Call the PCM_OP_SEARCH
    	        PCM_OP(ctxp, PCM_OP_SEARCH, 0, pt_src_iflistp, &pt_src_oflistp, ebufp);
	        if(PIN_ERRBUF_IS_ERR(ebufp))
	        {
		    PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "read mso_config_bb_pt_t: Error", ebufp);
		    goto cleanup;
	        }
	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "read mso_config_bb_pt_t: Search Output Flist", pt_src_oflistp);
	        res_flistp = PIN_FLIST_ELEM_GET(pt_src_oflistp, PIN_FLD_RESULTS, 0, 0, ebufp);
		if( res_flistp ){
			indicator = *(int32 *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_INDICATOR, 0, ebufp);
			if(indicator == 1) {
			    *plan_type = strdup("PREPAID");
			} else {
			    *plan_type = strdup("ADVANCE");
			}
			svc_code = PIN_FLIST_FLD_TAKE(res_flistp, PIN_FLD_SERVICE_CODE, 0, ebufp);
			*prov_tag = strdup(svc_code);
		}
	        PIN_FLIST_DESTROY_EX(&pt_src_iflistp, NULL); 
	        PIN_FLIST_DESTROY_EX(&pt_src_oflistp, NULL); 
	    }
	}
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_prod_details: Error", ebufp);
	}
cleanup:
	return;
}

static void
fm_mso_get_event_flist(
	pcm_context_t		*ctxp, 
	poid_t			*item_pdp, 
	pin_flist_t		**event_flistp, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*src_iflistp = NULL;
	pin_flist_t		*src_oflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int			flag = SRCH_DISTINCT;
	int64			db = 0;
	char			*templ = "select X from /event where F1 = V1 ";
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;
	db = PIN_POID_GET_DB(item_pdp);
	src_iflistp = PIN_FLIST_CREATE(ebufp);
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(src_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(src_iflistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	PIN_FLIST_FLD_SET(src_iflistp, PIN_FLD_TEMPLATE, (void *)templ, ebufp);
	//Add the 1st arg
	args_flistp = PIN_FLIST_ELEM_ADD(src_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_ITEM_OBJ, (void *)item_pdp, ebufp);
	//Add the Result Array
	res_flistp = PIN_FLIST_ELEM_ADD(src_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_event_flist: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_event_flist: Search Input Flist", src_iflistp);
	//Call the PCM_OP_SEARCH
	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_READ_UNCOMMITTED, src_iflistp, &src_oflistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_event_flist: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_event_flist: Search Output Flist", src_oflistp);
	*event_flistp = PIN_FLIST_COPY(src_oflistp, ebufp);
cleanup:
	PIN_FLIST_DESTROY_EX(&src_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&src_oflistp, NULL);
	return;
}

static void
fm_mso_get_event_details(
	pcm_context_t		*ctxp, 
	pin_flist_t		*evt_flistp, 
	pin_decimal_t		**charge, 
	pin_decimal_t		**discount, 
	char			**event_desc, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*temp_flistp = NULL;
	int			rec_id = 0;
	pin_cookie_t		cookie = NULL;
	pin_decimal_t		*amt = NULL;
	pin_decimal_t		*temp_amt = NULL;
	pin_decimal_t		*disc = NULL;
	pin_decimal_t		*temp_disc = NULL;
	void			*vp = NULL;
	int32			resource_id = 0;
	int			is_found = 0;
	
	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_event_details: Event Flist", evt_flistp);
	*event_desc = (char *)PIN_FLIST_FLD_TAKE(evt_flistp, PIN_FLD_SYS_DESCR, 0, ebufp);
	//Initialize temp_amt and temp_disc
	amt = pbo_decimal_from_str("0", ebufp);
	disc = pbo_decimal_from_str("0", ebufp);
	while ( (temp_flistp = PIN_FLIST_ELEM_GET_NEXT (evt_flistp,
		PIN_FLD_BAL_IMPACTS, &rec_id, 1,&cookie, ebufp))
		!= (pin_flist_t *)NULL )
	{
		vp = PIN_FLIST_FLD_GET( temp_flistp, PIN_FLD_RESOURCE_ID,
					1, ebufp );
		if( vp ){
			resource_id = *(int32 *)vp;
		}
		if( resource_id == 356 ){
			is_found = 1;
			temp_amt = (pin_decimal_t *)PIN_FLIST_FLD_GET(temp_flistp,
				PIN_FLD_AMOUNT, 0, ebufp);
			if(!pbo_decimal_is_null(temp_amt, ebufp)){
				//Sum up the bal impacts amount
				pbo_decimal_add_assign(amt, temp_amt, ebufp);
			}
			temp_disc = (pin_decimal_t *)PIN_FLIST_FLD_GET(temp_flistp,
				PIN_FLD_DISCOUNT, 0, ebufp);
			if(!pbo_decimal_is_null(temp_disc, ebufp)){
				//Sum up the bal impacts discount
				pbo_decimal_add_assign(disc, temp_disc, ebufp);
			}
			if(PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_mso_get_event_details: Error", ebufp);
				goto cleanup;
			}
		}
	}
	if( is_found == 1 ){
		//Copy the Charge and Discount
		*charge = pbo_decimal_copy(amt, ebufp);
		*discount = pbo_decimal_copy(disc, ebufp);
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
			"fm_mso_get_event_details: Got the charge and discount");
	}
cleanup:
	if(!pbo_decimal_is_null(amt, ebufp))
		pbo_decimal_destroy(&amt);
	if(!pbo_decimal_is_null(disc, ebufp))
		pbo_decimal_destroy(&disc);
	PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_event_details: Return");
	return;
}

static void
fm_mso_get_prov_tag(
	pcm_context_t		*ctxp, 
	poid_t			*plan_pdp, 
	poid_t			*srv_pdp, 
	char			**prov_tag, 
	char			**plan_type, 
	char			**plan_code, 
	char			**plan_name, 
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*rfld_iflistp = NULL;
	pin_flist_t		*rfld_oflistp = NULL;
	pin_flist_t		*args_flistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	poid_t			*s_pdp = NULL;
	int64			db = 0;
	int			flag = SRCH_DISTINCT;
	int			*ind = 0;
	char			*templ = "select X from /mso_config_bb_pt where F1 = V1 ";

	if(PIN_ERRBUF_IS_ERR(ebufp))
		return;
	//Read the Plan
	rfld_iflistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_POID, (void *)plan_pdp, ebufp);
	PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_CODE, (void *)NULL, ebufp);
	PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_NAME, (void *)NULL, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_prov_tag: Read fields input", rfld_iflistp);
	//Call the PCM_OP_READ_FLDS opcode
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, rfld_iflistp, &rfld_oflistp, ebufp);

	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_prov_tag: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"fm_mso_get_prov_tag: Read fields output", rfld_oflistp);

	//Get the name and code
	*plan_name = (char *)PIN_FLIST_FLD_TAKE(rfld_oflistp, PIN_FLD_NAME, 0, ebufp);
	*plan_code = (char *)PIN_FLIST_FLD_TAKE(rfld_oflistp, PIN_FLD_CODE, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_prov_tag: Error", ebufp);
		goto cleanup;
	}

	//Free the memory
	PIN_FLIST_DESTROY_EX(&rfld_iflistp, NULL);
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, NULL);
	
	//Search for the /mso_config_bb_pt
	rfld_iflistp = PIN_FLIST_CREATE(ebufp);
	db = PIN_POID_GET_DB(plan_pdp);
	s_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(rfld_iflistp, PIN_FLD_POID, (void *)s_pdp, ebufp);
	PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_FLAGS, (void *)&flag, ebufp);
	PIN_FLIST_FLD_SET(rfld_iflistp, PIN_FLD_TEMPLATE, (void *)templ, ebufp);
	//Add the 1st arg
	args_flistp = PIN_FLIST_ELEM_ADD(rfld_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_SERVICE_CODE, (void *)*plan_name, ebufp);
	//Add the Result Array
	res_flistp = PIN_FLIST_ELEM_ADD(rfld_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_prov_tag: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prov_tag: Search Input Flist", rfld_iflistp);
	//Call the PCM_OP_SEARCH
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, rfld_iflistp, &rfld_oflistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_prov_tag: Error", ebufp);
		goto cleanup;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_get_prov_tag: Search Output Flist", rfld_oflistp);
	res_flistp = PIN_FLIST_ELEM_GET(rfld_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if(res_flistp)
	{
		*prov_tag = (char *)PIN_FLIST_FLD_TAKE(res_flistp, PIN_FLD_PROVISIONING_TAG, 0, ebufp);
		ind = (int *)PIN_FLIST_FLD_GET(res_flistp, PIN_FLD_INDICATOR, 0, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"fm_mso_get_prov_tag: Error", ebufp);
			goto cleanup;
		}
		if(*ind == 1)
			*plan_type = strdup("PREPAID");
		else
			*plan_type = strdup("POSTPAID");
	}
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_mso_get_prov_tag: Error", ebufp);
		goto cleanup;
	}
cleanup:
	PIN_FLIST_DESTROY_EX(&rfld_oflistp, ebufp);
	PIN_FLIST_DESTROY_EX(&rfld_iflistp, ebufp);
	return;
}


static void
mso_op_catv_bill_now(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_flist_t             **r_flistpp,
        pin_errbuf_t            *ebufp )
{

	pin_flist_t	*srch_iflistp  = NULL;
	pin_flist_t	*srch_oflistp = NULL;
	pin_flist_t	*args_flistp = NULL;
	pin_flist_t	*results_flistp = NULL;
	pin_flist_t	*srch_res_flistp = NULL;
	poid_t		*srch_poid = NULL;
	poid_t		*item_pdp = NULL;
	pin_flist_t	*wrt_iflistp = NULL;
	pin_flist_t	*wrt_oflistp = NULL;
	pin_flist_t	*values_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*billnow_iflistp = NULL;
	pin_flist_t	*billnow_oflistp = NULL;
	pin_flist_t	*item_flistp = NULL;
	char		*template = "select X from /item where 1.F1 = V1 and 1.F2 = V2";
	char		*bulk_wrt = "update X for /item where 1.F1 = V1 and 1.F2 = V2";
	int32		srch_flags  = 256;
	int32		st_status = 9;
	int32		pending = 1;
	int32		r_elemid = 0;
	pin_cookie_t	r_cookie = NULL;
	int64		db = 1;
	time_t		end_t = 1498847399;
	

	if(PIN_ERRBUF_IS_ERR(ebufp))
        {
		return;
	}

	srch_iflistp = PIN_FLIST_CREATE(ebufp);
	srch_poid  = PIN_POID_CREATE(db, "/search", -1, ebufp);		
	PIN_FLIST_FLD_PUT(srch_iflistp, PIN_FLD_POID, srch_poid, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
	PIN_FLIST_FLD_SET(srch_iflistp, PIN_FLD_TEMPLATE, template, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	args_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &st_status, ebufp);
	results_flistp = PIN_FLIST_ELEM_ADD(srch_iflistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_POID, NULL, ebufp);
	
	PIN_ERR_LOG_FLIST(3, "Search input flist", srch_iflistp);
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_iflistp, &srch_oflistp, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
        {
		PIN_ERR_LOG_MSG(1, "Error during Item search");
		goto CLEANUP;
	}
	srch_res_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
	if(srch_res_flistp)
	{
		wrt_iflistp = PIN_FLIST_CREATE(ebufp);
		item_pdp = PIN_POID_CREATE(db, "/item", -1, ebufp);
		PIN_FLIST_FLD_PUT(wrt_iflistp, PIN_FLD_POID, item_pdp, ebufp);
		PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
		PIN_FLIST_FLD_SET(wrt_iflistp, PIN_FLD_TEMPLATE, bulk_wrt, ebufp);
		args_flistp = PIN_FLIST_ELEM_ADD(wrt_iflistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, args_flistp, PIN_FLD_ACCOUNT_OBJ, ebufp);
	        args_flistp = PIN_FLIST_ELEM_ADD(wrt_iflistp, PIN_FLD_ARGS, 2, ebufp);
        	PIN_FLIST_FLD_SET(args_flistp, PIN_FLD_STATUS, &st_status, ebufp);
		values_flistp = PIN_FLIST_ELEM_ADD(wrt_iflistp, PIN_FLD_VALUES, 0, ebufp);
		PIN_FLIST_FLD_SET(values_flistp, PIN_FLD_STATUS, &pending, ebufp);
		PIN_ERR_LOG_FLIST(3, "Bulk write flds inflistp ",wrt_iflistp);
		PCM_OP(ctxp, PCM_OP_BULK_WRITE_FLDS, 0,  wrt_iflistp, &wrt_oflistp, ebufp);
		if(PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERR_LOG_MSG(1, "Error during Item search");
			goto CLEANUP;
		}
		//Status update done preparing input for billnow
		billnow_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ,  billnow_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BILLINFO_OBJ, billnow_iflistp, PIN_FLD_BILLINFO_OBJ, ebufp);
		PIN_FLIST_FLD_SET(billnow_iflistp, PIN_FLD_PROGRAM_NAME, "pin_bill_accts", ebufp);
		PIN_FLIST_FLD_SET(billnow_iflistp, PIN_FLD_END_T, &end_t, ebufp);
	      	while((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_oflistp, PIN_FLD_RESULTS,&r_elemid,1,&r_cookie,ebufp)) != NULL)
		{
			item_flistp = PIN_FLIST_ELEM_ADD (billnow_iflistp, PIN_FLD_ITEMS, r_elemid, ebufp);
			PIN_FLIST_FLD_COPY(res_flistp, PIN_FLD_POID, item_flistp, PIN_FLD_ITEM_OBJ, ebufp);
		}
		PIN_ERR_LOG_FLIST(3, "billnow input flist", billnow_iflistp);
		PCM_OP(ctxp, PCM_OP_BILL_MAKE_BILL_NOW, 0, billnow_iflistp, &billnow_oflistp, ebufp);
	    	if(PIN_ERRBUF_IS_ERR(ebufp))
                {
                        PIN_ERR_LOG_MSG(1, "Error during Item search");
                        goto CLEANUP;
                }
		PIN_ERR_LOG_FLIST(3, "Billnow output flistp", billnow_oflistp);
		*r_flistpp = PIN_FLIST_COPY(billnow_oflistp, ebufp);
	}
	else
	{
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_STATUS, &pending, ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ERROR_CODE, "50000", ebufp);
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_ERROR_DESCR, "NO items to bill", ebufp);
	}

	CLEANUP:
		PIN_FLIST_DESTROY_EX(&srch_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&billnow_oflistp, NULL);
		PIN_FLIST_DESTROY_EX(&billnow_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&wrt_iflistp, NULL);
		PIN_FLIST_DESTROY_EX(&wrt_oflistp, NULL);
		return;

}
	
