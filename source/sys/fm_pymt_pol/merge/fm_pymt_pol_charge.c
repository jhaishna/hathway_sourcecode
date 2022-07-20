/*
 *
 *      Copyright (c) 1998 - 2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)%Portal Version: fm_pymt_pol_charge.c:RWSmod7.3.1Int:1:2007-Jun-28 05:30:25 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_PYMT_POL_CHARGE operation. 
 *******************************************************************/


#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_pymt.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"


/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_pymt_pol_charge(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static void 
fm_pymt_pol_charge(
	pcm_context_t   	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_charge_set_reason_id(
	pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        pin_errbuf_t            *ebufp);

static void
fm_pymt_pol_charge_process_result(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	int32                   *reason_id,
	int32			*domain_idp,
	pin_errbuf_t            *ebufp);

static void 
fm_pymt_pol_charge_set_domain_id(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t		*res_flistp,
        int32                   *domain_idp,
        pin_errbuf_t            *ebufp);
/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_CHARGE operation.
 *******************************************************************/
void
op_pymt_pol_charge(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pin_flist_t		*r_flistp = NULL;
	pcm_context_t		*ctxp = connp->dm_ctx;

	if (PIN_ERR_IS_ERR(ebufp))
	{
		return ;
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_CHARGE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_charge opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: The input Flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_charge input flist", i_flistp);
	/**********************************************************
	 * Copy the input flist to r_flsitp as. Do not change the 
	 * Input Flist
	 *********************************************************/
	r_flistp = PIN_FLIST_COPY(i_flistp,ebufp);

	fm_pymt_pol_charge(ctxp, i_flistp, r_flistp,ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		/***************************************************
		 * Log something and return nothing.
		 **************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_charge error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;

	} 
	else
	{

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		*o_flistpp = r_flistp;

		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_pymt_pol_charge return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_pymt_pol_charge()
 ******************************************************************/
static void 
fm_pymt_pol_charge(
	pcm_context_t           *ctxp,
	pin_flist_t             *i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t            *ebufp)
{
	if (PIN_ERR_IS_ERR(ebufp))
	{
		return ;
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	fm_pymt_pol_charge_set_reason_id(ctxp,r_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_charge", ebufp);
	}	

	return;
}

/*******************************************************************
 * fm_pymt_pol_charge_set_reason_id()
 * This function sets the PIN_FLD_REASON_ID
 *******************************************************************/
static void
fm_pymt_pol_charge_set_reason_id(
	pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t		*res_flistp = NULL;
	pin_flist_t		*reason_flistp = NULL;
	pin_flist_t		*p_flistp = NULL;
	pin_flist_t		*iinfo_flistp = NULL;
	pin_cookie_t            cookie = NULL;
	int32			domain_id = PIN_PYMT_REASON_DOMAIN_ID;
	int32			reason_id = 0;
	int32			elem_id  = 0;
	void 			*vp = NULL;
	
	if (PIN_ERR_IS_ERR(ebufp))
	{
		return ;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	vp = (int32 *) PIN_FLIST_FLD_GET(i_flistp,
			PIN_FLD_STR_VERSION, 1, ebufp);
	if(vp){
		domain_id = * (int32 *) vp;
	}

	while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS,
                        &elem_id, 1, &cookie, ebufp)) != NULL) {
                p_flistp = PIN_FLIST_SUBSTR_GET(res_flistp,
                                        PIN_FLD_PAYMENT, 1, ebufp);
		if(p_flistp)
		{
			iinfo_flistp = PIN_FLIST_SUBSTR_GET(p_flistp, 
					PIN_FLD_INHERITED_INFO, 1, ebufp);
			if (iinfo_flistp) {
				fm_pymt_pol_charge_process_result(ctxp, 
						iinfo_flistp,
						&reason_id,
						&domain_id,
						ebufp);
				reason_flistp = PIN_FLIST_ELEM_ADD(res_flistp,
							PIN_FLD_PAYMENT_REASONS,
							0, ebufp);
				PIN_FLIST_FLD_SET(reason_flistp,
						PIN_FLD_REASON_ID,
						(void *) &reason_id, ebufp);
				fm_pymt_pol_charge_set_domain_id(ctxp,
						p_flistp,
						reason_flistp,
						&domain_id,
						ebufp);
				
			}
		}
	}
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,
				"fm_pymt_pol_charge_set_reason_id", ebufp);
	}

	return;
}
/*******************************************************************
 * fm_pymt_pol_chagre_process_result()
 * This function reads the response from vendor
 * and assigns a reason ID for the response
 *******************************************************************/
static void
fm_pymt_pol_charge_process_result(
	pcm_context_t           *ctxp,
	pin_flist_t		*i_flistp,
	int			*reason_id,
	int			*domain_idp,
	pin_errbuf_t		*ebufp)
{
	pin_flist_t             *info_flistp = NULL;
	pin_errbuf_t            any_field_ebuf;
	pin_cookie_t            cookie = NULL;
	pin_fld_num_t		field = 0;
	int32			rec_id = 0;
	pin_charge_result_t	*resultp = NULL;
	if (PIN_ERR_IS_ERR(ebufp))
	{
		return ;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_CLEAR_ERR(&any_field_ebuf);	

	while (PIN_FLIST_ANY_GET_NEXT(i_flistp, &field, 
				&rec_id, &cookie, &any_field_ebuf) != NULL ){
		if (field == PIN_FLD_CC_INFO || field == PIN_FLD_DD_INFO) {
			info_flistp = PIN_FLIST_ELEM_GET(i_flistp, 
					field, 0, 0, ebufp);
			resultp = (pin_charge_result_t *)PIN_FLIST_FLD_GET(info_flistp,
					PIN_FLD_RESULT, 0,ebufp);
			/*******************************************
			 * Set the PIN_FLD_RESULT_ID in the PIN_FLD_RESULTS 
			 * array
			 ******************************************/
			if (resultp){
				/******************************
				 *Set Default Reason ID to 0
				 *******************************/
				switch (*resultp) {
				case PIN_CHARGE_RES_PASS: 
					*reason_id = PIN_REASON_ID_APPROVED;
					*domain_idp = PIN_PYMT_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_NO_ANS:
					*reason_id = PIN_REASON_ID_FAIL_NO_ANS;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_ADDR_AVS:
					*reason_id = PIN_REASON_ID_FAIL_ADDR_AVS;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_ADDR_LOC:
					*reason_id = PIN_REASON_ID_FAIL_ADDR_LOC;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_ADDR_ZIP:
					*reason_id = PIN_REASON_ID_FAIL_ADDR_ZIP;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_CARD_BAD:
					*reason_id = PIN_REASON_ID_FAIL_CARD_BAD;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_DECL_SOFT:
					*reason_id = PIN_REASON_ID_FAIL_DECL_SOFT;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_SRVC_UNAVAIL:
					*reason_id = PIN_REASON_ID_FAIL_SRVC_UNAVAIL;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_DECL_HARD:
					*reason_id = PIN_REASON_ID_FAIL_DECL_HARD;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_NO_MIN:
					*reason_id = PIN_REASON_ID_FAIL_NO_MIN;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_INVALID_CMD:
					*reason_id = PIN_REASON_ID_INVALID_CMD;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_SELECT_ITEMS:
					*reason_id = PIN_REASON_ID_FAIL_SELECT_ITEMS;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_CVV_BAD:
					*reason_id = PIN_REASON_ID_FAIL_CVV_BAD;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_NO_CREDIT_BALANCE:
					*reason_id =  PIN_REASON_ID_NO_CREDIT_BALANCE;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_LOGICAL_PROBLEM:
					*reason_id = PIN_REASON_ID_FAIL_LOGICAL_PROBLEM;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_FORMAT_ERROR:
					*reason_id = PIN_REASON_ID_FAIL_FORMAT_ERROR;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_INVALID_CONTENT:
					*reason_id = PIN_REASON_ID_FAIL_INVALID_CONTENT;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				case PIN_CHARGE_RES_FAIL_TECHNICAL_PROBLEM:
					*reason_id = PIN_REASON_ID_FAIL_TECHNICAL_PROBLEM;
					*domain_idp = PIN_PYMT_FAILED_REASON_DOMAIN_ID;
					break;
				default:
					*reason_id = PIN_REASON_ID_DEFAULT;
					/*******************************************
					 * Setting the default reason domain
					 * since we are setting default reason ID
					 * The following line can be commented if
					 * the domain id has to be retained as 
					 * it was passed in the function.
					 ******************************************/
					*domain_idp = PIN_PYMT_REASON_DOMAIN_ID;
					break;
				}
			}
                }
	}
	/*************************************************
	 * Check Error buf and Debug it
	 ************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_DEBUG,
                                "fm_pymt_pol_charge_process_result", ebufp);
	}

	return;
}
/*****************************************************************
 * fm_pymt_pol_charge_set_domain_id()
 * Function to set the reason domain ID
 ******************************************************************/
static void
fm_pymt_pol_charge_set_domain_id(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
	pin_flist_t		*res_flistp,
        int32                   *domain_idp,
        pin_errbuf_t            *ebufp)
{
        void                    *vp = NULL;

        if (PIN_ERR_IS_ERR(ebufp))
        {
                return ;
        }
        PIN_ERR_CLEAR_ERR(ebufp);
	vp = PIN_FLIST_FLD_GET(i_flistp,
			PIN_FLD_REASON_DOMAIN_ID, 1,ebufp);
	/*****************************************************
	 * Set the domain ID in the Results array
	 *****************************************************/
	if(!vp){
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_REASON_DOMAIN_ID,
					(void *) domain_idp, ebufp);

        }
	else{
		PIN_FLIST_FLD_SET(res_flistp, PIN_FLD_REASON_DOMAIN_ID,
						vp, ebufp);
	}

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_pymt_pol_charge_set_domain_id", ebufp);
        }
        return ;
}
