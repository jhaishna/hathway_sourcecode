/*******************************************************************
 *
 *      Copyright (c) 2000 - 2007 Oracle. All rights reserved.
 *
 *      This material is the confidential property of Oracle Corporation
 *      or its subsidiaries or licensors and may be used, reproduced, stored
 *      or transmitted only in accordance with a valid Oracle license or
 *      sublicense agreement.
 *
 *******************************************************************/

#ifndef lint
static  char Sccs_id[] = "@(#)%Portal Version: fm_rate_pol_post_tax.c:IDCmod7.3PatchInt:1:2007-Apr-05 15:18:41 %";
#endif

/*******************************************************************
 * Contains the PCM_OP_RATE_POL_POST_TAX operation. 
 *
 * Used to provide a chance to modify/update any tax data 
 * that is coming from the DM right after tax calculation.
 *
 *******************************************************************/

#include <stdio.h> 
#include <string.h> 
 
#include "pcm.h"
#include "ops/rate.h"
#include "pin_bill.h"
#include "pin_cust.h"
#include "pin_rate.h"
#include "cm_fm.h"
#include "pin_errs.h"
#include "pinlog.h"

#define FILE_SOURCE_ID  "fm_rate_pol_post_tax.c(4)"

PIN_IMPORT int32 tax_return_loglevel;
PIN_IMPORT int32 tax_return_juris;

/*******************************************************************
 * Routines contained within.
 *******************************************************************/
EXPORT_OP void
op_rate_pol_post_tax(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

static void
fm_rate_pol_post_tax(
	pcm_context_t		*ctxp,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp);

/*******************************************************************
 * Adding #ifdef effectively comments out the code, as the reference  
 * to this fucntion is also based on this macro. This is done to 
 * nullify the compiler warning. 
 *
 *******************************************************************/
#ifdef ADD_USF_SURCHARGE
static void
fm_rate_pol_add_surcharge(
	pin_flist_t		*i_flistp,
	pin_flist_t		*o_flistp,
	pin_errbuf_t		*ebufp);
#endif

/*******************************************************************
 * Main routine for the PCM_OP_RATE_POL_POST_TAX operation.
 *******************************************************************/
void
op_rate_pol_post_tax(
	cm_nap_connection_t	*connp,
	int32			opcode,
	int32			flags,
	pin_flist_t		*i_flistp,
	pin_flist_t		**o_flistpp,
	pin_errbuf_t		*ebufp)
{
	pcm_context_t		*ctxp = connp->dm_ctx;


	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Insanity check.
	 */
	if (opcode != PCM_OP_RATE_POL_POST_TAX) {
		pin_set_err(ebufp, PIN_ERRLOC_FM,
			PIN_ERRCLASS_SYSTEM_DETERMINATE,
			PIN_ERR_BAD_OPCODE, 0, 0, opcode);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_post_tax opcode error", ebufp);
		return;
	}

	/*
	 * Debug: What we got.
	 */
	PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
		"op_rate_pol_post_tax input flist", i_flistp);

	/*
	 * Do the actual op in a sub.
	 */
	fm_rate_pol_post_tax(ctxp, i_flistp, o_flistpp, ebufp);

	/*
	 * Error?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"op_rate_pol_post_tax error", ebufp);
	} else {
		/*
		 * Debug: What we're sending back.
		 */
		PIN_ERR_LOG_FLIST(PIN_ERR_LEVEL_DEBUG,
			"op_rate_pol_post_tax return flist", *o_flistpp);
	}

	return;
}

/*******************************************************************
 * fm_rate_pol_post_tax():
 *
 *    Use this policy to modify/update any tax information in the
 *    output flist after taxes have been calculated.  
 *
 *******************************************************************/
static void
fm_rate_pol_post_tax(
	pcm_context_t	*ctxp,
	pin_flist_t	*i_flistp,
	pin_flist_t	**o_flistpp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*ret_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	pin_flist_t	*t_flistp = NULL;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie2 = NULL;
	int32		elemid = 0;
	int32		elemid2 = 0;
	char		*reason = NULL;
	char		*descr = NULL;
	char		*name = NULL;
	char		buf[512];

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	/*
	 * Prepare the return flist
	 */
	ret_flistp = PIN_FLIST_COPY(i_flistp, ebufp);

	/*
	 * Print the message from the DM to the pinlog according to loglevel:
	 * 	0 - do not print anything
	 *	1 - print message as an error
	 * 	2 - print message as a warning (default)
	 */
	while( (flistp = PIN_FLIST_ELEM_TAKE_NEXT(ret_flistp, PIN_FLD_MESSAGES,
		 &elemid, 1,   &cookie, ebufp)) != (pin_flist_t *)NULL) {

		reason = (char *)PIN_FLIST_FLD_GET( flistp,
			PIN_FLD_REASON_CODE, 0, ebufp);
		switch (tax_return_loglevel) {
		case 0:
			/* do not print anything */
			break;
		case 1:
			PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_ERROR, reason);
			break;
		default:
			PIN_ERR_LOG_MSG( PIN_ERR_LEVEL_WARNING, reason);
			break;
		}
		PIN_FLIST_DESTROY_EX(&flistp, NULL);
	}

#ifdef ADD_USF_SURCHARGE
	/* an example of how to add a surcharge */
	fm_rate_pol_add_surcharge(i_flistp, ret_flistp, ebufp);
#endif

	/* Traverse the TAXES array from the return flist */
	elemid = 0; cookie = NULL;
	while ((tax_return_juris == 1) &&
		(t_flistp = PIN_FLIST_ELEM_GET_NEXT(ret_flistp, PIN_FLD_TAXES,
		&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		elemid2 = 0; cookie2 = NULL;
		/* Traverse the SUBTOTALS array from the TAXES element */
		while ((flistp = PIN_FLIST_ELEM_GET_NEXT(t_flistp,
			PIN_FLD_SUBTOTAL, &elemid2, 1, &cookie2, ebufp)) !=
			(pin_flist_t*)NULL) {

			descr = (char*)PIN_FLIST_FLD_GET(flistp,
				PIN_FLD_DESCR, 1, ebufp);
			name = (char*)PIN_FLIST_FLD_GET(flistp,
				PIN_FLD_NAME, 1, ebufp);
			if (descr && name) {
				pin_snprintf(buf, sizeof(buf), "%s; %s", name, descr);
				PIN_FLIST_FLD_SET(flistp, PIN_FLD_NAME,
					(void*)buf, ebufp);
			}
		}
	}

	/*
	 * Errors?
	 */
	if (PIN_ERR_IS_ERR(ebufp)) {
		PIN_FLIST_DESTROY_EX(&ret_flistp, NULL);
		PIN_ERR_LOG_EBUF(PIN_ERR_LEVEL_ERROR,
			"fm_rate_pol_post_tax error", ebufp);
	} else {
		*o_flistpp = ret_flistp;
	}

	return;
}


/*******************************************************************
 * Adding #ifdef effectively comments out the code, as the reference
 * to this fucntion is also based on this macro. This is done to 
 * nullify the compiler warning. 
 *
 *******************************************************************/

#ifdef ADD_USF_SURCHARGE
/*******************************************************************
 * fm_rate_pol_add_surcharge():
 *
 *    Sample code to add a surcharge (USF).
 *
 *******************************************************************/
static void 
fm_rate_pol_add_surcharge(
	pin_flist_t	*i_flistp,
	pin_flist_t	*o_flistp,
	pin_errbuf_t	*ebufp)
{
	pin_flist_t	*t_flistp = NULL;
	pin_flist_t	*s_flistp = NULL;
	pin_flist_t	*flistp = NULL;
	pin_cookie_t	cookie = NULL;
	pin_cookie_t	cookie2 = NULL;
	int32		elemid = 0;
	int32		elemid2 = 0;
	int32		cnt = 0;
	int32		type = PIN_RATE_TAX_JUR_FED;
	int32		subtype = 99;
	void		*vp = NULL;
	pin_decimal_t	*tax = NULL;
	pin_decimal_t	*taxes = NULL;
	pin_decimal_t	*gross = NULL;
	pin_decimal_t	*exempt = NULL;
	char		*desc = "Universal Service Fee";
	pin_decimal_t	*rate = NULL;  /* 4% */
	int32 		precision = 5;

	if (PIN_ERR_IS_ERR(ebufp))
		return;
	PIN_ERR_CLEAR_ERR(ebufp);

	rate = (pin_decimal_t *)NULL;
	rate = pbo_decimal_from_str("0.04",ebufp);
        exempt = pbo_decimal_from_str("0.0",ebufp);

	/* Traverse the TAXES array from the output flist */
	while ((t_flistp = PIN_FLIST_ELEM_GET_NEXT(o_flistp, PIN_FLD_TAXES,
		&elemid, 1, &cookie, ebufp)) != (pin_flist_t *)NULL) {

		tax = (pin_decimal_t *)NULL;
        	taxes = (pin_decimal_t *)NULL;
        	gross = (pin_decimal_t *)NULL;

		/* Get the number of SUBTOTALs elements */
		cnt = PIN_FLIST_ELEM_COUNT(t_flistp, PIN_FLD_SUBTOTAL, ebufp);

		/* Make a copy of the first SUBTOTAL to be used as a template */
		flistp = PIN_FLIST_ELEM_GET_NEXT(t_flistp, PIN_FLD_SUBTOTAL, 
			&elemid2, 1, &cookie2, ebufp);
		s_flistp = PIN_FLIST_COPY(flistp, ebufp);
		PIN_FLIST_ELEM_PUT(t_flistp, s_flistp, PIN_FLD_SUBTOTAL, 
			cnt, ebufp);
		
		/* Get total tax amount for the TAXES element */
		vp = PIN_FLIST_FLD_GET(t_flistp, PIN_FLD_TAX, 0, ebufp);
		taxes = pbo_decimal_copy( (pin_decimal_t*)vp, ebufp);

		/* Get taxable amount */
		vp = PIN_FLIST_FLD_GET(s_flistp,PIN_FLD_AMOUNT_GROSS,0,ebufp);
		gross = pbo_decimal_copy((pin_decimal_t*)vp, ebufp);

		/* Compute the tax */
		/*tax = gross * rate; */
	 	  tax = pbo_decimal_multiply( gross, rate, ebufp);	

		/* Set values into the new SUBTOTAL element */
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_TYPE, 
			(void*)&type, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_SUBTYPE,
			(void*)&subtype, ebufp);
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_DESCR,
			(void*)desc, ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_PERCENT,
			(void*)pbo_decimal_round( rate ,
			precision, ROUND_HALF_UP, ebufp), ebufp); 
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_TAX,
			(void*) pbo_decimal_round( tax,
			precision, ROUND_HALF_UP, ebufp), ebufp);
		PIN_FLIST_FLD_PUT(s_flistp, PIN_FLD_AMOUNT_TAXED,
			(void*) pbo_decimal_round(gross,
			precision, ROUND_HALF_UP, ebufp), ebufp);	
		PIN_FLIST_FLD_SET(s_flistp, PIN_FLD_AMOUNT_EXEMPT,
			(void*)exempt, ebufp);

		/* Add the tax to the running total */
		/* taxes += tax; */
		pbo_decimal_add_assign(taxes, tax, ebufp);
		PIN_FLIST_FLD_PUT(t_flistp, PIN_FLD_TAX,
			(void*)pbo_decimal_round( taxes , 
			precision, ROUND_HALF_UP, ebufp), ebufp);

		if( tax ) {
                        pbo_decimal_destroy(&tax);
                }
                if ( taxes ) {
                        pbo_decimal_destroy(&taxes);
                }
                if ( gross ) {
                        pbo_decimal_destroy(&gross);
                }

	}
        if ( exempt ) {
        	pbo_decimal_destroy(&exempt);
	}
	if ( rate ) {
		pbo_decimal_destroy(&rate);
	}
	return;
}
#endif
