/*
 *
* Copyright (c) 1999, 2018, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation or its
 *      licensors and may be used, reproduced, stored or transmitted only in
 *      accordance with a valid Oracle license or sublicense agreement.
 *
 */

#ifndef lint
static  char    Sccs_id[] = "@(#)$Id: fm_pymt_pol_spec_validate.c /cgbubrm_mainbrm.portalbase/2 2018/12/21 00:56:20 agangrad Exp $";
#endif

/*******************************************************************
 * Contains the PCM_OP_PYMT_POL_SPEC_VALIDATE operation. 
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/pymt.h"
#include "pin_pymt.h"
#include "pin_cc_patterns.h"
#include "pin_cust.h"
#include "pin_currency.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"



/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_pymt_pol_spec_validate(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_spec_valid_prep_cust(
	pcm_context_t	*ctxp,
        int32		flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	pin_errbuf_t	*ebufp);

static void
fm_pymt_pol_spec_valid_done_cc(
	pcm_context_t		*ctxp,
        int32			flags,
	pin_flist_t		*pay_flistp,
	poid_t			*a_pdp,
	int32			*resultp,
	pin_errbuf_t		*ebufp);

static void
fm_pymt_pol_spec_valid_done_dd(
	pcm_context_t		*ctxp,
        int32			flags,
	pin_flist_t		*pay_flistp,
	poid_t			*a_pdp,
	int32			*resultp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/

/*******************************************************************
 * Main routine for the PCM_OP_PYMT_POL_SPEC_VALIDATE operation.
 *******************************************************************/
void
op_pymt_pol_spec_validate(
        cm_nap_connection_t	*connp,
	int32			opcode,
        int32			flags,
        pin_flist_t		*i_flistp,
        pin_flist_t		**o_flistpp,
        pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*trans_flistp = NULL;

	char			*action = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_PYMT_POL_SPEC_VALIDATE) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_spec_validate opcode error", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_pymt_pol_spec_validate input flist", i_flistp);

	/***********************************************************
	* Drop PIN_FLD_TRANSACTIONS array from CHARGES.
	* To use/enable Stored Credential (CIT/MIT) feature
	* comment the next 3 lines of code
	*
	* If using Stored credential feature, make sure to
	* add transaction type(PIN_FLD_TYPE) corresponding to
	* the message type set on PIN_FLD_TRANSACTIONS &
	* In case using Stored Credential (CIT/MIT) feature
	* if message type is CIT(CSTO), Transaction Id(PIN_FLD_TRANS_ID)
	* should not be present on PIN_FLD_TRANSACTIONS
	************************************************************/
	trans_flistp = (pin_flist_t *)PIN_FLIST_ELEM_TAKE(i_flistp,
		PIN_FLD_TRANSACTIONS, PIN_ELEMID_ANY, 1, ebufp);
	PIN_FLIST_DESTROY_EX(&trans_flistp, NULL);

	/***********************************************************
	 * Prep the return flist.
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Get the requested action. The following are supported:
	 *	- "prep customer"
	 *	- "commit customer"
	 *
	 ***********************************************************/
	action = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACTION, 0, ebufp);

	if (action != (char *)NULL) {
		if (!strcmp(action, "prep customer") ||
		   !strcmp(action, "commit customer")) {
			fm_pymt_pol_spec_valid_prep_cust(ctxp, flags,
				i_flistp, r_flistp, ebufp);
		} else {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE,
				PIN_ERR_BAD_VALUE, PIN_FLD_ACTION, 0, 0);
		}
	}

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {

		/***************************************************
		 * Log something and return nothing.
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_pymt_pol_spec_validate error", ebufp);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		*o_flistpp = NULL;

	} else {

		/***************************************************
		 * Point the real return flist to the right thing.
		 ***************************************************/
		PIN_ERR_CLEAR_ERR(ebufp);
		*o_flistpp = r_flistp;

		/***************************************************
		 * Debug: What we're sending back.
		 ***************************************************/
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_pymt_pol_spec_validate return flist", r_flistp);

	}

	return;
}


/*******************************************************************
 * fm_pymt_pol_spec_valid_prep_cust():
 *
 *	Fill the the spec flist for the "prep customer" action.
 *
 *******************************************************************/
static void
fm_pymt_pol_spec_valid_prep_cust(
	pcm_context_t	*ctxp,
	int32		flags,
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	pin_errbuf_t	*ebufp)
{
	poid_t		*a_pdp = NULL;
	poid_t		*p_pdp = NULL;
	pin_bill_type_t	*p_type = NULL;
	int32		result = 0;
	char		*merchant = NULL;
	int32		err = 0;
	int32		currency = 0;
	pin_flist_t	*bill_flistp = NULL;
	pin_flist_t	*pay_flistp = NULL;
	pin_flist_t	*read_flistp = NULL;
	pin_flist_t	*ach_flistp = NULL;
	pin_flist_t	*cc_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	pin_cookie_t	cookie = NULL;
	int32		elemid = 0;
	int32		cc_validate_flag = 0;
	int32		dd_validate_flag = 0;
	void		*valp = NULL;
	int64		db = 0;
	void		*vp = NULL;
	int32		rec_id = 0;
	int32		srch_flags = 0;
	char			*cp = NULL;
  	pin_token_card_type_t 	*card_type = NULL;
	pin_flist_t     *trans_flistp = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * What poid do we use? 
	 *	for brand-enabled systems use current brand
	 *	otherwise use the poid specified in pin.conf
	 ***********************************************************/
	if (cm_fm_is_scoping_enabled()) {
		a_pdp = cm_fm_perm_get_brand_account(ebufp);
	 	db = PIN_POID_GET_DB( a_pdp );

		/* if brand is root and in multidb environment
		 * we have to match db in input flist's poid and root brand's poid
		 */
		if(PIN_POID_GET_ID( a_pdp) == 1){
			p_pdp = PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_POID, 0, ebufp );
	 		db = PIN_POID_GET_DB( p_pdp );

			a_pdp = PIN_POID_CREATE( db, PIN_POID_GET_TYPE( a_pdp ), 
					PIN_POID_GET_ID( a_pdp ), ebufp );
		}else{
			a_pdp = PIN_POID_COPY(a_pdp, ebufp);		
		}

	} else {
		/* what db should I look in for an account? */
		p_pdp = PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_POID, 0, ebufp );

	 	db = PIN_POID_GET_DB( p_pdp );

		/* ok now lets get validate_acct */
		p_pdp = NULL;
		pin_conf(FM_PYMT_POL, "validate_acct", PIN_FLDT_POID,
			(caddr_t *)&p_pdp, &err); 
			
	        switch (err) {
	        case PIN_ERR_NONE:
	                break;
	        default:
	                pin_set_err(ebufp, PIN_ERRLOC_FM,
	                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
	                        PIN_ERR_INVALID_CONF, 0, 0, err);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"validate_acct configuration error",ebufp);
			break;
	        }
	        
		/* ok now make a good validate_acct from the db and the   */
		/* pin_conf entry.  If we don't have a poid from pin.conf */
		/* an error was set and we will exit out below.           */
		if( !PIN_POID_IS_NULL( p_pdp ) ) {
			a_pdp = PIN_POID_CREATE( db, PIN_POID_GET_TYPE( p_pdp ), 
					PIN_POID_GET_ID( p_pdp ), ebufp );
		}

		/* we don't need the pin_conf value anymore */
		PIN_POID_DESTROY(p_pdp, ebufp);  	
	}

	if ((!PIN_ERR_IS_ERR(ebufp)) && (PIN_POID_GET_ID(a_pdp) <= (int64)0)) {
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        PIN_ERR_INVALID_CONF, PIN_FLD_POID, 0, 0);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_POID_DESTROY(a_pdp, NULL);
		/***************************************************
		 * Log the error and return
		 ***************************************************/
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_spec_valid_prep_cust error", ebufp);
		return;
	}

	PIN_FLIST_FLD_PUT(o_flistp, PIN_FLD_POID, (void *)a_pdp, ebufp);
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(o_flistp, PIN_FLD_POID, 0, ebufp);


	/***********************************************************
	 * Set default currency to dollars 
	 ***********************************************************/
	currency = PIN_CURRENCY_DOLLARS;
	PIN_FLIST_FLD_SET( o_flistp, PIN_FLD_CURRENCY,
		(void *)&currency, ebufp);

	cookie = NULL;
	bill_flistp = PIN_FLIST_ELEM_GET_NEXT( i_flistp, PIN_FLD_BILLINFO,
			&elemid, 0, &cookie, ebufp);

	cookie = NULL;
	p_pdp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_PAYINFO_OBJ, 1, ebufp);

	pay_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_PAYINFO, 0, 0, ebufp);
	
	p_type = (pin_bill_type_t *)PIN_FLIST_FLD_GET(bill_flistp,
		PIN_FLD_PAY_TYPE, 0, ebufp);

	/***********************************************************
	 * If currency information is available, update in o_flistp
	 ***********************************************************/
	if ((vp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_CURRENCY, 1, ebufp))) {
		PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_CURRENCY, vp, ebufp);
	}

	/***********************************************************
	 * update in o_flistp with pay_type information.
	 ***********************************************************/
	if ((vp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_PAY_TYPE, 1, ebufp))) {
		PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_PAY_TYPE, vp, ebufp);
	}

	if (vp && *(int32 *)vp == PIN_PAY_TYPE_INVOICE) {
		/***********************************************************
		 * If invoice do not read the config/ach object.
		 * Set the merchant to a null value
		 **********************************************************/
		PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_MERCHANT, 
							(void *)"", ebufp);
	}
	else {
		/***********************************************************
		 * Get the merchant based on the /config/ach value
		***********************************************************/
		read_flistp = PIN_FLIST_CREATE(ebufp);
		p_pdp = PIN_POID_CREATE(db, "/search", -1, ebufp);
		PIN_FLIST_FLD_PUT(read_flistp, PIN_FLD_POID, p_pdp, ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_TEMPLATE,
			(void *)" select X from /config/ach where F1 = V1 ", ebufp);
		PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_FLAGS, &srch_flags, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(read_flistp, PIN_FLD_ARGS, 1, ebufp);
		p_pdp = PIN_POID_CREATE(db, "/config/ach", -1, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID, p_pdp, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(read_flistp, PIN_FLD_RESULTS, 
							PIN_ELEMID_ANY, ebufp);
	
		vp = PIN_FLIST_FLD_GET(pay_flistp, PIN_FLD_ACH, 0, ebufp);
		if (vp) rec_id = *(int32 *)vp;
		PIN_FLIST_ELEM_SET(flistp, NULL, PIN_FLD_ACH_INFO, rec_id, ebufp);
	
		PCM_OP(ctxp, PCM_OP_SEARCH, PCM_OPFLG_READ_UNCOMMITTED,
				read_flistp, &ach_flistp, ebufp);

		flistp = PIN_FLIST_ELEM_GET(ach_flistp, PIN_FLD_RESULTS, 0, 0, ebufp);
		flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_ACH_INFO, rec_id, 0, ebufp);
		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_MERCHANT, 0, ebufp);
		PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_MERCHANT, vp, ebufp);
		PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_ACH, &rec_id, ebufp);

		vp = PIN_FLIST_FLD_TAKE(flistp, PIN_FLD_POID_VAL, 0, ebufp);
		flistp = PIN_FLIST_ELEM_ADD(o_flistp, PIN_FLD_BATCH_INFO, 0, ebufp);
		PIN_FLIST_FLD_PUT(flistp, PIN_FLD_POID_VAL, vp, ebufp);

		PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&ach_flistp, NULL);
	}		

	/***********************************************************
	 * Set cc_validate and dd_validate flags
	 ***********************************************************/
	cc_validate_flag = FM_CC_VALIDATE_FLAG_DEFAULT;
	dd_validate_flag = FM_DD_VALIDATE_FLAG_DEFAULT;

	pin_conf(FM_PYMT_POL, FM_CC_VALIDATE_TOKEN, PIN_FLDT_INT,
			(caddr_t *)&valp, &err);
	switch (err) {
	case PIN_ERR_NONE:
		cc_validate_flag = *((int32 *)valp);
		free(valp);
		break;
 
	case PIN_ERR_NOT_FOUND:
		break;
 
	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, err, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Unable to read cc_validate flag from pin.conf",ebufp);
		break;
	}

	pin_conf(FM_PYMT_POL, FM_DD_VALIDATE_TOKEN, PIN_FLDT_INT,
			(caddr_t *)&valp, &err);
	switch (err) {
	case PIN_ERR_NONE:
		dd_validate_flag = *((int32 *)valp);
		free(valp);
		break;
 
	case PIN_ERR_NOT_FOUND:
		break;
 
	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, err, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Unable to read dd_validate flag from pin.conf",ebufp);
		break;
	}

	/***********************************************************
	 * Result depends on bill type.
	 * For now, we only validate CC's (if not already validated)
	 ***********************************************************/
	if (p_type != (pin_bill_type_t *)NULL) {
		switch (*p_type) {
		case PIN_BILL_TYPE_CC:

			/* validate CC's */
			if( !cc_validate_flag) {			
				result = PIN_BOOLEAN_FALSE;
			} else {
				fm_pymt_pol_spec_valid_done_cc(ctxp, 
					flags, pay_flistp, a_pdp, 
					&result, ebufp);
			}

			/*******************************************************
			 * To use/enable Stored Credential (CIT/MIT) feature,
			 * add PIN_FLD_TRANSACTIONS array to o_flistp here.
			 * 
		 	 * PIN_FLD_TRANSACTIONS  ARRAY [0] allocated 20, used 3
		 	 * 	PIN_FLD_MODE           ENUM [0] <value> 
		 	 * 	PIN_FLD_TRANS_ID        STR [0] <value>
		 	 *	PIN_FLD_FLAGS           INT [0] <value> 
			 *	PIN_FLD_AMOUNT_ORIG     DECIMAL [0] <value>
			 *
			 * Note : set PIN_FLD_AMOUNT_ORIG for JCB and DI cards 
			 *.       in case of MRAU transaction.
			 * Note: TRANS_ID should not be sent for Master Card
		 	 *******************************************************/

			cc_flistp = (pin_flist_t *) PIN_FLIST_SUBSTR_GET(pay_flistp,
                                          PIN_FLD_CC_INFO, 0, ebufp);

                         if(cc_flistp) {
                                cp = (char *)PIN_FLIST_FLD_GET(cc_flistp, PIN_FLD_DEBIT_NUM, 1, ebufp);
                                card_type = (pin_token_card_type_t *)PIN_FLIST_FLD_GET(cc_flistp, PIN_FLD_CARD_TYPE, 1, ebufp);
                         }
                         vp = PIN_FLIST_FLD_GET(bill_flistp, PIN_FLD_CURRENCY, 1, ebufp);
                         if(vp){
                                currency = *(int32*)(vp);
                         }
                         if(IS_CITMIT_JCB_CARD(cp, *card_type, ebufp) && currency != PIN_CURRENCY_DOLLARS) {
                                trans_flistp = (pin_flist_t *)PIN_FLIST_ELEM_TAKE(i_flistp,
                                        PIN_FLD_TRANSACTIONS, PIN_ELEMID_ANY, 1, ebufp);
                                PIN_FLIST_DESTROY_EX(&trans_flistp, NULL);
                         }
			break;
		case PIN_BILL_TYPE_DD:

			/* validate DD's */
			if( !dd_validate_flag) {			
				result = PIN_BOOLEAN_FALSE;
			} else {
				fm_pymt_pol_spec_valid_done_dd(ctxp, 
					flags, pay_flistp, a_pdp, 
					&result, ebufp);
			}
			break;

		default:
			/* nothing to validate for now */
			result = PIN_BOOLEAN_FALSE;
			break;
		}

		PIN_FLIST_FLD_SET(o_flistp, PIN_FLD_BOOLEAN,
			(void *)&result, ebufp);
	}

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_pymt_pol_spec_valid_prep_cust error", ebufp);
	}
	
	return;
}

/*******************************************************************
 * fm_pymt_pol_spec_valid_done_cc():
 *
 *	Decide whether to validate the given creditcard number.
 *
 * 	If the card has already been sucessfully validated during
 *	the past hour (arbitrary time value) we assume that it is
 *	ok and don't revalidate (result = PIN_BOOLEAN_FALSE).
 *
 *	If no sucessful validations are found, or we encounter an
 *	error, validation should occur (result = PIN_BOOLEAN_TRUE).
 *
 *******************************************************************/
static void
fm_pymt_pol_spec_valid_done_cc(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*pay_flistp,
	poid_t			*a_pdp,
	int32			*resultp,
	pin_errbuf_t		*ebufp)
{
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t             *cc_flistp = NULL;
	poid_t			*s_pdp = NULL;
	void                    *vp = NULL;

	int64			id = 0;
	time_t			start_t = 0;
	time_t			end_t = 0;
	int32			result = 0;
	int32			count = 0;

	int32			cc_revalidate = 0;
	void			*valp = NULL;
	int32			err = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	cc_flistp = (pin_flist_t *) PIN_FLIST_SUBSTR_GET(pay_flistp, 
		                          PIN_FLD_CC_INFO, 0, ebufp);
	if (cc_flistp == NULL) {
		*resultp = PIN_BOOLEAN_FALSE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING,
		"fm_pymt_pol_spec_valid_done_cc  credit card info not found", 
			ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		return;
	}

	/************************************************************
	 * Assume we will validate.
	 ************************************************************/
	*resultp = PIN_BOOLEAN_TRUE;

	/************************************************************
	 * Create the search flist
	 ************************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);

	id = (int64)208;
	s_pdp = PIN_POID_CREATE(pin_poid_get_db(a_pdp), "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PARAMETERS,
		(void *)"billing/validate/cc", ebufp);


	/***********************************************************
	 * Get cc_revalidation interval
	 ***********************************************************/
	cc_revalidate = FM_CC_REVALIDATE_DEFAULT;
	pin_conf(FM_PYMT_POL, FM_CC_REVALIDATE_TOKEN, PIN_FLDT_INT,
			(caddr_t *)&valp, &err);
	switch (err) {
	case PIN_ERR_NONE:
		cc_revalidate = *((int32 *)valp);
		free(valp);
		break;
 
	case PIN_ERR_NOT_FOUND:
		break;
 
	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, err, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Unable to read cc_revalidation interval from pin.conf",
			ebufp);
		break;
	}

	/************************************************************
	 * Args
	 ************************************************************/
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_CC_INFO, ebufp);
	vp = PIN_FLIST_FLD_GET(cc_flistp, PIN_FLD_DEBIT_NUM, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DEBIT_NUM, (void *)vp, ebufp);

	end_t = pin_virtual_time(NULL);
	start_t = end_t - cc_revalidate;
	end_t+=1;
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, (void *)&start_t, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, (void *)&end_t, ebufp);

        /************************************************************
         * Results
         ************************************************************/
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_VALIDATION, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, (void *)NULL, ebufp);

        /************************************************************
         * Perform the search
         ************************************************************/
        PCM_OP(ctxp, PCM_OP_GLOBAL_SEARCH, flags, s_flistp, &r_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		goto cleanup;
	}

        /************************************************************
	 * Did we find a successful validation?
         ************************************************************/
	flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS,
		&count, 1, &cookie, ebufp);
	while (flistp != (pin_flist_t *)NULL) {

		/************************************************************
		 * Check this result
		 ************************************************************/
		flistp = PIN_FLIST_SUBSTR_GET(flistp,
			PIN_FLD_VALIDATION, 0, ebufp);

		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_RESULT, 0, ebufp);

		if (vp != (void *)NULL) {

			result = *((int32 *)vp);

			switch (result) {
			case PIN_CHARGE_RES_PASS:
			case PIN_CHARGE_RES_SRVC_UNAVAIL:
			case PIN_CHARGE_RES_FAIL_ADDR_LOC:
				*resultp = PIN_BOOLEAN_FALSE;
				break;
			default:
				break;
			}
		}

		/************************************************************
		 * Next result
		 ************************************************************/
		flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS,
			&count, 1, &cookie, ebufp);
	}
	
cleanup:
	/************************************************************
	 * Clean up.
	 ************************************************************/
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	/************************************************************
	 * Warn about, but otherwise ignore errors
	 ************************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING,
			"fm_pymt_pol_spec_valid_done_cc error", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
	}

	return;
}


/*******************************************************************
 * fm_pymt_pol_spec_valid_done_dd():
 *
 *	Decide whether to validate the given bank account number.
 *
 * 	If the card has already been sucessfully validated during
 *	the past hour (arbitrary time value) we assume that it is
 *	ok and don't revalidate (result = PIN_BOOLEAN_FALSE).
 *
 *	If no sucessful validations are found, or we encounter an
 *	error, validation should occur (result = PIN_BOOLEAN_TRUE).
 *
 *******************************************************************/
static void
fm_pymt_pol_spec_valid_done_dd(
	pcm_context_t		*ctxp,
	int32			flags,
	pin_flist_t		*pay_flistp,
	poid_t			*a_pdp,
	int32			*resultp,
	pin_errbuf_t		*ebufp)
{
	pin_cookie_t		cookie = NULL;
	pin_flist_t		*s_flistp = NULL;
	pin_flist_t		*r_flistp = NULL;
	pin_flist_t		*flistp = NULL;
	pin_flist_t             *dd_flistp = NULL;
	poid_t			*s_pdp = NULL;
	void                    *vp = NULL;

	int64			id = 0;
	time_t			start_t = 0;
	time_t			end_t = 0;
	int32			result = 0;
	int32			count = 0;

	int32			dd_revalidate = 0;
	void			*valp = NULL;
	int32			err = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	dd_flistp = (pin_flist_t *) PIN_FLIST_SUBSTR_GET(pay_flistp, 
		                          PIN_FLD_DD_INFO, 0, ebufp);
	if (dd_flistp == NULL) {
		*resultp = PIN_BOOLEAN_FALSE;
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING,
		"fm_pymt_pol_spec_valid_done_dd: direct debit info not found", 
			ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
		return;
	}

	/**********************************************************
	 * Assume we will validate.
	 **********************************************************/
	*resultp = PIN_BOOLEAN_TRUE;

	/**********************************************************
	 * Create the search flist
	 **********************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);

	/**********************************************************
	 * Use 4 arg search for events, template id=210
	 **********************************************************/
	id = (int64) 210;

	s_pdp = PIN_POID_CREATE(pin_poid_get_db(a_pdp), "/search", id, ebufp);
	PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_POID, (void *)s_pdp, ebufp);

	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_PARAMETERS,
		(void *)"billing/validate/dd", ebufp);


	/***********************************************************
	 * Get dd_revalidation interval
	 ***********************************************************/
	dd_revalidate = FM_DD_REVALIDATE_DEFAULT;
	pin_conf(FM_PYMT_POL, FM_DD_REVALIDATE_TOKEN, PIN_FLDT_INT,
			(caddr_t *)&valp, &err);
	switch (err) {
	case PIN_ERR_NONE:
		dd_revalidate = *((int32 *)valp);
		free(valp);
		break;
 
	case PIN_ERR_NOT_FOUND:
		break;
 
	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, err, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"Unable to read dd_revalidation interval from pin.conf",
			ebufp);
		break;
	}

	/***************************************************************
	 * Args
	 ***************************************************************/
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 1, ebufp);
	flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_DD_INFO, ebufp);
	vp = PIN_FLIST_FLD_GET(dd_flistp, PIN_FLD_DEBIT_NUM, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_DEBIT_NUM, (void *)vp, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 2, ebufp);
	flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_DD_INFO, ebufp);
	vp = PIN_FLIST_FLD_GET(dd_flistp, PIN_FLD_BANK_NO, 0, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_BANK_NO, (void *)vp, ebufp);

	end_t = pin_virtual_time(NULL);
	start_t = end_t - dd_revalidate;
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 3, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, (void *)&start_t, ebufp);

	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_ARGS, 4, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T, (void *)&end_t, ebufp);

        /***************************************************************
         * Results
         ***************************************************************/
	flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_RESULTS, 0, ebufp);
	flistp = PIN_FLIST_SUBSTR_ADD(flistp, PIN_FLD_VALIDATION, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_RESULT, (void *)NULL, ebufp);

        /***************************************************************
         * Perform the search
         ***************************************************************/
        PCM_OP(ctxp, PCM_OP_GLOBAL_SEARCH, flags, s_flistp, &r_flistp, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		goto cleanup;
	}

        /***************************************************************
	 * Did we find a successful validation?
         ***************************************************************/
	flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS,
		&count, 1, &cookie, ebufp);
	while (flistp != (pin_flist_t *)NULL) {

		/*
		 * Check this result
		 */
		flistp = PIN_FLIST_SUBSTR_GET(flistp,
			PIN_FLD_VALIDATION, 0, ebufp);

		vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_RESULT, 0, ebufp);

		if (vp != (void *)NULL) {

			result = *((int32 *)vp);

			switch (result) {
			case PIN_CHARGE_RES_PASS:
			case PIN_CHARGE_RES_SRVC_UNAVAIL:
			case PIN_CHARGE_RES_FAIL_ADDR_LOC:
				*resultp = PIN_BOOLEAN_FALSE;
				break;
			default:
				break;
			}
		}

		/********************************************************
		 * Next result
		 ********************************************************/
		flistp = PIN_FLIST_ELEM_GET_NEXT(r_flistp, PIN_FLD_RESULTS,
			&count, 1, &cookie, ebufp);
	}
	
cleanup:
	/**************************************************************
	 * Clean up.
	 **************************************************************/
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	/**************************************************************
	 * Warn about, but otherwise ignore errors
	 **************************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_WARNING,
			"fm_pymt_pol_spec_valid_done_dd error", ebufp);
		PIN_ERR_CLEAR_ERR(ebufp);
	}

	return;
}
