/*******************************************************************
 * File:	fm_mso_ar_settlement.c14-11-2013
 * Opcode:	MSO_OP_AR_SETTLEMENT , 13154
 * Owner:	Asish Joshi & Vilva Sabarikanth
 * Created:	17-NOV-2013
 * Description: MSO_OP_AR_SETTLEMENT: 13154
 *
 * Document History
 *
 * DATE         /  CHANGE                                       /  Author
 * 17-NOV-2013   / Initial Version                               /Asish Joshi
 * 17-Oct-2014   / Added BB Customzations and Error Handling     / Harish
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_do_settlement.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
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

#define EVENT_SETTLEMENT "EVENT_SETTLEMENT"
#define BILL_SETTLEMENT "BILL_SETTLEMENT"
#define ITEM_SETTLEMENT "ITEM_SETTLEMENT"

/*******************************************************************
 * MSO_OP_AR_SETTLEMENT 
 *******************************************************************/

EXPORT_OP void
mso_op_ar_settlement(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_ar_settlement(
	pcm_context_t		*ctxp,
	int32				flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern
void fm_mso_create_lifecycle_evt(
	pcm_context_t		*ctxp,
	pin_flist_t		*in_flistp,
	pin_flist_t		*out_flistp,
	int32			action,
	pin_errbuf_t		*ebufp);

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

/*******************************************************************
 * MSO_OP_AR_SETTLEMENT  
 *******************************************************************/
void 
mso_op_ar_settlement(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	int			status = 0;

	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	
	PIN_ERRBUF_CLEAR(ebufp);

	// Insanity Check 

	if (opcode != MSO_OP_AR_SETTLEMENT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"mso_op_ar_settlement error",
			ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_AR_SETTLEMENT Input flist:", i_flistp);

	fm_mso_ar_settlement(ctxp, flags, i_flistp, r_flistpp, ebufp);
 	
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{	
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"fm_mso_ar_settlement error", ebufp);
		PIN_ERRBUF_CLEAR(ebufp);
		status = 1;
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "fm_mso_ar_settlement error");
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, 
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp ), ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "53002", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR,
						"fm_mso_ar_settlement error", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status, ebufp);
	}
	else
	{
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"fm_mso_ar_settlement output flist", *r_flistpp);
		
	}
	
	return;
}




static void 
fm_mso_ar_settlement(
	pcm_context_t		*ctxp,
	int32				flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*result_flistp = NULL;
	pin_flist_t		*result_flist = NULL;
	pin_flist_t		*result2_flistp = NULL;
	pin_flist_t		*s_oflistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*arg_flist = NULL;
	pin_flist_t		*rs_flistp = NULL;
	pin_flist_t		*d_iflistp = NULL;
	pin_flist_t		*temp1_flistp = NULL;
	pin_flist_t		*temp11_flistp = NULL;
	pin_flist_t		*temp_flistp = NULL;
	pin_flist_t		*temp2_flistp = NULL;
	pin_flist_t		*item_in_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*s_inflistp = NULL;
	pin_flist_t		*flistp = NULL;

	pin_errbuf_t		*ebufp2;

	poid_t			*user_id = NULL;
	poid_t			*item_pdp = NULL;
	poid_t			*srch_pdp = NULL;
	poid_t			*bill_pdp = NULL;
	poid_t			*ar_billinfo_pdp = NULL;
	poid_t			*d_pdp = NULL;
	
	char			*acnt_no = NULL;
	char			*bill_no = NULL;
	char			*template = "select X from /event_bal_impacts where F1 = V1" ;
	char			*template1 = "select X from /bill where F1 = V1 and F2.id = F3.id";
	
	int64			db = -1;
	int64			ar_billinfo_pid = 0;
	int64			billinfo_pid = 0;

	int32			*flag_adj_type = NULL;
	int32			*settle_type = NULL;
	int32			*tax_adj_type = NULL;
	int32			*resource_id = NULL;
	int32			*flag = NULL;
	int32			settlement_success = 0;
	int32			adjustment_failure = 1;
	int32			account_type = 0;
	int32			amount = 0;
	int32			*result = NULL;
	int32			err_flg = 0;
	int32			srch_flag = 512;
	int32			elemid = 0;
	int32			reso_id = 356;

	//void			*v_ptr=NULL;
	
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_AR_SETTLEMENT input flist", i_flistp );
	settle_type = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 0, ebufp );
	tax_adj_type = (int32*)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TAX_FLAGS, 0, ebufp );
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERRBUF_CLEAR(ebufp);
		result_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &adjustment_failure, ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
		PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53095", ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Unknown error- outflist flist", result_flistp);
		*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
		return;
	}
	
		
	
	if((*settle_type)==0) // then do bill settlement
	{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside bill settlement ");
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "MSO_OP_AR_SETTLEMENT input flist", i_flistp );
		s_inflistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, s_inflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, s_inflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, s_inflistp, PIN_FLD_DESCR, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_TAX_FLAGS, s_inflistp, PIN_FLD_FLAGS, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_DOMAIN_ID, s_inflistp, PIN_FLD_REASON_DOMAIN_ID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_REASON_ID, s_inflistp, PIN_FLD_REASON_ID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_AMOUNT, s_inflistp, PIN_FLD_AMOUNT, ebufp);
		item_in_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_ITEMS, PIN_ELEMID_ANY, 1, ebufp);
		if(item_in_flistp)
		{
			flistp = PIN_FLIST_ELEM_ADD(s_inflistp, PIN_FLD_ITEMS, 0, ebufp);
			PIN_FLIST_FLD_COPY(item_in_flistp, PIN_FLD_ITEM_OBJ, flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(item_in_flistp, PIN_FLD_AMOUNT, flistp, PIN_FLD_AMOUNT, ebufp);
		}
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_BILL_SETTLEMENT input flist", s_inflistp );
		PCM_OP(ctxp, PCM_OP_AR_BILL_SETTLEMENT, 0, s_inflistp, &s_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&s_inflistp, NULL); 
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_BILL_SETTLEMENT output flist", s_oflistp );
		result = (int32*)PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_RESULT, 0, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &adjustment_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Bill Settlement Failed", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53096", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);

			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_BILL_SETTLEMENT error outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			goto CLEANUP;
		}  
		else if ( result && *result ==  0) // Settlement Failed
		{
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &adjustment_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53096", ebufp);
			PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);

			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BILL_SETTLEMENT failed- outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			goto CLEANUP;;
		}
		else if (result && *result ==1 )
		{
			//mso_op_settlement_gen_lc_event(ctxp, i_flistp, s_oflistp, BILL_SETTLEMENT, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_NAME, BILL_SETTLEMENT, ebufp);
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, s_oflistp, ID_SETTLEMENT_OF_DISPUTE, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_CLEAR(ebufp);
			}

			// Preparing output flist for bill adjustment
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, "Settlement Success:", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &settlement_success, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "BILL_SETTLEMENT success outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
			return;
		}
		
	}


	if((*settle_type)==1) // then do item settlement
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside item settlement ");

		//Input Flist mandatory fields check

		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "input flist error - Item poid missing",ebufp);
			err_flg=1;
		}
				
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"input flist error - PIN_FLD_DESCR missing");
			err_flg=1;
		}
		
		if (err_flg==1)
		{
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &adjustment_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53095", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Data in-sufficient outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			return;
		}
		
		
		d_iflistp = PIN_FLIST_COPY(i_flistp, ebufp);
		PIN_FLIST_FLD_SET(d_iflistp, PIN_FLD_FLAGS, tax_adj_type, ebufp);
		PIN_FLIST_FLD_DROP(d_iflistp, PIN_FLD_TAX_FLAGS, ebufp);
		PIN_FLIST_FLD_DROP(d_iflistp, PIN_FLD_USERID, ebufp);
		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ITEM_SETTLEMENT input flist", d_iflistp );
		
		PCM_OP(ctxp, PCM_OP_AR_ITEM_SETTLEMENT, 0, d_iflistp, &s_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &adjustment_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Item Settlement Failed", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53097", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);

			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ITEM_SETTLEMENT error outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
			return;
		}  

		if ((*(int32*)PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_RESULT,1, ebufp))==0) // Settlement Failed
		{
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &adjustment_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53097", ebufp);
			PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);

			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ITEM_SETTLEMENT failed - output flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			return;
		}
		

		else
		{
			//mso_op_settlement_gen_lc_event(ctxp, i_flistp, s_oflistp, ITEM_SETTLEMENT, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_NAME, ITEM_SETTLEMENT, ebufp);
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, s_oflistp, ID_SETTLEMENT_OF_DISPUTE, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_CLEAR(ebufp);
			}

			// Preparing output flist for item adjustment
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, "Settlement Success:", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &settlement_success, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "ITEM_SETTLEMENT success output flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			return;
		}
	}


	if((*settle_type)==2) // then do event settlement
	{
		
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG," Inside event settlement ");

		temp1_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_EVENTS, PIN_ELEMID_ANY, 1, ebufp );
			if (PIN_ERRBUF_IS_ERR(ebufp)) err_flg=1;
		temp11_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SETTLEMENTS, PIN_ELEMID_ANY, 1, ebufp );
			if (PIN_ERRBUF_IS_ERR(ebufp)) err_flg=1;
		//Input Flist mandatory fields check
		
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,"input flist error - Account poid missing");
			err_flg=1;
		}
		
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_PROGRAM_NAME, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "input flist error - PIN_FLD_PROGRAM_NAME missing");
			err_flg=1;
		}
		
		PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ITEM_OBJ, 1, ebufp);
		if (PIN_ERRBUF_IS_ERR(ebufp)) 
		{
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "input flist error - PIN_FLD_ITEM_OBJ missing");
			err_flg=1;
		}

				
		if (err_flg==1)
		{
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &adjustment_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Input Error", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53095", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);

			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "Data in-sufficient outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			return;
		}
		
		d_iflistp = PIN_FLIST_CREATE(ebufp);	
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, d_iflistp, PIN_FLD_POID, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ITEM_OBJ, d_iflistp, PIN_FLD_ITEM_OBJ, ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, d_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
		temp_flistp = PIN_FLIST_ELEM_ADD(d_iflistp, PIN_FLD_SETTLEMENTS, 0, ebufp );
		PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_RESOURCE_ID, &reso_id, ebufp);
		//PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_FLAGS, &tax_adj_type, ebufp);
		PIN_FLIST_FLD_SET(temp_flistp, PIN_FLD_AMOUNT, PIN_FLIST_FLD_GET(temp11_flistp, PIN_FLD_AMOUNT, 1, ebufp), ebufp);
		
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_DESCR, d_iflistp, PIN_FLD_DESCR, ebufp);

		
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_EVENT_SETTLEMENT input flist", d_iflistp );
		
		PCM_OP(ctxp, PCM_OP_AR_EVENT_SETTLEMENT, 0, d_iflistp, &s_oflistp, ebufp);
		PIN_FLIST_DESTROY_EX(&d_iflistp, NULL);

		
		if (PIN_ERRBUF_IS_ERR(ebufp))
		{
			PIN_ERRBUF_CLEAR(ebufp);
			result_flistp = PIN_FLIST_CREATE(ebufp2);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &adjustment_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Event Settlement Failed", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53098", ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);

			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_EVENT_SETTLEMENT error outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
			return;
		}  
		else if (*(int32*)PIN_FLIST_FLD_GET(s_oflistp, PIN_FLD_RESULT,1, ebufp)==0) // Settlement Failed
		{
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &adjustment_failure, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53098", ebufp);
			PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_DESCR, result_flistp, PIN_FLD_ERROR_DESCR, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);

			
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "EVENT_SETTLEMENT failed - output flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
			return;

		}
		else
		{
			//mso_op_settlement_gen_lc_event(ctxp, i_flistp, s_oflistp, EVENT_SETTLEMENT, ebufp);
			PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_NAME, EVENT_SETTLEMENT, ebufp);
			fm_mso_create_lifecycle_evt(ctxp, i_flistp, s_oflistp, ID_SETTLEMENT_OF_DISPUTE, ebufp);
			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Lifecycle event creation error", ebufp);
				PIN_ERRBUF_CLEAR(ebufp);
			}

			if (PIN_ERRBUF_IS_ERR(ebufp))
			{
				PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, "Settlement Failure in Lifecycle");
				PIN_ERRBUF_CLEAR(ebufp);
				result_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &adjustment_failure, ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_DESCR, "Settlement Failed in Lifecycle Event", ebufp);
				PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_ERROR_CODE, "53096", ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, result_flistp, PIN_FLD_POID, ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
				PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "SETTLEMENT error outflist flist", result_flistp);
				*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
				PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
				PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
				return;
			}
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "PCM_OP_AR_ITEM_SETTLEMENT outflist flist", s_oflistp);
			// Preparing output flist for item adjustment
			result_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(s_oflistp, PIN_FLD_ITEM_OBJ, result_flistp, PIN_FLD_POID, ebufp);

			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_USERID, result_flistp, PIN_FLD_USERID, ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_DESCR, "Event Settlement Success:", ebufp);
			PIN_FLIST_FLD_SET(result_flistp, PIN_FLD_STATUS, &settlement_success, ebufp);
		
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "EVENT_SETTLEMENT success outflist flist", result_flistp);
			*r_flistpp=PIN_FLIST_COPY(result_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&result_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&s_oflistp, NULL);
			return;
		}
	}

	CLEANUP:

	return;
}


static void mso_find_item_poid(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	int32			children = 0;
	poid_t			*s_item_pdp = NULL;
	int32			d_items = 2;



	s_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, s_flistp, PIN_FLD_POID, ebufp);
	PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_BILLINFO_OBJ, s_flistp, PIN_FLD_AR_BILLINFO_OBJ, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_INCLUDE_CHILDREN, &children, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_STATUS, &d_items, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "item poid retrieve input flist", s_flistp);
	PCM_OP(ctxp, PCM_OP_AR_GET_DISPUTES, 0, s_flistp, r_flistpp, ebufp);
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "item poid retrieve output flist", *r_flistpp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No Disputes found for processing", ebufp);
	}
	flistp = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_RESULTS, 0, ebufp);
	s_item_pdp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_POID, 0, ebufp);
	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "No Disputes found for processing", ebufp);
		*r_flistpp=NULL;
	}
	else
	{
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_POID, s_item_pdp, ebufp);
	}
	PIN_ERRBUF_CLEAR(ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	return;
}
