/*******************************************************************
 *
* Copyright (c) 1996, 2018, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_pymt_pol_pre_collect.c /cgbubrm_mainbrm.portalbase/1 2018/10/03 00:16:23 agangrad Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include "pcm.h"
#include "ops/pymt.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_bdfs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "psiu_business_params.h"
#include "fm_utils_bparams_cache.h"

EXPORT_OP void
op_pymt_pol_pre_collect(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_find_outstanding_chkpts(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**chkpt_flistpp,
	pin_errbuf_t	*ebufp);

static int32
fm_pymt_pol_is_chkpt_outstanding(
	poid_t		*a_pdp,
	pin_flist_t	*chkpt_flistp,
	pin_errbuf_t	*ebufp);

static int32
fm_pymt_pol_pre_collect_validate_refund(
	poid_t		*ctxp,
	pin_decimal_t	*amtp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp);

static int32
fm_pymt_pol_pre_collect_billinfo_has_refund(        
	pcm_context_t   *ctxp,
	poid_t		*a_pdp,
	poid_t          *billinfo_pdp,        
	pin_errbuf_t    *ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_PRE_COLLECT  command
 *******************************************************************/
void
op_pymt_pol_pre_collect(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t		*c_flistp = NULL;
	pin_flist_t		*chkpt_flistp = NULL;
	pin_flist_t		*rc_flistp = NULL;
	pin_charge_cmd_t	*cmdp = NULL;
	pin_decimal_t		*amtp = NULL;
	poid_t			*a_pdp = NULL;
	double			*d_ptr = NULL;
	char			*descr = NULL;
	int32			*selectp = NULL;
	int32			elemid = 0;
	int32			result = 0;
	int32			err = 0;
	int32			flag = 0;
	int32			sflag = 0;
	int32			rec_id = 0;
	pin_decimal_t		*minimum_pay = NULL;
	pin_decimal_t		*minimum_ref = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*a_flistp = NULL;
	pin_flist_t		*b_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*trans_flistp = NULL;
	poid_t			*s_pdp	  = NULL;	  
	int32			*mandate_status = 0;
	pin_pymt_result_t	*statusp = NULL;
	int32			*resultp  = NULL;
	char                    vendor_name[256] = {" "};
 	void                    *vp = NULL;
	char			*dd_vendorp = NULL;
	char			min_ref_conf[8] = {0};
	char			min_pay_conf[8] = {0};
	
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = (pin_flist_t *)NULL;

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_PRE_COLLECT) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_pymt_pol_pre_collect", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_pre_collect input flist", in_flistp);

	/***********************************************************
	 * Get command from very first element of array
	 * note: we cannot rely that it has index [0] or [1] or ...
	 ***********************************************************/
	flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp, PIN_FLD_CHARGES, 
					&elemid, 0, &cookie, ebufp);

	cmdp = (pin_charge_cmd_t *)PIN_FLIST_FLD_GET(flistp,
					PIN_FLD_COMMAND, 0, ebufp);
	



	/***********************************************************
	 * Select the list of outstanding checkpoint events.
	 * Checkpoints' existance has sense for following commands only: 
	 * authorization, deposit, conditional deposit and refund
	 ***********************************************************/
	if (cmdp && 
		((*cmdp == PIN_CHARGE_CMD_CONDITION) ||
		 (*cmdp == PIN_CHARGE_CMD_REFUND) ||
		 (*cmdp == PIN_CHARGE_CMD_AUTH_ONLY) ||
		 (*cmdp == PIN_CHARGE_CMD_DEPOSIT))) {
		/***********************************************************
		 * New algorithm is to search for checkpoint upfront and
		 * pass a flag that indicates that search is done
		 ***********************************************************/
		pin_flist_t		*b_flistp = NULL;
		int32			*b_flags = 0;

		/***********************************************************
		 * Get checkpoints' flag from very first element of array
		 * note: we cannot rely that it has index [0] or [1] or ...
		 ***********************************************************/
		elemid = 0;
		cookie = NULL;
		b_flistp = PIN_FLIST_ELEM_GET_NEXT(in_flistp,PIN_FLD_BATCH_INFO,
						   &elemid, 1, &cookie, ebufp);

		if (b_flistp != 0) {
			b_flags = (int32*)PIN_FLIST_FLD_GET(b_flistp,
						PIN_FLD_STATUS_FLAGS, 1, ebufp);
		}

		/***********************************************************
		 * 1. If no flags do checkpoints selection
		 * 2. If flag is not set do checkpoint selection
		 * 3. If flag is set don't perform checkpoint selection
		 ***********************************************************/
		if(!((b_flags != 0)&&(*b_flags & PIN_PAYMENT_BATCH_CHKPTS))){
			fm_pymt_pol_find_outstanding_chkpts(ctxp, in_flistp, 
							&chkpt_flistp, ebufp);
		}
	}

	/***********************************************************
	 * Get the minimum charge amount from the pin.conf. If the
	 * entry is missing from the pin.conf assume the default
	 * minimum value of 2.00
	 ***********************************************************/
	pin_conf("fm_pymt_pol", "minimum_payment", PIN_FLDT_NUM, 
		(caddr_t *)&d_ptr, &err);
	if (d_ptr) {
		minimum_pay = pbo_decimal_from_double(*d_ptr, ebufp);
		pin_snprintf(min_pay_conf,sizeof(min_pay_conf),"%.2f",*d_ptr);
		fm_utils_bparam_cache_update_from_pinconf(PSIU_BPARAMS_AR_PARAMS,
			PSIU_BPARAMS_MINIMUM_PAYMENT,min_pay_conf,ebufp);
		free(d_ptr);
        } else {
		minimum_pay = fm_utils_bparams_cache_get_decimal(ctxp, PSIU_BPARAMS_AR_PARAMS,
			PSIU_BPARAMS_MINIMUM_PAYMENT, 1, ebufp);
		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_CLEAR_ERR(ebufp);
			minimum_pay = pbo_decimal_from_str("2.00", ebufp);
		}
		
        }

	/***********************************************************
	 * Get the minimum refund amount from the pin.conf. If the
	 * entry is missing from the pin.conf assume the default
	 * minimum value of -2.00
	 ***********************************************************/
	pin_conf("fm_pymt_pol", "minimum_refund", PIN_FLDT_NUM, 
			(caddr_t *)&d_ptr, &err);
	if (d_ptr) {
		minimum_ref = pbo_decimal_from_double(*d_ptr, ebufp);
		pin_snprintf(min_ref_conf,sizeof(min_ref_conf),"%.2f",*d_ptr);
		fm_utils_bparam_cache_update_from_pinconf(PSIU_BPARAMS_AR_PARAMS,PSIU_BPARAMS_MINIMUM_REFUND,
				min_ref_conf,ebufp);
		free(d_ptr);
        } else {
		minimum_ref = fm_utils_bparams_cache_get_decimal(ctxp, PSIU_BPARAMS_AR_PARAMS,
		PSIU_BPARAMS_MINIMUM_REFUND, 1, ebufp);
		if (PIN_ERR_IS_ERR(ebufp)) {
                        PIN_ERR_CLEAR_ERR(ebufp);
			minimum_ref = pbo_decimal_from_str("2.00", ebufp);
		}
	}

	/***********************************************************
	 * Get the dd_vendor value from the pin.conf. 
	 ***********************************************************/
	pin_conf("cm", "dd_vendor", PIN_FLDT_STR, 
			(caddr_t *)&dd_vendorp, &err);

	c_flistp = PIN_FLIST_COPY(in_flistp, ebufp);

	/***********************************************************
	 * Walk the charges array and check each element.
	 ***********************************************************/
	elemid = 0;
	cookie = NULL;

	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(c_flistp, PIN_FLD_CHARGES,
		&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		/**********************************************************
		 * Drop PIN_FLD_TRANSACTIONS arrays from CHARGES.
		 * To use/enable Stored Credential (CIT/MIT) feature
		 * comment the next 3 lines of "code"
		 *	
		 * If using Stored credential feature, make sure to
		 * add transaction type(PIN_FLD_TYPE) corresponding to
		 * the message type set on PIN_FLD_TRANSACTIONS &
		 * If message type is CIT, Transaction Id(PIN_FLD_TRANS_ID)
		 * should not be present on PIN_FLD_TRANSACTIONS
		 *********************************************************/
		trans_flistp = (pin_flist_t *)PIN_FLIST_ELEM_TAKE(flistp,
				PIN_FLD_TRANSACTIONS, PIN_ELEMID_ANY, 1, ebufp);
		PIN_FLIST_DESTROY_EX(&trans_flistp, NULL);	

					
		statusp = (pin_pymt_result_t *) PIN_FLIST_FLD_GET(flistp,
				PIN_FLD_STATUS, 1, ebufp);
		resultp = (int32 *) PIN_FLIST_FLD_GET(flistp,PIN_FLD_RESULT, 1, ebufp);
                if((statusp && *statusp >= PIN_PYMT_SUSPENSE 
				&& *statusp < PIN_PYMT_FAILED) 
			|| (resultp && *resultp == PIN_PAYMENT_RES_FAIL)){
			/*******************************************
			 * Do nothing if a payment in suspense
			 *******************************************/
			continue;
		}

		amtp = (pin_decimal_t *)PIN_FLIST_FLD_GET(flistp, 
			PIN_FLD_AMOUNT, 1, ebufp);
		cmdp = (pin_charge_cmd_t *)PIN_FLIST_FLD_GET(flistp,
			PIN_FLD_COMMAND, 0, ebufp);
		a_pdp = (poid_t *)PIN_FLIST_FLD_GET(flistp, 
			PIN_FLD_ACCOUNT_OBJ,0,ebufp);
		selectp = (int *)PIN_FLIST_FLD_GET(flistp,
			PIN_FLD_SELECT_RESULT, 1, ebufp);

		/***************************************************
		 * Did PCM_OP_PYMT_SELECT_ITEMS retured OK?
		 ***************************************************/
		if (selectp && (*selectp != PIN_SELECT_RESULT_PASS)){
			result = PIN_CHARGE_RES_FAIL_SELECT_ITEMS;
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, 
				(void *)&result, ebufp);

			descr = "Select Items Failed";
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, 
				(void *)descr, ebufp);

			continue;
		}

		/***************************************************
		 * Get the amount for this charge. If the amount to
		 * be charged is 0.0, then don't even bother dealing
		 * with this element.
		 ***************************************************/
		if ((amtp && 
			pbo_decimal_compare(amtp, minimum_pay, ebufp) < 0) &&
			 (cmdp &&  ((*cmdp == PIN_CHARGE_CMD_CONDITION) ||
			(*cmdp == PIN_CHARGE_CMD_AUTH_ONLY) ||
			(*cmdp == PIN_CHARGE_CMD_DEPOSIT)))) {
			result = PIN_CHARGE_RES_FAIL_NO_MIN;
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, 
				(void *)&result, ebufp);

			descr = "Below Minimum";
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, 
				(void *)descr, ebufp);

			continue;
		}

		/***************************************************
		 * Set the minimum refund to compare.
		 ***************************************************/
		if (cmdp && ((*cmdp == PIN_CHARGE_CMD_NONE) &&
			(pbo_decimal_sign(amtp, ebufp) < 0))) {

			pbo_decimal_negate_assign(minimum_ref, ebufp);
		}

		/***************************************************
		 * If command and amount are not NULL and if the
		 * command is refund or if the commnd is none, but
		 * has a negative amount, check the amount with
		 * the minimum refund. 
		 ***************************************************/
		if (cmdp && amtp && (((*cmdp == PIN_CHARGE_CMD_REFUND) &&
			(pbo_decimal_compare(amtp, minimum_ref, ebufp) < 0)) ||
			((*cmdp == PIN_CHARGE_CMD_NONE) && 
			(pbo_decimal_sign(amtp, ebufp) < 0) &&
			(pbo_decimal_compare(amtp, minimum_ref, ebufp) > 0)))) {

			result = PIN_CHARGE_RES_FAIL_NO_MIN;
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, 
					(void *)&result, ebufp);

			descr = "Below Minimum";
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, 
					(void *)descr, ebufp);

			continue;
		}

		/***************************************************
		 * If command and amount are not NULL and if the
		 * command is REFUND or if the command is NONE and
		 * its a negative amount, make sure that the account
		 * has sufficient credit balance. If not, set the 
		 * RESULT code to be NO_CREDIT_BALANCE.
		 ***************************************************/
		if (cmdp && amtp && ((*cmdp == PIN_CHARGE_CMD_REFUND) ||
			((*cmdp == PIN_CHARGE_CMD_NONE) && 
			(pbo_decimal_sign(amtp, ebufp) < 0)))) {

			flag = fm_pymt_pol_pre_collect_validate_refund(ctxp,
					amtp, flistp, ebufp);
			if (flag == PIN_BOOLEAN_FALSE) {

				result = PIN_CHARGE_RES_NO_CREDIT_BALANCE;
				PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, 
						(void *)&result, ebufp);

				descr = "No credit available";
				PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, 
						(void *)descr, ebufp);
			}
		}

		/***************************************************
		 * Is there a outstanding checkpoint for this acct?
		 ***************************************************/
		if ((chkpt_flistp != (pin_flist_t *)NULL) && cmdp &&
			((*cmdp == PIN_CHARGE_CMD_CONDITION) ||
			(*cmdp == PIN_CHARGE_CMD_REFUND) ||
			(*cmdp == PIN_CHARGE_CMD_AUTH_ONLY) ||
			(*cmdp == PIN_CHARGE_CMD_DEPOSIT))) {

			if (fm_pymt_pol_is_chkpt_outstanding(a_pdp,
				chkpt_flistp, ebufp)) {

				result = PIN_CHARGE_RES_CHECKPOINT;
				PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, 
					(void *)&result, ebufp);

				descr = "Checkpoint Outstanding";
				PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, 
					(void *)descr, ebufp);

				continue;
			}
		}
		s_flistp = PIN_FLIST_CREATE(ebufp);
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_ACH, 1, ebufp);
		if (vp) {
			vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0,
								 ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, 
						vp, ebufp);
			vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_ACH, 0,
                                                                 ebufp);	
			if (vp) {
				rec_id = *(int32 *)vp;
			}
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACH, 
						&rec_id, ebufp);

		        PCM_OP(ctxp, PCM_OP_PYMT_GET_ACH_INFO, sflag,
					s_flistp, &r_flistp, ebufp);

			rc_flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS,
								 0, 0, ebufp);
			vp = PIN_FLIST_FLD_GET(rc_flistp,
       		                 		PIN_FLD_NAME, 0, ebufp);
			memset(vendor_name, 0, sizeof(vendor_name));
			if ((vp != (void *)NULL) && strlen(vp)) {
				strcpy(vendor_name, (char *)vp);
			}
		}
		 PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
                 PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

		/*************************************************************
		 * If Bertelsmann then check the mandate status.
		 *************************************************************/
		if (dd_vendorp && !strcasecmp(dd_vendorp,
			"Bertelsmann") && cmdp && *cmdp != PIN_CHARGE_CMD_RFR 
			&& vendor_name && !strcasecmp(vendor_name, 
			"bertelsmann")) {

			s_flistp = PIN_FLIST_CREATE(ebufp);
			s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp),
				"/search/pin", 0, ebufp);
			PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, 
						(void *)s_pdp, ebufp);
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, 
						(void *)&sflag, ebufp);
		
			PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, 
				"select X from /payinfo where F1 = V1 ", ebufp);

			a_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 
								1, ebufp);
			PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_ACCOUNT_OBJ, 
								a_pdp, ebufp);

			a_flistp = PIN_FLIST_ELEM_ADD(s_flistp, 
						PIN_FLD_RESULTS, 0, ebufp);
			b_flistp = PIN_FLIST_ELEM_ADD(a_flistp, 
						PIN_FLD_DD_INFO, 0, ebufp);
			PIN_FLIST_FLD_SET(b_flistp, PIN_FLD_MANDATE_STATUS, 
							(void *)0, ebufp);
			
			PCM_OP(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, 
									ebufp);

			if (r_flistp != (pin_flist_t *)NULL) {
				a_flistp = PIN_FLIST_ELEM_GET(r_flistp, 
						PIN_FLD_RESULTS, 0, 1, ebufp);
				b_flistp = PIN_FLIST_ELEM_GET(a_flistp, 
						PIN_FLD_DD_INFO, 0, 1, ebufp);

				/***********************************************
			 	* Do not allow accounts with the following
			 	* mandate_statuses to go to collection.
			 	***********************************************/
				mandate_status = PIN_FLIST_FLD_GET(b_flistp, 
						PIN_FLD_MANDATE_STATUS, 1, ebufp);
			
				if (!mandate_status || 
				   ((*mandate_status != PIN_MANDATE_RECEIVED) &&
				   (*mandate_status != PIN_MANDATE_NOT_REQUIRED)))
				{
					result = PIN_CHARGE_RES_FAIL_SELECT_ITEMS;
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, 
						(void *)&result, ebufp);

					descr = "Mandate Outstanding";
					PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, 
						(void *)descr, ebufp);
				}
			}

			/***********************************************
			 * Clean up.
			 ***********************************************/
			PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		}

		/***************************************************
		 * Check the command for this element.
		 ***************************************************/
		if (cmdp == NULL) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_APPLICATION, 
				PIN_ERR_MISSING_ARG,
				PIN_FLD_COMMAND, elemid, 0);

			continue;
		}

		/***************************************************
		 * Is it ok?
		 ***************************************************/

		switch (*cmdp) {
		case PIN_CHARGE_CMD_NONE:
		case PIN_CHARGE_CMD_AUTH_ONLY:
		case PIN_CHARGE_CMD_CONDITION:
		case PIN_CHARGE_CMD_DEPOSIT:
		case PIN_CHARGE_CMD_REFUND:
		case PIN_CHARGE_CMD_RFR:
		case PIN_CHARGE_CMD_RESUBMIT:
			break;
		default:
			result = PIN_CHARGE_RES_INVALID_CMD;
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, 
				(void *)&result, ebufp);

			descr = "Invalid Command";
			PIN_FLIST_FLD_SET(flistp, PIN_FLD_DESCR, 
				(void *)descr, ebufp);
		}
	}

	/***********************************************************
	 * Clean up.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&chkpt_flistp, NULL);
	pbo_decimal_destroy(&minimum_pay);
	pbo_decimal_destroy(&minimum_ref);

	if (dd_vendorp != (char *)NULL) {
		free(dd_vendorp);
		dd_vendorp = NULL;
	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(&c_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_pre_collect error", ebufp);
	} else {
		*ret_flistpp = c_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_pymt_pol_pre_collect return flist", 
			*ret_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_pymt_pol_find_outstanding_chkpts():
 *
 *	Search for all outstanding checkpoints in the system.
 *
 *******************************************************************/
static void
fm_pymt_pol_find_outstanding_chkpts(ctxp, i_flistp, chkpt_flistpp, ebufp)
	pcm_context_t	*ctxp;
	pin_flist_t	*i_flistp;
	pin_flist_t	**chkpt_flistpp;
	pin_errbuf_t	*ebufp;
{
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*a_flistp = NULL;
	pin_flist_t	*flistp = NULL;	
	poid_t		*s_pdp = NULL;
	poid_t		*a_pdp = NULL;
	void		*vp = NULL;
	char 		template[256] = {""};
	int32		dummy = 0;
	int32		s_flags = 0;
	int32		pay_type = 0;
	int32		rec_id = 0;
	pin_cookie_t	cookie = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************************
	* Check in the charges array for the  pay_type. If it is not present 
	* then read the account object for the pay_type. 
	* Assumption made is the batch has charges pertaining to one pay_type.
	***********************************************************************/
	flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_CHARGES, 
						&rec_id, 0, &cookie, ebufp);
	vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_PAY_TYPE, 1, ebufp);

	if (vp) pay_type = *(int32 *)vp;
	else {
		a_flistp = PIN_FLIST_CREATE(ebufp);
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
		if (!vp) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_find_outstanding_chkpts - No BILLINFO OBJ error", ebufp);
			PIN_FLIST_DESTROY_EX(&a_flistp, NULL);
			return;
		}
			
		PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_POID, vp, ebufp);
		PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_PAY_TYPE, (void *)0, ebufp);

		PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, a_flistp, &r_flistp, ebufp);

		vp = PIN_FLIST_FLD_GET(r_flistp, PIN_FLD_PAY_TYPE, 0, ebufp);

		if (vp) pay_type = *(int32 *)vp;

		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_find_outstanding_chkpts error", ebufp);
			PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&a_flistp, NULL);
			return;
		}

		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&a_flistp, NULL);
	}
		
	/***********************************************************
	 * Null out the return flist.
	 ***********************************************************/
	*chkpt_flistpp = NULL;

	/***********************************************************
	 * Allocate the search flist.
	 ***********************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Create a 2 arg dynamic search template for event table.
	 ***********************************************************/
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, 
		PIN_FLD_POID, 0, ebufp);
	s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp), 
			"/search/pin", 0, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);

	/***********************************************************
	 * First check checkpoints, if so get accounts
	 ***********************************************************/

	/***********************************************************
	 * Search template is substitution for SQL count(*) clause
	 * with some optimizations
	 ***********************************************************/
	template[sizeof(template) - 1] = (char)NULL;
	snprintf (template, sizeof(template) - 1,
		"select X from /account where poid_id0 = 1 and F1 like V1 and "
              	"exists ( select obj_id0 from event_billing_charge_t "
              	"where obj_id0 > 0 and result >= %d and pay_type = %d ) "
              	,(int32)PIN_CHARGE_RES_AUTH_PENDING, pay_type);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,
					  (void *)template, ebufp);

	/*dummy arg to make DM parser happy*/
	s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp),"/account", -1, ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, s_pdp , ebufp);

	/***********************************************************
	 * Create the search results.
	 ***********************************************************/
	vp = (void *)NULL;
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, vp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_pre_collect checkpoint search flist", s_flistp);

	/***********************************************************
	 * Do the search.
	 ***********************************************************/
	PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_READ_UNCOMMITTED|PCM_OPFLG_SEARCH_DB, 
		   s_flistp, &r_flistp, ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_pre_collect checkpoint search-result flist", r_flistp);

	/***********************************************************
	 * Did we find any checkpoints?
	 ***********************************************************/
	if (PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp)) {
	 
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	
		/*******************************************************
		 * Search template.
		 *******************************************************/
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE,
				  "select X from /event/billing/charge "
				  "where event_billing_charge_t.obj_id0 > 0 "
                                  "and F1 >= V1 and F2 = V2 ", ebufp);

		/***********************************************************
		 * 1st arg. PIN_FLD_RESULT >= PIN_CHARGE_RES_AUTH_PENDING (888)
		 ***********************************************************/	
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
		flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_CHARGE, ebufp);
		dummy = (int)PIN_CHARGE_RES_AUTH_PENDING;
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, 
						  (void *)&dummy, ebufp);
	
		/***********************************************************
		 * 2nd arg. PIN_FLD_PAY_TYPE = pay_type
		 ***********************************************************/
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
		flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_CHARGE, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_PAY_TYPE, &pay_type, ebufp);
	
		/***********************************************************
		 * Update the search results.
		 ***********************************************************/
		vp = (void *)NULL;
		flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, vp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_pre_collect account search flist", s_flistp);

		/***********************************************************
		 * Do the search.
		 ***********************************************************/
		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_READ_UNCOMMITTED|PCM_OPFLG_SEARCH_DB, 
			   s_flistp, &r_flistp, ebufp);

		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_pre_collect account search-result flist", r_flistp);

		/***********************************************************
		 * Return collection of selected accounts
		 ***********************************************************/
		if (PIN_FLIST_ELEM_COUNT(r_flistp, PIN_FLD_RESULTS, ebufp)) {
			*chkpt_flistpp = r_flistp;
		} 

	} else {	
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	}

	/***********************************************************
	 * Clean up.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_find_outstanding_chkpts error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_pymt_pol_is_chkpt_outstanding():
 *
 *
 *******************************************************************/
static int32
fm_pymt_pol_is_chkpt_outstanding(a_pdp, chkpt_flistp, ebufp)
	poid_t		*a_pdp;
	pin_flist_t	*chkpt_flistp;
	pin_errbuf_t	*ebufp;
{
	pin_cookie_t	cookie = NULL;
	pin_flist_t	*flistp = NULL;	
	poid_t		*pdp = NULL;

	int32 	rec_id = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return (PIN_BOOLEAN_FALSE);
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_is_chkpt_outstanding error", ebufp);
	}

	/***********************************************************
	 * Walk thru the list of all outstanding checkpoints and
	 * look for any checkpoint for the account passed in.
	 ***********************************************************/
	while ((flistp = PIN_FLIST_ELEM_GET_NEXT(chkpt_flistp, PIN_FLD_RESULTS,
		&rec_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		pdp = (poid_t *)PIN_FLIST_FLD_GET(flistp,
			PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

		if (PIN_POID_COMPARE(a_pdp, pdp, 0, ebufp) == 0) {
			return (PIN_BOOLEAN_TRUE);
		}
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_is_chkpt_outstanding error", ebufp);
	}

	return (PIN_BOOLEAN_FALSE);
}

/*******************************************************************
 * fm_pymt_pol_pre_collect_validate_refund():
 *
 * 	Validate the refund.
 *******************************************************************/
static int32
fm_pymt_pol_pre_collect_validate_refund(ctxp, amtp, i_flistp, ebufp)
	pcm_context_t	*ctxp;
	pin_decimal_t	*amtp;
	pin_flist_t	*i_flistp;
	pin_errbuf_t	*ebufp;
{
	int32           srch_flags = SRCH_DISTINCT;
	pin_flist_t     *srch_flistp = NULL;
	pin_flist_t	*flistp = NULL;	
	pin_flist_t	*res_flistp = NULL;	
	pin_flist_t     *billinfo_flistp = NULL;
	int32		ret_val = PIN_BOOLEAN_FALSE;
	int32           elem_id = 0;
	poid_t		*a_pdp = NULL;
	poid_t		*pdp = NULL;
	poid_t          *srch_pdp = NULL;
	void		*vp = NULL;
	pin_cookie_t    cookie = NULL;
        pin_flist_t     *bill_flistp = NULL;
        pin_flist_t     *bill_read_flistp = NULL;
        pin_flist_t     *bill_ret_flistp = NULL;
        poid_t          *bill_pdp = NULL;
	poid_t          *billinfo_pdp = NULL;
	int32		is_bill_obj = PIN_BOOLEAN_FALSE;


	if (PIN_ERR_IS_ERR(ebufp))
		return (PIN_BOOLEAN_FALSE);
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Get the account poid
	 ***********************************************************/
	a_pdp = PIN_FLIST_FLD_GET(i_flistp, 
		PIN_FLD_ACCOUNT_OBJ, 0, ebufp);

        /***********************************************************
         * Check if BILLS array is specified. If yes then consider
         * only that billinfo.
         ***********************************************************/
        bill_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_BILLS, PIN_ELEMID_ANY, 1, ebufp);

        if (bill_flistp) {

                bill_pdp = (poid_t *)PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_BILL_OBJ, 1, ebufp);

                if (!PIN_POID_IS_NULL(bill_pdp)) {
                        bill_read_flistp = PIN_FLIST_CREATE(ebufp);

                        PIN_FLIST_FLD_SET(bill_read_flistp, PIN_FLD_POID, bill_pdp, ebufp);
                        PIN_FLIST_FLD_SET(bill_read_flistp, PIN_FLD_BILLINFO_OBJ, NULL, ebufp);

                        PCM_OPREF(ctxp, PCM_OP_READ_FLDS, 0, bill_read_flistp, &bill_ret_flistp, ebufp);
                        PIN_FLIST_DESTROY_EX(&bill_read_flistp, NULL);

			billinfo_pdp = (poid_t *)PIN_FLIST_FLD_GET(bill_ret_flistp, PIN_FLD_BILLINFO_OBJ, 0, ebufp);
			ret_val = fm_pymt_pol_pre_collect_billinfo_has_refund(ctxp, billinfo_pdp, a_pdp, ebufp);
			PIN_FLIST_DESTROY_EX(&bill_ret_flistp, NULL);
			is_bill_obj = PIN_BOOLEAN_TRUE;
                }
	}
	if (is_bill_obj == PIN_BOOLEAN_FALSE) {
		/***********************************************************
		 * Create the flist to get all billinfos of that account.
		 ***********************************************************/
		srch_flistp = PIN_FLIST_CREATE(ebufp);

		srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp), "/search", -1, ebufp);
		PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, srch_pdp, ebufp);

		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);

	       	vp = (void *) "select X from /billinfo where F1 = V1";
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, vp, ebufp);

		flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_ACCOUNT_OBJ, a_pdp, ebufp);

		flistp = PIN_FLIST_ELEM_ADD (srch_flistp, PIN_FLD_RESULTS, 0, ebufp);
		PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, NULL, ebufp);

		PCM_OPREF(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);

		/***************************************************************
		 * Validate refund for every billinfo belonging to that account
		 ***************************************************************/
		while ((billinfo_flistp = PIN_FLIST_ELEM_GET_NEXT(res_flistp, PIN_FLD_RESULTS,
                	         &elem_id, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

			billinfo_pdp = (poid_t *)PIN_FLIST_FLD_GET(billinfo_flistp, PIN_FLD_POID, 0, ebufp);
			ret_val = fm_pymt_pol_pre_collect_billinfo_has_refund(ctxp, billinfo_pdp, a_pdp, ebufp);
			if (ret_val == PIN_BOOLEAN_TRUE) {
				break;
			}
		}
		PIN_FLIST_DESTROY_EX(&res_flistp, NULL);
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_pre_collect_validate_refund error", 
			ebufp);
	}

	return (ret_val);
}

/*******************************************************************
 * fm_pymt_pol_pre_collect_billinfo_has_refund()
 * 
 * Input:
 *	billinfo_pdp	- Billinfo Poid
 *	a_pdp		- Account Poid
 *
 * Output:
 *	0 - when billinfo does not have refund
 *	1 - when billinfo has refund
 *
 * Functionality:
 *	Checks if billinfo has any refund amount
 *******************************************************************/
static int32
fm_pymt_pol_pre_collect_billinfo_has_refund(
	pcm_context_t	*ctxp,
	poid_t		*billinfo_pdp,
	poid_t		*a_pdp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t     *flistp = NULL;
	pin_flist_t     *r_flistp = NULL;
	poid_t		*s_pdp = NULL;
	int32		s_flags = SRCH_CALC_ONLY_1;
	void		*vp = NULL;
	int32           status = 0;
	int32		ret_val = PIN_BOOLEAN_FALSE;
	pin_decimal_t	*amountp = NULL;
	pin_decimal_t	*dummyp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return (PIN_BOOLEAN_FALSE);
	PIN_ERR_CLEAR_ERR(ebufp);

	s_flistp = PIN_FLIST_CREATE(ebufp);

	s_pdp = PIN_POID_CREATE(PIN_POID_GET_DB(a_pdp),
				"/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, s_pdp, ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);                

	vp = (void *) "select sum( F1 ) from /item where F2 = V2 "
				"and ( F3 = V3  or F4 = V4 ) ";
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TEMPLATE, vp, ebufp);

	dummyp = pin_decimal("0.0", ebufp);
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	PIN_FLIST_FLD_PUT(flistp, PIN_FLD_DUE, dummyp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_AR_BILLINFO_OBJ, billinfo_pdp, ebufp);

	status = PIN_ITEM_STATUS_OPEN;
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, &status, ebufp);

	status = PIN_ITEM_STATUS_PENDING;
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_STATUS, &status, ebufp);

	flistp = PIN_FLIST_ELEM_ADD ( s_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_AMOUNT, 1, ebufp );

	PCM_OPREF(ctxp, PCM_OP_SEARCH, 0, s_flistp, &r_flistp, ebufp);
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

	/***********************************************************
	 * Get the currency resource balance for the account.
	 ***********************************************************/
	flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);

	amountp = (pin_decimal_t *)PIN_FLIST_ELEM_GET(flistp, PIN_FLD_AMOUNT, 
						PIN_ELEMID_ANY, 1, ebufp);

	/***********************************************************
	 * If billinfo has refundable amount return true.
	 ***********************************************************/
	if (!pbo_decimal_is_null(amountp, ebufp)) {
		if (pbo_decimal_sign(amountp, ebufp) < 0) {
			ret_val = PIN_BOOLEAN_TRUE;
		}
	}

	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_pymt_pol_pre_collect_billinfo_has_refund error",
                        ebufp);
        }
	return ret_val;
}
