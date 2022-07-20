/*
 *
 * Copyright (c) 2004, 2009, Oracle and/or its affiliates. 
 * All rights reserved. 
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_pymt_pol_apply_fee.c /cgbubrm_7.5.0.portalbase/1 2015/11/27 05:45:23 nishahan Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_PYMT_POL_APPLY_FEE operation. 
 *******************************************************************/

/*******************************************************************
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
op_pymt_pol_apply_fee(
	cm_nap_connection_t	*connp,
	int			opcode,
	int			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void 
fm_pymt_pol_apply_fee(
	pcm_context_t   	*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		*r_flistp,
	pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_apply_fee_set_flag(
        pcm_context_t           *ctxp,
        pin_flist_t             *i_flistp,
        int                     *apply_fee,
        pin_errbuf_t            *ebufp);
static void 
fm_pymt_pol_apply_fee_parse_customer_segment(
	char                    *cust_segmentp,
	int                     *cust_segment_idp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_APPLY_FEE operation.
 *******************************************************************/
void
op_pymt_pol_apply_fee(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	void 			*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
	{
		return ;
	}
	PIN_ERR_CLEAR_ERR(ebufp);
	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_APPLY_FEE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_apply_fee opcode error", ebufp);
		*o_flistpp = NULL;
		return;
	}
	
	/***********************************************************
	 * Debug: The input Flist
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_apply_fee input flist", i_flistp);

	/***********************************************************
	 * Prep the return flist.
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Get the poid from the Input Flist.
	 ***********************************************************/
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_POID, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, vp, ebufp);
	fm_pymt_pol_apply_fee(ctxp, i_flistp, r_flistp, ebufp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		/***************************************************
		 * Log Error Buuffer and return.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_apply_fee error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;

	} 
	else
	{

		*o_flistpp = r_flistp;
		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_pymt_pol_apply_fee return flist", r_flistp);
	}

	return;
}

/********************************************************************
 * fm_pymt_pol_apply_fee()
 ********************************************************************/
static void 
fm_pymt_pol_apply_fee(
	pcm_context_t           *ctxp,
	pin_flist_t 		*i_flistp,
	pin_flist_t 		*r_flistp,
	pin_errbuf_t            *ebufp)
{
	int                     apply_fee = PIN_APPLY_FEE_FALSE;
        pin_flist_t             *charge_flistp = NULL;
        pin_flist_t             *billinfo_flistp = NULL;
        pin_flist_t             *results_flistp = NULL;
        pin_flist_t             *e_flistp = NULL;
        pin_flist_t             *ex_flistp = NULL;
        pin_flist_t             *p_flistp = NULL;
	pin_pay_type_t		*pay_typep = NULL;
	void                    *vp = NULL;
        char                    *event_typep = NULL;
        int32                   elem_id = 0;
        pin_cookie_t            cookie = NULL;
	char 			*cust_segmentp = NULL;
	int			cust_segment_id = 0;

	if (PIN_ERR_IS_ERR(ebufp))
	{
		return ;
	}
        PIN_ERR_CLEAR_ERR(ebufp);

	/*****************************************************
         * Get the charges array from the input flist 
	 *****************************************************/
        while ((charge_flistp = PIN_FLIST_ELEM_GET_NEXT(i_flistp,
                                        PIN_FLD_CHARGES,
                                        &elem_id, 1, &cookie, ebufp)) != NULL)
        {
		pay_typep = (pin_pay_type_t *) PIN_FLIST_FLD_GET(charge_flistp,
					PIN_FLD_PAY_TYPE, 0, ebufp);	
                /***********************************************************
                 * This is the function that is called to decide if
                 * the fees has to be applied for the failed payment.
                 ***********************************************************/
		fm_pymt_pol_apply_fee_set_flag(ctxp, charge_flistp,
							&apply_fee, ebufp);

                results_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_RESULTS,
                                                        elem_id, ebufp);
                /***********************************************************
                 * Get account Poid from the Charges array
                 ***********************************************************/
                vp = PIN_FLIST_FLD_GET(charge_flistp,
                                        PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
                PIN_FLIST_FLD_SET(results_flistp,
                                        PIN_FLD_ACCOUNT_OBJ, vp, ebufp);

                /**********************************************************
                 * Get the BILLINFO OBJ
                 **********************************************************/
		vp = PIN_FLIST_FLD_GET(charge_flistp,
				PIN_FLD_BILLINFO_OBJ, 0, ebufp);		
		PIN_FLIST_FLD_SET(results_flistp,
					PIN_FLD_BILLINFO_OBJ, vp, ebufp);

                /***********************************************************
                 * Check if Extended Info is passed in the input flist
                 * if Extendedinfo is not passed then Create one.
		 * Though it a mandatory field but if we get a SUBSTRUCT 
		 * with no fields attached the return PIN_FLIST_SUBSTR_GET 
		 * will not set an error and return a NULL FList
                 ***********************************************************/
                ex_flistp = PIN_FLIST_SUBSTR_GET(charge_flistp,
                                        PIN_FLD_EXTENDED_INFO, 1, ebufp);
		if(ex_flistp)
		{
			e_flistp = PIN_FLIST_COPY(ex_flistp,ebufp);
		}
		else{
			e_flistp = PIN_FLIST_CREATE(ebufp);
		}
		
                /***********************************************************
                 * Check if PIN_FLD_FAILED_PAYMENT_FEE SUBSTRUCT is passed 
                 * in EXTENED INFO
		 * If it not passed then Create one.
		 * Though it a mandatory field but if we get a SUBSTRUCT 
		 * with no fields attached then  PIN_FLIST_SUBSTR_GET 
		 * will not set an error and return a NULL FList
                 ***********************************************************/
		p_flistp = PIN_FLIST_SUBSTR_GET(e_flistp,
					PIN_FLD_FAILED_PAYMENT_FEE, 1, ebufp);
		if(!p_flistp){
			p_flistp = PIN_FLIST_SUBSTR_ADD(e_flistp, 
					PIN_FLD_FAILED_PAYMENT_FEE, ebufp);
		}
                /**********************************************************
                 * Get the PAY TYPE
		 * Check if it is already present in the Failed payment fee
		 * substruct.
                 **********************************************************/
		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_PAY_TYPE, 1, ebufp);
		/**********************************************************
		 * Dont get the Pay type from charges flist
		 * for PIN_PAY_TYPE_FAILED
		 **********************************************************/
		if((!vp) && (pay_typep) && (*pay_typep != PIN_PAY_TYPE_FAILED)){
			vp = PIN_FLIST_FLD_GET(charge_flistp, PIN_FLD_PAY_TYPE, 
							0, ebufp);
			PIN_FLIST_FLD_SET(p_flistp, PIN_FLD_PAY_TYPE, 
							vp, ebufp);
		}

                /**********************************************************
                 * Get the CHANNEL ID
		 * Check if it is already present in the failed payment fee
		 * substruct.
                 **********************************************************/
		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_CHANNEL_ID,
					1, ebufp);
		/**********************************************************
		 * Dont get the channel ID  from charges array if 
		 * Paytype = PIN_PAY_TYPE_FAILED
		 **********************************************************/
		if((!vp) && (pay_typep) && (*pay_typep != PIN_PAY_TYPE_FAILED)){
			vp = PIN_FLIST_FLD_GET(charge_flistp,
					PIN_FLD_CHANNEL_ID, 1, ebufp);
			if (vp)
			{
				PIN_FLIST_FLD_SET(p_flistp,
					PIN_FLD_CHANNEL_ID, vp, ebufp);
			}
		}

                /**********************************************************
                 * Get the Transaction ID
		 * Check if it is already present in the failed payment fee
		 * substruct.
                 **********************************************************/
		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_TRANS_ID,
					1, ebufp);
		/**********************************************************
		 * Dont get the Transactio ID from charges flist
		 * if pay type = PIN_PAY_TYPE_FAILED
		 **********************************************************/
		if((!vp) && (pay_typep) && (*pay_typep != PIN_PAY_TYPE_FAILED)){
			vp = PIN_FLIST_FLD_GET(charge_flistp,
					PIN_FLD_TRANS_ID, 1, ebufp);
			if (vp)
			{
				PIN_FLIST_FLD_SET(p_flistp,
					PIN_FLD_TRANS_ID, vp, ebufp);
			}
		}

                /**********************************************************
		 * Get the Customer Segment
		 * Check if it is already present in the failed payment fee
		 * substruct. If its not there check the charges array for
		 * for PIN_FLD_CUSTOMER_SEGMENT_LIST.
		 **********************************************************/
		vp = PIN_FLIST_FLD_GET(p_flistp, PIN_FLD_CUSTOMER_SEGMENT,
					1, ebufp);
		if(!vp){
			cust_segmentp = (char *)PIN_FLIST_FLD_GET(charge_flistp,
						PIN_FLD_CUSTOMER_SEGMENT_LIST, 
						1, ebufp);
			if(cust_segmentp)
			{
				fm_pymt_pol_apply_fee_parse_customer_segment(
							cust_segmentp,
							&cust_segment_id,
							ebufp);
				PIN_FLIST_FLD_SET(p_flistp,
						PIN_FLD_CUSTOMER_SEGMENT,
						(void *) &cust_segment_id,
						ebufp);
			}
                }
		/**********************************************************
		 * Get the  orignal payment Amount
		 * Check if it is already present in the failed payment fee
		 * substruct.
		 **********************************************************/
		vp = PIN_FLIST_FLD_GET(p_flistp,
					PIN_FLD_AMOUNT_ORIGINAL_PAYMENT,
					1, ebufp);
		if(!vp){
			vp = PIN_FLIST_FLD_GET(charge_flistp,
				PIN_FLD_AMOUNT_ORIGINAL_PAYMENT, 1, ebufp);
			if (vp){
				PIN_FLIST_FLD_SET(p_flistp,
					PIN_FLD_AMOUNT_ORIGINAL_PAYMENT, vp,
					ebufp);
			}	
		}
		/***********************************************************
		 * Attch the Extended Info substruct to the Results Array
		 ***********************************************************/
		PIN_FLIST_SUBSTR_SET(results_flistp, (void *) e_flistp, 
						PIN_FLD_EXTENDED_INFO, ebufp);
                /***********************************************************
                 * Check if the fees has to be applied or not.
                 * And if fees has to be applied then what kind of fees
                 * event has to be created.
                 ***********************************************************/
                if (apply_fee == PIN_APPLY_FEE_TRUE)
                {
                        event_typep =  (char *)
                        malloc(sizeof(PIN_OBJ_EVENT_BILLING_FEE_FAILED_PAYMENT));

			if (!event_typep) {
				pin_set_err(ebufp, PIN_ERRLOC_FM,
					PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_NO_MEM , 0, 0, 0);	

				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_pymt_pol_apply_fee opcode error",
								ebufp);

				PIN_FLIST_DESTROY_EX(&e_flistp, NULL);
				return;
			}


                        strcpy(event_typep,
                                PIN_OBJ_EVENT_BILLING_FEE_FAILED_PAYMENT);
                        vp = (void *) event_typep;
                        PIN_FLIST_FLD_SET(results_flistp,
                                        PIN_FLD_EVENT_TYPE, vp, ebufp);
                        /******************************************************
			 * Set PIN_FLD_BOOLEAN in the the output flist to "0"
                         ******************************************************/
                        vp = (void *) &apply_fee;
                        PIN_FLIST_FLD_SET(results_flistp,
                                          PIN_FLD_BOOLEAN, vp, ebufp);
			free(event_typep);
                }
                else
                {
                        vp = (void *) &apply_fee;
                        PIN_FLIST_FLD_SET(results_flistp, PIN_FLD_BOOLEAN,
                                          vp, ebufp);
                }
	PIN_FLIST_DESTROY_EX(&e_flistp, NULL);
	}
	if (PIN_ERR_IS_ERR(ebufp)) {
	/***************************************************
	 * Log Error Buffer and return.
	 ***************************************************/
	PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_pymt_pol_apply_fee error", ebufp);
	}
	return ;
}
	
/********************************************************************
 * fm_pymt_pol_apply_fee_set_flag()
 * Read the charges array for Customer segment, payment channel etc. 
 * And decide if the fees has to be applied for these records.
 ********************************************************************/
static void
fm_pymt_pol_apply_fee_set_flag(
			pcm_context_t           *ctxp,
			pin_flist_t		*i_flistp,
			int			*apply_fee,
			pin_errbuf_t            *ebufp)
{
	pin_flist_t		*r_flsitp = NULL;
	int32                   elem_id = 0;
	pin_cookie_t            cookie = NULL;
	void			*vp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
	{
		return ;
	}
        PIN_ERR_CLEAR_ERR(ebufp);

	*apply_fee = PIN_APPLY_FEE_FALSE;
	/**********************************************************
	 * Set apply_fee to PIN_APPLY_FEE_TRUE, if a fees event 
	 * has to be created. 
	 * In the following Implimentation apply_fee is set to 
	 * PIN_APPLY_FEE_TRUE is payment event is created for 
	 * failed payment.
	 * The creation of event is identified by presnce of 
	 * POID in the PIN_FLD_RESULTS array.
	 * What this means is if payment event is not created then
	 * there is no need to create payment failure event. And
	 * fees need not be applied.
	 **********************************************************/
	r_flsitp = PIN_FLIST_ELEM_GET_NEXT(i_flistp, PIN_FLD_RESULTS,
						&elem_id, 1, &cookie, ebufp);
	if(r_flsitp){
		vp = PIN_FLIST_FLD_GET(r_flsitp, PIN_FLD_POID, 1, ebufp);
		if (vp){
			*apply_fee = PIN_APPLY_FEE_TRUE;
		}
	}
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_apply_fee_set_flag error",
			ebufp);
	}
        return ;
		
}

/**************************************************************************
 * fm_pymt_pol_apply_fee_parse_customer_segment()
 * Parse the customer segment string and get the first customer segment ID
 * This string is pipe seperated
 **************************************************************************/
static void
fm_pymt_pol_apply_fee_parse_customer_segment(
	char 			*cust_segmentp,
	int			*cust_segment_idp,
	pin_errbuf_t		*ebufp)
{
        int             len = 0;
        char            *p = NULL;
        char            *temp = NULL;
	
	if (PIN_ERR_IS_ERR(ebufp))
	{
		return ;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

        len = strlen(cust_segmentp);
        p = cust_segmentp;
	/******************************************************************
         * Remove the leading Zeroes
         ******************************************************************/
	if (p && len > 0) {
		while (p[0] == ' ') {
			p = p + sizeof(char);
                }
                temp = p;
	}
	
	/**********************************************************
	 * Get the first customer segment ID from the 
	 * customer segment string
	 **********************************************************/
	len = 0;
	while (temp && (temp[0] != '|')){
		len++;
		temp = temp + sizeof(char);
	}
	
	temp = NULL;
	temp = (char *) malloc(sizeof(char) * (len+1));
	memset(temp, 0, (len + 1));

	if (!temp) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
		PIN_ERRCLASS_SYSTEM_DETERMINATE,
		PIN_ERR_NO_MEM , 0, 0, 0);

		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_apply_fee_parse_customer_segment opcode error",
									ebufp);	
		return;
	}	

	strncpy(temp, p, len);
	/**********************************************
	 * Null terminate the string
	 **********************************************/
	*(temp + len) = '\0';
	*cust_segment_idp = atoi(temp);

	free(temp);

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_pymt_pol_apply_fee_parse_customer_segment", ebufp);
	}
	return ;
}
