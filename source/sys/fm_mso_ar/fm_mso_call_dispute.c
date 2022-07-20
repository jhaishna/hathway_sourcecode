/*******************************************************************
 * File:	fm_mso_ar_dispute_details.c14-11-2013
 * Opcode:	MSO_OP_AR_DISPUTE 
 * Owner:	Asish Joshi
 * Created:	17-NOV-2013
 * Description: 
 * MSO_OP_AR_DISPUTE:                   13106

Sample input flist Bill adjustment
0 PIN_FLD_POID             POID [0] 0.0.0.1 /bill 27012 0
0 PIN_FLD_AMOUNT        DECIMAL [0] -2
0 PIN_FLD_PROGRAM_NAME      STR [0] "Customer Center"
0 PIN_FLD_REASON_DOMAIN_ID  INT [0] 16
0 PIN_FLD_REASON_ID         INT [0] 2
0 PIN_FLD_DESCR             STR [0] "Customer unaware of charges"
0 PIN_FLD_FLAGS             INT [0] 2

 Sample input flist for item adjustment
0 PIN_FLD_POID             POID [0] 0.0.0.1 /item/cycle_forward 370970 5
0 PIN_FLD_USERID           POID [0] 0.0.0.1 /account 26216 13
0 PIN_FLD_AMOUNT        DECIMAL [0] -2
0 PIN_FLD_PROGRAM_NAME      STR [0] "MSO_OP_AR_DISPUTE"
0 PIN_FLD_DESCR             STR [0] "ITEM_ADJUSTMENT"
0 PIN_FLD_FLAGS             INT [0] 2 

Sample input flist for event adjustment
0 PIN_FLD_EVENTS             ARRAY [0] allocated 1, used 1
1     PIN_FLD_POID            POID [0] 0.0.0.1 /event/session 220025470857535512 0
0 PIN_FLD_POID                POID [0] 0.0.0.1 /account 26216 13
0 PIN_FLD_USERID              POID [0] 0.0.0.1 /account 26216 13
0 PIN_FLD_DISPUTES    ARRAY [0] allocated 2, used 2
1     PIN_FLD_PERCENT      DECIMAL [0] 25
1     PIN_FLD_RESOURCE_ID      INT [0] 356
0 PIN_FLD_PROGRAM_NAME         STR [0] "Customer Center"
0 PIN_FLD_DESCR                STR [0] "Service down"
0 PIN_FLD_REASON_DOMAIN_ID     INT [0] 14
0 PIN_FLD_REASON_ID            INT [0] 1 

//============================================================================
// Document History
//
// DATE   	/  CHANGE                                       /  Author
//17-NOV-2013	/ Initial Version				/Asish Joshi	
//17-Oct-2014	/ Added BB Customzations and Error Handling	/ Harish
//========/===================================================/===============
//13.11.03/  Template creation                                / Peter Kosut
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_call_dispute.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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
#include "mso_lifecycle_id.h"

#define EVENT_DISPUTE "EVENT_DISPUTE"
#define BILL_DISPUTE "BILL_DISPUTE"
#define ITEM_DISPUTE "ITEM_DISPUTE"

/*******************************************************************
 * MSO_OP_AR_DISPUTE
 *******************************************************************/

EXPORT_OP void
mso_op_ar_dispute(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_ar_dispute_details(
	pcm_context_t		*ctxp,
	int32				flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void 
fm_mso_ar_return_err_flist(
	pcm_context_t		*ctxp,
	char			*err_code,
	char			*err_msg,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

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


/*******************************************************************
 * MSO_OP_AR_DISPUTE  
 *******************************************************************/
void 
mso_op_ar_dispute(
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

	int32			*flag_dispute_type = NULL;
	int32			*flag = NULL;
	int			status = 0;
	
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	
	PIN_ERRBUF_CLEAR(ebufp);

	// Insanity Check 

	if (opcode != MSO_OP_AR_DISPUTE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"MSO_OP_AR_DISPUTE error",
			ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "mso_op_ar_dispute Input Flist:", i_flistp);

	fm_mso_ar_dispute_details(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_ar_dispute_details error", ebufp);
		PIN_ERRBUF_RESET(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_ar_dispute_details error");
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, 
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ), ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53090", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
						"fm_mso_ar_dispute_details error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);

	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"fm_mso_ar_dispute_details output flist", *r_flistpp);
		
	}
	
	return;
}




static void 
fm_mso_ar_dispute_details(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*d_oflistp = NULL;
	pin_flist_t		*d_iflistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*temp1_flistp = NULL;
	pin_flist_t		*temp2_flistp = NULL;
	pin_flist_t		*temp11_flistp = NULL;
	
	

	poid_t			*user_id = NULL;
	poid_t			*bill_pdp = NULL;

	char			*acnt_no = NULL;
	
	int64			db = -1;

	int32			*flag_dispute_type = NULL;
	int32			*output_result = NULL;
	int32			*flag = NULL;
	int32			dispute_success = 0;
	int32			dispute_failure = 1;
	int32			without_tax = 1;
	int32			account_type = 0;
	int32			amount = 0;
	int32			tax_adj_type = 0;
	int32			reso_id = 356;
	int32			reason_id = 0;

	void			*vp = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		return;

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_ar_dispute_details input flist",i_flistp);
	PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - poid missing",ebufp);
		fm_mso_ar_return_err_flist(ctxp, "53090","input flist error - poid missing", r_flistpp , ebufp);
		return;
	}

	flag_dispute_type = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp );
	tax_adj_type = *(int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TAX_FLAGS, 0, ebufp );
	vp =  (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_REASON, 1, ebufp );
	if (vp)
	{
		reason_id = *(int32*)vp;
	}
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		result_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &dispute_failure, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53090", ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Invalid Flags passed", ebufp);
		PIN_FLIST_FLD_COPY(d_oflistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
	
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "CALL_DISPUTE outflist flist", result_flistp);
		*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
		return;
	}  
		
	
	if(flag_dispute_type && *flag_dispute_type==0) // then do bill dispute
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside bill Dispute ");
		//Input Flist mandatory fields check
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - Bill poid missing",ebufp);
			fm_mso_ar_return_err_flist(ctxp, "53090","input flist error - Bill poid missing", r_flistpp , ebufp);
			return;
		}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - dispute Amount missing",ebufp);
			fm_mso_ar_return_err_flist(ctxp, "53090","input flist error - dispute Amount missing", r_flistpp , ebufp);
			return;
		}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - program_name missing",ebufp);
			fm_mso_ar_return_err_flist(ctxp, "53090","input flist error - program_name missing", r_flistpp , ebufp);
			return;
		}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DESCR, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - descr missing",ebufp);
			fm_mso_ar_return_err_flist(ctxp, "53090","input flist error - descr missing", r_flistpp , ebufp);
			return;
		}
		
		//Create input flist

		d_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, d_iflistp, PIN_FLD_AMOUNT, ebufp); //amount entered must be <= bill due amount
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, d_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_DOMAIN_ID, d_iflistp, PIN_FLD_REASON_DOMAIN_ID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, d_iflistp, PIN_FLD_REASON_ID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, d_iflistp, PIN_FLD_USERID, ebufp);	  
		//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON, d_iflistp, PIN_FLD_REASON_ID, ebufp);
		PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_REASON_ID, &reason_id, ebufp);
		PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_FLAGS, &tax_adj_type, ebufp);
			
			
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_BILL_DISPUTE input flist", d_iflistp );
		PCM_OP(ctxp, PCM_OP_AR_BILL_DISPUTE, 0, d_iflistp, &d_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_BILL_DISPUTE output flist", d_oflistp );
		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		 {
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &dispute_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53090", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Bill Dispute Failed", ebufp);
			PIN_FLIST_FLD_COPY(d_oflistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BILL_DISPUTE Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_oflistp, NULL);
			return;
		 }  
		
		output_result = (int32*)PIN_FLIST_FLD_GET(d_oflistp, PIN_FLD_RESULT, 1, ebufp );
		if(!output_result || *output_result != 1) //It failed
		{
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &dispute_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53090", ebufp);
			PIN_FLIST_FLD_COPY(d_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(d_oflistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BILL_DISPUTE Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_oflistp, NULL);
			return;

		}
		else
		{
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_NAME, BILL_DISPUTE, ebufp);
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, d_oflistp, ID_DISPUTE, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_BILL_DISPUTE outflist flist", d_oflistp);
			// Preparing output flist for bill dispute
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, "Dispute Raised Successfully", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &dispute_success, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BILL_DISPUTE outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_oflistp, NULL);
			return;
		}
	}


	if(flag_dispute_type && *flag_dispute_type==1) // then do item disp
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside item dispute ");
		//Input Flist mandatory fields check

		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - item poid missing",ebufp);
			fm_mso_ar_return_err_flist(ctxp, "53090","input flist error - item poid missing", r_flistpp , ebufp);
			return;
		}

		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - adjustment Amount missing",ebufp);
			fm_mso_ar_return_err_flist(ctxp, "53090","input flist error - adjustment Amount missing", r_flistpp , ebufp);
			return;
		}

		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - PIN_FLD_PROGRAM_NAME missing",ebufp);
			return;
		}

		
		//Create input flist

		d_iflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, d_iflistp, PIN_FLD_AMOUNT, ebufp); //amount entered must be <= item due amount
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, d_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, d_iflistp, PIN_FLD_USERID, ebufp);
		//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON, d_iflistp, PIN_FLD_STRING_ID, ebufp);
		PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_STRING_ID, &reason_id, ebufp);
		PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_FLAGS, &tax_adj_type, ebufp);
		
		
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ITEM_DISPUTE input flist", d_iflistp );
		PCM_OP(ctxp, PCM_OP_AR_ITEM_DISPUTE, 0, d_iflistp, &d_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ITEM_DISPUTE outflist flist", d_oflistp);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		 {
			PIN_ERRBUF_RESET(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &dispute_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53091", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Item Dispute Failed", ebufp);
			PIN_FLIST_FLD_COPY(d_oflistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ITEM_Dispute Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_oflistp, NULL);
			return;
		 } 

		output_result = (int32*)PIN_FLIST_FLD_GET(d_oflistp, PIN_FLD_RESULT, 1, ebufp );
		if(output_result && *output_result == 0)
		{
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &dispute_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53091", ebufp);
			PIN_FLIST_FLD_COPY(d_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(d_oflistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ITEM_DISPUTE Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_oflistp, NULL);
			return;	
		}
		else
		{
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_NAME, ITEM_DISPUTE, ebufp);
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, d_oflistp, ID_DISPUTE, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			// Preparing output flist for item dispute
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, "Disputed Successfully", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &dispute_success, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ITEM_DISPUTE outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_oflistp, NULL);
			return;
		}
	}


	if( flag_dispute_type && *flag_dispute_type==2) // then do event dispute
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside event dispute ");

		temp1_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, 0, ebufp );
		temp11_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DISPUTES, PIN_ELEMID_ANY, 0, ebufp );
		
		//Input Flist mandatory fields check
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - Event poid missing",ebufp);
			fm_mso_ar_return_err_flist(ctxp, "53090","input flist error - Event poid missing", r_flistpp , ebufp);
			return;
		}
		PIN_FLIST_FLD_GET(temp1_flistp, PIN_FLD_POID, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - adjustment Amount missing",ebufp);
			fm_mso_ar_return_err_flist(ctxp, "53090","input flist error - adjustment Amount missing", r_flistpp , ebufp);
			return;
		}
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - program_name missing",ebufp);
			fm_mso_ar_return_err_flist(ctxp, "53090","input flist error - program_name missing", r_flistpp , ebufp);
			return;
		}
		

					   
		d_iflistp = PIN_FLIST_CREATE(ebufp);	
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, d_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		temp_flistp = PIN_FLIST_ELEM_ADD(d_iflistp, PIN_FLD_DISPUTES, 0, ebufp );
		PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_RESOURCE_ID, &reso_id, ebufp);
		PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_FLAGS, &tax_adj_type, ebufp);
		PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_AMOUNT, PIN_FLIST_FLD_GET(temp11_flistp, PIN_FLD_AMOUNT, 1, ebufp), ebufp);
		temp2_flistp = PIN_FLIST_ELEM_ADD(d_iflistp, PIN_FLD_EVENTS, 0, ebufp );
		PIN_FLIST_FLD_SET(temp2_flistp, PIN_FLD_POID, PIN_FLIST_FLD_GET(temp1_flistp, PIN_FLD_POID, 1, ebufp), ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, d_iflistp, PIN_FLD_DESCR, ebufp);
		//PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON, d_iflistp, PIN_FLD_REASON_ID, ebufp);
		
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_EVENT_DISPUTE input flist", d_iflistp );
		PCM_OP(ctxp, PCM_OP_AR_EVENT_DISPUTE, 0, d_iflistp, &d_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_EVENT_DISPUTE output flist", d_oflistp );
		PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);

		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &dispute_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53092", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "Event Dispute Failed", ebufp);
			PIN_FLIST_FLD_COPY(d_oflistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "EVENT_Dispute Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_oflistp, NULL);
			
			return;
		} 
		 
		output_result = (int32*)PIN_FLIST_FLD_GET(d_oflistp, PIN_FLD_RESULT, 1, ebufp );
		if(!output_result || *output_result!=1)
		{
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &dispute_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53092", ebufp);
			PIN_FLIST_FLD_COPY(d_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(d_oflistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "EVENT_DISPUTE Fail outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_oflistp, NULL);
			
			return;
		}

		
		else
		{
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_NAME, EVENT_DISPUTE, ebufp);
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, d_oflistp, ID_DISPUTE, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_RESET(ebufp);
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_EVENT_DISPUTE outflist flist", d_oflistp);
			// Preparing output flist for item adjustment
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, "Dispute Success:", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &dispute_success, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "EVENT_Dispute outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&d_oflistp, NULL);
			
			return;
		}
	}
}


static void 
fm_mso_ar_return_err_flist(
	pcm_context_t		*ctxp,
	char			*err_code,
	char			*err_msg,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	int32			dispute_failure = 1;
	int64			db = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
		PIN_ERRBUF_CLEAR(ebufp);

	*r_flistpp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_PUT(*r_flistpp, PIN_FLD_POID, PIN_POID_CREATE(db, "/account", -1, ebufp), ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &dispute_failure, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, err_code, ebufp);
	PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, err_msg, ebufp);

	return;
}
