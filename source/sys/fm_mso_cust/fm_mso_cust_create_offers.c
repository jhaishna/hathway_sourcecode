/*******************************************************************
 * File:	fm_mso_cust_create_offers.c
 * Opcode:	MSO_OP_CUST_CREATE_OFFERS 
 * Owner:	SREEKANTH Y	
 * Created:	20-AUG-2016	
 * Description: 
 *
 * Modification History:
 * Modified By:
 * Date:
 * Modification details
 *
 * This opcode is for getting CATV ET Refund amount when called in CALC_ONLY mode
 *
 *******************************************************************/

#ifndef lint
static const char Sccs_id[] = "@(#)%Portal Version: fm_mso_cust_create_offers.c:CUPmod7.3PatchInt:1:2006-Dec-01 16:31:18 %";
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcm.h"
#include "ops/act.h"
#include "ops/cust.h"
#include "pin_bill.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "mso_ops_flds.h"
#include "mso_lifecycle_id.h"
#include "mso_cust.h"
#include "mso_prov.h"

/*******************************************************************
 * MSO_OP_CUST_CREATE_OFFERS 
 *******************************************************************/
extern int32
fm_mso_trans_open(
	pcm_context_t           *ctxp,
	poid_t                  *pdp,
	int32                   flag,
	pin_errbuf_t            *ebufp);

extern void
fm_mso_trans_commit(
	pcm_context_t           *ctxp,
	poid_t                  *pdp,
	pin_errbuf_t            *ebufp);

extern void
fm_mso_trans_abort(
	pcm_context_t           *ctxp,
	poid_t                  *pdp,
	pin_errbuf_t            *ebufp);

EXPORT_OP void
op_mso_cust_create_offers(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_mso_cust_create_offers(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp);

extern
void fm_mso_create_lifecycle_evt(
        pcm_context_t           *ctxp,
        pin_flist_t             *in_flistp,
        pin_flist_t             *out_flistp,
        int32                   action,
        pin_errbuf_t            *ebufp);

/*******************************************************************
 * MSO_OP_CUST_CREATE_OFFERS  
 *******************************************************************/
void 
op_mso_cust_create_offers(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;

	*r_flistpp		= NULL;
	int32                   local = LOCAL_TRANS_OPEN_FAIL;
	int32                   *status = NULL;
	int32                   status_uncommitted = 2;
	poid_t                  *inp_pdp = NULL;

	int			failure = 1;
	
	if (PIN_ERRBUF_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERRBUF_CLEAR(ebufp);

	/*******************************************************************
	 * Insanity Check 
	 *******************************************************************/
	if (opcode != MSO_OP_CUST_CREATE_OFFERS) {
		pin_set_err(ebufp, PIN_ERRLOC_FM, 
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_mso_cust_create_offers error",
			ebufp);
		return;
	}

	/*******************************************************************
	 * Debug: Input flist 
	 *******************************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, 
		"op_mso_cust_create_offers input flist", i_flistp);

	inp_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	local = fm_mso_trans_open(ctxp, inp_pdp, 3, ebufp);
	if(PIN_ERRBUF_IS_ERR(ebufp))
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Error", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51413", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Error", ebufp);
			return;
	}
	if ( local == LOCAL_TRANS_OPEN_FAIL )
	{
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Unable to open Local Transaction: Already Open", ebufp);
			PIN_ERRBUF_RESET(ebufp);
			*r_flistpp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_POID, inp_pdp, ebufp );
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_STATUS, &status_uncommitted, ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_CODE, "51414", ebufp);
			PIN_FLIST_FLD_SET(*r_flistpp , PIN_FLD_ERROR_DESCR, "Unable to open Local Transaction: Already Open", ebufp);
			return;
	}

	
	/*******************************************************************
	 * Call the default implementation 
	 *******************************************************************/
	fm_mso_cust_create_offers(ctxp,  i_flistp, r_flistpp, ebufp);
	status = PIN_FLIST_FLD_GET(*r_flistpp, PIN_FLD_STATUS, 1, ebufp);
 	/***********************************************************
	 * Results.
	 ***********************************************************/

	if (PIN_ERRBUF_IS_ERR(ebufp)) 
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"op_mso_cust_create_offers error", ebufp);
		*r_flistpp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, *r_flistpp, PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, *r_flistpp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &failure, ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_CODE, "71000", ebufp);
		PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_ERROR_DESCR, "Error in while creating Offer", ebufp);
		if(local == LOCAL_TRANS_OPEN_SUCCESS )
			fm_mso_trans_abort(ctxp, inp_pdp, ebufp);
		return;
	}
	else if(local == LOCAL_TRANS_OPEN_SUCCESS && (status && *status == 1)){
		fm_mso_trans_abort(ctxp, inp_pdp, ebufp);	
	}
	else
	{
		if(local == LOCAL_TRANS_OPEN_SUCCESS ){
			PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,	"op_mso_cust_create_offers output flist", *r_flistpp);
			fm_mso_trans_commit(ctxp, inp_pdp, ebufp);
		}
		else
                {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Transaction NOT Comitted");
                        PIN_FLIST_FLD_SET(*r_flistpp, PIN_FLD_STATUS, &status_uncommitted, ebufp);
                }
	}
	return;
}

/*******************************************************************
 * This is the default implementation for this policy
 *******************************************************************/
static void 
fm_mso_cust_create_offers(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**r_flistpp,
	pin_errbuf_t		*ebufp)
{

	pin_flist_t		*ret_flistp = NULL;
	pin_flist_t		*srch_oflistp = NULL;
	pin_flist_t		*results_flistp = NULL;
	pin_flist_t		*create_flistp = NULL;
	pin_flist_t		*mso_purch_flistp = NULL;
	pin_flist_t		*mso_prod_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	pin_flist_t		*srvc_flistp = NULL;
	pin_flist_t		*debit_iflistp = NULL;
	pin_flist_t		*debit_oflistp = NULL;
	pin_flist_t		*debit_flistp = NULL;
	pin_flist_t		*provaction_inflistp = NULL;
	pin_flist_t		*provaction_outflistp = NULL;
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*svc_up_flistp = NULL;
	pin_flist_t 		*mso_bp_flistp = NULL;

	poid_t			*acct_pdp = NULL;
	poid_t			*srvc_pdp = NULL;
	poid_t			*ret_pdp = NULL;
	poid_t			*mso_bb_bp_obj = NULL;
	poid_t			*mso_bb_obj = NULL;

	int32			*flag = NULL;
	int32			*offer_type = NULL;
	int32			*tech = NULL;
	int32			error_codep = 0;
	int32			opcode = MSO_OP_CUST_CREATE_OFFERS;
	int32			create_offer_failure = 1;
	int32			success = 0;
	int32			oflags = 1;
	int32			fup_status = 1;
	int32			prov_flags = DOC_BB_ADD_MB;
	int32			plan_type = 0;
	int32			res_id = 0;

	int			elem_id =0;
	int			is_apply_now = 0;
	int			ret_val =0;
	int			status = 1;

	char			*prov_tag = NULL; 
	char			*plan_name = NULL;

	char 			error_descr[200]="";
	char 			valid_to_str[50]="";

	time_t			*valid_to_t = NULL;
	time_t			valid_from_t = 0;
	time_t			*old_valid_to_t = NULL;

	pin_decimal_t		*initial_amount = NULL;
	pin_decimal_t		*amount = NULL;

	void			*vp = NULL;
	pin_cookie_t		cookie = NULL;
	int			*prov_call = NULL;
	int			fup_stat = 0;	
	
	int			*fup_flag = NULL;
	
	if (PIN_ERRBUF_IS_ERR(ebufp))
	{
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
			"Error in calling fm_mso_cust_create_offers", ebufp);
		return;
	}
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_offers input flist", i_flistp);
	
	acct_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACCOUNT_OBJ, 1, ebufp);
	srvc_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_SERVICE_OBJ, 1, ebufp);
	valid_to_t = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_VALID_TO, 1, ebufp);

	if (srvc_pdp && strcmp(PIN_POID_GET_TYPE(srvc_pdp), "/service/telco/broadband") == 0){
		offer_type = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_TYPE, 1, ebufp);
		if (!offer_type || ( offer_type && (*offer_type != 1 && *offer_type != 2 )))
		{
			strcpy(error_descr, "Please provide valid offer type");
			ret_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
			PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, ret_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_offer_failure, ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "71000", ebufp);
			PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, error_descr, ebufp);
			*r_flistpp = ret_flistp;
			return;
		}
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_FLAGS, &oflags, ebufp);
		fm_mso_search_offer_entries(ctxp, i_flistp, &srch_oflistp, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_offers search output flist", srch_oflistp);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in calling fm_mso_cust_create_offers", ebufp);
			return;
				
		}
		if (srch_oflistp && PIN_FLIST_ELEM_COUNT(srch_oflistp, PIN_FLD_RESULTS, ebufp)  > 0){
			results_flistp = PIN_FLIST_ELEM_GET(srch_oflistp, PIN_FLD_RESULTS, 0, 1, ebufp);
			old_valid_to_t = PIN_FLIST_FLD_GET(results_flistp, PIN_FLD_VALID_TO, 1, ebufp);
			if (valid_to_t && old_valid_to_t && *old_valid_to_t > 0 && *valid_to_t > *old_valid_to_t ){
				valid_from_t = *old_valid_to_t+1; // VALID_FROM should be equal to next of current expirey date 
				is_apply_now = 1; //If set to 1, no need to update balance and provision since offer was already applied.
			}
			else {
				ret_flistp = PIN_FLIST_CREATE(ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
				PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, ret_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_offer_failure, ebufp);
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "71000", ebufp);
				if(old_valid_to_t)
					strftime( valid_to_str, sizeof(valid_to_str), "%d-%b-%Y",  localtime( (time_t *) old_valid_to_t) );
				if(!old_valid_to_t || (old_valid_to_t && *old_valid_to_t == 0)){
					strcpy(error_descr, "Account has already lifetime valid offer.");
				}
				else if (offer_type && *offer_type == 1)
				{
					sprintf(error_descr, "Please select VALIDITY END DATE greater than the existing discount offer validity end date %s", 	
									valid_to_str);
				}
				else if (offer_type && *offer_type == 2)
				{
					sprintf(error_descr, "Please select VALIDITY END DATE  greater than the existing DATA offer validity end date %s", 
									valid_to_str);
				}
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
						error_descr, ebufp);
				*r_flistpp = ret_flistp;
				PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
				return;
			}
		}
		else{
			valid_from_t = pin_virtual_time(NULL);
		}
		//Proceed to create config entry for DISCOUNT and DATA offers based on the input
		PIN_FLIST_FLD_SET(i_flistp, PIN_FLD_VALID_FROM, &valid_from_t, ebufp);
		PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_FLAGS, ebufp);
		create_flistp = PIN_FLIST_COPY(i_flistp, ebufp);
		PIN_FLIST_FLD_DROP(create_flistp, PIN_FLD_PROGRAM_NAME, ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_offers create obj inflist", create_flistp);
		PCM_OP(ctxp, PCM_OP_CREATE_OBJ, 0, create_flistp, &ret_flistp, ebufp);;
		PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
		if (PIN_ERRBUF_IS_ERR(ebufp)){
			PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
			PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
				"Error in calling fm_mso_cust_create_offers", ebufp);
			return;
			
		}
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_cust_create_offers create obj outflist", ret_flistp);
		if (ret_flistp){
			ret_pdp = PIN_FLIST_FLD_GET(ret_flistp, PIN_FLD_POID, 1, ebufp);
			if (ret_pdp && PIN_POID_GET_ID(ret_pdp) > 1){
				// Added changes for updating balance immediately...
				if(is_apply_now == 0 && (offer_type && *offer_type == 2) && (srvc_pdp && PIN_POID_GET_ID(srvc_pdp) > 1) ){
					fm_mso_utils_get_mso_baseplan_details(ctxp, acct_pdp, srvc_pdp, MSO_PROV_STATE_ACTIVE, status, &mso_purch_flistp, ebufp);
					if (PIN_ERRBUF_IS_ERR(ebufp)){
						PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
							"Error in getting mso pruchase details", ebufp);
						return;
						
					}
					if(mso_purch_flistp && PIN_FLIST_ELEM_COUNT(mso_purch_flistp, PIN_FLD_RESULTS, ebufp) > 0){
						mso_bp_flistp = PIN_FLIST_ELEM_GET(mso_purch_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
						if(mso_bp_flistp){
							mso_bb_obj = PIN_FLIST_FLD_GET(mso_bp_flistp, PIN_FLD_POID, 1, ebufp);
							mso_bb_bp_obj = PIN_FLIST_FLD_GET(mso_bp_flistp, PIN_FLD_PLAN_OBJ, 1, ebufp);
							if(mso_bb_obj){
								while ((mso_prod_flistp = PIN_FLIST_ELEM_GET_NEXT(mso_bp_flistp, 															PIN_FLD_PRODUCTS, &elem_id, 1, &cookie, ebufp)) != NULL){
									if(mso_prod_flistp){
										prov_tag = PIN_FLIST_FLD_GET(mso_prod_flistp, PIN_FLD_PROVISIONING_TAG, 1, ebufp);
										if(prov_tag && strcmp(prov_tag, "") != 0){
											break;
										}
									}
								}
							}
						}
					}
					else{
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Active Base Plan purchased plan not found" );
						PIN_ERRBUF_RESET(ebufp);
						PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
						ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, ret_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_offer_failure, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "71000", ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Active Base Plan purchased plan not found", ebufp);
						*r_flistpp = ret_flistp;
						return;
					}
					if(mso_bb_bp_obj){
						fm_mso_utils_read_any_object(ctxp, mso_bb_bp_obj, &create_flistp, ebufp);
						if(create_flistp){
							plan_name = PIN_FLIST_FLD_GET(create_flistp, PIN_FLD_NAME, 1, ebufp);
						}
					}
					fm_mso_utils_read_any_object(ctxp, srvc_pdp, &srvc_flistp, ebufp);
					if(srvc_flistp){
						tmp_flistp = PIN_FLIST_SUBSTR_GET(srvc_flistp, MSO_FLD_BB_INFO, 1, ebufp);
						if(tmp_flistp){
							tech = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_TECHNOLOGY, 1, ebufp);
							fup_flag = PIN_FLIST_FLD_GET(tmp_flistp, MSO_FLD_FUP_STATUS, 0, ebufp);
							fup_stat = *fup_flag;
						}
					}
					if(plan_name && prov_tag && tech ){	
						fm_quota_check(ctxp, plan_name, prov_tag, tech, &plan_type, &initial_amount, &error_codep, ebufp);
						if (PIN_ERRBUF_IS_ERR(ebufp)){
							PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&mso_purch_flistp, NULL);
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
								"Error in Checking plan quota details", ebufp);
							return;
							
						}
					}
					else{
						PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&mso_purch_flistp, NULL);
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, 
							"Error while getting plan details");
						ret_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, ret_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_offer_failure, ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "71001", ebufp);
						PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Error - Base Plan details not found", ebufp);
						*r_flistpp = ret_flistp;
						return;
					}
					if(plan_type == 1)
						res_id = MSO_FREE_MB;
					else if(plan_type == 2)
						res_id = MSO_FUP_TRACK;
					else if(plan_type == 0)
						res_id = MSO_TRCK_USG;
					//Call PCM_OP_BILL_DEBIT to add the offer to balance immediately post activation		
					amount = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AMOUNT, 1, ebufp);
					debit_iflistp=PIN_FLIST_CREATE (ebufp);
					PIN_FLIST_FLD_SET(debit_iflistp, PIN_FLD_POID, acct_pdp, ebufp);
					PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, debit_iflistp, PIN_FLD_PROGRAM_NAME, ebufp);
					PIN_FLIST_FLD_SET(debit_iflistp, PIN_FLD_SERVICE_OBJ, srvc_pdp, ebufp);
					PIN_FLIST_FLD_COPY(srvc_flistp, PIN_FLD_BAL_GRP_OBJ, debit_iflistp, PIN_FLD_BAL_GRP_OBJ, ebufp);

					debit_flistp = PIN_FLIST_ELEM_ADD(debit_iflistp, PIN_FLD_DEBIT ,res_id, ebufp);
					PIN_FLIST_FLD_SET(debit_flistp, PIN_FLD_BAL_OPERAND, pbo_decimal_negate(amount, ebufp), ebufp);
					PCM_OP(ctxp, PCM_OP_BILL_DEBIT, 0, debit_iflistp, &debit_oflistp, ebufp);
					PIN_FLIST_DESTROY_EX(&debit_iflistp, NULL);

					if(PIN_ERRBUF_IS_ERR(ebufp) ){	
						PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,"Error PCM_OP_BILL_DEBIT",ebufp);
						PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
						PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
						PIN_FLIST_DESTROY_EX(&mso_purch_flistp, NULL);
					}
					PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,"output flist of PCM_OP_BILL_DEBIT",debit_oflistp);
					if(debit_oflistp){
						//Call provisioning action to add DATA and call ADD_DATA_DOCSIS action.		
						prov_flags = DOC_BB_ADD_MB;
						provaction_inflistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, provaction_inflistp, PIN_FLD_POID, ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, provaction_inflistp, PIN_FLD_SERVICE_OBJ, ebufp);
						PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_PURCHASED_PLAN_OBJ, mso_bb_obj, ebufp);
						PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_FLAGS, &prov_flags, ebufp);
						PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_FUP_STATUS, &fup_status, ebufp);
						PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_OLD_PURCHASED_PLAN_OBJ, mso_bb_obj, ebufp);
						PIN_FLIST_FLD_SET(provaction_inflistp, MSO_FLD_OLD_PLAN_OBJ, mso_bb_bp_obj, ebufp);
						PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_OPCODE, &opcode, ebufp);
						
						PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_USERID,provaction_inflistp,PIN_FLD_USERID,ebufp);
						PIN_FLIST_FLD_COPY(i_flistp,PIN_FLD_PROGRAM_NAME,provaction_inflistp,PIN_FLD_PROGRAM_NAME,ebufp);
						res_flistp = PIN_FLIST_ELEM_GET(debit_oflistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY,1, ebufp);
						PIN_FLIST_ELEM_COPY(res_flistp, PIN_FLD_SUB_BAL_IMPACTS, 0, 																		provaction_inflistp, PIN_FLD_SUB_BAL_IMPACTS, 0, ebufp);
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_create_action: provisioning input list", 
											provaction_inflistp);
						PCM_OP(ctxp, MSO_OP_PROV_CREATE_ACTION , 0, provaction_inflistp, &provaction_outflistp, ebufp);
						PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
						if(provaction_outflistp != NULL)
						{
							prov_call = PIN_FLIST_FLD_GET(provaction_outflistp, PIN_FLD_STATUS, 1, ebufp);
						}
						if (PIN_ERRBUF_IS_ERR(ebufp) || (prov_call && *prov_call == 1))
						{
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error in calling MSO_OP_PROV_CREATE_ACTION ", ebufp);
							PIN_ERRBUF_RESET(ebufp);
							PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
							ret_flistp = PIN_FLIST_CREATE(ebufp);
                                                        PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, ret_flistp, PIN_FLD_POID, ebufp );
                                                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_offer_failure, ebufp);
                                                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51754", ebufp);
                                                        PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR,
                                                                                "Error after returning from provisioning opcode", ebufp);
							*r_flistpp = ret_flistp;
							PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&mso_purch_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&debit_oflistp, NULL);
							return;
						} 
						PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_prov_create_action: provisioning output flist", 
												provaction_outflistp);
						PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);
						/***********Updating FUP customer's fup limit status********/
						if(fup_stat == 2)
						{
							provaction_inflistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_SET(provaction_inflistp, PIN_FLD_POID, srvc_pdp, ebufp);
							svc_up_flistp = PIN_FLIST_ELEM_ADD(provaction_inflistp, MSO_FLD_BB_INFO, 0, ebufp);
							PIN_FLIST_FLD_SET(svc_up_flistp, MSO_FLD_FUP_STATUS, &status, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_fup_status input list", provaction_inflistp);
							PCM_OP(ctxp, PCM_OP_WRITE_FLDS, 32, provaction_inflistp, &provaction_outflistp, ebufp);
							PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG, "fm_mso_update_service_fup_status output list", provaction_outflistp);
							PIN_FLIST_DESTROY_EX(&provaction_inflistp, NULL);
							PIN_FLIST_DESTROY_EX(&provaction_outflistp, NULL);	
							if (PIN_ERRBUF_IS_ERR(ebufp))
                                                	{
								PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "Error while updating fup_status ", ebufp);
								PIN_ERRBUF_RESET(ebufp);
								ret_flistp = PIN_FLIST_CREATE(ebufp);
								PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, ret_flistp, PIN_FLD_POID, ebufp );
								PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_offer_failure, ebufp);
								PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51767", ebufp);
								PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR,                                                                                                                                                      "Error while updating service fup status", ebufp);
								*r_flistpp = ret_flistp;
								PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&mso_purch_flistp, NULL);
								PIN_FLIST_DESTROY_EX(&debit_oflistp, NULL);
								return;
							}
						}
						if(fm_mso_update_mso_purplan_status(ctxp,mso_bb_obj, MSO_PROV_IN_PROGRESS, ebufp ) == 0)
						{
							ret_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, ret_flistp, PIN_FLD_POID, ebufp );
							PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_offer_failure, ebufp);
							PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51754", ebufp);
							PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 
										"Error while updating mso_purhcased_plan status.", ebufp);
							*r_flistpp = ret_flistp;
							PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&mso_purch_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&debit_oflistp, NULL);
							return;
						}
						svc_up_flistp = NULL;
						svc_up_flistp = PIN_FLIST_CREATE(ebufp);
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, svc_up_flistp, PIN_FLD_POID, ebufp );
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, svc_up_flistp, PIN_FLD_SERVICE_OBJ, ebufp );
						PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_PROGRAM_NAME, svc_up_flistp, PIN_FLD_PROGRAM_NAME, ebufp );
						
						ret_val = fm_mso_update_ser_prov_status(ctxp, i_flistp, MSO_PROV_IN_PROGRESS, ebufp );
						if(ret_val)
						{
							ret_flistp = PIN_FLIST_CREATE(ebufp);
							PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_ACCOUNT_OBJ, ret_flistp, PIN_FLD_POID, ebufp );
							PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_offer_failure, ebufp);
							PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "51767", ebufp);
							PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, 																			"Error while updating service provisioning status.", ebufp);
							*r_flistpp = ret_flistp;
							PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&mso_purch_flistp, NULL);
							PIN_FLIST_DESTROY_EX(&debit_oflistp, NULL);
							PIN_FLIST_DESTROY_EX(&svc_up_flistp, NULL);
							return;
						}
					}
					PIN_FLIST_DESTROY_EX(&srvc_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&create_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&mso_purch_flistp, NULL);
					PIN_FLIST_DESTROY_EX(&debit_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&svc_up_flistp, NULL);
				}
				else{
					if(is_apply_now = 1){
						PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "No need to provision and update balances.. entry is already tehre..");
					}
				}
				PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &success, ebufp);
				fm_mso_create_lifecycle_evt(ctxp, i_flistp, ret_flistp, ID_ADD_OFFERS, ebufp);
				if (PIN_ERRBUF_IS_ERR(ebufp)){
					PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
					PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
					PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, 
						"Error in creating fm_mso_cust_create_offers lifecycle event", ebufp);
					return;
					
				}
			}
		}
		PIN_FLIST_DESTROY_EX(&srch_oflistp, NULL);
	}else{
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Not a broadband service");
		ret_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_POID, ret_flistp, PIN_FLD_POID,ebufp);
		PIN_FLIST_FLD_COPY(i_flistp, PIN_FLD_SERVICE_OBJ, ret_flistp, PIN_FLD_SERVICE_OBJ, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_STATUS, &create_offer_failure, ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_CODE, "71000", ebufp);
		PIN_FLIST_FLD_SET(ret_flistp, PIN_FLD_ERROR_DESCR, "Invalid Service, not a Broadband service.", ebufp);
		*r_flistpp = ret_flistp;
		return;
	}
		
	*r_flistpp = ret_flistp;
	return;
}
