/*******************************************************************
 *
* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_valid_payinfo.c /cgbubrm_mainbrm.portalbase/8 2019/03/20 02:33:16 sagaur Exp $";
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#include "pcm.h"
#include "ops/bill.h"
#include "ops/pymt.h"
#include "ops/cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pin_inv.h"
#include "pin_currency.h"
#include "pin_cc.h"
#include "pin_cc_patterns.h"
#include "pinlog.h"
#include "fm_utils.h"
#include "fm_bill_utils.h"
#include "pin_bill.h"
#include "pin_pymt.h"
#include "pin_type.h"

#define IBAN_MAX_LENGTH 34
#define IBAN_MIN_LENGTH 5
#define BIC_LEN_8 8
#define BIC_LEN_11 11

/*******************************************************************
 * external symbol for paymentterm cm_cache pointer
 *******************************************************************/
extern cm_cache_t *fm_cust_pol_paymentterm_ptr;

/*******************************************************************
 * Routines contained herein.
 *******************************************************************/
EXPORT_OP void
op_cust_pol_valid_payinfo(
        cm_nap_connection_t	*connp,
	int			opcode,
        int			flags,
        pin_flist_t		*in_flistp,
        pin_flist_t		**ret_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_valid_payinfo(
	cm_nap_connection_t	*connp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	int  		        flags,
        int                     optional,
        pin_errbuf_t		*ebufp);


static void
fm_cust_pol_valid_payinfo_addrs(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*r_flistp,
	int32		partial,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_cc(
	cm_nap_connection_t	*connp,
	int		flags,
	poid_t		*a_pdp,
	poid_t		*bi_pdp,
	pin_flist_t	*b_flistp,
	pin_flist_t	*r_flistp,
        int              optional,
        int32           customer_centric_flag,
        int32           clrhouse_validate_flag,
        int32           ach,
	char		*merchant,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_dd(
	pcm_context_t	*ctxp,
        int             flags,
        poid_t          *a_pdp,         /* account poid */
        pin_flist_t     *in_flistp,     /* input flist to valid_payinfo */
        pin_flist_t     *b_flistp,      /* PIN_FLD_DD_INFO */
	pin_flist_t	*r_flistp,      /* return flist */
	int32           partial,
        int32           clrhouse_validate_flag,
        int32           ach,
	char		*merchant,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_subord(
	pcm_context_t	*ctxp,
        int32           flags,
	poid_t		*a_pdp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*r_flistp,
	pin_errbuf_t	*ebufp);

static
void fm_cust_pol_valid_payinfo_sepa(
        cm_nap_connection_t     *connp,
        poid_t                  *a_pdp,
        pin_flist_t     *i_flistp,
        pin_errbuf_t    *ebufp);

static int32
fm_cust_pol_iban_mod97_check(
	int             len,
	char            *ibanp);
	
static void
fm_cust_pol_valid_payinfo_ddebit(
	pin_flist_t	*b_flistp,
	pin_flist_t	*r_flistp,
        int              optional,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_checkdigit(
	char		*card,
	int		length,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_sanity(
	char		*card,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_check_exp(
	char		*exp_date,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_check_account(
	char		*numb,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_check_key(
	char		*key,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_check_bank(
	char		*numb,
	int		i_fld,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_checksum(
	char		*numb,
	char		*numbr,
	char		*account,
	char		*key,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_validate_clrhouse(
	pcm_context_t	*ctxp,
	poid_t		*a_pdp,
	poid_t		*bi_pdp,
	pin_flist_t	*b_flistp,
        int32           pay_type,
        int32           ach,
	char		*merchant,
	pin_errbuf_t	*ebufp);

static void
fm_cust_pol_valid_payinfo_map_country_cleanup(pin_flist_t *i_flistp,
        pin_errbuf_t *ebufp);
        
static int
fm_cust_pol_valid_payinfo_map_country(char *oldcountry,
                                      char **canon_country,
                                      pin_flist_t *i_flistp,
                                      pin_flist_t *r_flistp,
                                      pin_errbuf_t *ebufp);
static void
fm_cust_pol_prep_payinfo_common(
	pcm_context_t	*ctxp,
	u_int		flags,
	const char   	*poid_type,
        int32           partial,
	poid_t         	*pdp,
	pin_flist_t	*b_flistp,
	pin_errbuf_t	*ebufp);

static int32
fm_cust_pol_need_revalidate(int32 field,
                            pin_flist_t *flistp,
                            pin_errbuf_t *ebufp);
static void
fm_cust_pol_validate_paymentterm ( pin_flist_t *i_flistp, 
				   pin_flist_t *r_flistp,
				   pin_errbuf_t *ebufp);

static void
fm_cust_pol_validate_invoice_type(pin_flist_t *i_flistp,
                                pin_flist_t *r_flistp,
                                pin_errbuf_t *ebufp);

static
int32 fm_cust_pol_valid_creditor_id(
	pcm_context_t	*ctxp, 
	poid_t			*a_pdp,
	char			*creditor_id, 
	char			*creditor_name,
	pin_errbuf_t	*ebufp);
	
extern void
fm_cust_pol_map_country(
        pin_flist_t     *flistp,
        pin_flist_t     *r_flistp, 
        char            *country,
        pin_errbuf_t    *ebufp);
	
/*******************************************************************
 * Routines needed from elsewhere.
 *******************************************************************/
extern void
fm_cust_pol_validate_fld_value (
	pcm_context_t   *ctxp,
	pin_flist_t     *in_flistp,
	pin_flist_t     *i_flistp,
	pin_flist_t     *r_flistp,
	pin_fld_num_t   pin_fld_field_num,
	int           pin_fld_element_id,
	char            *cfg_name,
	int             type,
	pin_errbuf_t    *ebufp);

extern pin_flist_t *
fm_cust_pol_valid_add_fail(pin_flist_t	*r_flistp,
			   int	field,
			   int	elemid,
			   int	result,
			   char		*descr,
			   void		*val,
			   pin_errbuf_t	*ebufp);

extern void
fm_cust_pol_map_country(
        pin_flist_t     *flistp,
        pin_flist_t     *r_flistp,
        char            *country,
        pin_errbuf_t    *ebufp);

extern time_t fm_utils_time_round_to_midnight();

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_VALID_PAYINFO  command
 *******************************************************************/
void
op_cust_pol_valid_payinfo(
        cm_nap_connection_t	*connp,
	int		opcode,
        int		flags,
        pin_flist_t	*in_flistp,
        pin_flist_t	**ret_flistpp,
        pin_errbuf_t	*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t	    	*r_flistp = NULL;
	int                 	optional = 0;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_VALID_PAYINFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_valid_payinfo", ebufp);
		return;
	}

	/***********************************************************
	 * Debug - What we got.
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_valid_payinfo input flist", in_flistp);

	/***********************************************************
	 * We will not open any transactions with Policy FM
	 * since policies should NEVER modify the database.
	 ***********************************************************/

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	optional = fm_utils_op_is_ancestor(connp->coip, 
		PCM_OP_CUST_MODIFY_PAYINFO);
	fm_cust_pol_valid_payinfo(connp, in_flistp, &r_flistp, 
		flags, optional, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL; 
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_valid_payinfo error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_valid_payinfo return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo()
 *
 *	Validate the given payinfo.
 *
 *		- Start by assuming validation will pass.
 *		- Validate the common info
 *		- Validate the pay_type
 *		- If valid pay_type, validate the info
 *			specific to that pay_type.
 *
 *	The validation of the common info and the pay_type
 *	specific info is handled in task specific subroutines.
 *
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo(
	cm_nap_connection_t	*connp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	int           		flags,
        int                     optional,
    	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	pin_flist_t	*a_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t     *flistp = NULL;
	pin_flist_t	*debtor_flistp = NULL;
	poid_t		*pay_pdp = NULL;
	poid_t		*a_pdp = NULL;
	poid_t		*bi_pdp = NULL;
   	const char  	*pay_type = NULL;
	int32		result = 0;
	int32		field = 0;
	void		*vp = NULL;
	int32		*partialp = 0;
    /* The default dummy value has been changed from 0 to 1 */
	int32		dummy = 1;
	int32		ach = 0;
	int32		clrhouse_validate_flag = PIN_BOOLEAN_FALSE;
	char		*merchant = NULL;
	pin_flist_t	*read_flistp = NULL;
	pin_flist_t	*read_ret_flistp = NULL;
	pin_flist_t	*cc_flistp = NULL;
	int32		*currencyp = NULL;
	char 		*tokenp = NULL;
	pin_token_card_type_t *card_type = NULL;	
	pin_token_card_type_t card_type_unknown = PIN_TOKEN_UNKNOWN;
	int32		customer_centric_flag = 0;
	int		err = 0;
        
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	r_flistp = PIN_FLIST_CREATE(ebufp);
	*o_flistpp = r_flistp;

	/***********************************************************
	 * Get (and add) the poid.
	 ***********************************************************/
	pay_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, 
			PIN_FLD_POID, 0, ebufp);
	a_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, 
			PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, (void *)pay_pdp, ebufp);

	merchant = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MERCHANT, 
								1, ebufp);

	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ACH, 1, ebufp);
	if (vp) ach = *(int32 *)vp;

	/***********************************************************
	 * Initialize the partial flag
	 ***********************************************************/
	partialp = (int32 *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_FLAGS, 
			1, ebufp);
	if (partialp == NULL) {
		partialp = &dummy;
	} else {
		optional = *partialp;
	}	

	/***********************************************************
	 * For now, assume pass.
	 ***********************************************************/
	result = PIN_CUST_VERIFY_PASSED;
	PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);

	/***********************************************************
	 * The rest of the work is done in a sub per pay_type.
	 ***********************************************************/
	s_flistp = PIN_FLIST_CREATE(ebufp);
	a_flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_FIELD, 0, ebufp);

	PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_RESULT, (void *)&result, ebufp);

       /***********************************************************
         * Validate the Payment term here
         **********************************************************/	       
	fm_cust_pol_validate_paymentterm (i_flistp, s_flistp, ebufp);  

	/***********************************************************
         * Validate Invoice Type here
         **********************************************************/
        fm_cust_pol_validate_invoice_type(i_flistp, s_flistp, ebufp);

	pay_type = PIN_POID_GET_TYPE(pay_pdp);  
	if (pay_type && !strcmp(pay_type, PIN_OBJ_TYPE_PAYINFO_CC)) {
		field = PIN_FLD_CC_INFO;
		PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_FIELD_NUM,
                        (void *)&field, ebufp);
		flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_CC_INFO, 
			PIN_ELEMID_ANY, 1 , ebufp);

		if (flistp) {	
                    /* gets customer centric payment flag*/
                    PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, FM_CC_CUSTOMER_CENTRIC);
                    pin_conf(FM_PYMT_POL, FM_CC_CUSTOMER_CENTRIC, PIN_FLDT_INT,
                        (caddr_t *)&vp, &err);
                    if (err == PIN_ERR_NONE) {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "customer_centric flag found.");
                        customer_centric_flag = *((int32 *)vp);
                        free(vp);
                    }
        
                    if (!customer_centric_flag) {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "customer_centric is disabled");
                    } else {
                        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "customer_centric is enabled");
                        card_type = &card_type_unknown;
                    }

                        clrhouse_validate_flag =
                                fm_cust_pol_need_revalidate(field, flistp,
                                                            ebufp);
		    if (customer_centric_flag) {	
                        /* validate cc only, no need to fill missing info */
			bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, 
					PIN_FLD_BILLINFO_OBJ, 1, ebufp);

				fm_cust_pol_valid_payinfo_cc(connp, flags, a_pdp, bi_pdp, 
					flistp, a_flistp, optional, customer_centric_flag,
					clrhouse_validate_flag, ach, merchant, ebufp);
 
		    } else {	
                        /******************************************************
                         * Fill in missing fields that are required from the
                         * database for the validation steps
                         *****************************************************/
                        fm_cust_pol_prep_payinfo_common(ctxp, flags, pay_type,
                                                        *partialp,
                                                        pay_pdp, flistp,
                                                        ebufp);
			fm_cust_pol_valid_payinfo_addrs(ctxp, i_flistp, 
				flistp, a_flistp, *partialp, ebufp);
			if (PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_cust_pol_valid_payinfo error"
					": address validation error", 
					ebufp);
				return;
				/*****/
			}
			bi_pdp = (poid_t *)PIN_FLIST_FLD_GET(i_flistp, 
					PIN_FLD_BILLINFO_OBJ, 1, ebufp);

				fm_cust_pol_valid_payinfo_cc(connp, flags, a_pdp,bi_pdp, 
					flistp, a_flistp, optional, customer_centric_flag,
					clrhouse_validate_flag, ach, merchant, ebufp);
			tokenp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DEBIT_NUM, 0, ebufp);
			card_type = PIN_FLIST_FLD_GET(flistp, PIN_FLD_CARD_TYPE, 1, ebufp);
		    }
		}
	}
	if (pay_type && !strcmp(pay_type, PIN_OBJ_TYPE_PAYINFO_DDEBIT)) {
		field = PIN_FLD_DDEBIT_INFO;
		PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_FIELD_NUM,
			(void *)&field, ebufp);
		flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DDEBIT_INFO, 
			PIN_ELEMID_ANY, 1 , ebufp);
		if (flistp) {		
			/******************************************************
			 * Fill in missing fields that are required from the
			 * database for the validation steps
			 *****************************************************/
			fm_cust_pol_prep_payinfo_common(ctxp, flags, pay_type,
							*partialp,
							pay_pdp, flistp,
							ebufp);
			fm_cust_pol_valid_payinfo_addrs(ctxp, i_flistp, flistp, 
				a_flistp, *partialp, ebufp);
			if (PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_cust_pol_valid_payinfo error"
					": address validation error", 
					ebufp);
				return;
				/*****/
			}

			fm_cust_pol_valid_payinfo_ddebit( flistp, a_flistp,
				 optional, ebufp);
		}
	}
	if (pay_type && !strcmp(pay_type, PIN_OBJ_TYPE_PAYINFO_INVOICE)) {
		field = PIN_FLD_INV_INFO;
		PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_FIELD_NUM,
			(void *)&field, ebufp);
		flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_INV_INFO, 
			PIN_ELEMID_ANY, 1 , ebufp);
		if (flistp) {		
			/******************************************************
			 * Fill in missing fields that are required from the
			 * database for the validation steps
			 *****************************************************/
			fm_cust_pol_prep_payinfo_common(ctxp, flags, pay_type,
							*partialp,
							pay_pdp, flistp,
							ebufp);
			fm_cust_pol_valid_payinfo_addrs(ctxp, i_flistp, flistp, 
				a_flistp, *partialp, ebufp);
			if (PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_cust_pol_valid_payinfo error"
					": address validation error", 
					ebufp);
				return;
				/*****/
			}

		}
	}
	if (pay_type && !strcmp(pay_type, PIN_OBJ_TYPE_PAYINFO_DD)) {
		field = PIN_FLD_DD_INFO;
		PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_FIELD_NUM,
			(void *)&field, ebufp);
		flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DD_INFO, 
			PIN_ELEMID_ANY, 1 , ebufp);
		if (flistp) {
			clrhouse_validate_flag =
				fm_cust_pol_need_revalidate(field, flistp,
							    ebufp);
			/******************************************************
			 * Fill in missing fields that are required from the
			 * database for the validation steps
			 *****************************************************/
			fm_cust_pol_prep_payinfo_common(ctxp, flags, pay_type,
							*partialp,
							pay_pdp, flistp,
							ebufp);
			fm_cust_pol_valid_payinfo_addrs(ctxp, i_flistp, flistp, 
				a_flistp, *partialp, ebufp);
			if (PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_cust_pol_valid_payinfo error"
					": address validation error",
					ebufp);
				return;
				/*****/
			}
			fm_cust_pol_valid_payinfo_dd(ctxp, flags, a_pdp,
						     i_flistp, flistp,
						     a_flistp, optional,
						     clrhouse_validate_flag, ach,
						     merchant, ebufp);
		}
	}
	if (pay_type && !strcmp(pay_type, PIN_OBJ_TYPE_PAYINFO_SUBORD)) {
		field = PIN_FLD_SUBORD_INFO;
		PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_FIELD_NUM,
			(void *)&field, ebufp);
		flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SUBORD_INFO,
			PIN_ELEMID_ANY, 1, ebufp);
		if (flistp) {
			/******************************************************
			 * Fill in missing fields that are required from the
			 * database for the validation steps
			 *****************************************************/
			fm_cust_pol_prep_payinfo_common(ctxp, flags, pay_type,
							*partialp,
							pay_pdp, flistp,
							ebufp);
			fm_cust_pol_valid_payinfo_subord(ctxp, flags, a_pdp,
							 flistp, a_flistp,
							 ebufp);
			if (PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"fm_cust_pol_valid_payinfo error"
					": subord validation error",
					ebufp);
				return;
				/*****/
			}
		}
	}
	if (pay_type && !strcmp(pay_type, PIN_OBJ_TYPE_PAYINFO_SEPA)) {

		//Currency check
		/* SEPA payments is allowed only for EURO accounts
		 * (primary currency has to be EURO) by default. Needs customization here if
		 * accounts with any currency needs to be allowed */
		if (!PIN_POID_IS_TYPE_ONLY(a_pdp)) {

			read_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, a_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_CURRENCY, NULL, ebufp);
			
			PCM_OPREF(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_ret_flistp, ebufp);
			PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
			
			currencyp = (int *)PIN_FLIST_FLD_GET(read_ret_flistp, PIN_FLD_CURRENCY, 0, ebufp);
			if (currencyp && *currencyp != PIN_CURRENCY_EUR) {
				pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
					PIN_ERR_VALIDATION_FAILED, PIN_FLD_CURRENCY, 0, 0, PIN_DOMAIN_ERRORS,
					PIN_ERR_CUST_SEPA_CURRENCY_NOT_EURO,
					1, 0, NULL);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_pol_valid_payinfo error: "
					"For SEPA pay type, currency "
                                        "of the account should be EURO", ebufp);
				PIN_FLIST_DESTROY_EX(&read_ret_flistp, NULL);
				return;
			}
			PIN_FLIST_DESTROY_EX(&read_ret_flistp, NULL);
		}

		field = PIN_FLD_SEPA_INFO;
		PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_FIELD_NUM,
                        (void *)&field, ebufp);
		flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SEPA_INFO,
				PIN_ELEMID_ANY, 1, ebufp);
		vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
                if (vp) {
                        PIN_FLIST_FLD_SET(flistp, PIN_FLD_END_T,
                                (void *)vp, ebufp);
                }
				
		if (flistp) {

		        debtor_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_DEBTOR_INFO,
					PIN_ELEMID_ANY, 1, ebufp);

			if (PIN_POID_IS_TYPE_ONLY(pay_pdp) && (debtor_flistp == NULL)) {
				pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
					PIN_ERR_VALIDATION_FAILED, PIN_FLD_DEBTOR_INFO, 0, 0, 
					PIN_DOMAIN_ERRORS, PIN_ERR_DEBTOR_INFO_MISSING, 1, 0, NULL);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_pol_valid_payinfo error: "
					"For SEPA pay type, debtor information is mandatory", ebufp);
				return;
			}
			/******************************************************
			 * Fill in missing fields that are required from the
			 * database for the validation steps
			 *****************************************************/
			fm_cust_pol_prep_payinfo_common(ctxp, flags, pay_type,
							*partialp, pay_pdp, flistp, ebufp);
			fm_cust_pol_valid_payinfo_sepa(connp, a_pdp, flistp, ebufp);
			if (PIN_ERR_IS_ERR(ebufp)) {
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_pol_valid_payinfo error"
					": sepa validation error", ebufp);
				return;
				/*****/
			}
		}
    }

	/***********************************************************
	 * Set the result.  Destroy the field list if no error.
	 ***********************************************************/
	vp = PIN_FLIST_ELEM_GET(a_flistp, PIN_FLD_FIELD, PIN_ELEMID_ANY, 
		1, ebufp);
	if (vp) {
		result = PIN_CUST_VERIFY_FAILED;
		PIN_FLIST_FLD_SET(a_flistp, PIN_FLD_RESULT, 
			(void *)&result, ebufp);
		PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, 
			(void *)&result, ebufp);
		PIN_FLIST_CONCAT(r_flistp, s_flistp, ebufp);
	} else {
		PIN_FLIST_ELEM_DROP(s_flistp, PIN_FLD_FIELD, 0, ebufp);
		vp = PIN_FLIST_ELEM_GET(s_flistp, PIN_FLD_FIELD, PIN_ELEMID_ANY, 
			1, ebufp);
		if (vp) {
			result = PIN_CUST_VERIFY_FAILED;
			PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_RESULT, 
				(void *)&result, ebufp);
			PIN_FLIST_CONCAT(r_flistp, s_flistp, ebufp);
		} else if (flistp) {
			PIN_FLIST_ELEM_COPY(flistp, PIN_FLD_TRANSACTIONS, PIN_ELEMID_ANY,
				r_flistp, PIN_FLD_TRANSACTIONS, 0, ebufp);
		}
	}
	cc_flistp = PIN_FLIST_ELEM_ADD(r_flistp, PIN_FLD_CC_INFO, 0, ebufp);
        PIN_FLIST_FLD_SET(cc_flistp, PIN_FLD_DEBIT_NUM, (void *)tokenp , ebufp);
	PIN_FLIST_FLD_SET(cc_flistp, PIN_FLD_CARD_TYPE, card_type, ebufp);

	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	
	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_payinfo error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo_addrs():
 *
 *	Validate the address fields in the payinfo objs.
 *
 *	The following fields are checked for existence and/or proper 
 *	format:
 *		- PIN_FLD_CITY
 *		- PIN_FLD_STATE
 *		- PIN_FLD_ZIP
 *		- PIN_FLD_COUNTRY		
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_addrs(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*r_flistp,
	int32		partial,
	pin_errbuf_t	*ebufp)
{
	void   	*vp = NULL;
	char	*cfg_obj_name = NULL;
	char    *canon_country = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"fm_cust_pol_valid_payinfo_addrs input flist", i_flistp);


	/****************************************************
	 * Mandatory fields -   PIN_FLD_CITY.
	 *                      PIN_FLD_STATE.
	 *                      PIN_FLD_ZIP.
	 *                      PIN_FLD_COUNTRY.
	 * If partial flag is set, these fields are not 
	 * considered mandatory.
	 ****************************************************/
	if (!PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_CITY, 1, ebufp) && !partial) {
		fm_cust_pol_valid_add_fail(r_flistp, 
			PIN_FLD_CITY, 
			(int)NULL, PIN_ERR_MISSING_ARG, 
			PIN_CUST_MISSING_ARG_ERR_MSG, 
			(void *)NULL, 
			ebufp);
		goto checkout;
		/*************/
	}
	
	if (!PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE, 1, ebufp) && !partial) {
		fm_cust_pol_valid_add_fail(r_flistp, 
			PIN_FLD_STATE,
			(int)NULL, PIN_ERR_MISSING_ARG, 
			PIN_CUST_MISSING_ARG_ERR_MSG,
			(void *)NULL, 
			ebufp);
		goto checkout;
		/************/
	}

	if(!PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_ZIP, 1, ebufp) && !partial) {
		fm_cust_pol_valid_add_fail(r_flistp, 
			PIN_FLD_ZIP,
			(int)NULL, PIN_ERR_MISSING_ARG, 
			PIN_CUST_MISSING_ARG_ERR_MSG,
			(void *)NULL, 
			ebufp);
		goto checkout;
		/*************/
	}

	vp = PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_COUNTRY, 1, ebufp);
	if( vp == NULL) {
		if( !partial) {
			fm_cust_pol_valid_add_fail(r_flistp, 
				PIN_FLD_COUNTRY,
				(int)NULL, PIN_ERR_MISSING_ARG, 
				PIN_CUST_MISSING_ARG_ERR_MSG,
				(void *)vp, 
				ebufp);
			goto checkout;
			/************/
		}
		else {
			if (PIN_FLIST_FLD_GET( i_flistp,PIN_FLD_STATE,1,ebufp) 
			||  PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_ZIP,1,ebufp) ) {
				fm_cust_pol_valid_add_fail(r_flistp, 
					PIN_FLD_COUNTRY, 
					(int)NULL, PIN_ERR_MISSING_ARG, 
					PIN_CUST_MISSING_ARG_ERR_MSG, 
					(void *)vp, ebufp);
				goto checkout;
				/*************/
			}
		}
	}
	else {
		if (fm_cust_pol_valid_payinfo_map_country(vp,
			&canon_country, i_flistp, r_flistp, ebufp) == 0) {
			goto checkout;
			/************/
		}
	}


	/************************************************************
	 * Get config object name for PIN_FLD_STATE from
	 * /config/locales_validate.
	 ************************************************************/
	if (PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_STATE, 1, ebufp)) {
		cfg_obj_name = fm_utils_get_fld_validate_name(ctxp,
			canon_country, "PIN_FLD_STATE", ebufp);

		if (cfg_obj_name != (char *)NULL) {
			/*****************************************************
			* Try to validate based on /config/fld_validate 
			* (XXX_STATE) obj.
			******************************************************/
			fm_cust_pol_validate_fld_value( ctxp, in_flistp, 
				i_flistp, r_flistp, PIN_FLD_STATE,
				0, cfg_obj_name, 0, ebufp);
		}
	}

	/************************************************************
	 * Get config object name for PIN_FLD_ZIP from
	 * /config/locales_validate.
	 ************************************************************/
	if ( PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_ZIP, 1, ebufp)) {
		cfg_obj_name = fm_utils_get_fld_validate_name(ctxp,
			canon_country,
			"PIN_FLD_ZIP", ebufp);

		if (cfg_obj_name != (char *)NULL) {
			/*****************************************************
			* Try to validate based on /config/fld_validate 
			* (XXX_ZIP) obj.
			*****************************************************/
			fm_cust_pol_validate_fld_value( ctxp, in_flistp, 
				i_flistp, r_flistp, PIN_FLD_ZIP,
				0, cfg_obj_name, 0, ebufp);
		}
	}

checkout:
	fm_cust_pol_valid_payinfo_map_country_cleanup(i_flistp, ebufp);
	/*************************************************
	 * Error?
	 *************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_cust_pol_valid_payinfo_addrs error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo_cc():
 *
 *	Validate the creditcard specific billing info.
 *
 *	The following checks are performed:
 *		- Debit number is reasonable (checksum)
 *		- Debit number is accepted type
 *		- Debit expire has not already passed
 *		- CVV is present if cvv2_required flag is set to 1
 *		- CID is present if cid_required flag is set to 1
 *
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_cc(
	cm_nap_connection_t	*connp,
	int			flags,
	poid_t			*a_pdp,
	poid_t			*bi_pdp,
	pin_flist_t		*b_flistp,
	pin_flist_t		*r_flistp,
	int			optional,
        int32                   customer_centric_flag,
        int32                   clrhouse_validate_flag,
        int32                   ach,
	char			*merchant,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t	*ctxp = connp->dm_ctx;
	char		*cp = NULL;
	char		*sid_p = NULL;
	int32		validate = PIN_BOOLEAN_TRUE;
	int32		error = 0;
	int32		*i_ptr = NULL;
	int32		cvv_reqd = 0;
	int32		cid_reqd = 0;
	int64		account_id = 0;
	int32 		*ctype = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	if (customer_centric_flag) {
		goto check_clrhouse;
	}
	/*
	 * Verify the creditcard number is reasonable
	 */
	ctype = PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_CARD_TYPE,
		1, ebufp);
	if(!ctype || ctype && *ctype == 0) { //if DEBIT_NUM is not tokenized

		cp = (char *)PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_DEBIT_NUM, 
			optional, ebufp);

		if (cp != (char *)NULL) {
			fm_cust_pol_valid_payinfo_sanity(cp, ebufp);
		}

		if (PIN_ERR_IS_ERR(ebufp)) {
			PIN_ERR_CLEAR_ERR(ebufp);

			fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_DEBIT_NUM,
				(int)NULL, PIN_ERR_BAD_VALUE, 
				PIN_CUST_BAD_VALUE_ERR_MSG,
				(void *)cp, ebufp);
			validate = PIN_BOOLEAN_FALSE;

		}
	}

	/*
	 * Check if CVV/CID is present if required
	 */

	sid_p = (char *)PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_SECURITY_ID,
		1, ebufp);


	if ((cp) && pin_cc_pattern_match(cp, CC_VISA_PATTERN, ebufp)) {

		/*
		 * Check if CVV is required for VISA. 
		 */

		pin_conf(FM_PYMT_POL, "cvv2_required", PIN_FLDT_INT,
                        (caddr_t *)&i_ptr, &error);
		if (error == PIN_ERR_NONE) {
			cvv_reqd = *i_ptr;
		}
		free(i_ptr);

		if (cvv_reqd) {

			/* When creating/modifying the payinfo the cvv2 id is 
			** always required to sent in. If there is no cvv2 id 
			** then raise error. 
			*/
			if ((sid_p == (char *)NULL) || (strlen(sid_p) == 0)) {
				fm_cust_pol_valid_add_fail(r_flistp, 
					PIN_FLD_SECURITY_ID, (int)NULL, 
					PIN_ERR_MISSING_ARG, 
					PIN_CUST_MISSING_ARG_ERR_MSG,
					(void *)sid_p, ebufp);
				validate = PIN_BOOLEAN_FALSE;
			}

		}
		if ((sid_p != (char *)NULL) && strlen(sid_p)){

			/*
			 * Length of CVV2 should be 3.
			 */

			if (strlen(sid_p) != CC_CVV2_LENGTH) {
				fm_cust_pol_valid_add_fail(r_flistp, 
					PIN_FLD_SECURITY_ID, (int)NULL, 
					PIN_ERR_BAD_VALUE, 
					PIN_CUST_BAD_VALUE_ERR_MSG,
					(void *)sid_p, ebufp);
				validate = PIN_BOOLEAN_FALSE;
			}
		}

	} else if ((cp) && pin_cc_pattern_match(cp, 
					CC_AMERICAN_EXPRESS_PATTERN, ebufp)) {

		/*
		 * Check if CID is required for AMEX.
		 */

		pin_conf(FM_PYMT_POL, "cid_required", PIN_FLDT_INT,
                        (caddr_t *)&i_ptr, &error);
		if (error == PIN_ERR_NONE) {
			cid_reqd = *i_ptr;
		}
		free(i_ptr);

		if (cid_reqd) {

			/* When creating/modifying the payinfo the cid in not
			** always sent in.If there is no cid then raise error 
			*/
			
			if ((sid_p == (char *)NULL) || (strlen(sid_p) == 0)) {
				fm_cust_pol_valid_add_fail(r_flistp, 
					PIN_FLD_SECURITY_ID, (int)NULL, 
					PIN_ERR_MISSING_ARG, 
					PIN_CUST_MISSING_ARG_ERR_MSG,
					(void *)sid_p, ebufp);
				validate = PIN_BOOLEAN_FALSE;
			}

		}
		if ((sid_p != (char *)NULL) && strlen(sid_p)) {

			/*
			 * Length of CID should be 4.
			 */

			if (strlen(sid_p) != CC_CID_LENGTH) {
				fm_cust_pol_valid_add_fail(r_flistp, 
					PIN_FLD_SECURITY_ID, (int)NULL, 
					PIN_ERR_BAD_VALUE, 
					PIN_CUST_BAD_VALUE_ERR_MSG,
					(void *)sid_p, ebufp);
				validate = PIN_BOOLEAN_FALSE;
			}
		}

	}

	/*
	 * Verify the expiration date is reasonable
	 */
	cp = (char *)PIN_FLIST_FLD_GET(b_flistp,
		PIN_FLD_DEBIT_EXP, optional, ebufp);

	if (cp != (char *)NULL) {
		fm_cust_pol_valid_payinfo_check_exp(cp, ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_CLEAR_ERR(ebufp);

		(void)fm_cust_pol_valid_add_fail(r_flistp,
			PIN_FLD_DEBIT_EXP, (int)NULL,
			PIN_ERR_BAD_VALUE, 
			PIN_CUST_BAD_VALUE_ERR_MSG,
			(void *)cp, ebufp);
		validate = PIN_BOOLEAN_FALSE;

	}

check_clrhouse:
	/* 
	 * If not a valid account id no need to validate
	 * with clearinghouse.  This would be in case this
	 * function is being called to do a validation
	 * on the credit card data with a dummy acccount
   	 * number passed in.
	 */
	account_id = PIN_POID_GET_ID(a_pdp);
	if (account_id > 0) {
	
		if (!((flags & PCM_OPFLG_CUST_REGISTRATION) ||
			(validate == PIN_BOOLEAN_FALSE)) &&
                    clrhouse_validate_flag != PIN_BOOLEAN_FALSE) {

			/*
	 	 	* Time to validate the credit card with the 
		 	* clearinghouse.
	 	 	*/
			fm_cust_pol_validate_clrhouse(ctxp, a_pdp,bi_pdp, 
			  b_flistp,PIN_PAY_TYPE_CC, ach, merchant, ebufp);
			if (PIN_ERR_IS_ERR(ebufp)) {
                                /*
                                 * If any issues that are not from FM,
				 * dont clear the error.
				 * Pass the error to the callers.
				 */
				if(ebufp->location != PIN_ERRLOC_FM){
					return;
				}
				PIN_ERR_CLEAR_ERR(ebufp);

				cp = (char *)PIN_FLIST_FLD_GET(b_flistp, 
					PIN_FLD_DEBIT_NUM, optional, ebufp);

				(void)fm_cust_pol_valid_add_fail(r_flistp, 
					PIN_FLD_DEBIT_NUM, (int)NULL, 
					PIN_ERR_BAD_VALUE, 
					PIN_CUST_BAD_VALUE_ERR_MSG,
					(void *)cp, ebufp);
			}
		}
	}


	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_payinfo_cc error", ebufp);
	}

	return;
}

/*******************************************************************
 *fm_cust_pol_valid_payinfo_subord():
 *
 *	Validate the subordinate payment info.
 *
 *	The following check is performed:
 *		- Child account's currency is same as parent's.
 *
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_subord(
	pcm_context_t	*ctxp,
        int32           flags,
	poid_t		*a_pdp,
	pin_flist_t	*i_flistp,
	pin_flist_t	*r_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*c_flistp = NULL;
	poid_t		*p_pdp = NULL;
        poid_t          *pdp = NULL;
	void		*vp = NULL;
	u_int32		parent_currency = 0; /* no currency */
	u_int32		child_currency = 0; /* no currency */

	p_pdp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_AR_ACCOUNT_OBJ, 1, ebufp);
	if (!p_pdp) {
		goto checkout;
	}

	flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)p_pdp, ebufp);
	PIN_FLIST_FLD_SET(flistp, PIN_FLD_CURRENCY, vp, ebufp);
	PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, flistp, &c_flistp, ebufp);
	vp = PIN_FLIST_FLD_GET(c_flistp, PIN_FLD_CURRENCY, 0, ebufp);
	if ( vp ) {
		parent_currency = *(u_int32 *)vp;
	}
	PIN_FLIST_DESTROY_EX(&c_flistp, NULL);

        /******************************************************************
         * If this is during account registration then we can cheat on the
         * currency validation and pass the validation since we cover it in 
	 * set billinfo
         ******************************************************************/
        if ( !(flags & PCM_OPFLG_CUST_REGISTRATION) ) {
                PIN_FLIST_FLD_SET(flistp, PIN_FLD_POID, (void *)a_pdp, ebufp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, flistp, &c_flistp, ebufp);
                vp = PIN_FLIST_FLD_GET(c_flistp, PIN_FLD_CURRENCY, 0, ebufp);
		if ( vp ) {
                	child_currency = *(u_int32 *)vp;
		}
                PIN_FLIST_DESTROY_EX(&c_flistp, NULL);

		/*************************************************************
		 * if there is no child or parrent currency or the currencies
		 * of the child and parent differ
		 *************************************************************/
		if( child_currency == 0  || parent_currency == 0 ||
			child_currency != parent_currency ) {

			(void)fm_cust_pol_valid_add_fail(r_flistp, 
				PIN_FLD_CURRENCY, (int)NULL, PIN_ERR_BAD_VALUE,
				PIN_CUST_BAD_VALUE_ERR_MSG,
				(void *)&child_currency, ebufp);
		}
        }

checkout:

        /*
         * Error?
         */
        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "fm_cust_pol_valid_payinfo_subord error", ebufp);
        }

        PIN_FLIST_DESTROY_EX(&flistp, NULL);
	return;

}

/******************************************************************r
 * fm_cust_pol_valid_payinfo_checkdigit():
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_checkdigit(
	char		*card,
	int		length,
	pin_errbuf_t	*ebufp)
{
	int		i = 0;
	int		weight = 0;	/* Weight to Apply */
	int		sum = 0;	/* Sum of weights */
	int		digit = 0;	/* Digit being checked */
	long		mod = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	weight = 2;
	sum = 0;

	for (i = length - 2; i >= 0; i--) {
		digit = weight * (card[i] - '0');

		sum += (digit / 10) + (digit % 10);

		if (weight == 2)
			weight = 1;
		else
			weight = 2;
	}

	mod = (10 - (sum % 10)) % 10;
	digit = card[length - 1] - '0';

	if (digit != mod) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_DEBIT_NUM, 0, mod);
	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo_sanity():
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_sanity(
	char		*card,
	pin_errbuf_t	*ebufp)
{
	int		card_length = 0;
	int		cc_checksum_flag = 0;
	void            *valp = NULL;
	int		err = 0;
	int             digit = 0;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Some basic rules about card lentgths
	 */
	card_length = (int)strlen(card);

	/* simple sanity to avoid core dump problems with short numbers */
	if ( card_length <= 4 ) {
		ebufp->pin_err = PIN_ERR_BAD_VALUE;
		return;
	}

	switch(card[0]) {
	case '4':
		/* Looks like PIN_CC_TYPE_VISA */
		if ((card_length != 13) && (card_length != 16)) {
			ebufp->pin_err = PIN_ERR_BAD_VALUE;
			return;
		}
		break;
	case '5':
		/* Looks like PIN_CC_TYPE_MCARD */
		if ((card[1] < '1') || (card[1] > '5')) {
			/* Supposed to be '51' - '55' */
			ebufp->pin_err = PIN_ERR_BAD_VALUE;
			return;
		}
		if (card_length != 16) {
			ebufp->pin_err = PIN_ERR_BAD_VALUE;
			return;
		}
		break;
	case '2':
		/*
		 * Looks like PIN_CC_TYPE_MCARD starting with 2 
		 * Supposed to be '2221' - '2720'
		 */
		digit = 100*(card[1] - '0') + 10*(card[2] - '0') + (card[3] - '0');
		if ((digit < 221) || (digit > 720)) {
                        ebufp->pin_err = PIN_ERR_BAD_VALUE;
                        return;
                }
                if (card_length != 16) {
                        ebufp->pin_err = PIN_ERR_BAD_VALUE;
                        return;
                }
                break;
	case '3':
		switch (card[1]) {
		case '4':
		case '7':
			/* Looks like PIN_CC_TYPE_AMEX or PIN_CC_TYPE_OPTIMA */
			if (card_length != 15) {
				ebufp->pin_err = PIN_ERR_BAD_VALUE;
				return;
			}
			break;
		case '5':
			/* Looks like JCB */
			if ((card[2] < '2') || (card[2] > '8')) {
				/* Supposed to be '352' - '358' */
				ebufp->pin_err = PIN_ERR_BAD_VALUE;
				return;
			}
			if (card_length != 16) {
				ebufp->pin_err = PIN_ERR_BAD_VALUE;
				return;
			}
			break;
		case '0':
		case '6':
		case '8':
			/* Looks like Diners Club or Carte Blanche */
			if (card_length != 14) {
				ebufp->pin_err = PIN_ERR_BAD_VALUE;
				return;
			}
			break;
		default:
			ebufp->pin_err = PIN_ERR_BAD_VALUE;
			return;
		}
		break;
	case '6':
		/* Looks like PIN_CC_TYPE_DISCOVER */
		if ((strncmp(card, "6011", 4)) && (card[1] != '2') && 
			(card[1] != '3') && (card[1] != '8')) {
			ebufp->pin_err = PIN_ERR_BAD_VALUE;
			return;
		}
		if(card[1] == '2'){

                	if(card[2] == '3' || card[2] == '7'){
				ebufp->pin_err = PIN_ERR_BAD_VALUE;
				return;
			}
			if(card[2] == '1' && strncmp(card+3, "094", 3)){
				ebufp->pin_err = PIN_ERR_BAD_VALUE;
				return;
			}
			if(card[2] == '8' && !(2 <= card[3] - '0' <= 8 )){
				ebufp->pin_err = PIN_ERR_BAD_VALUE;
				return;
			}
		}

		if (card_length != 16) {
			ebufp->pin_err = PIN_ERR_BAD_VALUE;
			return;
		}
		break;
	case '9':
		/* Looks like Carte Blanche  or Discover */
		if ((card[1] < '4') || (card[1] == '7')) {
			ebufp->pin_err = PIN_ERR_BAD_VALUE;
			return;
		}
		if ( (card_length != 14) && (card_length != 16) ) {
			ebufp->pin_err = PIN_ERR_BAD_VALUE;
			return;
		}
		break;
	case '8':
                /*Looks like china union pay(Discover) card.*/
                if (card[1]!= '1') {
                        ebufp->pin_err = PIN_ERR_BAD_VALUE;
                        return;
                }
                if ( card_length != 16 ) {
                        ebufp->pin_err = PIN_ERR_BAD_VALUE;
                        return;
                }
                break;
	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_DEBIT_NUM, 0, card[0]);
		return;
	}


	/*
	 * Verify the CheckDigit
	 */

        /***********************************************************
         * Set cc_checksum flag
         ***********************************************************/
        cc_checksum_flag = FM_CC_CHECKSUM_FLAG_DEFAULT;
        pin_conf(FM_CUST_POL, FM_CC_CHECKSUM_TOKEN, PIN_FLDT_INT,
                        (caddr_t *)&valp, &err);
        switch (err) {
        case PIN_ERR_NONE:
                cc_checksum_flag = *((int *)valp);
		free( valp);
                break;
 
        case PIN_ERR_NOT_FOUND:
                break;
 
        default:
                pin_set_err(ebufp, PIN_ERRLOC_FM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE, err, 0, 0, 0);
                        PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                        "Unable to read cc_checksum from pin.conf",ebufp);
                break;
        }
	
	if( cc_checksum_flag) {
		fm_cust_pol_valid_payinfo_checkdigit(card, card_length, ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo_check_exp():
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_check_exp(
	char		*exp_date,
	pin_errbuf_t	*ebufp)
{
	struct tm	*tm = NULL;
	time_t		time_now = 0;
	int		exp_length = 0;
	int		month = 0;
	int		year = 0;
	char		temp[3];
	temp[0] = '\0';

	exp_length = (int)strlen(exp_date);

	/*
	 * We only take MMYY
	 */
	if (exp_length != 4) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_DEBIT_EXP, 0, exp_length);
		return;
	}

	/*
	 * Make sure we're talking numbers
	 */
	if (strspn(exp_date, "0123456789") != (unsigned int)exp_length) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_DEBIT_EXP, 0, exp_length);
		return;
	}

	/*
	 * Get today
	 */
	time_now = pin_virtual_time(NULL);
	tm = localtime(&time_now);
	if(!tm) {
  		pin_set_err(ebufp, PIN_ERRLOC_FM,
  			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
				0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"NULL pointer returned from localtime", ebufp);
		return;
 	}

	/*
	 * Parse the incoming date
	 */
	strncpy(temp, exp_date, 2);
	temp[2] = '\0';
	month = atoi(temp);
	strncpy(temp, &(exp_date[2]), 2);
	temp[2] = '\0';
	year = atoi(temp);
	if (year < 70)
		year += 100;

	/*
	 * See if the month is reasonable
	 */
	if ((month < 1) || (month > 12)) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_DEBIT_EXP, 0, month);
		return;
	}

	/*
	 * Now compare
	 */
	if (year < tm->tm_year) {

		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_DEBIT_EXP, 0, year);
		return;

	} else if (year == tm->tm_year) {

		if (month < tm->tm_mon + 1) {
			pin_set_err(ebufp, PIN_ERRLOC_FM,
			    PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			    PIN_FLD_DEBIT_EXP, 0, month);
			return;
		}

	}

	/*
	 * No errors.
	 */
	PIN_ERR_CLEAR_ERR(ebufp);

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo_ddebit():
 *
 *	Validate the direct debit specific billing info.
 *			(valid for france ).
 *
 *	The following checks are performed:
 *		- bank number is reasonable (5 digits)
 *		- branch number is reasonable (5 digits)
 *		- bank account is reasonable (11 characters)
 *		- these fields are reasonable (checksum)
 *
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_ddebit(
	pin_flist_t	*b_flistp,
	pin_flist_t	*r_flistp,
	int             optional,
	pin_errbuf_t	*ebufp)
{
	char		*bank = NULL;
	char		*branch = NULL;
	char		*account = NULL;
	char		*key = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Verify the bank number is reasonable
	 */
	bank = (char *)PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_BANK_NO, 
		optional, ebufp);

	if (bank != (char *)NULL) {
		fm_cust_pol_valid_payinfo_check_bank(bank, PIN_FLD_BANK_NO,
			ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_CLEAR_ERR(ebufp);

		(void)fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_BANK_NO,
			(int)NULL, PIN_ERR_BAD_VALUE, 
			PIN_CUST_BAD_VALUE_ERR_MSG,
			(void *)bank, ebufp);
	}

	/*
	 * Verify the branch number is reasonable
	 */
	branch = (char *)PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_BRANCH_NO, 
		optional, ebufp);

	if (branch != (char *)NULL) {
		fm_cust_pol_valid_payinfo_check_bank(branch, PIN_FLD_BRANCH_NO,
			ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_CLEAR_ERR(ebufp);

		(void)fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_BRANCH_NO,
			(int)NULL, PIN_ERR_BAD_VALUE, 
			PIN_CUST_BAD_VALUE_ERR_MSG,
			(void *)branch, ebufp);
	}

	/*
	 * Verify the bank account number is reasonable
	 */
	account = (char *)PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_BANK_ACCOUNT, 
		optional, ebufp);

	if (account != (char *)NULL) {
		fm_cust_pol_valid_payinfo_check_account(account, ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_CLEAR_ERR(ebufp);

		(void)fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_BANK_ACCOUNT,
			(int)NULL, PIN_ERR_BAD_VALUE, 
			PIN_CUST_BAD_VALUE_ERR_MSG,
			(void *)account, ebufp);
	}

	/*
	 * Verify the checksum number is reasonable
	 */
	key = (char *)PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_KEY_RIB, 
		optional, ebufp);

	if (key != (char *)NULL) {
		fm_cust_pol_valid_payinfo_check_key(key, ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_CLEAR_ERR(ebufp);

		(void)fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_KEY_RIB,
			(int)NULL, PIN_ERR_BAD_VALUE, 
			PIN_CUST_BAD_VALUE_ERR_MSG,
			(void *)key, ebufp);
	}


	/*
	 * compare the checksum number with the calculated one
	 */

	if ((bank != (char *)NULL) && (branch != (char *)NULL) && 
	    (account != (char *)NULL) && (key != (char *)NULL)) {
		/* Theoritically if one of the 4 fields is updated
		 * the 4 fields are present in the flist
		 * but if none is updated, none is present so no
		 * control can be done
		 */
		fm_cust_pol_valid_payinfo_checksum(bank, branch, account,
			key, ebufp);
	}

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_CLEAR_ERR(ebufp);

		(void)fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_KEY_RIB,
			(int)NULL, PIN_ERR_BAD_VALUE, 
			PIN_CUST_BAD_VALUE_ERR_MSG,
			(void *)key, ebufp);
	}

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_payinfo_ddebit error", ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo_dd():
 *
 *	The following checks are performed:
 *		- bank (RDFI) number is reasonable (9 digits)
 *		- bank account is reasonable (11 characters)
 *
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_dd(
	pcm_context_t	*ctxp,
        int             flags,
        poid_t          *a_pdp,         /* account poid */
        pin_flist_t     *in_flistp,     /* input flist to valid_payinfo */
        pin_flist_t     *b_flistp,      /* PIN_FLD_DD_INFO */
	pin_flist_t	*r_flistp,      /* return flist */
	int32           partial,
        int32           clrhouse_validate_flag,
        int32           ach,
	char		*merchant,
	pin_errbuf_t	*ebufp)
{
	char	*cfg_obj_name = NULL;
        char    *canon_country = NULL;
        void   	*vp = NULL;
	int64	account_id = 0;
	poid_t	*bi_pdp = NULL;

	if (PIN_ERR_IS_ERR(ebufp)) {
		return;
	}
	PIN_ERR_CLEAR_ERR(ebufp);

	/****************************************************
	 * Mandatory fields -   PIN_FLD_BANK_NO.
	 *                      PIN_FLD_DEBIT_NUM.
	 ****************************************************/
	vp = PIN_FLIST_FLD_GET( b_flistp, PIN_FLD_BANK_NO, 1, ebufp);
	if (!vp || ((vp && strlen((char *)vp) == 0) && !partial)) {
		fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_BANK_NO,
			(int)NULL, PIN_ERR_MISSING_ARG, 
			PIN_CUST_MISSING_ARG_ERR_MSG,
			(void *)NULL, ebufp);
		goto checkout;
		/************/
	}

	vp = PIN_FLIST_FLD_GET( b_flistp, PIN_FLD_DEBIT_NUM, 1, ebufp);
	if (!vp || ((vp && strlen((char *)vp) == 0) && !partial)) {
		fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_DEBIT_NUM,
			(int)NULL, PIN_ERR_MISSING_ARG, 
			PIN_CUST_MISSING_ARG_ERR_MSG,
			(void *)NULL, ebufp);
		goto checkout;
		/*************/
	}

	/****************************************************
	 * Check the bank_no and the debit no for the specified
	 * country 
	 ****************************************************/
	vp = PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_COUNTRY, 1, ebufp);
	if ( !vp && !partial) {
		fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_COUNTRY,
			(int)NULL, PIN_ERR_MISSING_ARG, 
			PIN_CUST_MISSING_ARG_ERR_MSG,
			(void *)NULL, ebufp);
		goto checkout;
		/*************/
	}
        else {
                if (fm_cust_pol_valid_payinfo_map_country(vp, &canon_country,
                        b_flistp, r_flistp, ebufp) == 0) {
                        goto checkout;
                }
        }

	/************************************************************
	 * Get config object name for PIN_FLD_DEBIT_NUM from
	 * /config/locales_validate.
	 ************************************************************/
	if (vp) {
		cfg_obj_name = fm_utils_get_fld_validate_name(ctxp,
                        canon_country, "PIN_FLD_BANK_NO", ebufp);

		if (cfg_obj_name != (char *)NULL) {
			/******************************************************
			 * Try to validate based on /config/fld_validate 
			 * (BANK_NO) obj.
			 *****************************************************/
			fm_cust_pol_validate_fld_value( ctxp, in_flistp, 
				b_flistp, 
				r_flistp,
				PIN_FLD_BANK_NO,
				0,              /* element_id   */
				"BANK_NO",
				0,
				ebufp);
		}
	}

	/************************************************************
	 * Get config object name for PIN_FLD_DEBIT_NUM from
	 * /config/locales_validate.
	 ************************************************************/
	if (vp) {

		cfg_obj_name = fm_utils_get_fld_validate_name(ctxp,
                        canon_country, "PIN_FLD_DEBIT_NUM", ebufp);

		if (cfg_obj_name != (char *)NULL) {
			/*****************************************************
			 * Try to validate based on /config/fld_validate 
			 * (DEBIT_NUM) obj.
			 *****************************************************/
			fm_cust_pol_validate_fld_value( ctxp, in_flistp, 
				b_flistp, 
				r_flistp,
				PIN_FLD_DEBIT_NUM,
				0,              /* element_id   */
				"DEBIT_NUM",
				0,
				ebufp);
		}
	}

        fm_cust_pol_valid_payinfo_map_country_cleanup(b_flistp, ebufp);

	/* 
	 * If not a valid account id no need to validate
	 * with clearinghouse.  This would be in case this
	 * function is being called to do a validation
	 * on the credit card data with a dummy acccount
   	 * number passed in.
	 */
	account_id = PIN_POID_GET_ID(a_pdp);
	if (account_id > 0) {
	
		if (!(flags & PCM_OPFLG_CUST_REGISTRATION) &&
                    clrhouse_validate_flag != PIN_BOOLEAN_FALSE) {

			bi_pdp = (poid_t *)PIN_FLIST_FLD_GET( in_flistp,
					 PIN_FLD_BILLINFO_OBJ, 1, ebufp);
			/*
	 	 	* Time to validate the credit card with the 
		 	* clearinghouse.
	 	 	*/
			fm_cust_pol_validate_clrhouse(ctxp, a_pdp,bi_pdp, b_flistp,
                                                      PIN_PAY_TYPE_DD, ach, merchant, ebufp);

			if (PIN_ERR_IS_ERR(ebufp)) {
                                /*
                                 * If any issues that are not from FM,
                                 * dont clear the error.
                                 * Pass the error to the callers.
                                 */
                                if(ebufp->location != PIN_ERRLOC_FM){
                                        return;
                                }

                                char *cp;
				PIN_ERR_CLEAR_ERR(ebufp);

				cp = (char *)PIN_FLIST_FLD_GET(b_flistp, 
					PIN_FLD_DEBIT_NUM, partial, ebufp);

				(void)fm_cust_pol_valid_add_fail(r_flistp, 
					PIN_FLD_DEBIT_NUM, (int)NULL, 
					PIN_ERR_BAD_VALUE, 
					PIN_CUST_BAD_VALUE_ERR_MSG,
					(void *)cp, ebufp);
			}
		}
	}

checkout:
	/*************************************************
	 * Error?
	 *************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_valid_payinfo_dd error", ebufp);
	}
	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo_check_bank():
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_check_bank(
	char		*numb,
	int		i_fld,
	pin_errbuf_t	*ebufp)
{
	int numb_length = (int)strlen(numb);

	/*
	 * We only accept 5 digit
	 */
	if (numb_length != 5) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			i_fld, 0, numb_length);
		return;
	}

	/*
	 * Make sure we're talking numbers
	 */
	if (strspn(numb, "0123456789") != (unsigned int)numb_length) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			i_fld, 0, numb_length);
		return;
	}

	/*
	 * No errors.
	 */
	PIN_ERR_CLEAR_ERR(ebufp);

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo_check_account():
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_check_account(
	char		*numb,
	pin_errbuf_t	*ebufp)
{
	int numb_length = (int)strlen(numb);

	/*
	 * We only accept 11 char
	 */
	if (numb_length != 11) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_BANK_ACCOUNT, 0, numb_length);
		return;
	}

	/*
	 * No errors.
	 */
	PIN_ERR_CLEAR_ERR(ebufp);

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo_check_key():
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_check_key(
	char		*key,
	pin_errbuf_t	*ebufp)
{
	int numb_length = (int)strlen(key);

	/*
	 * We only accept 2 digits
	 */
	if (numb_length != 2) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_KEY_RIB, 0, numb_length);
		return;
	}

	/*
	 * Make sure we're talking numbers
	 */
	if (strspn(key, "0123456789") != (unsigned int)numb_length) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_KEY_RIB, 0, numb_length);
		return;
	}

	/*
	 * No errors.
	 */
	PIN_ERR_CLEAR_ERR(ebufp);

	return;
}

/*******************************************************************
 * fm_cust_pol_valid_payinfo_checksum():
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_checksum(
	char		*numb,
	char		*numbr,
	char		*account,
	char		*key,
	pin_errbuf_t	*ebufp)
{
	int		i = 0;
	int		sum = 0;	/* calculated chksum of string */
	int		chksum = 0;	/* given chksum of string */
	int		digit = 0;	/* Digit being checked */
	char	       *totstr = NULL;	/* total string to be checked */
	int		numb_len = 0;
	int 		numbr_len = 0;
	int		account_len = 0;
	char           *totstr_p = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);


	if (numb == NULL || numbr == NULL || account == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_KEY_RIB, 0, sum);
		return;
	}


	numb_len = strlen(numb);
	numbr_len = strlen(numbr);
	account_len = strlen(account);
	totstr = (char *) pin_malloc(numb_len + numbr_len + account_len + 3 /* "00" */);
	if (totstr == NULL) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_KEY_RIB, 0, sum);
		return;
	}
	strncpy(totstr_p = totstr, numb, numb_len);
	totstr_p += numb_len;
	strncpy(totstr_p, numbr, numbr_len);
	totstr_p += numbr_len;
	strncpy(totstr_p, account, account_len);
	totstr_p += account_len;

	/* we append "00" to the bank nember, the branch number and
	 * the account number.
	 */
	strcpy(totstr_p, "00");


	sum = 0;

	for (i = 0; i < (int)strlen(totstr); i++) {
		/* if letters upper case only */
		totstr[i] = toupper(totstr[i]);
		if ((totstr[i] >= '0') && (totstr[i] <= '9')) {
			/* we keep the value */
			digit = totstr[i] - '0';
		} else if ((totstr[i] >= 'A') && (totstr[i] <= 'Z')) {
			/* we affect a number from 1 to 9 
			 * a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8, i=9
			 * j=1, k=2, l=3, m=4, n=5, o=6, p=7, q=8, r=9
			 * s=1, t=2, u=3, v=4, w=5, x=6, y=7, z=8. 
			 */
			digit = ((totstr[i] - 'A') % 9) +1;
		} else digit = 0; /* other char is replaced by 0 */

		sum = (sum * 10 + digit) % 97;

	}

	if (totstr) {
		pin_free(totstr);
	}

	sum = 97 - sum;
	chksum = atoi(key);

	if (chksum != sum) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_BAD_VALUE,
			PIN_FLD_KEY_RIB, 0, sum);
	} else {
		PIN_ERR_CLEAR_ERR(ebufp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_validate_clrhouse():
 *	Validate the with a clearing house
 *      pay_type differentiates between credit card or direct debit
 *******************************************************************/
static void
fm_cust_pol_validate_clrhouse(
	pcm_context_t	*ctxp,
	poid_t		*a_pdp,
	poid_t		*bi_pdp,
	pin_flist_t	*b_flistp,
        int32           pay_type,
        int32           ach,
	char		*merchant,
	pin_errbuf_t	*ebufp)
{
	pcm_context_t	*vctxp = NULL;
	pin_flist_t	*v_flistp = NULL;
	pin_flist_t	*c_flistp = NULL;
	pin_flist_t	*p_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*cc_flistp = NULL;
	char		*debit_num = NULL;
	int		*result = NULL;
	void		*valp = NULL;
        char            *program_namep = NULL;
        int32           *force_validp = NULL;
        pin_fld_num_t   fld = 0;
	int		err = 0;
	int		flag = 0;
	int		command = 0;
	time_t		now = 0;

	char 		*tokenp = NULL;
	pin_token_card_type_t *card_type = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	debit_num = (char *)PIN_FLIST_FLD_GET(b_flistp, 
			PIN_FLD_DEBIT_NUM, 0, ebufp);
	if (debit_num == (char *)NULL) {
		return;
	}

	/***********************************************************
         * Retrieve values and defaults depending on the type of
         * validation we're performing
         */
        switch (pay_type) {
        case PIN_PAY_TYPE_CC:
                pin_conf(FM_PYMT_POL, FM_CC_VALIDATE_TOKEN, PIN_FLDT_INT, 
			(caddr_t *)&valp, &err);
                program_namep = "CC Validation";
                fld = PIN_FLD_CC_INFO;
                break;
        case PIN_PAY_TYPE_DD:
                pin_conf(FM_PYMT_POL, FM_DD_VALIDATE_TOKEN, PIN_FLDT_INT, 
			(caddr_t *)&valp, &err);
                program_namep = "DD Validation";
                fld = PIN_FLD_DD_INFO;
                break;
        }

	/***********************************************************
	 * Check to see whether we need to do validation.
	 ***********************************************************/
	switch (err) {
		case PIN_ERR_NONE:
			if (valp) {
				flag = *((int *)valp);
				free(valp);
			}
			break;

		case PIN_ERR_NOT_FOUND:
			break;

		default:
			pin_set_err(ebufp, PIN_ERRLOC_FM,
				PIN_ERRCLASS_SYSTEM_DETERMINATE, err, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"Unable to read from pin.conf", ebufp);
			break;
	}

	/***********************************************************
	 * Bail, if we don't have to do any validation.
	 ***********************************************************/
	if ((flag == 0) && (pay_type == PIN_PAY_TYPE_CC)) {
		command = PIN_CHARGE_CMD_TOKEN_ONLY;
	}
	else {
		command = PIN_CHARGE_CMD_VERIFY;
	}

	/***********************************************************
	 * Prep the flist to do the actual validation.
	 ***********************************************************/
	v_flistp = PIN_FLIST_CREATE(ebufp);

	PIN_FLIST_FLD_SET(v_flistp, PIN_FLD_POID, 
		(void *)a_pdp, ebufp);
	PIN_FLIST_FLD_SET(v_flistp, PIN_FLD_PROGRAM_NAME, program_namep,
                          ebufp);

	now = pin_virtual_time((time_t *)NULL);
	PIN_FLIST_FLD_SET(v_flistp, PIN_FLD_END_T, 
		(void *)&now, ebufp);
	PIN_FLIST_FLD_SET(v_flistp, PIN_FLD_START_T, 
		(void *)&now, ebufp);

	/***********************************************************
	 * Add the CHARGES array.
	 ***********************************************************/
	c_flistp = PIN_FLIST_ELEM_ADD(v_flistp, PIN_FLD_CHARGES, 
			0, ebufp);
	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_ACCOUNT_OBJ, 
			(void *)a_pdp, ebufp);

	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_BILLINFO_OBJ, 
			(void *)bi_pdp, ebufp);

	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_COMMAND, 
			(void *)&command, ebufp);

	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_PAY_TYPE, 
			(void *)&pay_type, ebufp);

	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_MERCHANT, 
			(void *)merchant, ebufp);

	PIN_FLIST_FLD_SET(c_flistp, PIN_FLD_ACH, 
			(void *)&ach, ebufp);

	/***********************************************************
	 * Set the payinfo array.
	 ***********************************************************/
	cc_flistp = PIN_FLIST_COPY(b_flistp, ebufp);
	p_flistp = PIN_FLIST_ELEM_ADD(c_flistp, PIN_FLD_PAYINFO, 0, ebufp);
	PIN_FLIST_ELEM_PUT(p_flistp, cc_flistp, fld, 0, ebufp);

	/***********************************************************
	 * Open a separate context for validation.
	 ***********************************************************/
	PCM_CONTEXT_OPEN(&vctxp, (pin_flist_t *)0, ebufp);

	PCM_OP(vctxp, PCM_OP_PYMT_VALIDATE, PCM_OPFLG_READ_UNCOMMITTED, 
		v_flistp, &r_flistp, ebufp);
	PCM_CONTEXT_CLOSE(vctxp, 0, NULL);


	/* If the pay type is CC then the return flist will have token number which needs to be replaced */
	if (pay_type && (pay_type == PIN_PAY_TYPE_CC)) {
                flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, PIN_ELEMID_ANY, 1, ebufp);
                if (flistp) {
			PIN_FLIST_ELEM_COPY(flistp, PIN_FLD_TRANSACTIONS, PIN_ELEMID_ANY,
				b_flistp, PIN_FLD_TRANSACTIONS, 0, ebufp);
                        flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_CC_INFO, PIN_ELEMID_ANY, 1, ebufp);
                        if (flistp) {
                                tokenp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_DEBIT_NUM, 1, ebufp);
                                card_type = PIN_FLIST_FLD_GET(flistp, PIN_FLD_CARD_TYPE, 1, ebufp);
                                PIN_FLIST_FLD_SET(b_flistp, PIN_FLD_DEBIT_NUM, tokenp, ebufp);
                                PIN_FLIST_FLD_SET(b_flistp, PIN_FLD_CARD_TYPE, card_type, ebufp);
                        }
		}
	} 

	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_validate_clrhouse error", ebufp);

		PIN_FLIST_DESTROY_EX(&v_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		return;
	}

        /***********************************************************
         * Check the result of the validation.
         ***********************************************************/
        flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0, 1, ebufp);
        if (flistp) {
                result = (int *)PIN_FLIST_FLD_GET(flistp, PIN_FLD_RESULT, 0, ebufp);

                if ((result != (int *)NULL) && (*result != PIN_RESULT_PASS)) {

                        pin_set_err(ebufp, PIN_ERRLOC_FM,
                                PIN_ERRCLASS_APPLICATION,
                                PIN_ERR_BAD_VALUE,
                                PIN_FLD_DEBIT_NUM, 0, *result);
                }
        }

	/***********************************************************
	 * Cleanup.
	 ***********************************************************/
	PIN_FLIST_DESTROY_EX(&v_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);

	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_validate_clrhouse error", ebufp);
	}

	return;
}

/********************************************************************
 * Common routine used to figure out the canonical country from the
 * country passed. Moved to a subroutine since its used twice. Possibly
 * should be common to more files
 ********************************************************************/
static int
fm_cust_pol_valid_payinfo_map_country(char *oldcountry,
                                      char **canon_country,
                                      pin_flist_t *i_flistp,
                                      pin_flist_t *r_flistp,
                                      pin_errbuf_t *ebufp)
{
        fm_cust_pol_map_country(i_flistp, i_flistp, oldcountry, ebufp);
        *canon_country = (char *)
                PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CANON_COUNTRY, 1, ebufp);
        if (*canon_country == (char *)NULL) {
                /* A country was supplied but did not
                 * map to a canon country so failed
                 * validation.
                 */
                (void)fm_cust_pol_valid_add_fail(r_flistp,
                                                 PIN_FLD_COUNTRY, (int)NULL,
                                                 PIN_ERR_BAD_VALUE,
                                                 PIN_CUST_BAD_VALUE_ERR_MSG,
                                                 oldcountry, ebufp);
                return 0;
        }
        return -1;
}

/********************************************************************
 * Cleanup the work done above
 *******************************************************************/
static void
fm_cust_pol_valid_payinfo_map_country_cleanup(pin_flist_t *i_flistp,
                                              pin_errbuf_t *ebufp)
{
	/************************************************
	 * Drop CANON_COUNTRY since further processing 
	 * fails if this field is part of the flist.  This
	 * field was added during the country validation.
	 *************************************************/
	if ( PIN_FLIST_FLD_GET( i_flistp, PIN_FLD_CANON_COUNTRY, 1, ebufp)) {
		PIN_FLIST_FLD_DROP(i_flistp, PIN_FLD_CANON_COUNTRY, ebufp);
	}

}

/*******************************************************************
 * fm_cust_pol_prep_payinfo_common():
 *
 *	This routine preps fields in payinfo subtype.  For credit 
 *	cards, debit number and expiration dates are preped.  For
 *	both credit cards and invoices, if state zipcodes or country
 *	is being modified, then we attempt to make sure all three are 
 *	on the list so proper validation can occur.
 *******************************************************************/
static void
fm_cust_pol_prep_payinfo_common(
	pcm_context_t	*ctxp,
	u_int		flags,
	const char 	*poid_type,
        int32           partial,
	poid_t         	*pdp,
	pin_flist_t	*b_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*cp = NULL;
	char		exp[5] = {0};
	char		month[3] = {0};
	char		year[5] = {0};
	int		i = 0, j = 0;
	void		*statep = NULL;
	void		*zipp = NULL;
	void		*countryp = NULL;
	void		*cityp = NULL;
	void		*namep = NULL;
	void		*addrp = NULL;
	void		*expp = NULL;
	void		*nump = NULL;
	void		*banknop = NULL;
	void		*banktypep = NULL;
	void		*vp = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*sub_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;

	void		*valp=NULL;
	int32		perr = 0;
        int32           info_present = PIN_BOOLEAN_FALSE;

	exp[0] = month[0] = year[0] = '\0';


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Strip spaces from the debitnum (if given)
	 ***********************************************************/
	cp = (char *)PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_DEBIT_NUM, 1, ebufp);

	if (cp != (char *)NULL) {

		while (i < (int)strlen(cp)) {

			if (isspace(cp[i])) {
				i++;
				continue;
			} else {
				cp[j] = cp[i];
				i++;
				j++;
				continue;
			}

		}

		cp[j] = '\0';
	}

	/***********************************************************
	 * Convert expiration date to MMYY (if given)
	 ***********************************************************/
	cp = (char *)PIN_FLIST_FLD_GET(b_flistp,
	   PIN_FLD_DEBIT_EXP, 1, ebufp);

	if (cp != (char *)NULL) {

	/***********************************************************
	 * We handle a few possible variations 
	 ***********************************************************/
		switch (strlen(cp)) {
		case 3:
			/*
			 * Must be something like MYY
			 */
			pin_snprintf(exp, sizeof(exp), "0%s", cp);
			break;
		case 4:
			/*
			 * Assume either MMYY or M/YY
			 */
			if ((cp[1] < '0') || (cp[1] > '9')) {
				/* Assume something like M/YY */
				sscanf(cp, "%1s%*[/.-]%2s", month, year);
				pin_snprintf(exp, sizeof(exp), "%.2s%.2s", month, year);
			} else {
				pin_snprintf(exp, sizeof(exp), "%s", cp);
			}
			break;
		case 5:
			/*
			 * Assume MM/YY or similar
			 */
			sscanf(cp, "%2s%*[/.-]%2s", month, year);
			pin_snprintf(exp, sizeof(exp), "%.2s%.2s", month, year);
			break;
		 case 6:
                        /*
                         * Assume MMYYYY or similar
                         */
                        sscanf(cp, "%2s%4[^/.-]", month, year);
                        pin_snprintf(exp, sizeof(exp), "%.2s%.2s", month, (char *)&year[2]);
                        break;
		case 7:
			/*
			 * Assume MM/YYYY or similar
			 */
			sscanf(cp, "%2s%*[/.-]%4s", month, year);
			pin_snprintf(exp, sizeof(exp), "%.2s%.2s", month, (char *)&year[2]);
			break;
		default:
			/*
			 * Take the first 4 chars?
			 */
			pin_snprintf(exp, sizeof(exp), "%4.4s", cp);
			break;
		}

		/*
		 * Put the prepped date on the flist
		 */
		PIN_FLIST_FLD_SET(b_flistp, PIN_FLD_DEBIT_EXP,
			(void *)exp, ebufp);
	}

	/*****************************************************************
	 * Attempt to to get city, state, zip and country on the list for
	 * proper validation.
	 *****************************************************************/
	if (!(flags & PCM_OPFLG_CUST_CREATE_PAYINFO) && partial)  {
		
		cityp = PIN_FLIST_FLD_GET(b_flistp, 
				PIN_FLD_CITY, 1, ebufp);
		statep = PIN_FLIST_FLD_GET(b_flistp, 
				PIN_FLD_STATE, 1, ebufp);
		zipp = PIN_FLIST_FLD_GET(b_flistp, 
				PIN_FLD_ZIP, 1, ebufp);
		countryp = PIN_FLIST_FLD_GET(b_flistp, 
				PIN_FLD_COUNTRY, 1, ebufp);
		namep = PIN_FLIST_FLD_GET(b_flistp, 
				PIN_FLD_NAME, 1, ebufp);
		addrp = PIN_FLIST_FLD_GET(b_flistp, 
				PIN_FLD_ADDRESS, 1, ebufp);

		s_flistp = PIN_FLIST_CREATE(ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, (void *)pdp, ebufp);

		if (!strcmp(poid_type, "/payinfo/invoice")) {
			sub_flistp = PIN_FLIST_SUBSTR_ADD(s_flistp,
					PIN_FLD_INV_INFO, ebufp);
		} else if (!strcmp(poid_type, "/payinfo/cc")) {
			nump = PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_DEBIT_NUM,
						 1, ebufp);
			expp = PIN_FLIST_FLD_GET(b_flistp, 
					PIN_FLD_DEBIT_EXP, 1, ebufp);
			sub_flistp = PIN_FLIST_SUBSTR_ADD(s_flistp,
					PIN_FLD_CC_INFO, ebufp);

			if (!nump) {
				PIN_FLIST_FLD_SET(sub_flistp, 
					PIN_FLD_DEBIT_NUM, (void *)NULL, ebufp);
                                info_present = PIN_BOOLEAN_TRUE;
			}

			if (!expp) {
				PIN_FLIST_FLD_SET(sub_flistp, 
					PIN_FLD_DEBIT_EXP, (void *)NULL, ebufp);
                                info_present = PIN_BOOLEAN_TRUE;
			}
		} else if (!strcmp(poid_type, "/payinfo/dd")) {
			nump = PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_DEBIT_NUM,
						 1, ebufp);
			banknop = PIN_FLIST_FLD_GET(b_flistp, 
					PIN_FLD_BANK_NO, 1, ebufp);
			banktypep = PIN_FLIST_FLD_GET(b_flistp, 
					PIN_FLD_TYPE, 1, ebufp);
			sub_flistp = PIN_FLIST_SUBSTR_ADD(s_flistp,
					PIN_FLD_DD_INFO, ebufp);
			if (!nump) {
				PIN_FLIST_FLD_SET(sub_flistp, 
					PIN_FLD_DEBIT_NUM, (void *)NULL, ebufp);
                                info_present = PIN_BOOLEAN_TRUE;
			}
			if (!banknop) {
				PIN_FLIST_FLD_SET(sub_flistp, 
					PIN_FLD_BANK_NO, (void *)NULL, ebufp);
                                info_present = PIN_BOOLEAN_TRUE;
			}
			if (!banktypep) {
				PIN_FLIST_FLD_SET(sub_flistp, 
					PIN_FLD_TYPE, (void *)NULL, ebufp);
                                info_present = PIN_BOOLEAN_TRUE;
			}
		} else if (!strcmp(poid_type, "/payinfo/ddebit")) {
			/* added for direct debit feature	*/
			sub_flistp = PIN_FLIST_SUBSTR_ADD(s_flistp,
					PIN_FLD_DDEBIT_INFO, ebufp);
		} else {
			goto checkout;
		}

		if (!statep) {
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_STATE,
				(void *)NULL, ebufp);
			info_present = PIN_BOOLEAN_TRUE;
		}

		if (!zipp) {
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ZIP,
				(void *)NULL, ebufp);
                        info_present = PIN_BOOLEAN_TRUE;
		}

		if (!countryp) {
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_COUNTRY,
				(void *)NULL, ebufp);
                        info_present = PIN_BOOLEAN_TRUE;
		}

		if (!cityp) {
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_CITY,
				(void *)NULL, ebufp);
                        info_present = PIN_BOOLEAN_TRUE;
		}

		if (!namep) {
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_NAME,
				(void *)NULL, ebufp);
                        info_present = PIN_BOOLEAN_TRUE;
		}

		if (!addrp) {
			PIN_FLIST_FLD_SET(sub_flistp, PIN_FLD_ADDRESS,
				(void *)NULL, ebufp);
                        info_present = PIN_BOOLEAN_TRUE;
		}

                if (info_present) {
			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, s_flistp, 
				&r_flistp, ebufp);

			if (!strcmp(poid_type, "/payinfo/invoice")) {
				sub_flistp = PIN_FLIST_SUBSTR_GET(r_flistp,
					PIN_FLD_INV_INFO, 0, ebufp);
			} else if (!strcmp(poid_type, "/payinfo/cc")) {
				sub_flistp = PIN_FLIST_SUBSTR_GET(r_flistp,
					PIN_FLD_CC_INFO, 0, ebufp);
			} else if (!strcmp(poid_type, "/payinfo/dd")) {
				sub_flistp = PIN_FLIST_SUBSTR_GET(r_flistp,
					PIN_FLD_DD_INFO, 0, ebufp);
			} else if (!strcmp(poid_type, "/payinfo/ddebit")) {
				sub_flistp = PIN_FLIST_SUBSTR_GET(r_flistp,
					PIN_FLD_DDEBIT_INFO, 0, ebufp);
			}

                	vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_BANK_NO, 1, 
				ebufp);
                	if (vp)
                        	PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_BANK_NO, vp,
                                	ebufp);

                	vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_TYPE, 1, 
				ebufp);
                	if (vp)
                        	PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_TYPE, vp, 
					ebufp);
                
                	vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_CITY, 1, 
				ebufp);
			if (vp)
                        	PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_CITY, vp, 
					ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_STATE, 1, 
				ebufp);
			if (vp)
                        	PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_STATE, vp, 
					ebufp);

                	vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_ZIP, 1, 
				ebufp);
			if (vp)
                        	PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_ZIP, vp, 
					ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_COUNTRY, 1, 
				ebufp);
			if (vp)
                        	PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_COUNTRY, vp,
                                	ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_NAME, 1, 
				ebufp);
			if (vp)
                        	PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_NAME, vp, 
					ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_ADDRESS, 1, 
				ebufp);
			if (vp)
                        	PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_ADDRESS, vp,
                                	ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_DEBIT_EXP, 1,
                        	ebufp);
			if (vp)
                        	PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_DEBIT_EXP, vp,
                                	ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_DEBIT_NUM, 1,
                        	ebufp);
			if (vp)
                        	PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_DEBIT_NUM, vp,
                                	ebufp);
		}
		PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	}

	/***********************************************
	 * If country not provided, try to
	 * obtain country name from pin.conf
	 * if not there then use FM_DEFAULT_COUNTRY
	 ***********************************************/
	vp = PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_COUNTRY, 
			1, ebufp);
	if ((vp == (void *)NULL) || (strlen(vp) == 0)) {

		pin_conf(FM_CUST_POL, FM_COUNTRY_TOKEN, PIN_FLDT_STR,
                       	(caddr_t *)&valp, &perr);

		switch (perr) {
		case PIN_ERR_NONE:
			vp = (void *)valp;
			break;
 
		case PIN_ERR_NOT_FOUND:
			vp = (void *)strdup(FM_DEFAULT_COUNTRY);
			break;
 
		default:
			pin_set_err(ebufp, PIN_ERRLOC_FM,
			  PIN_ERRCLASS_SYSTEM_DETERMINATE, perr, 0, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			  "Unable to read country from pin.conf",ebufp);
			vp = (void *)strdup(FM_DEFAULT_COUNTRY);
			break;
		}
		PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_COUNTRY, vp, ebufp);
	}

checkout:
	/***********************************************************
	 * Error?
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_payinfo_common error", ebufp);
	}
	return;
}

#define FLIST_NEED_REVALIDATE(field) \
        vp = PIN_FLIST_FLD_GET(flistp, field, 1,ebufp); \
        if (vp) \
                   return PIN_BOOLEAN_TRUE;
        
/********************************************************************
 * Calculate if we need to revalidate the credit card/direct debit
 * information. Returns PIN_BOOLEAN_TRUE if we need to, and
 * PIN_BOOLEAN_FALSE if we don't
 *******************************************************************/
static int32
fm_cust_pol_need_revalidate(int32 field_type,
                            pin_flist_t *flistp,
                            pin_errbuf_t *ebufp)
{
        void *vp = NULL;
        
        if (PIN_ERR_IS_ERR(ebufp))
                return PIN_BOOLEAN_FALSE;
        PIN_ERR_CLEAR_ERR(ebufp);

        /*
         * Rules implemented here are as follows:
         * - If name, address, city, state, zip, country, credit card number,
         *   cvv2, or expiration date changed, revalidate
         * - If name, address, city, state, zip, country, bank RDFI #,
         *   account type, or bank account # changed, revalidate
         */

        FLIST_NEED_REVALIDATE(PIN_FLD_NAME);
        FLIST_NEED_REVALIDATE(PIN_FLD_CITY);
        FLIST_NEED_REVALIDATE(PIN_FLD_STATE);
        FLIST_NEED_REVALIDATE(PIN_FLD_ZIP);
        FLIST_NEED_REVALIDATE(PIN_FLD_COUNTRY);
        FLIST_NEED_REVALIDATE(PIN_FLD_ADDRESS);

        switch (field_type) {
        case PIN_FLD_DD_INFO:
                FLIST_NEED_REVALIDATE(PIN_FLD_BANK_NO);
                FLIST_NEED_REVALIDATE(PIN_FLD_DEBIT_NUM);
                FLIST_NEED_REVALIDATE(PIN_FLD_TYPE);
                break;
        case PIN_FLD_CC_INFO:
                FLIST_NEED_REVALIDATE(PIN_FLD_DEBIT_EXP);
                FLIST_NEED_REVALIDATE(PIN_FLD_DEBIT_NUM);
                FLIST_NEED_REVALIDATE(PIN_FLD_SECURITY_ID);
                break;
        }
        return PIN_BOOLEAN_FALSE;
}

/*******************************************************************
 * Verifies whether payment term type is a valid paymaent term
 * information. Returns PIN_BOOLEAN_TRUE if we need to, and
 * PIN_BOOLEAN_FALSE if we don't
 *******************************************************************/
static void fm_cust_pol_validate_paymentterm ( pin_flist_t *i_flistp, 
					 pin_flist_t *r_flistp,
					 pin_errbuf_t *ebufp)
{
  	pin_flist_t    *pt_config_flist = (pin_flist_t *)NULL;
	pin_flist_t    *res_flistp = NULL;
	int32          err = PIN_ERR_NONE;
	int32	       result = 0;
	char           *key = PIN_OBJ_TYPE_CONFIG_PAYMENTTERM;
        int32          *pt_typep = NULL; 
	
	if (PIN_ERR_IS_ERR(ebufp)) {
	        return;
	}

	/* Check whether there is a valid payment term */
	pt_typep = (int32 *)PIN_FLIST_FLD_GET(i_flistp,
						     PIN_FLD_PAYMENT_TERM, 
						     1, ebufp);
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_validate_paymentterm error", ebufp);
	        return;
	}

	if ((pt_typep == NULL) || (*pt_typep == 0)) {
 	        PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
				  "Payment term not defined in the inut flist,"
				  " there is nothing to validate ",
				   i_flistp);	        
	        return;
	}

	/* Check if global cache pointer is defined */
	if (fm_cust_pol_paymentterm_ptr == (cm_cache_t *)NULL && *pt_typep != 0) {
 	        PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				  "Global pointer "
				  "fm_config_paymentterm_data_flistp is NULL");
		fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_PAYMENT_TERMS,
					   (int)NULL, PIN_ERR_BAD_VALUE, 
					   PIN_CUST_BAD_VALUE_ERR_MSG,
					   pt_typep, ebufp);

		return;
	}

	/*
	 * Go through the config objects to find the paymentterm
	 */
      
 	pt_config_flist = cm_cache_find_entry(fm_cust_pol_paymentterm_ptr, 
					      key, &err);
        switch(err) {
        case PIN_ERR_NONE:
		break;
        case PIN_ERR_NOT_FOUND:
		if ( *pt_typep != 0 ) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
				"no cache flist is found and term is not zero" );
			fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_PAYMENT_TERMS,
					(int)NULL, PIN_ERR_BAD_VALUE,
					PIN_CUST_BAD_VALUE_ERR_MSG,
					pt_typep, ebufp);
		}
                break;
        default:
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG,
                        "fm_cust_pol_validate_paymentterm(): error "
			"accessing data dictionary cache.");
                pin_set_err(ebufp, PIN_ERRLOC_CM,
                        PIN_ERRCLASS_SYSTEM_DETERMINATE,
                        err, 0, 0, 0);
		PIN_FLIST_DESTROY_EX(&pt_config_flist, NULL);
                return;
                /*****/
        }

	res_flistp = (pin_flist_t *)PIN_FLIST_ELEM_GET(pt_config_flist, 
						       PIN_FLD_PAYMENT_TERMS, 
						       *pt_typep, 1, ebufp);
	if (res_flistp == (pin_flist_t *) NULL){
		fm_cust_pol_valid_add_fail(r_flistp, PIN_FLD_PAYMENT_TERMS,
				(int)NULL, PIN_ERR_BAD_VALUE,
				PIN_CUST_BAD_VALUE_ERR_MSG,
				pt_typep, ebufp);
	}

	if(PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
				"fm_cust_pol_validate_paymentterm() error", 
				 ebufp);
	}
	PIN_FLIST_DESTROY_EX(&pt_config_flist, NULL);
        return;
}

/*********************************************************************
 * fm_cust_pol_validate_invoice_type()
 * This functions validate the Invoice Type

 * Detail, Summary, Replacement, and Correction Letter Invoice Type Values

#define PIN_INV_TYPE_DETAIL_INVOICE     0
#define PIN_INV_TYPE_SUMMARY_INVOICE    1
#define PIN_INV_TYPE_REPLACEMENT_INVOICE   0
#define PIN_INV_TYPE_CORRECTION_LETTER     1

#define PIN_INV_TYPE_OF_REGULAR_INVOICE    0x01
#define PIN_INV_CORRECTIVE_TYPE_TO_USE     0x04

 So /payinfo.PIN_FLD_INV_TYPE can have any combination of three bits
so values from 0 to 7 are valid.

 ********************************************************************/
static
void fm_cust_pol_validate_invoice_type(pin_flist_t *i_flistp,
                                        pin_flist_t *r_flistp,
                                        pin_errbuf_t *ebufp)
{
        int32           *invoice_typep = NULL;
        int32           inv_type = 0;

        if (PIN_ERR_IS_ERR(ebufp)) {
                return;
        }
        invoice_typep = (int32 *)PIN_FLIST_FLD_GET(i_flistp,
                           PIN_FLD_INV_TYPE, 1, ebufp);

        if (invoice_typep) {
		inv_type = *(int32 *)invoice_typep;
	     	if ((inv_type  < 0) ||
             	     (inv_type > 7)) {
                        fm_cust_pol_valid_add_fail(r_flistp,
                                PIN_FLD_INV_TYPE,
                                0, PIN_ERR_BAD_VALUE,
                                PIN_CUST_BAD_VALUE_ERR_MSG,
                                (void*)invoice_typep, ebufp);
		}
        }

        if (PIN_ERR_IS_ERR(ebufp)) {
                PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
                "fm_cust_pol_validate_invoice_type error", ebufp);
        }

        return;
}

/*********************************************************************
 * fm_cust_pol_valid_payinfo_sepa()
 * This functions is for validating the IBAN and BIC values according
 * to the corresponding ISO standards. the function also validates the 
 * Creditor ID/Name specified in the mandate, and for the uniqueness
 * of the UMR.
 *
 * IBAN format:
 * [A-Z]{2,2}[0-9]{2,2}[a-zA-Z0-9]{1,30}
 *
 * BIC format:
 * [A-Z]{6,6}[A-Z2-9][A-NP-Z0-9]([A-Z0-9]{3,3}){0,1}  
 *********************************************************************/
static
void fm_cust_pol_valid_payinfo_sepa(
	cm_nap_connection_t     *connp,
	poid_t			*a_pdp,
	pin_flist_t	*i_flistp,
	pin_errbuf_t	*ebufp)
{

	pcm_context_t   *ctxp = connp->dm_ctx;
	char		*ibanp = NULL;
	char		*bicp = NULL;
	char		*creditor_id = NULL;
	char		*creditor_name = NULL;
	char		*umr = NULL;
	int32		len = 0;
	int32		i,j = 0;
	pin_flist_t	*srch_flistp = NULL;
	pin_flist_t	*srch_res_flistp = NULL;
	pin_flist_t	*res_flistp = NULL;
	pin_flist_t	*tmp_flistp = NULL;
	pin_flist_t	*read_flistp = NULL;
	pin_flist_t	*read_ret_flistp = NULL;
	pin_flist_t	*temp_sepa_info_flistp = NULL;
	pin_flist_t	*temp_sepa_flistp = NULL;
	void		*vp = NULL;
	void		*vptr = NULL;
	poid_t		*srch_pdp = NULL;
	int32		s_flags = SRCH_DISTINCT;
	time_t		now_t = fm_utils_time_round_to_midnight(pin_virtual_time((time_t *)NULL));
	time_t		signed_t = 0;
	time_t		created_t = 0;
	time_t		end_t = 0;
	time_t		effective_t = 0;
	int32		elem_id = 0;
	pin_cookie_t     cookie = NULL;
	int32 		payinfo_sepa_search_size = 100;
	int32 		count = 0;
	
	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/* IBAN validation according to ISO 13616:2007 standard */
	ibanp = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_IBAN, 1, ebufp);
	if (ibanp) {
	len = strlen(ibanp);
	if ((len > IBAN_MAX_LENGTH) || (len < IBAN_MIN_LENGTH)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, (char *)"Length of IBAN is not according to ISO 13616:2007 standard");
		pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
			PIN_ERR_VALIDATION_FAILED, PIN_FLD_IBAN, 0, 0, PIN_DOMAIN_ERRORS,
			PIN_ERR_CUST_INVALID_IBAN_LENGTH,
			1, 0, NULL);
		return;
	}

	for (i = 0; i < len; i++) {
		if ((i == 0) || (i == 1)) {
			if (!isalpha(ibanp[i]) || !isupper(ibanp[i])) {
				break;
			}
		}
		else if ((i == 2) || (i == 3)) {
			if (!(isdigit(ibanp[i]))) {
				break;
			}
		}
		else {
			if (!(isalnum(ibanp[i]))) {
				break;
			}
		}
	}

	if (i < len) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, (char *)"IBAN is not according to ISO 13616:2007 standard");
                pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
                        PIN_ERR_VALIDATION_FAILED, PIN_FLD_IBAN, 0, 0, PIN_DOMAIN_ERRORS,
                        PIN_ERR_CUST_INVALID_IBAN_FORMAT,
                        1, 0, NULL);
                return;
        }
		/* Mod 97 validation for IBAN */
		if (!fm_cust_pol_iban_mod97_check(len,ibanp)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, (char *)"IBAN is not mod97 compliant");
			pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
				PIN_ERR_VALIDATION_FAILED, PIN_FLD_IBAN, 0, 0, PIN_DOMAIN_ERRORS,
				PIN_ERR_CUST_NON_MOD97_IBAN,
				1, 0, NULL);
			return;	
		}
	}

	/* BIC validation according to ISO 9362:2009 standard */
	len = 0;
	bicp = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BIC, 1, ebufp);
	if (bicp) {
	len = strlen(bicp);

	if ((len != BIC_LEN_8) && (len != BIC_LEN_11)) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, (char *)"Length of BIC not according to "
			"standards. Should be 8 or 11 characters");
                pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
                        PIN_ERR_VALIDATION_FAILED, PIN_FLD_BIC, 0, 0, PIN_DOMAIN_ERRORS,
                        PIN_ERR_CUST_INVALID_BIC_LENGTH,
                        1, 0, NULL);
                return;
        }

        for (i = 0; i < len; i++) {
                if ((i >= 0) && (i <= 5)) {
			if (!isalpha(bicp[i]) || !isupper(bicp[i])) {
                                break;
                        }
                }
                else if (i == 6) {
			if ((!isalnum(bicp[i]) || (isalpha(bicp[i]) && !isupper(bicp[i])) || (bicp[i] == '0') || (bicp[i] == '1'))) {
                                break;
                        }
                }
		else if (i == 7) {
			if ((!isalnum(bicp[i])) || (isalpha(bicp[i]) && !isupper(bicp[i])) || (bicp[i] == 'O')) {
				break;
			}
		}
		else {
                        if (!isalnum(bicp[i]) || (isalpha(bicp[i]) && !isupper(bicp[i]))) {
                                break;
                        }
                }
        }
        if (i < len) {
                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, (char *)"BIC is not according to ISO 9362:2009 standard");
                pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
                        PIN_ERR_VALIDATION_FAILED, PIN_FLD_BIC, 0, 0, PIN_DOMAIN_ERRORS,
                        PIN_ERR_CUST_INVALID_BIC_FORMAT,
                        1, 0, NULL);
                return;
        }
	}
	
	// Check if the Creditor ID and Name are valid
	creditor_id = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CREDITOR_ID, 1, ebufp);
	creditor_name = (char *)PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_CREDITOR_NAME, 1, ebufp);
	if (creditor_id && creditor_name) {
		if (!fm_cust_pol_valid_creditor_id(ctxp, a_pdp, creditor_id, creditor_name, ebufp)) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, (char *)"Creditor ID/Creditor Name combination "
								"is not existant");
			pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
				PIN_ERR_VALIDATION_FAILED, PIN_FLD_CREDITOR_ID, 0, 0, PIN_DOMAIN_ERRORS,
				PIN_ERR_CUST_NON_EXISTANT_CREDITOR_INFO,
				1, 0, NULL);
			return;	
		}	
	}

	// Check if the UMR is already existing in the system for another payinfo
	umr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MANDATE_UNIQUE_REFERENCE, 1, ebufp);
	if (umr) {
		srch_flistp = PIN_FLIST_CREATE(ebufp);
		srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB (a_pdp),"/search", -1, ebufp);
		PIN_FLIST_FLD_PUT (srch_flistp, PIN_FLD_POID, (void *)srch_pdp, ebufp);

		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
		
		vp = (void *)"select X from /payinfo/sepa where F1 like V1";
		PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, vp, ebufp);
		
		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
		temp_sepa_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_SEPA_INFO, PIN_ELEMID_ANY, ebufp);		
		PIN_FLIST_FLD_SET(temp_sepa_flistp, PIN_FLD_MANDATE_UNIQUE_REFERENCE, "%", ebufp);
		
		tmp_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, payinfo_sepa_search_size, ebufp);
		PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);
		temp_sepa_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_SEPA_INFO, PIN_ELEMID_ANY, ebufp);
		PIN_FLIST_FLD_SET(temp_sepa_flistp, PIN_FLD_MANDATE_UNIQUE_REFERENCE, NULL, ebufp);
		
		PCM_OP(ctxp, PCM_OP_STEP_SEARCH, 0, srch_flistp, &srch_res_flistp, ebufp);
		count = PIN_FLIST_ELEM_COUNT(srch_res_flistp, PIN_FLD_RESULTS, ebufp);

		while (count) {
			while ((res_flistp = PIN_FLIST_ELEM_GET_NEXT(srch_res_flistp,
                        	                PIN_FLD_RESULTS, &elem_id, 1,
                                	        &cookie, ebufp)) != (pin_flist_t *)NULL)
			{
				temp_sepa_info_flistp = PIN_FLIST_ELEM_GET(res_flistp, PIN_FLD_SEPA_INFO, PIN_ELEMID_ANY, 1, ebufp);
				if(temp_sepa_info_flistp) {
					vp = (char *)PIN_FLIST_FLD_GET(temp_sepa_info_flistp, PIN_FLD_MANDATE_UNIQUE_REFERENCE, 1, ebufp);
				}
				if(!strcmp(vp, umr))
				{
					PCM_OP(ctxp, PCM_OP_STEP_END, 0, srch_flistp, &srch_res_flistp, ebufp);
					PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, (char *)"This UMR is already present in the system"
                                			"Please correct this value, or leave it empty so that we can generate one");
                        		pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
                                		PIN_ERR_VALIDATION_FAILED, PIN_FLD_MANDATE_UNIQUE_REFERENCE, 
						0, 0, PIN_DOMAIN_ERRORS, PIN_ERR_CUST_UMR_ALREADY_EXISTS,
                                		1, 0, NULL);
					PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
			                PIN_FLIST_DESTROY_EX(&srch_res_flistp, NULL);

                        		return;
				}	
							
			}
		  	PIN_FLIST_DESTROY_EX(&srch_res_flistp, NULL);
		  	PCM_OP(ctxp, PCM_OP_STEP_NEXT, 0, srch_flistp, &srch_res_flistp, ebufp); 
		  	count = PIN_FLIST_ELEM_COUNT(srch_res_flistp, PIN_FLD_RESULTS, ebufp);
		}
		PCM_OP(ctxp, PCM_OP_STEP_END, 0, srch_flistp, &srch_res_flistp, ebufp);
		PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
		PIN_FLIST_DESTROY_EX(&srch_res_flistp, NULL);
	}

	// Check for validity of  Mandate Signed time provided during Mandate registration
	vp = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_MANDATE_SIGNED_T, 1, ebufp);
	if (vp) {
		signed_t = fm_utils_time_round_to_midnight(*(time_t *)vp);
	}

	if (!fm_utils_op_is_ancestor(connp->coip, PCM_OP_CUST_MODIFY_PAYINFO)) {
		if (!vp) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, (char *)"MANDATE_SIGNED_T needs to be "
				"provided during mandate registration");
                        pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
                                        PIN_ERR_VALIDATION_FAILED, PIN_FLD_MANDATE_SIGNED_T, 0, 0);
                        return;
                }
	}
	if (signed_t) {
                vptr = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_END_T, 1, ebufp);
		if(vptr){
			end_t = fm_utils_time_round_to_midnight(*(time_t *)vptr);
		}
		if (!PIN_POID_IS_TYPE_ONLY(a_pdp)) {
			read_flistp = PIN_FLIST_CREATE(ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_POID, a_pdp, ebufp);
			PIN_FLIST_FLD_SET(read_flistp, PIN_FLD_EFFECTIVE_T, NULL, ebufp);

			PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, read_flistp, &read_ret_flistp, ebufp);

			vp = PIN_FLIST_FLD_GET(read_ret_flistp, PIN_FLD_EFFECTIVE_T, 0, ebufp);
			if(vp){
				effective_t = fm_utils_time_round_to_midnight(*((time_t *)vp));
			}

			PIN_FLIST_DESTROY_EX(&read_flistp, NULL);
			PIN_FLIST_DESTROY_EX(&read_ret_flistp, NULL);
		}
		
		/***********************************************************************
 		* Taking minimum of end_t and effective_t as created_t to handle
 		* The scenarios like backdated accout creation or changing the pay
 		* method for already existing account. In case of backdated acc creation 
 		* end_t will be backdate and effective_t will be current PVT.
 		* In case of setting sepa pay method to an existing account effective_t 
 		* will be account creation date and end_t will be PVT. 
 		* Hence, taking older of the dates as created_t.
 		* **********************************************************************/
		if( end_t && effective_t && (end_t < effective_t) ){
                        created_t = end_t;
                }
		else if( effective_t ){
			created_t = effective_t;
		}
		else if(end_t){
			created_t = end_t;
		}
		
		if (((created_t) && ((signed_t < created_t) || (signed_t > now_t))) || 
				((!created_t) && (signed_t != now_t))) {
			PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, (char *)"MANDATE_SIGNED_T cannot be "
				"in the future nor before account creation date.");
                        pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
                                PIN_ERR_VALIDATION_FAILED, PIN_FLD_MANDATE_SIGNED_T, 0, 0, PIN_DOMAIN_ERRORS,
                                PIN_ERR_CUST_INVALID_MANDATE_SIGNED_T,
                                1, 0, NULL);
			return;
		}
	}	
	else if (vp && !signed_t) {
		PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_ERROR, (char *)"Valid value for MANDATE_SIGNED_T has to be "
				"passed.");
		pin_errbuf_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_APPLICATION,
			PIN_ERR_VALIDATION_FAILED, PIN_FLD_MANDATE_SIGNED_T, 0, 0, PIN_DOMAIN_ERRORS,
			PIN_ERR_CUST_INVALID_MANDATE_SIGNED_T,
			1, 0, NULL);
		return;
	}
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
		"fm_cust_pol_valid_payinfo_sepa error", ebufp);
	}
	return;
}

static
int32 fm_cust_pol_iban_mod97_check(
	int                   len,
        char               *ibanp)
{
	int32   i,j = 0;
	char  code[50] ;
	strncpy(code, ibanp, 4);
	code[4] = '\0';
	for ( i=0;i<(len-4);i++)
	{
		ibanp[i]=ibanp[i+4];
	}
	for(j=0;i<len;i++,j++)
		ibanp[i]=code[j];
	int iban_chk[50];
	for (i=0,j=0;i<len;i++)
	{
		if (isalpha(ibanp[i]))
		{
			iban_chk[j++]=(toupper(ibanp[i])-55)/10;
			iban_chk[j++]=(toupper(ibanp[i])-55)%10;
		}
		else
		{
			iban_chk[j++]=ibanp[i]-48;
		}
	}
	len = j;
	int value,remainder = 1,part =0;
	int iban[5];
	j=0;
	while (len-j>1)
	{
		value = 1, iban[part] =0;
		if (len-j>8)
		{
			for(i=j+8;i>=j;i--)
			{
				iban[part] += iban_chk[i]*value;
				value *= 10;
			}
			remainder = iban[part]%97;
			if(remainder != 0)
			{
				iban_chk[j+8]=remainder%10;
				iban_chk[j+7]=remainder/10;
				if (iban_chk[j+7]==0)
					j=j+8;
				else
					j=j+7;
			}
			else
				j=j+9;
		}
		else
		{
			for(i=len-1;i>=j;i--)
			{
				iban[part] += iban_chk[i]*value;
				value *= 10;
			}
			remainder = iban[part]%97;
			j=len;
		}
		part++;
	}
	if (remainder !=1)
		return PIN_BOOLEAN_FALSE;
	else
		return PIN_BOOLEAN_TRUE;
}

static
int32 fm_cust_pol_valid_creditor_id(
	pcm_context_t	*ctxp, 
	poid_t			*a_pdp,
	char			*creditor_id, 
	char			*creditor_name,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t		*srch_flistp = NULL;
	pin_flist_t		*srch_res_flistp = NULL;
	pin_flist_t		*tmp_flistp = NULL;
	poid_t			*srch_pdp = NULL;
	int32			count = 0;
	int32			s_flags = SRCH_DISTINCT;
	void			*vp = NULL;
	
	if (PIN_ERR_IS_ERR(ebufp))
		return PIN_BOOLEAN_FALSE;
	PIN_ERR_CLEAR_ERR(ebufp);

	srch_flistp = PIN_FLIST_CREATE(ebufp);
	srch_pdp = PIN_POID_CREATE(PIN_POID_GET_DB (a_pdp), "/search", -1, ebufp);
	PIN_FLIST_FLD_PUT(srch_flistp, PIN_FLD_POID, (void *)srch_pdp, ebufp);

	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_FLAGS, &s_flags, ebufp);
	
	vp = (void *)"select X from /config/creditor where F1 like V1 and F2 like V2";
	PIN_FLIST_FLD_SET(srch_flistp, PIN_FLD_TEMPLATE, vp, ebufp);
	
	tmp_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 1, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_CREDITOR_INFO, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_CREDITOR_ID, (void *)creditor_id, ebufp);
	
	tmp_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_ARGS, 2, ebufp);
	tmp_flistp = PIN_FLIST_ELEM_ADD(tmp_flistp, PIN_FLD_CREDITOR_INFO, PIN_ELEMID_ANY, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_NAME, (void *)creditor_name, ebufp);
	
	tmp_flistp = PIN_FLIST_ELEM_ADD(srch_flistp, PIN_FLD_RESULTS, 0, ebufp);
	PIN_FLIST_FLD_SET(tmp_flistp, PIN_FLD_POID, NULL, ebufp);
	
	PCM_OP(ctxp, PCM_OP_SEARCH, 0, srch_flistp, &srch_res_flistp, ebufp);
	
	count = PIN_FLIST_ELEM_COUNT(srch_res_flistp, PIN_FLD_RESULTS, ebufp);	
		
	PIN_FLIST_DESTROY_EX(&srch_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&srch_res_flistp, NULL);
	
	if (!count) {
		return PIN_BOOLEAN_FALSE;
	}
	else {
		return PIN_BOOLEAN_TRUE;
	}	
}
