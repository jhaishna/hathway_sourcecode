/*******************************************************************
 *
* Copyright (c) 1999, 2017, Oracle and/or its affiliates. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its licensors and may be used, reproduced, stored or transmitted
 *      only in accordance with a valid Oracle license or sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_Id[] = "@(#)$Id: fm_cust_pol_prep_payinfo.c /cgbubrm_mainbrm.portalbase/1 2017/08/29 06:53:38 crunkana Exp $";
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "pcm.h"
#include "ops/cust.h"
#include "pin_cust.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pin_cust.h"
#include "pinlog.h"
#include "pin_bdfs.h"
#include "pin_pymt.h"
#include "pin_bill.h"
#include "pin_inv.h"        
#include "ops/pymt.h"        
#include "fm_utils.h"        
extern char *fm_cust_pol_cm_dd_vendor;
        
EXPORT_OP void
op_cust_pol_prep_payinfo(
	cm_nap_connection_t	*connp,
	u_int			opcode,
	u_int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_cust_pol_prep_payinfo(
	pcm_context_t		*ctxp,
	u_int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
        pin_errbuf_t		*ebufp);

static void
fm_cust_pol_prep_payinfo_common(
        pcm_context_t   *ctxp,
        u_int           flags,
        char            *poid_type,
	u_int		*pay_type,
        int32           partial,
        poid_t          *pdp,
        pin_flist_t     *b_flistp,
        pin_errbuf_t    *ebufp);
 
static int
fm_cust_pol_prep_payinfo_reset_bertelsmann_mandate(pcm_context_t *ctxp,
                                                   pin_flist_t *i_flistp,
                                                   poid_t *pdp,
                                                   pin_errbuf_t *ebufp);

static int
fm_cust_pol_prep_payinfo_check_flist_value(pin_flist_t *flistp,
                                           pin_fld_num_t fld,
                                           char *oldval,
                                           pin_errbuf_t *ebufp);

void
fm_cust_pol_prep_payinfo_set_ach(pcm_context_t	*ctxp,
				 pin_flist_t	*in_flistp,
				 pin_flist_t	**out_flistpp,
				 pin_errbuf_t	*ebufp);

extern void
fm_cust_pol_map_country(
        pin_flist_t     *flistp,
        pin_flist_t     *r_flistp, 
        char            *country,
        pin_errbuf_t    *ebufp);

extern void fm_utils_get_tids();

/*******************************************************************
 * Main routine for the PCM_OP_CUST_POL_PREP_PAYINFO  command
 *******************************************************************/
void
op_cust_pol_prep_payinfo(
	cm_nap_connection_t	*connp,
	u_int			opcode,
	u_int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**ret_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;
	pin_flist_t		*r_flistp = NULL;

	/***********************************************************
	 * Null out results until we have some.
	 ***********************************************************/
	*ret_flistpp = NULL;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
	 * Insanity check.
	 ***********************************************************/
	if (opcode != PCM_OP_CUST_POL_PREP_PAYINFO) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"bad opcode in op_cust_pol_prep_payinfo", ebufp);
		return;
	}

	/***********************************************************
	 * Debug: What did we get?
	 ***********************************************************/
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_cust_pol_prep_payinfo input flist", in_flistp);

	/***********************************************************
	 * Call main function to do it
	 ***********************************************************/
	fm_cust_pol_prep_payinfo(ctxp, flags, in_flistp, &r_flistp, ebufp);

	/***********************************************************
	 * Results.
	 ***********************************************************/
	if (PIN_ERR_IS_ERR(ebufp)) {
		*ret_flistpp = (pin_flist_t *)NULL;
		PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_cust_pol_prep_payinfo error", ebufp);
	} else {
		*ret_flistpp = r_flistp;
		PIN_ERR_CLEAR_ERR(ebufp);
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_cust_pol_prep_payinfo return flist", r_flistp);
	}

	return;
}

/*******************************************************************
 * fm_cust_pol_prep_payinfo()
 *
 *	Prep the payinfo to be ready for on-line registration.
 *
 *******************************************************************/
static void
fm_cust_pol_prep_payinfo(
	pcm_context_t		*ctxp,
	u_int			flags,
	pin_flist_t		*in_flistp,
	pin_flist_t		**out_flistpp,
	pin_errbuf_t		*ebufp)
{
	void            *vp = NULL;
	pin_flist_t     *flistp = NULL;
	pin_flist_t     *ret_flistp = NULL;
	pin_flist_t     *inv_flistp = NULL;
	pin_flist_t     *i_flistp = NULL;
    pin_flist_t     *b_flistp = NULL;	
	pin_flist_t		*debtor_flistp = NULL;
	pin_flist_t     *rout_flistp = NULL;
	pin_flist_t	*r_flistp = NULL;
	poid_t          *pdp = NULL;
	poid_t          *ipdp = NULL;
	poid_t          *new_pdp = NULL;
	char            poid_type[256] = {0};
	char            *obj_type = NULL;
	const char  	*p_type = NULL;
	char            *payment_type = NULL;
	char            *err_msg = NULL;
        poid_t          *pay_pdp = NULL;
	u_int64         database = 0;
	u_int           *pay_type = NULL;
	u_int           *type = NULL;
	int32           option = 0;
	time_t		due_t = 0;
	u_int		due_dom = 0;
	/* The default dummy value has been changed from 0 to 1 */
	int32		dummy = 1;
	poid_t		*valp = NULL;
	int32		perr = 0;
	int32           *partialp = NULL;
	int32           *payment_term_typep = NULL;
	int32           payment_term_zero = 0;
	int32           *offsetp = NULL;
	int32           offset_default = -1;
	int32		field = 0;
	int32		is_creation = PIN_BOOLEAN_FALSE;
	int32		inv_type = 0;
	int32		in_type = 0;
	int32		inv_type_passed = PIN_BOOLEAN_FALSE;
	void		*countryp = NULL;
	char		*canon_country = NULL;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/***********************************************************
         * Modify the Input Flist for formatting
         ***********************************************************/
        pay_pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp,
                        PIN_FLD_POID, 0, ebufp);
	
	if (pay_pdp && (PIN_POID_IS_TYPE_ONLY(pay_pdp))) {
		is_creation = PIN_BOOLEAN_TRUE;
	}

        partialp = (int32 *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_FLAGS,
                        1, ebufp);
        if (partialp == NULL) {
                partialp = ( int* )&dummy;
        }

        payment_type = (char *)PIN_POID_GET_TYPE(pay_pdp);

        type = (u_int *) PIN_FLIST_FLD_GET(in_flistp,
                        PIN_FLD_PAY_TYPE, 1, ebufp);

        i_flistp = ( pin_flist_t* ) PIN_FLIST_SUBSTR_GET(in_flistp,
                                        PIN_FLD_INHERITED_INFO, 1,ebufp);

        PIN_ERR_CLEAR_ERR( ebufp );
        /***********************************************************
         * Identify the bill type and then fill in the missing fields
           that are required from the database for the validation
           steps
         ***********************************************************/

	if( type != (u_int*)NULL ){
        	switch( *type ) {

        	case PIN_PAY_TYPE_CC:
                	b_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_CC_INFO,
                                        PIN_ELEMID_ANY,1,ebufp);
			field = PIN_FLD_CC_INFO;
                	break;
        	case PIN_PAY_TYPE_DD:
                	b_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DD_INFO,
                                        PIN_ELEMID_ANY,1,ebufp);
			field = PIN_FLD_DD_INFO;
                	break;
        	case PIN_PAY_TYPE_INVOICE:
                	b_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_INV_INFO,
                                        PIN_ELEMID_ANY,1,ebufp);
			field = PIN_FLD_INV_INFO;
                	break;
        	case PIN_PAY_TYPE_DDEBIT:
                	b_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_DDEBIT_INFO,
                        		PIN_ELEMID_ANY,1,ebufp);
			field = PIN_FLD_DDEBIT_INFO;
                	break;
        	case PIN_PAY_TYPE_SEPA:
                	b_flistp = PIN_FLIST_ELEM_GET(i_flistp, PIN_FLD_SEPA_INFO,
                        		PIN_ELEMID_ANY,1,ebufp);
			field = PIN_FLD_SEPA_INFO;
                	break;
        	default:
                	b_flistp = ( pin_flist_t* )NULL;
        	}
	} else {
		b_flistp = ( pin_flist_t* )NULL;
	}
	
        if( b_flistp ){

                fm_cust_pol_prep_payinfo_common(ctxp, flags, payment_type, type, 
                                        *partialp,pay_pdp,b_flistp,ebufp);
        } else if (is_creation) {
		switch( field ) {
			case PIN_FLD_CC_INFO:
				err_msg = "PIN_FLD_CC_INFO is missing";
				break;
			case PIN_FLD_DD_INFO:
				err_msg = "PIN_FLD_DD_INFO is missing";
				break;
			case PIN_FLD_INV_INFO:
				err_msg = "PIN_FLD_INV_INFO is missing";
				break;
			case PIN_FLD_DDEBIT_INFO:
				err_msg = "PIN_FLD_DDEBIT_INFO is missing";
				break;
			case PIN_FLD_SEPA_INFO:
				err_msg = "PIN_FLD_SEPA_INFO is missing";
				break;
		}
		if (err_msg) {
			pin_set_err(ebufp, PIN_ERRLOC_FM, 
				PIN_ERRCLASS_SYSTEM_DETERMINATE, 
				PIN_ERR_BAD_VALUE, field, 0, 0);
			PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, err_msg, ebufp);
				return;
		}
        }

	/***********************************************************
	 * Create outgoing flist
	 ***********************************************************/
	*out_flistpp = PIN_FLIST_CREATE(ebufp);

	/***********************************************************
	 * Get and set the account object poid for later use.
	 ***********************************************************/
	pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, 
			PIN_FLD_ACCOUNT_OBJ, 0, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, 
			PIN_FLD_ACCOUNT_OBJ, pdp, ebufp);

	/***********************************************************
	 *
	 * Need to specify where to store /invoice objects for this
	 * new account.  The /payinfo storable class contains a 
	 * field called PIN_FLD_INVOICE_OBJ which is used to store
	 * a type_only poid of the location where invoices should
	 * be stored for the account.  
	 *
	 * Default policy is to read the cm pin.conf and use the
	 *
	 ***********************************************************/

	pin_conf(FM_CUST_POL, FM_PAYINFO_INVOICE_DB_TOKEN, PIN_FLDT_POID,
		(caddr_t *)&valp, &perr);

	switch (perr) {
	case PIN_ERR_NONE:
		vp = (void *)valp;
		break;

	case PIN_ERR_NOT_FOUND:
		database = PIN_POID_GET_DB( pdp );
		vp = (void *)PIN_POID_CREATE( database, 
				"/invoice", -1, ebufp );
		break;

	default:
		pin_set_err(ebufp, PIN_ERRLOC_FM,
		  PIN_ERRCLASS_SYSTEM_DETERMINATE, perr, 0, 0, 0);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
  "Unable to read payinfo default invoice_db poid from pin.conf",ebufp);
		goto checkout;
	}

	PIN_FLIST_FLD_PUT(*out_flistpp, PIN_FLD_INVOICE_OBJ, vp, ebufp);

	/***********************************************************
	 * Now get the PIN_FLD_POID to continue
	 ***********************************************************/

	pdp = (poid_t *)PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_POID, 0, ebufp);
	memset(poid_type, '\0', sizeof(poid_type));

        /***********************************************************
         * Read and set the Payment term here
         **********************************************************/	       

        payment_term_typep = (int32 *)PIN_FLIST_FLD_GET(in_flistp,
				     PIN_FLD_PAYMENT_TERM, 1, ebufp);

	if (payment_term_typep){
	        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_PAYMENT_TERM,
			  (int32 *)payment_term_typep, ebufp);
	} else {

	        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_PAYMENT_TERM,
			  (int32 *)&payment_term_zero, ebufp);
	}
	/***********************************************************
	 * Set payment offset value (default is -1)
	 ***********************************************************/
	offsetp = (int32 *)PIN_FLIST_FLD_GET(in_flistp,
                                     PIN_FLD_PAYMENT_OFFSET, 1, ebufp);
	if (offsetp) 
	{
		if (*offsetp < 0){
        		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_PAYMENT_OFFSET,
                		(int32 *)&offset_default, ebufp);

		} else {
        		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_PAYMENT_OFFSET,
                		(int32 *)offsetp, ebufp);
		}
	} else {
        	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_PAYMENT_OFFSET,
                	(int32 *)&offset_default, ebufp);
	}


	/***********************************************************
         * Read and set the Payinfo Name here
         **********************************************************/
	vp = PIN_FLIST_FLD_GET(in_flistp,
                                     PIN_FLD_NAME, 1, ebufp);

        if(vp) {
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_NAME,
                                	vp, ebufp);
        }

	/***********************************************************
	 * Read and Set Detail/Summary invoice Type
	 **********************************************************/
	vp = PIN_FLIST_FLD_GET(in_flistp,
				PIN_FLD_INV_TYPE, 1, ebufp);


	if(vp) {
		in_type = *(int32 *)vp; 
		inv_type_passed = PIN_BOOLEAN_TRUE;
	}

	if(in_type & PIN_INV_TYPE_REPLACEMENT) {
		if(in_type & PIN_INV_TYPE_SUMMARY) 
		{
			inv_type = inv_type | 0x03; 
			
		}
	}
	else if(in_type & PIN_INV_TYPE_CORR_LETTER){
		if(in_type & PIN_INV_TYPE_DETAIL) {
			inv_type = inv_type | 0x04;
		}
		else {
 			inv_type = inv_type | 0x07;
	
		} 
	}
	else {
		inv_type = in_type;
	}

	if (inv_type_passed) {
		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_INV_TYPE,
							(void *)&inv_type, ebufp);
	}

	/***********************************************************
	 * Setup the right poid type depending on pay_type.
	 ***********************************************************/
	if (PIN_POID_GET_ID(pdp) == -1)  {	

		option = 0;
		database = PIN_POID_GET_DB(pdp);

		pay_type = (u_int *) PIN_FLIST_FLD_GET(in_flistp, 
			PIN_FLD_PAY_TYPE, 1, ebufp);

		if (pay_type == (u_int *)NULL) {
			strcpy(poid_type, "/payinfo");
			option = 1;
		} else { 
			switch (*pay_type) {
			case PIN_PAY_TYPE_UNDEFINED:
				strcpy(poid_type, "/payinfo");
				option = 1;
				break;

			case PIN_PAY_TYPE_PREPAID:
				strcpy(poid_type, "/payinfo/prepaid");
				option = 1;
				break;

			case PIN_PAY_TYPE_INVOICE:
				strcpy(poid_type, "/payinfo/invoice");
				break;

			case PIN_PAY_TYPE_DEBIT:
				strcpy(poid_type, "/payinfo/debit");
				option = 1;
				break;

			/* added with direct debit feature	*/
			case PIN_PAY_TYPE_DDEBIT:
				strcpy(poid_type, "/payinfo/ddebit");
				option = 1;
				break;

			case PIN_PAY_TYPE_CC:
				strcpy(poid_type, "/payinfo/cc");
				break;

			case PIN_PAY_TYPE_DD:
				strcpy(poid_type, "/payinfo/dd");
				option = 1;
				break;

			case PIN_PAY_TYPE_SEPA:
				strcpy(poid_type, "/payinfo/sepa");
				option = 1;
				break;

			case PIN_PAY_TYPE_SMARTC:
				strcpy(poid_type, "/payinfo/smartc");
				option = 1;
				break;

			default:
				strcpy(poid_type, "/payinfo");
				option = 1;
				break;
			}
		}
	
		new_pdp = PIN_POID_CREATE(database, poid_type, -1, ebufp);
		PIN_FLIST_FLD_PUT(*out_flistpp, PIN_FLD_POID, new_pdp, ebufp);
		
		due_t = (((60 * 60) * 24) * 30);
		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_RELATIVE_DUE_T, 
			(void *)&due_t, ebufp);
		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_DUE_DOM, 
			(void *)&due_dom, ebufp);
	} else {
		PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_POID, pdp, ebufp);
		p_type = PIN_POID_GET_TYPE(pdp);
		if ( p_type ) {
			strcpy(poid_type, p_type);
		}
		option = 1;
	}

	/***********************************************************
	 * Prepare the payinfo subtype - invoice or credit card.
	 * (new: and also for Direct Debit)
	 ***********************************************************/
	flistp = PIN_FLIST_SUBSTR_GET(in_flistp, PIN_FLD_INHERITED_INFO, 
		option, ebufp);
	if (!flistp) {
		goto checkout;
	}

	/***********************************************************
	 *
	 * INVOICE PAYMENT TYPE 
	 *
	 ***********************************************************/
	if (!strcmp(poid_type, "/payinfo/invoice")) {
		inv_flistp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_INV_INFO, 
			PIN_ELEMID_ANY, option, ebufp);
		if (inv_flistp) {
			PIN_FLIST_ELEM_SET(*out_flistpp, inv_flistp, 
					PIN_FLD_INV_INFO, 0, ebufp);
		}

	/***********************************************************
	 *
	 * CREDIT CARD PAYMENT TYPE
	 *
	 ***********************************************************/

	} else if (!strcmp(poid_type, "/payinfo/cc")) {

		vp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_CC_INFO, 
			PIN_ELEMID_ANY, option, ebufp);
		if (vp != NULL) {

			/* Set the ACH */
			fm_cust_pol_prep_payinfo_set_ach(ctxp, 
				in_flistp, out_flistpp, ebufp);

			PIN_FLIST_ELEM_SET(*out_flistpp, vp, 
				PIN_FLD_CC_INFO, 0,  ebufp);
		}


	/***********************************************************
	 *
	 * DIRECT DEBIT PAYMENT TYPE (US & CANADIAN & Bertelsmann)
	 *
	 ***********************************************************/

	} else if (!strcmp(poid_type, "/payinfo/dd")) {

		vp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_DD_INFO, 
			PIN_ELEMID_ANY, option, ebufp);
		if (vp != NULL) {
                        int32 mandate_status;
                        mandate_status = PIN_MANDATE_PENDING_PROVISION;

                        /* Execute the following iff Bertelsmann is enabled */
                        if (fm_cust_pol_cm_dd_vendor &&
                            !strcasecmp(fm_cust_pol_cm_dd_vendor,
                                        "Bertelsmann")) {
                                /* We need to reset the mandate status if:
                                 * - its a new account
                                 * - the name is changed
                                 * - the account number is changed
                                 * - the bank number is changed
                                 */
                                if (flags & PCM_OPFLG_CUST_CREATE_PAYINFO ||
                                    fm_cust_pol_prep_payinfo_reset_bertelsmann_mandate(ctxp, vp, pdp, ebufp)) {
                                        PIN_FLIST_FLD_SET(vp,
                                                       PIN_FLD_MANDATE_STATUS,
                                                       &mandate_status, ebufp);
                                }
                        }

			/* Set the ACH */
			fm_cust_pol_prep_payinfo_set_ach(ctxp, in_flistp, 
							out_flistpp, ebufp); 

			PIN_FLIST_ELEM_SET(*out_flistpp, vp, 
				PIN_FLD_DD_INFO, 0,  ebufp);
		}

	/***********************************************************
	 *
	 * DIRECT DEBIT PAYMENT TYPE (FTI)
	 *
	 ***********************************************************/

	} else if (!strcmp(poid_type, "/payinfo/ddebit")) {
		vp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_DDEBIT_INFO, 
			PIN_ELEMID_ANY, option, ebufp);
		if (vp != NULL) {

			/* Set the ACH */
			fm_cust_pol_prep_payinfo_set_ach(ctxp, in_flistp,
						 out_flistpp, ebufp);

			PIN_FLIST_ELEM_SET(*out_flistpp, vp, 
                                           PIN_FLD_DDEBIT_INFO, 0,
                                           ebufp);
                        
		}

	/***********************************************************
         *
         * SEPA PAYMENT TYPE
         *
         ***********************************************************/
        } else if (!strcmp(poid_type, "/payinfo/sepa")) {
                vp = PIN_FLIST_ELEM_GET(flistp, PIN_FLD_SEPA_INFO,
                        PIN_ELEMID_ANY, option, ebufp);
                if (vp != NULL) {				
					debtor_flistp = PIN_FLIST_ELEM_GET(vp, PIN_FLD_DEBTOR_INFO,
								PIN_ELEMID_ANY, 1, ebufp);

					if (debtor_flistp) {
						countryp = (char *)PIN_FLIST_FLD_GET(debtor_flistp, PIN_FLD_COUNTRY, 0, ebufp);
						fm_cust_pol_map_country(debtor_flistp, debtor_flistp, countryp, ebufp);

						canon_country = (char *)
								PIN_FLIST_FLD_GET(debtor_flistp, PIN_FLD_CANON_COUNTRY, 1, ebufp);

						if (canon_country == (char *)NULL) {
							/* A country was supplied but did not
							 * map to a canon country so failed
							 * validation.
							 */
							pin_set_err(ebufp, PIN_ERRLOC_FM, PIN_ERRCLASS_SYSTEM_DETERMINATE,
									PIN_ERR_VALIDATION_FAILED, PIN_FLD_COUNTRY, 0, 0);
							PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR, "fm_cust_pol_valid_payinfo error"
									": sepa validation error", ebufp);
							return;
						}
						else {
							PIN_FLIST_FLD_SET(debtor_flistp, PIN_FLD_COUNTRY, canon_country, ebufp);
							PIN_FLIST_FLD_DROP(debtor_flistp, PIN_FLD_CANON_COUNTRY, ebufp);
						}
					}
                    PIN_FLIST_ELEM_SET(*out_flistpp, vp,
                                        PIN_FLD_SEPA_INFO, 0, ebufp);										
                }

		}

	/* Read the payinfo object to get payterm value */
	if (pay_pdp && (PIN_POID_IS_TYPE_ONLY(pay_pdp))) {

                PIN_ERR_LOG_MSG(PIN_ERR_LEVEL_DEBUG, "Account Creation");

         }
         else {
                r_flistp = PIN_FLIST_CREATE(ebufp);
                PIN_FLIST_FLD_SET(r_flistp, PIN_FLD_POID, (void *)pay_pdp, ebufp);
                PIN_FLIST_FLD_SET(r_flistp,  PIN_FLD_PAYMENT_TERM, NULL, ebufp);
                PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, r_flistp, &rout_flistp, ebufp);
                PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
		if(!payment_term_typep)
		{
			PIN_FLIST_FLD_COPY(rout_flistp, PIN_FLD_PAYMENT_TERM, *out_flistpp, PIN_FLD_PAYMENT_TERM, ebufp);
		}
                
         }

checkout:
	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_payinfo error", ebufp);
	}
	return;
}

/***********************************************************************
 * fm_cust_pol_prep_payinfo_check_flist_value
 *
 * Check to see if the field specified in the flist has changed value
 * to the old one (passed in)
 **********************************************************************/
static int
fm_cust_pol_prep_payinfo_check_flist_value(pin_flist_t *flistp,
                                           pin_fld_num_t fld,
                                           char *oldval,
                                           pin_errbuf_t *ebufp)
{
        char *cptr = NULL;

        /* Bail early if there's an error */
        if (PIN_ERR_IS_ERR(ebufp)) {
                return PIN_BOOLEAN_FALSE;
        }
        PIN_ERR_CLEAR_ERR(ebufp);

        /* If there is an old value, check the new one */
        if (oldval) {
                cptr = PIN_FLIST_FLD_GET(flistp, fld, 0, ebufp);
                if (cptr && strcmp(cptr, oldval))
                        return PIN_BOOLEAN_TRUE;
        }
        return PIN_BOOLEAN_FALSE;
}
        
/***********************************************************************
 * fm_cust_pol_prep_payinfo_reset_bertelsmann_mandate
 * 
 * Reset the mandate field if the user changed any of the values that
 * are relevant to it
 **********************************************************************/
static int
fm_cust_pol_prep_payinfo_reset_bertelsmann_mandate(pcm_context_t *ctxp,
                                                   pin_flist_t *i_flistp,
                                                   poid_t *pdp,
                                                   pin_errbuf_t *ebufp)
{
        pin_flist_t *s_flistp = NULL;
        pin_flist_t *r_flistp = NULL;
        pin_flist_t *flistp = NULL;
        char *name = NULL;
        char *bank_no = NULL;
        char *debit_num = NULL;
        int32 rval = PIN_BOOLEAN_FALSE;
        
        if (PIN_ERR_IS_ERR(ebufp)) {
                return PIN_BOOLEAN_FALSE;
        }
        PIN_ERR_CLEAR_ERR(ebufp);

        /* Step 1. If this is a new payment type, then return yes */
        if (PIN_POID_GET_ID(pdp) == -1)
                return PIN_BOOLEAN_TRUE;

        name = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_NAME, 1, ebufp);
        bank_no = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_BANK_NO, 1, ebufp);
        debit_num = PIN_FLIST_FLD_GET(i_flistp, PIN_FLD_DEBIT_NUM, 1, ebufp);

        if (!name && !bank_no && !debit_num) {
                return PIN_BOOLEAN_FALSE;
        }
        
        /* If we get here, then it must be a change. So we need to read
         * the old information in in order to compare data
         */
        s_flistp = PIN_FLIST_CREATE(ebufp);
        PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, pdp, ebufp);

        /* Now choose the fields we want to retrieve */
        flistp = PIN_FLIST_ELEM_ADD(s_flistp, PIN_FLD_DD_INFO, 0, ebufp);
        if (name)
                PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME, NULL, ebufp);
        if (bank_no)
                PIN_FLIST_FLD_SET(flistp, PIN_FLD_BANK_NO, NULL, ebufp);
        if (debit_num)
                PIN_FLIST_FLD_SET(flistp, PIN_FLD_DEBIT_NUM, NULL, ebufp);

        PCM_OP(ctxp, PCM_OP_READ_FLDS, 0, s_flistp, &r_flistp, ebufp);
        PIN_FLIST_DESTROY_EX(&s_flistp, NULL);

        if (PIN_ERR_IS_ERR(ebufp)) {
                goto cleanup;
        }
        
        flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_DD_INFO, PIN_ELEMID_ANY,
                                    0, ebufp);

        /* Now check each field that is relevant to Bertelsmann */
        if (fm_cust_pol_prep_payinfo_check_flist_value(flistp,
                                                       PIN_FLD_NAME,
                                                       name, ebufp)) {
                rval = PIN_BOOLEAN_TRUE;
                goto cleanup;
        }
        if (fm_cust_pol_prep_payinfo_check_flist_value(flistp,
                                                       PIN_FLD_BANK_NO,
                                                       bank_no, ebufp)) {
                rval = PIN_BOOLEAN_TRUE;
                goto cleanup;
        }
        if (fm_cust_pol_prep_payinfo_check_flist_value(flistp,
                                                       PIN_FLD_DEBIT_NUM,
                                                       debit_num, ebufp)) {
                rval = PIN_BOOLEAN_TRUE;
                goto cleanup;
        }
 cleanup:                                                             
        PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
        return rval;
}

/****************************************************************************
** fm_cust_pol_prep_payinfo_set_ach():
** 
** This function will setup the Automatic Clearing House that needs to be used
**
******************************************************************************/
void
fm_cust_pol_prep_payinfo_set_ach(
	pcm_context_t	*ctxp,
	pin_flist_t	*in_flistp,
	pin_flist_t	**out_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*s_flistp	= NULL;
	pin_flist_t	*r_flistp	= NULL;
	pin_flist_t	*flistp		= NULL;
	poid_t		*a_pdp		= NULL;
	int32		rec_id		= 0;
	void		*vp		= NULL;
		
	if (PIN_ERR_IS_ERR(ebufp)) return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/** Call the PCM_OP_PYMT_GET_ACH_INFO to get the ACH and 
	 ** MERCHANT and set them.
	 **/
	a_pdp = PIN_FLIST_FLD_GET(*out_flistpp, PIN_FLD_POID, 0, ebufp);
	s_flistp = PIN_FLIST_CREATE(ebufp);
	PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_POID, a_pdp, ebufp);
        
	vp = PIN_FLIST_FLD_GET(in_flistp, PIN_FLD_ACH, 1, ebufp);
        if (vp) {
                rec_id = *(int32 *)vp;
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_ACH,
                                        (void *) &rec_id, ebufp); 
	}
	
	PCM_OP(ctxp, PCM_OP_PYMT_GET_ACH_INFO, 0, s_flistp,
                                                &r_flistp, ebufp);
	
	flistp = PIN_FLIST_ELEM_GET(r_flistp, PIN_FLD_RESULTS, 0,
					 PIN_ELEMID_ANY, ebufp);

        vp = PIN_FLIST_FLD_GET(flistp, PIN_FLD_MERCHANT, 0, ebufp);
	PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_MERCHANT, vp, ebufp);
        PIN_FLIST_FLD_SET(*out_flistpp, PIN_FLD_ACH, (void*)&rec_id, ebufp);
	
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_cust_pol_prep_payinfo_set_ach search error", ebufp);
	}
	PIN_FLIST_DESTROY_EX(&s_flistp, NULL);
	PIN_FLIST_DESTROY_EX(&r_flistp, NULL);
	return;
}

/*******************************************************************
 * fm_cust_pol_prep_payinfo_common():
 *
 *	This routine preps fields in payinfo subtype.  For credit 
 *	cards, debit number and expiration dates are preped.  For
 *	both credit cards and invoices, if state zipcodes or country
 *	is being modified, then we attempt to make sure all three are 
 *	on the list so proper validation can occur. For SEPA payment
 *	type if no mandate reference number is given, then a reference
 *	is generated.
 *******************************************************************/
static void
fm_cust_pol_prep_payinfo_common(
	pcm_context_t	*ctxp,
	u_int		flags,
	char            *poid_type,
	u_int		*pay_type,
        int32           partial,
	poid_t         	*pdp,
	pin_flist_t	*b_flistp,
	pin_errbuf_t	*ebufp)
{
	char		*cp = NULL;
	char		exp[5] = {0};
	char		month[3] = {0};
	char		year[5] = {0};
	int32		i = 0;
	int32		j = 0;
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
	int32		item_seq = 0;
	char		separator[64];
	char		sequence_id[64];
	char		item_no[256];
	time_t 		now = 0;
	int32 		man_type = 0;

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
	if (*pay_type != PIN_PAY_TYPE_SEPA) {

	if (!(flags & PCM_OPFLG_CUST_CREATE_PAYINFO) && partial )  {
		
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
				PIN_FLIST_FLD_SET(sub_flistp, 
					PIN_FLD_CARD_TYPE, (void *)NULL, ebufp);
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

	                vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_BANK_NO, 
				1, ebufp);
	                if (vp)
	                        PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_BANK_NO, 
					vp, ebufp);

	                vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_TYPE, 
				1, ebufp);
	                if (vp)
	                        PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_TYPE, 
					vp, ebufp);
                
	                vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_CITY, 
				1, ebufp);
			if (vp)
	                        PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_CITY, 
					vp, ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_STATE, 
				1, ebufp);
			if (vp)
	                        PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_STATE, 
					vp, ebufp);

	                vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_ZIP, 
				1, ebufp);
			if (vp)
	                        PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_ZIP, 
					vp, ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_COUNTRY, 
				1, ebufp);
			if (vp)
	                        PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_COUNTRY, 
					vp, ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_NAME, 
				1, ebufp);
			if (vp)
	                        PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_NAME, 
					vp, ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_ADDRESS, 
				1, ebufp);
			if (vp)
	                        PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_ADDRESS, 
					vp, ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_DEBIT_EXP, 
				1, ebufp);
			if (vp)
	                        PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_DEBIT_EXP, 
					vp, ebufp);

			vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_DEBIT_NUM, 
				1, ebufp);
			if (vp) {
	                        PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_DEBIT_NUM, 
					vp, ebufp);
			
				vp = PIN_FLIST_FLD_TAKE(sub_flistp, PIN_FLD_CARD_TYPE, 
					1, ebufp);
				if (vp) {
					PIN_FLIST_FLD_PUT(b_flistp, PIN_FLD_CARD_TYPE, 
						vp, ebufp);
				}
			}
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
	}
	else {
		/* Generate a mandate reference number if not passed. A new
		 * data sequence has been added for mandate reference
		 */
		vp = PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_MANDATE_UNIQUE_REFERENCE, 1, ebufp);
		if (!vp && (PIN_POID_IS_TYPE_ONLY(pdp))) {
			memset(separator, '\0', sizeof(separator));
			memset(sequence_id, '\0', sizeof(sequence_id));
			memset(item_no, '\0', sizeof(item_no));

			fm_utils_get_tids(ctxp, PIN_POID_GET_DB(pdp),
				1, PIN_SEQUENCE_TYPE_MANDATE_ID, &item_seq, 
				&sequence_id[0], &separator[0], ebufp);	

			sprintf(item_no, "%s%s%04d", sequence_id, separator, item_seq);
			PIN_FLIST_FLD_SET(b_flistp, PIN_FLD_MANDATE_UNIQUE_REFERENCE, 
				(void *)&item_no[0], ebufp);

		}

		if (PIN_POID_IS_TYPE_ONLY(pdp)) {
			now = pin_virtual_time((time_t *)NULL);
			PIN_FLIST_FLD_SET(b_flistp, PIN_FLD_LAST_POSTED_T, (void *)&now, ebufp);
		}

		/* Validate mandate type and set the right value */
		vp = (int32 *)PIN_FLIST_FLD_GET(b_flistp, PIN_FLD_MANDATE_TYPE, 1, ebufp);		
		if (vp) {
			if (*(int32 *)vp == 1) {
				man_type = PIN_MANDATE_TYPE_FRST;
				PIN_FLIST_FLD_SET(b_flistp, PIN_FLD_MANDATE_TYPE, &man_type, ebufp);
			}
			else if (*(int32 *)vp == 0){
				man_type = PIN_MANDATE_TYPE_OOFF;
				PIN_FLIST_FLD_SET(b_flistp, PIN_FLD_MANDATE_TYPE, &man_type, ebufp);
			}
			else {
				pin_set_err(ebufp, PIN_ERRLOC_FM,
						PIN_ERRCLASS_SYSTEM_DETERMINATE, PIN_ERR_VALIDATION_FAILED, 
						PIN_FLD_MANDATE_TYPE, 0, 0);
				PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
					"Invalid mandate type",ebufp);	
			}
		}
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
